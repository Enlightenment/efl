/* os dependant file code. for unix-y like fs's only for now */
/* if your os doesn't use unix-like fs starting with "/" for the root and */
/* the file path separator isn't "/" then you may need to help out by */
/* adding in a new set of functions here */

#ifndef _WIN32_WCE
/* UNIX compatability functions */

#include "evas_common.h"
#include "evas_private.h"

#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
/* get the casefold feature! */
#include <fnmatch.h>
#include <dirent.h>

int
evas_file_path_is_full_path(const char *path)
{
   if (!path) return 0;
   if (path[0] == '/') return 1;
   return 0;
}

char *
evas_file_path_join(const char *path, const char *end)
{
   char *res = NULL;
   int len;

   if ((!path) && (!end)) return NULL;
   if (!path) return strdup(end);
   if (!end) return strdup(path);
   len = strlen(path);
   len += strlen(end);
   len += strlen("/");
   res = malloc(len + 1);
   if (!res) return NULL;
   strcpy(res, path);
   strcat(res, "/");
   strcat(res, end);
   return res;
}

int
evas_file_path_exists(const char *path)
{
   struct stat st;

   if (!stat(path, &st)) return 1;
   return 0;
}

int
evas_file_path_is_file(const char *path)
{
   struct stat st;

   if (stat(path, &st) == -1) return 0;
   if (S_ISREG(st.st_mode)) return 1;
   return 0;
}

int
evas_file_path_is_dir(const char *path)
{
   struct stat st;

   if (stat(path, &st) == -1) return 0;
   if (S_ISDIR(st.st_mode)) return 1;
   return 0;
}

Eina_List *
evas_file_path_list(char *path, const char *match, int match_case)
{
   Eina_List *files = NULL;
   DIR *dir;

   dir = opendir(path);
   if (!dir) return NULL;
     {
	struct dirent      *dp;
	int flags;

	flags = FNM_PATHNAME;
#ifdef FNM_CASEFOLD
	if (!match_case)
	  flags |= FNM_CASEFOLD;
#else
/*#warning "Your libc does not provide case-insensitive matching!"*/
#endif
	while ((dp = readdir(dir)))
	  {
	     if ((!strcmp(dp->d_name, ".")) || (!strcmp(dp->d_name, "..")))
	       continue;
	     if (match)
	       {
		  if (fnmatch(match, dp->d_name, flags) == 0)
		    files = eina_list_append(files, strdup(dp->d_name));
	       }
	     else
	       files = eina_list_append(files, strdup(dp->d_name));
	  }
	closedir(dir);
     }
   return files;
}

DATA64
evas_file_modified_time(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   if (st.st_ctime > st.st_mtime) return (DATA64)st.st_ctime;
   else return (DATA64)st.st_mtime;
   return 0;
}

char *
evas_file_path_resolve(const char *file)
{
#if 0
   char buf[PATH_MAX], *buf2;
#endif

   return strdup(file);
#if 0
   if (!realpath(file, buf)) return NULL;
   buf2 = strdup(buf);
   return buf2;
#endif
}

#else
/* WIN32 WINCE compatability functions */

/* Forward declarations */

static DWORD     winstat(char *path);
static wchar_t  *convert_utf_unicode(const char* putf);
static char     *convert_unicode_utf(const wchar_t* punicode);


/* Unicode to utf and utf to unicode conversion functions */
static wchar_t *
convert_utf_unicode(const char *putf)
{
   int len;
   wchar_t *punicode;

   len = strlen(putf)+ 1; //add one for safety

   punicode = (wchar_t *)malloc(len * sizeof(wchar_t));

   if (punicode == NULL) return NULL;

#ifdef UNICODE
   strcpy(punicode,putf);
#else
   if (mbstowcs(punicode,putf,len) < 0)
     {
	free(punicode);
	return NULL;
     }
#endif
   return punicode;
}

static char *
convert_unicode_utf(const wchar_t *punicode)
{
   int len;
   char *putf;

   /* add one for safety */
   len = wcslen(punicode) + 1;

   /* this will alloc too many bytes */
   putf = (char *)malloc(len * sizeof(wchar_t));

   if (putf == NULL) return NULL;

#ifdef UNICODE
   strcpy(putf,punicode);
#else
   if (wcstombs(putf,punicode,len) < 0)
     {
	free(putf);
	return NULL;
     }
#endif
   return putf;
}

/*
 * win"stat"
 * This is like the stat function except that it reurns a bitmask (DWORD)
 * Since this library is complied using MBCS then the path is multibyte
 */
