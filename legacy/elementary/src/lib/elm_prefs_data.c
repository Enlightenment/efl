#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define ELM_PREFS_DATA_CHECK(prefs_data)                        \
  do                                                            \
    {                                                           \
       EINA_SAFETY_ON_NULL_RETURN(prefs_data);                  \
       if (!EINA_MAGIC_CHECK(prefs_data, ELM_PREFS_DATA_MAGIC)) \
         {                                                      \
            EINA_MAGIC_FAIL(prefs_data, ELM_PREFS_DATA_MAGIC);  \
            return;                                             \
         }                                                      \
       EINA_SAFETY_ON_TRUE_RETURN(prefs_data->refcount <= 0);   \
    }                                                           \
  while (0)

#define ELM_PREFS_DATA_CHECK_OR_RETURN_VAL(prefs_data, retval)            \
  do                                                                      \
    {                                                                     \
       EINA_SAFETY_ON_NULL_RETURN_VAL(prefs_data, retval);                \
       if (!EINA_MAGIC_CHECK(prefs_data, ELM_PREFS_DATA_MAGIC))           \
         {                                                                \
            EINA_MAGIC_FAIL(prefs_data, ELM_PREFS_DATA_MAGIC);            \
            return retval;                                                \
         }                                                                \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(prefs_data->refcount <= 0, retval); \
    }                                                                     \
  while (0)

/* EET-bound data */

static struct
{
   Elm_Prefs_Item_Type t;
   const char         *name;
} eet_mapping[] = {
   { ELM_PREFS_TYPE_BOOL, "boolean" },
   { ELM_PREFS_TYPE_INT, "integer" },
   { ELM_PREFS_TYPE_FLOAT, "float" },
   { ELM_PREFS_TYPE_TEXT, "text" },
   { ELM_PREFS_TYPE_TEXTAREA, "text" },
   { ELM_PREFS_TYPE_DATE, "date" },
   { ELM_PREFS_TYPE_PAGE, "page" },
   { ELM_PREFS_TYPE_UNKNOWN, NULL }
};

typedef struct _Eet_Boolean_Item
{
    Eina_Bool b;
} Eet_Boolean_Item;

typedef struct _Eet_Integer_Item
{
   int i;
} Eet_Integer_Item;

typedef struct _Eet_Float_Item
{
   float f;
} Eet_Float_Item;

typedef struct _Eet_String_Item
{
   const char *s;
} Eet_String_Item;

typedef struct _Eet_Date_Item
{
   unsigned int  d;
   unsigned int  m;
   unsigned int y;
} Eet_Date_Item;

typedef struct _Eet_Data_Item
{
   Elm_Prefs_Item_Type type;

   const char         *name;

   union
   {
      /* why structs here? ask to who designed eet unions */
      Eet_Boolean_Item b;
      Eet_Integer_Item i;
      Eet_Float_Item   f;
      Eet_String_Item  s;
      Eet_Date_Item    d;
   } value;
} Eet_Data_Item;

typedef struct _Eet_Data_Value
{
   unsigned int version;
   Eina_List   *values;
} Eet_Data_Value;

/* Run time data */
typedef struct _Elm_Prefs_Data_Item
{
   Elm_Prefs_Item_Type  type;
   Eina_Value           value;
} Elm_Prefs_Data_Item;

typedef struct _Elm_Prefs_Data_Event
{
   EINA_INLIST;

   Elm_Prefs_Data_Event_Type type;

   Elm_Prefs_Data_Event_Cb   cb;
   const void               *data;

   Eina_Bool                 deleted : 1;
} Elm_Prefs_Data_Event;

struct _Elm_Prefs_Data
{
   EINA_MAGIC;

   Eina_Hash    *keys;
   unsigned int  version;

   const char   *data_file;
   const char   *key;

   Eet_File_Mode mode;
   int           refcount;

   Ecore_Poller *saving_poller;

