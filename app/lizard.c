
#include <lizard/lizard.h>

int main(int argc, char** argv)
{
	LzArchive* archive;
	size_t archiveSize;
	uint8_t* archiveData;

	archive = LzArchive_New();

	archiveData = LzFile_Load("sample.7z", &archiveSize, 0);

	LzArchive_OpenData(archive, archiveData, archiveSize);

	LzArchive_Free(archive);

	return 0;

	//return lizard_main(argc, argv);
}
