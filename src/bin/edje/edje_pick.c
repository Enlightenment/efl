#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <Ecore_Getopt.h>

#ifdef _WIN32
# include <Evil.h>
#endif

#include "edje_cc.h"
#include "edje_private.h"

#define EDJE_PICK_HELP_STRING \
  "\nEdje Pick - the \"edj\" merging tool.\n\
===================================\n\n\
Use Edje Pick to compose a single edj file \
by selecting groups from edj files.\n\n\
Use Edje Pick as follows:\n\
Include files with \'--include\' file-name\n\
Follow the included file-name by names of selected groups from this file.\n\
To select a group use: \'--group\' group-name.\n\n\
You must specify merged file name with \'--output\' file-name.\n\
Use '--verbose' switch to get detailed log.\n\n\
To produce 3rd file \'out.edj\' that composed  of:\n\
Group \'test\' from \'ex1.edj\' and \'test2\', \'test3\', from \'ex2.edj\'\n\
edje_pick -i ex1.edj -g test -i ex2.edj -g test2 -g test3 -o out.edj\n\n\
By using \'--append\' whole file content is selected.\n\
No need to specify selected groups with this switch.\n\
Note that selected group with will override group with the same name\n\
from appended-file when using \'--append\' switch.\n\n\
Example, the command:\n\
edje_pick -a theme1.edj -i default.edj -g elm/button/base/default \
-g elm/check/base/default -o hybrid.edj\n\n\
will produce a 3rd theme file \'hybrid.edj\',\n\
composed of all theme1.edj components.\n\
Replacing the button and check with widgets taken from default theme.\n\
(Given that theme1.edj button, check group-name are as in default.edj)\n"

struct _Edje_Pick_Id
{
   int       old_id;
   int       new_id;
   Eina_Bool used;
};
typedef struct _Edje_Pick_Id Edje_Pick_Id;

struct _Edje_Pick_File_Params
{
   const char *name;
   Eina_List  *groups;
   Edje_File  *edf;    /* Keeps all file data after reading  */
   Eina_Bool   append; /* Take everything from this file */

   /* We hold list of IDs for each file */
   Eina_List  *scriptlist;
   Eina_List  *luascriptlist;
   Eina_List  *imagelist;
   Eina_List  *imagesetlist; /* List of IDs (Edje_Pick_Data) for image sets */
   Eina_List  *vectorlist;
   Eina_List  *samplelist;
   Eina_List  *tonelist;
   Eina_List  *vibrationlist;
   Eina_List  *fontlist;
};
typedef struct _Edje_Pick_File_Params Edje_Pick_File_Params;

struct _Edje_Pick_Data
{
   const char  *filename; /* Image, Sample File Name       */
   void        *entry; /* used to build output file dir FIXME: REMOVE THIS */
   void        *data; /* Data as taken from input file */

   int          size;
   Edje_Pick_Id id;
};
typedef struct _Edje_Pick_Data Edje_Pick_Data;

struct _Edje_Pick_Tone
{
   Edje_Sound_Tone *tone;
   Eina_Bool        used;
};
typedef struct _Edje_Pick_Tone Edje_Pick_Tone;

struct _Edje_Pick_Font
{
   Edje_Font *f;

   void      *data; /* Font data as take from source edj file */
   int        size; /* data size */
   Eina_Bool  used;
};
typedef struct _Edje_Pick_Font Edje_Pick_Font;

struct _Edje_Pick
{
   Eina_Bool              v; /* Verbose */
   Edje_Pick_File_Params *current_file;
};
typedef struct _Edje_Pick Edje_Pick;

static Edje_Pick context = { EINA_FALSE, NULL };

#define VERBOSE(COMMAND) if (context.v) { COMMAND; }

enum _Edje_Pick_Status
{
   EDJE_PICK_NO_ERROR,
   EDJE_PICK_OUT_FILENAME_MISSING,
   EDJE_PICK_FAILED_OPEN_INP,
   EDJE_PICK_FAILED_READ_INP,
   EDJE_PICK_GROUP_NOT_FOUND,
   EDJE_PICK_IMAGE_NOT_FOUND,
   EDJE_PICK_SAMPLE_NOT_FOUND,
   EDJE_PICK_INCLUDE_MISSING,
   EDJE_PICK_GROUP_MISSING,
   EDJE_PICK_PARSE_FAILED,
   EDJE_PICK_HELP_SHOWN,
   EDJE_PICK_DUP_GROUP
};
typedef enum _Edje_Pick_Status Edje_Pick_Status;

static void
_edje_pick_args_show(Eina_List *ifs, char *out)
{  /* Print command-line arguments after parsing phase */
  Edje_Pick_File_Params *p;
  Eina_List *l;
  char *g;

  EINA_LOG_INFO("Got args for <%d> input files.\n", eina_list_count(ifs));

  EINA_LIST_FOREACH(ifs, l, p)
    {
       Eina_List *ll;

       if (p->append)
         printf("\nFile name: %s\n\tGroups: ALL (append mode)\n", p->name);
       else
         {
            printf("\nFile name: %s\n\tGroups:\n", p->name);
            EINA_LIST_FOREACH(p->groups, ll, g)
              printf("\t\t%s\n", g);
         }
    }

  EINA_LOG_INFO("\nOutput file name was <%s>\n", out);
}

static void
_edje_pick_data_free(Eina_List *l)
{
   Edje_Pick_Data *ep;

   EINA_LIST_FREE(l, ep)
     {
        if (ep->filename) eina_stringshare_del(ep->filename);
        free(ep->data);
        free(ep);
     }
}

static void
_edje_pick_out_file_free(Edje_File *out_file)
{
   if (out_file)
     {
        /* Free output file memory allocation */
        if (out_file->ef)
          eet_close(out_file->ef);

        if (out_file->external_dir)
          {
             if (out_file->external_dir->entries)
               free(out_file->external_dir->entries);

             free(out_file->external_dir);
          }

        if (out_file->image_dir)
          {
             if (out_file->image_dir->entries)
               free(out_file->image_dir->entries);

             free(out_file->image_dir);
          }

        if (out_file->sound_dir)
          {
             if (out_file->sound_dir->samples)
               free(out_file->sound_dir->samples);

             if (out_file->sound_dir->tones)
               free(out_file->sound_dir->tones);

             free(out_file->sound_dir);
          }
        if (out_file->vibration_dir)
          {
             if (out_file->vibration_dir->samples)
               free(out_file->vibration_dir->samples);

             free(out_file->vibration_dir);
          }

        eina_list_free(out_file->color_classes);
        eina_hash_free_cb_set(out_file->collection, free);
        eina_hash_free(out_file->collection);
        eina_stringshare_del(out_file->compiler);

        free(out_file);
     }
}

static int
_edje_pick_cleanup(Eina_List *ifs, Edje_File *out_file, Edje_Pick_Status s)
{
   Edje_Pick_File_Params *p;
   Edje_Pick_Font *ft;
   void *n;

   _edje_pick_out_file_free(out_file);

   EINA_LIST_FREE(ifs, p)
     {
        EINA_LIST_FREE(p->groups, n)
          eina_stringshare_del(n);

        _edje_pick_data_free(p->scriptlist);
        p->scriptlist = NULL;

        _edje_pick_data_free(p->luascriptlist);
        p->luascriptlist = NULL;

        _edje_pick_data_free(p->imagelist);
        p->imagelist = NULL;

        _edje_pick_data_free(p->vectorlist);
        p->vectorlist = NULL;

        _edje_pick_data_free(p->imagesetlist);
        p->imagesetlist = NULL;

        _edje_pick_data_free(p->samplelist);

        EINA_LIST_FREE(p->tonelist, n)
          free(n);

        if (p->edf)
          _edje_cache_file_unref(p->edf);

        EINA_LIST_FREE(p->fontlist, ft)
          {
             Edje_Font *st = ft->f;

             eina_stringshare_del(st->name);
             eina_stringshare_del(st->file);
             free(ft->data);
             free(st);
             free(ft);
          }

        free(p);
     }

   switch (s)
     {
      case EDJE_PICK_OUT_FILENAME_MISSING:
        EINA_LOG_ERR("Output file name missing.\n");
        break;

      case EDJE_PICK_FAILED_OPEN_INP:
        EINA_LOG_ERR("Failed to open input file.\n");
        break;

      case EDJE_PICK_FAILED_READ_INP:
        EINA_LOG_ERR("Failed to read input file.\n");
        break;

      case EDJE_PICK_DUP_GROUP:
        EINA_LOG_ERR("Can't fetch groups with identical name from various files.\n");
        break;

      case EDJE_PICK_INCLUDE_MISSING:
        EINA_LOG_ERR("Cannot select groups when no input file included.\n");
        break;

      case EDJE_PICK_GROUP_MISSING:
        EINA_LOG_ERR("Group name missing for include file.\n");
        break;

      case EDJE_PICK_PARSE_FAILED:
        EINA_LOG_ERR("Command parsing failed.\n");
        break;

      default:
        return s;
     }

   _edje_edd_shutdown();
   eet_shutdown();
   return s;
}

