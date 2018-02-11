
#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#include <shlwapi.h>
#include <shlobj.h>
#pragma comment(lib, "shlwapi.lib")
#endif

#include <lizard/lizard.h>

int LzFile_Seek(FILE* fp, uint64_t offset, int origin)
{
#ifdef _WIN32
	return (int) _fseeki64(fp, offset, origin);
#elif defined(__APPLE__)
	return (int) fseeko(fp, offset, origin);
#else
	return (int) fseeko(fp, offset, origin);
#endif
}

uint64_t LzFile_Tell(FILE* fp)
{
#ifdef _WIN32
	return (uint64_t) _ftelli64(fp);
#elif defined(__APPLE__)
	return (uint64_t) ftello(fp);
#else
	return (uint64_t) ftello(fp);
#endif
}

uint64_t LzFile_Size(const char* filename)
{
	FILE* fp;
	uint64_t fileSize;

	fp = fopen(filename, "r");

	if (!fp)
		return 0;

	LzFile_Seek(fp, 0, SEEK_END);
	fileSize = LzFile_Tell(fp);
	fclose(fp);

	return fileSize;
}

const char* LzFile_Base(const char* filename)
{
	size_t length;
	char* separator;

	if (!filename)
		return NULL;

	separator = strrchr(filename, '\\');

	if (!separator)
		separator = strrchr(filename, '/');

	if (!separator)
		return filename;

	length = strlen(filename);

	if ((length - (separator - filename)) > 1)
		return separator + 1;

	return filename;
}

char* LzFile_Dir(const char* filename)
{
	char* dir;
	char* end;
	char* base;
	size_t length;

	base = (char*) LzFile_Base(filename);

	if (!base)
		return NULL;

	end = base - 1;

	if (end < filename)
		return NULL;

	length = end - filename;
	dir = malloc(length + 1);

	if (!dir)
		return NULL;

	CopyMemory(dir, filename, length);
	dir[length] = '\0';

	return dir;
}

const char* LzFile_Extension(const char* filename, bool dot)
{
	char* p;
	size_t length;

	if (!filename)
		return NULL;

	p = strrchr(filename, '.');

	if (!p)
		return NULL;

	if (dot)
		return p;

	length = strlen(filename);

	if ((length - (p - filename)) > 1)
		return p + 1;

	return NULL;
}

FILE* LzFile_Open(const char* path, const char* mode)
{
#ifndef _WIN32
	return fopen(path, mode);
#else
	FILE* fp = NULL;
	uint16_t modeW[32];
	uint16_t pathW[LZ_MAX_PATH];

	if (!path || !mode)
		return NULL;

	LzUnicode_UTF8toUTF16((uint8_t*) mode, -1, modeW, sizeof(modeW) / 2);
	LzUnicode_UTF8toUTF16((uint8_t*) path, -1, pathW, sizeof(pathW) / 2);

	fp = _wfopen(pathW, modeW);

	return fp;
#endif
}

uint8_t* LzFile_Load(const char* filename, size_t* size, uint32_t flags)
{
	FILE* fp = NULL;
	uint8_t* data = NULL;

	if (!filename || !size)
		return NULL;

	*size = 0;

	fp = LzFile_Open(filename, "rb");

	if (!fp)
		return NULL;

	LzFile_Seek(fp, 0, SEEK_END);
	*size = LzFile_Tell(fp);
	LzFile_Seek(fp, 0, SEEK_SET);

	data = malloc(*size + 1);

	if (!data)
		goto exit;

	data[*size] = '\0';

	if (fread(data, 1, *size, fp) != *size)
	{
		free(data);
		data = NULL;
		*size = 0;
	}

	if (flags)
		*size = *size + 1;

exit:
	fclose(fp);
	return data;
}

bool LzFile_Save(const char* filename, uint8_t* data, size_t size, uint32_t flags)
{
	FILE* fp = NULL;
	bool success = true;

	if (!filename || !data)
		return false;

	fp = LzFile_Open(filename, "wb");

	if (!fp)
		return false;

	if (fwrite(data, 1, size, fp) != size)
	{
		success = false;
	}

	fclose(fp);
	return success;
}
