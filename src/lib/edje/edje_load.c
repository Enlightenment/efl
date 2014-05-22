#include "edje_private.h"

typedef struct _Edje_Table_Items Edje_Table_Items;
struct _Edje_Table_Items
{
   Evas_Object *child;
   const char *part;
   unsigned short col;
   unsigned short row;
   unsigned short colspan;
   unsigned short rowspan;
};

typedef struct _Edje_Drag_Items Edje_Drag_Items;
struct _Edje_Drag_Items
{
   const char *part;
   FLOAT_T x, y, w, h;
   struct {
      FLOAT_T x, y;
   } step;
   struct {
      FLOAT_T x, y;
   } page;
};

void _edje_file_add(Edje *ed, const Eina_File *f);

/* START - Nested part support */
#define _edje_smart_nested_type "Evas_Smart_Nested"
typedef struct _Edje_Nested_Support Edje_Nested_Support;
struct _Edje_Nested_Support
{  /* We builed nested-parts list using this struct */
   Evas_Object *o;        /* Smart object containing nested children */
   unsigned char nested_children_count; /* Number of nested children */
};

Evas_Smart *
_edje_smart_nested_smart_class_new(void)
{
   static Evas_Smart_Class _sc = EVAS_SMART_CLASS_INIT_NAME_VERSION("EdjeNested");
   static const Evas_Smart_Class *class = NULL;
   static Evas_Smart *smart;

   if (smart)
     return smart;

   class = &_sc;
   smart = evas_smart_class_new(class);
   return smart;
}


Evas_Object *
edje_smart_nested_add(Evas *evas)
{
      return evas_object_smart_add(evas, _edje_smart_nested_smart_class_new());
}

/* END   - Nested part support */


#ifdef EDJE_PROGRAM_CACHE
static Eina_Bool  _edje_collection_free_prog_cache_matches_free_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata);
#endif
static void _edje_object_pack_item_hints_set(Evas_Object *obj, Edje_Pack_Element *it);
static void _cb_signal_repeat(void *data, Evas_Object *obj, const char *signal, const char *source);

static Eina_List *_edje_object_collect(Edje *ed);

static int _sort_defined_boxes(const void *a, const void *b);

/************************** API Routines **************************/

EAPI Eina_Bool
edje_object_file_set(Evas_Object *obj, const char *file, const char *group)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;

   eo_do(obj, edje_obj_file_set(file, group, &ret));
   return ret;
}

EAPI Eina_Bool
edje_object_mmap_set(Evas_Object *obj, const Eina_File *file, const char *group)
{
   if (!obj) return EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;

   eo_do(obj, edje_obj_mmap_set(file, group, &ret));
   return ret;
}

EAPI void
edje_object_file_get(const Evas_Object *obj, const char **file, const char **group)
{
   if (!obj) return;
   eo_do((Eo *)obj, edje_obj_file_get(file, group));
}

void
_file_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **file = va_arg(*list, const char **);
   const char **group = va_arg(*list, const char **);
   const Edje *ed = _pd;
   if (file) *file = ed->path;
   if (group) *group = ed->group;
}

EAPI Edje_Load_Error
edje_object_load_error_get(const Evas_Object *obj)
{
   if (!obj) return EDJE_LOAD_ERROR_NONE;
   Edje_Load_Error ret = EDJE_LOAD_ERROR_NONE;
   eo_do((Eo *)obj, edje_obj_load_error_get(&ret));
   return ret;
}

void
_load_error_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Edje_Load_Error *ret = va_arg(*list, Edje_Load_Error *);
   const Edje *ed = _pd;
   *ret = ed->load_error;
}

EAPI const char *
edje_load_error_str(Edje_Load_Error error)
{
   switch (error)
     {
      case EDJE_LOAD_ERROR_NONE:
         return "No Error";
      case EDJE_LOAD_ERROR_GENERIC:
         return "Generic Error";
      case EDJE_LOAD_ERROR_DOES_NOT_EXIST:
         return "File Does Not Exist";
      case EDJE_LOAD_ERROR_PERMISSION_DENIED:
         return "Permission Denied";
      case EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED:
         return "Resource Allocation Failed";
      case EDJE_LOAD_ERROR_CORRUPT_FILE:
         return "Corrupt File";
      case EDJE_LOAD_ERROR_UNKNOWN_FORMAT:
         return "Unknown Format";
      case EDJE_LOAD_ERROR_INCOMPATIBLE_FILE:
         return "Incompatible File";
      case EDJE_LOAD_ERROR_UNKNOWN_COLLECTION:
         return "Unknown Collection";
      case EDJE_LOAD_ERROR_RECURSIVE_REFERENCE:
         return "Recursive Reference";
      default:
         return "Unknown Error";
     }
}

EAPI Eina_List *
edje_mmap_collection_list(Eina_File *f)
{
   Eina_List *lst = NULL;
   Edje_File *edf;
   int error_ret = 0;
   
   if (!f) return NULL;
   edf = _edje_cache_file_coll_open(f, NULL, &error_ret, NULL, NULL);
   if (edf)
     {
        Eina_Iterator *i;
        const char *key;

        i = eina_hash_iterator_key_new(edf->collection);

        EINA_ITERATOR_FOREACH(i, key)
           lst = eina_list_append(lst, eina_stringshare_add(key));

        eina_iterator_free(i);

        _edje_cache_file_unref(edf);
     }

   return lst;
}                          

EAPI Eina_List *
edje_file_collection_list(const char *file)
{
   Eina_File *f;
   Eina_List *lst;

   if ((!file) || (!*file)) return NULL;
   f = eina_file_open(file, EINA_FALSE);

   lst = edje_mmap_collection_list(f);

   eina_file_close(f);
   return lst;
}

EAPI void
edje_file_collection_list_free(Eina_List *lst)
{
   while (lst)
     {
        if (eina_list_data_get(lst)) eina_stringshare_del(eina_list_data_get(lst));
        lst = eina_list_remove(lst, eina_list_data_get(lst));
     }
}

EAPI void
edje_mmap_collection_list_free(Eina_List *lst)
{
   edje_file_collection_list_free(lst);
}

EAPI Eina_Bool
edje_mmap_group_exists(Eina_File *f, const char *glob)
{
   Edje_File *edf;
   int error_ret = 0;
   Eina_Bool succeed = EINA_FALSE;
   Eina_Bool is_glob = EINA_FALSE;
   const char *p;

   if ((!f) || (!glob))
      return EINA_FALSE;

   edf = _edje_cache_file_coll_open(f, NULL, &error_ret, NULL, NULL);
   if (!edf) return EINA_FALSE;

   for (p = glob; *p; p++)
     {
       if ((*p == '*') || (*p == '?') || (*p == '['))
         {
           is_glob = EINA_TRUE;
           break;
         }
     }

   if (is_glob)
     {
       if (!edf->collection_patterns)
         {
           Edje_Part_Collection_Directory_Entry *ce;
           Eina_Iterator *i;
           Eina_List *l = NULL;

           i = eina_hash_iterator_data_new(edf->collection);

           EINA_ITERATOR_FOREACH(i, ce)
             l = eina_list_append(l, ce);

           eina_iterator_free(i);

           edf->collection_patterns = edje_match_collection_dir_init(l);
           eina_list_free(l);
         }

       succeed = edje_match_collection_dir_exec(edf->collection_patterns, glob);
       if (edf->collection_patterns)
         {
           edje_match_patterns_free(edf->collection_patterns);
           edf->collection_patterns = NULL;
         }
     }
   else
     {
        if (eina_hash_find(edf->collection, glob)) succeed = EINA_TRUE;
     }
   _edje_cache_file_unref(edf);

   DBG("edje_file_group_exists: '%s', '%s': %i.", eina_file_filename_get(f), glob, succeed);

   return succeed;
}

EAPI Eina_Bool
edje_file_group_exists(const char *file, const char *glob)
{
   Eina_File *f;
   Eina_Bool result;

   if ((!file) || (!*file) || (!glob))
     return EINA_FALSE;

   f = eina_file_open(file, EINA_FALSE);
   if (!f) return EINA_FALSE;

   result = edje_mmap_group_exists(f, glob);

   eina_file_close(f);

   return result;
}

