#include "private.h"

typedef struct _Elm_Params_Video
{
   Elm_Params base;
   const char *file;
   const char *uri;
   Eina_Bool play:1;
   Eina_Bool play_exists:1;
   Eina_Bool pause:1;
   Eina_Bool pause_exists:1;
   Eina_Bool stop:1;
   Eina_Bool stop_exists:1;
   Eina_Bool audio_mute:1;
   Eina_Bool audio_mute_exists:1;
   double audio_level;
   Eina_Bool audio_level_exists:1;
   double play_position;
   Eina_Bool play_position_exists:1;
   Eina_Bool remember_position:1;
   Eina_Bool remember_position_exists:1;
} Elm_Params_Video;

static void
external_video_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Video *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->file) elm_video_file_set(obj, p->file);
   if (p->uri) elm_video_uri_set(obj, p->uri);
   if (p->play_exists && p->play) elm_video_play(obj);
   if (p->pause_exists && p->pause) elm_video_pause(obj);
   if (p->stop_exists && p->stop) elm_video_stop(obj);
   if (p->audio_mute_exists) elm_video_audio_mute_set(obj, p->audio_mute);
   if (p->audio_level_exists) elm_video_audio_level_set(obj, p->audio_level);
   if (p->play_position_exists)
     elm_video_play_position_set(obj, p->play_position);
   if (p->remember_position_exists)
     elm_video_remember_position_set(obj, p->remember_position);
}

static Eina_Bool
external_video_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
       && (!strcmp(param->name, "file")))
     {
        elm_video_file_set(obj, param->s);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
       && (!strcmp(param->name, "uri")))
     {
        elm_video_uri_set(obj, param->s);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
       && (!strcmp(param->name, "play")))
     {
        if (param->i)
          elm_video_play(obj);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
       && (!strcmp(param->name, "pause")))
     {
        if (param->i)
          elm_video_pause(obj);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
       && (!strcmp(param->name, "stop")))
     {
        if (param->i)
          elm_video_stop(obj);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
       && (!strcmp(param->name, "audio mute")))
     {
        elm_video_audio_mute_set(obj, param->i);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
       && (!strcmp(param->name, "audio level")))
     {
        elm_video_audio_level_set(obj, param->d);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
       && (!strcmp(param->name, "play position")))
     {
        elm_video_play_position_set(obj, param->d);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
       && (!strcmp(param->name, "remember position")))
     {
        elm_video_remember_position_set(obj, param->i);
        return EINA_TRUE;
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_video_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
       && (!strcmp(param->name, "file")))
     {
        //        param->s = elm_video_file_get(obj);
        //        return EINA_TRUE;
        return EINA_FALSE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
                && (!strcmp(param->name, "uri")))
     {
        //        elm_video_uri_get(obj, param->s);
        //        return EINA_TRUE;
        return EINA_FALSE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
            && (!strcmp(param->name, "play")))
     {
        //        param->i = elm_video_play_get(obj); return EINA_TRUE;
        return EINA_FALSE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
            && (!strcmp(param->name, "pause")))
     {
             //        param->i = elm_video_pause_get(obj); return EINA_TRUE;
        return EINA_FALSE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
            && (!strcmp(param->name, "stop")))
     {
        //        param->i = elm_video_stop_get(obj); return EINA_TRUE;
        return EINA_FALSE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
            && (!strcmp(param->name, "audio mute")))
     {
        param->i = elm_video_audio_mute_get(obj);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE) &&
            (!strcmp(param->name, "audio level")))
     {
        param->d = elm_video_audio_level_get(obj);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
            && (!strcmp(param->name, "play position")))
     {
        param->d = elm_video_play_position_get(obj);
        return EINA_TRUE;
     }
   else if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
            && (!strcmp(param->name, "remember position")))
     {
        param->i = elm_video_remember_position_get(obj);
        return EINA_TRUE;
     }

   ERR("unknown parameter '%s' of type '%s'", param->name,
       edje_external_param_type_str(param->type));

   return EINA_FALSE; }

static void * external_video_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Video *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Video));
   if (!mem) return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "file"))
          mem->file = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "uri"))
          mem->uri = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "play"))
          {
             mem->play = param->i;
             mem->play_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "pause"))
          {
             mem->pause = param->i;
             mem->pause_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "stop"))
          {
             mem->stop = param->i;
             mem->stop_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "audio mute"))
          {
             mem->audio_mute = param->i;
             mem->audio_mute_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "audio level"))
          {
             mem->audio_level = param->d;
             mem->audio_level_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "play position"))
          {
             mem->play_position = param->d;
             mem->play_position_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "remember position"))
          {
             mem->remember_position = param->i;
             mem->remember_position = EINA_TRUE;
          }
     }
   return mem;
}

static Evas_Object *external_video_content_get(void *data __UNUSED__, const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
   ERR("No content.");
   return NULL;
}

static void external_video_params_free(void *params)
{
   Elm_Params_Video *mem = params;

   if (mem->file) eina_stringshare_del(mem->file);
   if (mem->uri) eina_stringshare_del(mem->uri);
   free(params);
}

static Edje_External_Param_Info external_video_params[] = {
     DEFINE_EXTERNAL_COMMON_PARAMS, EDJE_EXTERNAL_PARAM_INFO_STRING("file"),
     EDJE_EXTERNAL_PARAM_INFO_STRING("uri"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("play"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("pause"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("stop"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("audio mute"),
     EDJE_EXTERNAL_PARAM_INFO_DOUBLE("audio level"),
     EDJE_EXTERNAL_PARAM_INFO_DOUBLE("play position"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("remember position"),
     EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(video, "video");
DEFINE_EXTERNAL_TYPE_SIMPLE(video, "Video");