static DWORD
winstat(char *path)
{
   DWORD fa;
   wchar_t *pwpath;	/* A wide character type */

   pwpath = convert_utf_unicode(path);
   /* 0xFFFFFFFF is the error return val for the GetFile Attributes Function */
   /* so I am usin this as an error return up here */
   if (pwpath == NULL) return 0xFFFFFFFF;
   /* this function needed the wide string"*/
   /* I dont think that WinCe has mbcs equiv functions and only provides UNICODE*/
   fa = GetFileAttributesW(pwpath);
   free(pwpath);
   return fa;
}

int
evas_file_path_is_full_path(char *path)
{
   if (!path) return 0;
   if (path[0] == '\\') return 1;
   return 0;
}

char *
evas_file_path_join(char *path, char *end)
{
   char *res = NULL;
   int len;

   if ((!path) && (!end)) return NULL;
   if (!path) return strdup(end);
   if (!end) return strdup(path);
   len = strlen(path);
   len += strlen(end);
   len += strlen("\\");
   res = malloc(len + 1);
   if (!res) return NULL;
   strcpy(res, path);
   strcat(res, "\\");
   strcat(res, end);
   return res;
}

int
evas_file_path_exists(char *path)
{
   DWORD fa;

   fa = winstat(path);
   if (fa == 0xFFFFFFFF) return 0;
   return 1;
}

int
evas_file_path_is_file(char *path)
{
   DWORD fa;

   fa = winstat(path);
   if (fa == 0xFFFFFFFF) return 0;
   if (fa & FILE_ATTRIBUTE_DIRECTORY) return 0;
   return 1;
}

int
evas_file_path_is_dir(char *path)
{
   DWORD fa;

   fa = winstat(path);
   if (fa == 0xFFFFFFFF) return 0;
   if (fa & FILE_ATTRIBUTE_DIRECTORY) return 1;
   return 0;
}

Eina_List *
evas_file_path_list(char *path, char *match, int match_case)
{
   Eina_List *files = NULL;
   WIN32_FIND_DATAW find;
   HANDLE fh;
   int fullpathlen;
   char *pfp;		/* full path pointer */
   wchar_t *pfup;	/* full unicode path pointer */

   /*
    * work out the full path length of the combined patch and match
    * if we are looking for a specific match eg *.txt then we will will add
    * the length of *.txt and \\ to the string
    * if we are not looking for a match then we want to list the whole
    * directory and we find the length of \\*.*
    */
   fullpathlen = strlen(path);
   if (match)
     {
	fullpathlen += strlen("\\");
	fullpathlen += strlen(match);
     }
   else
     fullpathlen += strlen("\\*.*");

   /* Create the full search path */

   pfp = (char *)malloc(fullpathlen + 1); /* add one for safety*/
   if (pfp == NULL) return NULL;

   /* construct the full path */
   strcpy(pfp, path);
   if (match)
     {
	strcat(pfp,"\\");
	strcat(pfp,match);
     }
   else
     strcat(pfp,"\\*.*");

   /* pfp now contains the fully constructed path*/

   pfup = convert_utf_unicode(pfp);
   free(pfp); /* chuck it away now as we don't need it, we have a unicode version */
   if (pfup == NULL) return NULL;

   fh = FindFirstFileW(pfup,&find);
   free(pfup); /* chuck it away now as we don't need it, we have a handle */
   if (fh == INVALID_HANDLE_VALUE) return NULL;

   /* OK now go through the directory picking up filenames */
   do
     {
	if (!(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	  files = eina_list_append(files,convert_unicode_utf(find.cFileName));

     }
   while (FindNextFileW(fh,&find));
   FindClose(fh);

   return files;
}

DATA64
evas_file_modified_time(const char *file)
{
   WIN32_FIND_DATAW find;
   HANDLE fh;
   ULARGE_INTEGER modtime;
   wchar_t *pufile;

   pufile = convert_utf_unicode(file);
   if (pufile == NULL) return 0;

   fh = FindFirstFileW(pufile,&find);
   if (fh == INVALID_HANDLE_VALUE)
     {
	free(pufile);
	return 0;
     }
   FindClose(fh);
   free(pufile);

   modtime.u.HighPart = find.ftCreationTime.dwHighDateTime;
   modtime.u.LowPart = find.ftCreationTime.dwLowDateTime;

   return modtime.QuadPart;
}

char *
evas_file_path_resolve(const char *file)
{
   return strdup(file);
}
#endif