/* Look for group name in all input files that are not d1 */
static int
_group_name_in_other_file(Eina_List *inp_files, void *d1, void *d2)
{
   Edje_Pick_File_Params *inp_file = d1;
   char *group = d2; /* Group name to search */
   Eina_List *f;
   Edje_Pick_File_Params *current_file;

   EINA_LIST_FOREACH(inp_files, f, current_file)
     if (inp_file != current_file)
       if (eina_list_search_unsorted(current_file->groups,
                                     (Eina_Compare_Cb)strcmp,
                                     group))
         return 1;

   return 0;  /* Not found */
}

static int
_edje_pick_command_line_parse(int argc, char **argv,
                              Eina_List **ifs, char **ofn)
{  /* On return ifs is Input Files List, ofn is Output File Name */
  Eina_List *gpf = NULL;  /* List including counters of groups-per-file */
  Eina_List *a_files = NULL;
  Eina_List *i_files = NULL;
  Eina_List *l;
  Eina_List *ll;
  Eina_List *cg;
  Eina_List *groups = NULL;
  char *output_filename = NULL;
  Edje_Pick_File_Params *current_inp = NULL;
  Eina_List *files = NULL;   /* List of input files */
  int *c = NULL;
  char *str = NULL;
  int k;
  Eina_Bool show_help = EINA_FALSE;

  /* Define args syntax */
#define IS_GROUP(x)   ((!strcmp(x, "-g")) || (!strcmp(x, "--group")))
#define IS_INCLUDE(x) ((!strcmp(x, "-i")) || (!strcmp(x, "--include")))
#define IS_HELP(x)    ((!strcmp(x, "-h")) || (!strcmp(x, "--help")))
  static const Ecore_Getopt optdesc = {
     "edje_pick",
     NULL,
     "0.0",
     "(C) 2012 Enlightenment",
     "Public domain?",
     "Edje Pick - the \"edj\" merging tool.",

     EINA_TRUE,
     {
        ECORE_GETOPT_STORE_TRUE('v', "verbose", "Verbose"),
        ECORE_GETOPT_STORE('o', "output", "Output File",
                           ECORE_GETOPT_TYPE_STR),
        ECORE_GETOPT_APPEND_METAVAR('a', "append", "Append File",
                                    "STRING", ECORE_GETOPT_TYPE_STR),
        ECORE_GETOPT_APPEND_METAVAR('i', "include", "Include File",
                                    "STRING", ECORE_GETOPT_TYPE_STR),
        ECORE_GETOPT_APPEND_METAVAR('g', "group", "Add Group",
                                    "STRING", ECORE_GETOPT_TYPE_STR),
        ECORE_GETOPT_HELP('h', "help"),
        ECORE_GETOPT_SENTINEL
     }
  };

  Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_BOOL(context.v),
     ECORE_GETOPT_VALUE_STR(output_filename),
     ECORE_GETOPT_VALUE_LIST(a_files),
     ECORE_GETOPT_VALUE_LIST(i_files),
     ECORE_GETOPT_VALUE_LIST(groups),
     ECORE_GETOPT_VALUE_NONE
  };

  /* START - Read command line args */
  c = NULL;
  for (k = 1; k < argc; k++)
    {   /* Run through args, count how many groups per file */
      if (IS_GROUP(argv[k]))
        {
           if (!c)
             return _edje_pick_cleanup(files, NULL,
                                       EDJE_PICK_INCLUDE_MISSING);

           (*c)++;
           continue;
        }

      if (IS_INCLUDE(argv[k]))
        {
           c = calloc(1, sizeof(int));
           gpf = eina_list_append(gpf, c);
           continue;
        }

      show_help |= IS_HELP(argv[k]);
    }

  if (show_help)
    puts(EDJE_PICK_HELP_STRING);

  if (ecore_getopt_parse(&optdesc, values, argc, argv) < 0)
    {
       EINA_LIST_FREE(gpf, c)
         free(c);

       return _edje_pick_cleanup(files, NULL, EDJE_PICK_PARSE_FAILED);
    }

  if (show_help)
    {
       EINA_LIST_FREE(gpf, c)
         free(c);

       ecore_getopt_list_free(i_files);
       ecore_getopt_list_free(groups);
       return _edje_pick_cleanup(files, NULL, EDJE_PICK_HELP_SHOWN);
    }

  if (context.v) /* Changed to INFO if verbose */
    eina_log_level_set(EINA_LOG_LEVEL_INFO);

  EINA_LIST_FOREACH(a_files, l, str)
    {
       current_inp = calloc(1, sizeof(*current_inp));
       current_inp->append = EINA_TRUE;
       current_inp->name = eina_stringshare_add(str);
       files = eina_list_append(files, current_inp);
    }
  ecore_getopt_list_free(a_files);

  ll = gpf;
  cg = groups;
  EINA_LIST_FOREACH(i_files, l, str)
    {   /* Now match groups from groups-list with included files */
      current_inp = calloc(1, sizeof(*current_inp));
      current_inp->name = eina_stringshare_add(str);
      files = eina_list_append(files, current_inp);
      c = eina_list_data_get(ll);
      if (c)
        {
           while (*c)
             {
                char *g_name;
                if (!cg)
                  {
                     EINA_LIST_FREE(gpf, c)
                       free(c);

                     ecore_getopt_list_free(i_files);
                     ecore_getopt_list_free(groups);
                     return _edje_pick_cleanup(files, NULL,
                                               EDJE_PICK_GROUP_MISSING);
                  }

                g_name = eina_list_data_get(cg);
                if (_group_name_in_other_file(files, current_inp, g_name))
                  return _edje_pick_cleanup(files, NULL, EDJE_PICK_DUP_GROUP);

                if (!eina_list_search_unsorted(current_inp->groups,
                                               (Eina_Compare_Cb)strcmp, g_name))
                  current_inp->groups = eina_list_append(
                      current_inp->groups, eina_stringshare_add(g_name));

                cg = eina_list_next(cg);
                (*c)--;
             }
        }
      ll = eina_list_next(ll);
    }

  EINA_LIST_FREE(gpf, c)
    free(c);

  ecore_getopt_list_free(i_files);
  ecore_getopt_list_free(groups);

  if (!output_filename)
    return _edje_pick_cleanup(files, NULL, EDJE_PICK_OUT_FILENAME_MISSING);
  /* END   - Read command line args */

  /* Set output params, return OK */
  *ifs = files;
  *ofn = output_filename;
  return EDJE_PICK_NO_ERROR;
}

static void
_edje_pick_external_dir_update(Edje_File *o, Edje_File *edf)
{
   if (edf->external_dir && edf->external_dir->entries_count)
     {
        /* Add external-dir entries */
        unsigned int total = 0;
        unsigned int base = 0;
        Edje_External_Directory_Entry *entries;

        if (o->external_dir)
          base = total = o->external_dir->entries_count;
        else
          o->external_dir = calloc(1, sizeof(*(o->external_dir)));

        total += edf->external_dir->entries_count;

        entries = realloc(o->external_dir->entries,
                          total * sizeof(Edje_External_Directory_Entry));
        if (!entries)
          {
             EINA_LOG_ERR("Out of memory in realloc()");
             return;
          }
        o->external_dir->entries = entries;

        memcpy(&o->external_dir->entries[base], edf->external_dir->entries,
               edf->external_dir->entries_count *
               sizeof(Edje_External_Directory_Entry));

        o->external_dir->entries_count = total;
     }
}

