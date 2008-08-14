/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

void _edje_collection_free_part_description_free(Edje_Part_Description *desc, unsigned int free_strings);
static Evas_Bool _edje_file_collection_hash_foreach(const Evas_Hash *hash, const char *key, void *data, void *fdata);
#ifdef EDJE_PROGRAM_CACHE
static int  _edje_collection_free_prog_cache_matches_free_cb(Evas_Hash *hash, const char *key, void *data, void *fdata);
#endif
static int _edje_object_file_set_internal(Evas_Object *obj, const char *file, const char *group, Evas_List *group_path);
static void _cb_signal_repeat(void *data, Evas_Object *obj, const char *signal, const char *source);

static Evas_List *_edje_swallows_collect(Edje *ed);

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
edje_object_file_get(Evas_Object *obj, const char **file, const char **group)
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
edje_object_load_error_get(Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return EDJE_LOAD_ERROR_NONE;
   return ed->load_error;
}

/** Get a list of groups in an edje file
 * @param file The path to the edje file
 *
 * @return The Evas_List of group names (char *)
 *
 * Note: the list must be freed using edje_file_collection_list_free()
 * when you are done with it.
 */
EAPI Evas_List *
edje_file_collection_list(const char *file)
{
   Evas_List *lst = NULL;
   Edje_File *edf;
   int error_ret = 0;

   if ((!file) || (!*file)) return NULL;
   edf = _edje_cache_file_coll_open(file, NULL, &error_ret, NULL);
   if (edf != NULL)
     {
	if (edf->collection_dir)
	  {
	     Evas_List *l;

	     for (l = edf->collection_dir->entries; l; l = l->next)
	       {
		  Edje_Part_Collection_Directory_Entry *ce;

		  ce = l->data;
		  lst = evas_list_append(lst, evas_stringshare_add(ce->entry));
	       }
	  }
	_edje_cache_file_unref(edf);
     }
   return lst;
}

/** Free file collection list
 * @param lst The Evas_List of groups
 *
 * Frees the list returned by edje_file_collection_list().
 */
EAPI void
edje_file_collection_list_free(Evas_List *lst)
{
   while (lst)
     {
	if (lst->data) evas_stringshare_del(lst->data);
	lst = evas_list_remove(lst, lst->data);
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
	if ((edf != NULL) && (edf->data_cache != NULL))
	  {
	     str = evas_hash_find(edf->data_cache, key);
	     if (str) str = strdup(str);
	     _edje_cache_file_unref(edf);
	  }
     }
   return str;
}

static void
_edje_programs_patterns_clean(Edje *ed)
{
   _edje_signals_sources_patterns_clean(&ed->patterns.programs);
}

static void
_edje_programs_patterns_init(Edje *ed)
{
   Edje_Signals_Sources_Patterns *ssp = &ed->patterns.programs;
   Evas_List *programs = ed->collection->programs;

   if (ssp->signals_patterns)
     return;

   ssp->signals_patterns = edje_match_programs_signal_init(programs);
   ssp->sources_patterns = edje_match_programs_source_init(programs);
}

