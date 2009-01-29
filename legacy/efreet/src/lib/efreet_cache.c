/* vim: set sw=4 ts=4 sts=4 et: */
#include "Efreet.h"
#include "efreet_private.h"

/* FIXME: need a mmaped hash file that apps can share */

static Eina_Hash *icon_cache = NULL;

static void
path_free(const char *path)
{
   if (path == NON_EXISTING) return;
   eina_stringshare_del(path);
}

static void
icon_cache_add(void)
{
   if (icon_cache) return;
   icon_cache = eina_hash_string_superfast_new(EINA_FREE_CB(path_free));
}

void
efreet_cache_clear(void)
{
   if (!icon_cache) return;
   IF_FREE_HASH(icon_cache);
   icon_cache = NULL;
}

const char *
efreet_icon_hash_get(const char *theme_name, const char *icon, int size)
{
   const char *file;
   char buf[4096];

   if (!icon_cache) return NULL;
   snprintf(buf, sizeof(buf), "%s/::/%s/::/%i", theme_name, icon, size);
   file = eina_hash_find(icon_cache, buf);
   return file;
}

void
efreet_icon_hash_put(const char *theme_name, const char *icon, int size, const char *file)
{
   char buf[4096];
   
   icon_cache_add();
   if (!icon_cache) return;
   snprintf(buf, sizeof(buf), "%s/::/%s/::/%i", theme_name, icon, size);
   if (file == NON_EXISTING)
     eina_hash_add(icon_cache, buf, (void *)NON_EXISTING);
   else if (file)
     eina_hash_add(icon_cache, buf, (void *)eina_stringshare_add(file));
}
