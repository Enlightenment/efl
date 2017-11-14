#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_icon.eo.h"

static Elm_Theme theme_default =
{
  { NULL, NULL }, { NULL, NULL }, { NULL, NULL },
  NULL, NULL, NULL, NULL, NULL, 1, NULL
};

static Eina_List *themes = NULL;

static Eina_File *
_elm_theme_find_try(Elm_Theme *th, Eina_File *f, const char *group)
{
   if (edje_mmap_group_exists(f, group))
     {
        eina_hash_add(th->cache, group, eina_file_dup(f));
        return f;
     }
   return NULL;
}

static inline void
_elm_theme_item_finalize(Elm_Theme_Files *files,
                         const char *item,
                         Eina_File *f,
                         Eina_Bool prepend,
                         Eina_Bool istheme)
{
   /* Theme version history:
    * <110: legacy, had no version tag
    *  110: first supported version
    *  119: switched windows to always use border
    *       win group has no menu, no blocker
    *       border group has all required swallows (conformant, bg, win)
    *       data: "elm_bg_version" matches "version" ("119")
    */
   if (!f) return;
   if (istheme)
     {
        char *version;
        int v;

        if (!(version = edje_mmap_data_get(f, "version"))) return;
        v = atoi(version);
        if (v < 110) // bump this version number when we need to
          {
             WRN("Selected theme is too old (version = %d, needs >= 110)", v);
          }
        free(version);
     }
   if (prepend)
     {
        files->items = eina_list_prepend(files->items,
                                         eina_stringshare_add(item));
        files->handles = eina_list_prepend(files->handles, f);
     }
   else
     {
        files->items = eina_list_append(files->items,
                                        eina_stringshare_add(item));
        files->handles = eina_list_append(files->handles, f);
     }
}

static void
_elm_theme_file_item_add(Elm_Theme_Files *files, const char *item, Eina_Bool prepend, Eina_Bool istheme)
{
   Eina_Strbuf *buf = NULL;
   Eina_File *f = NULL;
   const char *home;

   home = eina_environment_home_get();
   buf = eina_strbuf_new();

   if ((item[0] == '/') ||
       ((item[0] == '.') && (item[1] == '/')) ||
       ((item[0] == '.') && (item[1] == '.') && (item[2] == '/')) ||
       ((isalpha(item[0])) && (item[1] == ':')))
     {
        f = eina_file_open(item, EINA_FALSE);
        if (!f) goto on_error;
     }
   else if (((item[0] == '~') && (item[1] == '/')))
     {
        eina_strbuf_append_printf(buf, "%s/%s", home, item + 2);

        f = eina_file_open(eina_strbuf_string_get(buf), EINA_FALSE);
        if (!f) goto on_error;
     }
   else
     {
        eina_strbuf_append_printf(buf,
                                  "%s/"ELEMENTARY_BASE_DIR"/themes/%s.edj",
                                  home, item);
        f = eina_file_open(eina_strbuf_string_get(buf), EINA_FALSE);
        _elm_theme_item_finalize(files, item, f, prepend, istheme);

        eina_strbuf_reset(buf);

        eina_strbuf_append_printf(buf,
                                  "%s/themes/%s.edj",
                                  _elm_data_dir, item);
        f = eina_file_open(eina_strbuf_string_get(buf), EINA_FALSE);
        /* Finalize will be done by the common one */
     }

   _elm_theme_item_finalize(files, item, f, prepend, istheme);

 on_error:
   if (buf) eina_strbuf_free(buf);
}

static void
_elm_theme_file_item_del(Elm_Theme_Files *files, const char *str)
{
   Eina_List *l, *ll;
   Eina_List *l2, *ll2;
   const char *item;

   str = eina_stringshare_add(str);
   l2 = files->handles;

   EINA_LIST_FOREACH_SAFE(files->items, l, ll, item)
     {
        ll2 = l2->next;

        if (item == str)
          {
             eina_file_close(eina_list_data_get(l2));
             eina_stringshare_del(item);

             files->handles = eina_list_remove_list(files->handles, l2);
             files->items = eina_list_remove_list(files->items, l);
          }

        l2 = ll2;
     }

   eina_stringshare_del(str);
}

