#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_icon.eo.h"

#include "efl_ui_theme.eo.h"

static Elm_Theme *theme_default = NULL;

static Eina_List *themes = NULL;

static Eina_File *
_elm_theme_find_try(Elm_Theme *th, Elm_Theme_File *etf, const char *group, Eina_Bool force)
{
   if (edje_mmap_group_exists(etf->handle, group))
     {
        if (etf->match_theme && (!force)) // overlay or extension
          {
             Elm_Theme_File *base_etf;
             Eina_Bool found = EINA_FALSE;

             EINA_INLIST_FOREACH(th->themes, base_etf)
               {
                  if (base_etf->base_theme != etf->match_theme) continue;
                  found = EINA_TRUE;
                  break;
               }
             if (!found) return NULL;
          }
        eina_hash_add(th->cache, group, eina_file_dup(etf->handle));
        return etf->handle;
     }
   return NULL;
}

static inline void
_elm_theme_item_finalize(Eina_Inlist **files,
                         const char *item,
                         Eina_File *f,
                         Eina_Bool prepend,
                         Eina_Bool istheme)
{
   Elm_Theme_File *etf;
   char *name;
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
   etf = calloc(1, sizeof(Elm_Theme_File));
   EINA_SAFETY_ON_NULL_RETURN(etf);
   etf->item = eina_stringshare_add(item);
   etf->handle = f;
   if (istheme)
     {
        name = edje_mmap_data_get(f, "efl_theme_base");
        etf->base_theme = eina_stringshare_add(name);
     }
   else
     {
        name = edje_mmap_data_get(f, "efl_theme_match");
        etf->match_theme = eina_stringshare_add(name);
     }
   free(name);
   if (prepend)
     {
        *files = eina_inlist_prepend(*files, EINA_INLIST_GET(etf));
     }
   else
     {
        *files = eina_inlist_append(*files, EINA_INLIST_GET(etf));
     }
}

static void
_elm_theme_file_item_add(Eina_Inlist **files, const char *item, Eina_Bool prepend, Eina_Bool istheme)
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
_elm_theme_file_item_del(Eina_Inlist **files, const char *str)
{
   Eina_Inlist *l;
   Elm_Theme_File *item;

   str = eina_stringshare_add(str);

   EINA_INLIST_FOREACH_SAFE(*files, l, item)
     {
        if (item->item != str) continue;
        eina_file_close(item->handle);
        eina_stringshare_del(item->item);
        *files = eina_inlist_remove(*files, EINA_INLIST_GET(item));
        free(item);
     }

   eina_stringshare_del(str);
}

static void
_elm_theme_file_mmap_del(Eina_Inlist **files, const Eina_File *file)
{
   Eina_Inlist *l;
   Elm_Theme_File *item;

   EINA_INLIST_FOREACH_SAFE(*files, l, item)
     {
        if (item->handle != file) continue;
        eina_file_close(item->handle);
        eina_stringshare_del(item->item);
        *files = eina_inlist_remove(*files, EINA_INLIST_GET(item));
        free(item);
     }
}

static void
_elm_theme_file_clean(Eina_Inlist **files)
{
   while (*files)
     {
        Elm_Theme_File *etf = EINA_INLIST_CONTAINER_GET(*files, Elm_Theme_File);

        eina_stringshare_del(etf->item);
        eina_file_close(etf->handle);
        eina_stringshare_del(etf->match_theme);
        *files = eina_inlist_remove(*files, *files);
        free(etf);
     }
}

static void
_elm_theme_clear(Elm_Theme *th)
{
   Efl_Ui_Theme_Data *td;

   _elm_theme_file_clean(&th->themes);
   _elm_theme_file_clean(&th->overlay);
   _elm_theme_file_clean(&th->extension);

   ELM_SAFE_FREE(th->overlay_items, eina_list_free);
   ELM_SAFE_FREE(th->theme_items, eina_list_free);
   ELM_SAFE_FREE(th->extension_items, eina_list_free);

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

   td = efl_data_scope_get(th->eo_theme, EFL_UI_THEME_CLASS);
   td->th = NULL;
   th->eo_theme = NULL;
}

