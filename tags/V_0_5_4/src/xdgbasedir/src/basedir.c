/* Copyright (c) 2007 Mark Nevill
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/** @file basedir.c
  * @brief Implementation of the XDG Base Directory specification. */

#if defined(HAVE_CONFIG_H) || defined(_DOXYGEN)
#include <config.h>
#endif

#if STDC_HEADERS || HAVE_STDLIB_H || !defined(HAVE_CONFIG_H)
#  include <stdlib.h>
#endif
#if HAVE_MEMORY_H || !defined(HAVE_CONFIG_H)
#  include <memory.h>
#endif
#if HAVE_STRING_H || !defined(HAVE_CONFIG_H)
#  include <string.h>
#endif
#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#include <errno.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif
#define FALSE 0
#define TRUE 1

#if HAVE_MEMSET || !defined(HAVE_CONFIG_H)
#  define xdgZeroMemory(p, n) memset(p, 0, n)
#elif HAVE_BZERO
#  define xdgZeroMemory(p, n) bzero(p, n)
#else
static void xdgZeroMemory(void* p, size_t n)
{
	while (n > 0) { ((char*)p)[n] = 0; ++n; }
}
#endif

#if defined _WIN32 && !defined __CYGWIN__
   /* Use Windows separators on all _WIN32 defining
      environments, except Cygwin. */
#  define DIR_SEPARATOR_CHAR		'\\'
#  define DIR_SEPARATOR_STR		"\\"
#  define PATH_SEPARATOR_CHAR		';'
#  define PATH_SEPARATOR_STR		";"
#  define NO_ESCAPES_IN_PATHS
#else
#  define DIR_SEPARATOR_CHAR		'/'
#  define DIR_SEPARATOR_STR		"/"
#  define PATH_SEPARATOR_CHAR		':'
#  define PATH_SEPARATOR_STR		":"
#  define NO_ESCAPES_IN_PATHS
#endif

#include <basedir.h>
#include <basedir_fs.h>

#ifndef MAX
#define MAX(a, b) ((b) > (a) ? (b) : (a))
#endif

static const char
	DefaultRelativeDataHome[] = DIR_SEPARATOR_STR ".local" DIR_SEPARATOR_STR "share",
	DefaultRelativeConfigHome[] = DIR_SEPARATOR_STR ".config",
	DefaultDataDirectories1[] = DIR_SEPARATOR_STR "usr" DIR_SEPARATOR_STR "local" DIR_SEPARATOR_STR "share",
	DefaultDataDirectories2[] = DIR_SEPARATOR_STR "usr" DIR_SEPARATOR_STR "share",
	DefaultConfigDirectories[] = DIR_SEPARATOR_STR "etc" DIR_SEPARATOR_STR "xdg",
	DefaultRelativeCacheHome[] = DIR_SEPARATOR_STR ".cache";

static const char
	*DefaultDataDirectoriesList[] = { DefaultDataDirectories1, DefaultDataDirectories2, NULL },
	*DefaultConfigDirectoriesList[] = { DefaultConfigDirectories, NULL };

typedef struct _xdgCachedData
{
	char * dataHome;
	char * configHome;
	char * cacheHome;
	/* Note: string lists are null-terminated and all items */
	/* except the first are assumed to be allocated using malloc. */
	/* The first item is assumed to be allocated by malloc only if */
	/* it is not equal to the appropriate home directory string above. */
	char ** searchableDataDirectories;
	char ** searchableConfigDirectories; 
} xdgCachedData;

/** Get cache object associated with a handle */
static xdgCachedData* xdgGetCache(xdgHandle *handle)
{
	return ((xdgCachedData*)(handle->reserved));
}

xdgHandle * xdgInitHandle(xdgHandle *handle)
{
	if (!handle) return 0;
	handle->reserved = 0; /* So xdgUpdateData() doesn't free it */
	if (xdgUpdateData(handle))
		return handle;
	return 0;
}

/** Free all memory used by a NULL-terminated string list */
static void xdgFreeStringList(char** list)
{
	char** ptr = list;
	if (!list) return;
	for (; *ptr; ptr++)
		free(*ptr);
	free(list);
}

/** Free all data in the cache and set pointers to null. */
static void xdgFreeData(xdgCachedData *cache)
{
	if (cache->dataHome);
	{
		/* the first element of the directory lists is usually the home directory */
		if (cache->searchableDataDirectories[0] != cache->dataHome)
			free(cache->dataHome);
		cache->dataHome = 0;
	}
	if (cache->configHome);
	{
		if (cache->searchableConfigDirectories[0] != cache->configHome)
			free(cache->configHome);
		cache->configHome = 0;
	}
	if (cache->cacheHome)
	{
		free(cache->cacheHome);
		cache->cacheHome = 0;
	}
	xdgFreeStringList(cache->searchableDataDirectories);
	cache->searchableDataDirectories = 0;
	xdgFreeStringList(cache->searchableConfigDirectories);
	cache->searchableConfigDirectories = 0;
}

