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

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "edje_cc.h"
#include "edje_private.h"

struct _Edje_Pick_Id
{
   int old_id;
   int new_id;
   Eina_Bool used;
};
typedef struct _Edje_Pick_Id Edje_Pick_Id;

struct _Edje_Pick_File_Params
{
   const char *name;
   Eina_List *groups;
   Edje_File *edf;     /* Keeps all file data after reading  */
   Eina_Bool append;   /* Take everything from this file */

   /* We hold list of IDs for each file */
   Eina_List *scriptlist;
   Eina_List *luascriptlist;
   Eina_List *imagelist;
   Eina_List *samplelist;
   Eina_List *tonelist;
};
typedef struct _Edje_Pick_File_Params Edje_Pick_File_Params;

struct _Edje_Pick_Data
{
   const char *filename; /* Image, Sample File Name */
   void *entry ;          /* used to build output file dir */
   void *data;            /* Data as taken from input file */

   int size;
   Edje_Pick_Id id;
};
typedef struct _Edje_Pick_Data Edje_Pick_Data;

struct _Edje_Pick_Tone
{
   Edje_Sound_Tone *tone;
   Eina_Bool used;
};
typedef struct _Edje_Pick_Tone Edje_Pick_Tone;

struct _Edje_Pick_Font
{
   Font *f;
   Eina_Bool used;
};
typedef struct _Edje_Pick_Font Edje_Pick_Font;

struct _Edje_Pick
{
   Eina_Bool v; /* Verbose */
   Edje_Pick_File_Params *current_file;
   Eina_List *fontlist;
};
typedef struct _Edje_Pick Edje_Pick;

static Edje_Pick context = { EINA_FALSE, NULL, NULL };

/* FIXME: Use Eina_Log */
#define VERBOSE(COMMAND) if (context.v) { COMMAND; }

enum _Edje_Pick_Status
  {
    EDJE_PICK_NO_ERROR,
    EDJE_PICK_NO_INPUT_FILES_DEFINED,
    EDJE_PICK_INCOMPLETE_STRUCT,
    EDJE_PICK_INP_NAME_MISSING,
    EDJE_PICK_DUP_FILENAME,
    EDJE_PICK_DUP_OUT_FILENAME,
    EDJE_PICK_OUT_FILENAME_MISSING,
    EDJE_PICK_FAILED_OPEN_INP,
    EDJE_PICK_FAILED_READ_INP,
    EDJE_PICK_GROUP_NOT_FOUND,
    EDJE_PICK_IMAGE_NOT_FOUND,
    EDJE_PICK_SAMPLE_NOT_FOUND,
    EDJE_PICK_DUP_GROUP
  };
typedef enum _Edje_Pick_Status Edje_Pick_Status;

enum _Edje_Pick_Parse_Mode
  {  /* Command-line parse modes */
    EDJE_PICK_UNDEFINED,
    EDJE_PICK_GET_FILENAME,
    EDJE_PICK_GET_GROUPS,
    EDJE_PICK_GET_OUTPUT_FILENAME
  };
typedef enum _Edje_Pick_Parse_Mode Edje_Pick_Parse_Mode;