static Edje_File *
_edje_pick_output_prepare(Edje_File *o, Edje_File *edf, char *name)
{
   /* Allocate and prepare header memory buffer */
   if (!o)
     {
        o = calloc(1, sizeof(Edje_File));
        o->compiler = eina_stringshare_add("edje_cc");
        o->version = edf->version;
        o->minor = edf->minor;
        o->feature_ver = edf->feature_ver;
        o->base_scale = edf->base_scale;
        o->collection = eina_hash_string_small_new(NULL);
        o->data = eina_hash_string_small_new(NULL);
        o->fonts = eina_hash_string_small_new(NULL);

        /* Open output file */
        o->ef = eet_open(name, EET_FILE_MODE_WRITE);
     }
   else
     {
        if (o->version != edf->version)
          {
             EINA_LOG_WARN("Warning: Merging files of various version.\n");
             if (o->version < edf->version)
               o->version = edf->version;
          }

        if (o->minor != edf->minor)
          {
             EINA_LOG_WARN("Warning: Merging files of various minor.\n");
             if (o->minor < edf->minor)
               o->minor = edf->minor;
          }

        if (o->feature_ver != edf->feature_ver)
          {
             EINA_LOG_WARN("Warning: Merging files of various feature_ver.\n");
             if (o->feature_ver < edf->feature_ver)
               o->feature_ver = edf->feature_ver;
          }
        if (NEQ(o->base_scale, edf->base_scale))
          {
             EINA_LOG_ERR("Error: Merging files of various base scale. Base scale of the files should be same.\n");
             free(o);
             return NULL;
          }
     }

   _edje_pick_external_dir_update(o, edf);
   return o;
}

static void
_edje_pick_header_alias_parent_add(Edje_File *out_file, Edje_File *edf, Edje_Part_Collection_Directory_Entry *ce, Edje_Part_Collection_Directory_Entry *ce_out)
{
   Edje_Part_Collection_Directory_Entry *ce_cor, *ce_new, *ce_f;
   Eina_Iterator *i;

   if (!ce->group_alias) return;

   i = eina_hash_iterator_data_new(edf->collection);
   EINA_ITERATOR_FOREACH(i, ce_cor)
     {
        /* find the alias parent group */
        if ((ce->id == ce_cor->id) && (!ce_cor->group_alias))
          {
             ce_f = eina_hash_find(out_file->collection, ce_cor->entry);
             if (!ce_f)
               {
                  ce_new = malloc(sizeof(*ce_new));
                  memcpy(ce_new, ce_cor, sizeof(*ce_new));
                  ce_new->id = ce_out->id;
                  eina_hash_direct_add(out_file->collection, ce_new->entry, ce_new);
               }
             else
               {
                  ce_out->id = ce_f->id;
               }
          }
     }
   eina_iterator_free(i);
}

static Eina_List *
_edje_pick_header_dependencies_check(Edje_File *out_file, Edje_File *edf, Eina_List *groups, Edje_Part_Collection_Directory_Entry *ce, int *current_id)
{
   char buf[1024];
   Edje_Part_Collection *edc;
   Edje_Part *part;
   unsigned int i, j;
   Edje_Pack_Element *item;
   Eina_List *dep_list = NULL, *dep_list_inner = NULL;

   edje_cache_emp_alloc(ce);

   snprintf(buf, sizeof(buf), "edje/collections/%i", ce->id);
   EINA_LOG_INFO("Trying to read group <%s>\n", buf);
   edc = eet_data_read(edf->ef, _edje_edd_edje_part_collection, buf);
   if (!edc)
     {
        EINA_LOG_ERR("Failed to read group <%s> id <%d>\n", ce->entry, ce->id);
        goto exit;
     }
#define GROUP_CHECK_AND_ADD(NAME)                                                                             \
  if (NAME)                                                                                                   \
    {                                                                                                         \
       Edje_Part_Collection_Directory_Entry *ce_cor;                                                          \
       if (eina_list_search_unsorted(groups, (void *)strcmp, NAME)) goto exit;                                \
       ce_cor = eina_hash_find(edf->collection, NAME);                                                        \
       if (!eina_hash_find(out_file->collection, ce_cor->entry))                                              \
         {                                                                                                    \
            dep_list_inner = _edje_pick_header_dependencies_check(out_file, edf, groups, ce_cor, current_id); \
            dep_list = eina_list_merge(dep_list, dep_list_inner);                                             \
         }                                                                                                    \
       if ((ce_cor) && (!eina_hash_find(out_file->collection, NAME)))                                         \
         {                                                                                                    \
            Edje_Part_Collection_Directory_Entry *ce_out;                                                     \
            ce_out = malloc(sizeof(*ce_out));                                                                 \
            memcpy(ce_out, ce_cor, sizeof(*ce_out));                                                          \
            ce_out->id = *current_id;                                                                         \
            if (ce_out->group_alias)                                                                          \
              _edje_pick_header_alias_parent_add(out_file, edf, ce, ce_out);                                  \
            EINA_LOG_INFO("Changing ID of group <%d> to <%d>\n", ce->id, ce_out->id);                         \
            eina_hash_direct_add(out_file->collection, ce_out->entry, ce_out);                                \
            (*current_id)++;                                                                                  \
            dep_list = eina_list_append(dep_list, eina_stringshare_add(ce_out->entry));                       \
         }                                                                                                    \
    }

   for (i = 0; i < edc->parts_count; i++)
     {
        part = edc->parts[i];
        GROUP_CHECK_AND_ADD(part->source);
        GROUP_CHECK_AND_ADD(part->source2);
        GROUP_CHECK_AND_ADD(part->source3);
        GROUP_CHECK_AND_ADD(part->source4);
        GROUP_CHECK_AND_ADD(part->source5);
        GROUP_CHECK_AND_ADD(part->source6);
        for (j = 0; j < part->items_count; j++)
          {
             item = part->items[j];
             GROUP_CHECK_AND_ADD(item->source);
          }
     }

exit:
   edje_cache_emp_free(ce);
   return dep_list;
#undef GROUP_CHECK_AND_ADD
}

static int
_edje_pick_data_update(Edje_File *out_file, Edje_File *edf)
{
   Eina_Bool status = EDJE_PICK_NO_ERROR;
   Eina_Iterator *i;
   Eina_Stringshare *key, *value;

   i = eina_hash_iterator_key_new(edf->data);
   EINA_ITERATOR_FOREACH(i, key)
     {
        if (eina_hash_find(out_file->data, key)) continue;
        value = eina_hash_find(edf->data, key);
        eina_hash_direct_add(out_file->data, key, value);
     }

   return status;
}

