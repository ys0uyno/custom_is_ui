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

HINSTANCE g_hinstance = NULL;
HHOOK g_hhook1 = NULL;

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
		CString bmp_file(path);
		bmp_file.AppendFormat(L"statusex%d.bmp", i);

		if (-1 == _taccess(bmp_file, 0))
		{
			if (6 == i) i = 0;
			continue;
		}

		CImage img;
		img.Load(bmp_file);
		img.Draw(hDC, rect);

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
			SkinH_Attach();

			HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_bbrd_proc, NULL, 0, NULL);
			CloseHandle(h);

			break;
		}
	}

	return CallNextHookEx(g_hhook1, nCode, wParam, lParam);
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
}

extern "C" __declspec(dllexport) void EndStatusexHook()
{
	if (NULL != g_hhook1)
		UnhookWindowsHookEx(g_hhook1);
}
