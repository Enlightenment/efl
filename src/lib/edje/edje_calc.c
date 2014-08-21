#include "edje_private.h"

static void _edje_part_make_rtl(Edje_Part_Description_Common *desc);
static Edje_Part_Description_Common *_edje_get_description_by_orientation(Edje *ed, Edje_Part_Description_Common *src, Edje_Part_Description_Common **dst, unsigned char type);

static void _edje_part_recalc_single(Edje *ed, Edje_Real_Part *ep,
                                     Edje_Part_Description_Common *desc, Edje_Part_Description_Common *chosen_desc,
                                     Edje_Real_Part *center, Edje_Real_Part *light, Edje_Real_Part *persp,
                                     Edje_Real_Part *rel1_to_x, Edje_Real_Part *rel1_to_y,
                                     Edje_Real_Part *rel2_to_x, Edje_Real_Part *rel2_to_y,
                                     Edje_Real_Part *confine_to, Edje_Real_Part *threshold,
				     Edje_Calc_Params *params, Edje_Real_Part_Set *set,
                                     Evas_Coord mmw, Evas_Coord mmh,
                                     FLOAT_T pos);

#define EINA_COW_CALC_PHYSICS_BEGIN(Calc, Write)	\
  EINA_COW_WRITE_BEGIN(_edje_calc_params_physics_cow, Calc->physics, Edje_Calc_Params_Physics, Write)

#define EINA_COW_CALC_PHYSICS_END(Calc, Write)	\
  EINA_COW_WRITE_END(_edje_calc_params_physics_cow, Calc->physics, Write)

#define EINA_COW_CALC_MAP_BEGIN(Calc, Write)	\
  EINA_COW_WRITE_BEGIN(_edje_calc_params_map_cow, Calc->map, Edje_Calc_Params_Map, Write)

#define EINA_COW_CALC_MAP_END(Calc, Write)	\
  EINA_COW_WRITE_END(_edje_calc_params_map_cow, Calc->map, Write);

void
_edje_part_pos_set(Edje *ed, Edje_Real_Part *ep, int mode, FLOAT_T pos, FLOAT_T v1, FLOAT_T v2, FLOAT_T v3, FLOAT_T v4)
{
   FLOAT_T fp_pos;
   FLOAT_T npos;
   double v[4];

   pos = CLAMP(pos, ZERO, FROM_INT(1));

   fp_pos = pos;

#if 0 // old code - easy to enable for comparing float vs fixed point
   /* take linear pos along timescale and use interpolation method */
   switch (mode)
     {
      case EDJE_TWEEN_MODE_SINUSOIDAL:
	 /* npos = (1.0 - cos(pos * PI)) / 2.0; */
	 npos = DIV2(SUB(FROM_INT(1),
			 COS(MUL(fp_pos,
				 PI))));
	 break;
      case EDJE_TWEEN_MODE_ACCELERATE:
	 /* npos = 1.0 - sin((PI / 2.0) + (pos * PI / 2.0)); */
	 npos = SUB(FROM_INT(1),
		    SIN(ADD(DIV2(PI),
			    MUL(fp_pos,
				DIV2(PI)))));
	 break;
      case EDJE_TWEEN_MODE_DECELERATE:
	 /* npos = sin(pos * PI / 2.0); */
	 npos = SIN(MUL(fp_pos,
			DIV2(PI)));
	break;
      case EDJE_TWEEN_MODE_LINEAR:
	 npos = fp_pos;
	 break;
      default:
         npos = fp_pos;
         break;
     }
#else
   switch (mode & EDJE_TWEEN_MODE_MASK)
     {
      case EDJE_TWEEN_MODE_SINUSOIDAL:
        npos = FROM_DOUBLE(ecore_animator_pos_map(TO_DOUBLE(pos),
                                                  ECORE_POS_MAP_SINUSOIDAL,
                                                  0.0, 0.0));
        break;
      case EDJE_TWEEN_MODE_ACCELERATE:
        npos = FROM_DOUBLE(ecore_animator_pos_map(TO_DOUBLE(pos),
                                                  ECORE_POS_MAP_ACCELERATE,
                                                  0.0, 0.0));
        break;
      case EDJE_TWEEN_MODE_DECELERATE:
        npos = FROM_DOUBLE(ecore_animator_pos_map(TO_DOUBLE(pos),
                                                  ECORE_POS_MAP_DECELERATE,
                                                  0.0, 0.0));
        break;
      case EDJE_TWEEN_MODE_LINEAR:
        npos = fp_pos;
/*        npos = FROM_DOUBLE(ecore_animator_pos_map(TO_DOUBLE(pos),
                                                  ECORE_POS_MAP_LINEAR, 
                                                  0.0, 0.0));
 */
        break;
      case EDJE_TWEEN_MODE_ACCELERATE_FACTOR:
        npos = FROM_DOUBLE(ecore_animator_pos_map(TO_DOUBLE(pos),
                                                  ECORE_POS_MAP_ACCELERATE_FACTOR,
                                                  TO_DOUBLE(v1), 0.0));
        break;
      case EDJE_TWEEN_MODE_DECELERATE_FACTOR:
        npos = FROM_DOUBLE(ecore_animator_pos_map(TO_DOUBLE(pos),
                                                  ECORE_POS_MAP_DECELERATE_FACTOR,
                                                  TO_DOUBLE(v1), 0.0));
        break;
      case EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR:
        npos = FROM_DOUBLE(ecore_animator_pos_map(TO_DOUBLE(pos),
                                                  ECORE_POS_MAP_SINUSOIDAL_FACTOR,
                                                  TO_DOUBLE(v1), 0.0));
        break;
      case EDJE_TWEEN_MODE_DIVISOR_INTERP:
        npos = FROM_DOUBLE(ecore_animator_pos_map(TO_DOUBLE(pos),
                                                  ECORE_POS_MAP_DIVISOR_INTERP,
                                                  TO_DOUBLE(v1), TO_DOUBLE(v2)));
        break;
      case EDJE_TWEEN_MODE_BOUNCE:
        npos = FROM_DOUBLE(ecore_animator_pos_map(TO_DOUBLE(pos),
                                                  ECORE_POS_MAP_BOUNCE,
                                                  TO_DOUBLE(v1), TO_DOUBLE(v2)));
        break;
      case EDJE_TWEEN_MODE_SPRING:
        npos = FROM_DOUBLE(ecore_animator_pos_map(TO_DOUBLE(pos),
                                                  ECORE_POS_MAP_SPRING,
                                                  TO_DOUBLE(v1), TO_DOUBLE(v2)));
        break;
      case EDJE_TWEEN_MODE_CUBIC_BEZIER:
        v[0] = TO_DOUBLE(v1);
        v[1] = TO_DOUBLE(v2);
        v[2] = TO_DOUBLE(v3);
        v[3] = TO_DOUBLE(v4);

        npos = FROM_DOUBLE(ecore_animator_pos_map_n(TO_DOUBLE(pos),
                                                    ECORE_POS_MAP_CUBIC_BEZIER,
                                                    4, v));
        break;
      default:
        npos = fp_pos;
        break;
     }
#endif
   if (npos == ep->description_pos) return;

   ep->description_pos = npos;

   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   ep->invalidate = EINA_TRUE;
#endif
}


/**
 * Returns part description
 *
 * @internal
 *
 * Converts part description to RTL-desc.
 *
 * @param desc Pointer to desc buffer.
 *
 **/
static void
_edje_part_make_rtl(Edje_Part_Description_Common *desc)
{
   double t;
   int i;

   if(!desc)
     return;

   /* This makes alignment right-oriented */
   desc->align.x = 1.0 - desc->align.x;

   /* same as above for relative components */
   t = desc->rel1.relative_x;
   desc->rel1.relative_x = 1.0 - desc->rel2.relative_x;
   desc->rel2.relative_x = 1.0 - t;

   /* +1 and +1 are because how edje works with right
    * side borders - nothing is printed beyond that limit
    *
    * rel2 is now to the left of rel1, and Edje assumes
    * the opposite so we switch corners on x-axis to define
    * offset from right to left */
   i = desc->rel1.offset_x;
   desc->rel1.offset_x = -(desc->rel2.offset_x + 1);
   desc->rel2.offset_x = -(i + 1);

   i = desc->rel1.id_x;
   desc->rel1.id_x = desc->rel2.id_x;
   desc->rel2.id_x = i;
}

/**
 * Returns part description
 *
 * @internal
 *
 * Returns part description according to object orientation.
 * When object is in RTL-orientation (RTL flag is set)
 * this returns the RTL-desc of it.
 * RTL-desc would be allocated if was not created by a previous call.
 * The dst pointer is updated in case of an allocation.
 *
 * @param ed Edje object.
 * @param src The Left To Right (LTR), original desc.
 * @param dst Pointer to Right To Left (RTL) desc-list.
 * @param type name of dec type. Example: "default".
 *
 * @return Edje part description.
 *
 **/
static Edje_Part_Description_Common *
_edje_get_description_by_orientation(Edje *ed, Edje_Part_Description_Common *src, Edje_Part_Description_Common **dst, unsigned char type)
{
   Edje_Part_Description_Common *desc_rtl = NULL;
   Edje_Part_Collection_Directory_Entry *ce;
   size_t memsize = 0;

   /* RTL flag is not set, return original description */
   if(!edje_object_mirrored_get(ed->obj))
      return src;

   if(*dst)
     return *dst; /* Was allocated before and we should use it */

#define EDIT_ALLOC_POOL_RTL(Short, Type, Name)                          \
         case EDJE_PART_TYPE_##Short:                                   \
           {                                                            \
              Edje_Part_Description_##Type *Name;                       \
              Name = eina_mempool_malloc(ce->mp_rtl.Short,              \
                    sizeof (Edje_Part_Description_##Type));             \
              desc_rtl = &Name->common;                                 \
              memsize = sizeof(Edje_Part_Description_##Type);           \
              break;                                                    \
           }

   ce = eina_hash_find(ed->file->collection, ed->group);

   switch (type)
     {
      case EDJE_PART_TYPE_RECTANGLE:
         desc_rtl = eina_mempool_malloc(ce->mp_rtl.RECTANGLE,
                                        sizeof (Edje_Part_Description_Common));
         ce->count.RECTANGLE++;
         memsize = sizeof(Edje_Part_Description_Common);
         break;
      case EDJE_PART_TYPE_SWALLOW:
         desc_rtl = eina_mempool_malloc(ce->mp_rtl.SWALLOW,
                                        sizeof (Edje_Part_Description_Common));
         ce->count.SWALLOW++;
         memsize = sizeof(Edje_Part_Description_Common);
         break;
      case EDJE_PART_TYPE_GROUP:
         desc_rtl = eina_mempool_malloc(ce->mp_rtl.GROUP,
                                        sizeof (Edje_Part_Description_Common));
         ce->count.GROUP++;
         memsize = sizeof(Edje_Part_Description_Common);
         break;
     case EDJE_PART_TYPE_SPACER:
         desc_rtl = eina_mempool_malloc(ce->mp_rtl.SPACER,
                                        sizeof (Edje_Part_Description_Common));
         ce->count.SPACER++;
         memsize = sizeof(Edje_Part_Description_Common);
         break;
         EDIT_ALLOC_POOL_RTL(TEXT, Text, text);
         EDIT_ALLOC_POOL_RTL(TEXTBLOCK, Text, text);
         EDIT_ALLOC_POOL_RTL(IMAGE, Image, image);
         EDIT_ALLOC_POOL_RTL(PROXY, Proxy, proxy);
         EDIT_ALLOC_POOL_RTL(BOX, Box, box);
         EDIT_ALLOC_POOL_RTL(TABLE, Table, table);
         EDIT_ALLOC_POOL_RTL(EXTERNAL, External, external_params);
     }

   if (desc_rtl)
      memcpy(desc_rtl, src, memsize);

   _edje_part_make_rtl(desc_rtl);

   *dst = desc_rtl;
   return desc_rtl;
}

Edje_Part_Description_Common *
_edje_part_description_find(Edje *ed, Edje_Real_Part *rp, const char *state_name,
                            double state_val, Eina_Bool approximate)
{
   Edje_Part *ep = rp->part;
   Edje_Part_Description_Common *ret = NULL;
   Edje_Part_Description_Common *d;

   double min_dst = 99999.0;
   unsigned int i;

   /* RTL flag is set, return RTL description */
   if(edje_object_mirrored_get(ed->obj))
     if(!ep->other.desc_rtl)
       ep->other.desc_rtl = (Edje_Part_Description_Common **)
          calloc(ep->other.desc_count,
                sizeof (Edje_Part_Description_Common *));

   if (!strcmp(state_name, "default") && state_val == 0.0)
     return _edje_get_description_by_orientation(ed,
           ep->default_desc, &ep->default_desc_rtl, ep->type);

   if (!strcmp(state_name, "custom"))
     return rp->custom ?
        _edje_get_description_by_orientation(ed, rp->custom->description,
              &rp->custom->description_rtl, ep->type) : NULL;

   if (!strcmp(state_name, "default") && approximate)
     {
        ret = _edje_get_description_by_orientation(ed, ep->default_desc,
                                                   &ep->default_desc_rtl,
                                                   ep->type);

        min_dst = ABS(ep->default_desc->state.value - state_val);
     }

   for (i = 0; i < ep->other.desc_count; ++i)
     {
        d = ep->other.desc[i];

        if (d->state.name && (d->state.name == state_name ||
                              !strcmp(d->state.name, state_name)))
          {
             if (!approximate)
               {
                  if (d->state.value == state_val)
                    return _edje_get_description_by_orientation(ed, d,
                                                                &ep->other.desc_rtl[i], ep->type);
                  else
                    continue;
               }
             else
               {
                  double dst;

                  dst = ABS(d->state.value - state_val);
                  if (dst < min_dst)
                    {
                       ret = _edje_get_description_by_orientation(ed, d,
                                                                  &ep->other.desc_rtl[i], ep->type);
                       min_dst = dst;
                    }
               }
          }
     }

   return ret;
}

static int
_edje_image_find(Evas_Object *obj, Edje *ed, Edje_Real_Part_Set **eps, 
                 Edje_Part_Description_Image *st, Edje_Part_Image_Id *imid)
{
   Edje_Image_Directory_Set_Entry *entry;
   Edje_Image_Directory_Set *set = NULL;
   Eina_List *l;
   int w = 0, h = 0, id;
   
   if (!st && !imid)         return -1;
   if (st && !st->image.set) return st->image.id;
   if (imid && !imid->set)   return imid->id;
   
   if (imid) id = imid->id;
   else      id = st->image.id;
   
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (eps && *eps)
     {
        if ((*eps)->id == id) set = (*eps)->set;
        if (set)
          {
             if (((*eps)->entry->size.min.w <= w) && 
                 (w <= (*eps)->entry->size.max.w))
               {
                  if (((*eps)->entry->size.min.h <= h) && 
                      (h <= (*eps)->entry->size.max.h))
                    {
                       return (*eps)->entry->id;
                    }
               }
          }
     }
   
   if (!set) set = ed->file->image_dir->sets + id;
   
   EINA_LIST_FOREACH(set->entries, l, entry)
     {
        if ((entry->size.min.w <= w) && (w <= entry->size.max.w))
          {
             if ((entry->size.min.h <= h) && (h <= entry->size.max.h))
               {
                  if (eps)
                    {
                       if (!*eps) *eps = calloc(1, sizeof(Edje_Real_Part_Set));
                       if (*eps)
                         {
                            (*eps)->entry = entry;
                            (*eps)->set = set;
                            (*eps)->id = id;
                         }
                    }
                  return entry->id;
               }
          }
     }
   
   return -1;
}

static void
_edje_real_part_image_set(Edje *ed, Edje_Real_Part *ep, FLOAT_T pos)
{
   int image_id;
   int image_count, image_num;

   image_id = _edje_image_find(ep->object, ed,
                               &ep->param1.set,
                               (Edje_Part_Description_Image*) ep->param1.description,
                               NULL);
   if (image_id < 0)
     {
        Edje_Image_Directory_Entry *ie;

        if (!ed->file->image_dir) ie = NULL;
        else ie = ed->file->image_dir->entries + (-image_id) - 1;
        if ((ie) &&
            (ie->source_type == EDJE_IMAGE_SOURCE_TYPE_EXTERNAL) &&
            (ie->entry))
          {
             evas_object_image_file_set(ep->object, ie->entry, NULL);
          }
     }
   else
     {
        image_count = 2;
        if (ep->param2)
          image_count += ((Edje_Part_Description_Image*) ep->param2->description)->image.tweens_count;
        image_num = TO_INT(MUL(pos, SUB(FROM_INT(image_count),
                                        FROM_DOUBLE(0.5))));
        if (image_num > (image_count - 1))
          image_num = image_count - 1;
        if (image_num <= 0)
          {
             image_id = _edje_image_find(ep->object, ed,
                                         &ep->param1.set,
                                         (Edje_Part_Description_Image*) ep->param1.description,
                                         NULL);
          }
        else
          if (ep->param2)
            {
               if (image_num == (image_count - 1))
                 {
                    image_id = _edje_image_find(ep->object, ed,
                                                &ep->param2->set,
                                                (Edje_Part_Description_Image*) ep->param2->description,
                                                NULL);
                 }
               else
                 {
                    Edje_Part_Image_Id *imid;

                    imid = ((Edje_Part_Description_Image*) ep->param2->description)->image.tweens[image_num - 1];
                    image_id = _edje_image_find(ep->object, ed, NULL, NULL, imid);
                 }
            }
        if (image_id < 0)
          {
             ERR("¨Part \"%s\" description, "
                 "\"%s\" %3.3f with image %i index has a missing image id in a set of %i !!!",
                 ep->part->name,
                 ep->param1.description->state.name,
                 ep->param1.description->state.value,
                 image_num,
                 image_count);
          }
        else
          {
             char buf[1024] = "edje/images/";

             /* Replace snprint("edje/images/%i") == memcpy + itoa */
             eina_convert_itoa(image_id, buf + 12); /* No need to check length as 2³² need only 10 characteres. */

             evas_object_image_mmap_set(ep->object, ed->file->f, buf);
             if (evas_object_image_load_error_get(ep->object) != EVAS_LOAD_ERROR_NONE)
               {
                  ERR("Error loading image collection \"%s\" from "
                      "file \"%s\". Missing EET Evas loader module?",
                      buf, ed->file->path);
                  switch (evas_object_image_load_error_get(ep->object))
                    {
                     case EVAS_LOAD_ERROR_GENERIC:
                        ERR("Error type: EVAS_LOAD_ERROR_GENERIC");
                        break;
                     case EVAS_LOAD_ERROR_DOES_NOT_EXIST:
                        ERR("Error type: EVAS_LOAD_ERROR_DOES_NOT_EXIST");
                        break;
                     case EVAS_LOAD_ERROR_PERMISSION_DENIED:
                        ERR("Error type: EVAS_LOAD_ERROR_PERMISSION_DENIED");
                        break;
                     case EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED:
                        ERR("Error type: EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED");
                        break;
                     case EVAS_LOAD_ERROR_CORRUPT_FILE:
                        ERR("Error type: EVAS_LOAD_ERROR_CORRUPT_FILE");
                        break;
                     case EVAS_LOAD_ERROR_UNKNOWN_FORMAT:
                        ERR("Error type: EVAS_LOAD_ERROR_UNKNOWN_FORMAT");
                        break;
                     default:
                        ERR("Error type: ???");
                        break;
                    }
               }
          }
     }
}

static void
_edje_real_part_rel_to_apply(Edje *ed EINA_UNUSED,
			     Edje_Real_Part *ep,
			     Edje_Real_Part_State *state)
{
   if (state->description)
     {
        if (ep->part->type == EDJE_PART_TYPE_EXTERNAL)
          {
             Edje_Part_Description_External *external;

             if ((ep->type != EDJE_RP_TYPE_SWALLOW) ||
                 (!ep->typedata.swallow)) return;
             
             external = (Edje_Part_Description_External*)state->description;
             
             if (state->external_params)
               _edje_external_parsed_params_free(ep->typedata.swallow->swallowed_object, state->external_params);
             state->external_params = _edje_external_params_parse(ep->typedata.swallow->swallowed_object, external->external_params);
          }
     }
}

void
_edje_part_description_apply(Edje *ed, Edje_Real_Part *ep, const char *d1, double v1, const char *d2, double v2)
{
   Edje_Part_Description_Common *epd1;
   Edje_Part_Description_Common *epd2 = NULL;
   Edje_Part_Description_Common *chosen_desc;

   Edje_Part_Description_Image *epdi;

   if (!d1) d1 = "default";

   epd1 = _edje_part_description_find(ed, ep, d1, v1, EINA_TRUE);
   if (!epd1)
     epd1 = ep->part->default_desc; /* never NULL */

   if (d2)
     epd2 = _edje_part_description_find(ed, ep, d2, v2, EINA_TRUE);

   epdi = (Edje_Part_Description_Image*) epd2;

   /* There is an animation if both description are different or if description is an image with tweens */
   if (epd2 && (epd1 != epd2 || (ep->part->type == EDJE_PART_TYPE_IMAGE && epdi->image.tweens_count)))
     {
        if (!ep->param2)
          {
             ep->param2 = eina_mempool_malloc(_edje_real_part_state_mp,
                                              sizeof(Edje_Real_Part_State));
             memset(ep->param2, 0, sizeof(Edje_Real_Part_State));
	     ep->param2->p.map = eina_cow_alloc(_edje_calc_params_map_cow);
#ifdef HAVE_EPHYSICS
	     ep->param2->p.physics = eina_cow_alloc(_edje_calc_params_physics_cow);
#endif
          }
        else if (ep->part->type == EDJE_PART_TYPE_EXTERNAL)
          {
             if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
                 (ep->typedata.swallow))
               _edje_external_parsed_params_free(ep->typedata.swallow->swallowed_object,
                                                 ep->param2->external_params);
          }
        ep->param2->external_params = NULL;
     }
   else
     if (ep->param2)
       {
          if (ep->part->type == EDJE_PART_TYPE_EXTERNAL)
            {
               if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
                   (ep->typedata.swallow))
                 _edje_external_parsed_params_free(ep->typedata.swallow->swallowed_object,
                                                   ep->param2->external_params);
            }
          if (ep->param2)
	    {
	      free(ep->param2->set);
	      eina_cow_free(_edje_calc_params_map_cow, (const Eina_Cow_Data **) &ep->param2->p.map);
#ifdef HAVE_EPHYSICS
	      eina_cow_free(_edje_calc_params_physics_cow, (const Eina_Cow_Data **) &ep->param2->p.physics);
#endif
	    }
          eina_mempool_free(_edje_real_part_state_mp, ep->param2);
          ep->param2 = NULL;
       }

   chosen_desc = ep->chosen_description;
   ep->param1.description = epd1;
   ep->chosen_description = epd1;

   _edje_real_part_rel_to_apply(ed, ep, &ep->param1);

   if (ep->param2)
     {
        ep->param2->description = epd2;

        _edje_real_part_rel_to_apply(ed, ep, ep->param2);

        if (ep->description_pos > FROM_DOUBLE(0.0))
          ep->chosen_description = epd2;
     }

   if (chosen_desc != ep->chosen_description &&
       ep->part->type == EDJE_PART_TYPE_EXTERNAL)
     _edje_external_recalc_apply(ed, ep, NULL, chosen_desc);

   ed->recalc_hints = EINA_TRUE;
   ed->dirty = EINA_TRUE;
   ed->recalc_call = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   ep->invalidate = EINA_TRUE;
#endif
}

void
_edje_recalc(Edje *ed)
{
   if ((ed->freeze > 0) || (_edje_util_freeze_val > 0))
     {
        ed->recalc = EINA_TRUE;
        if (!ed->calc_only)
          {
             if (_edje_util_freeze_val > 0)
               {
                  if (!ed->freeze_calc)
                    {
                       _edje_util_freeze_calc_count++;
                       _edje_util_freeze_calc_list = eina_list_append(_edje_util_freeze_calc_list, ed);
                       ed->freeze_calc = EINA_TRUE;
                    }
               }
             return;
          }
     }
// XXX: dont need this with current smart calc infra. remove me later
//   if (ed->postponed) return;
//   if (!ed->calc_only)
     evas_object_smart_changed(ed->obj);
// XXX: dont need this with current smart calc infra. remove me later
//   ed->postponed = EINA_TRUE;
}

void
_edje_recalc_do(Edje *ed)
{
   unsigned int i;
   Eina_Bool need_calc;

// XXX: dont need this with current smart calc infra. remove me later
//   ed->postponed = EINA_FALSE;
   need_calc = evas_object_smart_need_recalculate_get(ed->obj);
   evas_object_smart_need_recalculate_set(ed->obj, 0);
   if (!ed->dirty) return;
   ed->dirty = EINA_FALSE;
   ed->state++;
   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *ep;

        ep = ed->table_parts[i];
        ep->calculated = FLAG_NONE;
        ep->calculating = FLAG_NONE;
     }
   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *ep;

        ep = ed->table_parts[i];
        if (ep->calculated != FLAG_XY)
          _edje_part_recalc(ed, ep, (~ep->calculated) & FLAG_XY, NULL);
     }
   if (!ed->calc_only) ed->recalc = EINA_FALSE;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = EINA_FALSE;
   ed->text_part_change = EINA_FALSE;
