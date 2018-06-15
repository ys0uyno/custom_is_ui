// hook_ui_statusex.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_ui_statusex.h"
#include <atlimage.h>
#include "transparent_button.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TARGET_TITLE_INSTALL L"正在安装"
#define TARGET_TITLE_UNINSTALL L"正在卸载"
#define TARGET_TITLE L"InstallShield Wizard"

#define SHARED_MEMORY_BUFF_NAME L"_SHARED_MEMROY_YSOUYNO_"

#define IDC_TRANSPARENT_BUTTON_CLOSE 2
#define CORNER_SIZE 2

HINSTANCE g_hinstance = NULL;
HHOOK g_hhook1 = NULL;
HHOOK g_hhook2 = NULL;
HHOOK g_hhook3 = NULL;

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

	for (int i = 1; i <= 6; ++i)
	{
		CString bmp_file(/*path*/L"E:\\is_pictures\\");
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

		// after draw billboard picture repaint transparent button
		RECT temp_rect;
		GetClientRect(g_tb_button_close_hwnd, &temp_rect);
		InvalidateRect(g_tb_button_close_hwnd, &temp_rect, TRUE);

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

			AfxWinInit(GetModuleHandle(L"hook_ui_statusex"), NULL, GetCommandLine(), 0);
			OutputDebugString(L"after hook_ui_statusex WM_INITDIALOG");
			HWND hwnd = FindWindow(NULL, TARGET_TITLE);
			if (NULL == hwnd)
			{
				OutputDebugString(L"find window error");
				break;
			}

			g_hwnd = hwnd;

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
			ShowWindow(GetDlgItem(hwnd, 0x032), SW_HIDE);
			ShowWindow(GetDlgItem(hwnd, 0x514), SW_HIDE);
			ShowWindow(GetDlgItem(hwnd, 0x515), SW_HIDE);
			ShowWindow(GetDlgItem(hwnd, 0x033), SW_HIDE);
			ShowWindow(GetDlgItem(hwnd, 0x002), SW_HIDE);
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

			g_tb_button_close_hwnd = GetDlgItem(hwnd, IDC_TRANSPARENT_BUTTON_CLOSE);

			g_old_proc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)new_proc);

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

			SkinH_Attach();

			HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_bbrd_proc, NULL, 0, NULL);
			CloseHandle(h);

			break;
		}
	}

	return CallNextHookEx(g_hhook1, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *p = (MSG *)lParam;

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
		}
		break;
	case WM_LBUTTONDOWN:
		if (g_hwnd == p->hwnd)
		{
			PostMessage(p->hwnd, WM_NCLBUTTONDOWN, HTCAPTION, p->lParam);
		}
		break;
	}

	return CallNextHookEx(g_hhook2, nCode, wParam, lParam);
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
		}
		break;
	case WM_SIZE:
		{
			if (g_tb_button_close_hwnd)
			{
				RECT client_rect;
				GetClientRect(g_hwnd, &client_rect);

				MoveWindow(g_tb_button_close_hwnd,
					client_rect.right - 40 * 1 - 4,
					client_rect.top + 4,
					40,
					28,
					TRUE);
			}

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

	return CallNextHookEx(g_hhook3, nCode, wParam, lParam);
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
	g_hhook1 = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hinstance, tid);
	g_hhook2 = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstance, tid);
	g_hhook3 = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndStatusexHook()
{
	if (NULL != g_hhook1)
		UnhookWindowsHookEx(g_hhook1);

	if (NULL != g_hhook2)
		UnhookWindowsHookEx(g_hhook2);

	if (NULL != g_hhook3)
		UnhookWindowsHookEx(g_hhook3);
}
