// hook_ui_welcome.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_ui_welcome.h"
#include "transparent_button.h"
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define LOCAL_TEST1

#if defined LOCAL_TEST
#define TARGET_TITLE L"target_mfc_ui"
#else
#define TARGET_TITLE L"准备安装"
#endif

HHOOK g_hhook1 = NULL;
HHOOK g_hhook2 = NULL;
HHOOK g_hhook3 = NULL;
HINSTANCE g_hinstance = NULL;

HWND g_hwnd;
HWND g_hwnd_btn1;
HWND g_hwnd_btn2;

static HICON g_hicon1 = NULL;
static HICON g_hicon2 = NULL;
static HICON g_hicon3 = NULL;
RECT g_caption_rect;
BOOL g_bispressed;
BOOL g_bisdisabled;
RECT g_rcitem;

BOOL g_bmousetrack = TRUE;

transparent_button g_tb_button_close;
HWND g_tb_button_close_hwnd = NULL;

#define IDC_TRANSPARENT_BUTTON_CLOSE 9

#define CORNER_SIZE 2

WNDPROC g_old_proc;

static bool g_subclassed = false;

class pen_class
{
public:
	pen_class()
	{
		m_pen.CreatePen(PS_NULL, 1, RGB(0, 0, 0));
	}

	~pen_class()
	{
		m_pen.DeleteObject();
	}

public:
	CPen m_pen;
};

pen_class g_pen;

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// Chook_ui_welcomeApp

BEGIN_MESSAGE_MAP(Chook_ui_welcomeApp, CWinApp)
END_MESSAGE_MAP()


// Chook_ui_welcomeApp construction

Chook_ui_welcomeApp::Chook_ui_welcomeApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Chook_ui_welcomeApp object

Chook_ui_welcomeApp theApp;


// Chook_ui_welcomeApp initialization

BOOL Chook_ui_welcomeApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

static void PrepareImageRect(HWND hButtonWnd, BOOL bHasTitle,
	RECT* rpItem, RECT* rpTitle, BOOL bIsPressed, DWORD dwWidth, DWORD dwHeight, RECT* rpImage)
{
	RECT rBtn;
	CopyRect(rpImage, rpItem);
	GetClientRect(hButtonWnd, &rBtn);
	if (bHasTitle == FALSE)
	{
		// Center image horizontally
		LONG rpImageWidth = rpImage->right - rpImage->left;
		rpImage->left += ((rpImageWidth - (long)dwWidth) / 2);
	}
	else
	{
		// Image must be placed just inside the focus rect
		LONG rpTitleWidth = rpTitle->right - rpTitle->left;
		rpTitle->right = rpTitleWidth - dwWidth - 0;
		rpTitle->left = 0;
		rpImage->left = rBtn.right - dwWidth - 0;
		// Center image vertically
		LONG rpImageHeight = rpImage->bottom - rpImage->top;
		rpImage->top += ((rpImageHeight - (long)dwHeight) / 2);
	}

	if (bIsPressed)
		OffsetRect(rpImage, 1, 1);
}

static void DrawTheIcon1(HWND hButtonWnd, HDC* dc, BOOL bHasTitle,
	RECT* rpItem, RECT* rpTitle, BOOL bIsPressed, BOOL bIsDisabled)
{
	RECT rImage;
	PrepareImageRect(hButtonWnd, bHasTitle, rpItem, rpTitle, bIsPressed,
		rpItem->right - rpItem->left, rpItem->bottom - rpItem->top, &rImage);

	rImage.left = 0;
	rImage.top = 0;
	rImage.right = 75;
	rImage.bottom = 75;

	DrawState(*dc,
		NULL,
		NULL,
		(LPARAM)g_hicon1,
		0,
		rImage.left,
		rImage.top,
		(rImage.right - rImage.left),
		(rImage.bottom - rImage.top),
		(bIsDisabled ? DSS_DISABLED : DSS_NORMAL) | DST_ICON
		);
}