static int
_edje_pick_header_make(Edje_File *out_file, Edje_File *edf, Eina_List *ifs)
{
   static int current_group_id = 0;
   Edje_Part_Collection_Directory_Entry *ce;
   Eina_Bool status = EDJE_PICK_NO_ERROR;
   Eina_List *l, *alist = NULL;
   char *name1 = NULL;
   Eina_List *deps = NULL, *dep_list = NULL;
   Eina_Iterator *i;

   /* Build file header */
   if (context.current_file->append)
     {
        i = eina_hash_iterator_key_new(edf->collection);
        EINA_ITERATOR_FOREACH(i, name1)  /* Run through all keys */
          {
             Edje_Part_Collection_Directory_Entry *ce_out;

             /* Use ALL groups from this file */
             /* Test that no duplicate-group name for files in append mode */
             /* Done here because we don't read EDC before parse cmd line  */
             /* We SKIP group of file in append-mode if we got this group  */
             /* from file in include mode.                                 */
             if (_group_name_in_other_file(ifs, context.current_file, name1))
               continue;  /* Skip group of file in append mode */

             ce = eina_hash_find(edf->collection, name1);
             ce_out = malloc(sizeof(*ce_out));
             memcpy(ce_out, ce, sizeof(*ce_out));
             if (ce_out->group_alias)
               alist = eina_list_append(alist, eina_stringshare_add(name1));

             ce_out->id = current_group_id;
             current_group_id++;

             eina_hash_direct_add(out_file->collection, ce_out->entry, ce_out);

             /* Add this group to groups to handle for this file */
             context.current_file->groups = eina_list_append(
                 context.current_file->groups, eina_stringshare_add(name1));
          }

        eina_iterator_free(i);
        EINA_LIST_FOREACH(alist, l, name1)
          {
             Edje_Part_Collection_Directory_Entry *ce_cor;

             ce = eina_hash_find(edf->collection, name1);
             i = eina_hash_iterator_data_new(edf->collection);
             EINA_ITERATOR_FOREACH(i, ce_cor)
               {
                  if ((ce->id == ce_cor->id) && (!ce_cor->group_alias))
                    break;
               }
             ce = eina_hash_find(out_file->collection, ce_cor->entry);
             ce_cor = eina_hash_find(out_file->collection, name1);
             ce_cor->id = ce->id;
             eina_iterator_free(i);
          }
        EINA_LIST_FREE(alist, name1)
          eina_stringshare_del(name1);
     }
   else
     {
        EINA_LIST_FOREACH(context.current_file->groups, l, name1)
          {
             /* Verify group found then add to ouput file header */
             ce = eina_hash_find(edf->collection, name1);

             if (!ce)
               {
                  EINA_LOG_ERR("Group <%s> was not found in <%s> file.\n",
                               name1, context.current_file->name);
                  status = EDJE_PICK_GROUP_NOT_FOUND;
               }
             else
               {
                  Edje_Part_Collection_Directory_Entry *ce_out;

                  /* Add this groups to hash, with filname pefix for entries */
                  ce_out = malloc(sizeof(*ce_out));

                  memcpy(ce_out, ce, sizeof(*ce_out));
                  ce_out->id = current_group_id;
                  if (ce_out->group_alias)
                    _edje_pick_header_alias_parent_add(out_file, edf, ce, ce_out);
                  EINA_LOG_INFO("Changing ID of group <%d> to <%d>\n", ce->id, ce_out->id);
                  current_group_id++;

                  eina_hash_direct_add(out_file->collection, ce_out->entry,
                                       ce_out);
                  dep_list = _edje_pick_header_dependencies_check(out_file, edf, context.current_file->groups, ce, &current_group_id);
                  if (!deps) deps = dep_list;
                  else deps = eina_list_merge(deps, dep_list);
               }
          }
        EINA_LIST_FREE(deps, name1)
          context.current_file->groups = eina_list_append(context.current_file->groups, name1);
     }

   return status;
}

static int
_old_id_cmp(const void *d1, const void *d2)
{
   /* Find currect ID struct */
   return ((Edje_Pick_Data *)d1)->id.old_id - ((intptr_t)d2);
}

static int
_new_id_cmp(const void *d1, const void *d2)
{
   /* Find currect ID struct */
   return ((Edje_Pick_Data *)d1)->id.new_id - ((intptr_t)d2);
}

static int
_edje_pick_new_id_get(Eina_List *id_list, int id, Eina_Bool set_used)
{
   if (id >= 0)
     {
        Edje_Pick_Data *p_id = eina_list_search_unsorted(id_list,
                                                         _old_id_cmp,
                                                         (void *)(intptr_t)id);

        if (p_id)
          {
             if (set_used)
               p_id->id.used = EINA_TRUE;

             return p_id->id.new_id;
          }
     }

   return id;
}

static int
_edje_pick_image_new_id_get(Eina_List *id_list, int id)
{
   if (id >= 0)
     {
        Edje_Pick_Data *p_id = eina_list_search_unsorted(id_list,
                                                         _old_id_cmp,
                                                         (void *)(intptr_t)id);
        if (p_id) return p_id->id.new_id;
     }

   return -1;
}

static int
_edje_pick_image_old_id_get(Eina_List *id_list, int id)
{
   if (id >= 0)
     {
        Edje_Pick_Data *p_id = eina_list_search_unsorted(id_list,
                                                         _new_id_cmp,
                                                         (void *)(intptr_t)id);
        if (p_id) return p_id->id.old_id;
     }

   return -1;
}

static int
_edje_pick_images_copy(Edje_File *edf, Edje_File *o)
{
   int old_id;
   Eina_Bool status = EDJE_PICK_NO_ERROR;
   Eina_List *l, *l1;
   Edje_Pick_Data *image, *set, *vector;

   if (edf->image_dir)
     {
        if (edf->image_dir->entries) /* Copy image dir entries of current file */
          {
             Edje_Image_Directory_Entry *entries = NULL;

             EINA_LIST_FOREACH(context.current_file->imagelist, l, image)
               {
                  if (image->id.used)
                    {
                       o->image_dir->entries_count += 1;

                       /* alloc mem first time  or  re-allocate again (bigger array) */
                       entries = realloc(o->image_dir->entries,
                                         o->image_dir->entries_count *
                                         sizeof(Edje_Image_Directory_Entry));
                       if (!entries)
                         {
                            EINA_LOG_ERR("Out of memory in realloc()");
                            return EDJE_PICK_IMAGE_NOT_FOUND;
                         }
                       o->image_dir->entries = entries;

                       old_id = _edje_pick_image_old_id_get(
                           context.current_file->imagelist, o->image_dir->entries_count - 1);
                       /* Concatenate current file entries to re-allocated array */
                       memcpy(&o->image_dir->entries[o->image_dir->entries_count - 1],
                              &edf->image_dir->entries[old_id],
                              sizeof(Edje_Image_Directory_Entry));
                       o->image_dir->entries[o->image_dir->entries_count - 1].id = image->id.new_id;
                    }
               }
          }

        if (edf->image_dir->sets) /* Copy image dir sets of current file */
          {
             Edje_Image_Directory_Set *sets;

             EINA_LIST_FOREACH(context.current_file->imagesetlist, l, set)
               {
                  if (set->id.used)
                    {
                       Edje_Image_Directory_Set_Entry *e;

                       o->image_dir->sets_count += 1;
                       /* alloc mem first time  or  re-allocate again (bigger array) */
                       sets = realloc(o->image_dir->sets,
                                      o->image_dir->sets_count *
                                      sizeof(Edje_Image_Directory_Set));
                       if (!sets)
                         {
                            EINA_LOG_ERR("Out of memory in realloc()");
                            return EDJE_PICK_IMAGE_NOT_FOUND;
                         }
                       o->image_dir->sets = sets;

                       old_id = _edje_pick_image_old_id_get(
                           context.current_file->imagesetlist, o->image_dir->sets_count - 1);
                       /* Concatenate current file sets to re-allocated array */
                       memcpy(&o->image_dir->sets[o->image_dir->sets_count - 1],
                              &edf->image_dir->sets[old_id],
                              sizeof(Edje_Image_Directory_Set));
                       o->image_dir->sets[o->image_dir->sets_count - 1].id = set->id.new_id;

                       EINA_LIST_FOREACH(o->image_dir->sets[o->image_dir->sets_count - 1].entries, l1, e)
                         e->id = _edje_pick_image_new_id_get(
                             context.current_file->imagelist,
                             e->id);
                    }
               }
          }

        if (edf->image_dir->vectors) /* Copy vector dir entries of current file */
          {
             Edje_Vector_Directory_Entry *vectors = NULL;

             EINA_LIST_FOREACH(context.current_file->vectorlist, l, vector)
               {
                  if (vector->id.used)
                    {
                       o->image_dir->vectors_count += 1;

                       /* alloc mem first time  or  re-allocate again (bigger array) */
                       vectors = realloc(o->image_dir->vectors,
                                         o->image_dir->vectors_count *
                                         sizeof(Edje_Vector_Directory_Entry));
                       if (!vectors)
                         {
                            EINA_LOG_ERR("Out of memory in realloc()");
                            return EDJE_PICK_IMAGE_NOT_FOUND;
                         }
                       o->image_dir->vectors = vectors;

                       old_id = _edje_pick_image_old_id_get(
                           context.current_file->vectorlist, o->image_dir->vectors_count - 1);
                       /* Concatenate current file vectors to re-allocated array */
                       memcpy(&o->image_dir->vectors[o->image_dir->vectors_count - 1],
                              &edf->image_dir->vectors[old_id],
                              sizeof(Edje_Vector_Directory_Entry));
                       o->image_dir->vectors[o->image_dir->vectors_count - 1].id = vector->id.new_id;
                    }
               }
          }
     }

   return status;
}

