#include <Elementary.h>
#include "elm_priv.h"

static Elm_Theme theme_default =
{
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 1
};

static Eina_List *themes = NULL;

static void
_elm_theme_clear(Elm_Theme *th)
{
   const char *p;
   EINA_LIST_FREE(th->themes, p)
      eina_stringshare_del(p);
   EINA_LIST_FREE(th->overlay, p)
      eina_stringshare_del(p);
   EINA_LIST_FREE(th->extension, p)
      eina_stringshare_del(p);
   if (th->cache)
     {
        eina_hash_free(th->cache);
        th->cache = NULL;
     }
   if (th->cache_data)
     {
        eina_hash_free(th->cache_data);
        th->cache_data = NULL;
     }
   if (th->theme)
     {
        eina_stringshare_del(th->theme);
        th->theme = NULL;
     }
   if (th->ref_theme)
     {
        th->ref_theme->referrers =
           eina_list_remove(th->ref_theme->referrers, th);
        elm_theme_free(th->ref_theme);
        th->ref_theme = NULL;
     }
}

static const char *
_elm_theme_find_try(Elm_Theme *th, const char *f, const char *group)
{
   const char *file;

   if (edje_file_group_exists(f, group))
     {
        file = eina_stringshare_add(f);
        if (file)
          {
             eina_hash_add(th->cache, group, file);
             return file;
          }
     }
   return NULL;
}

static const char *
_elm_theme_theme_element_try(Elm_Theme *th, const char *home, const char *f, const char *group)
{
   char buf[PATH_MAX];
   const char *file = NULL;

   if ((f[0] == '/') || ((f[0] == '.') && (f[1] == '/')) ||
       ((f[0] == '.') && (f[1] == '.') && (f[2] == '/')) ||
       ((isalpha(f[0])) && (f[1] == ':')))
     return _elm_theme_find_try(th, f, group);
   else if (((f[0] == '~') && (f[1] == '/')))
     {
        snprintf(buf, sizeof(buf), "%s/%s", home, f + 2);
        return _elm_theme_find_try(th, buf, group);
     }
   snprintf(buf, sizeof(buf), "%s/"ELEMENTARY_BASE_DIR"/themes/%s.edj", home, f);
   file = _elm_theme_find_try(th, buf, group);
   if (file) return file;
   snprintf(buf, sizeof(buf), "%s/themes/%s.edj", _elm_data_dir, f);
   file = _elm_theme_find_try(th, buf, group);
   return file;
}

static const char *
_elm_theme_group_file_find(Elm_Theme *th, const char *group)
{
   const Eina_List *l;
   const char *f;
   static const char *home = NULL;
   const char *file = eina_hash_find(th->cache, group);

   if (file) return file;
   if (!home)
     {
        home = getenv("HOME");
        if (!home) home = "";
     }
   EINA_LIST_FOREACH(th->overlay, l, f)
     {
        file = _elm_theme_theme_element_try(th, home, f, group);
        if (file) return file;
     }
   EINA_LIST_FOREACH(th->themes, l, f)
     {
        file = _elm_theme_theme_element_try(th, home, f, group);
        if (file) return file;
     }
   EINA_LIST_FOREACH(th->extension, l, f)
     {
        file = _elm_theme_theme_element_try(th, home, f, group);
        if (file) return file;
     }
   if (th->ref_theme) return _elm_theme_group_file_find(th->ref_theme, group);
   return NULL;
}

static const char *
_elm_theme_find_data_try(Elm_Theme *th, const char *f, const char *key)
{
   char *data;
   const char *t;

   data = edje_file_data_get(f, key);
   t = eina_stringshare_add(data);
   free(data);
   if (t)
     {
        eina_hash_add(th->cache, key, t);
        return t;
     }
   return NULL;
}

static const char *
_elm_theme_theme_data_try(Elm_Theme *th, const char *home, const char *f, const char *key)
{
   char buf[PATH_MAX];
   const char *data = NULL;

   if ((f[0] == '/') || ((f[0] == '.') && (f[1] == '/')) ||
       ((f[0] == '.') && (f[1] == '.') && (f[2] == '/')) ||
       ((isalpha(f[0])) && (f[1] == ':')))
     return _elm_theme_find_data_try(th, f, key);
   else if (((f[0] == '~') && (f[1] == '/')))
     {
        snprintf(buf, sizeof(buf), "%s/%s", home, f + 2);
        return _elm_theme_find_try(th, buf, key);
     }
   snprintf(buf, sizeof(buf), "%s/"ELEMENTARY_BASE_DIR"/themes/%s.edj", home, f);
   data = _elm_theme_find_data_try(th, buf, key);
   if (data) return data;
   snprintf(buf, sizeof(buf), "%s/themes/%s.edj", _elm_data_dir, f);
   data = _elm_theme_find_data_try(th, buf, key);
   return data;
}

