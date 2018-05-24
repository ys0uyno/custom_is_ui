// hook_ui_custom.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_ui_custom.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

// Chook_ui_customApp

BEGIN_MESSAGE_MAP(Chook_ui_customApp, CWinApp)
END_MESSAGE_MAP()


// Chook_ui_customApp construction

Chook_ui_customApp::Chook_ui_customApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Chook_ui_customApp object

Chook_ui_customApp theApp;


// Chook_ui_customApp initialization

BOOL Chook_ui_customApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
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
			break;
		}
	}

	return CallNextHookEx(g_hhook1, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void BegCustomHook(HWND hwnd)
{
	DWORD tid = GetWindowThreadProcessId(hwnd, NULL);
	if (0 == tid)
	{
		OutputDebugString(L"GetWindowThreadProcessId return 0");
		return;
	}

	g_hinstance = GetModuleHandle(L"hook_ui_custom.dll");
	g_hhook1 = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndCustomHook()
{
	if (NULL != g_hhook1)
		UnhookWindowsHookEx(g_hhook1);
}