static int
_edje_pick_sounds_add(Edje_File *edf)
{
   char buf[1024];
   int size, k;
   void *data;
   Eina_Bool status = EDJE_PICK_NO_ERROR;
   static int current_sample_id = 0;

   if (edf->sound_dir)  /* Copy Sounds */
     {
        for (k = 0; k < (int)edf->sound_dir->samples_count; k++)
          {
             Edje_Sound_Sample *sample = &edf->sound_dir->samples[k];

             snprintf(buf, sizeof(buf), "edje/sounds/%i", sample->id);
             VERBOSE(EINA_LOG_INFO("Trying to read <%s>\n", sample->snd_src));

             data = eet_read(edf->ef, buf, &size);
             if (size)
               {
                  Edje_Pick_Data *smpl = malloc(sizeof(*smpl));
                  smpl->filename = eina_stringshare_add(sample->snd_src);
                  smpl->data = data;
                  smpl->size = size;
                  smpl->entry = (void *)sample;  /* for output file sound dir */
                  smpl->id.old_id = sample->id;
                  sample->id = smpl->id.new_id = current_sample_id;
                  smpl->id.used = EINA_FALSE;

                  VERBOSE(EINA_LOG_INFO("Read <%s> sample data <%p> size <%d>\n",
                                        buf, smpl->data, smpl->size));

                  current_sample_id++;
                  context.current_file->samplelist =
                    eina_list_append(context.current_file->samplelist, smpl);
               }
             else
               {
                  EINA_LOG_ERR("Sample <%s> was not found in <%s> file.\n",
                               sample->name, context.current_file->name);
                  status = EDJE_PICK_SAMPLE_NOT_FOUND;
                  /* Should that really be freed? Or is some other handling needed? */
                  free(data);
               }
          }

        for (k = 0; k < (int)edf->sound_dir->tones_count; k++)
          {
             /* Save all tones as well */
             Edje_Pick_Tone *t = malloc(sizeof(*t));

             t->tone = &edf->sound_dir->tones[k];
             /* Update ID to new ID */
             t->tone->id = _edje_pick_new_id_get(context.current_file->samplelist,   /* From samplelist */
                                                 t->tone->id, EINA_FALSE);

             t->used = EINA_FALSE;
             context.current_file->tonelist = eina_list_append(context.current_file->tonelist, t);
          }
     }

   return status;
}

static int
_edje_pick_vibrations_add(Edje_File *edf)
{
   char buf[1024];
   int size, k;
   void *data;
   Eina_Bool status = EDJE_PICK_NO_ERROR;
   static int current_sample_id = 0;

   if (edf->vibration_dir)  /* Copy Sounds */
     {
        for (k = 0; k < (int)edf->vibration_dir->samples_count; k++)
          {
             Edje_Vibration_Sample *sample = &edf->vibration_dir->samples[k];

             snprintf(buf, sizeof(buf), "edje/vibrations/%i", sample->id);
             VERBOSE(EINA_LOG_INFO("Trying to read <%s>\n", sample->name));

             data = eet_read(edf->ef, buf, &size);
             if (size)
               {
                  Edje_Pick_Data *smpl = malloc(sizeof(*smpl));
                  smpl->filename = eina_stringshare_add(sample->name);
                  smpl->data = data;
                  smpl->size = size;
                  smpl->entry = (void *)sample;  /* for output file vibration dir */
                  smpl->id.old_id = sample->id;
                  sample->id = smpl->id.new_id = current_sample_id;
                  smpl->id.used = EINA_FALSE;

                  VERBOSE(EINA_LOG_INFO("Read <%s> sample data <%p> size <%d>\n",
                                        buf, smpl->data, smpl->size));

                  current_sample_id++;
                  context.current_file->vibrationlist =
                    eina_list_append(context.current_file->vibrationlist, smpl);
               }
             else
               {
                  EINA_LOG_ERR("Sample <%s> was not found in <%s> file.\n",
                               sample->name, context.current_file->name);
                  status = EDJE_PICK_SAMPLE_NOT_FOUND;
                  /* Should that really be freed? Or is some other handling needed? */
                  free(data);
               }
          }
     }

   return status;
}

static int
_font_cmp(const void *d1, const void *d2)
{
   const Edje_Font *f1 = d1;
   const Edje_Font *f2 = d2;

   /* Same font if (d1->name == d2->name) AND (d1->file == d2->file) */
   return strcmp(f1->name, f2->name) |
          strcmp(f1->file, f2->file);
}

static int
_Edje_Pick_Fonts_add(Edje_File *out_file, Edje_File *edf)
{
   Eina_Iterator *it;
   Edje_Font *f;
   char buf[1024];

   if (!edf->fonts) return -1;

   it = eina_hash_iterator_data_new(edf->fonts);
   if (!it) return -1;

   EINA_ITERATOR_FOREACH(it, f)
     {
        if (!eina_list_search_unsorted(context.current_file->fontlist, _font_cmp, f))
          continue;
        /* Add only fonts that are NOT regestered in our list */
        Edje_Pick_Font *ft = malloc(sizeof(*ft));
        Edje_Font *st = malloc(sizeof(*st));
        ft->size = 0;

        st->name = (char *)eina_stringshare_add(f->name);
        st->file = (char *)eina_stringshare_add(f->file);

        snprintf(buf, sizeof(buf), "edje/fonts/%s", f->name);
        VERBOSE(EINA_LOG_INFO("Trying to read <%s>\n", f->name));
        ft->data = eet_read(edf->ef, buf, &ft->size);
        if (!ft->size)
          {
             eina_stringshare_del(st->name);
             eina_stringshare_del(st->file);
             free(st);
             free(ft);
             VERBOSE(EINA_LOG_INFO("Enable to read <%s>. Skip.\n", f->name));
             continue;
          }

        ft->f = st;
        ft->used = EINA_TRUE;  /* TODO: Fix this later */
        context.current_file->fontlist = eina_list_append(context.current_file->fontlist, ft);
        eina_hash_direct_add(out_file->fonts, st->name, st);
     }

   eina_iterator_free(it);

   return EDJE_PICK_NO_ERROR;
}

static int
_edje_pick_scripts_add(Edje_File *edf, int id, int new_id)
{
   int size;
   void *data;
   char buf[1024];

   /* Copy Script */
   snprintf(buf, sizeof(buf), "edje/scripts/embryo/compiled/%i", id);
   data = eet_read(edf->ef, buf, &size);
   if (size)
     {
        Edje_Pick_Data *s = calloc(1, sizeof(*s));

        s->data = data;
        s->size = size;
        s->id.old_id = id;
        s->id.new_id = new_id;
        s->id.used = EINA_TRUE;

        VERBOSE(EINA_LOG_INFO("Read embryo script <%s> data <%p> size <%d>\n",
                              buf, s->data, s->size));
        context.current_file->scriptlist = eina_list_append(context.current_file->scriptlist, s);
     }
   else
     {
        /* Should that really be freed? Or is some other handling needed? */
        free(data);
     }

   return EDJE_PICK_NO_ERROR;
}

static int
_edje_pick_lua_scripts_add(Edje_File *edf, int id, int new_id)
{
   int size;
   void *data;
   char buf[1024];

   /* Copy Script */
   snprintf(buf, sizeof(buf), "edje/scripts/lua/%i", id);
   data = eet_read(edf->ef, buf, &size);
   if (size)
     {
        Edje_Pick_Data *s = calloc(1, sizeof(*s));

        s->data = data;
        s->size = size;
        s->id.old_id = id;
        s->id.new_id = new_id;
        s->id.used = EINA_TRUE;

        VERBOSE(EINA_LOG_INFO("Read lua script <%s> data <%p> size <%d>\n",
                              buf, s->data, s->size));
        context.current_file->luascriptlist = eina_list_append(context.current_file->luascriptlist, s);
     }
   else
     {
        /* Should that really be freed? Or is some other handling needed? */
        free(data);
     }

   return EDJE_PICK_NO_ERROR;
}