#endif
   if (!ed->calc_only)
     {
        if (ed->recalc_call)
          evas_object_smart_callback_call(ed->obj, "recalc", NULL);
     }
   else
     evas_object_smart_need_recalculate_set(ed->obj, need_calc);
   ed->recalc_call = EINA_FALSE;

   if (ed->update_hints && ed->recalc_hints && !ed->calc_only)
     {
        Evas_Coord w, h;

        ed->recalc_hints = EINA_FALSE;

	eo_do(ed->obj, edje_obj_size_min_calc(&w, &h));
	eo_do(ed->obj, evas_obj_size_hint_min_set(w, h));
     }

   if (!ed->collection) return;

   for (i = 0; i < ed->collection->limits.parts_count; i++)
     {
        const char *name;
        unsigned char limit;
        int part;

        part = ed->collection->limits.parts[i].part;
        name = ed->collection->parts[part]->name;
        limit = ed->table_parts[part]->chosen_description->limit;
        switch (limit)
          {
           case 0:
              ed->collection->limits.parts[i].width = EDJE_PART_LIMIT_UNKNOWN;
              ed->collection->limits.parts[i].height = EDJE_PART_LIMIT_UNKNOWN;
              break;
           case 1:
              ed->collection->limits.parts[i].height = EDJE_PART_LIMIT_UNKNOWN;
              break;
           case 2:
              ed->collection->limits.parts[i].width = EDJE_PART_LIMIT_UNKNOWN;
              break;
           case 3:
              break;
          }

        if ((limit & 1) == 1)
          {
             if (ed->table_parts[part]->w > 0 &&
                 (ed->collection->limits.parts[i].width != EDJE_PART_LIMIT_OVER))
               {
                  ed->collection->limits.parts[i].width = EDJE_PART_LIMIT_OVER;
                  _edje_emit(ed, "limit,width,over", name);
               }
             else if (ed->table_parts[part]->w < 0 &&
                      ed->collection->limits.parts[i].width != EDJE_PART_LIMIT_BELOW)
               {
                  ed->collection->limits.parts[i].width = EDJE_PART_LIMIT_BELOW;
                  _edje_emit(ed, "limit,width,below", name);
               }
             else if (ed->table_parts[part]->w == 0 &&
                      ed->collection->limits.parts[i].width != EDJE_PART_LIMIT_ZERO)
               {
                  ed->collection->limits.parts[i].width = EDJE_PART_LIMIT_ZERO;
                  _edje_emit(ed, "limit,width,zero", name);
               }
          }
        if ((limit & 2) == 2)
          {
             if (ed->table_parts[part]->h > 0 &&
                 (ed->collection->limits.parts[i].height != EDJE_PART_LIMIT_OVER))
               {
                  ed->collection->limits.parts[i].height = EDJE_PART_LIMIT_OVER;
                  _edje_emit(ed, "limit,height,over", name);
               }
             else if (ed->table_parts[part]->h < 0 &&
                      ed->collection->limits.parts[i].height != EDJE_PART_LIMIT_BELOW)
               {
                  ed->collection->limits.parts[i].height = EDJE_PART_LIMIT_BELOW;
                  _edje_emit(ed, "limit,height,below", name);
               }
             else if (ed->table_parts[part]->h == 0 &&
                      ed->collection->limits.parts[i].height != EDJE_PART_LIMIT_ZERO)
               {
                  ed->collection->limits.parts[i].height = EDJE_PART_LIMIT_ZERO;
                  _edje_emit(ed, "limit,height,zero", name);
               }
          }
     }
}

int
_edje_part_dragable_calc(Edje *ed EINA_UNUSED, Edje_Real_Part *ep, FLOAT_T *x, FLOAT_T *y)
{
   if (ep->drag)
     {
        Eina_Bool tx = EINA_FALSE;
        Eina_Bool ty = EINA_FALSE;

        if (ep->drag->threshold)
          {
             // Check if we are in the threshold or not and cancel the movement.
             tx = ep->drag->threshold_x && ep->drag->threshold_started_x;
             ty = ep->drag->threshold_y && ep->drag->threshold_started_y;
          }
        if (ep->drag->confine_to)
          {
             FLOAT_T dx, dy, dw, dh;
             int ret = 0;

             if ((ep->part->dragable.x != 0) &&
                 (ep->part->dragable.y != 0 )) ret = 3;
             else if (ep->part->dragable.x != 0) ret = 1;
             else if (ep->part->dragable.y != 0) ret = 2;

             dx = FROM_INT(ep->x - ep->drag->confine_to->x);
             dw = FROM_INT(ep->drag->confine_to->w - ep->w);
             if (dw != ZERO) dx = DIV(dx, dw);
             else dx = ZERO;

             dy = FROM_INT(ep->y - ep->drag->confine_to->y);
             dh = FROM_INT(ep->drag->confine_to->h - ep->h);
             if (dh != ZERO) dy = DIV(dy, dh);
             else dy = ZERO;

             if (x) *x = tx ? ep->drag->x : dx;
             if (y) *y = ty ? ep->drag->y : dy;

             return ret;
          }
        else
          {
             if (x) *x = tx ? ep->drag->x : ADD(FROM_INT(ep->drag->tmp.x), ep->drag->x);
             if (y) *y = ty ? ep->drag->y : ADD(FROM_INT(ep->drag->tmp.y), ep->drag->y);
             return 0;
          }
     }
   if (x) *x = ZERO;
   if (y) *y = ZERO;
   return 0;
}

void
_edje_dragable_pos_set(Edje *ed, Edje_Real_Part *ep, FLOAT_T x, FLOAT_T y)
{
   /* check whether this part is dragable at all */
   if (!ep->drag) return;

   /* instead of checking for equality, we really should check that
    * the difference is greater than foo, but I have no idea what
    * value we would set foo to, because it would depend on the
    * size of the dragable...
    */
   if (ep->drag->x != x || ep->drag->tmp.x)
     {
        ep->drag->x = x;
        ep->drag->tmp.x = 0;
        ep->drag->need_reset = 0;
        ed->dirty = EINA_TRUE;
        ed->recalc_call = EINA_TRUE;
     }

   if (ep->drag->y != y || ep->drag->tmp.y)
     {
        ep->drag->y = y;
        ep->drag->tmp.y = 0;
        ep->drag->need_reset = 0;
        ed->dirty = EINA_TRUE;
        ed->recalc_call = EINA_TRUE;
     }

#ifdef EDJE_CALC_CACHE
   ep->invalidate = EINA_TRUE;
#endif
   _edje_recalc(ed); /* won't do anything if dirty flag isn't set */
}

static void
_edje_part_recalc_single_rel(Edje *ed,
                             Edje_Real_Part *ep EINA_UNUSED,
                             Edje_Part_Description_Common *desc,
                             Edje_Real_Part *rel1_to_x,
                             Edje_Real_Part *rel1_to_y,
                             Edje_Real_Part *rel2_to_x,
                             Edje_Real_Part *rel2_to_y,
                             Edje_Calc_Params *params)
{
   FLOAT_T x, w;
   FLOAT_T y, h;

   if (rel1_to_x)
     x = ADD(FROM_INT(desc->rel1.offset_x + rel1_to_x->x),
             SCALE(desc->rel1.relative_x, rel1_to_x->w));
   else
     x = ADD(FROM_INT(desc->rel1.offset_x),
             SCALE(desc->rel1.relative_x, ed->w));
   params->eval.x = x;

   if (rel2_to_x)
     w = ADD(SUB(ADD(FROM_INT(desc->rel2.offset_x + rel2_to_x->x),
                     SCALE(desc->rel2.relative_x, rel2_to_x->w)),
                 x),
             FROM_INT(1));
   else
     w = ADD(SUB(ADD(FROM_INT(desc->rel2.offset_x),
                     SCALE(desc->rel2.relative_x, ed->w)),
                 x),
             FROM_INT(1));
   params->eval.w = w;

   if (rel1_to_y)
     y = ADD(FROM_INT(desc->rel1.offset_y + rel1_to_y->y),
             SCALE(desc->rel1.relative_y, rel1_to_y->h));
   else
     y = ADD(FROM_INT(desc->rel1.offset_y),
             SCALE(desc->rel1.relative_y, ed->h));
   params->eval.y = y;

   if (rel2_to_y)
     h = ADD(SUB(ADD(FROM_INT(desc->rel2.offset_y + rel2_to_y->y),
                     SCALE(desc->rel2.relative_y, rel2_to_y->h)),
                 y),
             FROM_INT(1));
   else
     h = ADD(SUB(ADD(FROM_INT(desc->rel2.offset_y),
                     SCALE(desc->rel2.relative_y, ed->h)),
                 y),
             FROM_INT(1));
   params->eval.h = h;
}

