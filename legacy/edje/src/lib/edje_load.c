/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <string.h>

#include "edje_private.h"

void _edje_collection_free_part_description_free(Edje_Part_Description *desc, unsigned int free_strings);
static Eina_Bool _edje_file_collection_hash_foreach(const Eina_Hash *hash, const void *key, void *data, void *fdata);
#ifdef EDJE_PROGRAM_CACHE
static Eina_Bool  _edje_collection_free_prog_cache_matches_free_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata);
#endif
static int _edje_object_file_set_internal(Evas_Object *obj, const char *file, const char *group, Eina_List *group_path);
static void _edje_object_pack_item_hints_set(Evas_Object *obj, Edje_Pack_Element *it);
static void _cb_signal_repeat(void *data, Evas_Object *obj, const char *signal, const char *source);

static Eina_List *_edje_swallows_collect(Edje *ed);

/************************** API Routines **************************/

/* FIXDOC: Verify/expand doc */
/** Sets the EET file and group to load @a obj from
 * @param obj A valid Evas_Object handle
 * @param file The path to the EET file
 * @param group The group name in the Edje
 * @return 0 on Error\n
 * 1 on Success and sets EDJE_LOAD_ERROR_NONE
 *
 * Edje uses EET files, conventionally ending in .edj, to store object
 * descriptions. A single file contains multiple named groups. This function
 * specifies the file and group name to load @a obj from.
 */
EAPI int
edje_object_file_set(Evas_Object *obj, const char *file, const char *group)
{
   return _edje_object_file_set_internal(obj, file, group, NULL);
}

/* FIXDOC: Verify/expand doc. */
/** Get the file and group name that @a obj was loaded from
 * @param obj A valid Evas_Object handle
 * @param file A pointer to store a pointer to the filename in
 * @param group A pointer to store a pointer to the group name in
 *
 * This gets the EET file location and group for the given Evas_Object.
 * If @a obj is either not an edje file, or has not had its file/group set
 * using edje_object_file_set(), then both @a file and @a group will be set
 * to NULL.
 *
 * It is valid to pass in NULL for either @a file or @a group if you are not
 * interested in one of the values.
 */
EAPI void
edje_object_file_get(const Evas_Object *obj, const char **file, const char **group)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed)
     {
	if (file) *file = NULL;
	if (group) *group = NULL;
	return;
     }
   if (file) *file = ed->path;
   if (group) *group = ed->group;
}

/* FIXDOC: Verify. return error? */
/** Gets the Edje load error
 * @param obj A valid Evas_Object handle
 *
 * @return The Edje load error:\n
 * 0: No Error\n
 * 1: Generic Error\n
 * 2: Does not Exist\n
 * 3: Permission Denied\n
 * 4: Resource Allocation Failed\n
 * 5: Corrupt File\n
 * 6: Unknown Format\n
 * 7: Incompatible File
 */
EAPI int
edje_object_load_error_get(const Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return EDJE_LOAD_ERROR_NONE;
   return ed->load_error;
}

/** Get a list of groups in an edje file
 * @param file The path to the edje file
 *
 * @return The Eina_List of group names (char *)
 *
 * Note: the list must be freed using edje_file_collection_list_free()
 * when you are done with it.
 */
EAPI Eina_List *
edje_file_collection_list(const char *file)
{
   Eina_List *lst = NULL;
   Edje_File *edf;
   int error_ret = 0;

   if ((!file) || (!*file)) return NULL;
   edf = _edje_cache_file_coll_open(file, NULL, &error_ret, NULL);
   if (edf != NULL)
     {
	if (edf->collection_dir)
	  {
	     Eina_List *l;
	     Edje_Part_Collection_Directory_Entry *ce;

	     EINA_LIST_FOREACH(edf->collection_dir->entries, l, ce)
	       lst = eina_list_append(lst, eina_stringshare_add(ce->entry));
	  }
	_edje_cache_file_unref(edf);
     }
   return lst;
}

/** Free file collection list
 * @param lst The Eina_List of groups
 *
 * Frees the list returned by edje_file_collection_list().
 */
EAPI void
edje_file_collection_list_free(Eina_List *lst)
{
   while (lst)
     {
        if (eina_list_data_get(lst)) eina_stringshare_del(eina_list_data_get(lst));
	lst = eina_list_remove(lst, eina_list_data_get(lst));
     }
}

/** Determine whether a group matching glob exists in an edje file.
 * @param file The file path
 * @param glob A glob to match on
 *
 * @return 1 if a match is found, 0 otherwise
 */
