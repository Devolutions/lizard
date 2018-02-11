#ifndef LIZARD_API_H
#define LIZARD_API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int lizard_main(int argc, char** args);

bool lizard_archive_buffer_extract_to_file(uint8_t* archiveData, size_t archiveSize,
	uint16_t* filename, uint16_t* outputPath);

typedef struct lz_archive LzArchive;

int LzFile_Seek(FILE* fp, uint64_t offset, int origin);
uint64_t LzFile_Tell(FILE* fp);
uint64_t LzFile_Size(const char* filename);
const char* LzFile_Base(const char* filename);
char* LzFile_Dir(const char* filename);
const char* LzFile_Extension(const char* filename, bool dot);
FILE* LzFile_Open(const char* path, const char* mode);
uint8_t* LzFile_Load(const char* filename, size_t* size, uint32_t flags);

int LzArchive_NumFiles(LzArchive* ctx);

int LzArchive_OpenData(LzArchive* ctx, const uint8_t* data, size_t size);
int LzArchive_Close(LzArchive* ctx);

LzArchive* LzArchive_New(void);
void LzArchive_Free(LzArchive* ctx);

#ifdef __cplusplus
}
#endif

#endif /* LIZARD_API_H */
