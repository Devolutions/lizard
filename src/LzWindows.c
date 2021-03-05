#include <lizard/lizard.h>

#include <windows.h>
#include <strsafe.h>
#include <assert.h>

#define SERVICE_STATUS_QUERY_INTERVAL 100   // 0.1s
#define SERVICE_START_TIMEOUT 10000         // 10s
#define SERVICE_STOP_TIMEOUT 10000          // 10s

static HMODULE g_kernel32 = NULL;

fnIsWow64Process pfnIsWow64Process = NULL;
fnWow64DisableWow64FsRedirection pfnWow64DisableWow64FsRedirection = NULL;
fnWow64RevertWow64FsRedirection pfnWow64RevertWow64FsRedirection = NULL;

int LzMessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	int status = 0;
	WCHAR* lpTextW = NULL;
	WCHAR* lpCaptionW = NULL;

	if (lpText)
	{
		lpTextW = LzUnicode_UTF8toUTF16_dup(lpText);

		if (!lpTextW)
			goto cleanup;
	}

	if (lpCaption)
	{
		lpCaptionW = LzUnicode_UTF8toUTF16_dup(lpCaption);

		if (!lpCaptionW)
			goto cleanup;
	}

	status = MessageBoxW(hWnd, lpTextW, lpCaptionW, uType);

	cleanup:
	free(lpTextW);
	free(lpCaptionW);
	return status;
}

char* LzGetCommandLine()
{
	char* commandLine;
	uint16_t* commandLineW;

	commandLineW = GetCommandLineW();

	if (!commandLineW)
		return NULL;

	commandLine = LzUnicode_UTF16toUTF8_dup(commandLineW);

	return commandLine;
}

BOOL LzShellExecuteEx(SHELLEXECUTEINFOA *pExecInfo)
{
	BOOL result = FALSE;

	LPWSTR lpVerbW = 0;
	LPWSTR lpFileW = 0;
	LPWSTR lpParametersW = 0;
	LPWSTR lpDirectoryW = 0;
	LPWSTR lpClassW = 0;

	SHELLEXECUTEINFOW pExecInfoW;

	ZeroMemory(&pExecInfoW, sizeof(SHELLEXECUTEINFOW));

	if (pExecInfo->lpVerb)
	{
		lpVerbW = LzUnicode_UTF8toUTF16_dup(pExecInfo->lpVerb);

		if (!lpVerbW)
			goto cleanup;
	}

	if (pExecInfo->lpFile)
	{
		lpFileW = LzUnicode_UTF8toUTF16_dup(pExecInfo->lpFile);

		if (!lpFileW)
			goto cleanup;
	}

	if (pExecInfo->lpParameters)
	{
		lpParametersW = LzUnicode_UTF8toUTF16_dup(pExecInfo->lpParameters);

		if (!lpParametersW)
			goto cleanup;
	}

	if (pExecInfo->lpDirectory)
	{
		lpDirectoryW = LzUnicode_UTF8toUTF16_dup(pExecInfo->lpDirectory);

		if (!lpDirectoryW)
			goto cleanup;
	}

	if (pExecInfo->lpClass)
	{
		lpClassW = LzUnicode_UTF8toUTF16_dup(pExecInfo->lpClass);

		if (!lpClassW)
			goto cleanup;
	}

	pExecInfoW.cbSize = sizeof(pExecInfoW);
	pExecInfoW.fMask = pExecInfo->fMask;
	pExecInfoW.hwnd = pExecInfo->hwnd;
	pExecInfoW.lpVerb = lpVerbW;
	pExecInfoW.lpFile = lpFileW;
	pExecInfoW.lpParameters = lpParametersW;
	pExecInfoW.lpDirectory = lpDirectoryW;
	pExecInfoW.nShow = pExecInfo->nShow;
	pExecInfoW.hInstApp = pExecInfo->hInstApp;
	pExecInfoW.lpIDList = pExecInfo->lpIDList;
	pExecInfoW.lpClass = lpClassW;
	pExecInfoW.hkeyClass = pExecInfo->hkeyClass;
	pExecInfoW.dwHotKey = pExecInfo->dwHotKey;
	pExecInfoW.hIcon = pExecInfo->hIcon;
	pExecInfoW.hProcess = pExecInfo->hProcess;

	result = ShellExecuteExW(&pExecInfoW);


	cleanup:
	if (lpVerbW)
		free(lpVerbW);
	if (lpFileW)
		free(lpFileW);
	if (lpParametersW)
		free(lpParametersW);
	if (lpDirectoryW)
		free(lpDirectoryW);
	if (lpClassW)
		free(lpClassW);

	return result;
}

