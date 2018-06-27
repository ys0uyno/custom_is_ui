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

bool copy_pictures(const CString &src_dir)
{
	TCHAR temp_dir[MAX_PATH] = {0};
	if (0 == GetEnvironmentVariable(L"TEMP", temp_dir, MAX_PATH))
	{
		printf("GetEnvironmentVariable failed: %d\n", GetLastError());
		return false;
	}

	_tprintf(L"TEMP: %s\n", temp_dir);

	CString is_pictures_dir(temp_dir);
	is_pictures_dir.Append(L"\\is_pictures");
	_tprintf(L"TEMP: %s\n", is_pictures_dir);

	if (!PathFileExists(is_pictures_dir))
	{
		_tprintf(L"TEMP: %s NOT exists\n", is_pictures_dir);
		if (!CreateDirectory(is_pictures_dir, NULL))
		{
			printf("CreateDirectory failed: %d\n", GetLastError());
			return false;
		}
	}

	_tprintf(L"current dir: %s\n", src_dir);

	WIN32_FIND_DATA wfd;
	CString find_str = src_dir + L"\\*.bmp";
	HANDLE hFile = FindFirstFile(find_str.GetBuffer(), &wfd);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		do
		{
			_tprintf(L"found %s\n", wfd.cFileName);
			CopyFile(src_dir + L"\\" + wfd.cFileName,
				is_pictures_dir + L"\\" + wfd.cFileName,
				FALSE);
		} while (FindNextFile(hFile, &wfd));
	}

	find_str = src_dir + L"\\*.gif";
	hFile = FindFirstFile(find_str.GetBuffer(), &wfd);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		do
		{
			_tprintf(L"found %s\n", wfd.cFileName);
			CopyFile(src_dir + L"\\" + wfd.cFileName,
				is_pictures_dir + L"\\" + wfd.cFileName,
				FALSE);
		} while (FindNextFile(hFile, &wfd));
	}

	find_str = src_dir + L"\\*.png";
	hFile = FindFirstFile(find_str.GetBuffer(), &wfd);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		do
		{
			_tprintf(L"found %s\n", wfd.cFileName);
			CopyFile(src_dir + L"\\" + wfd.cFileName,
				is_pictures_dir + L"\\" + wfd.cFileName,
				FALSE);
		} while (FindNextFile(hFile, &wfd));
	}

	find_str = src_dir + L"\\*.jpg";
	hFile = FindFirstFile(find_str.GetBuffer(), &wfd);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		do
		{
			_tprintf(L"found %s\n", wfd.cFileName);
			CopyFile(src_dir + L"\\" + wfd.cFileName,
				is_pictures_dir + L"\\" + wfd.cFileName,
				FALSE);
		} while (FindNextFile(hFile, &wfd));
	}

	FindClose(hFile);
	return true;
}

int delete_dir(const CString &refcstrRootDirectory, bool bDeleteSubdirectories = true)
{
	bool bSubdirectory = false;
	HANDLE hFile;
	CString strFilePath;
	CString strPattern;
	WIN32_FIND_DATA FileInformation;

	strPattern = refcstrRootDirectory + L"\\*.*";
	hFile = FindFirstFile(strPattern, &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.Empty();
				strFilePath = refcstrRootDirectory + L"\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (bDeleteSubdirectories)
					{
						int iRC = delete_dir(strFilePath, bDeleteSubdirectories);
						if (iRC)
						{
							return iRC;
						}
					}
					else
					{
						bSubdirectory = true;
					}
				}
				else
				{
					if (SetFileAttributes(strFilePath, FILE_ATTRIBUTE_NORMAL) == FALSE)
					{
						return GetLastError();
					}

					if (DeleteFile(strFilePath) == FALSE)
					{
						return GetLastError();
					}
					else
					{
						printf("%ws deleted.\n", strFilePath);
					}
				}
			}
		} while (FindNextFile(hFile, &FileInformation) == TRUE);

		FindClose(hFile);

		DWORD dwError = GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
		{
			return dwError;
		}
		else
		{
			if (!bSubdirectory)
			{
				if (SetFileAttributes(refcstrRootDirectory, FILE_ATTRIBUTE_NORMAL) == FALSE)
				{
					return GetLastError();
				}

				if (RemoveDirectory(refcstrRootDirectory) == FALSE)
				{
					return GetLastError();
				}
			}
		}
	}

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

	CString current_dir;
	get_current_dir(current_dir);
	TCHAR temp[MAX_PATH] = {0};
	GetEnvironmentVariable(L"TEMP", temp, MAX_PATH);
	CString temp_dir(temp);

	// copy pictures to TEMP
	if (!copy_pictures(current_dir))
	{
		return -1;
	}

	CString str_exe;
	get_current_dir(str_exe);

	str_exe.Append(L"win32_dll_loader.exe");
	if (-1 == _taccess(str_exe, 0))
	{
		printf("win32_dll_loader.exe not found\n");
		delete_dir(temp_dir + L"\\is_pictures");
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
		delete_dir(temp_dir + L"\\is_pictures");
		return -1;
	}

	beg_hook();
	getchar();
	end_hook();

	delete_dir(temp_dir + L"\\is_pictures");

	SendMessage(FindWindow(NULL, L"win32_dll_loader"), WM_DESTROY, 0, 0);

	CloseHandle(pi.hProcess);
	FreeLibrary(hmodule);
	return 0;
}