static const char *
_elm_theme_data_find(Elm_Theme *th, const char *key)
{
   const Eina_List *l;
   const char *f;
   static const char *home = NULL;
   const char *data = eina_hash_find(th->cache_data, key);

   if (data) return data;
   if (!home)
     {
        home = getenv("HOME");
        if (!home) home = "";
     }
   EINA_LIST_FOREACH(th->overlay, l, f)
     {
        data = _elm_theme_theme_data_try(th, home, f, key);
        if (data) return data;
     }
   EINA_LIST_FOREACH(th->themes, l, f)
     {
        data = _elm_theme_theme_data_try(th, home, f, key);
        if (data) return data;
     }
   EINA_LIST_FOREACH(th->extension, l, f)
     {
        data = _elm_theme_theme_data_try(th, home, f, key);
        if (data) return data;
     }
   if (th->ref_theme) return _elm_theme_data_find(th->ref_theme, key);
   return NULL;
}

static void _elm_theme_idler_clean(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__);

static Eina_Bool
_elm_theme_reload_idler(void *data)
{
   Evas_Object *elm = data;

   elm_widget_theme(elm);
   evas_object_data_del(elm, "elm-theme-reload-idler");
   evas_object_event_callback_del(elm, EVAS_CALLBACK_DEL, _elm_theme_idler_clean);
   return EINA_FALSE;
}

static void
_elm_theme_idler_clean(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Ecore_Idler *idler;

   idler = evas_object_data_get(obj, "elm-theme-reload-idler");
   if (idler) ecore_idler_del(idler);
   evas_object_data_del(obj, "elm-theme-reload-idler");
}

static void
_elm_theme_reload(void *data __UNUSED__, Evas_Object *obj,
                  const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *elm;

   elm = evas_object_data_get(obj, "elm-parent");
   if (elm)
     {
        evas_object_event_callback_add(elm, EVAS_CALLBACK_DEL, _elm_theme_idler_clean, NULL);
        evas_object_data_set(elm, "elm-theme-reload-idler", ecore_idler_add(_elm_theme_reload_idler, elm));
     }
}                  

Eina_Bool
_elm_theme_object_set(Evas_Object *parent, Evas_Object *o, const char *clas, const char *group, const char *style)
{
   Elm_Theme *th = NULL;
   void *test;

   if (parent) th = elm_widget_theme_get(parent);
   test = evas_object_data_get(o, "edje,theme,watcher");
   if (!test)
     {
        edje_object_signal_callback_add(o, "edje,change,file", "edje", _elm_theme_reload, NULL);
        evas_object_data_set(o, "edje,theme,watcher", (void*) -1);
     }
   return _elm_theme_set(th, o, clas, group, style);
}

/* only issued by elm_icon.c */
Eina_Bool
_elm_theme_object_icon_set(Evas_Object *o,
                           const char *group,
                           const char *style)
{
   Elm_Theme *th = elm_widget_theme_get(o);

   return _elm_theme_icon_set(th, o, group, style);
}

Eina_Bool
_elm_theme_set(Elm_Theme *th, Evas_Object *o, const char *clas, const char *group, const char *style)
{
   const char *file;
   char buf2[1024];
   Eina_Bool ok;

   if ((!clas) || (!group) || (!style)) return EINA_FALSE;
   if (!th) th = &(theme_default);
   snprintf(buf2, sizeof(buf2), "elm/%s/%s/%s", clas, group, style);
   file = _elm_theme_group_file_find(th, buf2);
   if (file)
     {
        ok = edje_object_file_set(o, file, buf2);
        if (ok) return EINA_TRUE;
        else
          DBG("could not set theme group '%s' from file '%s': %s",
              buf2, file, edje_load_error_str(edje_object_load_error_get(o)));
     }
   snprintf(buf2, sizeof(buf2), "elm/%s/%s/default", clas, group);
   file = _elm_theme_group_file_find(th, buf2);
   if (!file) return EINA_FALSE;
   ok = edje_object_file_set(o, file, buf2);
   if (!ok)
     DBG("could not set theme group '%s' from file '%s': %s",
         buf2, file, edje_load_error_str(edje_object_load_error_get(o)));
   return ok;
}

