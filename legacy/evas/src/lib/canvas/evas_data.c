#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

void
evas_object_data_set(Evas_Object *obj, const char *key, void *data)
{
   Evas_Data_Node *node;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();   
   if (!key) return;
   
   evas_object_data_del(obj, key);
   
   node = malloc(sizeof(Evas_Data_Node));
   node->key = strdup(key);
   node->data = data;
   obj->data.elements = evas_list_prepend(obj->data.elements, node);
}

void *
evas_object_data_get(Evas_Object *obj, const char *key)
{
   Evas_List *l;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!key) return NULL;
   
   for (l = obj->data.elements; l; l = l->next)
     {
	Evas_Data_Node *node;
	
	node = l->data;
	if (!strcmp(node->key, key))
	  {
	     obj->data.elements = evas_list_remove(obj->data.elements, node);
	     obj->data.elements = evas_list_prepend(obj->data.elements, node);
	     return node->data;
	  }
     }
   return NULL;
}

void *
evas_object_data_del(Evas_Object *obj, const char *key)
{
   Evas_List *l;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (!key) return NULL;
   for (l = obj->data.elements; l; l = l->next)
     {
	Evas_Data_Node *node;
	
	node = l->data;
	if (!strcmp(node->key, key))
	  {
	     void *data;
	     
	     data = node->data;
	     obj->data.elements = evas_list_remove(obj->data.elements, node);
	     free(node->key);
	     free(node);
	     return data;
	  }
     }   
   return NULL;
}