static void DrawTheIcon2(HWND hButtonWnd, HDC* dc, BOOL bHasTitle,
	RECT* rpItem, RECT* rpTitle, BOOL bIsPressed, BOOL bIsDisabled)
{
	RECT rImage;
	PrepareImageRect(hButtonWnd, bHasTitle, rpItem, rpTitle, bIsPressed,
		rpItem->right - rpItem->left, rpItem->bottom - rpItem->top, &rImage);

	rImage.left = 0;
	rImage.top = 0;
	rImage.right = 40;
	rImage.bottom = 40;

	DrawState(*dc,
		NULL,
		NULL,
		(LPARAM)g_hicon2,
		0,
		rImage.left,
		rImage.top,
		(rImage.right - rImage.left),
		(rImage.bottom - rImage.top),
		(bIsDisabled ? DSS_DISABLED : DSS_NORMAL) | DST_ICON
		);
}

static void DrawTheIcon3(HWND hButtonWnd, HDC* dc, BOOL bHasTitle,
	RECT* rpItem, RECT* rpTitle, BOOL bIsPressed, BOOL bIsDisabled)
{
	RECT rImage;
	PrepareImageRect(hButtonWnd, bHasTitle, rpItem, rpTitle, bIsPressed,
		rpItem->right - rpItem->left, rpItem->bottom - rpItem->top, &rImage);

	rImage.left = 0;
	rImage.top = 0;
	rImage.right = 40;
	rImage.bottom = 40;

	DrawState(*dc,
		NULL,
		NULL,
		(LPARAM)g_hicon3,
		0,
		rImage.left,
		rImage.top,
		(rImage.right - rImage.left),
		(rImage.bottom - rImage.top),
		(bIsDisabled ? DSS_DISABLED : DSS_NORMAL) | DST_ICON
		);
}

void DrawBK(HDC dc, CImage *img, BUTTON_STATUS button_status, const transparent_button *tb, HWND tb_hwnd)
{
	if (!img)
	{
		return;
	}

	CRect rc;
	GetClientRect(tb_hwnd, &rc);
	CRect temp_rect;
	int nX = 0;
	int nY = 0;
	int nW = 0;
	int nH = 0;

	if (tb->m_b_autosize == true)
	{
		temp_rect.SetRect(0, 0, rc.Width(), rc.Height());
		if (img)
		{
			img->Draw(dc, temp_rect);
		}
	}
	else
	{
		if(button_status == BUTTON_NORMAL)
		{
			nW = tb->m_button_png_normal.width;
			nH = tb->m_button_png_normal.height;
		}
		else if (button_status == BUTTON_HOVER)
		{
			nW = tb->m_button_png_hover.width;
			nH = tb->m_button_png_hover.height;
		}
		else if (button_status == BUTTON_CLICK)
		{
			nW = tb->m_button_png_click.width;
			nH = tb->m_button_png_click.height;
		}
		else
		{
			nW = tb->m_button_png_disable.width;
			nH = tb->m_button_png_disable.height;
		}

		nX = (rc.Width() - nW) / 2;
		nY = (rc.Height() - nH) / 2;
		temp_rect.SetRect(nX, nY, nW + nX, nH + nY);
		if (img)
		{
			img->Draw(dc, temp_rect);
		}
	}
}

