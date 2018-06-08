// hook_ui_finish.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_ui_finish.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TARGET_TITLE L"²Ù×÷Íê³É"

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
		m_brush_normal.CreateSolidBrush(RGB(10, 220, 200));
		m_brush_hover.CreateSolidBrush(RGB(230, 220, 100));
		m_brush_click.CreateSolidBrush(RGB(130, 120, 0));
	}

	~brush_class()
	{
		m_brush_normal.DeleteObject();
		m_brush_hover.DeleteObject();
		m_brush_click.DeleteObject();
	}

public:
	CBrush m_brush_normal;
	CBrush m_brush_hover;
	CBrush m_brush_click;
};

brush_class g_brush;
BOOL g_bmousetrack = TRUE;

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

// Chook_ui_finishApp

BEGIN_MESSAGE_MAP(Chook_ui_finishApp, CWinApp)
END_MESSAGE_MAP()


// Chook_ui_finishApp construction

Chook_ui_finishApp::Chook_ui_finishApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Chook_ui_finishApp object

Chook_ui_finishApp theApp;


// Chook_ui_finishApp initialization

BOOL Chook_ui_finishApp::InitInstance()
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
			OutputDebugString(L"after hook_ui_finish WM_INITDIALOG");
			HWND hwnd = FindWindow(NULL, TARGET_TITLE);
			if (NULL == hwnd)
			{
				OutputDebugString(L"find window error");
				break;
			}

			HWND hwnd_btn = ::GetDlgItem(hwnd, 0x1);
			if (NULL == hwnd_btn)
				break;

			g_hwnd_btn1 = hwnd_btn;

			long lstyle = GetWindowLong(g_hwnd_btn1, GWL_STYLE);
			lstyle |= BS_OWNERDRAW;
			SetWindowLong(g_hwnd_btn1, GWL_STYLE, lstyle);

			g_hinstance = GetModuleHandle(L"hook_ui_finish.dll");

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
				if (lpDrawItemStruct->itemState & ODS_SELECTED)
				{
					// need called before call RoundRect
					pDC->SelectObject(&g_brush.m_brush_click);
				}

				CRect rect = lpDrawItemStruct->rcItem;
				pDC->RoundRect(0, 0, rect.right, rect.bottom, rect.Width() / 2, rect.Height());

				pDC->SetBkMode(TRANSPARENT);
				TCHAR button_text[MAX_PATH] = {0};
				GetWindowText(g_hwnd_btn1, button_text, MAX_PATH);
				pDC->DrawText(button_text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				pDC->RestoreDC(nSaveDC);
			}
			break;
		}
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
			GetClientRect(g_hwnd_btn1, &rect);
			pDC->RoundRect(0, 0, rect.right, rect.bottom, (rect.right - rect.left) / 2,
				rect.bottom - rect.top);

			pDC->SetBkMode(TRANSPARENT);
			TCHAR button_text[MAX_PATH] = {0};
			GetWindowText(g_hwnd_btn1, button_text, MAX_PATH);
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
			GetClientRect(g_hwnd_btn1, &rect);
			pDC->RoundRect(0, 0, rect.right, rect.bottom, (rect.right - rect.left) / 2,
				rect.bottom - rect.top);

			pDC->SetBkMode(TRANSPARENT);
			TCHAR button_text[MAX_PATH] = {0};
			GetWindowText(g_hwnd_btn1, button_text, MAX_PATH);
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
	}

	return CallNextHookEx(g_hhook3, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void BegFinishHook(HWND hwnd)
{
	DWORD tid = GetWindowThreadProcessId(hwnd, NULL);
	if (0 == tid)
	{
		OutputDebugString(L"GetWindowThreadProcessId return 0");
		return;
	}

	g_hinstance = GetModuleHandle(L"hook_ui_finish.dll");
	g_hhook1 = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hinstance, tid);
	g_hhook2 = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstance, tid);
	g_hhook3 = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndFinishHook()
{
	if (NULL != g_hhook1)
		UnhookWindowsHookEx(g_hhook1);

	if (NULL != g_hhook2)
		UnhookWindowsHookEx(g_hhook2);

	if (NULL != g_hhook3)
		UnhookWindowsHookEx(g_hhook3);
}
