
#include <windows.h>
#include <strsafe.h>

#include <7z.h>
#include <7zAlloc.h>
#include <7zBuf.h>
#include <7zCrc.h>
#include <7zFile.h>
#include <7zMemInStream.h>

#include <lizard/lizard.h>

static ISzAlloc g_Alloc = {SzAlloc, SzFree};

static int Buf_EnsureSize(CBuf* dest, size_t size)
{
	if (dest->size >= size)
		return 1;
	Buf_Free(dest, &g_Alloc);
	
	return Buf_Create(dest, size, &g_Alloc);
}

static SRes Utf16_To_Char(CBuf* buf, const UInt16* s, UINT codePage)
{
	unsigned len = 0;
	
	for (len = 0; s[len] != 0; len++);
	
	unsigned size = len * 3 + 100;
		
	if (!Buf_EnsureSize(buf, size))
		return SZ_ERROR_MEM;
		
	buf->data[0] = 0;
			
	if (len != 0)
	{
		char defaultChar = '_';
		BOOL defUsed;
		unsigned numChars = 0;
			
		numChars = WideCharToMultiByte(codePage, 0, s, len, (char*) buf->data, size, &defaultChar,
			&defUsed);
				
		if (numChars == 0 || numChars >= size)
			return SZ_ERROR_FAIL;
			
		buf->data[numChars] = 0;
	}

	return SZ_OK;
}

static SRes PrintString(const UInt16* s)
{
	CBuf buf;
	SRes res;
	Buf_Init(&buf);
	res = Utf16_To_Char(&buf, s, CP_OEMCP);
	
	if (res == SZ_OK)
		fputs((const char*) buf.data, stdout);
	
	Buf_Free(&buf, &g_Alloc);
	return res;
}

static WRes MyCreateDir(const UInt16* name)
{
	return CreateDirectoryW(name, NULL) ? 0 : GetLastError();
}

static void UInt64ToStr(UInt64 value, char* s)
{
	char temp[32];
	int pos = 0;
	
	do
	{
		temp[pos++] = (char) ('0' + (unsigned) (value % 10));
		value /= 10;
	}
	while (value != 0);
	
	do
		*s++ = temp[--pos];
	while (pos);
	
	*s = '\0';
}

static char* UIntToStr(char* s, unsigned value, int numDigits)
{
	char temp[16];
	int pos = 0;
	
	do
		temp[pos++] = (char) ('0' + (value % 10));
	while (value /= 10);
	
	for (numDigits -= pos; numDigits > 0; numDigits--)
		*s++ = '0';
	
	do
		*s++ = temp[--pos];
	while (pos);
	
	*s = '\0';
	
	return s;
}

static void UIntToStr_2(char* s, unsigned value)
{
	s[0] = (char) ('0' + (value / 10));
	s[1] = (char) ('0' + (value % 10));
}

#define PERIOD_4 (4 * 365 + 1)
#define PERIOD_100 (PERIOD_4 * 25 - 1)
#define PERIOD_400 (PERIOD_100 * 4 + 1)

