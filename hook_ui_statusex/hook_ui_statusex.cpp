// hook_ui_statusex.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_ui_statusex.h"
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TARGET_TITLE_INSTALL L"正在安装"
#define TARGET_TITLE_UNINSTALL L"正在卸载"
#define TARGET_TITLE L"InstallShield Wizard"

#define SHARED_MEMORY_BUFF_NAME L"_SHARED_MEMROY_YSOUYNO_"

HINSTANCE g_hinstance = NULL;
HHOOK g_hhook_wnd = NULL;
HHOOK g_hhook_wnd_ret = NULL;
HHOOK g_hhook_msg = NULL;

HWND g_hwnd = NULL;

WNDPROC g_old_proc;
static bool g_subclassed = false;

CString g_pictures_dir;

Gdiplus::Bitmap *g_pbk_image;
Gdiplus::Bitmap *g_pclose_image;

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

// Chook_ui_statusexApp

BEGIN_MESSAGE_MAP(Chook_ui_statusexApp, CWinApp)
END_MESSAGE_MAP()


// Chook_ui_statusexApp construction

Chook_ui_statusexApp::Chook_ui_statusexApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Chook_ui_statusexApp object

Chook_ui_statusexApp theApp;


// Chook_ui_statusexApp initialization

BOOL Chook_ui_statusexApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

int get_current_dir(CString &path)
{
	TCHAR temp[MAX_PATH] = {0};
	DWORD dwret = GetModuleFileName(NULL, temp, MAX_PATH);
	if (0 == dwret)
	{
		return -1;
	}

	TCHAR drive[_MAX_DRIVE] = { 0 };
	TCHAR dir[_MAX_DIR] = { 0 };
	TCHAR file[_MAX_FNAME] = { 0 };
	TCHAR ext[_MAX_EXT] = { 0 };
	errno_t err = 0;
	err = _tsplitpath_s(temp, drive, _MAX_DRIVE, dir, _MAX_DIR, file, _MAX_FNAME, ext, _MAX_EXT);
	if (0 != err)
	{
		return -1;
	}

	path += drive;
	path += dir;

	return 0;
}

