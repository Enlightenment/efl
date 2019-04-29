#define EFL_GFX_HINT_PROTECTED

#include "edje_private.h"

#include "../evas/canvas/evas_table_eo.h"

static void                          _edje_part_make_rtl(Edje_Part_Description_Common *desc);
static Edje_Part_Description_Common *_edje_get_description_by_orientation(Edje *ed, Edje_Part_Description_Common *src, Edje_Part_Description_Common **dst, unsigned char type);

static void                          _edje_part_recalc_single(Edje *ed, Edje_Real_Part *ep,
                                                              Edje_Part_Description_Common *desc, Edje_Part_Description_Common *chosen_desc,
                                                              Edje_Real_Part *center, Edje_Real_Part *light, Edje_Real_Part *persp,
                                                              Edje_Real_Part *rel1_to_x, Edje_Real_Part *rel1_to_y,
                                                              Edje_Real_Part *rel2_to_x, Edje_Real_Part *rel2_to_y,
                                                              Edje_Real_Part *clip_to,
                                                              Edje_Real_Part *confine_to, Edje_Real_Part *threshold,
                                                              Edje_Calc_Params *params,
                                                              Evas_Coord mmw, Evas_Coord mmh,
                                                              FLOAT_T pos);

#define EINA_COW_CALC_PHYSICS_BEGIN(Calc, Write) \
   _edje_calc_params_need_ext(Calc); \
   EINA_COW_WRITE_BEGIN(_edje_calc_params_physics_cow, Calc->ext->physics, Edje_Calc_Params_Physics, Write)

#define EINA_COW_CALC_PHYSICS_END(Calc, Write) \
   EINA_COW_WRITE_END(_edje_calc_params_physics_cow, Calc->ext->physics, Write)

#define EINA_COW_CALC_MAP_BEGIN(Calc, Write) \
   _edje_calc_params_need_ext(Calc); \
   EINA_COW_WRITE_BEGIN(_edje_calc_params_map_cow, Calc->ext->map, Edje_Calc_Params_Map, Write)

#define EINA_COW_CALC_MAP_END(Calc, Write) \
   EINA_COW_WRITE_END(_edje_calc_params_map_cow, Calc->ext->map, Write)

#ifdef BUILD_EDJE_FP

#define TYPE_EPSILON FLT_EPSILON

#else

#define TYPE_EPSILON DBL_EPSILON

#endif