Eina_Bool
_elm_theme_icon_set(Elm_Theme *th,
                    Evas_Object *o,
                    const char *group,
                    const char *style)
{
   const char *file;
   char buf2[1024];
   int w, h;

   if (!th) th = &(theme_default);
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/%s", group, style);
   file = _elm_theme_group_file_find(th, buf2);
   if (file)
     {
        elm_image_file_set(o, file, buf2);
        elm_image_object_size_get(o, &w, &h);
        if (w > 0) return EINA_TRUE;
     }
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/default", group);
   file = _elm_theme_group_file_find(th, buf2);

   if (!file) return EINA_FALSE;

   elm_image_file_set(o, file, buf2);
   elm_image_object_size_get(o, &w, &h);

   return w > 0;
}

Eina_Bool
_elm_theme_parse(Elm_Theme *th, const char *theme)
{
   Eina_List *names = NULL;
   const char *p, *pe;

   if (!th) th = &(theme_default);
   if (theme)
     {
        Eina_Strbuf *buf;

        buf = eina_strbuf_new();

        p = theme;
        pe = p;
        for (;;)
          {
             if ((pe[0] == '\\') && (pe[1] == ':'))
               {
                  eina_strbuf_append_char(buf, ':');
                  pe += 2;
               }
             else if ((*pe == ':') || (!*pe))
               { // p -> pe == 'name:'
                  if (pe > p)
                    {
                       const char *nn;

                       nn = eina_stringshare_add(eina_strbuf_string_get(buf));
                       if (nn) names = eina_list_append(names, nn);
                       eina_strbuf_reset(buf);
                    }
                  if (!*pe) break;
                  p = pe + 1;
                  pe = p;
               }
             else
               {
                  eina_strbuf_append_char(buf, *pe);
                  pe++;
               }
          }

        eina_strbuf_free(buf);
     }
   p = eina_list_data_get(eina_list_last(names));
   if ((!p) || ((p) && (strcmp(p, "default"))))
     {
        p = eina_stringshare_add("default");
        if (p) names = eina_list_append(names, p);
     }
   if (th->cache) eina_hash_free(th->cache);
   th->cache = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));
   if (th->cache_data) eina_hash_free(th->cache_data);
   th->cache_data = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));

   EINA_LIST_FREE(th->themes, p) eina_stringshare_del(p);

   th->themes = names;
   return EINA_TRUE;
}

void
_elm_theme_shutdown(void)
{
   _elm_theme_clear(&(theme_default));
}

EAPI Elm_Theme *
elm_theme_new(void)
{
   Elm_Theme *th = calloc(1, sizeof(Elm_Theme));
   if (!th) return NULL;
   th->ref = 1;
   th->themes = eina_list_append(th->themes, eina_stringshare_add("default"));
   themes = eina_list_append(themes, th);
   return th;
}

EAPI void
elm_theme_free(Elm_Theme *th)
{
   EINA_SAFETY_ON_NULL_RETURN(th);
   th->ref--;
   if (th->ref < 1)
     {
        _elm_theme_clear(th);
        themes = eina_list_remove(themes, th);
        free(th);
     }
}

EAPI void
elm_theme_copy(Elm_Theme *th, Elm_Theme *thdst)
{
   const Eina_List *l;
   const char *f;

   if (!th) th = &(theme_default);
   if (!thdst) thdst = &(theme_default);
   _elm_theme_clear(thdst);
   if (th->ref_theme)
     {
        thdst->ref_theme = th->ref_theme;
        thdst->ref_theme->referrers =
           eina_list_append(thdst->ref_theme->referrers, thdst);
        thdst->ref_theme->ref++;
     }
   EINA_LIST_FOREACH(th->overlay, l, f)
     {
        const char *s = eina_stringshare_add(f);
        if (s) thdst->overlay = eina_list_append(thdst->overlay, s);
     }
   EINA_LIST_FOREACH(th->themes, l, f)
     {
        const char *s = eina_stringshare_add(f);
        if (s) thdst->themes = eina_list_append(thdst->themes, s);
     }
   EINA_LIST_FOREACH(th->extension, l, f)
     {
        const char *s = eina_stringshare_add(f);
        if (s) thdst->extension = eina_list_append(thdst->extension, s);
     }
   if (th->theme) thdst->theme = eina_stringshare_add(th->theme);
   elm_theme_flush(thdst);
}