static void ConvertFileTimeToString(const CNtfsFileTime* nt, char* s)
{
	unsigned year, mon, hour, min, sec;
	Byte ms[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	unsigned t;
	UInt32 v;
	UInt64 v64 = nt->Low | ((UInt64) nt->High << 32);
	v64 /= 10000000;
	sec = (unsigned) (v64 % 60);
	v64 /= 60;
	min = (unsigned) (v64 % 60);
	v64 /= 60;
	hour = (unsigned) (v64 % 24);
	v64 /= 24;

	v = (UInt32) v64;

	year = (unsigned) (1601 + v / PERIOD_400 * 400);
	v %= PERIOD_400;

	t = v / PERIOD_100;
	if (t == 4)
		t = 3;
	year += t * 100;
	v -= t * PERIOD_100;
	t = v / PERIOD_4;
	if (t == 25)
		t = 24;
	year += t * 4;
	v -= t * PERIOD_4;
	t = v / 365;
	if (t == 4)
		t = 3;
	year += t;
	v -= t * 365;

	if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
		ms[1] = 29;
	for (mon = 0;; mon++)
	{
		unsigned d = ms[mon];
		if (v < d)
			break;
		v -= d;
	}
	s = UIntToStr(s, year, 4);
	*s++ = '-';
	UIntToStr_2(s, mon + 1);
	s[2] = '-';
	s += 3;
	UIntToStr_2(s, (unsigned) v + 1);
	s[2] = ' ';
	s += 3;
	UIntToStr_2(s, hour);
	s[2] = ':';
	s += 3;
	UIntToStr_2(s, min);
	s[2] = ':';
	s += 3;
	UIntToStr_2(s, sec);
	s[2] = 0;
}

void PrintError(char* sz)
{
	printf("\nERROR: %s\n", sz);
}

static void GetAttribString(UInt32 wa, Bool isDir, char* s)
{
	s[0] = (char) (((wa & FILE_ATTRIBUTE_DIRECTORY) != 0 || isDir) ? 'D' : '.');
	s[1] = (char) (((wa & FILE_ATTRIBUTE_READONLY) != 0) ? 'R' : '.');
	s[2] = (char) (((wa & FILE_ATTRIBUTE_HIDDEN) != 0) ? 'H' : '.');
	s[3] = (char) (((wa & FILE_ATTRIBUTE_SYSTEM) != 0) ? 'S' : '.');
	s[4] = (char) (((wa & FILE_ATTRIBUTE_ARCHIVE) != 0) ? 'A' : '.');
	s[5] = 0;
}

BOOL ExtractFromArchive(WCHAR* fileName, WCHAR* destinationFolder, BYTE* archiveData, size_t archiveSize)
{
	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;
	UInt16* temp = NULL;
	size_t tempSize = 0;
	CMemInStream archiveStream;
	BOOL fileExtracted = FALSE;
	WCHAR destinationPath[MAX_PATH + 1];

	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	MemInStream_Init(&archiveStream, archiveData, archiveSize);

	CrcGenerateTable();

	SzArEx_Init(&db);

	res = SzArEx_Open(&db, &archiveStream.s, &allocImp, &allocTempImp);

	if (res != SZ_OK)
	{
		PrintError("Failed to init archive");
		return FALSE;
	}

	UInt32 i;

	/*
	if you need cache, use these 3 variables.
	if you use external function, you can make these variable as static.
	*/
	UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
	Byte* outBuffer = 0; /* it must be 0 before first call for each new archive. */
	size_t outBufferSize = 0; /* it can have any value before first call (if outBuffer = 0) */

	for (i = 0; i < db.NumFiles; i++)
	{
		size_t offset = 0;
		size_t outSizeProcessed = 0;
		// const CSzFileItem *f = db.Files + i;
		size_t len;
		BOOL isDir = SzArEx_IsDir(&db, i);
		len = SzArEx_GetFileNameUtf16(&db, i, NULL);

		if (len > tempSize)
		{
			SzFree(NULL, temp);
			tempSize = len;
			temp = (UInt16*) SzAlloc(NULL, tempSize * sizeof(temp[0]));
			if (!temp)
			{
				res = SZ_ERROR_MEM;
				break;
			}
		}

		SzArEx_GetFileNameUtf16(&db, i, temp);

		if (wcscmp(temp, fileName) != 0)
			continue;

		char attr[8], s[32], t[32];
		UInt64 fileSize;

		GetAttribString(SzBitWithVals_Check(&db.Attribs, i) ? db.Attribs.Vals[i] : 0, isDir, attr);

		fileSize = SzArEx_GetFileSize(&db, i);
		UInt64ToStr(fileSize, s);

		if (SzBitWithVals_Check(&db.MTime, i))
			ConvertFileTimeToString(&db.MTime.Vals[i], t);
		else
		{
			size_t j;
			
			for (j = 0; j < 19; j++)
				t[j] = ' ';
			t[j] = '\0';
		}

		printf("%s %s %10s  ", t, attr, s);
		res = PrintString(temp);
		
		if (res != SZ_OK)
			break;

		res = SzArEx_Extract(&db, &archiveStream.s, i,
		                     &blockIndex, &outBuffer, &outBufferSize,
		                     &offset, &outSizeProcessed,
		                     &allocImp, &allocTempImp);
		if (res != SZ_OK)
			break;

		CSzFile outFile;
		size_t processedSize;
		size_t j;
		UInt16* name = temp;
		const UInt16* destPath = (const UInt16*) name;

		for (j = 0; name[j] != 0; j++)
		{
			if (name[j] == '/')
				destPath = name + j + 1;
		}

		StringCchPrintfW(destinationPath, sizeof(destinationPath), L"%s\\%s", destinationFolder, fileName);

		if (OutFile_OpenW(&outFile, destinationPath))
		{
			PrintError("can not open output file");
			res = SZ_ERROR_FAIL;
			break;
		}

		processedSize = outSizeProcessed;

		if (File_Write(&outFile, outBuffer + offset, &processedSize) != 0 || processedSize != outSizeProcessed)
		{
			PrintError("failed to write output file");
			res = SZ_ERROR_FAIL;
			break;
		}

		if (File_Close(&outFile))
		{
			PrintError("failed to close output file");
			res = SZ_ERROR_FAIL;
			break;
		}

		if (SzBitWithVals_Check(&db.Attribs, i))
			SetFileAttributesW(destPath, db.Attribs.Vals[i]);

		fileExtracted = TRUE;

		printf("\n");
	}
	IAlloc_Free(&allocImp, outBuffer);

	SzArEx_Free(&db, &allocImp);
	SzFree(NULL, temp);

	if (res == SZ_OK)
	{
		printf("\nEverything is Ok\n");
		return fileExtracted;
	}

	if (res == SZ_ERROR_UNSUPPORTED)
		PrintError("decoder doesn't support this archive");
	else if (res == SZ_ERROR_MEM)
		PrintError("can not allocate memory");
	else if (res == SZ_ERROR_CRC)
		PrintError("CRC error");
	else
		printf("\nERROR #%d\n", res);

	return fileExtracted;
}
