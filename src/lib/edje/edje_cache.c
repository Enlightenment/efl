#include "edje_private.h"

Eina_Hash *_edje_file_hash = NULL;
Eina_Hash *_edje_id_hash = NULL;

static Eina_Hash *_edje_requires_pending;
static int _edje_file_cache_size = 16;
static Eina_List *_edje_file_cache = NULL;

static int _edje_collection_cache_size = 16;

EAPI void
edje_cache_emp_alloc(Edje_Part_Collection_Directory_Entry *ce)
{
   /* Init Eina Mempools this is also used in edje_pick.c */
   char *buffer;
   ce->mp = calloc(1, sizeof(Edje_Part_Collection_Directory_Entry_Mp));
   if (!ce->mp) return;

#define INIT_EMP(Tp, Sz, Ce) \
  buffer = alloca(strlen(ce->entry) + strlen(#Tp) + 2); \
  sprintf(buffer, "%s/%s", ce->entry, #Tp); \
  Ce->mp->mp.Tp = eina_mempool_add("one_big", buffer, NULL, sizeof (Sz), Ce->count.Tp); \
  _emp_##Tp = Ce->mp->mp.Tp;

#define INIT_EMP_BOTH(Tp, Sz, Ce) \
  INIT_EMP(Tp, Sz, Ce) \
  Ce->mp->mp_rtl.Tp = eina_mempool_add("one_big", buffer, NULL, \
                                       sizeof(Sz), Ce->count.Tp);

  INIT_EMP_BOTH(RECTANGLE, Edje_Part_Description_Common, ce);
  INIT_EMP_BOTH(TEXT, Edje_Part_Description_Text, ce);
  INIT_EMP_BOTH(IMAGE, Edje_Part_Description_Image, ce);
  INIT_EMP_BOTH(PROXY, Edje_Part_Description_Proxy, ce);
  INIT_EMP_BOTH(SWALLOW, Edje_Part_Description_Common, ce);
  INIT_EMP_BOTH(TEXTBLOCK, Edje_Part_Description_Text, ce);
  INIT_EMP_BOTH(GROUP, Edje_Part_Description_Common, ce);
  INIT_EMP_BOTH(BOX, Edje_Part_Description_Box, ce);
  INIT_EMP_BOTH(TABLE, Edje_Part_Description_Table, ce);
  INIT_EMP_BOTH(EXTERNAL, Edje_Part_Description_External, ce);
  INIT_EMP_BOTH(SPACER, Edje_Part_Description_Common, ce);
  INIT_EMP_BOTH(SNAPSHOT, Edje_Part_Description_Snapshot, ce);
  INIT_EMP_BOTH(MESH_NODE, Edje_Part_Description_Mesh_Node, ce);
  INIT_EMP_BOTH(LIGHT, Edje_Part_Description_Light, ce);
  INIT_EMP_BOTH(CAMERA, Edje_Part_Description_Camera, ce);
  INIT_EMP_BOTH(VECTOR, Edje_Part_Description_Vector, ce);
  INIT_EMP(part, Edje_Part, ce);
}

EAPI void
edje_cache_emp_free(Edje_Part_Collection_Directory_Entry *ce)
{  /* Free Eina Mempools this is also used in edje_pick.c */
   /* Destroy all part and description. */
   ce->ref = NULL;

   if (!ce->mp) return;
   eina_mempool_del(ce->mp->mp.RECTANGLE);
   eina_mempool_del(ce->mp->mp.TEXT);
   eina_mempool_del(ce->mp->mp.IMAGE);
   eina_mempool_del(ce->mp->mp.PROXY);
   eina_mempool_del(ce->mp->mp.SWALLOW);
   eina_mempool_del(ce->mp->mp.TEXTBLOCK);
   eina_mempool_del(ce->mp->mp.GROUP);
   eina_mempool_del(ce->mp->mp.BOX);
   eina_mempool_del(ce->mp->mp.TABLE);
   eina_mempool_del(ce->mp->mp.EXTERNAL);
   eina_mempool_del(ce->mp->mp.SPACER);
   eina_mempool_del(ce->mp->mp.SNAPSHOT);
   eina_mempool_del(ce->mp->mp.MESH_NODE);
   eina_mempool_del(ce->mp->mp.LIGHT);
   eina_mempool_del(ce->mp->mp.CAMERA);
   eina_mempool_del(ce->mp->mp.VECTOR);
   eina_mempool_del(ce->mp->mp.part);
   memset(&ce->mp->mp, 0, sizeof(ce->mp->mp));

   eina_mempool_del(ce->mp->mp_rtl.RECTANGLE);
   eina_mempool_del(ce->mp->mp_rtl.TEXT);
   eina_mempool_del(ce->mp->mp_rtl.IMAGE);
   eina_mempool_del(ce->mp->mp_rtl.PROXY);
   eina_mempool_del(ce->mp->mp_rtl.SWALLOW);
   eina_mempool_del(ce->mp->mp_rtl.TEXTBLOCK);
   eina_mempool_del(ce->mp->mp_rtl.GROUP);
   eina_mempool_del(ce->mp->mp_rtl.BOX);
   eina_mempool_del(ce->mp->mp_rtl.TABLE);
   eina_mempool_del(ce->mp->mp_rtl.EXTERNAL);
   eina_mempool_del(ce->mp->mp_rtl.SPACER);
   eina_mempool_del(ce->mp->mp_rtl.SNAPSHOT);
   eina_mempool_del(ce->mp->mp_rtl.MESH_NODE);
   eina_mempool_del(ce->mp->mp_rtl.LIGHT);
   eina_mempool_del(ce->mp->mp_rtl.CAMERA);
   eina_mempool_del(ce->mp->mp_rtl.VECTOR);
   memset(&ce->mp->mp_rtl, 0, sizeof(ce->mp->mp_rtl));

   free(ce->mp);
}

void
_edje_programs_patterns_init(Edje_Part_Collection *edc)
{
   Edje_Signals_Sources_Patterns *ssp = &edc->patterns.programs;
   Edje_Program **all;
   unsigned int i, j;

   if (ssp->signals_patterns)
     return;

   if (getenv("EDJE_DUMP_PROGRAMS"))
     {
        INF("Group '%s' programs:", edc->part);
#define EDJE_DUMP_PROGRAM(Section)                    \
  for (i = 0; i < edc->programs.Section##_count; i++) \
    INF(#Section " for ('%s', '%s')", edc->programs.Section[i]->signal, edc->programs.Section[i]->source);

        EDJE_DUMP_PROGRAM(strcmp);
        EDJE_DUMP_PROGRAM(strncmp);
        EDJE_DUMP_PROGRAM(strrncmp);
        EDJE_DUMP_PROGRAM(fnmatch);
        EDJE_DUMP_PROGRAM(nocmp);
     }

   edje_match_program_hash_build(edc->programs.strcmp,
                                 edc->programs.strcmp_count,
                                 &ssp->exact_match);

   j = edc->programs.strncmp_count
     + edc->programs.strrncmp_count
     + edc->programs.fnmatch_count
     + edc->programs.nocmp_count;
   if (j == 0) return;

   all = malloc(sizeof (Edje_Program *) * j);
   if (!all) return;
   j = 0;

   /* FIXME: Build specialized data type for each case */
#define EDJE_LOAD_PROGRAMS_ADD(Array, Edc, It, Git, All)      \
  for (It = 0; It < Edc->programs.Array##_count; ++It, ++Git) \
    All[Git] = Edc->programs.Array[It];

   EDJE_LOAD_PROGRAMS_ADD(fnmatch, edc, i, j, all);
   EDJE_LOAD_PROGRAMS_ADD(strncmp, edc, i, j, all);
   EDJE_LOAD_PROGRAMS_ADD(strrncmp, edc, i, j, all);
   /* FIXME: Do a special pass for that one */
   EDJE_LOAD_PROGRAMS_ADD(nocmp, edc, i, j, all);

   ssp->u.programs.globing = all;
   ssp->u.programs.count = j;
   ssp->signals_patterns = edje_match_programs_signal_init(all, j);
   ssp->sources_patterns = edje_match_programs_source_init(all, j);
}

static Edje_Part_Collection *
_edje_file_coll_open(Edje_File *edf, const char *coll)
{
   Edje_Part_Collection *edc = NULL;
   Edje_Part_Collection_Directory_Entry *ce;
   int id = -1, size = 0;
   unsigned int n;
   Eina_List *l;
   char buf[256];
   void *data;

   ce = eina_hash_find(edf->collection, coll);
   if (!ce) return NULL;

   if (ce->ref)
     {
        ce->ref->references++;
        return ce->ref;
     }

   EINA_LIST_FOREACH(edf->collection_cache, l, edc)
     {
        if (!strcmp(edc->part, coll))
          {
             edc->references = 1;
             ce->ref = edc;

             edf->collection_cache = eina_list_remove_list(edf->collection_cache, l);
             return ce->ref;
          }
     }

   id = ce->id;
   if (id < 0) return NULL;

   edje_cache_emp_alloc(ce);
   snprintf(buf, sizeof(buf), "edje/collections/%i", id);
   edc = eet_data_read(edf->ef, _edje_edd_edje_part_collection, buf);
   if (!edc) return NULL;

   edc->references = 1;
   edc->part = ce->entry;

   /* For Edje file build with Edje 1.0 */
   if (edf->version <= 3 && edf->minor <= 1)
     {
        /* This will preserve previous rendering */
        unsigned int i;

        /* people expect signal to not be broadcasted */
        edc->broadcast_signal = EINA_FALSE;

        /* people expect text.align to be 0.0 0.0 */
        for (i = 0; i < edc->parts_count; ++i)
          {
             if (edc->parts[i]->type == EDJE_PART_TYPE_TEXTBLOCK)
               {
                  Edje_Part_Description_Text *text;
                  unsigned int j;

                  text = (Edje_Part_Description_Text *)edc->parts[i]->default_desc;
                  text->text.align.x = TO_DOUBLE(0.0);
                  text->text.align.y = TO_DOUBLE(0.0);

                  for (j = 0; j < edc->parts[i]->other.desc_count; ++j)
                    {
                       text = (Edje_Part_Description_Text *)edc->parts[i]->other.desc[j];
                       text->text.align.x = TO_DOUBLE(0.0);
                       text->text.align.y = TO_DOUBLE(0.0);
                    }
               }
          }
     }

   snprintf(buf, sizeof(buf), "edje/scripts/embryo/compiled/%i", id);
   data = eet_read(edf->ef, buf, &size);

   if (data)
     {
        edc->script = embryo_program_new(data, size);
        _edje_embryo_script_init(edc);
        free(data);
     }

   snprintf(buf, sizeof(buf), "edje/scripts/lua/%i", id);
   data = eet_read(edf->ef, buf, &size);

   if (data)
     {
        _edje_lua2_script_load(edc, data, size);
        free(data);
     }

   ce->ref = edc;

   _edje_programs_patterns_init(edc);

   n = edc->programs.fnmatch_count +
     edc->programs.strcmp_count +
     edc->programs.strncmp_count +
     edc->programs.strrncmp_count +
     edc->programs.nocmp_count;

   if (n > 0)
     {
        Edje_Program *pr;
        unsigned int i;

        edc->patterns.table_programs = malloc(sizeof(Edje_Program *) * n);
        if (edc->patterns.table_programs)
          {
             edc->patterns.table_programs_size = n;

#define EDJE_LOAD_BUILD_TABLE(Array, Edc, It, Tmp)     \
  for (It = 0; It < Edc->programs.Array##_count; ++It) \
    {                                                  \
       Tmp = Edc->programs.Array[It];                  \
       Edc->patterns.table_programs[Tmp->id] = Tmp;    \
    }

             EDJE_LOAD_BUILD_TABLE(fnmatch, edc, i, pr);
             EDJE_LOAD_BUILD_TABLE(strcmp, edc, i, pr);
             EDJE_LOAD_BUILD_TABLE(strncmp, edc, i, pr);
             EDJE_LOAD_BUILD_TABLE(strrncmp, edc, i, pr);
             EDJE_LOAD_BUILD_TABLE(nocmp, edc, i, pr);
          }
     }

   return edc;
}

// XXX: this is not pretty. some oooooold edje files do not store strings
// in their dictionary for hashes. this works around crashes loading such
// files
extern size_t  _edje_data_string_mapping_size;
extern void   *_edje_data_string_mapping;

static Edje_File *
_edje_file_open(const Eina_File *f, int *error_ret, time_t mtime, Eina_Bool coll)
{
   Edje_Color_Tree_Node *ctn;
   Edje_Color_Class *cc;
   Edje_Text_Class *tc;
   Edje_Size_Class *sc;
   Edje_File *edf;
   Eina_List *l, *ll;
   Eet_File *ef;
   char *name;
   void *mapping;
   size_t mapping_size;

   ef = eet_mmap(f);
   if (!ef)
     {
        *error_ret = EDJE_LOAD_ERROR_UNKNOWN_FORMAT;
        return NULL;
     }
   // XXX: ancient edje file workaround
   mapping = eina_file_map_all((Eina_File *)f, EINA_FILE_SEQUENTIAL);
   if (mapping)
     {
        mapping_size = eina_file_size_get(f);
        _edje_data_string_mapping = mapping;
        _edje_data_string_mapping_size = mapping_size;
     }
   edf = eet_data_read(ef, _edje_edd_edje_file, "edje/file");
   // XXX: ancient edje file workaround
   if (mapping)
     {
        eina_file_map_free((Eina_File *)f, mapping);
        _edje_data_string_mapping = NULL;
     }
   if (!edf)
     {
        *error_ret = EDJE_LOAD_ERROR_CORRUPT_FILE;
        eet_close(ef);
        return NULL;
     }

   edf->f = eina_file_dup(f);
   edf->ef = ef;
   edf->mtime = mtime;

   if (edf->version != EDJE_FILE_VERSION)
     {
        *error_ret = EDJE_LOAD_ERROR_INCOMPATIBLE_FILE;
        _edje_file_free(edf);
        return NULL;
     }
   if (!edf->collection && coll)
     {
        *error_ret = EDJE_LOAD_ERROR_CORRUPT_FILE;
        _edje_file_free(edf);
        return NULL;
     }

   if (edf->minor > EDJE_FILE_MINOR)
     {
        WRN("`%s` may use feature from a newer edje and could not show up as expected.",
            eina_file_filename_get(f));
     }
   if (edf->base_scale <= ZERO)
     {
        edf->base_scale = FROM_INT(1);
        WRN("The base_scale can not be a 0.0. It is changed the default value(1.0)");
     }

   /* Set default efl_version if there is no version information */
   if ((edf->efl_version.major == 0) && (edf->efl_version.minor == 0))
     {
        edf->efl_version.major = 1;
        edf->efl_version.minor = 18;
     }

   edf->path = eina_stringshare_add(eina_file_filename_get(f));
   edf->references = 1;

   /* This should be done at edje generation time */
   _edje_textblock_style_parse_and_fix(edf);

   edf->color_tree_hash = eina_hash_string_small_new(NULL);
   EINA_LIST_FOREACH(edf->color_tree, l, ctn)
     EINA_LIST_FOREACH(ctn->color_classes, ll, name)
       eina_hash_add(edf->color_tree_hash, name, ctn);

   edf->color_hash = eina_hash_string_small_new(NULL);
   EINA_LIST_FOREACH(edf->color_classes, l, cc)
     if (cc->name)
       eina_hash_direct_add(edf->color_hash, cc->name, cc);

   edf->text_hash = eina_hash_string_small_new(NULL);
   EINA_LIST_FOREACH(edf->text_classes, l, tc)
      if (tc->name)
         eina_hash_direct_add(edf->text_hash, tc->name, tc);

   edf->size_hash = eina_hash_string_small_new(NULL);
   EINA_LIST_FOREACH(edf->size_classes, l, sc)
     if (sc->name)
       eina_hash_direct_add(edf->size_hash, sc->name, sc);

   if (edf->requires_count)
     {
        unsigned int i;
        Edje_File *required_edf;
        char **requires = (char**)edf->requires;

        /* EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY_STRING returns an mmapped blob, not stringshares */
        edf->requires = malloc(edf->requires_count * sizeof(char*));
        for (i = 0; i < edf->requires_count; i++)
          {
             char *str = (char*)requires[i];

             edf->requires[i] = eina_stringshare_add(str);
             required_edf = eina_hash_find(_edje_id_hash, edf->requires[i]);
             if (required_edf)
               {
                  required_edf->references++;
                  continue;
               }
             ERR("edje file '%s' REQUIRES file '%s' which is not loaded", edf->path, edf->requires[i]);
             if (!_edje_requires_pending)
               _edje_requires_pending = eina_hash_stringshared_new(NULL);
             eina_hash_list_append(_edje_requires_pending, edf->requires[i], edf);
          }
        free(requires);
     }
   if (_edje_requires_pending && edf->id)
     {
        l = eina_hash_set(_edje_requires_pending, edf->id, NULL);

        edf->references += eina_list_count(l);
        eina_list_free(l);

        if (!eina_hash_population(_edje_requires_pending))
          {
             eina_hash_free(_edje_requires_pending);
             _edje_requires_pending = NULL;
          }
     }

   return edf;
}

#if 0
// FIXME: find a way to remove dangling file earlier
static void
_edje_file_dangling(Edje_File *edf)
{
   if (edf->dangling) return;
   edf->dangling = EINA_TRUE;

   eina_hash_del(_edje_file_hash, &edf->f, edf);
   if (!eina_hash_population(_edje_file_hash))
     {
        eina_hash_free(_edje_file_hash);
        _edje_file_hash = NULL;
     }
}

#endif

Edje_File *
_edje_cache_file_coll_open(const Eina_File *file, const char *coll, int *error_ret, Edje_Part_Collection **edc_ret, Edje *ed EINA_UNUSED)
{
   Edje_File *edf;
   Eina_List *l, *hist;
   Edje_Part_Collection *edc;
   Edje_Part *ep;

   if (!_edje_id_hash)
     _edje_id_hash = eina_hash_stringshared_new(NULL);
   if (!_edje_file_hash)
     {
        _edje_file_hash = eina_hash_pointer_new(NULL);
        goto find_list;
     }

   edf = eina_hash_find(_edje_file_hash, &file);
   if (edf)
     {
        edf->references++;
        goto open;
     }

find_list:
   EINA_LIST_FOREACH(_edje_file_cache, l, edf)
     {
        if (edf->f == file)
          {
             edf->references = 1;
             _edje_file_cache = eina_list_remove_list(_edje_file_cache, l);
             eina_hash_direct_add(_edje_file_hash, &edf->f, edf);
             goto open;
          }
     }

   edf = _edje_file_open(file, error_ret, eina_file_mtime_get(file), !!coll);
   if (!edf) return NULL;

   eina_hash_direct_add(_edje_file_hash, &edf->f, edf);
   if (edf->id)
     eina_hash_list_append(_edje_id_hash, edf->id, edf);
   /* return edf; */

open:
   if (!coll)
     return edf;

   edc = _edje_file_coll_open(edf, coll);
   if (!edc)
     {
        *error_ret = EDJE_LOAD_ERROR_UNKNOWN_COLLECTION;
     }
   else
     {
        if (!edc->checked)
          {
             unsigned int j;

             for (j = 0; j < edc->parts_count; ++j)
               {
                  Edje_Part *ep2;

                  ep = edc->parts[j];

                  /* Register any color classes in this parts descriptions. */
                  hist = NULL;
                  hist = eina_list_append(hist, ep);
                  ep2 = ep;
                  while (ep2->dragable.confine_id >= 0)
                    {
                       if (ep2->dragable.confine_id >= (int)edc->parts_count)
                         {
                            ERR("confine_to above limit. invalidating it.");
                            ep2->dragable.confine_id = -1;
                            break;
                         }

                       ep2 = edc->parts[ep2->dragable.confine_id];
                       if (eina_list_data_find(hist, ep2))
                         {
                            ERR("confine_to loops. invalidating loop.");
                            ep2->dragable.confine_id = -1;
                            break;
                         }
                       hist = eina_list_append(hist, ep2);
                    }
                  eina_list_free(hist);
                  hist = NULL;
                  hist = eina_list_append(hist, ep);
                  ep2 = ep;
                  while (ep2->dragable.event_id >= 0)
                    {
                       Edje_Part *prev;

                       if (ep2->dragable.event_id >= (int)edc->parts_count)
                         {
                            ERR("event_id above limit. invalidating it.");
                            ep2->dragable.event_id = -1;
                            break;
                         }
                       prev = ep2;

                       ep2 = edc->parts[ep2->dragable.event_id];
                       /* events_to may be used only with dragable */
                       if (!ep2->dragable.x && !ep2->dragable.y)
                         {
                            prev->dragable.event_id = -1;
                            break;
                         }

                       if (eina_list_data_find(hist, ep2))
                         {
                            ERR("events_to loops. invalidating loop.");
                            ep2->dragable.event_id = -1;
                            break;
                         }
                       hist = eina_list_append(hist, ep2);
                    }
                  eina_list_free(hist);
                  hist = NULL;
                  hist = eina_list_append(hist, ep);
                  ep2 = ep;
                  while (ep2->clip_to_id >= 0)
                    {
                       if (ep2->clip_to_id >= (int)edc->parts_count)
                         {
                            ERR("clip_to_id above limit. invalidating it.");
                            ep2->clip_to_id = -1;
                            break;
                         }

                       ep2 = edc->parts[ep2->clip_to_id];
                       if (eina_list_data_find(hist, ep2))
                         {
                            ERR("clip_to loops. invalidating loop.");
                            ep2->clip_to_id = -1;
                            break;
                         }
                       hist = eina_list_append(hist, ep2);
                    }
                  eina_list_free(hist);
                  hist = NULL;
               }
             edc->checked = 1;
          }
     }

   if (edc_ret) *edc_ret = edc;

   return edf;
}

void
_edje_cache_coll_clean(Edje_File *edf)
{
   while ((edf->collection_cache) &&
          (eina_list_count(edf->collection_cache) > (unsigned int)_edje_collection_cache_size))
     {
        Edje_Part_Collection_Directory_Entry *ce;
        Edje_Part_Collection *edc;

        edc = eina_list_data_get(eina_list_last(edf->collection_cache));
        edf->collection_cache = eina_list_remove_list(edf->collection_cache, eina_list_last(edf->collection_cache));

        ce = eina_hash_find(edf->collection, edc->part);
        _edje_collection_free(edf, edc, ce);
     }
}

void
_edje_cache_coll_flush(Edje_File *edf)
{
   while (edf->collection_cache)
     {
        Edje_Part_Collection_Directory_Entry *ce;
        Edje_Part_Collection *edc;
        Eina_List *last;

        last = eina_list_last(edf->collection_cache);
        edc = eina_list_data_get(last);
        edf->collection_cache = eina_list_remove_list(edf->collection_cache,
                                                      last);

        ce = eina_hash_find(edf->collection, edc->part);
        _edje_collection_free(edf, edc, ce);
     }
}

void
_edje_cache_coll_unref(Edje_File *edf, Edje_Part_Collection *edc)
{
   Edje_Part_Collection_Directory_Entry *ce;

   edc->references--;
   if (edc->references != 0) return;

   ce = eina_hash_find(edf->collection, edc->part);
   if (!ce)
     {
        ERR("Something is wrong with reference count of '%s'.", edc->part);
     }
   else if (ce->ref)
     {
        ce->ref = NULL;

        if (edf->dangling)
          {
             /* No need to keep the collection around if the file is dangling */
             _edje_collection_free(edf, edc, ce);
             _edje_cache_coll_flush(edf);
          }
        else
          {
             edf->collection_cache = eina_list_prepend(edf->collection_cache, edc);
             _edje_cache_coll_clean(edf);
          }
     }
}

static void
_edje_cache_file_clean(void)
{
   int count;

   count = eina_list_count(_edje_file_cache);
   while ((_edje_file_cache) && (count > _edje_file_cache_size))
     {
        Eina_List *last;
        Edje_File *edf;

        last = eina_list_last(_edje_file_cache);
        edf = eina_list_data_get(last);
        _edje_file_cache = eina_list_remove_list(_edje_file_cache, last);
        _edje_file_free(edf);
        count = eina_list_count(_edje_file_cache);
     }
}

EAPI void
_edje_cache_file_unref(Edje_File *edf)
{
   edf->references--;
   if (edf->references != 0) return;

   if (edf->requires_count)
     {
        unsigned int i;

        for (i = 0; i < edf->requires_count; i++)
          {
             Edje_File *required_edf = eina_hash_find(_edje_id_hash, edf->requires[i]);

             if (required_edf)
               _edje_cache_file_unref(edf);
             else if (_edje_requires_pending)
               {
                  eina_hash_list_remove(_edje_requires_pending, edf->requires[i], edf);
                  if (!eina_hash_population(_edje_requires_pending))
                    {
                       eina_hash_free(_edje_requires_pending);
                       _edje_requires_pending = NULL;
                    }
               }
          }
     }

   if (edf->dangling)
     {
        _edje_file_free(edf);
        return;
     }

   if (edf->id)
     eina_hash_list_remove(_edje_id_hash, edf->id, edf);
   if (!eina_hash_population(_edje_id_hash))
     {
        eina_hash_free(_edje_id_hash);
        _edje_id_hash = NULL;
     }
   eina_hash_del(_edje_file_hash, &edf->f, edf);
   if (!eina_hash_population(_edje_file_hash))
     {
        eina_hash_free(_edje_file_hash);
        _edje_file_hash = NULL;
     }
   _edje_file_cache = eina_list_prepend(_edje_file_cache, edf);
   _edje_cache_file_clean();
}

void
_edje_file_cache_shutdown(void)
{
   edje_file_cache_flush();
}

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI void
edje_file_cache_set(int count)
{
   if (count < 0) count = 0;
   _edje_file_cache_size = count;
   _edje_cache_file_clean();
}

EAPI int
edje_file_cache_get(void)
{
   return _edje_file_cache_size;
}

EAPI void
edje_file_cache_flush(void)
{
   int ps;

   ps = _edje_file_cache_size;
   _edje_file_cache_size = 0;
   _edje_cache_file_clean();
   _edje_file_cache_size = ps;
}

EAPI void
edje_collection_cache_set(int count)
{
   Eina_List *l;
   Edje_File *edf;

   if (count < 0) count = 0;
   _edje_collection_cache_size = count;
   EINA_LIST_FOREACH(_edje_file_cache, l, edf)
     _edje_cache_coll_clean(edf);
   /* FIXME: freach in file hash too! */
}

EAPI int
edje_collection_cache_get(void)
{
   return _edje_collection_cache_size;
}

EAPI void
edje_collection_cache_flush(void)
{
   int ps;
   Eina_List *l;
   Edje_File *edf;

   ps = _edje_collection_cache_size;
   _edje_collection_cache_size = 0;
   EINA_LIST_FOREACH(_edje_file_cache, l, edf)
     _edje_cache_coll_flush(edf);
   /* FIXME: freach in file hash too! */
   _edje_collection_cache_size = ps;
}

