// hook_ui_finish.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_ui_finish.h"
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TARGET_TITLE L"²Ù×÷Íê³É"

HINSTANCE g_hinstance = NULL;
HHOOK g_hhook_wnd = NULL;
HHOOK g_hhook_wnd_ret = NULL;
HHOOK g_hhook_msg = NULL;

HWND g_finish_button_hwnd = NULL;

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

WNDPROC g_old_proc;
static bool g_subclassed = false;

CString g_pictures_dir;

CRect g_finish_button_crect;
bool g_is_uninstall_finish = false;

Gdiplus::Bitmap *g_pinstall_image;
bool g_ishover = false;
static bool g_bmouseleave_once = false;

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

void FillPathRoundRect(Gdiplus::Graphics *pGraphics, Gdiplus::Rect r, Gdiplus::Color color, int radius)
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

	Gdiplus::SolidBrush br(color);
	pGraphics->FillPath(&br, &path);
}

void DrawPathRoundRect(Gdiplus::Graphics *pGraphics, Gdiplus::Rect r, Gdiplus::Color color, int radius, int width)
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
					if (!g_is_uninstall_finish)
					{
						Gdiplus::Graphics g(lpDrawItemStruct->hDC);

						Gdiplus::Rect dst_rect(
							g_finish_button_crect.left + 1,
							g_finish_button_crect.top + 1,
							g_finish_button_crect.Width() - 2,
							g_finish_button_crect.Height() - 2
							);

						int src_width = g_pinstall_image->GetWidth();
						int src_height = g_pinstall_image->GetHeight() / 3;

						if (lpDrawItemStruct->itemState & ODS_SELECTED)
						{
							g.DrawImage(g_pinstall_image, dst_rect,
								0, src_height * 2, src_width, src_height,
								Gdiplus::UnitPixel);
						}
						else if (g_ishover)
						{
							g.DrawImage(g_pinstall_image, dst_rect,
								0, src_height * 1, src_width, src_height,
								Gdiplus::UnitPixel);
						}
						else
						{
							g.DrawImage(g_pinstall_image, dst_rect,
								0, src_height * 0, src_width, src_height,
								Gdiplus::UnitPixel);
						}
					}
					else
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
						/*pDC->RoundRect(0, 0, rect.right, rect.bottom, rect.Width() / 3, rect.Height());*/

						Gdiplus::Graphics g(lpDrawItemStruct->hDC);
						Gdiplus::Rect gdi_rect(0, 0, rect.Width(), rect.Height());
						Gdiplus::Color gdi_color(255, 0, 173, 239);
						g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
						DrawPathRoundRect(&g, gdi_rect, gdi_color, rect.Height() / 2, 1);
						FillPathRoundRect(&g, gdi_rect, gdi_color, rect.Height() / 2);

						pDC->SetBkMode(TRANSPARENT);
						TCHAR button_text[MAX_PATH] = {0};
						GetWindowText(g_finish_button_hwnd, button_text, MAX_PATH);
						pDC->SetTextColor(RGB(255, 255, 255));
						pDC->DrawText(button_text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

						pDC->RestoreDC(nSaveDC);
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
	HDC hDC;
	TCHAR sz[MAX_PATH] = {0};
	RECT rect;
	CPoint cpoint;
	PAINTSTRUCT ps;

	switch (p->message)
	{
		case WM_MOUSEHOVER:
		{
			if (g_finish_button_hwnd == p->hwnd)
			{
				if (!g_is_uninstall_finish)
				{
					g_ishover = true;
					g_bmouseleave_once = false;

					InvalidateRect(g_finish_button_hwnd, &g_finish_button_crect, TRUE);
				}
				else
				{
					hDC = ::GetDC(g_finish_button_hwnd);
					swprintf_s(sz, L"WM_MOUSEHOVER, g_hwnd_btn: %x, hDC: %x, point: %d, %d",
						g_finish_button_hwnd, hDC, GET_X_LPARAM(p->lParam), GET_Y_LPARAM(p->lParam));
					OutputDebugString(sz);

					CDC* pDC = CDC::FromHandle(hDC);
					pDC->SelectObject(&g_brush.m_brush_hover);
					pDC->SelectObject(&g_brush.m_pen);
					pDC->SelectObject(&g_brush.m_font);
					GetClientRect(g_finish_button_hwnd, &rect);
					/*pDC->RoundRect(0, 0, rect.right, rect.bottom, (rect.right - rect.left) / 2,
						rect.bottom - rect.top);*/

					Gdiplus::Graphics g(hDC);
					Gdiplus::Rect gdi_rect(0, 0, rect.right - rect.left, rect.bottom - rect.top);
					Gdiplus::Color gdi_color(255, 4, 165, 227);
					g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
					DrawPathRoundRect(&g, gdi_rect, gdi_color, (rect.bottom - rect.top) / 2, 1);
					FillPathRoundRect(&g, gdi_rect, gdi_color, (rect.bottom - rect.top) / 2);

					pDC->SetBkMode(TRANSPARENT);
					TCHAR button_text[MAX_PATH] = {0};
					GetWindowText(g_finish_button_hwnd, button_text, MAX_PATH);
					pDC->SetTextColor(RGB(255, 255, 255));
					pDC->DrawText(button_text, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

					ReleaseDC(g_finish_button_hwnd, hDC);
				}
			}
		}
		break;
	case WM_MOUSELEAVE:
		{
			if (g_finish_button_hwnd == p->hwnd)
			{
				if (!g_is_uninstall_finish)
				{
					g_ishover = false;
					if (!g_bmouseleave_once)
					{
						g_bmouseleave_once = true;

						InvalidateRect(g_finish_button_hwnd, &g_finish_button_crect, TRUE);
					}
					g_bmousetrack = TRUE;
				}
				else
				{
					// OutputDebugString(L"WM_MOUSELEAVE");
					hDC = ::GetDC(g_finish_button_hwnd);

					CDC* pDC = CDC::FromHandle(hDC);
					pDC->SelectObject(&g_brush.m_brush_normal);
					pDC->SelectObject(&g_brush.m_pen);
					pDC->SelectObject(&g_brush.m_font);
					GetClientRect(g_finish_button_hwnd, &rect);
					/*pDC->RoundRect(0, 0, rect.right, rect.bottom, (rect.right - rect.left) / 2,
						rect.bottom - rect.top);*/

					Gdiplus::Graphics g(hDC);
					Gdiplus::Rect gdi_rect(0, 0, rect.right - rect.left, rect.bottom - rect.top);
					Gdiplus::Color gdi_color(255, 0, 173, 239);
					g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
					DrawPathRoundRect(&g, gdi_rect, gdi_color, (rect.bottom - rect.top) / 2, 1);
					FillPathRoundRect(&g, gdi_rect, gdi_color, (rect.bottom - rect.top) / 2);

					pDC->SetBkMode(TRANSPARENT);
					TCHAR button_text[MAX_PATH] = {0};
					GetWindowText(g_finish_button_hwnd, button_text, MAX_PATH);
					pDC->SetTextColor(RGB(255, 255, 255));
					pDC->DrawText(button_text, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

					g_bmousetrack = TRUE;
					ReleaseDC(g_finish_button_hwnd, hDC);
				}
			}
		}
		break;
	case WM_MOUSEMOVE:
		if (g_bmousetrack)
		{
			TRACKMOUSEEVENT csTME;
			csTME.cbSize = sizeof(csTME);
			csTME.dwFlags = TME_LEAVE | TME_HOVER;
			csTME.hwndTrack = g_finish_button_hwnd;
			csTME.dwHoverTime = 10/*HOVER_DEFAULT*/;
			::_TrackMouseEvent(&csTME);
			g_bmousetrack = FALSE;
		}
		break;
	case WM_PAINT:
		{
			OutputDebugString(L"WH_GETMESSAGE WM_PAINT");
			HWND banner_image_hwnd = GetDlgItem(g_hwnd, 0xffffffff);
			if (banner_image_hwnd && p->hwnd == banner_image_hwnd)
			{
				hDC = BeginPaint(p->hwnd, &ps);

				HWND banner_image_hwnd = GetDlgItem(g_hwnd, 0xffffffff);
				_stprintf_s(sz, L"banner_image_hwnd: %x", banner_image_hwnd);
				OutputDebugString(sz);

				if (banner_image_hwnd)
				{
					RECT client_rect;
					GetClientRect(g_hwnd, &client_rect);
					_stprintf_s(sz, L"client_rect: %d, %d, %d, %d", client_rect.left, client_rect.top,
						client_rect.right, client_rect.bottom);
					OutputDebugString(sz);

					if (!g_is_uninstall_finish)
					{
						MoveWindow(banner_image_hwnd,
							client_rect.left,
							client_rect.top,
							client_rect.right - client_rect.left,
							client_rect.bottom - client_rect.top,
							TRUE);
					}
					else
					{
						DWORD banner_pic_width = client_rect.right - client_rect.left;
						MoveWindow(banner_image_hwnd, client_rect.left, client_rect.top,
							banner_pic_width, (int)(banner_pic_width / 2.424), TRUE);
					}
				}

				HDC banner_image_hdc = GetDC(banner_image_hwnd);
				RECT banner_image_rect;
				GetClientRect(banner_image_hwnd, &banner_image_rect);

				CImage banner_image;
				if (!g_is_uninstall_finish)
				{
					banner_image.Load(g_pictures_dir + L"\\background_finish.jpg");
				}
				else
				{
					banner_image.Load(g_pictures_dir + L"\\banner.bmp");
				}

				banner_image.Draw(banner_image_hdc, banner_image_rect);
				ReleaseDC(banner_image_hwnd, banner_image_hdc);

				EndPaint(p->hwnd, &ps);
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

			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			ULONG_PTR gdiplusToken;
			Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

			OutputDebugString(L"after hook_ui_finish WM_INITDIALOG");
			AfxWinInit(GetModuleHandle(L"hook_ui_finish"), NULL, GetCommandLine(), 0);

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

			g_finish_button_hwnd = hwnd_btn;

			long lstyle = GetWindowLong(g_finish_button_hwnd, GWL_STYLE);
			lstyle |= BS_OWNERDRAW;
			SetWindowLong(g_finish_button_hwnd, GWL_STYLE, lstyle);

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
			GetClientRect(g_finish_button_hwnd, &temp_rect);
			InvalidateRect(g_finish_button_hwnd, &temp_rect, TRUE);

			g_finish_button_crect = temp_rect;

			if (g_finish_button_crect.Width() == g_finish_button_crect.Height())
			{
				OutputDebugString(L"installation finish");
				g_is_uninstall_finish = false;
			}
			else
			{
				OutputDebugString(L"uninstallation finish");
				g_is_uninstall_finish = true;
			}

			HWND banner_image_hwnd = GetDlgItem(g_hwnd, 0xffffffff);
			_stprintf_s(sz, L"banner_image_hwnd: %x", banner_image_hwnd);
			OutputDebugString(sz);

			if (banner_image_hwnd)
			{
				RECT client_rect;
				GetClientRect(g_hwnd, &client_rect);
				_stprintf_s(sz, L"client_rect: %d, %d, %d, %d", client_rect.left, client_rect.top,
					client_rect.right, client_rect.bottom);
				OutputDebugString(sz);

				if (!g_is_uninstall_finish)
				{
					MoveWindow(banner_image_hwnd,
						client_rect.left,
						client_rect.top,
						client_rect.right - client_rect.left,
						client_rect.bottom - client_rect.top,
						TRUE);
				}
				else
				{
					DWORD banner_pic_width = client_rect.right - client_rect.left;
					MoveWindow(banner_image_hwnd, client_rect.left, client_rect.top,
						banner_pic_width, (int)(banner_pic_width / 2.424), TRUE);
				}
			}

			TCHAR temp_dir[MAX_PATH] = {0};
			if (0 == GetEnvironmentVariable(L"TEMP", temp_dir, MAX_PATH))
			{
				printf("GetEnvironmentVariable failed: %d\n", GetLastError());
			}
			g_pictures_dir = temp_dir;
			g_pictures_dir += L"\\is_pictures";

			g_pinstall_image = Gdiplus::Bitmap::FromFile(g_pictures_dir + L"\\finish_button.png");

			g_old_proc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)new_proc);
		}
		break;
	}

	return CallNextHookEx(g_hhook_wnd_ret, nCode, wParam, lParam);
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
	g_hhook_wnd = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstance, tid);
	g_hhook_wnd_ret = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hinstance, tid);
	g_hhook_msg = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndFinishHook()
{
	if (NULL != g_hhook_wnd)
		UnhookWindowsHookEx(g_hhook_wnd);

	if (NULL != g_hhook_wnd_ret)
		UnhookWindowsHookEx(g_hhook_wnd_ret);

	if (NULL != g_hhook_msg)
		UnhookWindowsHookEx(g_hhook_msg);
}