static Edje_Internal_Aspect
_edje_part_recalc_single_aspect(Edje *ed,
                                Edje_Real_Part *ep,
                                Edje_Part_Description_Common *desc,
                                Edje_Calc_Params *params,
                                int *minw, int *minh,
                                int *maxw, int *maxh,
                                FLOAT_T pos)
{
   Edje_Internal_Aspect apref = EDJE_ASPECT_PREFER_NONE;
   FLOAT_T aspect, amax, amin;
   FLOAT_T new_w = ZERO, new_h = ZERO, want_x, want_y, want_w, want_h;

   if (params->eval.h <= ZERO) aspect = FROM_INT(999999);
   else aspect = DIV(params->eval.w, params->eval.h);
   amax = desc->aspect.max;
   amin = desc->aspect.min;
   if (desc->aspect.prefer == EDJE_ASPECT_PREFER_SOURCE &&
       ep->part->type == EDJE_PART_TYPE_IMAGE)
     {
        Evas_Coord w, h;

        /* We only need pose to find the right image that would be displayed,
           and the right aspect ratio in that case */
        _edje_real_part_image_set(ed, ep, pos);
        evas_object_image_size_get(ep->object, &w, &h);
        amin = amax = DIV(FROM_INT(w), FROM_INT(h));
     }
   if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
       (ep->typedata.swallow))
     {
        if ((ep->typedata.swallow->swallow_params.aspect.w > 0) &&
            (ep->typedata.swallow->swallow_params.aspect.h > 0))
          amin = amax =
          DIV(FROM_INT(ep->typedata.swallow->swallow_params.aspect.w),
              FROM_INT(ep->typedata.swallow->swallow_params.aspect.h));
     }
   want_x = params->eval.x;
   want_w = new_w = params->eval.w;

   want_y = params->eval.y;
   want_h = new_h = params->eval.h;

   if ((amin > ZERO) && (amax > ZERO))
     {
        apref = desc->aspect.prefer;
        if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
            (ep->typedata.swallow))
          {
             if (ep->typedata.swallow->swallow_params.aspect.mode > EDJE_ASPECT_CONTROL_NONE)
               {
                  switch (ep->typedata.swallow->swallow_params.aspect.mode)
                    {
                     case EDJE_ASPECT_CONTROL_NEITHER:
                       apref = EDJE_ASPECT_PREFER_NONE;
                       break;
                     case EDJE_ASPECT_CONTROL_HORIZONTAL:
                       apref = EDJE_ASPECT_PREFER_HORIZONTAL;
                       break;
                     case EDJE_ASPECT_CONTROL_VERTICAL:
                       apref = EDJE_ASPECT_PREFER_VERTICAL;
                       break;
                     case EDJE_ASPECT_CONTROL_BOTH:
                       apref = EDJE_ASPECT_PREFER_BOTH;
                       break;
                     default:
                       break;
                    }
               }
          }
        switch (apref)
          {
           case EDJE_ASPECT_PREFER_NONE:
              /* keep both dimensions in check */
              /* adjust for min aspect (width / height) */
              if ((amin > ZERO) && (aspect < amin))
                {
                   new_h = DIV(params->eval.w, amin);
                   new_w = MUL(amin, params->eval.h);
                }
              /* adjust for max aspect (width / height) */
              if ((amax > ZERO) && (aspect > amax))
                {
                   new_h = DIV(params->eval.w, amax);
                   new_w = MUL(amax, params->eval.h);
                }
              if ((amax > ZERO) && (new_w < params->eval.w))
                {
                   new_w = params->eval.w;
                   new_h = DIV(params->eval.w, amax);
                }
              if ((amax > ZERO) && (new_h < params->eval.h))
                {
                   new_w = MUL(amax, params->eval.h);
                   new_h = params->eval.h;
                }
              break;
              /* prefer vertical size as determiner */
           case  EDJE_ASPECT_PREFER_VERTICAL:
              /* keep both dimensions in check */
              /* adjust for max aspect (width / height) */
              if ((amax > ZERO) && (aspect > amax))
                new_w = MUL(amax, params->eval.h);
              /* adjust for min aspect (width / height) */
              if ((amin > ZERO) && (aspect < amin))
                new_w = MUL(amin, params->eval.h);
              break;
              /* prefer horizontal size as determiner */
           case EDJE_ASPECT_PREFER_HORIZONTAL:
              /* keep both dimensions in check */
              /* adjust for max aspect (width / height) */
              if ((amax > ZERO) && (aspect > amax))
                new_h = DIV(params->eval.w, amax);
              /* adjust for min aspect (width / height) */
              if ((amin > ZERO) && (aspect < amin))
                new_h = DIV(params->eval.w, amin);
              break;
           case EDJE_ASPECT_PREFER_SOURCE:
           case EDJE_ASPECT_PREFER_BOTH:
              /* keep both dimensions in check */
              /* adjust for max aspect (width / height) */
              if ((amax > ZERO) && (aspect > amax))
                {
                   new_w = MUL(amax, params->eval.h);
                   new_h = DIV(params->eval.w, amax);
                }
              /* adjust for min aspect (width / height) */
              if ((amin > ZERO) && (aspect < amin))
                {
                   new_w = MUL(amin, params->eval.h);
                   new_h = DIV(params->eval.w, amin);
                }
              break;
           default:
              break;
          }

        if (!((amin > ZERO) && (amax > ZERO) &&
              (apref == EDJE_ASPECT_PREFER_NONE)))
          {
             if ((*maxw >= 0) && (new_w > FROM_INT(*maxw)))
               new_w = FROM_INT(*maxw);
             if (new_w < FROM_INT(*minw))
               new_w = FROM_INT(*minw);

             if ((FROM_INT(*maxh) >= 0) && (new_h > FROM_INT(*maxh)))
               new_h = FROM_INT(*maxh);
             if (new_h < FROM_INT(*minh))
               new_h = FROM_INT(*minh);
          }

        /* do real adjustment */
        if (apref == EDJE_ASPECT_PREFER_BOTH)
          {
             if (amin == ZERO) amin = amax;
             if (amin != ZERO)
               {
                  /* fix h and vary w */
                  if (new_w > params->eval.w)
                    {
                       //		  params->w = new_w;
                       // EXCEEDS BOUNDS in W
                       new_h = DIV(params->eval.w, amin);
                       new_w = params->eval.w;
                       if (new_h > params->eval.h)
                         {
                            new_h = params->eval.h;
                            new_w = MUL(amin, params->eval.h);
                         }
                    }
                  /* fix w and vary h */
                  else
                    {
                       //		  params->h = new_h;
                       // EXCEEDS BOUNDS in H
                       new_h = params->eval.h;
                       new_w = MUL(amin, params->eval.h);
                       if (new_w > params->eval.w)
                         {
                            new_h = DIV(params->eval.w, amin);
                            new_w = params->eval.w;
                         }
                    }
                  params->eval.w = new_w;
                  params->eval.h = new_h;
               }
          }
     }
   if (apref != EDJE_ASPECT_PREFER_BOTH)
     {
        if ((amin > 0.0) && (amax > ZERO) && (apref == EDJE_ASPECT_PREFER_NONE))
          {
             params->eval.w = new_w;
             params->eval.h = new_h;
          }
        else if (SUB(params->eval.h, new_h) > SUB(params->eval.w, new_w))
          {
             if (params->eval.h < new_h)
               params->eval.h = new_h;
             else if (params->eval.h > new_h)
               params->eval.h = new_h;
             if (apref == EDJE_ASPECT_PREFER_VERTICAL)
               params->eval.w = new_w;
          }
        else
          {
             if (params->eval.w < new_w)
               params->eval.w = new_w;
             else if (params->eval.w > new_w)
               params->eval.w = new_w;
             if (apref == EDJE_ASPECT_PREFER_HORIZONTAL)
               params->eval.h = new_h;
          }
     }

   params->eval.x = ADD(want_x,
			MUL(SUB(want_w, params->eval.w),
			    desc->align.x));
   params->eval.y = ADD(want_y,
                        MUL(SUB(want_h, params->eval.h),
                            desc->align.y));
   return apref;
}

static void
_edje_part_recalc_single_step(Edje_Part_Description_Common *desc,
                              Edje_Calc_Params *params)
{
   if (desc->step.x > 0)
     {
        int steps;
        int new_w;

        steps = TO_INT(params->eval.w) / desc->step.x;
        new_w = desc->step.x * steps;
        if (params->eval.w > FROM_INT(new_w))
          {
             params->eval.x = ADD(params->eval.x, SCALE(desc->align.x, SUB(params->eval.w, FROM_INT(new_w))));
             params->eval.w = FROM_INT(new_w);
          }
     }

   if (desc->step.y > 0)
     {
        int steps;
        int new_h;

        steps = TO_INT(params->eval.h) / desc->step.y;
        new_h = desc->step.y * steps;
        if (params->eval.h > FROM_INT(new_h))
          {
             params->eval.y = ADD(params->eval.y, SCALE(desc->align.y, SUB(params->eval.h, FROM_INT(new_h))));
             params->eval.h = FROM_INT(new_h);
          }
     }
}

static double
_edje_part_recalc_single_textblock_scale_range_adjust(Edje_Part_Description_Text *chosen_desc, double base_scale, double scale)
{
   double size, min, max;

   if (chosen_desc->text.size == 0)
      return scale;

   min = base_scale * chosen_desc->text.size_range_min;
   max = chosen_desc->text.size_range_max * base_scale;
   size = chosen_desc->text.size * scale;

   if ((size > max) && (max > 0))
      scale = max / (double) chosen_desc->text.size;
   else if (size < min)
      scale = min / (double) chosen_desc->text.size;

   return scale;
}

