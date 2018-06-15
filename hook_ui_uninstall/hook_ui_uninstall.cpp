// hook_ui_uninstall.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_ui_uninstall.h"
#include "transparent_button.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TARGET_TITLE L"×¼±¸Ð¶ÔØ"
#define IDC_TRANSPARENT_BUTTON_CLOSE 9
#define CORNER_SIZE 2

HINSTANCE g_hinstance = NULL;
HHOOK g_hhook1 = NULL;
HHOOK g_hhook2 = NULL;
HHOOK g_hhook3 = NULL;

HWND g_hwnd_btn1 = NULL;

class brush_class
{
public:
	brush_class()
	{
		m_brush_normal.CreateSolidBrush(RGB(0, 173, 239));
		m_brush_hover.CreateSolidBrush(RGB(4, 165, 227));
		m_brush_click.CreateSolidBrush(RGB(8, 157, 214));
		m_pen.CreatePen(PS_NULL, 1, RGB(0, 0, 0));
		m_font.CreatePointFont(150, _T("Arial"));
	}

	~brush_class()
	{
		m_brush_normal.DeleteObject();
		m_brush_hover.DeleteObject();
		m_brush_click.DeleteObject();
		m_pen.DeleteObject();
		m_font.DeleteObject();
	}

public:
	CBrush m_brush_normal;
	CBrush m_brush_hover;
	CBrush m_brush_click;
	CPen m_pen;
	CFont m_font;
};

brush_class g_brush;
BOOL g_bmousetrack = TRUE;

HWND g_hwnd = NULL;
static bool g_subclassed = false;

transparent_button g_tb_button_close;
HWND g_tb_button_close_hwnd = NULL;

WNDPROC g_old_proc;

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

// Chook_ui_uninstallApp

BEGIN_MESSAGE_MAP(Chook_ui_uninstallApp, CWinApp)
END_MESSAGE_MAP()


// Chook_ui_uninstallApp construction

Chook_ui_uninstallApp::Chook_ui_uninstallApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Chook_ui_uninstallApp object

Chook_ui_uninstallApp theApp;


// Chook_ui_uninstallApp initialization

BOOL Chook_ui_uninstallApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
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

