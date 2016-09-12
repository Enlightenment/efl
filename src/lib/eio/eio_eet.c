/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *           Stephen "okra" Houston <UnixTitan@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#include "eio_private.h"
#include "Eio.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static void
_eio_eet_open_job(void *data, Ecore_Thread *thread)
{
   Eio_Eet_Open *eet = data;

   eet->result = eet_open(eet->filename, eet->mode);
   if (!eet->result) eio_file_thread_error(&eet->common, thread);
}

static void
_eio_eet_open_free(Eio_Eet_Open *eet)
{
   if (eet->filename) eina_stringshare_del(eet->filename);
   eio_file_free((Eio_File *)eet);
}

static void
_eio_eet_open_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Open *eet = data;

   eet->eet_cb((void*) eet->common.data, &eet->common, eet->result);
   _eio_eet_open_free(eet);
}

static void
_eio_eet_open_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Open *eet = data;

   eio_file_error(&eet->common);
   _eio_eet_open_free(eet);
}

static void
_eio_eet_close_job(void *data, Ecore_Thread *thread)
{
   Eio_Eet_Simple *eet = data;

   eet->error = eet_close(eet->ef);
   if (eet->error != EET_ERROR_NONE) eio_file_thread_error(&eet->common, thread);
}

static void
_eio_eet_sync_job(void *data, Ecore_Thread *thread)
{
   Eio_Eet_Simple *eet = data;

   eet->error = eet_sync(eet->ef);
   if (eet->error != EET_ERROR_NONE) eio_file_thread_error(&eet->common, thread);
}

static void
_eio_eet_simple_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Simple *eet = data;

   eet->common.done_cb((void*) eet->common.data, &eet->common);
   eio_file_free((Eio_File *)eet);
}

static void
_eio_eet_simple_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Simple *eet = data;

   eet->error_cb((void*) eet->common.data, &eet->common, eet->error);
   eio_file_free((Eio_File *)eet);
}

static void
_eio_eet_data_write_cipher_job(void *data, Ecore_Thread *thread)
{
   Eio_Eet_Write *ew = data;

   ew->result = eet_data_write_cipher(ew->ef, ew->edd,
                                      ew->name, ew->cipher_key,
                                      ew->write_data,
                                      ew->compress);
   if (ew->result == 0) eio_file_thread_error(&ew->common, thread);
}

static void
_eio_eet_write_cipher_free(Eio_Eet_Write *ew)
{
   eina_stringshare_del(ew->name);
   eina_stringshare_del(ew->cipher_key);
   eio_file_free((Eio_File *)ew);
}

static void
_eio_eet_data_write_cipher_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Write *ew = data;

   ew->done_cb((void*) ew->common.data, &ew->common, ew->result);
   _eio_eet_write_cipher_free(ew);
}

static void
_eio_eet_data_write_cipher_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Write *ew = data;

   eio_file_error(&ew->common);
   _eio_eet_write_cipher_free(ew);
}

static void
_eio_eet_image_write_job(void *data, Ecore_Thread *thread)
{
   Eio_Eet_Image_Write *eiw = data;

   eiw->result = eet_data_image_write_cipher(eiw->ef, eiw->name, eiw->cipher_key,
                                             eiw->write_data,
                                             eiw->w,
                                             eiw->h,
                                             eiw->alpha,
                                             eiw->compress,
                                             eiw->quality,
                                             eiw->lossy);
   if (!eiw->result) eio_file_thread_error(&eiw->common, thread);
}

static void
_eio_eet_image_write_free(Eio_Eet_Image_Write *eiw)
{
   eina_stringshare_del(eiw->name);
   eina_stringshare_del(eiw->cipher_key);
   eio_file_free(&eiw->common);
}

static void
_eio_eet_image_write_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Image_Write *eiw = data;

   eiw->done_cb((void*) eiw->common.data, &eiw->common, eiw->result);
   _eio_eet_image_write_free(eiw);
}

