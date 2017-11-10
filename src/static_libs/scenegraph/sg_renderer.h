#ifndef SG_RENDERER_H
#define SG_RENDERER_H

#include "eina_list.h"
#include "sg_node.h"

typedef struct _SG_Renderer_Func      SG_Renderer_Func;
typedef struct _SG_Renderer           SG_Renderer;

SG_Renderer* sg_renderer_create(void);
void sg_renderer_destroy(SG_Renderer* obj);

// to enable subclassing

void sg_renderer_ctr(SG_Renderer* obj);
void sg_renderer_dtr(SG_Renderer* obj);

struct _SG_Renderer_Func
{
   void             (*dtr)(SG_Renderer *obj);
   void             (*root_node_set)(SG_Renderer *obj, SG_Root_Node *node);
   SG_Root_Node    *(*root_node_get)(const SG_Renderer *obj);
   void             (*node_updater_set)(SG_Renderer *obj, SG_Node_Updater *updater);
   SG_Node_Updater *(*node_updater_get)(const SG_Renderer *obj);
   void             (*node_changed)(SG_Renderer *obj, SG_Node *node, SG_Node_Dirty_State state);
   void             (*preprocess)(SG_Renderer *obj);
   void             (*render_scene)(SG_Renderer *obj);
   void             (*render)(SG_Renderer *obj);
   void             (*release_cached_resources)(SG_Renderer *obj);
// base class table
   SG_Renderer_Func  *base;
};

#define SG_RENDERER_CLASS                          \
SG_Renderer_Func         *fn;                      \
SG_Root_Node             *m_root_node;             \
SG_Node_Updater          *m_node_updater;          \
Eina_List                *m_nodes_to_preprocess;   \
Eina_List                *m_nodes_dont_preprocess; \
Eina_Bool                 m_changed_emitted : 1;   \
Eina_Bool                 m_is_rendering : 1;      \
Eina_Bool                 m_is_preprocessing : 1;

// enable subclass end

struct _SG_Renderer
{
   SG_RENDERER_CLASS
};

#endif // SG_RENDERER_H
