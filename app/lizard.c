
#include <lizard/lizard.h>

static bool lizard_extract = true;
static bool lizard_list = true;
static char* lizard_input = "";
static char* lizard_output = "";
static char* lizard_version = "1.0.0";
static bool lizard_verbose = false;

void lizard_print_help()
{
	printf(
		"Lizard: A 7-Zip packer that sticks\n"
		"\n"
		"Usage:\n"
		"    lizard [options]\n"
		"\n"
		"Options:\n"
		"    -e                extract files from archive\n"
		"    -l                list files from archive\n"
		"    -i <file>         input file\n"
		"    -o <path>         output path\n"
		"    -h                print help\n"
		"    -v                print version (%s)\n"
		"    -V                verbose mode\n"
		"\n", lizard_version
	);
}

void lizard_print_version()
{
	printf("lizard version %s\n", lizard_version);
}

int main(int argc, char** argv)
{
	int index;
	int count;
	char* arg;
	LzArchive* archive;
	char filename[LZ_MAX_PATH];
	char fullname[LZ_MAX_PATH];

	if (argc < 2)
	{
		lizard_print_help();
		return 1;
	}

	for (index = 1; index < argc; index++)
	{
		arg = argv[index];

		if ((strlen(arg) == 2) && (arg[0] == '-'))
		{
			switch (arg[1])
			{
				case 'e':
					lizard_extract = true;
					break;

				case 'l':
					lizard_list = true;
					break;

				case 'i':
					if ((index + 1) < argc)
					{
						lizard_input = argv[index + 1];
						index++;
					}
					break;

				case 'o':
					if ((index + 1) < argc)
					{
						lizard_output = argv[index + 1];
						index++;
					}
					break;

				case 'h':
					lizard_print_help();
					break;

				case 'v':
					lizard_print_version();
					break;

				case 'V':
					lizard_verbose = true;
					break;
			}
		}
	}

	archive = LzArchive_New();

	if (LzArchive_OpenFile(archive, lizard_input) != LZ_OK)
	{
		fprintf(stderr, "could not open file: %s\n", lizard_input);
		return 0;
	}

	count = LzArchive_Count(archive);

	if (lizard_extract)
	{
		if (strlen(lizard_output) > 0)
		{
			LzMkDir(lizard_output, 0);
		}
	}

	for (index = 0; index < count; index++)
	{
		LzArchive_GetFileName(archive, index, filename, sizeof(filename));

		if (lizard_list)
		{
			printf("%s\n", filename);
		}

		if (lizard_extract)
		{
			LzArchive_ExtractFile(archive, index, filename, filename);
		}
	}

	LzArchive_Close(archive);
	LzArchive_Free(archive);

	return 0;
}
