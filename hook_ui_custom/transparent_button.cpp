// transparent_button.cpp : implementation file
//

#include "stdafx.h"
#include "transparent_button.h"

// transparent_button

transparent_button::transparent_button()
	: m_b_ishover(false)
	, m_b_isclicked(false)
	, m_b_autosize(true)
{
	m_button_png_normal.pimage = NULL;
	m_button_png_hover.pimage = NULL;
	m_button_png_click.pimage = NULL;
	m_button_png_disable.pimage = NULL;
	m_hinstance = LoadLibrary(L"hook_ui_custom.dll");
}

transparent_button::~transparent_button()
{
	if (NULL != m_button_png_normal.pimage)
	{
		delete m_button_png_normal.pimage;
		m_button_png_normal.pimage = NULL;
	}

	if (NULL != m_button_png_hover.pimage)
	{
		delete m_button_png_hover.pimage;
		m_button_png_hover.pimage = NULL;
	}

	if (NULL != m_button_png_click.pimage)
	{
		delete m_button_png_click.pimage;
		m_button_png_click.pimage = NULL;
	}

	if (NULL != m_button_png_disable.pimage)
	{
		delete m_button_png_disable.pimage;
		m_button_png_disable.pimage = NULL;
	}

	if (NULL != m_hinstance)
	{
		FreeLibrary(m_hinstance);
	}
}

BEGIN_MESSAGE_MAP(transparent_button, CButton)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// transparent_button message handlers

void transparent_button::Load(UINT IDBkGroup, int width, int height, const CString &resource_type)
{
	if (NULL == m_hinstance)
	{
		return;
	}

	HINSTANCE hinstance = m_hinstance;
	HRSRC hrsrc = FindResource(hinstance, MAKEINTRESOURCE(IDBkGroup), resource_type);
	if (NULL == hrsrc)
	{
		return;
	}

	DWORD len = SizeofResource(hinstance, hrsrc);
	BYTE *prsrc = (BYTE *)LoadResource(hinstance, hrsrc);
	if (NULL == prsrc)
	{
		return;
	}

	HGLOBAL hglobal = GlobalAlloc(GMEM_FIXED, len);
	BYTE *pmem = (BYTE *)GlobalLock(hglobal);
	memcpy(pmem, prsrc, len);
	IStream *pstm;
	CreateStreamOnHGlobal(hglobal, FALSE, &pstm);

	CImage old_image;
	old_image.Load(pstm);

	GlobalUnlock(hglobal);
	GlobalFree(hglobal);
	pstm->Release();
	FreeResource(prsrc);

	if (_T("PNG") == resource_type)
	{
		if (32 == old_image.GetBPP())
		{
			for(int i = 0; i < old_image.GetWidth(); i++)
			{
				for(int j = 0; j < old_image.GetHeight(); j++)
				{
					unsigned char* puc_color =
						reinterpret_cast<unsigned char *>(old_image.GetPixelAddress(i, j));
					puc_color[0] = puc_color[0] * puc_color[3] / 255;
					puc_color[1] = puc_color[1] * puc_color[3] / 255;
					puc_color[2] = puc_color[2] * puc_color[3] / 255;
				}
			}
		}
	}

	if (0 == width && 0 == height)
	{
		width = old_image.GetHeight();
	}

	if (0 == height)
	{
		height = old_image.GetHeight();
	}

	m_button_png_normal.width = width;
	m_button_png_normal.height = height;
	m_button_png_hover.width = width;
	m_button_png_hover.height = height;
	m_button_png_click.width = width;
	m_button_png_click.height = height;
	m_button_png_disable.width = width;
	m_button_png_disable.height = height;

	CImage **imgs[] =
	{
		&m_button_png_normal.pimage,
		&m_button_png_hover.pimage,
		&m_button_png_click.pimage,
		&m_button_png_disable.pimage
	};

	int pos_x = 0;
	for (int i = 0; i < 4 && pos_x <= (old_image.GetWidth() - width); i++, pos_x += width)
	{
		CImage *pimage = new CImage();
		if (NULL != *imgs[i])
		{
			delete *imgs[i];
			*imgs[i] = NULL;
		}
		*imgs[i] = pimage;

		if (_T("PNG") == resource_type)
		{
			BOOL bret = FALSE;
			if (32 == old_image.GetBPP())
			{
				bret = pimage->CreateEx(width, height, old_image.GetBPP(), BI_RGB, NULL,
					CImage::createAlphaChannel);
			}
			else
			{
				bret = pimage->CreateEx(width, height, old_image.GetBPP(), BI_RGB, NULL);
			}
			ASSERT(bret);
		}
		else
		{
			BOOL bret = FALSE;
			bret = pimage->CreateEx(width, height, old_image.GetBPP(), BI_RGB, NULL);
			ASSERT(bret);
		}

		CImageDC image_dc(*pimage);
		old_image.Draw(image_dc, 0, 0, width, height, pos_x, 0, width, height);
	}
}

void transparent_button::SetAutoSize(bool autosize)
{
	m_b_autosize = autosize;
}