EAPI char *
edje_mmap_data_get(const Eina_File *f, const char *key)
{
   Edje_File *edf;
   char *str = NULL;
   int error_ret = 0;

   if (!key) return NULL;

   edf = _edje_cache_file_coll_open(f, NULL, &error_ret, NULL, NULL);
   if (edf)
     {
        str = (char*) edje_string_get(eina_hash_find(edf->data, key));

        if (str) str = strdup(str);

        _edje_cache_file_unref(edf);
     }
   return str;
}

EAPI char *
edje_file_data_get(const char *file, const char *key)
{
   Eina_File *f;
   char *str;

   if (!key) return NULL;

   f = eina_file_open(file, EINA_FALSE);
   if (!f)
     {
        ERR("File [%s] can not be opened.", file);
        return NULL;
     }

   str = edje_mmap_data_get(f, key);

   eina_file_close(f);

   return str;
}

void
_edje_programs_patterns_clean(Edje_Part_Collection *edc)
{
   _edje_signals_sources_patterns_clean(&edc->patterns.programs);

   eina_rbtree_delete(edc->patterns.programs.exact_match,
                      EINA_RBTREE_FREE_CB(edje_match_signal_source_free),
                      NULL);
   edc->patterns.programs.exact_match = NULL;

   free(edc->patterns.programs.u.programs.globing);
   edc->patterns.programs.u.programs.globing = NULL;
}

#ifdef HAVE_EPHYSICS
static void
_edje_physics_world_update_cb(void *data, EPhysics_World *world EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Edje *edje = data;
   _edje_recalc_do(edje);
}
#endif

