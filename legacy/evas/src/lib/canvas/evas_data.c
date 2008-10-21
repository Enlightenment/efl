#include "evas_common.h"
#include "evas_private.h"

/**
 * @defgroup Evas_Object_Data_Group Object Data Functions
 *
 * Functions that retrieve and set data associated attached to an evas
 * object.
 */

/**
 * Set an attached data pointer to an object with a given string key.
 * @param obj The object to attach the data pointer to
 * @param key The string key for the data to access it
 * @param data The ponter to the data to be attached
 *
 * This attaches the pointer @p data to the object @p obj given the string
 * @p key. This pointer will stay "hooked" to the object until a new pointer
 * with the same string key is attached with evas_object_data_set() or it is
 * deleted with evas_object_data_del(). On deletion of the object @p obj, the
 * pointers will not be accessible from the object anymore.
 *
 * You can find the pointer attached under a string key using
 * evas_object_data_get(). It is the job of the calling application to free
 * any data pointed to by @p data when it is no longer required.
 *
 * If @p data is NULL, the old value stored at @p key will be removed but no
 * new value will be stored. This is synonymous with calling
 * evas_object_data_del() with @p obj and @p key.
 *
 * Example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = malloc(500);
 * evas_object_data_set(obj, "name_of_data", my_data);
 * printf("The data that was attached was %p\n", evas_object_data_get(obj, "name_of_data"));
 * @endcode
 * @ingroup Evas_Object_Data_Group
 */
EAPI void
evas_object_data_set(Evas_Object *obj, const char *key, const void *data)
{
   Evas_Data_Node *node;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!key) return;

   evas_object_data_del(obj, key);
   if (data == NULL) return;
   node = malloc(sizeof(Evas_Data_Node) + strlen(key) + 1);
   node->key = (char *)node + sizeof(Evas_Data_Node);
   strcpy(node->key, key);
   node->data = (void *)data;
   obj->data.elements = eina_list_prepend(obj->data.elements, node);
}

/**
 * Return an attached data pointer by its given string key.
 * @param obj The object to which the data was attached
 * @param key The string key the data was stored under
 * @return The data pointer stored, or NULL if none was stored
 *
 * This function will return the data pointer attached to the object @p obj
 * stored using the string key @p key. If the object is valid and data was
 * stored under the given key, the pointer that was stored will be reuturned.
 * If this is not the case, NULL will be returned, signifying an invalid object
 * or non-existent key. It is possible a NULL pointer was stored given that
 * key, but this situation is non-sensical and thus can be considered an error
 * as well. NULL pointers are never stored as this is the return value if an
 * error occurs.
 *
 * Example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = evas_object_data_get(obj, "name_of_my_data");
 * if (my_data) printf("Data stored was %p\n", my_data);
 * else printf("No data was stored on the object\n");
 * @endcode
 * @ingroup Evas_Object_Data_Group
 */
EAPI void *
evas_object_data_get(const Evas_Object *obj, const char *key)
{
   Eina_List *l;
   Evas_Data_Node *node;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!key) return NULL;

   EINA_LIST_FOREACH(obj->data.elements, l, node)
     {
	if (!strcmp(node->key, key))
	  {
	     Eina_List *lst;
	     lst = obj->data.elements;
	     lst = eina_list_promote_list(lst, l);
	     ((Evas_Object *)obj)->data.elements = lst;
	     return node->data;
	  }
     }
   return NULL;
}

/**
 * Delete at attached data pointer from an object.
 * @param obj The object to delete the data pointer from
 * @param key The string key the data was stored under
 * @return The original data pointer stored at @p key on @p obj
 *
 * This will remove thee stored data pointer from @p obj stored under @p key,
 * and return the original pointer stored under @p key, if any, nor NULL if
 * nothing was stored under that key.
 *
 * Example:
 *
 * @code
 * int *my_data;
 * extern Evas_Object *obj;
 *
 * my_data = evas_object_data_del(obj, "name_of_my_data");
 * @endcode
 * @ingroup Evas_Object_Data_Group
 */
EAPI void *
evas_object_data_del(Evas_Object *obj, const char *key)
{
   Eina_List *l;
   Evas_Data_Node *node;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!key) return NULL;
   EINA_LIST_FOREACH(obj->data.elements, l, node)
     {
	if (!strcmp(node->key, key))
	  {
	     void *data;

	     data = node->data;
	     obj->data.elements = eina_list_remove_list(obj->data.elements, l);
	     free(node);
	     return data;
	  }
     }
   return NULL;
}