EAPI int
edje_file_group_exists(const char *file, const char *glob)
{
   Edje_File *edf;
   int error_ret = 0;

   if ((!file) || (!*file)) return 0;
   edf = _edje_cache_file_coll_open(file, NULL, &error_ret, NULL);
   if (edf != NULL)
     {
	if (edf->collection_dir)
	  {
             Edje_Patterns *patterns;

             patterns =
               edje_match_collection_dir_init(edf->collection_dir->entries);
             if (edje_match_collection_dir_exec(patterns, glob))
               {
                  edje_match_patterns_free(patterns);
                  _edje_cache_file_unref(edf);
                  return 1;
               }
             edje_match_patterns_free(patterns);
	  }
	_edje_cache_file_unref(edf);
     }
   return 0;
}


/** Get data from the file level data block of an edje file
 * @param file The path to the .edj file
 * @param key The data key
 * @return The string value of the data
 *
 * If an edje file is built from the following edc:
 *
 * data {
 *   item: "key1" "value1";
 *   item: "key2" "value2";
 * }
 * collections { ... }
 *
 * Then, edje_file_data_get("key1") will return "value1"
 */
EAPI char *
edje_file_data_get(const char *file, const char *key)
{
   Edje_File *edf;
   char *str = NULL;
   int error_ret = 0;

   if (key)
     {
	edf = _edje_cache_file_coll_open(file, NULL, &error_ret, NULL);
	if (edf != NULL)
	  {
	     if (edf->data_cache != NULL)
	       {
		  str = eina_hash_find(edf->data_cache, key);
		  if (str) str = strdup(str);
	       }
	     _edje_cache_file_unref(edf);
	  }
     }
   return str;
}

static void
_edje_programs_patterns_clean(Edje *ed)
{
   _edje_signals_sources_patterns_clean(&ed->patterns.programs);

   eina_rbtree_delete(ed->patterns.programs.exact_match,
		      EINA_RBTREE_FREE_CB(edje_match_signal_source_free),
		      NULL);
   ed->patterns.programs.exact_match = NULL;

   ed->patterns.programs.globing = eina_list_free(ed->patterns.programs.globing);
}

static void
_edje_programs_patterns_init(Edje *ed)
{
   Edje_Signals_Sources_Patterns *ssp = &ed->patterns.programs;

   if (ssp->signals_patterns)
     return;

   ssp->globing = edje_match_program_hash_build(ed->collection->programs,
							  &ssp->exact_match);

   ssp->signals_patterns = edje_match_programs_signal_init(ssp->globing);
   ssp->sources_patterns = edje_match_programs_source_init(ssp->globing);
}