int
_edje_object_file_set_internal(Evas_Object *obj, const Eina_File *file, const char *group, const char *parent, Eina_List *group_path, Eina_Array *nested)
{
   Edje *ed;
   Evas *tev;
   Edje_Real_Part *rp;
   Eina_List *textblocks = NULL;
   Eina_List *sources = NULL;
   Eina_List *externals = NULL;
   Eina_List *collect = NULL;
   unsigned int n;
   Eina_Array parts;
   int group_path_started = 0;
   Evas_Object *nested_smart = NULL;

   /* Get data pointer of top-of-stack */
   int idx = eina_array_count(nested) - 1;
   Edje_Nested_Support *st_nested = (idx >= 0) ? eina_array_data_get(nested, idx) : NULL;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   if (!group) group = "";
   if ((ed->file) && (ed->file->f == file) &&
       (ed->group) && (!strcmp(group, ed->group)))
     {
        return 1;
     }

   tev = evas_object_evas_get(obj);
   evas_event_freeze(tev);

   collect = _edje_object_collect(ed);

   if (_edje_script_only(ed)) _edje_script_only_shutdown(ed);
   if (_edje_lua_script_only(ed)) _edje_lua_script_only_shutdown(ed);

#ifdef HAVE_EPHYSICS
   /* clear physics world  / shutdown ephysics */
   if ((ed->collection) && (ed->collection->physics_enabled))
     {
        ephysics_world_del(ed->world);
        ephysics_shutdown();
     }
#endif

   _edje_file_del(ed);

   eina_stringshare_replace(&ed->path, file ? eina_file_filename_get(file) : NULL);
   eina_stringshare_replace(&ed->group, group);

   ed->parent = eina_stringshare_add(parent);

   ed->load_error = EDJE_LOAD_ERROR_NONE;
   _edje_file_add(ed, file);
   ed->block_break = EINA_FALSE;

   if (ed->file && ed->file->external_dir)
     {
        unsigned int i;

        for (i = 0; i < ed->file->external_dir->entries_count; ++i)
          edje_module_load(ed->file->external_dir->entries[i].entry);
     }

   _edje_textblock_styles_add(ed);
   _edje_textblock_style_all_update(ed);

   ed->has_entries = EINA_FALSE;

   if (ed->collection)
     {
        eina_array_step_set(&parts, sizeof (Eina_Array), 8);

        if (ed->collection->prop.orientation != EDJE_ORIENTATION_AUTO)
          ed->is_rtl = (ed->collection->prop.orientation ==
                        EDJE_ORIENTATION_RTL);

        ed->groups = eina_list_append(ed->groups, ed);

        if (ed->collection->script_only)
          {
             ed->load_error = EDJE_LOAD_ERROR_NONE;
             _edje_script_only_init(ed);
          }
        else if (ed->collection->lua_script_only)
          {
             ed->load_error = EDJE_LOAD_ERROR_NONE;
             _edje_lua_script_only_init(ed);
          }
        else
          {
             unsigned int i;

             if (ed->collection->physics_enabled)
#ifdef HAVE_EPHYSICS
               {
                  ephysics_init();
                  ed->world = ephysics_world_new();
                  ephysics_world_event_callback_add(
                     ed->world, EPHYSICS_CALLBACK_WORLD_UPDATE,
                     _edje_physics_world_update_cb, ed);
                  ephysics_world_rate_set(ed->world,
                                          ed->collection->physics.world.rate);
                  ephysics_world_gravity_set(
                     ed->world, ed->collection->physics.world.gravity.x,
                     ed->collection->physics.world.gravity.y,
                     ed->collection->physics.world.gravity.z);
               }
#else
               ERR("Edje compiled without support to physics.");
#endif

               /* colorclass stuff */
               for (i = 0; i < ed->collection->parts_count; ++i)
                 {
                    Edje_Part *ep;
                    unsigned int k;

                    ep = ed->collection->parts[i];

                    /* Register any color classes in this parts descriptions. */
                    if ((ep->default_desc) && (ep->default_desc->color_class))
                      _edje_color_class_member_add(ed, ep->default_desc->color_class);

                    for (k = 0; k < ep->other.desc_count; k++)
                      {
                         Edje_Part_Description_Common *desc;

                         desc = ep->other.desc[k];

                         if (desc->color_class)
                           _edje_color_class_member_add(ed, desc->color_class);
                      }
                 }
               /* build real parts */
               for (n = 0; n < ed->collection->parts_count; n++)
                 {
                    Edje_Part *ep;
                    Eina_Bool memerr = EINA_FALSE;

                    ep = ed->collection->parts[n];

                    if (ep->nested_children_count)
                      {  /* Add object to nested parts list */
                       st_nested = malloc(sizeof(*st_nested));
                       nested_smart = st_nested->o = edje_smart_nested_add(tev);

                       /* We add 1 to children_count because the parent
                          object is added to smart obj children as well */
                       st_nested->nested_children_count =
                          ep->nested_children_count + 1;

                       evas_object_show(st_nested->o);

                       eina_array_push(nested, st_nested);
                      }

                    rp = eina_mempool_malloc(_edje_real_part_mp, sizeof(Edje_Real_Part));
                    if (!rp)
                      {
                         /* FIXME: destroy all allocated ressource, need to have a common exit point */
                         ed->load_error = EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                         goto on_error;
                      }

                    memset(rp, 0, sizeof (Edje_Real_Part));

                    rp->param1.p.map = eina_cow_alloc(_edje_calc_params_map_cow);
#ifdef HAVE_EPHYSICS
                    rp->param1.p.physics = eina_cow_alloc(_edje_calc_params_physics_cow);
#endif

                    if ((ep->dragable.x != 0) || (ep->dragable.y != 0))
                      {
                         rp->drag = calloc(1, sizeof (Edje_Real_Part_Drag));
                         if (!rp->drag)
                           {
                              ed->load_error = EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                              goto on_error;
                           }

                         rp->drag->step.x = FROM_INT(ep->dragable.step_x);
                         rp->drag->step.y = FROM_INT(ep->dragable.step_y);
                      }
                    // allow part type specific data - this keeps real_part smaller
                    switch (ep->type)
                      {
                       case EDJE_PART_TYPE_TEXT:
                       case EDJE_PART_TYPE_TEXTBLOCK:
                          rp->type = EDJE_RP_TYPE_TEXT;
                          rp->typedata.text = calloc(1, sizeof(Edje_Real_Part_Text));
                          if (!rp->typedata.text) memerr = EINA_TRUE;
                          break;
                       case EDJE_PART_TYPE_GROUP:
                       case EDJE_PART_TYPE_SWALLOW:
                       case EDJE_PART_TYPE_EXTERNAL:
                          rp->type = EDJE_RP_TYPE_SWALLOW;
                          rp->typedata.swallow = calloc(1, sizeof(Edje_Real_Part_Swallow));
                          if (!rp->typedata.swallow) memerr = EINA_TRUE;
                          break;
                       case EDJE_PART_TYPE_BOX:
                       case EDJE_PART_TYPE_TABLE:
                          rp->type = EDJE_RP_TYPE_CONTAINER;
                          rp->typedata.container = calloc(1, sizeof(Edje_Real_Part_Container));
                          if (!rp->typedata.container) memerr = EINA_TRUE;
                          break;
                       default:
                          break;
                      }

                    if (memerr)
                      {
                         if (rp->drag) free(rp->drag);
                         ed->load_error = EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                         eina_mempool_free(_edje_real_part_mp, rp);
                         evas_event_thaw(tev);
                         evas_event_thaw_eval(tev);
                         return 0;
                      }

                    _edje_ref(ed);
                    rp->part = ep;
                    eina_array_push(&parts, rp);
                    rp->param1.description =
                       _edje_part_description_find(ed, rp, "default", 0.0, EINA_TRUE);
                    rp->chosen_description = rp->param1.description;
                    if (!rp->param1.description)
                      ERR("no default part description for '%s'!",
                          rp->part->name);

                    switch (ep->type)
                      {
                       case EDJE_PART_TYPE_RECTANGLE:
                          rp->object = evas_object_rectangle_add(ed->base->evas);
                          break;
                       case EDJE_PART_TYPE_PROXY:
                       case EDJE_PART_TYPE_IMAGE:
                          rp->object = evas_object_image_add(ed->base->evas);
                          break;
                       case EDJE_PART_TYPE_TEXT:
                          _edje_text_part_on_add(ed, rp);
                          rp->object = evas_object_text_add(ed->base->evas);
                          evas_object_text_font_source_set(rp->object, ed->path);
                          break;
                       case EDJE_PART_TYPE_GROUP:
                          sources = eina_list_append(sources, rp);
                       case EDJE_PART_TYPE_SWALLOW:
                       case EDJE_PART_TYPE_EXTERNAL:
                          if (ep->type == EDJE_PART_TYPE_EXTERNAL)
                            externals = eina_list_append(externals, rp);
                          rp->object = evas_object_rectangle_add(ed->base->evas);
                          evas_object_color_set(rp->object, 0, 0, 0, 0);
                          evas_object_pass_events_set(rp->object, 1);
                          evas_object_pointer_mode_set(rp->object, EVAS_OBJECT_POINTER_MODE_NOGRAB);
                          _edje_callbacks_focus_add(rp->object, ed, rp);
                          break;
                       case EDJE_PART_TYPE_TEXTBLOCK:
                          textblocks = eina_list_append(textblocks, rp);
                          rp->object = evas_object_textblock_add(ed->base->evas);
                          break;
                       case EDJE_PART_TYPE_BOX:
                          sources = eina_list_append(sources, rp);
                          rp->object = evas_object_box_add(ed->base->evas);
                          rp->typedata.container->anim = _edje_box_layout_anim_new(rp->object);
                          break;
                       case EDJE_PART_TYPE_TABLE:
                          sources = eina_list_append(sources, rp);
                          rp->object = evas_object_table_add(ed->base->evas);
                          break;
                       case EDJE_PART_TYPE_GRADIENT:
                          ERR("SPANK ! SPANK ! SPANK ! YOU ARE USING GRADIENT IN PART %s FROM GROUP %s INSIDE FILE %s !! THEY ARE NOW REMOVED !",
                              ep->name, group, eina_file_filename_get(file));
                          break;
                       case EDJE_PART_TYPE_SPACER:
                          rp->object = NULL;
                          break;
                       default:
                          ERR("wrong part type %i!", ep->type);
                          break;
                      }

                    if (rp->object)
                      {
                         if (nested_smart)
                           {  /* Update this pointer to father object only
                                 this will make smart object size == father sz */
                              rp->nested_smart = nested_smart;
                              nested_smart = NULL;
                           }

                         if (st_nested && st_nested->nested_children_count)
                           {  /* Add this to list of children */
                              evas_object_smart_member_add(rp->object,
                                                           st_nested->o);

                              st_nested->nested_children_count--;

                              /* No more nested children for this obj */
                              while (st_nested && (st_nested->nested_children_count == 0))
                              {
                                 /* Loop to add smart counter as child */
                                 Evas_Object *p_obj = st_nested->o;

                                 st_nested = eina_array_pop(nested);
                                 free(st_nested);

                                 /* Check for parent in stack */
                                 idx = eina_array_count(nested) - 1;
                                 st_nested = (idx >= 0) ? eina_array_data_get(nested,idx) : NULL;

                                 if (st_nested)
                                   {
                                      st_nested->nested_children_count--;
                                      evas_object_smart_member_add(p_obj, st_nested->o);
                                   }
                                 else
                                   {
                                      evas_object_smart_member_add(p_obj, ed->obj);
                                   }
                              }
                         }
                       else
                         evas_object_smart_member_add(rp->object, ed->obj);

                       //		       evas_object_layer_set(rp->object, evas_object_layer_get(ed->obj));
                       if (ep->type != EDJE_PART_TYPE_SWALLOW && ep->type != EDJE_PART_TYPE_GROUP && ep->type != EDJE_PART_TYPE_EXTERNAL)
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
                                 evas_object_pointer_mode_set(rp->object, EVAS_OBJECT_POINTER_MODE_NOGRAB);
                              }
                            if (ep->precise_is_inside)
                              evas_object_precise_is_inside_set(rp->object, 1);
                         }
                       if (rp->part->clip_to_id < 0)
                         evas_object_clip_set(rp->object, ed->base->clipper);
                      }
                 }
               if (n > 0)
                 {
                    ed->table_parts = malloc(sizeof(Edje_Real_Part *) * n);
                    ed->table_parts_size = n;
                    /* FIXME: check malloc return */
                    n = eina_array_count(&parts) - 1;
                    while ((rp = eina_array_pop(&parts)))
                      {
                         ed->table_parts[n] = rp;
                         n--;
                      }
                    for (i = 0; i < ed->table_parts_size; i++)
                      {
                         rp = ed->table_parts[i];
                         if (rp->param1.description) /* FIXME: prevent rel to gone radient part to go wrong. You may
                                                        be able to remove this when all theme are correctly rewritten. */
                           {
                              if (rp->param1.description->rel1.id_x >= 0)
                                rp->param1.description->rel1.id_x %= ed->table_parts_size;
                              if (rp->param1.description->rel1.id_y >= 0)
                                rp->param1.description->rel1.id_y %= ed->table_parts_size;
                              if (rp->param1.description->rel2.id_x >= 0)
                                rp->param1.description->rel2.id_x %= ed->table_parts_size;
                              if (rp->param1.description->rel2.id_y >= 0)
                                rp->param1.description->rel2.id_y %= ed->table_parts_size;
                           }
                         if (rp->part->clip_to_id >= 0)
                           {
                              Edje_Real_Part *clip_to;

                              clip_to = ed->table_parts[rp->part->clip_to_id % ed->table_parts_size];
                              if (clip_to &&
                                  clip_to->object &&
                                  rp->object)
                                {
                                   evas_object_pass_events_set(clip_to->object, 1);
                                   evas_object_pointer_mode_set(clip_to->object, EVAS_OBJECT_POINTER_MODE_NOGRAB);
                                   evas_object_clip_set(rp->object, clip_to->object);
                                }
                           }
                         if (rp->drag)
                           {
                              if (rp->part->dragable.confine_id >= 0)
                                rp->drag->confine_to = ed->table_parts[rp->part->dragable.confine_id % ed->table_parts_size];
                              if (rp->part->dragable.threshold_id >= 0)
                                rp->drag->threshold = ed->table_parts[rp->part->dragable.threshold_id % ed->table_parts_size];
                           }

                         if ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
                             (rp->typedata.swallow))
                           {
                              rp->typedata.swallow->swallow_params.min.w = 0;
                              rp->typedata.swallow->swallow_params.min.h = 0;
                              rp->typedata.swallow->swallow_params.max.w = -1;
                              rp->typedata.swallow->swallow_params.max.h = -1;
                           }

                         if (rp->part->type == EDJE_PART_TYPE_TEXT
                             || rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
                           {
                              Edje_Part_Description_Text *text;

                              text = (Edje_Part_Description_Text *) rp->param1.description;

                              if (ed->file->feature_ver < 1)
                                {
                                   text->text.id_source = -1;
                                   text->text.id_text_source = -1;
                                }

                              if ((rp->type == EDJE_RP_TYPE_TEXT) &&
                                  (rp->typedata.text))
                                {
                                   if (text->text.id_source >= 0)
                                     rp->typedata.text->source = ed->table_parts[text->text.id_source % ed->table_parts_size];
                                   if (text->text.id_text_source >= 0)
                                     rp->typedata.text->text_source = ed->table_parts[text->text.id_text_source % ed->table_parts_size];
                                }
                              if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
                                {
                                   _edje_entry_real_part_init(ed, rp);
                                   if (!ed->has_entries)
                                     ed->has_entries = EINA_TRUE;
                                }
                           }
                      }
                 }

               _edje_ref(ed);
               _edje_block(ed);
               _edje_freeze(ed);
               //	     if (ed->collection->script) _edje_embryo_script_init(ed);
               _edje_var_init(ed);
               for (i = 0; i < ed->table_parts_size; i++)
                 {
                    rp = ed->table_parts[i];
                    evas_object_show(rp->object);
                    if (_edje_block_break(ed)) break;
                    if (rp->drag)
                      {
                         if (rp->part->dragable.x < 0) rp->drag->val.x = FROM_DOUBLE(1.0);
                         if (rp->part->dragable.y < 0) rp->drag->val.x = FROM_DOUBLE(1.0);
                         _edje_dragable_pos_set(ed, rp, rp->drag->val.x, rp->drag->val.y);
                      }
                 }
               ed->recalc_call = EINA_TRUE;
               ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
               ed->all_part_change = EINA_TRUE;
#endif
               if ((evas_object_clipees_get(ed->base->clipper)) &&
                   (evas_object_visible_get(obj)))
                 evas_object_show(ed->base->clipper);

               /* instantiate 'internal swallows' */
               EINA_LIST_FREE(externals, rp)
                 {
                    Edje_Part_Description_External *external;
                    Evas_Object *child_obj;

                    external = (Edje_Part_Description_External *) rp->part->default_desc;
                    child_obj = _edje_external_type_add(rp->part->source,
                                                        evas_object_evas_get(ed->obj), ed->obj,
                                                        external->external_params, rp->part->name);
                    if (child_obj)
                      {
                         _edje_real_part_swallow(ed, rp, child_obj, EINA_TRUE);
                         rp->param1.external_params = _edje_external_params_parse(child_obj,
                                                                                  external->external_params);
                         _edje_external_recalc_apply(ed, rp, NULL, rp->chosen_description);
                      }
                 }

               EINA_LIST_FREE(sources, rp)
                 {
                    /* XXX: curr_item and pack_it don't require to be NULL since
                     * XXX: they are just used when source != NULL and type == BOX,
                     * XXX: and they're always set in this case, but GCC fails to
                     * XXX: notice that, so let's shut it up
                     */
                    Edje_Pack_Element **curr_item = NULL;
                    unsigned int item_count = 0;
                    Edje_Pack_Element *pack_it = NULL;
                    const char *source = NULL;

                    switch (rp->part->type)
                      {
                       case EDJE_PART_TYPE_GROUP:
                          source = rp->part->source;
                          break;
                       case EDJE_PART_TYPE_BOX:
                       case EDJE_PART_TYPE_TABLE:
                          if (rp->part->items)
                            {
                               curr_item = rp->part->items;
                               item_count = rp->part->items_count;
                               if (item_count > 0)
                                 {
                                    pack_it = *curr_item;
                                    source = pack_it->source;
                                    item_count--;
                                    curr_item++;
                                 }
                            }
                          break;
                       default:
                          /* This list should only be filled by group, box or table, nothing else. */
                          abort();
                          continue;
                      }

                    while (source)
                      {
                         Eina_List *l;
                         Evas_Object *child_obj;
                         Edje_Pack_Element pack_it_copy;
                         const char *group_path_entry = eina_stringshare_add(source);
                         const char *data;

                         if (!group_path)
                           {
                              group_path = eina_list_append(NULL, eina_stringshare_add(group));
                              group_path_started = 1;
                           }
                         /* make sure that this group isn't already in the tree of parents */
                         EINA_LIST_FOREACH(group_path, l, data)
                           {
                              if (data == group_path_entry)
                                {
                                   ERR("recursive loop group '%s' already included inside part '%s' of group '%s' from file '%s'",
                                       group_path_entry, rp->part->name, group, eina_file_filename_get(file));
                                   ed->load_error = EDJE_LOAD_ERROR_RECURSIVE_REFERENCE;
                                   eina_stringshare_del(group_path_entry);
                                   goto on_error;
                                }
                           }

                         if (pack_it)
                           {
                              pack_it_copy = *pack_it;
                           }
                         else
                           {
                              memset(&pack_it_copy, 0, sizeof (pack_it_copy));
                           }

                         do
                           {
                              child_obj = edje_object_add(ed->base->evas);
                              group_path = eina_list_append(group_path, group_path_entry);
                              if (rp->part->type == EDJE_PART_TYPE_GROUP)
                                {
                                   _edje_real_part_swallow(ed, rp, child_obj, EINA_FALSE);
                                }

                              if (!_edje_object_file_set_internal(child_obj, file, source, rp->part->name, group_path, nested))
                                {
                                   ERR("impossible to set part '%s' of group '%s' from file '%s' to '%s'",
                                       rp->part->name, group_path_entry, eina_file_filename_get(file), source);
                                   ed->load_error = edje_object_load_error_get(child_obj);
                                   evas_object_del(child_obj);
                                   eina_stringshare_del(group_path_entry);
                                   goto on_error;
                                }

                              group_path = eina_list_remove(group_path, group_path_entry);

                              edje_object_propagate_callback_add(child_obj,
                                                                 _cb_signal_repeat,
                                                                 obj);
                              if (rp->part->type == EDJE_PART_TYPE_GROUP)
                                {
                                   Edje *edg = _edje_fetch(child_obj);
                                   ed->groups = eina_list_append(ed->groups, edg);
                                   evas_object_data_set(child_obj, "\377 edje.part_obj", rp);
                                   _edje_real_part_swallow(ed, rp, child_obj, EINA_TRUE);
                                   _edje_subobj_register(ed, child_obj);
                                   source = NULL;
                                }
                              else
                                {
                                   if ((rp->type == EDJE_RP_TYPE_CONTAINER) &&
                                       (rp->typedata.container))
                                     {
                                        Eina_Strbuf *buf = NULL;
                                        const char *name = pack_it_copy.name;

                                        pack_it->parent = rp;

                                        _edje_object_pack_item_hints_set(child_obj, &pack_it_copy);

                                        if (pack_it->spread.h > 1 && pack_it->spread.w >= 1)
                                          {
                                             buf = eina_strbuf_new();
                                             if (name)
                                               eina_strbuf_append_printf(buf, "%s{%i,%i}", name, pack_it_copy.col, pack_it_copy.row);
                                             else
                                               eina_strbuf_append_printf(buf, "%i,%i", pack_it_copy.col, pack_it_copy.row);
                                             name = eina_strbuf_string_get(buf);
                                          }
                                        if (name) evas_object_name_set(child_obj, name);
                                        if (buf) eina_strbuf_free(buf);

                                        if (rp->part->type == EDJE_PART_TYPE_BOX)
                                          {
                                             _edje_real_part_box_append(ed, rp, child_obj);
                                             evas_object_data_set(child_obj, "\377 edje.box_item", pack_it);
                                          }
                                        else if (rp->part->type == EDJE_PART_TYPE_TABLE)
                                          {
                                             _edje_real_part_table_pack(ed, rp, child_obj,
                                                                        pack_it_copy.col, pack_it_copy.row,
                                                                        pack_it_copy.colspan, pack_it_copy.rowspan);
                                             evas_object_data_set(child_obj, "\377 edje.table_item", pack_it);
                                          }
                                        _edje_subobj_register(ed, child_obj);
                                        evas_object_show(child_obj);
                                        rp->typedata.container->items = eina_list_append(rp->typedata.container->items, child_obj);
                                     }
                                }

                              pack_it_copy.spread.w--;
                              pack_it_copy.col++;
                              if (pack_it_copy.spread.w < 1 && pack_it)
                                {
                                   pack_it_copy.col = pack_it->col;
                                   pack_it_copy.row++;
                                   pack_it_copy.spread.h--;
                                   pack_it_copy.spread.w = pack_it->spread.w;
                                }
                           }
                         while (pack_it_copy.spread.h > 0);

                         eina_stringshare_del(group_path_entry);

                         if ((rp->type == EDJE_RP_TYPE_CONTAINER) &&
                             (rp->typedata.container))
                           {
                              if (item_count > 0)
                                {
                                   pack_it = *curr_item;
                                   source = pack_it->source;
                                   curr_item++;
                                   item_count--;
                                }
                              else
                                {
                                   source = NULL;
                                   curr_item = NULL;
                                   pack_it = NULL;
                                }
                           }
                      }
                 }

               if (group_path_started)
                 {
                    const char *str;

                    EINA_LIST_FREE(group_path, str)
                       eina_stringshare_del(str);
                 }

               /* reswallow any swallows that existed before setting the file */
               if (collect)
                 {
                    Edje_User_Defined *eud;
                    Eina_List *boxes = NULL;

                    EINA_LIST_FREE(collect, eud)
                      {
                         Evas_Object *child = NULL;

                         switch (eud->type)
                           {
                            case EDJE_USER_SWALLOW:
                               edje_object_part_swallow(obj, eud->part, eud->u.swallow.child);
                               child = eud->u.swallow.child;
                               break;
                            case EDJE_USER_BOX_PACK:
                               boxes = eina_list_append(boxes, eud);
                               eud = NULL;
                               break;
                            case EDJE_USER_TABLE_PACK:
                               edje_object_part_table_pack(obj, eud->part, eud->u.table.child,
                                                           eud->u.table.col, eud->u.table.row,
                                                           eud->u.table.colspan, eud->u.table.rowspan);
                               child = eud->u.table.child;
                               break;
                            case EDJE_USER_DRAG_STEP:
                               edje_object_part_drag_step_set(obj, eud->part,
                                                              eud->u.drag_position.x,
                                                              eud->u.drag_position.y);
                               break;
                            case EDJE_USER_DRAG_PAGE:
                               edje_object_part_drag_page_set(obj, eud->part,
                                                              eud->u.drag_position.x,
                                                              eud->u.drag_position.y);
                               break;
                            case EDJE_USER_DRAG_VALUE:
                               edje_object_part_drag_value_set(obj, eud->part,
                                                               eud->u.drag_position.x,
                                                               eud->u.drag_position.y);
                               break;
                            case EDJE_USER_DRAG_SIZE:
                               edje_object_part_drag_size_set(obj, eud->part,
                                                              eud->u.drag_size.w,
                                                              eud->u.drag_size.h);
                               break;
                            case EDJE_USER_STRING:
                               edje_object_part_text_set(obj, eud->part, eud->u.string.text);
                               eina_stringshare_del(eud->u.string.text);
                               break;
                           }
                         if (eud) _edje_user_definition_remove(eud, child);
                      }

                    boxes = eina_list_sort(boxes, -1, _sort_defined_boxes);
                    EINA_LIST_FREE(boxes, eud)
                      {
                         edje_object_part_box_append(obj, eud->part, eud->u.box.child);
                         _edje_user_definition_remove(eud, eud->u.box.child);
                      }
                 }

               if (edje_object_mirrored_get(obj))
                 edje_object_signal_emit(obj, "edje,state,rtl", "edje");
               else
                 edje_object_signal_emit(obj, "edje,state,ltr", "edje");

               _edje_recalc(ed);
               _edje_thaw(ed);
               _edje_unblock(ed);
               _edje_unref(ed);
               ed->load_error = EDJE_LOAD_ERROR_NONE;
               _edje_emit(ed, "load", NULL);

               /* instantiate 'internal textblock style' */
               EINA_LIST_FREE(textblocks, rp)
                  if (rp->part->default_desc)
                    {
                       Edje_Part_Description_Text *text;
                       Edje_Style *stl  = NULL;
                       const char *style;

                       text = (Edje_Part_Description_Text *) rp->part->default_desc;
                       style = edje_string_get(&text->text.style);
                       if (style)
                         {
                            Eina_List *l;

                            EINA_LIST_FOREACH(ed->file->styles, l, stl)
                              {
                                 if ((stl->name) && (!strcmp(stl->name, style))) break;
                                 stl = NULL;
                              }
                         }
                       if (stl)
                         {
                            if (evas_object_textblock_style_get(rp->object) != stl->style)
                              evas_object_textblock_style_set(rp->object, stl->style);
                         }
                    }
          }
        _edje_entry_init(ed);
        eina_array_flush(&parts);
        evas_event_thaw(tev);
        evas_event_thaw_eval(tev);
        return 1;
     }
   else
     {
        evas_event_thaw(tev);
        evas_event_thaw_eval(tev);
        return 0;
     }
   ed->load_error = EDJE_LOAD_ERROR_NONE;
   _edje_entry_init(ed);
   evas_event_thaw(tev);
   evas_event_thaw_eval(tev);
   return 1;

