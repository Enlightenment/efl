#include "edje_private.h"

#ifdef EDJE_PROGRAM_CACHE
static Eina_Bool  _edje_collection_free_prog_cache_matches_free_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata);
#endif
static void _edje_object_pack_item_hints_set(Evas_Object *obj, Edje_Pack_Element *it);
static void _cb_signal_repeat(void *data, Evas_Object *obj, const char *signal, const char *source);

static Eina_List *_edje_swallows_collect(Edje *ed);

/************************** API Routines **************************/

EAPI Eina_Bool
edje_object_file_set(Evas_Object *obj, const char *file, const char *group)
{
   Eina_Bool ret;
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed)
     return EINA_FALSE;
   ret = ed->api->file_set(obj, file, group);
   _edje_object_orientation_inform(obj);
   return ret;
}

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

EAPI Edje_Load_Error
edje_object_load_error_get(const Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return EDJE_LOAD_ERROR_NONE;
   return ed->load_error;
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
edje_file_collection_list(const char *file)
{
   Eina_List *lst = NULL;
   Edje_File *edf;
   int error_ret = 0;

   if ((!file) || (!*file)) return NULL;
   edf = _edje_cache_file_coll_open(file, NULL, &error_ret, NULL);
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

EAPI void
edje_file_collection_list_free(Eina_List *lst)
{
   while (lst)
     {
        if (eina_list_data_get(lst)) eina_stringshare_del(eina_list_data_get(lst));
	lst = eina_list_remove(lst, eina_list_data_get(lst));
     }
}

EAPI Eina_Bool
edje_file_group_exists(const char *file, const char *glob)
{
   Edje_File *edf;
   int error_ret = 0;
   Eina_Bool succeed = EINA_FALSE;
   Eina_Bool is_glob = EINA_FALSE;
   const char *p;

   if ((!file) || (!*file))
      return EINA_FALSE;

   edf = _edje_cache_file_coll_open(file, NULL, &error_ret, NULL);
   if (!edf)
      return EINA_FALSE;
   
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

   INF("edje_file_group_exists: '%s', '%s': %i\n", file, glob, succeed);

   return succeed;
}


EAPI char *
edje_file_data_get(const char *file, const char *key)
{
   Edje_File *edf;
   char *str = NULL;
   int error_ret = 0;

   if (key)
     {
	edf = _edje_cache_file_coll_open(file, NULL, &error_ret, NULL);
	if (edf)
	  {
	     str = (char*) edje_string_get(eina_hash_find(edf->data, key));

	     if (str) str = strdup(str);

	     _edje_cache_file_unref(edf);
	  }
     }
   return str;
}

void
_edje_programs_patterns_clean(Edje *ed)
{
   _edje_signals_sources_patterns_clean(&ed->patterns.programs);

   eina_rbtree_delete(ed->patterns.programs.exact_match,
		      EINA_RBTREE_FREE_CB(edje_match_signal_source_free),
		      NULL);
   ed->patterns.programs.exact_match = NULL;

   free(ed->patterns.programs.u.programs.globing);
   ed->patterns.programs.u.programs.globing = NULL;
}

void
_edje_programs_patterns_init(Edje *ed)
{
   Edje_Signals_Sources_Patterns *ssp = &ed->patterns.programs;
   Edje_Program **all;
   unsigned int i, j;

   if (ssp->signals_patterns)
     return;

   edje_match_program_hash_build(ed->collection->programs.strcmp,
				 ed->collection->programs.strcmp_count,
				 &ssp->exact_match);

   j = ed->collection->programs.strncmp_count
     + ed->collection->programs.strrncmp_count
     + ed->collection->programs.fnmatch_count
     + ed->collection->programs.nocmp_count;
   if (j == 0) return ;

   all = malloc(sizeof (Edje_Program *) * j);
   if (!all) return ;
   j = 0;

   /* FIXME: Build specialized data type for each case */
#define EDJE_LOAD_PROGRAMS_ADD(Array, Ed, It, Git, All)			\
   for (It = 0; It < Ed->collection->programs.Array##_count; ++It, ++Git) \
     All[Git] = Ed->collection->programs.Array[It];

   EDJE_LOAD_PROGRAMS_ADD(fnmatch, ed, i, j, all);
   EDJE_LOAD_PROGRAMS_ADD(strncmp, ed, i, j, all);
   EDJE_LOAD_PROGRAMS_ADD(strrncmp, ed, i, j, all);
   /* FIXME: Do a special pass for that one */
   EDJE_LOAD_PROGRAMS_ADD(nocmp, ed, i, j, all);

   ssp->u.programs.globing = all;
   ssp->u.programs.count = j;
   ssp->signals_patterns = edje_match_programs_signal_init(all, j);
   ssp->sources_patterns = edje_match_programs_source_init(all, j);
}

int
_edje_object_file_set_internal(Evas_Object *obj, const char *file, const char *group, const char *parent, Eina_List *group_path)
{
   Edje *ed;
   Evas *tev;
   Eina_List *old_swallows;
   unsigned int n;
   Eina_List *parts = NULL;
   int group_path_started = 0;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   if (!file) file = "";
   if (!group) group = "";
   if (((ed->path) && (!strcmp(file, ed->path))) &&
       (ed->group) && (!strcmp(group, ed->group)))
     return 1;

   tev = evas_object_evas_get(obj);
   evas_event_freeze(tev);
   old_swallows = _edje_swallows_collect(ed);

   if (_edje_script_only(ed)) _edje_script_only_shutdown(ed);
   if (_edje_lua_script_only(ed)) _edje_lua_script_only_shutdown(ed);
   _edje_file_del(ed);

   eina_stringshare_replace(&ed->path, file);
   eina_stringshare_replace(&ed->group, group);

   ed->parent = eina_stringshare_add(parent);

   ed->load_error = EDJE_LOAD_ERROR_NONE;
   _edje_file_add(ed);

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
	if (ed->collection->prop.orientation != EDJE_ORIENTATION_AUTO)
          ed->is_rtl = (ed->collection->prop.orientation ==
                        EDJE_ORIENTATION_RTL);

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
	     int errors = 0;

	     /* colorclass stuff */
	     for (i = 0; i < ed->collection->parts_count; ++i)
	       {
		  Edje_Part *ep;
		  unsigned int k;

		  ep = ed->collection->parts[i];

		  if (errors)
		    break;
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
		  Edje_Real_Part *rp;

		  ep = ed->collection->parts[n];
		  rp = eina_mempool_malloc(_edje_real_part_mp, sizeof(Edje_Real_Part));
		  if (!rp)
		    {
		       ed->load_error = EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                       evas_event_thaw(tev);
                       evas_event_thaw_eval(tev);
		       return 0;
		    }

		  memset(rp, 0, sizeof (Edje_Real_Part));

		  if ((ep->dragable.x != 0) || (ep->dragable.y != 0))
		    {
		       rp->drag = calloc(1, sizeof (Edje_Real_Part_Drag));
		       if (!rp->drag)
			 {
			    ed->load_error = EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
			    free(rp);
                            evas_event_thaw(tev);
                            evas_event_thaw_eval(tev);
                            return 0;
			 }

		       rp->drag->step.x = FROM_INT(ep->dragable.step_x);
		       rp->drag->step.y = FROM_INT(ep->dragable.step_y);
		    }

		  rp->edje = ed;
		  _edje_ref(rp->edje);
		  rp->part = ep;
		  parts = eina_list_append(parts, rp);
		  rp->param1.description = 
                    _edje_part_description_find(ed, rp, "default", 0.0);
		  rp->chosen_description = rp->param1.description;
		  if (!rp->param1.description)
		    ERR("no default part description!");

		  switch (ep->type)
		    {
		     case EDJE_PART_TYPE_RECTANGLE:
			rp->object = evas_object_rectangle_add(ed->base.evas);
			break;
                     case EDJE_PART_TYPE_PROXY:
		     case EDJE_PART_TYPE_IMAGE:
			rp->object = evas_object_image_add(ed->base.evas);
			break;
		     case EDJE_PART_TYPE_TEXT:
			_edje_text_part_on_add(ed, rp);
			rp->object = evas_object_text_add(ed->base.evas);
			evas_object_text_font_source_set(rp->object, ed->path);
			break;
		     case EDJE_PART_TYPE_SWALLOW:
		     case EDJE_PART_TYPE_GROUP:
		     case EDJE_PART_TYPE_EXTERNAL:
			rp->object = evas_object_rectangle_add(ed->base.evas);
			evas_object_color_set(rp->object, 0, 0, 0, 0);
			evas_object_pass_events_set(rp->object, 1);
			evas_object_pointer_mode_set(rp->object, EVAS_OBJECT_POINTER_MODE_NOGRAB);
			_edje_callbacks_focus_add(rp->object, ed, rp);
			break;
		     case EDJE_PART_TYPE_TEXTBLOCK:
			rp->object = evas_object_textblock_add(ed->base.evas);
			break;
		     case EDJE_PART_TYPE_BOX:
			rp->object = evas_object_box_add(ed->base.evas);
                        rp->anim = _edje_box_layout_anim_new(rp->object);
			break;
		     case EDJE_PART_TYPE_TABLE:
			rp->object = evas_object_table_add(ed->base.evas);
			break;
		     case EDJE_PART_TYPE_GRADIENT:
			ERR("SPANK ! SPANK ! SPANK ! YOU ARE USING GRADIENT IN PART %s FROM GROUP %s INSIDE FILE %s !! THEY ARE NOW REMOVED !",
			    ep->name, group, file);
		     default:
			ERR("wrong part type %i!", ep->type);
			break;
		    }

		  if (rp->object)
		    {
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
			 evas_object_clip_set(rp->object, ed->base.clipper);
		    }
	       }
	     if (n > 0)
	       {
		  Edje_Real_Part *rp;
		  Eina_List *l;

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
		       if (rp->param1.description) /* FIXME: prevent rel to gone radient part to go wrong. You may
						      be able to remove this when all theme are correctly rewritten. */
			 {
			    if (rp->param1.description->rel1.id_x >= 0)
			      rp->param1.rel1_to_x = ed->table_parts[rp->param1.description->rel1.id_x % ed->table_parts_size];
			    if (rp->param1.description->rel1.id_y >= 0)
			      rp->param1.rel1_to_y = ed->table_parts[rp->param1.description->rel1.id_y % ed->table_parts_size];
			    if (rp->param1.description->rel2.id_x >= 0)
			      rp->param1.rel2_to_x = ed->table_parts[rp->param1.description->rel2.id_x % ed->table_parts_size];
			    if (rp->param1.description->rel2.id_y >= 0)
			      rp->param1.rel2_to_y = ed->table_parts[rp->param1.description->rel2.id_y % ed->table_parts_size];
			 }
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
		       if (rp->drag)
			 {
			    if (rp->part->dragable.confine_id >= 0)
			      rp->drag->confine_to = ed->table_parts[rp->part->dragable.confine_id % ed->table_parts_size];
			 }

		       /* replay events for dragable */
		       if (rp->part->dragable.event_id >= 0)
			 {
			    rp->events_to =
			      ed->table_parts[rp->part->dragable.event_id % ed->table_parts_size];
			    /* events_to may be used only with dragable */
			    if (!rp->events_to->part->dragable.x &&
				!rp->events_to->part->dragable.y)
			      rp->events_to = NULL;
			 }

		       rp->swallow_params.min.w = 0;
		       rp->swallow_params.min.w = 0;
		       rp->swallow_params.max.w = -1;
		       rp->swallow_params.max.h = -1;

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

			    if (text->text.id_source >= 0)
			      rp->text.source = ed->table_parts[text->text.id_source % ed->table_parts_size];
			    if (text->text.id_text_source >= 0)
			      rp->text.text_source = ed->table_parts[text->text.id_text_source % ed->table_parts_size];
			    if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
			      {
				 _edje_entry_real_part_init(rp);
				 if (!ed->has_entries)
				   ed->has_entries = EINA_TRUE;
			      }
			 }
		    }
	       }
	     
	     _edje_programs_patterns_init(ed);

	     n = ed->collection->programs.fnmatch_count +
	       ed->collection->programs.strcmp_count +
	       ed->collection->programs.strncmp_count +
	       ed->collection->programs.strrncmp_count +
	       ed->collection->programs.nocmp_count;
	     if (n > 0)
	       {
		  Edje_Program *pr;

		  ed->table_programs = malloc(sizeof(Edje_Program *) * n);
		  if (ed->table_programs)
		    {
		       ed->table_programs_size = n;

#define EDJE_LOAD_BUILD_TABLE(Array, Ed, It, Tmp)	\
		       for (It = 0; It < Ed->collection->programs.Array##_count; ++It) \
			 {						\
			    Tmp = Ed->collection->programs.Array[It];	\
			    Ed->table_programs[Tmp->id] = Tmp;		\
			 }

		       EDJE_LOAD_BUILD_TABLE(fnmatch, ed, i, pr);
		       EDJE_LOAD_BUILD_TABLE(strcmp, ed, i, pr);
		       EDJE_LOAD_BUILD_TABLE(strncmp, ed, i, pr);
		       EDJE_LOAD_BUILD_TABLE(strrncmp, ed, i, pr);
		       EDJE_LOAD_BUILD_TABLE(nocmp, ed, i, pr);
		    }
	       }
	     _edje_ref(ed);
	     _edje_block(ed);
	     _edje_freeze(ed);
//	     if (ed->collection->script) _edje_embryo_script_init(ed);
	     _edje_var_init(ed);
	     for (i = 0; i < ed->table_parts_size; i++)
	       {
		  Edje_Real_Part *rp;
		  
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
	     ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	     ed->all_part_change = 1;
#endif
	     if ((evas_object_clipees_get(ed->base.clipper)) &&
		 (evas_object_visible_get(obj)))
	       evas_object_show(ed->base.clipper);
	     
	     /* instantiate 'internal swallows' */
	     for (i = 0; i < ed->table_parts_size; i++)
	       {
		  Edje_Real_Part *rp;
		  /* XXX: curr_item and pack_it don't require to be NULL since
		   * XXX: they are just used when source != NULL and type == BOX,
		   * XXX: and they're always set in this case, but GCC fails to
		   * XXX: notice that, so let's shut it up
		   */
		  Edje_Pack_Element **curr_item = NULL;
		  unsigned int item_count = 0;
		  Edje_Pack_Element *pack_it = NULL;
		  const char *source = NULL;
		  
		  rp = ed->table_parts[i];

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
		     case EDJE_PART_TYPE_EXTERNAL:
			  {
			     Edje_Part_Description_External *external;
			     Evas_Object *child_obj;

			     external = (Edje_Part_Description_External *) rp->part->default_desc;
			     child_obj = _edje_external_type_add(rp->part->source,
								 evas_object_evas_get(ed->obj), ed->obj,
								 external->external_params, rp->part->name);
			     if (child_obj)
			       {
				  _edje_real_part_swallow(rp, child_obj, EINA_TRUE);
				  rp->param1.external_params = _edje_external_params_parse(child_obj,
											   external->external_params);
				  _edje_external_recalc_apply(ed, rp, NULL, rp->chosen_description);
			       }
			  }
			continue;
		     default:
			continue;
		    }

		  while (source)
		    {
		       Eina_List *l;
		       Evas_Object *child_obj;
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
                                 evas_event_thaw(tev);
                                 evas_event_thaw_eval(tev);
				 return 0;
			      }
			 }
		       
		       child_obj = edje_object_add(ed->base.evas);
		       group_path = eina_list_append(group_path, group_path_entry);
		       if (rp->part->type == EDJE_PART_TYPE_GROUP)
			 {
                            _edje_real_part_swallow(rp, child_obj, EINA_FALSE);
			 }

		       if (!_edje_object_file_set_internal(child_obj, file, source, rp->part->name, group_path))
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
                            evas_event_thaw(tev);
                            evas_event_thaw_eval(tev);
			    return 0;
			 }

		       group_path = eina_list_remove(group_path, group_path_entry);
		       eina_stringshare_del(group_path_entry);

		       edje_object_signal_callback_add(child_obj, "*", "*", _cb_signal_repeat, obj);
		       if (rp->part->type == EDJE_PART_TYPE_GROUP)
			 {
                            _edje_real_part_swallow(rp, child_obj, EINA_TRUE);
			    source = NULL;
			 }
		       else
			 {
                            pack_it->parent = rp;

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
	  }
        _edje_entry_init(ed);
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
}

void
_edje_file_add(Edje *ed)
{
   if (!_edje_edd_edje_file) return;
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
   unsigned int i;

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
   Evas *tev = evas_object_evas_get(ed->obj);
   
   evas_event_freeze(tev);
   if (ed->freeze_calc)
     {
        _edje_freeze_calc_list = eina_list_remove(_edje_freeze_calc_list, ed);
        ed->freeze_calc = 0;
        _edje_freeze_calc_count--;
     }
   _edje_entry_shutdown(ed);
   _edje_message_del(ed);
   _edje_block_violate(ed);
   _edje_var_shutdown(ed);
   _edje_programs_patterns_clean(ed);
//   if (ed->collection)
//     {
//        if (ed->collection->script) _edje_embryo_script_shutdown(ed);
//     }

   if (!((ed->file) && (ed->collection)))
     {
        evas_event_thaw(tev);
        evas_event_thaw_eval(tev);
        return;
     }
   if (ed->table_parts)
     {
	unsigned int i;
	for (i = 0; i < ed->table_parts_size; i++)
	  {
	     Edje_Real_Part *rp;

	     rp = ed->table_parts[i];
	     if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
	       _edje_entry_real_part_shutdown(rp);
	     if (rp->object)
	       {
		  _edje_callbacks_del(rp->object, ed);
		  _edje_callbacks_focus_del(rp->object, ed);
		  evas_object_del(rp->object);
	       }
	     if (rp->swallowed_object)
	       {
                  _edje_real_part_swallow_clear(rp);
                  /* Objects swallowed by the app do not get deleted,
                   but those internally swallowed (GROUP type) do. */
		  switch (rp->part->type)
		    {
		     case EDJE_PART_TYPE_EXTERNAL:
			_edje_external_parsed_params_free(rp->swallowed_object, rp->param1.external_params);
			if (rp->param2)
			  _edje_external_parsed_params_free(rp->swallowed_object, rp->param2->external_params);
		     case EDJE_PART_TYPE_GROUP:
			evas_object_del(rp->swallowed_object);
		     default:
			break;
		    }
		  rp->swallowed_object = NULL;
	       }
	     if (rp->items)
	       {
		  /* evas_box/table handles deletion of objects */
		  rp->items = eina_list_free(rp->items);
	       }
             if (rp->anim)
               {
                  _edje_box_layout_free_data(rp->anim);
                  rp->anim = NULL;
               }
	     if (rp->text.text) eina_stringshare_del(rp->text.text);
	     if (rp->text.font) eina_stringshare_del(rp->text.font);
	     if (rp->text.cache.in_str) eina_stringshare_del(rp->text.cache.in_str);
	     if (rp->text.cache.out_str) eina_stringshare_del(rp->text.cache.out_str);

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
	       free(rp->param2->set);
	     eina_mempool_free(_edje_real_part_state_mp, rp->param2);

	     if (rp->custom)
	       free(rp->custom->set);
	     eina_mempool_free(_edje_real_part_state_mp, rp->custom);

	     _edje_unref(rp->edje);
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
   if (ed->L)
      _edje_lua2_script_shutdown(ed);
   if (ed->table_parts) free(ed->table_parts);
   ed->table_parts = NULL;
   ed->table_parts_size = 0;
   if (ed->table_programs) free(ed->table_programs);
   ed->table_programs = NULL;
   ed->table_programs_size = 0;
   ed->focused_part = NULL;
   evas_event_thaw(tev);
   evas_event_thaw_eval(tev);
}

void
_edje_file_free(Edje_File *edf)
{
   Edje_Color_Class *ecc;

#define HASH_FREE(Hash)				\
   if (Hash) eina_hash_free(Hash);		\
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

	/* Sets have been added after edje received eet dictionnary support */
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

   if (edf->external_dir)
     {
	if (edf->external_dir->entries) free(edf->external_dir->entries);
	free(edf->external_dir);
     }

   EINA_LIST_FREE(edf->color_classes, ecc)
     {
	if (edf->free_strings && ecc->name) eina_stringshare_del(ecc->name);
	free(ecc);
     }

   if (edf->collection_patterns) edje_match_patterns_free(edf->collection_patterns);
   if (edf->path) eina_stringshare_del(edf->path);
   if (edf->free_strings && edf->compiler) eina_stringshare_del(edf->compiler);
   _edje_textblock_style_cleanup(edf);
   if (edf->ef) eet_close(edf->ef);
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

#define EDJE_LOAD_PROGRAM_FREE(Array, Ec, It, FreeStrings)	\
   for (It = 0; It < Ec->programs.Array##_count; ++It)		\
     _edje_program_free(Ec->programs.Array[It], FreeStrings);	\
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
   if (ec->script) embryo_program_free(ec->script);
   _edje_lua2_script_unload(ec);

   /* Destroy all part and description. */
   eina_mempool_del(ce->mp.RECTANGLE);
   eina_mempool_del(ce->mp.TEXT);
   eina_mempool_del(ce->mp.IMAGE);
   eina_mempool_del(ce->mp.PROXY);
   eina_mempool_del(ce->mp.SWALLOW);
   eina_mempool_del(ce->mp.TEXTBLOCK);
   eina_mempool_del(ce->mp.GROUP);
   eina_mempool_del(ce->mp.BOX);
   eina_mempool_del(ce->mp.TABLE);
   eina_mempool_del(ce->mp.EXTERNAL);
   eina_mempool_del(ce->mp.part);
   memset(&ce->mp, 0, sizeof (ce->mp));

   eina_mempool_del(ce->mp_rtl.RECTANGLE);
   eina_mempool_del(ce->mp_rtl.TEXT);
   eina_mempool_del(ce->mp_rtl.IMAGE);
   eina_mempool_del(ce->mp_rtl.PROXY);
   eina_mempool_del(ce->mp_rtl.SWALLOW);
   eina_mempool_del(ce->mp_rtl.TEXTBLOCK);
   eina_mempool_del(ce->mp_rtl.GROUP);
   eina_mempool_del(ce->mp_rtl.BOX);
   eina_mempool_del(ce->mp_rtl.TABLE);
   eina_mempool_del(ce->mp_rtl.EXTERNAL);
   memset(&ce->mp_rtl, 0, sizeof (ce->mp_rtl));
   free(ec);
   ce->ref = NULL;
}

void
_edje_collection_free_part_description_clean(int type, Edje_Part_Description_Common *desc, Eina_Bool free_strings)
{
   if (free_strings && desc->color_class) eina_stringshare_del(desc->color_class);

   switch (type)
     {
      case EDJE_PART_TYPE_IMAGE:
	{
	   Edje_Part_Description_Image *img;
	   unsigned int i;

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

	      if (text->text.text.str)      eina_stringshare_del(text->text.text.str);
	      if (text->text.text_class)    eina_stringshare_del(text->text.text_class);
	      if (text->text.style.str)     eina_stringshare_del(text->text.style.str);
	      if (text->text.font.str)      eina_stringshare_del(text->text.font.str);
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
#define FREE_POOL(Type, Ce, Desc)					\
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
_edje_collection_free_prog_cache_matches_free_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata)
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
   Evas_Object	*parent;
   Edje		*ed;
   Edje         *ed_parent;
   char		 new_src[4096]; /* XXX is this max reasonable? */
   size_t	 length_parent = 0;
   size_t        length_index = 0;
   size_t	 length_source;
   int           i = 0;
   const char   *alias = NULL;

   parent = data;
   ed = _edje_fetch(obj);
   if (!ed) return;

   pack_it = evas_object_data_get(obj, "\377 edje.box_item");
   if (!pack_it) pack_it = evas_object_data_get(obj, "\377 edje.table_item");
   if (pack_it)
     {
        if (!pack_it->name)
          {
             Eina_List *child = NULL;
             Evas_Object *o;

             if (pack_it->parent->part->type == EDJE_PART_TYPE_BOX)
               {
                  child = evas_object_box_children_get(pack_it->parent->object);
               }
             else if (pack_it->parent->part->type == EDJE_PART_TYPE_TABLE)
               {
                  child = evas_object_table_children_get(pack_it->parent->object);
               }

             EINA_LIST_FREE(child, o)
               {
                  if (o == obj) break;
                  i++;
               }

             eina_list_free(child);

             length_index = 12;
          }
        else
          {
             length_index = strlen(pack_it->name) + 2;
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
             memcpy(new_src + length_parent, pack_it->name, length_index);
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

   edje_object_signal_emit(parent, sig, alias ? alias : new_src);
}