static int
_edje_object_file_set_internal(Evas_Object *obj, const char *file, const char *group, Eina_List *group_path)
{
   Edje *ed;
   int n;
   Eina_List *parts = NULL;
   Eina_List *old_swallows;
   int group_path_started = 0;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   if (!file) file = "";
   if (!group) group = "";
   if (((ed->path) && (!strcmp(file, ed->path))) &&
	(ed->group) && (!strcmp(group, ed->group)))
     return 1;

   old_swallows = _edje_swallows_collect(ed);

   if (_edje_script_only(ed)) _edje_script_only_shutdown(ed);
   _edje_file_del(ed);

   if (ed->path) eina_stringshare_del(ed->path);
   if (ed->group) eina_stringshare_del(ed->group);
   ed->path = eina_stringshare_add(file);
   ed->group = eina_stringshare_add(group);

   ed->load_error = EDJE_LOAD_ERROR_NONE;
  _edje_file_add(ed);

   _edje_textblock_styles_add(ed);
   _edje_textblock_style_all_update(ed);

   if (ed->collection)
     {
	if (ed->collection->script_only)
	  {
	     ed->load_error = EDJE_LOAD_ERROR_NONE;
	     _edje_script_only_init(ed);
	  }
	else
	  {
	     Eina_List *l;
	     int i;
	     int errors = 0;
	     Edje_Part *ep;

	     /* colorclass stuff */
	     EINA_LIST_FOREACH(ed->collection->parts, l, ep)
	       {
		  Eina_List *hist = NULL;
		  Edje_Part_Description *desc;

		  if (errors)
		    break;
		  /* Register any color classes in this parts descriptions. */
		  if ((ep->default_desc) && (ep->default_desc->color_class))
		    _edje_color_class_member_add(ed, ep->default_desc->color_class);

		  EINA_LIST_FOREACH(ep->other_desc, hist, desc)
		    if (desc->color_class)
		      _edje_color_class_member_add(ed, desc->color_class);
	       }
	     /* build real parts */
	     for (n = 0, l = ed->collection->parts; l; l = eina_list_next(l), n++)
	       {
		  Edje_Part *ep;
		  Edje_Real_Part *rp;

		  ep = eina_list_data_get(l);
		  rp = calloc(1, sizeof(Edje_Real_Part));
		  if (!rp)
		    {
		       ed->load_error = EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
		       return 0;
		    }
		  rp->edje = ed;
		  _edje_ref(rp->edje);
		  rp->part = ep;
		  parts = eina_list_append(parts, rp);
		  rp->param1.description = ep->default_desc;
		  rp->chosen_description = rp->param1.description;
		  if (!rp->param1.description)
		    printf("EDJE ERROR: no default part description!\n");
		  if (ep->type == EDJE_PART_TYPE_RECTANGLE)
		    rp->object = evas_object_rectangle_add(ed->evas);
		  else if (ep->type == EDJE_PART_TYPE_IMAGE)
		    rp->object = evas_object_image_add(ed->evas);
		  else if (ep->type == EDJE_PART_TYPE_TEXT)
		    {
		       _edje_text_part_on_add(ed, rp);
		       rp->object = evas_object_text_add(ed->evas);
		       evas_object_text_font_source_set(rp->object, ed->path);
		    }
		  else if (ep->type == EDJE_PART_TYPE_SWALLOW || ep->type == EDJE_PART_TYPE_GROUP)
		    {
		       rp->object = evas_object_rectangle_add(ed->evas);
		       evas_object_color_set(rp->object, 0, 0, 0, 0);
		       evas_object_pass_events_set(rp->object, 1);
		       evas_object_pointer_mode_set(rp->object, EVAS_OBJECT_POINTER_MODE_NOGRAB);
		    }
		  else if (ep->type == EDJE_PART_TYPE_TEXTBLOCK)
		    rp->object = evas_object_textblock_add(ed->evas);
		  else if (ep->type == EDJE_PART_TYPE_GRADIENT)
		    rp->object = evas_object_gradient_add(ed->evas);
		  else if (ep->type == EDJE_PART_TYPE_BOX)
		    {
		       rp->object = evas_object_box_add(ed->evas);
		    }
		  else if (ep->type == EDJE_PART_TYPE_TABLE)
		    rp->object = evas_object_table_add(ed->evas);
		  else
		    printf("EDJE ERROR: wrong part type %i!\n", ep->type);
		  if (rp->object)
		    {
		       evas_object_smart_member_add(rp->object, ed->obj);
//		       evas_object_layer_set(rp->object, evas_object_layer_get(ed->obj));
		       if (ep->type != EDJE_PART_TYPE_SWALLOW && ep->type != EDJE_PART_TYPE_GROUP)
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
			 evas_object_clip_set(rp->object, ed->clipper);
		    }
		  rp->drag.step.x = ep->dragable.step_x;
		  rp->drag.step.y = ep->dragable.step_y;
		  rp->gradient_id = -1;
	       }
	     if (n > 0)
	       {
		  Edje_Real_Part *rp;
		  ed->table_parts = malloc(sizeof(Edje_Real_Part *) * n);
		  ed->table_parts_size = n;
		  /* FIXME: check malloc return */
		  n = 0;
		  EINA_LIST_FOREACH(parts, l, rp)
		    {
		       ed->table_parts[n] = rp;
		       n++;
		    }
		  eina_list_free(parts);
		  for (i = 0; i < ed->table_parts_size; i++)
		    {
		       rp = ed->table_parts[i];
		       if (rp->param1.description->rel1.id_x >= 0)
			 rp->param1.rel1_to_x = ed->table_parts[rp->param1.description->rel1.id_x % ed->table_parts_size];
		       if (rp->param1.description->rel1.id_y >= 0)
			 rp->param1.rel1_to_y = ed->table_parts[rp->param1.description->rel1.id_y % ed->table_parts_size];
		       if (rp->param1.description->rel2.id_x >= 0)
			 rp->param1.rel2_to_x = ed->table_parts[rp->param1.description->rel2.id_x % ed->table_parts_size];
		       if (rp->param1.description->rel2.id_y >= 0)
			 rp->param1.rel2_to_y = ed->table_parts[rp->param1.description->rel2.id_y % ed->table_parts_size];
		       _edje_text_part_on_add_clippers(ed, rp);
		       if (rp->part->clip_to_id >= 0)
			 {
			    rp->clip_to = ed->table_parts[rp->part->clip_to_id % ed->table_parts_size];
			    if (rp->clip_to)
			      {
				 evas_object_pass_events_set(rp->clip_to->object, 1);
				 evas_object_pointer_mode_set(rp->clip_to->object, EVAS_OBJECT_POINTER_MODE_NOGRAB);
				 evas_object_clip_set(rp->object, rp->clip_to->object);
			      }
			 }
		       if (rp->part->dragable.confine_id >= 0)
			 rp->confine_to = ed->table_parts[rp->part->dragable.confine_id % ed->table_parts_size];
		       
		       /* replay events for dragable */
		       if (rp->part->dragable.events_id >= 0)
			 {
			    rp->events_to =
			      ed->table_parts[rp->part->dragable.events_id % ed->table_parts_size];
			    /* events_to may be used only with dragable */
			    if (!rp->events_to->part->dragable.x &&
				!rp->events_to->part->dragable.y)
			      rp->events_to = NULL;
			 }
		       
		       rp->swallow_params.min.w = 0;
		       rp->swallow_params.min.w = 0;
		       rp->swallow_params.max.w = -1;
		       rp->swallow_params.max.h = -1;
		       
		       if (ed->file->feature_ver < 1)
			 {
			    rp->param1.description->text.id_source = -1;
			    rp->param1.description->text.id_text_source = -1;
			 }
		       if (rp->param1.description->text.id_source >= 0)
			 rp->text.source = ed->table_parts[rp->param1.description->text.id_source % ed->table_parts_size];
		       if (rp->param1.description->text.id_text_source >= 0)
			 rp->text.text_source = ed->table_parts[rp->param1.description->text.id_text_source % ed->table_parts_size];
		       if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
			 _edje_entry_real_part_init(rp);
		    }
	       }
	     
	     _edje_programs_patterns_init(ed);
	     
	     n = eina_list_count(ed->collection->programs);
	     if (n > 0)
	       {
		  Edje_Program *pr;
		  /* FIXME: keeping a table AND a list is just bad - nuke list */
		  ed->table_programs = malloc(sizeof(Edje_Program *) * n);
		  ed->table_programs_size = n;
		  /* FIXME: check malloc return */
		  n = 0;
		  EINA_LIST_FOREACH(ed->collection->programs, l, pr)
		    {
		       ed->table_programs[n] = pr;
		       n++;
		    }
	       }
	     _edje_ref(ed);
	     _edje_block(ed);
	     _edje_freeze(ed);
	     if (ed->collection->script) _edje_embryo_script_init(ed);
	     _edje_var_init(ed);
	     for (i = 0; i < ed->table_parts_size; i++)
	       {
		  Edje_Real_Part *rp;
		  
		  rp = ed->table_parts[i];
		  evas_object_show(rp->object);
		  if (_edje_block_break(ed)) break;
		  if (rp->part->dragable.x < 0) rp->drag.val.x = 1.0;
		  if (rp->part->dragable.y < 0) rp->drag.val.x = 1.0;
		  _edje_dragable_pos_set(ed, rp, rp->drag.val.x, rp->drag.val.y);
	       }
	     ed->dirty = 1;
	     if ((evas_object_clipees_get(ed->clipper)) &&
		 (evas_object_visible_get(obj)))
	       evas_object_show(ed->clipper);
	     
	     /* instantiate 'internal swallows' */
	     for (i = 0; i < ed->table_parts_size; i++)
	       {
		  Edje_Real_Part *rp;
		  /* XXX: curr_item and pack_it don't require to be NULL since
		   * XXX: they are just used when source != NULL and type == BOX,
		   * XXX: and they're always set in this case, but GCC fails to
		   * XXX: notice that, so let's shut it up
		   */
		  Eina_List *curr_item = NULL;
		  Edje_Pack_Element *pack_it = NULL;
		  const char *source = NULL;
		  
		  rp = ed->table_parts[i];
		  if (rp->part->type != EDJE_PART_TYPE_GROUP &&
		      rp->part->type != EDJE_PART_TYPE_BOX &&
		      rp->part->type != EDJE_PART_TYPE_TABLE) continue;
		  if (rp->part->type == EDJE_PART_TYPE_GROUP)
		    source = rp->part->source;
		  else if (rp->part->type == EDJE_PART_TYPE_BOX ||
			   rp->part->type == EDJE_PART_TYPE_TABLE)
		    {
		       if (rp->part->items)
			 {
			    curr_item = rp->part->items;
			    pack_it = curr_item->data;
			    source = pack_it->source;
			 }
		    }
		  while (source)
		    {
		       Eina_List *l;
		       Evas_Object *child_obj;
		       Edje *child_ed;
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
				 _edje_thaw(ed);
				 _edje_unblock(ed);
				 _edje_unref(ed);
				 _edje_file_del(ed);
				 eina_stringshare_del(group_path_entry);
				 if (group_path_started)
				   {
				      eina_stringshare_del(eina_list_data_get(group_path));
				      eina_list_free(group_path);
				   }
				 ed->load_error = EDJE_LOAD_ERROR_RECURSIVE_REFERENCE;
				 return 0;
			      }
			 }
		       
		       child_obj = edje_object_add(ed->evas);
		       group_path = eina_list_append(group_path, group_path_entry);
		       if (!_edje_object_file_set_internal(child_obj, file, source, group_path))
			 {
			    _edje_thaw(ed);
			    _edje_unblock(ed);
			    _edje_unref(ed);
			    _edje_file_del(ed);
			    
			    if (group_path_started)
			      {
				 while (group_path)
				   {
				      eina_stringshare_del(eina_list_data_get(group_path));
				      group_path = eina_list_remove_list(group_path, group_path);
				   }
			      }
			    ed->load_error = edje_object_load_error_get(child_obj);
			    return 0;
			 }
		       child_ed = _edje_fetch(child_obj);
		       child_ed->parent = eina_stringshare_add(rp->part->name);
		       
		       group_path = eina_list_remove(group_path, group_path_entry);
		       eina_stringshare_del(group_path_entry);
		       
		       edje_object_signal_callback_add(child_obj, "*", "*", _cb_signal_repeat, obj);
		       if (rp->part->type == EDJE_PART_TYPE_GROUP)
			 {
			    _edje_real_part_swallow(rp, child_obj);
			    source = NULL;
			 }
		       else
			 {
			    _edje_object_pack_item_hints_set(child_obj, pack_it);
			    evas_object_show(child_obj);
			    if (pack_it->name)
			      evas_object_name_set(child_obj, pack_it->name);
			    if (rp->part->type == EDJE_PART_TYPE_BOX)
			      {
				 _edje_real_part_box_append(rp, child_obj);
				 evas_object_data_set(child_obj, "\377 edje.box_item", pack_it);
			      }
			    else if(rp->part->type == EDJE_PART_TYPE_TABLE)
			      {
				 _edje_real_part_table_pack(rp, child_obj, pack_it->col, pack_it->row, pack_it->colspan, pack_it->rowspan);
				 evas_object_data_set(child_obj, "\377 edje.table_item", pack_it);
			      }
			    rp->items = eina_list_append(rp->items, child_obj);
			    if (!(curr_item = curr_item->next))
			      source = NULL;
			    else
			      {
				 pack_it = curr_item->data;
				 source = pack_it->source;
			      }
			 }
		    }
	       }
	     
	     if (group_path_started)
	       {
		  while (group_path)
		    {
		       eina_stringshare_del(eina_list_data_get(group_path));
		       group_path = eina_list_remove_list(group_path, group_path);
		    }
	       }
	     
	     /* reswallow any swallows that existed before setting the file */
	     if (old_swallows)
	       {
		  while (old_swallows)
		    {
		       const char *name;
		       Evas_Object *swallow;
		       
		       name = eina_list_data_get(old_swallows);
		       old_swallows = eina_list_remove_list(old_swallows, old_swallows);
		       
		       swallow = eina_list_data_get(old_swallows);
		       old_swallows = eina_list_remove_list(old_swallows, old_swallows);
		       
		       edje_object_part_swallow(obj, name, swallow);
		       eina_stringshare_del(name);
		    }
	       }
	     
	     _edje_recalc(ed);
	     _edje_thaw(ed);
	     _edje_unblock(ed);
	     _edje_unref(ed);
	     ed->load_error = EDJE_LOAD_ERROR_NONE;
	     _edje_emit(ed, "load", NULL);
	     /* instantiate 'internal swallows' */
	     for (i = 0; i < ed->table_parts_size; i++)
	       {
		  Edje_Real_Part *rp;
		  
		  rp = ed->table_parts[i];
                  if ((rp->part->type == EDJE_PART_TYPE_TEXTBLOCK) &&
                      (rp->part->default_desc))
                    {
                       Edje_Style *stl  = NULL;
                       const char *style;
                       
                       style = rp->part->default_desc->text.style;
                       if (style)
                         {
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
	  }
	return 1;
     }
   else
     return 0;
   ed->load_error = EDJE_LOAD_ERROR_NONE;
   return 1;
}

void
_edje_file_add(Edje *ed)
{
   if (_edje_edd_edje_file == NULL) return;
   ed->file = _edje_cache_file_coll_open(ed->path, ed->group,
					 &(ed->load_error),
					 &(ed->collection));

   if (!ed->collection)
     {
	if (ed->file)
	  {
	     _edje_cache_file_unref(ed->file);
	     ed->file = NULL;
	  }
     }
}

static Eina_List *
_edje_swallows_collect(Edje *ed)
{
   Eina_List *swallows = NULL;
   int i;

   if (!ed->file || !ed->table_parts) return NULL;
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type != EDJE_PART_TYPE_SWALLOW || !rp->swallowed_object) continue;
	swallows = eina_list_append(swallows, eina_stringshare_add(rp->part->name));
	swallows = eina_list_append(swallows, rp->swallowed_object);
     }
   return swallows;
}

