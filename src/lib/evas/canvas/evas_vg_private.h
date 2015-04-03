#ifndef EVAS_VG_PRIVATE_H_
# define EVAS_VG_PRIVATE_H_

#include <Ector.h>

typedef struct _Efl_VG_Base_Data Efl_VG_Base_Data;
typedef struct _Efl_VG_Container_Data Efl_VG_Container_Data;
typedef struct _Efl_VG_Gradient_Data Efl_VG_Gradient_Data;

struct _Efl_VG_Base_Data
{
   Eina_Matrix3 *m;
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
};

struct _Efl_VG_Gradient_Data
{
   // FIXME: Later on we should deduplicate it somehow (Using Ector ?).
   Efl_Gfx_Gradient_Stop *colors;
   unsigned int colors_count;

   Efl_Gfx_Gradient_Spread s;
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
   eo_do(obj, eo_event_callback_call(EFL_GFX_CHANGED, NULL));
}

#define EFL_VG_COMPUTE_MATRIX(Current, Parent, Nd)                      \
  Eina_Matrix3 *Current = Nd->m;                                        \
  Eina_Matrix3 _matrix_tmp, translate;                                  \
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
            eina_matrix3_translate(&translate, -(Nd->x), -(Nd->y));     \
            eina_matrix3_compose(Parent, &translate, &_matrix_tmp);     \
            eina_matrix3_translate(&translate, (Nd->x), (Nd->y));       \
            eina_matrix3_compose(&_matrix_tmp, &translate, &_matrix_tmp); \
            Current = &_matrix_tmp;                                     \
         }                                                              \
    }


#endif
