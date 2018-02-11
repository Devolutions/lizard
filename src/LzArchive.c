
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
	CMemInStream dataStream;
};

void Lizard_Init()
{
	if (g_Initialized)
		return;

	CrcGenerateTable();
	g_Initialized = true;
}

int LzArchive_Init(LzArchive* ctx)
{
	if (ctx->initialized)
		return SZ_OK;

	Lizard_Init();
	
	SzArEx_Init(&ctx->db);

	ctx->allocator.Alloc = SzAlloc;
	ctx->allocator.Free = SzFree;

	ctx->initialized = true;
	
	return SZ_OK;
}

int LzArchive_OpenData(LzArchive* ctx, const uint8_t* data, size_t size)
{
	SRes res;
	int status;

	status = LzArchive_Init(ctx);

	if (status != SZ_OK)
		return status;

	MemInStream_Init(&ctx->dataStream, data, size);

	res = SzArEx_Open(&ctx->db, &ctx->dataStream.s, &ctx->allocator, &ctx->allocator);

	if (res != SZ_OK)
		return -1;

	return SZ_OK;
}

int LzArchive_NumFiles(LzArchive* ctx)
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
		return false;

	return SzArEx_GetFileSize(db, index);
}

int LzArchive_GetFileName(LzArchive* ctx, int index, char* filename, int cch)
{
	int status;
	const uint16_t* src;
	size_t offset, length;
	const CSzArEx* db = &ctx->db;

	if (index >= (int) db->NumFiles)
		return -1;

	offset = db->FileNameOffsets[index];
	length = db->FileNameOffsets[index + 1] - offset;
	src = &((uint16_t*) db->FileNames)[offset];

	status = LzUnicode_UTF16toUTF8(src, length, (uint8_t*) filename, cch);

	return status;
}

int LzArchive_Extract(LzArchive* ctx, int index, const char* filename)
{
	SRes res;
	size_t offset = 0;
	size_t outputSize = 0;
	uint8_t* outputData = NULL;
	size_t processedSize = 0;
	uint32_t blockIndex = 0xFFFFFFFF;
	uint8_t* extractedData = NULL;
	size_t extractedSize = 0;
	const CSzArEx* db = &ctx->db;

	if (index >= (int) db->NumFiles)
		return -1;

	if (SzArEx_IsDir(db, index))
		return -1;

	res = SzArEx_Extract(db, &ctx->dataStream.s, index, &blockIndex,
		&outputData, &outputSize, &offset, &processedSize, &ctx->allocator, &ctx->allocator);

	extractedData = &outputData[offset];
	extractedSize = processedSize;

	LzFile_Save(filename, extractedData, extractedSize, 0);

	return res;
}

int LzArchive_Close(LzArchive* ctx)
{
	SzArEx_Free(&ctx->db, &ctx->allocator);
	return SZ_OK;
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