static int
_edje_pick_image_entry_add(int id, Edje_File *edf, Edje_File *o)
{
   char buf[1024];
   int size;
   void *data;
   static int current_img_id = -1;

   int new_id = _edje_pick_image_new_id_get(context.current_file->imagelist, id);
   if (new_id >= 0) return new_id;

   if (edf->image_dir)
     {
        if (!o->image_dir) /* First time only */
          o->image_dir = calloc(1, sizeof(*(o->image_dir)));

        if (edf->image_dir->entries) /* Copy Images */
          {
             Edje_Image_Directory_Entry *img = &edf->image_dir->entries[id];

             snprintf(buf, sizeof(buf), "edje/images/%i", img->id);
             VERBOSE(EINA_LOG_INFO("Trying to read <%s>\n", img->entry));
             data = eet_read(edf->ef, buf, &size);
             if (size) /* Advance image ID and register this in imagelist */
               {
                  current_img_id++;
                  Edje_Pick_Data *image = malloc(sizeof(*image));

                  image->filename = eina_stringshare_add(img->entry);
                  image->data = data;
                  image->size = size;
                  image->entry = (void *)img;   /* for output file image dir */
                  image->id.old_id = img->id;
                  img->id = image->id.new_id = current_img_id;
                  image->id.used = EINA_TRUE;

                  VERBOSE(EINA_LOG_INFO("Read image <%s> data <%p> size <%d>, new_id : %d\n",
                                        buf, image->data, image->size, image->id.new_id));

                  context.current_file->imagelist = eina_list_append(
                      context.current_file->imagelist, image);
               }
             else
               {
                  if (img->entry)
                    {
                       EINA_LOG_ERR("Image <%s> was not found in <%s> file.\n",
                                    img->entry, context.current_file->name);
                    }
                  else
                    {
                       EINA_LOG_ERR("Image entry <%s> was not found in <%s> file.\n", buf, context.current_file->name);
                    }
                  /* Should that really be freed? Or is some other handling needed? */
                  free(data);
               }
          }
     }

   return current_img_id;
}

static int
_edje_pick_image_set_add(int id, Edje_File *edf, Edje_File *o)
{
   static int current_set_id = -1;
   Edje_Image_Directory_Set_Entry *ent;
   Eina_List *l;

   int new_id = _edje_pick_image_new_id_get(context.current_file->imagesetlist, id);
   if (new_id >= 0) return new_id;

   if (edf->image_dir)
     {
        if (!o->image_dir) /* First time only */
          o->image_dir = calloc(1, sizeof(*(o->image_dir)));

        if (edf->image_dir->sets) /* Fix IDs in sets to new assigned IDs of entries */
          {
             current_set_id++;
             Edje_Pick_Data *set = calloc(1, sizeof(*set));
             set->id.old_id = edf->image_dir->sets[id].id;
             set->id.new_id = current_set_id;
             set->id.used = EINA_TRUE;

             /* Save IDs in set-list, used in Desc update later */
             context.current_file->imagesetlist = eina_list_append(
                 context.current_file->imagesetlist, set);
             EINA_LIST_FOREACH(edf->image_dir->sets[id].entries, l, ent)
               {
                  _edje_pick_image_entry_add(ent->id, edf, o);
               }
          }
     }

   return current_set_id;
}

static int
_edje_pick_vector_entry_add(int id, Edje_File *edf, Edje_File *o)
{
   char buf[1024];
   int size;
   void *data;
   static int current_vg_id = -1;

   int new_id = _edje_pick_image_new_id_get(context.current_file->vectorlist, id);
   if (new_id >= 0) return new_id;

   if (edf->image_dir)
     {
        if (!o->image_dir) /* First time only */
          o->image_dir = calloc(1, sizeof(*(o->image_dir)));

        if (edf->image_dir->vectors) /* Copy Vectors */
          {
             Edje_Vector_Directory_Entry *vg = &edf->image_dir->vectors[id];

             snprintf(buf, sizeof(buf), "edje/vectors/%i", vg->id);
             VERBOSE(EINA_LOG_INFO("Trying to read <%s>\n", vg->entry));
             data = eet_read(edf->ef, buf, &size);
             if (size) /* Advance vector ID and register this in vectorlist */
               {
                  current_vg_id++;
                  Edje_Pick_Data *vector = malloc(sizeof(*vector));

                  vector->filename = eina_stringshare_add(vg->entry);
                  vector->data = data;
                  vector->size = size;
                  vector->entry = (void *)vg;   /* for output file vector dir */
                  vector->id.old_id = vg->id;
                  vg->id = vector->id.new_id = current_vg_id;
                  vector->id.used = EINA_TRUE;

                  VERBOSE(EINA_LOG_INFO("Read vector <%s> data <%p> size <%d>, new_id : %d\n",
                                        buf, vector->data, vector->size, vector->id.new_id));

                  context.current_file->vectorlist = eina_list_append(
                      context.current_file->vectorlist, vector);
               }
             else
               {
                  if (vg->entry)
                    {
                       EINA_LOG_ERR("Vector <%s> was not found in <%s> file.\n",
                                    vg->entry, context.current_file->name);
                    }
                  else
                    {
                       EINA_LOG_ERR("Vector entry <%s> was not found in <%s> file.\n", buf, context.current_file->name);
                    }
                  /* Should that really be freed? Or is some other handling needed? */
                  free(data);
               }
          }
     }

   return current_vg_id;
}

static void
_edje_pick_images_desc_update(Edje_Part_Description_Image *desc, Edje_File *edf, Edje_File *o)
{
   /* Update all IDs of images in descs */
   if (desc)
     {
        unsigned int k;
        int new_id = (desc->image.set) ?
          _edje_pick_image_set_add(desc->image.id, edf, o) :
          _edje_pick_image_entry_add(desc->image.id, edf, o);

        desc->image.id = new_id;

        for (k = 0; k < desc->image.tweens_count; k++)
          {
             new_id = (desc->image.set) ?
               _edje_pick_image_set_add(desc->image.tweens[k]->id, edf, o) :
               _edje_pick_image_entry_add(desc->image.tweens[k]->id, edf, o);

             desc->image.tweens[k]->id = new_id;
          }
     }
}

static void
_edje_pick_vectors_desc_update(Edje_Part_Description_Vector *desc, Edje_File *edf, Edje_File *o)
{
   /* Update all IDs of vectors in descs */
   if (desc)
     {
        int new_id = _edje_pick_vector_entry_add(desc->vg.id, edf, o);

        desc->vg.id = new_id;
     }
}

static void
_edje_pick_styles_desc_update(Edje_Part_Description_Text *desc, Edje_File *edf, Edje_File *o)
{
   Eina_List *l;
   Edje_Style *stl;
   const char *style;

   if (desc)
     {
        style = desc->text.style.str;

        if (style)
          {
             EINA_LIST_FOREACH(edf->styles, l, stl)
               if ((stl->name) && (!strcmp(stl->name, style)))
                 o->styles = eina_list_append(o->styles, stl);
          }
     }
}

static void
_edje_pick_color_class_desc_update(Edje_Part_Description_Common *desc, Edje_File *edf, Edje_File *o)
{
   Eina_List *l;
   Edje_Color_Class *cc;
   const char *color_class;

   if (desc)
     {
        color_class = desc->color_class;

        if (color_class)
          {
             EINA_LIST_FOREACH(edf->color_classes, l, cc)
               if ((cc->name) && (!strcmp(cc->name, color_class)))
                 o->color_classes = eina_list_append(o->color_classes, cc);
          }
     }
}

