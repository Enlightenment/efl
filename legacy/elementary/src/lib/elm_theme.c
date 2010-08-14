#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Theme
 *
 * Functions to modify the theme in the currently running app.
 */

static Elm_Theme theme_default =
{
   NULL, NULL, NULL, NULL,
     NULL, 1
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
       (isalpha(f[0]) && f[1] == ':'))
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
   return NULL;
}

int
_elm_theme_object_set(Evas_Object *parent, Evas_Object *o, const char *clas, const char *group, const char *style)
{
   Elm_Theme *th = NULL;
   if (parent) th = elm_widget_theme_get(parent);
   return _elm_theme_set(th, o, clas, group, style);
}

int
_elm_theme_object_icon_set(Evas_Object *parent, Evas_Object *o, const char *group, const char *style)
{
   Elm_Theme *th = NULL;
   if (parent) th = elm_widget_theme_get(parent);
   return _elm_theme_icon_set(th, o, group, style);
}

int
_elm_theme_set(Elm_Theme *th, Evas_Object *o, const char *clas, const char *group, const char *style)
{
   const char *file;
   char buf2[1024];
   int ok;

   if (!th) th = &(theme_default);
   snprintf(buf2, sizeof(buf2), "elm/%s/%s/%s", clas, group, style);
   file = _elm_theme_group_file_find(th, buf2);
   if (file)
     {
	ok = edje_object_file_set(o, file, buf2);
	if (ok) return 1;
     }
   snprintf(buf2, sizeof(buf2), "elm/%s/%s/default", clas, group);
   file = _elm_theme_group_file_find(th, buf2);
   if (!file) return 0;
   ok = edje_object_file_set(o, file, buf2);
   return ok;
}

int
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
	if (w > 0) return 1;
     }
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/default", group);
   file = _elm_theme_group_file_find(th, buf2);
   if (!file) return 0;
   _els_smart_icon_file_edje_set(o, file, buf2);
   _els_smart_icon_size_get(o, &w, &h);
   return (w > 0);
}

int
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
             if ((*pe == ':') || (*pe == 0))
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
                  if (*pe == 0) break;
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
   return 1;
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
   th->ref--;
   if (th->ref < 1)
     {
        _elm_theme_clear(th);
        themes = eina_list_remove(themes, th);
        free(th);
     }
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
   _elm_win_rescale();
}

/**
 * This flushes all themems (default and specific ones).
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
elm_object_theme_get(Evas_Object *obj)
{
   return elm_widget_theme_get(obj);
}