void
_edje_file_del(Edje *ed)
{
   _edje_message_del(ed);
   _edje_block_violate(ed);
   _edje_var_shutdown(ed);
   _edje_programs_patterns_clean(ed);

   if (!((ed->file) && (ed->collection))) return;
   if (ed->table_parts)
     {
	int i;
	for (i = 0; i < ed->table_parts_size; i++)
	  {
	     Edje_Real_Part *rp;

	     rp = ed->table_parts[i];
	     if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
	       _edje_entry_real_part_shutdown(rp);
	     if (rp->object)
	       {
		  _edje_text_real_part_on_del(ed, rp);
		  _edje_callbacks_del(rp->object);
		  evas_object_del(rp->object);
	       }
	     if (rp->swallowed_object)
	       {
		  evas_object_smart_member_del(rp->swallowed_object);
		  evas_object_event_callback_del(rp->swallowed_object,
						 EVAS_CALLBACK_FREE,
						 _edje_object_part_swallow_free_cb);
		  evas_object_clip_unset(rp->swallowed_object);
		  evas_object_data_del(rp->swallowed_object, "\377 edje.swallowing_part");
		  if (rp->part->mouse_events)
		     _edje_callbacks_del(rp->swallowed_object);

                  /* Objects swallowed by the app do not get deleted,
                   but those internally swallowed (GROUP type) do. */
		  if (rp->part->type == EDJE_PART_TYPE_GROUP)
		    evas_object_del(rp->swallowed_object);

		  rp->swallowed_object = NULL;
	       }
	     if (rp->items)
	       {
		  /* all internal, for now */
		  while (rp->items)
		    {
		       /* evas_box/table handles deletion of objects */
		       /*evas_object_del(rp->items->data);*/
		       rp->items = eina_list_remove_list(rp->items, rp->items);
		    }
	       }
	     if (rp->text.text) eina_stringshare_del(rp->text.text);
	     if (rp->text.font) eina_stringshare_del(rp->text.font);
	     if (rp->text.cache.in_str) eina_stringshare_del(rp->text.cache.in_str);
	     if (rp->text.cache.out_str) eina_stringshare_del(rp->text.cache.out_str);

	     if (rp->custom.description)
	       _edje_collection_free_part_description_free(rp->custom.description, ed->file->free_strings);

	     _edje_unref(rp->edje);
	     free(rp);
	  }
     }
   if ((ed->file) && (ed->collection))
     {
	Eina_List *l;
	Edje_Part *ep;

	_edje_textblock_styles_del(ed);
	EINA_LIST_FOREACH(ed->collection->parts, l, ep)
	  {
	     _edje_text_part_on_del(ed, ep);
	     _edje_color_class_on_del(ed, ep);
	  }
	_edje_cache_coll_unref(ed->file, ed->collection);
	ed->collection = NULL;
     }
   if (ed->file)
     {
	_edje_cache_file_unref(ed->file);
	ed->file = NULL;
     }
   if (ed->actions)
     {
	while (ed->actions)
	  {
	     Edje_Running_Program *runp;

	     _edje_anim_count--;
	     runp = eina_list_data_get(ed->actions);
	     ed->actions = eina_list_remove(ed->actions, runp);
	     free(runp);
	  }
     }
   _edje_animators = eina_list_remove(_edje_animators, ed);
   if (ed->pending_actions)
     {
	while (ed->pending_actions)
	  {
	     Edje_Pending_Program *pp;

	     pp = eina_list_data_get(ed->pending_actions);
	     ed->pending_actions = eina_list_remove(ed->pending_actions, pp);
	     ecore_timer_del(pp->timer);
	     free(pp);
	  }
     }
   if (ed->table_parts) free(ed->table_parts);
   ed->table_parts = NULL;
   ed->table_parts_size = 0;
   if (ed->table_programs) free(ed->table_programs);
   ed->table_programs = NULL;
   ed->table_programs_size = 0;
}
/**
 * Used to free the cached data values that are stored in the data_cache
 * hash table.
 */