static void
_elm_theme_file_mmap_del(Elm_Theme_Files *files, const Eina_File *file)
{
   Eina_List *l, *ll;
   Eina_List *l2, *ll2;
   Eina_File *f;

   l2 = files->items;
   EINA_LIST_FOREACH_SAFE(files->handles, l, ll, f)
     {
        ll2 = l2->next;

        if (f == file)
          {
             eina_file_close(f);
             eina_stringshare_del(eina_list_data_get(l2));

             files->handles = eina_list_remove_list(files->handles, l);
             files->items = eina_list_remove_list(files->items, l2);
          }

        l2 = ll2;
     }
}

static void
_elm_theme_file_clean(Elm_Theme_Files *files)
{
   const char *item;
   Eina_File *f;

   EINA_LIST_FREE(files->items, item)
     eina_stringshare_del(item);
   EINA_LIST_FREE(files->handles, f)
     eina_file_close(f);
}

static void
_elm_theme_clear(Elm_Theme *th)
{
   _elm_theme_file_clean(&th->themes);
   _elm_theme_file_clean(&th->overlay);
   _elm_theme_file_clean(&th->extension);

   ELM_SAFE_FREE(th->cache, eina_hash_free);
   ELM_SAFE_FREE(th->cache_data, eina_hash_free);
   ELM_SAFE_FREE(th->cache_style_load_failed, eina_hash_free);
   ELM_SAFE_FREE(th->theme, eina_stringshare_del);
   if (th->ref_theme)
     {
        th->ref_theme->referrers =
           eina_list_remove(th->ref_theme->referrers, th);
        elm_theme_free(th->ref_theme);
        th->ref_theme = NULL;
     }
}

Eina_File *
_elm_theme_group_file_find(Elm_Theme *th, const char *group)
{
   const Eina_List *l;
   Eina_File *file = eina_hash_find(th->cache, group);

   if (file) return file;

   EINA_LIST_FOREACH(th->overlay.handles, l, file)
     {
        file = _elm_theme_find_try(th, file, group);
        if (file) return file;
     }
   EINA_LIST_FOREACH(th->themes.handles, l, file)
     {
        file = _elm_theme_find_try(th, file, group);
        if (file) return file;
     }
   EINA_LIST_FOREACH(th->extension.handles, l, file)
     {
        file = _elm_theme_find_try(th, file, group);
        if (file) return file;
     }
   if (th->ref_theme) return _elm_theme_group_file_find(th->ref_theme, group);
   return NULL;
}

static const char *
_elm_theme_find_data_try(Elm_Theme *th, const Eina_File *f, const char *key)
{
   char *data;
   const char *t;

   data = edje_mmap_data_get(f, key);
   t = eina_stringshare_add(data);
   free(data);
   if (t)
     {
        eina_hash_add(th->cache_data, key, t);
        return t;
     }
   return NULL;
}

static const char *
_elm_theme_data_find(Elm_Theme *th, const char *key)
{
   const Eina_List *l;
   const Eina_File *f;

   const char *data = eina_hash_find(th->cache_data, key);
   if (data) return data;

   EINA_LIST_FOREACH(th->overlay.handles, l, f)
     {
        data = _elm_theme_find_data_try(th, f, key);
        if (data) return data;
     }
   EINA_LIST_FOREACH(th->themes.handles, l, f)
     {
        data = _elm_theme_find_data_try(th, f, key);
        if (data) return data;
     }
   EINA_LIST_FOREACH(th->extension.handles, l, f)
     {
        data = _elm_theme_find_data_try(th, f, key);
        if (data) return data;
     }
   if (th->ref_theme) return _elm_theme_data_find(th->ref_theme, key);
   return NULL;
}