BOOL LzGetModuleFileName(HMODULE module, LPSTR lpFileName, DWORD nSize)
{
	WCHAR path[MAX_PATH];

	if (!lpFileName || nSize == 0)
		return false;

	if (GetModuleFileNameW(module, path, sizeof(path)) == 0)
		return false;

	if (LzUnicode_UTF16toUTF8(path, -1, (uint8_t*) lpFileName, nSize) < 0)
		return false;

	return true;
}

int LzSetEnv(const char* name, const char* value)
{
	int result;

	wchar_t* nameW = LzUnicode_UTF8toUTF16_dup(name);
	wchar_t* valueW = LzUnicode_UTF8toUTF16_dup(value);

	if (!nameW || !valueW)
	{
		result = LZ_ERROR_PARAM;
		goto cleanup;
	}

	if (!SetEnvironmentVariableW(nameW, valueW))
	{
		result = LZ_ERROR_FAIL;
		goto cleanup;
	}

	result = LZ_OK;

	cleanup:
	if (nameW)
		free(nameW);
	if (valueW)
		free(valueW);

	return result;
}


BOOL LzCreateProcess(
	const char* lpApplicationName,
	char* lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	const char* lpCurrentDirectory,
	LPSTARTUPINFOA lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
{
	LPWSTR lpApplicationNameW = NULL;
	LPWSTR lpCommandLineW = NULL;
	LPWSTR lpCurrentDirectoryW = NULL;
	STARTUPINFOW startupInfoW;
	BOOL result = FALSE;

	ZeroMemory(&startupInfoW, sizeof(startupInfoW));
	startupInfoW.cb = sizeof(startupInfoW);
	startupInfoW.hStdError = lpStartupInfo->hStdError;
	startupInfoW.hStdOutput = lpStartupInfo->hStdOutput;
	startupInfoW.hStdInput = lpStartupInfo->hStdInput;
	startupInfoW.dwFlags = STARTF_USESTDHANDLES;

	if (lpApplicationName)
	{
		lpApplicationNameW = LzUnicode_UTF8toUTF16_dup(lpApplicationName);

		if (!lpApplicationNameW)
			goto cleanup;
	}

	if (lpCommandLine)
	{
		lpCommandLineW = LzUnicode_UTF8toUTF16_dup(lpCommandLine);

		if (!lpCommandLineW)
			goto cleanup;
	}

	if (lpCurrentDirectory)
	{
		lpCurrentDirectoryW = LzUnicode_UTF8toUTF16_dup(lpCurrentDirectory);

		if (!lpCurrentDirectoryW)
			goto cleanup;
	}

	result = CreateProcessW(
		lpApplicationNameW,
		lpCommandLineW,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectoryW,
		&startupInfoW,
		lpProcessInformation);

	cleanup:
	free(lpApplicationNameW);
	free(lpCommandLineW);
	free(lpCurrentDirectoryW);
	return result;
}

BOOL LzIsWow64()
{
	BOOL wow64 = FALSE;

	g_kernel32 = GetModuleHandleW(L"kernel32");

	if (!g_kernel32)
		return FALSE;

	pfnIsWow64Process =
		(fnIsWow64Process) GetProcAddress(g_kernel32, "IsWow64Process");
	pfnWow64DisableWow64FsRedirection =
		(fnWow64DisableWow64FsRedirection) GetProcAddress(g_kernel32, "Wow64DisableWow64FsRedirection");
	pfnWow64RevertWow64FsRedirection =
		(fnWow64RevertWow64FsRedirection) GetProcAddress(g_kernel32, "Wow64RevertWow64FsRedirection");

	if (!pfnIsWow64Process || !pfnWow64DisableWow64FsRedirection || !pfnWow64RevertWow64FsRedirection)
		return FALSE;

	if (!pfnIsWow64Process(GetCurrentProcess(), &wow64))
		return FALSE;

	return wow64;
}

int LzInstallService(const char* serviceName, const char* servicePath)
{
	int result;

	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hService = NULL;

	wchar_t* serviceNameW = 0;
	wchar_t* servicePathW = 0;

	if (serviceName)
	{
		serviceNameW = LzUnicode_UTF8toUTF16_dup(serviceName);

		if (!serviceNameW)
		{
			result = LZ_ERROR_PARAM;
			goto cleanup;
		}
	}
	else
	{
		result = LZ_ERROR_PARAM;
		goto cleanup;
	}

	if (servicePath)
	{
		servicePathW = LzUnicode_UTF8toUTF16_dup(servicePath);

		if (!servicePathW)
		{
			result = LZ_ERROR_PARAM;
			goto cleanup;
		}
	}
	else
	{
		result = LZ_ERROR_PARAM;
		goto cleanup;
	}

	hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if(!hSCManager)
	{
		result = LZ_ERROR_SERVICE_MANAGER_OPEN;
		goto cleanup;
	}

	hService = CreateServiceW(
		hSCManager,
		serviceNameW,
		serviceNameW,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		servicePathW,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);

	if(!hService)
	{
		result = LZ_ERROR_CREATE_SERVICE;
		goto cleanup;
	}

	result = 0;

	cleanup:
	if (hService)
		CloseServiceHandle(hService);
	if (hSCManager)
		CloseServiceHandle(hSCManager);

	if (serviceNameW)
		free(serviceNameW);
	if (servicePathW)
		free(servicePathW);

	return result;
}

int LzRemoveService(const char* serviceName)
{
	int result;

	wchar_t* serviceNameW = 0;

	SC_HANDLE hSCManager = 0;
	SC_HANDLE hService = 0;

	if (serviceName)
	{
		serviceNameW = LzUnicode_UTF8toUTF16_dup(serviceName);

		if (!serviceNameW)
		{
			result = LZ_ERROR_PARAM;
			goto cleanup;
		}
	}
	else
	{
		result = LZ_ERROR_PARAM;
		goto cleanup;
	}

	hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCManager)
	{
		result = LZ_ERROR_SERVICE_MANAGER_OPEN;
		goto cleanup;
	}

	hService = OpenServiceW(hSCManager, serviceNameW, SERVICE_STOP | DELETE);
	if(!hService)
	{
		result = LZ_ERROR_OPEN_SERVICE;
		goto cleanup;
	}

	if (!DeleteService(hService))
	{
		result = LZ_ERROR_REMOVE_SERVICE;
		goto cleanup;
	}

	result = 0;

	cleanup:
	if (hService)
		CloseServiceHandle(hService);
	if (hSCManager)
		CloseServiceHandle(hSCManager);

	if (serviceNameW)
		free(serviceNameW);

	return result;
}

