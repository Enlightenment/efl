#ifndef EO_PTR_INDIRECTION_H
#define EO_PTR_INDIRECTION_H

#include "Eo.h"
#include "eo_private.h"

/* Macro used to obtain the object pointer and return if fails. */

void _eo_pointer_error(const Eo *obj_id, const char *func_name, const char *file, int line, const char *fmt, ...);

#define _EO_POINTER_ERR(obj_id, fmt, ...) \
  _eo_pointer_error(obj_id, __FUNCTION__, __FILE__, __LINE__, fmt, __VA_ARGS__)

#define EO_OBJ_POINTER(obj_id, obj) \
   _Eo_Object *obj; \
   do { \
      obj = _eo_obj_pointer_get((Eo_Id)obj_id, __FUNCTION__, __FILE__, __LINE__); \
   } while (0)

#define EO_OBJ_POINTER_PROXY(obj_id, obj) \
   _Eo_Object *obj; \
   do { \
      obj = _eo_obj_pointer_get((Eo_Id)obj_id, func_name, file, line); \
   } while (0)

#define EO_OBJ_POINTER_RETURN_VAL(obj_id, obj, ret)  \
   _Eo_Object *obj; \
   do { \
      obj = _eo_obj_pointer_get((Eo_Id)obj_id, __FUNCTION__, __FILE__, __LINE__); \
      if (!obj) return (ret); \
   } while (0)

/* reports as another function using func_name, file and line variables */
#define EO_OBJ_POINTER_RETURN_VAL_PROXY(obj_id, obj, ret) \
   _Eo_Object *obj; \
   do { \
      obj = _eo_obj_pointer_get((Eo_Id)obj_id, func_name, file, line); \
      if (!obj) return (ret); \
   } while (0)

#define EO_OBJ_POINTER_RETURN(obj_id, obj) \
   _Eo_Object *obj; \
   do { \
      obj = _eo_obj_pointer_get((Eo_Id)obj_id, __FUNCTION__, __FILE__, __LINE__); \
      if (!obj) return; \
   } while (0)

#define EO_OBJ_POINTER_RETURN_PROXY(obj_id, obj) \
   _Eo_Object *obj; \
   do { \
      obj = _eo_obj_pointer_get((Eo_Id)obj_id, func_name, file, line); \
      if (!obj) return; \
   } while (0)

#define EO_OBJ_POINTER_GOTO(obj_id, obj, label) \
   _Eo_Object *obj; \
   do { \
      obj = _eo_obj_pointer_get((Eo_Id)obj_id, __FUNCTION__, __FILE__, __LINE__); \
      if (!obj) goto label; \
   } while (0)

#define EO_OBJ_POINTER_GOTO_PROXY(obj_id, obj, label) \
   _Eo_Object *obj; \
   do { \
      obj = _eo_obj_pointer_get((Eo_Id)obj_id, func_name, file, line); \
      if (!obj) goto label; \
   } while (0)

#define EO_CLASS_POINTER(klass_id, klass)   \
   _Efl_Class *klass; \
   do { \
      klass = _eo_class_pointer_get(klass_id); \
   } while (0)

#define EO_CLASS_POINTER_PROXY(klass_id, klass)   \
   _Efl_Class *klass; \
   do { \
      klass = _eo_class_pointer_get(klass_id); \
   } while (0)

#define EO_CLASS_POINTER_RETURN_VAL(klass_id, klass, ret) \
   _Efl_Class *klass; \
   do { \
      klass = _eo_class_pointer_get(klass_id); \
      if (!klass) { \
         _EO_POINTER_ERR(klass_id, "Class (%p) is an invalid ref.", klass_id); \
         return ret; \
      } \
   } while (0)

#define EO_CLASS_POINTER_RETURN_VAL_PROXY(klass_id, klass, ret) \
   _Efl_Class *klass; \
   do { \
      klass = _eo_class_pointer_get(klass_id); \
      if (!klass) { \
         _EO_POINTER_ERR(klass_id, "Class (%p) is an invalid ref.", klass_id); \
         return ret; \
      } \
   } while (0)

#define EO_CLASS_POINTER_RETURN(klass_id, klass)   \
   _Efl_Class *klass; \
   do { \
      klass = _eo_class_pointer_get(klass_id); \
      if (!klass) { \
         _EO_POINTER_ERR(klass_id, "Class (%p) is an invalid ref.", klass_id); \
         return; \
      } \
   } while (0)

#define EO_CLASS_POINTER_RETURN_PROXY(klass_id, klass) \
   _Efl_Class *klass; \
   do { \
      klass = _eo_class_pointer_get(klass_id); \
      if (!klass) { \
         _EO_POINTER_ERR(klass_id, "Class (%p) is an invalid ref.", klass_id); \
         return; \
      } \
   } while (0)

#define EO_CLASS_POINTER_GOTO(klass_id, klass, label) \
   _Efl_Class *klass; \
   do { \
      klass = _eo_class_pointer_get(klass_id); \
      if (!klass) goto label; \
   } while (0)

#define EO_CLASS_POINTER_GOTO_PROXY(klass_id, klass, label) \
   _Efl_Class *klass; \
   do { \
      klass = _eo_class_pointer_get(klass_id); \
      if (!klass) goto label; \
   } while (0)

#define EO_OBJ_DONE(obj_id) \
   _eo_obj_pointer_done((Eo_Id)obj_id)

#ifdef EFL_DEBUG
static inline void _eo_print(Eo_Id_Table_Data *tdata);
#endif

extern Eina_TLS _eo_table_data;

#include "eo_ptr_indirection.x"

extern Eo_Id_Data *_eo_table_data_shared;
extern Eo_Id_Table_Data *_eo_table_data_shared_data;

#endif