static Eina_Bool data_cache_free(const Eina_Hash *hash, const void *key, void *data, void *fdata)
{
   Edje_File    *edf;

   edf = fdata;
   if (edf->free_strings) eina_stringshare_del(data);
   return 1;
}

void
_edje_file_free(Edje_File *edf)
{
   if (edf->font_dir)
     {
	while (edf->font_dir->entries)
	  {
	     Edje_Font_Directory_Entry *fe;

	     fe = eina_list_data_get(edf->font_dir->entries);
	     edf->font_dir->entries =
	       eina_list_remove_list(edf->font_dir->entries, edf->font_dir->entries);
	     eina_hash_del(edf->font_hash, fe->entry, edf);
	     if (edf->free_strings && fe->path) eina_stringshare_del(fe->path);
	     free(fe);
	  }
	free(edf->font_dir);
     }
   if (edf->font_hash)
     {
	eina_hash_free(edf->font_hash);
	edf->font_hash = NULL;
     }
   if (edf->image_dir)
     {
	while (edf->image_dir->entries)
	  {
	     Edje_Image_Directory_Entry *ie;

	     ie = eina_list_data_get(edf->image_dir->entries);
	     edf->image_dir->entries =
	       eina_list_remove_list(edf->image_dir->entries, edf->image_dir->entries);
	     if (edf->free_strings && ie->entry) eina_stringshare_del(ie->entry);
	     free(ie);
	  }
	free(edf->image_dir);
     }
   if (edf->collection_dir)
     {
	while (edf->collection_dir->entries)
	  {
	     Edje_Part_Collection_Directory_Entry *ce;

	     ce = eina_list_data_get(edf->collection_dir->entries);
	     edf->collection_dir->entries =
	       eina_list_remove_list(edf->collection_dir->entries, edf->collection_dir->entries);
	     if (edf->free_strings && ce->entry) eina_stringshare_del(ce->entry);
	     free(ce);
	  }
	free(edf->collection_dir);
     }
   if (edf->spectrum_dir)
     {
	while (edf->spectrum_dir->entries)
	  {
	     Edje_Spectrum_Directory_Entry *se;

	     se = eina_list_data_get(edf->spectrum_dir->entries);
	     edf->spectrum_dir->entries =
	       eina_list_remove_list(edf->spectrum_dir->entries, edf->spectrum_dir->entries);
	     while (se->color_list)
	       {
		 free(eina_list_data_get(se->color_list));
		  se->color_list =
                    eina_list_remove_list(se->color_list, se->color_list);
	       }
             if (edf->free_strings)
               {
                  if (se->entry) eina_stringshare_del(se->entry);
                  if (se->filename) eina_stringshare_del(se->filename);
               }
	     free(se);
	  }
	free(edf->spectrum_dir);
     }
   while (edf->data)
     {
	Edje_Data *edt;

	edt = eina_list_data_get(edf->data);
	edf->data = eina_list_remove(edf->data, edt);
        if (edf->free_strings)
          {
             if (edt->key) eina_stringshare_del(edt->key);
             if (edt->value) eina_stringshare_del(edt->value);
          }
	free(edt);
     }
   if (edf->data_cache)
     {
	eina_hash_foreach(edf->data_cache, data_cache_free, edf);
	eina_hash_free(edf->data_cache);
	edf->data_cache = NULL;
     }

   while (edf->color_classes)
     {
	Edje_Color_Class *ecc;

	ecc = eina_list_data_get(edf->color_classes);
	edf->color_classes =
          eina_list_remove_list(edf->color_classes, edf->color_classes);
	if (edf->free_strings && ecc->name) eina_stringshare_del(ecc->name);
	free(ecc);
     }

   /* FIXME: free collection_hash and collection_cache */
   if (edf->collection_hash)
     {
	printf("EDJE ERROR:\n"
	       "\n"
	       "Naughty Programmer - spank spank!\n"
	       "\n"
	       "This program as probably called edje_shutdown() with active Edje objects\n"
	       "still around.\n This can cause problems as both Evas and Edje retain\n"
	       "references to the objects. you should shut down all canvases and objects\n"
	       "before calling edje_shutdown().\n"
	       "The following errors are the edje object files and parts that are still\n"
	       "hanging around, with their reference counts\n");
	eina_hash_foreach(edf->collection_hash,
                          _edje_file_collection_hash_foreach, edf);
	eina_hash_free(edf->collection_hash);
     }
   if (edf->path) eina_stringshare_del(edf->path);
   if (edf->free_strings && edf->compiler) eina_stringshare_del(edf->compiler);
   if (edf->collection_cache) _edje_cache_coll_flush(edf);
   _edje_textblock_style_cleanup(edf);
   if (edf->ef) eet_close(edf->ef);
   free(edf);
}