static void
_edje_part_recalc_single_textblock(FLOAT_T sc,
                                   Edje *ed,
                                   Edje_Real_Part *ep,
                                   Edje_Part_Description_Text *chosen_desc,
                                   Edje_Calc_Params *params,
                                   int *minw, int *minh,
                                   int *maxw, int *maxh)
{
   if ((ep->type != EDJE_RP_TYPE_TEXT) ||
       (!ep->typedata.text))
     return;
   if (chosen_desc)
     {
        Evas_Coord tw, th, ins_l, ins_r, ins_t, ins_b;
        const char *text = "";
        const char *style = "";
        Edje_Style *stl  = NULL;
        const char *tmp;
        Eina_List *l;

        if (chosen_desc->text.id_source >= 0)
          {
             Edje_Part_Description_Text *et;

             ep->typedata.text->source = ed->table_parts[chosen_desc->text.id_source % ed->table_parts_size];

             et = _edje_real_part_text_source_description_get(ep, NULL);
             tmp = edje_string_get(&et->text.style);
             if (tmp) style = tmp;
          }
        else
          {
             ep->typedata.text->source = NULL;

             tmp = edje_string_get(&chosen_desc->text.style);
             if (tmp) style = tmp;
          }

        if (chosen_desc->text.id_text_source >= 0)
          {
             Edje_Part_Description_Text *et;
             Edje_Real_Part *rp;

             ep->typedata.text->text_source = ed->table_parts[chosen_desc->text.id_text_source % ed->table_parts_size];

             et = _edje_real_part_text_text_source_description_get(ep, &rp);
             text = edje_string_get(&et->text.text);

             if (rp->typedata.text->text) text = rp->typedata.text->text;
          }
        else
          {
             ep->typedata.text->text_source = NULL;
             text = edje_string_get(&chosen_desc->text.text);
             if (ep->typedata.text->text) text = ep->typedata.text->text;
          }

        EINA_LIST_FOREACH(ed->file->styles, l, stl)
          {
             if ((stl->name) && (!strcmp(stl->name, style))) break;
             stl = NULL;
          }

        if (ep->part->scale)
          evas_object_scale_set(ep->object, TO_DOUBLE(sc));

        if ((chosen_desc->text.fit_x) || (chosen_desc->text.fit_y))
          {
             double base_s = 1.0;
             double orig_s;
             double s = base_s;

             if (ep->part->scale) base_s = TO_DOUBLE(sc);
	     eo_do(ep->object,
		   evas_obj_scale_set(base_s),
		   evas_obj_textblock_size_native_get(&tw, &th));

             orig_s = base_s;
             /* Now make it bigger so calculations will be more accurate
              * and less influenced by hinting... */
               {
                  orig_s = _edje_part_recalc_single_textblock_scale_range_adjust(chosen_desc, base_s,
                                                                                 orig_s * TO_INT(params->eval.w) / tw);
                  eo_do(ep->object,
                        evas_obj_scale_set(orig_s),
                        evas_obj_textblock_size_native_get(&tw, &th));
               }
             if (chosen_desc->text.fit_x)
               {
                  if (tw > 0)
                    {
                       s = _edje_part_recalc_single_textblock_scale_range_adjust(chosen_desc, base_s,
                                                                                 orig_s * TO_INT(params->eval.w) / tw);
		       eo_do(ep->object,
			     evas_obj_scale_set(s),
			     evas_obj_textblock_size_native_get(NULL, NULL));
                    }
               }
             if (chosen_desc->text.fit_y)
               {
                  if (th > 0)
                    {
                       double tmp_s = _edje_part_recalc_single_textblock_scale_range_adjust(chosen_desc, base_s,
                                                                                            orig_s * TO_INT(params->eval.h) / th);
                       /* If we already have X fit, restrict Y to be no bigger
                        * than what we got with X. */
                       if (!((chosen_desc->text.fit_x) && (tmp_s > s)))
                         {
                            s = tmp_s;
                         }

		       eo_do(ep->object,
			     evas_obj_scale_set(s),
			     evas_obj_textblock_size_native_get(NULL, NULL));
                    }
               }

             /* Final tuning, try going down 90% at a time, hoping it'll
              * actually end up being correct. */
               {
                  int i = 5; /* Tries before we give up. */
                  Evas_Coord fw, fh;
                  eo_do(ep->object,
                        evas_obj_textblock_size_native_get(&fw, &fh));

                  /* If we are still too big, try reducing the size to
                   * 95% each try. */
                  while ((i > 0) &&
                         ((chosen_desc->text.fit_x && (fw > TO_INT(params->eval.w))) ||
                          (chosen_desc->text.fit_y && (fh > TO_INT(params->eval.h)))))
                    {
                       double tmp_s = _edje_part_recalc_single_textblock_scale_range_adjust(chosen_desc, base_s, s * 0.95);

                       /* Break if we are not making any progress. */
                       if (tmp_s == s)
                          break;
                       s = tmp_s;

                       eo_do(ep->object,
			     evas_obj_scale_set(s),
                             evas_obj_textblock_size_native_get(&fw, &fh));
                       i--;
                    }
               }

          }

        if (stl)
          {
             const char *ptxt;

             if (evas_object_textblock_style_get(ep->object) != stl->style)
               evas_object_textblock_style_set(ep->object, stl->style);
             // FIXME: need to account for editing
             if (ep->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
               {
                  // do nothing - should be done elsewhere
               }
             else
               {
                  ptxt = evas_object_textblock_text_markup_get(ep->object);
                  if (((!ptxt) && (text)) ||
                      ((ptxt) && (text) && (strcmp(ptxt, text))) ||
                      ((ptxt) && (!text)))
                    evas_object_textblock_text_markup_set(ep->object, text);
               }
             if ((chosen_desc->text.min_x) || (chosen_desc->text.min_y))
               {
                  int mw = 0, mh = 0;

                  tw = th = 0;
                  if (!chosen_desc->text.min_x)
                    {
	               eo_do(ep->object,
	        	     evas_obj_size_set(TO_INT(params->eval.w), TO_INT(params->eval.h)),
	        	     evas_obj_textblock_size_formatted_get(&tw, &th));
                    }
                  else
                    evas_object_textblock_size_native_get(ep->object, &tw, &th);
                  evas_object_textblock_style_insets_get(ep->object, &ins_l,
                                                         &ins_r, &ins_t, &ins_b);
                  mw = ins_l + tw + ins_r;
                  mh = ins_t + th + ins_b;
                  if (minw && chosen_desc->text.min_x)
                    {
                       if (mw > *minw) *minw = mw;
                    }
                  if (minh && chosen_desc->text.min_y)
                    {
                       if (mh > *minh) *minh = mh;
                    }
               }
          }

        if ((chosen_desc->text.max_x) || (chosen_desc->text.max_y))
          {
             int mw = 0, mh = 0;

             tw = th = 0;
             if (!chosen_desc->text.max_x)
               {
		  eo_do(ep->object,
			evas_obj_size_set(TO_INT(params->eval.w), TO_INT(params->eval.h)),
			evas_obj_textblock_size_formatted_get(&tw, &th));
               }
             else
               evas_object_textblock_size_native_get(ep->object, &tw, &th);
             evas_object_textblock_style_insets_get(ep->object, &ins_l, &ins_r,
                                                    &ins_t, &ins_b);
             mw = ins_l + tw + ins_r;
             mh = ins_t + th + ins_b;
             if (maxw && chosen_desc->text.max_x)
               {
                  if (mw > *maxw) *maxw = mw;
                  if (minw && (*maxw < *minw)) *maxw = *minw;
               }
             if (maxh && chosen_desc->text.max_y)
               {
                  if (mh > *maxh) *maxh = mh;
                  if (minh && (*maxh < *minh)) *maxh = *minh;
               }
          }

        evas_object_textblock_valign_set(ep->object, TO_DOUBLE(chosen_desc->text.align.y));
     }
}

static void
_edje_textblock_recalc_apply(Edje *ed, Edje_Real_Part *ep,
			Edje_Calc_Params *params,
			Edje_Part_Description_Text *chosen_desc)
{
   /* FIXME: this is just an hack. */
   FLOAT_T sc;
   sc = DIV(ed->scale, ed->file->base_scale);
   if (sc == ZERO) sc = DIV(_edje_scale, ed->file->base_scale);
   if (chosen_desc->text.fit_x || chosen_desc->text.fit_y)
     {
        _edje_part_recalc_single_textblock(sc, ed, ep, chosen_desc, params,
              NULL, NULL, NULL, NULL);
     }
}

static void
_edje_part_recalc_single_text(FLOAT_T sc EINA_UNUSED,
                              Edje *ed,
                              Edje_Real_Part *ep,
                              Edje_Part_Description_Text *desc,
                              Edje_Part_Description_Text *chosen_desc,
                              Edje_Calc_Params *params,
                              int *minw, int *minh,
                              int *maxw, int *maxh)
#define RECALC_SINGLE_TEXT_USING_APPLY 1
#if RECALC_SINGLE_TEXT_USING_APPLY
/*
 * XXX TODO NOTE:
 *
 * Original _edje_part_recalc_single_text() was not working as
 * expected since it was not doing size fit, range, ellipsis and so
 * on.
 *
 * The purpose of this function compared with
 * _edje_text_recalc_apply() is to be faster, not calling Evas update
 * functions. However for text this is quite difficult given that to
 * fit we need to set the font, size, style, etc. If it was done
 * correctly, we'd save some calls to move and some color sets,
 * however those shouldn't matter much in the overall picture.
 *
 * I've changed this to force applying the value, it should be more
 * correct and not so slow. The previous code is kept below for
 * reference but should be removed before next release!
 *
 * -- Gustavo Barbieri at 20-Aug-2011
 */
{
   int mw, mh,size;
   char *sfont = NULL;

   _edje_text_class_font_get(ed, desc, &size, &sfont);
   free(sfont);
   params->type.text.size = size; /* XXX TODO used by further calcs, go inside recalc_apply? */

   _edje_text_recalc_apply(ed, ep, params, chosen_desc, EINA_TRUE);

   if ((!chosen_desc) ||
       ((!chosen_desc->text.min_x) && (!chosen_desc->text.min_y) &&
        (!chosen_desc->text.max_x) && (!chosen_desc->text.max_y)))
     return;

   // Note: No need to add padding to that, it's already in the geometry
   eo_do(ep->object, evas_obj_size_get(&mw, &mh));

   if (chosen_desc->text.max_x)
     {
        if ((*maxw < 0) || (mw < *maxw)) *maxw = mw;
     }
   if (chosen_desc->text.max_y)
     {
        if ((*maxh < 0) || (mh < *maxh)) *maxh = mh;
     }
   if (chosen_desc->text.min_x)
     {
        if (mw > *minw) *minw = mw;
     }
   if (chosen_desc->text.min_y)
     {
        if (mh > *minh) *minh = mh;
     }
}
#else
{
   char *sfont = NULL;
   int size;

   if (chosen_desc)
     {
        const char *text;
        const char *font;
        Evas_Coord tw, th;
        int inlined_font = 0;
        Edje_Real_Part *source, *text_source;

        /* Update a object_text part */

        if (chosen_desc->text.id_source >= 0)
          ep->typedata.text->source = ed->table_parts[chosen_desc->text.id_source % ed->table_parts_size];
        else
          source = ep->typedata.text->source = NULL;

        if (chosen_desc->text.id_text_source >= 0)
          ep->typedata.text->text_source = ed->table_parts[chosen_desc->text.id_text_source % ed->table_parts_size];
        else
          text_source = ep->typedata.text->text_source = NULL;

        if (ep->typedata.text->text_source)
          text = edje_string_get(&_edje_real_part_text_text_source_description_get(ep, &text_source)->text.text);
        else
          text = edje_string_get(&chosen_desc->text.text);

        if (ep->typedata.text->source)
          font = _edje_text_class_font_get(ed, _edje_real_part_text_source_description_get(ep, &source), &size, &sfont);
        else
          font = _edje_text_class_font_get(ed, chosen_desc, &size, &sfont);

        if (!font) font = "";

        if (text_source)
          {
             if (text_source->typedata.text->text) text = text_source->typedata.text->text;
          }
        else
          {
             if (ep->typedata.text->text) text = ep->typedata.text->text;
          }

        if (source)
          {
             if (source->typedata.text->font) font = source->typedata.text->font;
             if (source->typedata.text->size > 0) size = source->typedata.text->size;
          }
        else
          {
             if (ep->typedata.text->font) font = ep->typedata.text->font;
             if (ep->typedata.text->size > 0) size = ep->typedata.text->size;
          }
        if (!text) text = "";

        /* check if the font is embedded in the .eet */
        if (ed->file->fonts)
          {
             Edje_Font_Directory_Entry *fnt;

             fnt = eina_hash_find(ed->file->fonts, font);

             if (fnt)
               {
                  char *font2;

                  size_t len = strlen(font) + sizeof("edje/fonts/") + 1;
                  font2 = alloca(len);
                  sprintf(font2, "edje/fonts/%s", font);
                  font = font2;
                  inlined_font = 1;
               }
          }
        if (ep->part->scale)
          evas_object_scale_set(ep->object, TO_DOUBLE(sc));
        if (inlined_font)
          {
             evas_object_text_font_source_set(ep->object, ed->path);
          }
        else evas_object_text_font_source_set(ep->object, NULL);

        if ((_edje_fontset_append) && (font))
          {
             char *font2;

             font2 = malloc(strlen(font) + 1 + strlen(_edje_fontset_append) + 1);
             if (font2)
               {
                  strcpy(font2, font);
                  strcat(font2, ",");
                  strcat(font2, _edje_fontset_append);
                  evas_object_text_font_set(ep->object, font2, size);
                  free(font2);
               }
          }
        else
          evas_object_text_font_set(ep->object, font, size);
        if ((chosen_desc->text.min_x) || (chosen_desc->text.min_y) ||
            (chosen_desc->text.max_x) || (chosen_desc->text.max_y))
          {
             int mw, mh;
             Evas_Text_Style_Type
                style = EVAS_TEXT_STYLE_PLAIN,
                      shadow = EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT;
             const Evas_Text_Style_Type styles[] = {
                  EVAS_TEXT_STYLE_PLAIN,
                  EVAS_TEXT_STYLE_PLAIN,
                  EVAS_TEXT_STYLE_OUTLINE,
                  EVAS_TEXT_STYLE_SOFT_OUTLINE,
                  EVAS_TEXT_STYLE_SHADOW,
                  EVAS_TEXT_STYLE_SOFT_SHADOW,
                  EVAS_TEXT_STYLE_OUTLINE_SHADOW,
                  EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW,
                  EVAS_TEXT_STYLE_FAR_SHADOW,
                  EVAS_TEXT_STYLE_FAR_SOFT_SHADOW,
                  EVAS_TEXT_STYLE_GLOW
             };
             const Evas_Text_Style_Type shadows[] = {
                  EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT,
                  EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM,
                  EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT,
                  EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT,
                  EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT,
                EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP,
                EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT,
                EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT
             };

             if ((ep->part->effect & EVAS_TEXT_STYLE_MASK_BASIC)
                 < EDJE_TEXT_EFFECT_LAST)
               style = styles[ep->part->effect];
             shadow = shadows
                [(ep->part->effect & EDJE_TEXT_EFFECT_MASK_SHADOW_DIRECTION) >> 4];
             EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(style, shadow);

	     eo_do(ep->object,
		   evas_obj_text_style_set(style),
		   evas_obj_text_set(text),
		   evas_obj_size_get(&tw, &th));
             if (chosen_desc->text.max_x)
               {
                  int l, r;
                  evas_object_text_style_pad_get(ep->object, &l, &r, NULL, NULL);
                  mw = tw + l + r;
                  if ((*maxw < 0) || (mw < *maxw)) *maxw = mw;
               }
             if (chosen_desc->text.max_y)
               {
                  int t, b;
                  evas_object_text_style_pad_get(ep->object, NULL, NULL, &t, &b);
                  mh = th + t + b;
                  if ((*maxh < 0) || (mh < *maxh)) *maxh = mh;
               }
             if (chosen_desc->text.min_x)
               {
                  int l, r;
                  evas_object_text_style_pad_get(ep->object, &l, &r, NULL, NULL);
                  mw = tw + l + r;
                  if (mw > *minw) *minw = mw;
               }
             if (chosen_desc->text.min_y)
               {
                  int t, b;
                  evas_object_text_style_pad_get(ep->object, NULL, NULL, &t, &b);
                  mh = th + t + b;
                  if (mh > *minh) *minh = mh;
               }
          }
        if (sfont) free(sfont);
     }

   /* FIXME: Do we really need to call it twice if chosen_desc ? */
   sfont = NULL;
   _edje_text_class_font_get(ed, desc, &size, &sfont);
   free(sfont);
   params->type.text.size = size;
}
#endif

static void
_edje_part_recalc_single_min_length(FLOAT_T align, FLOAT_T *start, FLOAT_T *length, int min)
{
   if (min >= 0)
     {
        if (*length < FROM_INT(min))
          {
             *start = ADD(*start, (SCALE(align, (*length - min))));
             *length = FROM_INT(min);
          }
     }
}

static void
_edje_part_recalc_single_min(Edje_Part_Description_Common *desc,
                             Edje_Calc_Params *params,
                             int minw, int minh,
                             Edje_Internal_Aspect aspect)
{
   FLOAT_T tmp;
   FLOAT_T w;
   FLOAT_T h;

   w = params->eval.w ? params->eval.w : FROM_INT(99999);
   h = params->eval.h ? params->eval.h : 99999;

   switch (aspect)
     {
      case EDJE_ASPECT_PREFER_NONE:
         break;
      case EDJE_ASPECT_PREFER_VERTICAL:
         tmp = DIV(SCALE(params->eval.w, minh), h);
         if (tmp >= FROM_INT(minw))
           {
              minw = TO_INT(tmp);
              break;
           }
      case EDJE_ASPECT_PREFER_HORIZONTAL:
         tmp = DIV(SCALE(params->eval.h, minw), w);
         if (tmp >= FROM_INT(minh))
           {
              minh = TO_INT(tmp);
              break;
           }
      case EDJE_ASPECT_PREFER_SOURCE:
      case EDJE_ASPECT_PREFER_BOTH:
         tmp = DIV(SCALE(params->eval.w, minh), h);
         if (tmp >= FROM_INT(minw))
           {
              minw = TO_INT(tmp);
              break;
           }

         tmp = DIV(SCALE(params->eval.h, minw), w);
         if (tmp >= FROM_INT(minh))
           {
              minh = TO_INT(tmp);
              break;
           }

         break;
     }

   _edje_part_recalc_single_min_length(desc->align.x, &params->eval.x, &params->eval.w, minw);
   _edje_part_recalc_single_min_length(desc->align.y, &params->eval.y, &params->eval.h, minh);
}

static void
_edje_part_recalc_single_max_length(FLOAT_T align, FLOAT_T *start, FLOAT_T *length, int max)
{
   if (max >= 0)
     {
        if (*length > max)
          {
             *start  = ADD(*start, SCALE(align, (*length - max)));
             *length = max;
          }
     }
}

static void
_edje_part_recalc_single_max(Edje_Part_Description_Common *desc,
                             Edje_Calc_Params *params,
                             int maxw, int maxh,
                             Edje_Internal_Aspect aspect)
{
   FLOAT_T tmp;
   FLOAT_T w;
   FLOAT_T h;

   w = params->eval.w ? params->eval.w : FROM_INT(99999);
   h = params->eval.h ? params->eval.h : FROM_INT(99999);

   switch (aspect)
     {
      case EDJE_ASPECT_PREFER_NONE:
         break;
      case EDJE_ASPECT_PREFER_VERTICAL:
         tmp = DIV(SCALE(params->eval.w, maxh), h);
         if (tmp <= FROM_INT(maxw))
           {
              maxw = TO_INT(tmp);
              break;
           }
      case EDJE_ASPECT_PREFER_HORIZONTAL:
         tmp = DIV(SCALE(params->eval.h, maxw), w);
         if (tmp <= FROM_INT(maxh))
           {
              maxh = TO_INT(tmp);
              break;
           }
      case EDJE_ASPECT_PREFER_SOURCE:
      case EDJE_ASPECT_PREFER_BOTH:
         tmp = DIV(SCALE(params->eval.w, maxh), h);
         if (tmp <= FROM_INT(maxw))
           {
              maxw = TO_INT(tmp);
              break;
           }

         tmp = DIV(SCALE(params->eval.h, maxw), w);
         if (tmp <= FROM_INT(maxh))
           {
              maxh = TO_INT(tmp);
              break;
           }

         break;
     }

   _edje_part_recalc_single_max_length(desc->align.x, &params->eval.x, &params->eval.w, maxw);
   _edje_part_recalc_single_max_length(desc->align.y, &params->eval.y, &params->eval.h, maxh);
}

static void
_edje_part_recalc_single_drag_threshold(Edje_Real_Part *ep,
					Edje_Real_Part *threshold,
                                        Edje_Calc_Params *params)
{
   if (threshold)
     {
        if (ep->drag->threshold_started_x &&
            threshold->x < TO_INT(params->eval.x) &&
            TO_INT(params->eval.x) + TO_INT(params->eval.w) < threshold->x + threshold->w)
          {
             // Cancel movement to previous position due to our presence inside the threshold
             params->eval.x = FROM_INT(params->req_drag.x);
             params->eval.w = FROM_INT(params->req_drag.w);
	     ep->drag->threshold_x = EINA_TRUE;
          }
        else
          {
             params->req_drag.x = TO_INT(params->eval.x);
             params->req_drag.w = TO_INT(params->eval.w);
             ep->drag->threshold_started_x = EINA_FALSE;
          }
        if (ep->drag->threshold_started_y &&
            threshold->y < TO_INT(params->eval.y) &&
            TO_INT(params->eval.y) + TO_INT(params->eval.h) < threshold->y + threshold->h)
          {
             // Cancel movement to previous position due to our presence inside the threshold
             params->eval.y = FROM_INT(params->req_drag.y);
             params->eval.h = FROM_INT(params->req_drag.h);
	     ep->drag->threshold_y = EINA_TRUE;
          }
        else
          {
             params->req_drag.y = TO_INT(params->eval.y);
             params->req_drag.h = TO_INT(params->eval.h);
             ep->drag->threshold_started_y = EINA_FALSE;
          }
     }
   else
     {
        params->req_drag.x = TO_INT(params->eval.x);
        params->req_drag.w = TO_INT(params->eval.w);
        params->req_drag.y = TO_INT(params->eval.y);
        params->req_drag.h = TO_INT(params->eval.h);
     }
}

static void
_edje_part_recalc_single_drag(Edje_Real_Part *ep,
                              Edje_Real_Part *confine_to,
                              Edje_Real_Part *threshold,
                              Edje_Calc_Params *params,
                              int minw, int minh,
                              int maxw, int maxh)
{
   /* confine */
   if (confine_to)
     {
        int offset;
        int step;
        FLOAT_T v;

        /* complex dragable params */
        v = SCALE(ep->drag->size.x, confine_to->w);

        if ((minw > 0) && (TO_INT(v) < minw)) params->eval.w = FROM_INT(minw);
        else if ((maxw >= 0) && (TO_INT(v) > maxw)) params->eval.w = FROM_INT(maxw);
        else params->eval.w = v;

        offset = TO_INT(SCALE(ep->drag->x, (confine_to->w - TO_INT(params->eval.w))))
           + ep->drag->tmp.x;
        if (ep->part->dragable.step_x > 0)
          {
             params->eval.x = FROM_INT(confine_to->x +
                                       ((offset / ep->part->dragable.step_x) * ep->part->dragable.step_x));
          }
        else if (ep->part->dragable.count_x > 0)
          {
             step = (confine_to->w - TO_INT(params->eval.w)) / ep->part->dragable.count_x;
             if (step < 1) step = 1;
             params->eval.x = FROM_INT(confine_to->x +
                                       ((offset / step) * step));
          }

        v = SCALE(ep->drag->size.y, confine_to->h);

        if ((minh > 0) && (TO_INT(v) < minh)) params->eval.h = FROM_INT(minh);
        else if ((maxh >= 0) && (TO_INT(v) > maxh)) params->eval.h = FROM_INT(maxh);
        else params->eval.h = v;

        offset = TO_INT(SCALE(ep->drag->y, (confine_to->h - TO_INT(params->eval.h))))
           + ep->drag->tmp.y;
        if (ep->part->dragable.step_y > 0)
          {
             params->eval.y = FROM_INT(confine_to->y +
                                       ((offset / ep->part->dragable.step_y) * ep->part->dragable.step_y));
          }
        else if (ep->part->dragable.count_y > 0)
          {
             step = (confine_to->h - TO_INT(params->eval.h)) / ep->part->dragable.count_y;
             if (step < 1) step = 1;
             params->eval.y = FROM_INT(confine_to->y +
                                       ((offset / step) * step));
          }

        _edje_part_recalc_single_drag_threshold(ep, threshold, params);

        /* limit to confine */
        if (params->eval.x < FROM_INT(confine_to->x))
          {
             params->eval.x = FROM_INT(confine_to->x);
          }
        if ((ADD(params->eval.x, params->eval.w)) > FROM_INT(confine_to->x + confine_to->w))
          {
             params->eval.x = SUB(FROM_INT(confine_to->x + confine_to->w), params->eval.w);
          }
        if (params->eval.y < FROM_INT(confine_to->y))
          {
             params->eval.y = FROM_INT(confine_to->y);
          }
        if ((ADD(params->eval.y, params->eval.h)) > FROM_INT(confine_to->y + confine_to->h))
          {
             params->eval.y = SUB(FROM_INT(confine_to->y + confine_to->h), params->eval.h);
          }
     }
   else
     {
        /* simple dragable params */
        params->eval.x = ADD(ADD(params->eval.x, ep->drag->x), FROM_INT(ep->drag->tmp.x));
        params->eval.y = ADD(ADD(params->eval.y, ep->drag->y), FROM_INT(ep->drag->tmp.y));

        _edje_part_recalc_single_drag_threshold(ep, threshold, params);
     }
}

static void
_edje_part_recalc_single_fill(Edje_Real_Part *ep,
                              Edje_Part_Description_Spec_Fill *fill,
                              Edje_Calc_Params *params)
{
   int fw;
   int fh;

   params->smooth = fill->smooth;

   if (fill->type == EDJE_FILL_TYPE_TILE)
     evas_object_image_size_get(ep->object, &fw, NULL);
   else
     fw = params->final.w;

   params->type.common.fill.x = fill->pos_abs_x
      + TO_INT(SCALE(fill->pos_rel_x, fw));
   params->type.common.fill.w = fill->abs_x
      + TO_INT(SCALE(fill->rel_x, fw));

   if (fill->type == EDJE_FILL_TYPE_TILE)
     evas_object_image_size_get(ep->object, NULL, &fh);
   else
     fh = params->final.h;

   params->type.common.fill.y = fill->pos_abs_y
      + TO_INT(SCALE(fill->pos_rel_y, fh));
   params->type.common.fill.h = fill->abs_y
      + TO_INT(SCALE(fill->rel_y, fh));

   params->type.common.fill.angle = fill->angle;
   params->type.common.fill.spread = fill->spread;
}

static void
_edje_part_recalc_single_min_max(FLOAT_T sc,
				 Edje *ed,
                                 Edje_Real_Part *ep,
                                 Edje_Part_Description_Common *desc,
                                 int *minw, int *minh,
                                 int *maxw, int *maxh)
{
   *minw = desc->min.w;
   if (ep->part->scale) *minw = TO_INT(SCALE(sc, *minw));
   if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
       (ep->typedata.swallow))
     {
        if (ep->typedata.swallow->swallow_params.min.w > desc->min.w)
          *minw = ep->typedata.swallow->swallow_params.min.w;
     }

   if (ed->calc_only)
     {
        if (desc->minmul.have)
          {
             FLOAT_T mmw = desc->minmul.w;
             if (mmw != FROM_INT(1)) *minw = TO_INT(SCALE(mmw, *minw));
          }
     }

   if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
       (ep->typedata.swallow))
     {
        /* XXX TODO: remove need of EDJE_INF_MAX_W, see edje_util.c */
        if ((ep->typedata.swallow->swallow_params.max.w <= 0) ||
            (ep->typedata.swallow->swallow_params.max.w == EDJE_INF_MAX_W))
          {
             *maxw = desc->max.w;
             if (*maxw > 0)
               {
                  if (ep->part->scale) *maxw = TO_INT(SCALE(sc, *maxw));
                  if (*maxw < 1) *maxw = 1;
               }
          }
        else
          {
             if (desc->max.w <= 0)
               *maxw = ep->typedata.swallow->swallow_params.max.w;
             else
               {
                  *maxw = desc->max.w;
                  if (*maxw > 0)
                    {
                       if (ep->part->scale) *maxw = TO_INT(SCALE(sc, *maxw));
                       if (*maxw < 1) *maxw = 1;
                    }
                  if (ep->typedata.swallow->swallow_params.max.w < *maxw)
                    *maxw = ep->typedata.swallow->swallow_params.max.w;
               }
          }
     }
   else
     {
        *maxw = desc->max.w;
        if (*maxw > 0)
          {
             if (ep->part->scale) *maxw = TO_INT(SCALE(sc, *maxw));
             if (*maxw < 1) *maxw = 1;
          }
     }
   if ((ed->calc_only) && (desc->minmul.have) &&
       (desc->minmul.w != FROM_INT(1))) *maxw = *minw;
   if (*maxw >= 0)
     {
        if (*maxw < *minw) *maxw = *minw;
     }

   *minh = desc->min.h;
   if (ep->part->scale) *minh = TO_INT(SCALE(sc, *minh));
   if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
       (ep->typedata.swallow))
     {
        if (ep->typedata.swallow->swallow_params.min.h > desc->min.h)
          *minh = ep->typedata.swallow->swallow_params.min.h;
     }

   if (ed->calc_only)
     {
        if (desc->minmul.have)
          {
             FLOAT_T mmh = desc->minmul.h;
             if (mmh != FROM_INT(1)) *minh = TO_INT(SCALE(mmh, *minh));
          }
     }

   if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
       (ep->typedata.swallow))
     {
        /* XXX TODO: remove need of EDJE_INF_MAX_H, see edje_util.c */
        if ((ep->typedata.swallow->swallow_params.max.h <= 0) ||
            (ep->typedata.swallow->swallow_params.max.h == EDJE_INF_MAX_H))
          {
             *maxh = desc->max.h;
             if (*maxh > 0)
               {
                  if (ep->part->scale) *maxh = TO_INT(SCALE(sc, *maxh));
                  if (*maxh < 1) *maxh = 1;
               }
          }
        else
          {
             if (desc->max.h <= 0)
               *maxh = ep->typedata.swallow->swallow_params.max.h;
             else
               {
                  *maxh = desc->max.h;
                  if (*maxh > 0)
                    {
                       if (ep->part->scale) *maxh = TO_INT(SCALE(sc, *maxh));
                       if (*maxh < 1) *maxh = 1;
                    }
                  if (ep->typedata.swallow->swallow_params.max.h < *maxh)
                    *maxh = ep->typedata.swallow->swallow_params.max.h;
               }
          }
     }
   else
     {
        *maxh = desc->max.h;
        if (*maxh > 0)
          {
             if (ep->part->scale) *maxh = TO_INT(SCALE(sc, *maxh));
             if (*maxh < 1) *maxh = 1;
          }
     }
   if ((ed->calc_only) && (desc->minmul.have) &&
       (desc->minmul.h != FROM_INT(1))) *maxh = *minh;
   if (*maxh >= 0)
     {
        if (*maxh < *minh) *maxh = *minh;
     }
}

