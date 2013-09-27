#ifndef EO_PTR_INDIRECTION_H
#define EO_PTR_INDIRECTION_H

#include "Eo.h"
#include "eo_private.h"

/* Macro used to obtain the object pointer and return if fails. */

#ifdef HAVE_EO_ID

#define EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, ret) \
   _Eo_Object *obj; \
   do { \
        obj = _eo_obj_pointer_get((Eo_Id)obj_id); \
        if (!obj) return ret; \
   } while (0)

#define EO_OBJ_POINTER_RETURN(obj_id, obj)   \
   _Eo_Object *obj; \
   do { \
        obj = _eo_obj_pointer_get((Eo_Id)obj_id);   \
        if (!obj) return; \
   } while (0)

#define EO_CLASS_POINTER_RETURN_VAL(klass_id, klass, ret) \
   _Eo_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        if (!klass) return ret; \
   } while (0)

#define EO_CLASS_POINTER_RETURN(klass_id, klass)   \
   _Eo_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        if (!klass) return; \
   } while (0)

#else

#define EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, ret) \
   _Eo_Object *obj; \
   do { \
        obj = _eo_obj_pointer_get((Eo_Id)obj_id); \
        EO_MAGIC_RETURN_VAL((Eo_Base *) obj, EO_EINA_MAGIC, ret);  \
   } while (0)

#define EO_OBJ_POINTER_RETURN(obj_id, obj)   \
   _Eo_Object *obj; \
   do { \
        obj = _eo_obj_pointer_get((Eo_Id)obj_id);   \
        EO_MAGIC_RETURN((Eo_Base *) obj, EO_EINA_MAGIC);  \
   } while (0)

#define EO_CLASS_POINTER_RETURN_VAL(klass_id, klass, ret) \
   _Eo_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        EO_MAGIC_RETURN_VAL((Eo_Base *) klass, EO_CLASS_EINA_MAGIC, ret);  \
   } while (0)

#define EO_CLASS_POINTER_RETURN(klass_id, klass)   \
   _Eo_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        EO_MAGIC_RETURN((Eo_Base *) klass, EO_CLASS_EINA_MAGIC);  \
   } while (0)

#endif

#ifdef EFL_DEBUG
void _eo_print();
#endif

#include "eo_ptr_indirection.x"

#endif