on_error:
   eina_list_free(textblocks);
   eina_list_free(externals);
   eina_list_free(sources);
   eina_array_flush(&parts);
   _edje_thaw(ed);
   _edje_unblock(ed);
   _edje_unref(ed);
   _edje_file_del(ed);
   if (group_path_started)
     {
        const char *path;

        EINA_LIST_FREE(group_path, path)
           eina_stringshare_del(path);
     }
   evas_event_thaw(tev);
   evas_event_thaw_eval(tev);
   return 0;
}

void
_edje_file_add(Edje *ed, const Eina_File *f)
{
   if (!_edje_edd_edje_file) return;
   if (!f)
     {
        ed->load_error = EDJE_LOAD_ERROR_DOES_NOT_EXIST;
     }
   else
     {
        ed->file = _edje_cache_file_coll_open(f, ed->group,
                                              &(ed->load_error),
                                              &(ed->collection),
                                              ed);
     }

   if (!ed->collection)
     {
        if (ed->file)
          {
             _edje_cache_file_unref(ed->file);
             ed->file = NULL;
          }
     }
}

static int
_sort_defined_boxes(const void *a, const void *b)
{
   const Edje_User_Defined *euda = a;
   const Edje_User_Defined *eudb = b;

   if (euda->part - eudb->part != 0)
     return euda->part - eudb->part;
   return euda->u.box.index - eudb->u.box.index;
}

