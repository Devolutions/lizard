#ifndef LIZARD_API_H
#define LIZARD_API_H

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct lz_archive LzArchive;

#define LZ_OK							0
#define LZ_ERROR_DATA					-1
#define LZ_ERROR_MEM					-2
#define LZ_ERROR_CRC					-3
#define LZ_ERROR_UNSUPPORTED			-4
#define LZ_ERROR_PARAM					-5
#define LZ_ERROR_INPUT_EOF				-6
#define LZ_ERROR_OUTPUT_EOF				-7
#define LZ_ERROR_READ					-8
#define LZ_ERROR_WRITE					-9
#define LZ_ERROR_PROGRESS				-10
#define LZ_ERROR_FAIL					-11
#define LZ_ERROR_THREAD					-12
#define LZ_ERROR_ARCHIVE				-16
#define LZ_ERROR_NO_ARCHIVE				-17
#define LZ_ERROR_FILE					-30
#define LZ_ERROR_NOT_FOUND				-31
#define LZ_ERROR_UNEXPECTED				-32
#define LZ_ERROR_SERVICE_MANAGER_OPEN	-33
#define LZ_ERROR_CREATE_SERVICE	        -34
#define LZ_ERROR_OPEN_SERVICE			-35
#define LZ_ERROR_REMOVE_SERVICE			-36
#define LZ_ERROR_START_SERVICE			-37
#define LZ_ERROR_QUERY_SERVICE_INFO		-38
#define LZ_ERROR_SERVICE_START_TIMEOUT	-39
#define LZ_ERROR_SERVICE_STOP_TIMEOUT	-40
#define LZ_ERROR_STOP_SERVICE			-41

#define LZ_MAX_PATH		1024

#ifdef _WIN32
#define lz_snprintf		sprintf_s
#else
#define lz_snprintf		snprintf
#endif

#define LZ_PATH_SLASH_CHR		'/'
#define LZ_PATH_SLASH_STR		"/"

#define LZ_PATH_BACKSLASH_CHR		'\\'
#define LZ_PATH_BACKSLASH_STR		"\\"

#ifdef _WIN32
#define LZ_PATH_SEPARATOR_CHR		LZ_PATH_BACKSLASH_CHR
#define LZ_PATH_SEPARATOR_STR		LZ_PATH_BACKSLASH_STR
#else
#define LZ_PATH_SEPARATOR_CHR		LZ_PATH_SLASH_CHR
#define LZ_PATH_SEPARATOR_STR		LZ_PATH_SLASH_STR
#endif

#define LZ_PATH_STYLE_NATIVE		0
#define LZ_PATH_STYLE_WINDOWS		1
#define LZ_PATH_STYLE_UNIX		2

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
typedef BOOL (WINAPI* fnIsWow64Process)(HANDLE hProcess, PBOOL Wow64Process);
typedef BOOL (WINAPI* fnWow64DisableWow64FsRedirection)(PVOID* OldValue);
typedef BOOL (WINAPI* fnWow64RevertWow64FsRedirection)(PVOID OldValue);
#endif

int LzFile_Seek(FILE* fp, uint64_t offset, int origin);
uint64_t LzFile_Tell(FILE* fp);
uint64_t LzFile_Size(const char* filename);
const char* LzFile_Base(const char* filename);
char* LzFile_Dir(const char* filename);
const char* LzFile_Extension(const char* filename, bool dot);
FILE* LzFile_Open(const char* path, const char* mode);
uint8_t* LzFile_Load(const char* filename, size_t* size, uint32_t padding);
bool LzFile_Save(const char* filename, uint8_t* data, size_t size, int mode);
bool LzFile_Move(const char* src, const char* dst, bool replace);
bool LzFile_Exists(const char* filename);
bool LzFile_Delete(const char* filename);
int LzMkPath(const char* path, int mode);
int LzMkDir(const char* path, int mode);
int LzRmDir(const char* path);
int LzChMod(const char* filename, int mode);

int LzEnv_GetEnv(const char* name, char* value, int cch);
int LzEnv_GetCwd(char* path, int cch);
bool LzEnv_SetCwd(const char* path);
int LzEnv_GetTempPath(char* path, int cch);

int LzPathCchAppend(char* path, int cchPath, const char* more);
int LzPathCchConvert(char* path, size_t cch, int style);

int LzUnicode_UTF8toUTF16(const uint8_t* src, int cchSrc, uint16_t* dst, int cchDst);
int LzUnicode_UTF16toUTF8(const uint16_t* src, int cchSrc, uint8_t* dst, int cchDst);
char* LzUnicode_UTF16toUTF8_dup(const uint16_t* src);
uint16_t* LzUnicode_UTF8toUTF16_dup(const char* src);

int LzArchive_Count(LzArchive* ctx);
bool LzArchive_IsDir(LzArchive* ctx, int index);
size_t LzArchive_GetFileSize(LzArchive* ctx, int index);
int LzArchive_GetFileName(LzArchive* ctx, int index, char* filename, int cch);
int LzArchive_Find(LzArchive* ctx, const char* filename);

int LzArchive_ExtractData(LzArchive* ctx, int index, const char* filename, uint8_t** outputData, size_t* outputSize);
int LzArchive_ExtractFile(LzArchive* ctx, int index, const char* inputName, const char* outputName);

int LzArchive_OpenData(LzArchive* ctx, const uint8_t* data, size_t size);
int LzArchive_OpenFile(LzArchive* ctx, const char* filename);

int LzArchive_Close(LzArchive* ctx);

LzArchive* LzArchive_New(void);
void LzArchive_Free(LzArchive* ctx);

#ifdef _WIN32
int LzSetEnv(const char* name, const char* value);

BOOL LzIsWow64();
int LzMessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
char* LzGetCommandLine();

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
	LPPROCESS_INFORMATION lpProcessInformation);
BOOL LzShellExecuteEx(SHELLEXECUTEINFOA *pExecInfo);

int LzInstallService(const char* serviceName, const char* servicePath);
int LzRemoveService(const char* serviceName);
int LzStartService(const char* serviceName, int argc, const char** argv);
int LzStopService(const char* serviceName);
int LzIsServiceLaunched(const char* serviceName);
BOOL LzGetModuleFileName(HMODULE module, LPSTR lpFileName, DWORD nSize);

extern fnWow64DisableWow64FsRedirection pfnWow64DisableWow64FsRedirection;
extern fnWow64RevertWow64FsRedirection pfnWow64RevertWow64FsRedirection;

typedef struct lz_http LzHttp;

typedef int(*fnHttpWriteFunction)(void* param, LzHttp* ctx, uint8_t* data, DWORD len);

LzHttp* LzHttp_New(const char* userAgent);
void LzHttp_Free(LzHttp* ctx);

int LzHttp_Get(LzHttp* ctx, const char* url, fnHttpWriteFunction writeCallback, void* param, DWORD* error);
char* LzHttp_Response(LzHttp* ctx);
uint32_t LzHttp_ResponseLength(LzHttp* ctx);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LIZARD_API_H */
