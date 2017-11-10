#ifndef SG_NODE_H
#define SG_NODE_H

#include <Eina.h>

typedef struct _SG_Node_Func                  SG_Node_Func;
typedef struct _SG_Node                       SG_Node;
typedef struct _SG_Basic_Geometry_Node        SG_Basic_Geometry_Node;
typedef struct _SG_Geometry_Node              SG_Geometry_Node;
typedef struct _SG_Clip_Node                  SG_Clip_Node;
typedef struct _SG_Transform_Node             SG_Transform_Node;
typedef struct _SG_Opacity_Node               SG_Opacity_Node;
typedef struct _SG_Root_Node                  SG_Root_Node;


typedef enum _SG_Node_Type          SG_Node_Type;
typedef enum _SG_Node_Dirty_State   SG_Node_Dirty_State;
typedef enum _SG_Node_Flag          SG_Node_Flag;

typedef struct _SG_Node_Updater         SG_Node_Updater;
typedef struct _SG_Node_Updater_Func    SG_Node_Updater_Func;

EAPI SG_Root_Node*       sg_node_root_create(void);
EAPI SG_Geometry_Node*   sg_node_geometry_create(void);
EAPI SG_Clip_Node*       sg_node_clip_create(void);
EAPI SG_Transform_Node*  sg_node_transform_create(void);
EAPI SG_Opacity_Node*    sg_node_opacity_create(void);
EAPI void                sg_node_destroy(SG_Node *obj);

// forward declaration
typedef struct _SG_Renderer SG_Renderer;
typedef struct _SG_Geometry SG_Geometry;

 // to enable subclassing
void sg_basic_geometry_node_ctr(SG_Basic_Geometry_Node* obj);
void sg_geometry_node_ctr(SG_Geometry_Node* obj);
// enable subclass end

enum _SG_Node_Type {
   SG_NODE_TYPE_BASIC,
   SG_NODE_TYPE_GEOMETRY,
   SG_NODE_TYPE_TRANSFORM,
   SG_NODE_TYPE_CLIP,
   SG_NODE_TYPE_OPACITY,
   SG_NODE_TYPE_ROOT,
   SG_NODE_TYPE_RENDER
};

enum _SG_Node_Flag {
   // Lower 16 bites reserved for general node
   SG_NODE_OWNED_BY_PARENT                = 0x0001,
   SG_NODE_USE_PREPROCESS                 = 0x0002,

   // 0x00ff0000 bits reserved for node subclasses

   // SG_Basic_Geometry_Node
   SG_NODE_OWNS_GEOMETRY                  = 0x00010000,
   SG_NODE_OWNS_MATERIAL                  = 0x00020000,
   SG_NODE_OWNS_OPAQUE_MATERIAL           = 0x00040000,
   ISG_NODE_IS_VISITABLE                  = 0x01000000
};

enum _SG_Node_Dirty_State
{
   SG_NODE_DIRTY_SUBTREE_BLOCKED        = 0x0080,
   SG_NODE_DIRTY_MATRIX                 = 0x0100,
   SG_NODE_DIRTY_NODE_ADDED             = 0x0400,
   SG_NODE_DIRTY_NODE_REMOVED           = 0x0800,
   SG_NODE_DIRTY_GEOMETRY               = 0x1000,
   SG_NODE_DIRTY_MATERIAL               = 0x2000,
   SG_NODE_DIRTY_OPACITY                = 0x4000,
   SG_NODE_DIRTY_FORCE_UPDATE           = 0x8000,
   SG_NODE_DIRTY_USE_PREPROCESS         = SG_NODE_USE_PREPROCESS,
   DirtyPropagationMask                 = SG_NODE_DIRTY_MATRIX
                                         | SG_NODE_DIRTY_NODE_ADDED
                                         | SG_NODE_DIRTY_OPACITY
                                         | SG_NODE_DIRTY_FORCE_UPDATE

};

struct _SG_Node_Func
{
   void          (*dtr)(SG_Node *obj); /* virtual */
   SG_Node_Type  (*type)(const SG_Node *obj);
   SG_Node      *(*parent)(const SG_Node *obj);
   void          (*remove_child)(SG_Node *obj, SG_Node *node);
   void          (*remove_child_all)(SG_Node *obj);
   void          (*prepend_child)(SG_Node *obj, SG_Node *node);
   void          (*append_child)(SG_Node *obj, SG_Node *node);
   void          (*insert_child_before)(SG_Node *obj, SG_Node *node, SG_Node *before);
   void          (*insert_child_after)(SG_Node *obj, SG_Node *node, SG_Node *after);
   void          (*reparent_childs_to)(SG_Node *obj, SG_Node *new_parent);
   int           (*child_count)(const SG_Node *obj);
   SG_Node      *(*child_at)(const SG_Node *obj, int i);
   void          (*mark_dirty)(SG_Node *obj, SG_Node_Dirty_State bits);
   void          (*flags_set)(SG_Node *obj, SG_Node_Flag flags, Eina_Bool enable);
   SG_Node_Flag  (*flags)(SG_Node *obj);
   void          (*destroy)(SG_Node *obj);
   Eina_Bool     (*is_subtree_blocked)(const SG_Node *obj); /* virtual */
   void          (*preprocess)(SG_Node *obj); /* virtual */

