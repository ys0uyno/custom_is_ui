// hook_cbt_wnd_created.h : main header file for the hook_cbt_wnd_created DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Chook_cbt_wnd_createdApp
// See hook_cbt_wnd_created.cpp for the implementation of this class
//

class Chook_cbt_wnd_createdApp : public CWinApp
{
public:
	Chook_cbt_wnd_createdApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