static Eina_List *
_edje_object_collect(Edje *ed)
{
   Edje_User_Defined *eud;
   Eina_List *collect;
   Eina_List *l;

   collect = ed->user_defined;
   ed->user_defined = NULL;

   EINA_LIST_FOREACH(collect, l, eud)
     {
        switch (eud->type)
          {
           case EDJE_USER_STRING:
              eud->u.string.text = eina_stringshare_ref(eud->u.string.text);
              break;
           case EDJE_USER_BOX_PACK:
              if (eud->u.box.index == -1)
                {
                   Edje_User_Defined *search;
                   Edje_Real_Part *rp;
                   Eina_List *children;
                   Eina_List *ls;
                   Evas_Object *child;
                   int idx = 0;

                   rp = _edje_real_part_recursive_get(&ed, eud->part);
                   if (rp->part->type != EDJE_PART_TYPE_BOX) continue ;

                   children = evas_object_box_children_get(rp->object);
                   EINA_LIST_FREE(children, child)
                      if (!evas_object_data_get(child, "\377 edje.box_item"))
                        {
                           EINA_LIST_FOREACH(l, ls, search)
                             {
                                if (search->type == EDJE_USER_BOX_PACK &&
                                    search->u.box.child == child &&
                                    search->part == eud->part /* beauty of stringshare ! */)
                                  {
                                     search->u.box.index = idx++;
                                     break;
                                  }
                             }
                           _edje_real_part_box_remove(eud->ed, rp, child);
                        }
                }
              break;
           case EDJE_USER_TABLE_PACK:
                {
                   Edje_Real_Part *rp;

                   rp = _edje_real_part_recursive_get(&ed, eud->part);
                   if (rp->part->type != EDJE_PART_TYPE_TABLE) continue ;

                   _edje_real_part_table_unpack(eud->ed, rp, eud->u.table.child);
                   break;
                }
           case EDJE_USER_SWALLOW:
              edje_object_part_unswallow(NULL, eud->u.swallow.child);
              break;
           case EDJE_USER_DRAG_STEP:
           case EDJE_USER_DRAG_PAGE:
           case EDJE_USER_DRAG_VALUE:
           case EDJE_USER_DRAG_SIZE:
              break;
          }
     }

   return collect;
}