static void
_edje_part_recalc_single_map(Edje *ed,
                             Edje_Real_Part *ep EINA_UNUSED,
                             Edje_Real_Part *center,
                             Edje_Real_Part *light,
                             Edje_Real_Part *persp,
                             Edje_Part_Description_Common *desc,
                             Edje_Part_Description_Common *chosen_desc,
                             Edje_Calc_Params *params)
{
   params->mapped = chosen_desc->map.on;
   params->lighted = params->mapped ? !!light : 0;
   params->persp_on = params->mapped ? !!persp : 0;

   if (!params->mapped) return;

   EINA_COW_CALC_MAP_BEGIN(params, params_write)
     {
        if (center)
          {
             params_write->center.x = ed->x + center->x + (center->w / 2);
             params_write->center.y = ed->y + center->y + (center->h / 2);
          }
        else
          {
             params_write->center.x = ed->x + params->final.x + (params->final.w / 2);
             params_write->center.y = ed->y + params->final.y + (params->final.h / 2);
          }
        params_write->center.z = 0;

        params_write->rotation.x = desc->map.rot.x;
        params_write->rotation.y = desc->map.rot.y;
        params_write->rotation.z = desc->map.rot.z;

        if (light)
          {
             Edje_Part_Description_Common *light_desc2;
             FLOAT_T pos, pos2;

             params_write->light.x = ed->x + light->x + (light->w / 2);
             params_write->light.y = ed->y + light->y + (light->h / 2);

             pos = light->description_pos;
             pos2 = (pos < ZERO) ? ZERO : ((pos > FROM_INT(1)) ? FROM_INT(1) : pos);

             light_desc2 = light->param2 ? light->param2->description : NULL;

             /* take into account CURRENT state also */
             if (pos != ZERO && light_desc2)
               {
                  params_write->light.z = light->param1.description->persp.zplane +
                    TO_INT(SCALE(pos, light_desc2->persp.zplane - light->param1.description->persp.zplane));
                  params_write->light.r = light->param1.description->color.r +
                    TO_INT(SCALE(pos2, light_desc2->color.r - light->param1.description->color.r));
                  params_write->light.g = light->param1.description->color.g +
                    TO_INT(SCALE(pos2, light_desc2->color.g - light->param1.description->color.g));
                  params_write->light.b = light->param1.description->color.b +
                    TO_INT(SCALE(pos2, light_desc2->color.b - light->param1.description->color.b));
                  params_write->light.ar = light->param1.description->color2.r +
                    TO_INT(SCALE(pos2, light_desc2->color2.r - light->param1.description->color2.r));
                  params_write->light.ag = light->param1.description->color2.g +
                    TO_INT(SCALE(pos2, light_desc2->color2.g - light->param1.description->color2.g));
                  params_write->light.ab = light->param1.description->color2.b +
                    TO_INT(SCALE(pos2, light_desc2->color2.b - light->param1.description->color2.b));
               }
             else
               {
                  params_write->light.z = light->param1.description->persp.zplane;
                  params_write->light.r = light->param1.description->color.r;
                  params_write->light.g = light->param1.description->color.g;
                  params_write->light.b = light->param1.description->color.b;
                  params_write->light.ar = light->param1.description->color2.r;
                  params_write->light.ag = light->param1.description->color2.g;
                  params_write->light.ab = light->param1.description->color2.b;
               }
          }

        if (persp)
          {
             FLOAT_T pos;

             params_write->persp.x = ed->x + persp->x + (persp->w / 2);
             params_write->persp.y = ed->y + persp->y + (persp->h / 2);

             pos = persp->description_pos;

             if (pos != 0 && persp->param2)
               {
                  params_write->persp.z = persp->param1.description->persp.zplane +
                    TO_INT(SCALE(pos, persp->param2->description->persp.zplane -
                                 persp->param1.description->persp.zplane));
                  params_write->persp.focal = persp->param1.description->persp.focal +
                    TO_INT(SCALE(pos, persp->param2->description->persp.focal -
                                 persp->param1.description->persp.focal));
               }
             else
               {
                  params_write->persp.z = persp->param1.description->persp.zplane;
                  params_write->persp.focal = persp->param1.description->persp.focal;
               }
          }
        params_write->colors = desc->map.colors;
        params_write->colors_count = desc->map.colors_count;
     }
   EINA_COW_CALC_MAP_END(params, params_write);
}

static void
_edje_part_recalc_single(Edje *ed,
                         Edje_Real_Part *ep,
                         Edje_Part_Description_Common *desc,
                         Edje_Part_Description_Common *chosen_desc,
                         Edje_Real_Part *center,
                         Edje_Real_Part *light,
                         Edje_Real_Part *persp,
                         Edje_Real_Part *rel1_to_x,
                         Edje_Real_Part *rel1_to_y,
                         Edje_Real_Part *rel2_to_x,
                         Edje_Real_Part *rel2_to_y,
                         Edje_Real_Part *confine_to,
                         Edje_Real_Part *threshold,
                         Edje_Calc_Params *params,
                         Edje_Real_Part_Set *set,
                         Evas_Coord mmw, Evas_Coord mmh,
                         FLOAT_T pos)
{
   Edje_Color_Class *cc = NULL;
   Edje_Internal_Aspect apref;
   int minw = 0, minh = 0, maxw = 0, maxh = 0;
   FLOAT_T sc;

   sc = DIV(ed->scale, ed->file->base_scale);
   if (sc == ZERO) sc = DIV(_edje_scale, ed->file->base_scale);
   _edje_part_recalc_single_min_max(sc, ed, ep, desc, &minw, &minh, &maxw, &maxh);
   if (minw < mmw) minw = mmw;
   if (minh < mmh) minh = mmh;

   /* relative coords of top left & bottom right */
   _edje_part_recalc_single_rel(ed, ep, desc, rel1_to_x, rel1_to_y, rel2_to_x, rel2_to_y, params);

   /* aspect */
   apref = _edje_part_recalc_single_aspect(ed, ep, desc, params, &minw, &minh, &maxw, &maxh, pos);

   /* size step */
   _edje_part_recalc_single_step(desc, params);

   /* colors */
   if (ep->part->type != EDJE_PART_TYPE_SPACER)
     {
        if ((desc->color_class) && (*desc->color_class))
          cc = _edje_color_class_find(ed, desc->color_class);

        if (cc)
          {
             params->color.r = (((int)cc->r + 1) * desc->color.r) >> 8;
             params->color.g = (((int)cc->g + 1) * desc->color.g) >> 8;
             params->color.b = (((int)cc->b + 1) * desc->color.b) >> 8;
             params->color.a = (((int)cc->a + 1) * desc->color.a) >> 8;
          }
        else
          {
             params->color.r = desc->color.r;
             params->color.g = desc->color.g;
             params->color.b = desc->color.b;
             params->color.a = desc->color.a;
          }
     }

   /* visible */
   params->visible = desc->visible;

   /* set parameters, some are required for recalc_single_text[block] */
   switch (ep->part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
           {
              Edje_Part_Description_Image *img_desc = (Edje_Part_Description_Image*) desc;

              /* border */
              params->type.common.spec.image.l = img_desc->image.border.l;
              params->type.common.spec.image.r = img_desc->image.border.r;

              params->type.common.spec.image.t = img_desc->image.border.t;
              params->type.common.spec.image.b = img_desc->image.border.b;

              params->type.common.spec.image.border_scale_by = img_desc->image.border.scale_by;

              if (set && set->set)
                {
#define SET_BORDER_DEFINED(Result, Value) Result = Value ? Value : Result;
                   SET_BORDER_DEFINED(params->type.common.spec.image.l, set->entry->border.l);
                   SET_BORDER_DEFINED(params->type.common.spec.image.r, set->entry->border.r);
                   SET_BORDER_DEFINED(params->type.common.spec.image.t, set->entry->border.t);
                   SET_BORDER_DEFINED(params->type.common.spec.image.b, set->entry->border.b);

                   SET_BORDER_DEFINED(params->type.common.spec.image.border_scale_by, set->entry->border.scale_by);
                }

              break;
           }
      case EDJE_PART_TYPE_TEXT:
      case EDJE_PART_TYPE_TEXTBLOCK:
           {
              Edje_Part_Description_Text *text_desc = (Edje_Part_Description_Text*) desc;

              /* text.align */
              params->type.text.align.x = text_desc->text.align.x;
              params->type.text.align.y = text_desc->text.align.y;
              params->type.text.ellipsis = text_desc->text.ellipsis;

              /* text colors */
              if (cc)
                {
                   params->type.text.color2.r = (((int)cc->r2 + 1) * text_desc->common.color2.r) >> 8;
                   params->type.text.color2.g = (((int)cc->g2 + 1) * text_desc->common.color2.g) >> 8;
                   params->type.text.color2.b = (((int)cc->b2 + 1) * text_desc->common.color2.b) >> 8;
                   params->type.text.color2.a = (((int)cc->a2 + 1) * text_desc->common.color2.a) >> 8;
                   params->type.text.color3.r = (((int)cc->r3 + 1) * text_desc->text.color3.r) >> 8;
                   params->type.text.color3.g = (((int)cc->g3 + 1) * text_desc->text.color3.g) >> 8;
                   params->type.text.color3.b = (((int)cc->b3 + 1) * text_desc->text.color3.b) >> 8;
                   params->type.text.color3.a = (((int)cc->a3 + 1) * text_desc->text.color3.a) >> 8;
                }
              else
                {
                   params->type.text.color2.r = text_desc->common.color2.r;
                   params->type.text.color2.g = text_desc->common.color2.g;
                   params->type.text.color2.b = text_desc->common.color2.b;
                   params->type.text.color2.a = text_desc->common.color2.a;
                   params->type.text.color3.r = text_desc->text.color3.r;
                   params->type.text.color3.g = text_desc->text.color3.g;
                   params->type.text.color3.b = text_desc->text.color3.b;
                   params->type.text.color3.a = text_desc->text.color3.a;
                }

              break;
           }
      case EDJE_PART_TYPE_SPACER:
      case EDJE_PART_TYPE_RECTANGLE:
      case EDJE_PART_TYPE_BOX:
      case EDJE_PART_TYPE_TABLE:
      case EDJE_PART_TYPE_SWALLOW:
      case EDJE_PART_TYPE_GROUP:
      case EDJE_PART_TYPE_PROXY:
         break;
      case EDJE_PART_TYPE_GRADIENT:
         /* FIXME: THIS ONE SHOULD NEVER BE TRIGGERED. */
         break;
      default:
         break;
     }

   /* if we have text that wants to make the min size the text size... */
   if (ep->part->type == EDJE_PART_TYPE_TEXTBLOCK)
     _edje_part_recalc_single_textblock(sc, ed, ep, (Edje_Part_Description_Text*) chosen_desc, params, &minw, &minh, &maxw, &maxh);
   else if (ep->part->type == EDJE_PART_TYPE_TEXT)
     _edje_part_recalc_single_text(sc, ed, ep, (Edje_Part_Description_Text*) desc, (Edje_Part_Description_Text*) chosen_desc, params, &minw, &minh, &maxw, &maxh);

   if ((ep->part->type == EDJE_PART_TYPE_TABLE) &&
       (((((Edje_Part_Description_Table *)chosen_desc)->table.min.h) ||
         (((Edje_Part_Description_Table *)chosen_desc)->table.min.v))))
     {
        Evas_Coord lminw = 0, lminh = 0;

	eo_do(ep->object,
	      evas_obj_smart_need_recalculate_set(1),
	      evas_obj_smart_calculate(),
	      evas_obj_size_hint_min_get(&lminw, &lminh));
        if (((Edje_Part_Description_Table *)chosen_desc)->table.min.h)
          {
             if (lminw > minw) minw = lminw;
          }
        if (((Edje_Part_Description_Table *)chosen_desc)->table.min.v)
          {
             if (lminh > minh) minh = lminh;
          }
     }
   else if ((ep->part->type == EDJE_PART_TYPE_BOX) &&
            ((((Edje_Part_Description_Box *)chosen_desc)->box.min.h) ||
             (((Edje_Part_Description_Box *)chosen_desc)->box.min.v)))
     {
        Evas_Coord lminw = 0, lminh = 0;

	eo_do(ep->object,
	      evas_obj_smart_need_recalculate_set(1),
	      evas_obj_smart_calculate(),
	      evas_obj_size_hint_min_get(&lminw, &lminh));
        if (((Edje_Part_Description_Box *)chosen_desc)->box.min.h)
          {
             if (lminw > minw) minw = lminw;
          }
        if (((Edje_Part_Description_Box *)chosen_desc)->box.min.v)
          {
             if (lminh > minh) minh = lminh;
          }
     }
   else if ((ep->part->type == EDJE_PART_TYPE_IMAGE) &&
            (chosen_desc->min.limit || chosen_desc->max.limit))
     {
        Evas_Coord w, h;

        /* We only need pos to find the right image that would be displayed */
        /* Yes, if someone set aspect preference to SOURCE and also max,min
           to SOURCE, it will be under efficient, but who cares at the
           moment. */
        _edje_real_part_image_set(ed, ep, pos);
        evas_object_image_size_get(ep->object, &w, &h);

        if (chosen_desc->min.limit)
          {
             if (w > minw) minw = w;
             if (h > minh) minh = h;
          }
        if (chosen_desc->max.limit)
          {
             if ((maxw <= 0) || (w < maxw)) maxw = w;
             if ((maxh <= 0) || (h < maxh)) maxh = h;
          }
     }

   /* remember what our size is BEFORE we go limit it */
   params->req.x = TO_INT(params->eval.x);
   params->req.y = TO_INT(params->eval.y);
   params->req.w = TO_INT(params->eval.w);
   params->req.h = TO_INT(params->eval.h);

   /* adjust for min size */
   _edje_part_recalc_single_min(desc, params, minw, minh, apref);

   /* adjust for max size */
   _edje_part_recalc_single_max(desc, params, maxw, maxh, apref);

   /* take care of dragable part */
   if (ep->drag)
     _edje_part_recalc_single_drag(ep, confine_to, threshold, params, minw, minh, maxw, maxh);

   /* Update final size after last change to its position */
   params->final.x = TO_INT(params->eval.x);
   params->final.y = TO_INT(params->eval.y);
   params->final.w = TO_INT(params->eval.w);
   params->final.h = TO_INT(params->eval.h);

   /* Adjust rounding to not loose one pixels compared to float
      information only when rendering to avoid infinite adjustement
      when doing min restricted calc */
   if (ABS(params->final.x) + params->final.w < TO_INT(ADD(ABS(params->eval.x), params->eval.w)))
     {
        if (!ed->calc_only)
          {
             params->final.w += 1;
          }
        else
          {
             ep->invalidate = EINA_TRUE;
          }
     }
   else if (ABS(params->final.x) + params->final.w > TO_INT(ADD(ABS(params->eval.x), params->eval.w)))
     {
        if (!ed->calc_only)
          {
             params->final.w -= 1;
          }
        else
          {
             ep->invalidate = EINA_TRUE;
          }
     }
   if (ABS(params->final.y) + params->final.h < TO_INT(ADD(ABS(params->eval.y), params->eval.h)))
     {
        if (!ed->calc_only)
          {
             params->final.h += 1;
          }
        else
          {
             ep->invalidate = EINA_TRUE;
          }
     }
   else if (ABS(params->final.y) + params->final.h > TO_INT(ADD(ABS(params->eval.y), params->eval.h)))
     {
        if (!ed->calc_only)
          {
             params->final.h -= 1;
          }
        else
          {
             ep->invalidate = EINA_TRUE;
          }
     }

   /* fill */
   if (ep->part->type == EDJE_PART_TYPE_IMAGE)
     _edje_part_recalc_single_fill(ep, &((Edje_Part_Description_Image *)desc)->image.fill, params);
   else if (ep->part->type == EDJE_PART_TYPE_PROXY)
     _edje_part_recalc_single_fill(ep, &((Edje_Part_Description_Proxy *)desc)->proxy.fill, params);

#ifdef HAVE_EPHYSICS
   if (ep->part->physics_body || ep->body)
     {
        EINA_COW_CALC_PHYSICS_BEGIN(params, params_write)
          {
             params_write->mass = desc->physics.mass;
             params_write->restitution = desc->physics.restitution;
             params_write->friction = desc->physics.friction;
             params_write->damping.linear = desc->physics.damping.linear;
             params_write->damping.angular = desc->physics.damping.angular;
             params_write->sleep.linear = desc->physics.sleep.linear;
             params_write->sleep.angular = desc->physics.sleep.angular;
             params_write->material = desc->physics.material;
             params_write->density = desc->physics.density;
             params_write->hardness = desc->physics.hardness;
             params_write->ignore_part_pos = desc->physics.ignore_part_pos;
             params_write->light_on = desc->physics.light_on;
             params_write->mov_freedom.lin.x = desc->physics.mov_freedom.lin.x;
             params_write->mov_freedom.lin.y = desc->physics.mov_freedom.lin.y;
             params_write->mov_freedom.lin.z = desc->physics.mov_freedom.lin.z;
             params_write->mov_freedom.ang.x = desc->physics.mov_freedom.ang.x;
             params_write->mov_freedom.ang.y = desc->physics.mov_freedom.ang.y;
             params_write->mov_freedom.ang.z = desc->physics.mov_freedom.ang.z;
             params_write->backcull = desc->physics.backcull;
             params_write->z = desc->physics.z;
             params_write->depth = desc->physics.depth;
          }
        EINA_COW_CALC_PHYSICS_END(params, params_write);
     }
#endif
   _edje_part_recalc_single_map(ed, ep, center, light, persp, desc, chosen_desc, params);
}

static void
_edje_table_recalc_apply(Edje *ed EINA_UNUSED,
                         Edje_Real_Part *ep,
                         Edje_Calc_Params *p3 EINA_UNUSED,
                         Edje_Part_Description_Table *chosen_desc)
{
   eo_do(ep->object,
	 evas_obj_table_homogeneous_set(chosen_desc->table.homogeneous),
	 evas_obj_table_align_set(TO_DOUBLE(chosen_desc->table.align.x), TO_DOUBLE(chosen_desc->table.align.y)),
	 evas_obj_table_padding_set(chosen_desc->table.padding.x, chosen_desc->table.padding.y));
   if (evas_object_smart_need_recalculate_get(ep->object))
     {
        eo_do(ep->object,
	      evas_obj_smart_need_recalculate_set(0),
	      evas_obj_smart_calculate());
     }
}