static void
_edje_pick_resources_process(Edje_Part_Collection *edc, Edje_File *edf, Edje_File *o)
{
   /* Find what imagees are used, update IDs, mark as USED */
   /* Find what styles and color classes are used */
   unsigned int i, k;

   for (i = 0; i < edc->parts_count; i++)
     {
        /* Scan all parts, locate what resources used */
        Edje_Part *part = edc->parts[i];

        /* Update all color class in ALL descs of this part */
        _edje_pick_color_class_desc_update((Edje_Part_Description_Common *)part->default_desc, edf, o);

        for (k = 0; k < part->other.desc_count; k++)
          _edje_pick_color_class_desc_update((Edje_Part_Description_Common *)part->other.desc[k], edf, o);

        if (part->type == EDJE_PART_TYPE_IMAGE)
          {
             /* Update IDs of all images in ALL descs of this part */
             _edje_pick_images_desc_update((Edje_Part_Description_Image *)part->default_desc, edf, o);

             for (k = 0; k < part->other.desc_count; k++)
               _edje_pick_images_desc_update((Edje_Part_Description_Image *)part->other.desc[k], edf, o);
          }
        else if (part->type == EDJE_PART_TYPE_VECTOR)
          {
             /* Update IDs of all vectors in ALL descs of this part */
             _edje_pick_vectors_desc_update((Edje_Part_Description_Vector *)part->default_desc, edf, o);

             for (k = 0; k < part->other.desc_count; k++)
               _edje_pick_vectors_desc_update((Edje_Part_Description_Vector *)part->other.desc[k], edf, o);
          }
        else if (part->type == EDJE_PART_TYPE_TEXT || part->type == EDJE_PART_TYPE_TEXTBLOCK)
          {
             /* Update all styles in ALL descs of this part */
             _edje_pick_styles_desc_update((Edje_Part_Description_Text *)part->default_desc, edf, o);

             for (k = 0; k < part->other.desc_count; k++)
               _edje_pick_styles_desc_update((Edje_Part_Description_Text *)part->other.desc[k], edf, o);
          }
     }
}

static int
_sample_cmp(const void *d1, const void *d2)
{
   /* Locate sample by name */
   if (d2)
     {
        Edje_Sound_Sample *sample = ((Edje_Pick_Data *)d1)->entry;

        return strcmp(sample->name, d2);
     }

   return 1;
}

static int
_tone_cmp(const void *d1, const void *d2)
{
   /* Locate tone by name */
   if (d2)
     {
        Edje_Sound_Tone *tone = ((Edje_Pick_Tone *)d1)->tone;

        return strcmp(tone->name, d2);
     }

   return 1;
}

static void
_edje_pick_program_update(Edje_Program *prog)
{
   Edje_Pick_Data *p;
   Edje_Pick_Tone *t;

   /* Scan for used samples, update samples IDs */
   p = eina_list_search_unsorted(context.current_file->samplelist,
                                 (Eina_Compare_Cb)_sample_cmp,
                                 prog->sample_name);

   /* Sample is used by program, should be saved */
   if (p)
     p->id.used = EINA_TRUE;

   /* handle tones as well */
   t = eina_list_search_unsorted(context.current_file->tonelist,
                                 (Eina_Compare_Cb)_tone_cmp,
                                 prog->tone_name);

   /* Tone is used by program, should be saved */
   if (t)
     t->used = EINA_TRUE;
}

static int
_edje_pick_programs_process(Edje_Part_Collection *edc)
{
   /* This wil mark which samples are used and should be saved */
   unsigned int i;

   for (i = 0; i < edc->programs.fnmatch_count; i++)
     _edje_pick_program_update(edc->programs.fnmatch[i]);

   for (i = 0; i < edc->programs.strcmp_count; i++)
     _edje_pick_program_update(edc->programs.strcmp[i]);

   for (i = 0; i < edc->programs.strncmp_count; i++)
     _edje_pick_program_update(edc->programs.strncmp[i]);

   for (i = 0; i < edc->programs.strrncmp_count; i++)
     _edje_pick_program_update(edc->programs.strrncmp[i]);

   for (i = 0; i < edc->programs.nocmp_count; i++)
     _edje_pick_program_update(edc->programs.nocmp[i]);

   return EDJE_PICK_NO_ERROR;
}

static int
_edje_pick_collection_process(Edje_Part_Collection *edc, Edje_File *edf, Edje_File *o)
{
   /* Update all IDs, NAMES in current collection */
   Edje_Part_Collection_Directory_Entry *ce;

   ce = eina_hash_find(o->collection, edc->name);
   edc->id = ce->id;

   _edje_pick_resources_process(edc, edf, o);
   _edje_pick_programs_process(edc);

   return EDJE_PICK_NO_ERROR;
}

static void
_edje_pick_sound_dir_compose(Eina_List *samples, Eina_List *tones, Edje_File *o)
{  /* Compose sound_dir array from all used samples, tones */
  if (samples)
    {
       Edje_Sound_Sample *sample;
       Edje_Sound_Sample *p;
       Eina_List *l;

       o->sound_dir = calloc(1, sizeof(*(o->sound_dir)));
       o->sound_dir->samples = malloc(eina_list_count(samples) *
                                      sizeof(Edje_Sound_Sample));

       p = o->sound_dir->samples;
       EINA_LIST_FOREACH(samples, l, sample)
         {
            memcpy(p, sample, sizeof(Edje_Sound_Sample));
            p++;
         }

       o->sound_dir->samples_count = eina_list_count(samples);

       if (tones)
         {
            Edje_Sound_Tone *tone;
            Edje_Sound_Tone *t;

            o->sound_dir->tones = malloc(eina_list_count(tones) *
                                         sizeof(Edje_Sound_Tone));

            t = o->sound_dir->tones;
            EINA_LIST_FOREACH(tones, l, tone)
              {
                 memcpy(t, tone, sizeof(Edje_Sound_Tone));
                 t++;
              }

            o->sound_dir->tones_count = eina_list_count(tones);
         }
    }
}

static void
_edje_pick_vibration_dir_compose(Eina_List *vibrations, Edje_File *o)
{  /* Compose vibrationdir array from all used vibrations */
  if (vibrations)
    {
       Edje_Vibration_Sample *sample;
       Edje_Vibration_Sample *p;
       Eina_List *l;

       o->vibration_dir = calloc(1, sizeof(*(o->vibration_dir)));
       o->vibration_dir->samples = malloc(eina_list_count(vibrations) *
                                          sizeof(Edje_Vibration_Sample));

       p = o->vibration_dir->samples;
       EINA_LIST_FOREACH(vibrations, l, sample)
         {
            memcpy(p, sample, sizeof(Edje_Vibration_Sample));
            p++;
         }

       o->vibration_dir->samples_count = eina_list_count(vibrations);
    }
}