void DrawButtonText(HDC dc, const CString &strText, int nMove, BUTTON_STATUS button_status, HWND tb_hwnd)
{
	CRect rect;
	GetClientRect(tb_hwnd, &rect);
	rect.DeflateRect(nMove, nMove, 0, 0);

	CDC::FromHandle(dc)->SetBkMode(TRANSPARENT);

	if (button_status == BUTTON_NORMAL)
	{
		CDC::FromHandle(dc)->SetTextColor(RGB(30, 30, 30));
	}
	else if (button_status == BUTTON_HOVER)
	{
		CDC::FromHandle(dc)->SetTextColor(RGB(30, 30, 30));
	}
	else if (button_status == BUTTON_CLICK)
	{
		CDC::FromHandle(dc)->SetTextColor(RGB(30, 30, 30));
	}
	else
	{
		CDC::FromHandle(dc)->SetTextColor(RGB(100, 100, 100));
	}

	CDC::FromHandle(dc)->DrawText(strText, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
}

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT *p = (CWPSTRUCT *)lParam;
	LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)p->lParam;

	switch (p->message)
	{
	case WM_DRAWITEM:
		{
#if defined LOCAL_TEST
			if (lpDrawItemStruct->CtlID == 0x3e8)
#else
			if (lpDrawItemStruct->CtlID == 0x1)
#endif
			{
				CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
				int nSaveDC = pDC->SaveDC();
				pDC->SelectObject(&g_pen.m_pen);
				pDC->Ellipse(&lpDrawItemStruct->rcItem);

				g_caption_rect = lpDrawItemStruct->rcItem;
				g_bispressed  = (lpDrawItemStruct->itemState & ODS_SELECTED);
				g_bisdisabled = (lpDrawItemStruct->itemState & ODS_DISABLED);
				CopyRect(&g_rcitem, &lpDrawItemStruct->rcItem);
				DrawTheIcon1(g_hwnd_btn1, &lpDrawItemStruct->hDC, TRUE, &lpDrawItemStruct->rcItem,
					&g_caption_rect, g_bispressed, g_bisdisabled);

				if (lpDrawItemStruct->itemState & ODS_SELECTED)
				{
					DrawTheIcon3(g_hwnd_btn1, &lpDrawItemStruct->hDC, TRUE, &lpDrawItemStruct->rcItem,
						&g_caption_rect, g_bispressed, g_bisdisabled);
				}

				if (lpDrawItemStruct->itemState & ODS_DISABLED)
				{
					DrawTheIcon3(g_hwnd_btn1, &lpDrawItemStruct->hDC, TRUE, &lpDrawItemStruct->rcItem,
						&g_caption_rect, g_bispressed, FALSE);
				}

				pDC->RestoreDC(nSaveDC);
			}

#if defined LOCAL_TEST
			if (lpDrawItemStruct->CtlID == 0x3ea)
			{
				CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
				int nSaveDC = pDC->SaveDC();
				CRect rect = lpDrawItemStruct->rcItem;
				pDC->RoundRect(0, 0, rect.right, rect.bottom, rect.Width() / 2, rect.Height());

				// 重绘文本时不擦除背景，即透明模式
				// 如果选择OPAQUE（不透明），在文本四周有白色矩形边框
				pDC->SetBkMode(TRANSPARENT);
				TCHAR button_text[MAX_PATH] = {0};
				GetWindowText(g_hwnd_btn2, button_text, MAX_PATH);
				pDC->DrawText(button_text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				pDC->RestoreDC(nSaveDC);
			}
#endif
			switch (lpDrawItemStruct->CtlID)
			{
			case IDC_TRANSPARENT_BUTTON_CLOSE:
				{
					g_tb_button_close_hwnd = GetDlgItem(g_hwnd, IDC_TRANSPARENT_BUTTON_CLOSE);
					if (NULL == g_tb_button_close_hwnd)
						break;

					CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
					CRect rect = lpDrawItemStruct->rcItem;
					TCHAR strText[MAX_PATH] = {0};
					GetWindowText(g_tb_button_close_hwnd, strText, MAX_PATH);

					if(lpDrawItemStruct->itemState & ODS_DISABLED)
					{
						DrawBK(*pDC, g_tb_button_close.m_button_png_disable.pimage, BUTTON_DISABLE,
							&g_tb_button_close, g_tb_button_close_hwnd);
					}
					else if(lpDrawItemStruct->itemState & ODS_SELECTED
						|| (g_tb_button_close.m_b_ishover && g_tb_button_close.m_b_isclicked))
					{
						DrawBK(*pDC, g_tb_button_close.m_button_png_click.pimage, BUTTON_CLICK,
							&g_tb_button_close, g_tb_button_close_hwnd);
					}
					else if(g_tb_button_close.m_b_ishover)
					{
						DrawBK(*pDC, g_tb_button_close.m_button_png_hover.pimage, BUTTON_HOVER,
							&g_tb_button_close, g_tb_button_close_hwnd);
					}
					else
					{
						DrawBK(*pDC, g_tb_button_close.m_button_png_normal.pimage, BUTTON_NORMAL,
							&g_tb_button_close, g_tb_button_close_hwnd);
					}

					CString strTemp(strText);
					strTemp.Remove(' ');
					if (!strTemp.IsEmpty())
					{
						if(lpDrawItemStruct->itemState & ODS_DISABLED)
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_DISABLE, g_tb_button_close_hwnd);
						}
						else if(lpDrawItemStruct->itemState & ODS_SELECTED
							|| (g_tb_button_close.m_b_ishover && g_tb_button_close.m_b_isclicked))
						{
							DrawButtonText(*pDC, strText, 1, BUTTON_CLICK, g_tb_button_close_hwnd);
						}
						else if(g_tb_button_close.m_b_ishover)
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_HOVER, g_tb_button_close_hwnd);
						}
						else
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_NORMAL, g_tb_button_close_hwnd);
						}
					}
				}
				break;
			}
			break;
		}
	case WM_PAINT:
		{
			HDC hdc = ::GetDC(g_hwnd);
			CDC	*pDC = CDC::FromHandle(hdc);

			RECT rect;
			GetClientRect(g_hwnd, &rect);

			// outside of window border
			CPen *old_pen = NULL;
			CPen new_pen1(PS_SOLID, 1, RGB(27, 147, 186));
			old_pen = pDC->SelectObject(&new_pen1);

			pDC->MoveTo(rect.left, CORNER_SIZE);
			pDC->LineTo(CORNER_SIZE, rect.top);
			pDC->LineTo(rect.right - CORNER_SIZE - 1, rect.top);
			pDC->LineTo(rect.right - 1, CORNER_SIZE);
			pDC->LineTo(rect.right - 1, rect.bottom - CORNER_SIZE - 1);
			pDC->LineTo(rect.right - CORNER_SIZE - 1, rect.bottom - 1);
			pDC->LineTo(CORNER_SIZE, rect.bottom - 1);
			pDC->LineTo(rect.left, rect.bottom - CORNER_SIZE - 1);
			pDC->LineTo(rect.left, CORNER_SIZE);

			// fill in gaps
			pDC->MoveTo(rect.left + 1, CORNER_SIZE);
			pDC->LineTo(CORNER_SIZE + 1, rect.top);
			pDC->MoveTo(rect.right - CORNER_SIZE - 1, rect.top + 1);
			pDC->LineTo(rect.right - 1, CORNER_SIZE + 1);
			pDC->MoveTo(rect.right - 2, rect.bottom - CORNER_SIZE - 1);
			pDC->LineTo(rect.right - CORNER_SIZE - 1, rect.bottom - 1);
			pDC->MoveTo(CORNER_SIZE, rect.bottom - 2);
			pDC->LineTo(rect.left, rect.bottom - CORNER_SIZE - 2);

			pDC->SelectObject(old_pen);

			// inside of window border
			CPen new_pen2(PS_SOLID, 1, RGB(196, 234, 247));
			old_pen = pDC->SelectObject(&new_pen2);

			pDC->MoveTo(rect.left + 1, CORNER_SIZE + 1);
			pDC->LineTo(CORNER_SIZE + 1, rect.top + 1);
			pDC->LineTo(rect.right - CORNER_SIZE - 2, rect.top + 1);
			pDC->LineTo(rect.right - 2, CORNER_SIZE + 1);
			pDC->LineTo(rect.right - 2, rect.bottom - CORNER_SIZE - 2);
			pDC->LineTo(rect.right - CORNER_SIZE - 2, rect.bottom - 2);
			pDC->LineTo(CORNER_SIZE + 1, rect.bottom - 2);
			pDC->LineTo(rect.left + 1, rect.bottom - CORNER_SIZE - 2);
			pDC->LineTo(rect.left + 1, CORNER_SIZE + 1);
		}
		break;
	case WM_SIZE:
		{
			if (p->hwnd == g_hwnd)
			{
				// remove the four sharp corners of the border
				if (p->wParam != SIZE_MAXIMIZED)
				{
					RECT rc;
					GetClientRect(g_hwnd, &rc);

					CRgn rgn;
					CPoint points[8] =
					{
						CPoint(rc.left, CORNER_SIZE),
						CPoint(CORNER_SIZE, rc.top),
						CPoint(rc.right - CORNER_SIZE, rc.top),
						CPoint(rc.right, CORNER_SIZE),
						CPoint(rc.right, rc.bottom - CORNER_SIZE - 1),
						CPoint(rc.right - CORNER_SIZE - 1, rc.bottom),
						CPoint(CORNER_SIZE + 1, rc.bottom),
						CPoint(rc.left, rc.bottom - CORNER_SIZE - 1)
					};

					int nPolyCounts[1] = {8};
					int dd = rgn.CreatePolyPolygonRgn(points, nPolyCounts, 1, WINDING);
					SetWindowRgn(g_hwnd, rgn, TRUE);
				}
				else
				{
					SetWindowRgn(g_hwnd, NULL, FALSE);
				}
			}
		}
		break;
	}

	return CallNextHookEx(g_hhook1, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *p = (MSG *)lParam;
	HDC hDC;
	TCHAR sz[MAX_PATH] = {0};
	CRect crect;
	RECT rc;
	CPoint cpoint;
	PAINTSTRUCT ps;

	switch (p->message)
	{
	case WM_MOUSEHOVER:
		if (g_hwnd_btn1 == p->hwnd)
		{
			hDC = ::GetDC(g_hwnd_btn1);
			swprintf_s(sz, L"WM_MOUSEHOVER, g_hwnd_btn: %x, hDC: %x, point: %d, %d",
				g_hwnd_btn1, hDC, GET_X_LPARAM(p->lParam), GET_Y_LPARAM(p->lParam));
			OutputDebugString(sz);
			DrawTheIcon2(g_hwnd_btn1, &hDC, TRUE, &g_rcitem, &g_caption_rect, g_bispressed, FALSE);
			ReleaseDC(g_hwnd_btn1, hDC);
		}
		break;
	case WM_MOUSELEAVE:
		if (g_hwnd_btn1 == p->hwnd)
		{
			// OutputDebugString(L"WM_MOUSELEAVE");
			hDC = ::GetDC(g_hwnd_btn1);
			DrawTheIcon1(g_hwnd_btn1, &hDC, TRUE, &g_rcitem, &g_caption_rect, g_bispressed, FALSE);
			g_bmousetrack = TRUE;
			ReleaseDC(g_hwnd_btn1, hDC);
		}
		break;
	case WM_MOUSEMOVE:
		GetWindowRect(g_hwnd_btn1, &rc);
		crect = rc;
		GetCursorPos(&cpoint);
		if (crect.PtInRect(cpoint))
		{
			// OutputDebugString(L"in control");
		}
		else
		{
			// OutputDebugString(L"NOT in control");
		}

		if (g_bmousetrack)
		{
			TRACKMOUSEEVENT csTME;
			csTME.cbSize = sizeof(csTME);
			csTME.dwFlags = TME_LEAVE | TME_HOVER;
			csTME.hwndTrack = g_hwnd_btn1;
			csTME.dwHoverTime = 10/*HOVER_DEFAULT*/;
			::_TrackMouseEvent(&csTME);
			g_bmousetrack = FALSE;
		}
		break;
	case WM_LBUTTONDOWN:
		OutputDebugString(L"WM_LBUTTONDOWN hook");
		// how to move dialog without title bar
		// https://www.cnblogs.com/huhu0013/p/4640728.html
		// SendMessage(g_hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, p->lParam);

		// it is best to use the following PostMessage, because clicking on the ok
		// button when using SendMessage requires more than one click
		if (g_hwnd == p->hwnd)
		{
			PostMessage(p->hwnd, WM_NCLBUTTONDOWN, HTCAPTION, p->lParam);
		}
		break;
	case WM_PAINT:
		{
			HDC hdc = ::GetDC(g_hwnd);
			CDC	*pDC = CDC::FromHandle(hdc);

			RECT rect;
			GetClientRect(g_hwnd, &rect);

			// outside of window border
			CPen *old_pen = NULL;
			CPen new_pen1(PS_SOLID, 1, RGB(27, 147, 186));
			old_pen = pDC->SelectObject(&new_pen1);

			pDC->MoveTo(rect.left, CORNER_SIZE);
			pDC->LineTo(CORNER_SIZE, rect.top);
			pDC->LineTo(rect.right - CORNER_SIZE - 1, rect.top);
			pDC->LineTo(rect.right - 1, CORNER_SIZE);
			pDC->LineTo(rect.right - 1, rect.bottom - CORNER_SIZE - 1);
			pDC->LineTo(rect.right - CORNER_SIZE - 1, rect.bottom - 1);
			pDC->LineTo(CORNER_SIZE, rect.bottom - 1);
			pDC->LineTo(rect.left, rect.bottom - CORNER_SIZE - 1);
			pDC->LineTo(rect.left, CORNER_SIZE);

			// fill in gaps
			pDC->MoveTo(rect.left + 1, CORNER_SIZE);
			pDC->LineTo(CORNER_SIZE + 1, rect.top);
			pDC->MoveTo(rect.right - CORNER_SIZE - 1, rect.top + 1);
			pDC->LineTo(rect.right - 1, CORNER_SIZE + 1);
			pDC->MoveTo(rect.right - 2, rect.bottom - CORNER_SIZE - 1);
			pDC->LineTo(rect.right - CORNER_SIZE - 1, rect.bottom - 1);
			pDC->MoveTo(CORNER_SIZE, rect.bottom - 2);
			pDC->LineTo(rect.left, rect.bottom - CORNER_SIZE - 2);

			pDC->SelectObject(old_pen);

			// inside of window border
			CPen new_pen2(PS_SOLID, 1, RGB(196, 234, 247));
			old_pen = pDC->SelectObject(&new_pen2);

			pDC->MoveTo(rect.left + 1, CORNER_SIZE + 1);
			pDC->LineTo(CORNER_SIZE + 1, rect.top + 1);
			pDC->LineTo(rect.right - CORNER_SIZE - 2, rect.top + 1);
			pDC->LineTo(rect.right - 2, CORNER_SIZE + 1);
			pDC->LineTo(rect.right - 2, rect.bottom - CORNER_SIZE - 2);
			pDC->LineTo(rect.right - CORNER_SIZE - 2, rect.bottom - 2);
			pDC->LineTo(CORNER_SIZE + 1, rect.bottom - 2);
			pDC->LineTo(rect.left + 1, rect.bottom - CORNER_SIZE - 2);
			pDC->LineTo(rect.left + 1, CORNER_SIZE + 1);

			// draw banner image
			HWND banner_image_hwnd = GetDlgItem(g_hwnd, 0xffffffff);
			if (banner_image_hwnd && p->hwnd == banner_image_hwnd)
			{
				hDC = BeginPaint(p->hwnd, &ps);

				RECT client_rect;
				GetClientRect(g_hwnd, &client_rect);
				DWORD banner_width = client_rect.right - client_rect.left;

				MoveWindow(banner_image_hwnd,
					client_rect.left,
					client_rect.top,
					banner_width,
					(int)(banner_width / 2.424),
					TRUE);

				HDC banner_image_hdc = GetDC(banner_image_hwnd);
				RECT banner_image_rect;
				GetClientRect(banner_image_hwnd, &banner_image_rect);

				CImage banner_image;
				banner_image.Load(L"E:\\is_pictures\\banner.bmp");
				banner_image.Draw(banner_image_hdc, banner_image_rect);
				ReleaseDC(banner_image_hwnd, banner_image_hdc);

				EndPaint(p->hwnd, &ps);
			}
		}
		break;
	}

	return CallNextHookEx(g_hhook3, nCode, wParam, lParam);
}

