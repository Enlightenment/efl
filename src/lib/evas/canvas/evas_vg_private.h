#ifndef EVAS_VG_PRIVATE_H_
# define EVAS_VG_PRIVATE_H_

#include <Ector.h>

typedef struct _Efl_Canvas_Vg_Node_Data             Efl_Canvas_Vg_Node_Data;
typedef struct _Efl_Canvas_Vg_Container_Data        Efl_Canvas_Vg_Container_Data;
typedef struct _Efl_Canvas_Vg_Gradient_Data         Efl_Canvas_Vg_Gradient_Data;
typedef struct _Efl_Canvas_Vg_Interpolation         Efl_Canvas_Vg_Interpolation;
typedef struct _Efl_Canvas_Vg_Object_Data           Efl_Canvas_Vg_Object_Data;

typedef struct _Vg_Cache
{
   Eina_Hash             *vfd_hash;
   Eina_Hash             *vg_entry_hash;
   int                    ref;
} Vg_Cache;

typedef struct _Vg_Cache_Entry
{
   char                 *hash_key;
   const Eina_File      *file;
   Eina_Stringshare     *key;
   int                   w;
   int                   h;
   Efl_VG               *root[3];     //0: default, 1: start frame, 2: end frame
   int                   ref;
   Vg_File_Data         *vfd;

   Eina_Bool             mmap : 1;
} Vg_Cache_Entry;

// holds the vg tree info set by the user
typedef struct _Vg_User_Entry
{
   int                   w; // current surface width
   int                   h; // current surface height
   Efl_VG               *root;
} Vg_User_Entry;

struct _Efl_Canvas_Vg_Object_Data
{
   Efl_VG                    *root;
   Vg_Cache_Entry            *vg_entry;
   Vg_User_Entry             *user_entry; //holds the user set vg tree
   Evas_Object_Protected_Data *obj;
   Eina_Rect                  fill;
   Eina_Rect                  viewbox;
   unsigned int               width, height;
   Eina_Array                 cleanup;
   double                     align_x, align_y;
   Efl_Canvas_Vg_Fill_Mode    fill_mode;
   int                        frame_idx;
   Eina_File                 *file;
   Eina_Stringshare          *key;
   int                        cached_frame_idx;

   Eina_Bool                  changed : 1;
};

struct _Efl_Canvas_Vg_Node_Data
{
   Eina_Matrix3 *m;
   Efl_Canvas_Vg_Interpolation *intp;

   Ector_Renderer *renderer;

   Efl_VG *vg_obj;
   Efl_Canvas_Vg_Object_Data *vd;

   void (*render_pre)(Evas_Object_Protected_Data *vg_pd, Efl_VG *node,
         Efl_Canvas_Vg_Node_Data *nd,
         void *engine, void *output, void *contenxt, Ector_Surface *surface,
         Eina_Matrix3 *ptransform, Ector_Buffer *mask, int mask_op, void *data);
   void *data;

   double x, y;
   int r, g, b, a;
   Efl_Gfx_Change_Flag flags;

   Eina_Bool visibility : 1;
   Eina_Bool changed : 1;
};

typedef struct _Vg_Mask
{
   Evas_Object_Protected_Data *vg_pd;  //Vector Object (for accessing backend engine)
   Ector_Buffer *buffer;               //Mask Ector Buffer
   void *pixels;                       //Mask pixel buffer (actual data)
   Eina_Rect bound;                    //Mask boundary
   Eina_List *target;                  //Mask target
   int option;                         //Mask option
} Vg_Mask;

struct _Efl_Canvas_Vg_Container_Data
{
   Eina_List *children;
   Eina_Hash *names;

   //Masking feature.
   Efl_Canvas_Vg_Node *mask_src;         //Mask Source
   Vg_Mask mask;                         //Mask source data
};

struct _Efl_Canvas_Vg_Gradient_Data
{
   // FIXME: Later on we should deduplicate it somehow (Using Ector ?).
   Efl_Gfx_Gradient_Stop *colors;
   unsigned int colors_count;

   Efl_Gfx_Gradient_Spread spread;
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
Vg_Cache_Entry*             evas_cache_vg_entry_resize(Vg_Cache_Entry *entry, int w, int h);
Vg_Cache_Entry*             evas_cache_vg_entry_create(const Eina_File *file, const char *key, int w, int h, Eina_Bool mmap);
Efl_VG*                     evas_cache_vg_tree_get(Vg_Cache_Entry *vg_entry, unsigned int frame_num);
void                        evas_cache_vg_entry_del(Vg_Cache_Entry *vg_entry);
Vg_File_Data *              evas_cache_vg_file_open(const Eina_File *file, const char *key, Eina_Bool mmap);
Eina_Bool                   evas_cache_vg_file_save(Efl_VG *root, int w, int h, const char *file, const char *key, const char *flags);
Eina_Bool                   evas_cache_vg_entry_file_save(Vg_Cache_Entry *vg_entry, const char *file, const char *key, const char *flags);
double                      evas_cache_vg_anim_duration_get(const Vg_Cache_Entry *vg_entry);
unsigned int                evas_cache_vg_anim_frame_count_get(const Vg_Cache_Entry *vg_entry);
Eina_Size2D                 evas_cache_vg_entry_default_size_get(const Vg_Cache_Entry *vg_entry);
void                        efl_canvas_vg_node_vg_obj_set(Efl_VG *node, Efl_VG *vg_obj, Efl_Canvas_Vg_Object_Data *vd);
void                        efl_canvas_vg_node_change(Efl_VG *node);
void                        efl_canvas_vg_container_vg_obj_update(Efl_VG *obj, Efl_Canvas_Vg_Node_Data *nd);

static inline void
efl_canvas_vg_object_change(Efl_Canvas_Vg_Object_Data *vd)
{
   if (!vd || vd->changed) return;
   vd->changed = EINA_TRUE;
   evas_object_change(vd->obj->object, vd->obj);
}

static inline Efl_Canvas_Vg_Node_Data *
_evas_vg_render_pre(Evas_Object_Protected_Data *vg_pd, Efl_VG *child,
                    void *engine, void *output, void *context,
                    Ector_Surface *surface,
                    Eina_Matrix3 *transform,
                    Ector_Buffer *mask, int mask_op)
{
   if (!child) return NULL;
   Efl_Canvas_Vg_Node_Data *nd = efl_data_scope_get(child, EFL_CANVAS_VG_NODE_CLASS);
   if (nd) nd->render_pre(vg_pd, child, nd,
                          engine, output, context, surface,
                          transform, mask, mask_op, nd->data);
   return nd;
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