Efl_Ui_Theme_Apply
_elm_theme_object_set(Evas_Object *parent, Evas_Object *o, const char *clas, const char *group, const char *style)
{
   Elm_Theme *th = NULL;

   if (parent) th = elm_widget_theme_get(parent);
   return _elm_theme_set(th, o, clas, group, style, elm_widget_is_legacy(parent));
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

Efl_Ui_Theme_Apply
_elm_theme_set(Elm_Theme *th, Evas_Object *o, const char *clas, const char *group, const char *style, Eina_Bool is_legacy)
{
   Eina_File *file;
   char buf2[1024];
   const char *group_sep = "/";
   const char *style_sep = ":";

   if ((!clas) || !o) return EFL_UI_THEME_APPLY_FAILED;
   if (!th) th = &(theme_default);
   if (is_legacy)
     snprintf(buf2, sizeof(buf2), "elm/%s/%s/%s", clas, (group) ? group : "base", (style) ? style : "default");
   else
     snprintf(buf2, sizeof(buf2), "efl/%s%s%s%s%s", clas,
            ((group) ? group_sep : "\0"), ((group) ? group : "\0"),
            ((style) ? style_sep : "\0"), ((style) ? style : "\0"));
   if (!eina_hash_find(th->cache_style_load_failed, buf2))
     {
        file = _elm_theme_group_file_find(th, buf2);
        if (file)
          {
             if (edje_object_mmap_set(o, file, buf2)) return EFL_UI_THEME_APPLY_SUCCESS;
             else
               {
                  ERR("could not set theme group '%s' from file '%s': %s",
                      buf2,
                      eina_file_filename_get(file),
                      edje_load_error_str(edje_object_load_error_get(o)));
               }
          }
        //style not found, add to the not found list
        eina_hash_add(th->cache_style_load_failed, buf2, (void *)1);
     }

   if (!style)
     return EFL_UI_THEME_APPLY_FAILED;

   // Use the elementary default style.
   if (is_legacy)
     snprintf(buf2, sizeof(buf2), "elm/%s/%s/%s", clas, (group) ? group : "base", "default");
   else
     snprintf(buf2, sizeof(buf2), "efl/%s%s%s", clas,
            ((group) ? group_sep : "\0"), ((group) ? group : "\0"));
   if (!eina_hash_find(th->cache_style_load_failed, buf2))
     {
        file = _elm_theme_group_file_find(th, buf2);
        if (file)
          {
             if (edje_object_mmap_set(o, file, buf2))
               {
                  INF("could not set theme style '%s', fallback to default",
                      style);
                  return EFL_UI_THEME_APPLY_DEFAULT;
               }
             else
               {
                  INF("could not set theme group '%s' from file '%s': %s",
                      buf2,
                      eina_file_filename_get(file),
                      edje_load_error_str(edje_object_load_error_get(o)));
               }
          }
        //style not found, add to the not found list
        eina_hash_add(th->cache_style_load_failed, buf2, (void *)1);
     }
   return EFL_UI_THEME_APPLY_FAILED;
}

Eina_Bool
_elm_theme_icon_set(Elm_Theme *th,
                    Evas_Object *o,
                    const char *group,
                    const char *style)
{
   Eina_File *file;
   char buf2[1024];
   int w, h;

   if (efl_isa((o), ELM_ICON_CLASS) && elm_icon_standard_get(o) &&
       strcmp(elm_config_icon_theme_get(), ELM_CONFIG_ICON_THEME_ELEMENTARY))
     {
        elm_icon_standard_set(o, elm_icon_standard_get(o));
        return EINA_TRUE;
     }

   if (!th) th = &(theme_default);
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/%s", group, style);
   file = _elm_theme_group_file_find(th, buf2);
   if (file)
     {
        elm_image_mmap_set(o, file, buf2);
        elm_image_object_size_get(o, &w, &h);
        if (w > 0) return EINA_TRUE;
     }
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/default", group);
   file = _elm_theme_group_file_find(th, buf2);

   if (!file) return EINA_FALSE;

   elm_image_mmap_set(o, file, buf2);
   elm_image_object_size_get(o, &w, &h);

   return w > 0;
}

void
_elm_theme_parse(Elm_Theme *th, const char *theme)
{
   Eina_List *names = NULL;
   Eina_File *f;
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
#ifdef HAVE_ELEMENTARY_WIN32
             else if (isalpha(pe[0]) && (pe[1] == ':') &&
                      ((pe[2] == '/') || (pe[2] == '\\')))
               {
                  // Correct processing file path on  Windows OS "<disk>:/" or "<disk>:\"
                  eina_strbuf_append_char(buf, *pe);
                  pe++;
                  eina_strbuf_append_char(buf, *pe);
                  pe++;
               }
#endif
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
   if ((!p) || (strcmp(p, "default")))
     {
        p = eina_stringshare_add("default");
        if (p) names = eina_list_append(names, p);
     }
   if (th->cache) eina_hash_free(th->cache);
   th->cache = eina_hash_string_superfast_new(EINA_FREE_CB(eina_file_close));
   if (th->cache_data) eina_hash_free(th->cache_data);
   th->cache_data = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));
   if (th->cache_style_load_failed) eina_hash_free(th->cache_style_load_failed);
   th->cache_style_load_failed = eina_hash_string_superfast_new(NULL);
   EINA_LIST_FREE(th->themes.items, p) eina_stringshare_del(p);
   EINA_LIST_FREE(th->themes.handles, f) eina_file_close(f);

   EINA_LIST_FREE(names, p)
     _elm_theme_file_item_add(&th->themes, p, EINA_FALSE, EINA_TRUE);
}

