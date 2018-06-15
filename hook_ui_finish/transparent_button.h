#pragma once

#include <atlimage.h>

enum BUTTON_STATUS
{
	BUTTON_NORMAL,
	BUTTON_HOVER,
	BUTTON_CLICK,
	BUTTON_DISABLE
};

typedef struct _BUTTON_PNG
{
	int width;
	int height;
	CImage *pimage;
} BUTTON_PNG, *PBUTTON_PNG;

class transparent_button : public CButton
{
public:
	transparent_button();
	virtual ~transparent_button();

protected:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	DECLARE_MESSAGE_MAP()

public:
	void Load(UINT IDBkGroup, int width = 0, int height = 0, const CString &resource_type = _T("PNG"));
	void SetAutoSize(bool autosize);

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

public:
	void DrawBK(HDC dc, CImage *img, BUTTON_STATUS button_status);
	void DrawButtonText(HDC dc, const CString &strText, int nMove, BUTTON_STATUS button_status);
	virtual void PreSubclassWindow();

public:
	BUTTON_PNG m_button_png_normal;
	BUTTON_PNG m_button_png_hover;
	BUTTON_PNG m_button_png_click;
	BUTTON_PNG m_button_png_disable;
	bool m_b_ishover;
	bool m_b_isclicked;
	bool m_b_autosize;
	HINSTANCE m_hinstance;
};