void transparent_button::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct->CtlType == ODT_BUTTON);
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;
	CString strText = _T("");
	GetWindowText(strText);

	if(lpDrawItemStruct->itemState & ODS_DISABLED)
	{
		DrawBK(*pDC, m_button_png_disable.pimage, BUTTON_DISABLE);
	}
	else if(lpDrawItemStruct->itemState & ODS_SELECTED
		|| (m_b_ishover && m_b_isclicked))
	{
		DrawBK(*pDC, m_button_png_click.pimage, BUTTON_CLICK);
	}
	else if(m_b_ishover)
	{
		DrawBK(*pDC, m_button_png_hover.pimage, BUTTON_HOVER);
	}
	else
	{
		DrawBK(*pDC, m_button_png_normal.pimage, BUTTON_NORMAL);
	}

	CString strTemp(strText);
	strTemp.Remove(' ');
	if (!strTemp.IsEmpty())
	{
		if(lpDrawItemStruct->itemState & ODS_DISABLED)
		{
			DrawButtonText(*pDC,strText,0, BUTTON_DISABLE);
		}
		else if(lpDrawItemStruct->itemState & ODS_SELECTED
			|| (m_b_ishover && m_b_isclicked))
		{
			DrawButtonText(*pDC, strText,1, BUTTON_CLICK);
		}
		else if(m_b_ishover)
		{
			DrawButtonText(*pDC, strText,0, BUTTON_HOVER);
		}
		else
		{
			DrawButtonText(*pDC, strText,0, BUTTON_NORMAL);
		}
	}
}

void transparent_button::DrawBK(HDC dc, CImage *img, BUTTON_STATUS button_status)
{
	if (!img)
	{
		return;
	}

	CRect rc;
	GetClientRect(&rc);
	CRect temp_rect;
	int nX = 0;
	int nY = 0;
	int nW = 0;
	int nH = 0;

	if (m_b_autosize == true)
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
			nW = m_button_png_normal.width;
			nH = m_button_png_normal.height;
		}
		else if (button_status == BUTTON_HOVER)
		{
			nW = m_button_png_hover.width;
			nH = m_button_png_hover.height;
		}
		else if (button_status == BUTTON_CLICK)
		{
			nW = m_button_png_click.width;
			nH = m_button_png_click.height;
		}
		else
		{
			nW = m_button_png_disable.width;
			nH = m_button_png_disable.height;
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

void transparent_button::DrawButtonText(HDC dc, const CString &strText, int nMove, BUTTON_STATUS button_status)
{
	CRect rect;
	GetClientRect(&rect);
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

void transparent_button::PreSubclassWindow()
{
	ModifyStyle(0, BS_OWNERDRAW);
	CButton::PreSubclassWindow();
}

void transparent_button::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(rect);

	if(rect.PtInRect(point))
	{
		if(!(nFlags & MK_LBUTTON))
		{
			m_b_isclicked = FALSE;
		}

		if(GetCapture() != this)
		{
			SetCapture();
		}

		if (m_b_ishover == true)
		{
			//
		}
		else
		{
			m_b_ishover = true;
			SetFocus();
			Invalidate();
			CRect temp_rect(rect);
			ClientToScreen(&temp_rect);
			CPoint temp_point(temp_rect.left, temp_rect.top);

			::ScreenToClient(GetParent()->GetSafeHwnd(), &temp_point);
			temp_rect.SetRect(
				temp_point.x,
				temp_point.y,
				temp_point.x + rect.Width(),
				temp_point.y + rect.Height()
				);
			GetParent()->InvalidateRect(temp_rect, TRUE);
		}
	}
	else
	{
		ReleaseCapture();
		m_b_ishover = false;
		Invalidate();
		CRect temp_rect(rect);
		ClientToScreen(&temp_rect);
		CPoint temp_point(temp_rect.left, temp_rect.top);

		::ScreenToClient(GetParent()->GetSafeHwnd(), &temp_point);
		temp_rect.SetRect(
			temp_point.x,
			temp_point.y,
			temp_point.x + rect.Width(),
			temp_point.y + rect.Height()
			);
		GetParent()->InvalidateRect(temp_rect, TRUE);
	}

	CButton::OnMouseMove(nFlags, point);
}

void transparent_button::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_b_isclicked)
	{
		CRect rect;
		GetClientRect(rect);
		Invalidate();
		CRect temp_rect(rect);
		ClientToScreen(&temp_rect);
		CPoint temp_point(temp_rect.left, temp_rect.top);

		::ScreenToClient(GetParent()->GetSafeHwnd(), &temp_point);
		temp_rect.SetRect(
			temp_point.x,
			temp_point.y,
			temp_point.x + rect.Width(),
			temp_point.y + rect.Height()
			);
		GetParent()->InvalidateRect(temp_rect, TRUE);

		m_b_isclicked = false;
	}

	m_b_ishover = false;

	CButton::OnLButtonUp(nFlags, point);
}

void transparent_button::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(rect);
	CRect temp_rect(rect);
	ClientToScreen(&temp_rect);
	CPoint temp_point(temp_rect.left, temp_rect.top);

	::ScreenToClient(GetParent()->GetSafeHwnd(), &temp_point);
	temp_rect.SetRect(
		temp_point.x,
		temp_point.y,
		temp_point.x + rect.Width(),
		temp_point.y + rect.Height()
		);
	GetParent()->InvalidateRect(temp_rect, TRUE);

	m_b_isclicked = true;

	CButton::OnLButtonDown(nFlags, point);
}

BOOL transparent_button::OnEraseBkgnd(CDC *pDC)
{
	if(GetButtonStyle() & BS_OWNERDRAW)
	{
		return TRUE;
	}

	return CButton::OnEraseBkgnd(pDC);
}
