#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Theme
 *
 * Functions to modify the theme in the currently running app.
 */

static Elm_Theme theme_default =
{
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, 1
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
   snprintf(buf, sizeof(buf), "%s/.elementary/themes/%s.edj", home, f);
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

Eina_Bool
_elm_theme_object_set(Evas_Object *parent, Evas_Object *o, const char *clas, const char *group, const char *style)
{
   Elm_Theme *th = NULL;
   if (parent) th = elm_widget_theme_get(parent);
   return _elm_theme_set(th, o, clas, group, style);
}

Eina_Bool
_elm_theme_object_icon_set(Evas_Object *parent, Evas_Object *o, const char *group, const char *style)
{
   Elm_Theme *th = NULL;
   if (parent) th = elm_widget_theme_get(parent);
   return _elm_theme_icon_set(th, o, group, style);
}

Eina_Bool
_elm_theme_set(Elm_Theme *th, Evas_Object *o, const char *clas, const char *group, const char *style)
{
   const char *file;
   char buf2[1024];
   Eina_Bool ok;

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
_elm_theme_icon_set(Elm_Theme *th, Evas_Object *o, const char *group, const char *style)
{
   const char *file;
   char buf2[1024];
   int w, h;

   if (!th) th = &(theme_default);
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/%s", group, style);
   file = _elm_theme_group_file_find(th, buf2);
   if (file)
     {
        _els_smart_icon_file_edje_set(o, file, buf2);
        _els_smart_icon_size_get(o, &w, &h);
        if (w > 0) return EINA_TRUE;
     }
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/default", group);
   file = _elm_theme_group_file_find(th, buf2);
   if (!file) return EINA_FALSE;
   _els_smart_icon_file_edje_set(o, file, buf2);
   _els_smart_icon_size_get(o, &w, &h);
   return (w > 0);
}

Eina_Bool
_elm_theme_parse(Elm_Theme *th, const char *theme)
{
   Eina_List *names = NULL;
   const char *p, *pe;

   if (!th) th = &(theme_default);
   if (theme)
     {
        p = theme;
        pe = p;
        for (;;)
          {
             if ((*pe == ':') || (!*pe))
               { // p -> pe == 'name:'
                  if (pe > p)
                    {
                       char *n = malloc(pe - p + 1);
                       if (n)
                         {
                            const char *nn;

                            strncpy(n, p, pe - p);
                            n[pe - p] = 0;
                            nn = eina_stringshare_add(n);
                            if (nn) names = eina_list_append(names, nn);
                            free(n);
                         }
                    }
                  if (!*pe) break;
                  p = pe + 1;
                  pe = p;
               }
             else
               pe++;
          }
     }
   p = eina_list_data_get(eina_list_last(names));
   if ((!p) || ((p) && (strcmp(p, "default"))))
     {
        p = eina_stringshare_add("default");
        if (p) names = eina_list_append(names, p);
     }
   if (th->cache) eina_hash_free(th->cache);
   th->cache = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));

   EINA_LIST_FREE(th->themes, p) eina_stringshare_del(p);

   th->themes = names;
   return EINA_TRUE;
}

void
_elm_theme_shutdown(void)
{
   _elm_theme_clear(&(theme_default));
}

/**
 * Create a new specific theme
 *
 * This creates an empty specific theme that only uses the default theme. A
 * specific theme has its own private set of extensions and overlays too
 * (which are empty by default). Specific themes do not fall back to themes
 * of parent objects. They are not intended for this use. Use styles, overlays
 * and extensions when needed, but avoid specific themes unless there is no
 * other way (example: you want to have a preview of a new theme you are
 * selecting in a "theme selector" window. The preview is inside a scroller
 * and should display what the theme you selected will look like, but not
 * actually apply it yet. The child of the scroller will have a specific
 * theme set to show this preview before the user decides to apply it to all
 * applications).
 *
 * @ingroup Theme
 */
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

/**
 * Free a specific theme
 *
 * @param th The theme to free
 *
 * This frees a theme created with elm_theme_new().
 *
 * @ingroup Theme
 */
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

/**
 * Copy the theme fom the source to the destination theme
 *
 * @param th The source theme to copy from
 * @param thdst The destination theme to copy data to
 *
 * This makes a one-time static copy of all the theme config, extensions
 * and overlays from @p th to @p thdst. If @p th references a theme, then
 * @p thdst is also set to reference it, with all the theme settings,
 * overlays and extensions that @p th had.
 */
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

/**
 * Tell the source theme to reference the ref theme
 *
 * @param th The theme that will do the referencing
 * @param thref The theme that is the reference source
 *
 * This clears @p th to be empty and then sets it to refer to @p thref
 * so @p th acts as an override to @p thdst, but where its overrides
 * don't apply, it will fall through to @pthref for configuration.
 */
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

/**
 * Return the theme referred to
 *
 * @param th The theme to get the reference from
 * @return The referenced theme handle
 *
 * This gets the theme set as the reference theme by elm_theme_ref_set().
 * If no theme is set as a reference, NULL is returned.
 */