int
main(int argc, char **argv)
{
   char *name1, *output_filename = NULL;
   Eina_List *inp_files = NULL;
   int comp_mode = EET_COMPRESSION_DEFAULT;
   Edje_File *out_file = NULL;
   Eina_List *images = NULL;
   Eina_List *samples = NULL;
   Eina_List *tones = NULL;
   Eina_List *vibrations = NULL;

   Edje_Part_Collection *edc;
   Edje_Part_Collection_Directory_Entry *ce;
   Eet_File *ef;
   Eina_List *f, *l;
   char buf[1024];
   int k, bytes;

   ecore_app_no_system_modules();

   eina_init();
   eet_init();
   ecore_init();
   _edje_edd_init();
   eina_log_level_set(EINA_LOG_LEVEL_WARN);  /* Changed to INFO if verbose */

   k = _edje_pick_command_line_parse(argc, argv, &inp_files, &output_filename);
   if ( k != EDJE_PICK_NO_ERROR)
     return k;

   _edje_pick_args_show(inp_files, output_filename);

   /* START - Main loop scanning input files */
   EINA_LIST_FOREACH(inp_files, f, context.current_file)
     {
        Edje_File *edf;

        ef = eet_open(context.current_file->name, EET_FILE_MODE_READ);
        if (!ef)
          return _edje_pick_cleanup(inp_files, out_file,
                                    EDJE_PICK_FAILED_OPEN_INP);

        edf = eet_data_read(ef, _edje_edd_edje_file, "edje/file");
        if (!edf)
          return _edje_pick_cleanup(inp_files, out_file,
                                    EDJE_PICK_FAILED_READ_INP);

        context.current_file->edf = edf;
        edf->ef = ef;

        out_file = _edje_pick_output_prepare(out_file, edf, output_filename);
        if (!out_file)
          {
             _edje_cache_file_unref(edf);
             eet_close(ef);
             return _edje_pick_cleanup(inp_files, out_file, k);
          }

        k = _edje_pick_header_make(out_file, edf, inp_files);
        if (k != EDJE_PICK_NO_ERROR)
          {
             _edje_cache_file_unref(edf);
             eet_close(ef);
             return _edje_pick_cleanup(inp_files, out_file, k);
          }

        /* Build lists of all samples and fonts of input files    */
        _edje_pick_sounds_add(edf);  /* Add Sounds to samplelist          */
        _edje_pick_vibrations_add(edf);  /* Add Vibrations to samplelist  */

        _edje_pick_data_update(out_file, edf);

        /* Process Groups */
        EINA_LIST_FOREACH(context.current_file->groups, l, name1)
          {  /* Read group info */
            ce = eina_hash_find(edf->collection, name1);
            if (!ce || (ce->id < 0))
              {
                 EINA_LOG_ERR("Failed to find group <%s> id\n", name1);
                 _edje_cache_file_unref(edf);
                 return _edje_pick_cleanup(inp_files, out_file,
                                           EDJE_PICK_GROUP_NOT_FOUND);
              }

            VERBOSE(EINA_LOG_INFO("Copy group: <%s>\n", name1));

            edje_cache_emp_alloc(ce);

            snprintf(buf, sizeof(buf), "edje/collections/%i", ce->id);
            EINA_LOG_INFO("Trying to read group <%s>\n", buf);
            edc = eet_data_read(edf->ef, _edje_edd_edje_part_collection, buf);
            if (!edc)
              {
                 EINA_LOG_ERR("Failed to read group <%s> id <%d>\n", name1, ce->id);
                 _edje_cache_file_unref(edf);
                 return _edje_pick_cleanup(inp_files, out_file,
                                           EDJE_PICK_GROUP_NOT_FOUND);
              }

            /* Update IDs */
            _edje_pick_collection_process(edc, edf, out_file);

            /* Build lists of all scripts with new IDs */
            _edje_pick_scripts_add(edf, ce->id, edc->id);
            _edje_pick_lua_scripts_add(edf, ce->id, edc->id);

            {
               /* Write the group to output file using new id */
               snprintf(buf, sizeof(buf),
                        "edje/collections/%i", edc->id);
               bytes = eet_data_write(out_file->ef,
                                      _edje_edd_edje_part_collection,
                                      buf, edc, comp_mode);
               EINA_LOG_INFO("Wrote <%d> bytes for group <%s>\n", bytes, buf);
            }

            free(edc);
            edje_cache_emp_free(ce);
            eet_close(ef);
          }

        _edje_pick_images_copy(edf, out_file);  /* Add Images to imagelist */
        _edje_cache_file_unref(edf);
        _Edje_Pick_Fonts_add(out_file, edf);   /* Add fonts from file to fonts list */

        /* We SKIP writing source, just can't compose it */
        /* FIXME: use Edje_Edit code to generate source */
     } /* END   - Main loop scanning input files */

   /* Write rest of output */

   EINA_LIST_FOREACH(inp_files, f, context.current_file)
     {
        /* Write Scripts from ALL files */
        Edje_Pick_Data *s;
        Edje_Pick_Tone *tn;
        Edje_Pick_Font *fn;
        Eina_List *t;

        EINA_LIST_FOREACH(context.current_file->scriptlist, t, s)
          {
             /* Write Scripts */
             snprintf(buf, sizeof(buf),
                      "edje/scripts/embryo/compiled/%i", s->id.new_id);
             VERBOSE(EINA_LOG_INFO("wrote embryo scr <%s> data <%p> size <%d>\n",
                                   buf, s->data, s->size));
             eet_write(out_file->ef, buf, s->data, s->size, comp_mode);
          }

        EINA_LIST_FOREACH(context.current_file->luascriptlist, t, s)
          {
             /* Write Lua Scripts */
             snprintf(buf, sizeof(buf),
                      "edje/scripts/lua/%i", s->id.new_id);
             VERBOSE(EINA_LOG_INFO("wrote lua scr <%s> data <%p> size <%d>\n",
                                   buf, s->data, s->size));
             eet_write(out_file->ef, buf, s->data, s->size, comp_mode);
          }

        EINA_LIST_FOREACH(context.current_file->imagelist, t, s)
          {
             if (context.current_file->append || s->id.used)
               {
                  snprintf(buf, sizeof(buf), "edje/images/%i", s->id.new_id);
                  eet_write(out_file->ef, buf, s->data, s->size, EINA_TRUE);
                  VERBOSE(EINA_LOG_INFO("Wrote <%s> image data <%p> size <%d>\n", buf, s->data, s->size));
               }
          }

        EINA_LIST_FOREACH(context.current_file->vectorlist, t, s)
          {
             if (context.current_file->append || s->id.used)
               {
                  snprintf(buf, sizeof(buf), "edje/vectors/%i", s->id.new_id);
                  eet_write(out_file->ef, buf, s->data, s->size, EINA_TRUE);
                  VERBOSE(EINA_LOG_INFO("Wrote <%s> vector data <%p> size <%d>\n", buf, s->data, s->size));
               }
          }

        EINA_LIST_FOREACH(context.current_file->fontlist, t, fn)
          {
             if (context.current_file->append || fn->used)
               {
                  snprintf(buf, sizeof(buf), "edje/fonts/%s", fn->f->name);
                  eet_write(out_file->ef, buf, fn->data, fn->size, EINA_TRUE);
                  VERBOSE(EINA_LOG_INFO("Wrote <%s> fonts data <%p> size <%d>\n", buf, fn->data, fn->size));
               }
          }

        EINA_LIST_FOREACH(context.current_file->samplelist, l, s)
          {
             if (context.current_file->append || s->id.used) /* Write only used samples */
               {
                  samples = eina_list_append(samples, s->entry);

                  snprintf(buf, sizeof(buf), "edje/sounds/%i",
                           s->id.new_id);
                  eet_write(out_file->ef, buf,
                            s->data, s->size, EET_COMPRESSION_NONE);
                  VERBOSE(EINA_LOG_INFO("Wrote <%s> sample data <%p> size <%d>\n",
                                        buf, s->data, s->size));
               }
          }

        EINA_LIST_FOREACH(context.current_file->tonelist, l, tn)
          {
             if (context.current_file->append || tn->used)
               tones = eina_list_append(tones, tn->tone);
          }

        EINA_LIST_FOREACH(context.current_file->vibrationlist, l, s)
          {
             if (context.current_file->append || s->id.used) /* Write only used vibrations */
               {
                  vibrations = eina_list_append(vibrations, s->entry);

                  snprintf(buf, sizeof(buf), "edje/vibrations/%i",
                           s->id.new_id);
                  eet_write(out_file->ef, buf,
                            s->data, s->size, EINA_TRUE);
                  VERBOSE(EINA_LOG_INFO("Wrote <%s> vibration data <%p> size <%d>\n",
                                        buf, s->data, s->size));
               }
          }
     }

   _edje_pick_sound_dir_compose(samples, tones, out_file);
   _edje_pick_vibration_dir_compose(vibrations, out_file);

   /* Write file header after processing all groups */
   if (out_file)
     {
        bytes = eet_data_write(out_file->ef, _edje_edd_edje_file, "edje/file",
                               out_file, comp_mode);
        VERBOSE(EINA_LOG_INFO("Wrote <%d> bytes for file header.\n", bytes));
     }

   eina_list_free(images);
   eina_list_free(samples);
   eina_list_free(tones);

   if (output_filename)
     printf("Wrote <%s> output file.\n", output_filename);

   return _edje_pick_cleanup(inp_files, out_file, EDJE_PICK_NO_ERROR);
}