void
_edje_collection_free(Edje_File *edf, Edje_Part_Collection *ec)
{
   while (ec->programs)
     {
	Edje_Program *pr;

	pr = eina_list_data_get(ec->programs);
	ec->programs = eina_list_remove_list(ec->programs, ec->programs);
        if (edf->free_strings)
          {
             if (pr->name) eina_stringshare_del(pr->name);
             if (pr->signal) eina_stringshare_del(pr->signal);
             if (pr->source) eina_stringshare_del(pr->source);
             if (pr->state) eina_stringshare_del(pr->state);
             if (pr->state2) eina_stringshare_del(pr->state2);
          }
	while (pr->targets)
	  {
	     Edje_Program_Target *prt;

	     prt = eina_list_data_get(pr->targets);
	     pr->targets = eina_list_remove_list(pr->targets, pr->targets);
	     free(prt);
	  }
	while (pr->after)
	  {
	     Edje_Program_After *pa;

	     pa = eina_list_data_get(pr->after);
	     pr->after = eina_list_remove_list(pr->after, pr->after);
	     free(pa);
	  }
	free(pr);
     }
   while (ec->parts)
     {
	Edje_Part *ep;

	ep = eina_list_data_get(ec->parts);
	ec->parts = eina_list_remove(ec->parts, ep);
	if (edf->free_strings && ep->name) eina_stringshare_del(ep->name);
	if (ep->default_desc)
	  {
	     _edje_collection_free_part_description_free(ep->default_desc, edf->free_strings);
	     ep->default_desc = NULL;
	  }
	while (ep->other_desc)
	  {
	     Edje_Part_Description *desc;

	     desc = eina_list_data_get(ep->other_desc);
	     ep->other_desc = eina_list_remove(ep->other_desc, desc);
	     _edje_collection_free_part_description_free(desc, edf->free_strings);
	  }
	free(ep);
     }
   if (ec->data)
     {
	while (ec->data)
	  {
	     Edje_Data *edt;

	     edt = eina_list_data_get(ec->data);
	     ec->data = eina_list_remove(ec->data, edt);
             if (edf->free_strings)
               {
                  if (edt->key) eina_stringshare_del(edt->key);
                  if (edt->value) eina_stringshare_del(edt->value);
               }
	     free(edt);
	  }
     }
   if (edf->free_strings && ec->part) eina_stringshare_del(ec->part);
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
   if (ec->script) embryo_program_free(ec->script);
   free(ec);
}