static void
_eio_eet_image_write_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Image_Write *eiw = data;

   eio_file_error(&eiw->common);
   _eio_eet_image_write_free(eiw);
}

static void
_eio_eet_write_job(void *data, Ecore_Thread *thread)
{
   Eio_Eet_Write *ew = data;

   ew->result = eet_write_cipher(ew->ef,
                                 ew->name, ew->write_data,
                                 ew->size, ew->compress,
                                 ew->cipher_key);
   if (!ew->result) eio_file_thread_error(&ew->common, thread);
}

static void
_eio_eet_write_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Write *ew = data;

   ew->done_cb((void*) ew->common.data, &ew->common, ew->result);
   _eio_eet_write_cipher_free(ew);
}

static void
_eio_eet_write_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Write *ew = data;

   eio_file_error(&ew->common);
   _eio_eet_write_cipher_free(ew);
}

static void
_eio_eet_data_read_cipher_job(void *data, Ecore_Thread *thread)
{
   Eio_Eet_Read *er = data;

   er->result = eet_data_read_cipher(er->ef, er->edd,
                                     er->name, er->cipher_key);
   if (!er->result) eio_file_thread_error(&er->common, thread);
}

static void
_eio_eet_read_free(Eio_Eet_Read *er)
{
   eina_stringshare_del(er->name);
   eina_stringshare_del(er->cipher_key);
   eio_file_free(&er->common);
}

static void
_eio_eet_data_read_cipher_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Read *er = data;

   er->done_cb.eread((void*) er->common.data, &er->common, er->result);
   _eio_eet_read_free(er);
}

static void
_eio_eet_data_read_cipher_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Read *er = data;

   eio_file_error(&er->common);
   _eio_eet_read_free(er);
}

static void
_eio_eet_read_direct_job(void *data, Ecore_Thread *thread)
{
   Eio_Eet_Read *er = data;

   er->result = (void*) eet_read_direct(er->ef, er->name, &er->size);
   if (!er->result) eio_file_thread_error(&er->common, thread);
}

static void
_eio_eet_read_direct_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Read *er = data;

   er->done_cb.data((void*) er->common.data, &er->common,
                    er->result, er->size);
   _eio_eet_read_free(er);
}

static void
_eio_eet_read_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Read *er = data;

   eio_file_error(&er->common);
   _eio_eet_read_free(er);
}

static void
_eio_eet_read_cipher_job(void *data, Ecore_Thread *thread)
{
   Eio_Eet_Read *er = data;

   er->result = (void*) eet_read_cipher(er->ef, er->name,
                                        &er->size, er->cipher_key);
   if (!er->result) eio_file_thread_error(&er->common, thread);
}

