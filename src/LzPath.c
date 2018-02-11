
#include <lizard/lizard.h>

int LzPathCchAppend(char* path, int cchPath, const char* more)
{
	bool pathBackslash;
	bool moreBackslash;
	size_t moreLength;
	size_t pathLength;

	if (!path || !more || (cchPath < 1))
		return LZ_ERROR_PARAM;

	moreLength = strlen(more);
	pathLength = strlen(path);

	if (moreLength < 1)
		return LZ_OK; /* nothing to do */

	if (pathLength > 0)
		pathBackslash = (path[pathLength - 1] == LZ_PATH_SEPARATOR_CHR) ? true : false;
	else
		pathBackslash = true;

	moreBackslash = (more[0] == LZ_PATH_SEPARATOR_CHR) ? true : false;

	if (pathBackslash && moreBackslash)
	{
		if ((pathLength + moreLength - 1) < cchPath)
		{
			lz_snprintf(&path[pathLength], cchPath - pathLength, "%s", &more[1]);
			return LZ_OK;
		}
	}
	else if ((pathBackslash && !moreBackslash) || (!pathBackslash && moreBackslash))
	{
		if ((pathLength + moreLength) < cchPath)
		{
			lz_snprintf(&path[pathLength], cchPath - pathLength, "%s", more);
			return LZ_OK;
		}
	}
	else if (!pathBackslash && !moreBackslash)
	{
		if ((pathLength + moreLength + 1) < cchPath)
		{
			lz_snprintf(&path[pathLength], cchPath - pathLength, LZ_PATH_SEPARATOR_STR "%s", more);
			return LZ_OK;
		}
	}

	return LZ_ERROR_UNEXPECTED;
}

int LzPathCchConvert(char* path, size_t cch, int style)
{
	size_t index;

	if (style == LZ_PATH_STYLE_WINDOWS)
	{
		for (index = 0; index < cch; index++)
		{
			if (path[index] == LZ_PATH_SLASH_CHR)
				path[index] = LZ_PATH_BACKSLASH_CHR;
		}
	}
	else if (style == LZ_PATH_STYLE_UNIX)
	{
		for (index = 0; index < cch; index++)
		{
			if (path[index] == LZ_PATH_BACKSLASH_CHR)
				path[index] = LZ_PATH_SLASH_CHR;
		}
	}
	else if (style == LZ_PATH_STYLE_NATIVE)
	{
#ifdef _WIN32
		{
			/* Unix-style to Windows-style */

			for (index = 0; index < cch; index++)
			{
				if (path[index] == LZ_PATH_SLASH_CHR)
					path[index] = LZ_PATH_BACKSLASH_CHR;
			}
		}
#else
		{
			/* Windows-style to Unix-style */

			for (index = 0; index < cch; index++)
			{
				if (path[index] == LZ_PATH_BACKSLASH_CHR)
					path[index] = LZ_PATH_SLASH_CHR;
			}
		}
#endif
	}

	return LZ_OK;
}