static int
_edje_object_file_set_internal(Evas_Object *obj, const char *file, const char *group, Evas_List *group_path)
{
   Edje *ed;
   int n;
   Evas_List *parts = NULL;
   Evas_List *old_swallows;
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

   if (ed->path) evas_stringshare_del(ed->path);
   if (ed->group) evas_stringshare_del(ed->group);
   ed->path = evas_stringshare_add(file);
   ed->group = evas_stringshare_add(group);

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
	     Evas_List *l;
	     int i;
	     int errors = 0;

	     /* colorclass stuff */
	     for (l = ed->collection->parts; (l && ! errors); l = l->next)
	       {
		  Edje_Part *ep;
		  Evas_List *hist = NULL;
		  
		  /* Register any color classes in this parts descriptions. */
		  ep = l->data;
		  if ((ep->default_desc) && (ep->default_desc->color_class))
		    _edje_color_class_member_add(ed, ep->default_desc->color_class);
		  for (hist = ep->other_desc; hist; hist = hist->next)
		    {
		       Edje_Part_Description *desc;
		       
		       desc = hist->data;
		       if (desc->color_class)
			 _edje_color_class_member_add(ed, desc->color_class);
		    }
	       }
	     /* build real parts */
	     for (n = 0, l = ed->collection->parts; l; l = l->next, n++)
	       {
		  Edje_Part *ep;
		  Edje_Real_Part *rp;
		  
		  ep = l->data;
		  rp = calloc(1, sizeof(Edje_Real_Part));
		  if (!rp)
		    {
		       ed->load_error = EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
		       return 0;
		    }
		  rp->edje = ed;
		  _edje_ref(rp->edje);
		  rp->part = ep;
		  parts = evas_list_append(parts, rp);
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
		  for (l = parts; l; l = l->next)
		    {
		       rp = l->data;
		       ed->table_parts[n] = rp;
		       n++;
		    }
		  evas_list_free(parts);
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
		    }
	       }
	     
	     _edje_programs_patterns_init(ed);
	     
	     n = evas_list_count(ed->collection->programs);
	     if (n > 0)
	       {
		  /* FIXME: keeping a table AND a list is just bad - nuke list */
		  ed->table_programs = malloc(sizeof(Edje_Program *) * n);
		  ed->table_programs_size = n;
		  /* FIXME: check malloc return */
		  n = 0;
		  for (l = ed->collection->programs; l; l = l->next)
		    {
		       Edje_Program *pr;
		       
		       pr = l->data;
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
		  
		  rp = ed->table_parts[i];
		  if (rp->part->type != EDJE_PART_TYPE_GROUP) continue;
		  if (rp->part->source)
		    {
		       Evas_List *l;
		       Evas_Object *child_obj;
		       Edje *child_ed;
		       const char *group_path_entry = evas_stringshare_add(rp->part->source);
		       if (!group_path)
			 {
			    group_path = evas_list_append(NULL, evas_stringshare_add(group));
			    group_path_started = 1;
			 }
		       /* make sure that this group isn't already in the tree of parents */
		       for (l = group_path; l; l = l->next)
			 {
			    if (l->data == group_path_entry)
			      {
				 _edje_thaw(ed);
				 _edje_unblock(ed);
				 _edje_unref(ed);
				 _edje_file_del(ed);
				 evas_stringshare_del(group_path_entry);
				 if (group_path_started)
				   {
				      evas_stringshare_del(group_path->data);
				      evas_list_free(group_path);
				   }
				 ed->load_error = EDJE_LOAD_ERROR_RECURSIVE_REFERENCE;
				 return 0;
			      }
			 }
		       
		       child_obj = edje_object_add(ed->evas);
		       group_path = evas_list_append(group_path, group_path_entry);
		       if (!_edje_object_file_set_internal(child_obj, file, rp->part->source, group_path))
			 {
			    _edje_thaw(ed);
			    _edje_unblock(ed);
			    _edje_unref(ed);
			    _edje_file_del(ed);
			    
			    if (group_path_started)
			      {
				 while (group_path)
				   {
				      evas_stringshare_del(group_path->data);
				      group_path = evas_list_remove_list(group_path, group_path);
				   }
			      }
			    ed->load_error = edje_object_load_error_get(child_obj);
			    return 0;
			 }
		       child_ed = _edje_fetch(child_obj);
		       child_ed->parent = evas_stringshare_add(rp->part->name);
		       
		       group_path = evas_list_remove(group_path, group_path_entry);
		       evas_stringshare_del(group_path_entry);
		       
		       edje_object_signal_callback_add(child_obj, "*", "*", _cb_signal_repeat, obj);
		       _edje_real_part_swallow(rp, child_obj);
		    }
	       }
	     
	     if (group_path_started)
	       {
		  while (group_path)
		    {
		       evas_stringshare_del(group_path->data);
		       group_path = evas_list_remove_list(group_path, group_path);
		    }
	       }
	     
	     /* reswallow any swallows that existed before setting the file */
	     if (old_swallows)
	       {
		  while (old_swallows)
		    {
		       const char *name;
		       Evas_Object *swallow;
		       
		       name = old_swallows->data;
		       old_swallows = evas_list_remove_list(old_swallows, old_swallows);
		       
		       swallow = old_swallows->data;
		       old_swallows = evas_list_remove_list(old_swallows, old_swallows);
		       
		       edje_object_part_swallow(obj, name, swallow);
		       evas_stringshare_del(name);
		    }
	       }
	     
	     _edje_recalc(ed);
	     _edje_thaw(ed);
	     _edje_unblock(ed);
	     _edje_unref(ed);
	     ed->load_error = EDJE_LOAD_ERROR_NONE;
	     _edje_emit(ed, "load", NULL);
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

static Evas_List *
_edje_swallows_collect(Edje *ed)
{
   Evas_List *swallows = NULL;
   int i;

   if (!ed->file || !ed->table_parts) return NULL;
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type != EDJE_PART_TYPE_SWALLOW || !rp->swallowed_object) continue;
	swallows = evas_list_append(swallows, evas_stringshare_add(rp->part->name));
	swallows = evas_list_append(swallows, rp->swallowed_object);
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
   if ((ed->file) && (ed->collection))
     {
	Evas_List *l;

	_edje_textblock_styles_del(ed);
	for (l = ed->collection->parts; l; l = l->next)
	  {
	     Edje_Part *ep;

	     ep = l->data;
	     _edje_text_part_on_del(ed, ep);
	     _edje_color_class_on_del(ed, ep);
	  }
	_edje_cache_coll_unref(ed->file, ed->collection);
	ed->collection = NULL;
     }
   if (ed->table_parts)
     {
	int i;
	for (i = 0; i < ed->table_parts_size; i++)
	  {
	     Edje_Real_Part *rp;

	     rp = ed->table_parts[i];
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
	     if (rp->text.text) evas_stringshare_del(rp->text.text);
	     if (rp->text.font) evas_stringshare_del(rp->text.font);
	     if (rp->text.cache.in_str) evas_stringshare_del(rp->text.cache.in_str);
	     if (rp->text.cache.out_str) evas_stringshare_del(rp->text.cache.out_str);

	     if (rp->custom.description)
	       _edje_collection_free_part_description_free(rp->custom.description, ed->file->free_strings);

	     _edje_unref(rp->edje);
	     free(rp);
	  }
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
	     runp = ed->actions->data;
	     ed->actions = evas_list_remove(ed->actions, runp);
	     free(runp);
	  }
     }
   _edje_animators = evas_list_remove(_edje_animators, ed);
   if (ed->pending_actions)
     {
	while (ed->pending_actions)
	  {
	     Edje_Pending_Program *pp;

	     pp = ed->pending_actions->data;
	     ed->pending_actions = evas_list_remove(ed->pending_actions, pp);
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
static Evas_Bool data_cache_free(const Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Edje_File    *edf;

   edf = fdata;
   if (edf->free_strings) evas_stringshare_del(data);
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

	     fe = edf->font_dir->entries->data;
	     edf->font_dir->entries =
	       evas_list_remove_list(edf->font_dir->entries, edf->font_dir->entries);
	     edf->font_hash = evas_hash_del(edf->font_hash, fe->entry, edf);
	     if (edf->free_strings && fe->path) evas_stringshare_del(fe->path);
	     free(fe);
	  }
	free(edf->font_dir);
     }
   if (edf->image_dir)
     {
	while (edf->image_dir->entries)
	  {
	     Edje_Image_Directory_Entry *ie;

	     ie = edf->image_dir->entries->data;
	     edf->image_dir->entries =
	       evas_list_remove_list(edf->image_dir->entries, edf->image_dir->entries);
	     if (edf->free_strings && ie->entry) evas_stringshare_del(ie->entry);
	     free(ie);
	  }
	free(edf->image_dir);
     }
   if (edf->collection_dir)
     {
	while (edf->collection_dir->entries)
	  {
	     Edje_Part_Collection_Directory_Entry *ce;

	     ce = edf->collection_dir->entries->data;
	     edf->collection_dir->entries =
	       evas_list_remove_list(edf->collection_dir->entries, edf->collection_dir->entries);
	     if (edf->free_strings && ce->entry) evas_stringshare_del(ce->entry);
	     free(ce);
	  }
	free(edf->collection_dir);
     }
   if (edf->spectrum_dir)
     {
	while (edf->spectrum_dir->entries)
	  {
	     Edje_Spectrum_Directory_Entry *se;

	     se = edf->spectrum_dir->entries->data;
	     edf->spectrum_dir->entries =
	       evas_list_remove_list(edf->spectrum_dir->entries, edf->spectrum_dir->entries);
	     while (se->color_list)
	       {
		  free(se->color_list->data);
		  se->color_list =
                    evas_list_remove_list(se->color_list, se->color_list);
	       }
             if (edf->free_strings)
               {
                  if (se->entry) evas_stringshare_del(se->entry);
                  if (se->filename) evas_stringshare_del(se->filename);
               }
	     free(se);
	  }
	free(edf->spectrum_dir);
     }
   while (edf->data)
     {
	Edje_Data *edt;

	edt = edf->data->data;
	edf->data = evas_list_remove(edf->data, edt);
        if (edf->free_strings)
          {
             if (edt->key) evas_stringshare_del(edt->key);
             if (edt->value) evas_stringshare_del(edt->value);
          }
	free(edt);
     }
   if (edf->data_cache)
     {
	evas_hash_foreach(edf->data_cache, data_cache_free, edf);
	evas_hash_free(edf->data_cache);
	edf->data_cache = NULL;
     }

   while (edf->color_classes)
     {
	Edje_Color_Class *ecc;

	ecc = edf->color_classes->data;
	edf->color_classes =
          evas_list_remove_list(edf->color_classes, edf->color_classes);
	if (edf->free_strings && ecc->name) evas_stringshare_del(ecc->name);
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
	evas_hash_foreach(edf->collection_hash,
                          _edje_file_collection_hash_foreach, edf);
	evas_hash_free(edf->collection_hash);
     }
   if (edf->path) evas_stringshare_del(edf->path);
   if (edf->free_strings && edf->compiler) evas_stringshare_del(edf->compiler);
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

	pr = ec->programs->data;
	ec->programs = evas_list_remove_list(ec->programs, ec->programs);
        if (edf->free_strings)
          {
             if (pr->name) evas_stringshare_del(pr->name);
             if (pr->signal) evas_stringshare_del(pr->signal);
             if (pr->source) evas_stringshare_del(pr->source);
             if (pr->state) evas_stringshare_del(pr->state);
             if (pr->state2) evas_stringshare_del(pr->state2);
          }
	while (pr->targets)
	  {
	     Edje_Program_Target *prt;

	     prt = pr->targets->data;
	     pr->targets = evas_list_remove_list(pr->targets, pr->targets);
	     free(prt);
	  }
	while (pr->after)
	  {
	     Edje_Program_After *pa;

	     pa = pr->after->data;
	     pr->after = evas_list_remove_list(pr->after, pr->after);
	     free(pa);
	  }
	free(pr);
     }
   while (ec->parts)
     {
	Edje_Part *ep;

	ep = ec->parts->data;
	ec->parts = evas_list_remove(ec->parts, ep);
	if (edf->free_strings && ep->name) evas_stringshare_del(ep->name);
	if (ep->default_desc)
	  {
	     _edje_collection_free_part_description_free(ep->default_desc, edf->free_strings);
	     ep->default_desc = NULL;
	  }
	while (ep->other_desc)
	  {
	     Edje_Part_Description *desc;

	     desc = ep->other_desc->data;
	     ep->other_desc = evas_list_remove(ep->other_desc, desc);
	     _edje_collection_free_part_description_free(desc, edf->free_strings);
	  }
	free(ep);
     }
   if (ec->data)
     {
	while (ec->data)
	  {
	     Edje_Data *edt;

	     edt = ec->data->data;
	     ec->data = evas_list_remove(ec->data, edt);
             if (edf->free_strings)
               {
                  if (edt->key) evas_stringshare_del(edt->key);
                  if (edt->value) evas_stringshare_del(edt->value);
               }
	     free(edt);
	  }
     }
   if (edf->free_strings && ec->part) evas_stringshare_del(ec->part);
