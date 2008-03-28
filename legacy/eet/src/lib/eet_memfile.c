/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "Eet.h"
#include "Eet_private.h"

FILE *
_eet_memfile_read_open(const void *data, size_t size)
{
#ifdef HAVE_FMEMOPEN
   return fmemopen((void*)data, size, "rb");
#else
   FILE *f;

   f = tmpfile();
   if (!f)
     {
	printf("EET: Error opening tmp file (no fmemopen support fallback):\n");
	perror("tmpfile()");
	return NULL;
     }
   fwrite(data, size, 1, f);
   rewind(f);
   return f;
#endif
}

void
_eet_memfile_read_close(FILE *f)
{
#ifdef HAVE_FMEMOPEN
   fclose(f);
#else
   fclose(f);
#endif
}


typedef struct _Eet_Memfile_Write_Info Eet_Memfile_Write_Info;
struct _Eet_Memfile_Write_Info
{
   FILE *f;
   void **data;
   size_t *size;
};

#ifndef HAVE_OPEN_MEMSTREAM
static int                     _eet_memfile_info_alloc_num = 0;
static int                     _eet_memfile_info_num       = 0;
static Eet_Memfile_Write_Info *_eet_memfile_info           = NULL;
#endif

void _eet_memfile_shutdown()
{
#ifdef HAVE_OPEN_MEMSTREAM
   return;
#else
   int i;

   for (i = 0; i < _eet_memfile_info_num; i++)
     free(_eet_memfile_info[i].data);

   free(_eet_memfile_info);
   _eet_memfile_info = NULL;
#endif
}

FILE *
_eet_memfile_write_open(void **data, size_t *size)
{
#ifdef HAVE_OPEN_MEMSTREAM
   return open_memstream((char **)data, size);
#else
   FILE *f;

   _eet_memfile_info_num++;
   if (_eet_memfile_info_num > _eet_memfile_info_alloc_num)
     {
	Eet_Memfile_Write_Info *tmp;

	_eet_memfile_info_alloc_num += 16;
	tmp = realloc(_eet_memfile_info,
		      _eet_memfile_info_alloc_num *
		      sizeof(Eet_Memfile_Write_Info));
	if (!tmp)
	  {
	     _eet_memfile_info_alloc_num -= 16;
	     _eet_memfile_info_num--;
	     return NULL;
	  }
	_eet_memfile_info = tmp;
     }
   f = tmpfile();
   if (!f)
     {
	_eet_memfile_info_num--;
	return NULL;
     }
   _eet_memfile_info[_eet_memfile_info_num - 1].f = f;
   _eet_memfile_info[_eet_memfile_info_num - 1].data = data;
   _eet_memfile_info[_eet_memfile_info_num - 1].size = size;
   return f;
#endif
}

void
_eet_memfile_write_close(FILE *f)
{
#ifdef HAVE_OPEN_MEMSTREAM
   fclose(f);
#else
   int i;

   for (i = 0; i < _eet_memfile_info_num; i++)
     {
	if (_eet_memfile_info[i].f == f)
	  {
	     int j;

	     fseek(f, 0, SEEK_END);
	     (*(_eet_memfile_info[i].size)) = ftell(f);
	     rewind(f);
	     (*(_eet_memfile_info[i].data)) = malloc(*(_eet_memfile_info[i].size));
	     if (!(*(_eet_memfile_info[i].data)))
	       {
		  fclose(f);
		  (*(_eet_memfile_info[i].size)) = 0;
		  return;
	       }
	     fread((*(_eet_memfile_info[i].data)), (*(_eet_memfile_info[i].size)), 1, f);
	     for (j = i + 1; j < _eet_memfile_info_num; j++)
	       _eet_memfile_info[j - 1] = _eet_memfile_info[j];
	     _eet_memfile_info_num--;
	     fclose(f);
	     return;
	  }
     }
   fclose(f);
#endif
}