static void
_edje_pick_args_show(Eina_List *ifs, char *out)
{  /* Print command-line arguments after parsing phase */
   Edje_Pick_File_Params *p;
   Eina_List *l;
   char *g;

   printf("Got args for <%d> input files.\n", eina_list_count(ifs));

   EINA_LIST_FOREACH(ifs, l, p)
     {
        Eina_List *ll;

        printf("\nFile name: %s\n\tGroups:\n", p->name);
        EINA_LIST_FOREACH(p->groups, ll, g)
          printf("\t\t%s\n", g);
     }

   printf("\nOutput file name was <%s>\n", out);
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

static int
_edje_pick_cleanup(Eina_List *ifs, Edje_Pick_Status s)
{
   Edje_Pick_File_Params *p;
   Edje_Pick_Font *ft;
   void *n;

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

        _edje_pick_data_free(p->samplelist);

        EINA_LIST_FREE(p->tonelist, n)
          free(n);

        if (p->edf)
          free(p->edf);

        free(p);
     }

   EINA_LIST_FREE(context.fontlist, ft)
     {
        Font *st = ft->f;

        eina_stringshare_del(st->name);
        eina_stringshare_del(st->file);
        free(st);
        free(ft);
     }

   switch (s)
     {
      case EDJE_PICK_NO_INPUT_FILES_DEFINED:
         printf("No input files defined.\n");
         break;
      case EDJE_PICK_INCOMPLETE_STRUCT:
         printf("Missing name, groups or both for input file.\n");
         break;
      case EDJE_PICK_INP_NAME_MISSING:
         printf("Missing name for input file.\n");
         break;
      case EDJE_PICK_DUP_FILENAME:
         printf("Input file appears twice.\n");
         break;
      case EDJE_PICK_DUP_OUT_FILENAME:
         printf("Output file appears twice.\n");
         break;
      case EDJE_PICK_OUT_FILENAME_MISSING:
         printf("Output file name missing.\n");
         break;
      case EDJE_PICK_FAILED_OPEN_INP:
         printf("Failed to open input file.\n");
         break;
      case EDJE_PICK_FAILED_READ_INP:
         printf("Failed to read input file.\n");
         break;
      case EDJE_PICK_DUP_GROUP:
         printf("Can't fetch groups with identical name from various files.\n");
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
                                     (Eina_Compare_Cb) strcmp,
                                     group))
         return 1;

   return 0;  /* Not found */
}

static int
_edje_pick_command_line_parse(int argc, char **argv,
			      Eina_List **ifs, char **ofn)
{  /* On return ifs is Input Files List, ofn is Output File Name */
   char *output_filename = NULL;
   Edje_Pick_Parse_Mode pm = EDJE_PICK_UNDEFINED;
   Edje_Pick_File_Params *current_inp = NULL;
   Eina_List *files = NULL;  /* List of input files */
   int k;

   /* START - Read command line args */
   for(k = 1; k < argc; k++)
     {
        /* START - parse command line */
        if (strcmp(argv[k], "-a") == 0)
          {
             /* Switch mode to get file name */
             if (current_inp &&
                 ((!current_inp->name) ||
                  ((!current_inp->append) && !current_inp->groups)))
               return _edje_pick_cleanup(files, EDJE_PICK_INCOMPLETE_STRUCT);

             current_inp = calloc(1, sizeof(*current_inp));
             current_inp->append = EINA_TRUE;
             files = eina_list_append(files, current_inp);
             pm = EDJE_PICK_GET_FILENAME;
             continue;
          }

        if (strcmp(argv[k], "-i") == 0)
          {
             /* Switch mode to get file name */
             if (current_inp &&
                 ((!current_inp->name) ||
                  ((!current_inp->append) && !current_inp->groups)))
               return _edje_pick_cleanup(files, EDJE_PICK_INCOMPLETE_STRUCT);

             current_inp = calloc(1, sizeof(*current_inp));
             files = eina_list_append(files, current_inp);
             pm = EDJE_PICK_GET_FILENAME;
             continue;
          }

        if (strcmp(argv[k], "-g") == 0)
          {
             /* Switch mode to get groups names for current file */
             if (current_inp && (!current_inp->name))
               return _edje_pick_cleanup(files, EDJE_PICK_INP_NAME_MISSING);

             pm = EDJE_PICK_GET_GROUPS;
             continue;
          }

        if (strcmp(argv[k], "-o") == 0)
          {
             /* Switch to get output file name */
             if (current_inp &&
                 ((!current_inp->name) ||
                  ((!current_inp->append) && !current_inp->groups)))
               return _edje_pick_cleanup(files, EDJE_PICK_INCOMPLETE_STRUCT);

             pm = EDJE_PICK_GET_OUTPUT_FILENAME;
             continue;
          }

        if (strcmp(argv[k], "-v") == 0)
          {
             context.v = EINA_TRUE;  /* Verbose mode on */
             continue;
          }

        switch (pm)
          {
           case EDJE_PICK_GET_FILENAME:
              if (current_inp->name)
                return _edje_pick_cleanup(files, EDJE_PICK_DUP_FILENAME);
              else
                current_inp->name = argv[k];
              break;

           case EDJE_PICK_GET_GROUPS:
              if (!current_inp)
                return _edje_pick_cleanup(files, EDJE_PICK_INCOMPLETE_STRUCT);

              if (_group_name_in_other_file(files, current_inp, argv[k]))
                return _edje_pick_cleanup(files, EDJE_PICK_DUP_GROUP);

              if (!eina_list_search_unsorted(current_inp->groups,
                                             (Eina_Compare_Cb) strcmp,
                                             argv[k]))
                current_inp->groups = eina_list_append(current_inp->groups,
                                                       eina_stringshare_add(argv[k]));
              break;

           case EDJE_PICK_GET_OUTPUT_FILENAME:
              if (output_filename)
                return _edje_pick_cleanup(files, EDJE_PICK_DUP_OUT_FILENAME);

              output_filename = argv[k];
              break;
           default: ;
          }
     }  /* END   - parse command line */

   if (!files)
     return _edje_pick_cleanup(files, EDJE_PICK_NO_INPUT_FILES_DEFINED);

   if (current_inp &&
       ((!current_inp->name) ||
        ((!current_inp->append) && !current_inp->groups)))
     return _edje_pick_cleanup(files, EDJE_PICK_INCOMPLETE_STRUCT);

   if (!output_filename)
     return _edje_pick_cleanup(files, EDJE_PICK_OUT_FILENAME_MISSING);
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

        if (o->external_dir)
          base = total = o->external_dir->entries_count;
        else
          o->external_dir = calloc(1, sizeof(*(o->external_dir)));

        total += edf->external_dir->entries_count;

        o->external_dir->entries = realloc(o->external_dir->entries,
                                           total * sizeof(Edje_External_Directory_Entry));

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
        o->collection = eina_hash_string_small_new(NULL);

        /* Open output file */
        o->ef = eet_open(name, EET_FILE_MODE_WRITE);
     }
   else
     {
        if (o->version != edf->version)
          {
             printf("Warning: Merging files of various version.\n");
             if (o->version < edf->version)
               o->version = edf->version;
          }

        if (o->minor != edf->minor)
          {
             printf("Warning: Merging files of various minor.\n");
             if (o->minor < edf->minor)
               o->minor = edf->minor;
          }

        if (o->feature_ver != edf->feature_ver)
          {
             printf("Warning: Merging files of various feature_ver.\n");
             if (o->feature_ver < edf->feature_ver)
               o->feature_ver = edf->feature_ver;
          }
     }

   _edje_pick_external_dir_update(o, edf);
   return o;
}