static void
_edje_proxy_recalc_apply(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *p3, Edje_Part_Description_Proxy *chosen_desc, FLOAT_T pos)
{
   Edje_Real_Part *pp;
   int part_id = -1;

   if (ep->param2 && (pos >= FROM_DOUBLE(0.5)))
     part_id = ((Edje_Part_Description_Proxy*) ep->param2->description)->proxy.id;
   else
     part_id = chosen_desc->proxy.id;

   if ((p3->type.common.fill.w == 0) || (p3->type.common.fill.h == 0) ||
       (part_id < 0))
     {
        evas_object_image_source_set(ep->object, NULL);
        return;
     }
   pp = ed->table_parts[part_id % ed->table_parts_size];

   if (pp->nested_smart)  /* using nested_smart for nested parts */
     {
        evas_object_image_source_set(ep->object, pp->nested_smart);
     }
   else
     {
        switch (pp->part->type)
          {
           case EDJE_PART_TYPE_IMAGE:
           case EDJE_PART_TYPE_TEXT:
           case EDJE_PART_TYPE_TEXTBLOCK:
           case EDJE_PART_TYPE_RECTANGLE:
           case EDJE_PART_TYPE_BOX:
           case EDJE_PART_TYPE_TABLE:
           case EDJE_PART_TYPE_PROXY:
              evas_object_image_source_set(ep->object, pp->object);
              break;
           case EDJE_PART_TYPE_GRADIENT:
              /* FIXME: THIS ONE SHOULD NEVER BE TRIGGERED. */
              break;
           case EDJE_PART_TYPE_GROUP:
           case EDJE_PART_TYPE_SWALLOW:
           case EDJE_PART_TYPE_EXTERNAL:
             if ((pp->type == EDJE_RP_TYPE_SWALLOW) &&
                 (pp->typedata.swallow))
               {
                  evas_object_image_source_set(ep->object, pp->typedata.swallow->swallowed_object);
               }
              break;
           case EDJE_PART_TYPE_SPACER:
              /* FIXME: detect that at compile time and prevent it */
              break;
          }
     }

   eo_do(ep->object,
         evas_obj_image_fill_set(p3->type.common.fill.x,
                                 p3->type.common.fill.y,
                                 p3->type.common.fill.w,
                                 p3->type.common.fill.h),
         efl_image_smooth_scale_set(p3->smooth),
         evas_obj_image_source_visible_set(chosen_desc->proxy.source_visible),
         evas_obj_image_source_clip_set(chosen_desc->proxy.source_clip));
}

static void
_edje_image_recalc_apply(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *p3, Edje_Part_Description_Image *chosen_desc, FLOAT_T pos)
{
   FLOAT_T sc;

   sc = DIV(ed->scale, ed->file->base_scale);
   if (sc == ZERO) sc = DIV(_edje_scale, ed->file->base_scale);
   eo_do(ep->object,
	 evas_obj_image_fill_set(p3->type.common.fill.x, p3->type.common.fill.y,
				 p3->type.common.fill.w, p3->type.common.fill.h),
	 efl_image_smooth_scale_set(p3->smooth));
   if (chosen_desc->image.border.scale)
     {
        if (p3->type.common.spec.image.border_scale_by > FROM_DOUBLE(0.0))
          {
             FLOAT_T sc2 = MUL(sc, p3->type.common.spec.image.border_scale_by);
             evas_object_image_border_scale_set(ep->object, TO_DOUBLE(sc2));
          }
        else
           evas_object_image_border_scale_set(ep->object, TO_DOUBLE(sc));
     }
   else
     {
        if (p3->type.common.spec.image.border_scale_by > FROM_DOUBLE(0.0))
           evas_object_image_border_scale_set
           (ep->object, TO_DOUBLE(p3->type.common.spec.image.border_scale_by));
        else
          evas_object_image_border_scale_set(ep->object, 1.0);
     }
   evas_object_image_border_set(ep->object, p3->type.common.spec.image.l, p3->type.common.spec.image.r,
                                p3->type.common.spec.image.t, p3->type.common.spec.image.b);
   if (chosen_desc->image.border.no_fill == 0)
     evas_object_image_border_center_fill_set(ep->object, EVAS_BORDER_FILL_DEFAULT);
   else if (chosen_desc->image.border.no_fill == 1)
     evas_object_image_border_center_fill_set(ep->object, EVAS_BORDER_FILL_NONE);
   else if (chosen_desc->image.border.no_fill == 2)
     evas_object_image_border_center_fill_set(ep->object, EVAS_BORDER_FILL_SOLID);

   _edje_real_part_image_set(ed, ep, pos);
}

static Edje_Real_Part *
_edje_real_part_state_get(Edje *ed, Edje_Real_Part *ep, int flags, int id, int *state)
{
   Edje_Real_Part *result = NULL;

   if (id >= 0 && id != ep->part->id)
     {
        result = ed->table_parts[id % ed->table_parts_size];
        if (result)
          {
             if (!result->calculated) _edje_part_recalc(ed, result, flags, NULL);
#ifdef EDJE_CALC_CACHE
             if (state) *state = result->state;
#else
             (void) state;
#endif
          }
     }
   return result;
}

#ifdef HAVE_EPHYSICS
static Eina_Bool
_edje_physics_world_geometry_check(EPhysics_World *world)
{
   Evas_Coord w, h, d;
   ephysics_world_render_geometry_get(world, NULL, NULL, NULL, &w, &h, &d);
   return (w && h && d);
}

static void
_edje_physics_body_props_update(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *pf,
				Eina_Bool pos_update)
{
   ephysics_body_linear_movement_enable_set(ep->body,
                                            pf->physics->mov_freedom.lin.x,
                                            pf->physics->mov_freedom.lin.y,
                                            pf->physics->mov_freedom.lin.z);
   ephysics_body_angular_movement_enable_set(ep->body,
                                             pf->physics->mov_freedom.ang.x,
                                             pf->physics->mov_freedom.ang.y,
                                             pf->physics->mov_freedom.ang.z);

   /* Boundaries geometry and mass shouldn't be changed */
   if (ep->part->physics_body < EDJE_PART_PHYSICS_BODY_BOUNDARY_TOP)
     {
        Evas_Coord x, y, z, w, h, d;

        if (pos_update)
          {
             ephysics_body_move(ep->body,
				ed->x + pf->final.x,
                                ed->y + pf->final.y,
				pf->physics->z);
             ep->x = pf->final.x;
             ep->y = pf->final.y;
             ep->w = pf->final.w;
             ep->h = pf->final.h;
          }

        ephysics_body_geometry_get(ep->body, &x, &y, &z, &w, &h, &d);
        if ((d) && (d != pf->physics->depth))
          ephysics_body_resize(ep->body, w, h, pf->physics->depth);
        if (z != pf->physics->z)
          ephysics_body_move(ep->body, x, y, pf->physics->z);

        ephysics_body_material_set(ep->body, pf->physics->material);
        if (!pf->physics->material)
          {
             if (pf->physics->density)
               ephysics_body_density_set(ep->body, pf->physics->density);
             else
               ephysics_body_mass_set(ep->body, pf->physics->mass);
          }

        if ((ep->part->physics_body == EDJE_PART_PHYSICS_BODY_SOFT_BOX) ||
            (ep->part->physics_body == EDJE_PART_PHYSICS_BODY_SOFT_SPHERE) ||
            (ep->part->physics_body == EDJE_PART_PHYSICS_BODY_SOFT_CYLINDER) ||
            (ep->part->physics_body == EDJE_PART_PHYSICS_BODY_CLOTH))
          ephysics_body_soft_body_hardness_set(ep->body,
                                               pf->physics->hardness * 100);
     }

   if (!pf->physics->material)
     {
        ephysics_body_restitution_set(ep->body, pf->physics->restitution);
        ephysics_body_friction_set(ep->body, pf->physics->friction);
     }

   ephysics_body_damping_set(ep->body, pf->physics->damping.linear,
                             pf->physics->damping.angular);
   ephysics_body_sleeping_threshold_set(ep->body, pf->physics->sleep.linear,
                                        pf->physics->sleep.angular);
   ephysics_body_light_set(ep->body, pf->physics->light_on);
   ephysics_body_back_face_culling_set(ep->body, pf->physics->backcull);
}

static void
_edje_physics_body_update_cb(void *data, EPhysics_Body *body, void *event_info EINA_UNUSED)
{
   Edje_Real_Part *rp = data;
   Edje *ed = ephysics_body_data_get(body);

   ephysics_body_geometry_get(body, &(rp->x), &(rp->y), NULL,
                              &(rp->w), &(rp->h), NULL);
   ephysics_body_evas_object_update(body);
   ed->dirty = EINA_TRUE;
}

static void
_edje_physics_body_add(Edje *ed, Edje_Real_Part *rp, EPhysics_World *world)
{
   Eina_Bool resize = EINA_TRUE;
   Edje_Physics_Face *pface;
   Eina_List *l;

   switch (rp->part->physics_body)
     {
      case EDJE_PART_PHYSICS_BODY_RIGID_BOX:
         rp->body = ephysics_body_box_add(world);
         break;
      case EDJE_PART_PHYSICS_BODY_RIGID_SPHERE:
         rp->body = ephysics_body_sphere_add(world);
         break;
      case EDJE_PART_PHYSICS_BODY_RIGID_CYLINDER:
         rp->body = ephysics_body_cylinder_add(world);
         break;
      case EDJE_PART_PHYSICS_BODY_SOFT_BOX:
         rp->body = ephysics_body_soft_box_add(world);
         break;
      case EDJE_PART_PHYSICS_BODY_SOFT_SPHERE:
         rp->body = ephysics_body_soft_sphere_add(world, 0);
         break;
      case EDJE_PART_PHYSICS_BODY_SOFT_CYLINDER:
         rp->body = ephysics_body_soft_cylinder_add(world);
         break;
      case EDJE_PART_PHYSICS_BODY_CLOTH:
         rp->body = ephysics_body_cloth_add(world, 0, 0);
         break;
      case EDJE_PART_PHYSICS_BODY_BOUNDARY_TOP:
         rp->body = ephysics_body_top_boundary_add(world);
         resize = EINA_FALSE;
         break;
      case EDJE_PART_PHYSICS_BODY_BOUNDARY_BOTTOM:
         rp->body = ephysics_body_bottom_boundary_add(world);
         resize = EINA_FALSE;
         break;
      case EDJE_PART_PHYSICS_BODY_BOUNDARY_RIGHT:
         rp->body = ephysics_body_right_boundary_add(world);
         resize = EINA_FALSE;
         break;
      case EDJE_PART_PHYSICS_BODY_BOUNDARY_LEFT:
         rp->body = ephysics_body_left_boundary_add(world);
         resize = EINA_FALSE;
         break;
      case EDJE_PART_PHYSICS_BODY_BOUNDARY_FRONT:
         rp->body = ephysics_body_front_boundary_add(world);
         resize = EINA_FALSE;
         break;
      case EDJE_PART_PHYSICS_BODY_BOUNDARY_BACK:
         rp->body = ephysics_body_back_boundary_add(world);
         resize = EINA_FALSE;
         break;
      default:
         ERR("Invalid body: %i", rp->part->physics_body);
         return;
     }

   EINA_LIST_FOREACH(rp->part->default_desc->physics.faces, l, pface)
     {
        Evas_Object *edje_obj;
        Evas *evas;

        if (!pface->source) continue;

        evas = evas_object_evas_get(rp->object);
        edje_obj = edje_object_add(evas);
        if (!edje_obj) continue;

        edje_object_file_set(edje_obj, ed->path, pface->source);
        evas_object_resize(edje_obj, 1, 1);
        ephysics_body_face_evas_object_set(rp->body, pface->type,
                                           edje_obj, EINA_FALSE);
        rp->body_faces = eina_list_append(rp->body_faces, edje_obj);
     }

   ephysics_body_evas_object_set(rp->body, rp->object, resize);
   ephysics_body_event_callback_add(rp->body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                    _edje_physics_body_update_cb, rp);
   ephysics_body_data_set(rp->body, ed);
}
#endif

#define FINTP(_x1, _x2, _p)                     \
        (((_x1) == (_x2))                       \
         ? FROM_INT((_x1))                      \
         : ADD(FROM_INT(_x1),                   \
               SCALE((_p), (_x2) - (_x1))))

#define FFP(_x1, _x2, _p)                       \
        (((_x1) == (_x2))                       \
         ? (_x1)                                \
         : ADD(_x1, MUL(_p, SUB(_x2, _x1))));

#define INTP(_x1, _x2, _p) TO_INT(FINTP(_x1, _x2, _p))

static void
_map_colors_free(Edje_Calc_Params *pf)
{
   Edje_Map_Color **colors = pf->map->colors;
   int i;

   for (i = 0; i < (int) pf->map->colors_count; i++)
     free(colors[i]);
   free (colors);
}

