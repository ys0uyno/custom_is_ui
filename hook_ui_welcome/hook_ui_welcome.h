// hook_ui_welcome.h : main header file for the hook_ui_welcome DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Chook_ui_welcomeApp
// See hook_ui_welcome.cpp for the implementation of this class
//

class Chook_ui_welcomeApp : public CWinApp
{
public:
	Chook_ui_welcomeApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
