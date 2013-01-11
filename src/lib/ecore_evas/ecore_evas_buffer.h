#ifndef _ECORE_EVAS_BUFFER_PRIVATE_H_
#define _ECORE_EVAS_BUFFER_PRIVATE_H_

#include <stdlib.h>

#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"


typedef struct _Ecore_Evas_Engine_Buffer_Data Ecore_Evas_Engine_Buffer_Data;

struct _Ecore_Evas_Engine_Buffer_Data {
   void *pixels;
   Evas_Object *image;
   void  (*free_func) (void *data, void *pix);
   void *(*alloc_func) (void *data, int size);
   void *data;
};

#endif /* _ECORE_EVAS_BUFFER_PRIVATE_H_ */