EAPI void
elm_theme_ref_set(Elm_Theme *th, Elm_Theme *thref)
{
   if (!th) th = &(theme_default);
   if (!thref) thref = &(theme_default);
   if (th->ref_theme == thref) return;
   _elm_theme_clear(th);
   if (thref)
     {
        thref->referrers = eina_list_append(thref->referrers, th);
        thref->ref++;
     }
   th->ref_theme = thref;
   elm_theme_flush(th);
}

EAPI Elm_Theme *
elm_theme_ref_get(Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   return th->ref_theme;
}

EAPI Elm_Theme *
elm_theme_default_get(void)
{
   return &theme_default;
}

EAPI void
elm_theme_overlay_add(Elm_Theme *th, const char *item)
{
   const char *f = eina_stringshare_add(item);

   if (!th) th = &(theme_default);
   if (f) th->overlay = eina_list_prepend(th->overlay, f);
   elm_theme_flush(th);
}

EAPI void
elm_theme_overlay_del(Elm_Theme *th, const char *item)
{
   const Eina_List *l;
   const char *f, *s;

   if (!th) th = &(theme_default);
   s = eina_stringshare_add(item);
   EINA_LIST_FOREACH(th->overlay, l, f)
      if (f == s)
        {
           eina_stringshare_del(f);
           th->overlay = eina_list_remove_list(th->overlay, (Eina_List *)l);
           break;
        }
   eina_stringshare_del(s);
   elm_theme_flush(th);
}

EAPI const Eina_List *
elm_theme_overlay_list_get(const Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   return th->overlay;
}

EAPI void
elm_theme_extension_add(Elm_Theme *th, const char *item)
{
   const char *f = eina_stringshare_add(item);

   if (!th) th = &(theme_default);
   if (f) th->extension = eina_list_append(th->extension, f);
   elm_theme_flush(th);
}

EAPI void
elm_theme_extension_del(Elm_Theme *th, const char *item)
{
   const Eina_List *l;
   const char *f, *s;

   if (!th) th = &(theme_default);
   s = eina_stringshare_add(item);
   EINA_LIST_FOREACH(th->extension, l, f)
      if (f == s)
        {
           eina_stringshare_del(f);
           th->extension = eina_list_remove_list(th->extension, (Eina_List *)l);
           break;
        }
   eina_stringshare_del(s);
   elm_theme_flush(th);
}

EAPI const Eina_List *
elm_theme_extension_list_get(const Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   return th->extension;
}

EAPI void
elm_theme_set(Elm_Theme *th, const char *theme)
{
   if (!th) th = &(theme_default);
   _elm_theme_parse(th, theme);
   if (th->theme)
     {
        eina_stringshare_del(th->theme);
        th->theme = NULL;
     }
   elm_theme_flush(th);
   if (th == &(theme_default))
     eina_stringshare_replace(&_elm_config->theme, theme);
}