void
_elm_theme_shutdown(void)
{
   Elm_Theme *th;
   _elm_theme_clear(&(theme_default));
   EINA_LIST_FREE(themes, th)
     {
        _elm_theme_clear(th);
        free(th);
     }
}

EAPI Elm_Theme *
elm_theme_new(void)
{
   Elm_Theme *th = calloc(1, sizeof(Elm_Theme));
   if (!th) return NULL;
   th->ref = 1;
   _elm_theme_file_item_add(&th->themes, "default", EINA_FALSE, EINA_TRUE);
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

static void
elm_theme_files_copy(Elm_Theme_Files *dst, Elm_Theme_Files *src)
{
   const Eina_List *l;
   const Eina_File *f;
   const char *item;

   EINA_LIST_FOREACH(src->items, l, item)
     dst->items = eina_list_append(dst->items,
                                   eina_stringshare_ref(item));
   EINA_LIST_FOREACH(src->handles, l, f)
     dst->handles = eina_list_append(dst->handles,
                                     eina_file_dup(f));
}

EAPI void
elm_theme_copy(Elm_Theme *th, Elm_Theme *thdst)
{
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
   elm_theme_files_copy(&thdst->overlay, &th->overlay);
   elm_theme_files_copy(&thdst->themes, &th->themes);
   elm_theme_files_copy(&thdst->extension, &th->extension);

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
elm_theme_ref_get(const Elm_Theme *th)
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
   if (!item) return;
   if (!th) th = &(theme_default);
   _elm_theme_file_item_add(&th->overlay, item, EINA_TRUE, EINA_FALSE);
   elm_theme_flush(th);
}

EAPI void
elm_theme_overlay_del(Elm_Theme *th, const char *item)
{
   if (!item) return;
   if (!th) th = &(theme_default);
   _elm_theme_file_item_del(&th->overlay, item);
   elm_theme_flush(th);
}

EAPI void
elm_theme_overlay_mmap_add(Elm_Theme *th, const Eina_File *f)
{
   Eina_File *file = eina_file_dup(f);

   if (!th) th = &(theme_default);
   _elm_theme_item_finalize(&th->overlay, eina_file_filename_get(file), file, EINA_TRUE, EINA_FALSE);
   elm_theme_flush(th);
}

EAPI void
elm_theme_overlay_mmap_del(Elm_Theme *th, const Eina_File *f)
{
   if (!f) return;
   if (!th) th = &(theme_default);
   _elm_theme_file_mmap_del(&th->overlay, f);
   elm_theme_flush(th);
}

EAPI const Eina_List *
elm_theme_overlay_list_get(const Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   return th->overlay.items;
}

EAPI void
elm_theme_extension_add(Elm_Theme *th, const char *item)
{
   if (!item) return;
   if (!th) th = &(theme_default);
   _elm_theme_file_item_add(&th->extension, item, EINA_FALSE, EINA_FALSE);
   elm_theme_flush(th);
}

EAPI void
elm_theme_extension_del(Elm_Theme *th, const char *item)
{
   if (!item) return;
   if (!th) th = &(theme_default);
   _elm_theme_file_item_del(&th->extension, item);
   elm_theme_flush(th);
}

EAPI void
elm_theme_extension_mmap_add(Elm_Theme *th, const Eina_File *f)
{
   Eina_File *file = eina_file_dup(f);

   if (!f) return;
   if (!th) th = &(theme_default);
   _elm_theme_item_finalize(&th->extension, eina_file_filename_get(file), file, EINA_FALSE, EINA_FALSE);
   elm_theme_flush(th);
}

EAPI void
elm_theme_extension_mmap_del(Elm_Theme *th, const Eina_File *f)
{
   if (!f) return;
   if (!th) th = &(theme_default);
   _elm_theme_file_mmap_del(&th->extension, f);
   elm_theme_flush(th);
}

EAPI const Eina_List *
elm_theme_extension_list_get(const Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   return th->extension.items;
}

EAPI void
elm_theme_set(Elm_Theme *th, const char *theme)
{
   if (!th) th = &(theme_default);
   _elm_theme_parse(th, theme);
   ELM_SAFE_FREE(th->theme, eina_stringshare_del);
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
        EINA_LIST_FOREACH(th->themes.items, l, f)
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
   return th->themes.items;
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
        home = eina_environment_home_get();
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
   th->cache = eina_hash_string_superfast_new(EINA_FREE_CB(eina_file_close));
   if (th->cache_data) eina_hash_free(th->cache_data);
   th->cache_data = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));
   if (th->cache_style_load_failed) eina_hash_free(th->cache_style_load_failed);
   th->cache_style_load_failed = eina_hash_string_superfast_new(NULL);
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
        home = eina_environment_home_get();
        if (!home) home = "";
     }

   snprintf(buf, sizeof(buf), "%s/"ELEMENTARY_BASE_DIR"/themes", home);
   dir = ecore_file_ls(buf);
   EINA_LIST_FREE(dir, file)
     {
        snprintf(buf, sizeof(buf), "%s/"ELEMENTARY_BASE_DIR"/themes/%s", home, file);
        if ((!ecore_file_is_dir(buf)) && (ecore_file_size(buf) > 0))
          {
             if (eina_str_has_extension(file, "edj"))
               {
                  th = strdup(file);
                  s = strrchr(th, '.');
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
             if (eina_str_has_extension(file, "edj"))
               {
                  int dupp;

                  th = strdup(file);
                  s = strrchr(th, '.');
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

EAPI const char *
elm_theme_group_path_find(Elm_Theme *th, const char *group)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(group, NULL);
   Eina_File *th_file = NULL;
   if (!th) th = &(theme_default);

   th_file = _elm_theme_group_file_find(th, group);
   if (th_file)
     return eina_file_filename_get(th_file);
   return NULL;
}

static Eina_List *
_elm_theme_file_group_base_list(Eina_List *list,
                                Eina_List *handles,
                                const char *base, int len)
{
   Eina_Stringshare *c, *c2;
   Eina_List *coll;
   Eina_List *in, *l, *ll;
   Eina_File *f;

   EINA_LIST_FOREACH(handles, l, f)
     {
        coll = edje_mmap_collection_list(f);
        EINA_LIST_FOREACH(coll, ll, c)
          {
             // if base == start of collection str
             if (!strncmp(c, base, len))
               {
                  // check if already in list
                  EINA_LIST_FOREACH(list, in, c2)
                    if (c == c2)
                      break;

                  // if not already in list append shared str to list
                  if (!in)
                    list = eina_list_append(list, eina_stringshare_ref(c));
               }
          }
        edje_mmap_collection_list_free(coll);
     }

   return list;
}

EAPI Eina_List *
elm_theme_group_base_list(Elm_Theme *th, const char *base)
{
   Eina_List *list;
   int len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(base, NULL);
   if (!th) th = &(theme_default);

   // XXX: look results up in a hash for speed
   len = strlen(base);
   // go through all possible theme files and find collections that match
   list = _elm_theme_file_group_base_list(NULL, th->overlay.handles,
                                          base, len);
   list = _elm_theme_file_group_base_list(list, th->themes.handles,
                                          base, len);
   list = _elm_theme_file_group_base_list(list, th->extension.handles,
                                          base, len);

   // sort the list nicely at the end
   list = eina_list_sort(list, 0, EINA_COMPARE_CB(strcmp));
   // XXX: store results in hash for fast lookup...
   return list;
}

EAPI const char *
elm_theme_system_dir_get(void)
{
   static char *path = NULL;
   char buf[PATH_MAX];

   if (path) return path;

   snprintf(buf, sizeof(buf), "%s/themes", _elm_data_dir);
   path = strdup(buf);

   return path;
}

EAPI const char *
elm_theme_user_dir_get(void)
{
   static char *path = NULL;
   char buf[PATH_MAX];
   const char *home;

   if (path) return path;

   home = eina_environment_home_get();
   snprintf(buf, sizeof(buf), "%s/"ELEMENTARY_BASE_DIR"/themes", home);
   path = strdup(buf);

   return path;
}
