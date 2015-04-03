#ifndef EVAS_VG_PRIVATE_H_
# define EVAS_VG_PRIVATE_H_

#include <Ector.h>

typedef struct _Evas_VG_Node_Data Evas_VG_Node_Data;
typedef struct _Evas_VG_Container_Data Evas_VG_Container_Data;
typedef struct _Evas_VG_Gradient_Data Evas_VG_Gradient_Data;

struct _Evas_VG_Node_Data
{
   Eina_Matrix3 *m;
   Evas_VG_Node *mask;
   Ector_Renderer *renderer;

   void (*render_pre)(Eo *obj, Ector_Surface *s, void *data, Evas_VG_Node_Data *nd);
   void *data;

   double x, y;
   int r, g, b, a;

   Eina_Bool visibility : 1;
   Eina_Bool changed : 1;
};

struct _Evas_VG_Container_Data
{
   Eina_List *children;
};

struct _Evas_VG_Gradient_Data
{
   // FIXME: Later on we should deduplicate it somehow (Using Ector ?).
   Efl_Graphics_Gradient_Stop *colors;
   unsigned int colors_count;

   Efl_Graphics_Gradient_Spread s;
};

static inline void
_evas_vg_render_pre(Evas_VG_Node *child, Ector_Surface *s)
{
   Evas_VG_Node_Data *child_nd;

   // FIXME: Prevent infinite loop
   child_nd = eo_data_scope_get(child, EVAS_VG_NODE_CLASS);
   child_nd->render_pre(child, s, child_nd->data, child_nd);
}

#endif
