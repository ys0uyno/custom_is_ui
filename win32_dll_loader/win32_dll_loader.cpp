// win32_dll_loader.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "win32_dll_loader.h"

#define MAX_LOADSTRING 100

HMODULE g_welcome_hmodule;
typedef void (*BEGWELCOMEHOOK)(HWND);
typedef void (*ENDWELCOMEHOOK)();

HMODULE g_statusex_hmodule;
typedef void (*BEGSTATUSEXHOOK)(HWND);
typedef void (*ENDSTATUSEXHOOK)();

HMODULE g_finish_hmodule;
typedef void (*BEGFINISHHOOK)(HWND);
typedef void (*ENDFINISHHOOK)();

HMODULE g_custom_hmodule;
typedef void (*BEGCUSTOMHOOK)(HWND);
typedef void (*ENDCUSTOMHOOK)();

HMODULE g_uninstall_hmodule;
typedef void (*BEGUNINSTALLHOOK)(HWND);
typedef void (*ENDUNINSTALLHOOK)();

#define WM_WELCOME_UI_BEG WM_USER + 1
#define WM_WELCOME_UI_END WM_USER + 2

#define WM_STATUSEX_UI_BEG WM_USER + 3
#define WM_STATUSEX_UI_END WM_USER + 4

#define WM_FINISH_UI_BEG WM_USER + 5
#define WM_FINISH_UI_END WM_USER + 6

#define WM_CUSTOM_UI_BEG WM_USER + 7
#define WM_CUSTOM_UI_END WM_USER + 8

#define WM_UNINSTALL_UI_BEG WM_USER + 9
#define WM_UNINSTALL_UI_END WM_USER + 10

#define WM_QUERY_IS_WINDOW_RECT WM_USER + 11

#define TARGET_WELCOME_TILE L"准备安装"
#define TARGET_UNINSTALL_TILE L"准备卸载"

#define SHARED_MEMORY_BUFF_SIZE 4096
#define SHARED_MEMORY_BUFF_NAME L"_SHARED_MEMROY_YSOUYNO_"

BEGWELCOMEHOOK g_beg_welcome_hook;
ENDWELCOMEHOOK g_end_welcome_hook;

BEGSTATUSEXHOOK g_beg_statusex_hook;
ENDSTATUSEXHOOK g_end_statusex_hook;

BEGFINISHHOOK g_beg_finish_hook;
ENDFINISHHOOK g_end_finish_hook;

BEGCUSTOMHOOK g_beg_custom_hook;
ENDCUSTOMHOOK g_end_custom_hook;

BEGUNINSTALLHOOK g_beg_uninstall_hook;
ENDUNINSTALLHOOK g_end_uninstall_hook;