#ifdef EDJE_PROGRAM_CACHE
   if (ec->prog_cache.no_matches) evas_hash_free(ec->prog_cache.no_matches);
   if (ec->prog_cache.matches)
     {
	evas_hash_foreach(ec->prog_cache.matches,
			  _edje_collection_free_prog_cache_matches_free_cb,
			  NULL);
	evas_hash_free(ec->prog_cache.matches);
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

	pi = desc->image.tween_list->data;
	desc->image.tween_list = evas_list_remove(desc->image.tween_list, pi);
	free(pi);
     }
   if (free_strings)
     {
	if (desc->color_class)     evas_stringshare_del(desc->color_class);
	if (desc->text.text)       evas_stringshare_del(desc->text.text);
	if (desc->text.text_class) evas_stringshare_del(desc->text.text_class);
	if (desc->text.style)      evas_stringshare_del(desc->text.style);
	if (desc->text.font)       evas_stringshare_del(desc->text.font);
	if (desc->gradient.type)   evas_stringshare_del(desc->gradient.type);
	if (desc->gradient.params) evas_stringshare_del(desc->gradient.params);
     }
   free(desc);
}

static Evas_Bool
_edje_file_collection_hash_foreach(const Evas_Hash *hash, const char *key, void *data, void *fdata)
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
static int
_edje_collection_free_prog_cache_matches_free_cb(const Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   evas_list_free((Evas_List *)data);
   return 1;
   hash = NULL;
   fdata = NULL;
}
#endif

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
     return ;

   if (ed->parent)
     memcpy(new_src, ed->parent, length_parent);
   new_src[length_parent] = EDJE_PART_PATH_SEPARATOR;
   memcpy(new_src + length_parent + 1, source, length_source + 1);

   edje_object_signal_emit(parent, signal, new_src);
}