   int           walking;

   Eina_Inlist  *event_cbs;

   Eina_List    *deleted;

   Eina_Bool     autosave : 1;
   Eina_Bool     dirty : 1;
};

static int _elm_prefs_data_init_count = 0;
static Eet_Data_Descriptor *_bool_value_edd;
static Eet_Data_Descriptor *_int_value_edd;
static Eet_Data_Descriptor *_float_value_edd;
static Eet_Data_Descriptor *_str_value_edd;
static Eet_Data_Descriptor *_date_value_edd;

static Eet_Data_Descriptor *_values_edd;
static Eet_Data_Descriptor *_item_edd;
static Eet_Data_Descriptor *_item_unified_edd;

static const char *
_union_type_get(const void *data,
                Eina_Bool *unknow)
{
   const Elm_Prefs_Item_Type *t = data;
   int i;

   if (unknow)
     *unknow = EINA_FALSE;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (*t == eet_mapping[i].t)
       return eet_mapping[i].name;

   if (unknow)
     *unknow = EINA_TRUE;

   return NULL;
}

static Eina_Bool
_union_type_set(const char *type,
                void *data,
                Eina_Bool unknow)
{
   Elm_Prefs_Item_Type *t = data;
   int i;

   if (unknow)
     return EINA_FALSE;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (strcmp(eet_mapping[i].name, type) == 0)
       {
          *t = eet_mapping[i].t;
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

static void
_data_keys_hash_free_cb(void *data)
{
   Eina_Hash *values = data;
   eina_hash_free(values);
}

static void
_data_values_hash_free_cb(void *data)
{
   Elm_Prefs_Data_Item *it = data;

   eina_value_flush(&(it->value));
   free(it);
}

static void
_eet_data_load(Elm_Prefs_Data *prefs_data,
               Eet_File *eet_file,
               const char *key)
{
   Eina_Hash      *map;
   Eet_Data_Item  *it;
   Eet_Data_Value *values = eet_data_read(eet_file, _values_edd, key);

   map = eina_hash_string_superfast_new(_data_values_hash_free_cb);
   eina_hash_set(prefs_data->keys, key, map);

   if (!values)
     {
        INF("problem while reading from file %s,"
            " starting with no data", prefs_data->data_file);
        return;
     }
   else
     prefs_data->version = values->version;

   EINA_LIST_FREE(values->values, it)
     {
        Elm_Prefs_Data_Item *item = malloc(sizeof(*item));
        Eina_Bool skip        = EINA_FALSE;
        Eina_Bool set_err     = EINA_FALSE;
        Eina_Bool setup_err   = EINA_FALSE;

        item->type = it->type;

        switch (it->type)
          {
           case ELM_PREFS_TYPE_BOOL:
             if (!eina_value_setup(&(item->value), EINA_VALUE_TYPE_UCHAR))
               setup_err = EINA_TRUE;
             else if (!eina_value_set(&(item->value), it->value.b.b))
               set_err = EINA_TRUE;
             break;

           case ELM_PREFS_TYPE_INT:
             if (!eina_value_setup(&(item->value), EINA_VALUE_TYPE_INT))
               setup_err = EINA_TRUE;
             else if (!eina_value_set(&(item->value), it->value.i.i))
               set_err = EINA_TRUE;
             break;

           case ELM_PREFS_TYPE_FLOAT:
             if (!eina_value_setup(&(item->value), EINA_VALUE_TYPE_FLOAT))
               setup_err = EINA_TRUE;
             else if (!eina_value_set(&(item->value), it->value.f.f))
               set_err = EINA_TRUE;
             break;

           case ELM_PREFS_TYPE_DATE:
           {
              struct timeval val;
              struct tm t;

              memset(&val, 0, sizeof val);
              memset(&t, 0, sizeof t);

              t.tm_year = it->value.d.y - 1900;
              t.tm_mon = it->value.d.m - 1;
              t.tm_mday = it->value.d.d;
              val.tv_sec = mktime(&t);

              if (!eina_value_setup(&(item->value), EINA_VALUE_TYPE_TIMEVAL))
                setup_err = EINA_TRUE;
              else if (!eina_value_set(&(item->value), val))
                set_err = EINA_TRUE;
           }
           break;

           case ELM_PREFS_TYPE_PAGE:
             _eet_data_load(prefs_data, eet_file, it->value.s.s);

           case ELM_PREFS_TYPE_TEXTAREA:
           case ELM_PREFS_TYPE_TEXT: /* using text type for all
                                      * text-like data */
             if (!eina_value_setup(&(item->value), EINA_VALUE_TYPE_STRINGSHARE))
               setup_err = EINA_TRUE;
             else if (!eina_value_set(&(item->value), it->value.s.s))
               set_err = EINA_TRUE;
             eina_stringshare_del(it->value.s.s);
             break;

           default:
             ERR("bad item (type = %d) fetched from data file %s, skipping it",
                 it->type, prefs_data->data_file);
             ELM_SAFE_FREE(item, free);
             skip = EINA_TRUE;
             break;
          }

        if (setup_err || set_err)
          {
             ERR("failed to set value for item %s, skipping it", it->name);
             skip = EINA_TRUE;

             if (set_err) eina_value_flush(&(item->value));
             ELM_SAFE_FREE(item, free);
          }

        if (!skip) eina_hash_set(map, it->name, item);

        eina_stringshare_del(it->name);
        free(it);
     }

   free(values);
}

EAPI Elm_Prefs_Data *
elm_prefs_data_new(const char *data_file,
                   const char *key,
                   Eet_File_Mode mode)
{
   Eet_File       *eet_file;
   Elm_Prefs_Data *prefs_data;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(mode <= EET_FILE_MODE_INVALID, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(mode > EET_FILE_MODE_READ_WRITE, NULL);

   prefs_data = calloc(1, sizeof(*prefs_data));

   EINA_MAGIC_SET(prefs_data, ELM_PREFS_DATA_MAGIC);

   prefs_data->mode = mode;

   prefs_data->refcount = 1;

   if (mode == EET_FILE_MODE_READ)
     prefs_data->autosave = EINA_FALSE;

   if (data_file) prefs_data->data_file = eina_stringshare_add(data_file);
   else
     prefs_data->data_file = eina_stringshare_printf
     ("%s/%s", efreet_config_home_get(), _elm_appname);

   prefs_data->key = eina_stringshare_add(key ? key : "main");

   prefs_data->keys = eina_hash_string_superfast_new(_data_keys_hash_free_cb);

   /* we can only start from scratch (ignore input) and (over)write on
    * save in this case, so skip input reading */
   if (mode == EET_FILE_MODE_WRITE)
     {
        INF("write-only data mode on %s starting prefs with no data",
            prefs_data->data_file);
        return prefs_data;
     }

   eet_file = eet_open(prefs_data->data_file, prefs_data->mode);
   if (!eet_file)
     {
        char bkp[PATH_MAX];

        snprintf(bkp, sizeof(bkp), "%s.bkp", prefs_data->data_file);

        WRN("failed to load from requested file, trying backup one: %s\n",
            bkp);

        eet_file = eet_open(bkp, EET_FILE_MODE_READ);
     }

   if (eet_file)
     {
        _eet_data_load(prefs_data, eet_file, prefs_data->key);
        eet_close(eet_file);
     }

   return prefs_data;
}

EAPI unsigned int
elm_prefs_data_version_get(Elm_Prefs_Data *prefs_data)
{
   ELM_PREFS_DATA_CHECK_OR_RETURN_VAL(prefs_data, -1);

   return prefs_data->version;
}

static void
_eet_data_save(const Elm_Prefs_Data *prefs_data,
               Eet_File *eet_file,
               const char *key)
{
   Eet_Data_Value edv;

   Eet_Data_Item *it;
   Elm_Prefs_Data_Item *item;

   Eina_Hash *values;
   Eina_Iterator *itr;
   Eina_Hash_Tuple *tuple;

   /* now we have to translate our prefs_data->values hash of
    * Elm_Prefs_Data_Item values into an Eet_Data_Value node, so that
    * we can serialize the latter */

   edv.version = prefs_data->version;
   edv.values = NULL;

   values = eina_hash_find(prefs_data->keys, key);

   itr = eina_hash_iterator_tuple_new(values);
   EINA_ITERATOR_FOREACH(itr, tuple)
     {
        Eina_Bool err = EINA_FALSE;

        item = (Elm_Prefs_Data_Item*) tuple->data;

        it = malloc(sizeof(*it));
        it->name = tuple->key;
        it->type = item->type;

        if (it->type == ELM_PREFS_TYPE_PAGE)
          {
             const char *n;
             if (eina_value_get(&item->value, &n))
               _eet_data_save(prefs_data, eet_file, n);
             else
               err = EINA_TRUE;
          }

        const Eina_Value_Type *t = eina_value_type_get(&(item->value));

        if (t == EINA_VALUE_TYPE_UCHAR)
          {
             if (!eina_value_get(&(item->value), &(it->value.b.b)))
               err = EINA_TRUE;
          }
        else if (t == EINA_VALUE_TYPE_INT)
          {
             if (!eina_value_get(&(item->value), &(it->value.i.i)))
               err = EINA_TRUE;
          }
        else if (t == EINA_VALUE_TYPE_FLOAT)
          {
             if (!eina_value_get(&(item->value), &(it->value.f.f)))
               err = EINA_TRUE;
          }
        else if (t == EINA_VALUE_TYPE_STRINGSHARE)
          {
             if (!eina_value_get(&(item->value), &(it->value.s.s)))
               err = EINA_TRUE;
          }
        else if (t == EINA_VALUE_TYPE_TIMEVAL)
          {
             struct timeval val;
             struct tm *tm;

             if (eina_value_get(&(item->value), &val))
               {
                  time_t gmt = val.tv_sec;

                  tm = gmtime(&gmt);

                  it->value.d.y = tm->tm_year + 1900;
                  it->value.d.m = tm->tm_mon + 1;
                  it->value.d.d = tm->tm_mday;
               }
             else
               err = EINA_TRUE;
          }
        else
          {
             ERR("bad value found on elm prefs data, skipping it");
             free(it);
             continue;
          }

        if (err)
          {
             ERR("failed to get value from %s, skipping it", it->name);
             free(it);
             continue;
          }

        edv.values = eina_list_append(edv.values, it);
     }
   eina_iterator_free(itr);

   if (!(eet_data_write(eet_file, _values_edd, key, &edv,
                        EET_COMPRESSION_DEFAULT)))
     ERR("failed to write elm prefs data!");

   EINA_LIST_FREE(edv.values, it)
      free(it);
}

static void
_elm_prefs_data_save_do(const Elm_Prefs_Data *prefs_data,
                        const char *file,
                        const char *key)
{
   char bkp[PATH_MAX];
   Eet_File *eet_file;

   snprintf(bkp, sizeof(bkp), "%s.bkp", file);

   ecore_file_unlink(bkp);
   ecore_file_mv(file, bkp);
   eet_file = eet_open(file, EET_FILE_MODE_WRITE);
   if (eet_file)
     {
        _eet_data_save(prefs_data, eet_file, key);
        eet_close(eet_file);
     }
   else
     {
        ERR("failed to open elm prefs data file to write!");
        ecore_file_mv(bkp, file);
     }
}

static void
_event_cbs_clear(Elm_Prefs_Data *prefs_data)
{
   Elm_Prefs_Data_Event *evt;

   EINA_LIST_FREE(prefs_data->deleted, evt)
     {
        prefs_data->event_cbs =
          eina_inlist_remove(prefs_data->event_cbs, EINA_INLIST_GET(evt));
        free(evt);
     }
}

static void
_elm_prefs_data_event_callback_call(Elm_Prefs_Data *prefs_data,
                                    Elm_Prefs_Data_Event_Type type,
                                    void *event_info)
{
   Elm_Prefs_Data_Event *evt;

   prefs_data->walking++;
   EINA_INLIST_FOREACH(prefs_data->event_cbs, evt)
     {
        if ((evt->type == type) && (!evt->deleted))
          evt->cb((void *)evt->data, evt->type, prefs_data, event_info);
     }
   prefs_data->walking--;

   if (!prefs_data->walking)
     _event_cbs_clear(prefs_data);
}

static Eina_Bool
_elm_prefs_data_save(void *d)
{
   Elm_Prefs_Data *prefs_data = d;

   if (!prefs_data->dirty) goto end;

   if (prefs_data->saving_poller) /* only then we are auto-saving */
     _elm_prefs_data_event_callback_call
       (prefs_data, ELM_PREFS_DATA_EVENT_GROUP_AUTOSAVED,
       (char *)prefs_data->key);

   _elm_prefs_data_save_do(prefs_data, prefs_data->data_file, prefs_data->key);

   prefs_data->dirty = EINA_FALSE;

end:
   prefs_data->saving_poller = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_elm_prefs_data_mark_as_dirty(Elm_Prefs_Data *prefs_data)
{
   prefs_data->dirty = EINA_TRUE;

   if ((prefs_data->autosave) && (prefs_data->mode != EET_FILE_MODE_READ))
     {
        if (prefs_data->saving_poller) return;

        prefs_data->saving_poller = ecore_poller_add
            (ECORE_POLLER_CORE, 1, _elm_prefs_data_save, prefs_data);
     }
}

EAPI Eina_Bool
elm_prefs_data_version_set(Elm_Prefs_Data *prefs_data,
                           unsigned int version)
{
   ELM_PREFS_DATA_CHECK_OR_RETURN_VAL(prefs_data, EINA_FALSE);

   prefs_data->version = version;

   _elm_prefs_data_mark_as_dirty(prefs_data);

   return EINA_TRUE;
}

EAPI Elm_Prefs_Data *
elm_prefs_data_ref(Elm_Prefs_Data *prefs_data)
{
   ELM_PREFS_DATA_CHECK_OR_RETURN_VAL(prefs_data, NULL);

   prefs_data->refcount++;

   return prefs_data;
}

static void
_elm_prefs_data_del(Elm_Prefs_Data *prefs_data)
{
   if (prefs_data->saving_poller) ecore_poller_del(prefs_data->saving_poller);

   if (prefs_data->mode != EET_FILE_MODE_READ)
     _elm_prefs_data_save(prefs_data);

   while (prefs_data->event_cbs)
     {
        Elm_Prefs_Data_Event *ecb = EINA_INLIST_CONTAINER_GET
            (prefs_data->event_cbs, Elm_Prefs_Data_Event);

        prefs_data->event_cbs = eina_inlist_remove
            (prefs_data->event_cbs, prefs_data->event_cbs);
        free(ecb);
     }

   eina_hash_free(prefs_data->keys);

   eina_stringshare_del(prefs_data->data_file);
   eina_stringshare_del(prefs_data->key);

   free(prefs_data);
}

EAPI void
elm_prefs_data_unref(Elm_Prefs_Data *prefs_data)
{
   ELM_PREFS_DATA_CHECK(prefs_data);

   prefs_data->refcount--;

   if (prefs_data->refcount == 0)
     _elm_prefs_data_del(prefs_data);
}

EAPI Eina_Bool
elm_prefs_data_value_set(Elm_Prefs_Data *prefs_data,
                         const char *path,
                         const Elm_Prefs_Item_Type type,
                         const Eina_Value *value)
{
   char key[PATH_MAX];
   char *name;
   size_t n;
   Eina_Value tmp;
   Eina_Hash *values;
   Elm_Prefs_Data_Item *item;
   Elm_Prefs_Data_Event_Changed evt_info;

   ELM_PREFS_DATA_CHECK_OR_RETURN_VAL(prefs_data, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, EINA_FALSE);

   /* if there is a value, try to set it to a temporary value,
    * otherwise fail immediately */
   if (value)
     {
        if (!eina_value_copy(value, &tmp))
          {
             ERR("failed to set item '%s' to value %p", path, value);
             return EINA_FALSE;
          }
     }

   name = strrchr(path, ':');
   if (!name)
     {
        ERR("invalid item path: %s", path);
        return EINA_FALSE;
     }
   else
     {
        n = name - path;
        strncpy(key, path, n);
        key[n] = '\0';
        name++;
     }

   values = eina_hash_find(prefs_data->keys, key);
   if (!values)
     {
        /* new key, must add one */
        values = eina_hash_string_superfast_new(_data_values_hash_free_cb);
        eina_hash_set(prefs_data->keys, key, values);
     }

   item = eina_hash_find(values, name);
   if (item)
     {
        eina_value_flush(&(item->value));
        if (!value) /* had an item, but should delete */
          {
             eina_hash_del_by_key(values, name);
             free(item);
             /* THERE IS NO RETURN IN HERE, we need to emit changed signal */
          }
     }
   else
     {
        if (!value) return EINA_TRUE;  /* nothing to do, success.
                                          no signal to emit */

        /* had no item, but must add one */
        item = calloc(1, sizeof(*item));
        item->type = type;
        eina_hash_set(values, name, item);
     }

   /* there's an item, but its value is already flushed or
    * uninitialized, set it from tmp memory -- this is valid and
    * correct and it works */
   if (value) memcpy(&(item->value), &tmp, sizeof(tmp));

   /* emit signal to say it was changed (value == NULL is deleted) */
   evt_info.key = path;
   evt_info.value = value;
   _elm_prefs_data_event_callback_call
     (prefs_data, ELM_PREFS_DATA_EVENT_ITEM_CHANGED, &evt_info);

   _elm_prefs_data_mark_as_dirty(prefs_data);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_prefs_data_value_get(const Elm_Prefs_Data *prefs_data,
                         const char *path,
                         Elm_Prefs_Item_Type *type,
                         Eina_Value *value)
{
   char key[PATH_MAX];
   char *name;
   size_t n;
   Eina_Hash *values;
   Elm_Prefs_Data_Item *item;

   ELM_PREFS_DATA_CHECK_OR_RETURN_VAL(prefs_data, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);

   name = strrchr(path, ':');
   if (!name)
     {
        ERR("invalid item path: %s", path);
        return EINA_FALSE;
     }
   else
     {
        n = name - path;
        strncpy(key, path, n);
        key[n] = '\0';
        name++;
     }

   values = eina_hash_find(prefs_data->keys, key);
   if (!values) return EINA_FALSE;

   item = eina_hash_find(values, name);
   if (!item) return EINA_FALSE;

   if (!eina_value_copy(&(item->value), value))
     {
        ERR("failed to fetch the value of '%s' key", name);
        return EINA_FALSE;
     }

   if (type) *type = item->type;

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_prefs_data_event_callback_add(Elm_Prefs_Data *prefs_data,
                                  Elm_Prefs_Data_Event_Type type,
                                  Elm_Prefs_Data_Event_Cb cb,
                                  const void *cb_data)
{
   Elm_Prefs_Data_Event *evt;

   ELM_PREFS_DATA_CHECK_OR_RETURN_VAL(prefs_data, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, EINA_FALSE);

   EINA_SAFETY_ON_TRUE_RETURN_VAL
     (type >= ELM_PREFS_DATA_EVENT_LAST, EINA_FALSE);

   evt = malloc(sizeof(*evt));

   evt->cb = cb;
   evt->data = cb_data;
   evt->type = type;
   evt->deleted = EINA_FALSE;

   prefs_data->event_cbs = eina_inlist_append
       (prefs_data->event_cbs, EINA_INLIST_GET(evt));

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_prefs_data_event_callback_del(Elm_Prefs_Data *prefs_data,
                                  Elm_Prefs_Data_Event_Type type,
                                  Elm_Prefs_Data_Event_Cb cb,
                                  const void *cb_data)
{
   Elm_Prefs_Data_Event *evt, *found = NULL;

   ELM_PREFS_DATA_CHECK_OR_RETURN_VAL(prefs_data, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, EINA_FALSE);

   EINA_INLIST_FOREACH(prefs_data->event_cbs, evt)
     {
        if (!evt->deleted &&
            evt->cb == cb && evt->data == cb_data && evt->type == type)
          {
             found = evt;
             break;
          }
     }

   if (!found)
     return EINA_FALSE;

   if (prefs_data->walking > 0)
     {
        found->deleted = EINA_TRUE;
        prefs_data->deleted = eina_list_append(prefs_data->deleted, found);
        return EINA_TRUE;
     }

   prefs_data->event_cbs =
     eina_inlist_remove(prefs_data->event_cbs, EINA_INLIST_GET(found));
   free(found);

   return EINA_TRUE;
}

EAPI void
elm_prefs_data_autosave_set(Elm_Prefs_Data *prefs_data,
                            Eina_Bool autosave)
{
   ELM_PREFS_DATA_CHECK(prefs_data);

   if (prefs_data->mode == EET_FILE_MODE_READ) return;

   prefs_data->autosave = !!autosave;

   if (prefs_data->autosave == autosave) return;
   if ((prefs_data->autosave) && (prefs_data->dirty))
     {
        if (prefs_data->saving_poller) return;

        prefs_data->saving_poller = ecore_poller_add
            (ECORE_POLLER_CORE, 1, _elm_prefs_data_save, prefs_data);
     }
   else if ((!prefs_data->autosave) && (prefs_data->saving_poller))
     {
        ecore_poller_del(prefs_data->saving_poller);
        prefs_data->saving_poller = NULL;

        _elm_prefs_data_save(prefs_data);
     }
}

EAPI Eina_Bool
elm_prefs_data_autosave_get(const Elm_Prefs_Data *prefs_data)
{
   ELM_PREFS_DATA_CHECK_OR_RETURN_VAL(prefs_data, EINA_FALSE);
   return prefs_data->autosave;
}

EAPI Eina_Bool
elm_prefs_data_save(const Elm_Prefs_Data *prefs_data,
                    const char *file,
                    const char *key)
{
   ELM_PREFS_DATA_CHECK_OR_RETURN_VAL(prefs_data, EINA_FALSE);

   if (!key) key = prefs_data->key;

   if (file)
     {
        _elm_prefs_data_save_do(prefs_data, file, key);
     }
   else
     {
        if (prefs_data->mode == EET_FILE_MODE_READ)
          {
             ERR("read only file %s, we can't save", prefs_data->data_file);
             return EINA_FALSE;
          }
        if (prefs_data->saving_poller)
          {
             ecore_poller_del(prefs_data->saving_poller);
             ((Elm_Prefs_Data *)prefs_data)->saving_poller = NULL;
          }
        _elm_prefs_data_save_do
           (prefs_data, prefs_data->data_file, prefs_data->key);

        /* we only clean the dirty flag if we save to our original
         * file */
        ((Elm_Prefs_Data *)prefs_data)->dirty = EINA_FALSE;
     }

   return EINA_TRUE;
}

#define DESC_NEW(_type, _desc)                             \
  EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, _type); \
  _desc = eet_data_descriptor_stream_new(&eddc)

static Eet_Data_Descriptor *
_bool_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Eet_Boolean_Item, ret);
   EET_DATA_DESCRIPTOR_ADD_BASIC(ret, Eet_Boolean_Item, "b", b, EET_T_UCHAR);

   return ret;
}

static Eet_Data_Descriptor *
_int_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Eet_Integer_Item, ret);
   EET_DATA_DESCRIPTOR_ADD_BASIC(ret, Eet_Integer_Item, "i", i, EET_T_INT);

   return ret;
}

static Eet_Data_Descriptor *
_float_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Eet_Float_Item, ret);
   EET_DATA_DESCRIPTOR_ADD_BASIC(ret, Eet_Float_Item, "f", f, EET_T_FLOAT);

   return ret;
}