#define SET_QUATERNION(type)                                                 \
   double norm;                                                              \
   Eina_Quaternion quaternion;                                               \
                                                                             \
   eina_quaternion_set(&quaternion, pd_##type->type.orientation.data[0],     \
                                    pd_##type->type.orientation.data[1],     \
                                    pd_##type->type.orientation.data[2],     \
                                    pd_##type->type.orientation.data[3]);    \
                                                                             \
   norm = eina_quaternion_norm(&quaternion);                                 \
                                                                             \
   if ((norm - 0.0) <= TYPE_EPSILON)                                         \
     ERR("%s %s",                                                            \
         "{0, 0, 0, 0} quaternion gives rotation on non-zero angle",         \
         "around axis without orientation");                                 \
                                                                             \
   eina_quaternion_scale(&quaternion, &quaternion, 1/norm);                  \
                                                                             \
   evas_canvas3d_node_orientation_set(ep->node, quaternion.x, quaternion.y, \
                                      quaternion.z, quaternion.w);

#define SET_LOOK_AT(type)                                                 \
   evas_canvas3d_node_look_at_set(ep->node, pd_##type->type.position.space, \
                                  pd_##type->type.orientation.data[0], \
                                  pd_##type->type.orientation.data[1], \
                                  pd_##type->type.orientation.data[2], \
                                  pd_##type->type.position.space, \
                                  pd_##type->type.orientation.data[3], \
                                  pd_##type->type.orientation.data[4], \
                                  pd_##type->type.orientation.data[5]);

#define SET_LOOK_TO(type)                                                                    \
   Edje_Real_Part *look_to;                                                                  \
   Evas_Real x, y ,z;                                                                        \
   look_to = ed->table_parts[pd_##type->type.orientation.look_to % ed->table_parts_size];    \
   evas_canvas3d_node_position_get(look_to->node, pd_##type->type.position.space, &x, &y, &z);             \
   evas_canvas3d_node_look_at_set(ep->node, pd_##type->type.position.space, x, y, z, \
                                  pd_##type->type.position.space, \
                                  pd_##type->type.orientation.data[3], \
                                  pd_##type->type.orientation.data[4], \
                                  pd_##type->type.orientation.data[5]);

#define SET_ANGLE_AXIS(type)                                                             \
   evas_canvas3d_node_orientation_angle_axis_set(ep->node, pd_##type->type.orientation.data[0], \
                                                 pd_##type->type.orientation.data[1], \
                                                 pd_##type->type.orientation.data[2], \
                                                 pd_##type->type.orientation.data[3]);

void
_edje_calc_params_clear(Edje_Calc_Params *p)
{
   // this happens to cover type.common, type.text and type.node
   if (p->type.common) free(p->type.common);
   p->type.common = NULL;

   // handle cow stuff in one place
   if (p->ext)
     {
#ifdef EDJE_CALC_CACHE
        eina_cow_free(_edje_calc_params_map_cow, (const Eina_Cow_Data **)&(p->ext->map));
        p->ext->map = NULL;
# ifdef HAVE_EPHYSICS
        eina_cow_free(_edje_calc_params_physics_cow, (const Eina_Cow_Data **)&(p->ext->physics));
        p->ext->physics = NULL;
# endif
#endif
        free(p->ext);
        p->ext = NULL;
     }
}

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
   if (EQ(npos, ep->description_pos)) return;

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

   if (!desc)
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

static Edje_Part_Description_Common *
_edje_get_custom_description_by_orientation(Edje *ed, Edje_Part_Description_Common *src, Edje_Part_Description_Common **dst, unsigned char type)
{
   Edje_Part_Description_Common *ret;
   size_t memsize = 0;

   /* RTL flag is not set, return original description */
   if (!edje_object_mirrored_get(ed->obj))
     return src;

   if (!(*dst))
     {
        ret = _edje_get_description_by_orientation(ed, src, dst, type);
        return ret;
     }

#define POPULATE_MEMSIZE_RTL(Short, Type)                        \
case EDJE_PART_TYPE_##Short:                                          \
{                                                                     \
   memsize = sizeof(Edje_Part_Description_##Type);                    \
   break;                                                             \
}

   switch (type)
     {
        POPULATE_MEMSIZE_RTL(RECTANGLE, Common);
        POPULATE_MEMSIZE_RTL(SNAPSHOT, Snapshot);
        POPULATE_MEMSIZE_RTL(SWALLOW, Common);
        POPULATE_MEMSIZE_RTL(GROUP, Common);
        POPULATE_MEMSIZE_RTL(SPACER, Common);
        POPULATE_MEMSIZE_RTL(TEXT, Text);
        POPULATE_MEMSIZE_RTL(TEXTBLOCK, Text);
        POPULATE_MEMSIZE_RTL(IMAGE, Image);
        POPULATE_MEMSIZE_RTL(PROXY, Proxy);
        POPULATE_MEMSIZE_RTL(BOX, Box);
        POPULATE_MEMSIZE_RTL(TABLE, Table);
        POPULATE_MEMSIZE_RTL(EXTERNAL, External);
        POPULATE_MEMSIZE_RTL(CAMERA, Camera);
        POPULATE_MEMSIZE_RTL(LIGHT, Light);
        POPULATE_MEMSIZE_RTL(MESH_NODE, Mesh_Node);
        POPULATE_MEMSIZE_RTL(VECTOR, Vector);
     }
#undef POPULATE_MEMSIZE_RTL

   ret = *dst;
   memcpy(ret, src, memsize);
   _edje_part_make_rtl(ret);

   return ret;
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
   if (!edje_object_mirrored_get(ed->obj))
     return src;

   if (*dst)
     return *dst;  /* Was allocated before and we should use it */

#define EDIT_ALLOC_POOL_RTL(Short, Type, Name)                        \
case EDJE_PART_TYPE_##Short:                                          \
{                                                                     \
   Edje_Part_Description_##Type * Name;                               \
   Name = eina_mempool_malloc(ce->mp->mp_rtl.Short,                   \
                              sizeof (Edje_Part_Description_##Type)); \
   desc_rtl = &Name->common;                                          \
   memsize = sizeof(Edje_Part_Description_##Type);                    \
   break;                                                             \
}

   ce = eina_hash_find(ed->file->collection, ed->group);

   switch (type)
     {
      case EDJE_PART_TYPE_RECTANGLE:
        desc_rtl = eina_mempool_malloc(ce->mp->mp_rtl.RECTANGLE,
                                       sizeof (Edje_Part_Description_Common));
        ce->count.RECTANGLE++;
        memsize = sizeof(Edje_Part_Description_Common);
        break;

      case EDJE_PART_TYPE_SNAPSHOT:
        desc_rtl = eina_mempool_malloc(ce->mp->mp_rtl.SNAPSHOT,
                                       sizeof (Edje_Part_Description_Snapshot));
        ce->count.SNAPSHOT++;
        memsize = sizeof(Edje_Part_Description_Snapshot);
        break;

      case EDJE_PART_TYPE_SWALLOW:
        desc_rtl = eina_mempool_malloc(ce->mp->mp_rtl.SWALLOW,
                                       sizeof (Edje_Part_Description_Common));
        ce->count.SWALLOW++;
        memsize = sizeof(Edje_Part_Description_Common);
        break;

      case EDJE_PART_TYPE_GROUP:
        desc_rtl = eina_mempool_malloc(ce->mp->mp_rtl.GROUP,
                                       sizeof (Edje_Part_Description_Common));
        ce->count.GROUP++;
        memsize = sizeof(Edje_Part_Description_Common);
        break;

      case EDJE_PART_TYPE_SPACER:
        desc_rtl = eina_mempool_malloc(ce->mp->mp_rtl.SPACER,
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
        EDIT_ALLOC_POOL_RTL(CAMERA, Camera, camera);
        EDIT_ALLOC_POOL_RTL(LIGHT, Light, light);
        EDIT_ALLOC_POOL_RTL(MESH_NODE, Mesh_Node, mesh_node);
        EDIT_ALLOC_POOL_RTL(VECTOR, Vector, vector);
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
   if (edje_object_mirrored_get(ed->obj) && !ep->other.desc_rtl)
     {
        ep->other.desc_rtl = (Edje_Part_Description_Common **)
           calloc(ep->other.desc_count,
                  sizeof (Edje_Part_Description_Common *));
     }

   if (!strcmp(state_name, "default") && EQ(state_val, ZERO))
     return _edje_get_description_by_orientation(ed,
                                                 ep->default_desc, &ep->default_desc_rtl, ep->type);

   if (!strcmp(state_name, "custom"))
     return rp->custom ?
            _edje_get_custom_description_by_orientation(ed, rp->custom->description,
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
                  if (EQ(d->state.value, state_val))
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

   if (!st && !imid) return -1;
   if (st && !st->image.set) return st->image.id;
   if (imid && !imid->set) return imid->id;

   if (imid) id = imid->id;
   else id = st->image.id;

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

   entry = eina_list_data_get(set->entries);
   if (entry)
     {
        if (eps)
          {
             if (!*eps)
               *eps = calloc(1, sizeof (Edje_Real_Part_Set));
             if (*eps)
               {
                  (*eps)->entry = entry;
                  (*eps)->set = set;
                  (*eps)->id = id;
               }
          }
        return entry->id;
     }

   return -1;
}

static void
_edje_real_part_image_error_check(Edje_Real_Part *ep)
{
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

static Eina_Bool
_edje_real_part_image_internal_set(Edje_File *edf, Edje_Real_Part *ep, int image_id)
{
   char buf[1024] = "edje/images/";

   /* Replace snprint("edje/images/%i") == memcpy + itoa */
   eina_convert_itoa(image_id, buf + 12); /* No need to check length as 2³² need only 10 characteres. */

   evas_object_image_mmap_set(ep->object, edf->f, buf);
   if (evas_object_image_load_error_get(ep->object) != EVAS_LOAD_ERROR_NONE)
     {
        ERR("Error loading image collection \"%s\" from "
            "file \"%s\". Missing EET Evas loader module?",
            buf, edf->path);
        _edje_real_part_image_error_check(ep);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_edje_real_part_image_external_set(Edje_File *edf, Edje_Real_Part *ep, int image_id)
{
   Edje_Image_Directory_Entry *ie;

   if (!edf->image_dir) return EINA_FALSE;
   ie = edf->image_dir->entries + (-image_id) - 1;
   if ((ie) &&
       (ie->source_type == EDJE_IMAGE_SOURCE_TYPE_USER) &&
       (ie->entry))
     {
        evas_object_image_file_set(ep->object, ie->entry, NULL);
        _edje_real_part_image_error_check(ep);
        return EINA_TRUE;
     }
   else if ((ie) &&
       (ie->source_type == EDJE_IMAGE_SOURCE_TYPE_EXTERNAL) &&
       (ie->entry))
     {
        Edje_File *edff;
        Eina_List *l, *ll;

        l = eina_hash_find(_edje_id_hash, ie->external_id);
        EINA_LIST_FOREACH(l, ll, edff)
          {
             Edje_Image_Hash *eih = eina_hash_find(edff->image_id_hash, ie->entry);

             if (!eih) continue;
             if (eih->id < 0)
               return _edje_real_part_image_external_set(edff, ep, eih->id);
             else
               _edje_real_part_image_internal_set(edff, ep, eih->id);
             return EINA_TRUE;
          }
        return EINA_FALSE;
     }
   return EINA_FALSE;
}

static void
_edje_real_part_image_set(Edje *ed, Edje_Real_Part *ep, Edje_Real_Part_Set **set, FLOAT_T pos)
{
   int image_id;
   int image_count, image_num;

   image_id = _edje_image_find(ep->object, ed,
                               &ep->param1.set,
                               (Edje_Part_Description_Image *)ep->param1.description,
                               NULL);
   if (set) *set = ep->param1.set;
   if (image_id < 0)
     {
        _edje_real_part_image_external_set(ed->file, ep, image_id);
     }
   else
     {
        image_count = 2;
        if (ep->param2)
          image_count += ((Edje_Part_Description_Image *)ep->param2->description)->image.tweens_count;
        image_num = TO_INT(MUL(pos, SUB(FROM_INT(image_count),
                                        FROM_DOUBLE(0.5))));
        if (image_num > (image_count - 1))
          image_num = image_count - 1;
        if (image_num <= 0)
          {
             image_id = _edje_image_find(ep->object, ed,
                                         &ep->param1.set,
                                         (Edje_Part_Description_Image *)ep->param1.description,
                                         NULL);
             if (set) *set = ep->param1.set;
          }
        else
        if (ep->param2)
          {
             if (image_num == (image_count - 1))
               {
                  image_id = _edje_image_find(ep->object, ed,
                                              &ep->param2->set,
                                              (Edje_Part_Description_Image *)ep->param2->description,
                                              NULL);
                  if (set) *set = ep->param2->set;
               }
             else
               {
                  Edje_Part_Image_Id *imid;

                  imid = ((Edje_Part_Description_Image *)ep->param2->description)->image.tweens[image_num - 1];
                  image_id = _edje_image_find(ep->object, ed, NULL, NULL, imid);
                  if (set) *set = NULL;
               }
          }
        if (image_id < 0)
          {
             if (!_edje_real_part_image_external_set(ed->file, ep, image_id))
               ERR("Part \"%s\" description, "
                   "\"%s\" %3.3f with image %i index has a missing image id in a set of %i !!!",
                   ep->part->name,
                   ep->param1.description->state.name,
                   ep->param1.description->state.value,
                   image_num,
                   image_count);
          }
        else
          {
             _edje_real_part_image_internal_set(ed->file, ep, image_id);
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

             external = (Edje_Part_Description_External *)state->description;

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
   Edje_Part_Description_Common *last_desc;
   Eina_Bool change_w, change_h;
   Edje_Part_Description_Image *epdi;

   if (!d1) d1 = "default";

   epd1 = _edje_part_description_find(ed, ep, d1, v1, EINA_TRUE);
   if (!epd1)
     {
        ERR("Cannot find description \"%s\" in part \"%s\" from group \"%s\". Fallback to default description.",
            d1, ep->part->name, ed->group);
        epd1 = _edje_get_description_by_orientation(ed, ep->part->default_desc,
                                                    &ep->part->default_desc_rtl,
                                                    ep->type); /* never NULL */
     }

   if (d2)
     epd2 = _edje_part_description_find(ed, ep, d2, v2, EINA_TRUE);

   epdi = (Edje_Part_Description_Image *)epd2;
   /* There is an animation if both description are different or if description is an image with tweens */
   if (epd2 && (epd1 != epd2 || (ep->part->type == EDJE_PART_TYPE_IMAGE && epdi->image.tweens_count)))
     {
        if (!ep->param2)
          {
             ep->param2 = eina_mempool_malloc(_edje_real_part_state_mp,
                                              sizeof(Edje_Real_Part_State));
             if (ep->param2)
               memset(ep->param2, 0, sizeof(Edje_Real_Part_State));
          }
        else if (ep->part->type == EDJE_PART_TYPE_EXTERNAL)
          {
             if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
                 (ep->typedata.swallow))
               _edje_external_parsed_params_free(ep->typedata.swallow->swallowed_object,
                                                 ep->param2->external_params);
          }
        if (ep->param2)
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
#ifdef EDJE_CALC_CACHE
             _edje_calc_params_clear(&(ep->param2->p));
#endif
          }
        eina_mempool_free(_edje_real_part_state_mp, ep->param2);
        ep->param2 = NULL;
     }

   last_desc = ep->chosen_description;
   ep->param1.description = epd1;
   ep->chosen_description = epd1;
   if (last_desc)
     {
        change_w = ep->chosen_description->fixed.w != last_desc->fixed.w;
        change_h = ep->chosen_description->fixed.h != last_desc->fixed.h;
     }
   _edje_real_part_rel_to_apply(ed, ep, &ep->param1);

   if (ep->param2)
     {
        ep->param2->description = epd2;

        _edje_real_part_rel_to_apply(ed, ep, ep->param2);

        if (ep->description_pos > FROM_DOUBLE(0.0))
          ep->chosen_description = epd2;
     }

   if (last_desc != ep->chosen_description)
     {
        if (ep->part->type == EDJE_PART_TYPE_EXTERNAL)
          _edje_external_recalc_apply(ed, ep, NULL, last_desc);
        else if ((last_desc) && (ep->part->type == EDJE_PART_TYPE_GROUP))
          {
             Edje_Size *min, *max, *pmin, *pmax;

             min = &ep->chosen_description->min;
             max = &ep->chosen_description->max;
             pmin = &last_desc->min;
             pmax = &last_desc->max;
             if (change_w || change_h ||
                 (((pmin->w == pmax->w) && (pmin->h == pmax->h) && (pmin->w > 0) && (pmin->h > 0)) &&
                  (((min->w != max->w) || (min->h != max->h) || (min->w <= 0) || (min->h <= 0)))))
               {
                  Edje *ted = _edje_fetch(ep->typedata.swallow->swallowed_object);
                  if (ted)
                    {
                       ted->recalc_call = ted->dirty = ted->recalc_hints = EINA_TRUE;
                       _edje_recalc(ted);
                    }
               }

             edje_object_mirrored_set(ep->typedata.swallow->swallowed_object,
                   edje_object_mirrored_get(ed->obj));
          }
     }

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
   unsigned short i;
   Eina_Bool need_calc;
#ifdef EDJE_CALC_CACHE
   Eina_Bool need_reinit_state = EINA_FALSE;
#endif

// XXX: dont need this with current smart calc infra. remove me later
//   ed->postponed = EINA_FALSE;
   need_calc = evas_object_smart_need_recalculate_get(ed->obj);
   evas_object_smart_need_recalculate_set(ed->obj, 0);
   if (!ed->dirty) return;
   ed->dirty = EINA_FALSE;
   ed->state++;

   /* Avoid overflow problem */
   if (ed->state == USHRT_MAX)
     {
        ed->state = 0;
#ifdef EDJE_CALC_CACHE
        need_reinit_state = EINA_TRUE;
#endif
     }

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *ep;

        ep = ed->table_parts[i];
        ep->calculated = FLAG_NONE; // FIXME: this is dubious (see below)
        ep->calculating = FLAG_NONE;
#ifdef EDJE_CALC_CACHE
        if (need_reinit_state)
          {
             ep->state = 0;
             ep->param1.state = 0;
             if (ep->param2)
               ep->param2->state = 0;
          }
#endif
     }
   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *ep;

        ep = ed->table_parts[i];
        if (ep->calculated != FLAG_XY) // FIXME: this is always true (see for above)
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
          efl_event_callback_legacy_call(ed->obj, EFL_LAYOUT_EVENT_RECALC, NULL);
     }
   else
     evas_object_smart_need_recalculate_set(ed->obj, need_calc);
   ed->recalc_call = EINA_FALSE;

   if (ed->update_hints && ed->recalc_hints && !ed->calc_only)
     {
        Eina_Size2D min;

        ed->recalc_hints = EINA_FALSE;

        edje_object_size_min_calc(ed->obj, &min.w, &min.h);
        efl_gfx_hint_size_restricted_min_set(ed->obj, min);
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
                 (ep->part->dragable.y != 0)) ret = 3;
             else if (ep->part->dragable.x != 0)
               ret = 1;
             else if (ep->part->dragable.y != 0)
               ret = 2;

             dx = FROM_INT(ep->x - ep->drag->confine_to->x);
             dw = FROM_INT(ep->drag->confine_to->w - ep->w);
             if (NEQ(dw, ZERO)) dx = DIV(dx, dw);
             else dx = ZERO;

             dy = FROM_INT(ep->y - ep->drag->confine_to->y);
             dh = FROM_INT(ep->drag->confine_to->h - ep->h);
             if (NEQ(dh, ZERO)) dy = DIV(dy, dh);
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
   if (ep->drag->down.count > 0) return;

   /* instead of checking for equality, we really should check that
    * the difference is greater than foo, but I have no idea what
    * value we would set foo to, because it would depend on the
    * size of the dragable...
    */

   if (NEQ(ep->drag->x, x) || ep->drag->tmp.x)
     {
        ep->drag->x = x;
        ep->drag->tmp.x = 0;
        ep->drag->need_reset = 0;
        ed->dirty = EINA_TRUE;
        ed->recalc_call = EINA_TRUE;
     }

   if (NEQ(ep->drag->y, y) || ep->drag->tmp.y)
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
   FLOAT_T offset, sc;
   Eina_Bool offset_is_scaled = (desc->offset_is_scaled) && (ep->part->scale);

   if (offset_is_scaled)
     {
        sc = DIV(ed->scale, ed->file->base_scale);
        if (EQ(sc, ZERO)) sc = DIV(_edje_scale, ed->file->base_scale);
     }

   if (offset_is_scaled)
     offset = SCALE(sc, desc->rel1.offset_x);
   else
     offset = FROM_INT(desc->rel1.offset_x);

   if (rel1_to_x)
     x = ADD(ADD(offset, FROM_INT(rel1_to_x->x)),
             SCALE(desc->rel1.relative_x, rel1_to_x->w));
   else
     x = ADD(offset,
             SCALE(desc->rel1.relative_x, ed->w));
   params->eval.x = x;

   if (offset_is_scaled)
     offset = SUB(SCALE(sc, desc->rel2.offset_x + 1), FROM_INT(1));
   else
     offset = FROM_INT(desc->rel2.offset_x);

   if (rel2_to_x)
     w = ADD(SUB(ADD(ADD(offset, FROM_INT(rel2_to_x->x)),
                     SCALE(desc->rel2.relative_x, rel2_to_x->w)),
                 x),
             FROM_INT(1));
   else
     w = ADD(SUB(ADD(offset,
                     SCALE(desc->rel2.relative_x, ed->w)),
                 x),
             FROM_INT(1));
   params->eval.w = w;


   if (offset_is_scaled)
     offset = SCALE(sc, desc->rel1.offset_y);
   else
     offset = FROM_INT(desc->rel1.offset_y);

   if (rel1_to_y)
     y = ADD(ADD(offset, FROM_INT(rel1_to_y->y)),
             SCALE(desc->rel1.relative_y, rel1_to_y->h));
   else
     y = ADD(offset,
             SCALE(desc->rel1.relative_y, ed->h));
   params->eval.y = y;

   if (offset_is_scaled)
     offset = SUB(SCALE(sc, desc->rel2.offset_y + 1), FROM_INT(1));
   else
     offset = FROM_INT(desc->rel2.offset_y);

   if (rel2_to_y)
     h = ADD(SUB(ADD(ADD(offset, FROM_INT(rel2_to_y->y)),
                     SCALE(desc->rel2.relative_y, rel2_to_y->h)),
                 y),
             FROM_INT(1));
   else
     h = ADD(SUB(ADD(offset,
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
        _edje_real_part_image_set(ed, ep, NULL, pos);
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
             if (aspect < amin)
               {
                  new_h = DIV(params->eval.w, amin);
                  new_w = MUL(amin, params->eval.h);
               }
             /* adjust for max aspect (width / height) */
             if (aspect > amax)
               {
                  new_h = DIV(params->eval.w, amax);
                  new_w = MUL(amax, params->eval.h);
               }
             if (new_w < params->eval.w)
               {
                  new_w = params->eval.w;
                  new_h = DIV(params->eval.w, amax);
               }
             if (new_h < params->eval.h)
               {
                  new_w = MUL(amax, params->eval.h);
                  new_h = params->eval.h;
               }
             break;

           /* prefer vertical size as determiner */
           case  EDJE_ASPECT_PREFER_VERTICAL:
             /* keep both dimensions in check */
             /* adjust for max aspect (width / height) */
             if (aspect > amax)
               new_w = MUL(amax, params->eval.h);
             /* adjust for min aspect (width / height) */
             if (aspect < amin)
               new_w = MUL(amin, params->eval.h);
             break;

           /* prefer horizontal size as determiner */
           case EDJE_ASPECT_PREFER_HORIZONTAL:
             /* keep both dimensions in check */
             /* adjust for max aspect (width / height) */
             if (aspect > amax)
               new_h = DIV(params->eval.w, amax);
             /* adjust for min aspect (width / height) */
             if (aspect < amin)
               new_h = DIV(params->eval.w, amin);
             break;

           case EDJE_ASPECT_PREFER_SOURCE:
           case EDJE_ASPECT_PREFER_BOTH:
             /* keep both dimensions in check */
             /* adjust for max aspect (width / height) */
             if (aspect > amax)
               {
                  new_w = MUL(amax, params->eval.h);
                  new_h = DIV(params->eval.w, amax);
               }
             /* adjust for min aspect (width / height) */
             if (aspect < amin)
               {
                  new_w = MUL(amin, params->eval.h);
                  new_h = DIV(params->eval.w, amin);
               }
             break;

           default:
             break;
          }

        if (apref != EDJE_ASPECT_PREFER_NONE)
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
   int mw, mh, size;
   Eina_Size2D sz;
   char *sfont = NULL;

   _edje_text_class_font_get(ed, desc, &size, &sfont);
   free(sfont);

   _edje_calc_params_need_type_text(params);
   params->type.text->size = size; /* XXX TODO used by further calcs, go inside recalc_apply? */

   _edje_text_recalc_apply(ed, ep, params, chosen_desc, EINA_TRUE);

   if ((!chosen_desc) ||
       ((!chosen_desc->text.min_x) && (!chosen_desc->text.min_y) &&
        (!chosen_desc->text.max_x) && (!chosen_desc->text.max_y)))
     return;

   // Note: No need to add padding to that, it's already in the geometry
   sz = efl_gfx_entity_size_get(ep->object);
   mw = sz.w;
   mh = sz.h;

   if (chosen_desc->text.max_x)
     {
        if ((*maxw < 0) || (mw > *maxw)) *maxw = mw;
     }
   if (chosen_desc->text.max_y)
     {
        if ((*maxh < 0) || (mh > *maxh)) *maxh = mh;
     }
   if (chosen_desc->text.min_x)
     {
        if (mw > *minw) *minw = mw;
        if ((*maxw > -1) && (*minw > *maxw)) *minw = *maxw;
     }
   if (chosen_desc->text.min_y)
     {
        if (mh > *minh) *minh = mh;
        if ((*maxh > -1) && (*minh > *maxh)) *minh = *maxh;
     }
   if ((*maxw > -1) && (mw > *maxw)) mw = *maxw;
   if ((*maxh > -1) && (mh > *maxh)) mh = *maxh;

   evas_object_resize(ep->object, mw, mh);
}

#else
{
   char *sfont = NULL;
   int size;

   if (chosen_desc)
     {
        const char *text;
        const char *font;
        Eina_Size2D ts;
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

             evas_obj_text_style_set(ep->object, style);
             evas_obj_text_set(ep->object, text);
             ts = efl_gfx_entity_size_get(ep->object);
             if (chosen_desc->text.max_x)
               {
                  int l, r;
                  evas_object_text_style_pad_get(ep->object, &l, &r, NULL, NULL);
                  mw = ts.w + l + r;
                  if ((*maxw < 0) || (mw < *maxw)) *maxw = mw;
               }
             if (chosen_desc->text.max_y)
               {
                  int t, b;
                  evas_object_text_style_pad_get(ep->object, NULL, NULL, &t, &b);
                  mh = ts.h + t + b;
                  if ((*maxh < 0) || (mh < *maxh)) *maxh = mh;
               }
             if (chosen_desc->text.min_x)
               {
                  int l, r;
                  evas_object_text_style_pad_get(ep->object, &l, &r, NULL, NULL);
                  mw = ts.w + l + r;
                  if (mw > *minw) *minw = mw;
               }
             if (chosen_desc->text.min_y)
               {
                  int t, b;
                  evas_object_text_style_pad_get(ep->object, NULL, NULL, &t, &b);
                  mh = ts.h + t + b;
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

   w = NEQ(params->eval.w, ZERO) ? params->eval.w : FROM_INT(99999);
   h = NEQ(params->eval.h, ZERO) ? params->eval.h : FROM_INT(99999);

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
        EINA_FALLTHROUGH;

      case EDJE_ASPECT_PREFER_HORIZONTAL:
        tmp = DIV(SCALE(params->eval.h, minw), w);
        if (tmp >= FROM_INT(minh))
          {
             minh = TO_INT(tmp);
             break;
          }
        EINA_FALLTHROUGH;

      case EDJE_ASPECT_PREFER_SOURCE:
         EINA_FALLTHROUGH;
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
             *start = ADD(*start, SCALE(align, (*length - max)));
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

   w = NEQ(params->eval.w, 0) ? params->eval.w : FROM_INT(99999);
   h = NEQ(params->eval.h, 0) ? params->eval.h : FROM_INT(99999);

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
        EINA_FALLTHROUGH;

      case EDJE_ASPECT_PREFER_HORIZONTAL:
        tmp = DIV(SCALE(params->eval.h, maxw), w);
        if (tmp <= FROM_INT(maxh))
          {
             maxh = TO_INT(tmp);
             break;
          }
        EINA_FALLTHROUGH;

      case EDJE_ASPECT_PREFER_SOURCE:
        EINA_FALLTHROUGH;
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
   _edje_calc_params_need_ext(params);
   if (threshold)
     {
        if (ep->drag->threshold_started_x &&
            threshold->x < TO_INT(params->eval.x) &&
            TO_INT(params->eval.x) + TO_INT(params->eval.w) < threshold->x + threshold->w)
          {
             // Cancel movement to previous position due to our presence inside the threshold
             params->eval.x = FROM_INT(params->ext->req_drag.x);
             params->eval.w = FROM_INT(params->ext->req_drag.w);
             ep->drag->threshold_x = EINA_TRUE;
          }
        else
          {
             params->ext->req_drag.x = TO_INT(params->eval.x);
             params->ext->req_drag.w = TO_INT(params->eval.w);
             ep->drag->threshold_started_x = EINA_FALSE;
          }
        if (ep->drag->threshold_started_y &&
            threshold->y < TO_INT(params->eval.y) &&
            TO_INT(params->eval.y) + TO_INT(params->eval.h) < threshold->y + threshold->h)
          {
             // Cancel movement to previous position due to our presence inside the threshold
             params->eval.y = FROM_INT(params->ext->req_drag.y);
             params->eval.h = FROM_INT(params->ext->req_drag.h);
             ep->drag->threshold_y = EINA_TRUE;
          }
        else
          {
             params->ext->req_drag.y = TO_INT(params->eval.y);
             params->ext->req_drag.h = TO_INT(params->eval.h);
             ep->drag->threshold_started_y = EINA_FALSE;
          }
     }
   else
     {
        params->ext->req_drag.x = TO_INT(params->eval.x);
        params->ext->req_drag.w = TO_INT(params->eval.w);
        params->ext->req_drag.y = TO_INT(params->eval.y);
        params->ext->req_drag.h = TO_INT(params->eval.h);
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
        else if ((maxw >= 0) && (TO_INT(v) > maxw))
          params->eval.w = FROM_INT(maxw);
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
        else if ((maxh >= 0) && (TO_INT(v) > maxh))
          params->eval.h = FROM_INT(maxh);
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

   _edje_calc_params_need_type_common(params);
   params->type.common->fill.x = fill->pos_abs_x
     + TO_INT(SCALE(fill->pos_rel_x, fw));
   params->type.common->fill.w = fill->abs_x
     + TO_INT(SCALE(fill->rel_x, fw));

   if (fill->type == EDJE_FILL_TYPE_TILE)
     evas_object_image_size_get(ep->object, NULL, &fh);
   else
     fh = params->final.h;

   params->type.common->fill.y = fill->pos_abs_y
     + TO_INT(SCALE(fill->pos_rel_y, fh));
   params->type.common->fill.h = fill->abs_y
     + TO_INT(SCALE(fill->rel_y, fh));
}

static void
_edje_part_recalc_single_min_max(FLOAT_T sc,
                                 Edje *ed,
                                 Edje_Real_Part *ep,
                                 Edje_Part_Description_Common *desc,
                                 int *minw, int *minh,
                                 int *maxw, int *maxh)
{
   Edje_Size_Class *size_class = NULL;
   Evas_Coord mnw, mnh, mxw, mxh;

   if (desc->size_class)
     size_class = _edje_size_class_find(ed, desc->size_class);

   if (size_class)
     {
        mnw = size_class->minw;
        mnh = size_class->minh;
        mxw = size_class->maxw;
        mxh = size_class->maxh;
     }
   else
     {
        mnw = desc->min.w;
        mnh = desc->min.h;
        mxw = desc->max.w;
        mxh = desc->max.h;
     }

   *minw = mnw;
   if (ep->part->scale) *minw = TO_INT_ROUND(SCALE(sc, *minw));
   if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
       (ep->typedata.swallow))
     {
        if (ep->typedata.swallow->swallow_params.min.w > *minw)
          *minw = ep->typedata.swallow->swallow_params.min.w;
     }

   if (ed->calc_only)
     {
        if (desc->minmul.have)
          {
             FLOAT_T mmw = desc->minmul.w;
             if (NEQ(mmw, FROM_INT(1))) *minw = TO_INT(SCALE(mmw, *minw));
          }
     }

   if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
       (ep->typedata.swallow))
     {
        /* XXX TODO: remove need of EDJE_INF_MAX_W, see edje_util.c */
        if ((ep->typedata.swallow->swallow_params.max.w <= 0) ||
            (ep->typedata.swallow->swallow_params.max.w == EDJE_INF_MAX_W))
          {
             *maxw = mxw;
             if (*maxw > 0)
               {
                  if (ep->part->scale) *maxw = TO_INT_ROUND(SCALE(sc, *maxw));
                  if (*maxw < 1) *maxw = 1;
               }
          }
        else
          {
             if (mxw <= 0)
               *maxw = ep->typedata.swallow->swallow_params.max.w;
             else
               {
                  *maxw = mxw;
                  if (*maxw > 0)
                    {
                       if (ep->part->scale) *maxw = TO_INT_ROUND(SCALE(sc, *maxw));
                       if (*maxw < 1) *maxw = 1;
                    }
                  if (ep->typedata.swallow->swallow_params.max.w < *maxw)
                    *maxw = ep->typedata.swallow->swallow_params.max.w;
               }
          }
     }
   else
     {
        *maxw = mxw;
        if (*maxw > 0)
          {
             if (ep->part->scale) *maxw = TO_INT_ROUND(SCALE(sc, *maxw));
             if (*maxw < 1) *maxw = 1;
          }
     }
   if ((ed->calc_only) && (desc->minmul.have) &&
       (NEQ(desc->minmul.w, FROM_INT(1)))) *maxw = *minw;
   if (*maxw >= 0)
     {
        if (*maxw < *minw) *maxw = *minw;
     }

   *minh = mnh;
   if (ep->part->scale) *minh = TO_INT_ROUND(SCALE(sc, *minh));
   if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
       (ep->typedata.swallow))
     {
        if (ep->typedata.swallow->swallow_params.min.h > *minh)
          *minh = ep->typedata.swallow->swallow_params.min.h;
     }

   if (ed->calc_only)
     {
        if (desc->minmul.have)
          {
             FLOAT_T mmh = desc->minmul.h;
             if (NEQ(mmh, FROM_INT(1))) *minh = TO_INT(SCALE(mmh, *minh));
          }
     }

   if ((ep->type == EDJE_RP_TYPE_SWALLOW) &&
       (ep->typedata.swallow))
     {
        /* XXX TODO: remove need of EDJE_INF_MAX_H, see edje_util.c */
        if ((ep->typedata.swallow->swallow_params.max.h <= 0) ||
            (ep->typedata.swallow->swallow_params.max.h == EDJE_INF_MAX_H))
          {
             *maxh = mxh;
             if (*maxh > 0)
               {
                  if (ep->part->scale) *maxh = TO_INT_ROUND(SCALE(sc, *maxh));
                  if (*maxh < 1) *maxh = 1;
               }
          }
        else
          {
             if (mxh <= 0)
               *maxh = ep->typedata.swallow->swallow_params.max.h;
             else
               {
                  *maxh = mxh;
                  if (*maxh > 0)
                    {
                       if (ep->part->scale) *maxh = TO_INT_ROUND(SCALE(sc, *maxh));
                       if (*maxh < 1) *maxh = 1;
                    }
                  if (ep->typedata.swallow->swallow_params.max.h < *maxh)
                    *maxh = ep->typedata.swallow->swallow_params.max.h;
               }
          }
     }
   else
     {
        *maxh = mxh;
        if (*maxh > 0)
          {
             if (ep->part->scale) *maxh = TO_INT_ROUND(SCALE(sc, *maxh));
             if (*maxh < 1) *maxh = 1;
          }
     }
   if ((ed->calc_only) && (desc->minmul.have) &&
       (NEQ(desc->minmul.h, FROM_INT(1)))) *maxh = *minh;
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
      params_write->zoom.x = desc->map.zoom.x;
      params_write->zoom.y = desc->map.zoom.y;

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
           if (NEQ(pos, ZERO) && light_desc2)
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

           if (NEQ(pos, ZERO) && persp->param2)
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

static int
_filter_bsearch_cmp(const void *a, const void *b)
{
   const Edje_Gfx_Filter *filter = b;
   const char *key = a;

   return strcmp(key, filter->name);
}

static inline const char *
_edje_filter_get(Edje *ed, Edje_Part_Description_Spec_Filter *filter)
{
   if (!filter->code) return NULL;
   if (EINA_UNLIKELY(!filter->checked_data))
     {
        Edje_Gfx_Filter *found;

        filter->checked_data = EINA_TRUE;
        if (!ed->file->filter_dir)
          return filter->code;

        found = bsearch(filter->code, &(ed->file->filter_dir->filters[0]),
                        ed->file->filter_dir->filters_count,
                        sizeof(Edje_Gfx_Filter), _filter_bsearch_cmp);
        if (found)
          {
             filter->name = found->name;
             filter->code = found->script;
             filter->no_free = EINA_TRUE;
             return filter->code;
          }
     }
   return filter->code;
}

static void
_edje_part_pixel_adjust(Edje *ed,
                        Edje_Real_Part *ep,
                        Edje_Calc_Params *params)
{
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

   if (params->final.w < 0 || params->final.h < 0)
     ERR("The params final size became negative");

}

static void
_edje_part_recalc_single_filter(Edje *ed,
                                Edje_Real_Part *ep,
                                Edje_Part_Description_Common *desc,
                                Edje_Part_Description_Common *chosen_desc,
                                double pos)
{
   Edje_Part_Description_Spec_Filter *filter, *prevfilter;
   Eina_List *filter_sources = NULL, *prev_sources = NULL;
   const char *src1, *src2, *part, *code;
   Evas_Object *obj = ep->object;
   Eina_List *li1, *li2;

   /* handle TEXT, IMAGE, PROXY, SNAPSHOT part types here */
   if (ep->part->type == EDJE_PART_TYPE_TEXT)
     {
        Edje_Part_Description_Text *chosen_edt = (Edje_Part_Description_Text *) chosen_desc;
        Edje_Part_Description_Text *edt = (Edje_Part_Description_Text *) desc;
        filter = &chosen_edt->filter;
        prev_sources = edt->filter.sources;
        filter_sources = chosen_edt->filter.sources;
        prevfilter = &(edt->filter);
     }
   else if (ep->part->type == EDJE_PART_TYPE_IMAGE)
     {
        Edje_Part_Description_Image *chosen_edi = (Edje_Part_Description_Image *) chosen_desc;
        Edje_Part_Description_Image *edi = (Edje_Part_Description_Image *) desc;
        filter = &chosen_edi->filter;
        prev_sources = edi->filter.sources;
        filter_sources = chosen_edi->filter.sources;
        prevfilter = &(edi->filter);
     }
   else if (ep->part->type == EDJE_PART_TYPE_PROXY)
     {
        Edje_Part_Description_Proxy *chosen_edp = (Edje_Part_Description_Proxy *) chosen_desc;
        Edje_Part_Description_Proxy *edp = (Edje_Part_Description_Proxy *) desc;
        filter = &chosen_edp->filter;
        prev_sources = edp->filter.sources;
        filter_sources = chosen_edp->filter.sources;
        prevfilter = &(edp->filter);
     }
   else if (ep->part->type == EDJE_PART_TYPE_SNAPSHOT)
     {
        Edje_Part_Description_Snapshot *chosen_eds = (Edje_Part_Description_Snapshot *) chosen_desc;
        Edje_Part_Description_Snapshot *eds = (Edje_Part_Description_Snapshot *) desc;
        filter = &chosen_eds->filter;
        prev_sources = eds->filter.sources;
        filter_sources = chosen_eds->filter.sources;
        prevfilter = &(eds->filter);
     }
   else
     {
        CRI("Invalid call to filter recalc");
        return;
     }

   if ((!filter->code) && (!prevfilter->code)) return;

   /* common code below */
   code = _edje_filter_get(ed, filter);
   if (!code)
     {
        efl_gfx_filter_program_set(obj, NULL, NULL);
        return;
     }

   if (!filter->sources_set)
     {
        filter->sources_set = 1;
        prev_sources = NULL;
     }

   /* pass extra data items */
   if (filter->data)
     {
        unsigned int k;
        for (k = 0; k < filter->data_count; k++)
          {
             Edje_Part_Description_Spec_Filter_Data *data = &(filter->data[k]);
             if (data->invalid_cc)
                continue;
             if (!data->value)
               {
                  efl_gfx_filter_data_set(obj, data->name, NULL, EINA_FALSE);
               }
             else if (!strncmp(data->value, "color_class('", sizeof("color_class('") - 1))
               {
                  /* special handling for color classes even tho they're not that great */
                  char *ccname, *buffer, *r;
                  Edje_Color_Class *cc;

                  ccname = strdup(data->value + sizeof("color_class('") - 1);
                  if (ccname)
                    {
                       r = strchr(ccname, '\'');
                       if (r && (r[1] == ')') && (r[2] == '\0'))
                         {
                            *r = '\0';
                            cc = _edje_color_class_find(ed, ccname);
                            if (cc)
                              {
                                 static const char fmt[] =
                                       "{r=%d,g=%d,b=%d,a=%d,"
                                       "r2=%d,g2=%d,b2=%d,a2=%d,"
                                       "r3=%d,g3=%d,b3=%d,a3=%d}";
                                 int len = sizeof(fmt) + 20;
                                 len += strlen(data->name);
                                 buffer = alloca(len);
                                 snprintf(buffer, len - 1, fmt,
                                          (int) cc->r, (int) cc->g, (int) cc->b, (int) cc->a,
                                          (int) cc->r2, (int) cc->g2, (int) cc->b2, (int) cc->a2,
                                          (int) cc->r3, (int) cc->g3, (int) cc->b3, (int) cc->a3);
                                 buffer[len - 1] = 0;
                                 efl_gfx_filter_data_set(obj, data->name, buffer, EINA_TRUE);
                              }
                            else
                              {
                                 ERR("Unknown color class: %s", ccname);
                                 data->invalid_cc = EINA_TRUE;
                              }
                         }
                       else
                         {
                            ERR("Failed to parse color class: %s", data->value);
                            data->invalid_cc = EINA_TRUE;
                         }
                       free(ccname);
                    }
               }
             else
                efl_gfx_filter_data_set(obj, data->name, data->value, EINA_FALSE);
          }
     }
   efl_gfx_filter_program_set(obj, code, filter->name);
   if (prev_sources != filter_sources)
     {
        /* remove sources that are not there anymore
         * this O(n^2) loop assumes a very small number of sources */
        EINA_LIST_FOREACH(prev_sources, li1, src1)
          {
             Eina_Bool found = 0;
             EINA_LIST_FOREACH(filter_sources, li2, src2)
               {
                  if (!strcmp(src1, src2))
                    {
                       found = 1;
                       break;
                    }
               }
             if (!found)
               {
                  part = strchr(src1, ':');
                  if (!part)
                     efl_gfx_filter_source_set(obj, src1, NULL);
                  else
                    {
                       char *name = strdup(src1);
                       if (!name) continue ;
                       name[part - src1] = 0;
                       efl_gfx_filter_source_set(obj, name, NULL);
                       free(name);
                    }
               }
          }
        /* add all sources by part name */
        EINA_LIST_FOREACH(filter_sources, li1, src1)
          {
             Edje_Real_Part *rp;
             char *name = NULL;
             if ((part = strchr(src1, ':')) != NULL)
               {
                  name = strdup(src1);
                  if (!name) continue ;
                  name[part - src1] = 0;
                  part++;
               }
             else
                part = src1;
             rp = _edje_real_part_get(ed, part);
             efl_gfx_filter_source_set(obj, name ? name : part, rp ? rp->object : NULL);
             free(name);
          }
     }
   /* pass edje state for transitions */
   if (ep->param2)
     {
        efl_gfx_filter_state_set(obj, chosen_desc->state.name, chosen_desc->state.value,
              ep->param2->description->state.name, ep->param2->description->state.value,
              pos);
     }
   else
     {
        efl_gfx_filter_state_set(obj, chosen_desc->state.name, chosen_desc->state.value,
              NULL, 0.0, pos);
     }
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
                         Edje_Real_Part *clip_to,
                         Edje_Real_Part *confine_to,
                         Edje_Real_Part *threshold,
                         Edje_Calc_Params *params,
                         Evas_Coord mmw, Evas_Coord mmh,
                         FLOAT_T pos)
{
   Edje_Color_Class *cc = NULL;
   Edje_Internal_Aspect apref;
   int minw = 0, minh = 0, maxw = 0, maxh = 0;
   Eina_Bool fixedw = EINA_FALSE, fixedh = EINA_FALSE;
   FLOAT_T sc;

   sc = DIV(ed->scale, ed->file->base_scale);
   if (EQ(sc, ZERO)) sc = DIV(_edje_scale, ed->file->base_scale);
   _edje_part_recalc_single_min_max(sc, ed, ep, desc, &minw, &minh, &maxw, &maxh);
   if (minw < mmw) minw = mmw;
   if (minh < mmh) minh = mmh;

   /* relative coords of top left & bottom right */
   _edje_part_recalc_single_rel(ed, ep, desc, rel1_to_x, rel1_to_y, rel2_to_x, rel2_to_y, params);

   /* aspect */
   apref = _edje_part_recalc_single_aspect(ed, ep, desc, params, &minw, &minh, &maxw, &maxh, pos);

   /* size step */
   _edje_part_recalc_single_step(desc, params);

   /* check whether this part has fixed value or not*/
   if ((rel1_to_x == rel2_to_x) &&
       (EQ(desc->rel1.relative_x, desc->rel2.relative_x)) &&
       (!chosen_desc->fixed.w))
     {
        chosen_desc->fixed.w = 1;
        fixedw = EINA_TRUE;
     }

   if ((rel1_to_y == rel2_to_y) &&
       (EQ(desc->rel1.relative_y, desc->rel2.relative_y)) &&
       (!chosen_desc->fixed.h))
     {
        chosen_desc->fixed.h = 1;
        fixedh = EINA_TRUE;
     }
   if (fixedw || fixedh)
     {
        INF("file %s, group %s has a non-fixed part '%s'. You should add "
            "'fixed: %d %d'. But in order to optimize the edje calc, we "
            "add it automatically.", ed->path, ed->group, ep->part->name,
            fixedw, fixedh);
     }

   /* colors */
   if (ep->part->type != EDJE_PART_TYPE_SPACER)
     {
        if ((desc->color_class) && (*desc->color_class))
          cc = _edje_color_class_recursive_find(ed, desc->color_class);

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

   /* no_render override */
   if (EDJE_DESC_NO_RENDER_IS_SET(desc))
     {
        params->no_render = EDJE_DESC_NO_RENDER_VALUE(desc);
        params->no_render_apply = 1;
     }
   else
     {
        params->no_render = ep->part->no_render;
        params->no_render_apply = 0;
     }

   /* clip override */
   if (clip_to)
     {
        _edje_calc_params_need_ext(params);
        params->ext->clip_to = clip_to;
     }

   /* set parameters, some are required for recalc_single_text[block] */
   switch (ep->part->type)
     {
      case EDJE_PART_TYPE_IMAGE:
      {
         Edje_Real_Part_Set *set;
         Edje_Part_Description_Image *img_desc = (Edje_Part_Description_Image *)desc;

         _edje_real_part_image_set(ed, ep, &set, pos);

         /* border */
         _edje_calc_params_need_type_common(params);
         params->type.common->spec.image.l = img_desc->image.border.l;
         params->type.common->spec.image.r = img_desc->image.border.r;

         params->type.common->spec.image.t = img_desc->image.border.t;
         params->type.common->spec.image.b = img_desc->image.border.b;

         params->type.common->spec.image.border_scale_by = img_desc->image.border.scale_by;

         if (set && set->set)
           {
#define SET_BORDER_DEFINED(Result, Value) Result = Value ? Value : Result;
              SET_BORDER_DEFINED(params->type.common->spec.image.l, set->entry->border.l);
              SET_BORDER_DEFINED(params->type.common->spec.image.r, set->entry->border.r);
              SET_BORDER_DEFINED(params->type.common->spec.image.t, set->entry->border.t);
              SET_BORDER_DEFINED(params->type.common->spec.image.b, set->entry->border.b);

              params->type.common->spec.image.border_scale_by = NEQ(set->entry->border.scale_by, ZERO) ?
                set->entry->border.scale_by : params->type.common->spec.image.border_scale_by;
           }

         break;
      }

      case EDJE_PART_TYPE_TEXT:
      case EDJE_PART_TYPE_TEXTBLOCK:
      {
         Edje_Part_Description_Text *text_desc = (Edje_Part_Description_Text *)desc;

         _edje_calc_params_need_type_text(params);
         /* text.align */
         params->type.text->align.x = text_desc->text.align.x;
         params->type.text->align.y = text_desc->text.align.y;
         params->type.text->ellipsis = text_desc->text.ellipsis;

         /* text colors */
         if (cc)
           {
              params->type.text->color2.r = (((int)cc->r2 + 1) * text_desc->common.color2.r) >> 8;
              params->type.text->color2.g = (((int)cc->g2 + 1) * text_desc->common.color2.g) >> 8;
              params->type.text->color2.b = (((int)cc->b2 + 1) * text_desc->common.color2.b) >> 8;
              params->type.text->color2.a = (((int)cc->a2 + 1) * text_desc->common.color2.a) >> 8;
              params->type.text->color3.r = (((int)cc->r3 + 1) * text_desc->text.color3.r) >> 8;
              params->type.text->color3.g = (((int)cc->g3 + 1) * text_desc->text.color3.g) >> 8;
              params->type.text->color3.b = (((int)cc->b3 + 1) * text_desc->text.color3.b) >> 8;
              params->type.text->color3.a = (((int)cc->a3 + 1) * text_desc->text.color3.a) >> 8;
           }
         else
           {
              params->type.text->color2.r = text_desc->common.color2.r;
              params->type.text->color2.g = text_desc->common.color2.g;
              params->type.text->color2.b = text_desc->common.color2.b;
              params->type.text->color2.a = text_desc->common.color2.a;
              params->type.text->color3.r = text_desc->text.color3.r;
              params->type.text->color3.g = text_desc->text.color3.g;
              params->type.text->color3.b = text_desc->text.color3.b;
              params->type.text->color3.a = text_desc->text.color3.a;
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
      case EDJE_PART_TYPE_SNAPSHOT:
      case EDJE_PART_TYPE_VECTOR:
        break;

      case EDJE_PART_TYPE_LIGHT:
      {
         Edje_Part_Description_Light *light_desc = (Edje_Part_Description_Light *)desc;

         _edje_calc_params_need_type_node(params);
         params->type.node->data[0] = light_desc->light.orientation.data[0];
         params->type.node->point.x = light_desc->light.position.point.x;
         params->type.node->point.y = light_desc->light.position.point.y;
         params->type.node->point.z = light_desc->light.position.point.z;

         break;
      }

      case EDJE_PART_TYPE_CAMERA:
      {
         Edje_Part_Description_Camera *camera_desc = (Edje_Part_Description_Camera *)desc;

         _edje_calc_params_need_type_node(params);
         params->type.node->data[0] = camera_desc->camera.orientation.data[0];
         params->type.node->point.x = camera_desc->camera.position.point.x;
         params->type.node->point.y = camera_desc->camera.position.point.y;
         params->type.node->point.z = camera_desc->camera.position.point.z;

         break;
      }

      case EDJE_PART_TYPE_MESH_NODE:
      {
         Edje_Part_Description_Mesh_Node *mesh_desc = (Edje_Part_Description_Mesh_Node *)desc;

         _edje_calc_params_need_type_node(params);
         params->type.node->frame = mesh_desc->mesh_node.mesh.frame;
         params->type.node->data[0] = mesh_desc->mesh_node.orientation.data[0];
         params->type.node->point.x = mesh_desc->mesh_node.position.point.x;
         params->type.node->point.y = mesh_desc->mesh_node.position.point.y;
         params->type.node->point.z = mesh_desc->mesh_node.position.point.z;
         params->type.node->scale_3d.x = mesh_desc->mesh_node.scale_3d.x;
         params->type.node->scale_3d.y = mesh_desc->mesh_node.scale_3d.y;
         params->type.node->scale_3d.z = mesh_desc->mesh_node.scale_3d.z;

         break;
      }

      case EDJE_PART_TYPE_GRADIENT:
        /* FIXME: THIS ONE SHOULD NEVER BE TRIGGERED. */
        break;

      default:
        break;
     }

   /* if we have text that wants to make the min size the text size... */
   if (ep->part->type == EDJE_PART_TYPE_TEXTBLOCK)
     _edje_part_recalc_single_textblock(sc, ed, ep, (Edje_Part_Description_Text *)chosen_desc, params, &minw, &minh, &maxw, &maxh);
   else if (ep->part->type == EDJE_PART_TYPE_TEXT)
     {
        _edje_part_recalc_single_text(sc, ed, ep, (Edje_Part_Description_Text*) desc, (Edje_Part_Description_Text*) chosen_desc, params, &minw, &minh, &maxw, &maxh);
        _edje_part_recalc_single_filter(ed, ep, desc, chosen_desc, pos);
     }

   if ((ep->part->type == EDJE_PART_TYPE_TABLE) &&
       (((((Edje_Part_Description_Table *)chosen_desc)->table.min.h) ||
         (((Edje_Part_Description_Table *)chosen_desc)->table.min.v))))
     {
        Eina_Size2D lmin;

        efl_canvas_group_need_recalculate_set(ep->object, 1);
        efl_canvas_group_calculate(ep->object);
        lmin = efl_gfx_hint_size_restricted_min_get(ep->object);
        if (((Edje_Part_Description_Table *)chosen_desc)->table.min.h)
          {
             if (lmin.w > minw) minw = lmin.w;
          }
        if (((Edje_Part_Description_Table *)chosen_desc)->table.min.v)
          {
             if (lmin.h > minh) minh = lmin.h;
          }
     }
   else if ((ep->part->type == EDJE_PART_TYPE_BOX) &&
            ((((Edje_Part_Description_Box *)chosen_desc)->box.min.h) ||
             (((Edje_Part_Description_Box *)chosen_desc)->box.min.v)))
     {
        Eina_Size2D lmin;

        efl_canvas_group_need_recalculate_set(ep->object, 1);
        efl_canvas_group_calculate(ep->object);
        lmin = efl_gfx_hint_size_restricted_min_get(ep->object);
        if (((Edje_Part_Description_Box *)chosen_desc)->box.min.h)
          {
             if (lmin.w > minw) minw = lmin.w;
          }
        if (((Edje_Part_Description_Box *)chosen_desc)->box.min.v)
          {
             if (lmin.h > minh) minh = lmin.h;
          }
     }
   else if (ep->part->type == EDJE_PART_TYPE_IMAGE)
     {
        if (chosen_desc->min.limit || chosen_desc->max.limit)
          {
             Evas_Coord w, h;

             /* We only need pos to find the right image that would be displayed */
             /* Yes, if someone set aspect preference to SOURCE and also max,min
           to SOURCE, it will be under efficient, but who cares at the
           moment. */
             _edje_real_part_image_set(ed, ep, NULL, pos);
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
        _edje_part_recalc_single_filter(ed, ep, desc, chosen_desc, pos);
     }
   else if (ep->part->type == EDJE_PART_TYPE_PROXY)
     {
        _edje_part_recalc_single_filter(ed, ep, desc, chosen_desc, pos);
     }
   else if (ep->part->type == EDJE_PART_TYPE_SNAPSHOT)
     {
        _edje_part_recalc_single_filter(ed, ep, desc, chosen_desc, pos);
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

   _edje_part_pixel_adjust(ed, ep, params);
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
   evas_obj_table_homogeneous_set(ep->object, chosen_desc->table.homogeneous);
   evas_obj_table_align_set(ep->object, TO_DOUBLE(chosen_desc->table.align.x), TO_DOUBLE(chosen_desc->table.align.y));
   evas_obj_table_padding_set(ep->object, chosen_desc->table.padding.x, chosen_desc->table.padding.y);
   if (evas_object_smart_need_recalculate_get(ep->object))
     {
        efl_canvas_group_need_recalculate_set(ep->object, 0);
        efl_canvas_group_calculate(ep->object);
     }
}

static void
_edje_proxy_recalc_apply(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *p3, Edje_Part_Description_Proxy *chosen_desc, FLOAT_T pos)
{
   Edje_Real_Part *pp;
   int part_id = -1;

   if (ep->param2 && (pos >= FROM_DOUBLE(0.5)))
     part_id = ((Edje_Part_Description_Proxy *)ep->param2->description)->proxy.id;
   else
     part_id = chosen_desc->proxy.id;

   if ((p3->type.common->fill.w == 0) || (p3->type.common->fill.h == 0) ||
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
           case EDJE_PART_TYPE_SNAPSHOT:
           case EDJE_PART_TYPE_VECTOR:
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

   efl_gfx_fill_set(ep->object, (Eina_Rect) p3->type.common->fill);
   efl_gfx_image_smooth_scale_set(ep->object, p3->smooth);
   evas_object_image_source_visible_set(ep->object, chosen_desc->proxy.source_visible);
   evas_object_image_source_clip_set(ep->object, chosen_desc->proxy.source_clip);
}

static void
_edje_image_recalc_apply(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *p3, Edje_Part_Description_Image *chosen_desc, FLOAT_T pos)
{
   FLOAT_T sc;
   Edje_Real_Part_Set *set;

   sc = DIV(ed->scale, ed->file->base_scale);
   if (EQ(sc, ZERO)) sc = DIV(_edje_scale, ed->file->base_scale);

   _edje_real_part_image_set(ed, ep, &set, pos);

   /* border */
   _edje_calc_params_need_type_common(p3);
   p3->type.common->spec.image.l = chosen_desc->image.border.l;
   p3->type.common->spec.image.r = chosen_desc->image.border.r;

   p3->type.common->spec.image.t = chosen_desc->image.border.t;
   p3->type.common->spec.image.b = chosen_desc->image.border.b;

//  XXX: do NOT do this. you can use border scale by to animate transitions
//   p3->type.common->spec.image.border_scale_by = chosen_desc->image.border.scale_by;

   if (set && set->set)
     {
#define SET_BORDER_DEFINED(Result, Value) Result = Value ? Value : Result;
        SET_BORDER_DEFINED(p3->type.common->spec.image.l, set->entry->border.l);
        SET_BORDER_DEFINED(p3->type.common->spec.image.r, set->entry->border.r);
        SET_BORDER_DEFINED(p3->type.common->spec.image.t, set->entry->border.t);
        SET_BORDER_DEFINED(p3->type.common->spec.image.b, set->entry->border.b);

        p3->type.common->spec.image.border_scale_by = NEQ(set->entry->border.scale_by, ZERO) ?
          set->entry->border.scale_by : p3->type.common->spec.image.border_scale_by;
     }

   efl_gfx_fill_set(ep->object, (Eina_Rect) p3->type.common->fill);
   efl_gfx_image_smooth_scale_set(ep->object, p3->smooth);
   if (chosen_desc->image.border.scale)
     {
        if (p3->type.common->spec.image.border_scale_by > FROM_DOUBLE(0.0))
          {
             FLOAT_T sc2 = MUL(sc, p3->type.common->spec.image.border_scale_by);
             evas_object_image_border_scale_set(ep->object, TO_DOUBLE(sc2));
          }
        else
          evas_object_image_border_scale_set(ep->object, TO_DOUBLE(sc));
     }
   else
     {
        if (p3->type.common->spec.image.border_scale_by > FROM_DOUBLE(0.0))
          evas_object_image_border_scale_set
            (ep->object, TO_DOUBLE(p3->type.common->spec.image.border_scale_by));
        else
          evas_object_image_border_scale_set(ep->object, 1.0);
     }
   evas_object_image_border_set(ep->object, p3->type.common->spec.image.l, p3->type.common->spec.image.r,
                                p3->type.common->spec.image.t, p3->type.common->spec.image.b);
   if (chosen_desc->image.border.no_fill == 0)
     evas_object_image_border_center_fill_set(ep->object, EVAS_BORDER_FILL_DEFAULT);
   else if (chosen_desc->image.border.no_fill == 1)
     evas_object_image_border_center_fill_set(ep->object, EVAS_BORDER_FILL_NONE);
   else if (chosen_desc->image.border.no_fill == 2)
     evas_object_image_border_center_fill_set(ep->object, EVAS_BORDER_FILL_SOLID);
}

static void
_edje_vector_animation_running_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Canvas_Animation_Player_Event_Running *event_running = event->info;
   double pos = event_running->progress;
   Edje_Real_Part *ep = (Edje_Real_Part *)data;
   int frame, frame_count;

   frame_count = efl_gfx_frame_controller_frame_count_get(ep->object) - 1;
   if (frame_count < 0) frame_count = 0;

   if (ep->typedata.vector->backward)
     frame = ep->typedata.vector->start_frame - (int) (frame_count * pos);
   else
     frame = ep->typedata.vector->start_frame + (int) (frame_count * pos);

   efl_gfx_frame_controller_frame_set(ep->object, frame);

   if ((ep->typedata.vector->backward && (frame <= 0)) ||
       (!ep->typedata.vector->backward && (frame >= frame_count)))
     {
        if (ep->typedata.vector->loop)
          {
             if (ep->typedata.vector->backward)
               ep->typedata.vector->start_frame = frame_count;
             else
               ep->typedata.vector->start_frame = 0;

             efl_player_start(ep->typedata.vector->player);
          }
        else
          {
             efl_player_stop(ep->typedata.vector->player);
          }
     }
}

static void
_edje_vector_load_json(Edje *ed, Edje_Real_Part *ep, const char *key)
{
   Edje_Part_Description_Vector *desc = (Edje_Part_Description_Vector *)ep->chosen_description;
   Eina_File *file;
   char *json_data;
   double frame_duration;
   int json_data_len = 0;
   int frame_count;

   if (ep->typedata.vector->anim == NULL)
     {
        Eo *anim = efl_add(EFL_CANVAS_ANIMATION_CLASS, ed->base.evas);
        ep->typedata.vector->anim = anim;
     }

   if (ep->typedata.vector->player == NULL)
     {
        Eo *player = efl_add(EFL_CANVAS_ANIMATION_PLAYER_CLASS, ed->base.evas);
        efl_animation_player_animation_set(player, ep->typedata.vector->anim);
        efl_event_callback_add(player, EFL_ANIMATION_PLAYER_EVENT_RUNNING,
                               _edje_vector_animation_running_cb, ep);
        ep->typedata.vector->player = player;
     }

   if (ep->typedata.vector->current_id != desc->vg.id)
     {
        json_data = (char *)eet_read(ed->file->ef, key, &json_data_len);
        json_data[json_data_len] = '\0';
        file = eina_file_virtualize(NULL, json_data, json_data_len + 1, EINA_FALSE);
        efl_file_simple_mmap_load(ep->object, file, NULL);

        if (ep->typedata.vector->json_virtual_file)
          eina_file_close(ep->typedata.vector->json_virtual_file);
        ep->typedata.vector->json_virtual_file = file;

        if (ep->typedata.vector->json_data)
          free(ep->typedata.vector->json_data);
        ep->typedata.vector->json_data = json_data;

        ep->typedata.vector->current_id = desc->vg.id;
     }

   frame_duration = efl_gfx_frame_controller_frame_duration_get(ep->object, 0, 0);
   efl_animation_duration_set(ep->typedata.vector->anim, frame_duration);

   if (!ep->typedata.vector->is_playing)
     {
        frame_count = efl_gfx_frame_controller_frame_count_get(ep->object);
        efl_gfx_frame_controller_frame_set(ep->object, (int)(frame_count * desc->vg.frame));
     }
}

static void
_edje_vector_recalc_apply(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *p3 EINA_UNUSED, Edje_Part_Description_Vector *chosen_desc, FLOAT_T pos)
{
   int new_id = -1; //invalid svg
   int w, h;
   char src_key[32], dest_key[32];
   Efl_VG *src_root, *dest_root, *root;
   Edje_Vector_File_Type type = chosen_desc->vg.type;
   Edje_Vector_File_Type new_type = EDJE_VECTOR_FILE_TYPE_SVG;

   evas_object_geometry_get(ep->object, NULL, NULL, &w, &h);
   if( (w == 0) || (h == 0)) return;

   snprintf(src_key, sizeof(src_key), "edje/vectors/%i", chosen_desc->vg.id);

   if (type == EDJE_VECTOR_FILE_TYPE_JSON)
     {
        _edje_vector_load_json(ed, ep, src_key);

        return;
     }

   if (ep->param2)
     {
        Edje_Part_Description_Vector *next_state = (Edje_Part_Description_Vector *)ep->param2->description;
        if (chosen_desc->vg.id != next_state->vg.id)
          {
             new_id = next_state->vg.id;
             new_type = next_state->vg.type;
          }
        else
          {
             pos = 0;
          }
     }

   if ((new_id < 0) || (new_type == EDJE_VECTOR_FILE_TYPE_JSON))
     {
        efl_file_simple_load(ep->object, ed->file->path, src_key);
     }
   else
     {
        snprintf(dest_key, sizeof(dest_key), "edje/vectors/%i", new_id);

        efl_file_simple_load(ep->object, ed->file->path, src_key);
        src_root = efl_canvas_vg_object_root_node_get(ep->object);
        efl_ref(src_root);

        efl_file_simple_load(ep->object, ed->file->path, dest_key);
        dest_root = efl_canvas_vg_object_root_node_get(ep->object);
        efl_ref(dest_root);

        root = efl_duplicate(src_root);

        if (!efl_gfx_path_interpolate(root, src_root, dest_root, pos))
          {
             ERR("Can't interpolate check the svg file");
          }
        efl_canvas_vg_object_root_node_set(ep->object, root);
        efl_unref(src_root);
        efl_unref(dest_root);
     }
}

void
_edje_part_vector_anim_stop(Edje *ed EINA_UNUSED, Edje_Real_Part *rp)
{
   Edje_Part_Description_Vector *desc = (Edje_Part_Description_Vector *)rp->chosen_description;
   double frame_count = efl_gfx_frame_controller_frame_count_get(rp->object);

   efl_player_stop(rp->typedata.vector->player);
   efl_gfx_frame_controller_frame_set(rp->object, (int)(frame_count * desc->vg.frame));
   rp->typedata.vector->is_playing = EINA_FALSE;
}

void
_edje_part_vector_anim_pause(Edje *ed EINA_UNUSED, Edje_Real_Part *rp)
{
   efl_player_stop(rp->typedata.vector->player);
}

void
_edje_part_vector_anim_resume(Edje *ed EINA_UNUSED, Edje_Real_Part *rp)
{
   if (rp->typedata.vector->is_playing)
     {
        rp->typedata.vector->start_frame = efl_gfx_frame_controller_frame_get(rp->object);
        efl_player_start(rp->typedata.vector->player);
     }
}

void
_edje_part_vector_anim_play(Edje *ed EINA_UNUSED, Edje_Real_Part *rp, Eina_Bool backward, Eina_Bool loop)
{
   rp->typedata.vector->backward = backward;
   rp->typedata.vector->loop = loop;
   rp->typedata.vector->start_frame = efl_gfx_frame_controller_frame_get(rp->object);
   rp->typedata.vector->is_playing = EINA_TRUE;
   efl_player_start(rp->typedata.vector->player);
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
             (void)state;
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
   if (!EPH_LOAD()) return EINA_FALSE;
   EPH_CALL(ephysics_world_render_geometry_get)(world, NULL, NULL, NULL, &w, &h, &d);
   return w && h && d;
}

static void
_edje_physics_body_props_update(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *pf,
                                Eina_Bool pos_update)
{
   if (!EPH_LOAD()) return;
   _edje_calc_params_need_ext(pf);
   EPH_CALL(ephysics_body_linear_movement_enable_set)
     (ep->body,
      pf->ext->physics->mov_freedom.lin.x,
      pf->ext->physics->mov_freedom.lin.y,
      pf->ext->physics->mov_freedom.lin.z);
   EPH_CALL(ephysics_body_angular_movement_enable_set)
     (ep->body,
      pf->ext->physics->mov_freedom.ang.x,
      pf->ext->physics->mov_freedom.ang.y,
      pf->ext->physics->mov_freedom.ang.z);

   /* Boundaries geometry and mass shouldn't be changed */
   if (ep->part->physics_body < EDJE_PART_PHYSICS_BODY_BOUNDARY_TOP)
     {
        Evas_Coord x, y, z, w, h, d;

        if (pos_update)
          {
             EPH_CALL(ephysics_body_move)
               (ep->body,
                ed->x + pf->final.x,
                ed->y + pf->final.y,
                pf->ext->physics->z);
             ep->x = pf->final.x;
             ep->y = pf->final.y;
             ep->w = pf->final.w;
             ep->h = pf->final.h;
          }

        EPH_CALL(ephysics_body_geometry_get)(ep->body, &x, &y, &z, &w, &h, &d);
        if ((d) && (d != pf->ext->physics->depth))
          EPH_CALL(ephysics_body_resize)(ep->body, w, h, pf->ext->physics->depth);
        if (z != pf->ext->physics->z)
          EPH_CALL(ephysics_body_move)(ep->body, x, y, pf->ext->physics->z);

        EPH_CALL(ephysics_body_material_set)(ep->body, pf->ext->physics->material);
        if (!pf->ext->physics->material)
          {
             if (NEQ(pf->ext->physics->density, ZERO))
               EPH_CALL(ephysics_body_density_set)(ep->body, pf->ext->physics->density);
             else
               EPH_CALL(ephysics_body_mass_set)(ep->body, pf->ext->physics->mass);
          }

        if ((ep->part->physics_body == EDJE_PART_PHYSICS_BODY_SOFT_BOX) ||
            (ep->part->physics_body == EDJE_PART_PHYSICS_BODY_SOFT_SPHERE) ||
            (ep->part->physics_body == EDJE_PART_PHYSICS_BODY_SOFT_CYLINDER) ||
            (ep->part->physics_body == EDJE_PART_PHYSICS_BODY_CLOTH))
          EPH_CALL(ephysics_body_soft_body_hardness_set)
            (ep->body, pf->ext->physics->hardness * 100);
     }

   if (!pf->ext->physics->material)
     {
        EPH_CALL(ephysics_body_restitution_set)(ep->body, pf->ext->physics->restitution);
        EPH_CALL(ephysics_body_friction_set)(ep->body, pf->ext->physics->friction);
     }

   EPH_CALL(ephysics_body_damping_set)(ep->body, pf->ext->physics->damping.linear,
                                       pf->ext->physics->damping.angular);
   EPH_CALL(ephysics_body_sleeping_threshold_set)(ep->body, pf->ext->physics->sleep.linear,
                                                  pf->ext->physics->sleep.angular);
   EPH_CALL(ephysics_body_light_set)(ep->body, pf->ext->physics->light_on);
   EPH_CALL(ephysics_body_back_face_culling_set)(ep->body, pf->ext->physics->backcull);
}

static void
_edje_physics_body_update_cb(void *data, EPhysics_Body *body, void *event_info EINA_UNUSED)
{
   Edje_Real_Part *rp = data;
   Edje *ed;

   if (!EPH_LOAD()) return;
   ed = EPH_CALL(ephysics_body_data_get(body));
   EPH_CALL(ephysics_body_geometry_get)(body, &(rp->x), &(rp->y), NULL,
                                        &(rp->w), &(rp->h), NULL);
   EPH_CALL(ephysics_body_evas_object_update)(body);
   ed->dirty = EINA_TRUE;
}

static void
_edje_physics_body_add(Edje *ed, Edje_Real_Part *rp, EPhysics_World *world)
{
   Eina_Bool resize = EINA_TRUE;
   Edje_Physics_Face *pface;
   Eina_List *l;

   if (!EPH_LOAD()) return;
   switch (rp->part->physics_body)
     {
      case EDJE_PART_PHYSICS_BODY_RIGID_BOX:
        rp->body = EPH_CALL(ephysics_body_box_add)(world);
        break;

      case EDJE_PART_PHYSICS_BODY_RIGID_SPHERE:
        rp->body = EPH_CALL(ephysics_body_sphere_add)(world);
        break;

      case EDJE_PART_PHYSICS_BODY_RIGID_CYLINDER:
        rp->body = EPH_CALL(ephysics_body_cylinder_add)(world);
        break;

      case EDJE_PART_PHYSICS_BODY_SOFT_BOX:
        rp->body = EPH_CALL(ephysics_body_soft_box_add)(world);
        break;

      case EDJE_PART_PHYSICS_BODY_SOFT_SPHERE:
        rp->body = EPH_CALL(ephysics_body_soft_sphere_add)(world, 0);
        break;

      case EDJE_PART_PHYSICS_BODY_SOFT_CYLINDER:
        rp->body = EPH_CALL(ephysics_body_soft_cylinder_add)(world);
        break;

      case EDJE_PART_PHYSICS_BODY_CLOTH:
        rp->body = EPH_CALL(ephysics_body_cloth_add)(world, 0, 0);
        break;

      case EDJE_PART_PHYSICS_BODY_BOUNDARY_TOP:
        rp->body = EPH_CALL(ephysics_body_top_boundary_add)(world);
        resize = EINA_FALSE;
        break;

      case EDJE_PART_PHYSICS_BODY_BOUNDARY_BOTTOM:
        rp->body = EPH_CALL(ephysics_body_bottom_boundary_add)(world);
        resize = EINA_FALSE;
        break;

      case EDJE_PART_PHYSICS_BODY_BOUNDARY_RIGHT:
        rp->body = EPH_CALL(ephysics_body_right_boundary_add)(world);
        resize = EINA_FALSE;
        break;

      case EDJE_PART_PHYSICS_BODY_BOUNDARY_LEFT:
        rp->body = EPH_CALL(ephysics_body_left_boundary_add)(world);
        resize = EINA_FALSE;
        break;

      case EDJE_PART_PHYSICS_BODY_BOUNDARY_FRONT:
        rp->body = EPH_CALL(ephysics_body_front_boundary_add)(world);
        resize = EINA_FALSE;
        break;

      case EDJE_PART_PHYSICS_BODY_BOUNDARY_BACK:
        rp->body = EPH_CALL(ephysics_body_back_boundary_add)(world);
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
        EPH_CALL(ephysics_body_face_evas_object_set)(rp->body, pface->type,
                                                     edje_obj, EINA_FALSE);
        rp->body_faces = eina_list_append(rp->body_faces, edje_obj);
     }

   EPH_CALL(ephysics_body_evas_object_set)(rp->body, rp->object, resize);
   EPH_CALL(ephysics_body_event_callback_add)(rp->body, EPHYSICS_CALLBACK_BODY_UPDATE,
                                              _edje_physics_body_update_cb, rp);
   EPH_CALL(ephysics_body_data_set)(rp->body, ed);
}

#endif

#define FINTP(_x1, _x2, _p)                     \
  ((((int)_x1) == ((int)_x2))                   \
   ? FROM_INT((_x1))                            \
   : ADD(FROM_INT(_x1),                         \
         SCALE((_p), (_x2) - (_x1))))

#define FFP(_x1, _x2, _p) \
  (EQ((_x1), (_x2))       \
   ? (_x1)                \
   : ADD(_x1, MUL(_p, SUB(_x2, _x1))));

#define INTP(_x1, _x2, _p) TO_INT(FINTP(_x1, _x2, _p))

static void
_map_colors_free(Edje_Calc_Params *pf)
{
   if (pf->ext)
     {
        Edje_Map_Color **colors = pf->ext->map->colors;
        int i;

        for (i = 0; i < (int)pf->ext->map->colors_count; i++) free(colors[i]);
        free(colors);
     }
}

static Eina_Bool
_map_colors_interp(Edje_Calc_Params *p1, Edje_Calc_Params *p2,
                   Edje_Calc_Params_Map *pmap, FLOAT_T pos)
{
   Edje_Map_Color *col = NULL, *col2 = NULL, *col3;
   int i, j;

   unsigned char col1_r = 255, col1_g = 255, col1_b = 255, col1_a = 255;
   unsigned char col2_r = 255, col2_g = 255, col2_b = 255, col2_a = 255;

   _edje_calc_params_need_ext(p1);
   _edje_calc_params_need_ext(p2);

   if ((p1->ext->map->colors_count > 0) || (p2->ext->map->colors_count > 0))
     {
        pmap->colors_count = (p1->ext->map->colors_count > p2->ext->map->colors_count ? p1->ext->map->colors_count : p2->ext->map->colors_count);

        pmap->colors = (Edje_Map_Color **)malloc(sizeof(Edje_Map_Color *) * (int)pmap->colors_count);

        /* create all Map Color structs at first
           to make sure we won't get SIGSEV later on cleanup. */
        for (i = 0; i < (int)pmap->colors_count; i++)
          {
             col3 = malloc(sizeof(Edje_Map_Color));
             col3->idx = i; /* we don't care about index position anyway */

             /* find color with idx from first */
             for (j = 0; j < (int)p1->ext->map->colors_count; j++)
               {
                  col = p1->ext->map->colors[j];
                  if (col3->idx == col->idx)
                    {
                       col1_r = col->r;
                       col1_g = col->g;
                       col1_b = col->b;
                       col1_a = col->a;
                       break;
                    }
               }
             /* find color from idx from second */
             for (j = 0; j < (int)p2->ext->map->colors_count; j++)
               {
                  col2 = p2->ext->map->colors[j];
                  if (col3->idx == col2->idx)
                    {
                       col2_r = col2->r;
                       col2_g = col2->g;
                       col2_b = col2->b;
                       col2_a = col2->a;
                       break;
                    }
               }

             /* interpolate!
                if color didn't existed, then there are default 255 values */
             col3->r = INTP(col1_r, col2_r, pos);
             col3->g = INTP(col1_g, col2_g, pos);
             col3->b = INTP(col1_b, col2_b, pos);
             col3->a = INTP(col1_a, col2_a, pos);

             pmap->colors[i] = col3;
          }

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static void
_edje_map_prop_set(Evas_Map *map, const Edje_Calc_Params *pf,
                   Edje_Part_Description_Common *chosen_desc,
                   Edje_Real_Part *ep, Evas_Object *mo, Evas_Object *populate_obj)
{
   Edje_Map_Color *color, **colors;
   int colors_cnt, i;

   if (!pf->ext) return;

   colors = pf->ext->map->colors;
   colors_cnt = pf->ext->map->colors_count;

   evas_map_util_points_populate_from_object(map, populate_obj ?: ep->object);

   if (ep->part->type == EDJE_PART_TYPE_IMAGE ||
       ((ep->part->type == EDJE_PART_TYPE_SWALLOW) &&
        (efl_isa(mo, EFL_CANVAS_IMAGE_INTERNAL_CLASS) &&
         (!evas_object_image_source_get(mo))))
       )
     {
        int iw = 1, ih = 1;

        evas_object_image_size_get(mo, &iw, &ih);
        evas_map_point_image_uv_set(map, 0, 0.0, 0.0);
        evas_map_point_image_uv_set(map, 1, iw, 0.0);
        evas_map_point_image_uv_set(map, 2, iw, ih);
        evas_map_point_image_uv_set(map, 3, 0.0, ih);
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

   //zoom
   evas_map_util_zoom(map,
                      pf->ext->map->zoom.x, pf->ext->map->zoom.y,
                      pf->ext->map->center.x, pf->ext->map->center.y);

   //rotate
   evas_map_util_3d_rotate(map,
                           TO_DOUBLE(pf->ext->map->rotation.x),
                           TO_DOUBLE(pf->ext->map->rotation.y),
                           TO_DOUBLE(pf->ext->map->rotation.z),
                           pf->ext->map->center.x, pf->ext->map->center.y,
                           pf->ext->map->center.z);

   // calculate light color & position etc. if there is one
   if (pf->lighted)
     {
        evas_map_util_3d_lighting(map, pf->ext->map->light.x, pf->ext->map->light.y,
                                  pf->ext->map->light.z, pf->ext->map->light.r,
                                  pf->ext->map->light.g, pf->ext->map->light.b,
                                  pf->ext->map->light.ar, pf->ext->map->light.ag,
                                  pf->ext->map->light.ab);
     }

   // calculate perspective point
   if (chosen_desc->map.persp_on)
     {
        evas_map_util_3d_perspective(map,
                                     pf->ext->map->persp.x, pf->ext->map->persp.y,
                                     pf->ext->map->persp.z, pf->ext->map->persp.focal);
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
static Eina_Bool
_circular_dependency_find(Edje *ed, Edje_Real_Part *ep, Edje_Real_Part *cep, Eina_Array *arr)
{
   Edje_Real_Part *rp = NULL;

   if (cep == ep) return EINA_TRUE;
   if (!cep) cep = ep;

   if ((cep->calculating & FLAG_X))
     {
        if (cep->param1.description)
          {
             if (cep->param1.description->rel1.id_x >= 0)
               {
                  rp = ed->table_parts[cep->param1.description->rel1.id_x];
                  if (_circular_dependency_find(ed, ep, rp, arr))
                    {
                       eina_array_push(arr, eina_stringshare_add(rp->part->name));
                       return EINA_TRUE;
                    }
               }
             if (cep->param1.description->rel2.id_x >= 0)
               {
                  rp = ed->table_parts[cep->param1.description->rel2.id_x];
                  if (_circular_dependency_find(ed, ep, rp, arr))
                    {
                       eina_array_push(arr, eina_stringshare_add(rp->part->name));
                       return EINA_TRUE;
                    }
               }
          }

        if (cep->param2)
          {
             if (cep->param2->description->rel1.id_x >= 0)
               {
                  rp = ed->table_parts[cep->param2->description->rel1.id_x];
                  if (_circular_dependency_find(ed, ep, rp, arr))
                    {
                       eina_array_push(arr, eina_stringshare_add(rp->part->name));
                       return EINA_TRUE;
                    }
               }
             if (cep->param2->description->rel2.id_x >= 0)
               {
                  rp = ed->table_parts[cep->param2->description->rel2.id_x];
                  if (_circular_dependency_find(ed, ep, rp, arr))
                    {
                       eina_array_push(arr, eina_stringshare_add(rp->part->name));
                       return EINA_TRUE;
                    }
               }
          }
     }
   if ((cep->calculating & FLAG_Y))
     {
        if (cep->param1.description)
          {
             if (cep->param1.description->rel1.id_y >= 0)
               {
                  rp = ed->table_parts[cep->param1.description->rel1.id_y];
                  if (_circular_dependency_find(ed, ep, rp, arr))
                    {
                       eina_array_push(arr, eina_stringshare_add(rp->part->name));
                       return EINA_TRUE;
                    }
               }
             if (cep->param1.description->rel2.id_y >= 0)
               {
                  rp = ed->table_parts[cep->param1.description->rel2.id_y];
                  if (_circular_dependency_find(ed, ep, rp, arr))
                    {
                       eina_array_push(arr, eina_stringshare_add(rp->part->name));
                       return EINA_TRUE;
                    }
               }
          }
        if (cep->param2)
          {
             if (cep->param2->description->rel1.id_y >= 0)
               {
                  rp = ed->table_parts[cep->param2->description->rel1.id_y];
                  if (_circular_dependency_find(ed, ep, rp, arr))
                    {
                       eina_array_push(arr, eina_stringshare_add(rp->part->name));
                       return EINA_TRUE;
                    }
               }
             if (cep->param2->description->rel2.id_y >= 0)
               {
                  rp = ed->table_parts[cep->param2->description->rel2.id_y];
                  if (_circular_dependency_find(ed, ep, rp, arr))
                    {
                       eina_array_push(arr, eina_stringshare_add(rp->part->name));
                       return EINA_TRUE;
                    }
               }
          }
     }

   return EINA_FALSE;
}

static void
_edje_part_calc_params_memcpy(Edje_Calc_Params *p, Edje_Calc_Params *s, Edje_Part_Type t)
{
   _edje_calc_params_clear(p);
   memcpy(p, s, sizeof(Edje_Calc_Params));
   if (s->ext)
     {
        p->ext = NULL;
        _edje_calc_params_need_ext(p);
#ifdef EDJE_CALC_CACHE
        eina_cow_memcpy(_edje_calc_params_map_cow,
                        (const Eina_Cow_Data **)&(p->ext->map),
                        s->ext->map);
# ifdef HAVE_EPHYSICS
        eina_cow_memcpy(_edje_calc_params_physics_cow,
                        (const Eina_Cow_Data **)&(p->ext->physics),
                        s->ext->physics);
# endif
#endif
     }
   switch (t)
     {
      case EDJE_PART_TYPE_IMAGE:
          {
             Edje_Calc_Params_Type_Common *d = malloc(sizeof(*d));
             if (d)
               {
                  memcpy(d, s->type.common, sizeof(*d));
                  p->type.common = d;
               }
             else p->type.common = NULL;
          }
        break;
      case EDJE_PART_TYPE_TEXT:
      case EDJE_PART_TYPE_TEXTBLOCK:
          {
             Edje_Calc_Params_Type_Text *d = malloc(sizeof(*d));
             if (d)
               {
                  memcpy(d, s->type.text, sizeof(*d));
                  p->type.text = d;
               }
             else p->type.text = NULL;
          }
        break;
      case EDJE_PART_TYPE_LIGHT:
      case EDJE_PART_TYPE_CAMERA:
      case EDJE_PART_TYPE_MESH_NODE:
          {
             Edje_Calc_Params_Type_Node *d = malloc(sizeof(*d));
             if (d)
               {
                  memcpy(d, s->type.node, sizeof(*d));
                  p->type.node = d;
               }
             else p->type.node = NULL;
          }
        break;
      default:
        break;
     }
}

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
   int statecl = -1;
   Edje_Real_Part *center[2] = { NULL, NULL };
   Edje_Real_Part *light[2] = { NULL, NULL };
   Edje_Real_Part *persp[2] = { NULL, NULL };
   Edje_Real_Part *rp1[4] = { NULL, NULL, NULL, NULL };
   Edje_Real_Part *rp2[4] = { NULL, NULL, NULL, NULL };
   Edje_Real_Part *clip1 = NULL, *clip2 = NULL;
   Edje_Calc_Params *p1, *pf;
   Edje_Part_Description_Common *chosen_desc;
   Edje_Real_Part *confine_to = NULL;
   Edje_Real_Part *threshold = NULL;
   FLOAT_T pos = ZERO, pos2;
   Edje_Calc_Params lp3 = { { 0 } };
   Evas_Coord mmw = 0, mmh = 0;
   Eina_Bool map_colors_free = EINA_FALSE;

#ifdef EDJE_CALC_CACHE
#else
   lp1.type.common = NULL;
   lp1.ext = NULL;

   lp2.type.common = NULL;
   lp2.ext = NULL;
#endif

   lp3.type.common = NULL;
   lp3.ext = NULL;

   /* GRADIENT ARE GONE, WE MUST IGNORE IT FROM OLD FILE. */
   if (ep->part->type == EDJE_PART_TYPE_GRADIENT)
     {
        ERR("GRADIENT spotted during recalc ! That should never happen ! Send your edje file to devel ml.");
        return;
     }

   EINA_SAFETY_ON_FALSE_RETURN(flags != FLAG_NONE);
   if (!state)
     {
        if ((ep->calculated & FLAG_XY) == FLAG_XY) return;
        if ((ep->calculated & flags) == flags) return;
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

        Eina_Array *part_array = eina_array_new(10);;

        if (_circular_dependency_find(ed, ep, NULL, part_array))
          {
             Eina_Array_Iterator it;
             unsigned int i;
             char *part_name;
             char depends_path[PATH_MAX] = "";

             efl_event_callback_legacy_call(ed->obj, EFL_LAYOUT_EVENT_CIRCULAR_DEPENDENCY, part_array);
             strncat(depends_path, ep->part->name,
                     sizeof(depends_path) - strlen(depends_path) - 1);
             EINA_ARRAY_ITER_NEXT(part_array, i, part_name, it)
               {
                  strncat(depends_path, " -> ",
                          sizeof(depends_path) - strlen(depends_path) - 1);
                  strncat(depends_path, part_name,
                          sizeof(depends_path) - strlen(depends_path) - 1);
                  eina_stringshare_del(part_name);
               }
             ERR("Circular dependency in the group '%s' : %s",
                 ed->group, depends_path);
             eina_array_free(part_array);
          }
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
        Edje_Size *min = NULL, *max = NULL;

        if (ep->chosen_description)
          {
             min = &ep->chosen_description->min;
             max = &ep->chosen_description->max;
          }
        if (min && max && (min->w == max->w) && (min->h == max->h) && (min->w > 0) && (min->h > 0))
          mmw = min->w, mmh = min->h;
        else
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
        if (ep->param1.description->clip_to_id >= 0)
          {
             clip1 = ed->table_parts[ep->param1.description->clip_to_id % ed->table_parts_size];
             ed->has_state_clip = EINA_TRUE;
          }
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
        if (ep->param2->description->clip_to_id >= 0)
          {
             clip2 = ed->table_parts[ep->param2->description->clip_to_id % ed->table_parts_size];
             ed->has_state_clip = EINA_TRUE;
          }
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
   if (clip1)
     {
        _edje_part_recalc(ed, clip1, flags, NULL);
        statecl = clip1->state;
     }
   if (clip2)
     {
        _edje_part_recalc(ed, clip2, flags, NULL);
        if (statecl < clip2->state)
          statecl = clip2->state;
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

   if ((ep->part->type == EDJE_PART_TYPE_PROXY) && chosen_desc->visible)
     {
        int part_id = -1;

        if (ep->param2 && (pos >= FROM_DOUBLE(0.5)))
          part_id = ((Edje_Part_Description_Proxy *)ep->param2->description)->proxy.id;
        else
          part_id = ((Edje_Part_Description_Proxy *)chosen_desc)->proxy.id;

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
            (statecl >= ep->param1.state) ||
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
                                    rp1[Rel2X], rp1[Rel2Y], clip1, confine_to,
                                    threshold, p1, mmw, mmh,
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
             _edje_calc_params_clear(p1);
             _edje_part_calc_params_memcpy(p1, ep->current, ep->part->type);
          }

        p3 = &lp3;
#ifndef EDJE_CALC_CACHE
        p2 = &lp2;
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
            (statecl >= ep->param2->state) ||
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
                                    rp2[Rel2X], rp2[Rel2Y], clip2, confine_to,
                                    threshold, p2, mmw, mmh,
                                    pos);

#ifdef EDJE_CALC_CACHE
           if (flags == FLAG_XY) ep->param2->state = ed->state;
#endif
        }

        pos2 = pos;
        if (pos2 < ZERO) pos2 = ZERO;
        else if (pos2 > FROM_INT(1))
          pos2 = FROM_INT(1);
        beginning_pos = (pos < FROM_DOUBLE(0.5));
        part_type = ep->part->type;

        /* visible is special */
        if ((p1->visible) && (!p2->visible))
          p3->visible = NEQ(pos, FROM_INT(1));
        else if ((!p1->visible) && (p2->visible))
          p3->visible = NEQ(pos, ZERO);
        else
          p3->visible = p1->visible;

        /* clip_to will behave a bit like visible */
        if (EQ(pos, ZERO))
          {
             if ((p1->ext) && (p1->ext->clip_to))
               {
                  _edje_calc_params_need_ext(p3);
                  p3->ext->clip_to = p1->ext->clip_to;
               }
          }
        else if (EQ(pos, FROM_INT(1)))
          {
             if ((p2->ext) && (p2->ext->clip_to))
               {
                  _edje_calc_params_need_ext(p3);
                  p3->ext->clip_to = p2->ext->clip_to;
               }
          }
        else if ((!p1->ext) || (!p1->ext->clip_to))
          {
             if ((p2->ext) && (p2->ext->clip_to))
               {
                  _edje_calc_params_need_ext(p3);
                  p3->ext->clip_to = p2->ext->clip_to;
               }
             else
               {
                  if (p3->ext) p3->ext->clip_to = NULL;
               }
          }
        else
          {
             if ((p1->ext) && (p1->ext->clip_to))
               {
                  _edje_calc_params_need_ext(p3);
                  p3->ext->clip_to = p1->ext->clip_to;
               }
             else
               {
                  if (p3->ext) p3->ext->clip_to = NULL;
               }
          }

        /* and no_render as well */
        if ((p1->no_render) && (!p2->no_render))
          {
             p3->no_render = EQ(pos, FROM_INT(1));
             p3->no_render_apply = 1;
          }
        else if ((!p1->no_render) && (p2->no_render))
          {
             p3->no_render = EQ(pos, ZERO);
             p3->no_render_apply = 1;
          }
        else if (p1->no_render != ep->part->no_render)
          {
             p3->no_render = p1->no_render;
             p3->no_render_apply = 1;
          }
        else p3->no_render = ep->part->no_render;

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

        _edje_part_pixel_adjust(ed, ep, p3);

        if (ep->part->dragable.x)
          {
             _edje_calc_params_need_ext(p3);
             p3->ext->req_drag.x = INTP(p1->ext->req_drag.x, p2->ext->req_drag.x, pos);
             p3->ext->req_drag.w = INTP(p1->ext->req_drag.w, p2->ext->req_drag.w, pos);
          }
        if (ep->part->dragable.y)
          {
             _edje_calc_params_need_ext(p3);
             p3->ext->req_drag.y = INTP(p1->ext->req_drag.y, p2->ext->req_drag.y, pos);
             p3->ext->req_drag.h = INTP(p1->ext->req_drag.h, p2->ext->req_drag.h, pos);
          }

        p3->color.r = INTP(p1->color.r, p2->color.r, pos2);
        p3->color.g = INTP(p1->color.g, p2->color.g, pos2);
        p3->color.b = INTP(p1->color.b, p2->color.b, pos2);
        p3->color.a = INTP(p1->color.a, p2->color.a, pos2);

#ifdef HAVE_EPHYSICS
        if (ep->part->physics_body || ep->body)
          {
             _edje_calc_params_need_ext(p1);
             _edje_calc_params_need_ext(p2);
             EINA_COW_CALC_PHYSICS_BEGIN(p3, p3_write)
             {
                p3_write->mass = FFP(p1->ext->physics->mass,
                                     p2->ext->physics->mass,
                                     pos);
                p3_write->restitution = FFP(p1->ext->physics->restitution,
                                            p2->ext->physics->restitution,
                                            pos);
                p3_write->friction = FFP(p1->ext->physics->friction,
                                         p2->ext->physics->friction,
                                         pos);
                p3_write->density = FFP(p1->ext->physics->density,
                                        p2->ext->physics->density,
                                        pos);
                p3_write->hardness = FFP(p1->ext->physics->hardness,
                                         p2->ext->physics->hardness,
                                         pos);

                p3_write->damping.linear = FFP(p1->ext->physics->damping.linear,
                                               p2->ext->physics->damping.linear,
                                               pos);
                p3_write->damping.angular = FFP(p1->ext->physics->damping.angular,
                                                p2->ext->physics->damping.angular,
                                                pos);

                p3_write->sleep.linear = FFP(p1->ext->physics->sleep.linear,
                                             p2->ext->physics->sleep.linear,
                                             pos);
                p3_write->sleep.angular = FFP(p1->ext->physics->sleep.angular,
                                              p2->ext->physics->sleep.angular,
                                              pos);

                p3_write->z = INTP(p1->ext->physics->z, p2->ext->physics->z, pos);
                p3_write->depth = INTP(p1->ext->physics->depth, p2->ext->physics->depth, pos);

                if ((p1->ext->physics->ignore_part_pos) && (p2->ext->physics->ignore_part_pos))
                  p3_write->ignore_part_pos = 1;
                else
                  p3_write->ignore_part_pos = 0;

                if ((p1->ext->physics->material) && (p2->ext->physics->material))
                  p3_write->material = p1->ext->physics->material;
                else
                  p3_write->material = EPHYSICS_BODY_MATERIAL_CUSTOM;

                p3_write->light_on = p1->ext->physics->light_on || p2->ext->physics->light_on;
                p3_write->backcull = p1->ext->physics->backcull || p2->ext->physics->backcull;

                p3_write->mov_freedom.lin.x = p1->ext->physics->mov_freedom.lin.x ||
                  p2->ext->physics->mov_freedom.lin.x;
                p3_write->mov_freedom.lin.y = p1->ext->physics->mov_freedom.lin.y ||
                  p2->ext->physics->mov_freedom.lin.y;
                p3_write->mov_freedom.lin.z = p1->ext->physics->mov_freedom.lin.z ||
                  p2->ext->physics->mov_freedom.lin.z;
                p3_write->mov_freedom.ang.x = p1->ext->physics->mov_freedom.ang.x ||
                  p2->ext->physics->mov_freedom.ang.x;
                p3_write->mov_freedom.ang.y = p1->ext->physics->mov_freedom.ang.y ||
                  p2->ext->physics->mov_freedom.ang.y;
                p3_write->mov_freedom.ang.z = p1->ext->physics->mov_freedom.ang.z ||
                  p2->ext->physics->mov_freedom.ang.z;
             }
             EINA_COW_CALC_PHYSICS_END(p3, p3_write);
          }
#endif

        switch (part_type)
          {
           case EDJE_PART_TYPE_IMAGE:
             _edje_calc_params_need_type_common(p3);
             p3->type.common->spec.image.l = INTP(p1->type.common->spec.image.l, p2->type.common->spec.image.l, pos);
             p3->type.common->spec.image.r = INTP(p1->type.common->spec.image.r, p2->type.common->spec.image.r, pos);
             p3->type.common->spec.image.t = INTP(p1->type.common->spec.image.t, p2->type.common->spec.image.t, pos);
             p3->type.common->spec.image.b = INTP(p1->type.common->spec.image.b, p2->type.common->spec.image.b, pos);
             p3->type.common->spec.image.border_scale_by = FFP(p1->type.common->spec.image.border_scale_by, p2->type.common->spec.image.border_scale_by, pos);
             EINA_FALLTHROUGH;

             /* No break as proxy and image share code and object. */
           case EDJE_PART_TYPE_PROXY:
             _edje_calc_params_need_type_common(p3);
             p3->type.common->fill.x = INTP(p1->type.common->fill.x, p2->type.common->fill.x, pos);
             p3->type.common->fill.y = INTP(p1->type.common->fill.y, p2->type.common->fill.y, pos);
             p3->type.common->fill.w = INTP(p1->type.common->fill.w, p2->type.common->fill.w, pos);
             p3->type.common->fill.h = INTP(p1->type.common->fill.h, p2->type.common->fill.h, pos);
             break;

           case EDJE_PART_TYPE_TEXT:
             _edje_calc_params_need_type_text(p3);
             p3->type.text->size = INTP(p1->type.text->size, p2->type.text->size, pos);
             EINA_FALLTHROUGH;

             /* no break as we share code with the TEXTBLOCK type here. */
           case EDJE_PART_TYPE_TEXTBLOCK:
             _edje_calc_params_need_type_text(p3);
             p3->type.text->color2.r = INTP(p1->type.text->color2.r, p2->type.text->color2.r, pos2);
             p3->type.text->color2.g = INTP(p1->type.text->color2.g, p2->type.text->color2.g, pos2);
             p3->type.text->color2.b = INTP(p1->type.text->color2.b, p2->type.text->color2.b, pos2);
             p3->type.text->color2.a = INTP(p1->type.text->color2.a, p2->type.text->color2.a, pos2);

             p3->type.text->color3.r = INTP(p1->type.text->color3.r, p2->type.text->color3.r, pos2);
             p3->type.text->color3.g = INTP(p1->type.text->color3.g, p2->type.text->color3.g, pos2);
             p3->type.text->color3.b = INTP(p1->type.text->color3.b, p2->type.text->color3.b, pos2);
             p3->type.text->color3.a = INTP(p1->type.text->color3.a, p2->type.text->color3.a, pos2);

             p3->type.text->align.x = FFP(p1->type.text->align.x, p2->type.text->align.x, pos);
             p3->type.text->align.y = FFP(p1->type.text->align.y, p2->type.text->align.y, pos);
             p3->type.text->ellipsis = TO_DOUBLE(FINTP(p1->type.text->ellipsis, p2->type.text->ellipsis, pos2));
             break;
           case EDJE_PART_TYPE_MESH_NODE:
             _edje_calc_params_need_type_node(p3);
             p3->type.node->frame = INTP(p1->type.node->frame, p2->type.node->frame, pos);
             p3->type.node->data[0] = INTP(p1->type.node->data[0], p2->type.node->data[0], pos);

             p3->type.node->point.x = FFP(p1->type.node->point.x, p2->type.node->point.x, pos);
             p3->type.node->point.y = FFP(p1->type.node->point.y, p2->type.node->point.y, pos);
             p3->type.node->point.z = FFP(p1->type.node->point.z, p2->type.node->point.z, pos);

             p3->type.node->scale_3d.x = FFP(p1->type.node->scale_3d.x, p2->type.node->scale_3d.x, pos);
             p3->type.node->scale_3d.y = FFP(p1->type.node->scale_3d.y, p2->type.node->scale_3d.y, pos);
             p3->type.node->scale_3d.z = FFP(p1->type.node->scale_3d.z, p2->type.node->scale_3d.z, pos);
             break;
           case EDJE_PART_TYPE_CAMERA:
             p3->type.node->data[0] = FFP(p1->type.node->data[0], p2->type.node->data[0], pos);

             p3->type.node->point.x = FFP(p1->type.node->point.x, p2->type.node->point.x, pos);
             p3->type.node->point.y = FFP(p1->type.node->point.y, p2->type.node->point.y, pos);
             p3->type.node->point.z = FFP(p1->type.node->point.z, p2->type.node->point.z, pos);
             break;
           case EDJE_PART_TYPE_LIGHT:
             p3->type.node->data[0] = FFP(p1->type.node->data[0], p2->type.node->data[0], pos);

             p3->type.node->point.x = FFP(p1->type.node->point.x, p2->type.node->point.x, pos);
             p3->type.node->point.y = FFP(p1->type.node->point.y, p2->type.node->point.y, pos);
             p3->type.node->point.z = FFP(p1->type.node->point.z, p2->type.node->point.z, pos);
             break;
          }

        /* mapped is a special case like visible */
        if ((p1->mapped) && (!p2->mapped))
          p3->mapped = NEQ(pos, FROM_INT(1));
        else if ((!p1->mapped) && (p2->mapped))
          p3->mapped = NEQ(pos, ZERO);
        else
          p3->mapped = p1->mapped;

        p3->persp_on = p3->mapped ? p1->persp_on | p2->persp_on : 0;
        p3->lighted = p3->mapped ? p1->lighted | p2->lighted : 0;
        if (p3->mapped)
          {
             _edje_calc_params_need_ext(p1);
             _edje_calc_params_need_ext(p2);
             _edje_calc_params_need_ext(p3);
             EINA_COW_CALC_MAP_BEGIN(p3, p3_write)
             {
                p3_write->center.x = INTP(p1->ext->map->center.x, p2->ext->map->center.x, pos);
                p3_write->center.y = INTP(p1->ext->map->center.y, p2->ext->map->center.y, pos);
                p3_write->center.z = INTP(p1->ext->map->center.z, p2->ext->map->center.z, pos);
                p3_write->rotation.x = FFP(p1->ext->map->rotation.x, p2->ext->map->rotation.x, pos);
                p3_write->rotation.y = FFP(p1->ext->map->rotation.y, p2->ext->map->rotation.y, pos);
                p3_write->rotation.z = FFP(p1->ext->map->rotation.z, p2->ext->map->rotation.z, pos);
                p3_write->zoom.x = FFP(p1->ext->map->zoom.x, p2->ext->map->zoom.x, pos);
                p3_write->zoom.y = FFP(p1->ext->map->zoom.y, p2->ext->map->zoom.y, pos);

#define MIX(P1, P2, P3, pos, info) \
  P3->info = P1->ext->map->info + TO_INT(SCALE(pos, P2->ext->map->info - P1->ext->map->info));
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
                     memcpy(&p3_write->light, &p1->ext->map->light, sizeof (p1->ext->map->light));
                  }
                else if (p2->lighted)
                  {
                     memcpy(&p3_write->light, &p2->ext->map->light, sizeof (p2->ext->map->light));
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
                     memcpy(&p3_write->persp, &p1->ext->map->persp, sizeof (p1->ext->map->persp));
                  }
                else if (p2->persp_on)
                  {
                     memcpy(&p3_write->persp, &p2->ext->map->persp, sizeof (p2->ext->map->persp));
                  }
             }
             EINA_COW_CALC_MAP_END(p3, p3_write);
          }

#ifndef EDJE_CALC_CACHE
        _edje_calc_params_clear(&lp2);
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
        _edje_calc_params_clear(state);
        _edje_part_calc_params_memcpy(state, pf, ep->part->type);
     }

   ep->req = pf->req;

   if (ep->drag && ep->drag->need_reset)
     {
        FLOAT_T dx, dy;

        dx = ZERO;
        dy = ZERO;
        _edje_part_dragable_calc(ed, ep, &dx, &dy);
        ep->drag->x = dx;
        ep->drag->val.x = dx;
        ep->drag->y = dy;
        ep->drag->val.y = dy;
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
                  Edje_Part_Description_Image *img_desc = (Edje_Part_Description_Image *)chosen_desc;

                  evas_object_image_scale_hint_set(ep->object,
                                                   img_desc->image.scale_hint);
               }
             /* No break here as we share the rest of the code for all types. Intended fall-through*/
             EINA_FALLTHROUGH;

           case EDJE_PART_TYPE_PROXY:
             EINA_FALLTHROUGH;
           case EDJE_PART_TYPE_RECTANGLE:
             EINA_FALLTHROUGH;
           case EDJE_PART_TYPE_TEXTBLOCK:
             EINA_FALLTHROUGH;
           case EDJE_PART_TYPE_BOX:
             EINA_FALLTHROUGH;
           case EDJE_PART_TYPE_TABLE:
             EINA_FALLTHROUGH;
           case EDJE_PART_TYPE_SNAPSHOT:
             EINA_FALLTHROUGH;
           case EDJE_PART_TYPE_VECTOR:
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
                  if (pf->no_render_apply)
                    efl_canvas_object_no_render_set(ep->object, pf->no_render);
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
             if (pf->no_render_apply)
               efl_canvas_object_no_render_set(ep->object, pf->no_render);
#endif
             EINA_FALLTHROUGH;

           /* move and resize are needed for all previous object => no break here. */
           case EDJE_PART_TYPE_SWALLOW:
           case EDJE_PART_TYPE_GROUP:
           case EDJE_PART_TYPE_EXTERNAL:
             /* visibility and color have no meaning on SWALLOW and GROUP part. */
#ifdef HAVE_EPHYSICS
             efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(pf->final.w,  pf->final.h));
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
                    {
                       _edje_calc_params_need_ext(pf);
                       _edje_physics_body_props_update(ed, ep, pf, !pf->ext->physics->ignore_part_pos);
                    }
               }
             else
               efl_gfx_entity_position_set(ep->object, EINA_POSITION2D(ed->x + pf->final.x, ed->y + pf->final.y));
#else
             efl_gfx_entity_position_set(ep->object, EINA_POSITION2D(ed->x + pf->final.x, ed->y + pf->final.y));
             efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(pf->final.w,  pf->final.h));
#endif

             if (ep->nested_smart) /* Move, Resize all nested parts */
               {   /* Not really needed but will improve the bounding box evaluation done by Evas */
                  efl_gfx_entity_position_set(ep->nested_smart, EINA_POSITION2D(ed->x + pf->final.x, ed->y + pf->final.y));
                  efl_gfx_entity_size_set(ep->nested_smart, EINA_SIZE2D(pf->final.w,  pf->final.h));
               }
             if (ep->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
               _edje_entry_real_part_configure(ed, ep);

             /* handle clip overrides */
             if (ed->has_state_clip)
               {
                  if ((pf->ext) && (pf->ext->clip_to) && (pf->ext->clip_to->object))
                    evas_object_clip_set(ep->object, pf->ext->clip_to->object);
                  else if (ep->part->clip_to_id >= 0)
                    evas_object_clip_set(ep->object, ed->table_parts[ep->part->clip_to_id % ed->table_parts_size]->object);
                  else
                    evas_object_clip_set(ep->object, ed->base.clipper);
               }
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
           case EDJE_PART_TYPE_CAMERA:
             {
                Evas_Object *viewport;
                Evas_Canvas3D_Camera *camera = NULL;
                Edje_Part_Description_Camera *pd_camera;

                efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(pf->req.w,  pf->req.h));

                pd_camera = (Edje_Part_Description_Camera*) ep->chosen_description;

                efl_gfx_entity_position_set(ep->object, EINA_POSITION2D(ed->x + pf->final.x, ed->y + pf->final.y)),
                efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(pf->final.w,  pf->final.h));

                viewport = evas_object_image_source_get(ep->object);

                efl_gfx_entity_size_set(viewport, EINA_SIZE2D(pf->req.w,  pf->req.h));

                evas_object_image_source_visible_set(ep->object, EINA_FALSE);
                evas_object_image_source_events_set(ep->object, EINA_TRUE);
                evas_object_show(ep->object);

                camera = evas_canvas3d_node_camera_get(ep->node);

                evas_canvas3d_camera_projection_perspective_set(camera, pd_camera->camera.camera.fovy, pd_camera->camera.camera.aspect, pd_camera->camera.camera.frustum_near, pd_camera->camera.camera.frustum_far);

                _edje_calc_params_need_type_node(pf);
                evas_canvas3d_node_position_set(ep->node, pf->type.node->point.x, pf->type.node->point.y, pf->type.node->point.z);
                switch (pd_camera->camera.orientation.type)
                  {
                     case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_NONE:
                     case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_QUATERNION:
                       {
                          SET_QUATERNION(camera)
                          break;
                       }
                     case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_AT:
                       SET_LOOK_AT(camera)
                       break;
                     case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_TO:
                       {
                          SET_LOOK_TO(camera)
                          break;
                       }
                     case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_ANGLE_AXIS:
                       SET_ANGLE_AXIS(camera)
                       break;
                  }
                break;
             }
           case EDJE_PART_TYPE_LIGHT:
             {
                Evas_Canvas3D_Light *light_node = NULL;
                Edje_Part_Description_Light *pd_light;

                pd_light = (Edje_Part_Description_Light*) ep->chosen_description;
                light_node = evas_canvas3d_node_light_get(ep->node);

                evas_canvas3d_light_ambient_set(light_node, (float) pd_light->light.properties.ambient.r / 255, (float) pd_light->light.properties.ambient.g / 255, (float) pd_light->light.properties.ambient.b / 255, (float) pd_light->light.properties.ambient.a / 255);
                evas_canvas3d_light_diffuse_set(light_node, (float) pd_light->light.properties.diffuse.r / 255, (float) pd_light->light.properties.diffuse.g / 255, (float) pd_light->light.properties.diffuse.b / 255, (float) pd_light->light.properties.diffuse.a / 255);
                evas_canvas3d_light_specular_set(light_node, (float) pd_light->light.properties.specular.r / 255, (float) pd_light->light.properties.specular.g / 255, (float) pd_light->light.properties.specular.b / 255, (float) pd_light->light.properties.specular.a / 255);
                evas_canvas3d_light_directional_set(light_node, EINA_TRUE);
                evas_canvas3d_light_projection_perspective_set(light_node, pd_light->light.light.fovy, pd_light->light.light.aspect, pd_light->light.light.frustum_near, pd_light->light.light.frustum_far);

                _edje_calc_params_need_type_node(pf);
                evas_canvas3d_node_position_set(ep->node, pf->type.node->point.x, pf->type.node->point.y, pf->type.node->point.z);
                switch (pd_light->light.orientation.type)
                  {
                     case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_NONE:
                     case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_QUATERNION:
                       {
                          SET_QUATERNION(light);
                          break;
                       }
                     case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_AT:
                       SET_LOOK_AT(light)
                       break;
                     case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_TO:
                       {
                          SET_LOOK_TO(light)
                          break;
                       }
                     case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_ANGLE_AXIS:
                       SET_ANGLE_AXIS(light)
                       break;
                  }
                break;
             }
           case EDJE_PART_TYPE_MESH_NODE:
             {
                Evas_Canvas3D_Material *material = NULL;
                Evas_Canvas3D_Texture *texture = NULL;
                Evas_Canvas3D_Mesh *mesh = NULL;
                const char *proxy;
                Edje_Part_Description_Mesh_Node *pd_mesh_node;
                const Eina_List *meshes;
                const Eina_List *list;
                Eina_Bool frame_exist;

                meshes = evas_canvas3d_node_mesh_list_get(ep->node);

                EINA_LIST_FOREACH(meshes, list, mesh)
                  {
                     material = evas_canvas3d_mesh_frame_material_get(mesh, 0);
                     texture = evas_canvas3d_material_texture_get(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE);

                     pd_mesh_node = (Edje_Part_Description_Mesh_Node*) ep->chosen_description;

                     evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, EINA_TRUE);
                     evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, EINA_TRUE);
                     evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, EINA_TRUE);
                     evas_canvas3d_material_enable_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_NORMAL, pd_mesh_node->mesh_node.properties.normal);
                     evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_AMBIENT, (float) pd_mesh_node->mesh_node.properties.ambient.r / 255, (float) pd_mesh_node->mesh_node.properties.ambient.g / 255, (float) pd_mesh_node->mesh_node.properties.ambient.b / 255, (float) pd_mesh_node->mesh_node.properties.ambient.a / 255);
                     evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE, (float) pd_mesh_node->mesh_node.properties.diffuse.r / 255, (float)  pd_mesh_node->mesh_node.properties.diffuse.g / 255, (float)  pd_mesh_node->mesh_node.properties.diffuse.b / 255, (float)  pd_mesh_node->mesh_node.properties.diffuse.a / 255);
                     evas_canvas3d_material_color_set(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_SPECULAR, (float)  pd_mesh_node->mesh_node.properties.specular.r / 255, (float)  pd_mesh_node->mesh_node.properties.specular.g / 255, (float)  pd_mesh_node->mesh_node.properties.specular.b / 255, (float)  pd_mesh_node->mesh_node.properties.specular.a / 255);
                     evas_canvas3d_material_shininess_set(material, pd_mesh_node->mesh_node.properties.shininess);

                     switch(pd_mesh_node->mesh_node.mesh.primitive)
                       {
                          case EVAS_CANVAS3D_MESH_PRIMITIVE_CUBE:
                          case EVAS_CANVAS3D_MESH_PRIMITIVE_SPHERE:
                            {
                               Eo *primitive = NULL;
                               primitive = efl_add(EVAS_CANVAS3D_PRIMITIVE_CLASS, ed->base.evas);
                               evas_canvas3d_primitive_form_set(primitive, pd_mesh_node->mesh_node.mesh.primitive);

                               _edje_calc_params_need_type_node(pf);
                               frame_exist = evas_canvas3d_mesh_frame_exist(mesh, pf->type.node->frame);
                               if (!frame_exist)
                                 {
                                    evas_canvas3d_mesh_frame_material_set(mesh, pf->type.node->frame, material);
                                 }
                               evas_canvas3d_mesh_from_primitive_set(mesh, 0, primitive);
                               break;
                            }
                          default:
                            break;
                       }

                     if (pd_mesh_node->mesh_node.texture.need_texture && pd_mesh_node->mesh_node.texture.textured)
                       {
                          proxy = NULL;

                          texture = evas_canvas3d_material_texture_get(material, EVAS_CANVAS3D_MATERIAL_ATTRIB_DIFFUSE);

                          //proxy = _edje_image_name_find(ed, pd_mesh_node->mesh_node.texture.id);
                          /*FIXME Conflict with function _edje_image_name_find (two places in edje_utils and edje_edit.c,
                          temporary desicion need to clarify up to commit to phab*/
                          proxy = ed->file->image_dir->entries[pd_mesh_node->mesh_node.texture.id].entry;
                          if (proxy)
                            {
                               efl_file_simple_mmap_load(texture, ed->file->f, proxy);
                               evas_canvas3d_texture_filter_set(texture, pd_mesh_node->mesh_node.texture.filter1, pd_mesh_node->mesh_node.texture.filter2);
                               evas_canvas3d_texture_wrap_set(texture, pd_mesh_node->mesh_node.texture.wrap1, pd_mesh_node->mesh_node.texture.wrap2);
                            }
                       }

                     _edje_calc_params_need_type_node(pf);
                     frame_exist = evas_canvas3d_mesh_frame_exist(mesh, pf->type.node->frame);
                     if (!frame_exist)
                       {
                          evas_canvas3d_mesh_frame_add(mesh, pf->type.node->frame);
                          evas_canvas3d_mesh_frame_material_set(mesh, pf->type.node->frame, material);
                       }
                     evas_canvas3d_mesh_shader_mode_set(mesh, pd_mesh_node->mesh_node.properties.shade);
                     evas_canvas3d_mesh_vertex_assembly_set(mesh, pd_mesh_node->mesh_node.mesh.assembly);
                     evas_canvas3d_node_mesh_frame_set(ep->node, mesh, pf->type.node->frame);
                     evas_canvas3d_node_scale_set(ep->node, pf->type.node->scale_3d.x, pf->type.node->scale_3d.y, pf->type.node->scale_3d.z);
                     evas_canvas3d_node_position_set(ep->node, pf->type.node->point.x, pf->type.node->point.y, pf->type.node->point.z);
                     switch (pd_mesh_node->mesh_node.orientation.type)
                       {
                          case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_NONE:
                          case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_QUATERNION:
                            {
                               SET_QUATERNION(mesh_node)
                               break;
                            }
                          case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_AT:
                            SET_LOOK_AT(mesh_node)
                            break;
                          case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_LOOK_TO:
                            {
                               SET_LOOK_TO(mesh_node)
                               break;
                            }
                          case EVAS_CANVAS3D_NODE_ORIENTATION_TYPE_ANGLE_AXIS:
                            SET_ANGLE_AXIS(mesh_node)
                            break;
                       }
                  }
                break;
             }
          }

        /* Some object need special recalc. */
        switch (ep->part->type)
          {
           case EDJE_PART_TYPE_TEXT:
             _edje_text_recalc_apply(ed, ep, pf, (Edje_Part_Description_Text*) chosen_desc, EINA_FALSE);
             break;

           case EDJE_PART_TYPE_PROXY:
             _edje_proxy_recalc_apply(ed, ep, pf, (Edje_Part_Description_Proxy *)chosen_desc, pos);
             break;

           case EDJE_PART_TYPE_IMAGE:
             _edje_image_recalc_apply(ed, ep, pf, (Edje_Part_Description_Image *)chosen_desc, pos);
             break;

           case EDJE_PART_TYPE_BOX:
             _edje_box_recalc_apply(ed, ep, pf, (Edje_Part_Description_Box *)chosen_desc);
             break;

           case EDJE_PART_TYPE_TABLE:
             _edje_table_recalc_apply(ed, ep, pf, (Edje_Part_Description_Table *)chosen_desc);
             break;

           case EDJE_PART_TYPE_VECTOR:
             _edje_vector_recalc_apply(ed, ep, pf, (Edje_Part_Description_Vector *)chosen_desc, pos);
             break;

           case EDJE_PART_TYPE_TEXTBLOCK:
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
             if (ed->has_state_clip)
               {
                  if ((pf->ext) && (pf->ext->clip_to) && (pf->ext->clip_to->object))
                    evas_object_clip_set(ep->typedata.swallow->swallowed_object, pf->ext->clip_to->object);
                  else if (ep->part->clip_to_id >= 0)
                    evas_object_clip_set(ep->typedata.swallow->swallowed_object, ed->table_parts[ep->part->clip_to_id % ed->table_parts_size]->object);
                  else
                    evas_object_clip_set(ep->typedata.swallow->swallowed_object, ed->base.clipper);
               }

             if (pf->visible)
               {
                  Eina_Bool vis = EINA_TRUE;

                  if (ep->part->type == EDJE_PART_TYPE_GROUP)
                    vis = evas_object_visible_get(ed->obj);
                  efl_gfx_entity_position_set(ep->typedata.swallow->swallowed_object, EINA_POSITION2D(ed->x + pf->final.x, ed->y + pf->final.y));
                  efl_gfx_entity_size_set(ep->typedata.swallow->swallowed_object, EINA_SIZE2D(pf->final.w,  pf->final.h));
                  efl_gfx_entity_visible_set(ep->typedata.swallow->swallowed_object, vis);
               }
             else evas_object_hide(ep->typedata.swallow->swallowed_object);
             mo = ep->typedata.swallow->swallowed_object;
          }
        else mo = ep->object;
        if (ep->part->type != EDJE_PART_TYPE_SPACER)
          {
             Evas_Object *map_obj;
             Evas_Object *cursor_objs[EDJE_ENTRY_NUM_CURSOR_OBJS];
             int c = 0, num_cursors = 0;

             /* Apply map to smart obj holding nested parts */
             if (ep->nested_smart) map_obj = ep->nested_smart;
             else map_obj = mo;
             if (ep->part->type == EDJE_PART_TYPE_TEXTBLOCK)
               num_cursors = _edje_entry_real_part_cursor_objs_get(ep, cursor_objs);

             if (chosen_desc->map.on)
               {
                  static Evas_Map *map = NULL;

                  ed->have_mapped_part = EINA_TRUE;
                  // create map and populate with part geometry
                  if (!map) map = evas_map_new(4);

                  _edje_map_prop_set(map, pf, chosen_desc, ep, mo, NULL);

                  if (map_obj)
                    {
                       evas_object_map_set(map_obj, map);
                       evas_object_map_enable_set(map_obj, EINA_TRUE);
                       if (ep->part->type == EDJE_PART_TYPE_TEXTBLOCK)
                         {
                            for (c = 0; c < num_cursors; c++)
                              {
                                 _edje_map_prop_set(map, pf, chosen_desc, ep, mo, cursor_objs[c]);
                                 evas_object_map_set(cursor_objs[c], map);
                                 evas_object_map_enable_set(cursor_objs[c], EINA_TRUE);
                              }
                         }
                    }
               }
             else
               {
                  //Disable map only if map were enabled.
                  if (map_obj && evas_object_map_enable_get(map_obj))
                    {
#ifdef HAVE_EPHYSICS
                       if (!ep->nested_smart && !ep->body)
                         {
#endif
                            evas_object_map_enable_set(mo, EINA_FALSE);
                            evas_object_map_set(mo, NULL);
                            if (ep->part->type == EDJE_PART_TYPE_TEXTBLOCK)
                              {
                                 for (c = 0; c < num_cursors; c++)
                                   {
                                      evas_object_map_enable_set(cursor_objs[c], EINA_FALSE);
                                      evas_object_map_set(cursor_objs[c], NULL);
                                   }
                              }
#ifdef HAVE_EPHYSICS
                         }
#endif
                    }
               }
          }
     }

   if (map_colors_free) _map_colors_free(pf);

   ep->prev_description = chosen_desc;
#ifdef HAVE_EPHYSICS
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
        _edje_calc_params_clear(&lp3);
     }

#ifdef EDJE_CALC_CACHE
   if (ep->calculated == FLAG_XY)
     {
        ep->state = ed->state;
        ep->invalidate = EINA_FALSE;
     }
#else
   _edje_calc_params_clear(&lp1);
#endif

}
