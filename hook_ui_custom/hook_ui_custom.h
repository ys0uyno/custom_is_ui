// hook_ui_custom.h : main header file for the hook_ui_custom DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Chook_ui_customApp
// See hook_ui_custom.cpp for the implementation of this class
//

class Chook_ui_customApp : public CWinApp
{
public:
	Chook_ui_customApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
