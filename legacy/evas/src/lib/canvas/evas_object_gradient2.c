#include "evas_common.h"
#include "evas_private.h"
#include "evas_object_gradient2.h"

/**
 * @defgroup Evas_Object_Gradient2_Group Gradient2 Object Functions
 *
 * Functions that work on evas gradient2 objects.
 *
 */

/**
 * @addtogroup Evas_Object_Gradient2_Group
 * @{
 * @ingroup Evas_Object_Specific
 */

/**
 * Inserts a color stop to the given evas gradient object.
 *
 * The @p pos parameter determines where along the unit interval
 * [0,1] the color is to be inserted. The r,g,b,a data are assumed
 * input as being NON-PREMULTIPLIED.
 *
 * @param   obj      The given evas gradient object.
 * @param   r        Red component of the given color.
 * @param   g        Green component of the given color.
 * @param   b        Blue component of the given color.
 * @param   a        Alpha component of the given color.
 * @param   pos      The pos in [0,1] of this stop.
 */
EAPI void
evas_object_gradient2_color_np_stop_insert(Evas_Object *obj, int r, int g, int b, int a, float pos)
{
   Evas_Object_Gradient2 *og;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   og = (Evas_Object_Gradient2 *)(obj->object_data);
   MAGIC_CHECK(og, Evas_Object_Gradient2, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (og->engine_data)
      obj->layer->evas->engine.func->gradient2_color_np_stop_insert(obj->layer->evas->engine.data.output,
							     og,
							     r, g, b, a, pos);
   og->gradient_changed = 1;
   evas_object_change(obj);
}

/**
 * Deletes all stops set for the given evas gradient object or any set data.
 * @param   obj The given evas gradient object.
 */
EAPI void
evas_object_gradient2_clear(Evas_Object *obj)
{
   Evas_Object_Gradient2 *og;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   og = (Evas_Object_Gradient2 *)(obj->object_data);
   MAGIC_CHECK(og, Evas_Object_Gradient2, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (og->engine_data)
      obj->layer->evas->engine.func->gradient2_clear(obj->layer->evas->engine.data.output,
						     og);
   og->gradient_changed = 1;
   og->cur.gradient_opaque = 0;
   evas_object_change(obj);
}


/**
 * Sets the tiling mode for the given evas gradient object's fill.
 * @param   obj   The given evas gradient object.
 * @param   spread One of EVAS_TEXTURE_REFLECT, EVAS_TEXTURE_REPEAT,
 * EVAS_TEXTURE_RESTRICT, or EVAS_TEXTURE_PAD.
 */
EAPI void
evas_object_gradient2_fill_spread_set(Evas_Object *obj, int spread)
{
   Evas_Object_Gradient2 *og;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   og = (Evas_Object_Gradient2 *)(obj->object_data);
   MAGIC_CHECK(og, Evas_Object_Gradient2, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (spread == og->cur.fill.spread) return;
   og->cur.fill.spread = spread;
   og->gradient_changed = 1;
   evas_object_change(obj);
}

/**
 * Retrieves the spread (tiling mode) for the given gradient object's fill.
 * @param   obj The given evas gradient object.
 * @return  The current spread mode of the gradient object.
 */
EAPI int
evas_object_gradient2_fill_spread_get(const Evas_Object *obj)
{
   Evas_Object_Gradient2 *og;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_TEXTURE_REPEAT;
   MAGIC_CHECK_END();
   og = (Evas_Object_Gradient2 *)(obj->object_data);
   MAGIC_CHECK(og, Evas_Object_Gradient2, MAGIC_OBJ_GRADIENT);
   return EVAS_TEXTURE_REPEAT;
   MAGIC_CHECK_END();
   return og->cur.fill.spread;
}

EAPI void
evas_object_gradient2_fill_transform_set (Evas_Object *obj, Evas_Transform *t)
{
   Evas_Object_Gradient2 *og;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   og = (Evas_Object_Gradient2 *)(obj->object_data);
   MAGIC_CHECK(og, Evas_Object_Gradient2, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (!t)
     {
	og->cur.fill.transform.mxx = 1;
	og->cur.fill.transform.mxy = 0;
	og->cur.fill.transform.mxz = 0;
	og->cur.fill.transform.myx = 0;
	og->cur.fill.transform.myy = 1;
	og->cur.fill.transform.myz = 0;
	og->cur.fill.transform.mzx = 0;
	og->cur.fill.transform.mzy = 0;
	og->cur.fill.transform.mzz = 1;

	og->gradient_changed = 1;
	evas_object_change(obj);
	return;
     }
   if ( (og->cur.fill.transform.mxx == t->mxx) &&
        (og->cur.fill.transform.mxy == t->mxy) &&
	(og->cur.fill.transform.mxz == t->mxz) &&
	(og->cur.fill.transform.myx == t->myx) &&
	(og->cur.fill.transform.myy == t->myy) &&
	(og->cur.fill.transform.myz == t->myz) &&
	(og->cur.fill.transform.mzx == t->mzx) &&
	(og->cur.fill.transform.mzy == t->mzy) &&
	(og->cur.fill.transform.mzz == t->mzz) )
     return;

   og->cur.fill.transform.mxx = t->mxx;
   og->cur.fill.transform.mxy = t->mxy;
   og->cur.fill.transform.mxz = t->mxz;
   og->cur.fill.transform.myx = t->myx;
   og->cur.fill.transform.myy = t->myy;
   og->cur.fill.transform.myz = t->myz;
   og->cur.fill.transform.mzx = t->mzx;
   og->cur.fill.transform.mzy = t->mzy;
   og->cur.fill.transform.mzz = t->mzz;

   og->cur.fill.transform.is_identity = 0;
   if ( (t->mxx == 1) && (t->mxy == 0) && (t->mxz == 0) &&
	(t->myx == 0) && (t->myy == 1) && (t->myz == 0) &&
	(t->mzx == 0) && (t->mzy == 0) && (t->mzz == 1) )
     og->cur.fill.transform.is_identity = 1;
   og->gradient_changed = 1;
   evas_object_change(obj);
}

EAPI void
evas_object_gradient2_fill_transform_get (const Evas_Object *obj, Evas_Transform *t)
{
   Evas_Object_Gradient2 *og;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   og = (Evas_Object_Gradient2 *)(obj->object_data);
   MAGIC_CHECK(og, Evas_Object_Gradient2, MAGIC_OBJ_GRADIENT);
   return;
   MAGIC_CHECK_END();
   if (t)
     {
        t->mxx = og->cur.fill.transform.mxx;
        t->mxy = og->cur.fill.transform.mxy;
        t->mxz = og->cur.fill.transform.mxz;
        t->myx = og->cur.fill.transform.myx;
        t->myy = og->cur.fill.transform.myy;
        t->myz = og->cur.fill.transform.myz;
        t->mzx = og->cur.fill.transform.mzx;
        t->mzy = og->cur.fill.transform.mzy;
        t->mzz = og->cur.fill.transform.mzz;
     }
}
