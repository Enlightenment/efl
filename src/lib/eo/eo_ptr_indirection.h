#ifndef EO_PTR_INDIRECTION_H
#define EO_PTR_INDIRECTION_H

#include "Eo.h"
#include "eo_private.h"

/* Macro used to obtain the object pointer and return if fails. */

#ifdef HAVE_EO_ID

void _eo_pointer_error(const char *msg);

#define _EO_POINTER_ERR(fmt, ptr) \
   do { char buf[256]; sprintf(buf, fmt, ptr); _eo_pointer_error(buf); } while (0)

#define EO_OBJ_POINTER(obj_id, obj) \
  _Eo_Object *obj; \
  do { \
     obj = _eo_obj_pointer_get((Eo_Id)obj_id); \
  } while (0)

#define EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, ret)  \
  _Eo_Object *obj; \
  do { \
     obj = _eo_obj_pointer_get((Eo_Id)obj_id); \
     if (!obj) return (ret); \
  } while (0)

#define EO_OBJ_POINTER_RETURN(obj_id, obj) \
  _Eo_Object *obj; \
  do { \
     obj = _eo_obj_pointer_get((Eo_Id)obj_id); \
     if (!obj) return; \
  } while (0)

#define EO_OBJ_POINTER_GOTO(obj_id, obj, label) \
  _Eo_Object *obj; \
  do { \
     obj = _eo_obj_pointer_get((Eo_Id)obj_id); \
     if (!obj) goto label; \
  } while (0)

#define EO_CLASS_POINTER(klass_id, klass)   \
   _Efl_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
   } while (0)

#define EO_CLASS_POINTER_RETURN_VAL(klass_id, klass, ret) \
   _Efl_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        if (!klass) { \
             _EO_POINTER_ERR("Class (%p) is an invalid ref.", klass_id); \
             return ret; \
        } \
   } while (0)

#define EO_CLASS_POINTER_RETURN(klass_id, klass)   \
   _Efl_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        if (!klass) { \
             _EO_POINTER_ERR("Class (%p) is an invalid ref.", klass_id); \
             return; \
        } \
   } while (0)

#define EO_CLASS_POINTER_GOTO(klass_id, klass, label) \
   _Efl_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        if (!klass) goto label; \
   } while (0)

#define EO_OBJ_DONE(obj_id) \
   _eo_obj_pointer_done((Eo_Id)obj_id)

#else

#define EO_OBJ_POINTER(obj_id, obj) \
  _Eo_Object *obj; \
  do { \
     obj = _eo_obj_pointer_get((Eo_Id)obj_id); \
     if (obj && \
         !EINA_MAGIC_CHECK((Eo_Header *) obj, EO_EINA_MAGIC)) { \
        EINA_MAGIC_FAIL((Eo_Header *) obj, EO_EINA_MAGIC); \
     } \
  } while (0)

#define EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, ret) \
   _Eo_Object *obj; \
   do { \
        obj = _eo_obj_pointer_get((Eo_Id)obj_id); \
        if (!obj) return (ret); \
        EO_MAGIC_RETURN_VAL((Eo_Header *) obj, EO_EINA_MAGIC, ret);  \
   } while (0)

#define EO_OBJ_POINTER_RETURN(obj_id, obj)   \
   _Eo_Object *obj; \
   do { \
        obj = _eo_obj_pointer_get((Eo_Id)obj_id);   \
        if (!obj) return; \
        EO_MAGIC_RETURN((Eo_Header *) obj, EO_EINA_MAGIC);  \
   } while (0)

#define EO_OBJ_POINTER_GOTO(obj_id, obj, label)   \
   _Eo_Object *obj; \
   do { \
        obj = _eo_obj_pointer_get((Eo_Id)obj_id);   \
        if (!obj || \
            !EINA_MAGIC_CHECK((Eo_Header *) obj, EO_EINA_MAGIC)) goto label; \
   } while (0)

#define EO_CLASS_POINTER(klass_id, klass)   \
   _Efl_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        if (klass && \
            !EINA_MAGIC_CHECK((Eo_Header *) klas, EO_CLASS_EINA_MAGIC)) { \
        EO_MAGIC_FAIL((Eo_Header *) klass, EO_CLASS_EINA_MAGIC);  \
        } \
   } while (0)

#define EO_CLASS_POINTER_RETURN_VAL(klass_id, klass, ret) \
   _Efl_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        if (!klass) return (ret); \
        EO_MAGIC_RETURN_VAL((Eo_Header *) klass, EO_CLASS_EINA_MAGIC, ret);  \
   } while (0)

#define EO_CLASS_POINTER_RETURN(klass_id, klass)   \
   _Efl_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        if (!klass) return; \
        EO_MAGIC_RETURN((Eo_Header *) klass, EO_CLASS_EINA_MAGIC);  \
   } while (0)

#define EO_CLASS_POINTER_GOTO(klass_id, klass, label) \
   _Efl_Class *klass; \
   do { \
        klass = _eo_class_pointer_get(klass_id); \
        if (!klass) goto label; \
        if (klass && \
            !EINA_MAGIC_CHECK((Eo_Header *) klas, EO_CLASS_EINA_MAGIC)) { \
   } while (0)

#define EO_OBJ_DONE(obj_id)

#endif

#ifdef EFL_DEBUG
void _eo_print();
#endif

extern Eina_TLS _eo_table_data;

#include "eo_ptr_indirection.x"

extern Eo_Id_Data *_eo_table_data_shared;
extern Eo_Id_Table_Data *_eo_table_data_shared_data;

#endif

