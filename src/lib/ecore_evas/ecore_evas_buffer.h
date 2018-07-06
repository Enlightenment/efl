#ifndef _ECORE_EVAS_BUFFER_PRIVATE_H_
#define _ECORE_EVAS_BUFFER_PRIVATE_H_

typedef struct _Ecore_Evas_Engine_Buffer_Data Ecore_Evas_Engine_Buffer_Data;

struct _Ecore_Evas_Engine_Buffer_Data {
   void *pixels;
   Evas_Object *image;
   void  (*free_func) (void *data, void *pix);
   void *(*alloc_func) (void *data, int size);
   void *data;
   Eina_Bool in_render : 1;
   Eina_Bool resized : 1;
};

#endif /* _ECORE_EVAS_BUFFER_PRIVATE_H_ */
