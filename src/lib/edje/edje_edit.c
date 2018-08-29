/*
 * TODO
 * -----------------------------------------------------------------
 * Add LUA Support :)
 * Remove images/fonts
 * Clean the saving routines
 *
 */

#define _EDJE_EDIT_EO_CLASS_TYPE
#define EFL_CANVAS_GROUP_PROTECTED
#define EFL_CANVAS_GROUP_BETA

#include "edje_private.h"

#include "canvas/evas_canvas.eo.h"

#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT
#include "Edje_Edit.h"

#include <Eo.h>

#include "../../static_libs/buildsystem/buildsystem.h"

#ifdef MY_CLASS
# undef MY_CLASS
#endif

#define MY_CLASS EDJE_EDIT_CLASS

EAPI Eina_Error EDJE_EDIT_ERROR_GROUP_CURRENTLY_USED = 0;
EAPI Eina_Error EDJE_EDIT_ERROR_GROUP_REFERENCED = 0;
EAPI Eina_Error EDJE_EDIT_ERROR_GROUP_DOES_NOT_EXIST = 0;

/* Get eed(Edje_Edit*) from obj(Evas_Object*) */
#define GET_EED_OR_RETURN(RET)            \
  Edje_Edit *eed;                         \
  if (!efl_isa(obj, MY_CLASS))             \
    return RET;                           \
  eed = efl_data_scope_get(obj, MY_CLASS); \
  if (!eed) return RET;

/* Get ed(Edje*) from obj(Evas_Object*) */
#define GET_ED_OR_RETURN(RET)          \
  Edje *ed;                            \
  if (!efl_isa(obj, EFL_CANVAS_LAYOUT_CLASS)) \
    return RET;                        \
  ed = efl_data_scope_get(obj, EFL_CANVAS_LAYOUT_CLASS);

/* Get rp(Edje_Real_Part*) from obj(Evas_Object*) and part(char*) */
#define GET_RP_OR_RETURN(RET)         \
  GET_ED_OR_RETURN(RET)               \
  Edje_Real_Part *rp;                 \
  rp = _edje_real_part_get(ed, part); \
  if (!rp) return RET;

/* Get pd(Edje_Part_Description*) from obj(Evas_Object*), part(char*) and state (char*) */
#define GET_PD_OR_RETURN(RET)                                       \
  GET_EED_OR_RETURN(RET)                                            \
  GET_ED_OR_RETURN(RET)                                             \
  Edje_Real_Part *rp;                                               \
  Edje_Part_Description_Common *pd;                                 \
  rp = _edje_real_part_get(ed, part);                               \
  if (!rp) return RET;                                              \
  pd = _edje_part_description_find_byname(eed, part, state, value); \
  if (!pd) return RET;

/* Get epr(Edje_Program*) from obj(Evas_Object*) and prog(char*)*/
#define GET_EPR_OR_RETURN(RET)               \
  Edje_Program *epr;                         \
  if (!efl_isa(obj, MY_CLASS))                \
    return RET;                              \
  epr = _edje_program_get_byname(obj, prog); \
  if (!epr) return RET;

static void *
_alloc(size_t size)
{
   void *mem;

   mem = calloc(1, size);
   if (mem) return mem;
   ERR("Edje_Edit: Error. memory allocation of %i bytes failed. %s",
       (int)size, strerror(errno));
   return NULL;
}

/*************/
/* INTERNALS */
/*************/

/* Edje_Edit smart! Overloads the edje one adding some more control stuff */

typedef struct _Edje_Edit Edje_Edit;
struct _Edje_Edit
{
   Edje      *base;

   void      *bytecode;
   int        bytecode_size;

   char      *embryo_source;
   char      *embryo_processed;
   Eina_Hash *program_scripts;

   Eina_List *errors;

   Eina_Bool  bytecode_dirty : 1;
   Eina_Bool  embryo_source_dirty : 1;
   Eina_Bool  all_dirty : 1;
   Eina_Bool  script_need_recompile : 1;
};

typedef struct _Program_Script Program_Script;
struct _Program_Script
{
   int       id;
   char     *code;
   char     *processed;
   Eina_Bool dirty : 1;
   Eina_Bool delete_me : 1;
};

static Eina_Bool _edje_edit_edje_file_save(Eet_File *eetf, Edje_File *ef);

static void
_edje_edit_data_clean(Edje_Edit *eed)
{
   Edje_Edit_Script_Error *se;

   free(eed->bytecode);
   free(eed->embryo_source);
   free(eed->embryo_processed);

   if (eed->program_scripts)
     eina_hash_free(eed->program_scripts);

   EINA_LIST_FREE(eed->errors, se)
     {
        eina_stringshare_del(se->program_name);
        eina_stringshare_del(se->error_str);
        free(se);
     }

   eed->bytecode = NULL;
   eed->embryo_source = NULL;
   eed->embryo_processed = NULL;
   eed->program_scripts = NULL;
   eed->bytecode_size = 0;
   eed->bytecode_dirty = EINA_FALSE;
   eed->embryo_source_dirty = EINA_FALSE;
   eed->all_dirty = EINA_FALSE;
   eed->script_need_recompile = EINA_FALSE;
}

EOLIAN static void
_edje_edit_efl_canvas_group_group_del(Eo *obj, Edje_Edit *eed)
{
   _edje_edit_data_clean(eed);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

static void
_edje_edit_program_script_free(Program_Script *ps)
{
   free(ps->code);
   free(ps->processed);
   free(ps);
}

static Eet_File *
_edje_edit_eet_open(Edje *ed, Eet_File_Mode mode)
{
   Eet_File *eetf;

   switch (mode)
     {
      case EET_FILE_MODE_INVALID:
         return NULL;
      case EET_FILE_MODE_READ:
         return ed->file->ef;
      case EET_FILE_MODE_WRITE:
      case EET_FILE_MODE_READ_WRITE:
         eetf = eet_open(ed->path, mode);
         if (!eetf)
           ERR("Unable to open \"%s\" for writing output", ed->path);
         return eetf;
     }
   return NULL;
}

static void
_edje_edit_eet_close(Eet_File *ef)
{
   Eet_File_Mode mode = eet_mode_get(ef);
   if (mode != EET_FILE_MODE_READ)
     eet_close(ef);
}

static Eina_Bool
_load_scripts(Eo *obj, Edje_Edit *eed)
{
   Eet_File *ef;
   char **keys, buf[64];
   int count, i;
   int len = strlen("edje/scripts/embryo/source/");

   GET_ED_OR_RETURN(EINA_FALSE);

   eed->program_scripts = eina_hash_int32_new((Eina_Free_Cb)_edje_edit_program_script_free);

   ef = _edje_edit_eet_open(ed, EET_FILE_MODE_READ);

   snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%i",
            eed->base->collection->id);
   eed->embryo_source = eet_read(ef, buf, &count);

   snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%i/*",
            eed->base->collection->id);
   keys = eet_list(ef, buf, &count);
   if (keys)
     {
        for (i = 0; i < count; i++)
          {
             Program_Script *ps;
             int size;

             ps = calloc(1, sizeof(Program_Script));

             sscanf(keys[i] + len, "%*i/%i", &ps->id);
             ps->code = eet_read(ef, keys[i], &size);
             eina_hash_add(eed->program_scripts, &ps->id, ps);
          }
        free(keys);
     }
   _edje_edit_eet_close(ef);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_edje_edit_efl_file_file_set(Eo *obj, Edje_Edit *eed, const char *file, const char *group)
{
   _edje_edit_data_clean(eed);

   /* TODO and maybes:
    *  * The whole point of this thing is keep track of stuff such as
    *    strings to free and who knows what, so we need to take care
    *    of those if the file/group changes.
    *  * Maybe have the possibility to open just files, not always with
    *    a group given.
    *  * A way to skip the cache? Could help avoid some issues when editing
    *    a group being used by the application in some other way, or multiple
    *    opens of the same file.
    *  * Here we probably want to allow opening groups with broken references
    *    (GROUP parts or BOX/TABLE items pointing to non-existent/renamed
    *    groups).
    *  P.S. don't forget about mmap version below
    */
   file = eina_vpath_resolve(file);

   Eina_Bool int_ret;
   int_ret = efl_file_set(efl_super(obj, MY_CLASS), file, group);

   if (!int_ret)
     return EINA_FALSE;

   if (!_load_scripts(obj, eed))
     return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_edje_edit_efl_file_mmap_set(Eo *obj, Edje_Edit *eed, const Eina_File *mmap, const char *group)
{
   _edje_edit_data_clean(eed);

   Eina_Bool int_ret;
   int_ret = efl_file_mmap_set(efl_super(obj, MY_CLASS), mmap, group);
   if (!int_ret)
     return EINA_FALSE;

   if (!_load_scripts(obj, eed))
     return EINA_FALSE;

   return EINA_TRUE;
}

EAPI Evas_Object *
edje_edit_object_add(Evas *evas)
{
   evas = evas_find(evas);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(evas, EVAS_CANVAS_CLASS), NULL);
   return efl_add(MY_CLASS, evas, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_edje_edit_efl_object_constructor(Eo *obj, Edje_Edit *eed)
{
   eed->base = efl_data_ref(obj, EFL_CANVAS_LAYOUT_CLASS);

   return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_edje_edit_efl_object_destructor(Eo *obj, Edje_Edit *class_data EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
   efl_data_unref(obj, class_data);
}

/* End of Edje_Edit smart stuff */

static Edje_Part_Description_Common *
_edje_part_description_find_byname(Edje_Edit *eed, const char *part, const char *state, double value)
{
   Edje_Real_Part *rp;
   Edje_Part_Description_Common *pd;

   if (!eed || !part || !state) return NULL;

   rp = _edje_real_part_get(eed->base, part);
   if (!rp) return NULL;

   pd = _edje_part_description_find(eed->base, rp, state, value, EINA_FALSE);

   return pd;
}

static int
_edje_vector_id_find(Edje_Edit *eed, const char *vector_name)
{
   unsigned int i;

   if (!eed->base->file) return -1;
   if (!eed->base->file->image_dir) return -1;

   //printf("SEARCH IMAGE %s\n", vector_name);

   for (i = 0; i < eed->base->file->image_dir->vectors_count; ++i)
     if (eed->base->file->image_dir->vectors[i].entry
         && !strcmp(vector_name, eed->base->file->image_dir->vectors[i].entry))
       return i;

   return -1;
}

static int
_edje_image_id_find(Edje_Edit *eed, const char *image_name)
{
   unsigned int i;

   if (!eed->base->file) return -1;
   if (!eed->base->file->image_dir) return -1;

   //printf("SEARCH IMAGE %s\n", image_name);

   for (i = 0; i < eed->base->file->image_dir->entries_count; ++i)
     if (eed->base->file->image_dir->entries[i].entry
         && !strcmp(image_name, eed->base->file->image_dir->entries[i].entry))
       return i;

   return -1;
}

static int
_edje_set_id_find(Edje_Edit *eed, const char *set_name)
{
   unsigned int i;

   if (!eed->base->file) return -1;
   if (!eed->base->file->image_dir) return -1;

   for (i = 0; i < eed->base->file->image_dir->sets_count; ++i)
     if (eed->base->file->image_dir->sets[i].name
         && !strcmp(set_name, eed->base->file->image_dir->sets[i].name))
       return i;

   return -1;
}

static const char *
_edje_image_name_find(Edje_Edit *eed, int image_id)
{
   if (!eed->base->file) return NULL;
   if (!eed->base->file->image_dir) return NULL;

   /* Special case for external image */
   if (image_id < 0) image_id = -image_id - 1;

   //printf("SEARCH IMAGE ID %d\n", image_id);
   if ((unsigned int)image_id >= eed->base->file->image_dir->entries_count)
     return NULL;
   return eed->base->file->image_dir->entries[image_id].entry;
}

static const char *
_edje_set_name_find(Edje_Edit *eed, int set_id)
{
   if (!eed->base->file) return NULL;
   if (!eed->base->file->image_dir) return NULL;

   if ((unsigned int)set_id >= eed->base->file->image_dir->sets_count)
     return NULL;
   return eed->base->file->image_dir->sets[set_id].name;
}

static const char *
_edje_vector_name_find(Edje_Edit *eed, int vector_id)
{
   if (!eed->base->file) return NULL;
   if (!eed->base->file->image_dir) return NULL;

   if ((unsigned int)vector_id >= eed->base->file->image_dir->vectors_count)
     return NULL;
   return eed->base->file->image_dir->vectors[vector_id].entry;
}

static void
_edje_real_part_free(Edje *ed, Edje_Real_Part *rp)
{
   if (!rp) return;

   if (rp->object)
     {
        _edje_callbacks_del(rp->object, ed);
        evas_object_del(rp->object);
     }

   if ((rp->type == EDJE_RP_TYPE_SWALLOW) && (rp->typedata.swallow)
       && (rp->typedata.swallow->swallowed_object))
     {
        efl_parent_set(rp->typedata.swallow->swallowed_object, evas_object_evas_get(ed->obj));
        evas_object_smart_member_del(rp->typedata.swallow->swallowed_object);
        evas_object_event_callback_del(rp->typedata.swallow->swallowed_object,
                                       EVAS_CALLBACK_FREE, _edje_object_part_swallow_free_cb);
        evas_object_clip_unset(rp->typedata.swallow->swallowed_object);
        evas_object_data_del(rp->typedata.swallow->swallowed_object,
                             "\377 edje.swallowing_part");
        if (rp->part->mouse_events)
          _edje_callbacks_del(rp->typedata.swallow->swallowed_object, ed);

        if (rp->part->type == EDJE_PART_TYPE_GROUP ||
            rp->part->type == EDJE_PART_TYPE_EXTERNAL)
          evas_object_del(rp->typedata.swallow->swallowed_object);

        rp->typedata.swallow->swallowed_object = NULL;
     }

   if ((rp->type == EDJE_RP_TYPE_TEXT) && (rp->typedata.text) &&
       (rp->typedata.text->text)) eina_stringshare_del(rp->typedata.text->text);
   if ((rp->type == EDJE_RP_TYPE_TEXT) && (rp->typedata.text) &&
       (rp->typedata.text->font)) eina_stringshare_del(rp->typedata.text->font);
   if ((rp->type == EDJE_RP_TYPE_TEXT) && (rp->typedata.text) &&
       (rp->typedata.text->cache.in_str)) eina_stringshare_del(rp->typedata.text->cache.in_str);
   if ((rp->type == EDJE_RP_TYPE_TEXT) && (rp->typedata.text) &&
       (rp->typedata.text->cache.out_str)) eina_stringshare_del(rp->typedata.text->cache.out_str);

   if (rp->custom)
     {
        _edje_collection_free_part_description_clean(rp->part->type, rp->custom->description, 0);
        if (rp->custom) free(rp->custom->set);
        eina_mempool_free(_edje_real_part_state_mp, rp->custom);
        rp->custom = NULL;
     }

   free(rp->drag);

   if (rp->param2) free(rp->param2->set);
   eina_mempool_free(_edje_real_part_state_mp, rp->param2);

   _edje_unref(ed);
   eina_mempool_free(_edje_real_part_mp, rp);
}

static Eina_Bool
_edje_edit_file_import(Edje *ed, const char *path, const char *entry, int compress)
{
   Eina_File *f;
   Eet_File *eetf = NULL;
   void *fdata = NULL;
   long fsize = 0;

   /* Read data from file */
   f = eina_file_open(path, 0);
   if (!f)
     {
        ERR("Unable to open file \"%s\"", path);
        return EINA_FALSE;
     }

   fsize = eina_file_size_get(f);
   fdata = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!fdata)
     {
        ERR("Unable to map file \"%s\"", path);
        goto on_error;
     }

   /* Write file data to edje file */
   eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     goto on_error;

   if (eet_write(eetf, entry, fdata, fsize, compress) <= 0)
     {
        ERR("Unable to write \"%s\" as \"%s\" part entry",
            path, entry);
        goto on_error;
     }

   /* write the edje_file */
   if (!_edje_edit_edje_file_save(eetf, ed->file))
     goto on_error;

   _edje_edit_eet_close(eetf);

   eina_file_map_free(f, fdata);
   eina_file_close(f);

   return EINA_TRUE;

on_error:
   if (eetf) _edje_edit_eet_close(eetf);
   eina_file_map_free(f, fdata);
   eina_file_close(f);

   return EINA_FALSE;
}

static Eina_Bool
_edje_import_image_file(Edje *ed, const char *path, int id)
{
   char entry[PATH_MAX];
   Evas_Object *im;
   Eet_File *eetf;
   void *im_data;
   int im_w, im_h;
   int im_alpha;
   int bytes;

   /* Try to load the file */
   im = evas_object_image_add(ed->base.evas);
   if (!im) return EINA_FALSE;

   evas_object_image_file_set(im, path, NULL);
   if (evas_object_image_load_error_get(im) != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Edje_Edit: unable to load image \"%s\"."
            "Missing PNG or JPEG loader modules for Evas or "
            "file does not exist, or is not readable.", path);
        evas_object_del(im);
        im = NULL;
        return EINA_FALSE;
     }

   /* Write the loaded image to the edje file */

   evas_object_image_size_get(im, &im_w, &im_h);
   im_alpha = evas_object_image_alpha_get(im);
   im_data = evas_object_image_data_get(im, 0);
   if ((!im_data) || !(im_w > 0) || !(im_h > 0))
     {
        evas_object_del(im);
        return EINA_FALSE;
     }

   /* open the eet file */
   eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     {
        evas_object_del(im);
        return EINA_FALSE;
     }

   snprintf(entry, sizeof(entry), "edje/images/%i", id);

   /* write the image data */
   bytes = eet_data_image_write(eetf, entry,
                                im_data, im_w, im_h,
                                im_alpha,
                                0, 100, 1);
   if (bytes <= 0)
     {
        ERR("Unable to write image part \"%s\" part entry to %s",
            entry, ed->path);
        _edje_edit_eet_close(eetf);
        evas_object_del(im);
        return EINA_FALSE;
     }

   evas_object_del(im);

   /* write the edje_file */
   if (!_edje_edit_edje_file_save(eetf, ed->file))
     {
        eet_delete(eetf, entry);
        _edje_edit_eet_close(eetf);
        return EINA_FALSE;
     }

   _edje_edit_eet_close(eetf);
   return EINA_TRUE;
}

static int
_edje_part_id_find(Edje *ed, const char *part)
{
   unsigned int id;

   for (id = 0; id < ed->table_parts_size; id++)
     {
        Edje_Real_Part *rp = ed->table_parts[id];

        if (!strcmp(rp->part->name, part))
          return id;
     }
   return -1;
}

static const char *
_edje_part_name_find(Edje *ed, int id)
{
   unsigned int i;
   if (id < 0) return NULL;

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp = ed->table_parts[i];
        if (rp->part->id == id)
          return rp->part->name;
     }
   return NULL;
}

static void
_edje_part_description_id_set(int type, Edje_Part_Description_Common *c, int old_id, int new_id)
{
   if (c->rel1.id_x == old_id) c->rel1.id_x = new_id;
   if (c->rel1.id_y == old_id) c->rel1.id_y = new_id;
   if (c->rel2.id_x == old_id) c->rel2.id_x = new_id;
   if (c->rel2.id_y == old_id) c->rel2.id_y = new_id;
   if (c->clip_to_id == old_id) c->clip_to_id = new_id;

   if (type == EDJE_PART_TYPE_TEXT
       || type == EDJE_PART_TYPE_TEXTBLOCK)
     {
        Edje_Part_Description_Text *t;

        t = (Edje_Part_Description_Text *)c;

        if (t->text.id_source == old_id) t->text.id_source = new_id;
        if (t->text.id_text_source == old_id) t->text.id_text_source = new_id;
     }
}

static void
_edje_part_program_id_set(Edje_Program *epr, int old_id, int new_id)
{
   Edje_Program_Target *pt;
   Eina_List *ll, *l_next;

   if (epr->action != EDJE_ACTION_TYPE_STATE_SET)
     return;

   EINA_LIST_FOREACH_SAFE(epr->targets, ll, l_next, pt)
     {
        if (pt->id == old_id)
          {
             if (new_id == -1)
               epr->targets = eina_list_remove_list(epr->targets, ll);
             else
               pt->id = new_id;
          }
     }
}

static void
_edje_part_id_set(Edje *ed, Edje_Real_Part *rp, int new_id)
{
   /* This function change the id of a given real_part.
    * All the depedency will be updated too.
    * Also the table_parts is updated, and the current *rp in the table
    * is lost.
    * If new Id = -1 then all the depencies will be deleted
    */
   Edje_Part *part;
   unsigned int j;
   int old_id;

   part = rp->part;

   if (!part) return;
   //printf("CHANGE ID OF PART %s TO %d\n", part->name, new_id);

   if (!ed || new_id < -1) return;

   if (part->id == new_id) return;

   old_id = part->id;
   part->id = new_id;

   /* Fix all the dependecies in all parts... */
   for (j = 0; j < ed->collection->parts_count; ++j)
     {
        Edje_Part *p;
        unsigned int k;

        p = ed->collection->parts[j];

        //printf("   search id: %d in %s\n", old_id, p->name);
        if (p->clip_to_id == old_id) p->clip_to_id = new_id;
        if (p->dragable.confine_id == old_id) p->dragable.confine_id = new_id;

        /* ...in default description */
        _edje_part_description_id_set(p->type, p->default_desc, old_id, new_id);

        /* ...and in all other descriptions */
        for (k = 0; k < p->other.desc_count; ++k)
          _edje_part_description_id_set(p->type, p->other.desc[k], old_id, new_id);
     }

   /*...and also in programs targets */
#define EDJE_EDIT_PROGRAM_ID_SET(Array, Ed, It, Old, New)         \
  for (It = 0; It < Ed->collection->programs.Array##_count; ++It) \
    _edje_part_program_id_set(Ed->collection->programs.Array[It], Old, New);

   EDJE_EDIT_PROGRAM_ID_SET(fnmatch, ed, j, old_id, new_id);
   EDJE_EDIT_PROGRAM_ID_SET(strcmp, ed, j, old_id, new_id);
   EDJE_EDIT_PROGRAM_ID_SET(strncmp, ed, j, old_id, new_id);
   EDJE_EDIT_PROGRAM_ID_SET(strrncmp, ed, j, old_id, new_id);
   EDJE_EDIT_PROGRAM_ID_SET(nocmp, ed, j, old_id, new_id);

   /* Adjust table_parts */
   if (new_id >= 0)
     ed->table_parts[new_id] = rp;
}

static void
_edje_part_description_id_switch(int type, Edje_Part_Description_Common *c, int id1, int id2)
{
   if (c->rel1.id_x == id1)
     c->rel1.id_x = id2;
   else if (c->rel1.id_x == id2)
     c->rel1.id_x = id1;
   if (c->rel1.id_y == id1)
     c->rel1.id_y = id2;
   else if (c->rel1.id_y == id2)
     c->rel1.id_y = id1;
   if (c->rel2.id_x == id1)
     c->rel2.id_x = id2;
   else if (c->rel2.id_x == id2)
     c->rel2.id_x = id1;
   if (c->rel2.id_y == id1)
     c->rel2.id_y = id2;
   else if (c->rel2.id_y == id2)
     c->rel2.id_y = id1;
   if (c->clip_to_id == id1)
     c->clip_to_id = id2;
   else if (c->clip_to_id == id2)
     c->clip_to_id = id1;

   if (type == EDJE_PART_TYPE_TEXT
       || type == EDJE_PART_TYPE_TEXTBLOCK)
     {
        Edje_Part_Description_Text *t;
        t = (Edje_Part_Description_Text *)c;

        if (t->text.id_source == id1) t->text.id_source = id2;
        else if (t->text.id_source == id2)
          t->text.id_source = id1;
        if (t->text.id_text_source == id1) t->text.id_text_source = id2;
        else if (t->text.id_text_source == id2)
          t->text.id_text_source = id1;
     }

   if (c->map.id_persp == id1)
     c->map.id_persp = id2;
   else if (c->map.id_persp == id2)
     c->map.id_persp = id1;

   if (c->map.id_light == id1)
     c->map.id_light = id2;
   else if (c->map.id_light == id2)
     c->map.id_light = id1;


   if (c->map.rot.id_center == id1)
     c->map.rot.id_center = id2;
   else if (c->map.rot.id_center == id2)
     c->map.rot.id_center = id1;
}

static void
_edje_part_program_id_switch(Edje_Program *epr, int id1, int id2)
{
   Edje_Program_Target *pt;
   Eina_List *ll;

   if (epr->action != EDJE_ACTION_TYPE_STATE_SET)
     return;

   EINA_LIST_FOREACH(epr->targets, ll, pt)
     {
        if (pt->id == id1) pt->id = id2;
        else if (pt->id == id2)
          pt->id = id1;
     }
}

static void
_edje_parts_id_switch(Edje *ed, Edje_Real_Part *rp1, Edje_Real_Part *rp2)
{
   /* This function switch the id of two parts.
    * All the depedency will be updated too.
    * Also the table_parts is updated,
    * The parts list isn't touched
    */
   int id1;
   int id2;
   unsigned int i;

   //printf("SWITCH ID OF PART %d AND %d\n", rp1->part->id, rp2->part->id);

   if (!ed || !rp1 || !rp2) return;
   if (rp1 == rp2) return;

   id1 = rp1->part->id;
   id2 = rp2->part->id;

   /* Switch ids */
   rp1->part->id = id2;
   rp2->part->id = id1;

   /* adjust table_parts */
   ed->table_parts[id1] = rp2;
   ed->table_parts[id2] = rp1;

   // Fix all the dependecies in all parts...
   for (i = 0; i < ed->collection->parts_count; ++i)
     {
        unsigned int j;
        Edje_Part *p;

        p = ed->collection->parts[i];

        //printf("   search id: %d in %s\n", old_id, p->name);
        if (p->clip_to_id == id1) p->clip_to_id = id2;
        else if (p->clip_to_id == id2)
          p->clip_to_id = id1;
        if (p->dragable.confine_id == id1) p->dragable.confine_id = id2;
        else if (p->dragable.confine_id == id2)
          p->dragable.confine_id = id1;

        // ...in default description
        _edje_part_description_id_switch(p->type, p->default_desc, id1, id2);

        // ...and in all other descriptions
        for (j = 0; j < p->other.desc_count; ++j)
          _edje_part_description_id_switch(p->type, p->other.desc[j], id1, id2);
     }

   //...and also in programs targets
#define EDJE_EDIT_PROGRAM_SWITCH(Array, Ed, It, Id1, Id2)         \
  for (It = 0; It < Ed->collection->programs.Array##_count; ++It) \
    _edje_part_program_id_switch(Ed->collection->programs.Array[i], Id1, Id2);

   EDJE_EDIT_PROGRAM_SWITCH(fnmatch, ed, i, id1, id2);
   EDJE_EDIT_PROGRAM_SWITCH(strcmp, ed, i, id1, id2);
   EDJE_EDIT_PROGRAM_SWITCH(strncmp, ed, i, id1, id2);
   EDJE_EDIT_PROGRAM_SWITCH(strrncmp, ed, i, id1, id2);
   EDJE_EDIT_PROGRAM_SWITCH(nocmp, ed, i, id1, id2);
   //TODO Real part dependencies are ok?
}

static void
_edje_fix_parts_id(Edje *ed)
{
   /* We use this to clear the id hole leaved when a part is removed.
    * After the execution of this function all parts will have a right
    * (uniqe & ordered) id. The table_parts is also updated.
    */
   unsigned int i;
   int correct_id;
   unsigned short count;

   //printf("FIXING PARTS ID \n");

   //TODO order the list first to be more robust

   /* Give a correct id to all the parts */
   correct_id = 0;
   for (i = 0; i < ed->collection->parts_count; ++i)
     {
        Edje_Part *p;

        p = ed->collection->parts[i];

        //printf(" [%d]Checking part: %s id: %d\n", correct_id, p->name, p->id);
        if (p->id != correct_id)
          if (ed->table_parts[p->id])
            _edje_part_id_set(ed, ed->table_parts[p->id], correct_id);

        correct_id++;
     }

   /* If we have removed some parts realloc table_parts */
   count = (unsigned short)ed->collection->parts_count;
   if (count != ed->table_parts_size)
     {
        ed->table_parts = realloc(ed->table_parts, sizeof(Edje_Real_Part *) * count);
        ed->table_parts_size = count;
     }

   //printf("\n");
}

static void
_edje_if_string_free(Edje *ed, const char **str)
{
   Eet_Dictionary *dict;

   if (!ed || !str) return;
   if (!ed->file->allocated_strings) goto the_end;

   dict = eet_dictionary_get(ed->file->ef);

   if (!eet_dictionary_string_check(dict, *str))
     eina_stringshare_del(*str);

 the_end:
   *str = NULL;
}

static void
_edje_if_string_replace(Edje *ed, const char **str, const char *str_new)
{
   Eet_Dictionary *dict;

   if (!ed || !str) return;

   ed->file->allocated_strings = EINA_TRUE;
   dict = eet_dictionary_get(ed->file->ef);
   if (*str && eet_dictionary_string_check(dict, *str))
     *str = eina_stringshare_add(str_new);
   else
     eina_stringshare_replace(str, str_new);
}

static Edje_Style *
_edje_edit_style_get(Edje *ed, const char *name)
{
   Eina_List *l;
   Edje_Style *s;

   if (!ed || !ed->file || !ed->file->styles || !name)
     return NULL;

   EINA_LIST_FOREACH(ed->file->styles, l, s)
     if (s->name && !strcmp(s->name, name))
       return s;

   return NULL;
}

static Edje_Style_Tag *
_edje_edit_style_tag_get(Edje *ed, const char *style, const char *name)
{
   Eina_List *l;
   Edje_Style *s;
   Edje_Style_Tag *t;

   if (!ed || !ed->file || !ed->file->styles || !name)
     return NULL;

   s = _edje_edit_style_get(ed, style);

   EINA_LIST_FOREACH(s->tags, l, t)
     if (t->key && !strcmp(t->key, name))
       return t;

   return NULL;
}

static Edje_External_Directory_Entry *
_edje_edit_external_get(Edje *ed, const char *name)
{
   unsigned int i;

   if (!ed || !ed->file || !ed->file->external_dir || !name)
     return NULL;

   for (i = 0; i < ed->file->external_dir->entries_count; ++i)
     if (ed->file->external_dir->entries[i].entry
         && !strcmp(ed->file->external_dir->entries[i].entry, name))
       return ed->file->external_dir->entries + i;

   return NULL;
}

static void
_edje_edit_group_references_update(Evas_Object *obj, const char *old_group_name, const char *new_group_name)
{
   Eina_Iterator *i;
   Eina_List *pll, *pl;
//   Edje_Part_Collection *pc;
   Edje_Part_Collection_Directory_Entry *pce;
   char *part_name;
   const char *source, *old;
   Edje_Part_Type type;
   Evas_Object *part_obj;

   GET_ED_OR_RETURN();

//   pc = ed->collection;

   part_obj = edje_edit_object_add(ed->base.evas);

   old = eina_stringshare_add(old_group_name);

   i = eina_hash_iterator_data_new(ed->file->collection);

   EINA_ITERATOR_FOREACH(i, pce)
     {
        edje_object_file_set(part_obj, ed->file->path, pce->entry);

        pl = edje_edit_parts_list_get(part_obj);

        EINA_LIST_FOREACH(pl, pll, part_name)
          {
             source = edje_edit_part_source_get(part_obj, part_name);
             type = edje_edit_part_type_get(part_obj, part_name);

             if (type == EDJE_PART_TYPE_GROUP && source == old)
               edje_edit_part_source_set(part_obj, part_name, new_group_name);

             if (source)
               eina_stringshare_del(source);
          }
     }

   eina_iterator_free(i);

   eina_stringshare_del(old);

   evas_object_del(part_obj);
}

static void
_edje_edit_flag_script_dirty(Edje_Edit *eed, Eina_Bool all)
{
   eed->script_need_recompile = EINA_TRUE;
   if (all)
     eed->all_dirty = EINA_TRUE;
}

/*****************/
/*  GENERAL API  */
/*****************/

EAPI void
edje_edit_string_list_free(Eina_List *lst)
{
   //printf("FREE LIST: \n");
   while (lst)
     {
        if (eina_list_data_get(lst)) eina_stringshare_del(eina_list_data_get(lst));
        //printf("FREE: %s\n", eina_list_data_get(lst));
        lst = eina_list_remove(lst, eina_list_data_get(lst));
     }
}

EAPI void
edje_edit_string_free(const char *str)
{
   if (str) eina_stringshare_del(str);
}

EAPI const char *
edje_edit_compiler_get(Evas_Object *obj)
{
   GET_ED_OR_RETURN(0);
   return eina_stringshare_add(ed->file->compiler);
}

/****************/
/*  SOUNDS API  */
/****************/

EAPI Eina_List *
edje_edit_sound_samples_list_get(Evas_Object *obj)
{
   Eina_List *sounds_samples = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(NULL);

   if ((!ed->file) || (!ed->file->sound_dir))
     return NULL;

   for (i = 0; i < ed->file->sound_dir->samples_count; ++i)
     sounds_samples = eina_list_append(sounds_samples,
                                       eina_stringshare_add(ed->file->sound_dir->samples[i].name));

   return sounds_samples;
}

EAPI Eina_List *
edje_edit_sound_tones_list_get(Evas_Object *obj)
{
   Eina_List *sounds_tones = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(NULL);

   if ((!ed->file) || (!ed->file->sound_dir))
     return NULL;

   for (i = 0; i < ed->file->sound_dir->tones_count; ++i)
     sounds_tones = eina_list_append(sounds_tones,
                                     eina_stringshare_add(ed->file->sound_dir->tones[i].name));

   return sounds_tones;
}

static Eina_Bool _edje_edit_collection_save(Eet_File *eetf, Edje_Part_Collection *epc);

static Eina_Bool
_delete_play_actions(Evas_Object *obj, const char *name, int action_type, Eet_File *eetf)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   Eina_Iterator *it = eina_hash_iterator_data_new(ed->file->collection);
   Edje_Part_Collection_Directory_Entry *pce;

   EINA_ITERATOR_FOREACH(it, pce)
     {
        Eina_Bool is_collection_changed = EINA_FALSE;
        Evas_Object *eeo;
        Eina_List *programs_list;
        Edje *eed;
        int i;

        if (pce->group_alias)
          continue;

        eeo = edje_edit_object_add(ed->base.evas);
        if (!efl_isa(eeo, EFL_CANVAS_LAYOUT_CLASS))
          return EINA_FALSE;

        if (!edje_object_file_set(eeo, ed->file->path, pce->entry))
          {
             evas_object_del(eeo);
             continue;
          }

        programs_list = edje_edit_programs_list_get(eeo);
        if (!programs_list)
          {
             evas_object_del(eeo);
             continue;
          }

        eed = efl_data_scope_get(eeo, EFL_CANVAS_LAYOUT_CLASS);
        for (i = 0; i < eed->collection->patterns.table_programs_size; i++)
          {
             Edje_Program *program;

             program = eed->collection->patterns.table_programs[i];
             if (program->action != action_type)
               continue;

             if ((action_type == EDJE_ACTION_TYPE_SOUND_SAMPLE) &&
                 !strcmp(program->sample_name, name))
               {
                  program->speed = 0;
                  program->channel = EDJE_CHANNEL_EFFECT;
                  _edje_if_string_free(eed, &program->sample_name);
                  program->action = EDJE_ACTION_TYPE_NONE;
                  is_collection_changed = EINA_TRUE;
               }
             else if ((action_type == EDJE_ACTION_TYPE_SOUND_TONE) &&
                      !strcmp(program->tone_name, name))
               {
                  program->duration = 0;
                  _edje_if_string_free(eed, &program->tone_name);
                  program->action = EDJE_ACTION_TYPE_NONE;
                  is_collection_changed = EINA_TRUE;
               }
          }
        if (is_collection_changed)
          _edje_edit_collection_save(eetf, eed->collection);
        evas_object_del(eeo);
     }

   eina_iterator_free(it);
   return EINA_TRUE;
}

static void
_initialize_sound_dir(Edje *ed)
{
   if (ed->file->sound_dir)
     return;
   ed->file->sound_dir = _alloc(sizeof(Edje_Sound_Directory));
   ed->file->sound_dir->samples = NULL;
   ed->file->sound_dir->tones = NULL;
   ed->file->sound_dir->samples_count = 0;
   ed->file->sound_dir->tones_count = 0;
}

#define GET_TONE_BY_NAME(_tone_p, _name)                         \
  {                                                              \
     unsigned int i = 0;                                         \
     for (i = 0; i < ed->file->sound_dir->tones_count; ++i)      \
       {                                                         \
          _tone_p = ed->file->sound_dir->tones + i;              \
          if (!strcmp(_name, _tone_p->name))                     \
            break;                                               \
       }                                                         \
     if (i == ed->file->sound_dir->tones_count) _tone_p = NULL;  \
  }

EAPI Eina_Bool
edje_edit_sound_sample_add(Evas_Object *obj, const char *name, const char *snd_src)
{
   if (!name) return EINA_FALSE;
   if (!snd_src) return EINA_FALSE;

   GET_ED_OR_RETURN(EINA_FALSE);

   Edje_Sound_Sample *sound_sample = NULL;
   unsigned int i = 0;
   char sample[PATH_MAX];
   int id = 0;

   _initialize_sound_dir(ed);

   for (i = 0; i < ed->file->sound_dir->samples_count; ++i)
     {
        sound_sample = ed->file->sound_dir->samples + i;
        if (!strcmp(name, sound_sample->name))
          {
             WRN("Can not add new sound sample because"
                 "sample named \"%s\" already exists.", name);
             return EINA_FALSE;
          }
     }

   if (ed->file->sound_dir->samples)
     {
        sound_sample = ed->file->sound_dir->samples +
          ed->file->sound_dir->samples_count - 1;
        id = sound_sample->id + 1;
        snprintf(sample, sizeof(sample), "edje/sounds/%i", id);
     }
   else
     strcpy(sample, "edje/sounds/0");

   if (!_edje_edit_file_import(ed, snd_src, sample, 0))
     return EINA_FALSE;

   if (sound_sample)
     sound_sample++;
   ++ed->file->sound_dir->samples_count;

   ed->file->sound_dir->samples = realloc(ed->file->sound_dir->samples,
                                          sizeof(Edje_Sound_Sample) *
                                          ed->file->sound_dir->samples_count);

   sound_sample = ed->file->sound_dir->samples +
     ed->file->sound_dir->samples_count - 1;
   sound_sample->name = (char *)eina_stringshare_add(name);
   sound_sample->snd_src = (char *)eina_stringshare_add(ecore_file_file_get(snd_src));
   sound_sample->compression = EDJE_SOUND_SOURCE_TYPE_INLINE_RAW;
   sound_sample->id = id;
   sound_sample->mode = 0;
   sound_sample->quality = 0;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_sound_sample_del(Evas_Object *obj, const char *name)
{
   Edje_Sound_Sample *sound_sample = NULL;
   unsigned int i = 0;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name) return EINA_FALSE;
   if (!ed->file) return EINA_FALSE;
   if (!ed->path) return EINA_FALSE;

   if ((!ed->file->sound_dir) || (!ed->file->sound_dir->samples))
     {
        WRN("Unable to delete sample \"%s\". The samples list is empty.", name);
        return EINA_FALSE;
     }

   for (i = 0; i < ed->file->sound_dir->samples_count; ++i)
     {
        sound_sample = ed->file->sound_dir->samples + i;
        if (!strcmp(name, sound_sample->name))
          break;
     }
   if (i == ed->file->sound_dir->samples_count)
     {
        WRN("Unable to delete sample \"%s\". It does not exist.", name);
        return EINA_FALSE;
     }

   {
      char sample[PATH_MAX];
      Eet_File *eetf;
      Edje_Sound_Sample *sound_sample_last;

      eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
      if (!eetf)
        return EINA_FALSE;

      snprintf(sample, sizeof(sample), "edje/sounds/%i", sound_sample->id);

      if (eet_delete(eetf, sample) <= 0)
        {
           WRN("Unable to delete \"%s\" sound", sample);
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }

      _edje_if_string_free(ed, &sound_sample->name);
      --ed->file->sound_dir->samples_count;

      sound_sample_last = ed->file->sound_dir->samples +
        ed->file->sound_dir->samples_count - 1;

      while (sound_sample <= sound_sample_last)
        {
           *sound_sample = *(sound_sample + 1);
           sound_sample++;
        }

      ed->file->sound_dir->samples = realloc(ed->file->sound_dir->samples,
                                             sizeof(Edje_Sound_Sample) *
                                             ed->file->sound_dir->samples_count);

      if (!_delete_play_actions(obj, name, EDJE_ACTION_TYPE_SOUND_SAMPLE, eetf))
        {
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }

      if (!_edje_edit_edje_file_save(eetf, ed->file))
        {
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }
      _edje_edit_eet_close(eetf);
   }

   GET_EED_OR_RETURN(EINA_FALSE);
   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_sound_tone_add(Evas_Object *obj, const char *name, int frequency)
{
   if (!name) return EINA_FALSE;
   if ((frequency < 20) || (frequency > 20000))
     return EINA_FALSE;

   GET_ED_OR_RETURN(EINA_FALSE);

   Edje_Sound_Tone *sound_tone = NULL;
   Edje_Sound_Tone *sound_tones_array = NULL;
   unsigned int i = 0;
   int id = 0;

   _initialize_sound_dir(ed);

   for (i = 0; i < ed->file->sound_dir->tones_count; ++i)
     {
        sound_tone = ed->file->sound_dir->tones + i;
        if (!strcmp(name, sound_tone->name))
          {
             WRN("Can not add new tone because"
                 "tone named \"%s\" already exists.", name);
             return EINA_FALSE;
          }
     }
   ed->file->sound_dir->tones_count++;

   sound_tones_array = realloc(ed->file->sound_dir->tones,
                               sizeof(Edje_Sound_Tone) *
                               ed->file->sound_dir->tones_count);
   if (sound_tones_array)
     ed->file->sound_dir->tones = sound_tones_array;
   else return EINA_FALSE;

   sound_tone = ed->file->sound_dir->tones +
     ed->file->sound_dir->tones_count - 1;
   sound_tone->name = (char *)eina_stringshare_add(name);
   sound_tone->value = frequency;
   sound_tone->id = id;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_sound_tone_del(Evas_Object *obj, const char *name)
{
   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name) return EINA_FALSE;
   if (!ed->file) return EINA_FALSE;
   if (!ed->path) return EINA_FALSE;

   if ((!ed->file->sound_dir) || (!ed->file->sound_dir->tones))
     {
        WRN("Unable to delete tone \"%s\". The tones list is empty.", name);
        return EINA_FALSE;
     }

   Edje_Sound_Tone *sound_tone;
   GET_TONE_BY_NAME(sound_tone, name);
   if (!sound_tone)
     {
        WRN("Unable to delete tone \"%s\". It does not exist.", name);
        return EINA_FALSE;
     }

   {
      Eet_File *eetf;

      eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
      if (!eetf)
        return EINA_FALSE;

      _edje_if_string_free(ed, &sound_tone->name);
      --ed->file->sound_dir->tones_count;

      Edje_Sound_Tone *sound_tone_last = ed->file->sound_dir->tones +
        ed->file->sound_dir->tones_count - 1;

      while (sound_tone <= sound_tone_last)
        {
           *sound_tone = *(sound_tone + 1);
           sound_tone++;
        }

      ed->file->sound_dir->tones = realloc(ed->file->sound_dir->tones,
                                           sizeof(Edje_Sound_Tone) *
                                           ed->file->sound_dir->tones_count);

      if (!_delete_play_actions(obj, name, EDJE_ACTION_TYPE_SOUND_TONE, eetf))
        {
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }

      if (!_edje_edit_edje_file_save(eetf, ed->file))
        {
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }
      _edje_edit_eet_close(eetf);
   }

   GET_EED_OR_RETURN(EINA_FALSE);
   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_sound_tone_frequency_set(Evas_Object *obj, const char *name, int frequency)
{
   Edje_Sound_Tone *tone;
   if ((frequency < 20) || (frequency > 20000)) return EINA_FALSE;
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_TONE_BY_NAME(tone, name);
   if (tone)
     {
        tone->value = frequency;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EAPI int
edje_edit_sound_tone_frequency_get(Evas_Object *obj, const char *name)
{
   Edje_Sound_Tone *tone;
   GET_ED_OR_RETURN(-1);
   GET_TONE_BY_NAME(tone, name);
   if (tone)
     return tone->value;
   return -1;
}

EAPI double
edje_edit_sound_compression_rate_get(Evas_Object *obj, const char *sound)
{
   Edje_Sound_Sample *ss = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(-1);

   if ((!ed->file) || (!ed->file->sound_dir))
     return -1;

   for (i = 0; i < ed->file->sound_dir->samples_count; i++)
     {
        ss = ed->file->sound_dir->samples + i;
        if ((ss->name) && (!strcmp(sound, ss->name)))
          break;
     }

   if (i == ed->file->sound_dir->samples_count)
     return -1;

   return ss->quality;
}

EAPI Eina_Bool
edje_edit_sound_compression_rate_set(Evas_Object *obj, const char *sound, double rate)
{
   Edje_Sound_Sample *ss = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(EINA_FALSE);

   if ((!ed->file) || (!ed->file->sound_dir))
     return EINA_FALSE;

   for (i = 0; i < ed->file->sound_dir->samples_count; i++)
     {
        ss = ed->file->sound_dir->samples + i;
        if ((ss->name) && (!strcmp(sound, ss->name)))
          break;
     }

   if (i == ed->file->sound_dir->samples_count)
     return EINA_FALSE;

   ss->quality = rate;
   return EINA_TRUE;
}

#undef GET_TONE_BY_NAME

EAPI Edje_Edit_Sound_Comp
edje_edit_sound_compression_type_get(Evas_Object *obj, const char *sound)
{
   Edje_Sound_Sample *ss = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(-1);

   if ((!ed->file) || (!ed->file->sound_dir))
     return -1;

   for (i = 0; i < ed->file->sound_dir->samples_count; i++)
     {
        ss = ed->file->sound_dir->samples + i;
        if ((ss->name) && (!strcmp(sound, ss->name)))
          break;
     }

   if (i == ed->file->sound_dir->samples_count)
     return -1;

   return (Edje_Edit_Sound_Comp)ss->compression;
}

EAPI Eina_Bool
edje_edit_sound_compression_type_set(Evas_Object *obj, const char *sound, Edje_Edit_Sound_Comp sc)
{
   Edje_Sound_Sample *ss = NULL;
   unsigned int i;

   if ((sc <= EDJE_EDIT_SOUND_COMP_NONE) ||
       (sc > EDJE_EDIT_SOUND_COMP_AS_IS))
     return EINA_FALSE;
   GET_ED_OR_RETURN(EINA_FALSE);

   if ((!ed->file) || (!ed->file->sound_dir))
     return EINA_FALSE;

   for (i = 0; i < ed->file->sound_dir->samples_count; i++)
     {
        ss = ed->file->sound_dir->samples + i;
        if ((ss->name) && (!strcmp(sound, ss->name)))
          break;
     }

   if (i == ed->file->sound_dir->samples_count)
     return EINA_FALSE;

   ss->compression = (int)sc;
   return EINA_TRUE;
}

EAPI Eina_Binbuf *
edje_edit_sound_samplebuffer_get(Evas_Object *obj, const char *sample_name)
{
   Eet_File *ef;
   Edje_Sound_Sample *sample;
   char snd_id_str[PATH_MAX];
   int i, len;
   const void *data;

   if (!sample_name)
     {
        ERR("Given Sample Name is NULL\n");
        return NULL;
     }

   GET_ED_OR_RETURN(NULL);

   if ((!ed) || (!ed->file) || (!ed->file->sound_dir))
     return NULL;

   for (i = 0; i < (int)ed->file->sound_dir->samples_count; i++)
     {
        sample = &ed->file->sound_dir->samples[i];
        if (!strcmp(sample->name, sample_name))
          {
             ef = _edje_edit_eet_open(ed, EET_FILE_MODE_READ);
             if (!ef)
               return NULL;
             snprintf(snd_id_str, sizeof(snd_id_str), "edje/sounds/%i", sample->id);
             data = eet_read(ef, snd_id_str, &len);
             if (len <= 0)
               {
                  ERR("Sample from edj file '%s' has 0 length", ed->path);
                  _edje_edit_eet_close(ef);
                  return NULL;
               }
             _edje_edit_eet_close(ef);
             return eina_binbuf_manage_new(data, len, EINA_TRUE);
          }
     }
   return NULL;
}

EAPI const char *
edje_edit_sound_samplesource_get(Evas_Object *obj, const char *sample_name)
{
   Edje_Sound_Sample *sample;
   int i;

   if (!sample_name)
     {
        ERR("Given Sample Name is NULL");
        return NULL;
     }

   GET_ED_OR_RETURN(NULL);

   if ((!ed) || (!ed->file) || (!ed->file->sound_dir))
     return NULL;

   for (i = 0; i < (int)ed->file->sound_dir->samples_count; i++)
     {
        sample = &ed->file->sound_dir->samples[i];
        if (!strcmp(sample->name, sample_name))
          return eina_stringshare_add(sample->snd_src);
     }

   return NULL;
}

/****************/
/*  GROUPS API  */
/****************/

static void
_mempools_add(Edje_Part_Collection_Directory_Entry *de)
{
   de->mp = calloc(1, sizeof(Edje_Part_Collection_Directory_Entry_Mp));
   if (!de->mp)
     return;
#define EDIT_EMN(Tp, Sz, Ce) \
  Ce->mp->mp.Tp = eina_mempool_add("chained_mempool", #Tp, NULL, sizeof (Sz), 8);
#define EDIT_EMNP(Tp, Sz, Ce) \
  Ce->mp->mp_rtl.Tp = eina_mempool_add("chained_mempool", #Tp, NULL, sizeof (Sz), 8);

   EDIT_EMN(RECTANGLE, Edje_Part_Description_Common, de);
   EDIT_EMN(TEXT, Edje_Part_Description_Text, de);
   EDIT_EMN(IMAGE, Edje_Part_Description_Image, de);
   EDIT_EMN(PROXY, Edje_Part_Description_Proxy, de);
   EDIT_EMN(SWALLOW, Edje_Part_Description_Common, de);
   EDIT_EMN(TEXTBLOCK, Edje_Part_Description_Text, de);
   EDIT_EMN(GROUP, Edje_Part_Description_Common, de);
   EDIT_EMN(BOX, Edje_Part_Description_Box, de);
   EDIT_EMN(TABLE, Edje_Part_Description_Table, de);
   EDIT_EMN(EXTERNAL, Edje_Part_Description_External, de);
   EDIT_EMN(SPACER, Edje_Part_Description_Common, de);
   EDIT_EMN(SNAPSHOT, Edje_Part_Description_Snapshot, de);
   EDIT_EMN(MESH_NODE, Edje_Part_Description_Mesh_Node, de);
   EDIT_EMN(LIGHT, Edje_Part_Description_Light, de);
   EDIT_EMN(CAMERA, Edje_Part_Description_Camera, de);
   EDIT_EMN(part, Edje_Part, de);

   EDIT_EMNP(RECTANGLE, Edje_Part_Description_Common, de);
   EDIT_EMNP(TEXT, Edje_Part_Description_Text, de);
   EDIT_EMNP(IMAGE, Edje_Part_Description_Image, de);
   EDIT_EMNP(PROXY, Edje_Part_Description_Proxy, de);
   EDIT_EMNP(SWALLOW, Edje_Part_Description_Common, de);
   EDIT_EMNP(TEXTBLOCK, Edje_Part_Description_Text, de);
   EDIT_EMNP(GROUP, Edje_Part_Description_Common, de);
   EDIT_EMNP(BOX, Edje_Part_Description_Box, de);
   EDIT_EMNP(TABLE, Edje_Part_Description_Table, de);
   EDIT_EMNP(EXTERNAL, Edje_Part_Description_External, de);
   EDIT_EMNP(SPACER, Edje_Part_Description_Common, de);
   EDIT_EMNP(SNAPSHOT, Edje_Part_Description_Snapshot, de);
   EDIT_EMNP(MESH_NODE, Edje_Part_Description_Mesh_Node, de);
   EDIT_EMNP(LIGHT, Edje_Part_Description_Light, de);
   EDIT_EMNP(CAMERA, Edje_Part_Description_Camera, de);
}

EAPI Eina_Bool
edje_edit_group_copy(Evas_Object *obj, const char *group_name, const char *copy_name)
{
   Edje_Part_Collection_Directory_Entry *e;
   Edje_Part_Collection_Directory_Entry *de;
   Edje_Part_Collection_Directory_Entry *d;
   Edje_Part_Collection *epc;
   int id;
   int search;
   Eet_File *eetf;
   char buf[64];
   int count, script_count;
   void *data;
   char **keys, **c;
   Eina_Bool save_status = EINA_FALSE;

   GET_ED_OR_RETURN(EINA_FALSE);
   if (!ed->file || !ed->file->collection)
     return EINA_FALSE;

   e = eina_hash_find(ed->file->collection, group_name);
   if (!e) return EINA_FALSE;
   if (eina_hash_find(ed->file->collection, copy_name))
     return EINA_FALSE;

   eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     return EINA_FALSE;

   snprintf(buf, sizeof(buf), "edje/collections/%d", e->id);
   epc = eet_data_read(eetf, _edje_edd_edje_part_collection, buf);
   if (!epc)
     {
        _edje_edit_eet_close(eetf);
        return EINA_FALSE;
     }

   /* Search first free id */
   id = -1;
   search = 0;
   while (id == -1)
     {
        Eina_Iterator *i;
        Eina_Bool found = 0;

        i = eina_hash_iterator_data_new(ed->file->collection);

        EINA_ITERATOR_FOREACH(i, d)
          {
             if (search == d->id)
               {
                  found = 1;
                  break;
               }
          }

        eina_iterator_free(i);

        if (!found) id = search;
        else search++;
     }

   /* Create structs */
   de = _alloc(sizeof(Edje_Part_Collection_Directory_Entry));
   if (!de)
     {
        _edje_edit_eet_close(eetf);
        return EINA_FALSE;
     }

   /* Init Edje_Part_Collection_Directory_Entry */
   de->id = id;
   de->entry = eina_stringshare_add(copy_name);
   memcpy(&de->count, &e->count, sizeof (de->count));

   eina_hash_direct_add(ed->file->collection, de->entry, de);

   _mempools_add(de);

   epc->id = id;
   epc->part = eina_stringshare_add(copy_name);
   snprintf(buf, sizeof(buf), "edje/collections/%i", epc->id);
   eet_data_write(eetf, _edje_edd_edje_part_collection, buf, epc, 1);

   /* Copying Scripts */
   snprintf(buf, sizeof(buf), "edje/scripts/embryo/compiled/%d", e->id);
   data = eet_read(eetf, buf, &count);
   snprintf(buf, sizeof(buf), "edje/scripts/embryo/compiled/%d", epc->id);
   eet_write(eetf, buf, data, count, 1);

   snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%d", e->id);
   data = eet_read(eetf, buf, &count);
   snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%d", epc->id);
   eet_write(eetf, buf, data, count, 0);

   snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%d/*", e->id);
   keys = eet_list(eetf, buf, &count);
   if (keys)
     {
        while (count)
          {
             count--;
             data = eet_read(eetf, keys[count], &script_count);
             /* we need to save id of every script we are going to copy. */
             c = eina_str_split(keys[count], "/", 6);
             snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%d/%s", epc->id, c[5]);
             eet_write(eetf, buf, data, script_count, 0);
             free(c[0]);
             free(c);
          }
        free(keys);
     }

   save_status = _edje_edit_edje_file_save(eetf, ed->file);

   _edje_collection_free(ed->file, epc, de);
   _edje_edit_eet_close(eetf);

   return save_status;
}

EAPI Eina_Bool
edje_edit_group_add(Evas_Object *obj, const char *name)
{
   Edje_Part_Collection_Directory_Entry *de;
   Edje_Part_Collection_Directory_Entry *d;
   Edje_Part_Collection *pc;
   int id;
   int search;
   //Code *cd;

   GET_ED_OR_RETURN(EINA_FALSE);

   //printf("ADD GROUP: %s \n", name);

   /* check if a group with the same name already exists */
   if (eina_hash_find(ed->file->collection, name))
     return EINA_FALSE;

   /* Create structs */
   de = _alloc(sizeof(Edje_Part_Collection_Directory_Entry));
   if (!de) return EINA_FALSE;

   pc = _alloc(sizeof(Edje_Part_Collection));
   if (!pc)
     {
        free(de);
        return EINA_FALSE;
     }

   /* Search first free id */
   id = -1;
   search = 0;
   while (id == -1)
     {
        Eina_Iterator *i;
        Eina_Bool found = 0;

        i = eina_hash_iterator_data_new(ed->file->collection);

        EINA_ITERATOR_FOREACH(i, d)
          {
             // printf("search if %d is free [id %d]\n", search, d->id);
             if (search == d->id)
               {
                  found = 1;
                  break;
               }
          }

        eina_iterator_free(i);

        if (!found) id = search;
        else search++;
     }

   /* Init Edje_Part_Collection_Directory_Entry */
   //printf(" new id: %d\n", id);
   de->id = id;
   de->entry = eina_stringshare_add(name);
   eina_hash_direct_add(ed->file->collection, de->entry, de);

   /* Init Edje_Part_Collection */
   pc->id = id;
   pc->references = 0;
   memset(&pc->programs, 0, sizeof (pc->programs));
   pc->parts = NULL;
   pc->data = NULL;
   pc->script = NULL;
   pc->part = eina_stringshare_add(name);

   //cd = _alloc(sizeof(Code));
   //codes = eina_list_append(codes, cd);

   _mempools_add(de);

   ed->file->collection_cache = eina_list_prepend(ed->file->collection_cache, pc);
   _edje_cache_coll_clean(ed->file);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_group_del(Evas_Object *obj, const char *group_name)
{
   Edje_Part_Collection_Directory_Entry *e, *e_del;
   Edje_Part_Collection *die = NULL;
   Edje_Part_Collection *g;
   Eina_List *l;
   Eet_File *eetf;
   char buf[64], **keys;
   int count;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (strcmp(ed->group, group_name) == 0) return EINA_FALSE;
   e = eina_hash_find(ed->file->collection, group_name);
   if (!e) return EINA_FALSE;
   if (e->id == ed->collection->id) return EINA_FALSE;
   if (e->ref) return EINA_FALSE;

   _edje_edit_group_references_update(obj, group_name, NULL);

   EINA_LIST_FOREACH(ed->file->collection_cache, l, g)
     {
        if (strcmp(g->part, e->entry) == 0)
          {
             ed->file->collection_cache =
               eina_list_remove_list(ed->file->collection_cache, l);
             die = g;
             break;
          }
     }

   /* Remove collection/id from eet file */
   eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     return EINA_FALSE;

   snprintf(buf, sizeof(buf), "edje/collections/%d", e->id);
   eet_delete(eetf, buf);
   snprintf(buf, sizeof(buf), "edje/scripts/embryo/compiled/%d", e->id);
   eet_delete(eetf, buf);
   snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%d", e->id);
   eet_delete(eetf, buf);
   snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%d/*", e->id);
   keys = eet_list(eetf, buf, &count);
   if (keys)
     {
        do
          {
             count--;
             eet_delete(eetf, keys[count]);
          }
        while (count);
        free(keys);
     }
   _edje_edit_eet_close(eetf);

   l = NULL; g = NULL;
   /* Free Group and all it's Aliases */
   if (!e->group_alias)
     {
        EINA_LIST_FOREACH(ed->file->collection_cache, l, g)
          {
             if (e->id == g->id)
               {
                  ed->file->collection_cache =
                    eina_list_remove_list(ed->file->collection_cache, l);
                  e_del = eina_hash_find(ed->file->collection, g->part);
                  eina_hash_del(ed->file->collection, g->part, e_del);
                  _edje_collection_free(ed->file, g, e_del);
               }
          }
     }
   if (die) _edje_collection_free(ed->file, die, e);
   eina_hash_del(ed->file->collection, group_name, e);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_group_exist(Evas_Object *obj, const char *group)
{
   GET_ED_OR_RETURN(EINA_FALSE);

   if (eina_hash_find(ed->file->collection, group))
     return EINA_TRUE;
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_group_name_set(Evas_Object *obj, const char *new_name)
{
   Edje_Part_Collection_Directory_Entry *pce;
   Edje_Part_Collection *pc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!new_name) return EINA_FALSE;

   pc = ed->collection;

   if (!strcmp(pc->part, new_name)) return EINA_TRUE;

   if (edje_edit_group_exist(obj, new_name)) return EINA_FALSE;

   _edje_edit_group_references_update(obj, pc->part, new_name);

   //printf("Set name of current group: %s [id: %d][new name: %s]\n",
   // pc->part, pc->id, new_name);

   //if (pc->part && ed->file->free_strings) eina_stringshare_del(pc->part); TODO FIXME
   pce = eina_hash_find(ed->file->collection, pc->part);

   eina_hash_move(ed->file->collection, pce->entry, new_name);

   pce->entry = eina_stringshare_add(new_name);
   pc->part = pce->entry;
   eina_stringshare_replace(&ed->group, new_name);

   return EINA_TRUE;
}

#define FUNC_GROUP_ACCESSOR(Class, Value)                           \
  EAPI int                                                          \
  edje_edit_group_##Class##_##Value##_get(Evas_Object * obj)        \
  {                                                                 \
     GET_ED_OR_RETURN(-1);                                          \
     if (!ed->collection) return -1;                                \
     return ed->collection->prop.Class.Value;                       \
  }                                                                 \
  EAPI Eina_Bool                                                    \
  edje_edit_group_##Class##_##Value##_set(Evas_Object * obj, int v) \
  {                                                                 \
     GET_ED_OR_RETURN(EINA_FALSE);                                  \
     if (!ed->collection) return EINA_FALSE;                        \
     if (v < 0) return EINA_FALSE;                                  \
     ed->collection->prop.Class.Value = v;                          \
     return EINA_TRUE;                                              \
  }

FUNC_GROUP_ACCESSOR(min, w);
FUNC_GROUP_ACCESSOR(min, h);
FUNC_GROUP_ACCESSOR(max, w);
FUNC_GROUP_ACCESSOR(max, h);

EAPI unsigned char
edje_edit_group_orientation_get(Evas_Object *obj)
{
   GET_ED_OR_RETURN(-1);
   if (!ed->collection) return -1;
   return ed->collection->prop.orientation;
}

EAPI Eina_Bool
edje_edit_group_orientation_set(Evas_Object *obj, unsigned char orient)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   if (!ed->collection) return EINA_FALSE;
   if (orient > 2) return EINA_FALSE;
   ed->collection->prop.orientation = orient;
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_group_broadcast_signal_get(Evas_Object *obj)
{
   GET_ED_OR_RETURN(EINA_TRUE);
   if (!ed->collection) return EINA_TRUE;
   return ed->collection->broadcast_signal;
}

EAPI Eina_Bool
edje_edit_group_broadcast_signal_set(Evas_Object *obj, Eina_Bool bs)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   if (!ed->collection) return EINA_FALSE;
   ed->collection->broadcast_signal = bs ? 1 : 0;
   return EINA_TRUE;
}

#define LIMITS(TYPE)                                                                     \
  EAPI Eina_List *                                                                       \
  edje_edit_group_limits_##TYPE##_list_get(Evas_Object * obj)                            \
  {                                                                                      \
     Eina_List *limits = NULL;                                                           \
     unsigned int i;                                                                     \
     Edje_Edit_Limit *lim;                                                               \
                                                                                         \
     GET_ED_OR_RETURN(NULL);                                                             \
                                                                                         \
     if (!ed->file || !ed->collection)                                                   \
       return NULL;                                                                      \
     for (i = 0; i < ed->collection->limits.TYPE##_count; i++)                           \
       {                                                                                 \
          lim = malloc(sizeof(Edje_Edit_Limit));                                         \
          lim->name = eina_stringshare_add(ed->collection->limits.TYPE[i]->name);        \
          lim->value = ed->collection->limits.TYPE[i]->value;                            \
          limits = eina_list_append(limits, &lim);                                       \
       }                                                                                 \
                                                                                         \
     return limits;                                                                      \
  }                                                                                      \
                                                                                         \
  EAPI Eina_Bool                                                                         \
  edje_edit_group_limits_##TYPE##_del(Evas_Object * obj, const char *name, int value)    \
  {                                                                                      \
     unsigned int i;                                                                     \
     unsigned int new_count;                                                             \
                                                                                         \
     if ((!name) || (value < 1))                                                         \
       return EINA_FALSE;                                                                \
     GET_ED_OR_RETURN(EINA_FALSE);                                                       \
     GET_EED_OR_RETURN(EINA_FALSE);                                                      \
                                                                                         \
     new_count = ed->collection->limits.TYPE##_count - 1;                                \
     for (i = 0; i < ed->collection->limits.TYPE##_count; i++)                           \
       if ((ed->collection->limits.TYPE[i]->value == value)                              \
           && (!strcmp(ed->collection->limits.TYPE[i]->name, name)))                     \
         {                                                                               \
            _edje_if_string_free(ed, &ed->collection->limits.TYPE[i]->name);             \
            free(ed->collection->limits.TYPE[i]);                                        \
            if (i < new_count)                                                           \
              {                                                                          \
                 ed->collection->limits.TYPE[i] =                                        \
                   ed->collection->limits.TYPE[ed->collection->limits.TYPE##_count - 1]; \
              }                                                                          \
            ed->collection->limits.TYPE = realloc(ed->collection->limits.TYPE,           \
                                                  new_count * sizeof(Edje_Limit *));     \
            --ed->collection->limits.TYPE##_count;                                       \
            _edje_edit_flag_script_dirty(eed, EINA_TRUE);                                \
                                                                                         \
            return EINA_TRUE;                                                            \
         }                                                                               \
     return EINA_FALSE;                                                                  \
  }                                                                                      \
                                                                                         \
  EAPI Eina_Bool                                                                         \
  edje_edit_group_limits_##TYPE##_add(Evas_Object * obj, const char *name, int value)    \
  {                                                                                      \
     unsigned int i;                                                                     \
     unsigned int new_count;                                                             \
                                                                                         \
     if ((!name) || (value < 1))                                                         \
       return EINA_FALSE;                                                                \
     GET_ED_OR_RETURN(EINA_FALSE);                                                       \
                                                                                         \
     for (i = 0; i < ed->collection->limits.TYPE##_count; i++)                           \
       if ((ed->collection->limits.TYPE[i]->value == value)                              \
           && (!strcmp(ed->collection->limits.TYPE[i]->name, name)))                     \
         {                                                                               \
            return EINA_FALSE;                                                           \
         }                                                                               \
     new_count = ed->collection->limits.TYPE##_count + 1;                                \
     ed->collection->limits.TYPE = realloc(ed->collection->limits.TYPE,                  \
                                           new_count * sizeof(Edje_Limit *));            \
     ed->collection->limits.TYPE[new_count - 1] = malloc(sizeof(Edje_Limit));            \
     ed->collection->limits.TYPE[new_count - 1]->name = eina_stringshare_add(name);      \
     ed->collection->limits.TYPE[new_count - 1]->value = value;                          \
     ++ed->collection->limits.TYPE##_count;                                              \
     return EINA_TRUE;                                                                   \
  }

LIMITS(vertical);
LIMITS(horizontal);

EAPI void
edje_edit_limits_list_free(Eina_List *list)
{
   Edje_Edit_Limit *lim = eina_list_data_get(list);
   Edje_Edit_Limit *item;
   EINA_LIST_FREE(list, item)
     eina_stringshare_del(item->name);
   free(lim);
}

/****************/
/*  ALIAS  API  */
/****************/

EAPI Eina_List *
edje_edit_group_aliases_get(Evas_Object *obj, const char *group_name)
{
   Eina_Iterator *i;
   Edje_Part_Collection_Directory_Entry *e;
   Edje_Part_Collection_Directory_Entry *d;
   Eina_List *alias_list = NULL;

   GET_ED_OR_RETURN(NULL);
   if (!ed->file || !ed->file->collection)
     return NULL;

   e = eina_hash_find(ed->file->collection, group_name);
   if (!e) return NULL;

   i = eina_hash_iterator_data_new(ed->file->collection);
   EINA_ITERATOR_FOREACH(i, d)
     {
        if ((e->id == d->id) && (d->group_alias))
          alias_list = eina_list_append(alias_list, eina_stringshare_add(d->entry));
     }
   eina_iterator_free(i);

   return alias_list;
}

EAPI Eina_Bool
edje_edit_group_alias_is(Evas_Object *obj, const char *alias_name)
{
   Edje_Part_Collection_Directory_Entry *e;

   GET_ED_OR_RETURN(EINA_FALSE);
   if (!ed->file || !ed->file->collection)
     return EINA_FALSE;

   e = eina_hash_find(ed->file->collection, alias_name);
   if (!e) return EINA_FALSE;

   return e->group_alias;
}

EAPI const char *
edje_edit_group_aliased_get(Evas_Object *obj, const char *alias_name)
{
   Eina_Iterator *i;
   Edje_Part_Collection_Directory_Entry *e;
   Edje_Part_Collection_Directory_Entry *d;
   const char *group_name = NULL;

   GET_ED_OR_RETURN(NULL);
   if (!ed->file || !ed->file->collection)
     return NULL;

   e = eina_hash_find(ed->file->collection, alias_name);
   if (!e) return NULL;
   if (!e->group_alias) return eina_stringshare_add(alias_name);

   i = eina_hash_iterator_data_new(ed->file->collection);
   EINA_ITERATOR_FOREACH(i, d)
     {
        if ((e->id == d->id) && (!d->group_alias))
          {
             group_name = d->entry;
             break;
          }
     }
   eina_iterator_free(i);

   return eina_stringshare_add(group_name);
}

EAPI Eina_Bool
edje_edit_group_alias_add(Evas_Object *obj, const char *group_name, const char *alias_name)
{
   Edje_Part_Collection_Directory_Entry *e;
   Edje_Part_Collection_Directory_Entry *de;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->collection)
     return EINA_FALSE;

   /* check if a group with the same alias already exists */
   if (eina_hash_find(ed->file->collection, alias_name))
     return EINA_FALSE;
   /* check if a group that is being aliased is really exists */
   e = eina_hash_find(ed->file->collection, group_name);
   if (!e) return EINA_FALSE;
   /* check that a group that is being aliased is not an alias */
   if (e->group_alias) return EINA_FALSE;

   /* Create structs */
   de = _alloc(sizeof(Edje_Part_Collection_Directory_Entry));
   if (!de) return EINA_FALSE;

   /* Init Edje_Part_Collection_Directory_Entry */
   de->id = e->id;
   de->entry = eina_stringshare_add(alias_name);
   de->group_alias = EINA_TRUE;

   memcpy(&de->count, &e->count, sizeof (de->count));
   eina_hash_direct_add(ed->file->collection, de->entry, de);

   _mempools_add(de);

   return EINA_TRUE;
}

/***************/
/*  DATA API   */
/***************/

EAPI Eina_List *
edje_edit_group_data_list_get(Evas_Object *obj)
{
   Eina_Iterator *it;
   Eina_List *datas = NULL;
   const char *key;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->collection || !ed->collection->data)
     return NULL;

   it = eina_hash_iterator_key_new(ed->collection->data);
   if (!it) return NULL;

   EINA_ITERATOR_FOREACH(it, key)
     datas = eina_list_append(datas, eina_stringshare_add(key));

   eina_iterator_free(it);

   return datas;
}

EAPI Eina_List *
edje_edit_data_list_get(Evas_Object *obj)
{
   Eina_Iterator *i;
   Eina_List *datas = NULL;
   const char *key;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->data)
     return NULL;

   i = eina_hash_iterator_key_new(ed->file->data);

   EINA_ITERATOR_FOREACH(i, key)
     datas = eina_list_append(datas, eina_stringshare_add(key));

   eina_iterator_free(i);

   return datas;
}

EAPI Eina_Bool
edje_edit_group_data_add(Evas_Object *obj, const char *key, const char *value)
{
   Edje_String *es;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!key || !ed->file || !ed->collection)
     return EINA_FALSE;

   if (!ed->collection->data)
     ed->collection->data = eina_hash_string_small_new(NULL);

   if (eina_hash_find(ed->collection->data, key))
     return EINA_FALSE;

   es = calloc(1, sizeof(Edje_String));
   if (!es)
     return EINA_FALSE;
   es->str = eina_stringshare_add(value);

   if (!eina_hash_add(ed->collection->data, key, es))
     {
        eina_stringshare_del(es->str);
        free(es);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_data_add(Evas_Object *obj, const char *itemname, const char *value)
{
   Edje_String *es;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!itemname || !ed->file)
     return EINA_FALSE;

   if (!ed->file->data)
     ed->file->data = eina_hash_string_small_new(NULL);

   if (eina_hash_find(ed->file->data, itemname))
     return EINA_FALSE;

   es = calloc(1, sizeof(Edje_String));
   if (!es)
     return EINA_FALSE;
   es->str = eina_stringshare_add(value);

   if (!eina_hash_add(ed->file->data, itemname, es))
     {
        eina_stringshare_del(es->str);
        free(es);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_group_data_del(Evas_Object *obj, const char *key)
{
   Edje_String *value;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!key || !ed->file || !ed->collection)
     return EINA_FALSE;

   value = eina_hash_find(ed->collection->data, key);
   if (!value) return EINA_FALSE;

   eina_hash_del(ed->collection->data, key, value);
   _edje_if_string_free(ed, &value->str);
   free(value);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_data_del(Evas_Object *obj, const char *itemname)
{
   Edje_String *value;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!itemname || !ed->file || !ed->file->data)
     return 0;

   value = eina_hash_find(ed->file->data, itemname);
   if (!value)
     return EINA_FALSE;

   eina_hash_del(ed->file->data, itemname, value);
   _edje_if_string_free(ed, &value->str);
   free(value);

   return EINA_TRUE;
}

EAPI const char *
edje_edit_group_data_value_get(Evas_Object *obj, const char *key)
{
   Edje_String *value;

   GET_ED_OR_RETURN(NULL);

   if (!key || !ed->file || !ed->collection)
     return NULL;

   value = eina_hash_find(ed->collection->data, key);
   if (!value)
     return NULL;

   return eina_stringshare_add(edje_string_get(value));
}

EAPI const char *
edje_edit_data_value_get(Evas_Object *obj, const char *itemname)
{
   Edje_String *value;

   GET_ED_OR_RETURN(NULL);

   if (!itemname || !ed->file || !ed->file->data)
     return NULL;

   value = eina_hash_find(ed->file->data, itemname);
   if (!value)
     return NULL;

   return eina_stringshare_add(edje_string_get(value));
}

EAPI Eina_Bool
edje_edit_group_data_value_set(Evas_Object *obj, const char *key, const char *value)
{
   Edje_String *es;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!key || !value || !ed->file || !ed->collection)
     return EINA_FALSE;

   es = eina_hash_find(ed->collection->data, key);
   if (es)
     {
        _edje_if_string_replace(ed, &es->str, value);
        es->id = 0;
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_data_value_set(Evas_Object *obj, const char *itemname, const char *value)
{
   Edje_String *es;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!itemname || !value || !ed->file || !ed->file->data)
     return EINA_FALSE;

   es = eina_hash_find(ed->file->data, itemname);
   if (es)
     {
        _edje_if_string_replace(ed, &es->str, value);
        es->id = 0;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_group_data_name_set(Evas_Object *obj, const char *key, const char *new_key)
{
   GET_ED_OR_RETURN(EINA_FALSE);

   if (!key || !new_key || !ed->file || !ed->collection)
     {
        return EINA_FALSE;
     }

   return eina_hash_move(ed->collection->data, key, new_key);
}

EAPI Eina_Bool
edje_edit_data_name_set(Evas_Object *obj, const char *itemname, const char *newname)
{
   GET_ED_OR_RETURN(EINA_FALSE);

   if (!itemname || !newname || !ed->file || !ed->file->data)
     return EINA_FALSE;

   return eina_hash_move(ed->file->data, itemname, newname);
}

/***********************/
/*  COLOR CLASSES API  */
/***********************/

EAPI Eina_List *
edje_edit_color_classes_list_get(Evas_Object *obj)
{
   Eina_List *classes = NULL;
   Eina_List *l;
   Edje_Color_Class *cc;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->color_classes)
     return NULL;
   //printf("GET CLASSES LIST %d %d\n", eina_list_count(ed->color_classes), eina_list_count(ed->file->color_classes));
   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     classes = eina_list_append(classes, eina_stringshare_add(cc->name));

   return classes;
}

EAPI Eina_Bool
edje_edit_color_class_colors_get(Evas_Object *obj, const char *class_name, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3)
{
   Eina_List *l;
   Edje_Color_Class *cc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->color_classes)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     if (!strcmp(cc->name, class_name))
       {
          if (r) *r = cc->r;
          if (g) *g = cc->g;
          if (b) *b = cc->b;
          if (a) *a = cc->a;

          if (r2) *r2 = cc->r2;
          if (g2) *g2 = cc->g2;
          if (b2) *b2 = cc->b2;
          if (a2) *a2 = cc->a2;

          if (r3) *r3 = cc->r3;
          if (g3) *g3 = cc->g3;
          if (b3) *b3 = cc->b3;
          if (a3) *a3 = cc->a3;

          return EINA_TRUE;
       }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_color_class_colors_set(Evas_Object *obj, const char *class_name, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Eina_List *l;
   Edje_Color_Class *cc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->color_classes)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     if (!strcmp(cc->name, class_name))
       {
          if (r > -1) cc->r = r;
          if (g > -1) cc->g = g;
          if (b > -1) cc->b = b;
          if (a > -1) cc->a = a;

          if (r2 > -1) cc->r2 = r2;
          if (g2 > -1) cc->g2 = g2;
          if (b2 > -1) cc->b2 = b2;
          if (a2 > -1) cc->a2 = a2;

          if (r3 > -1) cc->r3 = r3;
          if (g3 > -1) cc->g3 = g3;
          if (b3 > -1) cc->b3 = b3;
          if (a3 > -1) cc->a3 = a3;

          return EINA_TRUE;
       }
   return EINA_FALSE;
}

EAPI Eina_Stringshare *
edje_edit_color_class_description_get(Evas_Object *obj, const char *class_name)
{
   Eina_List *l;
   Edje_Color_Class *cc;

   GET_ED_OR_RETURN(NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(class_name, NULL);

   if (!ed->file || !ed->file->color_classes)
     return NULL;
   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     if (eina_streq(cc->name, class_name))
       return cc->desc;
   return NULL;
}

EAPI Eina_Bool
edje_edit_color_class_description_set(Evas_Object *obj, const char *class_name, const char *desc)
{
   Eina_List *l;
   Edje_Color_Class *cc;

   GET_ED_OR_RETURN(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(class_name, EINA_FALSE);

   if (!ed->file || !ed->file->color_classes)
     return EINA_FALSE;
   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     if (eina_streq(cc->name, class_name))
       {
          eina_stringshare_replace(&cc->desc, desc);
          return EINA_TRUE;
       }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_color_class_add(Evas_Object *obj, const char *name)
{
   Eina_List *l;
   Edje_Color_Class *c;
   Edje_Color_Class *cc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name || !ed->file)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     if (strcmp(cc->name, name) == 0)
       return EINA_FALSE;

   c = _alloc(sizeof(Edje_Color_Class));
   if (!c) return EINA_FALSE;

   c->name = (char *)eina_stringshare_add(name);
   c->r = c->g = c->b = c->a = 255;
   c->r2 = c->g2 = c->b2 = c->a2 = 255;
   c->r3 = c->g3 = c->b3 = c->a3 = 255;

   ed->file->color_classes = eina_list_append(ed->file->color_classes, c);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_color_class_del(Evas_Object *obj, const char *name)
{
   Eina_List *l;
   Edje_Color_Class *cc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name || !ed->file || !ed->file->color_classes)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     if (strcmp(cc->name, name) == 0)
       {
          _edje_if_string_free(ed, &cc->name);
          ed->file->color_classes = eina_list_remove(ed->file->color_classes, cc);
          free(cc);
          return EINA_TRUE;
       }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_color_class_name_set(Evas_Object *obj, const char *name, const char *newname)
{
   Eina_List *l;
   Edje_Color_Class *cc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->color_classes)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     if (!strcmp(cc->name, name))
       {
          _edje_if_string_replace(ed, &cc->name, newname);
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

/*********************/
/*  TEXT STYLES API  */
/*********************/

EAPI Eina_List *
edje_edit_styles_list_get(Evas_Object *obj)
{
   Eina_List *styles = NULL;
   Eina_List *l;
   Edje_Style *s;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->styles)
     return NULL;
   //printf("GET STYLES LIST %d\n", eina_list_count(ed->file->styles));
   EINA_LIST_FOREACH(ed->file->styles, l, s)
     styles = eina_list_append(styles, eina_stringshare_add(s->name));

   return styles;
}

EAPI Eina_Bool
edje_edit_style_add(Evas_Object *obj, const char *style)
{
   Edje_Style *s;

   GET_ED_OR_RETURN(EINA_FALSE);
   //printf("ADD STYLE '%s'\n", style);

   s = _edje_edit_style_get(ed, style);
   if (s) return EINA_FALSE;

   s = _alloc(sizeof(Edje_Style));
   if (!s) return EINA_FALSE;
   s->name = (char *)eina_stringshare_add(style);
   s->tags = NULL;
   s->style = NULL;

   ed->file->styles = eina_list_append(ed->file->styles, s);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_style_del(Evas_Object *obj, const char *style)
{
   Edje_Style *s;

   GET_ED_OR_RETURN(EINA_FALSE);

   s = _edje_edit_style_get(ed, style);
   if (!s) return EINA_FALSE;

   ed->file->styles = eina_list_remove(ed->file->styles, s);

   _edje_if_string_free(ed, (const char **)&s->name);
   evas_textblock_style_free(s->style);
   while (s->tags)
     {
        Edje_Style_Tag *t;

        t = s->tags->data;

        s->tags = eina_list_remove(s->tags, t);
        _edje_if_string_free(ed, &t->key);
        _edje_if_string_free(ed, &t->value);
        _edje_if_string_free(ed, &t->font);
        _edje_if_string_free(ed, &t->text_class);
        free(t);
     }
   free(s);
   return EINA_TRUE;
}

EAPI Eina_List *
edje_edit_style_tags_list_get(Evas_Object *obj, const char *style)
{
   Eina_List *tags = NULL;
   Eina_List *l;
   Edje_Style *s;
   Edje_Style_Tag *t;

   GET_ED_OR_RETURN(NULL);
   if (!ed->file || !ed->file->styles || !style)
     return NULL;

   s = _edje_edit_style_get(ed, style);

   //printf("GET STYLE TAG LIST %d\n", eina_list_count(s->tags));
   EINA_LIST_FOREACH(s->tags, l, t)
     tags = eina_list_append(tags, eina_stringshare_add(t->key));

   return tags;
}

EAPI Eina_Bool
edje_edit_style_tag_name_set(Evas_Object *obj, const char *style, const char *tag, const char *new_name)
{
   Edje_Style_Tag *t;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->styles || !style || !tag)
     return EINA_FALSE;

   t = _edje_edit_style_tag_get(ed, style, tag);
   if (!t) return EINA_FALSE;
   _edje_if_string_replace(ed, &t->key, new_name);
   return EINA_TRUE;
}

EAPI const char *
edje_edit_style_tag_value_get(Evas_Object *obj, const char *style, const char *tag)
{
   Edje_Style_Tag *t;

   GET_ED_OR_RETURN(NULL);
   //printf("GET TAG '%s' FOR STYLE '%s'\n", tag, style);

   if (!ed->file || !ed->file->styles || !style || !tag)
     return NULL;

   t = _edje_edit_style_tag_get(ed, style, tag);
   if (t && t->value)
     return eina_stringshare_add(t->value);

   return NULL;
}

EAPI Eina_Bool
edje_edit_style_tag_value_set(Evas_Object *obj, const char *style, const char *tag, const char *new_value)
{
   Edje_Style_Tag *t;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->styles || !style || !tag || !new_value)
     return EINA_FALSE;

   t = _edje_edit_style_tag_get(ed, style, tag);
   if (!t) return EINA_FALSE;
   _edje_if_string_replace(ed, &t->value, new_value);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_style_tag_add(Evas_Object *obj, const char *style, const char *tag_name)
{
   Edje_Style *s;
   Edje_Style_Tag *t;

   GET_ED_OR_RETURN(EINA_FALSE);
   //printf("ADD TAG '%s' IN STYLE '%s'\n", tag_name, style);

   t = _edje_edit_style_tag_get(ed, style, tag_name);
   if (t) return EINA_FALSE;
   s = _edje_edit_style_get(ed, style);
   if (!s) return EINA_FALSE;

   t = _alloc(sizeof(Edje_Style_Tag));
   if (!t) return EINA_FALSE;
   t->key = eina_stringshare_add(tag_name);
   t->value = NULL;
   t->font = NULL;
   t->text_class = NULL;

   s->tags = eina_list_append(s->tags, t);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_style_tag_del(Evas_Object *obj, const char *style, const char *tag)
{
   Edje_Style *s;
   Edje_Style_Tag *t;

   GET_ED_OR_RETURN(EINA_FALSE);
   if (!ed->file || !ed->file->styles || !style || !tag )
     return EINA_FALSE;

   s = _edje_edit_style_get(ed, style);
   t = _edje_edit_style_tag_get(ed, style, tag);
   if (!s || !t) return EINA_FALSE;

   s->tags = eina_list_remove(s->tags, t);
   _edje_if_string_free(ed, &t->key);
   _edje_if_string_free(ed, &t->value);
   _edje_if_string_free(ed, &t->font);
   _edje_if_string_free(ed, &t->text_class);
   free(t);
   t = NULL;
   return EINA_TRUE;
}

/*******************/
/*  EXTERNALS API  */
/*******************/

EAPI Eina_List *
edje_edit_externals_list_get(Evas_Object *obj)
{
   Eina_List *externals = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->external_dir)
     return NULL;
   //printf("GET STYLES LIST %d\n", eina_list_count(ed->file->styles));
   for (i = 0; i < ed->file->external_dir->entries_count; ++i)
     externals = eina_list_append(externals,
                                  eina_stringshare_add(ed->file->external_dir->entries[i].entry));

   return externals;
}

EAPI Eina_Bool
edje_edit_external_add(Evas_Object *obj, const char *external)
{
   Edje_External_Directory_Entry *e;
   unsigned int freeid;
   unsigned int i;

   GET_ED_OR_RETURN(EINA_FALSE);

   e = _edje_edit_external_get(ed, external);
   if (e) return EINA_FALSE;

   if (!ed->file->external_dir)
     ed->file->external_dir = _alloc(sizeof(Edje_External_Directory));
   if (!ed->file->external_dir) return EINA_FALSE;

   for (i = 0; i < ed->file->external_dir->entries_count; ++i)
     if (!ed->file->external_dir->entries[i].entry)
       break;

   if (i == ed->file->external_dir->entries_count)
     {
        Edje_External_Directory_Entry *tmp;
        unsigned int max;

        max = ed->file->external_dir->entries_count + 1;
        tmp = realloc(ed->file->external_dir->entries,
                      sizeof (Edje_External_Directory_Entry) * max);

        if (!tmp) return EINA_FALSE;

        ed->file->external_dir->entries = tmp;
        freeid = ed->file->external_dir->entries_count;
        ed->file->external_dir->entries_count = max;
     }
   else
     freeid = i;

   ed->file->external_dir->entries[freeid].entry = (char *)eina_stringshare_add(external);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_external_del(Evas_Object *obj, const char *external)
{
   Edje_External_Directory_Entry *e;

   GET_ED_OR_RETURN(EINA_FALSE);

   e = _edje_edit_external_get(ed, external);
   if (!e) return EINA_FALSE;

   _edje_if_string_free(ed, &e->entry);
   e->entry = NULL;

   return EINA_TRUE;
}

/***************/
/*  PARTS API  */
/***************/

EAPI Edje_Edit_Select_Mode
edje_edit_part_select_mode_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)
     return EINA_FALSE;

   return (Edje_Edit_Select_Mode)rp->part->select_mode;
}

EAPI Eina_Bool
edje_edit_part_select_mode_set(Evas_Object *obj, const char *part, Edje_Edit_Select_Mode mode)
{
   if (mode > EDJE_EDIT_SELECT_MODE_EXPLICIT)
     return EINA_FALSE;
   GET_RP_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)
     return EINA_FALSE;

   rp->part->select_mode = (unsigned char)mode;
   return EINA_TRUE;
}

EAPI Edje_Edit_Entry_Mode
edje_edit_part_entry_mode_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)
     return EINA_FALSE;

   return (Edje_Edit_Entry_Mode)rp->part->entry_mode;
}

EAPI Eina_Bool
edje_edit_part_entry_mode_set(Evas_Object *obj, const char *part, Edje_Edit_Entry_Mode mode)
{
   if (mode > EDJE_EDIT_ENTRY_MODE_PASSWORD)
     return EINA_FALSE;
   GET_RP_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)
     return EINA_FALSE;

   rp->part->entry_mode = (unsigned char)mode;
   return EINA_TRUE;
}

EAPI Eina_List *
edje_edit_parts_list_get(Evas_Object *obj)
{
   Eina_List *parts = NULL;
   unsigned short i;

   GET_ED_OR_RETURN(NULL);

   //printf("EE: Found %d parts\n", ed->table_parts_size);

   parts = NULL;
   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;

        rp = ed->table_parts[i];
        parts = eina_list_append(parts, eina_stringshare_add(rp->part->name));
     }

   return parts;
}

EAPI Eina_Bool
edje_edit_part_name_set(Evas_Object *obj, const char *part, const char *new_name)
{
   GET_EED_OR_RETURN(EINA_FALSE);
   GET_RP_OR_RETURN(EINA_FALSE);

   if (!new_name) return EINA_FALSE;
   if (!strcmp(part, new_name)) return EINA_TRUE;
   if (_edje_real_part_get(ed, new_name)) return EINA_FALSE;

   //printf("Set name of part: %s [new name: %s]\n", part, new_name);

   _edje_if_string_replace(ed, &rp->part->name, new_name);

   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

#define FUNC_PART_API_STRING(Value)                                                    \
  EAPI const char *                                                                    \
  edje_edit_part_api_##Value##_get(Evas_Object * obj, const char *part)                \
  {                                                                                    \
     GET_RP_OR_RETURN(NULL);                                                           \
     return eina_stringshare_add(rp->part->api.Value);                                 \
  }                                                                                    \
  EAPI Eina_Bool                                                                       \
  edje_edit_part_api_##Value##_set(Evas_Object * obj, const char *part, const char *s) \
  {                                                                                    \
     GET_RP_OR_RETURN(EINA_FALSE);                                                     \
     _edje_if_string_replace(ed, &rp->part->api.Value, s);                             \
     return EINA_TRUE;                                                                 \
  }

FUNC_PART_API_STRING(name);
FUNC_PART_API_STRING(description);

static Eina_Bool
_edje_edit_real_part_add(Evas_Object *obj, const char *name, Edje_Part_Type type, const char *source)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Part_Collection *pc;
   Edje_Part **tmp;
   Edje_Part *ep;
   Edje_Real_Part *rp;
   int id;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (ed->table_parts_size == 0xffff) return EINA_FALSE;
   //printf("ADD PART: %s [type: %d]\n", name, type);

   /* Check if part already exists */
   if (_edje_real_part_get(ed, name))
     return EINA_FALSE;

   if (!ed->file) return EINA_FALSE;

   ce = eina_hash_find(ed->file->collection, ed->group);

   /* Alloc Edje_Part or return */
   ep = eina_mempool_malloc(ce->mp->mp.part, sizeof(Edje_Part));
   if (!ep) return EINA_FALSE;
   memset(ep, 0, sizeof(Edje_Part));

   /* Alloc Edje_Real_Part or return */
   rp = eina_mempool_malloc(_edje_real_part_mp, sizeof(Edje_Real_Part));
   if (!rp)
     {
        eina_mempool_free(ce->mp->mp.part, ep);
        return EINA_FALSE;
     }
   memset(rp, 0, sizeof(Edje_Real_Part));

   /* Init Edje_Part */
   pc = ed->collection;

   tmp = realloc(pc->parts, (pc->parts_count + 1) * sizeof (Edje_Part *));
   if (!tmp)
     {
        eina_mempool_free(ce->mp->mp.part, ep);
        eina_mempool_free(_edje_real_part_mp, rp);
        return EINA_FALSE;
     }

   id = pc->parts_count++;

   pc->parts = tmp;
   pc->parts[id] = ep;

   ep->id = id;
   ep->type = type;
   ep->name = eina_stringshare_add(name);
   ep->mouse_events = 1;
   ep->repeat_events = 0;
   ep->anti_alias = 1;
   ep->ignore_flags = EVAS_EVENT_FLAG_NONE;
   ep->mask_flags = EVAS_EVENT_FLAG_NONE;
   ep->pointer_mode = EVAS_OBJECT_POINTER_MODE_AUTOGRAB;
   ep->precise_is_inside = 0;
   ep->use_alternate_font_metrics = 0;
   ep->clip_to_id = -1;
   ep->dragable.confine_id = -1;
   ep->dragable.threshold_id = -1;
   ep->dragable.event_id = -1;
   if (source)
     ep->source = eina_stringshare_add(source);

   ep->default_desc = NULL;
   ep->other.desc = NULL;
   ep->other.desc_count = 0;

   /* Init Edje_Real_Part */
   _edje_ref(ed);
   rp->part = ep;

   if (ep->type == EDJE_PART_TYPE_RECTANGLE)
     rp->object = evas_object_rectangle_add(ed->base.evas);
   else if (ep->type == EDJE_PART_TYPE_VECTOR)
     {
        rp->type = EDJE_PART_TYPE_VECTOR;
        rp->object = evas_object_vg_add(ed->base.evas);
     }
   else if (ep->type == EDJE_PART_TYPE_IMAGE || ep->type == EDJE_PART_TYPE_PROXY)
     rp->object = evas_object_image_add(ed->base.evas);
   else if (ep->type == EDJE_PART_TYPE_TEXT)
     {
        rp->type = EDJE_RP_TYPE_TEXT;
        rp->typedata.text = calloc(1, sizeof(Edje_Real_Part_Text));
        _edje_text_part_on_add(ed, rp);
        rp->object = evas_object_text_add(ed->base.evas);
        evas_object_text_font_source_set(rp->object, ed->path);
     }
   else if (ep->type == EDJE_PART_TYPE_SWALLOW ||
            ep->type == EDJE_PART_TYPE_GROUP ||
            ep->type == EDJE_PART_TYPE_EXTERNAL)
     {
        rp->type = EDJE_RP_TYPE_SWALLOW;
        rp->typedata.swallow = calloc(1, sizeof(Edje_Real_Part_Swallow));
        rp->object = evas_object_rectangle_add(ed->base.evas);
        evas_object_color_set(rp->object, 0, 0, 0, 0);
        evas_object_pass_events_set(rp->object, 1);
        evas_object_pointer_mode_set(rp->object, EVAS_OBJECT_POINTER_MODE_NOGRAB);
     }
   else if (ep->type == EDJE_PART_TYPE_TEXTBLOCK)
     {
        rp->type = EDJE_RP_TYPE_TEXT;
        rp->typedata.text = calloc(1, sizeof(Edje_Real_Part_Text));
        rp->object = evas_object_textblock_add(ed->base.evas);
     }
   else if (ep->type == EDJE_PART_TYPE_BOX)
     {
        rp->type = EDJE_RP_TYPE_CONTAINER;
        rp->typedata.container = calloc(1, sizeof(Edje_Real_Part_Container));
        rp->object = evas_object_box_add(ed->base.evas);
        rp->typedata.container->anim = _edje_box_layout_anim_new(rp->object);
     }
   else if (ep->type == EDJE_PART_TYPE_TABLE)
     {
        rp->type = EDJE_RP_TYPE_CONTAINER;
        rp->typedata.container = calloc(1, sizeof(Edje_Real_Part_Container));
        rp->object = evas_object_table_add(ed->base.evas);
     }
   else if (ep->type != EDJE_PART_TYPE_SPACER)
     ERR("wrong part type %i!", ep->type);
   if (rp->object)
     {
        evas_object_show(rp->object);
        evas_object_smart_member_add(rp->object, ed->obj);
        evas_object_layer_set(rp->object, evas_object_layer_get(ed->obj));
        if (ep->type == EDJE_PART_TYPE_SWALLOW)
          {
             efl_parent_set(rp->object, ed->obj);
          }
        else if (ep->type != EDJE_PART_TYPE_GROUP)
          {
             if (ep->mouse_events)
               {
                  _edje_callbacks_add(rp->object, ed, rp);
                  if (ep->repeat_events)
                    evas_object_repeat_events_set(rp->object, 1);

                  if (ep->pointer_mode != EVAS_OBJECT_POINTER_MODE_AUTOGRAB)
                    evas_object_pointer_mode_set(rp->object, ep->pointer_mode);
               }
             else
               {
                  evas_object_pass_events_set(rp->object, 1);
                  evas_object_pointer_mode_set(rp->object,
                                               EVAS_OBJECT_POINTER_MODE_NOGRAB);
               }
             if (ep->precise_is_inside)
               evas_object_precise_is_inside_set(rp->object, 1);
          }
        if (ep->type == EDJE_PART_TYPE_EXTERNAL)
          {
             Evas_Object *child;
             child = _edje_external_type_add(source, evas_object_evas_get(ed->obj), ed->obj, NULL, name);
             if (child)
               _edje_real_part_swallow(ed, rp, child, EINA_TRUE);
          }
        evas_object_clip_set(rp->object, ed->base.clipper);
        evas_object_show(ed->base.clipper);
     }

   /* Update table_parts */
   ed->table_parts_size++;
   ed->table_parts = realloc(ed->table_parts,
                             sizeof(Edje_Real_Part *) * ed->table_parts_size);

   ed->table_parts[ep->id % ed->table_parts_size] = rp;

   /* Create default description */
   if (!edje_edit_state_add(obj, name, "default", 0.0))
     {
        _edje_if_string_free(ed, &ep->name);
        if (source)
          _edje_if_string_free(ed, &ep->source);
        eina_mempool_free(ce->mp->mp.part, ep);
        eina_mempool_free(_edje_real_part_mp, rp);
        return EINA_FALSE;
     }
   edje_edit_part_selected_state_set(obj, name, "default", 0.0);

   ce->count.part++;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_add(Evas_Object *obj, const char *name, Edje_Part_Type type)
{
   if (type == EDJE_PART_TYPE_EXTERNAL)
     return EINA_FALSE;
   return _edje_edit_real_part_add(obj, name, type, NULL);
}

EAPI Eina_Bool
edje_edit_part_external_add(Evas_Object *obj, const char *name, const char *source)
{
   if (!source)
     return EINA_FALSE;
   return _edje_edit_real_part_add(obj, name, EDJE_PART_TYPE_EXTERNAL, source);
}

EAPI Eina_Bool
edje_edit_part_del(Evas_Object *obj, const char *part)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Part_Collection *pc;
   Edje_Part *ep;
   unsigned int k;
   unsigned int id;
   unsigned short i, j;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_RP_OR_RETURN(EINA_FALSE);

   //printf("REMOVE PART: %s\n", part);

   ep = rp->part;
   id = ep->id;

   /* Unlik Edje_Real_Parts that link to the removed one */
   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *real;

        if (i == id) continue;  //don't check the deleted id
        real = ed->table_parts[i];

        if ((rp->type == EDJE_RP_TYPE_TEXT) &&
            (real->typedata.text) && (real->typedata.text->source == rp))
          real->typedata.text->source = NULL;
        if ((rp->type == EDJE_RP_TYPE_TEXT) &&
            (real->typedata.text) && (real->typedata.text->text_source == rp))
          real->typedata.text->text_source = NULL;

        if (real->part->clip_to_id == rp->part->id)
          {
             evas_object_clip_set(real->object, ed->base.clipper);
             real->part->clip_to_id = -1;
          }
        if (real->drag && real->drag->confine_to == rp)
          {
             real->drag->confine_to = NULL;
          }
        if (real->part->dragable.event_id != -1)
          {
             if (real->part->dragable.event_id == TO_INT(id))
               real->part->dragable.event_id = -1;
             else if (i > id)
               real->part->dragable.event_id--;
          }

        if (real->part->default_desc->map.id_persp == rp->part->id)
          real->part->default_desc->map.id_persp = -1;
        if (real->part->default_desc->map.id_light == rp->part->id)
          real->part->default_desc->map.id_light = -1;
        if (real->part->default_desc->map.rot.id_center == rp->part->id)
          real->part->default_desc->map.rot.id_center = -1;

        for (j = 0; j < real->part->other.desc_count; ++j)
          {
             if (real->part->other.desc[j]->map.id_persp == rp->part->id)
               real->part->other.desc[j]->map.id_persp = -1;
             if (real->part->other.desc[j]->map.id_light == rp->part->id)
               real->part->other.desc[j]->map.id_light = -1;
             if (real->part->other.desc[j]->map.rot.id_center == rp->part->id)
               real->part->other.desc[j]->map.rot.id_center = -1;
          }
     }

   /* Unlink all the parts and descriptions that refer to id */
   _edje_part_id_set(ed, rp, -1);

   /* Remove part from parts list */
   pc = ed->collection;
   pc->parts_count--;
   if (id < pc->parts_count) /* Forward parts */
     {
        int mcount = (pc->parts_count - id) * sizeof(Edje_Part *);
        memmove(&pc->parts[id], &pc->parts[id + 1], mcount);
     }
   pc->parts[pc->parts_count] = NULL;
   _edje_fix_parts_id(ed);

   /* Free Edje_Part and all descriptions */
   ce = eina_hash_find(ed->file->collection, ed->group);

   _edje_if_string_free(ed, &ep->name);
   if (ep->default_desc)
     {
        _edje_collection_free_part_description_free(ep->type, ep->default_desc, ce, 0);
        ep->default_desc = NULL;
     }

   for (k = 0; k < ep->other.desc_count; ++k)
     _edje_collection_free_part_description_free(ep->type, ep->other.desc[k], ce, 0);

   free(ep->other.desc);
   eina_mempool_free(ce->mp->mp.part, ep);

   /* Free Edje_Real_Part */
   _edje_real_part_free(ed, rp);

   /* if all parts are gone, hide the clipper */
   if (ed->table_parts_size == 0)
     evas_object_hide(ed->base.clipper);

   edje_object_calc_force(obj);

   ce->count.part--;

   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_edje_edit_part_state_copy(Evas_Object *obj, const char *part_from, const char *part_to, const char *from, double val_from, const char *to, double val_to);

EAPI Eina_Bool
edje_edit_part_copy(Evas_Object *obj, const char *part, const char *new_copy)
{
   Edje_Part *ep, *epcopy;
   unsigned int i, count;
   Edje_Real_Part *rpcopy;

   GET_RP_OR_RETURN(EINA_FALSE);

   ep = rp->part;

   /* Check if part doesn't exists */
   if (_edje_real_part_get(ed, new_copy))
     return EINA_FALSE;

   if (!ed->file) return EINA_FALSE;

   /* Create part (EXTERNAL or not) */
   if (!_edje_edit_real_part_add(obj, new_copy, ep->type, ep->source))
     return EINA_FALSE;

   /* Copy part's data */
   rpcopy = _edje_real_part_get(ed, new_copy);
   if (!rpcopy)
     return EINA_FALSE;
   epcopy = rpcopy->part;

#define _PARAM_PART_COPY(param) \
  epcopy->param = ep->param;

   _PARAM_PART_COPY(scale)
   _PARAM_PART_COPY(mouse_events)
   _PARAM_PART_COPY(repeat_events)
   _PARAM_PART_COPY(ignore_flags)
   _PARAM_PART_COPY(mask_flags)
   _PARAM_PART_COPY(pointer_mode)
   _PARAM_PART_COPY(precise_is_inside)
   _PARAM_PART_COPY(use_alternate_font_metrics)
   _PARAM_PART_COPY(clip_to_id)
   _PARAM_PART_COPY(dragable.event_id)
   _PARAM_PART_COPY(dragable.confine_id)
   _PARAM_PART_COPY(dragable.threshold_id)
   _PARAM_PART_COPY(dragable.step_x)
   _PARAM_PART_COPY(dragable.step_y)
   _PARAM_PART_COPY(dragable.count_x)
   _PARAM_PART_COPY(dragable.count_y)
   _PARAM_PART_COPY(dragable.x)
   _PARAM_PART_COPY(dragable.y)

   if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
     {
        epcopy->source2 = (char *)eina_stringshare_add(ep->source2);
        epcopy->source3 = (char *)eina_stringshare_add(ep->source3);
        epcopy->source4 = (char *)eina_stringshare_add(ep->source4);
        epcopy->source5 = (char *)eina_stringshare_add(ep->source5);
        epcopy->source6 = (char *)eina_stringshare_add(ep->source6);
     }

#ifdef HAVE_EPHYSICS
   _PARAM_PART_COPY(physics_body)
#endif

   _PARAM_PART_COPY(effect)
   _PARAM_PART_COPY(entry_mode)
   _PARAM_PART_COPY(select_mode)
   _PARAM_PART_COPY(cursor_mode)
   _PARAM_PART_COPY(multiline)
   _PARAM_PART_COPY(access)

   if (ep->api.name)
     epcopy->api.name = eina_stringshare_add(ep->api.name);
   if (ep->api.description)
     epcopy->api.description = eina_stringshare_add(ep->api.description);

#undef _PARAM_PART_COPY

   /* Copy default state */
   _edje_edit_part_state_copy(obj, part, new_copy, "default", 0.0, "default", 0.0);

   /* Copy all other states */
   count = rp->part->other.desc_count;
   for (i = 0; i < count; ++i)
     {
        _edje_edit_part_state_copy(obj, part, new_copy,
                                   rp->part->other.desc[i]->state.name, rp->part->other.desc[i]->state.value,
                                   rp->part->other.desc[i]->state.name, rp->part->other.desc[i]->state.value);
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_exist(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(EINA_FALSE);
   return EINA_TRUE;
}

EAPI const char *
edje_edit_part_below_get(Evas_Object *obj, const char *part)
{
   Edje_Real_Part *prev;

   GET_RP_OR_RETURN(0);

   if (rp->part->id < 1) return NULL;

   prev = ed->table_parts[(rp->part->id - 1) % ed->table_parts_size];

   return eina_stringshare_add(prev->part->name);
}

EAPI const char *
edje_edit_part_above_get(Evas_Object *obj, const char *part)
{
   Edje_Real_Part *next;

   GET_RP_OR_RETURN(0);

   if ((unsigned short)rp->part->id >= ed->table_parts_size - 1) return 0;

   next = ed->table_parts[(rp->part->id + 1) % ed->table_parts_size];

   return eina_stringshare_add(next->part->name);
}

EAPI Eina_Bool
edje_edit_part_restack_below(Evas_Object *obj, const char *part)
{
   Edje_Part_Collection *group;
   Edje_Real_Part *prev;
   Edje_Part *swap;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_RP_OR_RETURN(EINA_FALSE);

   //printf("RESTACK PART: %s BELOW\n", part);

   if (rp->part->id < 1) return EINA_FALSE;
   group = ed->collection;

   /* update parts list */
   prev = ed->table_parts[(rp->part->id - 1) % ed->table_parts_size];

   swap = group->parts[rp->part->id];
   group->parts[rp->part->id] = group->parts[prev->part->id];
   group->parts[prev->part->id] = swap;

   _edje_parts_id_switch(ed, rp, prev);

   evas_object_stack_below(rp->object, prev->object);
   if ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
       (rp->typedata.swallow) && (rp->typedata.swallow->swallowed_object))
     evas_object_stack_above(rp->typedata.swallow->swallowed_object, rp->object);

   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_restack_part_below(Evas_Object *obj, const char *part, const char *below)
{
   Edje_Part_Collection *group;
   Edje_Real_Part *rp, *rp_below, *prev;
   Edje_Part *swap;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_ED_OR_RETURN(EINA_FALSE);
   rp = _edje_real_part_get(ed, part);
   if (!rp) return EINA_FALSE;
   rp_below = _edje_real_part_get(ed, below);
   if (!rp_below) return EINA_FALSE;

   group = ed->collection;

   while (rp->part->id != (rp_below->part->id - 1))
     {
        if (rp->part->id > rp_below->part->id)
          prev = ed->table_parts[(rp->part->id - 1) % ed->table_parts_size];
        else
          prev = ed->table_parts[(rp->part->id + 1) % ed->table_parts_size];
        swap = group->parts[rp->part->id];
        group->parts[rp->part->id] = group->parts[prev->part->id];
        group->parts[prev->part->id] = swap;
        _edje_parts_id_switch(ed, rp, prev);
     }

   evas_object_stack_below(rp->object, rp_below->object);
   if ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
       (rp->typedata.swallow) && (rp->typedata.swallow->swallowed_object))
     evas_object_stack_above(rp->typedata.swallow->swallowed_object, rp->object);

   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_restack_above(Evas_Object *obj, const char *part)
{
   Edje_Part_Collection *group;
   Edje_Real_Part *next;
   Edje_Part *swap;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_RP_OR_RETURN(EINA_FALSE);

   //printf("RESTACK PART: %s ABOVE\n", part);

   if ((unsigned short)rp->part->id >= ed->table_parts_size - 1) return EINA_FALSE;

   group = ed->collection;

   /* update parts list */
   next = ed->table_parts[(rp->part->id + 1) % ed->table_parts_size];

   swap = group->parts[rp->part->id];
   group->parts[rp->part->id] = group->parts[next->part->id];
   group->parts[next->part->id] = swap;

   /* update ids */
   _edje_parts_id_switch(ed, rp, next);

   evas_object_stack_above(rp->object, next->object);
   if ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
       (rp->typedata.swallow) && (rp->typedata.swallow->swallowed_object))
     evas_object_stack_above(rp->typedata.swallow->swallowed_object, rp->object);

   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_restack_part_above(Evas_Object *obj, const char *part, const char *above)
{
   Edje_Part_Collection *group;
   Edje_Real_Part *rp, *rp_above, *next;
   Edje_Part *swap;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_ED_OR_RETURN(EINA_FALSE);
   rp = _edje_real_part_get(ed, part);
   if (!rp) return EINA_FALSE;
   rp_above = _edje_real_part_get(ed, above);
   if (!rp_above) return EINA_FALSE;

   group = ed->collection;

   while (rp->part->id != rp_above->part->id + 1)
     {
        if (rp->part->id > rp_above->part->id)
          next = ed->table_parts[(rp->part->id - 1) % ed->table_parts_size];
        else
          next = ed->table_parts[(rp->part->id + 1) % ed->table_parts_size];
        swap = group->parts[rp->part->id];
        group->parts[rp->part->id] = group->parts[next->part->id];
        group->parts[next->part->id] = swap;
        _edje_parts_id_switch(ed, rp, next);
     }

   evas_object_stack_above(rp->object, rp_above->object);
   if ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
       (rp->typedata.swallow) && (rp->typedata.swallow->swallowed_object))
     evas_object_stack_above(rp->typedata.swallow->swallowed_object, rp->object);

   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

EAPI Edje_Part_Type
edje_edit_part_type_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   return rp->part->type;
}

EAPI const char *
edje_edit_part_selected_state_get(Evas_Object *obj, const char *part, double *value)
{
   GET_RP_OR_RETURN(NULL);

   if (!rp->chosen_description)
     {
        if (value) *value = 0.0;  // FIXME: Make sure edje_edit supports correctly the default having any value
        return eina_stringshare_add("default");
     }

   if (value) *value = rp->chosen_description->state.value;
   return eina_stringshare_add(rp->chosen_description->state.name);
}

EAPI Eina_Bool
edje_edit_part_selected_state_set(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Common *pd;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_RP_OR_RETURN(EINA_FALSE);

   pd = _edje_part_description_find_byname(eed, part, state, value);
   if (!pd) return EINA_FALSE;

   //printf("EDJE: Set state: %s %f\n", pd->state.name, pd->state.value);
   _edje_part_description_apply(ed, rp, pd->state.name, pd->state.value, NULL, 0.0);

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

static const char *
_edje_part_clip_to_get(Edje *ed, Edje_Real_Part *rp)
{
   Edje_Real_Part *clip = NULL;

   if (rp->part->clip_to_id < 0) return NULL;

   clip = ed->table_parts[rp->part->clip_to_id % ed->table_parts_size];
   if (!clip || !clip->part || !clip->part->name) return NULL;

   return eina_stringshare_add(clip->part->name);
}

EAPI const char *
edje_edit_part_clip_to_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(NULL);

   return _edje_part_clip_to_get(ed, rp);
}

EAPI Eina_Bool
edje_edit_part_clip_to_set(Evas_Object *obj, const char *part, const char *clip_to)
{
   Edje_Real_Part *clip;
   Evas_Object *o, *oo;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* unset clipping */
   if (!clip_to)
     {
        evas_object_clip_set(rp->object, ed->base.clipper);
        if ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
            (rp->typedata.swallow) && (rp->typedata.swallow->swallowed_object))
          evas_object_clip_set(rp->typedata.swallow->swallowed_object, ed->base.clipper);

        rp->part->clip_to_id = -1;

        edje_object_calc_force(obj);

        return EINA_TRUE;
     }

   /* set clipping */
   //printf("Set clip_to for part: %s [to: %s]\n", part, clip_to);
   clip = _edje_real_part_get(ed, clip_to);
   if (!clip || !clip->part) return EINA_FALSE;
   o = clip->object;
   while ((oo = evas_object_clip_get(o)))
     {
        if (o == rp->object)
          return EINA_FALSE;
        o = oo;
     }

   rp->part->clip_to_id = clip->part->id;

   if ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
       (rp->typedata.swallow) && (rp->typedata.swallow->swallowed_object))
     evas_object_clip_set(rp->typedata.swallow->swallowed_object, clip->object);

   edje_object_calc_force(obj);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_mouse_events_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(EINA_FALSE);
   return rp->part->mouse_events;
}

EAPI Eina_Bool
edje_edit_part_mouse_events_set(Evas_Object *obj, const char *part, Eina_Bool mouse_events)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if (!rp->object) return EINA_FALSE;

   rp->part->mouse_events = mouse_events ? 1 : 0;

   if (mouse_events)
     {
        evas_object_pass_events_set(rp->object, 0);
        _edje_callbacks_add(rp->object, ed, rp);
     }
   else
     {
        evas_object_pass_events_set(rp->object, 1);
        _edje_callbacks_del(rp->object, ed);
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_anti_alias_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(EINA_FALSE);
   return rp->part->anti_alias;
}

EAPI Eina_Bool
edje_edit_part_anti_alias_set(Evas_Object *obj, const char *part, Eina_Bool anti_alias)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if (!rp->object) return EINA_FALSE;

   rp->part->anti_alias = anti_alias ? 1 : 0;
   evas_object_anti_alias_set(obj, rp->part->anti_alias);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_repeat_events_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(EINA_FALSE);
   return rp->part->repeat_events;
}

EAPI Eina_Bool
edje_edit_part_repeat_events_set(Evas_Object *obj, const char *part, Eina_Bool repeat_events)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if (!rp->object) return EINA_FALSE;

   rp->part->repeat_events = repeat_events ? 1 : 0;

   if (repeat_events)
     evas_object_repeat_events_set(rp->object, 1);
   else
     evas_object_repeat_events_set(rp->object, 0);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_use_alternate_font_metrics_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   return rp->part->use_alternate_font_metrics;
}

EAPI Eina_Bool
edje_edit_part_use_alternate_font_metrics_set(Evas_Object *obj, const char *part, Eina_Bool use)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if ((!rp->object) ||
       ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
        (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)))
     return EINA_FALSE;

   rp->part->use_alternate_font_metrics = use;
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_multiline_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   return rp->part->multiline;
}

EAPI Eina_Bool
edje_edit_part_multiline_set(Evas_Object *obj, const char *part, Eina_Bool multiline)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if ((!rp->object) ||
       ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
        (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)))
     return EINA_FALSE;

   rp->part->multiline = multiline;
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_precise_is_inside_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   return rp->part->precise_is_inside;
}

EAPI Eina_Bool
edje_edit_part_precise_is_inside_set(Evas_Object *obj, const char *part, Eina_Bool precise_is_inside)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if (!rp->object) return EINA_FALSE;

   rp->part->precise_is_inside = precise_is_inside;
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_access_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   return rp->part->access;
}

EAPI Eina_Bool
edje_edit_part_access_set(Evas_Object *obj, const char *part, Eina_Bool access_mode)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if (!rp->object) return EINA_FALSE;

   rp->part->access = access_mode;
   return EINA_TRUE;
}

EAPI Evas_Event_Flags
edje_edit_part_ignore_flags_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   return rp->part->ignore_flags;
}

EAPI Eina_Bool
edje_edit_part_ignore_flags_set(Evas_Object *obj, const char *part, Evas_Event_Flags ignore_flags)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if (!rp->object) return EINA_FALSE;

   rp->part->ignore_flags = ignore_flags;
   return EINA_TRUE;
}

EAPI Evas_Event_Flags
edje_edit_part_mask_flags_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   return rp->part->mask_flags;
}

EAPI Eina_Bool
edje_edit_part_mask_flags_set(Evas_Object *obj, const char *part, Evas_Event_Flags mask_flags)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if (!rp->object) return EINA_FALSE;

   rp->part->mask_flags = mask_flags;
   return EINA_TRUE;
}

EAPI Evas_Object_Pointer_Mode
edje_edit_part_pointer_mode_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   return rp->part->pointer_mode;
}

EAPI Eina_Bool
edje_edit_part_pointer_mode_set(Evas_Object *obj, const char *part, Evas_Object_Pointer_Mode pointer_mode)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if (!rp->object) return EINA_FALSE;

   rp->part->pointer_mode = pointer_mode;
   return EINA_TRUE;
}

EAPI unsigned char
edje_edit_part_cursor_mode_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;

   return rp->part->cursor_mode;
}

EAPI Eina_Bool
edje_edit_part_cursor_mode_set(Evas_Object *obj, const char *part, unsigned char cursor_mode)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   if ((!rp->object) ||
       ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
        (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)))
     return EINA_FALSE;

   rp->part->cursor_mode = cursor_mode;
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_scale_set(Evas_Object *obj, const char *part, Eina_Bool scale)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   rp->part->scale = scale ? 1 : 0;
   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_scale_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   return rp->part->scale;
}

EAPI const char *
edje_edit_part_source_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(NULL);

   //printf("Get source for part: %s\n", part);
   if (!rp->part->source) return NULL;

   return eina_stringshare_add(rp->part->source);
}

static Eina_Bool
_check_recursive_reference(Edje *ed, const char *source, Eina_List *group_path, Edje_Part *part)
{
   char *data;
   Edje_Part_Collection_Directory_Entry *pce;
   Eina_List *l, *part_list, *pll;
   Eina_Bool no_ref = EINA_TRUE;
   Eina_Stringshare *part_name, *part_source = NULL;
   Edje_Part_Type type;

   if (!source) return EINA_TRUE;

   pce = eina_hash_find(ed->file->collection, source);

   /* forcing collection load into memory */
   Evas_Object *part_obj = edje_edit_object_add(ed->base.evas);
   edje_object_file_set(part_obj, ed->file->path, pce->entry);
   /* Go through every part to find parts with type GROUP */
   part_list = edje_edit_parts_list_get(part_obj);
   EINA_LIST_FOREACH(part_list, pll, part_name)
     {
        eina_stringshare_del(part_source);
        part_source = edje_edit_part_source_get(part_obj, part_name);
        type = edje_edit_part_type_get(part_obj, part_name);
        if ((type == EDJE_PART_TYPE_GROUP) && part_source)
          {
             /* Make sure that this group isn't already in the tree of parents */
             EINA_LIST_FOREACH(group_path, l, data)
               {
                  if (data == part_source)
                    {
                       no_ref = EINA_FALSE;
                       goto end;
                    }
               }
             group_path = eina_list_append(group_path, source);
             no_ref &= _check_recursive_reference(ed, part_source, group_path, part);
          }

        /* We did a loop here... this part doesn't have source yet,
           but if it will set, it'll be a recursive reference. */
        if (!strcmp(part_name, part->name)) /* TODO: check if part->name is also stringshare and values can be compared with == */
          {
             no_ref = EINA_FALSE;
             goto end;
          }
     }
end:
   eina_stringshare_del(part_source);
   edje_edit_string_list_free(part_list);
   evas_object_del(part_obj);
   return no_ref;
}

EAPI Eina_Bool
edje_edit_part_source_set(Evas_Object *obj, const char *part, const char *source)
{
   GET_RP_OR_RETURN(EINA_FALSE);

   Evas_Object *child_obj;
   Eina_List *group_path = NULL;
   //printf("Set source for part: %s [source: %s]\n", part, source);

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_GROUP:
        /* find source group */
        if (!_check_recursive_reference(ed, source, group_path, rp->part))
          return EINA_FALSE;

        if ((rp->typedata.swallow) && (rp->typedata.swallow->swallowed_object))
          {
             evas_object_del(rp->typedata.swallow->swallowed_object);
             _edje_real_part_swallow_clear(ed, rp);
          }
        if (source)
          {
             child_obj = edje_object_add(ed->base.evas);
             edje_object_file_set(child_obj, ed->file->path, source);
             _edje_real_part_swallow(ed, rp, child_obj, EINA_TRUE);
          }
        EINA_FALLTHROUGH;

      // this fall through case is intentional
      case EDJE_PART_TYPE_TEXTBLOCK:
        if (source) _edje_if_string_replace(ed, &rp->part->source, source);
        else _edje_if_string_free(ed, &rp->part->source);
        return EINA_TRUE;

      case EDJE_PART_TYPE_EXTERNAL: //EXTERNAL part has source property but it cannot be changed
        break;

      default:
        break;
     }
   return EINA_FALSE;
}

#define TEXT_BLOCK_SOURCE_GET(N)                                          \
  EAPI const char *                                                       \
  edje_edit_part_source ## N ## _get(Evas_Object * obj, const char *part) \
  {                                                                       \
     GET_RP_OR_RETURN(NULL);                                              \
     if (!rp->part->source ## N) return NULL;                             \
     return eina_stringshare_add(rp->part->source ## N);                  \
  }

#define TEXT_BLOCK_SOURCE_SET(N)                                                             \
  EAPI Eina_Bool                                                                             \
  edje_edit_part_source ## N ##_set(Evas_Object * obj, const char *part, const char *source) \
  {                                                                                          \
     GET_RP_OR_RETURN(EINA_FALSE);                                                           \
                                                                                             \
     if (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)                                         \
       return EINA_FALSE;                                                                    \
                                                                                             \
     eina_stringshare_del(rp->part->source ## N);                                            \
     if (source)                                                                             \
       rp->part->source ## N = eina_stringshare_add(source);                                 \
     else                                                                                    \
       rp->part->source ## N = NULL;                                                         \
     edje_object_calc_force(obj);                                                            \
     return EINA_TRUE;                                                                       \
  }
TEXT_BLOCK_SOURCE_GET(2);
TEXT_BLOCK_SOURCE_GET(3);
TEXT_BLOCK_SOURCE_GET(4);
TEXT_BLOCK_SOURCE_GET(5);
TEXT_BLOCK_SOURCE_GET(6);

TEXT_BLOCK_SOURCE_SET(2);
TEXT_BLOCK_SOURCE_SET(3);
TEXT_BLOCK_SOURCE_SET(4);
TEXT_BLOCK_SOURCE_SET(5);
TEXT_BLOCK_SOURCE_SET(6);

EAPI int
edje_edit_part_drag_x_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   return rp->part->dragable.x;
}

EAPI Eina_Bool
edje_edit_part_drag_x_set(Evas_Object *obj, const char *part, int drag)
{
   GET_RP_OR_RETURN(EINA_FALSE);
   rp->part->dragable.x = drag;

   if (!drag && !rp->part->dragable.y)
     {
        free(rp->drag);
        rp->drag = NULL;
        return EINA_TRUE;
     }

   if (rp->drag) return EINA_TRUE;

   rp->drag = _alloc(sizeof (Edje_Real_Part_Drag));
   if (!rp->drag) return EINA_FALSE;

   rp->drag->step.x = rp->part->dragable.step_x;
   rp->drag->step.y = rp->part->dragable.step_y;
   return EINA_TRUE;
}

EAPI int
edje_edit_part_drag_y_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);
   return rp->part->dragable.y;
}

EAPI Eina_Bool
edje_edit_part_drag_y_set(Evas_Object *obj, const char *part, int drag)
{
   GET_RP_OR_RETURN(EINA_FALSE);
   rp->part->dragable.y = drag;

   if (!drag && !rp->part->dragable.x)
     {
        free(rp->drag);
        rp->drag = NULL;
        return EINA_TRUE;
     }

   if (rp->drag) return EINA_TRUE;

   rp->drag = _alloc(sizeof (Edje_Real_Part_Drag));
   if (!rp->drag) return EINA_FALSE;

   rp->drag->step.x = rp->part->dragable.step_x;
   rp->drag->step.y = rp->part->dragable.step_y;
   return EINA_TRUE;
}

#define FUNC_PART_DRAG_INT(Class, Value)                                                  \
  EAPI int                                                                                \
  edje_edit_part_drag_##Class##_##Value##_get(Evas_Object * obj, const char *part)        \
  {                                                                                       \
     GET_RP_OR_RETURN(0);                                                                 \
     return rp->part->dragable.Class##_##Value;                                           \
  }                                                                                       \
  EAPI Eina_Bool                                                                          \
  edje_edit_part_drag_##Class##_##Value##_set(Evas_Object * obj, const char *part, int v) \
  {                                                                                       \
     GET_RP_OR_RETURN(EINA_FALSE);                                                        \
     rp->part->dragable.Class##_##Value = v;                                              \
     return EINA_TRUE;                                                                    \
  }

FUNC_PART_DRAG_INT(step, x);
FUNC_PART_DRAG_INT(step, y);
FUNC_PART_DRAG_INT(count, x);
FUNC_PART_DRAG_INT(count, y);

#define FUNC_PART_DRAG_ID(Id)                                                        \
  EAPI const char *                                                                  \
  edje_edit_part_drag_##Id##_get(Evas_Object * obj, const char *part)                \
  {                                                                                  \
     Edje_Real_Part *p;                                                              \
                                                                                     \
     GET_RP_OR_RETURN(NULL);                                                         \
                                                                                     \
     if (rp->part->dragable.Id##_id < 0)                                             \
       return NULL;                                                                  \
                                                                                     \
     p = ed->table_parts[rp->part->dragable.Id##_id];                                \
     return eina_stringshare_add(p->part->name);                                     \
  }                                                                                  \
  EAPI Eina_Bool                                                                     \
  edje_edit_part_drag_##Id##_set(Evas_Object * obj, const char *part, const char *e) \
  {                                                                                  \
     Edje_Real_Part *e_part;                                                         \
                                                                                     \
     eina_error_set(0);                                                              \
     if ((!obj) || (!part))                                                          \
       return EINA_FALSE;                                                            \
                                                                                     \
     GET_RP_OR_RETURN(EINA_FALSE);                                                   \
     if (!e)                                                                         \
       {                                                                             \
          rp->part->dragable.Id##_id = -1;                                           \
          return EINA_TRUE;                                                          \
       }                                                                             \
     e_part = _edje_real_part_get(ed, e);                                            \
     if (!e_part) return EINA_FALSE;                                                 \
     rp->part->dragable.Id##_id = e_part->part->id;                                  \
     return EINA_TRUE;                                                               \
  }

FUNC_PART_DRAG_ID(confine);
FUNC_PART_DRAG_ID(event);
FUNC_PART_DRAG_ID(threshold);

/***************************/
/*         BOX API         */
/***************************/

#define FUNC_STATE_BOX_LAYOUT(Layout)                                                                                    \
  EAPI Eina_Stringshare *                                                                                                \
  edje_edit_state_box_##Layout##_get(Evas_Object * obj, const char *part, const char *state, double value)               \
  {                                                                                                                      \
     GET_PD_OR_RETURN(0)                                                                                                 \
     if (rp->part->type == EDJE_PART_TYPE_BOX)                                                                           \
       {                                                                                                                 \
          Edje_Part_Description_Box *box;                                                                                \
          box = (Edje_Part_Description_Box *)pd;                                                                         \
          return eina_stringshare_add(box->box.Layout);                                                                  \
       }                                                                                                                 \
     return NULL;                                                                                                        \
  }                                                                                                                      \
  EAPI Eina_Bool                                                                                                         \
  edje_edit_state_box_##Layout##_set(Evas_Object * obj, const char *part, const char *state, double value, char *layout) \
  {                                                                                                                      \
     GET_PD_OR_RETURN(EINA_FALSE)                                                                                        \
     if (rp->part->type == EDJE_PART_TYPE_BOX)                                                                           \
       {                                                                                                                 \
          Edje_Part_Description_Box *box;                                                                                \
          box = (Edje_Part_Description_Box *)pd;                                                                         \
          box->box.Layout = layout;                                                                                      \
       }                                                                                                                 \
     else                                                                                                                \
       return EINA_FALSE;                                                                                                \
     return EINA_TRUE;                                                                                                   \
  }

FUNC_STATE_BOX_LAYOUT(layout);
FUNC_STATE_BOX_LAYOUT(alt_layout);

/***************************/
/*        TABLE API        */
/***************************/

EAPI unsigned char
edje_edit_state_table_homogeneous_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(0)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *table;
         table = (Edje_Part_Description_Table *)pd;
         return table->table.homogeneous;
      }

      default:
        return 0;
     }
   return 0;
}

EAPI Eina_Bool
edje_edit_state_table_homogeneous_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char homogeneous)
{
   GET_PD_OR_RETURN(EINA_FALSE)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *table;
         table = (Edje_Part_Description_Table *)pd;
         table->table.homogeneous = homogeneous;
         break;
      }

      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/***************************/
/*     BOX & TABLE API     */
/***************************/

#define FUNC_CONTAINER_BOOL(CLASS, VALUE) \
EAPI Eina_Bool \
edje_edit_state_container_##CLASS##_##VALUE##_get(Evas_Object *obj, const char *part, const char *state, double value) \
{ \
   Eina_Bool val; \
   GET_PD_OR_RETURN(EINA_FALSE) \
   switch (rp->part->type) \
     { \
      case EDJE_PART_TYPE_TABLE: \
      { \
         Edje_Part_Description_Table *table; \
         table = (Edje_Part_Description_Table *)pd; \
         val = table->table.CLASS.VALUE; \
         break; \
      } \
      case EDJE_PART_TYPE_BOX: \
      { \
         Edje_Part_Description_Box *box; \
         box = (Edje_Part_Description_Box *)pd; \
         val = box->box.CLASS.VALUE; \
         break; \
      } \
      default: \
       val = EINA_FALSE; \
     } \
   return val; \
} \
EAPI Eina_Bool \
edje_edit_state_container_##CLASS##_##VALUE##_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool new_val) \
{ \
   GET_PD_OR_RETURN(EINA_FALSE) \
   switch (rp->part->type) \
     { \
      case EDJE_PART_TYPE_TABLE: \
      { \
         Edje_Part_Description_Table *table; \
         table = (Edje_Part_Description_Table *)pd; \
         table->table.CLASS.VALUE = new_val; \
         break; \
      } \
      case EDJE_PART_TYPE_BOX: \
      { \
         Edje_Part_Description_Box *box; \
         box = (Edje_Part_Description_Box *)pd; \
         box->box.CLASS.VALUE = new_val; \
         break; \
      } \
      default: \
        return EINA_FALSE; \
     } \
   return EINA_TRUE; \
}

FUNC_CONTAINER_BOOL(min, v)
FUNC_CONTAINER_BOOL(min, h)

#undef FUNC_CONTAINER_BOOL

#define FUNC_CONTAINER_INT(CLASS, VALUE) \
EAPI int \
edje_edit_state_container_##CLASS##_##VALUE##_get(Evas_Object *obj, const char *part, const char *state, double value) \
{ \
   int val; \
   GET_PD_OR_RETURN(EINA_FALSE) \
   switch (rp->part->type) \
     { \
      case EDJE_PART_TYPE_TABLE: \
      { \
         Edje_Part_Description_Table *table; \
         table = (Edje_Part_Description_Table *)pd; \
         val = table->table.CLASS.VALUE; \
         break; \
      } \
      case EDJE_PART_TYPE_BOX: \
      { \
         Edje_Part_Description_Box *box; \
         box = (Edje_Part_Description_Box *)pd; \
         val = box->box.CLASS.VALUE; \
         break; \
      } \
      default: \
        val = 0; \
     } \
   return val; \
} \
EAPI Eina_Bool \
edje_edit_state_container_##CLASS##_##VALUE##_set(Evas_Object *obj, const char *part, const char *state, double value, int new_val) \
{ \
   GET_PD_OR_RETURN(EINA_FALSE) \
   switch (rp->part->type) \
     { \
      case EDJE_PART_TYPE_TABLE: \
      { \
         Edje_Part_Description_Table *table; \
         table = (Edje_Part_Description_Table *)pd; \
         table->table.CLASS.VALUE = new_val; \
         break; \
      } \
      case EDJE_PART_TYPE_BOX: \
      { \
         Edje_Part_Description_Box *box; \
         box = (Edje_Part_Description_Box *)pd; \
         box->box.CLASS.VALUE = new_val; \
         break; \
      } \
      default: \
        return EINA_FALSE; \
     } \
   return EINA_TRUE; \
}

FUNC_CONTAINER_INT(padding, x)
FUNC_CONTAINER_INT(padding, y)

#undef FUNC_CONTAINER_INT

#define FUNC_CONTAINER_DOUBLE(CLASS, VALUE) \
EAPI double \
edje_edit_state_container_##CLASS##_##VALUE##_get(Evas_Object *obj, const char *part, const char *state, double value) \
{ \
   double val; \
   GET_PD_OR_RETURN(0.0) \
   switch (rp->part->type) \
     { \
      case EDJE_PART_TYPE_TABLE: \
      { \
         Edje_Part_Description_Table *table; \
         table = (Edje_Part_Description_Table *)pd; \
         val = FROM_DOUBLE(table->table.CLASS.VALUE); \
         break; \
      } \
      case EDJE_PART_TYPE_BOX: \
      { \
         Edje_Part_Description_Box *box; \
         box = (Edje_Part_Description_Box *)pd; \
         val = FROM_DOUBLE(box->box.CLASS.VALUE); \
         break; \
      } \
      default: \
         val = 0.0; \
     } \
   return val; \
} \
EAPI Eina_Bool \
edje_edit_state_container_##CLASS##_##VALUE##_set(Evas_Object *obj, const char *part, const char *state, double value, double new_val) \
{ \
   GET_PD_OR_RETURN(EINA_FALSE) \
   switch (rp->part->type) \
     { \
      case EDJE_PART_TYPE_TABLE: \
      { \
         Edje_Part_Description_Table *table; \
         table = (Edje_Part_Description_Table *)pd; \
         table->table.CLASS.VALUE = TO_DOUBLE(new_val); \
         break; \
      } \
      case EDJE_PART_TYPE_BOX: \
      { \
         Edje_Part_Description_Box *box; \
         box = (Edje_Part_Description_Box *)pd; \
         box->box.CLASS.VALUE = TO_DOUBLE(new_val); \
         break; \
      } \
      default: \
        return EINA_FALSE; \
     } \
   return EINA_TRUE; \
}

FUNC_CONTAINER_DOUBLE(align, x)
FUNC_CONTAINER_DOUBLE(align, y)

#undef FUNC_CONTAINER_DOUBLE

EAPI Eina_Bool
edje_edit_state_container_align_get(Evas_Object *obj, const char *part, const char *state, double value, double *x, double *y)
{
   GET_PD_OR_RETURN(EINA_FALSE)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *table;
         table = (Edje_Part_Description_Table *)pd;
         if (x) *x = FROM_DOUBLE(table->table.align.x);
         if (y) *y = FROM_DOUBLE(table->table.align.y);
         break;
      }

      case EDJE_PART_TYPE_BOX:
      {
         Edje_Part_Description_Box *box;
         box = (Edje_Part_Description_Box *)pd;
         if (x) *x = FROM_DOUBLE(box->box.align.x);
         if (y) *y = FROM_DOUBLE(box->box.align.y);
         break;
      }

      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_container_align_set(Evas_Object *obj, const char *part, const char *state, double value, double x, double y)
{
   GET_PD_OR_RETURN(EINA_FALSE)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *table;
         table = (Edje_Part_Description_Table *)pd;
         table->table.align.x = TO_DOUBLE(x);
         table->table.align.y = TO_DOUBLE(y);
         break;
      }

      case EDJE_PART_TYPE_BOX:
      {
         Edje_Part_Description_Box *box;
         box = (Edje_Part_Description_Box *)pd;
         box->box.align.x = x;
         box->box.align.y = y;
         break;
      }

      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_container_padding_get(Evas_Object *obj, const char *part, const char *state, double value, int *x, int *y)
{
   GET_PD_OR_RETURN(EINA_FALSE)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *table;
         table = (Edje_Part_Description_Table *)pd;
         if (x) *x = table->table.padding.x;
         if (y) *y = table->table.padding.y;
         break;
      }

      case EDJE_PART_TYPE_BOX:
      {
         Edje_Part_Description_Box *box;
         box = (Edje_Part_Description_Box *)pd;
         if (x) *x = box->box.padding.x;
         if (y) *y = box->box.padding.y;
         break;
      }

      default:
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_container_padding_set(Evas_Object *obj, const char *part, const char *state, double value, int x, int y)
{
   GET_PD_OR_RETURN(EINA_FALSE)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *table;
         table = (Edje_Part_Description_Table *)pd;
         table->table.padding.x = x;
         table->table.padding.y = y;
         break;
      }

      case EDJE_PART_TYPE_BOX:
      {
         Edje_Part_Description_Box *box;
         box = (Edje_Part_Description_Box *)pd;
         box->box.padding.x = x;
         box->box.padding.y = y;
         break;
      }

      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_container_min_get(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool *h, Eina_Bool *v)
{
   GET_PD_OR_RETURN(EINA_FALSE)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *table;
         table = (Edje_Part_Description_Table *)pd;
         if (h) *h = table->table.min.h;
         if (v) *v = table->table.min.v;
         break;
      }

      case EDJE_PART_TYPE_BOX:
      {
         Edje_Part_Description_Box *box;
         box = (Edje_Part_Description_Box *)pd;
         if (h) *h = box->box.min.h;
         if (v) *v = box->box.min.v;
         break;
      }

      default:
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_container_min_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool h, Eina_Bool v)
{
   GET_PD_OR_RETURN(EINA_FALSE)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *table;
         table = (Edje_Part_Description_Table *)pd;
         table->table.min.h = h;
         table->table.min.v = v;
         break;
      }

      case EDJE_PART_TYPE_BOX:
      {
         Edje_Part_Description_Box *box;
         box = (Edje_Part_Description_Box *)pd;
         box->box.min.h = h;
         box->box.min.v = v;
         break;
      }

      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/***************************/
/*  BOX & TABLE ITEMS API  */
/***************************/

static void
_edje_edit_part_item_insert(Edje_Part *ep, unsigned int item_position, const char *item_name, const char *source_group)
{
   Edje_Pack_Element *item;
   unsigned int i;

   ep->items_count++;
   ep->items = realloc(ep->items, sizeof (Edje_Pack_Element *) * ep->items_count);
   item = _alloc(sizeof (Edje_Pack_Element));
   /* shift all items to the end */
   for (i = ep->items_count - 1; i > item_position; i--)
     ep->items[i] = ep->items[i - 1];
   ep->items[item_position] = item;

   item->type = EDJE_PART_TYPE_GROUP;
   item->name = eina_stringshare_add(item_name);
   item->source = eina_stringshare_add(source_group);
   item->min.w = 0;
   item->min.h = 0;
   item->prefer.w = 0;
   item->prefer.h = 0;
   item->max.w = -1;
   item->max.h = -1;
   item->padding.l = 0;
   item->padding.r = 0;
   item->padding.t = 0;
   item->padding.b = 0;
   item->align.x = FROM_DOUBLE(0.5);
   item->align.y = FROM_DOUBLE(0.5);
   item->weight.x = FROM_DOUBLE(0.0);
   item->weight.y = FROM_DOUBLE(0.0);
   item->aspect.w = 0;
   item->aspect.h = 0;
   item->aspect.mode = EDJE_ASPECT_CONTROL_NONE;
   item->options = NULL;
   item->col = -1;
   item->row = -1;
   item->colspan = 1;
   item->rowspan = 1;
   item->spread.w = 1;
   item->spread.h = 1;
}

EAPI Eina_Bool
edje_edit_part_item_append(Evas_Object *obj, const char *part, const char *item_name, const char *source_group)
{
   Edje_Part *ep;
   unsigned int i;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   /* check if a source group is exists. */
   if (!eina_hash_find(ed->file->collection, source_group))
     return EINA_FALSE;

   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          return EINA_FALSE;
     }

   _edje_edit_part_item_insert(ep, ep->items_count, item_name, source_group);

   return EINA_TRUE;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_insert_before(Evas_Object *obj, const char *part, const char *item_name, const char *item_before, const char *source_group)
{
   Edje_Part *ep;
   unsigned int i;
   int item_before_position = -1;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   /* check if a source group is exists. */
   if (!eina_hash_find(ed->file->collection, source_group))
     return EINA_FALSE;

   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          return EINA_FALSE;
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_before)))
          item_before_position = i;
     }

   if (item_before_position == -1) return EINA_FALSE;

   _edje_edit_part_item_insert(ep, (unsigned)item_before_position, item_name, source_group);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_item_insert_before_index(Evas_Object *obj, const char *part, const char *item_name, unsigned int index, const char *source_group)
{
   Edje_Part *ep;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   /* check if a source group is exists. */
   if (!eina_hash_find(ed->file->collection, source_group))
     return EINA_FALSE;

   _edje_edit_part_item_insert(ep, index, item_name, source_group);

   return EINA_TRUE;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_insert_after(Evas_Object *obj, const char *part, const char *item_name, const char *item_after, const char *source_group)
{
   Edje_Part *ep;
   unsigned int i;
   int item_after_position = -1;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   /* check if a source group is exists. */
   if (!eina_hash_find(ed->file->collection, source_group))
     return EINA_FALSE;

   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          return EINA_FALSE;
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_after)))
          item_after_position = i;
     }

   if (item_after_position == -1) return EINA_FALSE;

   item_after_position++;
   _edje_edit_part_item_insert(ep, (unsigned)item_after_position, item_name, source_group);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_item_insert_after_index(Evas_Object *obj, const char *part, const char *item_name, unsigned int index, const char *source_group)
{
   Edje_Part *ep;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   /* check if a source group is exists. */
   if (!eina_hash_find(ed->file->collection, source_group))
     return EINA_FALSE;

   _edje_edit_part_item_insert(ep, index, item_name, source_group);

   return EINA_TRUE;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_insert_at(Evas_Object *obj, const char *part, const char *item_name, const char *source_group, unsigned int place)
{
   Edje_Part *ep;
   unsigned int i;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;
   if (place > ep->items_count - 1)
     return EINA_FALSE;

   if (!ed->file) return EINA_FALSE;

   /* check if a source group is exists. */
   if (!eina_hash_find(ed->file->collection, source_group))
     return EINA_FALSE;

   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          return EINA_FALSE;
     }

   _edje_edit_part_item_insert(ep, place, item_name, source_group);

   return EINA_TRUE;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_move_below(Evas_Object *obj, const char *part, const char *item_name)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item;
   unsigned int item_place = 0;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          {
             item_place = i;
             break;
          }
     }

   if (!item_place)
     return EINA_FALSE;

   item = ep->items[item_place - 1];
   ep->items[item_place - 1] = ep->items[item_place];
   ep->items[item_place] = item;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_item_move_below_index(Evas_Object *obj, const char *part, unsigned int index)
{
   Edje_Part *ep;
   Edje_Pack_Element *item;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   item = ep->items[index - 1];
   ep->items[index - 1] = ep->items[index];
   ep->items[index] = item;

   return EINA_TRUE;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_move_above(Evas_Object *obj, const char *part, const char *item_name)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item;
   unsigned int item_place = 0;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          {
             item_place = i;
             break;
          }
     }

   if (item_place == ep->items_count - 1)
     return EINA_FALSE;

   item = ep->items[item_place + 1];
   ep->items[item_place + 1] = ep->items[item_place];
   ep->items[item_place] = item;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_item_move_above_index(Evas_Object *obj, const char *part, unsigned int index)
{
   Edje_Part *ep;
   Edje_Pack_Element *item;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   item = ep->items[index + 1];
   ep->items[index + 1] = ep->items[index];
   ep->items[index] = item;

   return EINA_TRUE;
}

/* deprecated */
EAPI Eina_List *
edje_edit_part_items_list_get(Evas_Object *obj, const char *part)
{
   Edje_Part *ep;
   unsigned int i;
   Eina_List *items_list = NULL;
   GET_RP_OR_RETURN(NULL);
   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return NULL;
   ep = rp->part;
   if (!ed->file) return NULL;

   for (i = 0; i < ep->items_count; ++i)
     items_list = eina_list_append(items_list,
                                   eina_stringshare_add(ep->items[i]->name));

   return items_list;
}

EAPI int
edje_edit_part_items_count_get(Evas_Object *obj, const char *part)
{
   Edje_Part *ep;

   GET_RP_OR_RETURN(-1);
   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return -1;

   ep = rp->part;
   if (!ed->file) return -1;

   return  ep->items_count;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_del(Evas_Object *obj, const char *part, const char *name)
{
   Edje_Part *ep;
   Edje_Pack_Element *item;
   unsigned int i;
   GET_RP_OR_RETURN(EINA_FALSE);
   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;
   ep = rp->part;
   if (!ed->file) return EINA_FALSE;

   for (i = 0; i < ep->items_count; ++i)
     {
        item = ep->items[i];
        if (!strcmp(name, item->name))
          break;
     }
   if (i == ep->items_count)
     {
        WRN("Unable to delete item \"%s\". It does not exist.", name);
        return EINA_FALSE;
     }

   {
      Edje_Pack_Element **tmp;
      _edje_if_string_free(ed, &item->name);
      --ep->items_count;

      while (i < ep->items_count)
        {
           ep->items[i] = ep->items[i + 1];
           i++;
        }

      if (ep->items_count != 0)
        {
           tmp = realloc(ep->items, sizeof(Edje_Pack_Element *) * ep->items_count);
           if (!tmp)
             {
                free(item);
                return EINA_FALSE;
             }
           ep->items = tmp;
        }
      else
        ep->items = NULL;
   }

   GET_EED_OR_RETURN(EINA_FALSE);
   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_item_index_del(Evas_Object *obj, const char *part, unsigned int index)
{
   Edje_Part *ep;
   Edje_Pack_Element *item;

   GET_RP_OR_RETURN(EINA_FALSE);
   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;
   ep = rp->part;
   if (!ed->file) return EINA_FALSE;

   item = ep->items[index];
   {
      Edje_Pack_Element **tmp;
      _edje_if_string_free(ed, &item->name);
      --ep->items_count;

      while (index < ep->items_count)
        {
           ep->items[index] = ep->items[index + 1];
           index++;
        }

      if (ep->items_count != 0)
        {
           tmp = realloc(ep->items, sizeof(Edje_Pack_Element *) * ep->items_count);
           if (!tmp)
             {
                free(item);
                return EINA_FALSE;
             }
           ep->items = tmp;
        }
      else
        ep->items = NULL;
   }

   GET_EED_OR_RETURN(EINA_FALSE);
   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_item_index_name_set(Evas_Object *obj, const char *part, unsigned int index, const char *name)
{
   Edje_Part *ep;
   unsigned int i;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;
   if (rp->part->items_count < index)
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   /* check if a part with given name is exists. */
   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, name)))
          return EINA_FALSE;
     }

   eina_stringshare_del(ep->items[index]->name);
   ep->items[index]->name = eina_stringshare_add(name);

   return EINA_TRUE;
}

EAPI const char *
edje_edit_part_item_index_name_get(Evas_Object *obj, const char *part, unsigned int index)
{
   Edje_Part *ep;

   GET_RP_OR_RETURN(NULL);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return NULL;
   if (rp->part->items_count < index)
     return NULL;

   ep = rp->part;

   if (!ed->file) return NULL;

   return eina_stringshare_add(ep->items[index]->name);
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_source_set(Evas_Object *obj, const char *part, const char *item_name, const char *source_group)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item = NULL;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   /* check if a source group is exists. */
   if (!eina_hash_find(ed->file->collection, source_group))
     return EINA_FALSE;

   /* check if item is exists and get it. */
   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          {
             item = ep->items[i];
             break;
          }
     }
   if (!item) return EINA_FALSE;

   item->source = eina_stringshare_add(source_group);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_item_index_source_set(Evas_Object *obj, const char *part, unsigned int index, const char *source_group)
{
   Edje_Part *ep;

   GET_RP_OR_RETURN(EINA_FALSE);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (!ed->file) return EINA_FALSE;

   /* check if a source group is exists. */
   if (!eina_hash_find(ed->file->collection, source_group))
     return EINA_FALSE;

   ep->items[index]->source = eina_stringshare_add(source_group);

   return EINA_TRUE;
}

EINA_DEPRECATED
EAPI const char *
edje_edit_part_item_source_get(Evas_Object *obj, const char *part, const char *item_name)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item = NULL;

   GET_RP_OR_RETURN(NULL);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return NULL;

   ep = rp->part;

   if (!ed->file) return NULL;

   /* check if item is exists and get it. */
   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          {
             item = ep->items[i];
             break;
          }
     }
   if (!item) return NULL;

   return eina_stringshare_add(item->source);
}

EAPI const char *
edje_edit_part_item_index_source_get(Evas_Object *obj, const char *part, unsigned int index)
{
   Edje_Part *ep;

   GET_RP_OR_RETURN(NULL);

   /* There is only Box and Table is allowed. */
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return NULL;

   ep = rp->part;

   if (!ed->file) return NULL;

   return eina_stringshare_add(ep->items[index]->source);
}

/* deprecated */
#define FUNC_PART_ITEM_INT(Class, Value, Min)                                                                    \
  EAPI int                                                                                                       \
  edje_edit_part_item_##Class##_##Value##_get(Evas_Object * obj, const char *part, const char *item_name)        \
  {                                                                                                              \
     Edje_Part *ep;                                                                                              \
     unsigned int i;                                                                                             \
     Edje_Pack_Element *item = NULL;                                                                             \
     if ((!obj) || (!part) || (!item_name))                                                                      \
       return Min;                                                                                               \
     GET_RP_OR_RETURN(Min);                                                                                      \
     ep = rp->part;                                                                                              \
     for (i = 0; i < ep->items_count; ++i)                                                                       \
       {                                                                                                         \
          if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))                                    \
            {                                                                                                    \
               item = ep->items[i];                                                                              \
               break;                                                                                            \
            }                                                                                                    \
       }                                                                                                         \
     if (!item) return Min;                                                                                      \
     return item->Class.Value;                                                                                   \
  }                                                                                                              \
  EAPI Eina_Bool                                                                                                 \
  edje_edit_part_item_##Class##_##Value##_set(Evas_Object * obj, const char *part, const char *item_name, int v) \
  {                                                                                                              \
     Edje_Part *ep;                                                                                              \
     unsigned int i;                                                                                             \
     Edje_Pack_Element *item = NULL;                                                                             \
     if ((!obj) || (!part) || (!item_name))                                                                      \
       return EINA_FALSE;                                                                                        \
     if (v < Min) return EINA_FALSE;                                                                             \
     GET_RP_OR_RETURN(EINA_FALSE);                                                                               \
     ep = rp->part;                                                                                              \
     if ((rp->part->type != EDJE_PART_TYPE_BOX) &&                                                               \
         (rp->part->type != EDJE_PART_TYPE_TABLE))                                                               \
       return EINA_FALSE;                                                                                        \
     for (i = 0; i < ep->items_count; ++i)                                                                       \
       {                                                                                                         \
          if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))                                    \
            {                                                                                                    \
               item = ep->items[i];                                                                              \
               break;                                                                                            \
            }                                                                                                    \
       }                                                                                                         \
     if (!item) return EINA_FALSE;                                                                               \
     item->Class.Value = v;                                                                                      \
     return EINA_TRUE;                                                                                           \
  }

FUNC_PART_ITEM_INT(min, w, 0);
FUNC_PART_ITEM_INT(min, h, 0);
FUNC_PART_ITEM_INT(max, w, -1);
FUNC_PART_ITEM_INT(max, h, -1);
FUNC_PART_ITEM_INT(aspect, w, 0);
FUNC_PART_ITEM_INT(aspect, h, 0);
FUNC_PART_ITEM_INT(prefer, w, 0);
FUNC_PART_ITEM_INT(prefer, h, 0);
FUNC_PART_ITEM_INT(spread, w, 0);
FUNC_PART_ITEM_INT(spread, h, 0);

#define FUNC_PART_ITEM_INDEX_INT(Class, Value, Min)                                                                 \
  EAPI int                                                                                                          \
  edje_edit_part_item_index_##Class##_##Value##_get(Evas_Object * obj, const char *part, unsigned int index)        \
  {                                                                                                                 \
     Edje_Part *ep;                                                                                                 \
     if ((!obj) || (!part))                                                                                         \
       return Min;                                                                                                  \
     GET_RP_OR_RETURN(Min);                                                                                         \
     ep = rp->part;                                                                                                 \
     if (index >= ep->items_count)                                                                                  \
       return Min;                                                                                                  \
     return ep->items[index]->Class.Value;                                                                          \
  }                                                                                                                 \
  EAPI Eina_Bool                                                                                                    \
  edje_edit_part_item_index_##Class##_##Value##_set(Evas_Object * obj, const char *part, unsigned int index, int v) \
  {                                                                                                                 \
     Edje_Part *ep;                                                                                                 \
     if ((!obj) || (!part))                                                                                         \
       return EINA_FALSE;                                                                                           \
     if (v < Min) return EINA_FALSE;                                                                                \
     GET_RP_OR_RETURN(EINA_FALSE);                                                                                  \
     ep = rp->part;                                                                                                 \
     if ((rp->part->type != EDJE_PART_TYPE_BOX) &&                                                                  \
         (rp->part->type != EDJE_PART_TYPE_TABLE))                                                                  \
       return EINA_FALSE;                                                                                           \
     if (index >= ep->items_count)                                                                                  \
       return Min;                                                                                                  \
     ep->items[index]->Class.Value = v;                                                                             \
     return EINA_TRUE;                                                                                              \
  }

FUNC_PART_ITEM_INDEX_INT(min, w, 0);
FUNC_PART_ITEM_INDEX_INT(min, h, 0);
FUNC_PART_ITEM_INDEX_INT(max, w, -1);
FUNC_PART_ITEM_INDEX_INT(max, h, -1);
FUNC_PART_ITEM_INDEX_INT(aspect, w, 0);
FUNC_PART_ITEM_INDEX_INT(aspect, h, 0);
FUNC_PART_ITEM_INDEX_INT(prefer, w, 0);
FUNC_PART_ITEM_INDEX_INT(prefer, h, 0);
FUNC_PART_ITEM_INDEX_INT(spread, w, 0);
FUNC_PART_ITEM_INDEX_INT(spread, h, 0);

/* deprecated */
EAPI Edje_Aspect_Control
edje_edit_part_item_aspect_mode_get(Evas_Object *obj, const char *part, const char *item_name)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item = NULL;

   if ((!obj) || (!part) || (!item_name))
     return EDJE_ASPECT_CONTROL_NONE;

   GET_RP_OR_RETURN(EDJE_ASPECT_CONTROL_NONE);

   ep = rp->part;
   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          {
             item = ep->items[i];
             break;
          }
     }
   if (!item) return EDJE_ASPECT_CONTROL_NONE;

   return item->aspect.mode;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_aspect_mode_set(Evas_Object *obj, const char *part, const char *item_name, Edje_Aspect_Control mode)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item = NULL;

   if ((!obj) || (!part) || (!item_name))
     return EINA_FALSE;

   GET_RP_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          {
             item = ep->items[i];
             break;
          }
     }
   if (!item) return EINA_FALSE;

   item->aspect.mode = mode;

   return EINA_TRUE;
}

EAPI Edje_Aspect_Control
edje_edit_part_item_index_aspect_mode_get(Evas_Object *obj, const char *part, unsigned int index)
{
   Edje_Part *ep;

   if ((!obj) || (!part))
     return EDJE_ASPECT_CONTROL_NONE;

   GET_RP_OR_RETURN(EDJE_ASPECT_CONTROL_NONE);

   ep = rp->part;
   if (index >= ep->items_count)
     return EDJE_ASPECT_CONTROL_NONE;

   return ep->items[index]->aspect.mode;
}

EAPI Eina_Bool
edje_edit_part_item_index_aspect_mode_set(Evas_Object *obj, const char *part, unsigned int index, Edje_Aspect_Control mode)
{
   Edje_Part *ep;

   if ((!obj) || (!part))
     return EINA_FALSE;

   GET_RP_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;

   ep = rp->part;

   if (index >= ep->items_count)
     return EINA_FALSE;

   ep->items[index]->aspect.mode = mode;

   return EINA_TRUE;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_padding_get(Evas_Object *obj, const char *part, const char *item_name, int *l, int *r, int *t, int *b)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item = NULL;

   if ((!obj) || (!part) || (!item_name))
     return EINA_FALSE;

   GET_RP_OR_RETURN(EINA_FALSE);

   ep = rp->part;
   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          {
             item = ep->items[i];
             break;
          }
     }
   if (!item) return EINA_FALSE;

   if (l) *l = item->padding.l;
   if (t) *t = item->padding.t;
   if (r) *r = item->padding.r;
   if (b) *b = item->padding.b;

   return EINA_TRUE;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_padding_set(Evas_Object *obj, const char *part, const char *item_name, int l, int r, int t, int b)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item = NULL;

   if ((!obj) || (!part) || (!item_name))
     return EINA_FALSE;

   GET_RP_OR_RETURN(EINA_FALSE);

   ep = rp->part;
   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          {
             item = ep->items[i];
             break;
          }
     }
   if (!item) return EINA_FALSE;

   if (l > -1) item->padding.l = l;
   else return EINA_FALSE;
   if (t > -1) item->padding.t = t;
   else return EINA_FALSE;
   if (r > -1) item->padding.r = r;
   else return EINA_FALSE;
   if (b > -1) item->padding.b = b;
   else return EINA_FALSE;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_item_index_padding_get(Evas_Object *obj, const char *part, unsigned int index, int *l, int *r, int *t, int *b)
{
   Edje_Part *ep;

   if ((!obj) || (!part))
     return EINA_FALSE;

   GET_RP_OR_RETURN(EINA_FALSE);

   ep = rp->part;
   if (l) *l = ep->items[index]->padding.l;
   if (t) *t = ep->items[index]->padding.t;
   if (r) *r = ep->items[index]->padding.r;
   if (b) *b = ep->items[index]->padding.b;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_part_item_index_padding_set(Evas_Object *obj, const char *part, unsigned int index, int l, int r, int t, int b)
{
   Edje_Part *ep;

   if ((!obj) || (!part))
     return EINA_FALSE;

   GET_RP_OR_RETURN(EINA_FALSE);

   ep = rp->part;
   if (l > -1) ep->items[index]->padding.l = l;
   else return EINA_FALSE;
   if (t > -1) ep->items[index]->padding.t = t;
   else return EINA_FALSE;
   if (r > -1) ep->items[index]->padding.r = r;
   else return EINA_FALSE;
   if (b > -1) ep->items[index]->padding.b = b;
   else return EINA_FALSE;

   return EINA_TRUE;
}

/* deprecated */
#define FUNC_PART_ITEM_DOUBLE(Name, Value, Min, Max)                                                     \
  EAPI double                                                                                            \
  edje_edit_part_item_##Name##_get(Evas_Object * obj, const char *part, const char *item_name)           \
  {                                                                                                      \
     Edje_Part *ep;                                                                                      \
     unsigned int i;                                                                                     \
     Edje_Pack_Element *item = NULL;                                                                     \
     if ((!obj) || (!part) || (!item_name))                                                              \
       return 0.0;                                                                                       \
     GET_RP_OR_RETURN(0.0);                                                                              \
     ep = rp->part;                                                                                      \
     for (i = 0; i < ep->items_count; ++i)                                                               \
       {                                                                                                 \
          if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))                            \
            {                                                                                            \
               item = ep->items[i];                                                                      \
               break;                                                                                    \
            }                                                                                            \
       }                                                                                                 \
     if (!item) return EINA_FALSE;                                                                       \
     return TO_DOUBLE(item->Value);                                                                      \
  }                                                                                                      \
  EAPI Eina_Bool                                                                                         \
  edje_edit_part_item_##Name##_set(Evas_Object * obj, const char *part, const char *item_name, double v) \
  {                                                                                                      \
     Edje_Part *ep;                                                                                      \
     unsigned int i;                                                                                     \
     Edje_Pack_Element *item = NULL;                                                                     \
     if ((!obj) || (!part) || (!item_name))                                                              \
       return EINA_FALSE;                                                                                \
     if ((v < Min) || (v > Max))                                                                         \
       return EINA_FALSE;                                                                                \
     GET_RP_OR_RETURN(EINA_FALSE);                                                                       \
     ep = rp->part;                                                                                      \
     if ((rp->part->type != EDJE_PART_TYPE_BOX) &&                                                       \
         (rp->part->type != EDJE_PART_TYPE_TABLE))                                                       \
       return EINA_FALSE;                                                                                \
     for (i = 0; i < ep->items_count; ++i)                                                               \
       {                                                                                                 \
          if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))                            \
            {                                                                                            \
               item = ep->items[i];                                                                      \
               break;                                                                                    \
            }                                                                                            \
       }                                                                                                 \
     if (!item) return EINA_FALSE;                                                                       \
     item->Value = FROM_DOUBLE(v);                                                                       \
     return EINA_TRUE;                                                                                   \
  }

FUNC_PART_ITEM_DOUBLE(align_x, align.x, -1.0, 1.0);
FUNC_PART_ITEM_DOUBLE(align_y, align.y, -1.0, 1.0);
FUNC_PART_ITEM_DOUBLE(weight_x, weight.x, 0.0, 99999.990);
FUNC_PART_ITEM_DOUBLE(weight_y, weight.y, 0.0, 99999.990);

#undef FUNC_PART_ITEM_DOUBLE

#define FUNC_PART_ITEM_INDEX_DOUBLE(Name, Value, Min, Max)                                                  \
  EAPI double                                                                                               \
  edje_edit_part_item_index_##Name##_get(Evas_Object * obj, const char *part, unsigned int index)           \
  {                                                                                                         \
     Edje_Part *ep;                                                                                         \
     if ((!obj) || (!part))                                                                                 \
       return 0.0;                                                                                          \
     GET_RP_OR_RETURN(0.0);                                                                                 \
     ep = rp->part;                                                                                         \
     return TO_DOUBLE(ep->items[index]->Value);                                                             \
  }                                                                                                         \
  EAPI Eina_Bool                                                                                            \
  edje_edit_part_item_index_##Name##_set(Evas_Object * obj, const char *part, unsigned int index, double v) \
  {                                                                                                         \
     Edje_Part *ep;                                                                                         \
     if ((!obj) || (!part))                                                                                 \
       return EINA_FALSE;                                                                                   \
     if ((v < Min) || (v > Max))                                                                            \
       return EINA_FALSE;                                                                                   \
     GET_RP_OR_RETURN(EINA_FALSE);                                                                          \
     ep = rp->part;                                                                                         \
     if ((rp->part->type != EDJE_PART_TYPE_BOX) &&                                                          \
         (rp->part->type != EDJE_PART_TYPE_TABLE))                                                          \
       return EINA_FALSE;                                                                                   \
     ep->items[index]->Value = FROM_DOUBLE(v);                                                              \
     return EINA_TRUE;                                                                                      \
  }

FUNC_PART_ITEM_INDEX_DOUBLE(align_x, align.x, -1.0, 1.0);
FUNC_PART_ITEM_INDEX_DOUBLE(align_y, align.y, -1.0, 1.0);
FUNC_PART_ITEM_INDEX_DOUBLE(weight_x, weight.x, 0.0, 99999.990);
FUNC_PART_ITEM_INDEX_DOUBLE(weight_y, weight.y, 0.0, 99999.990);

#undef FUNC_PART_ITEM_INDEX_DOUBLE

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_position_get(Evas_Object *obj, const char *part, const char *item_name, unsigned short *col, unsigned short *row)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item = NULL;
   if ((!obj) || (!part) || (!item_name))
     return EINA_FALSE;
   GET_RP_OR_RETURN(EINA_FALSE);
   ep = rp->part;
   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          {
             item = ep->items[i];
             break;
          }
     }
   if (!item) return EINA_FALSE;
   *col = item->col;
   *row = item->row;
   return EINA_TRUE;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_position_set(Evas_Object *obj, const char *part, const char *item_name, unsigned short col, unsigned short row)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item = NULL;
   if ((!obj) || (!part) || (!item_name))
     return EINA_FALSE;
   GET_RP_OR_RETURN(EINA_FALSE);
   ep = rp->part;
   if (rp->part->type != EDJE_PART_TYPE_TABLE)
     return EINA_FALSE;
   for (i = 0; i < ep->items_count; ++i)
     {
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name)))
          {
             item = ep->items[i];
             break;
          }
     }
   if (!item) return EINA_FALSE;
   item->col = col;
   item->row = row;
   return EINA_TRUE;
}

#define FUNC_PART_ITEM_USHORT(CLASS, VALUE) \
EAPI unsigned short \
edje_edit_part_item_##CLASS##_##VALUE##_get(Evas_Object *obj, const char *part, const char *item_name) \
{ \
   Edje_Part *ep; \
   unsigned int i; \
   Edje_Pack_Element *item = NULL; \
   GET_RP_OR_RETURN(0); \
   if (!item_name) return 0; \
   ep = rp->part; \
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return 0; \
   for (i = 0; i < ep->items_count; ++i) \
     { \
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name))) \
          { \
             item = ep->items[i]; \
             break; \
          } \
     } \
   if (!item) return 0; \
   return item->VALUE; \
} \
EAPI Eina_Bool \
edje_edit_part_item_##CLASS##_##VALUE##_set(Evas_Object *obj, const char *part, const char *item_name, unsigned short new_val) \
{ \
   Edje_Part *ep; \
   unsigned int i; \
   Edje_Pack_Element *item = NULL; \
   GET_RP_OR_RETURN(EINA_FALSE); \
   if (!item_name) return EINA_FALSE; \
   ep = rp->part; \
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE; \
   for (i = 0; i < ep->items_count; ++i) \
     { \
        if (ep->items[i]->name && (!strcmp(ep->items[i]->name, item_name))) \
          { \
             item = ep->items[i]; \
             break; \
          } \
     } \
   if (!item) return EINA_FALSE; \
   item->VALUE = new_val; \
   return EINA_TRUE; \
}

FUNC_PART_ITEM_USHORT(position, col)
FUNC_PART_ITEM_USHORT(position, row)

#undef FUNC_PART_ITEM_USHORT

#define FUNC_PART_ITEM_INDEX_USHORT(CLASS, VALUE) \
EAPI unsigned short \
edje_edit_part_item_index_##CLASS##_##VALUE##_get(Evas_Object *obj, const char *part, unsigned int index) \
{ \
   Edje_Part *ep; \
   GET_RP_OR_RETURN(0); \
   ep = rp->part; \
   if (ep->type != EDJE_PART_TYPE_TABLE) return 0; \
   return ep->items[index]->VALUE; \
} \
EAPI Eina_Bool \
edje_edit_part_item_index_##CLASS##_##VALUE##_set(Evas_Object *obj, const char *part, unsigned int index, unsigned short new_val) \
{ \
   Edje_Part *ep; \
   GET_RP_OR_RETURN(EINA_FALSE); \
   ep = rp->part; \
   if (ep->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE; \
   ep->items[index]->VALUE = new_val; \
   return EINA_TRUE; \
}

FUNC_PART_ITEM_INDEX_USHORT(position, col)
FUNC_PART_ITEM_INDEX_USHORT(position, row)

#undef FUNC_PART_ITEM_INDEX_USHORT

/* deprecated */
EAPI void
edje_edit_part_item_span_get(Evas_Object *obj, const char *part, const char *item_name, unsigned char *col, unsigned char *row)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item = NULL;
   if ((!obj) || (!part) || (!item_name))
     return;
   GET_RP_OR_RETURN();
   ep = rp->part;
   for (i = 0; i < ep->items_count; ++i)
     {
        if ((ep->items[i]->name) && (!strcmp(ep->items[i]->name, item_name)))
          {
             item = ep->items[i];
             break;
          }
     }
   if (!item) return;
   if (col) *col = item->colspan;
   if (row) *row = item->rowspan;
   return;
}

/* deprecated */
EAPI Eina_Bool
edje_edit_part_item_span_set(Evas_Object *obj, const char *part, const char *item_name, unsigned char col, unsigned char row)
{
   Edje_Part *ep;
   unsigned int i;
   Edje_Pack_Element *item = NULL;
   if ((!obj) || (!part) || (!item_name))
     return EINA_FALSE;
   GET_RP_OR_RETURN(EINA_FALSE);
   ep = rp->part;
   if ((rp->part->type != EDJE_PART_TYPE_BOX) &&
       (rp->part->type != EDJE_PART_TYPE_TABLE))
     return EINA_FALSE;
   for (i = 0; i < ep->items_count; i++)
     {
        if ((ep->items[i]->name) && (!strcmp(ep->items[i]->name, item_name)))
          {
             item = ep->items[i];
             break;
          }
     }
   if (!item) return EINA_FALSE;
   item->colspan = col;
   item->rowspan = row;
   return EINA_TRUE;
}

#define FUNC_PART_ITEM_USHORT(CLASS, VALUE, MEMBER) \
EAPI unsigned short \
edje_edit_part_item_##CLASS##_##VALUE##_get(Evas_Object *obj, const char *part, const char *item_name) \
{ \
   Edje_Part *ep; \
   unsigned int i; \
   Edje_Pack_Element *item = NULL; \
   GET_RP_OR_RETURN(0); \
   if (!item_name) return 0; \
   ep = rp->part; \
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE; \
   for (i = 0; i < ep->items_count; ++i) \
     { \
        if ((ep->items[i]->name) && (!strcmp(ep->items[i]->name, item_name))) \
          { \
             item = ep->items[i]; \
             break; \
          } \
     } \
   if (!item) return 0; \
   return  item->MEMBER; \
} \
EAPI Eina_Bool \
edje_edit_part_item_##CLASS##_##VALUE##_set(Evas_Object *obj, const char *part, const char *item_name, unsigned short new_val) \
{ \
   Edje_Part *ep; \
   unsigned int i; \
   Edje_Pack_Element *item = NULL; \
   GET_RP_OR_RETURN(EINA_FALSE); \
   if (!item_name) return EINA_FALSE; \
   ep = rp->part; \
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE; \
   for (i = 0; i < ep->items_count; i++) \
     { \
        if ((ep->items[i]->name) && (!strcmp(ep->items[i]->name, item_name))) \
          { \
             item = ep->items[i]; \
             break; \
          } \
     } \
   if (!item) return EINA_FALSE; \
   item->MEMBER = new_val; \
   return EINA_TRUE; \
}

FUNC_PART_ITEM_USHORT(span, col, colspan)
FUNC_PART_ITEM_USHORT(span, row, rowspan)

#undef FUNC_PART_ITEM_USHORT

#define FUNC_PART_ITEM_INDEX_USHORT(CLASS, VALUE, MEMBER) \
EAPI unsigned short \
edje_edit_part_item_index_##CLASS##_##VALUE##_get(Evas_Object *obj, const char *part, unsigned int index) \
{ \
   Edje_Part *ep; \
   GET_RP_OR_RETURN(0); \
   ep = rp->part; \
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE; \
   return  ep->items[index]->MEMBER; \
} \
EAPI Eina_Bool \
edje_edit_part_item_index_##CLASS##_##VALUE##_set(Evas_Object *obj, const char *part, unsigned int index, unsigned short new_val) \
{ \
   Edje_Part *ep; \
   GET_RP_OR_RETURN(EINA_FALSE); \
   ep = rp->part; \
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE; \
   ep->items[index]->MEMBER = new_val; \
   return EINA_TRUE; \
}

FUNC_PART_ITEM_INDEX_USHORT(span, col, colspan)
FUNC_PART_ITEM_INDEX_USHORT(span, row, rowspan)

#undef FUNC_PART_ITEM_INDEX_USHORT

/*********************/
/*  PART STATES API  */
/*********************/
EAPI Eina_List *
edje_edit_part_states_list_get(Evas_Object *obj, const char *part)
{
   char state_name[PATH_MAX];
   Eina_List *states = NULL;
   unsigned int i;

   GET_RP_OR_RETURN(NULL);

   //Is there a better place to put this? maybe edje_edit_init() ?
   setlocale(LC_NUMERIC, "C");

   states = NULL;

   //append default state
   snprintf(state_name, PATH_MAX,
            "%s %.2f",
            rp->part->default_desc->state.name,
            rp->part->default_desc->state.value);
   states = eina_list_append(states, eina_stringshare_add(state_name));
   //printf("NEW STATE def: %s\n", state->state.name);

   //append other states
   for (i = 0; i < rp->part->other.desc_count; ++i)
     {
        snprintf(state_name, sizeof(state_name),
                 "%s %.2f",
                 rp->part->other.desc[i]->state.name,
                 rp->part->other.desc[i]->state.value);
        states = eina_list_append(states, eina_stringshare_add(state_name));
        //printf("NEW STATE: %s\n", state_name);
     }
   return states;
}

EAPI Eina_Bool
edje_edit_state_name_set(Evas_Object *obj, const char *part, const char *state, double value, const char *new_name, double new_value)
{
   int part_id;
   int i;

   GET_PD_OR_RETURN(EINA_FALSE);
   //printf("Set name of state: %s in part: %s [new name: %s]\n",
   //     part, state, new_name);

   if (!new_name) return EINA_FALSE;

   /* update programs */
   /* update the 'state' field in all programs. update only if program has
      a single target */
   part_id = _edje_part_id_find(ed, part);
   for (i = 0; i < ed->collection->patterns.table_programs_size; i++)
     {
        Edje_Program *epr = ed->collection->patterns.table_programs[i];

        if (eina_list_count(epr->targets) == 1)
          {
             Edje_Program_Target *t = eina_list_data_get(epr->targets);

             if (t->id == part_id &&
                 !strcmp(epr->state, pd->state.name) &&
                 EQ(pd->state.value, epr->value))
               {
                  _edje_if_string_replace(ed, &epr->state, new_name);
                  epr->value = value;
               }
          }
     }

   /* set name */
   _edje_if_string_replace(ed, &pd->state.name, new_name);
   /* set value */
   pd->state.value = new_value;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_del(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Part_Description_Common *pd;
   unsigned int i;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_RP_OR_RETURN(EINA_FALSE);

   if (!edje_edit_state_exist(obj, part, state, value))
     return EINA_FALSE;

   pd = _edje_part_description_find_byname(eed, part, state, value);
   if (!pd) return EINA_FALSE;

   /* Don't allow to delete default state, for now at least; */
   if (pd == rp->part->default_desc)
     return EINA_FALSE;

   /* And if we are deleting the current state, go back to default first */
   if (pd == rp->chosen_description)
     _edje_part_description_apply(ed, rp, "default", 0.0, NULL, 0.0);

   ce = eina_hash_find(ed->file->collection, ed->group);

   for (i = 0; i < rp->part->other.desc_count; ++i)
     if (pd == rp->part->other.desc[i])
       {
          memmove(rp->part->other.desc + i,
                  rp->part->other.desc + i + 1,
                  sizeof (Edje_Part_Description_Common *) * (rp->part->other.desc_count - i - 1));
          rp->part->other.desc_count--;
          break;
       }

   _edje_collection_free_part_description_free(rp->part->type, pd, ce, 0);
   return EINA_TRUE;
}

static Edje_Part_Description_Common *
_edje_edit_state_alloc(int type, Edje *ed)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Part_Description_Common *pd = NULL;

   ce = eina_hash_find(ed->file->collection, ed->group);

   switch (type)
     {
      case EDJE_PART_TYPE_RECTANGLE:
        pd = eina_mempool_malloc(ce->mp->mp.RECTANGLE, sizeof (Edje_Part_Description_Common));
        if (!pd) return NULL;
        ce->count.RECTANGLE++;
        break;

      case EDJE_PART_TYPE_SPACER:
        pd = eina_mempool_malloc(ce->mp->mp.SPACER, sizeof (Edje_Part_Description_Common));
        if (!pd) return NULL;
        ce->count.SPACER++;
        break;

      case EDJE_PART_TYPE_SWALLOW:
        pd = eina_mempool_malloc(ce->mp->mp.SWALLOW, sizeof (Edje_Part_Description_Common));
        if (!pd) return NULL;
        ce->count.SWALLOW++;
        break;

      case EDJE_PART_TYPE_GROUP:
        pd = eina_mempool_malloc(ce->mp->mp.GROUP, sizeof (Edje_Part_Description_Common));
        if (!pd) return NULL;
        ce->count.GROUP++;
        break;

#define EDIT_ALLOC_POOL(Short, Type, Name)                            \
case EDJE_PART_TYPE_##Short:                                          \
{                                                                     \
   Edje_Part_Description_##Type *Name = NULL;                         \
                                                                      \
   Name = eina_mempool_malloc(ce->mp->mp.Short,                       \
                              sizeof (Edje_Part_Description_##Type)); \
   if (!Name) return NULL;                                            \
   memset(Name, 0, sizeof(Edje_Part_Description_##Type));             \
   pd = &Name->common;                                                \
   ce->count.Short++;                                                 \
   break;                                                             \
}

        EDIT_ALLOC_POOL(IMAGE, Image, image);
        EDIT_ALLOC_POOL(PROXY, Proxy, proxy);
        EDIT_ALLOC_POOL(TEXT, Text, text);
        EDIT_ALLOC_POOL(TEXTBLOCK, Text, text);
        EDIT_ALLOC_POOL(BOX, Box, box);
        EDIT_ALLOC_POOL(TABLE, Table, table);
        EDIT_ALLOC_POOL(EXTERNAL, External, external_params);
        EDIT_ALLOC_POOL(VECTOR, Vector, vector);
     }

   return pd;
}

EAPI Eina_Bool
edje_edit_state_add(Evas_Object *obj, const char *part, const char *name, double value)
{
   Edje_Part_Description_Common *pd;

   GET_RP_OR_RETURN(EINA_FALSE);

   if (edje_edit_state_exist(obj, part, name, value))
     return EINA_FALSE;

   //printf("ADD STATE: %s TO PART: %s\n", name , part);
   pd = _edje_edit_state_alloc(rp->part->type, ed);
   if (!pd) return EINA_FALSE;

   if (!rp->part->default_desc)
     {
        rp->part->default_desc = pd;
     }
   else
     {
        Edje_Part_Description_Common **tmp;

        tmp = realloc(rp->part->other.desc,
                      sizeof (Edje_Part_Description_Common *) * (rp->part->other.desc_count + 1));
        if (!tmp)
          {
             free(pd);
             return EINA_FALSE;
          }
        rp->part->other.desc = tmp;
        rp->part->other.desc[rp->part->other.desc_count++] = pd;
     }

   memset(pd, 0, sizeof (*pd));

   pd->state.name = eina_stringshare_add(name);
   pd->state.value = value;
   pd->visible = 1;
   pd->align.x = 0.5;
   pd->align.y = 0.5;
   pd->min.w = 0;
   pd->min.h = 0;
   pd->fixed.w = 0;
   pd->fixed.h = 0;
   pd->max.w = -1;
   pd->max.h = -1;
   pd->rel1.relative_x = 0.0;
   pd->rel1.relative_y = 0.0;
   pd->rel1.offset_x = 0;
   pd->rel1.offset_y = 0;
   pd->rel1.id_x = -1;
   pd->rel1.id_y = -1;
   pd->rel2.relative_x = 1.0;
   pd->rel2.relative_y = 1.0;
   pd->rel2.offset_x = -1;
   pd->rel2.offset_y = -1;
   pd->rel2.id_x = -1;
   pd->rel2.id_y = -1;
   pd->clip_to_id = -1;
   pd->color_class = NULL;
   pd->color.r = 255;
   pd->color.g = 255;
   pd->color.b = 255;
   pd->color.a = 255;
   pd->color2.r = 0;
   pd->color2.g = 0;
   pd->color2.b = 0;
   pd->color2.a = 255;
   pd->map.id_persp = -1;
   pd->map.id_light = -1;
   pd->map.rot.id_center = -1;
   pd->map.rot.x = FROM_DOUBLE(0.0);
   pd->map.rot.y = FROM_DOUBLE(0.0);
   pd->map.rot.z = FROM_DOUBLE(0.0);
   pd->map.colors = NULL;
   pd->map.on = EINA_FALSE;
   pd->map.smooth = EINA_TRUE;
   pd->map.alpha = EINA_TRUE;
   pd->map.backcull = EINA_FALSE;
   pd->map.persp_on = EINA_FALSE;
   pd->persp.zplane = EINA_FALSE;
   pd->map.zoom.x = FROM_DOUBLE(1.0);
   pd->map.zoom.y = FROM_DOUBLE(1.0);
   pd->align_3d.x = FROM_DOUBLE(0.5);
   pd->align_3d.y = FROM_DOUBLE(0.5);
   pd->align_3d.z = FROM_DOUBLE(0.5);
   pd->persp.focal = 1000;

   if (rp->part->type == EDJE_PART_TYPE_TEXT
       || rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
     {
        Edje_Part_Description_Text *text;

        text = (Edje_Part_Description_Text *)pd;

        memset(&text->text, 0, sizeof (text->text));

        text->text.color3.r = 0;
        text->text.color3.g = 0;
        text->text.color3.b = 0;
        text->text.color3.a = 128;
        text->text.align.x = 0.5;
        text->text.align.y = 0.5;
        text->text.id_source = -1;
        text->text.id_text_source = -1;
     }
   else if (rp->part->type == EDJE_PART_TYPE_IMAGE)
     {
        Edje_Part_Description_Image *img;

        img = (Edje_Part_Description_Image *)pd;

        memset(&img->image, 0, sizeof (img->image));

        img->image.id = -1;
        img->image.fill.smooth = 1;
        img->image.fill.pos_rel_x = 0.0;
        img->image.fill.pos_abs_x = 0;
        img->image.fill.rel_x = 1.0;
        img->image.fill.abs_x = 0;
        img->image.fill.pos_rel_y = 0.0;
        img->image.fill.pos_abs_y = 0;
        img->image.fill.rel_y = 1.0;
        img->image.fill.abs_y = 0;
        img->image.fill.type = EDJE_FILL_TYPE_SCALE;
     }
   else if (rp->part->type == EDJE_PART_TYPE_PROXY)
     {
        Edje_Part_Description_Proxy *pro;

        pro = (Edje_Part_Description_Proxy *)pd;

        memset(&pro->proxy, 0, sizeof (pro->proxy));

        pro->proxy.id = -1;
        pro->proxy.source_visible = EINA_TRUE;
        pro->proxy.source_clip = EINA_TRUE;
        pro->proxy.fill.smooth = 1;
        pro->proxy.fill.pos_rel_x = 0.0;
        pro->proxy.fill.pos_abs_x = 0;
        pro->proxy.fill.rel_x = 1.0;
        pro->proxy.fill.abs_x = 0;
        pro->proxy.fill.pos_rel_y = 0.0;
        pro->proxy.fill.pos_abs_y = 0;
        pro->proxy.fill.rel_y = 1.0;
        pro->proxy.fill.abs_y = 0;
        pro->proxy.fill.type = EDJE_FILL_TYPE_SCALE;
     }
   else if (rp->part->type == EDJE_PART_TYPE_EXTERNAL)
     {
        Edje_Part_Description_External *external;
        Edje_External_Param_Info *pi;

        external = (Edje_Part_Description_External *)pd;

        external->external_params = NULL;

        if (rp->part->source)
          {
             pi = (Edje_External_Param_Info *)edje_external_param_info_get(rp->part->source);
             while (pi && pi->name)
               {
                  Edje_External_Param *p;
                  p = _alloc(sizeof(Edje_External_Param));
                  /* error checking.. meh */
                  p->name = eina_stringshare_add(pi->name);
                  p->type = pi->type;
                  switch (p->type)
                    {
                     case EDJE_EXTERNAL_PARAM_TYPE_INT:
                     case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
                       if (pi->info.i.def != EDJE_EXTERNAL_INT_UNSET)
                         p->i = pi->info.i.def;
                       break;

                     case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
                       if (NEQ(pi->info.d.def, EDJE_EXTERNAL_DOUBLE_UNSET))
                         p->d = pi->info.d.def;
                       break;

                     case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
                       if (pi->info.c.def)
                         p->s = eina_stringshare_add(pi->info.c.def);
                       break;

                     case EDJE_EXTERNAL_PARAM_TYPE_STRING:
                       if (pi->info.s.def)
                         p->s = eina_stringshare_add(pi->info.s.def);
                       break;

                     default:
                       ERR("unknown external parameter type '%d'", p->type);
                    }
                  external->external_params = eina_list_append(external->external_params, p);
                  pi++;
               }
             if (external->external_params && rp->typedata.swallow)
               rp->param1.external_params = _edje_external_params_parse(rp->typedata.swallow->swallowed_object, external->external_params);
          }
     }
   else if (rp->part->type == EDJE_PART_TYPE_BOX)
     {
        Edje_Part_Description_Box *box;

        box = (Edje_Part_Description_Box *)pd;
        memset(&box->box, 0, sizeof (box->box));
     }
   else if (rp->part->type == EDJE_PART_TYPE_TABLE)
     {
        Edje_Part_Description_Table *table;

        table = (Edje_Part_Description_Table *)pd;
        memset(&table->table, 0, sizeof (table->table));
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_exist(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);
   return EINA_TRUE;
}

static Eina_Bool
_edje_edit_part_state_copy(Evas_Object *obj, const char *part, const char *part_to, const char *from, double val_from, const char *to, double val_to)
{
   Edje_Part_Description_Common *pdfrom, *pdto;
   Edje_External_Param *p;
   Edje_Real_Part *rpto;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_RP_OR_RETURN(EINA_FALSE);

   pdfrom = _edje_part_description_find_byname(eed, part, from, val_from);
   if (!pdfrom)
     return EINA_FALSE;

   rpto = _edje_real_part_get(ed, part_to);
   if (!rpto)
     return EINA_FALSE;
   pdto = _edje_part_description_find_byname(eed, part_to, to, val_to);
   if (!pdto)
     {
        Edje_Part_Description_Common **tmp;

        pdto = _edje_edit_state_alloc(rpto->part->type, ed);
        if (!pdto) return EINA_FALSE;
        /* No need to check for default desc, at this point it must exist */

        tmp = realloc(rpto->part->other.desc,
                      sizeof (Edje_Part_Description_Common *) * (rpto->part->other.desc_count + 1));
        if (!tmp)
          {
             free(pdto);
             return EINA_FALSE;
          }
        rpto->part->other.desc = tmp;
        rpto->part->other.desc[rpto->part->other.desc_count++] = pdto;
     }

#define PD_STRING_COPY(To, From, _x) \
  if (From->_x)                      \
    To->_x = (char *)eina_stringshare_add(From->_x);

   /* Copy all value */
   *pdto = *pdfrom;
   /* Keeping the pdto state name and value */
   pdto->state.name = eina_stringshare_add(to);
   pdto->state.value = val_to;
   /* Update pointer. */
   PD_STRING_COPY(pdto, pdfrom, color_class);

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *pro_to = (Edje_Part_Description_Proxy *)pdto;
         Edje_Part_Description_Proxy *pro_from = (Edje_Part_Description_Proxy *)pdfrom;

         pro_to->proxy = pro_from->proxy;

         break;
      }

      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *img_to = (Edje_Part_Description_Image *)pdto;
         Edje_Part_Description_Image *img_from = (Edje_Part_Description_Image *)pdfrom;
         unsigned int i;

         img_to->image = img_from->image;

         img_to->image.tweens_count = img_from->image.tweens_count;
         img_to->image.tweens = calloc(img_to->image.tweens_count,
                                       sizeof (Edje_Part_Image_Id *));
         if (!img_to->image.tweens)
           break;

         for (i = 0; i < img_to->image.tweens_count; ++i)
           {
              Edje_Part_Image_Id *new_i;
              new_i = _alloc(sizeof(Edje_Part_Image_Id));
              if (!new_i) continue;

              *new_i = *img_from->image.tweens[i];

              img_to->image.tweens[i] = new_i;
           }
         break;
      }

      case EDJE_PART_TYPE_TEXT:
      case EDJE_PART_TYPE_TEXTBLOCK:
      {
         Edje_Part_Description_Text *text_to = (Edje_Part_Description_Text *)pdto;
         Edje_Part_Description_Text *text_from = (Edje_Part_Description_Text *)pdfrom;

         text_to->text = text_from->text;

         /* Update pointers. */
         PD_STRING_COPY(text_to, text_from, text.text.str);
         PD_STRING_COPY(text_to, text_from, text.text_class);
         PD_STRING_COPY(text_to, text_from, text.style.str);
         PD_STRING_COPY(text_to, text_from, text.font.str);
         PD_STRING_COPY(text_to, text_from, text.repch.str);
         break;
      }

      case EDJE_PART_TYPE_BOX:
      {
         Edje_Part_Description_Box *box_to = (Edje_Part_Description_Box *)pdto;
         Edje_Part_Description_Box *box_from = (Edje_Part_Description_Box *)pdfrom;

         box_to->box = box_from->box;

         PD_STRING_COPY(box_to, box_from, box.layout);
         PD_STRING_COPY(box_to, box_from, box.alt_layout);
         break;
      }

      case EDJE_PART_TYPE_TABLE:
      {
         Edje_Part_Description_Table *table_to = (Edje_Part_Description_Table *)pdto;
         Edje_Part_Description_Table *table_from = (Edje_Part_Description_Table *)pdfrom;

         table_to->table = table_from->table;
         break;
      }

      case EDJE_PART_TYPE_EXTERNAL:
      {
         Edje_Part_Description_External *ext_to = (Edje_Part_Description_External *)pdto;
         Edje_Part_Description_External *ext_from = (Edje_Part_Description_External *)pdfrom;
         Eina_List *l;

         /* XXX: optimize this, most likely we don't need to remove and add */
         EINA_LIST_FREE(ext_to->external_params, p)
           {
              _edje_if_string_free(ed, &p->name);
              if (p->s)
                _edje_if_string_free(ed, &p->s);
              free(p);
           }
         EINA_LIST_FOREACH(ext_from->external_params, l, p)
           {
              Edje_External_Param *new_p;
              new_p = _alloc(sizeof(Edje_External_Param));
              new_p->name = eina_stringshare_add(p->name);
              new_p->type = p->type;
              switch (p->type)
                {
                 case EDJE_EXTERNAL_PARAM_TYPE_INT:
                   new_p->i = p->i;
                   break;

                 case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
                   new_p->d = p->d;
                   break;

                 case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
                 case EDJE_EXTERNAL_PARAM_TYPE_STRING:
                   new_p->s = eina_stringshare_add(p->s);
                   break;

                 default:
                   break;
                }
              ext_to->external_params = eina_list_append(ext_to->external_params, new_p);
           }
         break;
      }
     }

#undef PD_STRING_COPY

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_copy(Evas_Object *obj, const char *part, const char *from, double val_from, const char *to, double val_to)
{
   return _edje_edit_part_state_copy(obj, part, part, from, val_from, to, val_to);
}

#define FUNC_STATE_RELATIVE_DOUBLE(Sub, Value)                                                                                   \
  EAPI double                                                                                                                    \
  edje_edit_state_##Sub##_relative_##Value##_get(Evas_Object * obj, const char *part, const char *state, double value)           \
  {                                                                                                                              \
     GET_PD_OR_RETURN(0);                                                                                                        \
     return TO_DOUBLE(pd->Sub.relative_##Value);                                                                                 \
  }                                                                                                                              \
  EAPI Eina_Bool                                                                                                                 \
  edje_edit_state_##Sub##_relative_##Value##_set(Evas_Object * obj, const char *part, const char *state, double value, double v) \
  {                                                                                                                              \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                               \
     pd->Sub.relative_##Value = FROM_DOUBLE(v);                                                                                  \
     edje_object_calc_force(obj);                                                                                                \
     return EINA_TRUE;                                                                                                           \
  }

FUNC_STATE_RELATIVE_DOUBLE(rel1, x);
FUNC_STATE_RELATIVE_DOUBLE(rel1, y);
FUNC_STATE_RELATIVE_DOUBLE(rel2, x);
FUNC_STATE_RELATIVE_DOUBLE(rel2, y);

#define FUNC_STATE_OFFSET_INT(Sub, Value)                                                                                   \
  EAPI int                                                                                                                  \
  edje_edit_state_##Sub##_offset_##Value##_get(Evas_Object * obj, const char *part, const char *state, double value)        \
  {                                                                                                                         \
     GET_PD_OR_RETURN(0);                                                                                                   \
     return pd->Sub.offset_##Value;                                                                                         \
  }                                                                                                                         \
  EAPI Eina_Bool                                                                                                            \
  edje_edit_state_##Sub##_offset_##Value##_set(Evas_Object * obj, const char *part, const char *state, double value, int v) \
  {                                                                                                                         \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                          \
     pd->Sub.offset_##Value = v;                                                                                            \
     edje_object_calc_force(obj);                                                                                           \
     return EINA_TRUE;                                                                                                      \
  }

FUNC_STATE_OFFSET_INT(rel1, x);
FUNC_STATE_OFFSET_INT(rel1, y);
FUNC_STATE_OFFSET_INT(rel2, x);
FUNC_STATE_OFFSET_INT(rel2, y);

#define FUNC_STATE_REL(Sub, Value)                                                                                               \
  EAPI const char *                                                                                                              \
  edje_edit_state_##Sub##_to_##Value##_get(Evas_Object * obj, const char *part, const char *state, double value)                 \
  {                                                                                                                              \
     Edje_Real_Part *rel;                                                                                                        \
     GET_PD_OR_RETURN(NULL);                                                                                                     \
     if (pd->Sub.id_##Value == -1) return NULL;                                                                                  \
     rel = ed->table_parts[pd->Sub.id_##Value % ed->table_parts_size];                                                           \
     if (rel->part->name) return eina_stringshare_add(rel->part->name);                                                          \
     return NULL;                                                                                                                \
  }                                                                                                                              \
  EAPI Eina_Bool                                                                                                                 \
  edje_edit_state_##Sub##_to_##Value##_set(Evas_Object * obj, const char *part, const char *state, double value, const char *to) \
  {                                                                                                                              \
     Edje_Real_Part *relp;                                                                                                       \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                               \
     if ((to) && (strcmp(to, "")))                                                                                               \
       {                                                                                                                         \
          relp = _edje_real_part_get(ed, to);                                                                                    \
          if (!relp) return EINA_FALSE;                                                                                          \
          pd->Sub.id_##Value = relp->part->id;                                                                                   \
          return EINA_TRUE;                                                                                                      \
       }                                                                                                                         \
     else                                                                                                                        \
       {                                                                                                                         \
          pd->Sub.id_##Value = -1;                                                                                               \
          return EINA_TRUE;                                                                                                      \
       }                                                                                                                         \
  }
//note after this call edje_edit_part_selected_state_set() to update !! need to fix this
//_edje_part_description_apply(ed, rp, pd->state.name, pd->state.value, "state", 0.1); //Why segfault??
// edje_object_calc_force(obj);//don't work for redraw

FUNC_STATE_REL(rel1, x);
FUNC_STATE_REL(rel1, y);
FUNC_STATE_REL(rel2, x);
FUNC_STATE_REL(rel2, y);

//colors
#define FUNC_COLOR(Code)                                                                                                             \
  EAPI void                                                                                                                          \
  edje_edit_state_##Code##_get(Evas_Object * obj, const char *part, const char *state, double value, int *r, int *g, int *b, int *a) \
  {                                                                                                                                  \
     GET_PD_OR_RETURN();                                                                                                             \
                                                                                                                                     \
     if (r) *r = pd->Code.r;                                                                                                         \
     if (g) *g = pd->Code.g;                                                                                                         \
     if (b) *b = pd->Code.b;                                                                                                         \
     if (a) *a = pd->Code.a;                                                                                                         \
  }                                                                                                                                  \
  EAPI Eina_Bool                                                                                                                     \
  edje_edit_state_##Code##_set(Evas_Object * obj, const char *part, const char *state, double value, int r, int g, int b, int a)     \
  {                                                                                                                                  \
     if ((!obj) || (!part) || (!state))                                                                                              \
       return EINA_FALSE;                                                                                                            \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                                   \
                                                                                                                                     \
     if (r > -1 && r < 256) pd->Code.r = r;                                                                                          \
     else return EINA_FALSE;                                                                                                         \
     if (g > -1 && g < 256) pd->Code.g = g;                                                                                          \
     else return EINA_FALSE;                                                                                                         \
     if (b > -1 && b < 256) pd->Code.b = b;                                                                                          \
     else return EINA_FALSE;                                                                                                         \
     if (a > -1 && a < 256) pd->Code.a = a;                                                                                          \
     else return EINA_FALSE;                                                                                                         \
                                                                                                                                     \
     edje_object_calc_force(obj);                                                                                                    \
     return EINA_TRUE;                                                                                                               \
  }

FUNC_COLOR(color);
FUNC_COLOR(color2);

EAPI void
edje_edit_state_color3_get(Evas_Object *obj, const char *part, const char *state, double value, int *r, int *g, int *b, int *a)
{
   Edje_Part_Description_Text *txt;

   GET_PD_OR_RETURN();

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     {
        if (r) *r = 0;
        if (g) *g = 0;
        if (b) *b = 0;
        if (a) *a = 0;
        return;
     }

   txt = (Edje_Part_Description_Text *)pd;

   if (r) *r = txt->text.color3.r;
   if (g) *g = txt->text.color3.g;
   if (b) *b = txt->text.color3.b;
   if (a) *a = txt->text.color3.a;
}

EAPI Eina_Bool
edje_edit_state_color3_set(Evas_Object *obj, const char *part, const char *state, double value, int r, int g, int b, int a)
{
   Edje_Part_Description_Text *txt;

   if ((!obj) || (!part) || (!state))
     return EINA_FALSE;
   GET_PD_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   txt = (Edje_Part_Description_Text *)pd;

   if (r > -1 && r < 256) txt->text.color3.r = r;
   else return EINA_FALSE;
   if (g > -1 && g < 256) txt->text.color3.g = g;
   else return EINA_FALSE;
   if (b > -1 && b < 256) txt->text.color3.b = b;
   else return EINA_FALSE;
   if (a > -1 && a < 256) txt->text.color3.a = a;
   else return EINA_FALSE;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

#define FUNC_STATE_DOUBLE(Class, Value)                                                                                   \
  EAPI double                                                                                                             \
  edje_edit_state_##Class##_##Value##_get(Evas_Object * obj, const char *part, const char *state, double value)           \
  {                                                                                                                       \
     GET_PD_OR_RETURN(0);                                                                                                 \
     return TO_DOUBLE(pd->Class.Value);                                                                                   \
  }                                                                                                                       \
  EAPI Eina_Bool                                                                                                          \
  edje_edit_state_##Class##_##Value##_set(Evas_Object * obj, const char *part, const char *state, double value, double v) \
  {                                                                                                                       \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                        \
     pd->Class.Value = FROM_DOUBLE(v);                                                                                    \
     edje_object_calc_force(obj);                                                                                         \
     return EINA_TRUE;                                                                                                    \
  }

#define FUNC_STATE_INT(Class, Value, Min)                                                                              \
  EAPI int                                                                                                             \
  edje_edit_state_##Class##_##Value##_get(Evas_Object * obj, const char *part, const char *state, double value)        \
  {                                                                                                                    \
     GET_PD_OR_RETURN(0);                                                                                              \
     return pd->Class.Value;                                                                                           \
  }                                                                                                                    \
  EAPI Eina_Bool                                                                                                       \
  edje_edit_state_##Class##_##Value##_set(Evas_Object * obj, const char *part, const char *state, double value, int v) \
  {                                                                                                                    \
     if ((!obj) || (!part) || (!state))                                                                                \
       return EINA_FALSE;                                                                                              \
     if (v < Min) return EINA_FALSE;                                                                                   \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                     \
     pd->Class.Value = v;                                                                                              \
     edje_object_calc_force(obj);                                                                                      \
     return EINA_TRUE;                                                                                                 \
  }

#define FUNC_STATE_BOOL(Class, Value)                                                                                        \
  EAPI Eina_Bool                                                                                                             \
  edje_edit_state_##Class##_##Value##_get(Evas_Object * obj, const char *part, const char *state, double value)              \
  {                                                                                                                          \
     GET_PD_OR_RETURN(0);                                                                                                    \
     return pd->Class.Value;                                                                                                 \
  }                                                                                                                          \
  EAPI Eina_Bool                                                                                                             \
  edje_edit_state_##Class##_##Value##_set(Evas_Object * obj, const char *part, const char *state, double value, Eina_Bool v) \
  {                                                                                                                          \
     if ((!obj) || (!part) || (!state))                                                                                      \
       return EINA_FALSE;                                                                                                    \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                           \
     pd->Class.Value = v;                                                                                                    \
     edje_object_calc_force(obj);                                                                                            \
     return EINA_TRUE;                                                                                                       \
  }

FUNC_STATE_DOUBLE(align, x);
FUNC_STATE_DOUBLE(align, y);
FUNC_STATE_INT(min, w, 0);
FUNC_STATE_INT(min, h, 0);
FUNC_STATE_INT(max, w, -1);
FUNC_STATE_INT(max, h, -1);
FUNC_STATE_BOOL(fixed, w);
FUNC_STATE_BOOL(fixed, h);
FUNC_STATE_DOUBLE(aspect, min);
FUNC_STATE_DOUBLE(aspect, max);
FUNC_STATE_DOUBLE(minmul, w);
FUNC_STATE_DOUBLE(minmul, h);

EAPI Eina_Bool
edje_edit_state_size_class_set(Evas_Object *obj, const char *part, const char *state, double value, const char *size_class)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   _edje_if_string_replace(ed, &pd->size_class, size_class);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_fill_smooth_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *img;
         img = (Edje_Part_Description_Image *)pd;
         return img->image.fill.smooth;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *pro;
         pro = (Edje_Part_Description_Proxy *)pd;
         return pro->proxy.fill.smooth;
      }
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_state_fill_smooth_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool smooth)
{
   GET_PD_OR_RETURN(EINA_FALSE)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *img;
         img = (Edje_Part_Description_Image *)pd;
         img->image.fill.smooth = smooth;
         return EINA_TRUE;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *pro;
         pro = (Edje_Part_Description_Proxy *)pd;
         pro->proxy.fill.smooth = smooth;
         return EINA_TRUE;
      }
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_state_fill_type_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char fill_type)
{
   GET_PD_OR_RETURN(EINA_FALSE)
   if (fill_type >= EDJE_FILL_TYPE_LAST)
     return EINA_FALSE;

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *img;
         img = (Edje_Part_Description_Image *)pd;
         img->image.fill.type = fill_type;
         return EINA_TRUE;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *pro;
         pro = (Edje_Part_Description_Proxy *)pd;
         pro->proxy.fill.type = fill_type;
         return EINA_TRUE;
      }
     }

   return EINA_FALSE;
}

EAPI unsigned char
edje_edit_state_fill_type_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EDJE_FILL_TYPE_LAST)

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Part_Description_Image *img;
         img = (Edje_Part_Description_Image *)pd;
         return img->image.fill.type;
      }

      case EDJE_PART_TYPE_PROXY:
      {
         Edje_Part_Description_Proxy *pro;
         pro = (Edje_Part_Description_Proxy *)pd;
         return pro->proxy.fill.type;
      }
     }

   return EDJE_FILL_TYPE_LAST;
}

#define FUNC_STATE_DOUBLE_FILL(Class, Type, Value)                                                                                     \
  EAPI double                                                                                                                          \
  edje_edit_state_fill_##Type##_relative_##Value##_get(Evas_Object * obj, const char *part, const char *state, double value)           \
  {                                                                                                                                    \
     GET_PD_OR_RETURN(0);                                                                                                              \
                                                                                                                                       \
     switch (rp->part->type)                                                                                                           \
       {                                                                                                                               \
        case EDJE_PART_TYPE_IMAGE:                                                                                                     \
        {                                                                                                                              \
           Edje_Part_Description_Image *img;                                                                                           \
                                                                                                                                       \
           img = (Edje_Part_Description_Image *)pd;                                                                                    \
                                                                                                                                       \
           return TO_DOUBLE(img->image.fill.Class##rel_##Value);                                                                       \
        }                                                                                                                              \
        case EDJE_PART_TYPE_PROXY:                                                                                                     \
        {                                                                                                                              \
           Edje_Part_Description_Proxy *pro;                                                                                           \
                                                                                                                                       \
           pro = (Edje_Part_Description_Proxy *)pd;                                                                                    \
                                                                                                                                       \
           return TO_DOUBLE(pro->proxy.fill.Class##rel_##Value);                                                                       \
        }                                                                                                                              \
       }                                                                                                                               \
                                                                                                                                       \
     return 0;                                                                                                                         \
  }                                                                                                                                    \
  EAPI Eina_Bool                                                                                                                       \
  edje_edit_state_fill_##Type##_relative_##Value##_set(Evas_Object * obj, const char *part, const char *state, double value, double v) \
  {                                                                                                                                    \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                                     \
                                                                                                                                       \
     switch (rp->part->type)                                                                                                           \
       {                                                                                                                               \
        case EDJE_PART_TYPE_IMAGE:                                                                                                     \
        {                                                                                                                              \
           Edje_Part_Description_Image *img;                                                                                           \
                                                                                                                                       \
           img = (Edje_Part_Description_Image *)pd;                                                                                    \
                                                                                                                                       \
           img->image.fill.Class##rel_##Value = FROM_DOUBLE(v);                                                                        \
                                                                                                                                       \
           break;                                                                                                                      \
        }                                                                                                                              \
        case EDJE_PART_TYPE_PROXY:                                                                                                     \
        {                                                                                                                              \
           Edje_Part_Description_Proxy *pro;                                                                                           \
                                                                                                                                       \
           pro = (Edje_Part_Description_Proxy *)pd;                                                                                    \
                                                                                                                                       \
           pro->proxy.fill.Class##rel_##Value = FROM_DOUBLE(v);                                                                        \
                                                                                                                                       \
           break;                                                                                                                      \
        }                                                                                                                              \
        default:                                                                                                                       \
          return EINA_FALSE;                                                                                                           \
       }                                                                                                                               \
                                                                                                                                       \
     edje_object_calc_force(obj);                                                                                                      \
     return EINA_TRUE;                                                                                                                 \
  }

#define FUNC_STATE_INT_FILL(Class, Type, Value)                                                                                      \
  EAPI int                                                                                                                           \
  edje_edit_state_fill_##Type##_offset_##Value##_get(Evas_Object * obj, const char *part, const char *state, double value)           \
  {                                                                                                                                  \
     GET_PD_OR_RETURN(0);                                                                                                            \
                                                                                                                                     \
     switch (rp->part->type)                                                                                                         \
       {                                                                                                                             \
        case EDJE_PART_TYPE_IMAGE:                                                                                                   \
        {                                                                                                                            \
           Edje_Part_Description_Image *img;                                                                                         \
                                                                                                                                     \
           img = (Edje_Part_Description_Image *)pd;                                                                                  \
                                                                                                                                     \
           return img->image.fill.Class##abs_##Value;                                                                                \
        }                                                                                                                            \
        case EDJE_PART_TYPE_PROXY:                                                                                                   \
        {                                                                                                                            \
           Edje_Part_Description_Proxy *pro;                                                                                         \
                                                                                                                                     \
           pro = (Edje_Part_Description_Proxy *)pd;                                                                                  \
                                                                                                                                     \
           return pro->proxy.fill.Class##abs_##Value;                                                                                \
        }                                                                                                                            \
       }                                                                                                                             \
     return 0;                                                                                                                       \
  }                                                                                                                                  \
  EAPI Eina_Bool                                                                                                                     \
  edje_edit_state_fill_##Type##_offset_##Value##_set(Evas_Object * obj, const char *part, const char *state, double value, double v) \
  {                                                                                                                                  \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                                   \
                                                                                                                                     \
     switch (rp->part->type)                                                                                                         \
       {                                                                                                                             \
        case EDJE_PART_TYPE_IMAGE:                                                                                                   \
        {                                                                                                                            \
           Edje_Part_Description_Image *img;                                                                                         \
                                                                                                                                     \
           img = (Edje_Part_Description_Image *)pd;                                                                                  \
                                                                                                                                     \
           img->image.fill.Class##abs_##Value = FROM_DOUBLE(v);                                                                      \
           break;                                                                                                                    \
        }                                                                                                                            \
        case EDJE_PART_TYPE_PROXY:                                                                                                   \
        {                                                                                                                            \
           Edje_Part_Description_Proxy *pro;                                                                                         \
                                                                                                                                     \
           pro = (Edje_Part_Description_Proxy *)pd;                                                                                  \
                                                                                                                                     \
           pro->proxy.fill.Class##abs_##Value = FROM_DOUBLE(v);                                                                      \
           break;                                                                                                                    \
        }                                                                                                                            \
        default:                                                                                                                     \
          return EINA_FALSE;                                                                                                         \
       }                                                                                                                             \
                                                                                                                                     \
     edje_object_calc_force(obj);                                                                                                    \
     return EINA_TRUE;                                                                                                               \
  }

FUNC_STATE_DOUBLE_FILL(pos_, origin, x);
FUNC_STATE_DOUBLE_FILL(pos_, origin, y);
FUNC_STATE_INT_FILL(pos_, origin, x);
FUNC_STATE_INT_FILL(pos_, origin, y);

FUNC_STATE_DOUBLE_FILL(, size, x);
FUNC_STATE_DOUBLE_FILL(, size, y);
FUNC_STATE_INT_FILL(, size, x);
FUNC_STATE_INT_FILL(, size, y);

EAPI Eina_Bool
edje_edit_state_visible_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   //printf("Get state visible flag of part: %s state: %s\n", part, state);
   return pd->visible;
}

EAPI Eina_Bool
edje_edit_state_visible_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool visible)
{
   if ((!obj) || (!part) || (!state))
     return EINA_FALSE;
   GET_PD_OR_RETURN(EINA_FALSE);

   if (visible) pd->visible = 1;
   else pd->visible = 0;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI unsigned char
edje_edit_state_aspect_pref_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(0);

   //printf("GET ASPECT_PREF of state '%s' [%d]\n", state, pd->aspect.prefer);
   return pd->aspect.prefer;
}

EAPI Eina_Bool
edje_edit_state_aspect_pref_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char pref)
{
   GET_PD_OR_RETURN(EINA_FALSE);
   if (pref > 4) return EINA_FALSE;
   pd->aspect.prefer = pref;
   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI const char *
edje_edit_state_color_class_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(NULL);
   //printf("Get ColorClass of part: %s state: %s\n", part, state);
   return eina_stringshare_add(pd->color_class);
}

EAPI Eina_Bool
edje_edit_state_color_class_set(Evas_Object *obj, const char *part, const char *state, double value, const char *color_class)
{
   Eina_List *l;
   Edje_Color_Class *cc;

   if ((!obj) || (!part) || (!state)) return EINA_FALSE;
   GET_PD_OR_RETURN(EINA_FALSE);

   if (!color_class)
     {
        pd->color_class = NULL;
        return EINA_TRUE;
     }

   if (!ed->file->color_classes) return EINA_FALSE;
   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     {
        if (strcmp(cc->name, color_class) == 0)
          {
             pd->color_class = eina_stringshare_add(color_class);
             edje_object_calc_force(obj);
             return EINA_TRUE;
          }
     }
   pd->color_class = NULL;
   return EINA_FALSE;
}

EAPI const Eina_List *
edje_edit_state_external_params_list_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_External *external;

   GET_PD_OR_RETURN(NULL);

   if (rp->part->type != EDJE_PART_TYPE_EXTERNAL)
     return NULL;

   external = (Edje_Part_Description_External *)pd;

   return external->external_params;
}

EAPI Eina_Bool
edje_edit_state_external_param_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Edje_External_Param_Type *type, void **val)
{
   Edje_Part_Description_External *external;
   Edje_External_Param *p;
   Eina_List *l;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_EXTERNAL)
     return EINA_FALSE;

   if (!param)
     return EINA_FALSE;

   external = (Edje_Part_Description_External *)pd;

   EINA_LIST_FOREACH(external->external_params, l, p)
     if (!strcmp(p->name, param))
       {
          if (type) *type = p->type;
          if (val)
            switch (p->type)
              {
               case EDJE_EXTERNAL_PARAM_TYPE_INT:
               case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
                 *val = &p->i;
                 break;

               case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
                 *val = &p->d;
                 break;

               case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
               case EDJE_EXTERNAL_PARAM_TYPE_STRING:
                 *val = (void *)p->s;
                 break;

               default:
                 ERR("unknown external parameter type '%d'", p->type);
              }
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_state_external_param_int_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, int *val)
{
   Edje_Part_Description_External *external;
   Edje_External_Param *p;
   Eina_List *l;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_EXTERNAL)
     {
        if (val) *val = 0;
        return EINA_FALSE;
     }

   if (!param)
     return EINA_FALSE;

   external = (Edje_Part_Description_External *)pd;

   EINA_LIST_FOREACH(external->external_params, l, p)
     if (!strcmp(p->name, param))
       {
          if (p->type != EDJE_EXTERNAL_PARAM_TYPE_INT)
            return EINA_FALSE;
          if (val)
            *val = p->i;
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_state_external_param_bool_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Eina_Bool *val)
{
   Edje_Part_Description_External *external;
   Edje_External_Param *p;
   Eina_List *l;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_EXTERNAL)
     {
        if (val) *val = 0;
        return EINA_FALSE;
     }

   if (!param)
     return EINA_FALSE;

   external = (Edje_Part_Description_External *)pd;

   EINA_LIST_FOREACH(external->external_params, l, p)
     if (!strcmp(p->name, param))
       {
          if (p->type != EDJE_EXTERNAL_PARAM_TYPE_BOOL)
            return EINA_FALSE;
          if (val)
            *val = p->i;
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_state_external_param_double_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, double *val)
{
   Edje_Part_Description_External *external;
   Edje_External_Param *p;
   Eina_List *l;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_EXTERNAL)
     {
        if (val) *val = 0;
        return EINA_FALSE;
     }

   if (!param)
     return EINA_FALSE;

   external = (Edje_Part_Description_External *)pd;

   EINA_LIST_FOREACH(external->external_params, l, p)
     if (!strcmp(p->name, param))
       {
          if (p->type != EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
            return EINA_FALSE;
          if (val)
            *val = p->d;
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_state_external_param_string_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char **val)
{
   Edje_Part_Description_External *external;
   Edje_External_Param *p;
   Eina_List *l;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_EXTERNAL)
     {
        if (val) *val = NULL;
        return EINA_FALSE;
     }

   if (!param)
     return EINA_FALSE;

   external = (Edje_Part_Description_External *)pd;

   EINA_LIST_FOREACH(external->external_params, l, p)
     if (!strcmp(p->name, param))
       {
          if (p->type != EDJE_EXTERNAL_PARAM_TYPE_STRING)
            return EINA_FALSE;
          if (val)
            *val = p->s;
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_state_external_param_choice_get(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char **val)
{
   Edje_Part_Description_External *external;
   Edje_External_Param *p;
   Eina_List *l;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_EXTERNAL)
     {
        if (val) *val = NULL;
        return EINA_FALSE;
     }

   if (!param)
     return EINA_FALSE;

   external = (Edje_Part_Description_External *)pd;

   EINA_LIST_FOREACH(external->external_params, l, p)
     if (!strcmp(p->name, param))
       {
          if (p->type != EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
            return EINA_FALSE;
          if (val)
            *val = p->s;
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_state_external_param_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Edje_External_Param_Type type, ...)
{
   va_list ap;
   Eina_List *l;
   Edje_Part_Description_External *external;
   Edje_External_Param *p = NULL, old_p = { 0, 0, 0, 0, 0 };
   int found = 0;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_EXTERNAL)
     return EINA_FALSE;

   if (!param)
     return EINA_FALSE;

   external = (Edje_Part_Description_External *)pd;

   va_start(ap, type);

   EINA_LIST_FOREACH(external->external_params, l, p)
     if (!strcmp(p->name, param))
       {
          found = 1;
          old_p = *p;
          break;
       }

   if (!found)
     {
        p = _alloc(sizeof(Edje_External_Param));
        if (!p)
          {
             va_end(ap);
             return EINA_FALSE;
          }
        p->name = eina_stringshare_add(param);
     }

   p->type = type;
   p->i = 0;
   p->d = 0;
   _edje_if_string_free(ed, &p->s);

   switch (type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_INT:
      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
        p->i = (int)va_arg(ap, int);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
        p->d = (double)va_arg(ap, double);
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
        p->s = eina_stringshare_add((const char *)va_arg(ap, char *));
        break;

      default:
        ERR("unknown external parameter type '%d'", type);
        va_end(ap);
        if (!found) free(p);
        else *p = old_p;
        return EINA_FALSE;
     }

   va_end(ap);

   //FIXME:
   //For now, we're just setting the value if the state is the selected state.
   //This is a conceptual error and is incoherent with the rest of the API!
   {
      const char *sname;
      double svalue;
      sname = edje_edit_part_selected_state_get(obj, part, &svalue);
      if (!strcmp(state, sname) && EQ(svalue, value))
        if (!edje_object_part_external_param_set(obj, part, p))
          if ((type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE) ||
              (type == EDJE_EXTERNAL_PARAM_TYPE_STRING))
            {
               _edje_if_string_free(ed, &p->s);
               if (!found) free(p);
               else *p = old_p;
               eina_stringshare_del(sname);
               return EINA_FALSE;
            }
      eina_stringshare_del(sname);
   }

   if (!found)
     {
        external->external_params = eina_list_append(external->external_params, p);
     }

   if (rp->typedata.swallow && rp->param1.external_params)
     _edje_external_parsed_params_free(rp->typedata.swallow->swallowed_object,
                                       rp->param1.external_params);
   if (rp->typedata.swallow && external->external_params)
     rp->param1.external_params = \
       _edje_external_params_parse(rp->typedata.swallow->swallowed_object,
                                   external->external_params);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_external_param_int_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, int val)
{
   return edje_edit_state_external_param_set(obj, part, state, value, param, EDJE_EXTERNAL_PARAM_TYPE_INT, val);
}

EAPI Eina_Bool
edje_edit_state_external_param_bool_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, Eina_Bool val)
{
   return edje_edit_state_external_param_set(obj, part, state, value, param, EDJE_EXTERNAL_PARAM_TYPE_BOOL, (int)val);
}

EAPI Eina_Bool
edje_edit_state_external_param_double_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, double val)
{
   return edje_edit_state_external_param_set(obj, part, state, value, param, EDJE_EXTERNAL_PARAM_TYPE_DOUBLE, val);
}

EAPI Eina_Bool
edje_edit_state_external_param_string_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char *val)
{
   return edje_edit_state_external_param_set(obj, part, state, value, param, EDJE_EXTERNAL_PARAM_TYPE_STRING, val);
}

EAPI Eina_Bool
edje_edit_state_external_param_choice_set(Evas_Object *obj, const char *part, const char *state, double value, const char *param, const char *val)
{
   return edje_edit_state_external_param_set(obj, part, state, value, param, EDJE_EXTERNAL_PARAM_TYPE_CHOICE, val);
}

EAPI Eina_Bool
edje_edit_state_step_set(Evas_Object *obj, const char *part, const char *state, double value, int step_x, int step_y)
{
   GET_PD_OR_RETURN(EINA_FALSE);
   pd->step.x = step_x;
   pd->step.y = step_y;
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_step_get(Evas_Object *obj, const char *part, const char *state, double value, int *step_x, int *step_y)
{
   GET_PD_OR_RETURN(EINA_FALSE);
   if (step_x) *step_x = (int)pd->step.x;
   if (step_y) *step_y = (int)pd->step.y;
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_limit_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char limit)
{
   GET_PD_OR_RETURN(EINA_FALSE);
   if (limit >= EDJE_STATE_LIMIT_LAST)
     return EINA_FALSE;
   pd->limit = limit;
   return EINA_TRUE;
}

EAPI unsigned char
edje_edit_state_limit_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EDJE_STATE_LIMIT_LAST);
   return pd->limit;
}

/**************/
/*  MAP API */
/**************/

EAPI const char *
edje_edit_state_map_light_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Real_Part *erl;

   GET_PD_OR_RETURN(NULL);

   if (pd->map.id_light == -1) return NULL;

   erl = ed->table_parts[pd->map.id_light];
   if (erl->part->name)
     return eina_stringshare_add(erl->part->name);

   return NULL;
}

EAPI const char *
edje_edit_state_map_rotation_center_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Real_Part *erl;

   GET_PD_OR_RETURN(NULL);

   if (pd->map.rot.id_center == -1) return NULL;

   erl = ed->table_parts[pd->map.rot.id_center];
   if (erl->part->name)
     return eina_stringshare_add(erl->part->name);

   return NULL;
}

EAPI Eina_Bool
edje_edit_state_map_backface_cull_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   return pd->map.backcull;
}

EAPI Eina_Bool
edje_edit_state_map_backface_cull_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool bool)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   pd->map.backcull = bool;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_perspective_on_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   return pd->map.persp_on;
}

EAPI Eina_Bool
edje_edit_state_map_perspective_on_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool bool)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   pd->map.persp_on = bool;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_alpha_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   return pd->map.alpha;
}

EAPI Eina_Bool
edje_edit_state_map_alpha_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool bool)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   pd->map.alpha = bool;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_smooth_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool bool)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   pd->map.smooth = bool;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_smooth_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   return pd->map.smooth;
}

EAPI Eina_Bool
edje_edit_state_map_rotation_set(Evas_Object *obj, const char *part, const char *state, double value, double x, double y, double z)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   pd->map.rot.x = FROM_DOUBLE(x);
   pd->map.rot.y = FROM_DOUBLE(y);
   pd->map.rot.z = FROM_DOUBLE(z);

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_rotation_get(Evas_Object *obj, const char *part, const char *state, double value, double *x, double *y, double *z)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   if (x) *x = TO_DOUBLE(pd->map.rot.x);
   if (y) *y = TO_DOUBLE(pd->map.rot.y);
   if (z) *z = TO_DOUBLE(pd->map.rot.z);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_zoom_set(Evas_Object *obj, const char *part, const char *state, double value, double x, double y)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   pd->map.zoom.x = FROM_DOUBLE(x);
   pd->map.zoom.y = FROM_DOUBLE(y);

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_zoom_get(Evas_Object *obj, const char *part, const char *state, double value, double *x, double *y)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   if (x) *x = TO_DOUBLE(pd->map.zoom.x);
   if (y) *y = TO_DOUBLE(pd->map.zoom.y);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_perspective_zplane_set(Evas_Object *obj, const char *part, const char *state, double value, int zplane)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   pd->persp.zplane = zplane;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI int
edje_edit_state_map_perspective_zplane_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   return pd->persp.zplane;
}

EAPI Eina_Bool
edje_edit_state_map_perspective_focal_set(Evas_Object *obj, const char *part, const char *state, double value, int focal)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   pd->persp.focal = focal;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI int
edje_edit_state_map_perspective_focal_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   return pd->persp.focal;
}

EAPI Eina_Bool
edje_edit_state_map_light_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source_part)
{
   int src_id = -1;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (source_part)
     src_id = _edje_part_id_find(ed, source_part);

   pd->map.id_light = src_id;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_rotation_center_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source_part)
{
   int src_id = -1;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (source_part)
     src_id = _edje_part_id_find(ed, source_part);

   pd->map.rot.id_center = src_id;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_point_color_get(Evas_Object *obj, const char *part, const char *state, double value, int idx, int *r, int *g, int *b, int *a)
{
   Edje_Map_Color *color = NULL;
   unsigned int i;

   if ((!obj) || (!part) || (!state))
     return EINA_FALSE;

   GET_PD_OR_RETURN(EINA_FALSE);

   /* check if current color is exists and get it. */
   for (i = 0; i < pd->map.colors_count; ++i)
     {
        if (pd->map.colors[i]->idx == idx)
          {
             color = pd->map.colors[i];
             break;
          }
     }
   if (!color)
     {
        if (r) *r = 255;
        if (g) *g = 255;
        if (b) *b = 255;
        if (a) *a = 255;
     }
   else
     {
        if (r) *r = color->r;
        if (g) *g = color->g;
        if (b) *b = color->b;
        if (a) *a = color->a;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_point_color_set(Evas_Object *obj, const char *part, const char *state, double value, int idx, int r, int g, int b, int a)
{
   Edje_Map_Color *color = NULL;
   unsigned int i;

   if ((!obj) || (!part) || (!state))
     return EINA_FALSE;

   GET_PD_OR_RETURN(EINA_FALSE);

   /* check if current color is exists and get it. */
   for (i = 0; i < pd->map.colors_count; ++i)
     {
        if (pd->map.colors[i]->idx == idx)
          {
             color = pd->map.colors[i];
             break;
          }
     }

   if (!color)
     {
        if (!(color = _alloc(sizeof(Edje_Map_Color))))
          return EINA_FALSE;

        pd->map.colors_count++;
        pd->map.colors =
          realloc(pd->map.colors,
                  sizeof(Edje_Map_Color *) * pd->map.colors_count);
        pd->map.colors[pd->map.colors_count - 1] = color;
     }

   color->idx = idx;
   if ((r > -1) && (r < 256)) color->r = r;
   else return EINA_FALSE;
   if ((g > -1) && (g < 256)) color->g = g;
   else return EINA_FALSE;
   if ((b > -1) && (b < 256)) color->b = b;
   else return EINA_FALSE;
   if ((a > -1) && (a < 256)) color->a = a;
   else return EINA_FALSE;

   return EINA_TRUE;
}

EAPI const char *
edje_edit_state_map_perspective_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Real_Part *erl;

   GET_PD_OR_RETURN(NULL);

   if (pd->map.id_persp == -1) return NULL;

   erl = ed->table_parts[pd->map.id_persp];
   if (erl->part->name)
     return eina_stringshare_add(erl->part->name);

   return NULL;
}

EAPI Eina_Bool
edje_edit_state_map_perspective_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source_part)
{
   int src_id = -1;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (source_part)
     src_id = _edje_part_id_find(ed, source_part);

   pd->map.id_persp = src_id;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_map_on_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   return pd->map.on;
}

EAPI Eina_Bool
edje_edit_state_map_on_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool on)
{
   GET_PD_OR_RETURN(EINA_FALSE);

   pd->map.on = on;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

/*********************/
/*  SIZE CLASSES API */
/*********************/

EAPI Eina_List *
edje_edit_size_classes_list_get(Evas_Object *obj)
{
   Eina_List *classes = NULL;
   Eina_List *l;
   Edje_Size_Class *sc;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->size_classes)
     return NULL;
   EINA_LIST_FOREACH(ed->file->size_classes, l, sc)
     classes = eina_list_append(classes, eina_stringshare_add(sc->name));

   return classes;
}

EAPI Eina_Bool
edje_edit_size_class_add(Evas_Object *obj, const char *name)
{
   Eina_List *l;
   Edje_Size_Class *sc, *s;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name || !ed->file)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->size_classes, l, sc)
     if (strcmp(sc->name, name) == 0)
       return EINA_FALSE;

   s = _alloc(sizeof(Edje_Size_Class));
   if (!s) return EINA_FALSE;

   s->name = eina_stringshare_add(name);
   /* set default values for max */
   s->maxw = -1;
   s->maxh = -1;

   ed->file->size_classes = eina_list_append(ed->file->size_classes, s);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_size_class_del(Evas_Object *obj, const char *name)
{
   Eina_List *l;
   Edje_Size_Class *sc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name || !ed->file || !ed->file->size_classes)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->size_classes, l, sc)
     if (strcmp(sc->name, name) == 0)
       {
          _edje_if_string_free(ed, &sc->name);
          ed->file->size_classes = eina_list_remove(ed->file->size_classes, sc);
          free(sc);
          return EINA_TRUE;
       }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_size_class_name_set(Evas_Object *obj, const char *name, const char *newname)
{
   Eina_List *l;
   Edje_Size_Class *sc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->size_classes || !newname)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->size_classes, l, sc)
     if (!strcmp(sc->name, name))
       {
          _edje_if_string_replace(ed, &sc->name, newname);
          return EINA_TRUE;
       }

   return EINA_FALSE;
}
#define FUNC_SIZE_CLASS(TYPE, VALUE, MIN)                                              \
EAPI Evas_Coord                                                                        \
edje_edit_size_class_##TYPE##_##VALUE##_get(Evas_Object *obj, const char *class_name)  \
{                                                                                      \
   Eina_List *l;                                                                       \
   Edje_Size_Class *sc;                                                                \
                                                                                       \
   GET_ED_OR_RETURN(EINA_FALSE);                                                       \
                                                                                       \
   if (!ed->file || !ed->file->size_classes || !class_name)                            \
     return EINA_FALSE;                                                                \
                                                                                       \
   EINA_LIST_FOREACH(ed->file->size_classes, l, sc)                                    \
      if (!strcmp(sc->name, class_name))                                               \
        return sc->TYPE##VALUE;                                                        \
                                                                                       \
   return 0;                                                                           \
}                                                                                      \
EAPI Eina_Bool                                                                         \
edje_edit_size_class_##TYPE##_##VALUE##_set(Evas_Object *obj, const char *class_name, Evas_Coord size)\
{                                                                                      \
   Eina_List *l;                                                                       \
   Edje_Size_Class *sc;                                                                \
                                                                                       \
   GET_ED_OR_RETURN(EINA_FALSE);                                                       \
                                                                                       \
   if (!ed->file || !ed->file->size_classes || !class_name)                            \
     return EINA_FALSE;                                                                \
   if (size < MIN)                                                                     \
     return EINA_FALSE;                                                                \
                                                                                       \
   EINA_LIST_FOREACH(ed->file->size_classes, l, sc)                                    \
     if (!strcmp(sc->name, class_name))                                                \
       {                                                                               \
          sc->TYPE##VALUE = size;                                                      \
          return EINA_TRUE;                                                            \
       }                                                                               \
   return EINA_FALSE;                                                                  \
}

FUNC_SIZE_CLASS(min, w, 0)
FUNC_SIZE_CLASS(min, h, 0)
FUNC_SIZE_CLASS(max, w, -1)
FUNC_SIZE_CLASS(max, h, -1)

/*********************/
/*  TEXT CLASSES API */
/*********************/

EAPI Eina_List *
edje_edit_text_classes_list_get(Evas_Object *obj)
{
   Eina_List *classes = NULL;
   Eina_List *l;
   Edje_Text_Class *tc;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->text_classes)
     return NULL;
   EINA_LIST_FOREACH(ed->file->text_classes, l, tc)
     classes = eina_list_append(classes, eina_stringshare_add(tc->name));

   return classes;
}

EAPI Eina_Bool
edje_edit_text_class_add(Evas_Object *obj, const char *name)
{
   Eina_List *l;
   Edje_Text_Class *tc, *t;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name || !ed->file)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->text_classes, l, tc)
     if (strcmp(tc->name, name) == 0)
       return EINA_FALSE;

   t = _alloc(sizeof(Edje_Text_Class));
   if (!t) return EINA_FALSE;

   t->name = eina_stringshare_add(name);

   ed->file->text_classes = eina_list_append(ed->file->text_classes, t);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_text_class_del(Evas_Object *obj, const char *name)
{
   Eina_List *l;
   Edje_Text_Class *tc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name || !ed->file || !ed->file->text_classes)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->text_classes, l, tc)
     if (strcmp(tc->name, name) == 0)
       {
          _edje_if_string_free(ed, &tc->name);
          _edje_if_string_free(ed, &tc->font);
          ed->file->text_classes = eina_list_remove(ed->file->text_classes, tc);
          free(tc);
          return EINA_TRUE;
       }
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_text_class_name_set(Evas_Object *obj, const char *name, const char *newname)
{
   Eina_List *l;
   Edje_Text_Class *tc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->text_classes)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->text_classes, l, tc)
     if (!strcmp(tc->name, name))
       {
          _edje_if_string_replace(ed, &tc->name, newname);
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

EAPI Eina_Stringshare *
edje_edit_text_class_font_get(Evas_Object *obj, const char *class_name)
{
   Eina_List *l;
   Edje_Text_Class *tc;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->text_classes)
     return NULL;

   EINA_LIST_FOREACH(ed->file->text_classes, l, tc)
      if (!strcmp(tc->name, class_name))
        return eina_stringshare_add(tc->font);

   return NULL;
}

EAPI Eina_Bool
edje_edit_text_class_font_set(Evas_Object *obj, const char *class_name, const char *font)
{
   Eina_List *l;
   Edje_Text_Class *tc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->text_classes || !class_name)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->text_classes, l, tc)
     if (!strcmp(tc->name, class_name))
       {
          _edje_if_string_replace(ed, &tc->font, font);
          return EINA_TRUE;
       }
   return EINA_FALSE;
}

EAPI Evas_Font_Size
edje_edit_text_class_size_get(Evas_Object *obj, const char *class_name)
{
   Eina_List *l;
   Edje_Text_Class *tc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->text_classes)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->text_classes, l, tc)
      if (!strcmp(tc->name, class_name))
        return tc->size;

   return 0;
}

EAPI Eina_Bool
edje_edit_text_class_size_set(Evas_Object *obj, const char *class_name, Evas_Font_Size size)
{
   Eina_List *l;
   Edje_Text_Class *tc;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file || !ed->file->text_classes || !class_name)
     return EINA_FALSE;
   if (size < 0)
     return EINA_FALSE;

   EINA_LIST_FOREACH(ed->file->text_classes, l, tc)
     if (!strcmp(tc->name, class_name))
       {
          tc->size = size;
          return EINA_TRUE;
       }
   return EINA_FALSE;
}


/**************/
/*  TEXT API */
/**************/

EAPI const char *
edje_edit_state_text_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Text *txt;

   GET_PD_OR_RETURN(NULL);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return NULL;

   txt = (Edje_Part_Description_Text *)pd;

   return eina_stringshare_add(edje_string_get(&txt->text.text));
}

EAPI Eina_Bool
edje_edit_state_text_set(Evas_Object *obj, const char *part, const char *state, double value, const char *text)
{
   Edje_Part_Description_Text *txt;
   Edje_Real_Part *real;
   unsigned short i;

   if (!text)
     return EINA_FALSE;
   GET_PD_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   txt = (Edje_Part_Description_Text *)pd;

   _edje_if_string_replace(ed, &txt->text.text.str, text);
   txt->text.text.id = 0;

   for (i = 0; i < ed->table_parts_size; i++)
     {
        real = ed->table_parts[i];
        if (((rp->part->type == EDJE_PART_TYPE_TEXT) ||
             (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)) &&
            (real->typedata.text) && (real->typedata.text->text_source == rp))
          {
             txt = _edje_real_part_text_text_source_description_get(real, NULL);
             _edje_if_string_replace(ed, &txt->text.text.str, text);
             txt->text.text.id = 0;
          }
     }

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI int
edje_edit_state_text_size_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Text *txt;

   GET_PD_OR_RETURN(-1);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return -1;

   txt = (Edje_Part_Description_Text *)pd;

   return txt->text.size;
}

EAPI Eina_Bool
edje_edit_state_text_size_set(Evas_Object *obj, const char *part, const char *state, double value, int size)
{
   Edje_Part_Description_Text *txt;

   if (size < 0) return EINA_FALSE;
   GET_PD_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   txt = (Edje_Part_Description_Text *)pd;

   txt->text.size = size;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

#define FUNC_TEXT_DOUBLE(Name, Value, Min)                                                                          \
  EAPI double                                                                                                       \
  edje_edit_state_text_##Name##_get(Evas_Object * obj, const char *part, const char *state, double value)           \
  {                                                                                                                 \
     Edje_Part_Description_Text *txt;                                                                               \
                                                                                                                    \
     GET_PD_OR_RETURN(0);                                                                                           \
                                                                                                                    \
     if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&                                                                 \
         (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))                                                              \
       return 0;                                                                                                    \
                                                                                                                    \
     txt = (Edje_Part_Description_Text *)pd;                                                                        \
     return TO_DOUBLE(txt->text.Value);                                                                             \
  }                                                                                                                 \
  EAPI Eina_Bool                                                                                                    \
  edje_edit_state_text_##Name##_set(Evas_Object * obj, const char *part, const char *state, double value, double v) \
  {                                                                                                                 \
     Edje_Part_Description_Text *txt;                                                                               \
     if ((!obj) || (!part) || (!state))                                                                             \
       return EINA_FALSE;                                                                                           \
     if ((v < Min) || (v > 1.0))                                                                                    \
       return EINA_FALSE;                                                                                           \
                                                                                                                    \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                  \
                                                                                                                    \
     if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&                                                                 \
         (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))                                                              \
       return EINA_FALSE;                                                                                           \
                                                                                                                    \
     txt = (Edje_Part_Description_Text *)pd;                                                                        \
     txt->text.Value = FROM_DOUBLE(v);                                                                              \
     edje_object_calc_force(obj);                                                                                   \
     return EINA_TRUE;                                                                                              \
  }

FUNC_TEXT_DOUBLE(align_x, align.x, -1.0);
FUNC_TEXT_DOUBLE(align_y, align.y, 0.0);
FUNC_TEXT_DOUBLE(elipsis, ellipsis, -1.0);

#define FUNC_TEXT_BOOL(Name, Type)                                                                                              \
  EAPI Eina_Bool                                                                                                                \
  edje_edit_state_text_##Name##_##Type##_get(Evas_Object * obj, const char *part, const char *state, double value)              \
  {                                                                                                                             \
     Edje_Part_Description_Text *txt;                                                                                           \
                                                                                                                                \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                              \
                                                                                                                                \
     if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&                                                                             \
         (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))                                                                          \
       return EINA_FALSE;                                                                                                       \
                                                                                                                                \
     txt = (Edje_Part_Description_Text *)pd;                                                                                    \
     return txt->text.Name##_##Type;                                                                                            \
  }                                                                                                                             \
  EAPI Eina_Bool                                                                                                                \
  edje_edit_state_text_##Name##_##Type##_set(Evas_Object * obj, const char *part, const char *state, double value, Eina_Bool v) \
  {                                                                                                                             \
     Edje_Part_Description_Text *txt;                                                                                           \
     if ((!obj) || (!part) || (!state))                                                                                         \
       return EINA_FALSE;                                                                                                       \
     GET_PD_OR_RETURN(EINA_FALSE);                                                                                              \
                                                                                                                                \
     if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&                                                                             \
         (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))                                                                          \
       return EINA_FALSE;                                                                                                       \
                                                                                                                                \
     txt = (Edje_Part_Description_Text *)pd;                                                                                    \
     txt->text.Name##_##Type = v ? 1 : 0;                                                                                       \
     edje_object_calc_force(obj);                                                                                               \
     return EINA_TRUE;                                                                                                          \
  }

FUNC_TEXT_BOOL(fit, x);
FUNC_TEXT_BOOL(fit, y);
FUNC_TEXT_BOOL(min, x);
FUNC_TEXT_BOOL(min, y);
FUNC_TEXT_BOOL(max, x);
FUNC_TEXT_BOOL(max, y);

EAPI const char *
edje_edit_state_text_style_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Text *txt;

   GET_PD_OR_RETURN(NULL);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return NULL;

   txt = (Edje_Part_Description_Text *)pd;
   if (txt->text.style.str)
     return eina_stringshare_add(txt->text.style.str);
   else
     return NULL;
}

EAPI Eina_Bool
edje_edit_state_text_style_set(Evas_Object *obj, const char *part, const char *state, double value, const char *style)
{
   Edje_Part_Description_Text *txt;

   GET_PD_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   txt = (Edje_Part_Description_Text *)pd;
   if (style == NULL )
     {
        _edje_if_string_free(ed, &txt->text.style.str);
        txt->text.style.str = NULL;
     }
   else
     {
        _edje_if_string_replace(ed, &txt->text.style.str, style);
     }

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_List *
edje_edit_fonts_list_get(Evas_Object *obj)
{
   Eina_Iterator *it;
   Eina_List *fonts = NULL;
   Edje_Font_Directory_Entry *f;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->fonts) return NULL;

   it = eina_hash_iterator_data_new(ed->file->fonts);
   if (!it) return NULL;

   EINA_ITERATOR_FOREACH(it, f)
     fonts = eina_list_append(fonts, eina_stringshare_add(f->entry));

   eina_iterator_free(it);

   return fonts;
}

EAPI Eina_Bool
edje_edit_font_add(Evas_Object *obj, const char *path, const char *alias)
{
   char entry[PATH_MAX];
   const char *new_path;
   struct stat st;
   Edje_Font_Directory_Entry *fnt;

   GET_ED_OR_RETURN(EINA_FALSE);

   INF("ADD FONT: %s", path);

   if (!path) return EINA_FALSE;
   if (stat(path, &st) || !S_ISREG(st.st_mode)) return EINA_FALSE;
   if (!ed->file) return EINA_FALSE;
   if (!ed->path) return EINA_FALSE;

   /* Alias */
   if (alias)
     {
        new_path = ecore_file_file_get(path);
     }
   else
     {
        alias = ecore_file_file_get(path);
        new_path = alias;
     }
   snprintf(entry, sizeof(entry), "edje/fonts/%s", alias);

   /* Initializing a new font hash, if no exist */
   if (!ed->file->fonts)
     {
        ed->file->fonts = eina_hash_string_small_new(NULL);
        if (!ed->file->fonts) return EINA_FALSE;
     }

   /* Check if exists */
   fnt = eina_hash_find(ed->file->fonts, alias);
   if (fnt)
     return EINA_FALSE;

   /* Create Edje_Font_Directory_Entry */
   fnt = _alloc(sizeof(Edje_Font_Directory_Entry));
   if (!fnt)
     {
        ERR("Unable to alloc font entry part \"%s\"", alias);
        return EINA_FALSE;
     }
   fnt->file = eina_stringshare_add(new_path);
   fnt->entry = eina_stringshare_add(alias);

   eina_hash_direct_add(ed->file->fonts, fnt->entry, fnt);

   /* Import font */
   if (!_edje_edit_file_import(ed, path, entry, 1))
     {
        eina_hash_del(ed->file->fonts, fnt->entry, fnt);
        eina_stringshare_del(fnt->file);
        eina_stringshare_del(fnt->entry);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_font_del(Evas_Object *obj, const char *alias)
{
   Edje_Font_Directory_Entry *fnt;

   GET_ED_OR_RETURN(EINA_FALSE);

   INF("DEL FONT: %s", alias);

   if (!alias) return EINA_FALSE;
   if (!ed->file) return EINA_FALSE;
   if (!ed->path) return EINA_FALSE;

   fnt = eina_hash_find(ed->file->fonts, alias);
   if (!fnt)
     {
        WRN("Unable to find font entry part \"%s\"", alias);
        return EINA_FALSE;
     }

   /* Erase font to edje file */
   {
      char entry[PATH_MAX];
      Eet_File *eetf;

      /* open the eet file */
      eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
      if (!eetf)
        return EINA_FALSE;

      snprintf(entry, sizeof(entry), "edje/fonts/%s", alias);

      if (eet_delete(eetf, entry) <= 0)
        {
           ERR("Unable to delete \"%s\" font entry", entry);
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }

      /* write the edje_file */
      if (!_edje_edit_edje_file_save(eetf, ed->file))
        {
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }
      _edje_edit_eet_close(eetf);
   }

   eina_hash_del(ed->file->fonts, alias, fnt);

   return EINA_TRUE;
}

EAPI const char *
edje_edit_font_path_get(Evas_Object *obj, const char *alias)
{
   Eina_Iterator *it;
   Edje_Font_Directory_Entry *f;
   const char *str = NULL;

   if (!alias) return NULL;
   GET_ED_OR_RETURN(NULL);

   if (!ed->file || !ed->file->fonts) return NULL;

   it = eina_hash_iterator_data_new(ed->file->fonts);
   if (!it) return NULL;

   EINA_ITERATOR_FOREACH(it, f)
     if (!strcmp(f->entry, alias))
       {
          str = f->file;
          break;
       }

   eina_iterator_free(it);
   return eina_stringshare_add(str);
}

EAPI const char *
edje_edit_state_font_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Text *txt;

   GET_PD_OR_RETURN(NULL);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return NULL;

   txt = (Edje_Part_Description_Text *)pd;

   return eina_stringshare_add(edje_string_get(&txt->text.font));
}

EAPI Eina_Bool
edje_edit_state_font_set(Evas_Object *obj, const char *part, const char *state, double value, const char *font)
{
   Edje_Part_Description_Text *txt;

   GET_PD_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   txt = (Edje_Part_Description_Text *)pd;

   _edje_if_string_replace(ed, &txt->text.font.str, font);
   txt->text.font.id = 0;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Edje_Text_Effect
edje_edit_part_effect_get(Evas_Object *obj, const char *part)
{
   GET_RP_OR_RETURN(0);

   //printf("GET EFFECT of part: %s\n", part);
   return rp->part->effect;
}

EAPI Eina_Bool
edje_edit_part_effect_set(Evas_Object *obj, const char *part, Edje_Text_Effect effect)
{
   if ((!obj) || (!part)) return EINA_FALSE;
   GET_RP_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;
   rp->part->effect = effect;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI const char *
edje_edit_state_text_text_source_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Real_Part *rel;

   GET_PD_OR_RETURN(NULL);

   if ((rp->part->type == EDJE_PART_TYPE_TEXT) ||
       (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK))
     {
        Edje_Part_Description_Text *txt;
        txt = (Edje_Part_Description_Text *)pd;
        if (txt->text.id_text_source == -1) return NULL;
        rel = ed->table_parts[txt->text.id_text_source % ed->table_parts_size];
        if (rel->part->name) return eina_stringshare_add(rel->part->name);
     }

   return NULL;
}

EAPI Eina_Bool
edje_edit_state_text_text_source_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source)
{
   Edje_Part_Description_Text *txt;
   int id_text_source;

   GET_PD_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   if (source)
     {
        txt = (Edje_Part_Description_Text *)pd;

        id_text_source = _edje_part_id_find(ed, source);
        txt->text.id_text_source = id_text_source;

        txt->text.text.str = eina_stringshare_add(NULL);
        txt->text.text.id = 0;
     }
   else
     {
        txt = (Edje_Part_Description_Text *)pd;
        txt->text.id_text_source = -1;
        txt->text.text.str = eina_stringshare_add(NULL);
     }

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI const char *
edje_edit_state_text_source_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Real_Part *rel;

   GET_PD_OR_RETURN(NULL);

   if ((rp->part->type == EDJE_PART_TYPE_TEXT) ||
       (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK))
     {
        Edje_Part_Description_Text *txt;
        txt = (Edje_Part_Description_Text *)pd;
        if (txt->text.id_source == -1) return NULL;
        rel = ed->table_parts[txt->text.id_source % ed->table_parts_size];
        if (rel->part->name) return eina_stringshare_add(rel->part->name);
     }

   return NULL;
}

EAPI Eina_Bool
edje_edit_state_text_source_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source)
{
   Edje_Part_Description_Text *txt;
   int id_source;

   GET_PD_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   if (source)
     {
        txt = (Edje_Part_Description_Text *)pd;

        id_source = _edje_part_id_find(ed, source);
        txt->text.id_source = id_source;
     }
   else
     {
        txt = (Edje_Part_Description_Text *)pd;
        txt->text.id_source = -1;
        txt->text.text.str = eina_stringshare_add(NULL);
     }
   /* need to recalc, because the source part can has a text */
   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI const char *
edje_edit_state_text_class_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Text *txt;
   GET_PD_OR_RETURN(NULL);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return NULL;
   txt = (Edje_Part_Description_Text *)pd;

   return eina_stringshare_add(txt->text.text_class);
}

EAPI Eina_Bool
edje_edit_state_text_class_set(Evas_Object *obj, const char *part, const char *state, double value, const char *text_class)
{
   Edje_Part_Description_Text *txt;
   GET_PD_OR_RETURN(EINA_FALSE);

   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;
   txt = (Edje_Part_Description_Text *)pd;

   txt->text.text_class = (char *)eina_stringshare_add(text_class);
   return EINA_TRUE;
}

EAPI const char *
edje_edit_state_text_repch_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Text *txt;

   GET_PD_OR_RETURN(NULL);
   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return NULL;

   txt = (Edje_Part_Description_Text *)pd;

   return eina_stringshare_add(edje_string_get(&txt->text.repch));
}

EAPI Eina_Bool
edje_edit_state_text_repch_set(Evas_Object *obj, const char *part, const char *state, double value, const char *repch)
{
   Edje_Part_Description_Text *txt;

   if (!repch) return EINA_FALSE;
   GET_PD_OR_RETURN(EINA_FALSE);
   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   txt = (Edje_Part_Description_Text *)pd;
   _edje_if_string_replace(ed, &txt->text.repch.str, repch);
   txt->text.repch.id = 0;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_text_size_range_min_max_get(Evas_Object *obj, const char *part, const char *state, double value, int *min, int *max)
{
   Edje_Part_Description_Text *txt;
   GET_PD_OR_RETURN(EINA_FALSE);
   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   txt = (Edje_Part_Description_Text *)pd;
   if (min) *min = txt->text.size_range_min;
   if (max) *max = txt->text.size_range_max;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_text_size_range_min_max_set(Evas_Object *obj, const char *part, const char *state, double value, int min, int max)
{
   Edje_Part_Description_Text *txt;
   GET_PD_OR_RETURN(EINA_FALSE);
   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return EINA_FALSE;

   txt = (Edje_Part_Description_Text *)pd;
   txt->text.size_range_min = min;
   txt->text.size_range_max = max;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_proxy_source_set(Evas_Object *obj, const char *part, const char *state, double value, const char *source_name)
{
   GET_PD_OR_RETURN(EINA_FALSE);
   if (rp->part->type != EDJE_PART_TYPE_PROXY)
     return EINA_FALSE;

   Edje_Part_Description_Proxy *proxy_part = (Edje_Part_Description_Proxy *)pd;

   if (source_name)
     {
        int source_id = _edje_part_id_find(ed, source_name);
        if (source_id < 0)
          return EINA_FALSE;
        proxy_part->proxy.id = source_id;
     }
   else proxy_part->proxy.id = -1;

   return EINA_TRUE;
}

EAPI Eina_Stringshare *
edje_edit_state_proxy_source_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(NULL);
   if (rp->part->type != EDJE_PART_TYPE_PROXY)
     return NULL;

   Edje_Part_Description_Proxy *proxy_part = (Edje_Part_Description_Proxy *)pd;
   const char *source_name;
   source_name = _edje_part_name_find(ed, proxy_part->proxy.id);

   return eina_stringshare_add(source_name);
}

EAPI Eina_Bool
edje_edit_state_proxy_source_clip_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool clip)
{
   GET_PD_OR_RETURN(EINA_FALSE);
   if (rp->part->type != EDJE_PART_TYPE_PROXY)
     return EINA_FALSE;

   Edje_Part_Description_Proxy *proxy_part = (Edje_Part_Description_Proxy *)pd;
   proxy_part->proxy.source_clip = clip;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_proxy_source_clip_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);
   if (rp->part->type != EDJE_PART_TYPE_PROXY)
     return EINA_FALSE;

   Edje_Part_Description_Proxy *proxy_part = (Edje_Part_Description_Proxy *)pd;

   return proxy_part->proxy.source_clip;
}

EAPI Eina_Bool
edje_edit_state_proxy_source_visible_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool visibility)
{
   GET_PD_OR_RETURN(EINA_FALSE);
   if (rp->part->type != EDJE_PART_TYPE_PROXY)
     return EINA_FALSE;

   Edje_Part_Description_Proxy *proxy_part = (Edje_Part_Description_Proxy *)pd;
   proxy_part->proxy.source_visible = visibility;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_proxy_source_visible_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   GET_PD_OR_RETURN(EINA_FALSE);
   if (rp->part->type != EDJE_PART_TYPE_PROXY)
     return EINA_FALSE;

   Edje_Part_Description_Proxy *proxy_part = (Edje_Part_Description_Proxy *)pd;

   return proxy_part->proxy.source_visible;
}

/*****************/
/* IMAGE SET API */
/*****************/

EAPI Eina_Bool
edje_edit_image_set_exists(Evas_Object *obj, const char *image)
{
   Edje_Image_Directory_Set *de;
   unsigned int i;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file) return EINA_FALSE;
   if (!ed->file->image_dir) return EINA_FALSE;

   // Gets the Set Entry
   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        if (de->name && !strcmp(de->name, image))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI int
edje_edit_image_set_id_get(Evas_Object *obj, const char *image_name)
{
   GET_EED_OR_RETURN(-1);

   return _edje_set_id_find(eed, image_name);
}

EAPI Eina_Bool
edje_edit_image_set_rename(Evas_Object *obj, const char *set, const char *new_set)
{
   Edje_Image_Directory_Set *de = NULL;
   unsigned int i;
   GET_ED_OR_RETURN(EINA_FALSE);

   if (!new_set) return EINA_FALSE;

   // Check if image set with name 'new_set' already exists
   if (edje_edit_image_set_id_get(obj, new_set) >= 0)
     return EINA_FALSE;

   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        if ((de->name) && (!strcmp(set, de->name)))
          break;
     }
   if (i == ed->file->image_dir->sets_count) return EINA_FALSE;

   de->name = (char *)new_set;

   return EINA_TRUE;
}

EAPI Eina_List *
edje_edit_image_set_list_get(Evas_Object *obj)
{
   Eina_List *sets = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file) return NULL;
   if (!ed->file->image_dir) return NULL;

   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     sets = eina_list_append(sets,
                             eina_stringshare_add(ed->file->image_dir->sets[i].name));

   return sets;
}

EAPI Eina_Bool
edje_edit_image_set_add(Evas_Object *obj, const char *name)
{
   Edje_Image_Directory_Set *de;
   unsigned int i;
   int free_id = -1;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name) return EINA_FALSE;
   if (!ed->file) return EINA_FALSE;

   /* Create Image_Directory if not exist */
   if (!ed->file->image_dir)
     {
        ed->file->image_dir = _alloc(sizeof(Edje_Image_Directory));
        if (!ed->file->image_dir) return EINA_FALSE;
     }

   /* Loop trough image directory to find if image exist */
   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;

        if (!de->name)
          free_id = i;
        else if (!strcmp(name, de->name))
          return EINA_FALSE;
     }

   if (free_id == -1)
     {
        Edje_Image_Directory_Set *tmp;
        unsigned int count;

        count = ed->file->image_dir->sets_count + 1;

        tmp = realloc(ed->file->image_dir->sets,
                      sizeof (Edje_Image_Directory_Set) * count);
        if (!tmp) return EINA_FALSE;

        ed->file->image_dir->sets = tmp;
        free_id = ed->file->image_dir->sets_count;
        ed->file->image_dir->sets_count = count;
     }

   /* Set Image Entry */
   de = ed->file->image_dir->sets + free_id;
   de->name = (char *)name;
   de->id = free_id;
   de->entries = NULL;

   return EINA_TRUE;
}

EAPI Eina_List *
edje_edit_set_usage_list_get(Evas_Object *obj, const char *name, Eina_Bool first_only)
{
   Eina_List *result = NULL;
   Eina_Iterator *it;
   Edje_Part_Collection_Directory_Entry *pce;
   Edje_Part_Image_Use *item;
   Edje_Part *part;
   Edje_Part_Description_Image *part_desc_image;
   unsigned int i, j, k;
   int set_id;

   GET_ED_OR_RETURN(NULL);

   set_id = edje_edit_image_set_id_get(obj, name);
   if (set_id < 0)
     return NULL;

   it = eina_hash_iterator_data_new(ed->file->collection);

#define ITEM_ADD()                                                             \
  item = (Edje_Part_Image_Use *)calloc(1, sizeof(Edje_Part_Image_Use));        \
  item->group = eina_stringshare_add(pce->entry);                              \
  item->part = eina_stringshare_add(part->name);                               \
  item->state.name = eina_stringshare_add(part_desc_image->common.state.name); \
  item->state.value = part_desc_image->common.state.value;                     \
  result = eina_list_append(result, item);

   #define FIND_IN_PART_DESCRIPTION()                        \
  if ((part_desc_image->image.id == set_id) &&               \
      (part_desc_image->image.set))                          \
    {                                                        \
       ITEM_ADD();                                           \
       if (first_only)                                       \
         goto end;                                           \
       else                                                  \
         continue;                                           \
    }                                                        \
  for (k = 0; k < part_desc_image->image.tweens_count; k++)  \
    {                                                        \
       if ((part_desc_image->image.tweens[k]->id == set_id)  \
           && (part_desc_image->image.tweens[k]->set))       \
         {                                                   \
            ITEM_ADD();                                      \
            if (first_only)                                  \
              goto end;                                      \
            else                                             \
              continue;                                      \
         }                                                   \
    }

   EINA_ITERATOR_FOREACH(it, pce)
     {
        if (!pce->ref) continue;
        for (i = 0; i < pce->ref->parts_count; i++)
          {
             if (!pce->ref->parts) continue;
             part = pce->ref->parts[i];
             if (part->type == EDJE_PART_TYPE_IMAGE)
               {
                  part_desc_image = (Edje_Part_Description_Image *)part->default_desc;
                  FIND_IN_PART_DESCRIPTION();
                  for (j = 0; j < part->other.desc_count; j++)
                    {
                       part_desc_image = (Edje_Part_Description_Image *)part->other.desc[j];
                       FIND_IN_PART_DESCRIPTION();
                    }
               }
          }
     }
   #undef ITEM_ADD
   #undef FIND_IN_PART_DESCRIPTION
end:
   eina_iterator_free(it);

   return result;
}

EAPI Eina_Bool
edje_edit_image_set_del(Evas_Object *obj, const char *name)
{
   Edje_Image_Directory_Set *de = NULL, *de_last = NULL;
   Edje_Image_Directory_Set_Entry *dim = NULL;
   Eet_File *eetf;
   unsigned int i, j, k;
   Eina_List *used;
   Eina_Iterator *it;
   Edje_Part_Collection_Directory_Entry *pce;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file) return EINA_FALSE;
   if (!ed->file->image_dir) return EINA_FALSE;
   /* check if used */
   used = edje_edit_set_usage_list_get(obj, name, EINA_TRUE);
   if (used)
     {
        edje_edit_image_usage_list_free(used);
        WRN("Set \"%s\" is used", name);
        return EINA_FALSE;
     }
   edje_edit_image_usage_list_free(used);

   /* find set */
   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        if ((de->name) && (!strcmp(name, de->name)))
          break;
     }
   if (i == ed->file->image_dir->sets_count) return EINA_FALSE;
   de_last = ed->file->image_dir->sets + ed->file->image_dir->sets_count - 1;

   /* clear all links to images */
   EINA_LIST_FREE(de->entries, dim)
      free(dim);

   --ed->file->image_dir->sets_count;

   /* open the eet file */
   eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     return EINA_FALSE;

   if (de_last->id != de->id)
     {
        Edje_Part *part;
        Edje_Part_Description_Image *part_desc_image;

        de->name = de_last->name;
        it = eina_hash_iterator_data_new(ed->file->collection);
        EINA_ITERATOR_FOREACH(it, pce)
          {
             if (!pce->ref) continue;
             for (i = 0; i < pce->ref->parts_count; i++)
               {
                  if (!pce->ref->parts) continue;
                  part = pce->ref->parts[i];
                  if (part->type == EDJE_PART_TYPE_IMAGE)
                    {
                       part_desc_image = (Edje_Part_Description_Image *)part->default_desc;
                       if ((part_desc_image->image.id == de_last->id) && (part_desc_image->image.set))
                         part_desc_image->image.id = de->id;
                       for (k = 0; k < part_desc_image->image.tweens_count; k++)
                         if ((part_desc_image->image.id == de_last->id) && (part_desc_image->image.set))
                           part_desc_image->image.id = de->id;

                       for (j = 0; j < part->other.desc_count; j++)
                         {
                            part_desc_image = (Edje_Part_Description_Image *)part->other.desc[j];
                            if ((part_desc_image->image.id == de_last->id) && (part_desc_image->image.set))
                              part_desc_image->image.id = de->id;
                            for (k = 0; k < part_desc_image->image.tweens_count; k++)
                              if ((part_desc_image->image.id == de_last->id) && (part_desc_image->image.set))
                                part_desc_image->image.id = de->id;
                         }
                    }
               }
             if (!_edje_edit_collection_save(eetf, pce->ref))
               {
                  _edje_edit_eet_close(eetf);
                  return EINA_FALSE;
               }
          }
        eina_iterator_free(it);
     }
   ed->file->image_dir->sets = realloc(ed->file->image_dir->sets,
                                          sizeof(Edje_Image_Directory_Set_Entry) *
                                          ed->file->image_dir->sets_count);

   _edje_edit_eet_close(eetf);

   return EINA_TRUE;
}

EAPI Eina_List *
edje_edit_image_set_images_list_get(Evas_Object *obj, const char *name)
{
   Eina_List *images = NULL, *l;
   Edje_Image_Directory_Set *de = NULL;
   Edje_Image_Directory_Set_Entry *dim = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file) return NULL;
   if (!ed->file->image_dir) return NULL;
   if (!name) return NULL;

   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        if ((de->name) && (!strcmp(name, de->name)))
          break;
     }
   if (i == ed->file->image_dir->sets_count) return NULL;

   EINA_LIST_FOREACH(de->entries, l, dim)
     {
        images = eina_list_append(images, eina_stringshare_add(dim->name));
     }

   return images;
}

EAPI Eina_Bool
edje_edit_image_set_image_add(Evas_Object *obj, const char *set_name, const char *name)
{
   Edje_Image_Directory_Set *de = NULL;
   Edje_Image_Directory_Set_Entry *dim = NULL;
   unsigned int i;
   int id;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file) return EINA_FALSE;
   if (!ed->file->image_dir) return EINA_FALSE;
   if (!name) return EINA_FALSE;
   id = edje_edit_image_id_get(obj, name);
   if (id < 0) return EINA_FALSE;

   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        if ((de->name) && (!strcmp(set_name, de->name)))
          break;
     }
   if (i == ed->file->image_dir->sets_count) return EINA_FALSE;

   dim = (Edje_Image_Directory_Set_Entry *)calloc(1, sizeof(Edje_Image_Directory_Set_Entry));
   dim->name = name;
   dim->id = id;
   de->entries = eina_list_append(de->entries, dim);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_image_set_image_del(Evas_Object *obj, const char *set_name, unsigned int place)
{
   Edje_Image_Directory_Set *de = NULL;
   Edje_Image_Directory_Set_Entry *dim = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file) return EINA_FALSE;
   if (!ed->file->image_dir) return EINA_FALSE;

   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        if ((de->name) && (!strcmp(set_name, de->name)))
          break;
     }
   if (i == ed->file->image_dir->sets_count) return EINA_FALSE;

   dim = eina_list_nth(de->entries, place);
   if (!dim) return EINA_FALSE;

   de->entries = eina_list_remove_list(de->entries, eina_list_nth_list(de->entries, place));
   free(dim);

   return EINA_TRUE;
}

#define FUNC_IMAGE_SET_API_SIZE(Value) \
 EAPI Eina_Bool \
 edje_edit_image_set_image_##Value##_get(Evas_Object *obj, const char *set_name, unsigned int place, int *w, int *h) \
 { \
    Edje_Image_Directory_Set *de = NULL; \
    Edje_Image_Directory_Set_Entry *dim = NULL; \
    unsigned int i; \
    GET_ED_OR_RETURN(EINA_FALSE); \
    if (!ed->file) return EINA_FALSE; \
    if (!ed->file->image_dir) return EINA_FALSE; \
    for (i = 0; i < ed->file->image_dir->sets_count; ++i) \
      { \
         de = ed->file->image_dir->sets + i; \
         if ((de->name) && (!strcmp(set_name, de->name))) \
           break; \
      } \
    if (i == ed->file->image_dir->sets_count) return EINA_FALSE; \
    dim = eina_list_nth(de->entries, place); \
    if (!dim) return EINA_FALSE; \
    if (w) *w = dim->size.Value.w; \
    if (h) *h = dim->size.Value.h; \
    return EINA_TRUE; \
 } \
 EAPI Eina_Bool \
 edje_edit_image_set_image_##Value##_set(Evas_Object *obj, const char *set_name, unsigned int place, int w, int h) \
 { \
    Edje_Image_Directory_Set *de = NULL; \
    Edje_Image_Directory_Set_Entry *dim = NULL; \
    unsigned int i; \
    GET_ED_OR_RETURN(EINA_FALSE); \
    if (!ed->file) return EINA_FALSE; \
    if (!ed->file->image_dir) return EINA_FALSE; \
    for (i = 0; i < ed->file->image_dir->sets_count; ++i) \
      { \
         de = ed->file->image_dir->sets + i; \
         if ((de->name) && (!strcmp(set_name, de->name))) \
           break; \
      } \
    if (i == ed->file->image_dir->sets_count) return EINA_FALSE; \
    dim = eina_list_nth(de->entries, place); \
    if (!dim) return EINA_FALSE; \
    dim->size.Value.w = w; \
    dim->size.Value.h = h; \
    return EINA_TRUE; \
 }

FUNC_IMAGE_SET_API_SIZE(min);
FUNC_IMAGE_SET_API_SIZE(max);

EAPI Eina_Bool
edje_edit_image_set_image_border_get(Evas_Object *obj, const char *set_name, unsigned int place, int *l, int *r, int *t, int *b)
{
   Edje_Image_Directory_Set *de = NULL;
   Edje_Image_Directory_Set_Entry *dim = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file) return EINA_FALSE;
   if (!ed->file->image_dir) return EINA_FALSE;

   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        if ((de->name) && (!strcmp(set_name, de->name)))
          break;
     }
   if (i == ed->file->image_dir->sets_count) return EINA_FALSE;

   dim = eina_list_nth(de->entries, place);
   if (!dim) return EINA_FALSE;

   if (l) *l = dim->border.l;
   if (r) *r = dim->border.r;
   if (t) *t = dim->border.t;
   if (b) *b = dim->border.b;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_image_set_image_border_set(Evas_Object *obj, const char *set_name, unsigned int place, int l, int r, int t, int b)
{
   Edje_Image_Directory_Set *de = NULL;
   Edje_Image_Directory_Set_Entry *dim = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file) return EINA_FALSE;
   if (!ed->file->image_dir) return EINA_FALSE;

   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        if ((de->name) && (!strcmp(set_name, de->name)))
          break;
     }
   if (i == ed->file->image_dir->sets_count) return EINA_FALSE;

   dim = eina_list_nth(de->entries, place);
   if (!dim) return EINA_FALSE;

   if (l >= 0) dim->border.l = l;
   if (r >= 0) dim->border.r = r;
   if (t >= 0) dim->border.t = t;
   if (b >= 0) dim->border.b = b;

   return EINA_TRUE;
}

EAPI double
edje_edit_image_set_image_border_scale_get(Evas_Object *obj, const char *set_name, unsigned int place)
{
   Edje_Image_Directory_Set *de = NULL;
   Edje_Image_Directory_Set_Entry *dim = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(-1);

   if (!ed->file) return -1;
   if (!ed->file->image_dir) return -1;

   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        if ((de->name) && (!strcmp(set_name, de->name)))
          break;
     }
   if (i == ed->file->image_dir->sets_count) return -1;

   dim = eina_list_nth(de->entries, place);
   if (!dim) return -1;

   return dim->border.scale_by;
}

EAPI Eina_Bool
edje_edit_image_set_image_border_scale_set(Evas_Object *obj, const char *set_name, unsigned int place, double scale_by)
{
   Edje_Image_Directory_Set *de = NULL;
   Edje_Image_Directory_Set_Entry *dim = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file) return EINA_FALSE;
   if (!ed->file->image_dir) return EINA_FALSE;

   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        if ((de->name) && (!strcmp(set_name, de->name)))
          break;
     }
   if (i == ed->file->image_dir->sets_count) return EINA_FALSE;

   dim = eina_list_nth(de->entries, place);
   if (!dim) return EINA_FALSE;

   if (scale_by >= 0) dim->border.scale_by = scale_by;

   return EINA_TRUE;
}

/****************/
/*  VECTOR API  */
/****************/

EAPI Eina_List *
edje_edit_vectors_list_get(Evas_Object *obj)
{
   Eina_List *vectors = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file) return NULL;
   if (!ed->file->image_dir) return NULL;

   for (i = 0; i < ed->file->image_dir->vectors_count; ++i)
     vectors = eina_list_append(vectors,
                               eina_stringshare_add(ed->file->image_dir->vectors[i].entry));

   return vectors;
}

EAPI int
edje_edit_vector_id_get(Evas_Object *obj, const char *vector_name)
{
   GET_EED_OR_RETURN(-1);

   return _edje_vector_id_find(eed, vector_name);
}

EAPI Eina_List *
edje_edit_vector_usage_list_get(Evas_Object *obj, const char *name, Eina_Bool first_only)
{
   Eina_List *result = NULL;
   Eina_Iterator *it;
   Edje_Part_Collection_Directory_Entry *pce;
   Edje_Part_Image_Use *item;
   Edje_Part *part;
   Edje_Part_Description_Vector *part_desc_vector;
   unsigned int i, j;
   int vector_id;

   GET_ED_OR_RETURN(NULL);

   vector_id = edje_edit_vector_id_get(obj, name);
   if (vector_id < 0)
     return NULL;

   it = eina_hash_iterator_data_new(ed->file->collection);

#define ITEM_ADD()                                                             \
  item = (Edje_Part_Image_Use *)calloc(1, sizeof(Edje_Part_Image_Use));        \
  item->group = eina_stringshare_add(pce->entry);                              \
  item->part = eina_stringshare_add(part->name);                               \
  item->state.name = eina_stringshare_add(part_desc_vector->common.state.name);\
  item->state.value = part_desc_vector->common.state.value;                    \
  result = eina_list_append(result, item);

#define FIND_IN_PART_DESCRIPTION()                           \
  if (part_desc_vector->vg.id == vector_id)                  \
    {                                                        \
       ITEM_ADD();                                           \
       if (first_only)                                       \
         goto end;                                           \
       else                                                  \
         continue;                                           \
    }

   EINA_ITERATOR_FOREACH(it, pce)
     {
        if (!pce->ref) continue;
        for (i = 0; i < pce->ref->parts_count; i++)
          {
             if (!pce->ref->parts) continue;
             part = pce->ref->parts[i];
             if (part->type == EDJE_PART_TYPE_VECTOR)
               {
                  part_desc_vector = (Edje_Part_Description_Vector *)part->default_desc;
                  FIND_IN_PART_DESCRIPTION();
                  for (j = 0; j < part->other.desc_count; j++)
                    {
                       part_desc_vector = (Edje_Part_Description_Vector *)part->other.desc[j];
                       FIND_IN_PART_DESCRIPTION();
                    }
               }
          }
     }

   #undef ITEM_ADD
   #undef FIND_IN_PART_DESCRIPTION
end:
   eina_iterator_free(it);

   return result;
}

EAPI Eina_Bool
edje_edit_vector_del(Evas_Object *obj, const char *name)
{
   Edje_Vector_Directory_Entry *de, *de_last;
   unsigned int i, j;
   Eina_List *used;
   Eina_Iterator *it;
   Edje_Part_Collection_Directory_Entry *pce;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name) return EINA_FALSE;
   if (!ed->file) return EINA_FALSE;
   if (!ed->path) return EINA_FALSE;

   used = edje_edit_vector_usage_list_get(obj, name, EINA_TRUE);
   if (used)
     {
        edje_edit_image_usage_list_free(used);
        WRN("Vector \"%s\" is used", name);
        return EINA_FALSE;
     }
   edje_edit_image_usage_list_free(used);

   /* Create Image_Directory if not exist */
   if (!ed->file->image_dir)
     goto invalid_image;

   for (i = 0; i < ed->file->image_dir->vectors_count; ++i)
     {
        de = ed->file->image_dir->vectors + i;

        if ((de->entry) && (!strcmp(name, de->entry)))
          break;
     }
   if (i == ed->file->image_dir->vectors_count)
     goto invalid_image;

   de_last = ed->file->image_dir->vectors + ed->file->image_dir->vectors_count - 1;

   {
      char entry[PATH_MAX];
      char last_entry[PATH_MAX];
      Eet_File *eetf;
      void *data;
      int size = 0;

      /* open the eet file */
      eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
      if (!eetf)
        return EINA_FALSE;

      snprintf(entry, sizeof(entry), "edje/vectors/%i", de->id);

      if (eet_delete(eetf, entry) <= 0)
        {
           ERR("Unable to delete \"%s\" vector entry", entry);
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }
      if (de_last->id != de->id)
        {
           snprintf(last_entry, sizeof(last_entry), "edje/vectors/%i", de_last->id);
           data = eet_read(eetf, last_entry, &size);
           eet_delete(eetf, last_entry);
           eet_write(eetf, entry, data, size, 0);
        }

      _edje_if_string_free(ed, &de->entry);
      --ed->file->image_dir->vectors_count;

      if (de_last->id != de->id)
        {
           Edje_Part *part;
           Edje_Part_Description_Vector *part_desc_vector;

           de->entry = de_last->entry;
           it = eina_hash_iterator_data_new(ed->file->collection);
           EINA_ITERATOR_FOREACH(it, pce)
             {
                if (!pce->ref) continue;
                for (i = 0; i < pce->ref->parts_count; i++)
                  {
                     if (!pce->ref->parts) continue;
                     part = pce->ref->parts[i];
                     if (part->type == EDJE_PART_TYPE_VECTOR)
                       {
                          part_desc_vector = (Edje_Part_Description_Vector *)part->default_desc;
                          if (part_desc_vector->vg.id == de_last->id)
                            part_desc_vector->vg.id = de->id;

                          for (j = 0; j < part->other.desc_count; j++)
                            {
                               part_desc_vector = (Edje_Part_Description_Vector *)part->other.desc[j];
                               if (part_desc_vector->vg.id == de_last->id)
                                 part_desc_vector->vg.id = de->id;
                            }
                       }
                  }
                if (!_edje_edit_collection_save(eetf, pce->ref))
                  {
                     _edje_edit_eet_close(eetf);
                     return EINA_FALSE;
                  }
             }
           eina_iterator_free(it);
        }
      ed->file->image_dir->vectors = realloc(ed->file->image_dir->vectors,
                                             sizeof(Edje_Vector_Directory_Entry) *
                                             ed->file->image_dir->vectors_count);
      /* write the edje_file */
      if (!_edje_edit_edje_file_save(eetf, ed->file))
        {
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }

      _edje_edit_eet_close(eetf);
   }
   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;

invalid_image:
   WRN("Unable to find vector entry part \"%s\"", name);
   return EINA_FALSE;
}

/****************/
/*  IMAGES API  */
/****************/

EAPI Eina_List *
edje_edit_images_list_get(Evas_Object *obj)
{
   Eina_List *images = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file) return NULL;
   if (!ed->file->image_dir) return NULL;

   //printf("GET IMAGES LIST for %s\n", ed->file->path);
   for (i = 0; i < ed->file->image_dir->entries_count; ++i)
     images = eina_list_append(images,
                               eina_stringshare_add(ed->file->image_dir->entries[i].entry));

   return images;
}

EAPI Eina_Bool
edje_edit_image_add(Evas_Object *obj, const char *path)
{
   Edje_Image_Directory_Entry *de;
   unsigned int i;
   int free_id = -1;
   const char *name;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!path) return EINA_FALSE;
   if (!ed->file) return EINA_FALSE;
   if (!ed->path) return EINA_FALSE;

   /* Create Image_Directory if not exist */
   if (!ed->file->image_dir)
     {
        ed->file->image_dir = _alloc(sizeof(Edje_Image_Directory));
        if (!ed->file->image_dir) return EINA_FALSE;
     }

   /* Image name */
   name = ecore_file_file_get(path);

   /* Loop trough image directory to find if image exist */
   for (i = 0; i < ed->file->image_dir->entries_count; ++i)
     {
        de = ed->file->image_dir->entries + i;

        if (!de->entry)
          free_id = i;
        else if (!strcmp(name, de->entry))
          return EINA_FALSE;
     }

   if (free_id == -1)
     {
        Edje_Image_Directory_Entry *tmp;
        unsigned int count;

        count = ed->file->image_dir->entries_count + 1;

        tmp = realloc(ed->file->image_dir->entries,
                      sizeof (Edje_Image_Directory_Entry) * count);
        if (!tmp) return EINA_FALSE;

        ed->file->image_dir->entries = tmp;
        free_id = ed->file->image_dir->entries_count;
        ed->file->image_dir->entries_count = count;
     }

   /* Set Image Entry */
   de = ed->file->image_dir->entries + free_id;
   de->entry = eina_stringshare_add(name);
   de->id = free_id;
   de->source_type = 1;
   de->source_param = 1;

   /* Import image */
   if (!_edje_import_image_file(ed, path, free_id))
     {
        eina_stringshare_del(de->entry);
        de->entry = NULL;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_image_replace(Evas_Object *obj, const char *name, const char *new_name)
{
   Eina_Iterator *it;
   Edje_Part_Collection_Directory_Entry *pce;
   Edje_Part *part;
   Eet_File *eetf;
   Edje_Part_Description_Image *part_desc_image;
   unsigned int i, j, k;
   int image_id, new_image_id;

   GET_ED_OR_RETURN(EINA_FALSE);

   image_id = edje_edit_image_id_get(obj, name);
   new_image_id = edje_edit_image_id_get(obj, new_name);
   if ((image_id < 0) || (new_image_id < 0))
     return EINA_FALSE;

   it = eina_hash_iterator_data_new(ed->file->collection);

   /* open the eet file */
   eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     {
        eina_iterator_free(it);
        return EINA_FALSE;
     }

   EINA_ITERATOR_FOREACH(it, pce)
     {
        for (i = 0; i < pce->ref->parts_count; i++)
          {
             part = pce->ref->parts[i];
             if (part->type == EDJE_PART_TYPE_IMAGE)
               {
                  part_desc_image = (Edje_Part_Description_Image *)part->default_desc;
                  if (part_desc_image->image.id == image_id)
                    part_desc_image->image.id = new_image_id;
                  for (k = 0; k < part_desc_image->image.tweens_count; k++)
                    if (part_desc_image->image.tweens[k]->id == image_id)
                      part_desc_image->image.id = new_image_id;
                  for (j = 0; j < part->other.desc_count; j++)
                    {
                       part_desc_image = (Edje_Part_Description_Image *)part->other.desc[j];
                       if (part_desc_image->image.id == image_id)
                         part_desc_image->image.id = new_image_id;
                       for (k = 0; k < part_desc_image->image.tweens_count; k++)
                         if (part_desc_image->image.tweens[k]->id == image_id)
                           part_desc_image->image.id = new_image_id;
                    }
               }
             if (!_edje_edit_collection_save(eetf, pce->ref))
               {
                  _edje_edit_eet_close(eetf);
                  eina_iterator_free(it);
                  return EINA_FALSE;
               }
          }
     }
   eina_iterator_free(it);

   _edje_edit_eet_close(eetf);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_image_rename(Evas_Object *obj, const char *name, const char *new_name)
{
   Edje_Image_Directory_Entry *de = NULL;
   unsigned int i;
   GET_ED_OR_RETURN(EINA_FALSE);

   // Check if image with 'new_name' already exists
   if (edje_edit_image_id_get(obj, new_name) >= 0)
     return EINA_FALSE;

   for (i = 0; i < ed->file->image_dir->entries_count; ++i)
     {
        de = ed->file->image_dir->entries + i;
        if ((de->entry) && (!strcmp(name, de->entry)))
          break;
     }
   if (i == ed->file->image_dir->entries_count) return EINA_FALSE;

   _edje_if_string_replace(ed, &de->entry, new_name);

   return EINA_TRUE;
}

EAPI Eina_List *
edje_edit_image_usage_list_get(Evas_Object *obj, const char *name, Eina_Bool first_only)
{
   Eina_List *result = NULL, *l;
   Eina_Iterator *it;
   Edje_Part_Collection_Directory_Entry *pce;
   Edje_Part_Image_Use *item;
   Edje_Part *part;
   Edje_Part_Description_Image *part_desc_image;
   Edje_Image_Directory_Set *de = NULL;
   Edje_Image_Directory_Set_Entry *dim = NULL;
   unsigned int i, j, k;
   int image_id;

   GET_ED_OR_RETURN(NULL);

   image_id = edje_edit_image_id_get(obj, name);
   if (image_id < 0)
     return NULL;

   if (edje_edit_image_compression_type_get(obj, name) ==
       EDJE_EDIT_IMAGE_COMP_USER)
     image_id = -1 - image_id;

   it = eina_hash_iterator_data_new(ed->file->collection);

#define ITEM_ADD()                                                             \
  item = (Edje_Part_Image_Use *)calloc(1, sizeof(Edje_Part_Image_Use));        \
  item->group = eina_stringshare_add(pce->entry);                              \
  item->part = eina_stringshare_add(part->name);                               \
  item->state.name = eina_stringshare_add(part_desc_image->common.state.name); \
  item->state.value = part_desc_image->common.state.value;                     \
  result = eina_list_append(result, item);

#define ITEM_SET_ADD()                                                  \
  item = (Edje_Part_Image_Use *)calloc(1, sizeof(Edje_Part_Image_Use)); \
  item->group = eina_stringshare_add(de->name);                         \
  item->part = NULL;                                                    \
  item->state.name = NULL;                                              \
  item->state.value = -1;                                               \
  result = eina_list_append(result, item);

#define FIND_IN_PART_DESCRIPTION()                           \
  if ((part_desc_image->image.id == image_id) &&             \
      (!part_desc_image->image.set))                         \
    {                                                        \
       ITEM_ADD();                                           \
       if (first_only)                                       \
         goto end;                                           \
       else                                                  \
         continue;                                           \
    }                                                        \
  for (k = 0; k < part_desc_image->image.tweens_count; k++)  \
    {                                                        \
       if ((part_desc_image->image.tweens[k]->id == image_id)\
           && (!part_desc_image->image.tweens[k]->set))      \
         {                                                   \
            ITEM_ADD();                                      \
            if (first_only)                                  \
              goto end;                                      \
            else                                             \
              continue;                                      \
         }                                                   \
    }

   EINA_ITERATOR_FOREACH(it, pce)
     {
        if (!pce->ref) continue;
        for (i = 0; i < pce->ref->parts_count; i++)
          {
             if (!pce->ref->parts) continue;
             part = pce->ref->parts[i];
             if (part->type == EDJE_PART_TYPE_IMAGE)
               {
                  part_desc_image = (Edje_Part_Description_Image *)part->default_desc;
                  FIND_IN_PART_DESCRIPTION();
                  for (j = 0; j < part->other.desc_count; j++)
                    {
                       part_desc_image = (Edje_Part_Description_Image *)part->other.desc[j];
                       FIND_IN_PART_DESCRIPTION();
                    }
               }
          }
     }

   /* NOW CHECKING IF IMAGE IS USED INSIDE OF SET */
   for (i = 0; i < ed->file->image_dir->sets_count; ++i)
     {
        de = ed->file->image_dir->sets + i;
        EINA_LIST_FOREACH(de->entries, l, dim)
          {
             if (dim->id == image_id)
               {
                  ITEM_SET_ADD()
               }
          }
     }

   #undef ITEM_ADD
   #undef FIND_IN_PART_DESCRIPTION
end:
   eina_iterator_free(it);

   return result;
}

EAPI void
edje_edit_image_usage_list_free(Eina_List *list)
{
   Edje_Part_Image_Use *item;
   EINA_LIST_FREE(list, item)
     {
        eina_stringshare_del(item->group);
        eina_stringshare_del(item->part);
        eina_stringshare_del(item->state.name);
        free(item);
     }
}

EAPI Eina_Bool
edje_edit_image_del(Evas_Object *obj, const char *name)
{
   Edje_Image_Directory_Entry *de, *de_last;
   unsigned int i, j, k;
   Eina_List *used;
   Eina_Iterator *it;
   Edje_Part_Collection_Directory_Entry *pce;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name) return EINA_FALSE;
   if (!ed->file) return EINA_FALSE;
   if (!ed->path) return EINA_FALSE;

   used = edje_edit_image_usage_list_get(obj, name, EINA_TRUE);
   if (used)
     {
        edje_edit_image_usage_list_free(used);
        WRN("Image \"%s\" is used", name);
        return EINA_FALSE;
     }
   edje_edit_image_usage_list_free(used);

   /* Create Image_Directory if not exist */
   if (!ed->file->image_dir)
     goto invalid_image;

   for (i = 0; i < ed->file->image_dir->entries_count; ++i)
     {
        de = ed->file->image_dir->entries + i;

        if ((de->entry) && (!strcmp(name, de->entry)))
          break;
     }
   if (i == ed->file->image_dir->entries_count)
     goto invalid_image;

   de_last = ed->file->image_dir->entries + ed->file->image_dir->entries_count - 1;

   {
      char entry[PATH_MAX];
      char last_entry[PATH_MAX];
      Eet_File *eetf;
      void *data;
      int size = 0;

      /* open the eet file */
      eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
      if (!eetf)
        return EINA_FALSE;

      snprintf(entry, sizeof(entry), "edje/images/%i", de->id);

      if (eet_delete(eetf, entry) <= 0)
        {
           ERR("Unable to delete \"%s\" font entry", entry);
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }
      if (de_last->id != de->id)
        {
           snprintf(last_entry, sizeof(last_entry), "edje/images/%i", de_last->id);
           data = eet_read(eetf, last_entry, &size);
           eet_delete(eetf, last_entry);
           eet_write(eetf, entry, data, size, 0);
        }

      _edje_if_string_free(ed, &de->entry);
      --ed->file->image_dir->entries_count;

      if (de_last->id != de->id)
        {
           Edje_Part *part;
           Edje_Part_Description_Image *part_desc_image;

           de->entry = de_last->entry;
           it = eina_hash_iterator_data_new(ed->file->collection);
           EINA_ITERATOR_FOREACH(it, pce)
             {
                if (!pce->ref) continue;
                for (i = 0; i < pce->ref->parts_count; i++)
                  {
                     if (!pce->ref->parts) continue;
                     part = pce->ref->parts[i];
                     if (part->type == EDJE_PART_TYPE_IMAGE)
                       {
                          part_desc_image = (Edje_Part_Description_Image *)part->default_desc;
                          if (part_desc_image->image.id == de_last->id)
                            part_desc_image->image.id = de->id;
                          for (k = 0; k < part_desc_image->image.tweens_count; k++)
                            if (part_desc_image->image.id == de_last->id)
                              part_desc_image->image.id = de->id;

                          for (j = 0; j < part->other.desc_count; j++)
                            {
                               part_desc_image = (Edje_Part_Description_Image *)part->other.desc[j];
                               if (part_desc_image->image.id == de_last->id)
                                 part_desc_image->image.id = de->id;
                               for (k = 0; k < part_desc_image->image.tweens_count; k++)
                                 if (part_desc_image->image.id == de_last->id)
                                   part_desc_image->image.id = de->id;
                            }
                       }
                  }
                if (!_edje_edit_collection_save(eetf, pce->ref))
                  {
                     _edje_edit_eet_close(eetf);
                     return EINA_FALSE;
                  }
             }
           eina_iterator_free(it);
        }
      ed->file->image_dir->entries = realloc(ed->file->image_dir->entries,
                                             sizeof(Edje_Image_Directory_Entry) *
                                             ed->file->image_dir->entries_count);
      /* write the edje_file */
      if (!_edje_edit_edje_file_save(eetf, ed->file))
        {
           _edje_edit_eet_close(eetf);
           return EINA_FALSE;
        }

      _edje_edit_eet_close(eetf);
   }
   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;

invalid_image:
   WRN("Unable to find image entry part \"%s\"", name);
   return EINA_FALSE;
}

EAPI Eina_Bool
edje_edit_image_data_add(Evas_Object *obj, const char *name, int id)
{
   Edje_Image_Directory_Entry *de;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!name) return EINA_FALSE;
   if (!ed->file) return EINA_FALSE;
   if (!ed->path) return EINA_FALSE;

   /* Create Image_Directory if not exist */
   if (!ed->file->image_dir)
     {
        ed->file->image_dir = _alloc(sizeof(Edje_Image_Directory));
        if (!ed->file->image_dir) return EINA_FALSE;
     }

   /* Loop trough image directory to find if image exist */
   if (id < 0) id = -id - 1;
   if ((unsigned int)id >= ed->file->image_dir->entries_count) return EINA_FALSE;

   de = ed->file->image_dir->entries + id;
   _edje_if_string_replace(ed, &de->entry, name);
   de->source_type = 1;
   de->source_param = 1;

   return EINA_TRUE;
}

EAPI int
edje_edit_image_id_get(Evas_Object *obj, const char *image_name)
{
   GET_EED_OR_RETURN(-1);

   return _edje_image_id_find(eed, image_name);
}

EAPI Edje_Edit_Image_Comp
edje_edit_image_compression_type_get(Evas_Object *obj, const char *image)
{
   Edje_Image_Directory_Entry *de = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(-1);

   if (!ed->file) return -1;
   if (!ed->file->image_dir) return -1;

   for (i = 0; i < ed->file->image_dir->entries_count; ++i)
     {
        de = ed->file->image_dir->entries + i;

        if (de->entry
            && !strcmp(image, de->entry))
          break;
     }

   if (i == ed->file->image_dir->entries_count) return -1;

   switch (de->source_type)
     {
      case EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT:
        if (de->source_param == 0) // RAW
          return EDJE_EDIT_IMAGE_COMP_RAW;
        else // COMP
          return EDJE_EDIT_IMAGE_COMP_COMP;

      case EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY: // LOSSY
        return EDJE_EDIT_IMAGE_COMP_LOSSY;

      case EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY_ETC1: // LOSSY_ETC1
        return EDJE_EDIT_IMAGE_COMP_LOSSY_ETC1;

      case EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY_ETC2: // LOSSY_ETC2
        return EDJE_EDIT_IMAGE_COMP_LOSSY_ETC2;

      case EDJE_IMAGE_SOURCE_TYPE_USER: // USER
        return EDJE_EDIT_IMAGE_COMP_USER;
     }

   return -1;
}

EAPI Eina_Bool
edje_edit_image_compression_type_set(Evas_Object *obj, const char *image, Edje_Edit_Image_Comp ic)
{
   Edje_Image_Directory_Entry *de = NULL;
   unsigned int i;

   GET_ED_OR_RETURN(EINA_FALSE);

   if (!ed->file) return EINA_FALSE;
   if (!ed->file->image_dir) return EINA_FALSE;

   for (i = 0; i < ed->file->image_dir->entries_count; ++i)
     {
        de = ed->file->image_dir->entries + i;
        if ((de->entry) && (!strcmp(image, de->entry)))
          break;
     }

   if (i == ed->file->image_dir->entries_count) return EINA_FALSE;

   switch (ic)
     {
      case EDJE_EDIT_IMAGE_COMP_RAW: // RAW
        if (de->source_param == 0)
          {
             de->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT;
             return EINA_TRUE;
          }
        else
          return EINA_FALSE;

      case EDJE_EDIT_IMAGE_COMP_COMP: // COMP
        if (de->source_param == 1)
          {
             de->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_PERFECT;
             return EINA_TRUE;
          }
        else
          return EINA_FALSE;

      case EDJE_EDIT_IMAGE_COMP_LOSSY: // LOSSY
      {
         de->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY;
         return EINA_TRUE;
      }

      case EDJE_EDIT_IMAGE_COMP_LOSSY_ETC1: // LOSSY_ETC1
      {
         de->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY_ETC1;
         return EINA_TRUE;
      }

      case EDJE_EDIT_IMAGE_COMP_LOSSY_ETC2: // LOSSY_ETC2
      {
         de->source_type = EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY_ETC2;
         return EINA_TRUE;
      }

      case EDJE_EDIT_IMAGE_COMP_USER: // USER
      {
         de->source_type = EDJE_IMAGE_SOURCE_TYPE_USER;
         return EINA_TRUE;
      }
     }
   return EINA_FALSE;
}

EAPI int
edje_edit_image_compression_rate_get(Evas_Object *obj, const char *image)
{
   Edje_Image_Directory_Entry *de;
   unsigned int i;

   GET_ED_OR_RETURN(-1);

   // Gets the Image Entry
   for (i = 0; i < ed->file->image_dir->entries_count; ++i)
     {
        de = ed->file->image_dir->entries + i;
        if (de->entry
            && !strcmp(de->entry, image))
          break;
     }

   if (i == ed->file->image_dir->entries_count) return -1;
   if ((de->source_type != EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY)
       && (de->source_type != EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY_ETC1)
       && (de->source_type != EDJE_IMAGE_SOURCE_TYPE_INLINE_LOSSY_ETC2))
     return -2;

   return de->source_param;
}

EAPI const char *
edje_edit_state_image_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Image *img;
   const char *image;

   GET_PD_OR_RETURN(NULL);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return NULL;

   img = (Edje_Part_Description_Image *)pd;

   if (!img->image.set)
     image = _edje_image_name_find(eed, img->image.id);
   else
     image = _edje_set_name_find(eed, img->image.id);
   if (!image) return NULL;

   //printf("GET IMAGE for %s [%s]\n", state, image);
   return eina_stringshare_add(image);
}

EAPI Eina_Bool
edje_edit_state_image_set(Evas_Object *obj, const char *part, const char *state, double value, const char *image)
{
   Edje_Part_Description_Image *img;
   int id;
   Eina_Bool image_set = EINA_FALSE;

   if ((!obj) || (!part) || (!state) || (!image))
     return EINA_FALSE;

   eina_error_set(0);
   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return EINA_FALSE;


   img = (Edje_Part_Description_Image *)pd;

   id = _edje_image_id_find(eed, image);
   if (id <= -1)
     {
        id = _edje_set_id_find(eed, image);
        image_set = EINA_TRUE;
     }

   if (id > -1) img->image.id = id;
   else return EINA_FALSE;
   img->image.set = image_set;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI const char *
edje_edit_state_vector_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Vector *vec;
   const char *vector;

   GET_PD_OR_RETURN(NULL);

   if (rp->part->type != EDJE_PART_TYPE_VECTOR)
     return NULL;

   vec = (Edje_Part_Description_Vector *)pd;

   vector = _edje_vector_name_find(eed, vec->vg.id);
   if (!vector) return NULL;

   //printf("GET IMAGE for %s [%s]\n", state, image);
   return eina_stringshare_add(vector);
}

EAPI Eina_Bool
edje_edit_state_vector_set(Evas_Object *obj, const char *part, const char *state, double value, const char *vector)
{
   Edje_Part_Description_Vector *vec;
   int id;

   if ((!obj) || (!part) || (!state) || (!vector))
     return EINA_FALSE;

   eina_error_set(0);
   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_VECTOR)
     return EINA_FALSE;

   vec = (Edje_Part_Description_Vector *)pd;

   id = _edje_vector_id_find(eed, vector);

   if (id > -1) vec->vg.id = id;
   else return EINA_FALSE;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_List *
edje_edit_state_tweens_list_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Image *img;
   Eina_List *tweens = NULL;
   const char *name;
   unsigned int i;

   GET_PD_OR_RETURN(NULL);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return NULL;

   img = (Edje_Part_Description_Image *)pd;

   for (i = 0; i < img->image.tweens_count; ++i)
     {
        if (!img->image.tweens[i]->set)
          name = _edje_image_name_find(eed, img->image.tweens[i]->id);
        else
          name = _edje_set_name_find(eed, img->image.tweens[i]->id);
        //printf("   t: %s\n", name);
        tweens = eina_list_append(tweens, eina_stringshare_add(name));
     }

   return tweens;
}

EAPI Eina_Bool
edje_edit_state_tween_add(Evas_Object *obj, const char *part, const char *state, double value, const char *tween)
{
   Edje_Part_Description_Image *img;
   Edje_Part_Image_Id **tmp;
   Edje_Part_Image_Id *i;
   int id;
   Eina_Bool set = EINA_FALSE;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return EINA_FALSE;

   id = _edje_image_id_find(eed, tween);
   if (id < EINA_FALSE)
     {
        set = EINA_TRUE;
        id = _edje_set_id_find(eed, tween);
     }

   if (id < EINA_FALSE) return 0;

   /* alloc Edje_Part_Image_Id */
   i = _alloc(sizeof(Edje_Part_Image_Id));
   if (!i) return EINA_FALSE;
   i->id = id;
   i->set = set;

   img = (Edje_Part_Description_Image *)pd;

   /* add to tween list */
   tmp = realloc(img->image.tweens,
                 sizeof(Edje_Part_Image_Id *) * (img->image.tweens_count + 1));
   if (!tmp)
     {
        free(i);
        return EINA_FALSE;
     }

   tmp[img->image.tweens_count++] = i;
   img->image.tweens = tmp;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_tween_insert_at(Evas_Object *obj, const char *part, const char *state, double value, const char *tween, int place)
{
   Edje_Part_Description_Image *img;
   Edje_Part_Image_Id **tmp;
   Edje_Part_Image_Id *i;
   int id;
   unsigned int j;

   if (place < 0)
     return EINA_FALSE;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return EINA_FALSE;

   id = _edje_image_id_find(eed, tween);
   if (id < EINA_FALSE) return 0;

   /* alloc Edje_Part_Image_Id */
   i = _alloc(sizeof(Edje_Part_Image_Id));
   if (!i) return EINA_FALSE;
   i->id = id;

   img = (Edje_Part_Description_Image *)pd;

   if ((unsigned)place > img->image.tweens_count)
     {
        free(i);
        return EINA_FALSE;
     }

   /* add to tween list */
   tmp = realloc(img->image.tweens,
                 sizeof(Edje_Part_Image_Id *) * (img->image.tweens_count + 1));
   if (!tmp)
     {
        free(i);
        return EINA_FALSE;
     }

   img->image.tweens_count++;
   for (j = img->image.tweens_count - 1; j > (unsigned)place; j--)
     tmp[j] = tmp[j - 1];
   tmp[place] = i;
   img->image.tweens = tmp;

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_tween_del(Evas_Object *obj, const char *part, const char *state, double value, const char *tween)
{
   Edje_Part_Description_Image *img;
   unsigned int i;
   int search;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return EINA_FALSE;

   img = (Edje_Part_Description_Image *)pd;

   if (!img->image.tweens_count) return EINA_FALSE;

   search = _edje_set_id_find(eed, tween);
   if (search < 0)
     search = _edje_image_id_find(eed, tween);

   if (search < 0) return EINA_FALSE;

   for (i = 0; i < img->image.tweens_count; ++i)
     {
        if (img->image.tweens[i]->id == search)
          {
             img->image.tweens_count--;
             free(img->image.tweens[i]);
             memmove(img->image.tweens + i,
                     img->image.tweens + i + 1,
                     sizeof (Edje_Part_Description_Image *) * (img->image.tweens_count - i));
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

EAPI void
edje_edit_state_image_border_get(Evas_Object *obj, const char *part, const char *state, double value, int *l, int *r, int *t, int *b)
{
   Edje_Part_Description_Image *img;

   GET_PD_OR_RETURN();

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     {
        if (l) *l = 0;
        if (r) *r = 0;
        if (t) *t = 0;
        if (b) *b = 0;
        return;
     }

   img = (Edje_Part_Description_Image *)pd;

   //printf("GET IMAGE_BORDER of state '%s'\n", state);

   if (l) *l = img->image.border.l;
   if (r) *r = img->image.border.r;
   if (t) *t = img->image.border.t;
   if (b) *b = img->image.border.b;
}

EAPI Eina_Bool
edje_edit_state_image_border_set(Evas_Object *obj, const char *part, const char *state, double value, int l, int r, int t, int b)
{
   Edje_Part_Description_Image *img;

   if ((!obj) || (!part) || (!state))
     return EINA_FALSE;
   if ((l < -1) || (r < -1) || (t < -1) || (b < -1))
     return EINA_FALSE;

   eina_error_set(0);
   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return EINA_FALSE;

   img = (Edje_Part_Description_Image *)pd;

   img->image.border.l = l;
   img->image.border.r = r;
   img->image.border.t = t;
   img->image.border.b = b;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_state_image_border_scale_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Image *img;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return EINA_FALSE;

   img = (Edje_Part_Description_Image *)pd;

   return img->image.border.scale;
}

EAPI Eina_Bool
edje_edit_state_image_border_scale_set(Evas_Object *obj, const char *part, const char *state, double value, Eina_Bool scale)
{
   Edje_Part_Description_Image *img;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return EINA_FALSE;

   img = (Edje_Part_Description_Image *)pd;

   img->image.border.scale = scale;

   return EINA_TRUE;
}

EAPI double
edje_edit_state_image_border_scale_by_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Image *img;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return EINA_FALSE;

   img = (Edje_Part_Description_Image *)pd;

   return TO_DOUBLE(img->image.border.scale_by);
}

EAPI Eina_Bool
edje_edit_state_image_border_scale_by_set(Evas_Object *obj, const char *part, const char *state, double value, double border_scale)
{
   Edje_Part_Description_Image *img;

   GET_PD_OR_RETURN(EINA_FALSE);

   if (border_scale < 0.0)
     return EINA_FALSE;

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return EINA_FALSE;

   img = (Edje_Part_Description_Image *)pd;

   img->image.border.scale_by = FROM_DOUBLE(border_scale);

   return EINA_TRUE;
}

EAPI unsigned char
edje_edit_state_image_border_fill_get(Evas_Object *obj, const char *part, const char *state, double value)
{
   Edje_Part_Description_Image *img;

   GET_PD_OR_RETURN(0);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return 0;

   img = (Edje_Part_Description_Image *)pd;

   if (img->image.border.no_fill == 2) return 2;
   else return !img->image.border.no_fill;

   return 0;
}

EAPI Eina_Bool
edje_edit_state_image_border_fill_set(Evas_Object *obj, const char *part, const char *state, double value, unsigned char fill)
{
   Edje_Part_Description_Image *img;

   if ((!obj) || (!part) || (!state))
     return EINA_FALSE;
   if (fill > 2)
     return EINA_FALSE;

   eina_error_set(0);
   GET_PD_OR_RETURN(EINA_FALSE);

   if (rp->part->type != EDJE_PART_TYPE_IMAGE)
     return EINA_FALSE;

   img = (Edje_Part_Description_Image *)pd;

   if (fill == 2) img->image.border.no_fill = 2;
   else img->image.border.no_fill = !fill;

   edje_object_calc_force(obj);
   return EINA_TRUE;
}

/******************/
/*  PROGRAMS API  */
/******************/
static int
_edje_program_id_find(Edje_Edit *eed, const char *program)
{
   Edje_Program *epr;
   int i;

   for (i = 0; i < eed->base->collection->patterns.table_programs_size; i++)
     {
        epr = eed->base->collection->patterns.table_programs[i];
        if (epr->name && !strcmp(epr->name, program))
          return epr->id;
     }
   return -1;
}

static Edje_Program *
_edje_program_get_byname(Evas_Object *obj, const char *prog_name)
{
   Edje_Program *epr;
   int i;

   GET_ED_OR_RETURN(NULL);

   if (!prog_name) return NULL;

   for (i = 0; i < ed->collection->patterns.table_programs_size; i++)
     {
        epr = ed->collection->patterns.table_programs[i];
        if ((epr->name) && (strcmp(epr->name, prog_name) == 0))
          return epr;
     }
   return NULL;
}

EAPI Eina_List *
edje_edit_programs_list_get(Evas_Object *obj)
{
   Eina_List *progs = NULL;
   int i;

   GET_ED_OR_RETURN(NULL);

   //printf("EE: Found %d programs\n", ed->table_programs_size);

   for (i = 0; i < ed->collection->patterns.table_programs_size; i++)
     {
        Edje_Program *epr;

        epr = ed->collection->patterns.table_programs[i];
        /* XXX: bad, we miss programs this way, but since you can't access
         * them in any way without a name, better ignore them.  */
        if (!epr->name) continue;
        progs = eina_list_append(progs, eina_stringshare_add(epr->name));
     }

   return progs;
}

EAPI Eina_Bool
edje_edit_program_add(Evas_Object *obj, const char *name)
{
   Edje_Program *epr;

   GET_ED_OR_RETURN(EINA_FALSE);

   //printf("ADD PROGRAM [new name: %s]\n", name);

   //Check if program already exists
   if (_edje_program_get_byname(obj, name))
     return EINA_FALSE;

   //Alloc Edje_Program or return
   epr = _alloc(sizeof(Edje_Program));
   if (!epr) return EINA_FALSE;

   //Add program to group
   // pc = ed->collection;

   /* By default, source and signal are empty, so they fill in nocmp category */
   ed->collection->programs.nocmp = realloc(ed->collection->programs.nocmp,
                                            sizeof (Edje_Program *) * (ed->collection->programs.nocmp_count + 1));
   ed->collection->programs.nocmp[ed->collection->programs.nocmp_count++] = epr;

   //Init Edje_Program
   epr->id = ed->collection->patterns.table_programs_size;
   epr->name = eina_stringshare_add(name);
   epr->signal = NULL;
   epr->source = NULL;
   epr->filter.part = NULL;
   epr->filter.state = NULL;
   epr->in.from = 0.0;
   epr->in.range = 0.0;
   epr->action = 0;
   epr->seat = NULL;
   epr->state = NULL;
   epr->value = 0.0;
   epr->state2 = NULL;
   epr->value2 = 0.0;
   epr->tween.mode = 1;
   epr->tween.time = ZERO;
   epr->targets = NULL;
   epr->after = NULL;
   epr->sample_name = NULL;
   epr->speed = 1.0;
   epr->channel = EDJE_CHANNEL_EFFECT;
   epr->tone_name = NULL;
   epr->duration = 0.1;

   //Update table_programs
   ed->collection->patterns.table_programs_size++;
   ed->collection->patterns.table_programs = realloc(ed->collection->patterns.table_programs,
                                                     sizeof(Edje_Program *) * ed->collection->patterns.table_programs_size);
   ed->collection->patterns.table_programs[epr->id % ed->collection->patterns.table_programs_size] = epr;

   //Update patterns
   _edje_programs_patterns_clean(ed->collection);
   _edje_programs_patterns_init(ed->collection);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_del(Evas_Object *obj, const char *prog)
{
   Eina_List *l, *l_next;
   Edje_Program_Target *prt;
   Edje_Program_After *pa;
   Edje_Program *p;
   Program_Script *ps, *old_ps;
   int id, i;
   int old_id = -1;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   //pc = ed->collection;

   //Remove program from programs list
   id = epr->id;
   _edje_program_remove(ed->collection, epr);

   /* fix table program */
   if (epr->id != ed->collection->patterns.table_programs_size - 1)
     {
        /* If the removed program is not the last in the list/table,
         * put the last one in its place and update references to it later */
        ed->collection->patterns.table_programs[epr->id] = ed->collection->patterns.table_programs[ed->collection->patterns.table_programs_size - 1];
        old_id = ed->collection->patterns.table_programs_size - 1;
        ed->collection->patterns.table_programs[epr->id]->id = epr->id;
     }

   ps = eina_hash_find(eed->program_scripts, &id);
   old_ps = eina_hash_find(eed->program_scripts, &old_id);
   if (old_ps)
     {
        if (!ps)
          {
             ps = _alloc(sizeof(Program_Script));
             ps->id = id;
             eina_hash_add(eed->program_scripts, &id, ps);
          }
        else
          {
             free(ps->code);
             free(ps->processed);
             ps->processed = NULL;
             ps->delete_me = EINA_FALSE;
          }
        ps->code = old_ps->code;
        old_ps->code = NULL;
        free(old_ps->processed);
        old_ps->processed = NULL;
        ps->dirty = EINA_TRUE;
        old_ps->dirty = EINA_FALSE;
        old_ps->delete_me = EINA_TRUE;
     }
   else if (ps)
     {
        ps->dirty = EINA_FALSE;
        ps->delete_me = EINA_TRUE;
     }

   //Free Edje_Program
   _edje_if_string_free(ed, &epr->name);
   _edje_if_string_free(ed, &epr->signal);
   _edje_if_string_free(ed, &epr->source);
   _edje_if_string_free(ed, &epr->filter.part);
   _edje_if_string_free(ed, &epr->filter.state);
   _edje_if_string_free(ed, &epr->seat);
   _edje_if_string_free(ed, &epr->state);
   _edje_if_string_free(ed, &epr->state2);
   _edje_if_string_free(ed, &epr->sample_name);
   _edje_if_string_free(ed, &epr->tone_name);

   EINA_LIST_FREE(epr->targets, prt)
     free(prt);
   EINA_LIST_FREE(epr->after, pa)
     free(pa);
   free(epr);

   ed->collection->patterns.table_programs_size--;
   ed->collection->patterns.table_programs = realloc(ed->collection->patterns.table_programs,
                                                     sizeof(Edje_Program *) * ed->collection->patterns.table_programs_size);

   //We also update all other programs that point to old_id and id
   for (i = 0; i < ed->collection->patterns.table_programs_size; i++)
     {
        p = ed->collection->patterns.table_programs[i];

        /* check in afters */
        EINA_LIST_FOREACH_SAFE(p->after, l, l_next, pa)
          {
             if (pa->id == old_id)
               pa->id = id;
             else if (pa->id == id)
               {
                  p->after = eina_list_remove_list(p->after, l);
                  free(pa);
               }
          }
        /* check in targets */
        if (p->action == EDJE_ACTION_TYPE_ACTION_STOP)
          {
             Edje_Program_Target *pt;

             EINA_LIST_FOREACH_SAFE(p->targets, l, l_next, pt)
               {
                  if (pt->id == old_id)
                    pt->id = id;
                  else if (pt->id == id)
                    {
                       p->targets = eina_list_remove_list(p->targets, l);
                       free(pt);
                    }
               }
          }
     }

   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_exist(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_run(Evas_Object *obj, const char *prog)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   _edje_program_run(ed, epr, 0, "", "", NULL);
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_stop_all(Evas_Object *obj)
{
   GET_ED_OR_RETURN(EINA_FALSE);

   Eina_List *l, *ln;
   Edje_Running_Program *runp;

   EINA_LIST_FOREACH_SAFE(ed->actions, l, ln, runp)
     _edje_program_end(ed, runp);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_transition_state_set(Evas_Object *obj, const char *prog, double position)
{
   Edje_Program_Target *pt;
   Edje_Real_Part *rp;
   Eina_List *l;

   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);
   if (position < 0 || position > 1) return EINA_FALSE;
   if (epr->action != EDJE_ACTION_TYPE_STATE_SET) return EINA_FALSE;

   EINA_LIST_FOREACH(epr->targets, l, pt)
     {
        if (pt->id >= 0)
          {
             rp = ed->table_parts[pt->id % ed->table_parts_size];
             if (rp)
               {
                  _edje_part_description_apply(ed, rp,
                                               rp->param1.description->state.name,
                                               rp->param1.description->state.value,
                                               epr->state,
                                               epr->value);
                  _edje_part_pos_set(ed, rp,
                                     epr->tween.mode, position,
                                     epr->tween.v1,
                                     epr->tween.v2,
                                     epr->tween.v3,
                                     epr->tween.v4);
               }
          }
     }
   _edje_recalc(ed);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_name_set(Evas_Object *obj, const char *prog, const char *new_name)
{
   GET_EED_OR_RETURN(EINA_FALSE);
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   if (!new_name) return EINA_FALSE;

   if (!strcmp(prog, new_name)) return EINA_TRUE;

   if (_edje_program_get_byname(obj, new_name)) return EINA_FALSE;

   //printf("SET NAME for program: %s [new name: %s]\n", prog, new_name);

   _edje_if_string_replace(ed, &epr->name, new_name);

   _edje_edit_flag_script_dirty(eed, EINA_TRUE);

   return EINA_TRUE;
}

EAPI const char *
edje_edit_program_source_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->source) return NULL;
   //printf("GET SOURCE for program: %s [%s]\n", prog, epr->source);
   return eina_stringshare_add(epr->source);
}

EAPI Eina_Bool
edje_edit_program_source_set(Evas_Object *obj, const char *prog, const char *source)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   /* Remove from program array */
   _edje_program_remove(ed->collection, epr);

   /* Insert it back */
   _edje_if_string_replace(ed, &epr->source, source);
   _edje_program_insert(ed->collection, epr);

   //Update patterns
   _edje_programs_patterns_clean(ed->collection);
   _edje_programs_patterns_init(ed->collection);

   return EINA_TRUE;
}

EAPI const char *
edje_edit_program_sample_name_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->sample_name) return NULL;
   return eina_stringshare_add(epr->sample_name);
}

EAPI Eina_Bool
edje_edit_program_sample_name_set(Evas_Object *obj, const char *prog, const char *name)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   if (!name) return EINA_FALSE;

   _edje_if_string_replace(ed, &epr->sample_name, name);

   return EINA_TRUE;
}

EAPI const char *
edje_edit_program_tone_name_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->tone_name) return NULL;
   return eina_stringshare_add(epr->tone_name);
}

EAPI Eina_Bool
edje_edit_program_tone_name_set(Evas_Object *obj, const char *prog, const char *name)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   if (!name) return EINA_FALSE;

   _edje_if_string_replace(ed, &epr->tone_name, name);

   return EINA_TRUE;
}

EAPI double
edje_edit_program_sample_speed_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   return epr->speed;
}

EAPI Eina_Bool
edje_edit_program_sample_speed_set(Evas_Object *obj, const char *prog, double speed)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   if (speed < 0) return EINA_FALSE;

   epr->speed = speed;

   return EINA_TRUE;
}

EAPI double
edje_edit_program_tone_duration_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   return epr->duration;
}

EAPI Eina_Bool
edje_edit_program_tone_duration_set(Evas_Object *obj, const char *prog, double duration)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   if (duration < 0) return EINA_FALSE;

   epr->duration = duration;

   return EINA_TRUE;
}

EAPI unsigned char
edje_edit_program_channel_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(0);

   return epr->channel;
}

EAPI Eina_Bool
edje_edit_program_channel_set(Evas_Object *obj, const char *prog, Edje_Channel channel)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   epr->channel = channel;

   return EINA_TRUE;
}

EAPI const char *
edje_edit_program_filter_part_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->filter.part) return NULL;
   return eina_stringshare_add(epr->filter.part);
}

EAPI Eina_Bool
edje_edit_program_filter_part_set(Evas_Object *obj, const char *prog, const char *filter_part)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   _edje_if_string_replace(ed, &epr->filter.part, filter_part);

   return EINA_TRUE;
}

EAPI const char *
edje_edit_program_filter_state_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->filter.state) return NULL;
   return eina_stringshare_add(epr->filter.state);
}

EAPI Eina_Bool
edje_edit_program_filter_state_set(Evas_Object *obj, const char *prog, const char *filter_state)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   _edje_if_string_replace(ed, &epr->filter.state, filter_state);

   return EINA_TRUE;
}

EAPI const char *
edje_edit_program_signal_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->signal) return NULL;
   //printf("GET SIGNAL for program: %s [%s]\n", prog, epr->signal);
   return eina_stringshare_add(epr->signal);
}

EAPI Eina_Bool
edje_edit_program_signal_set(Evas_Object *obj, const char *prog, const char *sig)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   /* Remove from program array */
   _edje_program_remove(ed->collection, epr);

   /* Insert it back */
   _edje_if_string_replace(ed, &epr->signal, sig);
   _edje_program_insert(ed->collection, epr);

   //Update patterns
   _edje_programs_patterns_clean(ed->collection);
   _edje_programs_patterns_init(ed->collection);

   return EINA_TRUE;
}

EAPI const char *
edje_edit_program_state_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->state) return NULL;
   //printf("GET STATE for program: %s [%s %.2f]\n", prog, epr->state, epr->value);
   return eina_stringshare_add(epr->state);
}

EAPI Eina_Bool
edje_edit_program_state_set(Evas_Object *obj, const char *prog, const char *state)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   //printf("SET STATE for program: %s\n", prog);

   _edje_if_string_replace(ed, &epr->state, state);

   return EINA_TRUE;
}

EAPI const char *
edje_edit_program_state2_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   if (!epr->state2) return NULL;
   //printf("GET STATE2 for program: %s [%s %.2f]\n", prog, epr->state2, epr->value2);
   return eina_stringshare_add(epr->state2);
}

EAPI Eina_Bool
edje_edit_program_state2_set(Evas_Object *obj, const char *prog, const char *state2)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   //printf("SET STATE2 for program: %s\n", prog);

   _edje_if_string_replace(ed, &epr->state2, state2);

   return EINA_TRUE;
}

EAPI double
edje_edit_program_value_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   //printf("GET VALUE for program: %s [%s %.2f]\n", prog, epr->state, epr->value);
   return epr->value;
}

EAPI Eina_Bool
edje_edit_program_value_set(Evas_Object *obj, const char *prog, double value)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   //printf("SET VALUE for program: %s [%.2f]\n", prog, value);
   epr->value = value;
   return EINA_TRUE;
}

EAPI double
edje_edit_program_value2_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   //printf("GET VALUE2 for program: %s [%s %.2f]\n", prog, epr->state2, epr->value2);
   return epr->value2;
}

EAPI Eina_Bool
edje_edit_program_value2_set(Evas_Object *obj, const char *prog, double value)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   //printf("SET VALUE for program: %s [%.2f]\n", prog, value);
   epr->value2 = value;
   return EINA_TRUE;
}

EAPI double
edje_edit_program_in_from_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(0);

   //printf("GET IN.FROM for program: %s [%f]\n", prog, epr->in.from);
   return epr->in.from;
}

EAPI Eina_Bool
edje_edit_program_in_from_set(Evas_Object *obj, const char *prog, double seconds)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   //printf("SET IN.FROM for program: %s [%f]\n", prog, epr->in.from);
   epr->in.from = seconds;
   return EINA_TRUE;
}

EAPI double
edje_edit_program_in_range_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(0);

   //printf("GET IN.RANGE for program: %s [%f]\n", prog, epr->in.range);
   return epr->in.range;
}

EAPI Eina_Bool
edje_edit_program_in_range_set(Evas_Object *obj, const char *prog, double seconds)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   //printf("SET IN.RANGE for program: %s [%f]\n", prog, epr->in.range);
   epr->in.range = seconds;
   return EINA_TRUE;
}

EAPI Edje_Tween_Mode
edje_edit_program_transition_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   //printf("GET TRANSITION for program: %s [%d]\n", prog, epr->tween.mode);
   return epr->tween.mode;
}

EAPI Eina_Bool
edje_edit_program_transition_set(Evas_Object *obj, const char *prog, Edje_Tween_Mode transition)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   //printf("GET TRANSITION for program: %s [%d]\n", prog, epr->tween.mode);
   epr->tween.mode = transition;
   return EINA_TRUE;
}

#define FUNC_PROGRAM_TRANSITION_VALUE(Num)                                                         \
  EAPI double                                                                                      \
  edje_edit_program_transition_value##Num##_get(Evas_Object * obj, const char *prog)               \
  {                                                                                                \
     eina_error_set(0);                                                                            \
                                                                                                   \
     GET_EPR_OR_RETURN(-1);                                                                        \
                                                                                                   \
     return TO_DOUBLE(epr->tween.v##Num);                                                          \
  }                                                                                                \
  EAPI Eina_Bool                                                                                   \
  edje_edit_program_transition_value##Num##_set(Evas_Object * obj, const char *prog, double value) \
  {                                                                                                \
     eina_error_set(0);                                                                            \
                                                                                                   \
     GET_EPR_OR_RETURN(EINA_FALSE);                                                                \
                                                                                                   \
     epr->tween.v##Num = FROM_DOUBLE(value);                                                       \
     return EINA_TRUE;                                                                             \
  }

FUNC_PROGRAM_TRANSITION_VALUE(1)
FUNC_PROGRAM_TRANSITION_VALUE(2)
FUNC_PROGRAM_TRANSITION_VALUE(3)
FUNC_PROGRAM_TRANSITION_VALUE(4)

#undef FUNC_PROGRAM_TRANSITION_VALUE

EAPI double
edje_edit_program_transition_time_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   //printf("GET TRANSITION_TIME for program: %s [%.4f]\n", prog, epr->tween.time);
   return TO_DOUBLE(epr->tween.time);
}

EAPI Eina_Bool
edje_edit_program_transition_time_set(Evas_Object *obj, const char *prog, double seconds)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   //printf("GET TRANSITION_TIME for program: %s [%.4f]\n", prog, epr->tween.time);
   epr->tween.time = FROM_DOUBLE(seconds);
   return EINA_TRUE;
}

EAPI Edje_Action_Type
edje_edit_program_action_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(-1);

   //printf("GET ACTION for program: %s [%d]\n", prog, epr->action);
   return epr->action;
}

EAPI Eina_Bool
edje_edit_program_action_set(Evas_Object *obj, const char *prog, Edje_Action_Type action)
{
   Program_Script *ps;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   //printf("SET ACTION for program: %s [%d]\n", prog, action);
   if (action >= EDJE_ACTION_TYPE_LAST) return EINA_FALSE;

   if ((Edje_Action_Type)epr->action == action)
     return EINA_TRUE;

   if (action == EDJE_ACTION_TYPE_SCRIPT)
     {
        ps = eina_hash_find(eed->program_scripts, &epr->id);
        if (!ps)
          {
             ps = _alloc(sizeof(Program_Script));
             if (!ps)
               return EINA_FALSE;
          }
        ps->id = epr->id;
        ps->code = strdup("");
        ps->dirty = EINA_TRUE;
        ps->delete_me = EINA_FALSE;
        eina_hash_set(eed->program_scripts, &ps->id, ps);
        _edje_edit_flag_script_dirty(eed, EINA_FALSE);
     }
   if (epr->action == EDJE_ACTION_TYPE_SCRIPT)
     {
        ps = eina_hash_find(eed->program_scripts, &epr->id);
        if (ps)
          {
             free(ps->code);
             free(ps->processed);
             ps->code = ps->processed = NULL;
             ps->dirty = EINA_FALSE;
             ps->delete_me = EINA_TRUE;
             _edje_edit_flag_script_dirty(eed, EINA_FALSE);
          }
     }

   switch (action)
     {
      case EDJE_ACTION_TYPE_STATE_SET:
      case EDJE_ACTION_TYPE_SIGNAL_EMIT:
      case EDJE_ACTION_TYPE_DRAG_VAL_SET:
      case EDJE_ACTION_TYPE_DRAG_VAL_STEP:
      case EDJE_ACTION_TYPE_DRAG_VAL_PAGE:
      case EDJE_ACTION_TYPE_FOCUS_SET:
      case EDJE_ACTION_TYPE_FOCUS_OBJECT:
#ifdef HAVE_EPHYSICS
      case EDJE_ACTION_TYPE_PHYSICS_FORCES_CLEAR:
      case EDJE_ACTION_TYPE_PHYSICS_STOP:
      case EDJE_ACTION_TYPE_PHYSICS_ROT_SET:
#endif
        /*This actions have part as a target so targets list can be leaved untouched
           if it was not list of programs (EDJE_ACTION_TYPE_ACTION_STOP) */
        if (epr->action == EDJE_ACTION_TYPE_ACTION_STOP)
          edje_edit_program_targets_clear(obj, prog);
        break;

      case EDJE_ACTION_TYPE_ACTION_STOP:
      /*this action needs programs as targets*/
      default:
        /*other actions do not need targets so we need to delete them all */
        edje_edit_program_targets_clear(obj, prog);
     }

   epr->action = action;
   return EINA_TRUE;
}

static Eina_List *
_edje_program_targets_get(Evas_Object *obj, Edje_Program *epr)
{
   Eina_List *l, *targets = NULL;
   Edje_Program_Target *t;

   GET_ED_OR_RETURN(NULL);

   EINA_LIST_FOREACH(epr->targets, l, t)
     {
        switch (epr->action)
          {
           /*action types, that does not support targets*/
           case EDJE_ACTION_TYPE_SCRIPT:
           case EDJE_ACTION_TYPE_SOUND_SAMPLE:
           case EDJE_ACTION_TYPE_SOUND_TONE:
           case EDJE_ACTION_TYPE_VIBRATION_SAMPLE:
           case EDJE_ACTION_TYPE_PARAM_COPY:
           case EDJE_ACTION_TYPE_PARAM_SET:
#ifdef HAVE_EPHYSICS
           case EDJE_ACTION_TYPE_PHYSICS_IMPULSE:
           case EDJE_ACTION_TYPE_PHYSICS_TORQUE_IMPULSE:
           case EDJE_ACTION_TYPE_PHYSICS_FORCE:
           case EDJE_ACTION_TYPE_PHYSICS_TORQUE:
           case EDJE_ACTION_TYPE_PHYSICS_VEL_SET:
           case EDJE_ACTION_TYPE_PHYSICS_ANG_VEL_SET:
#endif
           break;

           /* the target is a program */
           case EDJE_ACTION_TYPE_ACTION_STOP:
             {
               Edje_Program *p = NULL;

               p = ed->collection->patterns.table_programs[t->id % ed->collection->patterns.table_programs_size];
               if (p && p->name)
                 targets = eina_list_append(targets,
                                            eina_stringshare_add(p->name));
             }
           break;

           /* the target is a part */
           case EDJE_ACTION_TYPE_SIGNAL_EMIT:
           case EDJE_ACTION_TYPE_STATE_SET:
           case EDJE_ACTION_TYPE_DRAG_VAL_SET:
           case EDJE_ACTION_TYPE_DRAG_VAL_STEP:
           case EDJE_ACTION_TYPE_DRAG_VAL_PAGE:
           case EDJE_ACTION_TYPE_FOCUS_SET:
           case EDJE_ACTION_TYPE_FOCUS_OBJECT:
#ifdef HAVE_EPHYSICS
           case EDJE_ACTION_TYPE_PHYSICS_FORCES_CLEAR:
           case EDJE_ACTION_TYPE_PHYSICS_STOP:
           case EDJE_ACTION_TYPE_PHYSICS_ROT_SET:
#endif
             {
                Edje_Real_Part *p = NULL;
                p = ed->table_parts[t->id % ed->table_parts_size];
                if (p && p->part && p->part->name)
                  targets = eina_list_append(targets,
                                             eina_stringshare_add(p->part->name));
             }
           break;
          }
     }
   return targets;
}

EAPI Eina_List *
edje_edit_program_targets_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   return _edje_program_targets_get(obj, epr);
}

EAPI Eina_Bool
edje_edit_program_targets_clear(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   while (epr->targets)
     {
        Edje_Program_Target *prt;

        prt = eina_list_data_get(epr->targets);
        epr->targets = eina_list_remove_list(epr->targets, epr->targets);
        free(prt);
     }

   return EINA_TRUE;
}

static int
_program_target_id_get(Evas_Object *obj, Edje *ed, Edje_Action_Type action, const char *target)
{
   int id = -1;
   Edje_Program *tar;
   Edje_Real_Part *rp;

   switch (action)
     {
      case EDJE_ACTION_TYPE_STATE_SET:
      case EDJE_ACTION_TYPE_SIGNAL_EMIT:
      case EDJE_ACTION_TYPE_DRAG_VAL_SET:
      case EDJE_ACTION_TYPE_DRAG_VAL_STEP:
      case EDJE_ACTION_TYPE_DRAG_VAL_PAGE:
      case EDJE_ACTION_TYPE_FOCUS_SET:
      case EDJE_ACTION_TYPE_FOCUS_OBJECT:
#ifdef HAVE_EPHYSICS
      case EDJE_ACTION_TYPE_PHYSICS_FORCES_CLEAR:
      case EDJE_ACTION_TYPE_PHYSICS_STOP:
      case EDJE_ACTION_TYPE_PHYSICS_ROT_SET:
#endif
        /* the target is a part */
        rp = _edje_real_part_get(ed, target);
        if (!rp) return -1;
        id = rp->part->id;
        break;

      case EDJE_ACTION_TYPE_ACTION_STOP:
        /* the target is a program */
        tar = _edje_program_get_byname(obj, target);
        if (!tar) return -1;
        id = tar->id;
        break;

      default:
        return -1;
     }

   return id;
}

EAPI Eina_Bool
edje_edit_program_target_add(Evas_Object *obj, const char *prog, const char *target)
{
   int id;
   Edje_Program_Target *t;

   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   id = _program_target_id_get(obj, ed, epr->action, target);
   if (id == -1) return EINA_FALSE;

   t = _alloc(sizeof(Edje_Program_Target));
   if (!t) return EINA_FALSE;

   t->id = id;
   epr->targets = eina_list_append(epr->targets, t);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_target_insert_at(Evas_Object *obj, const char *prog, const char *target, int place)
{
   int id;
   Edje_Program_Target *t;
   Eina_List *l;

   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   id = _program_target_id_get(obj, ed, epr->action, target);
   if (id == -1) return EINA_FALSE;

   t = _alloc(sizeof(Edje_Program_Target));
   if (!t) return EINA_FALSE;

   t->id = id;
   if ((unsigned)place >= eina_list_count(epr->targets))
     epr->targets = eina_list_append(epr->targets, t);
   else
     {
        l = eina_list_nth_list(epr->targets, place);
        epr->targets = eina_list_prepend_relative_list(epr->targets, t, l);
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_target_del(Evas_Object *obj, const char *prog, const char *target)
{
   int id;
   Eina_List *l;
   Edje_Program_Target *t;

   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   id = _program_target_id_get(obj, ed, epr->action, target);
   if (id == -1) return EINA_FALSE;

   EINA_LIST_FOREACH(epr->targets, l, t)
     if (t->id == id)
       break;
   epr->targets = eina_list_remove_list(epr->targets, l);
   free(t);

   return EINA_TRUE;
}

static Eina_List *
_edje_program_afters_get(Evas_Object *obj, Edje_Program *epr)
{
   Eina_List *l, *afters = NULL;
   Edje_Program_After *a;

   GET_ED_OR_RETURN(NULL);

   // printf("GET AFTERS for program: %s [count: %d]\n", prog, eina_list_count(epr->after));
   EINA_LIST_FOREACH(epr->after, l, a)
     {
        Edje_Program *p = NULL;

        p = ed->collection->patterns.table_programs[a->id % ed->collection->patterns.table_programs_size];
        if (p && p->name)
          {
             //printf("   a: %d name: %s\n", a->id, p->name);
             afters = eina_list_append(afters, eina_stringshare_add(p->name));
          }
     }
   return afters;
}

EAPI Eina_List *
edje_edit_program_afters_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   return _edje_program_afters_get(obj, epr);
}

EAPI Eina_Bool
edje_edit_program_afters_clear(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(EINA_FALSE);

   while (epr->after)
     {
        Edje_Program_After *pa;

        pa = eina_list_data_get(epr->after);
        epr->after = eina_list_remove_list(epr->after, epr->after);
        free(pa);
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_after_add(Evas_Object *obj, const char *prog, const char *after)
{
   Edje_Program *af;
   Edje_Program_After *a;

   GET_EPR_OR_RETURN(EINA_FALSE);

   af = _edje_program_get_byname(obj, after);
   if (!af) return EINA_FALSE;

   a = _alloc(sizeof(Edje_Program_After));
   if (!a) return EINA_FALSE;

   a->id = af->id;

   epr->after = eina_list_append(epr->after, a);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_after_insert_at(Evas_Object *obj, const char *prog, const char *after, int place)
{
   Edje_Program *af;
   Edje_Program_After *a;
   Eina_List *l;

   GET_EPR_OR_RETURN(EINA_FALSE);

   if (place < 0)
     return EINA_FALSE;

   af = _edje_program_get_byname(obj, after);
   if (!af) return EINA_FALSE;

   a = _alloc(sizeof(Edje_Program_After));
   if (!a) return EINA_FALSE;

   a->id = af->id;

   if ((unsigned)place >= eina_list_count(epr->after))
     epr->after = eina_list_append(epr->after, a);
   else
     {
        l = eina_list_nth_list(epr->after, place);
        epr->after = eina_list_prepend_relative_list(epr->after, a, l);
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_after_del(Evas_Object *obj, const char *prog, const char *after)
{
   Edje_Program *af;
   Edje_Program_After *a;
   Eina_List *l;

   GET_EPR_OR_RETURN(EINA_FALSE);

   af = _edje_program_get_byname(obj, after);
   if (!af) return EINA_FALSE;

   EINA_LIST_FOREACH(epr->after, l, a)
     if (a->id == af->id)
       {
          epr->after = eina_list_remove_list(epr->after, l);
          break;
       }

   return EINA_TRUE;
}

EAPI const char *
edje_edit_program_api_name_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   return eina_stringshare_add(epr->api.name);
}

EAPI const char *
edje_edit_program_api_description_get(Evas_Object *obj, const char *prog)
{
   GET_EPR_OR_RETURN(NULL);

   return eina_stringshare_add(epr->api.description);
}

EAPI Eina_Bool
edje_edit_program_api_name_set(Evas_Object *obj, const char *prog, const char *name)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   _edje_if_string_replace(ed, &epr->api.name, name);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_program_api_description_set(Evas_Object *obj, const char *prog, const char *description)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   _edje_if_string_replace(ed, &epr->api.description, description);

   return EINA_TRUE;
}

/*************************/
/*  EMBRYO SCRIPTS  API  */
/*************************/
EAPI char *
edje_edit_script_get(Evas_Object *obj)
{
   GET_EED_OR_RETURN(NULL);
   GET_ED_OR_RETURN(NULL);

   if (!ed->collection) return NULL;
   if (!eed->embryo_source) return NULL;

   return strdup(eed->embryo_source);
}

EAPI Eina_Bool
edje_edit_script_set(Evas_Object *obj, const char *code)
{
   GET_EED_OR_RETURN(EINA_FALSE);

   free(eed->embryo_source);
   free(eed->embryo_processed);

   if (code)
     eed->embryo_source = strdup(code);
   else
     eed->embryo_source = NULL;
   eed->embryo_processed = NULL;

   eed->embryo_source_dirty = EINA_TRUE;

   _edje_edit_flag_script_dirty(eed, EINA_FALSE);
   return EINA_TRUE;
}

EAPI char *
edje_edit_script_program_get(Evas_Object *obj, const char *prog)
{
   Program_Script *ps;

   GET_EED_OR_RETURN(NULL);
   GET_EPR_OR_RETURN(NULL);

   if (epr->action != EDJE_ACTION_TYPE_SCRIPT)
     return NULL;

   ps = eina_hash_find(eed->program_scripts, &epr->id);
   if (!ps) /* mmm? it should be there, even if empty */
     return NULL;

   return ps->code ? strdup(ps->code) : NULL;
}

EAPI Eina_Bool
edje_edit_script_program_set(Evas_Object *obj, const char *prog, const char *code)
{
   Program_Script *ps;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_EPR_OR_RETURN(EINA_FALSE);

   if (epr->action != EDJE_ACTION_TYPE_SCRIPT)
     return EINA_FALSE;

   ps = eina_hash_find(eed->program_scripts, &epr->id);
   if (!ps) /* ???? how so? */
     return EINA_FALSE;

   free(ps->code);
   free(ps->processed);

   if (code)
     ps->code = strdup(code);
   else
     ps->code = NULL;
   ps->processed = NULL;
   ps->dirty = EINA_TRUE;

   _edje_edit_flag_script_dirty(eed, EINA_FALSE);
   return EINA_TRUE;
}

static int
__part_replace(Edje_Edit *eed, char *pcode, char *name)
{
   int id;

   id = _edje_part_id_find(eed->base, name);
   if (id < 0)
     return 0;
   return eina_convert_itoa(id, pcode);
}

static int
__program_replace(Edje_Edit *eed, char *pcode, char *name)
{
   int id;

   id = _edje_program_id_find(eed, name);
   if (id < 0)
     return 0;
   return eina_convert_itoa(id, pcode);
}

static int
__group_replace(Edje_Edit *eed EINA_UNUSED, char *pcode, char *name)
{
   strcpy(pcode, name);
   return strlen(name) + 1;
}

static int
__image_replace(Edje_Edit *eed, char *pcode, char *name)
{
   int id;

   id = _edje_image_id_find(eed, name);
   if (id < 0)
     return 0;
   return eina_convert_itoa(id, pcode);
}

static char *
_edje_edit_script_process(Edje_Edit *eed, const char *progname, char *code)
{
   char *pcode, *psrc, *pdst;
   int codesize, pcodesize;
   int quoted = 0, escaped = 0;
   int line = 1;
   Eina_Bool success = EINA_TRUE;

   codesize = strlen(code);
   pcode = malloc(codesize + 1);
   if (!pcode)
     return NULL;

   pcodesize = 0;
   psrc = code;
   pdst = pcode;
   while (*psrc)
     {
        if (!quoted)
          {
             char *ptr = NULL;
             const char *what = NULL;
             int (*func)(Edje_Edit *, char *, char *);

             if (*psrc == 'P')
               {
                  if (!strncmp(psrc, "PART:\"", 6))
                    {
                       psrc += 6;
                       ptr = psrc;
                       func = __part_replace;
                       what = "part";
                    }
                  else if (!strncmp(psrc, "PROGRAM:\"", 9))
                    {
                       psrc += 9;
                       ptr = psrc;
                       func = __program_replace;
                       what = "program";
                    }
               }
             else if (*psrc == 'G')
               {
                  if (!strncmp(psrc, "GROUP:\"", 7))
                    {
                       psrc += 7;
                       ptr = psrc;
                       func = __group_replace;
                       what = "group";
                    }
               }
             else if (*psrc == 'I')
               {
                  if (!strncmp(psrc, "IMAGE:\"", 7))
                    {
                       psrc += 7;
                       ptr = psrc;
                       func = __image_replace;
                       what = "image";
                    }
               }
             else if (*psrc == '#')
               {
                  while (*psrc)
                    if (*psrc == '\n')
                      break;
                  line++;
                  continue;
               }
             else if (*psrc == '\"')
               quoted = 1;
             else if (*psrc == '\n')
               line++;

             if (ptr)
               {
                  int i = 0, inesc = 0;
                  char *name;
                  while (*psrc)
                    {
                       if (!inesc)
                         {
                            if (*psrc == '\\')
                              inesc = 1;
                            else if (*psrc == '\"')
                              {
                                 /* string concatenation as in "foo""bar" */
                                 if (*(psrc + 1) != '\"')
                                   {
                                      psrc++;
                                      break;
                                   }
                                 else
                                   psrc++;
                              }
                         }
                       else
                         inesc = 0;
                       psrc++;
                    }
                  name = alloca(psrc - ptr);
                  inesc = 0;
                  while (*ptr)
                    {
                       if (!inesc)
                         {
                            if (*ptr == '\\')
                              inesc = 1;
                            else if (*ptr == '\"')
                              {
                                 if (*(ptr + 1) == '\"')
                                   ptr++;
                                 else
                                   {
                                      name[i] = 0;
                                      break;
                                   }
                              }
                            else
                              {
                                 name[i] = *ptr;
                                 name[i + 1] = 0;
                                 i++;
                              }
                         }
                       else
                         inesc = 0;
                       ptr++;
                    }
                  i = func(eed, pdst, name);
                  if (!i)
                    {
                       Edje_Edit_Script_Error *se;
                       se = malloc(sizeof(Edje_Edit_Script_Error));
                       se->program_name = progname ?
                         eina_stringshare_add(progname) : NULL;
                       se->line = line;
                       se->error_str = eina_stringshare_printf(
                           "Referenced %s '%s' could not be found in object.",
                           what, name);
                       eed->errors = eina_list_append(eed->errors, se);
                       success = EINA_FALSE;
                    }
                  else
                    {
                       pcodesize += i;
                       pdst += i;
                    }
                  /* replaced reference for the right value, now go
                   * to the next iteration */
                  continue;
               }
          }
        else
          {
             if (!escaped)
               {
                  if (*psrc == '\"')
                    quoted = 0;
                  else if (*psrc == '\\')
                    escaped = 1;
               }
             else if (escaped)
               escaped = 0;
          }
        *pdst = *psrc;
        pdst++;
        psrc++;
        pcodesize++;
     }

   if (!success)
     {
        free(pcode);
        return NULL;
     }

   if (pcodesize < codesize)
     pcode = realloc(pcode, pcodesize + 1);
   pcode[pcodesize] = 0;

   return pcode;
}

static Eina_Bool
_edje_edit_embryo_rebuild(Edje_Edit *eed)
{
   FILE *f;
   int fd, size, ret;
   Eina_Tmpstr *tmp_in;
   Eina_Tmpstr *tmp_out;
   char embryo_cc_path[PATH_MAX] = "";
   char inc_path[PATH_MAX] = "";
   char buf[4096];
   Eina_Iterator *it;
   Program_Script *ps;
   Edje_Part_Collection *edc;
   Eina_Bool success = EINA_TRUE; /* we are optimists! */
   Edje_Edit_Script_Error *se;
   Eina_Prefix *pfx;

   EINA_LIST_FREE(eed->errors, se)
     {
        eina_stringshare_del(se->program_name);
        eina_stringshare_del(se->error_str);
        free(se);
     }

   pfx = eina_prefix_new(NULL, /* argv[0] value (optional) */
                         edje_init, /* an optional symbol to check path of */
                         "EDJE", /* env var prefix to use (XXX_PREFIX, XXX_BIN_DIR etc. */
                         "edje", /* dir to add after "share" (PREFIX/share/DIRNAME) */
                         "include/edje.inc", /* a magic file to check for in PREFIX/share/DIRNAME for success */
                         PACKAGE_BIN_DIR,    /* package bin dir @ compile time */
                         PACKAGE_LIB_DIR,    /* package lib dir @ compile time */
                         PACKAGE_DATA_DIR,   /* package data dir @ compile time */
                         PACKAGE_DATA_DIR    /* if locale needed  use LOCALE_DIR */
                         );
#ifdef _WIN32
# define BIN_EXT ".exe"
#else
# define BIN_EXT
#endif

   bs_binary_get(embryo_cc_path, sizeof(embryo_cc_path), "embryo", "embryo_cc");

   bs_data_path_get(inc_path, sizeof(inc_path), "edje", "include");


   if (embryo_cc_path[0] == '\0')
     {
        snprintf(embryo_cc_path, sizeof(embryo_cc_path),
                 "%s/embryo_cc" BIN_EXT,
                 eina_prefix_bin_get(pfx));
        snprintf(inc_path, sizeof(inc_path),
                 "%s/include",
                 eina_prefix_data_get(pfx));
     }
#undef BIN_EXT

   eina_prefix_free(pfx);

   fd = eina_file_mkstemp("edje_edit.sma-tmp-XXXXXX", &tmp_in);
   if (fd < 0)
     return EINA_FALSE;  /* FIXME: report something */

   f = fdopen(fd, "wb");
   if (!f)
     {
        close(fd);
        unlink(tmp_in);
        return EINA_FALSE;
     }

   fprintf(f, "#include <edje>\n");
   if (eed->embryo_source)
     {
        if (eed->all_dirty)
          {
             free(eed->embryo_processed);
             eed->embryo_processed = NULL;
          }
        if (!eed->embryo_processed)
          eed->embryo_processed = _edje_edit_script_process(eed, NULL,
                                                            eed->embryo_source);
        if (!eed->embryo_processed)
          {
             /* oops.. an error finding references parts or something.
              * we could flag it and do some lighter processing of the
              * rest of the scripts, in order to find all the errors of
              * this kind and report them at once, but knowing already
              * that the script will not compile we can avoid some work
              */
             success = EINA_FALSE;
          }
        else
          fprintf(f, "%s", eed->embryo_processed);
     }

   it = eina_hash_iterator_data_new(eed->program_scripts);
   EINA_ITERATOR_FOREACH(it, ps)
     {
        Edje_Program *epr;

        if (ps->delete_me)
          continue;
        if (eed->all_dirty)
          {
             free(ps->processed);
             ps->processed = NULL;
          }
        epr = eed->base->collection->patterns.table_programs[ps->id];
        if (!ps->processed)
          ps->processed = _edje_edit_script_process(eed, epr->name, ps->code);
        if (!ps->processed)
          {
             /* oops.. an error finding references parts or something.
              * we could flag it and do some lighter processing of the
              * rest of the scripts, in order to find all the errors of
              * this kind and report them at once, but knowing already
              * that the script will not compile we can avoid some work
              */
             success = EINA_FALSE;
             continue;
          }
        fprintf(f, "public _p%i(sig[], src[]) {\n", ps->id);
        fprintf(f, "%s", ps->processed);
        fprintf(f, "}\n");
     }
   eina_iterator_free(it);

   fclose(f);

   if (!success)
     goto almost_out;

   fd = eina_file_mkstemp("edje_edit.amx-tmp-XXXXXX", &tmp_out);
   if (fd < 0)
     {
        success = EINA_FALSE;
        goto almost_out;
     }

   snprintf(buf, sizeof(buf), "%s -i %s -o %s %s",
            embryo_cc_path, inc_path, tmp_out, tmp_in);
   ret = system(buf);

   if ((ret < 0) || (ret > 1))
     {
        success = EINA_FALSE;
        close(fd);
        goto the_doorway;
     }

   f = fdopen(fd, "rb");
   if (!f)
     {
        success = EINA_FALSE;
        close(fd);
        goto the_doorway;
     }

   fseek(f, 0, SEEK_END);
   size = ftell(f);
   rewind(f);

   free(eed->bytecode);
   if (size > 0)
     {
        eed->bytecode = malloc(size);
        if (!eed->bytecode)
          {
             success = EINA_FALSE;
             goto the_way_out;
          }
        if (fread(eed->bytecode, size, 1, f) != 1)
          {
             success = EINA_FALSE;
             goto the_way_out;
          }
     }
   else
     eed->bytecode = NULL;  /* correctness mostly, I don't see why we
                               would get a 0 sized program */

   eed->bytecode_size = size;
   eed->bytecode_dirty = EINA_TRUE;
   eed->script_need_recompile = EINA_FALSE;
   eed->all_dirty = EINA_FALSE;

   edc = eed->base->collection;
   embryo_program_free(edc->script);
   edc->script = embryo_program_new(eed->bytecode, eed->bytecode_size);
   _edje_embryo_script_init(edc);
   _edje_var_init(eed->base);

the_way_out:
   fclose(f);
the_doorway:
   if (tmp_out)
     {
        unlink(tmp_out);
        eina_tmpstr_del(tmp_out);
     }
almost_out:
   unlink(tmp_in);
   eina_tmpstr_del(tmp_in);

   return success;
}

EAPI Eina_Bool
edje_edit_script_compile(Evas_Object *obj)
{
   GET_EED_OR_RETURN(EINA_FALSE);

   if (!eed->script_need_recompile)
     return EINA_TRUE;

   return _edje_edit_embryo_rebuild(eed);
}

EAPI const Eina_List *
edje_edit_script_error_list_get(Evas_Object *obj)
{
   GET_EED_OR_RETURN(NULL);
   return eed->errors;
}

/***************************/
/*  EDC SOURCE GENERATION  */
/***************************/
#define I0 ""
#define I1 "   "
#define I2 "      "
#define I3 "         "
#define I4 "            "
#define I5 "               "
#define I6 "                  "
#define I7 "                     "

#define BUF_APPEND(STR) \
  ret &= eina_strbuf_append(buf, STR)

#define BUF_APPENDF(FMT, ...) \
  ret &= eina_strbuf_append_printf(buf, FMT, ##__VA_ARGS__)

#define COLLECT_RESOURCE(condition_data, list)           \
  if (condition_data)                                    \
    {                                                    \
       if (!eina_list_data_find(list, condition_data))   \
         list = eina_list_append(list, condition_data);  \
    }

static const char *types[] = {"NONE", "RECT", "TEXT", "IMAGE", "SWALLOW", "TEXTBLOCK", "GRADIENT", "GROUP", "BOX", "TABLE", "EXTERNAL", "PROXY", "SPACER"};
static const char *effects[] = {"NONE", "PLAIN", "OUTLINE", "SOFT_OUTLINE", "SHADOW", "SOFT_SHADOW", "OUTLINE_SHADOW", "OUTLINE_SOFT_SHADOW", "FAR_SHADOW", "FAR_SOFT_SHADOW", "GLOW"};
static const char *shadow_direction[] = {"BOTTOM_RIGHT", "BOTTOM", "BOTTOM_LEFT", "LEFT", "TOP_LEFT", "TOP", "TOP_RIGHT", "RIGHT"};
static const char *prefers[] = {"NONE", "VERTICAL", "HORIZONTAL", "BOTH", "SOURCE"};
static const char *entry_mode[] = {"NONE", "PLAIN", "EDITABLE", "PASSWORD"};
static const char *aspect_mode[] = {"NONE", "NEITHER", "HORIZONTAL", "VERTICAL", "BOTH"};

static Eina_Bool
 _edje_generate_source_of_group(Edje *ed, Edje_Part_Collection_Directory_Entry *pce, Eina_Strbuf *buf);

static Eina_Bool
 _edje_generate_source_of_style(Edje *ed, const char *name, Eina_Strbuf *buf);

static Eina_Bool
 _edje_generate_source_of_colorclass(Edje *ed, const char *name, Eina_Strbuf *buf);

static Eina_Strbuf *
_edje_generate_image_set_source(Evas_Object *obj, const char *entry)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Bool ret = EINA_FALSE;
   if (!buf) return NULL;

   BUF_APPENDF(I1 "set { name: \"%s\";\n", entry);
   Eina_List *images = NULL, *ll = NULL;
   const char *image_name = NULL;
   unsigned int place = 0;

   images = edje_edit_image_set_images_list_get(obj, entry);
   EINA_LIST_FOREACH(images, ll, image_name)
     {
        BUF_APPEND(I2 "image {\n");
        int comp = edje_edit_image_compression_type_get(obj, image_name);
        if (comp < 0)
          {
            eina_strbuf_free(buf);
            return NULL;
          }
        BUF_APPENDF(I3 "image: \"%s\" ", image_name);

        if (comp == EDJE_EDIT_IMAGE_COMP_LOSSY)
          BUF_APPENDF("LOSSY %d;\n",
                      edje_edit_image_compression_rate_get(obj, image_name));
        else if (comp == EDJE_EDIT_IMAGE_COMP_LOSSY_ETC1)
          BUF_APPENDF("LOSSY_ETC1 %d;\n",
                      edje_edit_image_compression_rate_get(obj, image_name));
        else if (comp == EDJE_EDIT_IMAGE_COMP_LOSSY_ETC2)
          BUF_APPENDF("LOSSY_ETC2 %d;\n",
                      edje_edit_image_compression_rate_get(obj, image_name));
        else if (comp == EDJE_EDIT_IMAGE_COMP_RAW)
          BUF_APPEND("RAW;\n");
        else if (comp == EDJE_EDIT_IMAGE_COMP_USER)
          BUF_APPEND("USER;\n");
        else
          BUF_APPEND("COMP;\n");

        int min_w = 0, min_h = 0, max_w = 0, max_h = 0;
        edje_edit_image_set_image_min_get(obj, entry, place, &min_w, &min_h);
        edje_edit_image_set_image_max_get(obj, entry, place, &max_w, &max_h);
        if (min_w != 0 || min_h != 0 || max_w != 0 || max_h != 0)
          BUF_APPENDF(I3 "size: %d %d %d %d;\n", min_w, min_h, max_w, max_h);

        int l = 0, r = 0, t = 0, b = 0;
        edje_edit_image_set_image_border_get(obj, entry, place, &l, &r, &t, &b);
        if (l != 0 || r != 0 || t != 0 || b != 0)
          BUF_APPENDF(I3 "border: %d %d %d %d;\n", l, r, t, b);

        double scale_by = 0;
        scale_by = edje_edit_image_set_image_border_scale_get(obj, entry, place);
        if (NEQ(scale_by, 0))
          BUF_APPENDF(I3 "border_scale_by: %.3f;\n", scale_by);

        BUF_APPEND(I2 "}\n");
        place++;
     }
   BUF_APPEND(I1 "}\n");

   return buf;
}


static Eina_Strbuf *
_edje_generate_image_source(Evas_Object *obj, const char *entry)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Bool ret = EINA_TRUE;
   if (!buf) return NULL;

   int comp = edje_edit_image_compression_type_get(obj, entry);
   if (comp < 0) goto error;

   BUF_APPENDF("image: \"%s\" ", entry);

   if (comp == EDJE_EDIT_IMAGE_COMP_LOSSY)
     BUF_APPENDF("LOSSY %d;\n",
                 edje_edit_image_compression_rate_get(obj, entry));
   else if (comp == EDJE_EDIT_IMAGE_COMP_LOSSY_ETC1)
     BUF_APPENDF("LOSSY_ETC1 %d;\n",
                 edje_edit_image_compression_rate_get(obj, entry));
   else if (comp == EDJE_EDIT_IMAGE_COMP_LOSSY_ETC2)
     BUF_APPENDF("LOSSY_ETC2 %d;\n",
                 edje_edit_image_compression_rate_get(obj, entry));
   else if (comp == EDJE_EDIT_IMAGE_COMP_RAW)
     BUF_APPEND("RAW;\n");
   else if (comp == EDJE_EDIT_IMAGE_COMP_USER)
     BUF_APPEND("USER;\n");
   else
     BUF_APPEND("COMP;\n");

   if (!ret) goto error;

   return buf;

error:
   ERR("Generating EDC for Image");
   eina_strbuf_free(buf);
   return NULL;
}

static const char *
_edje_edit_source_generate(Evas_Object *obj, Eina_Bool without_global_data)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Part *part;
   Edje_Part_Description_Common *part_desc;
   Edje_Part_Description_Image *part_desc_image;
   Edje_Part_Description_Text *part_desc_text;
   unsigned short i;
   unsigned int j;
   const char *entry;
   const char *str;
   Eina_Strbuf *buf = NULL;
   Eina_Bool ret = EINA_TRUE;
   Eina_List *images = NULL, *color_classes = NULL, *styles = NULL, *fonts = NULL;
   Eina_List *images_set = NULL;
   Eina_List *sounds = NULL;
   Eina_List *l;

   GET_ED_OR_RETURN(NULL);
   GET_EED_OR_RETURN(NULL);

   if (!ed->file) return NULL;

   ce = eina_hash_find(ed->file->collection, ed->group);
   if (!ce) return NULL;

   /* Go through all of group's parts to find all resources needed for that group. */
   for (i = 0; i < ed->table_parts_size; i++)
     {
        part = ed->table_parts[i]->part;
        part_desc = (Edje_Part_Description_Common *)part->default_desc;

        /* find all image parts and collect all images required by those parts. */
        if (part->type == EDJE_PART_TYPE_IMAGE)
          {
             /*  parse "default" description of this part. */
             part_desc_image = (Edje_Part_Description_Image *)part->default_desc;

             if (part_desc_image->image.set)
               {
                  entry = _edje_set_name_find(eed, part_desc_image->image.id);
                  COLLECT_RESOURCE(entry, images_set);
               }
             else
               {
                  /* find image name according to it's id that is in description */
                  entry = _edje_image_name_find(eed, part_desc_image->image.id);
                  COLLECT_RESOURCE(entry, images);
               }
             for (j = 0; j < part_desc_image->image.tweens_count; j++)
               {
                  entry = _edje_image_name_find(eed, part_desc_image->image.tweens[j]->id);
                  COLLECT_RESOURCE(entry, images);
               }
             /*  look through all other's descriptions. */
             for (j = 0; j < part->other.desc_count; j++)
               {
                  part_desc_image = (Edje_Part_Description_Image *)part->other.desc[j];

                  if (part_desc_image->image.set)
                    {
                       entry = _edje_set_name_find(eed, part_desc_image->image.id);
                       COLLECT_RESOURCE(entry, images_set);
                    }
                  else
                    {
                       /* find image name according to it's id that is in description */
                       entry = _edje_image_name_find(eed, part_desc_image->image.id);
                       COLLECT_RESOURCE(entry, images);
                    }
              }
          }
        /* find all text, textblock part and fonts, styles required by those parts. */
        if ((part->type == EDJE_PART_TYPE_TEXTBLOCK) ||
            (part->type == EDJE_PART_TYPE_TEXT))
          {
             part_desc_text = (Edje_Part_Description_Text *)part->default_desc;
             COLLECT_RESOURCE(part_desc_text->text.style.str, styles);
             if (part_desc_text->text.font.str)
               {
                  Edje_Font_Directory_Entry *fnt;
                  fnt = eina_hash_find(ed->file->fonts, part_desc_text->text.font.str);
                  COLLECT_RESOURCE(fnt, fonts);
               }
             for (j = 0; j < part->other.desc_count; j++)
               {
                  part_desc_text = (Edje_Part_Description_Text *)part->other.desc[j];
                  COLLECT_RESOURCE(part_desc_text->text.style.str, styles);
                  if (part_desc_text->text.font.str)
                    {
                       Edje_Font_Directory_Entry *fnt;
                       fnt = eina_hash_find(ed->file->fonts, part_desc_text->text.font.str);
                       COLLECT_RESOURCE(fnt, fonts);
                    }
               }
          }
        /* find all color_classes required by those every part. */
        COLLECT_RESOURCE(part_desc->color_class, color_classes);
        for (j = 0; j < part->other.desc_count; j++)
          {
             part_desc = part->other.desc[j];
             COLLECT_RESOURCE(part_desc->color_class, color_classes);
          }
     }
   /* collect all sound samples, that uses in current collection */
   for (j = 0;
        j < (unsigned int)ed->collection->patterns.table_programs_size;
        j++)
     {
        Edje_Program *epr;
        Edje_Sound_Sample *sample;
        epr = ed->collection->patterns.table_programs[j];
        if ((!epr) || (epr->action != EDJE_ACTION_TYPE_SOUND_SAMPLE))
          continue;
        for (i = 0; i < (unsigned int)ed->file->sound_dir->samples_count; i++)
          {
             sample = &ed->file->sound_dir->samples[i];
             if (!strcmp(sample->name, epr->sample_name))
               {
                  COLLECT_RESOURCE(sample, sounds);
                  break;
               }
          }
     }

   buf = eina_strbuf_new();

   /* If data items exist, print them */
   if (!without_global_data && ed->file->data)
     {
        Edje_String *es;
        size_t data_len = 0;
        char *escaped_entry = NULL;
        char *escaped_string = NULL;
        BUF_APPEND(I0 "data {\n");
        Eina_Iterator *it = eina_hash_iterator_key_new(ed->file->data);
        EINA_ITERATOR_FOREACH(it, entry)
          {
             es = eina_hash_find(ed->file->data, entry);
             str = edje_string_get(es);
             if (!str) break;
             data_len = strlen(str);
             /* In case when data ends with '\n' character, this item recognize
              * as data.file. This data will not generated into the source code
              * of group. */
             if (str[data_len - 1] == '\n') continue;
             escaped_entry = eina_str_escape(entry);
             escaped_string = eina_str_escape(str);
             BUF_APPENDF(I1 "item: \"%s\" \"%s\";\n", escaped_entry, escaped_string);
             free(escaped_entry);
             free(escaped_string);
          }
        eina_iterator_free(it);
        BUF_APPEND(I0 "}\n\n");
     }

   if (ed->file->text_classes)
     {
        BUF_APPEND(I0 "text_classes {\n");
        Edje_Text_Class *tc;

        EINA_LIST_FOREACH(ed->file->text_classes, l, tc)
          {
             BUF_APPENDF(I1 "text_class {\n");
             BUF_APPENDF(I2 "name: \"%s\";\n", tc->name);
             if (tc->font)
               BUF_APPENDF(I2 "font: \"%s\";\n", tc->font);
             if (tc->size > 0)
               BUF_APPENDF(I2 "size: %d;\n", tc->size);
             BUF_APPENDF(I1 "}\n");
          }

        BUF_APPEND(I0 "}\n\n");
     }

   if (ed->file->size_classes)
     {
        BUF_APPEND(I0 "size_classes {\n");
        Edje_Size_Class *sc;

        EINA_LIST_FOREACH(ed->file->size_classes, l, sc)
          {
             BUF_APPENDF(I1 "size_class {\n");
             BUF_APPENDF(I2 "name: \"%s\";\n", sc->name);
             if ((sc->minw > 0) || (sc->minh > 0))
               BUF_APPENDF(I2 "min: %d %d;\n", sc->minw, sc->minh);
             if ((sc->maxw >= -1) || (sc->maxh >= -1))
               BUF_APPENDF(I2 "max: %d %d;\n", sc->maxw, sc->maxh);
             BUF_APPENDF(I1 "}\n");
          }

        BUF_APPEND(I0 "}\n\n");
     }

   /* if images were found, print them */
   if (images || images_set)
     {
        BUF_APPEND(I0 "images {\n");

        EINA_LIST_FOREACH(images, l, entry)
          {
             Eina_Strbuf *gen_buf = _edje_generate_image_source(obj, entry);
             if (!gen_buf) continue;

             BUF_APPENDF(I1 "%s", eina_strbuf_string_get(gen_buf));
             eina_strbuf_free(gen_buf);
          }

        EINA_LIST_FOREACH(images_set, l, entry)
          {
             Eina_Strbuf *gen_buf = _edje_generate_image_set_source(obj, entry);
             if (!gen_buf) continue;

             BUF_APPENDF("%s", eina_strbuf_string_get(gen_buf));
             eina_strbuf_free(gen_buf);
          }


        BUF_APPEND(I0 "}\n\n");
        eina_list_free(images);
     }
   /* if styles were found, print them */
   if (styles)
     {
        BUF_APPEND(I0 "styles {\n");
        EINA_LIST_FOREACH(styles, l, entry)
          _edje_generate_source_of_style(ed, entry, buf);
        BUF_APPEND(I0 "}\n\n");
        eina_list_free(styles);
     }
   /* if fonts were found, print them */
   if (fonts)
     {
        BUF_APPEND(I0 "fonts {\n");
        Edje_Font_Directory_Entry *fnt;

        EINA_LIST_FOREACH(fonts, l, fnt)
          {
             BUF_APPENDF(I1 "font: \"%s\" \"%s\";\n", fnt->file,
                         fnt->entry);
          }

        BUF_APPEND(I0 "}\n\n");
        eina_list_free(fonts);
     }
   /* if color_classes were found, print them */
   if (!without_global_data && color_classes)
     {
        BUF_APPEND(I0 "color_classes {\n");

        EINA_LIST_FOREACH(color_classes, l, entry)
          _edje_generate_source_of_colorclass(ed, entry, buf);

        BUF_APPEND(I0 "}\n\n");
        eina_list_free(color_classes);
     }


   /* print the main code of group collections */
   if (!without_global_data) BUF_APPEND(I0 "collections {\n");
   /* if sounds were found, print them */
   if (sounds)
     {
        Edje_Sound_Sample *uses_sample;
        BUF_APPEND(I1 "sounds {\n");
        EINA_LIST_FOREACH(sounds, l, uses_sample)
          {
             BUF_APPEND(I2 "sample {\n");
             BUF_APPENDF(I3 "name: \"%s\" ", uses_sample->name);
             switch (uses_sample->compression)
               {
                case EDJE_SOUND_SOURCE_TYPE_INLINE_RAW:
                {
                   BUF_APPEND("RAW;\n");
                   break;
                }

                case EDJE_SOUND_SOURCE_TYPE_INLINE_COMP:
                {
                   BUF_APPEND("COMP;\n");
                   break;
                }

                case EDJE_SOUND_SOURCE_TYPE_INLINE_LOSSY:
                {
                   BUF_APPENDF("LOSSY %f;\n", uses_sample->quality);
                   break;
                }

                case EDJE_SOUND_SOURCE_TYPE_INLINE_AS_IS:
                {
                   BUF_APPEND("AS_IS;\n");
                   break;
                }

                default:
                  break;
               }
             BUF_APPENDF(I3 "source: \"%s\";\n", uses_sample->snd_src);
             BUF_APPEND(I2 "}\n");
          }
        BUF_APPEND(I1 "}\n");
     }
   _edje_generate_source_of_group(ed, ce, buf);
   if (!without_global_data) BUF_APPEND(I0 "}");

   if (!ret)
     {
        ERR("Generating EDC for This Group.");
        eina_strbuf_free(buf);
        return NULL;
     }

   /* return resulted source code of the group */
   str = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return str;
}

EAPI const char *
edje_edit_source_generate(Evas_Object *obj)
{
   return _edje_edit_source_generate(obj, EINA_FALSE);
}

EAPI const char *
edje_edit_object_source_generate(Evas_Object *obj)
{
   return _edje_edit_source_generate(obj, EINA_TRUE);
}

EAPI const char *
edje_edit_data_source_generate(Evas_Object *obj)
{
   Eina_Strbuf *buf = NULL;
   Eina_Bool ret = EINA_TRUE;
   Eina_Stringshare *str = NULL;
   const char *entry;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file) return NULL;

   buf = eina_strbuf_new();

   /* If data items exist, print them */
   if (ed->file->data)
     {
        Edje_String *es;
        size_t data_len = 0;
        char *escaped_entry = NULL;
        char *escaped_string = NULL;
        BUF_APPEND(I0 "data {\n");
        Eina_Iterator *it = eina_hash_iterator_key_new(ed->file->data);
        EINA_ITERATOR_FOREACH(it, entry)
          {
             es = eina_hash_find(ed->file->data, entry);
             str = edje_string_get(es);
             if (!str) break;
             data_len = strlen(str);
             /* In case when data ends with '\n' character, this item recognize
              * as data.file. This data will not generated into the source code
              * of group. */
             if (str[data_len - 1] == '\n') continue;
             escaped_entry = eina_str_escape(entry);
             escaped_string = eina_str_escape(str);
             BUF_APPENDF(I1 "item: \"%s\" \"%s\";\n", escaped_entry, escaped_string);
             free(escaped_entry);
             free(escaped_string);
          }
        eina_iterator_free(it);
        BUF_APPEND(I0 "}\n\n");
     }

   /* return resulted source code of the group */
   if (ret)
     str = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return str;
}

EAPI Eina_List *
edje_edit_object_color_class_list_get(Evas_Object *obj)
{
   Edje_Part_Collection_Directory_Entry *ce;
   Edje_Part *part;
   Edje_Part_Description_Common *part_desc;
   unsigned int j, i;
   Eina_List *color_classes = NULL;

   GET_ED_OR_RETURN(NULL);

   if (!ed->file) return NULL;

   ce = eina_hash_find(ed->file->collection, ed->group);
   if (!ce) return NULL;

   /* Go through all of group's parts to find all resources needed for that group. */
   for (i = 0; i < ed->table_parts_size; i++)
     {
        part = ed->table_parts[i]->part;
        part_desc = (Edje_Part_Description_Common *)part->default_desc;

        /* find all color_classes required by those every part. */
        COLLECT_RESOURCE(part_desc->color_class, color_classes);
        for (j = 0; j < part->other.desc_count; j++)
          {
             part_desc = part->other.desc[j];
             COLLECT_RESOURCE(part_desc->color_class, color_classes);
          }
     }

   return color_classes;
}

EAPI const char *
edje_edit_color_classes_source_generate(Evas_Object *obj, Eina_List *color_classes)
{
   Eina_Strbuf *buf;
   Eina_List *l;
   const char *entry;
   Eina_Bool ret = EINA_TRUE;
   Eina_Stringshare *str = NULL;

   GET_ED_OR_RETURN(NULL);

   buf = eina_strbuf_new();

   BUF_APPEND(I0 "color_classes {\n");

   EINA_LIST_FOREACH(color_classes, l, entry)
     _edje_generate_source_of_colorclass(ed, entry, buf);

   BUF_APPEND(I0 "}\n\n");

   if (ret)
     str = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return str;
}

#undef COLLECT_RESOURCE

static Eina_Bool
_edje_generate_source_of_sizeclass(Edje *ed, const char *name, Eina_Strbuf *buf)
{
   Eina_List *l;
   Edje_Size_Class *sc;
   Eina_Bool ret = EINA_TRUE;

   EINA_LIST_FOREACH(ed->file->size_classes, l, sc)
      if (!strcmp(sc->name, name))
        {
           BUF_APPENDF(I1 "size_class {\n");
           BUF_APPENDF(I2 "name: \"%s\";\n", sc->name);
           if ((sc->minw > 0) || (sc->minh > 0))
             BUF_APPENDF(I2 "min: %d %d;\n", sc->minw, sc->minh);
           if ((sc->maxw >= -1) || (sc->maxh >= -1))
             BUF_APPENDF(I2 "max: %d %d;\n", sc->maxw, sc->maxh);
           BUF_APPENDF(I1 "}\n");
        }

   return ret;
}

static Eina_Bool
_edje_generate_source_of_textclass(Edje *ed, const char *name, Eina_Strbuf *buf)
{
   Eina_List *l;
   Edje_Text_Class *tc;
   Eina_Bool ret = EINA_TRUE;

   EINA_LIST_FOREACH(ed->file->text_classes, l, tc)
      if (!strcmp(tc->name, name))
        {
           BUF_APPENDF(I1 "text_class {\n");
           BUF_APPENDF(I2 "name: \"%s\";\n", tc->name);
           if (tc->font)
             BUF_APPENDF(I2 "font: \"%s\";\n", tc->font);
           if (tc->size > 0)
             BUF_APPENDF(I2 "size: %d;\n", tc->size);
           BUF_APPENDF(I1 "}\n");
        }

   return ret;
}

static Eina_Bool
_edje_generate_source_of_colorclass(Edje *ed, const char *name, Eina_Strbuf *buf)
{
   Eina_List *l;
   Edje_Color_Class *cc;
   Eina_Bool ret = EINA_TRUE;

   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     if (!strcmp(cc->name, name))
       {
          BUF_APPENDF(I1 "color_class { name: \"%s\";\n", cc->name);
          BUF_APPENDF(I2 "color: %d %d %d %d;\n", cc->r, cc->g, cc->b, cc->a);
          BUF_APPENDF(I2 "color2: %d %d %d %d;\n", cc->r2, cc->g2, cc->b2, cc->a2);
          BUF_APPENDF(I2 "color3: %d %d %d %d;\n", cc->r3, cc->g3, cc->b3, cc->a3);
          BUF_APPEND(I1 "}\n");
       }
   return ret;
}

static Eina_Bool
_edje_generate_source_of_style(Edje *ed, const char *name, Eina_Strbuf *buf)
{
   Eina_List *l, *ll;
   Edje_Style *s;
   Edje_Style_Tag *t;
   Eina_Bool ret = EINA_TRUE;

   int len, i;
   #define ESCAPE_VAL(VAL)                     \
  for (i = 0, len = strlen(VAL); i < len; i++) \
    switch (VAL[i])                            \
      {                                        \
       case '\n':                              \
       {                                       \
          BUF_APPENDF("%s", "\\n");            \
          break;                               \
       }                                       \
       case '\t':                              \
       {                                       \
          BUF_APPENDF("%s", "\\t");            \
          break;                               \
       }                                       \
       case '"':                               \
       {                                       \
          BUF_APPENDF("%s", "\\\"");           \
          break;                               \
       }                                       \
       case '\\':                              \
       {                                       \
          BUF_APPENDF("%s", "\\\\");           \
          break;                               \
       }                                       \
       default: BUF_APPENDF("%c", VAL[i]);     \
      }

   EINA_LIST_FOREACH(ed->file->styles, l, s)
     if (!strcmp(s->name, name))
       {
          t = s->tags ? s->tags->data : NULL;
          BUF_APPENDF(I1 "style { name:\"%s\";\n", s->name);
          if (t && t->value)
            {
               BUF_APPEND(I2 "base: \"");
               ESCAPE_VAL(t->value);
               BUF_APPEND("\";\n");
            }

          EINA_LIST_FOREACH(s->tags, ll, t)
            if (ll->prev && t && t->value)
              {
                 BUF_APPENDF(I2 "tag: \"%s\" \"", t->key);
                 ESCAPE_VAL(t->value);
                 BUF_APPEND("\";\n");
              }
          BUF_APPEND(I1 "}\n");
          return ret;
       }
   #undef ESCAPE_VAL
   return EINA_FALSE;
}

static Eina_Bool
_edje_generate_source_of_program(Evas_Object *obj, const char *program, Eina_Strbuf *buf)
{
   Eina_List *l, *ll;
   const char *s;
   double db, db2, v1, v2, v3, v4;
   char *data;
   Eina_Bool ret = EINA_TRUE;
   Eina_Bool no_transition = EINA_FALSE;
   const char *api_name, *api_description;
   Edje_Program *epr;
   int tweenmode = 0;

   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EED_OR_RETURN(EINA_FALSE);

   epr = _edje_program_get_byname(obj, program);

   BUF_APPENDF(I3 "program { name: \"%s\";\n", program);

   /* Signal */
   s = eina_stringshare_add(epr->signal);
   if ((s != NULL) && (strcmp(s, "")))
     {
        BUF_APPENDF(I4 "signal: \"%s\";\n", s);
        edje_edit_string_free(s);
     }

   /* Source */
   s = eina_stringshare_add(epr->source);
   if ((s != NULL) && (strcmp(s, "")))
     {
        BUF_APPENDF(I4 "source: \"%s\";\n", s);
        edje_edit_string_free(s);
     }

   /* Filter */
   if (epr->filter.part && epr->filter.state)
     {
        BUF_APPENDF(I4 "filter: \"%s\" \"%s\";\n",
                    epr->filter.part, epr->filter.state);
     }

   /* Action */
   switch (epr->action)
     {
      case EDJE_ACTION_TYPE_ACTION_STOP:
        BUF_APPEND(I4 "action: ACTION_STOP;\n");
        break;

      case EDJE_ACTION_TYPE_STATE_SET:
        if (epr->state)
          {
             BUF_APPENDF(I4 "action: STATE_SET \"%s\" %.2f;\n", epr->state,
                         edje_edit_program_value_get(obj, program));
          }
        break;

      case EDJE_ACTION_TYPE_SIGNAL_EMIT:
        if (epr->state && epr->state2)
          BUF_APPENDF(I4 "action: SIGNAL_EMIT \"%s\" \"%s\";\n", epr->state, epr->state2);
        break;

      case EDJE_ACTION_TYPE_SCRIPT:
      {
         Program_Script *ps;

         ps = eina_hash_find(eed->program_scripts, &epr->id);
         if (ps && !ps->delete_me)
           {
              BUF_APPEND(I4 "script {\n");
              BUF_APPEND(ps->code);
              BUF_APPEND(I4 "}\n");
           }
      }
      break;

      case EDJE_ACTION_TYPE_SOUND_SAMPLE:
      {
         BUF_APPEND(I4 "action: PLAY_SAMPLE ");
         BUF_APPENDF("\"%s\" %.4f", epr->sample_name, epr->speed);
         switch (epr->channel)
           {
            case EDJE_CHANNEL_BACKGROUND:
            {
               BUF_APPEND(" BACKGROUND");
               break;
            }

            case EDJE_CHANNEL_MUSIC:
            {
               BUF_APPEND(" MUSIC");
               break;
            }

            case EDJE_CHANNEL_FOREGROUND:
            {
               BUF_APPEND(" FOREGROUND");
               break;
            }

            case EDJE_CHANNEL_INTERFACE:
            {
               BUF_APPEND(" INTERFACE");
               break;
            }

            case EDJE_CHANNEL_INPUT:
            {
               BUF_APPEND(" INPUT");
               break;
            }

            case EDJE_CHANNEL_ALERT:
            {
               BUF_APPEND(" ALERT");
               break;
            }

            default:
              break;
           }
         BUF_APPENDF(";\n");
         break;
      }

      case EDJE_ACTION_TYPE_SOUND_TONE:
      {
         BUF_APPEND(I4 "action: PLAY_TONE ");
         BUF_APPENDF("\"%s\" %.4f;\n", epr->tone_name, epr->duration);
         break;
      }

      case EDJE_ACTION_TYPE_DRAG_VAL_SET:
      {
         BUF_APPEND(I4 "action: DRAG_VAL_SET ");
         BUF_APPENDF("%.4f %.4f;\n", epr->value, epr->value2);
         break;
      }

      case EDJE_ACTION_TYPE_DRAG_VAL_STEP:
      {
         BUF_APPEND(I4 "action: DRAG_VAL_STEP ");
         BUF_APPENDF("%.4f %.4f;\n", epr->value, epr->value2);
         break;
      }

      case EDJE_ACTION_TYPE_DRAG_VAL_PAGE:
      {
         BUF_APPEND(I4 "action: DRAG_VAL_PAGE ");
         BUF_APPENDF("%.4f %.4f;\n", epr->value, epr->value2);
         break;
      }

      case EDJE_ACTION_TYPE_FOCUS_SET:
      {
         if (epr->seat)
           {
              BUF_APPEND(I4 "action: FOCUS_SET ");
              BUF_APPENDF("\"%s\";\n", epr->seat);
           }
         else
           BUF_APPEND(I4 "action: FOCUS_SET;\n");
         break;
      }

      case EDJE_ACTION_TYPE_FOCUS_OBJECT:
      {
         if (epr->seat)
           {
              BUF_APPEND(I4 "action: FOCUS_OBJECT ");
              BUF_APPENDF("\"%s\";\n", epr->seat);
           }
         else
           BUF_APPEND(I4 "action: FOCUS_OBJECT;\n");
         break;
      }

      case EDJE_ACTION_TYPE_PARAM_COPY:
      {
         Edje_Real_Part *src_part, *dst_part;

         src_part = ed->table_parts[epr->param.src % ed->table_parts_size];
         dst_part = ed->table_parts[epr->param.dst % ed->table_parts_size];

         if (!src_part || !dst_part) break;

         BUF_APPEND(I4 "action: PARAM_COPY ");
         BUF_APPENDF("\"%s\" \"%s\" \"%s\" \"%s\";\n",
                     src_part->part->name, epr->state,
                     dst_part->part->name, epr->state2);
         break;
      }

      case EDJE_ACTION_TYPE_PARAM_SET:
      {
         Edje_Real_Part *part;

         part = ed->table_parts[epr->param.dst % ed->table_parts_size];

         if (!part) break;

         BUF_APPEND(I4 "action: PARAM_SET ");
         BUF_APPENDF("\"%s\" \"%s\" \"%s\";\n",
                     part->part->name, epr->state, epr->state2);
         break;
       }

      default:
        break;
     }

   /* Transition */
   db = TO_DOUBLE(epr->tween.time);
   tweenmode = (epr->tween.mode & (~(EDJE_TWEEN_MODE_OPT_FROM_CURRENT)));
   switch (tweenmode)
     {
      case EDJE_TWEEN_MODE_LINEAR:
        if (NEQ(db, ZERO))
          BUF_APPENDF(I4 "transition: LINEAR %.5f", db);
        else
          no_transition = EINA_TRUE;
        break;

      case EDJE_TWEEN_MODE_ACCELERATE:
        BUF_APPENDF(I4 "transition: ACCELERATE %.5f", db);
        break;

      case EDJE_TWEEN_MODE_DECELERATE:
        BUF_APPENDF(I4 "transition: DECELERATE %.5f", db);
        break;

      case EDJE_TWEEN_MODE_SINUSOIDAL:
        BUF_APPENDF(I4 "transition: SINUSOIDAL %.5f", db);
        break;

      case EDJE_TWEEN_MODE_ACCELERATE_FACTOR:
        v1 = TO_DOUBLE(epr->tween.v1);
        BUF_APPENDF(I4 "transition: ACCELERATE_FACTOR %.5f %.5f", db, v1);
        break;

      case EDJE_TWEEN_MODE_DECELERATE_FACTOR:
        v1 = TO_DOUBLE(epr->tween.v1);
        BUF_APPENDF(I4 "transition: DECELERATE_FACTOR %.5f %.5f", db, v1);
        break;

      case EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR:
        v1 = TO_DOUBLE(epr->tween.v1);
        BUF_APPENDF(I4 "transition: SINUSOIDAL_FACTOR %.5f %.5f", db, v1);
        break;

      case EDJE_TWEEN_MODE_DIVISOR_INTERP:
        v1 = TO_DOUBLE(epr->tween.v1);
        v2 = TO_DOUBLE(epr->tween.v2);
        BUF_APPENDF(I4 "transition: DIVISOR_INTERP %.5f %.5f %.5f", db, v1, v2);
        break;

      case EDJE_TWEEN_MODE_BOUNCE:
        v1 = TO_DOUBLE(epr->tween.v1);
        v2 = TO_DOUBLE(epr->tween.v2);
        BUF_APPENDF(I4 "transition: BOUNCE %.5f %.5f %.5f", db, v1, v2);
        break;

      case EDJE_TWEEN_MODE_SPRING:
        v1 = TO_DOUBLE(epr->tween.v1);
        v2 = TO_DOUBLE(epr->tween.v2);
        BUF_APPENDF(I4 "transition: SPRING %.5f %.5f %.5f", db, v1, v2);
        break;

      case EDJE_TWEEN_MODE_CUBIC_BEZIER:
        v1 = TO_DOUBLE(epr->tween.v1);
        v2 = TO_DOUBLE(epr->tween.v2);
        v3 = TO_DOUBLE(epr->tween.v3);
        v4 = TO_DOUBLE(epr->tween.v4);
        BUF_APPENDF(I4 "transition: CUBIC_BEZIER %.5f %.5f %.5f %.5f %.5f", db, v1, v2, v3, v4);
        break;

      default:
        no_transition = EINA_TRUE;
        break;
     }

   if (!no_transition)
     {
        if (epr->tween.mode & EDJE_TWEEN_MODE_OPT_FROM_CURRENT)
          BUF_APPENDF(" CURRENT;\n");
        else
          BUF_APPENDF(";\n");
     }
   /* In */
   db = epr->in.from;
   db2 = epr->in.range;
   if (NEQ(db, ZERO) || NEQ(db2, ZERO))
     BUF_APPENDF(I4 "in: %.5f %.5f;\n", db, db2);

   /* Targets */
   if ((ll = _edje_program_targets_get(obj, epr)))
     {
        EINA_LIST_FOREACH(ll, l, data)
          BUF_APPENDF(I4 "target: \"%s\";\n", data);
        edje_edit_string_list_free(ll);
     }

   /* Afters */
   if ((ll = _edje_program_afters_get(obj, epr)))
     {
        EINA_LIST_FOREACH(ll, l, data)
          BUF_APPENDF(I4 "after: \"%s\";\n", data);
        edje_edit_string_list_free(ll);
     }

   // TODO Support script {}
   /* api */
   api_name = eina_stringshare_add(epr->api.name);
   api_description = eina_stringshare_add(epr->api.description);

   if (api_name || api_description)
     {
        if (api_name && api_description)
          {
             BUF_APPENDF(I4 "api: \"%s\" \"%s\";\n", api_name, api_description);
             edje_edit_string_free(api_name);
             edje_edit_string_free(api_description);
          }
        else
        if (api_name)
          {
             BUF_APPENDF(I4 "api: \"%s\" \"\";\n", api_name);
             edje_edit_string_free(api_name);
          }
        else
          {
             BUF_APPENDF(I4 "api: \"\" \"%s\";\n", api_description);
             edje_edit_string_free(api_description);
          }
     }

   BUF_APPEND(I3 "}\n");
   return ret;
}

static void
_edje_source_with_double_values_append(const char *param_name, char val_num, double val1, double val2, Eina_Strbuf *buf, Eina_Bool *ret_value)
{
   Eina_Strbuf *string;
   Eina_Bool ret = EINA_TRUE;

   if ((val_num != 1) && (val_num != 2))
     {
        *ret_value = EINA_FALSE;
        return;
     }

   string = eina_strbuf_new();
   if (param_name)
     eina_strbuf_append_printf(string, "%s:", param_name);
   eina_strbuf_append_printf(string, " %.2f", val1);
   if (val_num == 2)
     eina_strbuf_append_printf(string, " %.2f", val2);
   eina_strbuf_append(string, ";\n");
   BUF_APPEND(eina_strbuf_string_get(string));

   *ret_value = ret;
   eina_strbuf_free(string);
}

#define INHERIT_CHECK(ATTRIBUTE) (pd->ATTRIBUTE != inherit_pd->ATTRIBUTE)

#define INHERIT_CHECK_DOUBLE(ATTRIBUTE_1, ATTRIBUTE_2) ((NEQ(pd->ATTRIBUTE_1, inherit_pd->ATTRIBUTE_1)) || (NEQ(pd->ATTRIBUTE_2, inherit_pd->ATTRIBUTE_2)))

#define INHERIT_CHECK_STRING(ATTRIBUTE_STR) (strcmp(inherit_pd->ATTRIBUTE_STR, pd->ATTRIBUTE_STR))

static void
_edje_generate_source_state_relative(Edje *ed,
                                     Edje_Part_Description_Common *pd,
                                     Edje_Part_Description_Common *inherit_pd,
                                     Eina_Strbuf *buf)
{

   Eina_Bool ret = EINA_TRUE;
   int attr_amount;
   int indent_space = strlen(I6);

   Eina_Bool relative = EINA_FALSE;
   Eina_Bool offset = EINA_FALSE;
   Eina_Bool rel_to = EINA_FALSE;
   Eina_Bool rel_to_x = EINA_FALSE;
   Eina_Bool rel_to_y = EINA_FALSE;

   if (inherit_pd)
     {
         relative = !INHERIT_CHECK_DOUBLE(rel1.relative_x, rel1.relative_y) ? EINA_FALSE : EINA_TRUE;
         offset = !INHERIT_CHECK_DOUBLE(rel1.offset_x, rel1.offset_y) ? EINA_FALSE : EINA_TRUE;
         if ((pd->rel1.id_x != inherit_pd->rel1.id_x) || (pd->rel1.id_y != inherit_pd->rel1.id_y))
           {
              if (pd->rel1.id_x == inherit_pd->rel1.id_x && pd->rel1.id_y != inherit_pd->rel1.id_y)
                {
                   rel_to = 1;
                   rel_to_y = 1;
                }
              else if (pd->rel1.id_x != inherit_pd->rel1.id_x && pd->rel1.id_y == inherit_pd->rel1.id_y)
                {
                   rel_to = 1;
                   rel_to_x = 1;
                }
              else if (pd->rel1.id_x == pd->rel1.id_y && pd->rel1.id_x != -1)
                {
                   rel_to = 1;
                   rel_to_x = EINA_FALSE;
                   rel_to_y = EINA_FALSE;
                }
              else
                {
                   rel_to = 2;
                   rel_to_x = 1;
                   rel_to_y = 1;
                }
           }

     }
   else
     {
        relative = (EQ(pd->rel1.relative_x, ZERO) && EQ(pd->rel1.relative_y, ZERO)) ? EINA_FALSE : EINA_TRUE;
        offset = ((pd->rel1.offset_x == 0) && (pd->rel1.offset_y == 0)) ? EINA_FALSE : EINA_TRUE;
        if ((pd->rel1.id_x != -1) || (pd->rel1.id_y != -1))
          {
             if (pd->rel1.id_x == -1 && pd->rel1.id_y != -1)
               {
                  rel_to = 1;
                  rel_to_y = 1;
               }
             else if (pd->rel1.id_x != -1 && pd->rel1.id_y == -1)
               {
                  rel_to = 1;
                  rel_to_x = 1;
               }
             else if (pd->rel1.id_x == pd->rel1.id_y && pd->rel1.id_x != -1)
               {
                  rel_to = 1;
                  rel_to_x = EINA_FALSE;
                  rel_to_y = EINA_FALSE;
               }
             else
               {
                  rel_to = 2;
                  rel_to_x = 1;
                  rel_to_y = 1;
               }
          }
    }

   attr_amount = relative + offset + rel_to;

   indent_space = strlen(I6);
   if (attr_amount == 1)
     indent_space = 0;

   //Rel1
   if (attr_amount)
     {
        if (attr_amount > 1)
          BUF_APPEND(I5 "rel1 {\n");
        else
          BUF_APPEND(I5 "rel1.");

        if (relative)
          {
             char relative_str[strlen("relative") + indent_space + 1];
             snprintf(relative_str, strlen("relative") + indent_space + 1,
                      "%*srelative", indent_space, "");
             _edje_source_with_double_values_append(relative_str, 2,
                                                  TO_DOUBLE(pd->rel1.relative_x),
                                                  TO_DOUBLE(pd->rel1.relative_y),
                                                  buf, &ret);
          }

        if (offset)
          BUF_APPENDF("%*soffset: %d %d;\n", indent_space, "",
                      pd->rel1.offset_x, pd->rel1.offset_y);

        if (rel_to != 0)
          {
            if (rel_to_x == 0 && rel_to_y == 0)
              {
                 BUF_APPENDF("%*sto: \"%s\";\n", indent_space, "",
                             pd->rel1.id_x == -1 ? "" :  ed->table_parts[pd->rel1.id_x]->part->name);
              }
            if (rel_to_x == 1)
              {
                 BUF_APPENDF("%*sto_x: \"%s\";\n", indent_space, "",
                            pd->rel1.id_x == -1 ? "" :ed->table_parts[pd->rel1.id_x]->part->name);
              }

            if (rel_to_y == 1)
              {
                BUF_APPENDF("%*sto_y: \"%s\";\n", indent_space, "",
                           pd->rel1.id_y == -1 ? "" : ed->table_parts[pd->rel1.id_y]->part->name);
             }
          }
        if (attr_amount > 1)
           BUF_APPEND(I5 "}\n");
     }

   //Rel 2
   relative = EINA_FALSE;
   offset = EINA_FALSE;
   rel_to = EINA_FALSE;
   rel_to_x = EINA_FALSE;
   rel_to_y = EINA_FALSE;
   if (inherit_pd)
     {
         relative = !INHERIT_CHECK_DOUBLE(rel2.relative_x, rel2.relative_y) ? EINA_FALSE : EINA_TRUE;
         offset = !INHERIT_CHECK_DOUBLE(rel2.offset_x, rel2.offset_y) ? EINA_FALSE : EINA_TRUE;
         if ((pd->rel2.id_x != inherit_pd->rel2.id_x) || (pd->rel2.id_y != inherit_pd->rel2.id_y))
           {
              if (pd->rel2.id_x == inherit_pd->rel2.id_x && pd->rel2.id_y != inherit_pd->rel2.id_y)
                {
                   rel_to = 1;
                   rel_to_y = 1;
                }
              else if (pd->rel2.id_x != inherit_pd->rel2.id_x && pd->rel2.id_y == inherit_pd->rel2.id_y)
                {
                   rel_to = 1;
                   rel_to_x = 1;
                }
              else if (pd->rel2.id_x == pd->rel2.id_y && pd->rel2.id_x != -1)
                {
                   rel_to = 1;
                   rel_to_x = EINA_FALSE;
                   rel_to_y = EINA_FALSE;
                }
              else
                {
                   rel_to = 2;
                   rel_to_x = 1;
                   rel_to_y = 1;
                }
           }

     }
   else
     {
        relative = (EQ(pd->rel2.relative_x, FROM_INT(1)) && EQ(pd->rel2.relative_y, FROM_INT(1))) ? EINA_FALSE : EINA_TRUE;
        offset = ((pd->rel2.offset_x == -1) && (pd->rel2.offset_y == -1)) ? EINA_FALSE : EINA_TRUE;
        if ((pd->rel2.id_x != -1) || (pd->rel2.id_y != -1))
          {
             if (pd->rel2.id_x == -1 && pd->rel2.id_y != -1)
               {
                  rel_to = 1;
                  rel_to_y = 1;
               }
             else if (pd->rel2.id_x != -1 && pd->rel2.id_y == -1)
               {
                  rel_to = 1;
                  rel_to_x = 1;
               }
             else if (pd->rel2.id_x == pd->rel2.id_y && pd->rel2.id_x != -1)
               {
                  rel_to = 1;
                  rel_to_x = EINA_FALSE;
                  rel_to_y = EINA_FALSE;
               }
             else
               {
                  rel_to = 2;
                  rel_to_x = 1;
                  rel_to_y = 1;
               }
          }
    }

   attr_amount = relative + offset + rel_to;

   indent_space = strlen(I6);
   if (attr_amount == 1)
     indent_space = 0;

   if (attr_amount)
     {
        if (attr_amount > 1)
          BUF_APPEND(I5 "rel2 {\n");
        else
          BUF_APPEND(I5 "rel2.");

        if (relative)
          {
             char relative_str[strlen("relative") + indent_space + 1];
             snprintf(relative_str, strlen("relative") + indent_space + 1,
                      "%*srelative", indent_space, "");
             _edje_source_with_double_values_append(relative_str, 2,
                                                  TO_DOUBLE(pd->rel2.relative_x),
                                                  TO_DOUBLE(pd->rel2.relative_y),
                                                  buf, &ret);
          }

        if (offset)
          BUF_APPENDF("%*soffset: %d %d;\n", indent_space, "",
                      pd->rel2.offset_x, pd->rel2.offset_y);

        if (rel_to != 0)
          {
            if (rel_to_x == 0 && rel_to_y == 0 )
              {
                 BUF_APPENDF("%*sto: \"%s\";\n", indent_space, "",
                              pd->rel2.id_x == -1 ? "" : ed->table_parts[pd->rel2.id_x]->part->name);
              }
            if (rel_to_x == 1)
              {
                 BUF_APPENDF("%*sto_x: \"%s\";\n", indent_space, "",
                            pd->rel2.id_x == -1 ? "" : ed->table_parts[pd->rel2.id_x]->part->name);
              }

            if (rel_to_y == 1)
              {
                BUF_APPENDF("%*sto_y: \"%s\";\n", indent_space, "",
                           pd->rel2.id_y == -1 ? "" : ed->table_parts[pd->rel2.id_y]->part->name);
             }
          }
        if (attr_amount > 1)
           BUF_APPEND(I5 "}\n");
     }
}

static void
_edje_generate_source_state_image(Edje_Edit *eed, Evas_Object *obj,
                                  const char *part, const char *state, double value,
                                  Edje_Part_Description_Common *pd,
                                  Edje_Part_Description_Common *inherit_pd,
                                  Eina_Strbuf *buf)
{
   Eina_Bool ret = EINA_TRUE;
   Eina_List *l, *ll;
   int attr_amount = 0;
   int indent_space = strlen(I6);
   char *data;
   unsigned int i;

   Eina_Bool name = EINA_FALSE;
   Eina_Bool border = EINA_FALSE;
   Eina_Bool border_scale_by = EINA_FALSE;
   Eina_Bool scale_hint = EINA_FALSE;
   Eina_Bool border_no_fill = EINA_FALSE;
   Eina_Bool border_scale = EINA_FALSE;
   Eina_Bool tweens = EINA_FALSE;

   Edje_Part_Description_Image *img;
   img = (Edje_Part_Description_Image *)pd;
   Edje_Part_Description_Image *inherit_pd_img = (Edje_Part_Description_Image *)inherit_pd;


   if (inherit_pd)
     {
        if (img->image.id != -1)
          name = (img->image.id == inherit_pd_img->image.id) ? EINA_FALSE : EINA_TRUE;

        border = ((img->image.border.l == inherit_pd_img->image.border.l) &&
                  (img->image.border.r == inherit_pd_img->image.border.r) &&
                  (img->image.border.t == inherit_pd_img->image.border.t) &&
                  (img->image.border.b == inherit_pd_img->image.border.b)) ? EINA_FALSE : EINA_TRUE;

        border_scale_by = EQ(img->image.border.scale_by, inherit_pd_img->image.border.scale_by) ? EINA_FALSE : EINA_TRUE;

        scale_hint = EQ(img->image.scale_hint, inherit_pd_img->image.scale_hint) ? EINA_FALSE : EINA_TRUE;

        border_no_fill = (img->image.border.no_fill == inherit_pd_img->image.border.no_fill) ? EINA_FALSE : EINA_TRUE;

        border_scale = (img->image.border.scale == inherit_pd_img->image.border.scale) ? EINA_FALSE : EINA_TRUE;

        if (img->image.tweens_count == inherit_pd_img->image.tweens_count)
          {
             for (i = 0; i < img->image.tweens_count; i++)
               {
                  if (img->image.tweens[i]->id == inherit_pd_img->image.tweens[i]->id)
                    continue;
                  else
                    {
                       tweens = EINA_TRUE;
                       break;
                    }
               }
          }
     }
   else
     {
        name = (img->image.id == -1) ? EINA_FALSE : EINA_TRUE;
        border = (img->image.border.l == 0 && img->image.border.r == 0 &&
                  img->image.border.t == 0 && img->image.border.b == 0) ? EINA_FALSE : EINA_TRUE;
        border_scale_by = EQ(img->image.border.scale_by, ZERO) ? EINA_FALSE : EINA_TRUE;
        scale_hint = (img->image.scale_hint == EVAS_IMAGE_SCALE_HINT_NONE) ? EINA_FALSE : EINA_TRUE;
        border_no_fill = (img->image.border.no_fill == 0) ? EINA_FALSE : EINA_TRUE;
        border_scale = (img->image.border.scale == 0) ? EINA_FALSE : EINA_TRUE;
        tweens = (img->image.tweens_count == 0) ? EINA_FALSE : EINA_TRUE;
     }


   attr_amount =  name + border + border_scale_by + scale_hint + border_no_fill + border_scale + tweens;

   if (attr_amount == 0) goto fill;
   if (attr_amount == 1)
     indent_space = 0;

   if (attr_amount > 1)
      BUF_APPEND(I5 "image {\n");
   else
      BUF_APPEND(I5 "image.");

   if (name)
     {
        if (img->image.set)
          {
             BUF_APPENDF("%*snormal: \"%s\";\n", indent_space, "", _edje_set_name_find(eed, img->image.id));
          }
        else
          {
             BUF_APPENDF("%*snormal: \"%s\";\n", indent_space, "", _edje_image_name_find(eed, img->image.id));
          }
     }

   if (tweens)
     {
        ll = edje_edit_state_tweens_list_get(obj, part, state, value);
        EINA_LIST_FOREACH(ll, l, data)
           BUF_APPENDF("%*stween: \"%s\";\n", indent_space, "", data);
        edje_edit_string_list_free(ll);
     }

   if (border)
     BUF_APPENDF("%*sborder: %d %d %d %d;\n", indent_space, "",
                 img->image.border.l, img->image.border.r,
                 img->image.border.t, img->image.border.b);

   if (border_scale_by)
     {
        char border_scale_by_str[strlen("border_scale_by") + indent_space + 1];
        snprintf(border_scale_by_str, strlen("border_scale_by") + indent_space + 1,
                      "%*sborder_scale_by", indent_space, "");
        _edje_source_with_double_values_append(border_scale_by_str, 1,
                                              TO_DOUBLE(img->image.border.scale_by),
                                              0.0, buf, &ret);
     }

   if (border_scale)
     BUF_APPENDF("%*sborder_scale: 1;\n", indent_space, "");

   if (scale_hint && img->image.scale_hint == EVAS_IMAGE_SCALE_HINT_DYNAMIC)
     {
        BUF_APPENDF("%*sscale_hint: DYNAMIC;\n", indent_space, "");
     }
   else if (scale_hint && img->image.scale_hint == EVAS_IMAGE_SCALE_HINT_STATIC)
     {
        BUF_APPENDF("%*sscale_hint: STATIC;\n", indent_space, "");
     }

   if (border_no_fill && img->image.border.no_fill == 1)
     {
       BUF_APPENDF("%*smiddle: NONE;\n", indent_space, "");
     }
   else if (border_no_fill && img->image.border.no_fill == 2)
     {
       BUF_APPENDF("%*smiddle: SOLID;\n", indent_space, "");
     }

   if (attr_amount > 1)
     BUF_APPEND(I5 "}\n"); //image

   //Fill
fill:
   attr_amount = 0;
   int attr_orig_amount = 0;
   int attr_size_amount = 0;

   Eina_Bool smooth = EINA_FALSE;
   Eina_Bool type = EINA_FALSE;
   Eina_Bool orig_rel = EINA_FALSE;
   Eina_Bool orig_abs = EINA_FALSE;
   Eina_Bool size_rel = EINA_FALSE;
   Eina_Bool size_abs = EINA_FALSE;

   if (inherit_pd_img)
     {
        smooth = (inherit_pd_img->image.fill.smooth == img->image.fill.smooth) ? EINA_FALSE : EINA_TRUE;
        type = (inherit_pd_img->image.fill.type == img->image.fill.type) ? EINA_FALSE : EINA_TRUE;
        orig_rel = (EQ(inherit_pd_img->image.fill.pos_rel_x, img->image.fill.pos_rel_x) &&
                    EQ(inherit_pd_img->image.fill.pos_rel_y, img->image.fill.pos_rel_y)) ? EINA_FALSE : EINA_TRUE;
        orig_abs = ((inherit_pd_img->image.fill.pos_abs_x == img->image.fill.pos_abs_x) &&
                    (inherit_pd_img->image.fill.pos_abs_y == img->image.fill.pos_abs_y)) ? EINA_FALSE : EINA_TRUE;

        size_rel = (EQ(inherit_pd_img->image.fill.rel_x, img->image.fill.rel_x) &&
                    EQ(inherit_pd_img->image.fill.rel_y, img->image.fill.rel_y)) ? EINA_FALSE : EINA_TRUE;
        size_abs = ((inherit_pd_img->image.fill.abs_x == img->image.fill.abs_x) &&
                    (inherit_pd_img->image.fill.abs_y == img->image.fill.abs_y)) ? EINA_FALSE : EINA_TRUE;
     }
   else
     {
        smooth = (img->image.fill.smooth == 1) ? EINA_FALSE : EINA_TRUE;
        type = (img->image.fill.type == EDJE_FILL_TYPE_SCALE) ? EINA_FALSE : EINA_TRUE;
        orig_rel = (EQ(img->image.fill.pos_rel_x, ZERO) && EQ(img->image.fill.pos_rel_y, ZERO)) ? EINA_FALSE : EINA_TRUE;
        orig_abs = ((img->image.fill.pos_abs_x == 0) && (img->image.fill.pos_abs_y == 0)) ? EINA_FALSE : EINA_TRUE;
        size_rel = (EQ(img->image.fill.rel_x, FROM_INT(1)) && EQ(img->image.fill.rel_y, FROM_INT(1))) ? EINA_FALSE : EINA_TRUE;
        size_abs = ((img->image.fill.abs_x == 0) && (img->image.fill.abs_y == 0)) ? EINA_FALSE : EINA_TRUE;
     }

   attr_orig_amount = orig_rel + orig_abs;
   attr_size_amount = size_rel + size_abs;
   attr_amount = smooth + type + attr_orig_amount + attr_size_amount;

   if (attr_amount == 0) return;

   indent_space = 0;
   if (attr_amount > 1 || attr_size_amount || attr_orig_amount)
     indent_space = strlen(I6);

   if (attr_amount)
     {
       if (attr_amount > 1 || attr_size_amount || attr_orig_amount)
         BUF_APPEND(I5 "fill {\n");
       else
         BUF_APPEND(I5 "fill.");

       if (smooth)
         BUF_APPENDF("%*ssmooth: 0;\n", indent_space, "");

       if (type)
         BUF_APPENDF("%*stype: TILE;\n", indent_space, "");

       if (attr_orig_amount)
         {
           indent_space = 0;
           if (attr_orig_amount > 1)
             indent_space = strlen(I7);
           if (attr_orig_amount > 1)
             BUF_APPEND(I6 "origin {\n");
           else
             BUF_APPEND(I6 "origin.");

           if (orig_rel)
             {
               char relative[strlen("relative") + indent_space + 1];
               snprintf(relative, strlen("relative") + indent_space + 1,
                        "%*srelative", indent_space, "");
               _edje_source_with_double_values_append(relative, 2,
                        TO_DOUBLE(img->image.fill.pos_rel_x),
                        TO_DOUBLE(img->image.fill.pos_rel_y),
                        buf, &ret);
             }

           if (orig_abs)
             BUF_APPENDF("%*soffset: %d %d;\n", indent_space, "",
                         img->image.fill.pos_abs_x, img->image.fill.pos_abs_y);

            if (attr_orig_amount > 1)
              BUF_APPEND(I6 "}\n");
         }

       if (attr_size_amount)
         {
           indent_space = 0;
           if (attr_size_amount > 1)
             indent_space = strlen(I7);

           if (attr_size_amount > 1)
             BUF_APPEND(I6 "size {\n");
           else
             BUF_APPEND(I6 "size.");

           if (size_rel)
             {
               char relative[strlen("relative") + indent_space + 1];
               snprintf(relative, strlen("relative") + indent_space + 1,
                        "%*srelative", indent_space, "");
               _edje_source_with_double_values_append(relative, 2,
                        TO_DOUBLE(img->image.fill.rel_x),
                        TO_DOUBLE(img->image.fill.rel_y),
                        buf, &ret);
             }

           if (size_abs)
             BUF_APPENDF("%*soffset: %d %d;\n", indent_space, "",
                         img->image.fill.abs_x, img->image.fill.abs_y);

           if (attr_size_amount > 1)
             BUF_APPEND(I6 "}\n");
         }

       if (attr_amount > 1 || attr_size_amount || attr_orig_amount)
         BUF_APPEND(I5 "}\n");
    }
}

static void
_edje_generate_source_state_map(Edje *ed,
                                Edje_Part_Description_Common *pd,
                                Edje_Part_Description_Common *inherit_pd,
                                Eina_Strbuf *buf)
{
   int attr_amount = 0;
   int attr_rotate_amount = 0;
   int indent_space = 0;
   unsigned int i = 0;
   Eina_Bool ret = EINA_FALSE;

   Eina_Bool persp = EINA_FALSE;
   Eina_Bool light = EINA_FALSE;
   Eina_Bool colors_count = EINA_FALSE;
   Eina_Bool backcull = EINA_FALSE;
   Eina_Bool on = EINA_FALSE;
   Eina_Bool persp_on = EINA_FALSE;
   Eina_Bool smooth = EINA_FALSE;
   Eina_Bool alpha = EINA_FALSE;
   Eina_Bool center = 0, x = 0, y = 0, z = EINA_FALSE;

   if (inherit_pd)
     {
        persp = (inherit_pd->map.id_persp == pd->map.id_persp) ? EINA_FALSE : EINA_TRUE;
        light = (inherit_pd->map.id_light == pd->map.id_light) ? EINA_FALSE : EINA_TRUE;
        colors_count = (inherit_pd->map.colors_count == pd->map.colors_count) ? EINA_FALSE : EINA_TRUE;
        backcull = (inherit_pd->map.backcull == pd->map.backcull) ? EINA_FALSE : EINA_TRUE;
        on = (inherit_pd->map.on == pd->map.on) ? EINA_FALSE : EINA_TRUE;
        persp_on = (inherit_pd->map.persp_on == pd->map.persp_on) ? EINA_FALSE : EINA_TRUE;
        smooth = (inherit_pd->map.smooth == pd->map.smooth) ? EINA_FALSE : EINA_TRUE;
        alpha = (inherit_pd->map.alpha == pd->map.alpha) ? EINA_FALSE : EINA_TRUE;

        center = (inherit_pd->map.rot.id_center == pd->map.rot.id_center) ? EINA_FALSE : EINA_TRUE;
        x = EQ(inherit_pd->map.rot.x, pd->map.rot.x) ? EINA_FALSE : EINA_TRUE;
        y = EQ(inherit_pd->map.rot.y, pd->map.rot.y) ? EINA_FALSE : EINA_TRUE;
        z = EQ(inherit_pd->map.rot.z, pd->map.rot.z) ? EINA_FALSE : EINA_TRUE;
     }
   else
     {
        persp = (pd->map.id_persp == -1) ? EINA_FALSE : EINA_TRUE;
        light = (pd->map.id_light == -1) ? EINA_FALSE : EINA_TRUE;
        colors_count = (pd->map.colors_count == 0) ? EINA_FALSE : EINA_TRUE;
        backcull = (pd->map.backcull == EINA_FALSE) ? EINA_FALSE : EINA_TRUE;
        on = (pd->map.on == EINA_FALSE) ? EINA_FALSE : EINA_TRUE;
        persp_on = (pd->map.persp_on == EINA_FALSE) ? EINA_FALSE : EINA_TRUE;
        smooth = (pd->map.smooth == EINA_TRUE) ? EINA_FALSE : EINA_TRUE;
        alpha = (pd->map.alpha == EINA_TRUE) ? EINA_FALSE : EINA_TRUE;

        center = (pd->map.rot.id_center == -1) ? EINA_FALSE : EINA_TRUE;
        x = EQ(pd->map.rot.x, ZERO) ? EINA_FALSE : EINA_TRUE;
        y = EQ(pd->map.rot.y, ZERO) ? EINA_FALSE : EINA_TRUE;
        z = EQ(pd->map.rot.z, ZERO) ? EINA_FALSE : EINA_TRUE;
     }

   attr_amount  = persp + light + colors_count + backcull + on + persp_on + smooth + alpha;
   attr_rotate_amount = center + x + y + z;

   if (attr_rotate_amount > 0)
     attr_amount += 2;

   if (attr_amount == 0) return;

   if (attr_amount > 1)
     indent_space = strlen(I6);

   if (attr_amount)
     {
        if (attr_amount > 1)
          BUF_APPEND(I5 "map {\n");
        else
          BUF_APPEND(I5 "map.");

        if (persp)
          BUF_APPENDF("%*sperspective: \"%s\";\n", indent_space, "",
                      (_edje_part_name_find(ed, pd->map.id_persp) == NULL ? "" : _edje_part_name_find(ed, pd->map.id_persp)));

        if (light)
          BUF_APPENDF("%*slight: \"%s\";\n", indent_space, "",
                      _edje_part_name_find(ed, pd->map.id_light));

        if (backcull)
          BUF_APPENDF("%*sbackface_cull: 1;\n", indent_space, "");

        if (on)
          BUF_APPENDF("%*son: 1;\n", indent_space, "");

        if (persp_on)
          BUF_APPENDF("%*sperspective_on: 1;\n", indent_space, "");

        if (smooth)
          BUF_APPENDF("%*ssmooth: 0;\n", indent_space, "");

        if (alpha)
          BUF_APPENDF("%*salpha: 0;\n", indent_space, "");

        if (colors_count)
          {
             for (i = 0; i < pd->map.colors_count; ++i)
               {
                  if ((pd->map.colors[i]->r != 255) || (pd->map.colors[i]->g != 255) ||
                      (pd->map.colors[i]->b != 255) || (pd->map.colors[i]->a != 255))
                    BUF_APPENDF(I6 "color: %d %d %d %d %d;\n", pd->map.colors[i]->idx,
                                pd->map.colors[i]->r, pd->map.colors[i]->g,
                                pd->map.colors[i]->b, pd->map.colors[i]->a);
               }
          }

        if (attr_rotate_amount)
          {
             if (attr_rotate_amount > 1)
               {
                 BUF_APPEND(I6 "rotation {\n");
                 indent_space = strlen(I7);
               }
             else
               {
                  BUF_APPENDF(I6 "rotation.");
                  indent_space = 0;
               }

             if (center)
               BUF_APPENDF("%*scenter: \"%s\";\n", indent_space, "",
                           (_edje_part_name_find(ed, pd->map.rot.id_center) == NULL ? "" :
                            _edje_part_name_find(ed, pd->map.rot.id_center)));

             if (x)
               {
                 char rot_x[strlen("x") + indent_space + 1];
                 snprintf(rot_x, strlen("x") + indent_space + 1,
                          "%*sx", indent_space, "");
                 _edje_source_with_double_values_append(rot_x, 1,
                                                   TO_DOUBLE(pd->map.rot.x),
                                                   0.0, buf, &ret);
               }
             if (y)
               {
                 char rot_y[strlen("y") + indent_space + 1];
                 snprintf(rot_y, strlen("y") + indent_space + 1,
                          "%*sy", indent_space, "");
                 _edje_source_with_double_values_append(rot_y, 1,
                                                   TO_DOUBLE(pd->map.rot.y),
                                                   0.0, buf, &ret);
               }
             if (z)
               {
                 char rot_z[strlen("z") + indent_space + 1];
                 snprintf(rot_z, strlen("z") + indent_space + 1,
                          "%*sz", indent_space, "");
                 _edje_source_with_double_values_append(rot_z, 1,
                                                    TO_DOUBLE(pd->map.rot.z),
                                                    0.0, buf, &ret);
               }

             if (attr_rotate_amount > 1)
               BUF_APPEND(I6 "}\n");
          }

        if (attr_amount > 1)
          BUF_APPEND(I5 "}\n");
     }
}

static void
_edje_generate_source_state_proxy(Edje *ed, Edje_Part_Description_Common *pd,
                                  Edje_Part_Description_Common *inherit_pd,
                                  Eina_Strbuf *buf)
{
   int attr_amount = 0;
   int indent_space = 0;
   Eina_Bool ret = 1;
   Edje_Part_Description_Proxy *pro = (Edje_Part_Description_Proxy *)pd;
   Edje_Part_Description_Proxy *inherit_pd_pro = (Edje_Part_Description_Proxy *) inherit_pd;

   int source_visible = EINA_FALSE;
   int source_clip = EINA_FALSE;
   int source = EINA_FALSE;

   if (inherit_pd_pro)
     {
        source = (inherit_pd_pro->proxy.id == pro->proxy.id) ? EINA_FALSE : EINA_TRUE;

        source_visible = (inherit_pd_pro->proxy.source_visible == pro->proxy.source_visible) ? EINA_FALSE : EINA_TRUE;
        source_clip = (inherit_pd_pro->proxy.source_clip == pro->proxy.source_clip) ? EINA_FALSE : EINA_TRUE;
     }
   else
     {
        source = (pro->proxy.id == -1) ? EINA_FALSE : EINA_TRUE;
        source_visible = (pro->proxy.source_visible == 1) ? EINA_FALSE : EINA_TRUE;
        source_clip = (pro->proxy.source_clip == 1) ? EINA_FALSE : EINA_TRUE;
     }

   if (source)
     BUF_APPENDF(I5 "source: \"%s\";\n", (pro->proxy.id >= 0) ? _edje_part_name_find(ed, pro->proxy.id) : "");

   attr_amount = source_visible + source_clip;
   if (!attr_amount) goto fill_proxy;
   if (attr_amount > 1)
     indent_space = strlen(I6);

   //Proxy block
   if (attr_amount)
     {
        if (attr_amount > 1)
          BUF_APPEND(I5 "proxy {\n");
        else
          BUF_APPEND(I5 "proxy.");

        if (source_visible)
          BUF_APPENDF("%*ssource_visible: 0;\n", indent_space, "");

        if (source_clip)
          BUF_APPENDF("%*ssource_clip: 0;\n", indent_space, "");

        if (attr_amount > 1)
          BUF_APPEND(I5 "}\n");
     }

fill_proxy:
   attr_amount = 0;

   //Fill
   int attr_orig_amount = 0;
   int attr_size_amount = 0;

   Eina_Bool smooth = EINA_FALSE;
   Eina_Bool type = EINA_FALSE;
   Eina_Bool orig_rel = EINA_FALSE;
   Eina_Bool orig_abs = EINA_FALSE;
   Eina_Bool size_rel = EINA_FALSE;
   Eina_Bool size_abs = EINA_FALSE;

   if (inherit_pd_pro)
     {
        smooth = (inherit_pd_pro->proxy.fill.smooth == pro->proxy.fill.smooth) ? EINA_FALSE : EINA_TRUE;
        type = (inherit_pd_pro->proxy.fill.type == pro->proxy.fill.type) ? EINA_FALSE : EINA_TRUE;
        orig_rel = (EQ(inherit_pd_pro->proxy.fill.pos_rel_x, pro->proxy.fill.pos_rel_x) &&
                    EQ(inherit_pd_pro->proxy.fill.pos_rel_y, pro->proxy.fill.pos_rel_y)) ? EINA_FALSE : EINA_TRUE;
        orig_abs = ((inherit_pd_pro->proxy.fill.pos_abs_x == pro->proxy.fill.pos_abs_x) &&
                    (inherit_pd_pro->proxy.fill.pos_abs_y == pro->proxy.fill.pos_abs_y)) ? EINA_FALSE : EINA_TRUE;

        size_rel = (EQ(inherit_pd_pro->proxy.fill.rel_x, pro->proxy.fill.rel_x) &&
                    EQ(inherit_pd_pro->proxy.fill.rel_y, pro->proxy.fill.rel_y)) ? EINA_FALSE : EINA_TRUE;
        size_abs = ((inherit_pd_pro->proxy.fill.abs_x == pro->proxy.fill.abs_x) &&
                    (inherit_pd_pro->proxy.fill.abs_y == pro->proxy.fill.abs_y)) ? EINA_FALSE : EINA_TRUE;
     }
   else
     {
        smooth = (pro->proxy.fill.smooth == 1) ? EINA_FALSE : EINA_TRUE;
        type = (pro->proxy.fill.type == EDJE_FILL_TYPE_SCALE) ? EINA_FALSE : EINA_TRUE;
        orig_rel = (EQ(pro->proxy.fill.pos_rel_x, ZERO) && EQ(pro->proxy.fill.pos_rel_y, ZERO)) ? EINA_FALSE : EINA_TRUE;
        orig_abs = ((pro->proxy.fill.pos_abs_x == 0) && (pro->proxy.fill.pos_abs_y == 0)) ? EINA_FALSE : EINA_TRUE;
        size_rel = (EQ(pro->proxy.fill.rel_x, FROM_INT(1)) && EQ(pro->proxy.fill.rel_y, FROM_INT(1))) ? EINA_FALSE : EINA_TRUE;
        size_abs = ((pro->proxy.fill.abs_x == 0) && (pro->proxy.fill.abs_y == 0)) ? EINA_FALSE : EINA_TRUE;
     }

   attr_orig_amount = orig_rel + orig_abs;
   attr_size_amount = size_rel + size_abs;
   attr_amount = smooth + type + attr_orig_amount + attr_size_amount;

   if (attr_amount == 0) return;

   indent_space = 0;
   if (attr_amount > 1 || attr_size_amount || attr_orig_amount)
     indent_space = strlen(I6);

   if (attr_amount)
     {
       if (attr_amount > 1 || attr_size_amount || attr_orig_amount)
         BUF_APPEND(I5 "fill {\n");
       else
         BUF_APPEND(I5 "fill.");

       if (smooth)
         BUF_APPENDF("%*ssmooth: 0;\n", indent_space, "");

       if (type)
         BUF_APPENDF("%*stype: TILE;\n", indent_space, "");

       if (attr_orig_amount)
         {
           indent_space = 0;
           if (attr_orig_amount > 1)
             indent_space = strlen(I7);
           if (attr_orig_amount > 1)
             BUF_APPEND(I6 "origin {\n");
           else
             BUF_APPEND(I6 "origin.");

           if (orig_rel)
             {
               char relative[strlen("relative") + indent_space + 1];
               snprintf(relative, strlen("relative") + indent_space + 1,
                        "%*srelative", indent_space, "");
               _edje_source_with_double_values_append(relative, 2,
                        TO_DOUBLE(pro->proxy.fill.pos_rel_x),
                        TO_DOUBLE(pro->proxy.fill.pos_rel_y),
                        buf, &ret);
             }

           if (orig_abs)
             BUF_APPENDF("%*soffset: %d %d;\n", indent_space, "",
                         pro->proxy.fill.pos_abs_x, pro->proxy.fill.pos_abs_y);

            if (attr_orig_amount > 1)
              BUF_APPEND(I6 "}\n");
         }

       if (attr_size_amount)
         {
           indent_space = 0;
           if (attr_size_amount > 1)
             indent_space = strlen(I7);

           if (attr_size_amount > 1)
             BUF_APPEND(I6 "size {\n");
           else
             BUF_APPEND(I6 "size.");

           if (size_rel)
             {
               char relative[strlen("relative") + indent_space + 1];
               snprintf(relative, strlen("relative") + indent_space + 1,
                        "%*srelative", indent_space, "");
               _edje_source_with_double_values_append(relative, 2,
                        TO_DOUBLE(pro->proxy.fill.rel_x),
                        TO_DOUBLE(pro->proxy.fill.rel_y),
                        buf, &ret);
             }

           if (size_abs)
             BUF_APPENDF("%*soffset: %d %d;\n", indent_space, "",
                         pro->proxy.fill.abs_x, pro->proxy.fill.abs_y);

           if (attr_size_amount > 1)
             BUF_APPEND(I6 "}\n");
         }

       if (attr_amount > 1 || attr_size_amount || attr_orig_amount)
         BUF_APPEND(I5 "}\n");
    }
}

static void
_edje_generate_source_state_table(Edje_Part_Description_Common *pd,
                                  Edje_Part_Description_Common *inherit_pd,
                                  Eina_Strbuf *buf)
{
   int attr_amount = 0;
   int indent_space = 0;
   Eina_Bool ret = EINA_FALSE;

   Edje_Part_Description_Table *table = (Edje_Part_Description_Table *)pd;
   Edje_Part_Description_Table *inherit_pd_table = (Edje_Part_Description_Table *) inherit_pd;

   Eina_Bool homogeneous = EINA_FALSE;
   Eina_Bool align = EINA_FALSE;
   Eina_Bool padding = EINA_FALSE;
   Eina_Bool min = EINA_FALSE;

   if (inherit_pd_table)
     {
        homogeneous = (inherit_pd_table->table.homogeneous == table->table.homogeneous) ? EINA_FALSE : EINA_TRUE;

        align = (EQ(inherit_pd_table->table.align.x, table->table.align.x) &&
                 EQ(inherit_pd_table->table.align.y, table->table.align.y)) ? EINA_FALSE : EINA_TRUE;

        padding = ((inherit_pd_table->table.padding.x == table->table.padding.x) &&
                   (inherit_pd_table->table.padding.y == table->table.padding.y)) ? EINA_FALSE : EINA_TRUE;

        min = ((inherit_pd_table->table.min.h == table->table.min.h) &&
               (inherit_pd_table->table.min.v == table->table.min.v)) ? EINA_FALSE : EINA_TRUE;
     }
   else
     {
        homogeneous = (table->table.homogeneous == EDJE_OBJECT_TABLE_HOMOGENEOUS_NONE) ? EINA_FALSE : EINA_TRUE;
        align = (EQ(table->table.align.x, FROM_DOUBLE(0.5)) && EQ(table->table.align.y, FROM_DOUBLE(0.5))) ? EINA_FALSE : EINA_TRUE;
        padding = (table->table.padding.x == 0 && table->table.padding.y == 0) ? EINA_FALSE : EINA_TRUE;
        min = (table->table.min.h == 0 && table->table.min.v == 0) ? EINA_FALSE : EINA_TRUE;
     }

   attr_amount = homogeneous + align + padding + min;

   if (attr_amount == 0) return;

   if (attr_amount > 1)
     indent_space = strlen(I6);

   if (attr_amount)
     {
        if (attr_amount > 1)
          BUF_APPEND(I5 "table {\n");
        else
          BUF_APPEND(I5 "table.");

        switch (table->table.homogeneous)
          {
           case EDJE_OBJECT_TABLE_HOMOGENEOUS_TABLE:
           {
              BUF_APPENDF("%*shomogeneous: TABLE;\n", indent_space, "");
              break;
           }

           case EDJE_OBJECT_TABLE_HOMOGENEOUS_ITEM:
           {
              BUF_APPENDF("%*shomogeneous: ITEM;\n", indent_space, "");
              break;
           }
          }

        if (align)
          {
             char align_str[strlen("align") + indent_space + 1];
             snprintf(align_str, strlen("align") + indent_space + 1,
                      "%*salign", indent_space, "");
             _edje_source_with_double_values_append(align_str, 2,
                                     TO_DOUBLE(table->table.align.x),
                                     TO_DOUBLE(table->table.align.y),
                                     buf, &ret);
          }

        if (padding)
          BUF_APPENDF("%*spadding: %d %d;\n", indent_space, "",
                      table->table.padding.x, table->table.padding.y);

        if (min)
          BUF_APPENDF("%*smin: %d %d;\n", indent_space, "",
                      table->table.min.h, table->table.min.v);

        if (attr_amount > 1)
          BUF_APPEND(I5 "}\n");
   }
}

static void
_edje_generate_source_state_box(Edje_Part_Description_Common *pd,
                                Edje_Part_Description_Common *inherit_pd,
                                Eina_Strbuf *buf)
{
   int attr_amount = 0;
   int indent_space = 0;
   Eina_Bool ret = EINA_FALSE;

   Edje_Part_Description_Box *box = (Edje_Part_Description_Box *)pd;
   Edje_Part_Description_Box *inherit_pd_box = (Edje_Part_Description_Box *)inherit_pd;

   Eina_Bool alt_layout = EINA_FALSE;
   Eina_Bool layout = EINA_FALSE;
   Eina_Bool align = EINA_FALSE;
   Eina_Bool padding = EINA_FALSE;
   Eina_Bool min = EINA_FALSE;

   if (inherit_pd_box)
     {
        layout = ((inherit_pd_box->box.layout != NULL && box->box.layout != NULL &&
                  !strcmp(inherit_pd_box->box.layout, box->box.layout)) ||
                  (inherit_pd_box->box.layout == NULL && box->box.layout == NULL)) ? EINA_FALSE : EINA_TRUE;

        alt_layout = ((inherit_pd_box->box.alt_layout != NULL && box->box.alt_layout != NULL &&
                      !strcmp(inherit_pd_box->box.alt_layout, box->box.alt_layout)) ||
                      (inherit_pd_box->box.alt_layout == NULL && box->box.alt_layout == NULL)) ? EINA_FALSE : EINA_TRUE;

        align = (EQ(inherit_pd_box->box.align.x, box->box.align.x) &&
                 EQ(inherit_pd_box->box.align.y, box->box.align.y)) ? EINA_FALSE : EINA_TRUE;

        padding = ((inherit_pd_box->box.padding.x == box->box.padding.x) &&
                   (inherit_pd_box->box.padding.y == box->box.padding.y)) ? EINA_FALSE : EINA_TRUE;

        min = ((inherit_pd_box->box.min.h == box->box.min.h) &&
               (inherit_pd_box->box.min.v == box->box.min.v)) ? EINA_FALSE : EINA_TRUE;
     }
   else
     {
        layout = (box->box.layout == NULL) ? EINA_FALSE : EINA_TRUE;
        alt_layout = (box->box.alt_layout == NULL) ? EINA_FALSE : EINA_TRUE;
        align = EQ(box->box.align.x, FROM_DOUBLE(0.5)) && EQ(box->box.align.y, FROM_DOUBLE(0.5)) ? EINA_FALSE : EINA_TRUE;
        padding = (box->box.padding.x == 0 && box->box.padding.y == 0) ? EINA_FALSE : EINA_TRUE;
        min =  (box->box.min.h == 0 && box->box.min.v == 0) ? EINA_FALSE : EINA_TRUE;
     }

   attr_amount = (layout || alt_layout) + align + padding + min;

   if (attr_amount == 0) return;

   if (attr_amount > 1)
     indent_space = strlen(I6);

   if (attr_amount)
    {
       if (attr_amount > 1)
         BUF_APPEND(I5 "box {\n");
       else
         BUF_APPEND(I5 "box.");

        if (layout && alt_layout)
          BUF_APPENDF("%*slayout: \"%s\" \"%s\";\n", indent_space, "",
                      box->box.layout, box->box.alt_layout);
        else if (!layout && alt_layout)
          BUF_APPENDF("%*slayout: \"horizontal\" \"%s\";\n", indent_space, "",
                      box->box.alt_layout);
        else if (layout && !alt_layout)
          BUF_APPENDF("%*slayout: \"%s\";\n", indent_space, "",
                      box->box.layout);

        if (align)
          {
             char align_str[strlen("align") + indent_space + 1];
             snprintf(align_str, strlen("align") + indent_space + 1,
                      "%*salign", indent_space, "");
             _edje_source_with_double_values_append(align_str, 2,
                                    TO_DOUBLE(box->box.align.x),
                                    TO_DOUBLE(box->box.align.y),
                                    buf, &ret);
          }

        if (padding)
          BUF_APPENDF("%*spadding: %d %d;\n", indent_space, "",
                      box->box.padding.x, box->box.padding.y);

        if (min)
          BUF_APPENDF("%*smin: %d %d;\n", indent_space, "",
                      box->box.min.h, box->box.min.v);

        if (attr_amount > 1)
          BUF_APPEND(I5 "}\n");
   }
}

static void
_edje_generate_source_state_text(Edje *ed, Edje_Part_Description_Common *pd,
                                 Edje_Part_Description_Common *inherit_pd,
                                 Eina_Strbuf *buf)
{
   int attr_amount = 0;
   int indent_space = 0;
   Eina_Bool ret = EINA_TRUE;

   Eina_Bool color_3 = EINA_FALSE;
   Eina_Bool domain = EINA_FALSE;
   Eina_Bool text = EINA_FALSE;
   Eina_Bool font = EINA_FALSE;
   Eina_Bool repch = EINA_FALSE;
   Eina_Bool size = EINA_FALSE;
   Eina_Bool text_class = EINA_FALSE;
   Eina_Bool size_range = EINA_FALSE;
   Eina_Bool fit = EINA_FALSE;
   Eina_Bool min = EINA_FALSE;
   Eina_Bool max = EINA_FALSE;
   Eina_Bool align = EINA_FALSE;
   Eina_Bool source = EINA_FALSE;
   Eina_Bool text_source = EINA_FALSE;
   Eina_Bool ellipsis = EINA_FALSE;
   Eina_Bool style = EINA_FALSE;

   Edje_Part_Description_Text *txt = (Edje_Part_Description_Text *)pd;
   Edje_Part_Description_Text *inherit_pd_txt = (Edje_Part_Description_Text *)inherit_pd;

   if (inherit_pd_txt)
     {
        text = ((edje_string_id_get(&txt->text.text) ==
                 (edje_string_id_get(&inherit_pd_txt->text.text)))) ? EINA_FALSE : EINA_TRUE;

        font = ((edje_string_id_get(&inherit_pd_txt->text.font) ==
                 (edje_string_id_get(&txt->text.font)))) ? EINA_FALSE : EINA_TRUE;

        repch = ((edje_string_id_get(&inherit_pd_txt->text.repch) ==
                  (edje_string_id_get(&txt->text.repch)))) ? EINA_FALSE : EINA_TRUE;

        size = (inherit_pd_txt->text.size == txt->text.size) ? EINA_FALSE : EINA_TRUE;

        text_class = ((inherit_pd_txt->text.text_class == txt->text.text_class) ||
                      ((inherit_pd_txt->text.text_class != NULL) &&
                      (txt->text.text_class != NULL) &&
                      (!strcmp(inherit_pd_txt->text.text_class, txt->text.text_class)))) ? EINA_FALSE : EINA_TRUE;

        domain = ((inherit_pd_txt->text.domain == txt->text.domain) ||
                      ((inherit_pd_txt->text.domain != NULL) &&
                      (txt->text.domain != NULL) &&
                      (!strcmp(inherit_pd_txt->text.domain, txt->text.domain)))) ? EINA_FALSE : EINA_TRUE;

        size_range = ((inherit_pd_txt->text.size_range_max == txt->text.size_range_max) &&
                      (inherit_pd_txt->text.size_range_min == txt->text.size_range_min)) ? EINA_FALSE : EINA_TRUE;

        fit = ((inherit_pd_txt->text.fit_x == txt->text.fit_x) &&
               (inherit_pd_txt->text.fit_y == txt->text.fit_y)) ? EINA_FALSE : EINA_TRUE;

        min = ((inherit_pd_txt->text.min_x == txt->text.min_x) &&
               (inherit_pd_txt->text.min_y == txt->text.min_y)) ? EINA_FALSE : EINA_TRUE;

        max = ((inherit_pd_txt->text.max_x == txt->text.max_x) &&
               (inherit_pd_txt->text.max_y == txt->text.max_y)) ? EINA_FALSE : EINA_TRUE;

        align = (EQ(inherit_pd_txt->text.align.x, txt->text.align.x) &&
                 EQ(inherit_pd_txt->text.align.y, txt->text.align.y)) ? EINA_FALSE : EINA_TRUE;

        source = ((inherit_pd_txt->text.id_source == txt->text.id_source)) ? EINA_FALSE : EINA_TRUE;

        text_source = ((inherit_pd_txt->text.id_text_source == txt->text.id_text_source)) ? EINA_FALSE : EINA_TRUE;


        ellipsis = (EQ(inherit_pd_txt->text.ellipsis, txt->text.ellipsis)) ? EINA_FALSE : EINA_TRUE;

        style = (edje_string_id_get(&inherit_pd_txt->text.style) ==
                 edje_string_id_get(&txt->text.style)) ? EINA_FALSE : EINA_TRUE;

        color_3 = ((inherit_pd_txt->text.color3.r == txt->text.color3.r) &&
                   (inherit_pd_txt->text.color3.g == txt->text.color3.g) &&
                   (inherit_pd_txt->text.color3.b == txt->text.color3.b) &&
                   (inherit_pd_txt->text.color3.a == txt->text.color3.a)) ? EINA_FALSE : EINA_TRUE;

     }
   else
     {
        text = (edje_string_get(&txt->text.text) == NULL) ? EINA_FALSE : EINA_TRUE;
        font = (edje_string_get(&txt->text.font) == NULL) ? EINA_FALSE : EINA_TRUE;
        repch = (edje_string_get(&txt->text.repch) == NULL) ? EINA_FALSE : EINA_TRUE;
        size = (txt->text.size == 0) ? EINA_FALSE : EINA_TRUE;
        text_class = (txt->text.text_class == NULL) ? EINA_FALSE : EINA_TRUE;
        domain = (txt->text.domain == NULL) ? EINA_FALSE : EINA_TRUE;
        size_range = ((txt->text.size_range_min == 0) && (txt->text.size_range_max == 0)) ? EINA_FALSE : EINA_TRUE;
        fit = ((txt->text.fit_x == 0) && (txt->text.fit_y == 0)) ? EINA_FALSE : EINA_TRUE;
        min = ((txt->text.min_x == 0) && (txt->text.min_y == 0)) ? EINA_FALSE : EINA_TRUE;
        max = ((txt->text.max_x == 0) && (txt->text.max_y == 0)) ? EINA_FALSE : EINA_TRUE;
        align = (EQ(txt->text.align.x, FROM_DOUBLE(0.5)) && EQ(txt->text.align.y, FROM_DOUBLE(0.5))) ? EINA_FALSE : EINA_TRUE;
        source = (txt->text.id_source == -1) ? EINA_FALSE : EINA_TRUE;
        text_source = (txt->text.id_text_source == -1) ? EINA_FALSE : EINA_TRUE;
        ellipsis = EQ(txt->text.ellipsis, ZERO) ? EINA_FALSE : EINA_TRUE;
        style = (edje_string_id_get(&txt->text.style) == 0) ? EINA_FALSE : EINA_TRUE;
        color_3 = ((txt->text.color3.r == 0) && (txt->text.color3.g == 0) &&
                   (txt->text.color3.b == 0) && (txt->text.color3.a == 128)) ? EINA_FALSE : EINA_TRUE;
     }

   if (color_3)
     BUF_APPENDF(I5 "color3: %d %d %d %d;\n",
                      txt->text.color3.r, txt->text.color3.g, txt->text.color3.b, txt->text.color3.a);


   attr_amount = text + font + repch + size + text_class + domain +
                 size_range + fit + min + max + align + source + text_source + ellipsis + style;

   if (attr_amount == 0) return;

   if (attr_amount > 1)
     indent_space = strlen(I6);

   if (attr_amount > 1)
     BUF_APPEND(I5 "text {\n");
   else
     BUF_APPEND(I5 "text.");

   if (text)
     {
        if (txt->text.text.id)
          BUF_APPENDF("%*stext: _(\"%s\");\n", indent_space, "", edje_string_id_get(&txt->text.text));
        else if (edje_string_get(&txt->text.text) == NULL)
          BUF_APPENDF("%*stext: \"\";\n", indent_space, "");
        else
          BUF_APPENDF("%*stext: \"%s\";\n", indent_space, "", edje_string_get(&txt->text.text));
     }

   if (font)
     {
        if (txt->text.font.id)
          BUF_APPENDF("%*sfont: _(\"%s\");\n", indent_space, "", edje_string_id_get(&txt->text.font));
        else if (edje_string_get(&txt->text.font) == NULL)
          BUF_APPENDF("%*sfont: \"\";\n", indent_space, "");
        else
          BUF_APPENDF("%*sfont: \"%s\";\n", indent_space, "", edje_string_get(&txt->text.font));
     }

   if (repch)
     BUF_APPENDF("%*srepch: \"%s\";\n", indent_space, "", edje_string_id_get(&txt->text.repch));

   if (size)
     BUF_APPENDF("%*ssize: %d;\n", indent_space, "", txt->text.size);

   if (text_class)
     BUF_APPENDF("%*stext_class: \"%s\";\n", indent_space, "", txt->text.text_class);

   if (domain)
     BUF_APPENDF("%*sdomain: \"%s\";\n", indent_space, "", txt->text.domain);

   if (size_range)
     BUF_APPENDF("%*ssize_range: %d %d;\n", indent_space, "", txt->text.size_range_min, txt->text.size_range_max);

   if (fit)
     BUF_APPENDF("%*sfit: %d %d;\n", indent_space, "", txt->text.fit_x, txt->text.fit_y);

   if (min)
     BUF_APPENDF("%*smin: %d %d;\n", indent_space, "", txt->text.min_x, txt->text.min_y);

   if (max)
     BUF_APPENDF("%*smax: %d %d;\n", indent_space, "", txt->text.max_x, txt->text.max_y);

   if (align)
     {
        char align_str[strlen("align") + indent_space + 1];
        snprintf(align_str, strlen("align") + indent_space + 1,
                 "%*salign", indent_space, "");
        _edje_source_with_double_values_append(align_str, 2,
                                            TO_DOUBLE(txt->text.align.x),
                                            TO_DOUBLE(txt->text.align.y),
                                            buf, &ret);
     }

   if (source)
     BUF_APPENDF("%*ssource: \"%s\";\n", indent_space, "", _edje_part_name_find(ed, txt->text.id_source));

   if (text_source)
     BUF_APPENDF("%*stext_source: \"%s\";\n", indent_space, "", _edje_part_name_find(ed, txt->text.id_text_source));

   if (ellipsis)
     {
        char ellipsis_str[strlen("ellipsis") + indent_space + 1];
        snprintf(ellipsis_str, strlen("ellipsis") + indent_space + 1,
                 "%*sellipsis", indent_space, "");
         _edje_source_with_double_values_append(ellipsis_str, 1,
                                               txt->text.ellipsis,
                                               0.0, buf, &ret);
     }
   if (style)
     BUF_APPENDF("%*sstyle: \"%s\";\n", indent_space, "", edje_string_id_get(&txt->text.style));

   //TODO Filter
   if (attr_amount > 1)
     BUF_APPEND(I5 "}\n");
}

#define COMMON_STATE_ATTRIBUTES_AMOUNT 32
static int
_edje_common_desc_diff_calculate(Edje_Part_Description_Common *ed,
                                 Edje_Part_Description_Common *inherit_pd)
{
   int diffs_amount= 0;

   diffs_amount += (NEQ(ed->align.x, inherit_pd->align.x) || NEQ(ed->align.y, inherit_pd->align.y)) ? 1 : 0;

   diffs_amount += ((ed->minmul.have != inherit_pd->minmul.have) ||
                    NEQ(ed->minmul.w, inherit_pd->minmul.w) ||
                    NEQ(ed->minmul.h, inherit_pd->minmul.h)) ? 1 : 0;
   diffs_amount += ((ed->min.w != inherit_pd->min.w) ||
                    (ed->min.h != inherit_pd->min.h) ||
                    (ed->min.limit != inherit_pd->min.limit)) ? 1 : 0;
   diffs_amount += ((ed->step.x != inherit_pd->step.x) || (ed->step.y != inherit_pd->step.y)) ? 1 : 0;
   diffs_amount += (NEQ(ed->aspect.min, inherit_pd->aspect.min) ||
                    NEQ(ed->aspect.max, inherit_pd->aspect.max) ||
                    (ed->aspect.prefer!= inherit_pd->aspect.prefer)) ? 1 : 0;
   diffs_amount += ((ed->color_class != NULL && inherit_pd->color_class == NULL) ||
                    (ed->color_class == NULL && inherit_pd->color_class != NULL) ||
                    (ed->color_class != NULL && inherit_pd->color_class != NULL &&
                     strcmp(ed->color_class, inherit_pd->color_class))) ? 1 : 0;
   diffs_amount += (ed->clip_to_id != inherit_pd->clip_to_id) ? 1 : 0;
   diffs_amount += ((ed->size_class != NULL && inherit_pd->size_class == NULL) ||
                    (ed->size_class == NULL && inherit_pd->size_class != NULL) ||
                    (ed->size_class != NULL && inherit_pd->size_class != NULL &&
                     strcmp(ed->size_class, inherit_pd->size_class))) ? 1 : 0;

   /*Rel1 block*/
   diffs_amount += (NEQ(ed->rel1.relative_x, inherit_pd->rel1.relative_x) ||
                    NEQ(ed->rel1.relative_y, inherit_pd->rel1.relative_y)) ? 1 : 0;
   diffs_amount += ((ed->rel1.offset_x != inherit_pd->rel1.offset_x) ||
                    (ed->rel1.offset_y != inherit_pd->rel1.offset_y)) ? 1 : 0;
   diffs_amount += (ed->rel1.id_x != inherit_pd->rel1.id_x) ? 1 : 0;
   diffs_amount += (ed->rel1.id_y != inherit_pd->rel1.id_y) ? 1 : 0;
   /*End of Rel1 block*/

   /*Rel2 block*/
   diffs_amount += (NEQ(ed->rel2.relative_x, inherit_pd->rel2.relative_x) ||
                    NEQ(ed->rel2.relative_y, inherit_pd->rel2.relative_y)) ? 1 : 0;
   diffs_amount += ((ed->rel2.offset_x != inherit_pd->rel2.offset_x) ||
                    (ed->rel2.offset_y != inherit_pd->rel2.offset_y)) ? 1 : 0;
   diffs_amount += (ed->rel2.id_x != inherit_pd->rel2.id_x) ? 1 : 0;
   diffs_amount += (ed->rel2.id_y != inherit_pd->rel2.id_y) ? 1 : 0;
   /*End of Rel1 block*/

   diffs_amount += (ed->visible != inherit_pd->visible) ? 1 : 0;

   diffs_amount += (inherit_pd->map.id_persp == ed->map.id_persp) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += (inherit_pd->map.id_light == ed->map.id_light) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += (inherit_pd->map.colors_count == ed->map.colors_count) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += (inherit_pd->map.backcull == ed->map.backcull) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += (inherit_pd->map.on == ed->map.on) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += (inherit_pd->map.persp_on == ed->map.persp_on) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += (inherit_pd->map.smooth == ed->map.smooth) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += (inherit_pd->map.alpha == ed->map.alpha) ? EINA_FALSE : EINA_TRUE;

   diffs_amount += (inherit_pd->map.rot.id_center == ed->map.rot.id_center) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += EQ(inherit_pd->map.rot.x, ed->map.rot.x) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += EQ(inherit_pd->map.rot.y, ed->map.rot.y) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += EQ(inherit_pd->map.rot.z, ed->map.rot.z) ? EINA_FALSE : EINA_TRUE;

   /*In case when colors are equal do not append points into diffs_amount*/
   diffs_amount += ((ed->color.r == inherit_pd->color.r) &&
                    (ed->color.g == inherit_pd->color.g) &&
                    (ed->color.b == inherit_pd->color.g) &&
                    (ed->color.a == inherit_pd->color.a)) ? EINA_FALSE : EINA_TRUE;
   diffs_amount += ((ed->color2.r == inherit_pd->color2.r) &&
                    (ed->color2.g == inherit_pd->color2.g) &&
                    (ed->color2.b == inherit_pd->color2.g) &&
                    (ed->color2.a == inherit_pd->color2.a)) ? EINA_FALSE : EINA_TRUE;

   return diffs_amount;
}

#define TEXT_STATE_ATTRIBUTES_AMOUNT (14 + COMMON_STATE_ATTRIBUTES_AMOUNT)
static int
_edje_text_desc_diff_calculate(Edje_Part_Description_Common *ed, Edje_Part_Description_Common *inherit_pd)
{
   int diffs_amount=  _edje_common_desc_diff_calculate(ed, inherit_pd);

   Edje_Part_Description_Text *ed_text = (Edje_Part_Description_Text *) ed;
   Edje_Part_Description_Text *inherit_pd_text = (Edje_Part_Description_Text *) inherit_pd;

   diffs_amount += ((ed_text->text.color3.r == inherit_pd_text->text.color3.r) &&
                    (ed_text->text.color3.g == inherit_pd_text->text.color3.g) &&
                    (ed_text->text.color3.b == inherit_pd_text->text.color3.g) &&
                    (ed_text->text.color3.a == inherit_pd_text->text.color3.a)) ? EINA_FALSE : EINA_TRUE;

   /*Descriprion specific comparsion */
   diffs_amount += ((ed_text->text.text.str == NULL && inherit_pd_text->text.text.str != NULL) ||
                    (ed_text->text.text.str != NULL && inherit_pd_text->text.text.str == NULL) ||
                    (ed_text->text.text.str != NULL && inherit_pd_text->text.text.str != NULL &&
                     strcmp(ed_text->text.text.str, inherit_pd_text->text.text.str))) ? 1 : 0;

   diffs_amount += ((ed_text->text.domain != NULL && inherit_pd_text->text.domain == NULL) ||
                    (ed_text->text.domain == NULL && inherit_pd_text->text.domain != NULL) ||
                    (ed_text->text.domain != NULL && inherit_pd_text->text.domain != NULL &&
                     strcmp(ed_text->text.domain, inherit_pd_text->text.domain))) ? 1 : 0;

   diffs_amount += ((ed_text->text.text_class != NULL && inherit_pd_text->text.text_class == NULL) ||
                    (ed_text->text.text_class == NULL && inherit_pd_text->text.text_class != NULL) ||
                    (ed_text->text.text_class != NULL && inherit_pd_text->text.text_class != NULL &&
                     strcmp(ed_text->text.text_class, inherit_pd_text->text.text_class))) ? 1 : 0;

   diffs_amount += ((ed_text->text.font.str == NULL && inherit_pd_text->text.font.str != NULL) ||
                    (ed_text->text.font.str != NULL && inherit_pd_text->text.font.str == NULL) ||
                    (ed_text->text.font.str != NULL && inherit_pd_text->text.font.str != NULL &&
                     strcmp(ed_text->text.font.str, inherit_pd_text->text.font.str))) ? 1 : 0;

   diffs_amount += (NEQ(ed_text->text.align.x, inherit_pd_text->text.align.x) ||
                    NEQ(ed_text->text.align.y, inherit_pd_text->text.align.y)) ? 1 : 0;
   diffs_amount += NEQ(ed_text->text.ellipsis, inherit_pd_text->text.ellipsis) ? 1 : 0;
   diffs_amount += (ed_text->text.size != inherit_pd_text->text.size) ? 1 : 0;
   diffs_amount += (ed_text->text.id_source != inherit_pd_text->text.id_source) ? 1 : 0;
   diffs_amount += (ed_text->text.id_text_source != inherit_pd_text->text.id_text_source) ? 1 : 0;
   diffs_amount += ((ed_text->text.fit_x != inherit_pd_text->text.fit_x) ||
                    (ed_text->text.fit_y != inherit_pd_text->text.fit_y)) ? 1 : 0;
   diffs_amount += ((ed_text->text.min_x != inherit_pd_text->text.min_x) ||
                    (ed_text->text.min_y != inherit_pd_text->text.min_y)) ? 1 : 0;
   diffs_amount += ((ed_text->text.max_x != inherit_pd_text->text.max_x) ||
                    (ed_text->text.max_y != inherit_pd_text->text.max_y)) ? 1 : 0;
   diffs_amount += ((ed_text->text.size_range_min != inherit_pd_text->text.size_range_min) ||
                    (ed_text->text.size_range_max != inherit_pd_text->text.size_range_max)) ? 1 : 0;

   return diffs_amount;
}

#define IMAGE_STATE_ATTRIBUTES_AMOUNT (5 + COMMON_STATE_ATTRIBUTES_AMOUNT)
static int
_edje_image_desc_diff_calculate(Edje_Edit *eed,
                                Edje_Part_Description_Common *pd,
                                Edje_Part_Description_Common *inherit_pd)
{
   int diffs_amount = _edje_common_desc_diff_calculate(pd, inherit_pd);

   /*TODO: support tweens */
   Edje_Part_Description_Image *image_pd = (Edje_Part_Description_Image *) pd;
   Edje_Part_Description_Image *inherit_pd_image = (Edje_Part_Description_Image *) inherit_pd;

   const char *image_name = _edje_image_name_find(eed, image_pd->image.id);
   const char *inherit_name = _edje_image_name_find(eed, inherit_pd_image->image.id);
   diffs_amount += ((image_name != NULL) && (inherit_name != NULL) &&
                    (!strcmp(image_name, inherit_name))) ? EINA_FALSE : EINA_TRUE;

   diffs_amount += ((image_pd->image.border.l == inherit_pd_image->image.border.l) &&
             (image_pd->image.border.r == inherit_pd_image->image.border.r) &&
             (image_pd->image.border.t == inherit_pd_image->image.border.t) &&
             (image_pd->image.border.b == inherit_pd_image->image.border.b)) ? EINA_FALSE : EINA_TRUE;

   diffs_amount += (image_pd->image.border.scale == inherit_pd_image->image.border.scale) ? EINA_FALSE : EINA_TRUE;

   diffs_amount += EQ(image_pd->image.border.scale_by, inherit_pd_image->image.border.scale_by) ? EINA_FALSE : EINA_TRUE;

   diffs_amount += (image_pd->image.scale_hint == inherit_pd_image->image.scale_hint) ? EINA_FALSE : EINA_TRUE;

   diffs_amount += (image_pd->image.border.no_fill == inherit_pd_image->image.border.no_fill) ? EINA_FALSE : EINA_TRUE;

   return diffs_amount;
}

static Edje_Part_Description_Common *
_edje_generate_source_of_state_inherit(Edje_Edit *eed EINA_UNUSED, Edje_Part *ep, Edje_Part_Description_Common *pd)
{
/*
 * DIFFERENCE_LEVEL constant describe coefficient of difference.
 * All states pairs, that has the difference coefficient below
 * than this constant, could be inhereted one by one.
 * Value 10 mean that states should has maximum 10% differences in all attributes
 */

#define DIFFERENCE_LEVEL 10
   unsigned int i = 0;
   int diff_min = 99;
   int diff_coeff = 99;
   int diff_amount = 0;

   Edje_Part_Description_Common *inherit = NULL;
   Edje_Part_Description_Common *desc = NULL;

   if (EQ(pd->state.value, ZERO) && !strcmp(pd->state.name, "default"))
     return NULL;

   /*calculate  metric of difference*/
   switch (ep->type)
     {
      /*TODO: add speceific part types description diff calculating*/
      case EDJE_PART_TYPE_TEXT:
         diff_amount = _edje_text_desc_diff_calculate(pd, ep->default_desc);
         diff_coeff = (int)(((100 * diff_amount) / (TEXT_STATE_ATTRIBUTES_AMOUNT)));
         break;
      case EDJE_PART_TYPE_IMAGE:
         diff_amount = _edje_image_desc_diff_calculate(eed, pd, ep->default_desc);
         diff_coeff = (int)(((100 * diff_amount) / (IMAGE_STATE_ATTRIBUTES_AMOUNT)));
         break;
      default:
         diff_amount = _edje_common_desc_diff_calculate(pd, ep->default_desc);
         diff_coeff = (int)(((100 * diff_amount) / (COMMON_STATE_ATTRIBUTES_AMOUNT)));
         break;
     }

  if ((diff_coeff <= DIFFERENCE_LEVEL) && (diff_coeff < diff_min))
     {
        diff_min = diff_coeff;
        inherit = ep->default_desc;
     }

   /*
    * For case when beetwen current state and "default" state little amount of
    * differencies - stop search
    */
   if (inherit && diff_amount <= 1)
     return inherit;

   for (i = 0; i < ep->other.desc_count; i++)
     {
        desc = ep->other.desc[i];

        if (!strcmp(desc->state.name, pd->state.name) && EQ(desc->state.value, pd->state.value))
          break;

        /*calculate  metric of difference*/
         switch (ep->type)
           {
            /*TODO: add speceific part types description diff calculating*/
            case EDJE_PART_TYPE_TEXT:
               diff_coeff = _edje_text_desc_diff_calculate(pd, desc);
               break;
            case EDJE_PART_TYPE_IMAGE:
               diff_coeff = _edje_image_desc_diff_calculate(eed, pd, desc);
               break;
            default:
               diff_coeff = _edje_common_desc_diff_calculate(pd, desc);
               break;
           }
         if ((diff_coeff <= DIFFERENCE_LEVEL) && (diff_coeff < diff_min))
           {
              diff_min = diff_coeff;
              inherit = desc;
           }
     }
   return inherit;
}

static Eina_Bool
_edje_generate_source_of_state(Evas_Object *obj, const char *part, const char *state, double value, Eina_Strbuf *buf)
{
   Eina_List *l, *ll;
   Eina_Bool ret = EINA_TRUE;

   GET_PD_OR_RETURN(EINA_FALSE);

   BUF_APPENDF(I4 "description { state: \"%s\"", pd->state.name);
   _edje_source_with_double_values_append(NULL, 1, pd->state.value, 0, buf, &ret);

   Edje_Part_Description_Common *inherit_pd = _edje_generate_source_of_state_inherit(eed, rp->part, pd);

   Eina_Bool color = EINA_FALSE;
   Eina_Bool color_2 = EINA_FALSE;
   Eina_Bool color_class = EINA_FALSE;
   Eina_Bool align = EINA_FALSE;
   Eina_Bool fixed = EINA_FALSE;
   Eina_Bool size_class = EINA_FALSE;
   Eina_Bool minmul = EINA_FALSE;
   Eina_Bool min = EINA_FALSE, min_source = EINA_FALSE;
   Eina_Bool max = EINA_FALSE, max_source = EINA_FALSE;
   Eina_Bool aspect = EINA_FALSE;
   Eina_Bool aspect_prefer = EINA_FALSE;
   Eina_Bool step = EINA_FALSE;
   Eina_Bool limit = EINA_FALSE;
   Eina_Bool zplane = EINA_FALSE;
   Eina_Bool focal = EINA_FALSE;
   Eina_Bool clip_to = EINA_FALSE;

   if (inherit_pd)
     {
        BUF_APPENDF(I5 "inherit: \"%s\" %.2f;\n", inherit_pd->state.name, inherit_pd->state.value);

        color =  ((pd->color.r == inherit_pd->color.r) &&
                  (pd->color.g == inherit_pd->color.g) &&
                  (pd->color.b == inherit_pd->color.b) &&
                  (pd->color.a == inherit_pd->color.a)) ? EINA_FALSE : EINA_TRUE;

        color_2 =  ((pd->color2.r == inherit_pd->color2.r) &&
                    (pd->color2.g == inherit_pd->color2.g) &&
                    (pd->color2.b == inherit_pd->color2.b) &&
                    (pd->color2.a == inherit_pd->color2.a)) ? EINA_FALSE : EINA_TRUE;

        color_class = ((pd->color_class == inherit_pd->color_class) ||
                       ((pd->color_class != NULL) &&
                        (inherit_pd->color_class != NULL) &&
                        (!strcmp(pd->color_class, inherit_pd->color_class)))) ? EINA_FALSE : EINA_TRUE;

        align = (EQ(pd->align.x, inherit_pd->align.x) &&
                 EQ(pd->align.y, inherit_pd->align.y)) ? EINA_FALSE : EINA_TRUE;

        fixed = ((pd->fixed.w == inherit_pd->fixed.w) &&
                 (pd->fixed.h == inherit_pd->fixed.h)) ? EINA_FALSE : EINA_TRUE;

        min = ((pd->min.w == inherit_pd->min.w) &&
               (pd->min.h == inherit_pd->min.h)) ? EINA_FALSE : EINA_TRUE;

        min_source = (pd->min.limit == inherit_pd->min.limit) ? EINA_FALSE : EINA_TRUE;

        max = ((pd->max.w == inherit_pd->max.w) &&
               (pd->max.h == inherit_pd->max.h)) ? EINA_FALSE : EINA_TRUE;

        max_source = (pd->max.limit == inherit_pd->max.limit) ? EINA_FALSE : EINA_TRUE;

        minmul = ((pd->minmul.have == inherit_pd->minmul.have) &&
                  EQ(pd->minmul.w, inherit_pd->minmul.w) &&
                  EQ(pd->minmul.h, inherit_pd->minmul.h)) ? EINA_FALSE : EINA_TRUE;

        size_class = ((pd->size_class == inherit_pd->size_class) ||
                      ((pd->size_class != NULL) && (inherit_pd->size_class != NULL) &&
                       (!strcmp(pd->size_class, inherit_pd->size_class)))) ? EINA_FALSE : EINA_TRUE;

        step = ((pd->step.x == inherit_pd->step.x) &&
                (pd->step.y == inherit_pd->step.y)) ? EINA_FALSE : EINA_TRUE;

        aspect = (EQ(pd->aspect.min, inherit_pd->aspect.min) &&
                  EQ(pd->aspect.max, inherit_pd->aspect.max)) ? EINA_FALSE : EINA_TRUE;

        aspect_prefer = (pd->aspect.prefer == inherit_pd->aspect.prefer) ? EINA_FALSE : EINA_TRUE;

        limit = (pd->limit == inherit_pd->limit) ? EINA_FALSE : EINA_TRUE;

        zplane = (pd->persp.zplane == inherit_pd->persp.zplane) ? EINA_FALSE : EINA_TRUE;

        focal = (pd->persp.focal == inherit_pd->persp.focal) ? EINA_FALSE : EINA_TRUE;

        clip_to = (pd->clip_to_id == inherit_pd->clip_to_id) ? EINA_FALSE : EINA_TRUE;
     }
   else
     {
        color = ((pd->color.r == 255) && (pd->color.g == 255) &&
                 (pd->color.b == 255) && (pd->color.a == 255)) ? EINA_FALSE : EINA_TRUE;

        color_2 = ((pd->color2.r == 0) && (pd->color2.g == 0) &&
                   (pd->color2.b == 0) && (pd->color2.a == 255)) ? EINA_FALSE : EINA_TRUE;

        color_class = (pd->color_class == NULL) ? EINA_FALSE : EINA_TRUE;

        align = (EQ(pd->align.x, FROM_DOUBLE(0.5)) && EQ(pd->align.y, FROM_DOUBLE(0.5))) ? EINA_FALSE : EINA_TRUE;

        fixed = ((pd->fixed.w == 0) && (pd->fixed.h == 0)) ? EINA_FALSE : EINA_TRUE;

        min = ((pd->min.w == 0) && (pd->min.h == 0)) ? EINA_FALSE : EINA_TRUE;
        min_source = pd->min.limit;

        max = ((pd->max.w == -1) && (pd->max.h == -1)) ? EINA_FALSE : EINA_TRUE;
        max_source = pd->max.limit;

        minmul = ((pd->minmul.have == 0) ||
                  (EQ(pd->minmul.w, FROM_INT(1)) && EQ(pd->minmul.h, FROM_INT(1)))) ? EINA_FALSE : EINA_TRUE;

        size_class = (pd->size_class == NULL) ? EINA_FALSE : EINA_TRUE;

        step = ((pd->step.x == 0) && (pd->step.y == 0)) ? EINA_FALSE : EINA_TRUE;

        aspect = (EQ(pd->aspect.min, ZERO) && EQ(pd->aspect.max, ZERO)) ? EINA_FALSE : EINA_TRUE;

        aspect_prefer = (pd->aspect.prefer == 0) ? EINA_FALSE : EINA_TRUE;

        limit = (pd->limit == EDJE_STATE_LIMIT_NONE) ? EINA_FALSE : EINA_TRUE;

        zplane = (pd->persp.zplane == 0) ? EINA_FALSE : EINA_TRUE;

        focal = (pd->persp.focal == 1000) ? EINA_FALSE : EINA_TRUE;

        clip_to = (pd->clip_to_id == -1) ? EINA_FALSE : EINA_TRUE;
     }

   if (inherit_pd && inherit_pd->visible != pd->visible)
     {
       BUF_APPENDF(I5 "visible: %d;\n", pd->visible);
     }
   else if (!pd->visible)
     {
       BUF_APPEND(I5 "visible: 0;\n");
     }

   if (limit)
     {
        switch (pd->limit)
          {
           case EDJE_STATE_LIMIT_WIDTH:
           {
              BUF_APPEND(I5 "limit: WIDTH;\n");
              break;
           }

           case EDJE_STATE_LIMIT_HEIGHT:
           {
              BUF_APPEND(I5 "limit: HEIGHT;\n");
              break;
           }

           case EDJE_STATE_LIMIT_BOTH:
           {
              BUF_APPEND(I5 "limit: BOTH;\n");
              break;
           }
          }
     }

   if (align)
     _edje_source_with_double_values_append(I5 "align", 2,
                                            TO_DOUBLE(pd->align.x),
                                            TO_DOUBLE(pd->align.y),
                                            buf, &ret);

   if (fixed)
     BUF_APPENDF(I5 "fixed: %d %d;\n", pd->fixed.w, pd->fixed.h);

   if (min && !min_source)
     BUF_APPENDF(I5 "min: %d %d;\n", pd->min.w, pd->min.h);
   else if (min_source)
     BUF_APPEND(I5 "min: SOURCE;\n");

   if (max && !max_source)
     BUF_APPENDF(I5 "max: %d %d;\n", pd->max.w, pd->max.h);
   else if (max_source)
     BUF_APPEND(I5 "max: SOURCE;\n");

   if (minmul)
     _edje_source_with_double_values_append(I5 "minmul", 2,
                                            TO_DOUBLE(pd->minmul.w),
                                            TO_DOUBLE(pd->minmul.h),
                                            buf, &ret);

   if (size_class)
     BUF_APPENDF(I5 "size_class: \"%s\";\n", pd->size_class);

   if (step)
     BUF_APPENDF(I5 "step: %d %d;\n", TO_INT(pd->step.x), TO_INT(pd->step.y));

   if (aspect)
     _edje_source_with_double_values_append(I5 "aspect", 2,
                                            TO_DOUBLE(pd->aspect.min),
                                            TO_DOUBLE(pd->aspect.max),
                                            buf, &ret);

   if (aspect_prefer)
     BUF_APPENDF(I5 "aspect_preference: %s;\n", prefers[(int)pd->aspect.prefer]);

   if (clip_to)
     BUF_APPENDF(I5 "clip_to: \"%s\";\n",
                      (pd->clip_to_id == -1 ? "" : _edje_part_name_find(ed, pd->clip_to_id)));


   if (rp->part->type != EDJE_PART_TYPE_SPACER)
     {
        if (color_class)
          BUF_APPENDF(I5 "color_class: \"%s\";\n",
                      (pd->color_class == NULL ? "" : pd->color_class));

        if (color)
          BUF_APPENDF(I5 "color: %d %d %d %d;\n",
                      pd->color.r, pd->color.g, pd->color.b, pd->color.a);
        if (color_2)
          BUF_APPENDF(I5 "color2: %d %d %d %d;\n",
                      pd->color2.r, pd->color2.g, pd->color2.b, pd->color2.a);
     }

   //Box
   if (rp->part->type == EDJE_PART_TYPE_BOX)
     _edje_generate_source_state_box(pd, inherit_pd, buf);

   //Table
   if (rp->part->type == EDJE_PART_TYPE_TABLE)
     _edje_generate_source_state_table(pd, inherit_pd, buf);

   //Image
   if (rp->part->type == EDJE_PART_TYPE_IMAGE)
     _edje_generate_source_state_image(eed, obj, part, state, value, pd, inherit_pd, buf) ;

   //Proxy
   if (rp->part->type == EDJE_PART_TYPE_PROXY)
     _edje_generate_source_state_proxy(ed, pd, inherit_pd, buf);

   //Text
   if ((rp->part->type == EDJE_PART_TYPE_TEXT) ||
       (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK))
     _edje_generate_source_state_text(ed, pd, inherit_pd, buf);

   //External
   if (rp->part->type == EDJE_PART_TYPE_EXTERNAL)
     {
        if ((ll = (Eina_List *)edje_edit_state_external_params_list_get(obj, part, state, value)))
          {
             Edje_External_Param *p;

             BUF_APPEND(I5 "params {\n");
             EINA_LIST_FOREACH(ll, l, p)
               {
                  switch (p->type)
                    {
                     case EDJE_EXTERNAL_PARAM_TYPE_INT:
                       BUF_APPENDF(I6 "int: \"%s\" \"%d\";\n", p->name, p->i);
                       break;

                     case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
                       BUF_APPENDF(I6 "double: \"%s\" \"%g\";\n", p->name, p->d);
                       break;

                     case EDJE_EXTERNAL_PARAM_TYPE_STRING:
                       if (p->s)
                         BUF_APPENDF(I6 "string: \"%s\" \"%s\";\n", p->name,
                                     p->s);
                       break;

                     case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
                       BUF_APPENDF(I6 "bool: \"%s\" \"%d\";\n", p->name, p->i);
                       break;

                     case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
                       if (p->s)
                         BUF_APPENDF(I6 "choice: \"%s\" \"%s\";\n", p->name,
                                     p->s);
                       break;

                     default:
                       break;
                    }
               }
             BUF_APPEND(I5 "}\n");
          }
     }

   //Relative block
   _edje_generate_source_state_relative(ed, pd, inherit_pd, buf);

   //Map
   _edje_generate_source_state_map(ed, pd, inherit_pd, buf);

   if (zplane || focal)
     {
        if (zplane && focal)
          {
             BUF_APPEND(I5 "perspective {\n");
             BUF_APPENDF(I6 "zplane: %d;\n", pd->persp.zplane);
             BUF_APPENDF(I6 "focal: %d;\n", pd->persp.focal);
             BUF_APPEND(I5 "}\n");
          }
        else
          {
             BUF_APPEND(I5 "perspective.");
             if (zplane)
               BUF_APPENDF("zplane: %d;\n", pd->persp.zplane);
             if (focal)
               BUF_APPENDF("focal: %d;\n", pd->persp.focal);
          }
     }

   BUF_APPEND(I4 "}\n"); //description
   return ret;
}

static Eina_Bool
_edje_generate_source_of_part(Evas_Object *obj, Edje_Part *ep, Eina_Strbuf *buf)
{
   const char *part = ep->name;
   const char *str;
   Eina_List *l, *ll;
   char *data;
   Eina_Bool ret = EINA_TRUE;
   const char *api_name, *api_description;
   Edje_Pack_Element *item;

   GET_RP_OR_RETURN(EINA_FALSE);

   BUF_APPENDF(I3 "part { name: \"%s\";\n", part);
   BUF_APPENDF(I4 "type: %s;\n", types[rp->part->type]);
   if (!rp->part->mouse_events)
     BUF_APPEND(I4 "mouse_events: 0;\n");
   if (rp->part->repeat_events)
     BUF_APPEND(I4 "repeat_events: 1;\n");
   if (rp->part->scale)
     BUF_APPEND(I4 "scale: 1;\n");

   if (rp->part->ignore_flags)
     BUF_APPENDF(I4 "ignore_flags: \"ON_HOLD\";\n");
   if (rp->part->mask_flags)
     BUF_APPENDF(I4 "mask_flags: \"ON_HOLD\";\n");
   if (rp->part->pointer_mode == EVAS_OBJECT_POINTER_MODE_NOGRAB)
     BUF_APPEND(I4 "pointer_mode: NOGRAB;\n");
   if (rp->part->precise_is_inside)
     BUF_APPEND(I4 "precise_is_inside: 1;\n");
   if (rp->part->access)
     BUF_APPEND(I4 "access: 1;\n");
   if (rp->part->allowed_seats)
     {
        Edje_Part_Allowed_Seat *seat;
        unsigned int i;

        BUF_APPEND(I4 "allowed_seats:");
        for (i = 0; i < rp->part->allowed_seats_count; i++)
          {
             seat = rp->part->allowed_seats[i];
             BUF_APPENDF(" %s", seat->name);
          }
        BUF_APPEND(";\n");
     }

   if ((str = _edje_part_clip_to_get(ed, rp)))
     {
        BUF_APPENDF(I4 "clip_to: \"%s\";\n", str);
        edje_edit_string_free(str);
     }
   if ((rp->part->type == EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->type == EDJE_PART_TYPE_TEXT))
     if (rp->part->use_alternate_font_metrics)
       BUF_APPENDF(I4 "use_alternate_font_metrics: 1;\n");

   if ((rp->part->type == EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->type == EDJE_PART_TYPE_GROUP))
     {
        if ((str = eina_stringshare_add(rp->part->source)))
          {
             BUF_APPENDF(I4 "source: \"%s\";\n", str);
             eina_stringshare_del(str);
          }
        if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
          {
             if ((str = eina_stringshare_add(rp->part->source2)))
               {
                  BUF_APPENDF(I4 "source2: \"%s\";\n", str);
                  eina_stringshare_del(str);
               }
             if ((str = eina_stringshare_add(rp->part->source3)))
               {
                  BUF_APPENDF(I4 "source3: \"%s\";\n", str);
                  eina_stringshare_del(str);
               }
             if ((str = eina_stringshare_add(rp->part->source4)))
               {
                  BUF_APPENDF(I4 "source4: \"%s\";\n", str);
                  eina_stringshare_del(str);
               }
             if ((str = eina_stringshare_add(rp->part->source5)))
               {
                  BUF_APPENDF(I4 "source5: \"%s\";\n", str);
                  eina_stringshare_del(str);
               }
             if ((str = eina_stringshare_add(rp->part->source6)))
               {
                  BUF_APPENDF(I4 "source6: \"%s\";\n", str);
                  eina_stringshare_del(str);
               }
             if (rp->part->entry_mode)
               BUF_APPENDF(I4 "entry_mode: \"%s\";\n",
                           entry_mode[rp->part->entry_mode]);
             if (rp->part->select_mode)
               BUF_APPENDF(I4 "select_mode: \"EXPLICIT\";\n");
             if (rp->part->cursor_mode)
               BUF_APPENDF(I4 "cursor_mode: \"BEFORE\";\n");
             if (rp->part->multiline)
               BUF_APPEND(I4 "multiline: 1;\n");
          }
     }

   if (rp->part->effect)
     {
        int effect = rp->part->effect;
        if (effect & EDJE_TEXT_EFFECT_MASK_SHADOW_DIRECTION)
          {
             BUF_APPENDF(I4 "effect: %s %s;\n",
                         effects[effect & ~EDJE_TEXT_EFFECT_MASK_SHADOW_DIRECTION],
                         shadow_direction[effect >> 4]);
          }
        else
          BUF_APPENDF(I4 "effect: %s;\n",
                      effects[effect]);
     }

   //Box
   if ((edje_edit_part_type_get(obj, part) == EDJE_PART_TYPE_BOX) ||
       (edje_edit_part_type_get(obj, part) == EDJE_PART_TYPE_TABLE))
     {
        if (ep->items_count != 0)
          {
             unsigned int i;

             if (edje_edit_part_type_get(obj, part) == EDJE_PART_TYPE_BOX)
               BUF_APPEND(I4 "box {\n");
             else
               BUF_APPEND(I4 "table {\n");
             BUF_APPEND(I5 "items {\n");
             for (i = 0; i < ep->items_count; ++i)
               {
                  item = ep->items[i];
                  BUF_APPEND(I6 "item {\n");
                  BUF_APPENDF(I7 "type: %s;\n", types[item->type]);
                  if (item->name)
                    BUF_APPENDF(I7 "name: \"%s\";\n", item->name);
                  if (item->source)
                    BUF_APPENDF(I7 "source: \"%s\";\n", item->source);
                  if ((item->min.w != 0) || (item->min.h != 0))
                    BUF_APPENDF(I7 "min: %d %d;\n", item->min.w, item->min.h);
                  if ((item->max.w != -1) || (item->max.h != -1))
                    BUF_APPENDF(I7 "max: %d %d;\n", item->max.w, item->max.h);
                  if (item->aspect.mode)
                    BUF_APPENDF(I7 "aspect_mode: \"%s\";\n", aspect_mode[item->aspect.mode]);
                  if ((item->aspect.w != 0) || (item->aspect.h != 0))
                    BUF_APPENDF(I7 "aspect: %d %d;\n", item->aspect.w, item->aspect.h);
                  if ((item->prefer.w != 0) || (item->prefer.h != 0))
                    BUF_APPENDF(I7 "prefer: %d %d;\n", item->prefer.w, item->prefer.h);
                  if ((item->spread.w != 1) || (item->spread.h != 1))
                    BUF_APPENDF(I7 "spread: %d %d;\n", item->spread.w, item->spread.h);
                  if ((item->padding.l != 0) || (item->padding.t != 0) ||
                      (item->padding.r != 0) || (item->padding.b != 0))
                    BUF_APPENDF(I7 "padding: %d %d %d %d;\n",
                                item->padding.l, item->padding.r,
                                item->padding.t, item->padding.b);
                  if (NEQ(item->weight.x, ZERO) || NEQ(item->weight.y, ZERO))
                    _edje_source_with_double_values_append(I7 "weight", 2,
                                                           TO_DOUBLE(item->weight.x),
                                                           TO_DOUBLE(item->weight.y),
                                                           buf, &ret);
                  if (NEQ(item->align.x, FROM_DOUBLE(0.5)) || NEQ(item->align.y, FROM_DOUBLE(0.5)))
                    _edje_source_with_double_values_append(I7 "align", 2,
                                                           TO_DOUBLE(item->align.x),
                                                           TO_DOUBLE(item->align.y),
                                                           buf, &ret);

                  if (edje_edit_part_type_get(obj, part) == EDJE_PART_TYPE_TABLE)
                    BUF_APPENDF(I7 "position: %d %d;\n", item->col, item->row);
                  if ((item->colspan != 1) || (item->rowspan != 1))
                    BUF_APPENDF(I7 "span: %d %d;\n", item->colspan, item->rowspan);

                  //TODO weight
                  //TODO options
                  //TODO col
                  //TODO row

                  BUF_APPEND(I6 "}\n");
               }
             BUF_APPEND(I5 "}\n");
             BUF_APPEND(I4 "}\n");
          }
     }

   //Dragable
   str = edje_edit_part_drag_event_get(obj, part);
   if (rp->part->dragable.x || rp->part->dragable.y || str)
     {
        BUF_APPEND(I4 "dragable {\n");
        if (str)
          {
             BUF_APPENDF(I5 "events: \"%s\";\n", str);
             edje_edit_string_free(str);
          }

        if (rp->part->dragable.x || rp->part->dragable.y)
          {
             BUF_APPENDF(I5 "x: %d %d %d;\n",
                         rp->part->dragable.x,
                         rp->part->dragable.step_x,
                         rp->part->dragable.count_x);
             BUF_APPENDF(I5 "y: %d %d %d;\n",
                         rp->part->dragable.y,
                         rp->part->dragable.step_y,
                         rp->part->dragable.count_y);
             if ((str = edje_edit_part_drag_confine_get(obj, part)))
               {
                  BUF_APPENDF(I5 "confine: \"%s\";\n", str);
                  edje_edit_string_free(str);
               }
             if ((str = edje_edit_part_drag_threshold_get(obj, part)))
               {
                  BUF_APPENDF(I5 "threshold: \"%s\";\n", str);
                  edje_edit_string_free(str);
               }
          }
        BUF_APPEND(I4 "}\n");
     }

   //Descriptions
   ll = edje_edit_part_states_list_get(obj, part);
   EINA_LIST_FOREACH(ll, l, data)
     {
        char state[512], *delim;
        double value;
        strncpy(state, data, sizeof(state) - 1); /* if we go over it, too bad.. the list of states may need to change to provide name and value separated */
        delim = strchr(state, ' ');
        if (!delim) continue;
        *delim = '\0';
        delim++;
        value = strtod(delim, NULL);
        ret &= _edje_generate_source_of_state(obj, part, state, value, buf);
     }
   edje_edit_string_list_free(ll);

   api_name = eina_stringshare_add(rp->part->api.name);
   api_description = eina_stringshare_add(rp->part->api.description);
   if (api_name || api_description)
     {
        if (api_name && api_description)
          {
             BUF_APPENDF(I4 "api: \"%s\" \"%s\";\n", api_name, api_description);
             eina_stringshare_del(api_name);
             eina_stringshare_del(api_description);
          }
        else
        if (api_name)
          {
             BUF_APPENDF(I4 "api: \"%s\" \"\";\n", api_name);
             eina_stringshare_del(api_name);
          }
        else
          {
             BUF_APPENDF(I4 "api: \"\" \"%s\";\n", api_description);
             eina_stringshare_del(api_description);
          }
     }

   BUF_APPEND(I3 "}\n"); //part
   return ret;
}

static void
_edje_generate_source_of_sounds(Edje_Sound_Directory *sound_directory, Eina_Strbuf *buf)
{
   unsigned int i = 0;
   Eina_Bool ret = EINA_TRUE;
   Edje_Sound_Sample *current_sample = sound_directory->samples;
   Edje_Sound_Tone *current_tone = sound_directory->tones;
   BUF_APPEND(I1 "sounds {\n");

   for (i = 0; i < sound_directory->samples_count; i++, current_sample++)
     {
        BUF_APPEND(I2 "sample {\n");
        BUF_APPENDF(I3 "name: \"%s\" ", current_sample->name);
        switch (current_sample->compression)
          {
           case EDJE_SOUND_SOURCE_TYPE_INLINE_RAW:
           {
              BUF_APPEND("RAW;\n");
              break;
           }

           case EDJE_SOUND_SOURCE_TYPE_INLINE_COMP:
           {
              BUF_APPEND("COMP;\n");
              break;
           }

           case EDJE_SOUND_SOURCE_TYPE_INLINE_LOSSY:
           {
              BUF_APPENDF("LOSSY %f;\n", current_sample->quality);
              break;
           }

           case EDJE_SOUND_SOURCE_TYPE_INLINE_AS_IS:
           {
              BUF_APPEND("AS_IS;\n");
              break;
           }

           default:
             break;
          }
        BUF_APPENDF(I3 "source: \"%s\";\n", current_sample->snd_src);
        BUF_APPEND(I2 "}\n");
     }
   for (i = 0; i < sound_directory->tones_count; i++, current_tone++)
     {
        BUF_APPEND(I2 "tone: ");
        BUF_APPENDF("\"%s\" %d;\n", current_tone->name, current_tone->value);
     }

   BUF_APPEND(I1 "}\n");
}

static void
_edje_limits_source_generate(const Edje *ed, Eina_Strbuf *buf, Eina_Bool *res)
{
   Eina_Bool ret = *res;
   unsigned int i;

   if (!ed->collection->limits.vertical_count &&
       !ed->collection->limits.horizontal_count) return;

   BUF_APPEND(I2 "limits {\n");

   for (i = 0; i < ed->collection->limits.vertical_count; i++)
     BUF_APPENDF(I3 "vertical: \"%s\" %d;\n",
                 ed->collection->limits.vertical[i]->name,
                 ed->collection->limits.vertical[i]->value);
   for (i = 0; i < ed->collection->limits.horizontal_count; i++)
     BUF_APPENDF(I3 "horizontal: \"%s\" %d;\n",
                 ed->collection->limits.horizontal[i]->name,
                 ed->collection->limits.horizontal[i]->value);

   BUF_APPEND(I2 "}\n");

   *res = ret;
}

static Eina_Bool
_edje_generate_source_of_group(Edje *ed, Edje_Part_Collection_Directory_Entry *pce, Eina_Strbuf *buf)
{
   Edje_Edit *eed;
   Eet_File *ef;
   Evas_Object *obj;
   Eina_List *l, *ll;
   unsigned int i;
   int w, h, orient;
   char *data;
   const char *group = pce->entry;
   Edje_Part_Collection *pc;
   Eina_Bool ret = EINA_TRUE, broadcast;
   Eina_List *alias_list = NULL;
   const char *alias;
   Eina_Iterator *it;
   int len;
   char *tmp_alias;
   const char *aliased;
   double base_scale;

   obj = edje_edit_object_add(ed->base.evas);
   if (!edje_object_file_set(obj, ed->file->path, group)) return EINA_FALSE;

   ef = _edje_edit_eet_open(ed, EET_FILE_MODE_READ);
   if (!ef)
     {
        evas_object_del(obj);
        return EINA_FALSE;
     }

   eed = efl_data_scope_get(obj, MY_CLASS);
   pc = eed->base->collection;
   alias_list = edje_edit_group_aliases_get(obj, group);

   base_scale = edje_object_base_scale_get(obj);
   if (fabs(base_scale - 1.0) > DBL_EPSILON)
     BUF_APPENDF(I1 "base_scale: \"%f\";\n",base_scale);

   BUF_APPENDF(I1 "group { name: \"%s\";\n", group);
   EINA_LIST_FOREACH(alias_list, l, data)
     BUF_APPENDF(I2 "alias: \"%s\";\n", data);
   edje_edit_string_list_free(alias_list);

   w = edje_edit_group_min_w_get(obj);
   h = edje_edit_group_min_h_get(obj);
   if ((w > 0) || (h > 0))
     BUF_APPENDF(I2 "min: %d %d;\n", w, h);
   w = edje_edit_group_max_w_get(obj);
   h = edje_edit_group_max_h_get(obj);
   if ((w > 0) || (h > 0))
     BUF_APPENDF(I2 "max: %d %d;\n", w, h);

   orient = edje_edit_group_orientation_get(obj);
   switch (orient)
     {
      case (EDJE_ORIENTATION_LTR):
        BUF_APPENDF(I2 "orientation: LTR;\n");
        break;

      case (EDJE_ORIENTATION_RTL):
        BUF_APPENDF(I2 "orientation: RTL;\n");
        break;

      case (EDJE_ORIENTATION_AUTO):
      default:
        break;
     }
   broadcast = edje_edit_group_broadcast_signal_get(obj);
   if (!broadcast) BUF_APPENDF(I2 "broadcast_signal: %d;\n", broadcast);

   /* Limits */
   _edje_limits_source_generate(ed, buf, &ret);

   if (pc->use_custom_seat_names)
     BUF_APPENDF(I2 "use_custom_seat_names: 1;\n");

   /* Data */
   if (pc->data)
     {
        Eina_Hash_Tuple *tuple;
        BUF_APPEND(I2 "data {\n");

        it = eina_hash_iterator_tuple_new(pc->data);

        if (!it)
          {
             ERR("Generating EDC for Group[%s] data.", group);
             return EINA_FALSE;
          }

        EINA_ITERATOR_FOREACH(it, tuple)
          BUF_APPENDF(I3 "item: \"%s\" \"%s\";\n", (char *)tuple->key,
                      edje_string_get(tuple->data));

        eina_iterator_free(it);
        BUF_APPEND(I2 "}\n");
     }

   if (eed->embryo_source)
     {
        BUF_APPEND(I2 "script {\n");
        BUF_APPEND(eed->embryo_source);
        BUF_APPEND(I2 "}\n");
     }

   /* Parts */
   BUF_APPEND(I2 "parts {\n");
   if ((pc->aliased) && (pc->alias))
     {
        it = eina_hash_iterator_data_new(pc->alias);
        EINA_ITERATOR_FOREACH(it, alias)
          {
             tmp_alias = strdup(alias);
             aliased = _edje_find_alias(pc->aliased, tmp_alias, &len);
             BUF_APPENDF(I3 "alias: \"%s\" \"%s\";\n", aliased, alias);
             free(tmp_alias);
          }
        eina_iterator_free(it);
     }
   for (i = 0; i < pc->parts_count; i++)
     {
        Edje_Part *ep;
        ep = pc->parts[i];
        ret &= _edje_generate_source_of_part(obj, ep, buf);
     }
   BUF_APPEND(I2 "}\n"); //parts

   if (!ret)
     {
        ERR("Generating EDC for Group[%s] Parts.", group);
        return EINA_FALSE;
     }

   /* Programs */
   if ((ll = edje_edit_programs_list_get(obj)))
     {
        BUF_APPEND(I2 "programs {\n");
        EINA_LIST_FOREACH(ll, l, data)
          ret &= _edje_generate_source_of_program(obj, data, buf);
        BUF_APPEND(I2 "}\n");
        edje_edit_string_list_free(ll);
     }
   BUF_APPEND(I1 "}\n"); //group

   if (!ret)
     {
        ERR("Generating EDC for Group[%s] Programs.", group);
        evas_object_del(obj);
        return EINA_FALSE;
     }

   _edje_edit_eet_close(ef);
   evas_object_del(obj);
   return ret;
}

static Eina_Strbuf *
_edje_generate_source(Evas_Object *obj)
{
   Eina_Strbuf *buf;

   Eina_List *l, *ll, *ll_set;
   Edje_Font_Directory_Entry *fnt;

   char *entry;
   Eina_Bool ret = EINA_TRUE;

   GET_ED_OR_RETURN(NULL);

   /* Open a str buffer */

   buf = eina_strbuf_new();
   if (!buf) return NULL;

   /* Write edc into file */
   //TODO Probably we need to save the file before generation

   /* Images */
   ll_set = edje_edit_image_set_list_get(obj);
   ll = edje_edit_images_list_get(obj);
   if (ll || ll_set)
     {
        BUF_APPEND(I0 "images {\n");

        EINA_LIST_FOREACH(ll, l, entry)
          {
             Eina_Strbuf *gen_buf = _edje_generate_image_source(obj, entry);
             if (!gen_buf) continue;

             BUF_APPENDF(I1 "%s", eina_strbuf_string_get(gen_buf));
             eina_strbuf_free(gen_buf);
          }

         EINA_LIST_FOREACH(ll_set, l, entry)
          {
             Eina_Strbuf *gen_buf = _edje_generate_image_set_source(obj, entry);
             if (!gen_buf) continue;

             BUF_APPENDF("%s", eina_strbuf_string_get(gen_buf));
             eina_strbuf_free(gen_buf);
          }

        BUF_APPEND(I0 "}\n\n");
        edje_edit_string_list_free(ll);
        edje_edit_string_list_free(ll_set);

        if (!ret)
          {
             ERR("Generating EDC for Images");
             eina_strbuf_free(buf);
             return NULL;
          }
     }

   /* Fonts */
   if (ed->file->fonts)
     {
        Eina_Iterator *it;

        it = eina_hash_iterator_data_new(ed->file->fonts);
        if (it)
          {
             BUF_APPEND(I0 "fonts {\n");

             EINA_ITERATOR_FOREACH(it, fnt)
               BUF_APPENDF(I1 "font: \"%s\" \"%s\";\n", fnt->file,
                           fnt->entry);

             BUF_APPEND(I0 "}\n\n");
             eina_iterator_free(it);

             if (!ret)
               {
                  ERR("Generating EDC for Fonts");
                  eina_strbuf_free(buf);
                  return NULL;
               }
          }
     }

   /* Data */
   if ((ll = edje_edit_data_list_get(obj)))
     {
        BUF_APPEND(I0 "data {\n");

        EINA_LIST_FOREACH(ll, l, entry)
          BUF_APPENDF(I1 "item: \"%s\" \"%s\";\n", entry,
                      edje_edit_data_value_get(obj, entry));

        BUF_APPEND(I0 "}\n\n");
        edje_edit_string_list_free(ll);

        if (!ret)
          {
             ERR("Generating EDC for Data");
             eina_strbuf_free(buf);
             return NULL;
          }
     }
   /* Size Classes */
   if ((ll = edje_edit_size_classes_list_get(obj)))
     {
        BUF_APPEND(I0 "size_classes {\n");

        EINA_LIST_FOREACH(ll, l, entry)
          _edje_generate_source_of_sizeclass(ed, entry, buf);

        BUF_APPEND(I0 "}\n\n");
        edje_edit_string_list_free(ll);

        if (!ret)
          {
             ERR("Generating EDC for Size Classes");
             eina_strbuf_free(buf);
             return NULL;
          }
     }

   /* Text Classes */
   if ((ll = edje_edit_text_classes_list_get(obj)))
     {
        BUF_APPEND(I0 "text_classes {\n");

        EINA_LIST_FOREACH(ll, l, entry)
          _edje_generate_source_of_textclass(ed, entry, buf);

        BUF_APPEND(I0 "}\n\n");
        edje_edit_string_list_free(ll);

        if (!ret)
          {
             ERR("Generating EDC for Text Classes");
             eina_strbuf_free(buf);
             return NULL;
          }
     }

   /* Color Classes */
   if ((ll = edje_edit_color_classes_list_get(obj)))
     {
        BUF_APPEND(I0 "color_classes {\n");

        EINA_LIST_FOREACH(ll, l, entry)
          _edje_generate_source_of_colorclass(ed, entry, buf);

        BUF_APPEND(I0 "}\n\n");
        edje_edit_string_list_free(ll);

        if (!ret)
          {
             ERR("Generating EDC for Color Classes");
             eina_strbuf_free(buf);
             return NULL;
          }
     }

   /* Styles */
   if ((ll = edje_edit_styles_list_get(obj)))
     {
        BUF_APPEND(I0 "styles {\n");
        EINA_LIST_FOREACH(ll, l, entry)
          _edje_generate_source_of_style(ed, entry, buf);
        BUF_APPEND(I0 "}\n\n");
        edje_edit_string_list_free(ll);

        if (!ret)
          {
             ERR("Generating EDC for Styles");
             eina_strbuf_free(buf);
             return NULL;
          }
     }

   /* Externals */
   if ((ll = edje_edit_externals_list_get(obj)))
     {
        BUF_APPEND(I0 "externals {\n");
        EINA_LIST_FOREACH(ll, l, entry)
          BUF_APPENDF(I1 "external: \"%s\";\n", entry);

        BUF_APPEND(I0 "}\n\n");
        edje_edit_string_list_free(ll);

        if (!ret)
          {
             ERR("Generating EDC for Externals");
             eina_strbuf_free(buf);
             return NULL;
          }
     }

   /* Collections */
   if (ed->file->collection)
     {
        Eina_Iterator *it;
        Edje_Part_Collection_Directory_Entry *pce;
        BUF_APPEND("collections {\n");

        /* Sounds */
        if (ed->file->sound_dir)
          {
             _edje_generate_source_of_sounds(ed->file->sound_dir, buf);
          }

        it = eina_hash_iterator_data_new(ed->file->collection);

        if (!it)
          {
             ERR("Generating EDC for Collections");
             eina_strbuf_free(buf);
             return NULL;
          }

        EINA_ITERATOR_FOREACH(it, pce)
          {
             if (!pce->group_alias)
               ret &= _edje_generate_source_of_group(ed, pce, buf);
          }

        eina_iterator_free(it);
        BUF_APPEND("}\n\n");
     }

   return buf;
}

EAPI char *
edje_edit_full_source_generate(Evas_Object *obj)
{
   Eina_Strbuf *code = _edje_generate_source(obj);
   /* return resulted source code */
   char *ret = eina_strbuf_string_steal(code);
   eina_strbuf_free(code);
   return ret;
}

/*********************/
/*  SAVING ROUTINES  */
/*********************/
////////////////////////////////////////
typedef struct _SrcFile      SrcFile;
typedef struct _SrcFile_List SrcFile_List;

struct _SrcFile
{
   char       *name;
   const char *file;
};

struct _SrcFile_List
{
   Eina_List *list;
};

static Eet_Data_Descriptor *_srcfile_edd = NULL;
static Eet_Data_Descriptor *_srcfile_list_edd = NULL;

static void
source_edd(void)
{
   Eet_Data_Descriptor_Class eddc;

   if (_srcfile_edd) return;

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc), "srcfile", sizeof(SrcFile));
   _srcfile_edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_srcfile_edd, SrcFile, "name", name, EET_T_INLINED_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_srcfile_edd, SrcFile, "file", file, EET_T_INLINED_STRING);

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof (eddc), "srcfile_list", sizeof(SrcFile_List));
   _srcfile_list_edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_srcfile_list_edd, SrcFile_List, "list", list, _srcfile_edd);
}

/////////////////////////////////////////

static Eina_Bool
_edje_edit_edje_file_save(Eet_File *eetf, Edje_File *ef)
{
   /* Write Edje_File structure */
   INF("** Writing Edje_File* ed->file");
   if (eet_data_write(eetf, _edje_edd_edje_file, "edje/file", ef, 1) <= 0)
     {
        ERR("Error. unable to write \"edje_file\" entry to \"%s\"", ef->path);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_edje_edit_collection_save(Eet_File *eetf, Edje_Part_Collection *epc)
{
   char buf[256];

   snprintf(buf, sizeof(buf), "edje/collections/%i", epc->id);

   if (eet_data_write(eetf, _edje_edd_edje_part_collection, buf, epc, 1) > 0)
     return EINA_TRUE;

   ERR("Error. unable to write \"%s\" part entry", buf);
   return EINA_FALSE;
}

static Eina_Bool
_edje_edit_source_save(Eet_File *eetf, Evas_Object *obj)
{
   SrcFile *sf;
   SrcFile_List *sfl;
   Eina_Strbuf *source_file;
   Eina_Bool ret = EINA_TRUE;

   source_file = _edje_generate_source(obj);
   if (!source_file)
     {
        ERR("Can't create edc source");
        return EINA_FALSE;
     }

   //open the temp file and put the contents in SrcFile
   sf = _alloc(sizeof(SrcFile));
   if (!sf)
     {
        ERR("Unable to create source file struct");
        ret = EINA_FALSE;
        goto save_free_source;
     }
   sf->name = strdup("generated_source.edc");
   if (!sf->name)
     {
        ERR("Unable to alloc filename");
        ret = EINA_FALSE;
        goto save_free_sf;
     }

   sf->file = eina_strbuf_string_get(source_file);

   //create the needed list of source files (only one)
   sfl = _alloc(sizeof(SrcFile_List));
   if (!sfl)
     {
        ERR("Unable to create file list");
        ret = EINA_FALSE;
        goto save_free_filename;
     }
   sfl->list = NULL;
   sfl->list = eina_list_append(sfl->list, sf);
   if (!sfl->list)
     {
        ERR("Error. unable to append file in list");
        ret = EINA_FALSE;
        goto save_free_sfl;
     }

   // write the sources list to the eet file
   source_edd();
   if (eet_data_write(eetf, _srcfile_list_edd, "edje_sources", sfl, 1) <= 0)
     {
        ERR("Unable to write edc source");
        ret = EINA_FALSE;
     }

   /* Clear stuff */
   eina_list_free(sfl->list);
save_free_sfl:
   free(sfl);
save_free_filename:
   free(sf->name);
save_free_sf:
   free(sf);
save_free_source:
   eina_strbuf_free(source_file);
   return ret;
}

Eina_Bool
_edje_edit_internal_save(Evas_Object *obj, int current_only, Eina_Bool generate_source)
{
   Edje_File *ef;
   Eet_File *eetf;

   GET_EED_OR_RETURN(EINA_FALSE);
   GET_ED_OR_RETURN(EINA_FALSE);

   ef = ed->file;
   if (!ef) return EINA_FALSE;

   INF("***********  Saving file ******************");
   INF("** path: %s", ef->path);

   /* Open the eet file */
   eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     return EINA_FALSE;

   /* Set compiler name */
   if (strcmp(ef->compiler, "edje_edit"))
     _edje_if_string_replace(ed, &ef->compiler, "edje_edit");

   if (!_edje_edit_edje_file_save(eetf, ef))
     {
        _edje_edit_eet_close(eetf);
        return EINA_FALSE;
     }

   if (current_only)
     {
        if (ed->collection)
          {
             INF("** Writing Edje_Part_Collection* ed->collection "
                 "[id: %d]", ed->collection->id);
             if (!_edje_edit_collection_save(eetf, ed->collection))
               {
                  _edje_edit_eet_close(eetf);
                  return EINA_FALSE;
               }
          }
     }
   else
     {
        Eina_List *l;
        Edje_Part_Collection *edc;
        Edje_Part_Collection_Directory_Entry *ce;
        Eina_Iterator *it;

        INF("** Writing all collections");

        EINA_LIST_FOREACH(ef->collection_cache, l, edc)
          {
             INF("** Writing cache Edje_Part_Collection* ed->collection "
                 "[id: %d]", edc->id);
             if (!_edje_edit_collection_save(eetf, edc))
               {
                  _edje_edit_eet_close(eetf);
                  return EINA_FALSE;
               }
          }

        it = eina_hash_iterator_data_new(ef->collection);
        while (eina_iterator_next(it, (void **)&ce))
          {
             if (ce->ref)
               {
                  INF("** Writing hash Edje_Part_Collection* ed->collection "
                      "[id: %d]", ce->id);
                  if (!_edje_edit_collection_save(eetf, ce->ref))
                    {
                       _edje_edit_eet_close(eetf);
                       return EINA_FALSE;
                    }
               }
          }
        eina_iterator_free(it);
     }

   if ((eed->bytecode_dirty || eed->script_need_recompile) && ed->collection)
     {
        char buf[64];
        Eina_Iterator *it;
        Program_Script *ps;
        Eina_List *deathnote = NULL;

        if (eed->bytecode_dirty)
          {
             snprintf(buf, sizeof(buf), "edje/scripts/embryo/compiled/%i",
                      ed->collection->id);
             eet_write(eetf, buf, eed->bytecode, eed->bytecode_size, 1);
             free(eed->bytecode);
             eed->bytecode = NULL;
             eed->bytecode_size = 0;
             eed->bytecode_dirty = EINA_FALSE;
          }

        if (eed->embryo_source_dirty)
          {
             snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%i",
                      ed->collection->id);
             eet_write(eetf, buf, eed->embryo_source,
                       strlen(eed->embryo_source) + 1, 1);
             eed->embryo_source_dirty = EINA_FALSE;
          }

        it = eina_hash_iterator_data_new(eed->program_scripts);
        EINA_ITERATOR_FOREACH(it, ps)
          {
             if (ps->dirty)
               {
                  snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%i/%i",
                           ed->collection->id, ps->id);
                  eet_write(eetf, buf, ps->code, strlen(ps->code) + 1, 1);
                  ps->dirty = EINA_FALSE;
               }
             else if (ps->delete_me)
               {
                  snprintf(buf, sizeof(buf), "edje/scripts/embryo/source/%i/%i",
                           ed->collection->id, ps->id);
                  eet_delete(eetf, buf);
                  deathnote = eina_list_append(deathnote, ps);
               }
          }
        eina_iterator_free(it);

        EINA_LIST_FREE(deathnote, ps)
          eina_hash_del(eed->program_scripts, &ps->id, ps);
     }

   if (generate_source)
     if (!_edje_edit_source_save(eetf, obj))
       {
          _edje_edit_eet_close(eetf);
          return EINA_FALSE;
       }

   _edje_edit_eet_close(eetf);

   /* Update mtime */
   {
      struct stat st;
      if (stat(ed->path, &st) != 0)
        return EINA_FALSE;
      ef->mtime = st.st_mtime;
   }

   INF("***********  Saving DONE ******************");
   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_clean_save_as(Evas_Object *obj, const char *new_file_name)
{
   Eet_File *ef, *ef_out;
   GET_ED_OR_RETURN(EINA_FALSE);
   GET_EED_OR_RETURN(EINA_FALSE);

   if (!ed->file) return EINA_FALSE;

   if (ecore_file_exists(new_file_name))
     {
        ERR("Error. file \"%s\" already exists",
            new_file_name);
        return EINA_FALSE;
     }
   ef = _edje_edit_eet_open(ed, EET_FILE_MODE_READ);
   if (!ef)
     return EINA_FALSE;
   ef_out = eet_open(new_file_name, EET_FILE_MODE_WRITE);
   if (!ef_out)
     {
        ERR("Error. unable to open \"%s\" for writing output",
            new_file_name);
        _edje_edit_eet_close(ef);
        return EINA_FALSE;
     }

   /* copying file structure */
   if (!_edje_edit_edje_file_save(ef_out, ed->file))
     {
        _edje_edit_eet_close(ef);
        eet_close(ef_out);
        return EINA_FALSE;
     }

   int count = 0;
   char **ent;
   int i;
   int size = 0;
   const void *data;

   ent = eet_list(ef, "*", &count);
   if (ent)
     {
        /* copying data */
        for (i = 0; i < count; i++)
          {
             /* Skiping entries that need special saving */
             if (!strcmp(ent[i], "edje/file")) continue;
             if (!strcmp(ent[i], "edje_sources")) continue;
             if (strstr(ent[i], "collection")) continue;

             data = eet_read_direct(ef, ent[i], &size);
             if (data) eet_write(ef_out, ent[i], data, size, 0);
             else
               {
                  data = eet_read(ef, ent[i], &size);
                  eet_write(ef_out, ent[i], data, size, 1);
               }
          }
        free(ent);
     }

   /* copying groups */
   Edje_Part_Collection_Directory_Entry *ce;
   Evas_Object *part_obj;
   part_obj = edje_edit_object_add(ed->base.evas);
   Eina_Iterator *it = eina_hash_iterator_data_new(ed->file->collection);
   EINA_ITERATOR_FOREACH(it, ce)
     {
        /* forcing collection load into memory */
        edje_object_file_set(part_obj, ed->file->path, ce->entry);

        _edje_edit_collection_save(ef_out, ce->ref);
     }
   eina_iterator_free(it);

   /* generating source code */
   _edje_edit_source_save(ef_out, obj);

   _edje_edit_eet_close(ef);
   eet_close(ef_out);

   return EINA_TRUE;
}

EAPI Eina_Bool
edje_edit_save(Evas_Object *obj)
{
   return _edje_edit_internal_save(obj, 1, EINA_TRUE);
}

EAPI Eina_Bool
edje_edit_save_all(Evas_Object *obj)
{
   return _edje_edit_internal_save(obj, 0, EINA_TRUE);
}

EAPI Eina_Bool
edje_edit_without_source_save(Evas_Object *obj, Eina_Bool current_group)
{
   GET_ED_OR_RETURN(EINA_FALSE);
   Eet_File *eetf = NULL;
   SrcFile_List *sfl = NULL;

   if (!_edje_edit_internal_save(obj, current_group, EINA_FALSE))
     {
        ERR("Unable save binary data into file");
        return EINA_FALSE;
     }

   sfl = _alloc(sizeof(SrcFile_List));
   if (!sfl)
     {
        ERR("Unable to create file list");
        return EINA_FALSE;
     }
   sfl->list = NULL;
   eetf = _edje_edit_eet_open(ed, EET_FILE_MODE_READ_WRITE);
   if (!eetf)
     {
        free(sfl);
        return EINA_FALSE;
     }
   source_edd();
   if (eet_data_write(eetf, _srcfile_list_edd, "edje_sources", sfl, 1) <= 0)
     {
        ERR("Unable to clean edc source from edj file");
        free(sfl);
        _edje_edit_eet_close(eetf);
        return EINA_FALSE;
     }

   free(sfl);
   _edje_edit_eet_close(eetf);
   return EINA_TRUE;
}

EAPI void
edje_edit_print_internal_status(Evas_Object *obj)
{
/*
   Edje_Program *epr;
   unsigned int i;
   int j;
 */
   Eina_Strbuf *source_file;
   GET_EED_OR_RETURN();

   source_file = _edje_generate_source(obj);
   if (source_file) eina_strbuf_free(source_file);
/*
   INF("****** CHECKIN' INTERNAL STRUCTS STATUS *********");

   INF("path: '%s', group: '%s', parent: '%s'",
       ed->path, ed->group, ed->parent);

   INF("Parts [table:%d list:%d]", ed->table_parts_size,
       ed->collection->parts_count);
   for (i = 0; i < ed->collection->parts_count; ++i)
     {
        Edje_Real_Part *rp;
        Edje_Part *p;

        p = ed->collection->parts[i];

        rp = ed->table_parts[p->id % ed->table_parts_size];
        printf("part[%d]: '%s' ", p->id, p->name);
        if (p == rp->part)
          printf("OK!\n");
        else
          printf("WRONG (table[%d]->name = '%s')\n", p->id, rp->part->name);
     }

   INF("Programs [table:%d list:%d,%d,%d,%d,%d]", ed->table_programs_size,
       ed->collection->programs.fnmatch_count,
       ed->collection->programs.strcmp_count,
       ed->collection->programs.strncmp_count,
       ed->collection->programs.strrncmp_count,
       ed->collection->programs.nocmp_count);
   for(j = 0; j < ed->table_programs_size; ++j)
     {
        epr = ed->table_programs[i % ed->table_programs_size];
        printf("program[%d]: '%s'\n", epr->id, epr->name);
     }

   INF("******************  END  ************************");
 */
}

/* Internal EO APIs and hidden overrides */

#define EDJE_EDIT_EXTRA_OPS \
   EFL_CANVAS_GROUP_DEL_OPS(edje_edit)

#include "edje_edit.eo.c"
