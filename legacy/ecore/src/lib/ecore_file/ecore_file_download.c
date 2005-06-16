/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"

int
ecore_file_download(const char *url, const char *dst_dir)
{
   if (!ecore_file_is_dir(dst_dir)) return 0;

   /* FIXME: Add handlers for http and ftp! */
   if (!strncmp(url, "file://", 7))
     {
	/* Just copy it */
	char buf[PATH_MAX];

	url += 7;
	snprintf(buf, sizeof(buf), "%s/%s", dst_dir, ecore_file_get_file(url));
	return ecore_file_cp(url, buf);
     }
   else
     {
	return 0;
     }
}
