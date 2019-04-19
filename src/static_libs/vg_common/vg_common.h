#ifndef VG_COMMON_H_
#define VG_COMMON_H_

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"

/******************************************************************************************
 * SVG Compatible feature implementation
 ******************************************************************************************/
typedef enum _Svg_Node_Type                Svg_Node_Type;
typedef enum _Svg_Length_Type              Svg_Length_Type;

typedef struct _Svg_Node                   Svg_Node;
typedef struct _Svg_Doc_Node               Svg_Doc_Node;
typedef struct _Svg_G_Node                 Svg_G_Node;
typedef struct _Svg_Defs_Node              Svg_Defs_Node;
typedef struct _Svg_Arc_Node               Svg_Arc_Node;
typedef struct _Svg_Circle_Node            Svg_Circle_Node;
typedef struct _Svg_Ellipse_Node           Svg_Ellipse_Node;
typedef struct _Svg_Polygon_Node           Svg_Polygon_Node;
typedef struct _Svg_Rect_Node              Svg_Rect_Node;
typedef struct _Svg_Path_Node              Svg_Path_Node;
typedef struct _Svg_Style_Property         Svg_Style_Property;
typedef struct _Svg_Line_Node              Svg_Line_Node;
typedef struct _Svg_Custom_Command_Node    Svg_Custom_Command_Node;

typedef struct  _Svg_Style_Stroke          Svg_Style_Stroke;
typedef struct  _Svg_Style_Fill            Svg_Style_Fill;
typedef enum    _Svg_Fill_Flags            Svg_Fill_Flags;
typedef enum    _Svg_Stroke_Flags          Svg_Stroke_Flags;

typedef enum   _Svg_Gradient_Type          Svg_Gradient_Type;
typedef struct _Svg_Style_Gradient         Svg_Style_Gradient;
typedef struct _Svg_Linear_Gradient        Svg_Linear_Gradient;
typedef struct _Svg_Radial_Gradient        Svg_Radial_Gradient;
typedef struct _Svg_Paint                  Svg_Paint;

enum _Svg_Node_Type
{
   SVG_NODE_DOC,
   SVG_NODE_G,
   SVG_NODE_DEFS,
   SVG_NODE_SWITCH,
   SVG_NODE_ANIMATION,
   SVG_NODE_ARC,
   SVG_NODE_CIRCLE,
   SVG_NODE_ELLIPSE,
   SVG_NODE_IMAGE,
   SVG_NODE_LINE,
   SVG_NODE_PATH,
   SVG_NODE_POLYGON,
   SVG_NODE_POLYLINE,
   SVG_NODE_RECT,
   SVG_NODE_TEXT,
   SVG_NODE_TEXTAREA,
   SVG_NODE_TSPAN,
   SVG_NODE_USE,
   SVG_NODE_VIDEO,
   SVG_NODE_CUSTOME_COMMAND,
   SVG_NODE_UNKNOWN
};

enum _Svg_Length_Type
{
   SVG_LT_PERCENT,
   SVG_LT_PX,
   SVG_LT_PC,
   SVG_LT_PT,
   SVG_LT_MM,
   SVG_LT_CM,
   SVG_LT_IN,
};

struct _Svg_Doc_Node
{
   double width;
   double height;
   double vx;
   double vy;
   double vw;
   double vh;
   Eina_Bool preserve_aspect;
   Svg_Node *defs;
};

struct _Svg_G_Node
{

};

struct _Svg_Defs_Node
{
   Eina_List   *gradients;
};

struct _Svg_Arc_Node
{

};

struct _Svg_Ellipse_Node
{
   double cx;
   double cy;
   double rx;
   double ry;
};

struct _Svg_Circle_Node
{
   double cx;
   double cy;
   double r;
};

struct _Svg_Rect_Node
{
   double x;
   double y;
   double w;
   double h;
   double rx;
   double ry;
};

struct _Svg_Line_Node
{
   double x1;
   double y1;
   double x2;
   double y2;
};

struct _Svg_Custom_Command_Node
{
   int                         points_count;
   int                         commands_count;
   double                     *points;
   Efl_Gfx_Path_Command       *commands;
};

struct _Svg_Path_Node
{
   Eina_Stringshare *path;
};

struct _Svg_Polygon_Node
{
   int points_count;
   double *points;
};


enum _Svg_Gradient_Type
{
   SVG_LINEAR_GRADIENT,
   SVG_RADIAL_GRADIENT
};
struct _Svg_Linear_Gradient
{
   double x1;
   double y1;
   double x2;
   double y2;
};

struct _Svg_Radial_Gradient
{
   double cx;
   double cy;
   double fx;
   double fy;
   double r;
};