HANDLE g_hmap = NULL;
LPVOID g_pmap = NULL;

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	g_welcome_hmodule = LoadLibrary(L"hook_ui_welcome");
	if (NULL == g_welcome_hmodule)
	{
		OutputDebugString(L"LoadLibrary hook_ui_welcome failed");
	}

	g_beg_welcome_hook = (BEGWELCOMEHOOK)GetProcAddress(g_welcome_hmodule, "BegWelcomeHook");
	if (NULL == g_beg_welcome_hook)
	{
		OutputDebugString(L"g_beg_welcome_hook null");
	}

	g_end_welcome_hook = (ENDWELCOMEHOOK)GetProcAddress(g_welcome_hmodule, "EndWelcomeHook");
	if (NULL == g_end_welcome_hook)
	{
		OutputDebugString(L"g_end_welcome_hook null");
	}

	g_statusex_hmodule = LoadLibrary(L"hook_ui_statusex");
	if (NULL == g_statusex_hmodule)
	{
		OutputDebugString(L"LoadLibrary hook_ui_statusex failed");
	}

	g_beg_statusex_hook = (BEGWELCOMEHOOK)GetProcAddress(g_statusex_hmodule, "BegStatusexHook");
	if (NULL == g_beg_statusex_hook)
	{
		OutputDebugString(L"g_beg_statusex_hook null");
	}

	g_end_statusex_hook = (ENDWELCOMEHOOK)GetProcAddress(g_statusex_hmodule, "EndStatusexHook");
	if (NULL == g_end_statusex_hook)
	{
		OutputDebugString(L"g_end_statusex_hook null");
	}

	g_finish_hmodule = LoadLibrary(L"hook_ui_finish");
	if (NULL == g_finish_hmodule)
	{
		OutputDebugString(L"LoadLibrary hook_ui_finish failed");
	}

	g_beg_finish_hook = (BEGFINISHHOOK)GetProcAddress(g_finish_hmodule, "BegFinishHook");
	if (NULL == g_beg_finish_hook)
	{
		OutputDebugString(L"g_beg_finish_hook null");
	}

	g_end_finish_hook = (ENDFINISHHOOK)GetProcAddress(g_finish_hmodule, "EndFinishHook");
	if (NULL == g_end_finish_hook)
	{
		OutputDebugString(L"g_end_finish_hook null");
	}

	g_custom_hmodule = LoadLibrary(L"hook_ui_custom");
	if (NULL == g_custom_hmodule)
	{
		OutputDebugString(L"LoadLibrary hook_ui_custom failed");
	}

	g_beg_custom_hook = (BEGCUSTOMHOOK)GetProcAddress(g_custom_hmodule, "BegCustomHook");
	if (NULL == g_beg_custom_hook)
	{
		OutputDebugString(L"g_beg_custom_hook null");
	}

	g_end_custom_hook = (ENDCUSTOMHOOK)GetProcAddress(g_custom_hmodule, "EndCustomHook");
	if (NULL == g_end_custom_hook)
	{
		OutputDebugString(L"g_end_custom_hook null");
	}

	g_uninstall_hmodule = LoadLibrary(L"hook_ui_uninstall");
	if (NULL == g_uninstall_hmodule)
	{
		OutputDebugString(L"LoadLibrary hook_ui_uninstall failed");
	}

	g_beg_uninstall_hook = (BEGUNINSTALLHOOK)GetProcAddress(g_uninstall_hmodule, "BegUninstallHook");
	if (NULL == g_beg_uninstall_hook)
	{
		OutputDebugString(L"g_beg_uninstall_hook null");
	}

	g_end_uninstall_hook = (ENDUNINSTALLHOOK)GetProcAddress(g_uninstall_hmodule, "EndUninstallHook");
	if (NULL == g_end_uninstall_hook)
	{
		OutputDebugString(L"g_end_uninstall_hook null");
	}

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32_DLL_LOADER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32_DLL_LOADER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32_DLL_LOADER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN32_DLL_LOADER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_MINIMIZE);
   UpdateWindow(hWnd);

   return TRUE;
}

void stop_bbrd_event()
{
	HANDLE h = CreateEvent(NULL, TRUE, FALSE, L"stop_bbrd_event");
	SetEvent(h);
	CloseHandle(h);

	h = CreateEvent(NULL, TRUE, FALSE, L"stop_bbrd_event_done");
	OutputDebugString(L"wait stop_bbrd_event_done");
	WaitForSingleObject(h, 2000);
	OutputDebugString(L"get stop_bbrd_event_done");
	CloseHandle(h);
}

void free_all_dlls()
{
	if (g_welcome_hmodule)
		FreeLibrary(g_welcome_hmodule);
	if (g_statusex_hmodule)
		FreeLibrary(g_statusex_hmodule);
	if (g_finish_hmodule)
		FreeLibrary(g_finish_hmodule);
	if (g_custom_hmodule)
		FreeLibrary(g_custom_hmodule);
	if (g_uninstall_hmodule)
		FreeLibrary(g_uninstall_hmodule);
}

