#include "evas_common.h"
#include "evas_private.h"

/**
 * @addtogroup Evas_Object_Group
 * @{
 */

/**
 * Sets the name of the given evas object to the given name.
 * @param   obj  The given object.
 * @param   name The given name.
 */
EAPI void
evas_object_name_set(Evas_Object *obj, const char *name)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->name)
     {
	obj->layer->evas->name_hash = evas_hash_del(obj->layer->evas->name_hash, obj->name, obj);
	free(obj->name);
     }
   if (!name) obj->name = NULL;
   else
     {
	obj->name = strdup(name);
	obj->layer->evas->name_hash = evas_hash_add(obj->layer->evas->name_hash, obj->name, obj);
     }
}

/**
 * Retrieves the name of the given evas object.
 * @param   obj The given object.
 * @return  The name of the object.  @c NULL if no name has been given
 *          to the object.
 */
EAPI const char *
evas_object_name_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   return obj->name;
}

/**
 * Retrieves the object on the given evas with the given name.
 * @param   e    The given evas.
 * @param   name The given name.
 * @return  If successful, the evas object with the given name.  Otherwise,
 *          @c NULL.
 */
EAPI Evas_Object *
evas_object_name_find(const Evas *e, const char *name)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (!name) return NULL;
   return (Evas_Object *)evas_hash_find(e->name_hash, name);
}

/**
 * @}
 */