/**
 * IMPORTANT!
 * Talking about parsing gradient variables
 *
 * All variables (like x1,x2,y1,y2,fx,fy,rx,ry,r, etc) would be percentages
 * and then all recalculations would be done after that if userSpaceOnUse
 * is set or not (recalculation depends on that).
 *
 * If gradientUnits="userSpaceOnUse" (grad->user_space is set to true)
 * > Gradient variables (x1,x2,r,fx etc) contains percentages of entire cavas
 * > size.
 *
 * If gradientUnits="objectBoundingBox" (grad->user_space is set to false)
 * > Gradient variables (x1,x2,r,fx etc) contain percentages of 'whatever'
 * > figure.
 *
 * So later on, while using gradient, please be careful and
 * check user_space to use and transform sizes correctly.
 */
struct _Svg_Style_Gradient
{
   Svg_Gradient_Type type;
   Eina_Stringshare  *id;
   Eina_Stringshare  *ref;
   Efl_Gfx_Gradient_Spread spread;
   Eina_List   *stops; // Efl_Gfx_Gradient_Stop
   Svg_Radial_Gradient *radial;
   Svg_Linear_Gradient *linear;
   Eina_Matrix3 *transform;
   Eina_Bool user_space;
   Eina_Bool use_percentage;
};

struct _Svg_Paint
{
   int        r;
   int        g;
   int        b;
   Eina_Bool  none;
   Eina_Bool  cur_color;
   Svg_Style_Gradient  *gradient;
   Eina_Stringshare    *url;
};

enum _Svg_Fill_Flags
{
   SVG_FILL_FLAGS_PAINT     = 0x1,
   SVG_FILL_FLAGS_OPACITY   = 0x2,
   SVG_FILL_FLAGS_GRADIENT  = 0x4,
   SVG_FILL_FLAGS_FILL_RULE = 0x8
};

enum _Svg_Stroke_Flags
{
   SVG_STROKE_FLAGS_PAINT    = 0x1,
   SVG_STROKE_FLAGS_OPACITY  = 0x2,
   SVG_STROKE_FLAGS_GRADIENT = 0x4,
   SVG_STROKE_FLAGS_SCALE    = 0x8,
   SVG_STROKE_FLAGS_WIDTH    = 0x10,
   SVG_STROKE_FLAGS_CAP      = 0x20,
   SVG_STROKE_FLAGS_JOIN     = 0x40,
   SVG_STROKE_FLAGS_DASH     = 0x80,
};

struct _Svg_Style_Fill
{
   Svg_Fill_Flags       flags;
   Svg_Paint            paint;
   int                  opacity;
   Efl_Gfx_Fill_Rule    fill_rule;
};

struct _Svg_Style_Stroke
{
   Svg_Stroke_Flags     flags;
   Svg_Paint            paint;
   int                  opacity;
   double               scale;
   double               width;
   double               centered;
   Efl_Gfx_Cap          cap;
   Efl_Gfx_Join         join;
   Efl_Gfx_Dash        *dash;
   int                  dash_count;
};

struct _Svg_Style_Property
{
   Svg_Style_Fill     fill;
   Svg_Style_Stroke   stroke;
   // the color property indirectly 
   // used by fill and stroke
   int                r;
   int                g;
   int                b;
};

struct _Svg_Node
{
   Svg_Node_Type        type;
   Svg_Node            *parent;
   Eina_List           *child;
   Eina_Stringshare    *id;
   Svg_Style_Property  *style;
   Eina_Matrix3        *transform;
   union
     {
        Svg_G_Node   g;
        Svg_Doc_Node doc;
        Svg_Defs_Node defs;
        Svg_Arc_Node arc;
        Svg_Circle_Node circle;
        Svg_Ellipse_Node ellipse;
        Svg_Polygon_Node polygon;
        Svg_Polygon_Node polyline;
        Svg_Rect_Node rect;
        Svg_Path_Node path;
        Svg_Line_Node line;
        Svg_Custom_Command_Node command;
     }node;
};

enum _Svg_Style_Type
{
   SVG_STYLE_QUALITY,
   SVG_STYLE_FILL,
   SVG_STYLE_VIEWPORT_FILL,
   SVG_STYLE_FONT,
   SVG_STYLE_STROKE,
   SVG_STYLE_SOLID_COLOR,
   SVG_STYLE_GRADIENT,
   SVG_STYLE_TRANSFORM,
   SVG_STYLE_OPACITY,
   SVG_STYLE_COMP_OP
};

Eet_Data_Descriptor * vg_common_svg_node_eet(void);
void vg_common_svg_node_eet_destroy(void);
Vg_File_Data * vg_common_svg_create_vg_node(Svg_Node *node);
Svg_Node *vg_common_svg_create_svg_node(Vg_File_Data *node);
void vg_common_svg_node_free(Svg_Node *node);

#endif //EVAS_VG_COMMON_H_