void
_edje_file_del(Edje *ed)
{
   Edje_User_Defined *eud;
   Evas *tev = NULL;

   if (ed->obj) tev = evas_object_evas_get(ed->obj);

   ed->groups = eina_list_free(ed->groups);

   if (tev) evas_event_freeze(tev);
   if (ed->freeze_calc)
     {
        _edje_freeze_calc_list = eina_list_remove(_edje_freeze_calc_list, ed);
        ed->freeze_calc = EINA_FALSE;
        _edje_freeze_calc_count--;
     }
   _edje_entry_shutdown(ed);
   _edje_message_del(ed);
   _edje_block_violate(ed);
   _edje_var_shutdown(ed);
   //   if (ed->collection)
   //     {
   //        if (ed->collection->script) _edje_embryo_script_shutdown(ed);
   //     }

   if (!((ed->file) && (ed->collection)))
     {
        if (tev)
          {
             evas_event_thaw(tev);
             evas_event_thaw_eval(tev);
          }
        return;
     }

   while (ed->user_defined)
     {
        eud = eina_list_data_get(ed->user_defined);
        _edje_user_definition_free(eud);
     }

   if (ed->table_parts)
     {
        unsigned int i;
        for (i = 0; i < ed->table_parts_size; i++)
          {
             Edje_Real_Part *rp;
#ifdef HAVE_EPHYSICS
             Evas_Object *face_obj;
#endif

             rp = ed->table_parts[i];

#ifdef HAVE_EPHYSICS
             EINA_LIST_FREE(rp->body_faces, face_obj)
                evas_object_del(face_obj);
#endif

             if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
               _edje_entry_real_part_shutdown(ed, rp);

             if ((rp->type == EDJE_RP_TYPE_CONTAINER) &&
                 (rp->typedata.container))
               {
                  if (rp->typedata.container->items)
                    {
                       /* evas_box/table handles deletion of objects */
                       rp->typedata.container->items = eina_list_free(rp->typedata.container->items);
                    }
                  if (rp->typedata.container->anim)
                    {
                       _edje_box_layout_free_data(rp->typedata.container->anim);
                       rp->typedata.container->anim = NULL;
                    }
                  free(rp->typedata.container);
               }
             else if ((rp->type == EDJE_RP_TYPE_TEXT) &&
                      (rp->typedata.text))
               {
                  eina_stringshare_del(rp->typedata.text->text);
                  eina_stringshare_del(rp->typedata.text->font);
                  eina_stringshare_del(rp->typedata.text->cache.in_str);
                  eina_stringshare_del(rp->typedata.text->cache.out_str);
                  eina_stringshare_del(rp->typedata.text->filter);
                  free(rp->typedata.text);
               }
             else if ((rp->type == EDJE_RP_TYPE_SWALLOW) &&
                      (rp->typedata.swallow))
               {
                  if (rp->typedata.swallow->swallowed_object)
                    {
                       /* Objects swallowed by the app do not get deleted,
                          but those internally swallowed (GROUP type) do. */
                       switch (rp->part->type)
                         {
                          case EDJE_PART_TYPE_EXTERNAL:
                            _edje_external_parsed_params_free(rp->typedata.swallow->swallowed_object, rp->param1.external_params);
                            if (rp->param2)
                              _edje_external_parsed_params_free(rp->typedata.swallow->swallowed_object, rp->param2->external_params);
                          case EDJE_PART_TYPE_GROUP:
                            evas_object_del(rp->typedata.swallow->swallowed_object);
                          default:
                            break;
                         }
                       _edje_real_part_swallow_clear(ed, rp);
                       rp->typedata.swallow->swallowed_object = NULL;
                    }
                  free(rp->typedata.swallow);
               }

             if (rp->object)
               {
                  _edje_callbacks_focus_del(rp->object, ed);
                  _edje_callbacks_del(rp->object, ed);
                  evas_object_del(rp->object);
               }

             if (rp->custom)
               {
                  // xxx: lua2
                  _edje_collection_free_part_description_clean(rp->part->type,
                                                               rp->custom->description,
                                                               ed->file->free_strings);
                  free(rp->custom->description);
                  rp->custom->description = NULL;
               }

             /* Cleanup optional part. */
             free(rp->drag);
             free(rp->param1.set);

             if (rp->param2)
               {
                  free(rp->param2->set);
                  eina_cow_free(_edje_calc_params_map_cow, (const Eina_Cow_Data **) &rp->param2->p.map);
#ifdef HAVE_EPHYSICS
                  eina_cow_free(_edje_calc_params_physics_cow, (const Eina_Cow_Data **) &rp->param2->p.physics);
#endif
               }
             eina_mempool_free(_edje_real_part_state_mp, rp->param2);

             if (rp->custom)
               {
                  free(rp->custom->set);
                  eina_cow_free(_edje_calc_params_map_cow, (const Eina_Cow_Data **) &rp->custom->p.map);
#ifdef HAVE_EPHYSICS
                  eina_cow_free(_edje_calc_params_physics_cow, (const Eina_Cow_Data **) &rp->custom->p.physics);
#endif
               }
             eina_mempool_free(_edje_real_part_state_mp, rp->custom);

             if (rp->current)
               {
                  eina_cow_free(_edje_calc_params_map_cow, (const Eina_Cow_Data **) &rp->current->map);
#ifdef HAVE_EPHYSICS
                  eina_cow_free(_edje_calc_params_physics_cow, (const Eina_Cow_Data **) &rp->current->physics);
#endif
                  free(rp->current);
                  rp->current = NULL;
               }
             _edje_unref(ed);
             eina_cow_free(_edje_calc_params_map_cow, (const Eina_Cow_Data **) &rp->param1.p.map);
#ifdef HAVE_EPHYSICS
             eina_cow_free(_edje_calc_params_physics_cow, (const Eina_Cow_Data **) &rp->param1.p.physics);
#endif
             eina_mempool_free(_edje_real_part_mp, rp);
          }
     }
   if ((ed->file) && (ed->collection))
     {
        Edje_Part *ep;
        unsigned int i;

        _edje_textblock_styles_del(ed);
        for (i = 0; i < ed->collection->parts_count; ++i)
          {
             ep = ed->collection->parts[i];

             _edje_text_part_on_del(ed, ep);
             _edje_color_class_on_del(ed, ep);
          }

        _edje_cache_coll_unref(ed->file, ed->collection);
        ed->collection = NULL;
     }
   if (ed->file)
     {
#ifdef HAVE_EIO
        ed->file->edjes = eina_list_remove(ed->file->edjes, ed);
#endif
        _edje_cache_file_unref(ed->file);
        ed->file = NULL;
     }
   if (ed->actions)
     {
        Edje_Running_Program *runp;

        EINA_LIST_FREE(ed->actions, runp)
          {
             _edje_anim_count--;
             free(runp);
          }
     }
   _edje_animators = eina_list_remove(_edje_animators, ed);
   if (ed->pending_actions)
     {
        Edje_Pending_Program *pp;

        EINA_LIST_FREE(ed->pending_actions, pp)
          {
             ecore_timer_del(pp->timer);
             free(pp);
          }
     }

   if (ed->L) _edje_lua2_script_shutdown(ed);
   while (ed->subobjs) _edje_subobj_unregister(ed, ed->subobjs->data);
   if (ed->table_parts) free(ed->table_parts);
   ed->table_parts = NULL;
   ed->table_parts_size = 0;
   ed->focused_part = NULL;
   if (tev)
     {
        evas_event_thaw(tev);
        evas_event_thaw_eval(tev);
     }
}