void
_edje_collection_free_part_description_free(Edje_Part_Description *desc, unsigned int free_strings)
{
   while (desc->image.tween_list)
     {
	Edje_Part_Image_Id *pi;

	pi = eina_list_data_get(desc->image.tween_list);
	desc->image.tween_list = eina_list_remove(desc->image.tween_list, pi);
	free(pi);
     }
   if (free_strings)
     {
	if (desc->color_class)     eina_stringshare_del(desc->color_class);
	if (desc->text.text)       eina_stringshare_del(desc->text.text);
	if (desc->text.text_class) eina_stringshare_del(desc->text.text_class);
	if (desc->text.style)      eina_stringshare_del(desc->text.style);
	if (desc->text.font)       eina_stringshare_del(desc->text.font);
	if (desc->gradient.type)   eina_stringshare_del(desc->gradient.type);
	if (desc->gradient.params) eina_stringshare_del(desc->gradient.params);
     }
   free(desc);
}

static Eina_Bool
_edje_file_collection_hash_foreach(const Eina_Hash *hash, const void *key, void *data, void *fdata)
{
   Edje_File *edf;
   Edje_Part_Collection *coll;

   edf = fdata;
   coll = data;
   printf("EEK: EDJE FILE: \"%s\" ref(%i) PART: \"%s\" ref(%i) \n",
	  edf->path, edf->references,
	  coll->part, coll->references);
   _edje_collection_free(edf, coll);

   return 1;
}

