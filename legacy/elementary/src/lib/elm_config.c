#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

Elm_Config *_elm_config = NULL;
char *_elm_profile = NULL;
static Eet_Data_Descriptor *_config_edd = NULL;

const char *_elm_engines[] = {
   "software_x11",
   "fb",
   "directfb",
   "software_16_x11",
   "software_8_x11",
   "xrender_x11",
   "opengl_x11",
   "software_gdi",
   "software_16_wince_gdi",
   "sdl",
   "software_16_sdl",
   "opengl_sdl",
   NULL
};

static void _desc_init(void);
static void _desc_shutdown(void);
static void _profile_fetch_from_conf(void);
static void _config_free(void);
static void _config_apply(void);
static Elm_Config * _config_user_load(void);
static Elm_Config * _config_system_load(void);
static void _config_load(void);
static void _config_update(void);
static void _env_get(void);
static size_t _elm_data_dir_snprintf(char *dst, size_t size, const char *fmt, ...) EINA_PRINTF(3 ,4);
static size_t _elm_user_dir_snprintf(char *dst, size_t size, const char *fmt, ...) EINA_PRINTF(3 ,4);

#ifdef HAVE_ELEMENTARY_X
static Ecore_Event_Handler *_prop_change_handler = NULL;
static Ecore_X_Window _root_1st = 0;
#define ATOM_COUNT 5
static Ecore_X_Atom _atom[ATOM_COUNT];
static Ecore_X_Atom _atom_config = 0;
static const char *_atom_names[ATOM_COUNT] =
  {
    "ENLIGHTENMENT_SCALE",
    "ENLIGHTENMENT_FINGER_SIZE",
    "ENLIGHTENMENT_THEME",
    "ENLIGHTENMENT_PROFILE",
    "ENLIGHTENMENT_CONFIG"
  };
#define ATOM_E_SCALE 0
#define ATOM_E_FINGER_SIZE 1
#define ATOM_E_THEME 2
#define ATOM_E_PROFILE 3
#define ATOM_E_CONFIG 4

static Eina_Bool _prop_config_get(void);
static Eina_Bool _prop_change(void *data __UNUSED__, int ev_type __UNUSED__, void *ev);

static Eina_Bool
_prop_config_get(void)
{
   int size = 0;
   Ecore_X_Atom atom;
   char buf[512];
   unsigned char *data = NULL;
   Elm_Config *config_data;

   snprintf(buf, sizeof(buf), "ENLIGHTENMENT_CONFIG_%s", _elm_profile);
   atom = ecore_x_atom_get(buf);
   _atom_config = atom;
   if (!ecore_x_window_prop_property_get(_root_1st,
                                         atom, _atom[ATOM_E_CONFIG],
                                         8, &data, &size))
     {
        if (!ecore_x_window_prop_property_get(_root_1st,
                                              _atom[ATOM_E_CONFIG],
                                              _atom[ATOM_E_CONFIG],
                                              8, &data, &size))
          return EINA_FALSE;
        else
          _atom_config = _atom[ATOM_E_CONFIG];
     }
   else
     _atom_config = atom;
   if (size < 1)
     {
        free(data);
        return EINA_FALSE;
     }
   config_data = eet_data_descriptor_decode(_config_edd, data, size);
   free(data);
   if (!config_data) return EINA_FALSE;

   /* What do we do on version mismatch when someone changes the
    * config in the rootwindow? */
   /* Most obvious case, new version and we are still linked to
    * whatever was there before, we just ignore until user restarts us */
   if (config_data->config_version > ELM_CONFIG_VERSION)
     return EINA_TRUE;
   /* What in the case the version is older? Do we even support those
    * cases or we only check for equality above? */

   _config_free();
   _elm_config = config_data;
   _config_apply();
   _elm_rescale();
   return EINA_TRUE;
}

