#ifndef SG_RECTANGLE_NODE_H
#define SG_RECTANGLE_NODE_H

#include "sg_node.h"

typedef struct _SG_Rectangle_Node_Func                  SG_Rectangle_Node_Func;
typedef struct _SG_Rectangle_Node                       SG_Rectangle_Node;

struct  SG_Rectangle_Node_Func
{
   void              (rect_set)(SG_Rectangle_Node *obj, Eina_Rect rect); /* pure virtual */
   Eina_Rect         (rect_get)(const SG_Rectangle_Node *obj); /* pure virtual */
   void              (color_set)(SG_Rectangle_Node *obj, int r, int g, int b , int a); /* pure virtual */
   void              (color_get)(SG_Rectangle_Node *obj, int *r, int *g, int *b , int *a); /* pure virtual */
};

#define SG_RECTANGLE_NODE_CLASS           \
   SG_GEOMETRY_NODE_CLASS                 \
   SG_Rectangle_Node_Func        *mfn;

struct _SG_Rectangle_Node
{
   SG_RECTANGLE_NODE_CLASS
};


#endif // SG_RECTANGLE_NODE_H