LRESULT CALLBACK CallWndRetProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
	)
{
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

			AfxWinInit(GetModuleHandle(L"hook_ui_uninstall"), NULL, GetCommandLine(), 0);
			OutputDebugString(L"after hook_ui_uninstall WM_INITDIALOG");
			HWND hwnd = FindWindow(NULL, TARGET_TITLE);
			if (NULL == hwnd)
			{
				OutputDebugString(L"find window error");
				break;
			}

			g_hwnd = hwnd;

			HWND hwnd_btn = ::GetDlgItem(hwnd, 0x1);
			if (NULL == hwnd_btn)
				break;

			g_hwnd_btn1 = hwnd_btn;

			long lstyle = GetWindowLong(g_hwnd_btn1, GWL_STYLE);
			lstyle |= BS_OWNERDRAW;
			SetWindowLong(g_hwnd_btn1, GWL_STYLE, lstyle);

			g_hinstance = GetModuleHandle(L"hook_ui_uninstall.dll");

			DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
			DWORD dwNewStyle = WS_OVERLAPPED
				| WS_VISIBLE
				| WS_SYSMENU
				| WS_MINIMIZEBOX
				| WS_MAXIMIZEBOX
				| WS_CLIPCHILDREN
				| WS_CLIPSIBLINGS;
			dwNewStyle &= dwStyle;
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

			SkinH_Attach();
			break;
		}
	}

	return CallNextHookEx(g_hhook1, nCode, wParam, lParam);
}

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT *p = (CWPSTRUCT *)lParam;
	LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)p->lParam;

	switch (p->message)
	{
	case WM_DRAWITEM:
		{
			if (lpDrawItemStruct->CtlID == 0x1)
			{
				CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
				int nSaveDC = pDC->SaveDC();
				pDC->SelectObject(&g_brush.m_brush_normal);
				pDC->SelectObject(&g_brush.m_pen);
				pDC->SelectObject(&g_brush.m_font);
				if (lpDrawItemStruct->itemState & ODS_SELECTED)
				{
					// need called before call RoundRect
					pDC->SelectObject(&g_brush.m_brush_click);
				}

				CRect rect = lpDrawItemStruct->rcItem;
				pDC->RoundRect(0, 0, rect.right, rect.bottom, rect.Width() / 3, rect.Height());

				pDC->SetBkMode(TRANSPARENT);
				TCHAR button_text[MAX_PATH] = {0};
				GetWindowText(g_hwnd_btn1, button_text, MAX_PATH);
				pDC->SetTextColor(RGB(255, 255, 255));
				pDC->DrawText(button_text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				pDC->RestoreDC(nSaveDC);
			}

			if (lpDrawItemStruct->CtlID == IDC_TRANSPARENT_BUTTON_CLOSE)
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
		}
		break;
	}

	return CallNextHookEx(g_hhook2, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *p = (MSG *)lParam;
	HDC hDC;
	TCHAR sz[MAX_PATH] = {0};
	RECT rect;
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

			CDC* pDC = CDC::FromHandle(hDC);
			pDC->SelectObject(&g_brush.m_brush_hover);
			pDC->SelectObject(&g_brush.m_pen);
			pDC->SelectObject(&g_brush.m_font);
			GetClientRect(g_hwnd_btn1, &rect);
			pDC->RoundRect(0, 0, rect.right, rect.bottom, (rect.right - rect.left) / 2,
				rect.bottom - rect.top);

			pDC->SetBkMode(TRANSPARENT);
			TCHAR button_text[MAX_PATH] = {0};
			GetWindowText(g_hwnd_btn1, button_text, MAX_PATH);
			pDC->SetTextColor(RGB(255, 255, 255));
			pDC->DrawText(button_text, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			ReleaseDC(g_hwnd_btn1, hDC);
		}
		break;
	case WM_MOUSELEAVE:
		if (g_hwnd_btn1 == p->hwnd)
		{
			// OutputDebugString(L"WM_MOUSELEAVE");
			hDC = ::GetDC(g_hwnd_btn1);

			CDC* pDC = CDC::FromHandle(hDC);
			pDC->SelectObject(&g_brush.m_brush_normal);
			pDC->SelectObject(&g_brush.m_pen);
			pDC->SelectObject(&g_brush.m_font);
			GetClientRect(g_hwnd_btn1, &rect);
			pDC->RoundRect(0, 0, rect.right, rect.bottom, (rect.right - rect.left) / 2,
				rect.bottom - rect.top);

			pDC->SetBkMode(TRANSPARENT);
			TCHAR button_text[MAX_PATH] = {0};
			GetWindowText(g_hwnd_btn1, button_text, MAX_PATH);
			pDC->SetTextColor(RGB(255, 255, 255));
			pDC->DrawText(button_text, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			g_bmousetrack = TRUE;
			ReleaseDC(g_hwnd_btn1, hDC);
		}
		break;
	case WM_MOUSEMOVE:
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
	case WM_LBUTTONDOWN:
		if (g_hwnd == p->hwnd)
		{
			PostMessage(p->hwnd, WM_NCLBUTTONDOWN, HTCAPTION, p->lParam);
		}
		break;
	}

	return CallNextHookEx(g_hhook3, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void BegUninstallHook(HWND hwnd)
{
	DWORD tid = GetWindowThreadProcessId(hwnd, NULL);
	if (0 == tid)
	{
		OutputDebugString(L"GetWindowThreadProcessId return 0");
		return;
	}

	g_hinstance = GetModuleHandle(L"hook_ui_uninstall.dll");
	g_hhook1 = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hinstance, tid);
	g_hhook2 = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstance, tid);
	g_hhook3 = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndUninstallHook()
{
	if (NULL != g_hhook1)
		UnhookWindowsHookEx(g_hhook1);

	if (NULL != g_hhook2)
		UnhookWindowsHookEx(g_hhook2);

	if (NULL != g_hhook3)
		UnhookWindowsHookEx(g_hhook3);
}
