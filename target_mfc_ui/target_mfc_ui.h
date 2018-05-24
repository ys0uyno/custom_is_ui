
// target_mfc_ui.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Ctarget_mfc_uiApp:
// See target_mfc_ui.cpp for the implementation of this class
//

class Ctarget_mfc_uiApp : public CWinApp
{
public:
	Ctarget_mfc_uiApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Ctarget_mfc_uiApp theApp;