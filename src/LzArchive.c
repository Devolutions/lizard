
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

int LzArchive_Close(LzArchive* ctx)
{
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
