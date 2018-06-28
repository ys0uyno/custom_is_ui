// hook_ui_welcome.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <atlimage.h>
#include "hook_ui_welcome.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define LOCAL_TEST1

#if defined LOCAL_TEST
#define TARGET_TITLE L"target_mfc_ui"
#else
#define TARGET_TITLE L"准备安装"
#endif

#define WM_QUERY_IS_WINDOW_RECT WM_USER + 11

HHOOK g_hhook_wnd = NULL;
HHOOK g_hhook_wnd_ret = NULL;
HHOOK g_hhook_msg = NULL;
HINSTANCE g_hinstance = NULL;

HWND g_hwnd;

#if defined LOCAL_TEST
HWND g_hwnd_btn2;
#else
#endif

BOOL g_bisdisabled;
BOOL g_bmousetrack = TRUE;

WNDPROC g_old_proc;
static bool g_subclassed = false;

#define IDC_BUTTON_CLOSE 1013
#define MARGIN 4
#define CLOSE_BUTTON_W 29
#define CLOSE_BUTTON_H 26
#define PICTURE_CONTROL_ID 0xffffffff

CString g_pictures_dir;

Gdiplus::Bitmap *g_pinstall_image;
Gdiplus::Bitmap *g_pclose_image;
Gdiplus::Bitmap *g_pbanner_image;

HWND g_install_button_hwnd;
CRect g_install_button_crect;
HWND g_close_button_hwnd;
CRect g_close_button_crect;

bool g_is_install_hover = false;
bool g_is_close_hover = false;
static bool g_bmouseleave_install_once = false;
static bool g_bmouseleave_close_once = false;
BOOL g_bmousetrack_install = TRUE;
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

