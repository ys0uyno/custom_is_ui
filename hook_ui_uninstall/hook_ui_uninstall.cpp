// hook_ui_uninstall.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_ui_uninstall.h"
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TARGET_TITLE L"×¼±¸Ð¶ÔØ"
#define CORNER_SIZE 2

#define WM_QUERY_IS_WINDOW_RECT WM_USER + 11

HINSTANCE g_hinstance = NULL;
HHOOK g_hhook_wnd = NULL;
HHOOK g_hhook_msg = NULL;
HHOOK g_hhook_wnd_ret = NULL;

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
BOOL g_bmousetrack_uninstall = TRUE;

HWND g_hwnd = NULL;

WNDPROC g_old_proc;
static bool g_subclassed = false;

#define IDC_BUTTON_CLOSE 1013
#define MARGIN 4
#define CLOSE_BUTTON_W 29
#define CLOSE_BUTTON_H 26
#define PICTURE_CONTROL_ID 0xffffffff

CString g_pictures_dir;

Gdiplus::Bitmap *g_pclose_image;
Gdiplus::Bitmap *g_pbanner_image;

HWND g_uninstall_button_hwnd;
CRect g_uninstall_button_crect;
HWND g_close_button_hwnd;
CRect g_close_button_crect;

bool g_is_close_hover = false;
static bool g_bmouseleave_close_once = false;
BOOL g_bmousetrack_close = TRUE;

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

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT *p = (CWPSTRUCT *)lParam;
	LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)p->lParam;

	switch (p->message)
	{
	case WM_DRAWITEM:
		{
			switch (lpDrawItemStruct->CtlID)
			{
			case 0x1:
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
					GetWindowText(g_uninstall_button_hwnd, button_text, MAX_PATH);
					pDC->SetTextColor(RGB(255, 255, 255));
					pDC->DrawText(button_text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

					pDC->RestoreDC(nSaveDC);
				}
				break;
			case IDC_BUTTON_CLOSE:
				{
					OutputDebugString(L"IDC_BUTTON_CLOSE");
					Gdiplus::Graphics g(lpDrawItemStruct->hDC);

					Gdiplus::Rect dst_rect(
						g_close_button_crect.left,
						g_close_button_crect.top,
						g_close_button_crect.Width(),
						g_close_button_crect.Height()
						);

					Gdiplus::Rect dst_rect_banner_copy(
						g_close_button_crect.left,
						g_close_button_crect.top,
						g_close_button_crect.Width() + 4, // for that border
						g_close_button_crect.Height()
						);

					int src_width = g_pclose_image->GetWidth();
					int src_height = g_pclose_image->GetHeight() / 4;

					if (lpDrawItemStruct->itemState & ODS_SELECTED)
					{
						g.DrawImage(g_pclose_image, dst_rect,
							0, src_height * 2, src_width, src_height,
							Gdiplus::UnitPixel);
					}
					else if (g_is_close_hover)
					{
						g.DrawImage(g_pclose_image, dst_rect,
							0, src_height * 1, src_width, src_height,
							Gdiplus::UnitPixel);
					}
					else
					{
						RECT temp_rect;
						GetClientRect(g_hwnd, &temp_rect);

						g.DrawImage(
							g_pbanner_image,
							dst_rect_banner_copy,
							temp_rect.right - 4 - 29,
							temp_rect.top + 4,
							src_width,
							src_height,
							Gdiplus::UnitPixel
							);

						g.DrawImage(g_pclose_image, dst_rect,
							0, src_height * 0, src_width, src_height,
							Gdiplus::UnitPixel);
					}
				} // case IDC_BUTTON_CLOSE
				break;
			}
		}
		break;
	}

	return CallNextHookEx(g_hhook_wnd, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *p = (MSG *)lParam;
	HDC hDC;
	RECT rect;
	PAINTSTRUCT ps;

	switch (p->message)
	{
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
			HWND banner_image_hwnd = GetDlgItem(g_hwnd, PICTURE_CONTROL_ID);
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
				banner_image.Load(g_pictures_dir + L"\\banner.bmp");
				banner_image.Draw(banner_image_hdc, banner_image_rect);
				ReleaseDC(banner_image_hwnd, banner_image_hdc);

				EndPaint(p->hwnd, &ps);
			}
		}
		break;
	case WM_MOUSEHOVER:
		{
			if (g_uninstall_button_hwnd == p->hwnd)
			{
				hDC = ::GetDC(g_uninstall_button_hwnd);

				CDC* pDC = CDC::FromHandle(hDC);
				pDC->SelectObject(&g_brush.m_brush_hover);
				pDC->SelectObject(&g_brush.m_pen);
				pDC->SelectObject(&g_brush.m_font);
				GetClientRect(g_uninstall_button_hwnd, &rect);
				pDC->RoundRect(0, 0, rect.right, rect.bottom, (rect.right - rect.left) / 2,
					rect.bottom - rect.top);

				pDC->SetBkMode(TRANSPARENT);
				TCHAR button_text[MAX_PATH] = {0};
				GetWindowText(g_uninstall_button_hwnd, button_text, MAX_PATH);
				pDC->SetTextColor(RGB(255, 255, 255));
				pDC->DrawText(button_text, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				ReleaseDC(g_uninstall_button_hwnd, hDC);
			}

			if (g_close_button_hwnd == p->hwnd)
			{
				OutputDebugString(L"IDC_BUTTON_CLOSE WM_MOUSEHOVER");
				g_is_close_hover = true;
				g_bmouseleave_close_once = false;

				InvalidateRect(g_close_button_hwnd, &g_close_button_crect, TRUE);
			}
		}
		break;
	case WM_MOUSELEAVE:
		{
			if (g_uninstall_button_hwnd == p->hwnd)
			{
				hDC = ::GetDC(g_uninstall_button_hwnd);

				CDC* pDC = CDC::FromHandle(hDC);
				pDC->SelectObject(&g_brush.m_brush_normal);
				pDC->SelectObject(&g_brush.m_pen);
				pDC->SelectObject(&g_brush.m_font);
				GetClientRect(g_uninstall_button_hwnd, &rect);
				pDC->RoundRect(0, 0, rect.right, rect.bottom, (rect.right - rect.left) / 2,
					rect.bottom - rect.top);

				pDC->SetBkMode(TRANSPARENT);
				TCHAR button_text[MAX_PATH] = {0};
				GetWindowText(g_uninstall_button_hwnd, button_text, MAX_PATH);
				pDC->SetTextColor(RGB(255, 255, 255));
				pDC->DrawText(button_text, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				g_bmousetrack_uninstall = TRUE;
				ReleaseDC(g_uninstall_button_hwnd, hDC);
			}

			if (g_close_button_hwnd == p->hwnd)
			{
				g_is_close_hover = false;
				if (!g_bmouseleave_close_once)
				{
					g_bmouseleave_close_once = true;

					InvalidateRect(g_close_button_hwnd, &g_close_button_crect, TRUE);
				}
				g_bmousetrack_close = TRUE;
			}
		}
		break;
	case WM_MOUSEMOVE:
		{
			if (g_bmousetrack_uninstall)
			{
				TRACKMOUSEEVENT csTME;
				csTME.cbSize = sizeof(csTME);
				csTME.dwFlags = TME_LEAVE | TME_HOVER;
				csTME.hwndTrack = g_uninstall_button_hwnd;
				csTME.dwHoverTime = 10/*HOVER_DEFAULT*/;
				::_TrackMouseEvent(&csTME);
				g_bmousetrack_uninstall = FALSE;
			}

			if (g_bmousetrack_close)
			{
				TRACKMOUSEEVENT csTME;
				csTME.cbSize = sizeof(csTME);
				csTME.dwFlags = TME_LEAVE | TME_HOVER;
				csTME.hwndTrack = g_close_button_hwnd;
				csTME.dwHoverTime = 10/*HOVER_DEFAULT*/;
				::_TrackMouseEvent(&csTME);
				g_bmousetrack_close = FALSE;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			if (g_hwnd == p->hwnd)
			{
				PostMessage(p->hwnd, WM_NCLBUTTONDOWN, HTCAPTION, p->lParam);
			}
		}
		break;
	}

	return CallNextHookEx(g_hhook_msg, nCode, wParam, lParam);
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

			OutputDebugString(L"after hook_ui_uninstall WM_INITDIALOG");

			// need to call GdiplusShutdown
			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			ULONG_PTR gdiplusToken;
			Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

			AfxWinInit(GetModuleHandle(L"hook_ui_uninstall"), 0, GetCommandLine(), 0);

			HWND hwnd = FindWindow(NULL, TARGET_TITLE);
			if (NULL == hwnd)
			{
				OutputDebugString(L"find window error");
				break;
			}

			g_hwnd = hwnd;

			HWND win32_hwnd = FindWindow(NULL, L"win32_dll_loader");
			if (NULL != win32_hwnd)
			{
				SendMessage(win32_hwnd, WM_QUERY_IS_WINDOW_RECT, 0, 0);
			}

			HWND hwnd_btn = GetDlgItem(hwnd, 0x1);
			if (NULL == hwnd_btn)
				break;

			g_uninstall_button_hwnd = hwnd_btn;

			hwnd_btn = GetDlgItem(hwnd, IDC_BUTTON_CLOSE);
			if (NULL == hwnd_btn)
				break;

			g_close_button_hwnd = hwnd_btn;

			long lstyle = GetWindowLong(g_uninstall_button_hwnd, GWL_STYLE);
			lstyle |= BS_OWNERDRAW;
			SetWindowLong(g_uninstall_button_hwnd, GWL_STYLE, lstyle);

			lstyle = GetWindowLong(g_close_button_hwnd, GWL_STYLE);
			lstyle |= BS_OWNERDRAW;
			SetWindowLong(g_close_button_hwnd, GWL_STYLE, lstyle);

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

			// after hook this control need to be painted just once
			RECT temp_rect;
			GetClientRect(g_uninstall_button_hwnd, &temp_rect);
			InvalidateRect(g_uninstall_button_hwnd, &temp_rect, TRUE);
			g_uninstall_button_crect = temp_rect;

			GetClientRect(g_close_button_hwnd, &temp_rect);
			InvalidateRect(g_close_button_hwnd, &g_close_button_crect, TRUE);
			g_close_button_crect = temp_rect;

			HWND banner_image_hwnd = GetDlgItem(hwnd, PICTURE_CONTROL_ID);
			if (banner_image_hwnd)
			{
				GetClientRect(hwnd, &temp_rect);
				DWORD banner_width = temp_rect.right - temp_rect.left;

				MoveWindow(banner_image_hwnd,
					temp_rect.left,
					temp_rect.top,
					banner_width,
					(int)(banner_width / 2.424),
					TRUE);

				MoveWindow(g_close_button_hwnd,
					temp_rect.right - MARGIN - CLOSE_BUTTON_W,
					temp_rect.top + MARGIN,
					CLOSE_BUTTON_W,
					CLOSE_BUTTON_H,
					TRUE);

				GetClientRect(g_close_button_hwnd, &temp_rect);
				g_close_button_crect = temp_rect;
			}

			TCHAR temp_dir[MAX_PATH] = {0};
			if (0 == GetEnvironmentVariable(L"TEMP", temp_dir, MAX_PATH))
			{
				printf("GetEnvironmentVariable failed: %d\n", GetLastError());
			}
			g_pictures_dir = temp_dir;
			g_pictures_dir += L"\\is_pictures";

			g_pclose_image = Gdiplus::Bitmap::FromFile(g_pictures_dir + L"\\close_button.png");
			g_pbanner_image = Gdiplus::Bitmap::FromFile(g_pictures_dir + L"\\banner.bmp");

			g_old_proc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)new_proc);
		}
		break;
	}

	return CallNextHookEx(g_hhook_wnd_ret, nCode, wParam, lParam);
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
	g_hhook_wnd = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstance, tid);
	g_hhook_msg = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstance, tid);
	g_hhook_wnd_ret = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndUninstallHook()
{
	if (NULL != g_hhook_wnd)
		UnhookWindowsHookEx(g_hhook_wnd);

	if (NULL != g_hhook_msg)
		UnhookWindowsHookEx(g_hhook_msg);

	if (NULL != g_hhook_wnd_ret)
		UnhookWindowsHookEx(g_hhook_wnd_ret);
}
