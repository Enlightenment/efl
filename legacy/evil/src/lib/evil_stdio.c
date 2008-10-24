
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"
#undef fopen


#if defined (_WIN32_WCE) && ! defined (__CEGCC__)

/*
 * Error related functions
 *
 */

void perror (const char *s)
{
   fprintf(stderr, "[Windows CE] error\n");
}

/*
 * Stream related functions
 *
 */

FILE *evil_fopen(const char *path, const char *mode)
{
   FILE *f;
   char *filename;

   if (*path != '\\')
     {
        char buf[PATH_MAX];
        int   l1;
        int   l2;

        if (!evil_getcwd(buf, PATH_MAX))
          return NULL;

        l1 = strlen(buf);
        l2 = strlen(path);
        filename = (char *)malloc(l1 + 1 + l2 + 1);
        memcpy(filename, buf, l1);
        filename[l1] = '\\';
        memcpy(filename + l1 + 1, path, l2);
        filename[l1 + 1 + l2] = '\0';
     }
   else
     filename = (char *)path;

   f = fopen(filename, mode);
   if (*path != '\\')
     free(filename);

   return f;
}

void rewind(FILE *stream)
{
  fseek(stream, 0, SEEK_SET);
}


#endif /* _WIN32_WCE && ! __CEGCC__ */
