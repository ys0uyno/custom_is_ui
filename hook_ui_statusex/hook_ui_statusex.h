// hook_ui_statusex.h : main header file for the hook_ui_statusex DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Chook_ui_statusexApp
// See hook_ui_statusex.cpp for the implementation of this class
//

class Chook_ui_statusexApp : public CWinApp
{
public:
	Chook_ui_statusexApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
