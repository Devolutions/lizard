/*
 * Recommended command-line:
 *  ASAN_OPTIONS="allocator_may_return_null=1" ./fuzzing/FuzzLizard -detect_leaks=0 corpus/
 */

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include <lizard/lizard.h>

void Fuzz_LzArchive(const uint8_t* data, size_t data_size)
{
    LzArchive* archive;
    int status;
    int index;
    
    archive = LzArchive_New();
    status = LzArchive_OpenData(archive, data, data_size);

    if (status == LZ_OK)
    {
        LzArchive_Count(archive);
        LzArchive_IsDir(archive, 0);
        LzArchive_IsDir(archive, 1);
        LzArchive_IsDir(archive, 2);
        
        index = LzArchive_Find(archive, "colors.json");

        if (index >= 0)
        {
            uint8_t* outputData;
            size_t outputSize;
            status = LzArchive_ExtractData(archive, 0, "colors.json", &outputData, &outputSize);

            if (status == LZ_OK)
            {
                free(outputData);
            }
        }

        LzArchive_Close(archive);
    }

    LzArchive_Free(archive);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t data_size)
{
	Fuzz_LzArchive(data, data_size);
	return 0;
}