#ifndef EVAS_VG_PRIVATE_H_
# define EVAS_VG_PRIVATE_H_

#include <Ector.h>

typedef struct _Efl_Canvas_Vg_Node_Data             Efl_Canvas_Vg_Node_Data;
typedef struct _Efl_Canvas_Vg_Container_Data        Efl_Canvas_Vg_Container_Data;
typedef struct _Efl_Canvas_Vg_Gradient_Data         Efl_Canvas_Vg_Gradient_Data;
typedef struct _Efl_Canvas_Vg_Interpolation         Efl_Canvas_Vg_Interpolation;


typedef struct _Efl_Canvas_Vg_Object_Data           Efl_Canvas_Vg_Object_Data;

typedef struct _Evas_Cache_Vg_Entry          Evas_Cache_Vg_Entry;
typedef struct _Evas_Cache_Vg                Evas_Cache_Vg;

struct _Evas_Cache_Vg
{
   Eina_Hash             *vg_hash;
   Eina_Hash             *active;
   int                    ref;
};

struct _Evas_Cache_Vg_Entry
{
   char                 *hash_key;
   Eina_Stringshare     *file;
   Eina_Stringshare     *key;
   int                   w;
   int                   h;
   Efl_VG               *root;
   int                   ref;
};

typedef struct _User_Vg_Entry
{
   int                   w; // current surface width
   int                   h; // current surface height
   Efl_VG               *root;
}User_Vg_Entry; // holds the vg tree info set by the user

struct _Efl_Canvas_Vg_Object_Data
{
   void                     *engine_data;
   Efl_VG                   *root;
   Evas_Cache_Vg_Entry      *vg_entry;
   User_Vg_Entry            *user_entry; // holds the user set vg tree
   Eina_Rect                 fill;
   Eina_Rect                 viewbox;
   unsigned int              width, height;
   Eina_Array                cleanup;
   double                    align_x, align_y;
   Efl_Canvas_Vg_Fill_Mode   fill_mode;
   Eina_Bool                 changed;
};

struct _Efl_Canvas_Vg_Node_Data
{
   Eina_Matrix3 *m;
   Efl_Canvas_Vg_Interpolation *intp;

   Efl_Canvas_Vg_Node *mask;
   Ector_Renderer *renderer;

   Efl_VG *vg_obj;

   void (*render_pre)(Eo *obj, Eina_Matrix3 *parent, Ector_Surface *s, void *data, Efl_Canvas_Vg_Node_Data *nd);
   void *data;

   double x, y;
   int r, g, b, a;
   Efl_Gfx_Change_Flag flags;

   Eina_Bool visibility : 1;
   Eina_Bool changed : 1;
   Eina_Bool parenting : 1;
};

struct _Efl_Canvas_Vg_Container_Data
{
   Eina_List *children;

   Eina_Hash *names;
};

struct _Efl_Canvas_Vg_Gradient_Data
{
   // FIXME: Later on we should deduplicate it somehow (Using Ector ?).
   Efl_Gfx_Gradient_Stop *colors;
   unsigned int colors_count;

   Efl_Gfx_Gradient_Spread s;
};

struct _Efl_Canvas_Vg_Interpolation
{
   Eina_Quaternion rotation;
   Eina_Quaternion perspective;
   Eina_Point_3D translation;
   Eina_Point_3D scale;
   Eina_Point_3D skew;
};


void                        evas_cache_vg_init(void);
void                        evas_cache_vg_shutdown(void);
Evas_Cache_Vg_Entry*        evas_cache_vg_entry_find(const char *file, const char *key, int w, int h);
Efl_VG*                     evas_cache_vg_tree_get(Evas_Cache_Vg_Entry *svg_entry);
void                        evas_cache_vg_entry_del(Evas_Cache_Vg_Entry *svg_entry);
Vg_File_Data *              evas_cache_vg_file_info(const char *file, const char *key);

Eina_Bool                   evas_vg_save_to_file(Vg_File_Data *evg_data, const char *file, const char *key, const char *flags);

void                        efl_canvas_vg_node_root_set(Efl_VG *node, Efl_VG *vg_obj);

static inline Efl_Canvas_Vg_Node_Data *
_evas_vg_render_pre(Efl_VG *child, Ector_Surface *s, Eina_Matrix3 *m)
{
   if (!child) return NULL;

   Efl_Canvas_Vg_Node_Data *child_nd = efl_data_scope_get(child, EFL_CANVAS_VG_NODE_CLASS);
   if (child_nd) child_nd->render_pre(child, m, s, child_nd->data, child_nd);
   return child_nd;
}

static inline void
_efl_canvas_vg_node_changed(Eo *obj)
{
   Efl_Gfx_Path_Change_Event ev = { EFL_GFX_CHANGE_FLAG_FILL };

   if (obj) efl_event_callback_call(obj, EFL_GFX_PATH_EVENT_CHANGED, &ev);
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
   if (*obj) efl_unref(*obj);
   *obj = NULL;
}

#define EFL_CANVAS_VG_COMPUTE_MATRIX(Current, Parent, Nd)                      \
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