EAPI Elm_Theme *
elm_theme_ref_get(Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   return th->ref_theme;
}

/**
 * Return the default theme
 *
 * @return The default theme handle
 *
 * This returns the internal default theme setup handle that all widgets
 * use implicitly unless a specific theme is set. This is also often use
 * as a shorthand of NULL.
 */
EAPI Elm_Theme *
elm_theme_default_get(void)
{
   return &theme_default;
}

/**
 * Prepends a theme overlay to the list of overlays
 *
 * @param th The theme to add to, or if NULL, the default theme
 * @param item The Edje file path to be used
 *
 * Use this if your application needs to provide some custom overlay theme
 * (An Edje file that replaces some default styles of widgets) where adding
 * new styles, or changing system theme configuration is not possible. Do
 * NOT use this instead of a proper system theme configuration. Use proper
 * configuration files, profiles, environment variables etc. to set a theme
 * so that the theme can be altered by simple confiugration by a user. Using
 * this call to achieve that effect is abusing the API and will create lots
 * of trouble.
 *
 * @ingroup Theme
 */
EAPI void
elm_theme_overlay_add(Elm_Theme *th, const char *item)
{
   const char *f = eina_stringshare_add(item);

   if (!th) th = &(theme_default);
   if (f) th->overlay = eina_list_prepend(th->overlay, f);
   elm_theme_flush(th);
}

/**
 * Delete a theme overlay from the list of overlays
 *
 * @param th The theme to delete from, or if NULL, the default theme
 * @param item The name of the theme overlay
 *
 * See elm_theme_overlay_add().
 *
 * @ingroup Theme
 */
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

/**
 * Appends a theme extension to the list of extensions.
 *
 * @param th The theme to add to, or if NULL, the default theme
 * @param item The Edje file path to be used
 *
 * This is intended when an application needs more styles of widgets or new
 * widget themes that the default does not provide (or may not provide). The
 * application has "extended" usage by coming up with new custom style names
 * for widgets for specific uses, but as these are not "standard", they are
 * not guaranteed to be provided by a default theme. This means the
 * application is required to provide these extra elements itself in specific
 * Edje files. This call adds one of those Edje files to the theme search
 * path to be search after the default theme. The use of this call is
 * encouraged when default styles do not meet the needs of the application.
 * Use this call instead of elm_theme_overlay_add() for almost all cases.
 *
 * @ingroup Theme
 */
EAPI void
elm_theme_extension_add(Elm_Theme *th, const char *item)
{
   const char *f = eina_stringshare_add(item);

   if (!th) th = &(theme_default);
   if (f) th->extension = eina_list_append(th->extension, f);
   elm_theme_flush(th);
}

/**
 * Deletes a theme extension from the list of extensions.
 *
 * @param th The theme to delete from, or if NULL, the default theme
 * @param item The name of the theme extension
 *
 * See elm_theme_extension_add().
 *
 * @ingroup Theme
 */
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

/**
 * Set the theme search order for the given theme
 *
 * @param th The theme to set the search order, or if NULL, the default theme
 * @param theme Theme search string
 *
 * This sets the search string for the theme in path-notation from first
 * theme to search, to last, delimited by the : character. Example:
 *
 * "shiny:/path/to/file.edj:default"
 *
 * See the ELM_THEME environment variable for more information.
 *
 * @ingroup Theme
 */
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
}

/**
 * Return the theme search order
 *
 * @param th The theme to get the search order, or if NULL, the default theme
 * @return The internal search order path
 *
 * See elm_theme_set() for more information.
 *
 * @ingroup Theme
 */
EAPI const char *
elm_theme_get(Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   if (!th->theme)
     {
        Eina_List *l;
        const char *f;
        char *tmp;
        int len;

        len = 0;
        EINA_LIST_FOREACH(th->themes, l, f)
          {
             len += strlen(f);
             if (l->next) len += 1;
          }
        tmp = alloca(len + 1);
        tmp[0] = 0;
        EINA_LIST_FOREACH(th->themes, l, f)
          {
             strcat(tmp, f);
             if (l->next) strcat(tmp, ":");
          }
        th->theme = eina_stringshare_add(tmp);
     }
   return th->theme;
}

/**
 * Return a list of theme elements to be used in a theme.
 *
 * @param th Theme to get the list of theme elements from.
 * @return The internal list of theme elements
 *
 * This returns the internal list of theme elements (will only be valid as
 * long as the theme is not modified by elm_theme_set() or theme is not
 * freed by elm_theme_free(). This is a list of strings which must not be
 * altered as they are also internal. If @p th is NULL, then the default
 * theme element list is returned.
 */
EAPI const Eina_List *
elm_theme_list_get(const Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   return th->themes;
}

