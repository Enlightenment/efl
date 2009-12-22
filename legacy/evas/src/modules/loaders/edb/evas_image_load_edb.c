s#include "evas_common.h"
#include "evas_private.h"

#include <Edb.h>
#include <zlib.h>


#define SWAP32(x) (x) = ((((x) & 0x000000ff ) << 24) | (((x) & 0x0000ff00 ) << 8) | (((x) & 0x00ff0000 ) >> 8) | (((x) & 0xff000000 ) >> 24))


static Eina_Bool evas_image_load_file_head_edb(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);
static Eina_Bool evas_image_load_file_data_edb(Image_Entry *ie, const char *file, const char *key, int *error) EINA_ARG_NONNULL(1, 2, 4);

static Evas_Image_Load_Func evas_image_load_edb_func =
{
  evas_image_load_file_head_edb,
  evas_image_load_file_data_edb
};

static Eina_Bool
evas_image_load_file_head_edb(Image_Entry *ie, const char *file, const char *key, int *error)
{
   int                  w, h, alpha, compression, size;
   E_DB_File           *db;
   DATA32              *ret;
   DATA32               header[8];

   if (!key)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   db = e_db_open_read((char *)file);
   if (!db)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   ret = e_db_data_get(db, (char *)key, &size);
   if (!ret)
     {
	e_db_close(db);
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   if (size < 32)
     {
	free(ret);
	e_db_close(db);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
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
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }
   w = header[1];
   h = header[2];
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) || 
       IMG_TOO_BIG(w, h))
     {
	free(ret);
	e_db_close(db);
	if (IMG_TOO_BIG(w, h))
	  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = EVAS_LOAD_ERROR_GENERIC;
	return EINA_FALSE;
     }
   alpha = header[3];
   compression = header[4];
   
   if ((compression == 0) && (size < ((w * h * 4) + 32)))
     {
	free(ret);
	e_db_close(db);
	*error = EVAS_LOAD_ERROR_GENERIC;
	return EINA_FALSE;
     }
   if (alpha) ie->flags.alpha = 1;
   ie->w = w;
   ie->h = h;
   free(ret);
   e_db_close(db);
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static Eina_Bool
evas_image_load_file_data_edb(Image_Entry *ie, const char *file, const char *key, int *error)
{
   int                  w, h, alpha, compression, size;
   E_DB_File           *db;
   DATA32              *ret;
   DATA32              *body;
   DATA32              *surface;
   DATA32               header[8];

   if (!key)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   db = e_db_open_read((char *)file);
   if (!db)
     {
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   ret = e_db_data_get(db, (char *)key, &size);
   if (!ret)
     {
	e_db_close(db);
	*error = EVAS_LOAD_ERROR_DOES_NOT_EXIST;
	return EINA_FALSE;
     }
   if (size < 32)
     {
	free(ret);
	e_db_close(db);
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
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
	*error = EVAS_LOAD_ERROR_CORRUPT_FILE;
	return EINA_FALSE;
     }
   w = header[1];
   h = header[2];
   if ((w < 1) || (h < 1) || (w > IMG_MAX_SIZE) || (h > IMG_MAX_SIZE) ||
       IMG_TOO_BIG(w, h))
     {
	free(ret);
	e_db_close(db);
	if (IMG_TOO_BIG(w, h))
	  *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	else
	  *error = EVAS_LOAD_ERROR_GENERIC;
	return EINA_FALSE;
     }
   
   alpha = header[3];
   compression = header[4];
   
   if ((compression == 0) && (size < ((w * h * 4) + 32)))
     {
	free(ret);
	e_db_close(db);
	*error = EVAS_LOAD_ERROR_GENERIC;
	return EINA_FALSE;
     }
   if (alpha) ie->flags.alpha = 1;
   body = &(ret[8]);
   evas_cache_image_surface_alloc(ie, w, h);
   surface = evas_cache_image_pixels(ie);
   if (!surface)
     {
	free(ret);
	e_db_close(db);
	*error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
	return EINA_FALSE;
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
   *error = EVAS_LOAD_ERROR_NONE;
   return EINA_TRUE;
}

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_edb_func);
   return 1;
}

static void
module_close(Evas_Module *em)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "edb",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, edb);

#ifndef EVAS_STATIC_BUILD_EDB
EVAS_EINA_MODULE_DEFINE(image_loader, edb);
#endif