void
_edje_file_free(Edje_File *edf)
{
   Edje_Color_Class *ecc;
#ifdef HAVE_EIO
   Ecore_Event_Handler *event;
#endif

#define HASH_FREE(Hash)                         \
   if (Hash) eina_hash_free(Hash);              \
   Hash = NULL;

   /* Clean cache before cleaning memory pool */
   if (edf->collection_cache) _edje_cache_coll_flush(edf);

   HASH_FREE(edf->fonts);
   HASH_FREE(edf->collection);
   HASH_FREE(edf->data);

   if (edf->image_dir)
     {
        unsigned int i;

        if (edf->free_strings)
          {
             for (i = 0; i < edf->image_dir->entries_count; ++i)
               eina_stringshare_del(edf->image_dir->entries[i].entry);
          }

        /* Sets have been added after edje received eet dictionary support */
        for (i = 0; i < edf->image_dir->sets_count; ++i)
          {
             Edje_Image_Directory_Set_Entry *se;

             EINA_LIST_FREE(edf->image_dir->sets[i].entries, se)
                free(se);

          }

        free(edf->image_dir->entries);
        free(edf->image_dir->sets);
        free(edf->image_dir);
     }
   if (edf->sound_dir)
     {
        unsigned int i;

        if (edf->free_strings)
          {
             for (i = 0; i < edf->sound_dir->samples_count; ++i)
               {
                  eina_stringshare_del(edf->sound_dir->samples[i].name);
                  eina_stringshare_del(edf->sound_dir->samples[i].snd_src);
               }

             for (i = 0; i < edf->sound_dir->tones_count; ++i)
               eina_stringshare_del(edf->sound_dir->tones[i].name);
          }
        free(edf->sound_dir->samples);
        free(edf->sound_dir->tones);
        free(edf->sound_dir);
     }

   if (edf->external_dir)
     {
        if (edf->external_dir->entries) free(edf->external_dir->entries);
        free(edf->external_dir);
     }

   eina_hash_free(edf->color_hash);
   EINA_LIST_FREE(edf->color_classes, ecc)
     {
        if (edf->free_strings && ecc->name) eina_stringshare_del(ecc->name);
        free(ecc);
     }

   if (edf->collection_patterns) edje_match_patterns_free(edf->collection_patterns);
#ifdef HAVE_EIO
   if (edf->timeout) ecore_timer_del(edf->timeout);
   EINA_LIST_FREE(edf->handlers, event)
      ecore_event_handler_del(event);
   eio_monitor_del(edf->monitor);
#endif
   if (edf->path) eina_stringshare_del(edf->path);
   if (edf->free_strings && edf->compiler) eina_stringshare_del(edf->compiler);
   _edje_textblock_style_cleanup(edf);
   if (edf->ef) eet_close(edf->ef);
   if (edf->f) eina_file_close(edf->f);
   free(edf);
}

static void
_edje_program_free(Edje_Program *pr, Eina_Bool free_strings)
{
   Edje_Program_Target *prt;
   Edje_Program_After *pa;

   if (free_strings)
     {
        if (pr->name) eina_stringshare_del(pr->name);
        if (pr->signal) eina_stringshare_del(pr->signal);
        if (pr->source) eina_stringshare_del(pr->source);
        if (pr->filter.part) eina_stringshare_del(pr->filter.part);
        if (pr->filter.state) eina_stringshare_del(pr->filter.state);
        if (pr->state) eina_stringshare_del(pr->state);
        if (pr->state2) eina_stringshare_del(pr->state2);
        if (pr->sample_name) eina_stringshare_del(pr->sample_name);
        if (pr->tone_name) eina_stringshare_del(pr->tone_name);
     }
   EINA_LIST_FREE(pr->targets, prt)
      free(prt);
   EINA_LIST_FREE(pr->after, pa)
      free(pa);
   free(pr);
}

void
_edje_collection_free(Edje_File *edf, Edje_Part_Collection *ec, Edje_Part_Collection_Directory_Entry *ce)
{
   unsigned int i;

   _edje_embryo_script_shutdown(ec);

#define EDJE_LOAD_PROGRAM_FREE(Array, Ec, It, FreeStrings)      \
   for (It = 0; It < Ec->programs.Array##_count; ++It)          \
   _edje_program_free(Ec->programs.Array[It], FreeStrings);     \
   free(Ec->programs.Array);

   EDJE_LOAD_PROGRAM_FREE(fnmatch, ec, i, edf->free_strings);
   EDJE_LOAD_PROGRAM_FREE(strcmp, ec, i, edf->free_strings);
   EDJE_LOAD_PROGRAM_FREE(strncmp, ec, i, edf->free_strings);
   EDJE_LOAD_PROGRAM_FREE(strrncmp, ec, i, edf->free_strings);
   EDJE_LOAD_PROGRAM_FREE(nocmp, ec, i, edf->free_strings);

   for (i = 0; i < ec->parts_count; ++i)
     {
        Edje_Part *ep;
        unsigned int j;

        ep = ec->parts[i];

        if (edf->free_strings && ep->name) eina_stringshare_del(ep->name);
        if (ep->default_desc)
          {
             _edje_collection_free_part_description_clean(ep->type, ep->default_desc, edf->free_strings);
             ep->default_desc = NULL;
          }
        for (j = 0; j < ep->other.desc_count; ++j)
          _edje_collection_free_part_description_clean(ep->type, ep->other.desc[j], edf->free_strings);

        free(ep->other.desc);
        /* Alloc for RTL objects in edje_calc.c:_edje_part_description_find() */
        if(ep->other.desc_rtl)
          free(ep->other.desc_rtl);

	for (j = 0; j < ep->items_count; ++j)
	  free(ep->items[j]);
        free(ep->items);
        // technically need this - but we ASSUME we use "one_big" so everything gets
        // freed in one go lower down when we del the mempool... but what if pool goes
        // "over"?
        eina_mempool_free(ce->mp.part, ep);
     }
   free(ec->parts);
   ec->parts = NULL;

   if (ec->data)
     {
        Eina_Iterator *it;
        Edje_String *es;

        it = eina_hash_iterator_data_new(ec->data);
        EINA_ITERATOR_FOREACH(it, es)
           free(es);
        eina_iterator_free(it);

        eina_hash_free(ec->data);
     }
#ifdef EDJE_PROGRAM_CACHE
   if (ec->prog_cache.no_matches) eina_hash_free(ec->prog_cache.no_matches);
   if (ec->prog_cache.matches)
     {
        eina_hash_foreach(ec->prog_cache.matches,
                          _edje_collection_free_prog_cache_matches_free_cb,
                          NULL);
        eina_hash_free(ec->prog_cache.matches);
     }
#endif
   _edje_programs_patterns_clean(ec);
   if (ec->patterns.table_programs) free(ec->patterns.table_programs);
   ec->patterns.table_programs = NULL;
   ec->patterns.table_programs_size = 0;

   if (ec->script) embryo_program_free(ec->script);
   _edje_lua2_script_unload(ec);

   eina_hash_free(ec->alias);
   eina_hash_free(ec->aliased);

   /* Destroy all part and description. */
   edje_cache_emp_free(ce);
   free(ec);
}

void
_edje_collection_free_part_description_clean(int type, Edje_Part_Description_Common *desc, Eina_Bool free_strings)
{
   unsigned int i;

   if (free_strings && desc->color_class) eina_stringshare_del(desc->color_class);
   //clean the map colors
   if (desc->map.colors)
     {
        for (i = 0; i < desc->map.colors_count; i++)
          free(desc->map.colors[i]);
        free(desc->map.colors);
     }

   switch (type)
     {
      case EDJE_PART_TYPE_IMAGE:
           {
              Edje_Part_Description_Image *img;

              img = (Edje_Part_Description_Image *) desc;

              for (i = 0; i < img->image.tweens_count; ++i)
                free(img->image.tweens[i]);
              free(img->image.tweens);
              break;
           }
      case EDJE_PART_TYPE_EXTERNAL:
           {
              Edje_Part_Description_External *external;

              external = (Edje_Part_Description_External *) desc;

              if (external->external_params)
                _edje_external_params_free(external->external_params, free_strings);
              break;
           }
      case EDJE_PART_TYPE_TEXT:
      case EDJE_PART_TYPE_TEXTBLOCK:
         if (free_strings)
           {
              Edje_Part_Description_Text *text;

              text = (Edje_Part_Description_Text *) desc;

              eina_stringshare_del(text->text.text.str);
              eina_stringshare_del(text->text.text_class);
              eina_stringshare_del(text->text.style.str);
              eina_stringshare_del(text->text.font.str);
              eina_stringshare_del(text->text.filter.str);
           }
         break;
     }
}

void
_edje_collection_free_part_description_free(int type,
                                            Edje_Part_Description_Common *desc,
                                            Edje_Part_Collection_Directory_Entry *ce,
                                            Eina_Bool free_strings)
{
#define FREE_POOL(Type, Ce, Desc)                                     \
 case EDJE_PART_TYPE_##Type: eina_mempool_free(Ce->mp.Type, Desc);    \
                             ce->count.Type--;                        \
   break;

   _edje_collection_free_part_description_clean(type, desc, free_strings);

   switch (type)
     {
        FREE_POOL(RECTANGLE, ce, desc);
        FREE_POOL(TEXT, ce, desc);
        FREE_POOL(IMAGE, ce, desc);
        FREE_POOL(PROXY, ce, desc);
        FREE_POOL(SWALLOW, ce, desc);
        FREE_POOL(TEXTBLOCK, ce, desc);
        FREE_POOL(GROUP, ce, desc);
        FREE_POOL(BOX, ce, desc);
        FREE_POOL(TABLE, ce, desc);
        FREE_POOL(EXTERNAL, ce, desc);
     }
}

#ifdef EDJE_PROGRAM_CACHE
static Eina_Bool
_edje_collection_free_prog_cache_matches_free_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   eina_list_free((Eina_List *)data);
   return EINA_TRUE;
   key = NULL;
   hash = NULL;
   fdata = NULL;
}
#endif