static Eet_Data_Descriptor *
_str_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Eet_String_Item, ret);
   EET_DATA_DESCRIPTOR_ADD_BASIC(ret, Eet_String_Item, "s", s, EET_T_STRING);

   return ret;
}

static Eet_Data_Descriptor *
_date_desc_new(void)
{
   Eet_Data_Descriptor_Class eddc;
   Eet_Data_Descriptor *ret;

   DESC_NEW(Eet_Date_Item, ret);
   EET_DATA_DESCRIPTOR_ADD_BASIC(ret, Eet_Date_Item, "y", y, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(ret, Eet_Date_Item, "m", m, EET_T_UINT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(ret, Eet_Date_Item, "d", d, EET_T_UINT);

   return ret;
}

static void
_elm_prefs_data_descriptors_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   DESC_NEW(Eet_Data_Value, _values_edd);
   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_values_edd, Eet_Data_Value, "version", version, EET_T_UINT);

   _bool_value_edd = _bool_desc_new();
   _int_value_edd = _int_desc_new();
   _float_value_edd = _float_desc_new();
   _str_value_edd = _str_desc_new();
   _date_value_edd = _date_desc_new();

   DESC_NEW(Eet_Data_Item, _item_edd);

   EET_DATA_DESCRIPTOR_ADD_BASIC
     (_item_edd, Eet_Data_Item, "name", name, EET_T_STRING);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _union_type_get;
   eddc.func.type_set = _union_type_set;
   _item_unified_edd = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "boolean", _bool_value_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "integer", _int_value_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "float", _float_value_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "text", _str_value_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "date", _date_value_edd);
   EET_DATA_DESCRIPTOR_ADD_MAPPING
     (_item_unified_edd, "page", _str_value_edd);

   EET_DATA_DESCRIPTOR_ADD_UNION
     (_item_edd, Eet_Data_Item, "value", value, type, _item_unified_edd);

   EET_DATA_DESCRIPTOR_ADD_LIST
     (_values_edd, Eet_Data_Value, "values", values, _item_edd);
}

#undef DESC_NEW

static void
_elm_prefs_data_descriptors_shutdown(void)
{
   eet_data_descriptor_free(_int_value_edd);
   eet_data_descriptor_free(_float_value_edd);
   eet_data_descriptor_free(_str_value_edd);
   eet_data_descriptor_free(_date_value_edd);
   eet_data_descriptor_free(_values_edd);
   eet_data_descriptor_free(_item_edd);
   eet_data_descriptor_free(_item_unified_edd);
}

void
_elm_prefs_data_init(void)
{
   _elm_prefs_data_init_count++;
   if (_elm_prefs_data_init_count > 1) return;

   eina_magic_string_set(ELM_PREFS_DATA_MAGIC, "Elm_Prefs_Data");

   _elm_prefs_data_descriptors_init();
}

void
_elm_prefs_data_shutdown(void)
{
   _elm_prefs_data_init_count--;
   if (_elm_prefs_data_init_count > 0) return;

   _elm_prefs_data_descriptors_shutdown();
}
