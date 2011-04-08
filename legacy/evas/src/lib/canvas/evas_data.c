#include "evas_common.h"
#include "evas_private.h"


EAPI void
evas_object_data_set(Evas_Object *obj, const char *key, const void *data)
{
   Evas_Data_Node *node;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!key) return;

   evas_object_data_del(obj, key);
   if (!data) return;
   node = malloc(sizeof(Evas_Data_Node) + strlen(key) + 1);
   node->key = (char *)node + sizeof(Evas_Data_Node);
   strcpy(node->key, key);
   node->data = (void *)data;
   obj->data.elements = eina_list_prepend(obj->data.elements, node);
}

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