static Eina_Bool
_map_colors_interp(Edje_Calc_Params *p1, Edje_Calc_Params *p2,
                   Edje_Calc_Params_Map *pmap, FLOAT_T pos)
{
   Edje_Map_Color *col, *col2, *col3;
   int i, j, idx = 0;
   Eina_Bool matched = EINA_FALSE;

   if ((p1->map->colors_count > 0) || (p2->map->colors_count > 0))
     {
        pmap->colors_count = (p1->map->colors_count > p2->map->colors_count ? p1->map->colors_count : p2->map->colors_count);

        pmap->colors = (Edje_Map_Color **) malloc(sizeof(Edje_Map_Color *) * (int) pmap->colors_count);

        for (i = 0; i < (int)p1->map->colors_count; i++)
          {
             col = p1->map->colors[i];
             col3 = malloc(sizeof(Edje_Map_Color));
             col3->idx = col->idx;

             for (j = 0; j < (int)p2->map->colors_count; j++)
               {
                  col2 = p2->map->colors[j];
                  if (col->idx != col2->idx) continue;
                  col3->r = INTP(col->r, col2->r, pos);
                  col3->g = INTP(col->g, col2->g, pos);
                  col3->b = INTP(col->b, col2->b, pos);
                  col3->a = INTP(col->a, col2->a, pos);
                  pmap->colors[idx] = col3;
                  matched = EINA_TRUE;
                  break;
               }
             if (!matched)
               {
                  col3->r = INTP(col->r, 255, pos);
                  col3->g = INTP(col->g, 255, pos);
                  col3->b = INTP(col->b, 255, pos);
                  col3->a = INTP(col->a, 255, pos);
                  pmap->colors[idx] = col3;
               }
             idx++;
             matched = EINA_FALSE;
          }
        for (i = 0; i < (int)p2->map->colors_count; i++)
          {
             col = p2->map->colors[i];

             for (j = 0; j < (int)p1->map->colors_count; j++)
               {
                  col2 = p1->map->colors[j];
                  if (col->idx != col2->idx) continue;
                  matched = EINA_TRUE;
                  break;
               }
             if (!matched)
               {
                  col3 = malloc(sizeof(Edje_Map_Color));
                  col3->idx = col->idx;
                  col3->r = INTP(255, col->r, pos);
                  col3->g = INTP(255, col->g, pos);
                  col3->b = INTP(255, col->b, pos);
                  col3->a = INTP(255, col->a, pos);
                  pmap->colors[idx] = col3;
               }
             idx++;
             matched = EINA_FALSE;
          }
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static void
_edje_map_prop_set(Evas_Map *map, const  Edje_Calc_Params *pf,
                   Edje_Part_Description_Common *chosen_desc,
                   Edje_Real_Part *ep, Evas_Object *mo)
{
   Edje_Map_Color **colors = pf->map->colors;
   int colors_cnt = pf->map->colors_count;
   int i;

   Edje_Map_Color *color;

   evas_map_util_points_populate_from_object(map, ep->object);

   if (ep->part->type == EDJE_PART_TYPE_IMAGE ||
       ((ep->part->type == EDJE_PART_TYPE_SWALLOW) &&
        (eo_isa(mo, EVAS_IMAGE_CLASS) &&
         (!evas_object_image_source_get(mo))))
      )
     {
        int iw = 1, ih = 1;

        evas_object_image_size_get(mo, &iw, &ih);
        evas_map_point_image_uv_set(map, 0, 0.0, 0.0);
        evas_map_point_image_uv_set(map, 1, iw , 0.0);
        evas_map_point_image_uv_set(map, 2, iw , ih );
        evas_map_point_image_uv_set(map, 3, 0.0, ih );
     }

   //map color
   if (colors_cnt == 0)
     {
        evas_map_point_color_set(map, 0, 255, 255, 255, 255);
        evas_map_point_color_set(map, 1, 255, 255, 255, 255);
        evas_map_point_color_set(map, 2, 255, 255, 255, 255);
        evas_map_point_color_set(map, 3, 255, 255, 255, 255);
     }
   else
     {
        for (i = 0; i < colors_cnt; i++)
          {
             color = colors[i];
             evas_map_point_color_set(map, color->idx, color->r, color->g,
                                      color->b, color->a);
          }
     }

   //rotate
   evas_map_util_3d_rotate(map,
                           TO_DOUBLE(pf->map->rotation.x),
                           TO_DOUBLE(pf->map->rotation.y),
                           TO_DOUBLE(pf->map->rotation.z),
                           pf->map->center.x, pf->map->center.y,
                           pf->map->center.z);

   // calculate light color & position etc. if there is one
   if (pf->lighted)
     {
        evas_map_util_3d_lighting(map, pf->map->light.x, pf->map->light.y,
                                  pf->map->light.z, pf->map->light.r,
                                  pf->map->light.g, pf->map->light.b,
                                  pf->map->light.ar, pf->map->light.ag,
                                  pf->map->light.ab);
     }

   // calculate perspective point
   if (chosen_desc->map.persp_on)
     {
        evas_map_util_3d_perspective(map,
                                     pf->map->persp.x, pf->map->persp.y,
                                     pf->map->persp.z, pf->map->persp.focal);
     }

   // handle backface culling (object is facing away from view
   if (chosen_desc->map.backcull)
     {
        if (pf->visible)
          {
             if (evas_map_util_clockwise_get(map))
               evas_object_show(mo);
             else evas_object_hide(mo);
          }
     }

   // handle smooth
   if (chosen_desc->map.smooth) evas_map_smooth_set(map, EINA_TRUE);
   else evas_map_smooth_set(map, EINA_FALSE);
   // handle alpha
   if (chosen_desc->map.alpha) evas_map_alpha_set(map, EINA_TRUE);
   else evas_map_alpha_set(map, EINA_FALSE);
}

#define Rel1X 0
#define Rel1Y 1
#define Rel2X 2
#define Rel2Y 3

void
_edje_part_recalc(Edje *ed, Edje_Real_Part *ep, int flags, Edje_Calc_Params *state)
{
#ifdef EDJE_CALC_CACHE
   Eina_Bool proxy_invalidate = EINA_FALSE;
   int state1 = -1;
   int state2 = -1;
   int statec = -1;
   int statet = -1;
#else
   Edje_Calc_Params lp1, lp2;
#endif
   int statec1 = -1;
   int statec2 = -1;
   int statel1 = -1;
   int statel2 = -1;
   int statep1 = -1;
   int statep2 = -1;
   Edje_Real_Part *center[2] = { NULL, NULL };
   Edje_Real_Part *light[2] = { NULL, NULL };
   Edje_Real_Part *persp[2] = { NULL, NULL };
   Edje_Real_Part *rp1[4] = { NULL, NULL, NULL, NULL };
   Edje_Real_Part *rp2[4] = { NULL, NULL, NULL, NULL };
   Edje_Calc_Params *p1, *pf;
   Edje_Part_Description_Common *chosen_desc;
   Edje_Real_Part *confine_to = NULL;
   Edje_Real_Part *threshold = NULL;
   FLOAT_T pos = ZERO, pos2;
   Edje_Calc_Params lp3;
   Evas_Coord mmw = 0, mmh = 0;
   Eina_Bool map_colors_free = EINA_FALSE;

   /* GRADIENT ARE GONE, WE MUST IGNORE IT FROM OLD FILE. */
   if (ep->part->type == EDJE_PART_TYPE_GRADIENT)
     {
        ERR("GRADIENT spotted during recalc ! That should never happen ! Send your edje file to devel ml.");
        return;
     }

   if ((ep->calculated & FLAG_XY) == FLAG_XY && !state)
     {
        return;
     }
   if (ep->calculating & flags)
     {
#if 1
        const char *axes = "NONE", *faxes = "NONE";

        if ((ep->calculating & FLAG_X) &&
            (ep->calculating & FLAG_Y))
          axes = "XY";
        else if ((ep->calculating & FLAG_X))
          axes = "X";
        else if ((ep->calculating & FLAG_Y))
          axes = "Y";

        if ((flags & FLAG_X) &&
            (flags & FLAG_Y))
          faxes = "XY";
        else if ((flags & FLAG_X))
          faxes = "X";
        else if ((flags & FLAG_Y))
          faxes = "Y";
        ERR("Circular dependency when calculating part \"%s\". "
            "Already calculating %s [%02x] axes. "
            "Need to calculate %s [%02x] axes",
            ep->part->name,
            axes, ep->calculating,
            faxes, flags);
#endif
        return;
     }

   if (ep->part->type == EDJE_PART_TYPE_GROUP &&
       ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
           (ep->typedata.swallow)) &&
       ep->typedata.swallow->swallowed_object)
     {
        edje_object_scale_set(ep->typedata.swallow->swallowed_object, TO_DOUBLE(ed->scale));

        if (ep->description_pos > FROM_DOUBLE(0.5) && ep->param2)
          {
             edje_object_update_hints_set(ep->typedata.swallow->swallowed_object, ep->param2->description->min.limit);
          }
        else
          {
             edje_object_update_hints_set(ep->typedata.swallow->swallowed_object, ep->param1.description->min.limit);
          }
        if (edje_object_update_hints_get(ep->typedata.swallow->swallowed_object))
          {
             Edje *ted;

             ted = _edje_fetch(ep->typedata.swallow->swallowed_object);
             _edje_recalc_do(ted);
          }
     }
   if (ep->part->type == EDJE_PART_TYPE_GROUP &&
       ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
           (ep->typedata.swallow)) &&
       ep->typedata.swallow->swallowed_object)
     {
        edje_object_size_min_calc(ep->typedata.swallow->swallowed_object,
                                  &mmw, &mmh);
     }

#ifdef EDJE_CALC_CACHE
   if (ep->state == ed->state && !state)
     return;
#endif

   if (ep->param1.description) 
     {
       if (ep->param1.description->rel1.id_x >= 0)
         rp1[Rel1X] = ed->table_parts[ep->param1.description->rel1.id_x];
       if (ep->param1.description->rel2.id_x >= 0)
         rp1[Rel2X] = ed->table_parts[ep->param1.description->rel2.id_x];
       if (ep->param1.description->rel1.id_y >= 0)
         rp1[Rel1Y] = ed->table_parts[ep->param1.description->rel1.id_y];
       if (ep->param1.description->rel2.id_y >= 0)
         rp1[Rel2Y] = ed->table_parts[ep->param1.description->rel2.id_y];
     }
   if (ep->param2)
     {
        if (ep->param2->description->rel1.id_x >= 0)
          rp2[Rel1X] = ed->table_parts[ep->param2->description->rel1.id_x];
        if (ep->param2->description->rel2.id_x >= 0)
          rp2[Rel2X] = ed->table_parts[ep->param2->description->rel2.id_x];
        if (ep->param2->description->rel1.id_y >= 0)
          rp2[Rel1Y] = ed->table_parts[ep->param2->description->rel1.id_y];
        if (ep->param2->description->rel2.id_y >= 0)
          rp2[Rel2Y] = ed->table_parts[ep->param2->description->rel2.id_y];
     }

   if (flags & FLAG_X)
     {
        ep->calculating |= flags & FLAG_X;
	if (rp1[Rel1X])
          {
             _edje_part_recalc(ed, rp1[Rel1X], FLAG_X, NULL);
#ifdef EDJE_CALC_CACHE
             state1 = rp1[Rel1X]->state;
#endif
          }
        if (rp1[Rel2X])
          {
             _edje_part_recalc(ed, rp1[Rel2X], FLAG_X, NULL);
#ifdef EDJE_CALC_CACHE
             if (state1 < rp1[Rel2X]->state)
               state1 = rp1[Rel2X]->state;
#endif
          }
        if (ep->param2)
          {
             if (rp2[Rel1X])
               {
                  _edje_part_recalc(ed, rp2[Rel1X], FLAG_X, NULL);
#ifdef EDJE_CALC_CACHE
                  state2 = rp2[Rel1X]->state;
#endif
               }
             if (rp2[Rel2X])
               {
                  _edje_part_recalc(ed, rp2[Rel2X], FLAG_X, NULL);
#ifdef EDJE_CALC_CACHE
                  if (state2 < rp2[Rel2X]->state)
                    state2 = rp2[Rel2X]->state;
#endif
               }
          }
     }
   if (flags & FLAG_Y)
     {
        ep->calculating |= flags & FLAG_Y;
        if (rp1[Rel1Y])
          {
             _edje_part_recalc(ed, rp1[Rel1Y], FLAG_Y, NULL);
#ifdef EDJE_CALC_CACHE
             if (state1 < rp1[Rel1Y]->state)
               state1 = rp1[Rel1Y]->state;
#endif
          }
        if (rp1[Rel2Y])
          {
             _edje_part_recalc(ed, rp1[Rel2Y], FLAG_Y, NULL);
#ifdef EDJE_CALC_CACHE
             if (state1 < rp1[Rel2Y]->state)
               state1 = rp1[Rel2Y]->state;
#endif
          }
        if (ep->param2)
          {
             if (rp2[Rel1Y])
               {
                  _edje_part_recalc(ed, rp2[Rel1Y], FLAG_Y, NULL);
#ifdef EDJE_CALC_CACHE
                  if (state2 < rp2[Rel1Y]->state)
                    state2 = rp2[Rel1Y]->state;
#endif
               }
             if (rp2[Rel2Y])
               {
                  _edje_part_recalc(ed, rp2[Rel2Y], FLAG_Y, NULL);
#ifdef EDJE_CALC_CACHE
                  if (state2 < rp2[Rel2Y]->state)
                    state2 = rp2[Rel2Y]->state;
#endif
               }
          }
     }
   if (ep->drag)
     {
        if (ep->drag->confine_to)
          {
             confine_to = ep->drag->confine_to;
             _edje_part_recalc(ed, confine_to, flags, NULL);
#ifdef EDJE_CALC_CACHE
             statec = confine_to->state;
#endif
          }
        if (ep->drag->threshold)
          {
             threshold = ep->drag->threshold;
             // We shall not recalculate the threshold position as
             // we use it's previous position to assert the threshold
             // the one before moving take action.
#ifdef EDJE_CALC_CACHE
             statet = threshold->state;
#endif
          }
     }
   //   if (ep->text.source)       _edje_part_recalc(ed, ep->text.source, flags);
   //   if (ep->text.text_source)  _edje_part_recalc(ed, ep->text.text_source, flags);

   /* actually calculate now */
   chosen_desc = ep->chosen_description;
   if (!chosen_desc)
     {
        ep->calculating = FLAG_NONE;
        ep->calculated |= flags;
        return;
     }

   pos = ep->description_pos;

   if (ep->part->type == EDJE_PART_TYPE_PROXY)
     {
        int part_id = -1;

        if (ep->param2 && (pos >= FROM_DOUBLE(0.5)))
          part_id = ((Edje_Part_Description_Proxy*) ep->param2->description)->proxy.id;
        else
          part_id = ((Edje_Part_Description_Proxy*) chosen_desc)->proxy.id;

#ifdef EDJE_CALC_CACHE
        Edje_Real_Part *pp;
        pp = _edje_real_part_state_get(ed, ep, flags, part_id, NULL);
        if (pp && pp->invalidate) proxy_invalidate = EINA_TRUE;
#else
        _edje_real_part_state_get(ed, ep, flags, part_id, NULL);
#endif
     }

   /* Recalc if needed the map center && light source */
   if (ep->param1.description->map.on)
     {
        center[0] = _edje_real_part_state_get(ed, ep, flags, ep->param1.description->map.rot.id_center, &statec1);
        light[0] = _edje_real_part_state_get(ed, ep, flags, ep->param1.description->map.id_light, &statel1);

        if (chosen_desc->map.persp_on)
          {
             persp[0] = _edje_real_part_state_get(ed, ep, flags, ep->param1.description->map.id_persp, &statep1);
          }
     }

   if (ep->param2 && ep->param2->description->map.on)
     {
        center[1] = _edje_real_part_state_get(ed, ep, flags, ep->param2->description->map.rot.id_center, &statec2);
        light[1] = _edje_real_part_state_get(ed, ep, flags, ep->param2->description->map.id_light, &statel2);

        if (chosen_desc->map.persp_on)
          {
             persp[1] = _edje_real_part_state_get(ed, ep, flags, ep->param2->description->map.id_persp, &statep2);
          }
     }

#ifndef EDJE_CALC_CACHE
   p1 = &lp1;
   p1.map = eina_cow_alloc(_edje_calc_params_map_cow);
#ifdef HAVE_EPHYSICS
   p1.physics = eina_cow_alloc(_edje_calc_params_physics_cow);
#endif
#else
   p1 = &ep->param1.p;
#endif

   if (ep->param1.description)
     {
#ifdef EDJE_CALC_CACHE
        if (ed->all_part_change ||
            ep->invalidate ||
            (state1 >= ep->param1.state) ||
            (statec >= ep->param1.state) ||
            (statet >= ep->param1.state) ||
            (statec1 >= ep->param1.state) ||
            (statel1 >= ep->param1.state) ||
            (statep1 >= ep->param1.state) ||
            proxy_invalidate ||
            state ||
            ed->need_map_update ||
            ((ep->part->type == EDJE_PART_TYPE_TEXT ||
              ep->part->type == EDJE_PART_TYPE_TEXTBLOCK) &&
             ed->text_part_change))
#endif
          {
             _edje_part_recalc_single(ed, ep, ep->param1.description,
                                      chosen_desc, center[0], light[0],
                                      persp[0], rp1[Rel1X], rp1[Rel1Y],
                                      rp1[Rel2X], rp1[Rel2Y], confine_to,
                                      threshold, p1, ep->param1.set, mmw, mmh,
                                      pos);
#ifdef EDJE_CALC_CACHE
             if (flags == FLAG_XY) ep->param1.state = ed->state;
#endif
          }
     }
   if (ep->param2)
     {
        int beginning_pos, part_type;
        Edje_Calc_Params *p2, *p3;

        if (ep->current)
          {
             const Edje_Calc_Params_Map *map;
#ifdef HAVE_EPHYSICS
             const Edje_Calc_Params_Physics *physics;
#endif

             map = p1->map;
#ifdef HAVE_EPHYSICS
             physics = p1->physics;
#endif

             /* FIXME: except for text, we don't need in that case to recalc p1 at all*/
             memcpy(p1, ep->current, sizeof (Edje_Calc_Params));

             p1->map = map;
#ifdef HAVE_EPHYSICS
             p1->physics = physics;
#endif
             
          }

        p3 = &lp3;
        lp3.map = eina_cow_alloc(_edje_calc_params_map_cow);
#ifdef HAVE_EPHYSICS
        lp3.physics = eina_cow_alloc(_edje_calc_params_physics_cow);
#endif

#ifndef EDJE_CALC_CACHE
        p2 = &lp2;
        lp2.map = eina_cow_alloc(_edje_calc_params_map_cow);
#ifdef HAVE_EPHYSICS
        lp2.physics = eina_cow_alloc(_edje_calc_params_physics_cow);
#endif
#else
        p2 = &ep->param2->p;

        if (ed->all_part_change ||
            ep->invalidate ||
            (state2 >= ep->param2->state) ||
            (statec >= ep->param2->state) ||
            (statet >= ep->param2->state) ||
            (statec2 >= ep->param2->state) ||
            (statel2 >= ep->param2->state) ||
            (statep2 >= ep->param2->state) ||
            proxy_invalidate ||
            state ||
            ed->need_map_update ||
            ((ep->part->type == EDJE_PART_TYPE_TEXT ||
              ep->part->type == EDJE_PART_TYPE_TEXTBLOCK) &&
             ed->text_part_change))
#endif
          {
             _edje_part_recalc_single(ed, ep, ep->param2->description,
                                      chosen_desc, center[1], light[1],
                                      persp[1], rp2[Rel1X], rp2[Rel1Y],
                                      rp2[Rel2X], rp2[Rel2Y], confine_to,
                                      threshold, p2, ep->param2->set, mmw, mmh,
                                      pos);
#ifdef EDJE_CALC_CACHE
             if (flags == FLAG_XY) ep->param2->state = ed->state;
#endif
          }

        pos2 = pos;
        if (pos2 < ZERO) pos2 = ZERO;
        else if (pos2 > FROM_INT(1)) pos2 = FROM_INT(1);
        beginning_pos = (pos < FROM_DOUBLE(0.5));
        part_type = ep->part->type;

        /* visible is special */
        if ((p1->visible) && (!p2->visible))
          p3->visible = (pos != FROM_INT(1));
        else if ((!p1->visible) && (p2->visible))
          p3->visible = (pos != ZERO);
        else
          p3->visible = p1->visible;

        p3->smooth = (beginning_pos) ? p1->smooth : p2->smooth;

        /* FIXME: do x and y separately base on flag */

        p3->final.x = INTP(p1->final.x, p2->final.x, pos);
        p3->final.y = INTP(p1->final.y, p2->final.y, pos);
        p3->final.w = INTP(p1->final.w, p2->final.w, pos);
        p3->final.h = INTP(p1->final.h, p2->final.h, pos);

        p3->eval.x = FFP(p1->eval.x, p2->eval.x, pos);
        p3->eval.y = FFP(p1->eval.y, p2->eval.y, pos);
        p3->eval.w = FFP(p1->eval.w, p2->eval.w, pos);
        p3->eval.h = FFP(p1->eval.h, p2->eval.h, pos);

        p3->req.x = INTP(p1->req.x, p2->req.x, pos);
        p3->req.y = INTP(p1->req.y, p2->req.y, pos);
        p3->req.w = INTP(p1->req.w, p2->req.w, pos);
        p3->req.h = INTP(p1->req.h, p2->req.h, pos);

        if (ep->part->dragable.x)
          {
             p3->req_drag.x = INTP(p1->req_drag.x, p2->req_drag.x, pos);
             p3->req_drag.w = INTP(p1->req_drag.w, p2->req_drag.w, pos);
          }
        if (ep->part->dragable.y)
          {
             p3->req_drag.y = INTP(p1->req_drag.y, p2->req_drag.y, pos);
             p3->req_drag.h = INTP(p1->req_drag.h, p2->req_drag.h, pos);
          }

        p3->color.r = INTP(p1->color.r, p2->color.r, pos2);
        p3->color.g = INTP(p1->color.g, p2->color.g, pos2);
        p3->color.b = INTP(p1->color.b, p2->color.b, pos2);
        p3->color.a = INTP(p1->color.a, p2->color.a, pos2);

#ifdef HAVE_EPHYSICS
        if (ep->part->physics_body || ep->body)
          {
             EINA_COW_CALC_PHYSICS_BEGIN(p3, p3_write)
               {
                  p3_write->mass = TO_DOUBLE(FINTP(p1->physics->mass, p2->physics->mass,
                                                   pos));
                  p3_write->restitution = TO_DOUBLE(FINTP(p1->physics->restitution,
                                                          p2->physics->restitution,
                                                          pos));
                  p3_write->friction = TO_DOUBLE(FINTP(p1->physics->friction,
                                                       p2->physics->friction, pos));
                  p3_write->density = TO_DOUBLE(FINTP(p1->physics->density,
                                                      p2->physics->density, pos));
                  p3_write->hardness = TO_DOUBLE(FINTP(p1->physics->hardness,
                                                       p2->physics->hardness, pos));

                  p3_write->damping.linear = TO_DOUBLE(FINTP(p1->physics->damping.linear,
                                                             p2->physics->damping.linear, pos));
                  p3_write->damping.angular = TO_DOUBLE(FINTP(p1->physics->damping.angular,
                                                              p2->physics->damping.angular, pos));

                  p3_write->sleep.linear = TO_DOUBLE(FINTP(p1->physics->sleep.linear,
                                                           p2->physics->sleep.linear, pos));
                  p3_write->sleep.angular = TO_DOUBLE(FINTP(p1->physics->sleep.angular,
                                                            p2->physics->sleep.angular, pos));

                  p3_write->z = INTP(p1->physics->z, p2->physics->z, pos);
                  p3_write->depth = INTP(p1->physics->depth, p2->physics->depth, pos);

                  if ((p1->physics->ignore_part_pos) && (p2->physics->ignore_part_pos))
                    p3_write->ignore_part_pos = 1;
                  else
                    p3_write->ignore_part_pos = 0;

                  if ((p1->physics->material) && (p2->physics->material))
                    p3_write->material = p1->physics->material;
                  else
                    p3_write->material = EPHYSICS_BODY_MATERIAL_CUSTOM;

                  p3_write->light_on = p1->physics->light_on || p2->physics->light_on;
                  p3_write->backcull = p1->physics->backcull || p2->physics->backcull;

                  p3_write->mov_freedom.lin.x = p1->physics->mov_freedom.lin.x ||
                    p2->physics->mov_freedom.lin.x;
                  p3_write->mov_freedom.lin.y = p1->physics->mov_freedom.lin.y ||
                    p2->physics->mov_freedom.lin.y;
                  p3_write->mov_freedom.lin.z = p1->physics->mov_freedom.lin.z ||
                    p2->physics->mov_freedom.lin.z;
                  p3_write->mov_freedom.ang.x = p1->physics->mov_freedom.ang.x ||
                    p2->physics->mov_freedom.ang.x;
                  p3_write->mov_freedom.ang.y = p1->physics->mov_freedom.ang.y ||
                    p2->physics->mov_freedom.ang.y;
                  p3_write->mov_freedom.ang.z = p1->physics->mov_freedom.ang.z ||
                    p2->physics->mov_freedom.ang.z;
               }
             EINA_COW_CALC_PHYSICS_END(p3, p3_write);
          }
#endif

        switch (part_type)
          {
           case EDJE_PART_TYPE_IMAGE:
              p3->type.common.spec.image.l = INTP(p1->type.common.spec.image.l, p2->type.common.spec.image.l, pos);
              p3->type.common.spec.image.r = INTP(p1->type.common.spec.image.r, p2->type.common.spec.image.r, pos);
              p3->type.common.spec.image.t = INTP(p1->type.common.spec.image.t, p2->type.common.spec.image.t, pos);
              p3->type.common.spec.image.b = INTP(p1->type.common.spec.image.b, p2->type.common.spec.image.b, pos);
              p3->type.common.spec.image.border_scale_by = FFP(p1->type.common.spec.image.border_scale_by, p2->type.common.spec.image.border_scale_by, pos);
           case EDJE_PART_TYPE_PROXY:
              p3->type.common.fill.x = INTP(p1->type.common.fill.x, p2->type.common.fill.x, pos);
              p3->type.common.fill.y = INTP(p1->type.common.fill.y, p2->type.common.fill.y, pos);
              p3->type.common.fill.w = INTP(p1->type.common.fill.w, p2->type.common.fill.w, pos);
              p3->type.common.fill.h = INTP(p1->type.common.fill.h, p2->type.common.fill.h, pos);
              break;
           case EDJE_PART_TYPE_TEXT:
              p3->type.text.size = INTP(p1->type.text.size, p2->type.text.size, pos);
              /* no break as we share code with the TEXTBLOCK type here. Intended fall-through */
           case EDJE_PART_TYPE_TEXTBLOCK:
              p3->type.text.color2.r = INTP(p1->type.text.color2.r, p2->type.text.color2.r, pos2);
              p3->type.text.color2.g = INTP(p1->type.text.color2.g, p2->type.text.color2.g, pos2);
              p3->type.text.color2.b = INTP(p1->type.text.color2.b, p2->type.text.color2.b, pos2);
              p3->type.text.color2.a = INTP(p1->type.text.color2.a, p2->type.text.color2.a, pos2);

              p3->type.text.color3.r = INTP(p1->type.text.color3.r, p2->type.text.color3.r, pos2);
              p3->type.text.color3.g = INTP(p1->type.text.color3.g, p2->type.text.color3.g, pos2);
              p3->type.text.color3.b = INTP(p1->type.text.color3.b, p2->type.text.color3.b, pos2);
              p3->type.text.color3.a = INTP(p1->type.text.color3.a, p2->type.text.color3.a, pos2);

              p3->type.text.align.x = FFP(p1->type.text.align.x, p2->type.text.align.x, pos);
              p3->type.text.align.y = FFP(p1->type.text.align.y, p2->type.text.align.y, pos);
              p3->type.text.ellipsis = TO_DOUBLE(FINTP(p1->type.text.ellipsis, p2->type.text.ellipsis, pos2));
              break;
          }

        /* mapped is a special case like visible */
        if ((p1->mapped) && (!p2->mapped))
          p3->mapped = (pos != FROM_INT(1));
        else if ((!p1->mapped) && (p2->mapped))
          p3->mapped = (pos != ZERO);
        else
          p3->mapped = p1->mapped;

        p3->persp_on = p3->mapped ? p1->persp_on | p2->persp_on : 0;
        p3->lighted = p3->mapped ? p1->lighted | p2->lighted : 0;
        if (p3->mapped)
          {
             EINA_COW_CALC_MAP_BEGIN(p3, p3_write)
               {
                  p3_write->center.x = INTP(p1->map->center.x, p2->map->center.x, pos);
                  p3_write->center.y = INTP(p1->map->center.y, p2->map->center.y, pos);
                  p3_write->center.z = INTP(p1->map->center.z, p2->map->center.z, pos);
                  p3_write->rotation.x = FFP(p1->map->rotation.x, p2->map->rotation.x, pos);
                  p3_write->rotation.y = FFP(p1->map->rotation.y, p2->map->rotation.y, pos);
                  p3_write->rotation.z = FFP(p1->map->rotation.z, p2->map->rotation.z, pos);

#define MIX(P1, P2, P3, pos, info)                                      \
                  P3->info = P1->map->info + TO_INT(SCALE(pos, P2->map->info - P1->map->info));
                  map_colors_free = _map_colors_interp(p1, p2, p3_write, pos);

                  if (p1->lighted && p2->lighted)
                    {
                       MIX(p1, p2, p3_write, pos, light.x);
                       MIX(p1, p2, p3_write, pos, light.y);
                       MIX(p1, p2, p3_write, pos, light.z);
                       MIX(p1, p2, p3_write, pos, light.r);
                       MIX(p1, p2, p3_write, pos, light.g);
                       MIX(p1, p2, p3_write, pos, light.b);
                       MIX(p1, p2, p3_write, pos, light.ar);
                       MIX(p1, p2, p3_write, pos, light.ag);
                       MIX(p1, p2, p3_write, pos, light.ab);
                    }
                  else if (p1->lighted)
                    {
                       memcpy(&p3_write->light, &p1->map->light, sizeof (p1->map->light));
                    }
                  else if (p2->lighted)
                    {
                       memcpy(&p3_write->light, &p2->map->light, sizeof (p2->map->light));
                    }

                  if (p1->persp_on && p2->persp_on)
                    {
                       MIX(p1, p2, p3_write, pos, persp.x);
                       MIX(p1, p2, p3_write, pos, persp.y);
                       MIX(p1, p2, p3_write, pos, persp.z);
                       MIX(p1, p2, p3_write, pos, persp.focal);
                    }
                  else if (p1->persp_on)
                    {
                       memcpy(&p3_write->persp, &p1->map->persp, sizeof (p1->map->persp));
                    }
                  else if (p2->persp_on)
                    {
                       memcpy(&p3_write->persp, &p2->map->persp, sizeof (p2->map->persp));
                    }
               }
             EINA_COW_CALC_MAP_END(p3, p3_write);
          }

#ifndef EDJE_CALC_CACHE
        eina_cow_free(_edje_calc_params_map_cow, (const Eina_Cow_Data **) &lp2.map);
#ifdef HAVE_EPHYSICS
        eina_cow_free(_edje_calc_params_physics_cow, (const Eina_Cow_Data **) &lp2.physics);
#endif
#endif
        pf = p3;
     }
   else
     {
        pf = p1;
     }

   if (!pf->persp_on && chosen_desc->map.persp_on)
     {
        if (ed->persp)
          {
             EINA_COW_CALC_MAP_BEGIN(pf, pf_write)
               {
                  pf_write->persp.x = ed->persp->px;
                  pf_write->persp.y = ed->persp->py;
                  pf_write->persp.z = ed->persp->z0;
                  pf_write->persp.focal = ed->persp->foc;
               }
             EINA_COW_CALC_MAP_END(pf, pf_write);
          }
        else
          {
             const Edje_Perspective *ps;

             // fixme: a tad inefficient as this is a has lookup
             ps = edje_object_perspective_get(ed->obj);
             if (!ps)
               ps = edje_evas_global_perspective_get(evas_object_evas_get(ed->obj));
             EINA_COW_CALC_MAP_BEGIN(pf, pf_write)
               {
                  if (ps)
                    {
                       pf_write->persp.x = ps->px;
                       pf_write->persp.y = ps->py;
                       pf_write->persp.z = ps->z0;
                       pf_write->persp.focal = ps->foc;
                    }
                  else
                    {
                       pf_write->persp.x = ed->x + (ed->w / 2);
                       pf_write->persp.y = ed->y + (ed->h / 2);
                       pf_write->persp.z = 0;
                       pf_write->persp.focal = 1000;
                    }
               }
             EINA_COW_CALC_MAP_END(pf, pf_write);
          }
     }

   if (state)
     {
        const Edje_Calc_Params_Map *map;
#ifdef HAVE_EPHYSICS
        const Edje_Calc_Params_Physics *physics;
#endif

        map = state->map;
#ifdef HAVE_EPHYSICS
        physics = state->physics;
#endif

        memcpy(state, pf, sizeof (Edje_Calc_Params));

        state->map = map;
#ifdef HAVE_EPHYSICS
        state->physics = physics;
#endif
        eina_cow_memcpy(_edje_calc_params_map_cow, (const Eina_Cow_Data **) &state->map, pf->map);
#ifdef HAVE_EPHYSICS
        eina_cow_memcpy(_edje_calc_params_physics_cow, (const Eina_Cow_Data **) &state->physics, pf->physics);
#endif
     }

   ep->req = pf->req;

   if (ep->drag && ep->drag->need_reset)
     {
        FLOAT_T dx, dy;

        dx = ZERO;
        dy = ZERO;
        _edje_part_dragable_calc(ed, ep, &dx, &dy);
        ep->drag->x = dx;
        ep->drag->y = dy;
        ep->drag->tmp.x = 0;
        ep->drag->tmp.y = 0;
        ep->drag->need_reset = 0;
     }
   if (!ed->calc_only)
     {
        Evas_Object *mo;

        /* Common move, resize and color_set for all part. */
        switch (ep->part->type)
          {
           case EDJE_PART_TYPE_IMAGE:
                {
                   Edje_Part_Description_Image *img_desc = (Edje_Part_Description_Image*) chosen_desc;

                   evas_object_image_scale_hint_set(ep->object,
                                                    img_desc->image.scale_hint);
                   /* No break here as we share the rest of the code for all types. Intended fall-through*/
                }
           case EDJE_PART_TYPE_PROXY:
           case EDJE_PART_TYPE_RECTANGLE:
           case EDJE_PART_TYPE_TEXTBLOCK:
           case EDJE_PART_TYPE_BOX:
           case EDJE_PART_TYPE_TABLE:
              evas_object_color_set(ep->object,
                                    (pf->color.r * pf->color.a) / 255,
                                    (pf->color.g * pf->color.a) / 255,
                                    (pf->color.b * pf->color.a) / 255,
                                    pf->color.a);

#ifdef HAVE_EPHYSICS
/* body attributes should be updated for invisible objects */
              if (!ep->part->physics_body)
                {
                   if (!pf->visible)
                     {
                        evas_object_hide(ep->object);
                        break;
                     }
                   evas_object_show(ep->object);
                }
              else if (!pf->visible)
                {
                   Evas_Object *face_obj;
                   Eina_List *l;

                   EINA_LIST_FOREACH(ep->body_faces, l, face_obj)
                      evas_object_hide(face_obj);
                   evas_object_hide(ep->object);
                }
#else
              if (!pf->visible)
                {
                   evas_object_hide(ep->object);
                   break;
                }
              evas_object_show(ep->object);
#endif
              /* move and resize are needed for all previous object => no break here. */
           case EDJE_PART_TYPE_SWALLOW:
           case EDJE_PART_TYPE_GROUP:
           case EDJE_PART_TYPE_EXTERNAL:
              /* visibility and color have no meaning on SWALLOW and GROUP part. */
#ifdef HAVE_EPHYSICS
              eo_do(ep->object,
                    evas_obj_size_set(pf->final.w, pf->final.h));
              if ((ep->part->physics_body) && (!ep->body))
                {
                   if (_edje_physics_world_geometry_check(ed->world))
                     {
		        _edje_physics_body_add(ed, ep, ed->world);
                        _edje_physics_body_props_update(ed, ep, pf, EINA_TRUE);
                     }
                }
              else if (ep->body)
                {
                   if (((ep->prev_description) &&
                        (chosen_desc != ep->prev_description)) ||
                       (pf != p1))
                     _edje_physics_body_props_update(ed, ep, pf, !pf->physics->ignore_part_pos);
                }
              else
                eo_do(ep->object,
                      evas_obj_position_set(ed->x + pf->final.x, ed->y + pf->final.y));
#else
	      eo_do(ep->object,
                    evas_obj_position_set(ed->x + pf->final.x, ed->y + pf->final.y),
		    evas_obj_size_set(pf->final.w, pf->final.h));
#endif

              if (ep->nested_smart)
                {  /* Move, Resize all nested parts */
                   /* Not really needed but will improve the bounding box evaluation done by Evas */
		   eo_do(ep->nested_smart,
			 evas_obj_position_set(ed->x + pf->final.x, ed->y + pf->final.y),
			 evas_obj_size_set(pf->final.w, pf->final.h));
                }
              if (ep->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
                _edje_entry_real_part_configure(ed, ep);
              break;
           case EDJE_PART_TYPE_TEXT:
              /* This is correctly handle in _edje_text_recalc_apply at the moment. */
              break;
           case EDJE_PART_TYPE_GRADIENT:
              /* FIXME: definitivly remove this code when we switch to new format. */
              abort();
              break;
           case EDJE_PART_TYPE_SPACER:
              /* We really should do nothing on SPACER part */
              break;
          }

        /* Some object need special recalc. */
        switch (ep->part->type)
          {
           case EDJE_PART_TYPE_TEXT:
              _edje_text_recalc_apply(ed, ep, pf, (Edje_Part_Description_Text*) chosen_desc, EINA_FALSE);
              break;
           case EDJE_PART_TYPE_PROXY:
              _edje_proxy_recalc_apply(ed, ep, pf, (Edje_Part_Description_Proxy*) chosen_desc, pos);
              break;
           case EDJE_PART_TYPE_IMAGE:
              _edje_image_recalc_apply(ed, ep, pf, (Edje_Part_Description_Image*) chosen_desc, pos);
              break;
           case EDJE_PART_TYPE_BOX:
              _edje_box_recalc_apply(ed, ep, pf, (Edje_Part_Description_Box*) chosen_desc);
              break;
           case EDJE_PART_TYPE_TABLE:
              _edje_table_recalc_apply(ed, ep, pf, (Edje_Part_Description_Table*) chosen_desc);
              break;
           case EDJE_PART_TYPE_TEXTBLOCK:
              _edje_textblock_recalc_apply(ed, ep, pf, (Edje_Part_Description_Text*) chosen_desc);
              break;
           case EDJE_PART_TYPE_EXTERNAL:
           case EDJE_PART_TYPE_RECTANGLE:
           case EDJE_PART_TYPE_SWALLOW:
           case EDJE_PART_TYPE_GROUP:
              /* Nothing special to do for this type of object. */
              break;
           case EDJE_PART_TYPE_GRADIENT:
              /* FIXME: definitivly remove this code when we switch to new format. */
              abort();
              break;
           case EDJE_PART_TYPE_SPACER:
              /* We really should do nothing on SPACER part */
              break;
          }

        if (((ep->type == EDJE_RP_TYPE_SWALLOW) &&
             (ep->typedata.swallow)) &&
            (ep->typedata.swallow->swallowed_object))
          {
             if (pf->visible)
               {
                  Eina_Bool vis = EINA_TRUE;

                  if (ep->part->type == EDJE_PART_TYPE_GROUP)
                    vis = evas_object_visible_get(ed->obj);
                  eo_do(ep->typedata.swallow->swallowed_object,
                        evas_obj_position_set(ed->x + pf->final.x, ed->y + pf->final.y),
                        evas_obj_size_set(pf->final.w, pf->final.h),
                        evas_obj_visibility_set(vis));
               }
             else evas_object_hide(ep->typedata.swallow->swallowed_object);
             mo = ep->typedata.swallow->swallowed_object;
          }
        else mo = ep->object;
        if (chosen_desc->map.on && ep->part->type != EDJE_PART_TYPE_SPACER)
          {
             static Evas_Map *map = NULL;

             ed->have_mapped_part = EINA_TRUE;
             // create map and populate with part geometry
             if (!map) map = evas_map_new(4);

             _edje_map_prop_set(map, pf, chosen_desc, ep, mo);

             if (ep->nested_smart)
               {  /* Apply map to smart obj holding nested parts */
                  eo_do(ep->nested_smart,
                        evas_obj_map_set(map),
                        evas_obj_map_enable_set(EINA_TRUE));
               }
             else
               {
                  if (mo)
                    eo_do(mo,
                          evas_obj_map_set(map),
                          evas_obj_map_enable_set(EINA_TRUE));
               }
          }
        else
          {
             if (ep->nested_smart)
               {  /* Cancel map of smart obj holding nested parts */
                  eo_do(ep->nested_smart,
                        evas_obj_map_enable_set(EINA_FALSE),
                        evas_obj_map_set(NULL));
               }
             else
               {
#ifdef HAVE_EPHYSICS
                  if (!ep->body)
                    {
#endif
                       if (mo)
                         eo_do(mo,
                               evas_obj_map_enable_set(0),
                               evas_obj_map_set(NULL));
#ifdef HAVE_EPHYSICS
                    }
#endif
               }
          }
     }

   if (map_colors_free) _map_colors_free(pf);

#ifdef HAVE_EPHYSICS
   ep->prev_description = chosen_desc;
   if (!ep->body)
     {
#endif
   ep->x = pf->final.x;
   ep->y = pf->final.y;
   ep->w = pf->final.w;
   ep->h = pf->final.h;
#ifdef HAVE_EPHYSICS
     }
#endif

   ep->calculated |= flags;
   ep->calculating = FLAG_NONE;

   if (pf == &lp3)
     {
        eina_cow_free(_edje_calc_params_map_cow, (const Eina_Cow_Data **) &lp3.map);
        lp3.map = NULL;
#ifdef HAVE_EPHYSICS
        eina_cow_free(_edje_calc_params_physics_cow, (const Eina_Cow_Data **) &lp3.physics);
        lp3.physics = NULL;
#endif
     }

#ifdef EDJE_CALC_CACHE
   if (ep->calculated == FLAG_XY)
     {
        ep->state = ed->state;
        ep->invalidate = EINA_FALSE;
     }
#else
   eina_cow_free(_edje_calc_params_map_cow, (const Eina_Cow_Data **) &lp1.map);
#ifdef HAVE_EPHYSICS
   eina_cow_free(_edje_calc_params_physics_cow, (const Eina_Cow_Data **) &lp1.physics);
#endif
#endif
}