void DrawPathRoundRect(HWND hwnd,
	Gdiplus::Graphics *pGraphics, Gdiplus::Rect r, Gdiplus::Color color, int radius, int width)
{
	int dia = 2 * radius;

	Gdiplus::Rect Corner(r.X, r.Y, dia, dia);

	Gdiplus::GraphicsPath path;

	// top left
	path.AddArc(Corner, 180, 90);

	// top right
	Corner.X += (int)(r.Width - dia - 1);
	path.AddArc(Corner, 270, 90);

	// bottom right
	Corner.Y += (int)(r.Height - dia - 1);
	path.AddArc(Corner, 0, 90);

	// bottom left
	Corner.X -= (int)(r.Width - dia - 1);
	path.AddArc(Corner, 90, 90);

	path.CloseFigure();

	Gdiplus::Pen pen(color, (Gdiplus::REAL)width);
	pen.SetAlignment(Gdiplus::PenAlignmentInset);
	pGraphics->DrawPath(&pen, &path);

	Gdiplus::Region region(&path);
	HDC hdc = GetDC(hwnd);
	Gdiplus::Graphics g(hdc);
	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	SetWindowRgn(hwnd, region.GetHRGN(&g), TRUE);
	ReleaseDC(hwnd, hdc);
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
#if defined LOCAL_TEST
			case 0x3e8:
#else
			case 0x1:
#endif
				{
					Gdiplus::Graphics g(lpDrawItemStruct->hDC);

					g_bisdisabled = (lpDrawItemStruct->itemState & ODS_DISABLED);

					Gdiplus::Rect dst_rect_disable(
						g_install_button_crect.left,
						g_install_button_crect.top,
						g_install_button_crect.Width(),
						g_install_button_crect.Height()
						);

					Gdiplus::Rect dst_rect_else(
						g_install_button_crect.left + 1,
						g_install_button_crect.top + 1,
						g_install_button_crect.Width() - 2,
						g_install_button_crect.Height() - 2
						);

					int src_width = g_pinstall_image->GetWidth();
					int src_height = g_pinstall_image->GetHeight() / 4;

					if (lpDrawItemStruct->itemState & ODS_DISABLED)
					{
						g.DrawImage(g_pinstall_image, dst_rect_disable,
							0, src_height * 3, src_width, src_height,
							Gdiplus::UnitPixel);
					}
					else if (lpDrawItemStruct->itemState & ODS_SELECTED)
					{
						g.DrawImage(g_pinstall_image, dst_rect_else,
							0, src_height * 2, src_width, src_height,
							Gdiplus::UnitPixel);
					}
					else if (g_is_install_hover)
					{
						g.DrawImage(g_pinstall_image, dst_rect_else,
							0, src_height * 1, src_width, src_height,
							Gdiplus::UnitPixel);
					}
					else
					{
						g.DrawImage(g_pinstall_image, dst_rect_else,
							0, src_height * 0, src_width, src_height,
							Gdiplus::UnitPixel);
					}
				}
				break;
#if defined LOCAL_TEST
			case 0x3ea:
				{
					CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
					int nSaveDC = pDC->SaveDC();
					CRect rect = lpDrawItemStruct->rcItem;
					pDC->RoundRect(0, 0, rect.right, rect.bottom, rect.Width() / 2, rect.Height());

					pDC->SetBkMode(TRANSPARENT);
					TCHAR button_text[MAX_PATH] = {0};
					GetWindowText(g_hwnd_btn2, button_text, MAX_PATH);
					pDC->DrawText(button_text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

					pDC->RestoreDC(nSaveDC);
				}
				break;
#endif
			case IDC_BUTTON_CLOSE:
				{
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
				}
				break;
			} // switch (lpDrawItemStruct->CtlID)
		} // case WM_DRAWITEM
		break;
	}

	return CallNextHookEx(g_hhook_wnd, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *p = (MSG *)lParam;
	TCHAR sz[MAX_PATH] = {0};
	PAINTSTRUCT ps;
	HDC hDC;

	switch (p->message)
	{
	case WM_PAINT:
		{
			HWND banner_image_hwnd = GetDlgItem(g_hwnd, PICTURE_CONTROL_ID);
			if (banner_image_hwnd && p->hwnd == banner_image_hwnd)
			{
				hDC = BeginPaint(p->hwnd, &ps);

				RECT client_rect;
				GetClientRect(g_hwnd, &client_rect);
				DWORD banner_pic_width = client_rect.right - client_rect.left;
				MoveWindow(banner_image_hwnd, client_rect.left, client_rect.top,
					banner_pic_width, (int)(banner_pic_width / 2.424), TRUE);

				HDC banner_image_hdc = GetDC(banner_image_hwnd);
				RECT banner_image_rect;
				GetClientRect(banner_image_hwnd, &banner_image_rect);
				CImage banner_image;
				banner_image.Load(g_pictures_dir + L"\\banner.bmp");
				banner_image.Draw(banner_image_hdc, banner_image_rect);
				ReleaseDC(banner_image_hwnd, banner_image_hdc);

				EndPaint(p->hwnd, &ps);
			}

			if (g_hwnd == p->hwnd)
			{
				HDC hdc = GetDC(p->hwnd);
				Gdiplus::Graphics g(hdc);
				RECT temp_rect;
				GetClientRect(g_hwnd, &temp_rect);
				CRect temp_crect(temp_rect);
				Gdiplus::Rect gdi_rect(0, 0, temp_crect.Width(), temp_crect.Height());
				g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
				DrawPathRoundRect(g_hwnd, &g, gdi_rect, NULL, 8, 0);
				ReleaseDC(g_hwnd, hdc);
			}
		}
		break;
	case WM_MOUSEHOVER:
		{
			if (g_install_button_hwnd == p->hwnd)
			{
				g_is_install_hover = true;
				g_bmouseleave_install_once = false;

				InvalidateRect(g_install_button_hwnd, &g_install_button_crect, TRUE);
			}

			if (g_close_button_hwnd == p->hwnd)
			{
				g_is_close_hover = true;
				g_bmouseleave_close_once = false;

				InvalidateRect(g_close_button_hwnd, &g_close_button_crect, TRUE);
			}
		}
		break;
	case WM_MOUSELEAVE:
		{
			if (g_install_button_hwnd == p->hwnd)
			{
				g_is_install_hover = false;
				if (!g_bmouseleave_install_once)
				{
					g_bmouseleave_install_once = true;

					InvalidateRect(g_install_button_hwnd, &g_install_button_crect, TRUE);
				}
				g_bmousetrack_install = TRUE;
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
			if (g_bmousetrack_install)
			{
				TRACKMOUSEEVENT csTME;
				csTME.cbSize = sizeof(csTME);
				csTME.dwFlags = TME_LEAVE | TME_HOVER;
				csTME.hwndTrack = g_install_button_hwnd;
				csTME.dwHoverTime = 10/*HOVER_DEFAULT*/;
				::_TrackMouseEvent(&csTME);
				g_bmousetrack_install = FALSE;
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
		}
		break;
	}

	return CallNextHookEx(g_hhook_msg, nCode, wParam, lParam);
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
	CWPRETSTRUCT *p = (CWPRETSTRUCT *)lParam;
	TCHAR sz[MAX_PATH] = {0};

	switch (p->message)
	{
	case WM_INITDIALOG:
		{
			if (g_subclassed)
			{
				break;
			}
			g_subclassed = true;

			OutputDebugString(L"after hook_ui_welcome WM_INITDIALOG");

			// need to call GdiplusShutdown
			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			ULONG_PTR gdiplusToken;
			Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

			AfxWinInit(GetModuleHandle(L"hook_ui_welcome"), NULL, GetCommandLine(), 0);
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

#if defined LOCAL_TEST
			HWND hwnd_btn = GetDlgItem(hwnd, 0x3e8);
#else
			HWND hwnd_btn = GetDlgItem(hwnd, 0x1);
#endif
			if (NULL == hwnd_btn)
				break;

			g_install_button_hwnd = hwnd_btn;

			hwnd_btn = GetDlgItem(hwnd, IDC_BUTTON_CLOSE);
			if (NULL == hwnd_btn)
				break;

			g_close_button_hwnd = hwnd_btn;

			long lstyle = GetWindowLong(g_install_button_hwnd, GWL_STYLE);
			lstyle |= BS_OWNERDRAW;
			SetWindowLong(g_install_button_hwnd, GWL_STYLE, lstyle);

			lstyle = GetWindowLong(g_close_button_hwnd, GWL_STYLE);
			lstyle |= BS_OWNERDRAW;
			SetWindowLong(g_close_button_hwnd, GWL_STYLE, lstyle);

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

			// after hook this control need to be painted just once
			RECT temp_rect;
			GetClientRect(g_install_button_hwnd, &temp_rect);
			InvalidateRect(g_install_button_hwnd, &temp_rect, TRUE);
			g_install_button_crect = temp_rect;

			GetClientRect(g_close_button_hwnd, &temp_rect);
			InvalidateRect(g_close_button_hwnd, &g_close_button_crect, TRUE);
			g_close_button_crect = temp_rect;

			// for DPI
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

			g_pinstall_image = Gdiplus::Bitmap::FromFile(g_pictures_dir + L"\\welcome_install_button.png");
			g_pclose_image = Gdiplus::Bitmap::FromFile(g_pictures_dir + L"\\close_button.png");
			g_pbanner_image = Gdiplus::Bitmap::FromFile(g_pictures_dir + L"\\banner.bmp");

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

			g_old_proc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)new_proc);
		}
		break;
	}

	return CallNextHookEx(g_hhook_wnd_ret, nCode, wParam, lParam);
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
	g_hhook_wnd = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstance, tid);
	g_hhook_wnd_ret = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hinstance, tid);
	g_hhook_msg = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndWelcomeHook()
{
	if (NULL != g_hhook_wnd)
		UnhookWindowsHookEx(g_hhook_wnd);

	if (NULL != g_hhook_wnd_ret)
		UnhookWindowsHookEx(g_hhook_wnd_ret);

	if (NULL != g_hhook_msg)
		UnhookWindowsHookEx(g_hhook_msg);
}