static void FreeWideArgs(int argc, wchar_t** argvW)
{
	assert(argvW != NULL);

	for (int i = 0; i < argc; ++i)
	{
		free(argvW[i + 1]);
	}

	free(argvW);
}

static int ConvertArgvToWideArgs(int argc, const char** argv, wchar_t*** pArgvW)
{
	int result;
	wchar_t** argvW = 0;

	assert(argc != 0);
	assert(argv != NULL);
	assert(pArgvW != NULL);

	if (argc > 0)
	{
		// allocate with null-terminator at the end
		argvW = calloc(argc + 1, sizeof(wchar_t*));
		if (!argvW)
		{
			result = LZ_ERROR_MEM;
			goto error;
		}

		for (int i = 0; i < argc; ++i)
		{
			argvW[i] = LzUnicode_UTF8toUTF16_dup(argv[i]);

			if (!argvW[i])
			{
				result = LZ_ERROR_PARAM;
				goto error;
			}
		}
	}

	*pArgvW = argvW;
	return LZ_OK;

	error:
	if (argvW)
		FreeWideArgs(argc, argvW);
	return result;
}

int GetServiceState(SC_HANDLE hService, DWORD* currentState)
{
	SERVICE_STATUS_PROCESS ssp;
	ZeroMemory(&ssp, sizeof(SERVICE_STATUS_PROCESS));
	DWORD dwBytesNeeded;

	assert(hService != NULL);
	assert(currentState != NULL);

	if (!QueryServiceStatusEx(
		hService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded ))
	{
		return LZ_ERROR_QUERY_SERVICE_INFO;
	}

	*currentState = ssp.dwCurrentState;
	return LZ_OK;
}

