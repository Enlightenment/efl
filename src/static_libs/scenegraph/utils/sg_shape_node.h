#ifndef SG_SHAPE_NODE_H
#define SG_SHAPE_NODE_H

#include "sg_node.h"

typedef struct _SG_Shape_Node_Func                  SG_Shape_Node_Func;
typedef struct _SG_Shape_Node                       SG_Shape_Node;

struct  SG_Shape_Node_Func
{
   void              (rect_set)(SG_Shape_Node *obj, Eina_Rect rect); /* pure virtual */
   Eina_Rect         (rect_get)(const SG_Shape_Node *obj); /* pure virtual */
   void              (color_set)(SG_Shape_Node *obj, int r, int g, int b , int a); /* pure virtual */
   void              (color_get)(SG_Shape_Node *obj, int *r, int *g, int *b , int *a); /* pure virtual */
   void              (stroke_color_set)(SG_Shape_Node *obj, int r, int g, int b , int a); /* pure virtual */
   void              (stroke_color_get)(SG_Shape_Node *obj, int *r, int *g, int *b , int *a); /* pure virtual */
   void              (stroke_width_set)(SG_Shape_Node *obj, double width); /* pure virtual */
   double            (stroke_width_get)(const SG_Shape_Node *obj); /* pure virtual */

};

#define SG_Shape_Node_CLASS           \
   SG_GEOMETRY_NODE_CLASS             \
   SG_Shape_Node_Func        *mfn;

struct _SG_Shape_Node
{
   SG_Shape_Node_CLASS
};


#endif //SG_RECTANGLE_NODE_H