EAPI const char *
elm_theme_get(Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   if (!th->theme)
     {
        Eina_Strbuf *buf;
        Eina_List *l;
        const char *f;

        buf = eina_strbuf_new();
        EINA_LIST_FOREACH(th->themes, l, f)
          {
             while (*f)
               {
                  if (*f == ':')
                    eina_strbuf_append_char(buf, '\\');
                  eina_strbuf_append_char(buf, *f);

                  f++;
               }
             if (l->next) eina_strbuf_append_char(buf, ':');
          }
        th->theme = eina_stringshare_add(eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
     }
   return th->theme;
}

EAPI const Eina_List *
elm_theme_list_get(const Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   return th->themes;
}

EAPI char *
elm_theme_list_item_path_get(const char *f, Eina_Bool *in_search_path)
{
   static const char *home = NULL;
   char buf[PATH_MAX];

   if (!f)
     {
        if (in_search_path) *in_search_path = EINA_FALSE;
        return NULL;
     }

   if (!home)
     {
        home = getenv("HOME");
        if (!home) home = "";
     }

   if ((f[0] == '/') || ((f[0] == '.') && (f[1] == '/')) ||
       ((f[0] == '.') && (f[1] == '.') && (f[2] == '/')) ||
       ((isalpha(f[0])) && (f[1] == ':')))
     {
        if (in_search_path) *in_search_path = EINA_FALSE;
        return strdup(f);
     }
   else if (((f[0] == '~') && (f[1] == '/')))
     {
        if (in_search_path) *in_search_path = EINA_FALSE;
        snprintf(buf, sizeof(buf), "%s/%s", home, f + 2);
        return strdup(buf);
     }
   snprintf(buf, sizeof(buf), "%s/"ELEMENTARY_BASE_DIR"/themes/%s.edj", home, f);
   if (ecore_file_exists(buf))
     {
        if (in_search_path) *in_search_path = EINA_TRUE;
        return strdup(buf);
     }

   snprintf(buf, sizeof(buf), "%s/themes/%s.edj", _elm_data_dir, f);
   if (ecore_file_exists(buf))
     {
        if (in_search_path) *in_search_path = EINA_TRUE;
        return strdup(buf);
     }

   if (in_search_path) *in_search_path = EINA_FALSE;
   return NULL;
}

EAPI void
elm_theme_flush(Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   if (th->cache) eina_hash_free(th->cache);
   th->cache = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));
   if (th->cache_data) eina_hash_free(th->cache_data);
   th->cache_data = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));
   _elm_win_rescale(th, EINA_TRUE);
   _elm_ews_wm_rescale(th, EINA_TRUE);
   if (th->referrers)
     {
        Eina_List *l;
        Elm_Theme *th2;

        EINA_LIST_FOREACH(th->referrers, l, th2) elm_theme_flush(th2);
     }
}

EAPI void
elm_theme_full_flush(void)
{
   Eina_List *l;
   Elm_Theme *th;

   EINA_LIST_FOREACH(themes, l, th)
     {
        elm_theme_flush(th);
     }
   elm_theme_flush(&(theme_default));
}

EAPI Eina_List *
elm_theme_name_available_list_new(void)
{
   Eina_List *list = NULL;
   Eina_List *dir, *l;
   char buf[PATH_MAX], *file, *s, *th;
   static const char *home = NULL;

   if (!home)
     {
        home = getenv("HOME");
        if (!home) home = "";
     }

   snprintf(buf, sizeof(buf), "%s/"ELEMENTARY_BASE_DIR"/themes", home);
   dir = ecore_file_ls(buf);
   EINA_LIST_FREE(dir, file)
     {
        snprintf(buf, sizeof(buf), "%s/"ELEMENTARY_BASE_DIR"/themes/%s", home, file);
        if ((!ecore_file_is_dir(buf)) && (ecore_file_size(buf) > 0))
          {
             s = strchr(file, '.');
             if ((s) && (!strcasecmp(s, ".edj")))
               {
                  th = strdup(file);
                  s = strchr(th, '.');
                  *s = 0;
                  list = eina_list_append(list, th);
               }
          }
        free(file);
     }

   snprintf(buf, sizeof(buf), "%s/themes", _elm_data_dir);
   dir = ecore_file_ls(buf);
   EINA_LIST_FREE(dir, file)
     {
        snprintf(buf, sizeof(buf), "%s/themes/%s", _elm_data_dir, file);
        if ((!ecore_file_is_dir(buf)) && (ecore_file_size(buf) > 0))
          {
             s = strchr(file, '.');
             if ((s) && (!strcasecmp(s, ".edj")))
               {
                  int dupp;

                  th = strdup(file);
                  s = strchr(th, '.');
                  *s = 0;
                  dupp = 0;
                  EINA_LIST_FOREACH(list, l, s)
                    {
                       if (!strcmp(s, th))
                         {
                            dupp = 1;
                            break;
                         }
                    }
                  if (dupp) free(th);
                  else list = eina_list_append(list, th);
               }
          }
        free(file);
     }
   list = eina_list_sort(list, 0, EINA_COMPARE_CB(strcasecmp));
   return list;
}

EAPI void
elm_theme_name_available_list_free(Eina_List *list)
{
   char *s;
   EINA_LIST_FREE(list, s) free(s);
}

EAPI void
elm_object_theme_set(Evas_Object *obj, Elm_Theme *th)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_theme_set(obj, th);
}

EAPI Elm_Theme *
elm_object_theme_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_theme_get(obj);
}

EAPI const char *
elm_theme_data_get(Elm_Theme *th, const char *key)
{
   if (!th) th = &(theme_default);
   return _elm_theme_data_find(th, key);
}