static Eina_Bool
_prop_change(void *data __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_X_Event_Window_Property *event = ev;

   if (event->win == _root_1st)
     {
        if (event->atom == _atom[ATOM_E_SCALE])
          {
             unsigned int val = 1000;

             if (ecore_x_window_prop_card32_get(event->win,
                                                event->atom,
                                                &val, 1) > 0)
               {
                  double pscale;

                  pscale = _elm_config->scale;
                  if (val > 0) _elm_config->scale = (double)val / 1000.0;
                  if (pscale != _elm_config->scale) _elm_rescale();
               }
          }
        else if (event->atom == _atom[ATOM_E_FINGER_SIZE])
          {
             unsigned int val = 1000;

             if (ecore_x_window_prop_card32_get(event->win,
                                                event->atom,
                                                &val, 1) > 0)
               {
                  int pfinger_size;

                  pfinger_size = _elm_config->finger_size;
                  _elm_config->finger_size = val;
                  if (pfinger_size != _elm_config->finger_size) _elm_rescale();
               }
          }
        else if (event->atom == _atom[ATOM_E_THEME])
          {
             char *val = NULL;

             val = ecore_x_window_prop_string_get(event->win,
                                                  event->atom);
             eina_stringshare_replace(&_elm_config->theme, val);
             if (val)
               {
                  _elm_theme_parse(NULL, val);
                  free(val);
                  _elm_rescale();
               }
          }
        else if (event->atom == _atom[ATOM_E_PROFILE])
          {
             char *val = NULL;

             val = ecore_x_window_prop_string_get(event->win,
                                                  event->atom);
             if (val)
               {
                  int changed = 0;

                  if (_elm_profile)
                    {
                       if (strcmp(_elm_profile, val)) changed = 1;
                       free(_elm_profile);
                    }
                  _elm_profile = val;
                  if (changed)
                    {
                       if (!_prop_config_get())
                         {
                            _config_free();
                            _config_load();
                            _config_apply();
                            _elm_rescale();
                         }
                    }
               }
          }
        else if (((_atom_config > 0) && (event->atom == _atom_config)) ||
                 (event->atom == _atom[ATOM_E_CONFIG]))
          {
             _prop_config_get();
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}
#endif

static void
_desc_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Config);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;

   _config_edd = eet_data_descriptor_file_new(&eddc);
   if (!_config_edd)
     {
        printf("EEEK! eet_data_descriptor_file_new() failed\n");
        return;
     }
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "config_version",
                                 config_version, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "engine", engine,
                                 EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "thumbscroll_enable",
                                 thumbscroll_enable, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "thumbscroll_threshold", thumbscroll_threshold,
                                 EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "thumbscroll_momentum_threshold",
                                 thumbscroll_momentum_threshold, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "thumbscroll_friction", thumbscroll_friction,
                                 EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "thumbscroll_bounce_friction",
                                 thumbscroll_bounce_friction, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "page_scroll_friction", page_scroll_friction,
                                 EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "bring_in_scroll_friction",
                                 bring_in_scroll_friction, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "zoom_friction",
                                 zoom_friction, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "thumbscroll_bounce_enable",
                                 thumbscroll_bounce_enable, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "scale", scale,
                                 EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "bgpixmap", bgpixmap,
                                 EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "compositing",
                                 compositing, EET_T_INT);
   /* EET_DATA_DESCRIPTOR_ADD_LIST(_config_edd, Elm_Config, "font_dirs",
      font_dirs, sub_edd); */
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "font_hinting",
                                 font_hinting, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "image_cache",
                                 image_cache, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "font_cache",
                                 font_cache, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "finger_size",
                                 finger_size, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "fps", fps,
                                 EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "theme", theme,
                                 EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "modules", modules,
                                 EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "tooltip_delay",
                                 tooltip_delay, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "cursor_engine_only",
                                 cursor_engine_only, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "focus_highlight_enable",
                                 focus_highlight_enable, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "focus_highlight_animate",
                                 focus_highlight_animate, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "toolbar_shrink_mode",
                                 toolbar_shrink_mode, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "fileselector_expand_enable",
                                 fileselector_expand_enable, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config,
                                 "inwin_dialogs_enable", inwin_dialogs_enable,
                                 EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "icon_size",
                                 icon_size, EET_T_INT);
}

static void
_desc_shutdown(void)
{
   if (!_config_edd) return;
   eet_data_descriptor_free(_config_edd);
   _config_edd = NULL;
}

static int
_sort_files_cb(const void *f1, const void *f2)
{
   return strcmp(f1, f2);
}

const char *
_elm_config_current_profile_get(void)
{
  return _elm_profile;
}

static size_t
_elm_data_dir_snprintf(char *dst, size_t size, const char *fmt, ...)
{
   size_t data_dir_len, off;
   va_list ap;

   data_dir_len = eina_strlcpy(dst, _elm_data_dir, size);

   off = data_dir_len + 1;
   if (off >= size)
     goto end;

   va_start(ap, fmt);
   dst[data_dir_len] = '/';

   off = off + vsnprintf(dst + off, size - off, fmt, ap);
   va_end(ap);

 end:
   return off;
}

