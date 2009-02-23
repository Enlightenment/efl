#include <Elementary.h>
#include "elm_priv.h"

static Eina_List *themes = NULL;
static Eina_Hash *cache = NULL;

static const char *
_elm_theme_group_file_find(const char *group)
{
   Eina_List *l;
   char buf[PATH_MAX];
   char *p;
   static const char *home = NULL;
   const char *file = eina_hash_find(cache, group);
   if (file) return file;
   if (!home)
     {
        home = getenv("HOME");
        if (!home) home = "";
     }
   for (l = themes; l; l = l->next)
     {
        char *f = l->data;
        
        if ((f[0] == '/') ||
            ((f[0] == '.') && (f[1] == '/')) ||
            ((f[0] == '.') && (f[1] == '.') && (f[2] == '/')))
          {
             if (edje_file_group_exists(f, group))
               {
                  file = eina_stringshare_add(f);
                  if (file)
                    {
                       eina_hash_add(cache, group, file);
                       return file;
                    }
               }
             return NULL;
          }
        else if (((f[0] == '~') && (f[1] == '/')))
          {
             snprintf(buf, sizeof(buf), "%s/%s", home, f + 2);
             if (edje_file_group_exists(f, group))
               {
                  file = eina_stringshare_add(f);
                  if (file)
                    {
                       eina_hash_add(cache, group, file);
                       return file;
                    }
               }
             return NULL;
          }
        snprintf(buf, sizeof(buf), "%s/.elementary/themes/%s.edj", home, f);
        if (edje_file_group_exists(buf, group))
          {
             file = eina_stringshare_add(buf);
             if (file)
               {
                  eina_hash_add(cache, group, file);
                  return file;
               }
          }
        snprintf(buf, sizeof(buf), "%s/themes/%s.edj", _elm_data_dir, f);
        if (edje_file_group_exists(buf, group))
          {
             file = eina_stringshare_add(buf);
             if (file)
               {
                  eina_hash_add(cache, group, file);
                  return file;
               }
          }
     }
   return NULL;
}

int
_elm_theme_set(Evas_Object *o, const char *clas, const char *group, const char *style)
{
   const char *file;
   char buf2[1024];
   int ok;
   
   snprintf(buf2, sizeof(buf2), "elm/%s/%s/%s", clas, group, style);
   file = _elm_theme_group_file_find(buf2);
   if (file)
     {
        ok = edje_object_file_set(o, file, buf2);
        if (ok) return 1;
     }
   snprintf(buf2, sizeof(buf2), "elm/%s/%s/default", clas, group);
   file = _elm_theme_group_file_find(buf2);
   if (!file) return 0;
   ok = edje_object_file_set(o, file, buf2);
   return ok;
}

int
_elm_theme_icon_set(Evas_Object *o, const char *group, const char *style)
{
   const char *file;
   char buf2[1024];
   int w, h;
   int ok;
   
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/%s", group, style);
   file = _elm_theme_group_file_find(buf2);
   if (file)
     {
        _els_smart_icon_file_edje_set(o, file, buf2);
        _els_smart_icon_size_get(o, &w, &h);
        if (w > 0) return 1;
     }
   snprintf(buf2, sizeof(buf2), "elm/icon/%s/default", group);
   file = _elm_theme_group_file_find(buf2);
   if (!file) return 0;
   _els_smart_icon_file_edje_set(o, file, buf2);
   _els_smart_icon_size_get(o, &w, &h);
   return (w > 0);
}

static Eina_Bool
_cache_free_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata)
{
   eina_stringshare_del(data);
}

int
_elm_theme_parse(const char *theme)
{
   Eina_List *names = NULL;
   const char *p, *pe;
   
   p = theme;
   pe = p;
   for (;;)
     {
        if ((*pe == ':') || (*pe == 0))
          { // p -> pe == 'name/'
             if (pe > p)
               {
                  char *n = malloc(pe - p + 1);
                  if (n)
                    {
                       const char *nn;
                       strncpy(n, p, pe - p);
                       n[pe - p] = 0;
                       nn = eina_stringshare_add(n);
                       if (nn)
                         names = eina_list_append(names, nn);
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
   p = eina_list_data_get(eina_list_last(names));
   if ((!p) || ((p) && (strcmp(p, "default"))))
     {
        p = eina_stringshare_add("default");
        if (p)
          names = eina_list_append(names, p);
     }
   if (cache)
     {
        eina_hash_foreach(cache, _cache_free_cb, NULL);
        eina_hash_free(cache);
        cache = NULL;
     }
   cache = eina_hash_string_superfast_new(NULL);
   
   while (themes)
     {
        eina_stringshare_del(themes->data);
        themes = eina_list_remove_list(themes, themes);
     }
   
   themes = names;
   return 1;
}