static int
_edje_pick_header_make(Edje_File *out_file , Edje_File *edf, Eina_List *ifs)
{
   static int current_group_id = 0;
   Edje_Part_Collection_Directory_Entry *ce;
   Eina_Bool status = EDJE_PICK_NO_ERROR;
   Eina_List *l, *lst = NULL;

   Eina_Iterator *i;
   const char *key;
   char *name1 = NULL;

   i = eina_hash_iterator_key_new(edf->collection);

   /* FIXME: Don't build a list, use iterator and hash where relevant */
   EINA_ITERATOR_FOREACH(i, key)  /* Make list of all keys */
     lst = eina_list_append(lst, eina_stringshare_add(key));

   eina_iterator_free(i);

   _edje_cache_file_unref(edf);

   /* Build file header */
   if (context.current_file->append)
     {
        EINA_LIST_FOREACH(lst, l, name1)
          {
             Edje_Part_Collection_Directory_Entry *ce_out;

             /* Use ALL groups from this file */
             /* Test that no duplicate-group name for files in append mode */
             /* Here because we don't read EDC before parse cmd line       */
             if (_group_name_in_other_file(ifs, context.current_file, name1))
               return EDJE_PICK_DUP_GROUP;

             ce = eina_hash_find(edf->collection, name1);
             ce_out = malloc(sizeof(*ce_out));
             memcpy(ce_out, ce, sizeof(*ce_out));

             ce_out->id = current_group_id;
             printf("Changing ID of group <%d> to <%d>\n",ce->id, ce_out->id);
             current_group_id++;

             eina_hash_direct_add(out_file->collection, ce_out->entry, ce_out);

             /* Add this group to groups to handle for this file */
             context.current_file->groups = eina_list_append(context.current_file->groups,
                                                             eina_stringshare_add(name1));
          }
     }
   else
     {
        EINA_LIST_FOREACH(context.current_file->groups, l , name1)
          {
             /* Verify group found then add to ouput file header */
             char *name2 = eina_list_search_unsorted(lst,
                                                     (Eina_Compare_Cb) strcmp,
                                                     name1);

             if (!name2)
               {
                  printf("Group <%s> was not found in <%s> file.\n",
                         name1, context.current_file->name);
                  status = EDJE_PICK_GROUP_NOT_FOUND;
               }
             else
               {
                  Edje_Part_Collection_Directory_Entry *ce_out;

                  /* Add this groups to hash, with filname pefix for entries */
                  ce = eina_hash_find(edf->collection, name2);
                  ce_out = malloc(sizeof(*ce_out));

                  memcpy(ce_out, ce, sizeof(*ce_out));

                  ce_out->id = current_group_id;
                  printf("Changing ID of group <%d> to <%d>\n",ce->id, ce_out->id);
                  current_group_id++;

                  eina_hash_direct_add(out_file->collection,ce_out->entry,ce_out);
               }
          }
     }

   EINA_LIST_FREE(lst, key)
     eina_stringshare_del(key);

   return status;
}