static void
_edje_object_pack_item_hints_set(Evas_Object *obj, Edje_Pack_Element *it)
{
   Evas_Coord w = 0, h = 0, minw, minh;
   Evas_Aspect_Control mode = EVAS_ASPECT_CONTROL_NONE;

   minw = it->min.w;
   minh = it->min.h;

   if ((minw <= 0) && (minh <= 0))
     {
        edje_object_size_min_get(obj, &w, &h);
        if ((w <= 0) && (h <= 0))
          edje_object_size_min_calc(obj, &w, &h);
     }
   else
     {
        w = minw;
        h = minh;
     }
   if (((minw <= 0) && (minh <= 0)) && ((w > 0) || (h > 0)))
     evas_object_size_hint_min_set(obj, w, h);
   else
     evas_object_size_hint_min_set(obj, minw, minh);

   evas_object_size_hint_request_set(obj, it->prefer.w, it->prefer.h);
   evas_object_size_hint_max_set(obj, it->max.w, it->max.h);
   evas_object_size_hint_padding_set(obj, it->padding.l, it->padding.r, it->padding.t, it->padding.b);
   evas_object_size_hint_align_set(obj, TO_DOUBLE(it->align.x), TO_DOUBLE(it->align.y));
   evas_object_size_hint_weight_set(obj, TO_DOUBLE(it->weight.x), TO_DOUBLE(it->weight.y));

   switch (it->aspect.mode)
     {
      case EDJE_ASPECT_CONTROL_NONE: mode = EVAS_ASPECT_CONTROL_NONE; break;
      case EDJE_ASPECT_CONTROL_NEITHER: mode = EVAS_ASPECT_CONTROL_NEITHER; break;
      case EDJE_ASPECT_CONTROL_HORIZONTAL: mode = EVAS_ASPECT_CONTROL_HORIZONTAL; break;
      case EDJE_ASPECT_CONTROL_VERTICAL: mode = EVAS_ASPECT_CONTROL_VERTICAL; break;
      case EDJE_ASPECT_CONTROL_BOTH: mode = EVAS_ASPECT_CONTROL_BOTH; break;
     }
   evas_object_size_hint_aspect_set(obj, mode, it->aspect.w, it->aspect.h);

   evas_object_resize(obj, w, h);
}

static const char *
_edje_find_alias(Eina_Hash *aliased, char *src, int *length)
{
   const char *alias;
   char *search;

   *length = strlen(src);
   if (*length == 0) return NULL;

   alias = eina_hash_find(aliased, src);
   if (alias) return alias;

   search = strrchr(src, EDJE_PART_PATH_SEPARATOR);
   if (search == NULL) return NULL;

   *search = '\0';
   alias = _edje_find_alias(aliased, src, length);
   *search = EDJE_PART_PATH_SEPARATOR;

   return alias;
}

static void
_cb_signal_repeat(void *data, Evas_Object *obj, const char *sig, const char *source)
{
   Edje_Pack_Element *pack_it;
   Evas_Object  *parent;
   Edje         *ed;
   Edje         *ed_parent;
   char          new_src[4096]; /* XXX is this max reasonable? */
   size_t        length_parent = 0;
   size_t        length_index = 0;
   size_t        length_source;
   int           i = 0;
   const char   *alias = NULL;
   const char   *name = NULL;
   Edje_Message_Signal emsg;

   parent = data;
   ed = _edje_fetch(obj);
   if (!ed) return;

   pack_it = evas_object_data_get(obj, "\377 edje.box_item");
   if (!pack_it) pack_it = evas_object_data_get(obj, "\377 edje.table_item");
   name = evas_object_name_get(obj);

   if (pack_it)
     {
        if (!name) name = pack_it->name;
        if (!name)
          {
             Eina_Iterator *it = NULL;
             Evas_Object *o;

             if (pack_it->parent->part->type == EDJE_PART_TYPE_BOX)
               {
                  it = evas_object_box_iterator_new(pack_it->parent->object);
               }
             else if (pack_it->parent->part->type == EDJE_PART_TYPE_TABLE)
               {
                  it = evas_object_table_iterator_new(pack_it->parent->object);
               }

             EINA_ITERATOR_FOREACH(it, o)
               {
                  if (o == obj) break;
                  i++;
               }
             eina_iterator_free(it);

             length_index = 12;
          }
        else
          {
             length_index = strlen(name) + 2;
          }
     }

   /* Replace snprint("%s%c%s") == memcpy + *new_src + memcat */
   if (ed->parent)
     length_parent = strlen(ed->parent);
   length_source = strlen(source);
   if (length_source + length_parent + 2 + length_index > sizeof(new_src))
     return;

   if (ed->parent)
     memcpy(new_src, ed->parent, length_parent);
   if (ed->parent && length_index)
     {
        new_src[length_parent++] = EDJE_PART_PATH_SEPARATOR_INDEXL;
        if (length_index == 12)
          length_parent += eina_convert_itoa(i, new_src + length_parent);
        else
          {
             memcpy(new_src + length_parent, name, length_index - 2);
             length_parent += length_index - 2;
          }
        new_src[length_parent++] = EDJE_PART_PATH_SEPARATOR_INDEXR;
     }

   new_src[length_parent] = EDJE_PART_PATH_SEPARATOR;
   memcpy(new_src + length_parent + 1, source, length_source + 1);

   /* Handle alias renaming */
   ed_parent = _edje_fetch(parent);
   if (ed_parent && ed_parent->collection && ed_parent->collection->aliased)
     {
        int length;

        alias = _edje_find_alias(ed_parent->collection->aliased, new_src, &length);

        if (alias)
          {
             int origin;

             /* Add back the end of the source */
             origin = strlen(new_src);
             length ++; /* Remove the trailing ':' from the count */
             if (origin > length)
               {
                  char *tmp;
                  size_t alias_length;

                  alias_length = strlen(alias);
                  tmp = alloca(alias_length + origin - length + 2);
                  memcpy(tmp, alias, alias_length);
                  tmp[alias_length] = EDJE_PART_PATH_SEPARATOR;
                  memcpy(tmp + alias_length + 1, new_src + length, origin - length + 1);

                  alias = tmp;
               }
          }
     }

   emsg.sig = sig;
   emsg.src = alias ? alias : new_src;
   emsg.data = NULL;
   if (ed_parent)
     _edje_message_send(ed_parent, EDJE_QUEUE_SCRIPT,
                        EDJE_MESSAGE_SIGNAL, 0, &emsg);
}
