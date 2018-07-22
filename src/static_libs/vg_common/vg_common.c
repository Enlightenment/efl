#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "vg_common.h"

#include <Eet.h>
#include <Evas.h>



Eet_Data_Descriptor *_eet_rect_node = NULL;
Eet_Data_Descriptor *_eet_circle_node = NULL;
Eet_Data_Descriptor *_eet_ellipse_node = NULL;
Eet_Data_Descriptor *_eet_gradient_stops_node = NULL;
Eet_Data_Descriptor *_eet_linear_gradient_node = NULL;
Eet_Data_Descriptor *_eet_radial_gradient_node = NULL;
Eet_Data_Descriptor *_eet_style_gradient_node = NULL;
Eet_Data_Descriptor *_eet_style_property_node = NULL;
Eet_Data_Descriptor *_eet_matrix3_node = NULL;
Eet_Data_Descriptor *_eet_doc_node = NULL;
Eet_Data_Descriptor *_eet_defs_node = NULL;
Eet_Data_Descriptor *_eet_g_node = NULL;
Eet_Data_Descriptor *_eet_arc_node = NULL;
Eet_Data_Descriptor *_eet_path_node = NULL;
Eet_Data_Descriptor *_eet_polygon_node = NULL;
Eet_Data_Descriptor *_eet_vg_node = NULL;
Eet_Data_Descriptor *_eet_line_node = NULL;
Eet_Data_Descriptor *_eet_custom_command_node = NULL;

#define FREE_DESCRIPTOR(eed)                      \
  if (eed)                              \
    {                                   \
       eet_data_descriptor_free((eed)); \
       (eed) = NULL;                    \
    }

static inline Eet_Data_Descriptor*
_eet_for_rect_node(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Rect_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Rect_Node, "x", x, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Rect_Node, "y", y, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Rect_Node, "w", w, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Rect_Node, "h", h, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Rect_Node, "rx", rx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Rect_Node, "ry", ry, EET_T_DOUBLE);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_line_node(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Line_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Line_Node, "x1", x1, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Line_Node, "y1", y1, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Line_Node, "x2", x2, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Line_Node, "y2", y2, EET_T_DOUBLE);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_circle_node(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Circle_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Circle_Node, "cx", cx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Circle_Node, "cy", cy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Circle_Node, "r", r, EET_T_DOUBLE);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_ellipse_node(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Ellipse_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Ellipse_Node, "cx", cx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Ellipse_Node, "cy", cy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Ellipse_Node, "rx", rx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Ellipse_Node, "ry", ry, EET_T_DOUBLE);
   return eet;
}


static inline Eet_Data_Descriptor*
_eet_for_gradient_stops(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Efl_Gfx_Gradient_Stop);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Efl_Gfx_Gradient_Stop, "offset", offset, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Efl_Gfx_Gradient_Stop, "r", r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Efl_Gfx_Gradient_Stop, "g", g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Efl_Gfx_Gradient_Stop, "b", b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Efl_Gfx_Gradient_Stop, "a", a, EET_T_INT);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_linear_gradient(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Linear_Gradient);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Linear_Gradient, "x1", x1, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Linear_Gradient, "y1", y1, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Linear_Gradient, "x2", x2, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Linear_Gradient, "y2", y2, EET_T_DOUBLE);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_radial_gradient(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Radial_Gradient);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Radial_Gradient, "cx", cx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Radial_Gradient, "cy", cy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Radial_Gradient, "fx", fx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Radial_Gradient, "fy", fy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Radial_Gradient, "r", r, EET_T_DOUBLE);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_style_gradient(void)
{
   Eet_Data_Descriptor_Class eetc;

   if (_eet_style_gradient_node) return _eet_style_gradient_node;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Style_Gradient);
   _eet_style_gradient_node = eet_data_descriptor_stream_new(&eetc);
   _eet_gradient_stops_node = _eet_for_gradient_stops();
   _eet_linear_gradient_node = _eet_for_linear_gradient();
   _eet_radial_gradient_node = _eet_for_radial_gradient();

   EET_DATA_DESCRIPTOR_ADD_BASIC(_eet_style_gradient_node, Svg_Style_Gradient, "type", type, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_eet_style_gradient_node, Svg_Style_Gradient, "id", id, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_eet_style_gradient_node, Svg_Style_Gradient, "spread", spread, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_LIST(_eet_style_gradient_node, Svg_Style_Gradient, "stops", stops, _eet_gradient_stops_node);
   EET_DATA_DESCRIPTOR_ADD_SUB(_eet_style_gradient_node, Svg_Style_Gradient, "radial", radial, _eet_radial_gradient_node);
   EET_DATA_DESCRIPTOR_ADD_SUB(_eet_style_gradient_node, Svg_Style_Gradient, "linear", linear, _eet_linear_gradient_node);

   return _eet_style_gradient_node;
}