int LzStartService(const char* serviceName, int argc, const char** argv)
{
	int result;

	wchar_t* serviceNameW = 0;
	wchar_t** argvW = 0;

	SC_HANDLE hSCManager = 0;
	SC_HANDLE hService = 0;

	DWORD startTime = 0;
	DWORD serviceState = 0;

	if (serviceName)
	{
		serviceNameW = LzUnicode_UTF8toUTF16_dup(serviceName);

		if (!serviceNameW)
		{
			result = LZ_ERROR_PARAM;
			goto cleanup;
		}
	}
	else
	{
		result = LZ_ERROR_PARAM;
		goto cleanup;
	}

	if (argc)
	{
		result = ConvertArgvToWideArgs(argc, argv, &argvW);
		if (!argvW)
		{
			goto cleanup;
		}
	}

	hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCManager)
	{
		result = LZ_ERROR_SERVICE_MANAGER_OPEN;
		goto cleanup;
	}

	hService = OpenServiceW(hSCManager, serviceNameW, SC_MANAGER_ALL_ACCESS);
	if(!hService)
	{
		result = LZ_ERROR_OPEN_SERVICE;
		goto cleanup;
	}

	if(!StartServiceW(hService, argc, argvW))
	{
		result = LZ_ERROR_START_SERVICE;
		goto cleanup;
	}

	// Wait for service to run

	if (GetServiceState(hService, &serviceState) != LZ_OK)
	{
		result = LZ_ERROR_QUERY_SERVICE_INFO;
		goto cleanup;
	}

	while (serviceState != SERVICE_RUNNING)
	{
		Sleep(SERVICE_STATUS_QUERY_INTERVAL);

		if (GetServiceState(hService, &serviceState) != LZ_OK)
		{
			result = LZ_ERROR_QUERY_SERVICE_INFO;
			goto cleanup;
		}

		if (serviceState == SERVICE_RUNNING)
		{
			break;
		}

		if (GetTickCount() - startTime > SERVICE_START_TIMEOUT)
		{
			result = LZ_ERROR_SERVICE_START_TIMEOUT;
			goto cleanup;
		}
	}

	result = 0;

	cleanup:
	if (hService)
		CloseServiceHandle(hService);
	if (hSCManager)
		CloseServiceHandle(hSCManager);
	if (argvW)
		FreeWideArgs(argc, argvW);
	if (serviceNameW)
		free(serviceNameW);

	return result;
}