static Eina_File *
_elm_theme_group_file_find_internal(Elm_Theme *th, const char *group, Eina_Bool force)
{
   Elm_Theme_File *etf;
   Eina_File *file = eina_hash_find(th->cache, group);

   if (file) return file;

   EINA_INLIST_FOREACH(th->overlay, etf)
     {
        file = _elm_theme_find_try(th, etf, group, force);
        if (file) return file;
     }
   EINA_INLIST_FOREACH(th->themes, etf)
     {
        file = _elm_theme_find_try(th, etf, group, force);
        if (file) return file;
     }
   EINA_INLIST_FOREACH(th->extension, etf)
     {
        file = _elm_theme_find_try(th, etf, group, force);
        if (file) return file;
     }
   if (th->ref_theme) return _elm_theme_group_file_find_internal(th->ref_theme, group, force);
   return NULL;
}

Eina_File *
_elm_theme_group_file_find(Elm_Theme *th, const char *group)
{
   Eina_File *file = _elm_theme_group_file_find_internal(th, group, EINA_FALSE);
   if (file) return file;
   file = _elm_theme_group_file_find_internal(th, group, EINA_TRUE);
   return file;
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
   Elm_Theme_File *etf;

   const char *data = eina_hash_find(th->cache_data, key);
   if (data) return data;

   EINA_INLIST_FOREACH(th->overlay, etf)
     {
        data = _elm_theme_find_data_try(th, etf->handle, key);
        if (data) return data;
     }
   EINA_INLIST_FOREACH(th->themes, etf)
     {
        data = _elm_theme_find_data_try(th, etf->handle, key);
        if (data) return data;
     }
   EINA_INLIST_FOREACH(th->extension, etf)
     {
        data = _elm_theme_find_data_try(th, etf->handle, key);
        if (data) return data;
     }
   if (th->ref_theme) return _elm_theme_data_find(th->ref_theme, key);
   return NULL;
}

Efl_Ui_Theme_Apply_Result
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

Efl_Ui_Theme_Apply_Result
_elm_theme_set(Elm_Theme *th, Evas_Object *o, const char *clas, const char *group, const char *style, Eina_Bool is_legacy)
{
   Eina_File *file;
   char buf2[1024];
   const char *group_sep = "/";
   const char *style_sep = ":";

   if ((!clas) || !o) return EFL_UI_THEME_APPLY_RESULT_FAIL;
   if (!th) th = theme_default;
   if (!th) return EFL_UI_THEME_APPLY_RESULT_FAIL;

   if (eina_streq(style, "default")) style = NULL;

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
             if (edje_object_mmap_set(o, file, buf2)) return EFL_UI_THEME_APPLY_RESULT_SUCCESS;
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
     return EFL_UI_THEME_APPLY_RESULT_FAIL;

   // Use the elementary default style.
   return (EFL_UI_THEME_APPLY_RESULT_DEFAULT &
           _elm_theme_set(th, o, clas, group, NULL, is_legacy));
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

   if (!th) th = theme_default;
   if (!th) return EINA_FALSE;

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
   const char *p, *pe;

   if (!th) th = theme_default;
   if (!th) return;

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
   _elm_theme_file_clean(&th->themes);
   th->theme_items = eina_list_free(th->theme_items);

   EINA_LIST_FREE(names, p)
     _elm_theme_file_item_add(&th->themes, p, EINA_FALSE, EINA_TRUE);
   elm_theme_get(th);
}

void
_elm_theme_init(void)
{
   Eo *theme_default_obj;
   Efl_Ui_Theme_Data *td;

   if (theme_default) return;

   theme_default_obj = efl_add(EFL_UI_THEME_CLASS, efl_main_loop_get());
   td = efl_data_scope_get(theme_default_obj, EFL_UI_THEME_CLASS);
   theme_default = td->th;
}

void
_elm_theme_shutdown(void)
{
   Elm_Theme *th;

   while (themes)
     {
        th = eina_list_data_get(themes);

        /* In theme object destructor, theme is deallocated and the theme is
         * removed from themes list in _elm_theme_free_internal().
         * Consequently, themes list is updated.
         */
        efl_del(th->eo_theme);
     }
}