/**
 * Return the full patrh for a theme element
 *
 * @param f The theme element name
 * @param in_search_path Pointer to a boolean to indicate if item is in the search path or not
 * @return The full path to the file found.
 *
 * This returns a string you should free with free() on success, NULL on
 * failure. This will search for the given theme element, and if it is a
 * full or relative path element or a simple searchable name. The returned
 * path is the full path to the file, if searched, and the file exists, or it
 * is simply the full path given in the element or a resolved path if
 * relative to home. The @p in_search_path boolean pointed to is set to
 * EINA_TRUE if the file was a searchable file andis in the search path,
 * and EINA_FALSE otherwise.
 */
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
   snprintf(buf, sizeof(buf), "%s/.elementary/themes/%s.edj", home, f);
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

/**
 * Flush the current theme.
 *
 * @param th Theme to flush
 *
 * This flushes caches that let elementary know where to find theme elements
 * in the given theme. If @p th is NULL, then the default theme is flushed.
 * Call this call if source theme data has changed in such a way as to
 * make any caches Elementary kept invalid.
 *
 * @ingroup Theme
 */
EAPI void
elm_theme_flush(Elm_Theme *th)
{
   if (!th) th = &(theme_default);
   if (th->cache) eina_hash_free(th->cache);
   th->cache = eina_hash_string_superfast_new(EINA_FREE_CB(eina_stringshare_del));
   _elm_win_rescale(th, EINA_TRUE);
   if (th->referrers)
     {
        Eina_List *l;
        Elm_Theme *th2;

        EINA_LIST_FOREACH(th->referrers, l, th2) elm_theme_flush(th2);
     }
}

/**
 * This flushes all themes (default and specific ones).
 *
 * This will flush all themes in the current application context, by calling
 * elm_theme_flush() on each of them.
 *
 * @ingroup Theme
 */
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

/**
 * Set the theme for all elementary using applications on the current display
 *
 * @param theme The name of the theme to use. Format same as the ELM_THEME
 * environment variable.
 *
 * @ingroup Theme
 */
EAPI void
elm_theme_all_set(const char *theme)
{
#ifdef HAVE_ELEMENTARY_X
   static Ecore_X_Atom atom = 0;

   if (!atom) atom = ecore_x_atom_get("ENLIGHTENMENT_THEME");
   ecore_x_window_prop_string_set(ecore_x_window_root_first_get(),
                                  atom, theme);
#endif
   elm_theme_set(NULL, theme);
}

/**
 * Return a list of theme elements in the theme search path
 *
 * @return A list of strings that are the theme element names.
 *
 * This lists all available theme files in the standard Elementary search path
 * for theme elements, and returns them in alphabetical order as theme
 * element names in a list of strings. Free this with
 * elm_theme_name_available_list_free() when you are done with the list.
 */
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

   snprintf(buf, sizeof(buf), "%s/.elementary/themes", home);
   dir = ecore_file_ls(buf);
   EINA_LIST_FREE(dir, file)
     {
        snprintf(buf, sizeof(buf), "%s/.elementary/themes/%s", home, file);
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
                  int dup;

                  th = strdup(file);
                  s = strchr(th, '.');
                  *s = 0;
                  dup = 0;
                  EINA_LIST_FOREACH(list, l, s)
                    {
                       if (!strcmp(s, th))
                         {
                            dup = 1;
                            break;
                         }
                    }
                  if (dup) free(th);
                  else list = eina_list_append(list, th);
               }
          }
        free(file);
     }
   list = eina_list_sort(list, 0, EINA_COMPARE_CB(strcasecmp));
   return list;
}

/**
 * Free the list returned by elm_theme_name_available_list_new()
 *
 * This frees the list of themes returned by
 * elm_theme_name_available_list_new(). Once freed the list should no longer
 * be used. a new list mys be created.
 */
EAPI void
elm_theme_name_available_list_free(Eina_List *list)
{
   char *s;
   EINA_LIST_FREE(list, s) free(s);
}

/**
 * Set a specific theme to be used for this object and its children
 *
 * @param obj The object to set the theme on
 * @param th The theme to set
 *
 * This sets a specific theme that will be used for the given object and any
 * child objects it has. If @p th is NULL then the theme to be used is
 * cleared and the object will inherit its theme from its parent (which
 * ultimately will use the default theme if no specific themes are set).
 *
 * Use special themes with great care as this will annoy users and make
 * configuration difficult. Avoid any custom themes at all if it can be
 * helped.
 *
 * @ingroup Theme
 */
EAPI void
elm_object_theme_set(Evas_Object *obj, Elm_Theme *th)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_theme_set(obj, th);
}

/**
 * Get the specific theme to be used
 *
 * @param obj The object to get the specific theme from
 * @return The specifc theme set.
 *
 * This will return a specific theme set, or NULL if no specific theme is
 * set on that object. It will not return inherited themes from parents, only
 * the specific theme set for that specific object. See elm_object_theme_set()
 * for more information.
 *
 * @ingroup Theme
 */
EAPI Elm_Theme *
elm_object_theme_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_theme_get(obj);
}