int LzStopService(const char* serviceName)
{
	int result;

	wchar_t* serviceNameW = 0;

	SC_HANDLE hSCManager = 0;
	SC_HANDLE hService = 0;

	DWORD startTime = 0;
	DWORD serviceState = 0;

	SERVICE_STATUS_PROCESS ssp;
	ZeroMemory(&ssp, sizeof(SERVICE_STATUS_PROCESS));

	if (serviceName)
	{
		serviceNameW = LzUnicode_UTF8toUTF16_dup(serviceName);

		if (!serviceNameW)
		{
			result = LZ_ERROR_PARAM;
			goto cleanup;
		}
	}
	else
	{
		result = LZ_ERROR_PARAM;
		goto cleanup;
	}

	hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCManager)
	{
		result = LZ_ERROR_PARAM;
		goto cleanup;
	}

	hService = OpenServiceW(hSCManager, serviceNameW, SC_MANAGER_ALL_ACCESS);
	if(!hService)
	{
		result = LZ_ERROR_PARAM;
		goto cleanup;
	}

	if (GetServiceState(hService, &serviceState) != LZ_OK)
	{
		result = LZ_ERROR_QUERY_SERVICE_INFO;
		goto cleanup;
	}

	if (serviceState == SERVICE_STOPPED)
	{
		result = LZ_OK;
		goto cleanup;
	}

	// If a stop is pending, wait for it.
	startTime = GetTickCount();

	while (serviceState == SERVICE_STOP_PENDING)
	{
		Sleep(SERVICE_STATUS_QUERY_INTERVAL);

		if (GetServiceState(hService, &serviceState) != LZ_OK)
		{
			result = LZ_ERROR_QUERY_SERVICE_INFO;
			goto cleanup;
		}

		if (serviceState == SERVICE_STOPPED)
		{
			result = LZ_OK;
			goto cleanup;
		}

		if (GetTickCount() - startTime > SERVICE_STOP_TIMEOUT)
		{
			result = LZ_ERROR_SERVICE_STOP_TIMEOUT;
			goto cleanup;
		}
	}

	if (!ControlService(
		hService,
		SERVICE_CONTROL_STOP,
		(LPSERVICE_STATUS) &ssp))
	{
		result = LZ_ERROR_STOP_SERVICE;
		goto cleanup;
	}

	startTime = GetTickCount();

	while (serviceState != SERVICE_STOPPED)
	{
		Sleep(SERVICE_STATUS_QUERY_INTERVAL);
		if (GetServiceState(hService, &serviceState) != LZ_OK)
		{
			result = LZ_ERROR_QUERY_SERVICE_INFO;
			goto cleanup;
		}

		if (serviceState == SERVICE_STOPPED)
			break;

		if (GetTickCount() - startTime > SERVICE_STOP_TIMEOUT)
		{
			result = LZ_ERROR_SERVICE_STOP_TIMEOUT;
			goto cleanup;
		}
	}

	result = LZ_OK;

	cleanup:
	if (hService)
		CloseServiceHandle(hService);
	if (hSCManager)
		CloseServiceHandle(hSCManager);

	if (serviceNameW)
		free(serviceNameW);

	return result;
}

int LzIsServiceLaunched(const char* serviceName)
{
	int result;

	wchar_t* serviceNameW = 0;

	SC_HANDLE hSCManager = 0;
	SC_HANDLE hService = 0;

	DWORD serviceState = 0;

	if (serviceName)
	{
		serviceNameW = LzUnicode_UTF8toUTF16_dup(serviceName);

		if (!serviceNameW)
		{
			result = LZ_ERROR_PARAM;
			goto cleanup;
		}
	}
	else
	{
		result = LZ_ERROR_PARAM;
		goto cleanup;
	}

	hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(!hSCManager)
	{
		result = LZ_ERROR_SERVICE_MANAGER_OPEN;
		goto cleanup;
	}

	hService = OpenServiceW(hSCManager, serviceNameW, SC_MANAGER_ALL_ACCESS);
	if(!hService)
	{
		result = LZ_ERROR_OPEN_SERVICE;
		goto cleanup;
	}

	if (GetServiceState(hService, &serviceState) != LZ_OK)
	{
		result = LZ_ERROR_QUERY_SERVICE_INFO;
		goto cleanup;
	}

	result = (serviceState != SERVICE_STOPPED) ? TRUE : FALSE;

	cleanup:
	if (hService)
		CloseServiceHandle(hService);
	if (hSCManager)
		CloseServiceHandle(hSCManager);

	if (serviceNameW)
		free(serviceNameW);

	return result;
}