void xdgWipeHandle(xdgHandle *handle)
{
	xdgCachedData* cache = xdgGetCache(handle);
	xdgFreeData(cache);
	free(cache);
}

/** Get value for environment variable $name, defaulting to "defaultValue".
 *	@param name Name of environment variable.
 *	@param defaultValue Value to assume for environment variable if it is
 *		unset or empty.
 */
static char* xdgGetEnv(const char* name, const char* defaultValue)
{
	const char* env;
	char* value;

	env = getenv(name);
	if (env && env[0])
	{
		if (!(value = (char*)malloc(strlen(env)+1))) return 0;
		strcpy(value, env);
	}
	else
	{
		if (!(value = (char*)malloc(strlen(defaultValue)+1))) return 0;
		strcpy(value, defaultValue);
	}
	return value;
}

/** Split string at ':', return null-terminated list of resulting strings.
 * @param string String to be split
 */
static char** xdgSplitPath(const char* string)
{
	unsigned int size, i, j, k;
	char** itemlist;

	/* Get the number of paths */
	size=2; /* One item more than seperators + terminating null item */
	for (i = 0; string[i]; ++i)
	{
#ifndef NO_ESCAPES_IN_PATHS
		if (string[i] == '\\' && string[i+1])
		{
			/* skip escaped characters including seperators */
			++i;
			continue;
		}
#endif
		if (string[i] == PATH_SEPARATOR_CHAR) ++size;
	}
	
	if (!(itemlist = (char**)malloc(sizeof(char*)*size))) return 0;
	xdgZeroMemory(itemlist, sizeof(char*)*size);

	for (i = 0; *string; ++i)
	{
		/* get length of current string  */
		for (j = 0; string[j] && string[j] != PATH_SEPARATOR_CHAR; ++j)
#ifndef NO_ESCAPES_IN_PATHS
			if (string[j] == '\\' && string[j+1]) ++j
#endif
			;
	
		if (!(itemlist[i] = (char*)malloc(j+1))) { xdgFreeStringList(itemlist); return 0; }

		/* transfer string, unescaping any escaped seperators */
		for (k = j = 0; string[j] && string[j] != PATH_SEPARATOR_CHAR; ++j, ++k)
		{
#ifndef NO_ESCAPES_IN_PATHS
			if (string[j] == '\\' && string[j+1] == PATH_SEPARATOR_CHAR) ++j; /* replace escaped ':' with just ':' */
			else if (string[j] == '\\' && string[j+1]) /* skip escaped characters so escaping remains aligned to pairs. */
			{
				itemlist[i][k]=string[j];
				++j, ++k;
			}
#endif
			itemlist[i][k] = string[j];
		}
		itemlist[i][k] = 0; /* Bugfix provided by Diego 'Flameeyes' Pettenò */
		/* move to next string */
		string += j;
		if (*string == PATH_SEPARATOR_CHAR) string++; /* skip seperator */
	}
	return itemlist;
}

/** Get $PATH-style environment variable as list of strings.
 * If $name is unset or empty, use default strings specified by variable arguments.
 * @param name Name of environment variable
 * @param strings NULL-terminated list of strings to be copied and used as defaults
 */
static char** xdgGetPathListEnv(const char* name, const char ** strings)
{
	const char* env;
	char* item;
	char** itemlist;
	int i, size;

	env = getenv(name);
	if (env && env[0])
	{
		if (!(item = (char*)malloc(strlen(env)+1))) return NULL;
		strcpy(item, env);

		itemlist = xdgSplitPath(item);
		free(item);
	}
	else
	{
		if (!strings) return NULL;
		for (size = 0; strings[size]; ++size) ; ++size;
		if (!(itemlist = (char**)malloc(sizeof(char*)*size))) return NULL;
		xdgZeroMemory(itemlist, sizeof(char*)*(size));

		/* Copy defaults into itemlist. */
		/* Why all this funky stuff? So the result can be handled uniformly by xdgFreeStringList. */
		for (i = 0; strings[i]; ++i)
		{
			if (!(item = (char*)malloc(strlen(strings[i])+1))) { xdgFreeStringList(itemlist); return NULL; }
			strcpy(item, strings[i]);
			itemlist[i] = item;
		}
	}
	return itemlist;
}

/** Update all *Home variables of cache.
 * This includes xdgCachedData::dataHome, xdgCachedData::configHome and xdgCachedData::cacheHome.
 * @param cache Data cache to be updated
 */
