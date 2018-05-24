// hook_ui_finish.h : main header file for the hook_ui_finish DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Chook_ui_finishApp
// See hook_ui_finish.cpp for the implementation of this class
//

class Chook_ui_finishApp : public CWinApp
{
public:
	Chook_ui_finishApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
