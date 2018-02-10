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

BOOL ExtractFromArchive(WCHAR* fileName, WCHAR* destinationFolder, BYTE* archiveData, size_t archiveSize);

#ifdef __cplusplus
}
#endif

#endif /* LIZARD_API_H */