int shared_memory_query_is_window_rect()
{
	HWND hwnd_welcome_ui = FindWindow(NULL, TARGET_WELCOME_TILE);
	HWND hwnd_uninstall_ui = FindWindow(NULL, TARGET_UNINSTALL_TILE);
	if (NULL == hwnd_welcome_ui && NULL == hwnd_uninstall_ui)
	{
		OutputDebugString(L"FindWindow failed");
		return -1;
	}

	TCHAR buff[MAX_PATH] = {0};
	HWND hwnd_target = (hwnd_welcome_ui == NULL ? hwnd_uninstall_ui : hwnd_welcome_ui);
	RECT is_window_rect = {0};
	GetWindowRect(hwnd_target, &is_window_rect);
	_stprintf_s(buff, L"IS window rect: %d, %d, %d, %d",
		is_window_rect.left, is_window_rect.top, is_window_rect.right, is_window_rect.bottom);
	OutputDebugString(buff);

	g_hmap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		SHARED_MEMORY_BUFF_SIZE,
		SHARED_MEMORY_BUFF_NAME
		);
	if (NULL == g_hmap)
	{
		_stprintf_s(buff, L"CreateFileMapping failed: %d", GetLastError());
		OutputDebugString(buff);
		return -1;
	}

	g_pmap = MapViewOfFile(
		g_hmap,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		SHARED_MEMORY_BUFF_SIZE
		);
	if (NULL == g_pmap)
	{
		_stprintf_s(buff, L"MapViewOfFile failed: %d", GetLastError());
		OutputDebugString(buff);
		CloseHandle(g_hmap);
		g_hmap = NULL;
		return -1;
	}

	memcpy(g_pmap, (const void *)&is_window_rect, sizeof(is_window_rect));

	return 0;
}

void close_shared_memory()
{
	if (g_pmap)
		UnmapViewOfFile(g_pmap);

	if (g_hmap)
		CloseHandle(g_hmap);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR sz[MAX_PATH] = {0};

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_WELCOME_UI_BEG:
		_stprintf_s(sz, L"WM_WELCOME_UI_BEG wparam: %x", wParam);
		OutputDebugString(sz);
		g_beg_welcome_hook((HWND)wParam);
		break;
	case WM_WELCOME_UI_END:
		g_end_welcome_hook();
		break;
	case WM_STATUSEX_UI_BEG:
		_stprintf_s(sz, L"WM_STATUSEX_UI_BEG wparam: %x", wParam);
		OutputDebugString(sz);
		g_beg_statusex_hook((HWND)wParam);
		break;
	case WM_STATUSEX_UI_END:
		stop_bbrd_event();
		g_end_statusex_hook();
		break;
	case WM_FINISH_UI_BEG:
		_stprintf_s(sz, L"WM_FINISH_UI_BEG wparam: %x", wParam);
		OutputDebugString(sz);
		g_beg_finish_hook((HWND)wParam);
		break;
	case WM_FINISH_UI_END:
		g_end_finish_hook();
		break;
	case WM_CUSTOM_UI_BEG:
		_stprintf_s(sz, L"WM_CUSTOM_UI_BEG wparam: %x", wParam);
		OutputDebugString(sz);
		g_beg_custom_hook((HWND)wParam);
		break;
	case WM_CUSTOM_UI_END:
		g_end_custom_hook();
		break;
	case WM_UNINSTALL_UI_BEG:
		_stprintf_s(sz, L"WM_UNINSTALL_UI_BEG wparam: %x", wParam);
		OutputDebugString(sz);
		g_beg_uninstall_hook((HWND)wParam);
		break;
	case WM_UNINSTALL_UI_END:
		g_end_uninstall_hook();
		break;
	case WM_QUERY_IS_WINDOW_RECT:
		shared_memory_query_is_window_rect();
		break;
	case WM_DESTROY:
		close_shared_memory();
		free_all_dlls();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