static int
_id_cmp(const void *d1, const void *d2)
{
   /* Find currect ID struct */
   return (((Edje_Pick_Data *) d1)->id.old_id - ((int) d2));
}

static int
_edje_pick_new_id_get(Eina_List *id_list, int id, Eina_Bool set_used)
{
   if (id >= 0)
     {
        Edje_Pick_Data *p_id = eina_list_search_unsorted(id_list,
                                                         _id_cmp,
                                                         (void *) id);


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
_edje_pick_images_add(Edje_File *edf)
{
   char buf[1024];
   int size, k;
   void *data;
   Eina_Bool status = EDJE_PICK_NO_ERROR;
   static int current_img_id = 0;

   if (edf->image_dir)  /* Copy Images */
     for (k = 0; k < (int) edf->image_dir->entries_count; k++)
       {
          Edje_Image_Directory_Entry *img = &edf->image_dir->entries[k];

          snprintf(buf, sizeof(buf), "edje/images/%i", img->id);
          VERBOSE(printf("Trying to read <%s>\n", img->entry));
          data = eet_read(edf->ef, buf, &size);
          if (size)
            {  /* Advance image ID and register this in imagelist */
               Edje_Pick_Data *image = malloc(sizeof(*image));

               image->filename = eina_stringshare_add(img->entry);
               image->data = data;
               image->size = size;
               image->entry = (void *) img;  /* for output file image dir */
               image->id.old_id = img->id;
               img->id = image->id.new_id = current_img_id;
               image->id.used = EINA_FALSE;

               VERBOSE(printf("Read image <%s> data <%p> size <%d>\n",
                              buf, image->data, image->size));

               current_img_id++;
               context.current_file->imagelist = eina_list_append(context.current_file->imagelist,
                                                                  image);
            }
          else
            {
               printf("Image <%s> was not found in <%s> file.\n",
                      img->entry , context.current_file->name);
               status = EDJE_PICK_IMAGE_NOT_FOUND;
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
        for (k = 0; k < (int) edf->sound_dir->samples_count; k++)
          {
             Edje_Sound_Sample *sample = &edf->sound_dir->samples[k];

             snprintf(buf, sizeof(buf), "edje/sounds/%i", sample->id);
             VERBOSE(printf("Trying to read <%s>\n", sample->name));

             data = eet_read(edf->ef, buf, &size);
             if (size)
               {
                  Edje_Pick_Data *smpl = malloc(sizeof(*smpl));
                  smpl->filename = eina_stringshare_add(sample->name);
                  smpl->data = data;
                  smpl->size = size;
                  smpl->entry = (void *) sample; /* for output file sound dir */
                  smpl->id.old_id = sample->id;
                  sample->id = smpl->id.new_id = current_sample_id;
                  smpl->id.used = EINA_FALSE;

                  VERBOSE(printf("Read <%s> sample data <%p> size <%d>\n",
                                 buf, smpl->data, smpl->size));

                  current_sample_id++;
                  context.current_file->samplelist =
                    eina_list_append(context.current_file->samplelist, smpl);
               }
             else
               {
                  printf("Sample <%s> was not found in <%s> file.\n",
                         sample->name, context.current_file->name);
                  status = EDJE_PICK_SAMPLE_NOT_FOUND;
               }
          }

        for (k = 0; k < (int) edf->sound_dir->tones_count; k++)
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
_font_cmp(const void *d1, const void *d2)
{
   /* Same font if (d1->name == d2->name) AND (d1->file == d2->file) */
   return (strcmp(((Font *) d1)->name, ((Font *) d2)->name) |
           strcmp(((Font *) d1)->file, ((Font *) d2)->file));
}

static int
_Edje_Pick_Fonts_add(Edje_File *edf)
{
   /* FIXME: share code with other bin */
   Eet_Data_Descriptor *_font_list_edd = NULL;
   Eet_Data_Descriptor *_font_edd;
   Eet_Data_Descriptor_Class eddc;
   Font_List *fl;
   Font *f;
   Eina_List *l;

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc),
                                             "font", sizeof (Font));
   _font_edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_font_edd, Font,
                                 "file", file, EET_T_INLINED_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_font_edd, Font,
                                 "name", name, EET_T_INLINED_STRING);

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc),
                                             "font_list", sizeof (Font_List));
   _font_list_edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_font_list_edd, Font_List,
                                "list", list, _font_edd);
   fl = eet_data_read(edf->ef, _font_list_edd, "edje_source_fontmap");

   EINA_LIST_FOREACH(fl->list, l, f)
     {
        if (!eina_list_search_unsorted(context.fontlist,
                                       _font_cmp, f))
          {
             /* Add only fonts that are NOT regestered in our list */
             Edje_Pick_Font *ft =  malloc(sizeof(*ft));
             Font *st = malloc(sizeof(*st));

             st->name = (char *) eina_stringshare_add(f->name);
             st->file = (char *) eina_stringshare_add(f->file);

             ft->f = st;
             ft->used = EINA_TRUE;  /* TODO: Fix this later */
             context.fontlist = eina_list_append(context.fontlist, ft);
          }
     }

   free(fl);
   eet_data_descriptor_free(_font_list_edd);
   eet_data_descriptor_free(_font_edd);

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

        VERBOSE(printf("Read embryo script <%s> data <%p> size <%d>\n",
                       buf, s->data, s->size));
        context.current_file->scriptlist = eina_list_append(context.current_file->scriptlist, s);
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

        VERBOSE(printf("Read lua script <%s> data <%p> size <%d>\n",
                       buf, s->data, s->size));
        context.current_file->luascriptlist = eina_list_append(context.current_file->luascriptlist, s);
     }

   return EDJE_PICK_NO_ERROR;
}

