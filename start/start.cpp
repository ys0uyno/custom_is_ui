// start.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <atlstr.h>

typedef void (*BEGCBTHOOK)();
typedef void (*ENDCBTHOOK)();

int get_current_dir(CString &path)
{
	TCHAR temp[MAX_PATH] = {0};
	DWORD dwret = GetModuleFileName(NULL, temp, MAX_PATH);
	if (0 == dwret)
	{
		return -1;
	}

	TCHAR drive[_MAX_DRIVE] = { 0 };
	TCHAR dir[_MAX_DIR] = { 0 };
	TCHAR file[_MAX_FNAME] = { 0 };
	TCHAR ext[_MAX_EXT] = { 0 };
	errno_t err = 0;
	err = _tsplitpath_s(temp, drive, _MAX_DRIVE, dir, _MAX_DIR,
		file, _MAX_FNAME, ext, _MAX_EXT);
	if (0 != err)
	{
		return -1;
	}

	path += drive;
	path += dir;

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HMODULE hmodule = LoadLibrary(L"hook_cbt_wnd_created");
	if (NULL == hmodule)
	{
		printf("LoadLibrary failed: %d\n", GetLastError());
		return -1;
	}

	BEGCBTHOOK beg_hook = (BEGCBTHOOK)GetProcAddress(hmodule, "BegCbtHook");
	if (NULL == beg_hook)
	{
		printf("GetProcAddress BegCbtHook failed: %d\n", GetLastError());
		FreeLibrary(hmodule);
		return -1;
	}

	ENDCBTHOOK end_hook = (ENDCBTHOOK)GetProcAddress(hmodule, "EndCbtHook");
	if (NULL == end_hook)
	{
		printf("GetProcAddress EndCbtHook failed: %d\n", GetLastError());
		FreeLibrary(hmodule);
		return -1;
	}

	CString str_exe;
	get_current_dir(str_exe);

	str_exe.Append(L"win32_dll_loader.exe");
	if (-1 == _taccess(str_exe, 0))
	{
		printf("win32_dll_loader.exe not found\n");
		return -1;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	if (!CreateProcess(NULL, str_exe.GetBuffer(), NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
	{
		printf("CreateProcess failed: %d\n", GetLastError());
		return -1;
	}

	beg_hook();
	getchar();
	end_hook();

	SendMessage(FindWindow(NULL, L"win32_dll_loader"), WM_DESTROY, 0, 0);

	CloseHandle(pi.hProcess);
	FreeLibrary(hmodule);
	return 0;
}

