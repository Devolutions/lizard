
#ifdef _WIN32
#include <windows.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

#include <lizard/lizard.h>

int LzEnv_GetEnv(const char* name, char* value, int cch)
{
	int status;

#ifdef _WIN32
	uint16_t nameW[512];
	uint16_t valueW[LZ_MAX_PATH];

	LzUnicode_UTF8toUTF16((uint8_t*) name, -1, nameW, sizeof(nameW) / 2);

	if (GetEnvironmentVariableW(nameW, valueW, sizeof(valueW) / 2) == 0)
		return -1;

	if (LzUnicode_UTF16toUTF8(valueW, -1, (uint8_t*) value, cch) < 1)
		return -1;

	status = strlen(value);
#else
	int len;
	char* env;

	env = getenv(name);

	if (!env)
		return -1;

	len = strlen(name);

	if (len < 1)
		return -1;

	status = len + 1;

	if (value && (cch > 0))
	{
		if (cch >= (len + 1))
		{
			strncpy(value, env, cch);
			value[cch - 1] = '\0';
			status = len;
		}
	}
#endif

	return status;
}

bool LzEnv_SetEnv(const char* name, const char* value)
{
#ifdef _WIN32
	uint16_t nameW[512];
	uint16_t valueW[LZ_MAX_PATH];

	if (!name)
		return false;

	LzUnicode_UTF8toUTF16((uint8_t*) name, -1, nameW, sizeof(nameW) / 2);
	LzUnicode_UTF8toUTF16((uint8_t*) value, -1, valueW, sizeof(valueW) / 2);

	return SetEnvironmentVariableW(nameW, valueW) ? true : false;
#else
	if (!name)
		return false;

	if (value)
	{
		if (setenv(name, value, 1) != 0)
			return false;
	}
	else
	{
		if (unsetenv(name) != 0)
			return false;
	}

	return true;
#endif
}

int LzEnv_GetCwd(char* path, int cch)
{
	int status;

#ifdef _WIN32
	uint16_t pathW[LZ_MAX_PATH];

	if (!GetCurrentDirectoryW(sizeof(pathW) / 2, pathW))
		return -1;

	if (LzUnicode_UTF16toUTF8(pathW, -1, (uint8_t*) path, cch) < 1)
		return -1;

	status = strlen(path);
#else
	int len;
	char* cwd;

	cwd = getcwd(NULL, 0);

	if (!cwd)
		return -1;

	len = strlen(cwd);

	if (len < 1)
		return -1;

	status = len + 1;

	if (path && (cch > 0))
	{
		if (cch >= (len + 1))
		{
			strncpy(path, cwd, cch);
			path[cch - 1] = '\0';
			status = len;
		}
	}
#endif

	return status;
}

bool LzEnv_SetCwd(const char* path)
{
	int status;

#ifdef _WIN32
	uint16_t pathW[LZ_MAX_PATH];

	LzUnicode_UTF8toUTF16((uint8_t*) path, -1, pathW, sizeof(pathW) / 2);

	status = SetCurrentDirectoryW(pathW);
	return (status != 0) ? true : false;
#else
	status = chdir(path);
	return (status == 0) ? true : false;
#endif
}

int LzEnv_GetTempPath(char* path, int cch)
{
	int status;

#ifdef _WIN32
	uint16_t pathW[LZ_MAX_PATH];

	if (!GetTempPathW(sizeof(pathW) / 2, pathW))
		return -1;

	if (LzUnicode_UTF16toUTF8(pathW, -1, (uint8_t*) path, cch) < 1)
		return -1;

	status = strlen(path);
#else
	status = LzEnv_GetEnv("TEMP", path, cch);
#endif

	return status;
}
