#include "evas_common.h"
#include "evas_private.h"

#include <Edb.h>
#include <zlib.h>


#define SWAP32(x) (x) = ((((x) & 0x000000ff ) << 24) | (((x) & 0x0000ff00 ) << 8) | (((x) & 0x00ff0000 ) >> 8) | (((x) & 0xff000000 ) >> 24))


int evas_image_load_file_head_edb(Image_Entry *ie, const char *file, const char *key);
int evas_image_load_file_data_edb(Image_Entry *ie, const char *file, const char *key);

Evas_Image_Load_Func evas_image_load_edb_func =
{
  evas_image_load_file_head_edb,
  evas_image_load_file_data_edb
};


int
evas_image_load_file_head_edb(Image_Entry *ie, const char *file, const char *key)
{
   int                  w, h, alpha, compression, size;
   E_DB_File           *db;
   DATA32              *ret;
   DATA32               header[8];

   if ((!file) || (!key)) return 0;
   db = e_db_open_read((char *)file);
   if (!db) return 0;
   ret = e_db_data_get(db, (char *)key, &size);
   if (!ret)
     {
	e_db_close(db);
	return 0;
     }
   if (size < 32)
     {
	free(ret);
	e_db_close(db);
	return 0;
     }
   memcpy(header, ret, 32);
#ifdef WORDS_BIGENDIAN
     {
	int i;

	for (i = 0; i < 8; i++) SWAP32(header[i]);
     }
#endif
   if (header[0] != 0xac1dfeed)
     {
	free(ret);
	e_db_close(db);
	return 0;
     }
   w = header[1];
   h = header[2];
   if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
     {
	free(ret);
	e_db_close(db);
	return 0;
     }
   alpha = header[3];
   compression = header[4];
   
   if ((compression == 0) && (size < ((w * h * 4) + 32)))
     {
	free(ret);
	e_db_close(db);
	return 0;
     }
   if (alpha) ie->flags.alpha = 1;
   ie->w = w;
   ie->h = h;
   free(ret);
   e_db_close(db);
   return 1;
}

int
evas_image_load_file_data_edb(Image_Entry *ie, const char *file, const char *key)
{
   int                  w, h, alpha, compression, size;
   E_DB_File           *db;
   DATA32              *ret;
   DATA32              *body;
   DATA32              *surface;
   DATA32               header[8];

   if ((!file) || (!key)) return 0;
   db = e_db_open_read((char *)file);
   if (!db) return 0;
   ret = e_db_data_get(db, (char *)key, &size);
   if (!ret)
     {
	e_db_close(db);
	return 0;
     }
   if (size < 32)
     {
	free(ret);
	e_db_close(db);
	return 0;
     }
   memcpy(header, ret, 32);
#ifdef WORDS_BIGENDIAN
     {
	int i;

	for (i = 0; i < 8; i++) SWAP32(header[i]);
     }
#endif
   if (header[0] != 0xac1dfeed)
     {
	free(ret);
	e_db_close(db);
	return 0;
     }
   w = header[1];
   h = header[2];
   if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
     {
	free(ret);
	e_db_close(db);
	return 0;
     }
   
   alpha = header[3];
   compression = header[4];
   
   if ((compression == 0) && (size < ((w * h * 4) + 32)))
     {
	free(ret);
	e_db_close(db);
	return 0;
     }
   if (alpha) ie->flags.alpha = 1;
   body = &(ret[8]);
   evas_cache_image_surface_alloc(ie, w, h);
   surface = evas_cache_image_pixels(ie);
   if (!surface)
     {
	free(ret);
	e_db_close(db);
	return 0;
     }
   if (!compression)
     {
#ifdef WORDS_BIGENDIAN
	  {
	     int x;

	     memcpy(surface, body, w * h * sizeof(DATA32));
	     for (x = 0; x < (w * h); x++) SWAP32(surface[x]);
	  }
#else
	memcpy(surface, body, w * h * sizeof(DATA32));
#endif
     }
   else
     {
	uLongf dlen;

	dlen = w * h * sizeof(DATA32);
	uncompress((Bytef *)surface, &dlen, (Bytef *)body,
		   (uLongf)(size - 32));
#ifdef WORDS_BIGENDIAN
	  {
	     int x;

	     for (x = 0; x < (w * h); x++) SWAP32(surface[x]);
	  }
#endif
     }
   evas_common_image_premul(ie);
   free(ret);
   e_db_close(db);
   return 1;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_edb_func);
   return 1;
}

EAPI void
module_close(void)
{
   
}

EAPI Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_IMAGE_LOADER,
     "edb",
     "none"
};