static void
_eio_eet_read_cipher_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Eet_Read *er = data;

   er->done_cb.read((void*) er->common.data, &er->common,
                    er->result, er->size);
   _eio_eet_read_free(er);
}

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI Eio_File *
eio_eet_open(const char *filename,
             Eet_File_Mode mode,
	     Eio_Eet_Open_Cb eet_cb,
	     Eio_Error_Cb error_cb,
	     const void *data)
{
   Eio_Eet_Open *eet;

   EINA_SAFETY_ON_NULL_RETURN_VAL(filename, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(eet_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   eet = eio_common_alloc(sizeof(Eio_Eet_Open));
   EINA_SAFETY_ON_NULL_RETURN_VAL(eet, NULL);

   eet->eet_cb = eet_cb;
   eet->filename = eina_stringshare_add(filename);
   eet->mode = mode;
   eet->result = NULL;

   if (!eio_file_set(&eet->common,
                     NULL,
                     error_cb,
                     data,
                     _eio_eet_open_job,
                     _eio_eet_open_end,
                     _eio_eet_open_cancel))
     return NULL;
   return &eet->common;
}

EAPI Eio_File *
eio_eet_close(Eet_File *ef,
	      Eio_Done_Cb done_cb,
	      Eio_Eet_Error_Cb error_cb,
	      const void *data)
{
   Eio_Eet_Simple *eet;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ef, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   eet = eio_common_alloc(sizeof(Eio_Eet_Simple));
   EINA_SAFETY_ON_NULL_RETURN_VAL(eet, NULL);

   eet->ef = ef;
   eet->error_cb = error_cb;
   eet->error = EET_ERROR_NONE;

   if (!eio_file_set(&eet->common,
                     done_cb,
                     NULL,
                     data,
                     _eio_eet_close_job,
                     _eio_eet_simple_end,
                     _eio_eet_simple_cancel))
     return NULL;
   return &eet->common;
}

EAPI Eio_File *
eio_eet_flush(Eet_File *ef,
	      Eio_Done_Cb done_cb,
	      Eio_Eet_Error_Cb error_cb,
	      const void *data)
{
   Eio_Eet_Simple *eet;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ef, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   eet = eio_common_alloc(sizeof(Eio_Eet_Simple));
   EINA_SAFETY_ON_NULL_RETURN_VAL(eet, NULL);

   eet->ef = ef;
   eet->error_cb = error_cb;
   eet->error = EET_ERROR_NONE;

   if (!eio_file_set(&eet->common,
                     done_cb,
                     NULL,
                     data,
                     _eio_eet_sync_job,
                     _eio_eet_simple_end,
                     _eio_eet_simple_cancel))
     return NULL;
   return &eet->common;
}

EAPI Eio_File *
eio_eet_sync(Eet_File *ef,
             Eio_Done_Cb done_cb,
             Eio_Eet_Error_Cb error_cb,
             const void *data)
{
   return eio_eet_flush(ef, done_cb, error_cb, data);
}

EAPI Eio_File *
eio_eet_data_write_cipher(Eet_File *ef,
			  Eet_Data_Descriptor *edd,
			  const char *name,
			  const char *cipher_key,
			  void *write_data,
			  int compress,
			  Eio_Done_Int_Cb done_cb,
			  Eio_Error_Cb error_cb,
			  const void *user_data)
{
   Eio_Eet_Write *ew;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ef, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(edd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   ew = eio_common_alloc(sizeof(Eio_Eet_Write));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ew, NULL);

   ew->ef = ef;
   ew->edd = edd;
   ew->name = eina_stringshare_add(name);
   ew->cipher_key = eina_stringshare_add(cipher_key);
   ew->write_data = write_data;
   ew->compress = compress;
   ew->done_cb = done_cb;
   ew->result = 0;

   if (!eio_file_set(&ew->common,
                     NULL,
                     error_cb,
                     user_data,
                     _eio_eet_data_write_cipher_job,
                     _eio_eet_data_write_cipher_end,
                     _eio_eet_data_write_cipher_cancel))
     return NULL;
   return &ew->common;
}

EAPI Eio_File *
eio_eet_data_read_cipher(Eet_File *ef,
			 Eet_Data_Descriptor *edd,
			 const char *name,
			 const char *cipher_key,
			 Eio_Done_ERead_Cb done_cb,
			 Eio_Error_Cb error_cb,
			 const void *data)
{
   Eio_Eet_Read *er;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ef, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(edd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   er = eio_common_alloc(sizeof(Eio_Eet_Read));
   EINA_SAFETY_ON_NULL_RETURN_VAL(er, NULL);

   er->ef = ef;
   er->edd = edd;
   er->name = eina_stringshare_add(name);
   er->cipher_key = eina_stringshare_add(cipher_key);
   er->done_cb.eread = done_cb;

   if (!eio_file_set(&er->common,
                     NULL,
                     error_cb,
                     data,
                     _eio_eet_data_read_cipher_job,
                     _eio_eet_data_read_cipher_end,
                     _eio_eet_data_read_cipher_cancel))
     return NULL;

   return &er->common;
}

EAPI Eio_File *
eio_eet_data_image_write_cipher(Eet_File *ef,
				const char *name,
				const char *cipher_key,
				void *write_data,
				unsigned int w,
				unsigned int h,
				int alpha,
				int compress,
				int quality,
				int lossy,
				Eio_Done_Int_Cb done_cb,
				Eio_Error_Cb error_cb,
				const void *user_data)
{
   Eio_Eet_Image_Write *eiw;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ef, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   eiw = eio_common_alloc(sizeof(Eio_Eet_Image_Write));
   EINA_SAFETY_ON_NULL_RETURN_VAL(eiw, NULL);

   eiw->ef = ef;
   eiw->name = eina_stringshare_add(name);
   eiw->cipher_key = eina_stringshare_add(cipher_key);
   eiw->write_data = write_data;
   eiw->w = w;
   eiw->h = h;
   eiw->alpha = alpha;
   eiw->compress = compress;
   eiw->quality = quality;
   eiw->lossy = lossy;
   eiw->done_cb = done_cb;
   eiw->result = 0;

   if (!eio_file_set(&eiw->common,
                     NULL,
                     error_cb,
                     user_data,
                     _eio_eet_image_write_job,
                     _eio_eet_image_write_end,
                     _eio_eet_image_write_cancel))
     return NULL;
   return &eiw->common;
}

EAPI Eio_File *
eio_eet_read_direct(Eet_File *ef,
		    const char *name,
		    Eio_Done_Data_Cb done_cb,
		    Eio_Error_Cb error_cb,
		    const void *data)
{
   Eio_Eet_Read *er;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ef, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   er = eio_common_alloc(sizeof(Eio_Eet_Read));
   EINA_SAFETY_ON_NULL_RETURN_VAL(er, NULL);

   er->ef = ef;
   er->name = eina_stringshare_add(name);
   er->cipher_key = NULL;
   er->done_cb.data = done_cb;
   er->result = NULL;

   if (!eio_file_set(&er->common,
                     NULL,
                     error_cb,
                     data,
                     _eio_eet_read_direct_job,
                     _eio_eet_read_direct_end,
                     _eio_eet_read_cancel))
     return NULL;

   return &er->common;
}

EAPI Eio_File *
eio_eet_read_cipher(Eet_File *ef,
		    const char *name,
		    const char *cipher_key,
		    Eio_Done_Read_Cb done_cb,
		    Eio_Error_Cb error_cb,
		    const void *data)
{
   Eio_Eet_Read *er;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ef, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   er = eio_common_alloc(sizeof(Eio_Eet_Read));
   EINA_SAFETY_ON_NULL_RETURN_VAL(er, NULL);

   er->ef = ef;
   er->name = eina_stringshare_add(name);
   er->cipher_key = eina_stringshare_add(cipher_key);
   er->done_cb.read = done_cb;
   er->result = NULL;

   if (!eio_file_set(&er->common,
                     NULL,
                     error_cb,
                     data,
                     _eio_eet_read_cipher_job,
                     _eio_eet_read_cipher_end,
                     _eio_eet_read_cancel))
     return NULL;
   return &er->common;
}

EAPI Eio_File *
eio_eet_write_cipher(Eet_File *ef,
		     const char *name,
		     void *write_data,
		     int size,
		     int compress,
		     const char *cipher_key,
		     Eio_Done_Int_Cb done_cb,
		     Eio_Error_Cb error_cb,
		     const void *user_data)
{
   Eio_Eet_Write *ew;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ef, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(error_cb, NULL);

   ew = eio_common_alloc(sizeof(Eio_Eet_Write));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ew, NULL);

   ew->ef = ef;
   ew->name = eina_stringshare_add(name);
   ew->cipher_key = eina_stringshare_add(cipher_key);
   ew->write_data = write_data;
   ew->size = size;
   ew->compress = compress;
   ew->done_cb = done_cb;
   ew->result = 0;

   if (!eio_file_set(&ew->common,
                     NULL,
                     error_cb,
                     user_data,
                     _eio_eet_write_job,
                     _eio_eet_write_end,
                     _eio_eet_write_cancel))
     return NULL;
   return &ew->common;
}