static size_t
_elm_user_dir_snprintf(char *dst, size_t size, const char *fmt, ...)
{
   const char *home;
   size_t user_dir_len, off;
   va_list ap;

   home = getenv("HOME");
   if (!home)
     home = "/";

   user_dir_len = eina_str_join_len(dst, size, '/', home, strlen(home),
                                    ".elementary", sizeof(".elementary") - 1);

   off = user_dir_len + 1;
   if (off >= size)
     goto end;

   va_start(ap, fmt);
   dst[user_dir_len] = '/';

   off = off + vsnprintf(dst + off, size - off, fmt, ap);
   va_end(ap);

 end:
   return off;
}

const char *
_elm_config_profile_dir_get(const char *prof, Eina_Bool is_user)
{
   char buf[PATH_MAX];

   if (!is_user)
     goto not_user;

   _elm_user_dir_snprintf(buf, sizeof(buf), "config/%s", prof);

   if (ecore_file_is_dir(buf))
     return strdup(buf);

   return NULL;

 not_user:
   snprintf(buf, sizeof(buf), "%s/config/%s", _elm_data_dir, prof);

   if (ecore_file_is_dir(buf))
     return strdup(buf);

   return NULL;
}

Eina_List *
_elm_config_profiles_list(void)
{
   const Eina_File_Direct_Info *info;
   Eina_List *flist = NULL;
   Eina_Iterator *file_it;
   char buf[PATH_MAX];
   const char *dir;
   size_t len;

   len = _elm_user_dir_snprintf(buf, sizeof(buf), "config");

   file_it = eina_file_direct_ls(buf);
   if (!file_it)
     goto sys;

   buf[len] = '/';
   len++;

   len = sizeof(buf) - len;

   EINA_ITERATOR_FOREACH(file_it, info)
     {
       if (info->name_length >= len)
         continue;

       if (info->type == EINA_FILE_DIR)
         {
           flist =
             eina_list_sorted_insert(flist, _sort_files_cb,
                                     eina_stringshare_add(info->path +
                                                          info->name_start));
         }
     }

   eina_iterator_free(file_it);

 sys:
   len = eina_str_join_len(buf, sizeof(buf), '/', _elm_data_dir,
                           strlen(_elm_data_dir), "config",
                           sizeof("config") - 1);

   file_it = eina_file_direct_ls(buf);
   if (!file_it)
     goto list_free;

   buf[len] = '/';
   len++;

#define S_ISDIR_CASE_DO                                                     \
     {                                                                      \
       const Eina_List *l;                                                  \
       const char *tmp;                                                     \
       EINA_LIST_FOREACH(flist, l, tmp)                                     \
         if (!strcmp(info->path + info->name_start, tmp))                   \
           break;                                                           \
                                                                            \
       if (!l)                                                              \
         flist =                                                            \
           eina_list_sorted_insert(flist, _sort_files_cb,                   \
                                   eina_stringshare_add(info->path +        \
                                                        info->name_start)); \
     }                                                                      \

   len = sizeof(buf) - len;
   EINA_ITERATOR_FOREACH(file_it, info)
     {
       if (info->name_length >= len)
         continue;

       switch (info->type)
         {
         case EINA_FILE_DIR:
           S_ISDIR_CASE_DO
           break;

         default:
           continue;
         }
     }
   return flist;

#undef S_ISDIR_CASE_DO

   eina_iterator_free(file_it);

 list_free:
   EINA_LIST_FREE(flist, dir)
     eina_stringshare_del(dir);

   return NULL;
}

static void
_profile_fetch_from_conf(void)
{
   char buf[PATH_MAX], *p, *s;
   Eet_File *ef = NULL;
   int len = 0;

   _elm_profile = strdup("default");

   // if env var - use profile without question
   s = getenv("ELM_PROFILE");
   if (s)
     {
        free(_elm_profile);
        _elm_profile = strdup(s);
        return;
     }

   // user profile
   _elm_user_dir_snprintf(buf, sizeof(buf), "config/profile.cfg");
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        p = eet_read(ef, "config", &len);
        if (p)
          {
             free(_elm_profile);
             _elm_profile = malloc(len + 1);
             memcpy(_elm_profile, p, len);
             _elm_profile[len] = 0;
             free(p);
          }
        eet_close(ef);
        if (!p) ef = NULL;
     }
   if (ef) return;

   // system profile
   _elm_data_dir_snprintf(buf, sizeof(buf), "config/profile.cfg");
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        p = eet_read(ef, "config", &len);
        if (p)
          {
             free(_elm_profile);
             _elm_profile = malloc(len + 1);
             memcpy(_elm_profile, p, len);
             _elm_profile[len] = 0;
             free(p);
          }
        eet_close(ef);
     }
}

