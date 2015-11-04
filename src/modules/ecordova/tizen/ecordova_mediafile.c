#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if defined(TIZEN_MEDIA_METADATA_EXTRACTOR) && defined(TIZEN_MEDIA_PLAYER) && defined(TIZEN_MEDIA_RECORDER)
#include "ecordova_mediafile_private.h"
#include <limits.h>

#define MY_CLASS ECORDOVA_MEDIAFILE_CLASS
#define MY_CLASS_NAME "Ecordova_MediaFile"

static void _extract_cb(void *, Ecore_Thread *);
static void _extract_end_cb(void *, Ecore_Thread *);
static void _extract_cancel_cb(void *, Ecore_Thread *);
static void _internal_error_notify(Ecordova_MediaFile_Data *pd);
static Eina_Bool _bool_metadata_get(metadata_extractor_h, metadata_extractor_attr_e);
static int _int_metadata_get(metadata_extractor_h, metadata_extractor_attr_e);

#define NO_ERROR (INT_MAX)

static Eo_Base *
_ecordova_mediafile_eo_base_constructor(Eo *obj, Ecordova_MediaFile_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->extractor = NULL;
   pd->error = NO_ERROR;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_mediafile_eo_base_destructor(Eo *obj, Ecordova_MediaFile_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->extractor)
     metadata_extractor_destroy(pd->extractor);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_mediafile_format_data_get(Eo *obj EINA_UNUSED,
                                    Ecordova_MediaFile_Data *pd)
{
   Ecore_Thread *thread = ecore_thread_run(_extract_cb,
                                           _extract_end_cb,
                                           _extract_cancel_cb,
                                           pd);
   if (!thread)
     _internal_error_notify(pd);
}

static void
_extract_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecordova_MediaFile_Data *pd = data;
   int ret;

   if (!pd->extractor)
     {
         ret = metadata_extractor_create(&pd->extractor);
         if (METADATA_EXTRACTOR_ERROR_NONE != ret)
           goto on_error;

         const char *url = eo_do_super_ret(pd->obj,
                                           MY_CLASS,
                                           url,
                                           ecordova_file_url_get());
         ret = metadata_extractor_set_path(pd->extractor, url);
         if (METADATA_EXTRACTOR_ERROR_NONE != ret)
           goto on_error;
     }

   pd->metadata.codecs = NULL; // TODO: what is it?
   pd->metadata.duration = _int_metadata_get(pd->extractor, METADATA_DURATION) / 1000;

   Eina_Bool has_video = _bool_metadata_get(pd->extractor, METADATA_HAS_VIDEO);
   Eina_Bool has_audio = _bool_metadata_get(pd->extractor, METADATA_HAS_AUDIO);

   DBG("has_video=%d, has_audio=%d", has_video, has_audio);
   if (has_video)
     {
        pd->metadata.width = _int_metadata_get(pd->extractor, METADATA_VIDEO_WIDTH);
        pd->metadata.height = _int_metadata_get(pd->extractor, METADATA_VIDEO_HEIGHT);
        pd->metadata.bitrate = _int_metadata_get(pd->extractor, METADATA_VIDEO_BITRATE);
     }
   else
     {
        pd->metadata.bitrate = _int_metadata_get(pd->extractor, METADATA_AUDIO_BITRATE);
     }

   pd->error = NO_ERROR;
   return;

on_error:
   pd->error = ECORDOVA_CAPTURE_ERRORCODE_CAPTURE_INTERNAL_ERR;
}

static void
_extract_end_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecordova_MediaFile_Data *pd = data;
   if (NO_ERROR != pd->error)
     {
        Ecordova_Capture_Error error = {.code = pd->error};
        eo_do(pd->obj,
              eo_event_callback_call(ECORDOVA_MEDIAFILE_EVENT_ERROR,
                                     &error));
        return;
     }

   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_MEDIAFILE_EVENT_SUCCESS,
                                &pd->metadata));
}

static void
_extract_cancel_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecordova_MediaFile_Data *pd = data;
   _internal_error_notify(pd);
}

static void
_internal_error_notify(Ecordova_MediaFile_Data *pd)
{
   Ecordova_Capture_Error error = {
     .code = ECORDOVA_CAPTURE_ERRORCODE_CAPTURE_INTERNAL_ERR
   };
   eo_do(pd->obj, eo_event_callback_call(ECORDOVA_MEDIAFILE_EVENT_ERROR, &error));
}

static Eina_Bool
_bool_metadata_get(metadata_extractor_h extractor,
                   metadata_extractor_attr_e attr)
{
   Eina_Bool result = EINA_FALSE;
   char *value = NULL;
   int ret = metadata_extractor_get_metadata(extractor, attr, &value);
   if (value)
     {
        result = strcmp(value, "TRUE") == 0;
        free(value);
     }

   EINA_SAFETY_ON_FALSE_RETURN_VAL(METADATA_EXTRACTOR_ERROR_NONE == ret, EINA_FALSE);
   return result;
}

static int
_int_metadata_get(metadata_extractor_h extractor,
                  metadata_extractor_attr_e attr)
{
   int result = 0;
   char *value = NULL;
   int ret = metadata_extractor_get_metadata(extractor, attr, &value);
   if (value)
     {
        result = atoi(value);
        free(value);
     }

   EINA_SAFETY_ON_FALSE_RETURN_VAL(METADATA_EXTRACTOR_ERROR_NONE == ret, 0);
   return result;
}

#include "ecordova_mediafile.eo.c"
#endif