#ifdef EDJE_PROGRAM_CACHE
static Eina_Bool
_edje_collection_free_prog_cache_matches_free_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata)
{
   eina_list_free((Eina_List *)data);
   return 1;
   key = NULL;
   hash = NULL;
   fdata = NULL;
}
#endif

static void
_edje_object_pack_item_hints_set(Evas_Object *obj, Edje_Pack_Element *it)
{
   Evas_Coord w = 0, h = 0, minw, minh;

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
   evas_object_size_hint_align_set(obj, it->align.x, it->align.y);
   evas_object_size_hint_weight_set(obj, it->weight.x, it->weight.y);
   evas_object_size_hint_aspect_set(obj, it->aspect.mode, it->aspect.w, it->aspect.h);

   evas_object_resize(obj, w, h);
}

static void
_cb_signal_repeat(void *data, Evas_Object *obj, const char *signal, const char *source)
{
   Evas_Object	*parent;
   Edje		*ed;
   char		 new_src[4096]; /* XXX is this max reasonable? */
   int		 length_parent = 0;
   int		 length_source;

   parent = data;
   ed = _edje_fetch(obj);
   if (!ed) return;
   /* Replace snprint("%s%c%s") == memcpy + *new_src + memcat */
   if (ed->parent)
     length_parent = strlen(ed->parent);
   length_source = strlen(source);
   if (length_source + length_parent + 2 > sizeof(new_src))
     return;

   if (ed->parent)
     memcpy(new_src, ed->parent, length_parent);
   new_src[length_parent] = EDJE_PART_PATH_SEPARATOR;
   memcpy(new_src + length_parent + 1, source, length_source + 1);

   edje_object_signal_emit(parent, signal, new_src);
}