static void
_edje_pick_styles_update(Edje_File *o, Edje_File *edf)
{
   /* Color Class in Edje_File */
   Eina_List *l;
   Edje_Style *stl;

   EINA_LIST_FOREACH(edf->styles, l, stl)
     o->styles = eina_list_append(o->styles, stl);
}

static void
_edje_pick_color_class_update(Edje_File *o, Edje_File *edf)
{
   /* Color Class in Edje_File */
   Eina_List *l;
   Edje_Color_Class *cc;

   EINA_LIST_FOREACH(edf->color_classes, l, cc)
     o->color_classes = eina_list_append(o->color_classes, cc);
}


static void
_edje_pick_images_desc_update(Edje_Part_Description_Image *desc)
{
   /* Update all IDs of images in descs */
   if (desc)
     {
        unsigned int k;

        desc->image.id = _edje_pick_new_id_get(context.current_file->imagelist,
                                               desc->image.id,
                                               EINA_TRUE);

        for (k = 0; k < desc->image.tweens_count; k++)
          desc->image.tweens[k]->id = _edje_pick_new_id_get(context.current_file->imagelist,
                                                            desc->image.tweens[k]->id ,
                                                            EINA_TRUE);
     }
}

static void
_edje_pick_images_process(Edje_Part_Collection *edc)
{
   /* Find what images are used, update IDs, mark as USED */
   unsigned int i;

   for (i = 0; i < edc->parts_count; i++)
     {
        /* Scan all parts, locate what images used */
        Edje_Part *part = edc->parts[i];

        if (part->type == EDJE_PART_TYPE_IMAGE)
          {
             /* Update IDs of all images in ALL descs of this part */
             unsigned int k;

             _edje_pick_images_desc_update((Edje_Part_Description_Image *) part->default_desc);

             for (k = 0; k < part->other.desc_count; k++)
               _edje_pick_images_desc_update((Edje_Part_Description_Image *) part->other.desc[k]);
          }
     }
}