static void
_config_free(void)
{
   const char *fontdir;

   if (!_elm_config) return;
   EINA_LIST_FREE(_elm_config->font_dirs, fontdir)
     {
        eina_stringshare_del(fontdir);
     }
   if (_elm_config->engine) eina_stringshare_del(_elm_config->engine);
   if (_elm_config->theme) eina_stringshare_del(_elm_config->theme);
   if (_elm_config->modules) eina_stringshare_del(_elm_config->modules);
   free(_elm_config);
   _elm_config = NULL;
}

static void
_config_apply(void)
{
   _elm_theme_parse(NULL, _elm_config->theme);
   ecore_animator_frametime_set(1.0 / _elm_config->fps);
}

static void
_config_sub_apply(void)
{
   edje_frametime_set(1.0 / _elm_config->fps);
   edje_scale_set(_elm_config->scale);
   if (_elm_config->modules) _elm_module_parse(_elm_config->modules);
}

static Elm_Config *
_config_user_load(void)
{
   Elm_Config *cfg = NULL;
   Eet_File *ef;
   char buf[PATH_MAX];

   _elm_user_dir_snprintf(buf, sizeof(buf), "config/%s/base.cfg",
                          _elm_profile);

   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        cfg = eet_data_read(ef, _config_edd, "config");
        eet_close(ef);
     }
   return cfg;
}

static Elm_Config *
_config_system_load(void)
{
   Elm_Config *cfg = NULL;
   Eet_File *ef;
   char buf[PATH_MAX];

   _elm_data_dir_snprintf(buf, sizeof(buf), "config/%s/base.cfg",
                          _elm_profile);

   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        cfg = eet_data_read(ef, _config_edd, "config");
        eet_close(ef);
     }
   return cfg;
}

static void
_config_load(void)
{
   _elm_config = _config_user_load();
   if (_elm_config)
     {
        if (_elm_config->config_version < ELM_CONFIG_VERSION)
          _config_update();
        return;
     }

   /* no user config, fallback for system. No need to check version for
    * this one, if it's not the right one, someone screwed up at the time
    * of installing it */
   _elm_config = _config_system_load();
   if (_elm_config) return;
   /* FIXME: config load could have failed because of a non-existent
    * profile. Fallback to default before moving on */

   // config load fail - defaults
   /* XXX: do these make sense? Only if it's valid to install the lib
    * without the config, but do we want that? */
   _elm_config = ELM_NEW(Elm_Config);
   _elm_config->config_version = ELM_CONFIG_VERSION;
   _elm_config->engine = eina_stringshare_add("software_x11");
   _elm_config->thumbscroll_enable = EINA_TRUE;
   _elm_config->thumbscroll_threshold = 24;
   _elm_config->thumbscroll_momentum_threshold = 100.0;
   _elm_config->thumbscroll_friction = 1.0;
   _elm_config->thumbscroll_bounce_friction = 0.5;
   _elm_config->page_scroll_friction = 0.5;
   _elm_config->bring_in_scroll_friction = 0.5;
   _elm_config->zoom_friction = 0.5;
   _elm_config->thumbscroll_bounce_enable = EINA_TRUE;
   _elm_config->scale = 1.0;
   _elm_config->bgpixmap = 0;
   _elm_config->font_hinting = 2;
   _elm_config->font_dirs = NULL;
   _elm_config->image_cache = 4096;
   _elm_config->font_cache = 512;
   _elm_config->finger_size = 40;
   _elm_config->compositing = 1;
   _elm_config->fps = 60.0;
   _elm_config->theme = eina_stringshare_add("default");
   _elm_config->modules = NULL;
   _elm_config->tooltip_delay = 1.0;
   _elm_config->cursor_engine_only = EINA_TRUE;
   _elm_config->focus_highlight_enable = EINA_FALSE;
   _elm_config->focus_highlight_animate = EINA_TRUE;
   _elm_config->toolbar_shrink_mode = 2;
   _elm_config->fileselector_expand_enable = EINA_FALSE;
   _elm_config->inwin_dialogs_enable = EINA_FALSE;
   _elm_config->icon_size = 32;
}