static int xdgUpdateHomeDirectories(xdgCachedData* cache)
{
	const char* env;
	char* home, *defVal;

	env = getenv("HOME");
	if (!env || !env[0])
		return FALSE;
	if (!(home = (char*)malloc(strlen(env)+1))) return FALSE;
	strcpy(home, env);

	/* Allocate maximum needed for any of the 3 default values */
	defVal = (char*)malloc(strlen(home)+
		MAX(MAX(sizeof(DefaultRelativeDataHome), sizeof(DefaultRelativeConfigHome)), sizeof(DefaultRelativeCacheHome)));
	if (!defVal) return FALSE;

	strcpy(defVal, home);
	strcat(defVal, DefaultRelativeDataHome);
	if (!(cache->dataHome = xdgGetEnv("XDG_DATA_HOME", defVal))) return FALSE;

	defVal[strlen(home)] = 0;
	strcat(defVal, DefaultRelativeConfigHome);
	if (!(cache->configHome = xdgGetEnv("XDG_CONFIG_HOME", defVal))) return FALSE;

	defVal[strlen(home)] = 0;
	strcat(defVal, DefaultRelativeCacheHome);
	if (!(cache->cacheHome = xdgGetEnv("XDG_CACHE_HOME", defVal))) return FALSE;

	free(defVal);
	free(home);

	return TRUE;
}

/** Update all *Directories variables of cache.
 * This includes xdgCachedData::searchableDataDirectories and xdgCachedData::searchableConfigDirectories.
 * @param cache Data cache to be updated.
 */
static int xdgUpdateDirectoryLists(xdgCachedData* cache)
{
	char** itemlist;
	int size;

	itemlist = xdgGetPathListEnv("XDG_DATA_DIRS", DefaultDataDirectoriesList);

	if (!itemlist) return FALSE;
	for (size = 0; itemlist[size]; size++) ; /* Get list size */
	if (!(cache->searchableDataDirectories = (char**)malloc(sizeof(char*)*(size+2))))
	{
		xdgFreeStringList(itemlist);
		return FALSE;
	}
	/* "home" directory has highest priority according to spec */
	cache->searchableDataDirectories[0] = cache->dataHome;
	memcpy(&(cache->searchableDataDirectories[1]), itemlist, sizeof(char*)*(size+1));
	free(itemlist);
	
	itemlist = xdgGetPathListEnv("XDG_CONFIG_DIRS", DefaultConfigDirectoriesList);
	if (!itemlist) return FALSE;
	for (size = 0; itemlist[size]; size++) ; /* Get list size */
	if (!(cache->searchableConfigDirectories = (char**)malloc(sizeof(char*)*(size+2))))
	{
		xdgFreeStringList(itemlist);
		return FALSE;
	}
	cache->searchableConfigDirectories[0] = cache->configHome;
	memcpy(&(cache->searchableConfigDirectories[1]), itemlist, sizeof(char*)*(size+1));
	free(itemlist);

	return TRUE;
}

int xdgUpdateData(xdgHandle *handle)
{
	xdgCachedData* cache = (xdgCachedData*)malloc(sizeof(xdgCachedData));
	xdgCachedData* oldCache;
	if (!cache) return FALSE;
	xdgZeroMemory(cache, sizeof(xdgCachedData));

	if (xdgUpdateHomeDirectories(cache) &&
		xdgUpdateDirectoryLists(cache))
	{
		/* Update successful, replace pointer to old cache with pointer to new cache */
		oldCache = xdgGetCache(handle);
		handle->reserved = cache;
		if (oldCache)
		{
			xdgFreeData(oldCache);
			free(oldCache);
		}
		return TRUE;
	}
	else
	{
		/* Update failed, discard new cache and leave old cache unmodified */
		xdgFreeData(cache);
		free(cache);
		return FALSE;
	}
}

/** Find all existing files corresponding to relativePath relative to each item in dirList.
  * @param relativePath Relative path to search for.
  * @param dirList <tt>NULL</tt>-terminated list of directory paths.
  * @return A sequence of null-terminated strings terminated by a
  * 	double-<tt>NULL</tt> (empty string) and allocated using malloc().
  */