DWORD WINAPI detach_skin_proc(PVOID arg)
{
	HANDLE h = CreateEvent(NULL, TRUE, FALSE, L"quit_skin");
	OutputDebugString(L"quit_skin event wait for signal");
	WaitForSingleObject(h, INFINITE);
	SkinH_Detach();
	OutputDebugString(L"quit_skin event thread quit");
	CloseHandle(h);
	return 0;
}

LRESULT CALLBACK new_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_CTLCOLORBTN)
	{
		HBRUSH hbrush;
		hbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
		SetBkMode((HDC)wParam, TRANSPARENT);
		return ((LRESULT)hbrush);
	}

	if (Msg == WM_DESTROY)
	{
		g_subclassed = false;
	}

	return CallWindowProc(g_old_proc, hWnd, Msg, wParam, lParam);
}

LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	TCHAR sz[MAX_PATH] = {0};

	CWPRETSTRUCT *p = (CWPRETSTRUCT *)lParam;
	switch (p->message)
	{
	case WM_INITDIALOG:
		{
			if (g_subclassed)
			{
				break;
			}
			g_subclassed = true;

			AfxWinInit(GetModuleHandle(L"hook_ui_welcome"), NULL, GetCommandLine(), 0);
			OutputDebugString(L"after hook_ui_welcome WM_INITDIALOG");
			HWND hwnd = FindWindow(NULL, TARGET_TITLE);
			if (NULL == hwnd)
			{
				OutputDebugString(L"find window error");
				break;
			}

			g_hwnd = hwnd;

#if defined LOCAL_TEST
			HWND hwnd_btn = ::GetDlgItem(hwnd, 0x3e8);
#else
			HWND hwnd_btn = ::GetDlgItem(hwnd, 0x1);
#endif
			if (NULL == hwnd_btn)
				break;

			g_hwnd_btn1 = hwnd_btn;

			long lstyle = GetWindowLong(g_hwnd_btn1, GWL_STYLE);
			lstyle |= BS_OWNERDRAW;
			SetWindowLong(g_hwnd_btn1, GWL_STYLE, lstyle);

#if defined LOCAL_TEST
			hwnd_btn = ::GetDlgItem(hwnd, 0x3ea);
			g_hwnd_btn2 = hwnd_btn;

			lstyle = GetWindowLong(g_hwnd_btn2, GWL_STYLE);
			lstyle |= BS_OWNERDRAW;
			SetWindowLong(g_hwnd_btn2, GWL_STYLE, lstyle);
#endif

			DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
			DWORD dwNewStyle = WS_OVERLAPPED
				| WS_VISIBLE
				| WS_SYSMENU
				| WS_MINIMIZEBOX
				| WS_MAXIMIZEBOX
				| WS_CLIPCHILDREN
				| WS_CLIPSIBLINGS;
			dwNewStyle &= dwStyle; // & will remove style
			SetWindowLong(hwnd, GWL_STYLE, dwNewStyle);

			DWORD dwExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			DWORD dwNewExStyle = WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR;
			dwNewExStyle &= dwExStyle;
			SetWindowLong(hwnd, GWL_EXSTYLE, dwNewExStyle);

			SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

			RECT client_rect;
			GetClientRect(hwnd, &client_rect);

			g_tb_button_close.Create(
				L"",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				CRect(
				client_rect.right - 40 * 1 - 4,
				client_rect.top + 4,
				client_rect.right - 40 * 1 - 4 + 40,
				client_rect.top + 28
				),
				CWnd::FromHandle(hwnd),
				IDC_TRANSPARENT_BUTTON_CLOSE
				);
			g_tb_button_close.SetAutoSize(false);
			g_tb_button_close.Load(IDB_CLOSE, 39);

			g_old_proc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)new_proc);

			g_hinstance = GetModuleHandle(L"hook_ui_welcome.dll");
			if(g_hicon1 == NULL)
				g_hicon1 = (HICON)LoadImage(g_hinstance, MAKEINTRESOURCE(IDI_NORMAL), IMAGE_ICON, 75, 75, 0);
			if(g_hicon2 == NULL)
				g_hicon2 = (HICON)LoadImage(g_hinstance, MAKEINTRESOURCE(IDI_HOVER), IMAGE_ICON, 75, 75, 0);
			if(g_hicon3 == NULL)
				g_hicon3 = (HICON)LoadImage(g_hinstance, MAKEINTRESOURCE(IDI_CLICK), IMAGE_ICON, 75, 75, 0);

			// after hook this control need to be painted just once
			RECT g_hwnd_btn1_rect;
			GetClientRect(g_hwnd_btn1, &g_hwnd_btn1_rect);
			InvalidateRect(g_hwnd_btn1, &g_hwnd_btn1_rect, TRUE);

			// for dpi
			HWND banner_image_hwnd = GetDlgItem(hwnd, 0xffffffff);
			if (banner_image_hwnd)
			{
				RECT client_rect;
				GetClientRect(hwnd, &client_rect);
				DWORD banner_width = client_rect.right - client_rect.left;

				MoveWindow(banner_image_hwnd,
					client_rect.left,
					client_rect.top,
					banner_width,
					(int)(banner_width / 2.424),
					TRUE);
			}

			// reposition control position
			// 0x3ea - agree agreement check box control
			// 0x3eb - agreement syslink control
			// 0x3f4 - custom options syslink control
			// 0x001 - install button
			RECT window_rect;
			GetWindowRect(hwnd, &window_rect);
			CRect window_crect(window_rect);

			RECT agree_check_rect;
			HWND agree_check_hwnd = GetDlgItem(hwnd, 0x3ea);
			if (agree_check_hwnd)
			{
				GetClientRect(agree_check_hwnd, &agree_check_rect);
				CRect temp_crect(agree_check_rect);

				MoveWindow(agree_check_hwnd,
					temp_crect.left + 10,
					temp_crect.top + (window_crect.Height() - temp_crect.Height() - 10),
					temp_crect.Width(),
					temp_crect.Height(),
					TRUE);
			}

			HWND agreement_syslink_hwnd = GetDlgItem(hwnd, 0x3eb);
			if (agreement_syslink_hwnd && agree_check_hwnd)
			{
				RECT temp_rect;
				GetClientRect(agreement_syslink_hwnd, &temp_rect);
				CRect temp_crect(temp_rect);

				MoveWindow(agreement_syslink_hwnd,
					temp_crect.left + (agree_check_rect.right - agree_check_rect.left + 4) + 10,
					temp_crect.top + (window_crect.Height() - temp_crect.Height() - 12),
					temp_crect.Width(),
					temp_crect.Height(),
					TRUE);
			}

			HWND options_syslink_hwnd = GetDlgItem(hwnd, 0x3f4);
			if (options_syslink_hwnd)
			{
				RECT temp_rect;
				GetClientRect(options_syslink_hwnd, &temp_rect);
				CRect temp_crect(temp_rect);

				MoveWindow(options_syslink_hwnd,
					temp_crect.left + (window_crect.Width() - temp_crect.Width()) - 10,
					temp_crect.top + (window_crect.Height() - temp_crect.Height() - 12),
					temp_crect.Width(),
					temp_crect.Height(),
					TRUE);
			}

			HWND install_button_hwnd = GetDlgItem(hwnd, 0x001);
			if (install_button_hwnd)
			{
				RECT temp_rect;
				GetClientRect(install_button_hwnd, &temp_rect);
				CRect temp_crect(temp_rect);

				MoveWindow(install_button_hwnd,
					temp_crect.left + (window_crect.Width() - temp_crect.Width()) / 2,
					temp_crect.top +
					(window_crect.Height() - temp_crect.Height() - (agree_check_rect.bottom - agree_check_rect.top) - 16),
					temp_crect.Width(),
					temp_crect.Height(),
					TRUE);
			}

			SkinH_Attach();

			// seem there is no way to hook WM_QUIT, WM_DESTROY and WM_CLOSE
			// see: https://stackoverflow.com/questions/13915332/hook-window-message-loop-wm-close
			/*HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)detach_skin_proc, NULL, 0, NULL);
			CloseHandle(h);*/
			break;
		}
	}

	return CallNextHookEx(g_hhook2, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void BegWelcomeHook(HWND hwnd)
{
	DWORD tid = GetWindowThreadProcessId(hwnd, NULL);
	if (0 == tid)
	{
		OutputDebugString(L"GetWindowThreadProcessId return 0");
		return;
	}

	g_hinstance = GetModuleHandle(L"hook_ui_welcome.dll");
	g_hhook1 = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstance, tid);
	g_hhook2 = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hinstance, tid);
	g_hhook3 = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndWelcomeHook()
{
	if (NULL != g_hhook1)
		UnhookWindowsHookEx(g_hhook1);

	if (NULL != g_hhook2)
		UnhookWindowsHookEx(g_hhook2);

	if (NULL != g_hhook3)
		UnhookWindowsHookEx(g_hhook3);
}
