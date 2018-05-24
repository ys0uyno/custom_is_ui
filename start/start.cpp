// start.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>

typedef void (*BEGCBTHOOK)();
typedef void (*ENDCBTHOOK)();

int _tmain(int argc, _TCHAR* argv[])
{
	HMODULE hmodule = LoadLibrary(L"hook_cbt_wnd_created");
	if (NULL == hmodule)
		return -1;

	BEGCBTHOOK beg_hook = (BEGCBTHOOK)GetProcAddress(hmodule, "BegCbtHook");
	if (NULL == beg_hook)
		return -1;

	ENDCBTHOOK end_hook = (ENDCBTHOOK)GetProcAddress(hmodule, "EndCbtHook");
	if (NULL == end_hook)
		return -1;

	beg_hook();
	getchar();
	end_hook();

	FreeLibrary(hmodule);
	return 0;
}

