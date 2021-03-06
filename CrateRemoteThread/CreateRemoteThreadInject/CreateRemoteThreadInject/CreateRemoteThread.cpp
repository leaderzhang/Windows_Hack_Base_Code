#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

void ShowError(char *pszText);
BOOL EnbalePrivileges(HANDLE hProcess, wchar_t *pszPrivilegesName);
BOOL CreateRemoteThreadInjectDll(DWORD dwProcessId, char *pszDllFileName);


int main()
{

	EnbalePrivileges(GetCurrentProcess(), SE_DEBUG_NAME);

	BOOL bRet = CreateRemoteThreadInjectDll(1044, "I:\\VS2013Project\\windows_hack_base_code\\CrateRemoteThread\\MessageBoxDll\\x64\\Debug\\MessageBoxDll.dll");

	if (FALSE == bRet)
	{
		printf("Inject Dll Error！\n");
	}
	printf("Inject Dll Error！\n");
	system("pause");

	return 0;
}


void ShowError(char *pszText)
{
	char szErr[MAX_PATH] = {0};
	sprintf(szErr, "%s Error[%d]\n", pszText, GetLastError());
	MessageBoxA(NULL, szErr, "ERROR", MB_OK);
}

BOOL EnbalePrivileges(HANDLE hProcess, wchar_t *pszPrivilegesName)
{
	HANDLE hToken = NULL;
	LUID luidValue = { 0 };
	TOKEN_PRIVILEGES tokenPrivileges = { 0 };
	BOOL bRet = FALSE;
	DWORD dwRet = 0;


	// 打开进程令牌并获取具有 TOKEN_ADJUST_PRIVILEGES 权限的进程令牌句柄
	bRet = ::OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken);
	if (FALSE == bRet)
	{
		ShowError("OpenProcessToken");
		return FALSE;
	}
	// 获取本地系统的 pszPrivilegesName 特权的LUID值
	bRet = ::LookupPrivilegeValue(NULL, pszPrivilegesName, &luidValue);
	if (FALSE == bRet)
	{
		ShowError("LookupPrivilegeValue");
		return FALSE;
	}
	// 设置提升权限信息
	tokenPrivileges.PrivilegeCount = 1;
	tokenPrivileges.Privileges[0].Luid = luidValue;
	tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	// 提升进程令牌访问权限
	bRet = ::AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, NULL, NULL);
	if (FALSE == bRet)
	{
		ShowError("AdjustTokenPrivileges");
		return FALSE;
	}
	else
	{
		// 根据错误码判断是否特权都设置成功
		dwRet = ::GetLastError();
		if (ERROR_SUCCESS == dwRet)
		{
			return TRUE;
		}
		else if (ERROR_NOT_ALL_ASSIGNED == dwRet)
		{
			ShowError("ERROR_NOT_ALL_ASSIGNED");
			return FALSE;
		}
	}

	return FALSE;
}

BOOL CreateRemoteThreadInjectDll(DWORD dwProcessId, char *pszDllFileName)
{
	HANDLE hProcess = NULL;
	LPVOID pDllAdrr = NULL;
	FARPROC pFunProcAddr = NULL;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (NULL == hProcess)
	{
		ShowError("OpenProcess");
		return FALSE;
	}

	pDllAdrr = VirtualAllocEx(hProcess, NULL, lstrlenA(pszDllFileName), MEM_COMMIT, PAGE_READWRITE);
	if (NULL == pDllAdrr)
	{
		ShowError("VirtualAllocEx");
		return FALSE;
	}

	if (NULL == WriteProcessMemory(hProcess, pDllAdrr, pszDllFileName, lstrlenA(pszDllFileName), NULL))
	{
		ShowError("WriteProcessMemory");
		return FALSE;
	}

	pFunProcAddr = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (NULL == pFunProcAddr)
	{
		ShowError("GetProcAddress");
		return FALSE;
	}

	HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFunProcAddr, pDllAdrr, 0, NULL);
	if (NULL == hRemoteThread)
	{
		ShowError("CreateRemoteThread");
		return FALSE;
	}

	CloseHandle(hProcess);

	return TRUE;
}