static const char *
_elm_config_eet_close_error_get(Eet_File *ef, char *file)
{
   Eet_Error err;
   const char *erstr = NULL;

   err = eet_close(ef);
   switch (err)
     {
      case EET_ERROR_WRITE_ERROR:
	erstr = "An error occurred while saving Elementary's "
		"settings to disk. The error could not be "
		"deterimined. The file where the error occurred was: "
		"%s. This file has been deleted to avoid corrupt data.";
	break;
      case EET_ERROR_WRITE_ERROR_FILE_TOO_BIG:
	erstr = "Elementary's settings files are too big "
		"for the file system they are being saved to. "
		"This error is very strange as the files should "
		"be extremely small. Please check the settings "
		"for your home directory. "
		"The file where the error occurred was: %s ."
		"This file has been deleted to avoid corrupt data.";
	break;
      case EET_ERROR_WRITE_ERROR_IO_ERROR:
	erstr = "An output error occurred when writing the settings "
		"files for Elementary. Your disk is having troubles "
		"and possibly needs replacement. "
		"The file where the error occurred was: %s ."
		"This file has been deleted to avoid corrupt data.";
	break;
      case EET_ERROR_WRITE_ERROR_OUT_OF_SPACE:
	erstr = "Elementary cannot write its settings file "
		"because it ran out of space to write the file. "
		"You have either run out of disk space or have "
		"gone over your quota limit. "
		"The file where the error occurred was: %s ."
		"This file has been deleted to avoid corrupt data.";
	break;
      case EET_ERROR_WRITE_ERROR_FILE_CLOSED:
	erstr = "Elementary unexpectedly had the settings file "
		"it was writing closed on it. This is very unusual. "
		"The file where the error occurred was: %s "
		"This file has been deleted to avoid corrupt data.";
	break;
      default:
	break;
     }
   if (erstr)
     {
	/* delete any partially-written file */
	ecore_file_unlink(file);
        return strdup(erstr);
     }

   return NULL;
}

static Eina_Bool
_elm_config_profile_save(void)
{
   char buf[4096], buf2[4096];
   int ok = 0, ret;
   const char *err;
   Eet_File *ef;
   size_t len;

   len = _elm_user_dir_snprintf(buf, sizeof(buf), "config/profile.cfg");
   if (len + 1 >= sizeof(buf))
     return EINA_FALSE;

   len = _elm_user_dir_snprintf(buf2,  sizeof(buf2), "config/profile.cfg.tmp");
   if (len + 1 >= sizeof(buf2))
     return EINA_FALSE;

   ef = eet_open(buf2, EET_FILE_MODE_WRITE);
   if (!ef)
     return EINA_FALSE;

   ok = eet_write(ef, "config", _elm_profile, strlen(_elm_profile), 0);
   if (!ok)
     goto err;

   err = _elm_config_eet_close_error_get(ef, buf2);
   if (err)
     {
       ERR("%s", err);
       free((void *)err);
       goto err;
     }

   ret = ecore_file_mv(buf2, buf);
   if (!ret)
     {
       ERR("Error saving Elementary's configuration file");
       goto err;
     }

   ecore_file_unlink(buf2);
   return EINA_TRUE;

 err:
   ecore_file_unlink(buf2);
   return EINA_FALSE;
}

Eina_Bool
_elm_config_save(void)
{
   char buf[4096], buf2[4096];
   int ok = 0, ret;
   const char *err;
   Eet_File *ef;
   size_t len;

   len = _elm_user_dir_snprintf(buf, sizeof(buf), "config/%s", _elm_profile);
   if (len + 1 >= sizeof(buf))
     return EINA_FALSE;

   ok = ecore_file_mkpath(buf);
   if (!ok)
     {
       ERR("Problem acessing Elementary's user configuration directory: %s",
           buf);
       return EINA_FALSE;
     }

   if (!_elm_config_profile_save())
     return EINA_FALSE;

   buf[len] = '/';
   len++;

   if (len + sizeof("base.cfg") >= sizeof(buf) - len)
     return EINA_FALSE;

   memcpy(buf + len, "base.cfg", sizeof("base.cfg"));
   len += sizeof("base.cfg") - 1;

   if (len + sizeof(".tmp") >= sizeof(buf))
     return EINA_FALSE;

   memcpy(buf2, buf, len);
   memcpy(buf2 + len, ".tmp", sizeof(".tmp"));

   ef = eet_open(buf2, EET_FILE_MODE_WRITE);
   if (!ef)
     return EINA_FALSE;

   ok = eet_data_write(ef, _config_edd, "config", _elm_config, 1);
   if (!ok)
     goto err;

   err = _elm_config_eet_close_error_get(ef, buf2);
   if (err)
     {
       ERR("%s", err);
       free((void *)err);
       goto err;
     }

   ret = ecore_file_mv(buf2, buf);
   if (!ret)
     {
       ERR("Error saving Elementary's configuration file");
       goto err;
     }

   ecore_file_unlink(buf2);
   return EINA_TRUE;

 err:
   ecore_file_unlink(buf2);
   return EINA_FALSE;
}