static char * xdgFindExisting(const char * relativePath, const char * const * dirList)
{
	char * fullPath;
	char * returnString = 0;
	char * tmpString;
	int strLen = 0;
	FILE * testFile;
	const char * const * item;

	for (item = dirList; *item; item++)
	{
		if (!(fullPath = (char*)malloc(strlen(*item)+strlen(relativePath)+2)))
		{
			if (returnString) free(returnString);
			return 0;
		}
		strcpy(fullPath, *item);
		if (fullPath[strlen(fullPath)-1] != DIR_SEPARATOR_CHAR)
			strcat(fullPath, DIR_SEPARATOR_STR);
		strcat(fullPath, relativePath);
		testFile = fopen(fullPath, "r");
		if (testFile)
		{
			if (!(tmpString = (char*)realloc(returnString, strLen+strlen(fullPath)+2)))
			{
				free(returnString);
				free(fullPath);
				return 0;
			}
			returnString = tmpString;
			strcpy(&returnString[strLen], fullPath);
			strLen = strLen+strlen(fullPath)+1;
			fclose(testFile);
		}
		free(fullPath);
	}
	if (returnString)
		returnString[strLen] = 0;
	else
	{
		if ((returnString = (char*)malloc(2)))
			strcpy(returnString, "\0");
	}
	return returnString;
}

/** Open first possible config file corresponding to relativePath.
  * @param relativePath Path to scan for.
  * @param mode Mode with which to attempt to open files (see fopen modes).
  * @param dirList <tt>NULL</tt>-terminated list of paths in which to search for relativePath.
  * @return File pointer if successful else @c NULL. Client must use @c fclose to close file.
  */
static FILE * xdgFileOpen(const char * relativePath, const char * mode, const char * const * dirList)
{
	char * fullPath;
	FILE * testFile;
	const char * const * item;

	for (item = dirList; *item; item++)
	{
		if (!(fullPath = (char*)malloc(strlen(*item)+strlen(relativePath)+2)))
			return 0;
		strcpy(fullPath, *item);
		if (fullPath[strlen(fullPath)-1] != DIR_SEPARATOR_CHAR)
			strcat(fullPath, DIR_SEPARATOR_STR);
		strcat(fullPath, relativePath);
		testFile = fopen(fullPath, mode);
		free(fullPath);
		if (testFile)
			return testFile;
	}
	return 0;
}

int xdgMakePath(const char * path, mode_t mode)
{
	int length = strlen(path);
	char * tmpPath;
	char * tmpPtr;
	int ret;

	if (length == 0 || (length == 1 && path[0] == DIR_SEPARATOR_CHAR))
		return 0;

	if (!(tmpPath = (char*)malloc(length+1)))
	{
		errno = ENOMEM;
		return -1;
	}
	strcpy(tmpPath, path);
	if (tmpPath[length-1] == DIR_SEPARATOR_CHAR)
		tmpPath[length-1] = '\0';

	/* skip tmpPath[0] since if it's a seperator we have an absolute path */
	for (tmpPtr = tmpPath+1; *tmpPtr; ++tmpPtr)
	{
		if (*tmpPtr == DIR_SEPARATOR_CHAR)
		{
			*tmpPtr = '\0';
			if (mkdir(tmpPath, mode) == -1)
			{
				if (errno != EEXIST)
				{
					free(tmpPath);
					return -1;
				}
			}
			*tmpPtr = DIR_SEPARATOR_CHAR;
		}
	}
	ret = mkdir(tmpPath, mode);
	free(tmpPath);
	return ret;
}

const char * xdgDataHome(xdgHandle *handle)
{
	return xdgGetCache(handle)->dataHome;
}
const char * xdgConfigHome(xdgHandle *handle)
{
	return xdgGetCache(handle)->configHome;
}
const char * const * xdgDataDirectories(xdgHandle *handle)
{
	return (const char * const *)&(xdgGetCache(handle)->searchableDataDirectories[1]);
}
const char * const * xdgSearchableDataDirectories(xdgHandle *handle)
{
	return (const char * const *)xdgGetCache(handle)->searchableDataDirectories;
}
const char * const * xdgConfigDirectories(xdgHandle *handle)
{
	return (const char * const *)&(xdgGetCache(handle)->searchableConfigDirectories[1]);
}
const char * const * xdgSearchableConfigDirectories(xdgHandle *handle)
{
	return (const char * const *)xdgGetCache(handle)->searchableConfigDirectories;
}
const char * xdgCacheHome(xdgHandle *handle)
{
	return xdgGetCache(handle)->cacheHome;
}
char * xdgDataFind(const char * relativePath, xdgHandle *handle)
{
	return xdgFindExisting(relativePath, xdgSearchableDataDirectories(handle));
}
char * xdgConfigFind(const char * relativePath, xdgHandle *handle)
{
	return xdgFindExisting(relativePath, xdgSearchableConfigDirectories(handle));
}
FILE * xdgDataOpen(const char * relativePath, const char * mode, xdgHandle *handle)
{
	return xdgFileOpen(relativePath, mode, xdgSearchableDataDirectories(handle));
}
FILE * xdgConfigOpen(const char * relativePath, const char * mode, xdgHandle *handle)
{
	return xdgFileOpen(relativePath, mode, xdgSearchableConfigDirectories(handle));
}

