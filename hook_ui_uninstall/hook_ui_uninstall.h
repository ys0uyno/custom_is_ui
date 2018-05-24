// hook_ui_uninstall.h : main header file for the hook_ui_uninstall DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Chook_ui_uninstallApp
// See hook_ui_uninstall.cpp for the implementation of this class
//

class Chook_ui_uninstallApp : public CWinApp
{
public:
	Chook_ui_uninstallApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