   // base table
   SG_Node_Func   *base;

   // SG_Transform_Node node api
   void                     (*matrix_set)(SG_Transform_Node *obj, const Eina_Matrix3 *matrix);
   const Eina_Matrix3 *     (*matrix_get)(const SG_Transform_Node *obj);
   void                     (*combined_matrix_set)(SG_Transform_Node *obj, const Eina_Matrix3 *matrix);
   const Eina_Matrix3 *     (*combined_matrix_get)(const SG_Transform_Node *obj);

   // SG_Opacity_Node api
   void                     (*opacity_set)(SG_Opacity_Node *obj, float opacity);
   float                    (*opacity_get)(const SG_Opacity_Node *obj);
   void                     (*combined_opacity_set)(SG_Opacity_Node *obj, float opacity);
   float                    (*combined_opacity_get)(const SG_Opacity_Node *obj);

   // SG_Clip_Node api
   void                     (*rect_hint_set)(SG_Clip_Node *obj, Eina_Bool rect_hint);
   Eina_Bool                (*rect_hint_get)(const SG_Clip_Node *obj);
   void                     (*clip_rect_set)(SG_Clip_Node *obj, Eina_Rect clip);
   Eina_Rect                (*clip_rect_get)(const SG_Clip_Node *obj);

   // SG_Basic_Geometry_Node api
   void                     (*clipper_set)(SG_Node *obj, SG_Node *clipper);
   const SG_Node*           (*clipper_get)(const SG_Node *obj);
   void                     (*render_matrix_set)(SG_Node *obj, const Eina_Matrix3 *matrix);
   const Eina_Matrix3*      (*render_matrix_get)(const SG_Node *obj);
   void                     (*geometry_set)(SG_Node *obj, SG_Geometry *geometry);
   const SG_Geometry*       (*geometry_get)(const SG_Node *obj);

   // SG_Geometry_Node api
   void                     (*inherited_opacity_set)(SG_Node *obj, float opacity);
   float                    (*inherited_opacity_get)(const SG_Node *obj);
};

#define SG_NODE_DATA                                 \
   SG_Node_Func         *fn;                         \
   SG_Node              *m_parent;                   \
   SG_Node_Type          m_type;                     \
   SG_Node              *m_first_child;              \
   SG_Node              *m_last_child;               \
   SG_Node              *m_next_sibling;             \
   SG_Node              *m_previous_sibling;         \
   int                   m_subtree_renderable_count; \
   SG_Node_Flag          m_node_flags;

#define SG_BASIC_GEOMETRY_NODE_DATA      \
   SG_NODE_DATA                          \
   const SG_Geometry        *m_geometry; \
   const Eina_Matrix3       *m_matrix;   \
   const SG_Node            *m_clipper;

#define SG_GEOMETRY_NODE_CLASS           \
   SG_BASIC_GEOMETRY_NODE_DATA           \
   float        m_opacity;

struct _SG_Node
{
   SG_NODE_DATA
};


struct _SG_Basic_Geometry_Node
{
   SG_BASIC_GEOMETRY_NODE_DATA
};

struct _SG_Geometry_Node
{
   SG_GEOMETRY_NODE_CLASS
};

struct _SG_Clip_Node
{
   SG_BASIC_GEOMETRY_NODE_DATA
   Eina_Rect    m_clip_rect;
   Eina_Bool    m_is_rectangular : 1;
};

struct _SG_Transform_Node
{
   SG_NODE_DATA
   Eina_Matrix3  *m_matrix;
   Eina_Matrix3  *m_combined_matrix;
};

struct _SG_Opacity_Node
{
   SG_NODE_DATA
   float   m_opacity;
   float   m_combined_opacity;
};

struct _SG_Root_Node
{
   SG_NODE_DATA
   SG_Renderer *m_renderer;
   Eina_Bool    m_in_dtr;
};


// SG_Node_Updater
void sg_node_updater_ctr(SG_Node_Updater* obj);

SG_Node_Updater * sg_node_updater_create(void);
void              sg_node_updater_destroy(SG_Node_Updater *obj);

struct _SG_Node_Updater_Func
{
   void            (*dtr)(SG_Node_Updater *obj);
   void            (*update_states)(SG_Node_Updater *obj, SG_Node *n);
   Eina_Bool       (*is_node_blocked)(const SG_Node_Updater *obj, SG_Node *n, SG_Node *root);
   // base table
   SG_Node_Updater_Func *base;
};

#define SG_NODE_UPDATOR_CLASS                            \
   SG_Node_Updater_Func     *fn;                         \
   Eina_Array               *m_combined_matrix_stack;    \
   Eina_Inarray             *m_opacity_stack;            \
   const SG_Node            *m_current_clip;             \
   int                       m_force_update;

struct _SG_Node_Updater
{
   SG_NODE_UPDATOR_CLASS
};
#endif // SG_NODE_H