static inline Eet_Data_Descriptor*
_eet_for_style_property(void)
{
   Eet_Data_Descriptor *eet, *eet_gradient, *eet_dash;
   Eet_Data_Descriptor_Class eetc, eetc_dash;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Style_Property);
   eet = eet_data_descriptor_stream_new(&eetc);
   eet_gradient = _eet_for_style_gradient();

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc_dash, Efl_Gfx_Dash);
   eet_dash = eet_data_descriptor_stream_new(&eetc_dash);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet_dash, Efl_Gfx_Dash, "length", length, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet_dash, Efl_Gfx_Dash, "gap", gap, EET_T_DOUBLE);

   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "r", r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "g", g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "b", b, EET_T_INT);
   // for fill
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "fill.flags", fill.flags, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "fill.paint.r", fill.paint.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "fill.paint.g", fill.paint.g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "fill.paint.b", fill.paint.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "fill.paint.none", fill.paint.none, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "fill.paint.cur_color", fill.paint.cur_color, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_SUB(eet, Svg_Style_Property, "fill.paint.gradient", fill.paint.gradient, eet_gradient);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "fill.paint.url", fill.paint.url, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "fill.opacity", fill.opacity, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "fill.fill_rule", fill.fill_rule, EET_T_INT);

   // for stroke
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.flags", stroke.flags, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.paint.r", stroke.paint.r, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.paint.g", stroke.paint.g, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.paint.b", stroke.paint.b, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.paint.none", stroke.paint.none, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.paint.cur_color", stroke.paint.cur_color, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_SUB(eet, Svg_Style_Property, "stroke.paint.gradient", stroke.paint.gradient, eet_gradient);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.paint.url", stroke.paint.url, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.opacity", stroke.opacity, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.scale", stroke.scale, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.width", stroke.width, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.centered", stroke.centered, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.cap", stroke.cap, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.join", stroke.join, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY(eet, Svg_Style_Property, "stroke.dash", stroke.dash, eet_dash);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Style_Property, "stroke.dash_count", stroke.dash_count, EET_T_INT);

   return eet;
}

