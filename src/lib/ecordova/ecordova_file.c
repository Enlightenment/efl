#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_file_private.h"

#define MY_CLASS ECORDOVA_FILE_CLASS
#define MY_CLASS_NAME "Ecordova_File"

static Eo_Base *
_ecordova_file_eo_base_constructor(Eo *obj, Ecordova_File_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_file_constructor(Eo *obj,
                           Ecordova_File_Data *pd,
                           const char *name,
                           const char *url,
                           const char *type,
                           time_t last_modified_date,
                           long size)
{
   DBG("(%p)", obj);
   pd->name = name ? strdup(name) : strdup("");
   pd->url = url ? strdup(url) : NULL;
   pd->type = type ? strdup(type) : NULL;
   pd->modified_date = last_modified_date;
   pd->size = size;

   pd->start = 0;
   pd->end = pd->size;
}

static void
_ecordova_file_eo_base_destructor(Eo *obj, Ecordova_File_Data *pd)
{
   DBG("(%p)", obj);

   free(pd->name);
   free(pd->url);
   free(pd->type);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Ecordova_File *
_ecordova_file_slice(Eo *obj EINA_UNUSED,
                     Ecordova_File_Data *pd,
                     long start,
                     long end)
{
   DBG("(%p) start=%ld end=%ld", obj, start, end);
   long size = pd->end - pd->start;
   long new_start = 0;
   long new_end = size;

   if (start < 0)
     new_start = MAX(size + start, 0);
   else
     new_start = MIN(size, start);

   if (end < 0)
     new_end = MAX(size + end, 0);
   else
     new_end = MIN(end, size);

   if (new_start > new_end)
     {
        long aux = new_start;
        new_start = new_end;
        new_end = aux;
     }

   Ecordova_File *new_file = eo_add(MY_CLASS, NULL,
     ecordova_file_constructor(pd->name,
                               pd->url,
                               pd->type,
                               pd->modified_date,
                               pd->size));
   EINA_SAFETY_ON_NULL_RETURN_VAL(new_file, NULL);

   Ecordova_File_Data *new_file_pd = eo_data_scope_get(new_file, MY_CLASS);
   EINA_SAFETY_ON_NULL_GOTO(new_file_pd, on_error);

   new_file_pd->start = pd->start + new_start;
   new_file_pd->end = pd->start + new_end;
   return new_file;

on_error:
   eo_unref(new_file);
   return NULL;
}

static const char *
_ecordova_file_name_get(Eo *obj EINA_UNUSED, Ecordova_File_Data *pd)
{
   DBG("(%p)", obj);
   return pd->name;
}

static const char *
_ecordova_file_url_get(Eo *obj EINA_UNUSED, Ecordova_File_Data *pd)
{
   DBG("(%p)", obj);
   return pd->url;
}

static long
_ecordova_file_start_get(Eo *obj EINA_UNUSED, Ecordova_File_Data *pd)
{
   DBG("(%p)", obj);
   return pd->start;
}

static long
_ecordova_file_end_get(Eo *obj EINA_UNUSED, Ecordova_File_Data *pd)
{
   DBG("(%p)", obj);
   return pd->end;
}

#include "ecordova_file.eo.c"