static int
_sample_cmp(const void *d1, const void *d2)
{
   /* Locate sample by name */
   if (d2)
     {
        Edje_Sound_Sample *sample = ((Edje_Pick_Data *) d1)->entry;

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
        Edje_Sound_Tone *tone = ((Edje_Pick_Tone *) d1)->tone;

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
                                 (Eina_Compare_Cb) _sample_cmp,
                                 prog->sample_name);

   /* Sample is used by program, should be saved */
   if (p)
     p->id.used = EINA_TRUE;

   /* handle tones as well */
   t = eina_list_search_unsorted(context.current_file->tonelist,
                                 (Eina_Compare_Cb) _tone_cmp,
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

   for(i = 0; i < edc->programs.fnmatch_count; i++)
     _edje_pick_program_update(edc->programs.fnmatch[i]);

   for(i = 0; i < edc->programs.strcmp_count; i++)
     _edje_pick_program_update(edc->programs.strcmp[i]);

   for(i = 0; i < edc->programs.strncmp_count; i++)
     _edje_pick_program_update(edc->programs.strncmp[i]);

   for(i = 0; i < edc->programs.strrncmp_count; i++)
     _edje_pick_program_update(edc->programs.strrncmp[i]);

   for(i = 0; i < edc->programs.nocmp_count; i++)
     _edje_pick_program_update(edc->programs.nocmp[i]);

   return EDJE_PICK_NO_ERROR;
}

static int
_edje_pick_collection_process(Edje_Part_Collection *edc)
{
   /* Update all IDs, NAMES in current collection */
   static int current_collection_id = 0;

   edc->id = current_collection_id;
   current_collection_id++;
   _edje_pick_images_process(edc);
   _edje_pick_programs_process(edc);

   return EDJE_PICK_NO_ERROR;
}

static void
_edje_pick_image_dir_compose(Eina_List *images,
			     Edje_Image_Directory_Set *sets,
			     unsigned int sets_count,
			     Edje_File *o)
{
   /* Compose image_dir array from all used images */
   if (images)
     {
        Edje_Image_Directory_Entry *entry;
        Edje_Image_Directory_Entry *p;
        Eina_List *l;

        o->image_dir = calloc(1, sizeof(*(o->image_dir)));

        o->image_dir->entries = malloc(eina_list_count(images) *
                                       sizeof(Edje_Image_Directory_Entry));

        p = o->image_dir->entries;
        EINA_LIST_FOREACH(images, l, entry)
          {
             memcpy(p, entry, sizeof(Edje_Image_Directory_Entry));
             p++;
          }

        o->image_dir->entries_count = eina_list_count(images);

        o->image_dir->sets = sets;
        o->image_dir->sets_count = sets_count;
     }
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
                  memcpy(t, tones, sizeof(Edje_Sound_Tone));
                  t++;
               }

             o->sound_dir->tones_count = eina_list_count(tones);
          }
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
   Edje_Image_Directory_Set *sets = NULL; /* ALL files sets composed here */
   unsigned int sets_count = 0;  /* ALL files sets-count accumolated here */

   Edje_Part_Collection *edc;
   Edje_Part_Collection_Directory_Entry *ce;
   Eet_File *ef;
   Font_List *fl;
   Eina_List *f, *l;
   char buf[1024];
   void *n;
   int k, bytes;

   if (argc < 4)
     {
        /* FIXME: display a proper help with information */
        printf("%s -i|-a input-file.edj -g group1 [group2 ...] [-i input_file2.edj -g ...] -o output-file.edj\n", argv[0]);

        return -1;
     }

   eina_init();
   eet_init();
   _edje_edd_init();

   /* FIXME: use Ecore_Getopt */
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
          return _edje_pick_cleanup(inp_files, EDJE_PICK_FAILED_OPEN_INP);

        edf = eet_data_read(ef, _edje_edd_edje_file, "edje/file");
        if (!edf)
          return _edje_pick_cleanup(inp_files, EDJE_PICK_FAILED_READ_INP);

        context.current_file->edf = edf;
        edf->ef = ef;

        out_file = _edje_pick_output_prepare(out_file, edf, output_filename);

        k = _edje_pick_header_make(out_file, edf, inp_files);
        if (k != EDJE_PICK_NO_ERROR)
          {
             eet_close(ef);
             eet_close(out_file->ef);
             return _edje_pick_cleanup(inp_files, k);
          }

        /* Build lists of all images, samples and fonts of input files    */
        _edje_pick_images_add(edf);  /* Add Images to imagelist           */
        _edje_pick_sounds_add(edf);  /* Add Sounds to samplelist          */
        _Edje_Pick_Fonts_add(edf);   /* Add fonts from file to fonts list */

        /* Copy styles, color class */
        _edje_pick_styles_update(out_file, edf);
        _edje_pick_color_class_update(out_file, edf);

        /* Process Groups */
        EINA_LIST_FOREACH(context.current_file->groups, l , name1)
          {  /* Read group info */
             ce = eina_hash_find(edf->collection, name1);
             if (!ce || (ce->id < 0))
               {
                  printf("Failed to find group <%s> id\n", name1);
                  return _edje_pick_cleanup(inp_files,
                                            EDJE_PICK_GROUP_NOT_FOUND);
               }

             VERBOSE(printf("Copy group: <%s>\n", name1));

             /* FIXME: share code with edje_cache.c */
             { /**  MEMPOOL ALLOC START *****************/
                char *buffer;
#define INIT_EMP(Tp, Sz, Ce)						\
                buffer = alloca(strlen(ce->entry) + strlen(#Tp) + 2);	\
                sprintf(buffer, "%s/%s", ce->entry, #Tp);               \
                Ce->mp.Tp = eina_mempool_add("one_big", buffer, NULL, sizeof (Sz), Ce->count.Tp); \
                _emp_##Tp = Ce->mp.Tp;

#define INIT_EMP_BOTH(Tp, Sz, Ce)                                       \
                INIT_EMP(Tp, Sz, Ce)

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
                INIT_EMP(part, Edje_Part, ce);
             } /**  MEMPOOL ALLOC END *******************/

             snprintf(buf, sizeof(buf), "edje/collections/%i", ce->id);
             printf("Trying to read group <%s>\n", buf);
             edc = eet_data_read(edf->ef, _edje_edd_edje_part_collection, buf);
             if (!edc)
               {
                  printf("Failed to read group <%s> id <%d>\n", name1, ce->id);
                  return _edje_pick_cleanup(inp_files,
                                            EDJE_PICK_GROUP_NOT_FOUND);
               }

             /* Update IDs */
             _edje_pick_collection_process(edc);

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
                printf("Wrote <%d> bytes for group <%s>\n", bytes,buf);
             }

             free(edc);
             eet_close(ef);
          }

        /* We SKIP writing source, just can't compose it */
        /* FIXME: use Edje_Edit code to generate source */
     } /* END   - Main loop scanning input files */

   /* Write rest of output */

   EINA_LIST_FOREACH(inp_files, f, context.current_file)
     {
        /* Write Scripts from ALL files */
        Edje_File *edf;
        Edje_Pick_Data *s;
        Edje_Pick_Tone *tn;
        Eina_List *t;

        EINA_LIST_FOREACH(context.current_file->scriptlist, t, s)
          {
             /* Write Scripts */
             snprintf(buf, sizeof(buf),
                      "edje/scripts/embryo/compiled/%i", s->id.new_id);
             VERBOSE(printf("wrote embryo scr <%s> data <%p> size <%d>\n",
                            buf, s->data, s->size));
             eet_write(out_file->ef, buf, s->data, s->size, comp_mode);
          }

        EINA_LIST_FOREACH(context.current_file->luascriptlist, t, s)
          {
             /* Write Lua Scripts */
             snprintf(buf, sizeof(buf),
                      "edje/scripts/lua/%i", s->id.new_id);
             VERBOSE(printf("wrote lua scr <%s> data <%p> size <%d>\n",
                            buf, s->data, s->size));
             eet_write(out_file->ef, buf, s->data, s->size, comp_mode);
          }

        edf = context.current_file->edf;

        EINA_LIST_FOREACH(context.current_file->imagelist, t, s)
          {
             if (context.current_file->append || s->id.used)
               {
                  images = eina_list_append(images, s->entry);


                  snprintf(buf, sizeof(buf), "edje/images/%i", s->id.new_id);
                  eet_write(out_file->ef, buf, s->data, s->size, EINA_TRUE);
                  VERBOSE(printf("Wrote <%s> image data <%p> size <%d>\n",
                                 buf, s->data, s->size));
               }
          }

        if (edf->image_dir && edf->image_dir->sets_count)
          {
             /* Update sets from current file sets */
             sets = realloc(sets,
                            (edf->image_dir->sets_count + sets_count)
                            * sizeof(Edje_Image_Directory_Set));

             memcpy(&sets[sets_count],
                    edf->image_dir->sets,
                    edf->image_dir->sets_count *
                    sizeof(Edje_Image_Directory_Set));

             sets_count += edf->image_dir->sets_count;
          }

        EINA_LIST_FOREACH(context.current_file->samplelist, l, s)
          {
             if (context.current_file->append || s->id.used)
               {  /* Write only used samples */
                  samples = eina_list_append(samples, s->entry);

                  snprintf(buf, sizeof(buf), "edje/sounds/%i",
                           s->id.new_id);
                  eet_write(out_file->ef, buf,
                            s->data, s->size,EINA_TRUE);
                  VERBOSE(printf("Wrote <%s> sample data <%p> size <%d>\n",
                                 buf, s->data, s->size));
               }
          }

        EINA_LIST_FOREACH(context.current_file->tonelist, l, tn)
          {
             if (context.current_file->append || tn->used)
               tones = eina_list_append(tones, tn->tone);
          }
     }

   /* Add all files images to out_file image_dir */
   _edje_pick_image_dir_compose(images, sets, sets_count, out_file);
   _edje_pick_sound_dir_compose(samples, tones, out_file);

   if (out_file->image_dir)
     {
        /* Fix sets IDs */
        unsigned int j, i;
        Edje_Image_Directory_Set *p;

        for (j = 0; j < out_file->image_dir->sets_count; ++j)
          {
             p = &out_file->image_dir->sets[k];

             for (i = 0; i < out_file->image_dir->entries_count; ++i)
               if (!strcmp(out_file->image_dir->entries[i].entry, p->name))
                 {
                    p->id = i;
                    break;
                 }
          }

     }

   /* Write file header after processing all groups */
   bytes = eet_data_write(out_file->ef, _edje_edd_edje_file, "edje/file",
                          out_file, comp_mode);

   VERBOSE(printf("Wrote <%d> bytes for file header.\n", bytes));

   eina_list_free(images);
   eina_list_free(samples);
   eina_list_free(tones);

   fl = calloc(1, sizeof(*fl));

   EINA_LIST_FOREACH(context.fontlist, l, n)
     {
        /*  Create a font list from used fonts */
        Edje_Pick_Font *fnt = n;
        if (context.current_file->append || fnt->used)
          fl->list = eina_list_append(fl->list, fnt->f);
     }

   {
      /* FIXME: share code with other bin */
      /* Write Fonts from all files */
      Eet_Data_Descriptor_Class eddc;
      Eet_Data_Descriptor *_font_list_edd = NULL;
      Eet_Data_Descriptor *_font_edd;

      eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc),
                                                "font", sizeof (Font));

      _font_edd = eet_data_descriptor_stream_new(&eddc);
      EET_DATA_DESCRIPTOR_ADD_BASIC(_font_edd, Font,
                                    "file", file, EET_T_INLINED_STRING);
      EET_DATA_DESCRIPTOR_ADD_BASIC(_font_edd, Font,
                                    "name", name, EET_T_INLINED_STRING);

      eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc),
                                                "font_list", sizeof (Font_List));
      _font_list_edd = eet_data_descriptor_stream_new(&eddc);
      EET_DATA_DESCRIPTOR_ADD_LIST(_font_list_edd, Font_List,
                                   "list", list, _font_edd);
      bytes = eet_data_write(out_file->ef, _font_list_edd,
                             "edje_source_fontmap", fl, comp_mode);
      VERBOSE(printf("Wrote <%d> bytes for fontmap.\n", bytes));

      eet_data_descriptor_free(_font_list_edd);
      eet_data_descriptor_free(_font_edd);
   }
   free(fl);

   if (sets)
     free(sets);

   printf("Wrote <%s> output file.\n", output_filename);
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

        eina_list_free(out_file->color_classes);
        eina_hash_free_cb_set(out_file->collection, free);
        eina_hash_free(out_file->collection);
        eina_stringshare_del(out_file->compiler);
        free(out_file);
     }

   return _edje_pick_cleanup(inp_files, EDJE_PICK_NO_ERROR);
}