static Eet_Data_Descriptor*
_eet_for_eina_matrix3(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Eina_Matrix3);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Eina_Matrix3, "xx", xx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Eina_Matrix3, "xy", xy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Eina_Matrix3, "xz", xz, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Eina_Matrix3, "yx", yx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Eina_Matrix3, "yy", yy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Eina_Matrix3, "yz", yz, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Eina_Matrix3, "zx", zx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Eina_Matrix3, "zy", zy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Eina_Matrix3, "zz", zz, EET_T_DOUBLE);

   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_doc_node(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Doc_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Doc_Node, "width", width, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Doc_Node, "height", height, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Doc_Node, "vx", vx, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Doc_Node, "vy", vy, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Doc_Node, "vw", vw, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Doc_Node, "vh", vh, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Doc_Node, "preserve_aspect", preserve_aspect, EET_T_INT);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_defs_node(void)
{
   Eet_Data_Descriptor *eet, *eet_gradient;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Defs_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   eet_gradient = _eet_for_style_gradient();

   EET_DATA_DESCRIPTOR_ADD_LIST(eet, Svg_Defs_Node, "gradients", gradients, eet_gradient);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_g_node(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_G_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_arc_node(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Arc_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_polygon_node(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Polygon_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Polygon_Node, "points_count", points_count, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC_VAR_ARRAY(eet, Svg_Polygon_Node, "points", points, EET_T_DOUBLE);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_custom_command_node(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Custom_Command_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Custom_Command_Node, "points_count", points_count, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC_VAR_ARRAY(eet, Svg_Custom_Command_Node, "points", points, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Custom_Command_Node, "commands_count", commands_count, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC_VAR_ARRAY(eet, Svg_Custom_Command_Node, "commands", commands, EET_T_INT);
   return eet;
}

static inline Eet_Data_Descriptor*
_eet_for_path_node(void)
{
   Eet_Data_Descriptor *eet;
   Eet_Data_Descriptor_Class eetc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Path_Node);
   eet = eet_data_descriptor_stream_new(&eetc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(eet, Svg_Path_Node, "path", path, EET_T_STRING);
   return eet;
}

struct
{
   Svg_Node_Type u;
   const char       *name;
} eet_mapping[] = {
   { SVG_NODE_DOC, "doc" },
   { SVG_NODE_G, "g" },
   { SVG_NODE_DEFS, "defs" },
   { SVG_NODE_ARC, "arc" },
   { SVG_NODE_CIRCLE, "circle" },
   { SVG_NODE_ELLIPSE, "ellipse" },
   { SVG_NODE_POLYGON, "polygon" },
   { SVG_NODE_POLYLINE, "polyline" },
   { SVG_NODE_RECT, "rect" },
   { SVG_NODE_PATH, "path" },
   { SVG_NODE_LINE, "line" },
   { SVG_NODE_CUSTOME_COMMAND, "command" },
   { SVG_NODE_UNKNOWN, NULL }
};

static const char *
/* union
   type_get() */
_union_type_get(const void *data,
                Eina_Bool  *unknow)
{
   const Svg_Node_Type *u = data;
   int i;

   if (unknow)
     *unknow = EINA_FALSE;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (*u == eet_mapping[i].u)
       return eet_mapping[i].name;

   if (unknow)
     *unknow = EINA_TRUE;
   return NULL;
} /* _union_type_get */

static Eina_Bool
_union_type_set(const char *type,
                void       *data,
                Eina_Bool   unknow)
{
   Svg_Node_Type *u = data;
   int i;

   if (unknow)
     return EINA_FALSE;

   for (i = 0; eet_mapping[i].name != NULL; ++i)
     if (strcmp(eet_mapping[i].name, type) == 0)
       {
          *u = eet_mapping[i].u;
          return EINA_TRUE;
       }
   return EINA_FALSE;
} /* _union_type_set */

Eet_Data_Descriptor *
vg_common_svg_node_eet(void)
{
   Eet_Data_Descriptor *eet_union;
   Eet_Data_Descriptor_Class eetc;

   if (_eet_vg_node) return _eet_vg_node;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eetc, Svg_Node);
   _eet_vg_node = eet_data_descriptor_stream_new(&eetc);

   eetc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eetc.func.type_get = _union_type_get;
   eetc.func.type_set = _union_type_set;
   eet_union = eet_data_descriptor_stream_new(&eetc);

   _eet_doc_node = _eet_for_doc_node();
   _eet_g_node = _eet_for_g_node();
   _eet_defs_node = _eet_for_defs_node();
   _eet_arc_node = _eet_for_arc_node();
   _eet_circle_node = _eet_for_circle_node();
   _eet_ellipse_node = _eet_for_ellipse_node();
   _eet_rect_node = _eet_for_rect_node();
   _eet_line_node = _eet_for_line_node();
   _eet_path_node = _eet_for_path_node();
   _eet_polygon_node = _eet_for_polygon_node();
   _eet_custom_command_node = _eet_for_custom_command_node();
   _eet_style_property_node = _eet_for_style_property();
   _eet_matrix3_node = _eet_for_eina_matrix3();



   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "doc", _eet_doc_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "g", _eet_g_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "defs", _eet_defs_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "arc", _eet_arc_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "circle", _eet_circle_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "ellipse", _eet_ellipse_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "rect", _eet_rect_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "line", _eet_line_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "path", _eet_path_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "polygon", _eet_polygon_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "polyline", _eet_polygon_node);
   EET_DATA_DESCRIPTOR_ADD_MAPPING(eet_union, "command", _eet_custom_command_node);

   EET_DATA_DESCRIPTOR_ADD_UNION(_eet_vg_node, Svg_Node, "node", node, type, eet_union);


   EET_DATA_DESCRIPTOR_ADD_LIST(_eet_vg_node, Svg_Node, "child", child, _eet_vg_node);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_eet_vg_node, Svg_Node, "id", id, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_SUB(_eet_vg_node, Svg_Node, "style", style, _eet_style_property_node);
   EET_DATA_DESCRIPTOR_ADD_SUB(_eet_vg_node, Svg_Node, "transform", transform, _eet_matrix3_node);

   return _eet_vg_node;
}

void 
vg_common_svg_node_eet_destroy(void)
{
   FREE_DESCRIPTOR(_eet_rect_node);
   FREE_DESCRIPTOR(_eet_circle_node);
   FREE_DESCRIPTOR(_eet_ellipse_node);
   FREE_DESCRIPTOR(_eet_gradient_stops_node);
   FREE_DESCRIPTOR(_eet_linear_gradient_node);
   FREE_DESCRIPTOR(_eet_radial_gradient_node);
   FREE_DESCRIPTOR(_eet_style_gradient_node);
   FREE_DESCRIPTOR(_eet_style_property_node);
   FREE_DESCRIPTOR(_eet_matrix3_node);
   FREE_DESCRIPTOR(_eet_doc_node);
   FREE_DESCRIPTOR(_eet_defs_node);
   FREE_DESCRIPTOR(_eet_g_node);
   FREE_DESCRIPTOR(_eet_arc_node);
   FREE_DESCRIPTOR(_eet_path_node);
   FREE_DESCRIPTOR(_eet_polygon_node);
   FREE_DESCRIPTOR(_eet_vg_node);
   FREE_DESCRIPTOR(_eet_line_node);
   FREE_DESCRIPTOR(_eet_custom_command_node);
}

static void
_svg_style_gradient_free(Svg_Style_Gradient *grad)
{
   Efl_Gfx_Gradient_Stop *stop;

   if (!grad) return;

   eina_stringshare_del(grad->id);
   eina_stringshare_del(grad->ref);
   free(grad->radial);
   free(grad->linear);

   EINA_LIST_FREE(grad->stops, stop)
     {
        free(stop);
     }
   free(grad);
}

static void
_node_style_free(Svg_Style_Property *style)
{
   if (!style) return;

   _svg_style_gradient_free(style->fill.paint.gradient);
   eina_stringshare_del(style->fill.paint.url);
   _svg_style_gradient_free(style->stroke.paint.gradient);
   eina_stringshare_del(style->stroke.paint.url);
   free(style);
}

void
vg_common_svg_node_free(Svg_Node *node)
{
   Svg_Node *child;
   Svg_Style_Gradient *grad;

   if (!node) return;

   EINA_LIST_FREE(node->child, child)
     {
        vg_common_svg_node_free(child);
     }

   eina_stringshare_del(node->id);
   free(node->transform);
   _node_style_free(node->style);
   switch (node->type)
     {
        case SVG_NODE_PATH:
           eina_stringshare_del(node->node.path.path);
           break;
        case SVG_NODE_POLYGON:
           free(node->node.polygon.points);
           break;
        case SVG_NODE_POLYLINE:
           free(node->node.polyline.points);
           break;
        case SVG_NODE_DOC:
           vg_common_svg_node_free(node->node.doc.defs);
           break;
        case SVG_NODE_DEFS:
           EINA_LIST_FREE(node->node.defs.gradients, grad)
             {
                _svg_style_gradient_free(grad);
             }
           break;
        default:
           break;
     }
  free(node);
}

static Efl_VG *
_apply_gradient_property(Svg_Style_Gradient *g, Efl_VG *vg, Efl_VG *parent, Vg_File_Data *vg_data)
{
   Efl_VG *grad_obj = NULL;
   Efl_Gfx_Gradient_Stop *stops, *stop;
   int stop_count = 0, i = 0;
   Eina_List *l;
   Eina_Matrix3 m; //for bbox translation
   Eina_Rect r = EINA_RECT( 0, 0, 1, 1 );
   Eina_Rect grad_geom = EINA_RECT(0, 0, 0, 0);
   int radius;

   //TODO: apply actual sizes (imporve bounds_get function?)...
   //for example with figures and paths
   if (!g->user_space)
     efl_gfx_path_bounds_get(vg, &r);
   else
     {
        r.w = vg_data->view_box.w;
        r.h = vg_data->view_box.h;
     }

   if (g->type == SVG_LINEAR_GRADIENT)
     {
        grad_obj = evas_vg_gradient_linear_add(parent);
        evas_vg_gradient_linear_start_set(grad_obj, g->linear->x1 * r.w + r.x, g->linear->y1 * r.h + r.y);
        evas_vg_gradient_linear_end_set(grad_obj, g->linear->x2 * r.w + r.x, g->linear->y2 * r.h + r.y);
     }
   else if (g->type == SVG_RADIAL_GRADIENT)
     {
        radius = sqrt(pow(r.w, 2) + pow(r.h, 2)) / sqrt(2.0);
        if (!g->user_space)
          {
             /**
              * That is according to Units in here
              *
              * https://www.w3.org/TR/2015/WD-SVG2-20150915/coords.html
              */
             int min = (r.h > r.w) ? r.w : r.h;
             radius = sqrt(pow(min, 2) + pow(min, 2)) / sqrt(2.0);
          }
        grad_obj = evas_vg_gradient_radial_add(parent);
        evas_vg_gradient_radial_center_set(grad_obj, g->radial->cx * r.w + r.x, g->radial->cy * r.h + r.y);
        evas_vg_gradient_radial_radius_set(grad_obj, g->radial->r * radius);
        evas_vg_gradient_radial_focal_set(grad_obj, g->radial->fx * r.w + r.x, g->radial->fy * r.h + r.y);

        /* in case of objectBoundingBox it need proper scaling */
        if (!g->user_space)
          {
             double scale_X = 1.0, scale_reversed_X = 1.0;
             double scale_Y = 1.0, scale_reversed_Y = 1.0;

             /* check the smallest size, find the scale value */
             if (r.h > r.w)
               {
                  scale_Y = ((double) r.w) / r.h;
                  scale_reversed_Y = ((double) r.h) / r.w;
               }
             else
               {
                  scale_X = ((double) r.h) / r.w;
                  scale_reversed_X = ((double) r.w) / r.h;
               }

             efl_gfx_path_bounds_get(grad_obj, &grad_geom);

             double cy = grad_geom.h / 2 + grad_geom.y;
             double cy_scaled = (grad_geom.h / 2) * scale_reversed_Y;
             double cx = grad_geom.w / 2 + grad_geom.x;
             double cx_scaled = (grad_geom.w / 2) * scale_reversed_X;

             /* matrix tranformation of gradient figure:
              * 0. we remember size of gradient and it's center point
              * 1. move all gradients to point {0;0}
              *    (so scale wont increase starting point)
              * 2. scale properly only according to the bigger size of entity
              * 3. move back so new center point would stay on position
              *    it had previously
              */
             eina_matrix3_identity(&m);
             eina_matrix3_translate(&m, grad_geom.x, grad_geom.y);
             eina_matrix3_scale(&m, scale_X, scale_Y);
             eina_matrix3_translate(&m, cx_scaled - cx, cy_scaled - cy);

             efl_canvas_vg_node_transformation_set(grad_obj, &m);
          }
     }
   else
     {
        // not a known gradient
        return NULL;
     }
   // apply common prperty
   evas_vg_gradient_spread_set(grad_obj, g->spread);
   // update the stops
   stop_count = eina_list_count(g->stops);
   if (stop_count)
     {
        stops = calloc(stop_count, sizeof(Efl_Gfx_Gradient_Stop));
        i = 0;
        EINA_LIST_FOREACH(g->stops, l, stop)
          {
             stops[i].r = stop->r;
             stops[i].g = stop->g;
             stops[i].b = stop->b;
             stops[i].a = stop->a;
             stops[i].offset = stop->offset;
             i++;
          }
        evas_vg_gradient_stop_set(grad_obj, stops, stop_count);
        free(stops);
     }
   return grad_obj;
}

// vg tree creation
static void
_apply_vg_property(Svg_Node *node, Efl_VG *vg, Efl_VG *parent, Vg_File_Data *vg_data)
{
   Svg_Style_Property *style = node->style;

   // update the vg name
   if (node->id)
     efl_name_set(vg, node->id);

   // apply the transformation
   if (node->transform)
     evas_vg_node_transformation_set(vg, node->transform);

   if ((node->type == SVG_NODE_G) || (node->type == SVG_NODE_DOC)) return;

   // apply the fill style property
   efl_gfx_shape_fill_rule_set(vg, style->fill.fill_rule);
   // if fill property is NULL then do nothing
   if (style->fill.paint.none)
     {
        //do nothing
     }
   else if (style->fill.paint.gradient)
     {
        // if the fill has gradient then apply.
        evas_vg_shape_fill_set(vg, _apply_gradient_property(style->fill.paint.gradient, vg, parent, vg_data));
     }
   else if (style->fill.paint.cur_color)
     {
        // apply the current style color
        evas_vg_node_color_set(vg, style->r, style->g,
                               style->b, style->fill.opacity);
     }
   else
     {
        // apply the fill color
        evas_vg_node_color_set(vg, style->fill.paint.r, style->fill.paint.g,
                               style->fill.paint.b, style->fill.opacity);
     }

   evas_vg_shape_stroke_width_set(vg, style->stroke.width);
   evas_vg_shape_stroke_cap_set(vg, style->stroke.cap);
   evas_vg_shape_stroke_join_set(vg, style->stroke.join);
   evas_vg_shape_stroke_scale_set(vg, style->stroke.scale);
   // if stroke property is NULL then do nothing
   if (style->stroke.paint.none)
     {
        //do nothing
     }
   else if (style->stroke.paint.gradient)
     {
        // if the fill has gradient then apply.
        evas_vg_shape_stroke_fill_set(vg, _apply_gradient_property(style->stroke.paint.gradient, vg, parent, vg_data));
     }
   else if (style->stroke.paint.url)
     {
        // apply the color pointed by url
        // TODO
     }
   else if (style->stroke.paint.cur_color)
     {
        // apply the current style color
        evas_vg_shape_stroke_color_set(vg, style->r, style->g,
                                       style->b, style->stroke.opacity);
     }
   else
     {
        // apply the stroke color
        evas_vg_shape_stroke_color_set(vg, style->stroke.paint.r, style->stroke.paint.g,
                                       style->stroke.paint.b, style->stroke.opacity);
     }
}

static void
_add_polyline(Efl_VG *vg, double *array, int size, Eina_Bool polygon)
{
   int i;

   if (size < 2) return;

   evas_vg_shape_append_move_to(vg, array[0], array[1]);
   for (i=2; i < size; i+=2)
     evas_vg_shape_append_line_to(vg, array[i], array[i+1]);

   if (polygon)
     evas_vg_shape_append_close(vg);
}

static Efl_VG *
vg_common_create_vg_node_helper(Svg_Node *node, Efl_VG *parent, Vg_File_Data *vg_data)
{
   Efl_VG *vg = NULL;
   Svg_Node *child;
   Eina_List *l;

   switch (node->type)
     {
        case SVG_NODE_DOC:
        case SVG_NODE_G:
           {
              if (!parent)
                vg = efl_add_ref(EFL_CANVAS_VG_CONTAINER_CLASS, NULL);
              else
                vg = efl_add(EFL_CANVAS_VG_CONTAINER_CLASS, parent);
              _apply_vg_property(node, vg, parent, vg_data);
              EINA_LIST_FOREACH(node->child, l, child)
                {
                   vg_common_create_vg_node_helper(child, vg, vg_data);
                }
              return vg;
           }
           break;
        case SVG_NODE_PATH:
           vg = evas_vg_shape_add(parent);
           evas_vg_shape_append_svg_path(vg, node->node.path.path);
           break;
        case SVG_NODE_POLYGON:
           vg = evas_vg_shape_add(parent);
           _add_polyline(vg, node->node.polygon.points, node->node.polygon.points_count, EINA_TRUE);
           break;
        case SVG_NODE_POLYLINE:
           vg = evas_vg_shape_add(parent);
           _add_polyline(vg, node->node.polygon.points, node->node.polygon.points_count, EINA_FALSE);
           break;
        case SVG_NODE_ELLIPSE:
           vg = evas_vg_shape_add(parent);
           evas_vg_shape_append_arc(vg, node->node.ellipse.cx - node->node.ellipse.rx,
                                          node->node.ellipse.cy - node->node.ellipse.ry,
                                          2*node->node.ellipse.rx, 2*node->node.ellipse.ry, 0, 360);
           evas_vg_shape_append_close(vg);
           break;
        case SVG_NODE_CIRCLE:
           vg = evas_vg_shape_add(parent);
           evas_vg_shape_append_circle(vg, node->node.circle.cx, node->node.circle.cy, node->node.circle.r);
           break;
        case SVG_NODE_RECT:
           vg = evas_vg_shape_add(parent);
           evas_vg_shape_append_rect(vg, node->node.rect.x, node->node.rect.y, node->node.rect.w, node->node.rect.h,
                                           node->node.rect.rx, node->node.rect.ry);
           break;
        case SVG_NODE_LINE:
           vg = evas_vg_shape_add(parent);
           evas_vg_shape_append_move_to(vg, node->node.line.x1, node->node.line.y1);
           evas_vg_shape_append_line_to(vg, node->node.line.x2, node->node.line.y2);
           break;
        case SVG_NODE_CUSTOME_COMMAND:
           vg = evas_vg_shape_add(parent);
           evas_vg_shape_path_set(vg, node->node.command.commands, node->node.command.points);
           break;
       default:
           break;
     }
   if (vg)
     _apply_vg_property(node, vg, parent, vg_data);
   return vg;
}

Vg_File_Data *
vg_common_create_vg_node(Svg_Node *node)
{
   Vg_File_Data *vg_data;

   if (!node || (node->type != SVG_NODE_DOC)) return NULL;

   vg_data = calloc(1, sizeof(Vg_File_Data));
   vg_data->view_box.x = node->node.doc.vx;
   vg_data->view_box.y = node->node.doc.vy;
   vg_data->view_box.w = node->node.doc.vw;
   vg_data->view_box.h = node->node.doc.vh;
   vg_data->preserve_aspect = node->node.doc.preserve_aspect;
   vg_data->root = vg_common_create_vg_node_helper(node, NULL, vg_data);

   return vg_data;
}

static Svg_Node *
_create_node(Svg_Node *parent, Svg_Node_Type type)
{
   Svg_Node *node = calloc(1, sizeof(Svg_Node));

   // default fill property
   node->style = calloc(1, sizeof(Svg_Style_Property));

   // update the default value of stroke and fill
   //https://www.w3.org/TR/SVGTiny12/painting.html#SpecifyingPaint
   // default fill color is black
   node->style->fill.paint.r = 0;
   node->style->fill.paint.g = 0;
   node->style->fill.paint.b = 0;
   node->style->fill.paint.none = EINA_FALSE;
   // default fill opacity is 1
   node->style->fill.opacity = 255;

   // default fill rule is nonzero
   node->style->fill.fill_rule = EFL_GFX_FILL_RULE_WINDING;

   // default stroke is none
   node->style->stroke.paint.none = EINA_TRUE;
   // default stroke opacity is 1
   node->style->stroke.opacity = 255;
   // default stroke width is 1
   node->style->stroke.width = 1;
   // default line cap is butt
   node->style->stroke.cap = EFL_GFX_CAP_BUTT;
   // default line join is miter
   node->style->stroke.join = EFL_GFX_JOIN_MITER;
   node->style->stroke.scale = 1.0;

   node->parent = parent;
   node->type = type;
   node->child = NULL;

   if (parent)
     parent->child = eina_list_append(parent->child, node);
   return node;
}

static Svg_Style_Gradient*
_create_gradient_node(Efl_VG *vg)
{
   const Efl_Gfx_Gradient_Stop *stops = NULL;
   Efl_Gfx_Gradient_Stop *new_stop;
   unsigned int count = 0, i;

   Svg_Style_Gradient *grad = calloc(1, sizeof(Svg_Style_Gradient));

   grad->spread = evas_vg_gradient_spread_get(vg);
   evas_vg_gradient_stop_get(vg, &stops, &count);
   for (i = 0; i < count; i++)
     {
        new_stop = calloc(1, sizeof(Efl_Gfx_Gradient_Stop));
        memcpy(new_stop, stops, sizeof(Efl_Gfx_Gradient_Stop));
        grad->stops = eina_list_append(grad->stops, new_stop);
        stops++;
     }
   if (efl_isa(vg, EFL_CANVAS_VG_GRADIENT_LINEAR_CLASS))
     {
        grad->type = SVG_LINEAR_GRADIENT;
        grad->linear = calloc(1, sizeof(Svg_Linear_Gradient));
        evas_vg_gradient_linear_start_get(vg, &grad->linear->x1, &grad->linear->y1);
        evas_vg_gradient_linear_end_get(vg, &grad->linear->x2, &grad->linear->y2);
     }
   else
     {
        grad->type = SVG_RADIAL_GRADIENT;
        grad->radial = calloc(1, sizeof(Svg_Radial_Gradient));
        evas_vg_gradient_radial_center_get(vg, &grad->radial->cx, &grad->radial->cy);
        evas_vg_gradient_radial_focal_get(vg, &grad->radial->fx, &grad->radial->fy);
        grad->radial->r = evas_vg_gradient_radial_radius_get(vg);
     }

   return grad;
}

static void
_apply_svg_property(Svg_Node *node, Efl_VG *vg)
{
   const Eina_Matrix3 *matrix;
   const char *id;
   Svg_Style_Property *style = node->style;

   // transformation
   if ((matrix = evas_vg_node_transformation_get(vg)))
     {
        node->transform = calloc(1, sizeof(Eina_Matrix3));
        eina_matrix3_copy(node->transform, matrix);
     }

   if ((id = efl_name_get(vg)))
     {
        node->id = eina_stringshare_add(id);
     }

   if (node->type == SVG_NODE_G) return;


   // apply the fill style property
   style->fill.fill_rule = efl_gfx_shape_fill_rule_get(vg);
   style->fill.paint.none = EINA_FALSE;
   if (evas_vg_shape_fill_get(vg))
     {
        // if the fill has gradient then apply.
        style->fill.paint.gradient = _create_gradient_node(evas_vg_shape_fill_get(vg));
     }
   else
     {
        evas_vg_node_color_get(vg, &style->fill.paint.r, &style->fill.paint.g,
                               &style->fill.paint.b, &style->fill.opacity);
     }

   // apply stroke style property
   style->stroke.paint.none = EINA_FALSE;

   if (evas_vg_shape_stroke_fill_get(vg))
     {
        // if the stroke has gradient then apply.
        style->stroke.paint.gradient = _create_gradient_node(evas_vg_shape_stroke_fill_get(vg));
     }
   else
     {
        // apply the stroke color
        evas_vg_shape_stroke_color_get(vg, &style->stroke.paint.r, &style->stroke.paint.g,
                                       &style->stroke.paint.b, &style->stroke.opacity);
     }

   style->stroke.width = (evas_vg_shape_stroke_width_get(vg));
   style->stroke.cap = evas_vg_shape_stroke_cap_get(vg);
   style->stroke.join = evas_vg_shape_stroke_join_get(vg);
   style->stroke.scale = evas_vg_shape_stroke_scale_get(vg);


}

static void
vg_common_create_svg_node_helper(Efl_VG *vg, Svg_Node *parent)
{
   Eina_Iterator *it;
   Efl_VG *child;
   Svg_Node *svg_node;
   const Efl_Gfx_Path_Command *commands;
   unsigned int points_count, commands_count;
   const double *points;

   if (efl_isa(vg, EFL_CANVAS_VG_CONTAINER_CLASS))
     {
        svg_node = _create_node(parent, SVG_NODE_G);
        _apply_svg_property(svg_node, vg);
        // apply property
        it = efl_canvas_vg_container_children_get(vg);
        EINA_ITERATOR_FOREACH(it, child)
          {
             vg_common_create_svg_node_helper(child, svg_node);
          }
     }
   else if (efl_isa(vg, EFL_CANVAS_VG_SHAPE_CLASS))
     {
        svg_node = _create_node(parent, SVG_NODE_CUSTOME_COMMAND);
        evas_vg_shape_path_get(vg, &commands, &points);
        evas_vg_shape_path_length_get(vg, &commands_count, &points_count);
        svg_node->node.command.commands_count = commands_count;
        svg_node->node.command.points_count = points_count;
        svg_node->node.command.points = calloc(points_count, sizeof(double));
        svg_node->node.command.commands = calloc(commands_count, sizeof(Efl_Gfx_Path_Command));
        memcpy(svg_node->node.command.commands, commands, sizeof (Efl_Gfx_Path_Command) * commands_count);
        memcpy(svg_node->node.command.points, points, sizeof (double) * points_count);
        _apply_svg_property(svg_node, vg);
     }
}

Svg_Node *
vg_common_create_svg_node(Vg_File_Data *node)
{
   Svg_Node *doc;

   if (!node || !node->root) return NULL;

   doc = _create_node(NULL, SVG_NODE_DOC);

   doc->node.doc.vx = node->view_box.x;
   doc->node.doc.vy = node->view_box.y;
   doc->node.doc.vw = node->view_box.w;
   doc->node.doc.vh = node->view_box.h;
   doc->node.doc.preserve_aspect = node->preserve_aspect;
   vg_common_create_svg_node_helper(node->root, doc);
   return doc;
}
