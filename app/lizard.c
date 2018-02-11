
#include <lizard/lizard.h>

int main(int argc, char** argv)
{
	int index;
	int count;
	LzArchive* archive;
	size_t archiveSize;
	uint8_t* archiveData;
	char filename[LZ_MAX_PATH];

	archive = LzArchive_New();

	archiveData = LzFile_Load("sample.7z", &archiveSize, 0);

	LzArchive_OpenData(archive, archiveData, archiveSize);

	count = LzArchive_NumFiles(archive);

	for (index = 0; index < count; index++)
	{
		LzArchive_GetFileName(archive, index, filename, sizeof(filename));
		printf("%s\n", filename);

		LzArchive_Extract(archive, index, LzFile_Base(filename));
	}

	LzArchive_Close(archive);
	LzArchive_Free(archive);

	return 0;

	//return lizard_main(argc, argv);
}