EAPI Elm_Theme *
elm_theme_new(void)
{
   Eo *obj = efl_add(EFL_UI_THEME_CLASS, efl_main_loop_get());
   Efl_Ui_Theme_Data *td = efl_data_scope_get(obj, EFL_UI_THEME_CLASS);
   return td->th;
}

EAPI void
elm_theme_free(Elm_Theme *th)
{
   EINA_SAFETY_ON_NULL_RETURN(th);

   /* Destructs theme object and theme is deallocated in
    * _elm_theme_free_internal() in theme object desctructor.
    */
   efl_unref(th->eo_theme);
}

static void
elm_theme_files_copy(Eina_Inlist **dst, Eina_Inlist **src)
{
   Elm_Theme_File *etf, *cpy;

   EINA_INLIST_FOREACH(*src, etf)
     {
        cpy = malloc(sizeof(Elm_Theme_File));
        EINA_SAFETY_ON_NULL_RETURN(cpy);
        cpy->item = eina_stringshare_ref(etf->item);
        cpy->handle = eina_file_dup(etf->handle);
        *dst = eina_inlist_append(*dst, EINA_INLIST_GET(cpy));
     }
}

EAPI void
elm_theme_copy(Elm_Theme *th, Elm_Theme *thdst)
{
   Eo *thdst_obj;
   Efl_Ui_Theme_Data *thdst_td;

   if (!th) th = theme_default;
   if (!th) return;
   if (!thdst) thdst = theme_default;
   if (!thdst) return;

   //Clear the given theme and restore the relationship with theme object.
   thdst_obj = thdst->eo_theme;
   _elm_theme_clear(thdst);
   thdst->eo_theme = thdst_obj;
   thdst_td = efl_data_scope_get(thdst_obj, EFL_UI_THEME_CLASS);
   thdst_td->th = thdst;

   if (th->ref_theme)
     {
        thdst->ref_theme = th->ref_theme;
        thdst->ref_theme->referrers =
           eina_list_append(thdst->ref_theme->referrers, thdst);
        efl_ref(thdst->ref_theme->eo_theme);
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
   Eo *th_obj;
   Efl_Ui_Theme_Data *th_td;

   if (!th) th = theme_default;
   if (!th) return;
   if (!thref) thref = theme_default;
   if (!thref) return;
   if (th->ref_theme == thref) return;

   //Clear the given theme and restore the relationship with theme object.
   th_obj = th->eo_theme;
   _elm_theme_clear(th);
   th->eo_theme = th_obj;
   th_td = efl_data_scope_get(th_obj, EFL_UI_THEME_CLASS);
   th_td->th = th;

   if (thref)
     {
        thref->referrers = eina_list_append(thref->referrers, th);
        efl_ref(thref->eo_theme);
     }
   th->ref_theme = thref;
   elm_theme_flush(th);
}

EAPI Elm_Theme *
elm_theme_ref_get(const Elm_Theme *th)
{
   if (!th) th = theme_default;
   if (!th) return NULL;
   return th->ref_theme;
}

EAPI Elm_Theme *
elm_theme_default_get(void)
{
   return theme_default;
}

EAPI void
elm_theme_overlay_add(Elm_Theme *th, const char *item)
{
   if (!th) th = theme_default;
   if (!th) return;
   efl_ui_theme_overlay_add(th->eo_theme, item);
}

EAPI void
elm_theme_overlay_del(Elm_Theme *th, const char *item)
{
   if (!th) th = theme_default;
   if (!th) return;
   efl_ui_theme_overlay_del(th->eo_theme, item);
}

EAPI void
elm_theme_overlay_mmap_add(Elm_Theme *th, const Eina_File *f)
{
   Eina_File *file = eina_file_dup(f);

   if (!th) th = theme_default;
   if (!th) return;
   th->overlay_items = eina_list_free(th->overlay_items);
   _elm_theme_item_finalize(&th->overlay, eina_file_filename_get(file), file, EINA_TRUE, EINA_FALSE);
   elm_theme_flush(th);
}

EAPI void
elm_theme_overlay_mmap_del(Elm_Theme *th, const Eina_File *f)
{
   if (!f) return;
   if (!th) th = theme_default;
   if (!th) return;
   th->overlay_items = eina_list_free(th->overlay_items);
   _elm_theme_file_mmap_del(&th->overlay, f);
   elm_theme_flush(th);
}

EAPI const Eina_List *
elm_theme_overlay_list_get(const Elm_Theme *th)
{
   if (!th) th = theme_default;
   if (!th) return NULL;
   if (!th->overlay_items)
     {
        Elm_Theme_File *etf;

        EINA_INLIST_FOREACH(th->overlay, etf)
          ((Elm_Theme*)th)->overlay_items = eina_list_append(th->overlay_items, etf->item);
     }
   return th->overlay_items;
}

EAPI void
elm_theme_extension_add(Elm_Theme *th, const char *item)
{
   if (!th) th = theme_default;
   if (!th) return;
   efl_ui_theme_extension_add(th->eo_theme, item);
}

EAPI void
elm_theme_extension_del(Elm_Theme *th, const char *item)
{
   if (!th) th = theme_default;
   if (!th) return;
   efl_ui_theme_extension_del(th->eo_theme, item);
}

EAPI void
elm_theme_extension_mmap_add(Elm_Theme *th, const Eina_File *f)
{
   Eina_File *file = eina_file_dup(f);

   if (!f) return;
   if (!th) th = theme_default;
   if (!th) return;
   th->extension_items = eina_list_free(th->extension_items);
   _elm_theme_item_finalize(&th->extension, eina_file_filename_get(file), file, EINA_FALSE, EINA_FALSE);
   elm_theme_flush(th);
}

EAPI void
elm_theme_extension_mmap_del(Elm_Theme *th, const Eina_File *f)
{
   if (!f) return;
   if (!th) th = theme_default;
   if (!th) return;
   th->extension_items = eina_list_free(th->extension_items);
   _elm_theme_file_mmap_del(&th->extension, f);
   elm_theme_flush(th);
}

EAPI const Eina_List *
elm_theme_extension_list_get(const Elm_Theme *th)
{
   if (!th) th = theme_default;
   if (!th) return NULL;
   if (!th->extension_items)
     {
        Elm_Theme_File *etf;

        EINA_INLIST_FOREACH(th->extension, etf)
          ((Elm_Theme*)th)->extension_items = eina_list_append(th->extension_items, etf->item);
     }
   return th->extension_items;
}

EAPI void
elm_theme_set(Elm_Theme *th, const char *theme)
{
   if (!th) th = theme_default;
   if (!th) return;
   _elm_theme_parse(th, theme);
   ELM_SAFE_FREE(th->theme, eina_stringshare_del);
   elm_theme_flush(th);
   if (th == theme_default)
     eina_stringshare_replace(&_elm_config->theme, theme);
}

EAPI const char *
elm_theme_get(Elm_Theme *th)
{
   if (!th) th = theme_default;
   if (!th) return NULL;
   if (!th->theme)
     {
        Eina_Strbuf *buf;
        Elm_Theme_File *etf;
        const char *f;

        buf = eina_strbuf_new();
        EINA_INLIST_FOREACH(th->themes, etf)
          {
             f = etf->item;
             while (*f)
               {
                  if (*f == ':')
                    eina_strbuf_append_char(buf, '\\');
                  eina_strbuf_append_char(buf, *f);

                  f++;
               }
             if (EINA_INLIST_GET(etf)->next)
               eina_strbuf_append_char(buf, ':');
          }
        th->theme = eina_stringshare_add(eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
     }
   return th->theme;
}

EAPI const Eina_List *
elm_theme_list_get(const Elm_Theme *th)
{
   if (!th) th = theme_default;
   if (!th) return NULL;
   if (!th->theme_items)
     {
        Elm_Theme_File *etf;

        EINA_INLIST_FOREACH(th->themes, etf)
          ((Elm_Theme*)th)->theme_items = eina_list_append(th->theme_items, etf->item);
     }
   return th->theme_items;
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
   if (!th) th = theme_default;
   if (!th) return;
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
   elm_theme_flush(theme_default);
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
   if (!th) th = theme_default;
   if (!th) return NULL;
   return _elm_theme_data_find(th, key);
}

EAPI const char *
elm_theme_group_path_find(Elm_Theme *th, const char *group)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(group, NULL);
   Eina_File *th_file = NULL;
   if (!th) th = theme_default;
   if (!th) return NULL;

   th_file = _elm_theme_group_file_find(th, group);
   if (th_file)
     return eina_file_filename_get(th_file);
   return NULL;
}

static Eina_List *
_elm_theme_file_group_base_list(Eina_List *list,
                                Eina_Inlist *handles,
                                const char *base, int len)
{
   Eina_Stringshare *c, *c2;
   Eina_List *coll;
   Eina_List *in, *ll;
   Elm_Theme_File *etf;

   EINA_INLIST_FOREACH(handles, etf)
     {
        coll = edje_mmap_collection_list(etf->handle);
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
   if (!th) th = theme_default;
   if (!th) return NULL;

   // XXX: look results up in a hash for speed
   len = strlen(base);
   // go through all possible theme files and find collections that match
   list = _elm_theme_file_group_base_list(NULL, th->overlay,
                                          base, len);
   list = _elm_theme_file_group_base_list(list, th->themes,
                                          base, len);
   list = _elm_theme_file_group_base_list(list, th->extension,
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

/* Allocates memory for theme and appends the theme to themes list. */
static Elm_Theme *
_elm_theme_new_internal(Eo *obj)
{
   Elm_Theme *th = calloc(1, sizeof(Elm_Theme));
   if (!th) return NULL;
   _elm_theme_file_item_add(&th->themes, "default", EINA_FALSE, EINA_TRUE);
   themes = eina_list_append(themes, th);
   th->eo_theme = obj;
   return th;
}

EOLIAN static Eo *
_efl_ui_theme_efl_object_constructor(Eo *obj, Efl_Ui_Theme_Data *pd)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_THEME_CLASS));

   pd->th = _elm_theme_new_internal(obj);

   return obj;
}

/* Removes the given theme from themes list and deallocates the given theme. */
static void
_elm_theme_free_internal(Elm_Theme *th)
{
   _elm_theme_clear(th);
   themes = eina_list_remove(themes, th);
   free(th);
}

EOLIAN static void
_efl_ui_theme_efl_object_destructor(Eo *obj, Efl_Ui_Theme_Data *pd)
{
   Eina_Bool is_theme_default = EINA_FALSE;

   if (pd->th == theme_default)
     is_theme_default = EINA_TRUE;

   _elm_theme_free_internal(pd->th);

   if (is_theme_default) theme_default = NULL;

   efl_destructor(efl_super(obj, EFL_UI_THEME_CLASS));
}

EOLIAN static Efl_Ui_Theme *
_efl_ui_theme_default_get(const Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   if (theme_default)
     return theme_default->eo_theme;

   return NULL;
}

EOLIAN static void
_efl_ui_theme_extension_add(Eo *obj EINA_UNUSED, Efl_Ui_Theme_Data *pd, const char *item)
{
   if (!item) return;
   pd->th->extension_items = eina_list_free(pd->th->extension_items);
   _elm_theme_file_item_add(&pd->th->extension, item, EINA_FALSE, EINA_FALSE);
   elm_theme_flush(pd->th);
}

EOLIAN static void
_efl_ui_theme_extension_del(Eo *obj EINA_UNUSED, Efl_Ui_Theme_Data *pd, const char *item)
{
   if (!item) return;
   pd->th->extension_items = eina_list_free(pd->th->extension_items);
   _elm_theme_file_item_del(&pd->th->extension, item);
   elm_theme_flush(pd->th);
}

EOLIAN static void
_efl_ui_theme_overlay_add(Eo *obj EINA_UNUSED, Efl_Ui_Theme_Data *pd, const char *item)
{
   if (!item) return;
   pd->th->overlay_items = eina_list_free(pd->th->overlay_items);
   _elm_theme_file_item_add(&pd->th->overlay, item, EINA_TRUE, EINA_FALSE);
   elm_theme_flush(pd->th);
}

EOLIAN static void
_efl_ui_theme_overlay_del(Eo *obj EINA_UNUSED, Efl_Ui_Theme_Data *pd, const char *item)
{
   if (!item) return;
   pd->th->overlay_items = eina_list_free(pd->th->overlay_items);
   _elm_theme_file_item_del(&pd->th->overlay, item);
   elm_theme_flush(pd->th);
}

#include "efl_ui_theme.eo.c"