static void
_config_update(void)
{
   Elm_Config *tcfg;

   tcfg = _config_system_load();
   if (!tcfg)
     {
        /* weird profile or something? We should probably fill
         * with hardcoded defaults, or get from default previx */
        return;
     }
#define IFCFG(v) if ((_elm_config->config_version & 0xffff) < (v)) {
#define IFCFGELSE } else {
#define IFCFGEND }
#define COPYVAL(x) do {_elm_config->x = tcfg->x;} while(0)
#define COPYPTR(x) do {_elm_config->x = tcfg->x; tcfg->x = NULL; } while(0)
#define COPYSTR(x) COPYPTR(x)

   /* we also need to update for property changes in the root window
    * if needed, but that will be dependent on new properties added
    * with each version */

   /* nothing here, just an example */
   /*
     IFCFG(0x0002);
     COPYVAL(some_value);
     IFCFGEND;
   */

#undef COPYSTR
#undef COPYPTR
#undef COPYVAL
#undef IFCFGEND
#undef IFCFGELSE
#undef IFCFG

   /* after updating user config, we must save */
}

static void
_env_get(void)
{
   char *s;

   s = getenv("ELM_ENGINE");
   if (s)
     {
        if ((!strcasecmp(s, "x11")) ||
            (!strcasecmp(s, "x")) ||
            (!strcasecmp(s, "software-x11")) ||
            (!strcasecmp(s, "software_x11")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_X11);
        else if ((!strcasecmp(s, "opengl")) ||
                 (!strcasecmp(s, "gl")) ||
                 (!strcasecmp(s, "opengl-x11")) ||
                 (!strcasecmp(s, "opengl_x11")))
          eina_stringshare_replace(&_elm_config->engine, ELM_OPENGL_X11);
        else if ((!strcasecmp(s, "x11-8")) ||
                 (!strcasecmp(s, "x18")) ||
                 (!strcasecmp(s, "software-8-x11")) ||
                 (!strcasecmp(s, "software_8_x11")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_8_X11);
        else if ((!strcasecmp(s, "x11-16")) ||
                 (!strcasecmp(s, "x16")) ||
                 (!strcasecmp(s, "software-16-x11")) ||
                 (!strcasecmp(s, "software_16_x11")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_16_X11);
        else if ((!strcasecmp(s, "xrender")) ||
                 (!strcasecmp(s, "xr")) ||
                 (!strcasecmp(s, "xrender-x11")) ||
                 (!strcasecmp(s, "xrender_x11")))
          eina_stringshare_replace(&_elm_config->engine, ELM_XRENDER_X11);
        else if ((!strcasecmp(s, "fb")) ||
                 (!strcasecmp(s, "software-fb")) ||
                 (!strcasecmp(s, "software_fb")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_FB);
        else if ((!strcasecmp(s, "directfb")) ||
                 (!strcasecmp(s, "dfb")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_DIRECTFB);
        else if ((!strcasecmp(s, "sdl")) ||
                 (!strcasecmp(s, "software-sdl")) ||
                 (!strcasecmp(s, "software_sdl")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_SDL);
        else if ((!strcasecmp(s, "sdl-16")) ||
                 (!strcasecmp(s, "software-16-sdl")) ||
                 (!strcasecmp(s, "software_16_sdl")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_16_SDL);
        else if ((!strcasecmp(s, "opengl-sdl")) ||
                 (!strcasecmp(s, "opengl_sdl")) ||
                 (!strcasecmp(s, "gl-sdl")) ||
                 (!strcasecmp(s, "gl_sdl")))
          eina_stringshare_replace(&_elm_config->engine, ELM_OPENGL_SDL);
        else if ((!strcasecmp(s, "gdi")) ||
                 (!strcasecmp(s, "software-gdi")) ||
                 (!strcasecmp(s, "software_gdi")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_WIN32);
        else if ((!strcasecmp(s, "wince-gdi")) ||
                 (!strcasecmp(s, "software-16-wince-gdi")) ||
                 (!strcasecmp(s, "software_16_wince_gdi")))
          eina_stringshare_replace(&_elm_config->engine, ELM_SOFTWARE_16_WINCE);
     }

   s = getenv("ELM_THUMBSCROLL_ENABLE");
   if (s) _elm_config->thumbscroll_enable = !!atoi(s);
   s = getenv("ELM_THUMBSCROLL_THRESHOLD");
   if (s) _elm_config->thumbscroll_threshold = atoi(s);
   // FIXME: floatformat locale issues here 1.0 vs 1,0 - should just be 1.0
   s = getenv("ELM_THUMBSCROLL_MOMENTUM_THRESHOLD");
   if (s) _elm_config->thumbscroll_momentum_threshold = atof(s);
   s = getenv("ELM_THUMBSCROLL_FRICTION");
   if (s) _elm_config->thumbscroll_friction = atof(s);
   s = getenv("ELM_PAGE_SCROLL_FRICTION");
   if (s) _elm_config->page_scroll_friction = atof(s);
   s = getenv("ELM_BRING_IN_SCROLL_FRICTION");
   if (s) _elm_config->bring_in_scroll_friction = atof(s);
   s = getenv("ELM_ZOOM_FRICTION");
   if (s) _elm_config->zoom_friction = atof(s);

   s = getenv("ELM_THEME");
   if (s) eina_stringshare_replace(&_elm_config->theme, s);

   s = getenv("ELM_FONT_HINTING");
   if (s)
     {
        if      (!strcasecmp(s, "none"))     _elm_config->font_hinting = 0;
        else if (!strcasecmp(s, "auto"))     _elm_config->font_hinting = 1;
        else if (!strcasecmp(s, "bytecode")) _elm_config->font_hinting = 2;
     }

   s = getenv("ELM_FONT_PATH");
   if (s)
     {
        const char *p, *pp;
        char *buf2;

        EINA_LIST_FREE(_elm_config->font_dirs, p)
          {
             eina_stringshare_del(p);
          }

        buf2 = alloca(strlen(s) + 1);
        p = s;
        pp = p;
        for (;;)
          {
             if ((*p == ':') || (*p == 0))
               {
                  int len;

                  len = p - pp;
                  strncpy(buf2, pp, len);
                  buf2[len] = 0;
                  _elm_config->font_dirs =
                    eina_list_append(_elm_config->font_dirs,
                                     eina_stringshare_add(buf2));
                  if (*p == 0) break;
                  p++;
                  pp = p;
               }
             else
               {
                  if (*p == 0) break;
                  p++;
               }
          }
     }

   s = getenv("ELM_IMAGE_CACHE");
   if (s) _elm_config->image_cache = atoi(s);

   s = getenv("ELM_FONT_CACHE");
   if (s) _elm_config->font_cache = atoi(s);

   s = getenv("ELM_SCALE");
   if (s) _elm_config->scale = atof(s);

   _elm_config->finger_size =
     (double)_elm_config->finger_size * _elm_config->scale;
   s = getenv("ELM_FINGER_SIZE");
   if (s) _elm_config->finger_size = atoi(s);

   s = getenv("ELM_FPS");
   if (s) _elm_config->fps = atof(s);
   if (_elm_config->fps < 1.0) _elm_config->fps = 1.0;

   s = getenv("ELM_MODULES");
   if (s) eina_stringshare_replace(&_elm_config->modules, s);

   s = getenv("ELM_TOOLTIP_DELAY");
   if (s)
     {
        double delay = atof(s);
        if (delay >= 0.0)
          _elm_config->tooltip_delay = delay;
     }

   s = getenv("ELM_CURSOR_ENGINE_ONLY");
   if (s) _elm_config->cursor_engine_only = !!atoi(s);

   s = getenv("ELM_FOCUS_HIGHLIGHT_ENABLE");
   if (s) _elm_config->focus_highlight_enable = !!atoi(s);

   s = getenv("ELM_FOCUS_HIGHLIGHT_ANIMATE");
   if (s) _elm_config->focus_highlight_animate = !!atoi(s);

   s = getenv("ELM_TOOLBAR_SHRINK_MODE");
   if (s) _elm_config->toolbar_shrink_mode = atoi(s);

   s = getenv("ELM_FILESELECTOR_EXPAND_ENABLE");
   if (s) _elm_config->fileselector_expand_enable = !!atoi(s);

   s = getenv("ELM_INWIN_DIALOGS_ENABLE");
   if (s) _elm_config->inwin_dialogs_enable = !!atoi(s);

   s = getenv("ELM_ICON_SIZE");
   if (s) _elm_config->icon_size = atoi(s);
}

void
_elm_config_init(void)
{
   _desc_init();
   _profile_fetch_from_conf();
   _config_load();
   _env_get();
   _config_apply();
}

void
_elm_config_sub_init(void)
{
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
  if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
      ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
      ENGINE_COMPARE(ELM_XRENDER_X11) ||
      ENGINE_COMPARE(ELM_OPENGL_X11))
#undef ENGINE_COMPARE
     {
#ifdef HAVE_ELEMENTARY_X
        unsigned int val = 1000;

        if (!ecore_x_init(NULL))
          {
             ERR("Cannot connect to X11 display. check $DISPLAY variable");
             exit(1);
          }
        _root_1st = ecore_x_window_root_first_get();

        if (!ecore_x_screen_is_composited(0))
          _elm_config->compositing = 0;

        ecore_x_atoms_get(_atom_names, ATOM_COUNT, _atom);
        ecore_x_event_mask_set(_root_1st,
                               ECORE_X_EVENT_MASK_WINDOW_PROPERTY);
        _prop_change_handler = ecore_event_handler_add
          (ECORE_X_EVENT_WINDOW_PROPERTY, _prop_change, NULL);
        if (!getenv("ELM_SCALE"))
          {
             if (ecore_x_window_prop_card32_get(_root_1st,
                                                _atom[ATOM_E_SCALE],
                                                &val, 1) > 0)
               {
                  if (val > 0)
                    {
                       _elm_config->scale = (double)val / 1000.0;
                       // FIXME: hack until e export finger size too
                       if (getenv("ELM_FINGER_SIZE"))
                         _elm_config->finger_size = 40.0 * _elm_config->scale;
                       edje_scale_set(_elm_config->scale);
                    }
               }
          }
        if (!getenv("ELM_FINGER_SIZE"))
          {
             if (ecore_x_window_prop_card32_get(_root_1st,
                                                _atom[ATOM_E_FINGER_SIZE],
                                                &val, 1) > 0)
               {
                  if (val > 0)
                    {
                       _elm_config->finger_size = val;
                    }
               }
          }
        if (!getenv("ELM_THEME"))
          {
             char *s;

             s = ecore_x_window_prop_string_get(_root_1st,
                                                _atom[ATOM_E_THEME]);
             if (s)
               {
                  eina_stringshare_replace(&_elm_config->theme, s);
                  _elm_theme_parse(NULL, s);
                  free(s);
               }
          }
        if (!getenv("ELM_PROFILE"))
          {
             char *s;

             s = ecore_x_window_prop_string_get(_root_1st,
                                                _atom[ATOM_E_PROFILE]);
             if (s)
               {
                  int changed = 0;

                  if (_elm_profile)
                    {
                       if (strcmp(_elm_profile, s)) changed = 1;
                       free(_elm_profile);
                    }
                  _elm_profile = s;
                  if (changed) _prop_config_get();
               }
          }
#endif
     }
   _config_sub_apply();
}

void
_elm_config_reload(void)
{
  _config_free();
  _config_load();
  _config_apply();
  _elm_rescale();
}

void
_elm_config_profile_set(const char *profile)
{
  Eina_Bool changed = EINA_FALSE;

  if (_elm_profile)
    {
      if (strcmp(_elm_profile, profile))
        changed = 1;
      free(_elm_profile);
    }

  _elm_profile = strdup(profile);

  if (changed)
    {
      _config_free();
      _config_load();
      _config_apply();
      _elm_rescale();
    }
}

void
_elm_config_shutdown(void)
{
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
  if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
      ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
      ENGINE_COMPARE(ELM_XRENDER_X11) ||
      ENGINE_COMPARE(ELM_OPENGL_X11))
#undef ENGINE_COMPARE
     {
#ifdef HAVE_ELEMENTARY_X
        ecore_event_handler_del(_prop_change_handler);
        _prop_change_handler = NULL;
#endif
     }
   _config_free();
   if (_elm_profile)
     {
        free(_elm_profile);
        _elm_profile = NULL;
     }
   _desc_shutdown();
}