DWORD WINAPI thread_bbrd_proc(PVOID arg)
{
	CString path;
	get_current_dir(path);
	if (0 == path.GetLength())
	{
		return -1;
	}

	TCHAR sz[MAX_PATH] = {0};
	_stprintf_s(sz, L"current dir: %s", path);
	OutputDebugString(sz);

	Sleep(1000);

	HWND hwnd_window_install = FindWindow(NULL, TARGET_TITLE_INSTALL);
	HWND hwnd_window_uninstall = FindWindow(NULL, TARGET_TITLE_UNINSTALL);
	if (NULL == hwnd_window_install && NULL == hwnd_window_uninstall)
	{
		_stprintf_s(sz, L"%s", L"BBRD FindWindow failed");
		OutputDebugString(sz);
		return -1;
	}

	HWND hwnd_window = (hwnd_window_install == NULL) ? hwnd_window_uninstall : hwnd_window_install;

	HWND hwnd_pic = ::GetDlgItem(hwnd_window, 0x34);
	if (NULL == hwnd_pic)
	{
		_stprintf_s(sz, L"GetDlgItem 0x34 failed: %d", GetLastError());
		OutputDebugString(sz);
		return -1;
	}

	HDC hDC = GetDC(hwnd_pic);
	if (NULL == hDC)
	{
		_stprintf_s(sz, L"GetDC failed: %d", GetLastError());
		OutputDebugString(sz);
		return -1;
	}

	RECT rect;
	BOOL ret = GetClientRect(hwnd_pic, &rect);
	if (FALSE == ret)
	{
		_stprintf_s(sz, L"GetClientRect failed: %d", GetLastError());
		OutputDebugString(sz);
		ReleaseDC(hwnd_pic, hDC);
		return -1;
	}

	HANDLE h = CreateEvent(NULL, TRUE, FALSE, L"stop_bbrd_event");

	TCHAR temp_dir[MAX_PATH] = {0};
	if (0 == GetEnvironmentVariable(L"TEMP", temp_dir, MAX_PATH))
	{
		printf("GetEnvironmentVariable failed: %d\n", GetLastError());
	}
	CString pictures_dir = temp_dir;

	for (int i = 1; i <= 6; ++i)
	{
		CString bmp_file(pictures_dir + L"\\is_pictures\\");
		bmp_file.AppendFormat(L"statusex%d.bmp", i);

		if (-1 == _taccess(bmp_file, 0))
		{
			// if no picture found, here is endless loop, setup.exe will crash,
			// "stop_bbrd_event" will not be got and skinh.dll will not be free
			if (6 == i) i = 0;
			continue;
		}

		CImage img;
		img.Load(bmp_file);
		img.Draw(hDC, rect);

		g_pbk_image = Gdiplus::Bitmap::FromFile(bmp_file);

		RECT temp_rect;
		GetClientRect(g_close_button_hwnd, &temp_rect);
		InvalidateRect(g_close_button_hwnd, &temp_rect, TRUE);

		DWORD ret = WaitForSingleObject(h, 3000); // 3s is bbrd interval time
		if (WAIT_OBJECT_0 == ret)
		{
			// stop signaled
			OutputDebugString(L"get stop_bbrd_event");
			break;
		}
		else if (WAIT_TIMEOUT == ret)
		{
			// show next picture
			OutputDebugString(L"show next picture");
			continue;
		}
	}

	CloseHandle(h);
	ReleaseDC(hwnd_pic, hDC);

	h = CreateEvent(NULL, TRUE, FALSE, L"stop_bbrd_event_done");
	SetEvent(h);
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
			case 0x2:
				{
					RECT temp_rect;
					GetClientRect(g_hwnd, &temp_rect);
					MoveWindow(
						g_close_button_hwnd,
						temp_rect.right - 4 - 29,
						temp_rect.top + 4,
						g_close_button_crect.Width(),
						g_close_button_crect.Height(),
						TRUE);

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
						g_close_button_crect.Width() + 4,
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
							g_pbk_image,
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
			}
		}
		break;
	}

	return CallNextHookEx(g_hhook_wnd, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *p = (MSG *)lParam;

	switch (p->message)
	{
	case WM_MOUSEHOVER:
		{
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

			OutputDebugString(L"after hook_ui_statusex WM_INITDIALOG");

			// need call GdiplusShutdown
			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			ULONG_PTR gdiplusToken;
			Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

			AfxWinInit(GetModuleHandle(L"hook_ui_statusex"), NULL, GetCommandLine(), 0);
			OutputDebugString(L"after hook_ui_statusex WM_INITDIALOG");
			HWND hwnd = FindWindow(NULL, TARGET_TITLE);
			if (NULL == hwnd)
			{
				OutputDebugString(L"find window error");
				break;
			}

			g_hwnd = hwnd;

			HWND hwnd_btn = GetDlgItem(hwnd, 0x2);
			if (NULL == hwnd_btn)
				break;

			g_close_button_hwnd = hwnd_btn;

			long lstyle = GetWindowLong(g_close_button_hwnd, GWL_STYLE);
			lstyle |= BS_OWNERDRAW;
			SetWindowLong(g_close_button_hwnd, GWL_STYLE, lstyle);

			// read shared memory to get IS window rect
			RECT is_window_rect = {0};
			HANDLE hmap = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, SHARED_MEMORY_BUFF_NAME);
			if (hmap)
			{
				LPVOID pmap = MapViewOfFile(hmap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
				if (pmap)
				{
					memcpy((void *)&is_window_rect, pmap, sizeof(is_window_rect));
					_stprintf_s(sz, L"IS window rect: %d, %d, %d, %d",
						is_window_rect.left, is_window_rect.top, is_window_rect.right, is_window_rect.bottom);
					OutputDebugString(sz);

					UnmapViewOfFile(pmap);
				}

				CloseHandle(hmap);
			}
			else
			{
				_stprintf_s(sz, L"OpenFileMapping failed: %d", GetLastError());
				OutputDebugString(sz);
			}

			if (0 != is_window_rect.right - is_window_rect.left &&
				0 != is_window_rect.bottom - is_window_rect.top)
			{
				MoveWindow(hwnd,
					is_window_rect.left,
					is_window_rect.top,
					is_window_rect.right - is_window_rect.left,
					is_window_rect.bottom - is_window_rect.top,
					TRUE
					);
			}

			// hide some control
			ShowWindow(GetDlgItem(hwnd, 0x032), SWP_HIDEWINDOW);
			ShowWindow(GetDlgItem(hwnd, 0x514), SW_HIDE);
			ShowWindow(GetDlgItem(hwnd, 0x515), SWP_HIDEWINDOW);
			ShowWindow(GetDlgItem(hwnd, 0x033), SWP_HIDEWINDOW);
			/*ShowWindow(GetDlgItem(hwnd, 0x002), SWP_HIDEWINDOW);*/
			SetDlgItemText(hwnd, 0x002, L"");

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

			dwStyle = GetWindowLong(hwnd, GWL_STYLE);
			dwStyle |= WS_CLIPCHILDREN;
			SetWindowLong(hwnd, GWL_STYLE, dwStyle);

			SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

			MoveWindow(g_close_button_hwnd, 0, 0, 29, 26, TRUE);

			// after hook this control need to be painted just once
			RECT temp_rect;
			GetClientRect(g_close_button_hwnd, &temp_rect);
			g_close_button_crect = temp_rect;
			InvalidateRect(g_close_button_hwnd, &g_close_button_crect, TRUE);

			TCHAR temp_dir[MAX_PATH] = {0};
			if (0 == GetEnvironmentVariable(L"TEMP", temp_dir, MAX_PATH))
			{
				printf("GetEnvironmentVariable failed: %d\n", GetLastError());
			}
			g_pictures_dir = temp_dir;
			g_pictures_dir += L"\\is_pictures";

			g_pclose_image = Gdiplus::Bitmap::FromFile(g_pictures_dir + L"\\close_button.png");

			HWND banner_image_hwnd = GetDlgItem(hwnd, 0x34);
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

			// repositioning control
			// 0x578 top text
			// 0x5aa middle text
			// 0x5dc bottom progress bar
			RECT is_client_rect = {0};
			GetClientRect(hwnd, &is_client_rect);
			CRect is_client_crect(is_client_rect);

			RECT rect_progress_bar = {0};
			HWND hwnd_progress_bar = GetDlgItem(hwnd, 0x5dc);
			if (hwnd_progress_bar)
			{
				GetClientRect(hwnd_progress_bar, &rect_progress_bar);
				MoveWindow(hwnd_progress_bar,
					is_client_crect.Height() / 8,
					(is_client_crect.Height() / 8) * 7,
					is_client_crect.Width() - is_client_crect.Height() / 4,
					rect_progress_bar.bottom - rect_progress_bar.top,
					TRUE);
			}

			RECT rect_status_text2 = {0};
			HWND hwnd_status_text2 = GetDlgItem(hwnd, 0x5aa);
			DWORD font_height = 0;
			if (hwnd_status_text2)
			{
				GetClientRect(hwnd_status_text2, &rect_status_text2);
				font_height = rect_status_text2.bottom - rect_status_text2.top;
				if (font_height > 0)
				{
					MoveWindow(hwnd_status_text2,
						is_client_crect.Height() / 8,
						(is_client_crect.Height() / 8) * 7 - (font_height / 2 + font_height) * 1,
						is_client_crect.Width() - is_client_crect.Height() / 4,
						rect_status_text2.bottom - rect_status_text2.top,
						TRUE);
				}
			}

			RECT rect_status_text1 = {0};
			HWND hwnd_status_text1 = GetDlgItem(hwnd, 0x578);
			if (hwnd_status_text1)
			{
				GetClientRect(hwnd_status_text1, &rect_status_text1);
				if (font_height > 0)
				{
					MoveWindow(hwnd_status_text1,
						is_client_crect.Height() / 8,
						(is_client_crect.Height() / 8) * 7 - (font_height / 2 + font_height) * 2,
						is_client_crect.Width() - is_client_crect.Height() / 4,
						rect_status_text1.bottom - rect_status_text1.top,
						TRUE);
				}
			}

			// hide branding
			HWND hwnd_hide = GetDlgItem(hwnd, 0x2c6);
			if (hwnd_hide && font_height > 0)
			{
				MoveWindow(hwnd_hide,
					is_client_crect.left + 2,
					(is_client_crect.Height() / 3) * 2,
					is_client_crect.Width() / 2,
					font_height * 2,
					TRUE);

				SetDlgItemText(hwnd, 0x2c6, L"");
			}

			g_old_proc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)new_proc);

			/*SkinH_Attach();*/

			HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_bbrd_proc, NULL, 0, NULL);
			CloseHandle(h);

			break;
		}
	}

	return CallNextHookEx(g_hhook_wnd_ret, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void BegStatusexHook(HWND hwnd)
{
	DWORD tid = GetWindowThreadProcessId(hwnd, NULL);
	if (0 == tid)
	{
		OutputDebugString(L"GetWindowThreadProcessId return 0");
		return;
	}

	g_hinstance = GetModuleHandle(L"hook_ui_statusex.dll");
	g_hhook_wnd = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstance, tid);
	g_hhook_wnd_ret = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hinstance, tid);
	g_hhook_msg = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndStatusexHook()
{
	if (NULL != g_hhook_wnd)
		UnhookWindowsHookEx(g_hhook_wnd);

	if (NULL != g_hhook_wnd_ret)
		UnhookWindowsHookEx(g_hhook_wnd_ret);

	if (NULL != g_hhook_msg)
		UnhookWindowsHookEx(g_hhook_msg);
}
