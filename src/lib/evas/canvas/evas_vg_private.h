#ifndef EVAS_VG_PRIVATE_H_
# define EVAS_VG_PRIVATE_H_

#include <Ector.h>

typedef struct _Efl_VG_Base_Data Efl_VG_Base_Data;
typedef struct _Efl_VG_Container_Data Efl_VG_Container_Data;
typedef struct _Efl_VG_Gradient_Data Efl_VG_Gradient_Data;
typedef struct _Efl_VG_Interpolation Efl_VG_Interpolation;

typedef struct _Evas_VG_Data      Evas_VG_Data;

struct _Evas_VG_Data
{
   void   *engine_data;
   Efl_VG *root;

   Eina_Rectangle fill;

   unsigned int width, height;

   Eina_Array cleanup;
};

struct _Efl_VG_Base_Data
{
   const char *name;

   Eina_Matrix3 *m;
   Efl_VG_Interpolation *intp;

   Efl_VG *mask;
   Ector_Renderer *renderer;

   void (*render_pre)(Eo *obj, Eina_Matrix3 *parent, Ector_Surface *s, void *data, Efl_VG_Base_Data *nd);
   void *data;

   double x, y;
   int r, g, b, a;

   Eina_Bool visibility : 1;
   Eina_Bool changed : 1;
};

struct _Efl_VG_Container_Data
{
   Eina_List *children;

   Eina_Hash *names;
};

struct _Efl_VG_Gradient_Data
{
   // FIXME: Later on we should deduplicate it somehow (Using Ector ?).
   Efl_Gfx_Gradient_Stop *colors;
   unsigned int colors_count;

   Efl_Gfx_Gradient_Spread s;
};

struct _Efl_VG_Interpolation
{
   Eina_Quaternion rotation;
   Eina_Quaternion perspective;
   Eina_Point_3D translation;
   Eina_Point_3D scale;
   Eina_Point_3D skew;
};

static inline Efl_VG_Base_Data *
_evas_vg_render_pre(Efl_VG *child, Ector_Surface *s, Eina_Matrix3 *m)
{
   Efl_VG_Base_Data *child_nd = NULL;

   // FIXME: Prevent infinite loop
   if (child)
     child_nd = eo_data_scope_get(child, EFL_VG_BASE_CLASS);
   if (child_nd)
     child_nd->render_pre(child, m, s, child_nd->data, child_nd);

   return child_nd;
}

static inline void
_efl_vg_base_changed(Eo *obj)
{
   eo_event_callback_call(obj, EFL_GFX_CHANGED, NULL);
}

static inline void *
_efl_vg_realloc(void *from, unsigned int sz)
{
   void *result;

   result = sz > 0 ? realloc(from, sz) : NULL;
   if (!result) free(from);

   return result;
}

static inline void
_efl_vg_clean_object(Eo **obj)
{
   if (*obj) eo_unref(*obj);
   *obj = NULL;
}

#define EFL_VG_COMPUTE_MATRIX(Current, Parent, Nd)                      \
  Eina_Matrix3 *Current = Nd->m;                                        \
  Eina_Matrix3 _matrix_tmp;                                             \
                                                                        \
  if (Parent)                                                           \
    {                                                                   \
       if (Current)                                                     \
         {                                                              \
            eina_matrix3_compose(Parent, Current, &_matrix_tmp);        \
            Current = &_matrix_tmp;                                     \
         }                                                              \
       else                                                             \
         {                                                              \
            eina_matrix3_identity(&_matrix_tmp);                        \
            eina_matrix3_translate(&_matrix_tmp, -(Nd->x), -(Nd->y));   \
            eina_matrix3_compose(Parent, &_matrix_tmp, &_matrix_tmp);   \
            eina_matrix3_translate(&_matrix_tmp, (Nd->x), (Nd->y));     \
            Current = &_matrix_tmp;                                     \
         }                                                              \
    }


#endif
