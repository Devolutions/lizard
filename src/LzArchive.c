
#include <lizard/lizard.h>

#include "7z.h"
#include "7zCrc.h"
#include "7zAlloc.h"
#include "7zMemInStream.h"

static bool g_Initialized = false;

struct lz_archive
{
	CSzArEx db;
	bool initialized;
	ISzAlloc allocator;
	size_t archiveSize;
	uint8_t* archiveData;
	CMemInStream dataStream;
};

int LzMapRes(int res)
{
	int status = LZ_ERROR_FAIL;

	switch (res)
	{
		case SZ_OK: status = LZ_OK; break;
		case SZ_ERROR_DATA: status = LZ_ERROR_DATA; break;
		case SZ_ERROR_MEM: status = LZ_ERROR_MEM; break;
		case SZ_ERROR_CRC: status = LZ_ERROR_CRC; break;
		case SZ_ERROR_UNSUPPORTED: status = LZ_ERROR_UNSUPPORTED; break;
		case SZ_ERROR_PARAM: status = LZ_ERROR_PARAM; break;
		case SZ_ERROR_INPUT_EOF: status = LZ_ERROR_INPUT_EOF; break;
		case SZ_ERROR_OUTPUT_EOF: status = LZ_ERROR_OUTPUT_EOF; break;
		case SZ_ERROR_READ: status = LZ_ERROR_READ; break;
		case SZ_ERROR_WRITE: status = LZ_ERROR_WRITE; break;
		case SZ_ERROR_PROGRESS: status = LZ_ERROR_PROGRESS; break;
		case SZ_ERROR_FAIL: status = LZ_ERROR_FAIL; break;
		case SZ_ERROR_THREAD: status = LZ_ERROR_THREAD; break;
		case SZ_ERROR_ARCHIVE: status = LZ_ERROR_ARCHIVE; break;
		case SZ_ERROR_NO_ARCHIVE: status = LZ_ERROR_NO_ARCHIVE; break;
	}

	return status;
}

void LzInit()
{
	if (g_Initialized)
		return;

	CrcGenerateTable();
	g_Initialized = true;
}

int LzArchive_Init(LzArchive* ctx)
{
	if (ctx->initialized)
		return LZ_OK;

	LzInit();
	
	SzArEx_Init(&ctx->db);

	ctx->allocator.Alloc = SzAlloc;
	ctx->allocator.Free = SzFree;

	ctx->initialized = true;
	
	return LZ_OK;
}

int LzArchive_OpenData(LzArchive* ctx, const uint8_t* data, size_t size)
{
	int status;

	status = LzArchive_Init(ctx);

	if (status != LZ_OK)
		return status;

	MemInStream_Init(&ctx->dataStream, data, size);

	status = LzMapRes(SzArEx_Open(&ctx->db, &ctx->dataStream.s, &ctx->allocator, &ctx->allocator));

	if (status != LZ_OK)
		return status;

	return LZ_OK;
}

int LzArchive_OpenFile(LzArchive* ctx, const char* filename)
{
	int status;

	ctx->archiveData = LzFile_Load(filename, &ctx->archiveSize, 0);

	if (!ctx->archiveData)
		return LZ_ERROR_FILE;

	status = LzArchive_OpenData(ctx, ctx->archiveData, ctx->archiveSize);

	return status;
}

int LzArchive_Count(LzArchive* ctx)
{
	return (int) ctx->db.NumFiles;
}

bool LzArchive_IsDir(LzArchive* ctx, int index)
{
	const CSzArEx* db = &ctx->db;

	if (index >= (int) db->NumFiles)
		return false;

	return SzArEx_IsDir(db, index) ? true : false;
}

size_t LzArchive_GetFileSize(LzArchive* ctx, int index)
{
	const CSzArEx* db = &ctx->db;

	if (index >= (int) db->NumFiles)
		return 0;

	return SzArEx_GetFileSize(db, index);
}

int LzArchive_GetFileName(LzArchive* ctx, int index, char* filename, int cch)
{
	int status;
	const uint16_t* src;
	size_t offset, length;
	const CSzArEx* db = &ctx->db;

	if (index >= (int) db->NumFiles)
		return LZ_ERROR_PARAM;

	offset = db->FileNameOffsets[index];
	length = db->FileNameOffsets[index + 1] - offset;
	src = &((uint16_t*) db->FileNames)[offset];

	status = LzUnicode_UTF16toUTF8(src, length, (uint8_t*) filename, cch);

	return status;
}

int LzArchive_Find(LzArchive* ctx, const char* filename)
{
	int index;
	int count;
	int status;
	bool found = false;
	char current[LZ_MAX_PATH];

	count = LzArchive_Count(ctx);

	for (index = 0; index < count; index++)
	{
		LzArchive_GetFileName(ctx, index, current, sizeof(current));

		if (!strcmp(current, filename))
		{
			found = true;
			break;
		}
	}

	status = found ? index : LZ_ERROR_NOT_FOUND;

	return status;
}

int LzArchive_ExtractData(LzArchive* ctx, int index, const char* filename, uint8_t** outputData, size_t* outputSize)
{
	int status;
	uint8_t* buffer = NULL;
	size_t offset = 0;
	size_t processedSize = 0;
	uint32_t blockIndex = 0xFFFFFFFF;
	const CSzArEx* db = &ctx->db;

	if (!outputData || !outputSize)
		return LZ_ERROR_PARAM;

	*outputData = NULL;
	*outputSize = 0;

	if (index < 0)
		index = LzArchive_Find(ctx, filename);

	if ((index < 0) || (index >= (int) db->NumFiles))
		return LZ_ERROR_PARAM;

	if (SzArEx_IsDir(db, index))
		return LZ_ERROR_PARAM;

	status = LzMapRes(SzArEx_Extract(db, &ctx->dataStream.s, index, &blockIndex,
		&buffer, outputSize, &offset, &processedSize, &ctx->allocator, &ctx->allocator));

	if (offset == 0)
	{
		*outputData = buffer;
		*outputSize = processedSize;
	}
	else
	{
		*outputData = (uint8_t*) calloc(processedSize, sizeof(uint8_t));

		if (!outputData)
		{
			status = LZ_ERROR_MEM;
			goto exit;
		}

		memcpy(*outputData, buffer + offset, processedSize);
		free(buffer);
	}

exit:
	if (status != LZ_OK)
		free(buffer);

	return status;
}

int LzArchive_ExtractFile(LzArchive* ctx, int index, const char* inputName, const char* outputName)
{
	int status;
	size_t outputSize = 0;
	uint8_t* outputData = NULL;
	const CSzArEx* db = &ctx->db;

	if (index < 0)
		index = LzArchive_Find(ctx, inputName);

	if (!outputName)
		outputName = inputName;

	status = LzArchive_ExtractData(ctx, index, inputName, &outputData, &outputSize);

	if (status != LZ_OK)
		return status;

	if (!LzFile_Save(outputName, outputData, outputSize, 0))
		status = LZ_ERROR_FILE;

	return status;
}

int LzArchive_Close(LzArchive* ctx)
{
	SzArEx_Free(&ctx->db, &ctx->allocator);

	if (ctx->archiveData)
	{
		ctx->archiveData = NULL;
		free(ctx->archiveData);
	}

	return LZ_OK;
}

LzArchive* LzArchive_New(void)
{
	LzArchive* ctx;

	ctx = (LzArchive*) calloc(1, sizeof(LzArchive));

	if (!ctx)
		return NULL;

	return ctx;
}

void LzArchive_Free(LzArchive* ctx)
{
	if (!ctx)
		return;

	free(ctx);
}
