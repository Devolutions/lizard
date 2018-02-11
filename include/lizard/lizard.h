#ifndef LIZARD_API_H
#define LIZARD_API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct lz_archive LzArchive;

#define LZ_OK			0
#define LZ_ERROR_DATA		-1
#define LZ_ERROR_MEM		-2
#define LZ_ERROR_CRC		-3
#define LZ_ERROR_UNSUPPORTED	-4
#define LZ_ERROR_PARAM		-5
#define LZ_ERROR_INPUT_EOF	-6
#define LZ_ERROR_OUTPUT_EOF	-7
#define LZ_ERROR_READ		-8
#define LZ_ERROR_WRITE		-9
#define LZ_ERROR_PROGRESS	-10
#define LZ_ERROR_FAIL		-11
#define LZ_ERROR_THREAD		-12
#define LZ_ERROR_ARCHIVE	-16
#define LZ_ERROR_NO_ARCHIVE	-17
#define LZ_ERROR_FILE		-30
#define LZ_ERROR_NOT_FOUND	-31

#define LZ_MAX_PATH		1024

#ifdef __cplusplus
extern "C" {
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
int LzMkDir(const char* path, int mode);
int LzRmDir(const char* path);
int LzChMod(const char* filename, int mode);

int LzEnv_GetEnv(const char* name, char* value, int cch);
int LzEnv_GetCwd(char* path, int cch);
bool LzEnv_SetCwd(const char* path);

int LzUnicode_UTF8toUTF16(const uint8_t* src, int cchSrc, uint16_t* dst, int cchDst);
int LzUnicode_UTF16toUTF8(const uint16_t* src, int cchSrc, uint8_t* dst, int cchDst);

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

#ifdef __cplusplus
}
#endif

#endif /* LIZARD_API_H */
