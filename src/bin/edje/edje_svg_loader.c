#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <edje_private.h>
#include <ctype.h>

#include <Eet.h>
#include <Eina.h>
#include <Evas.h>
#include <eina_matrix.h>

#ifndef A_VAL
#ifndef WORDS_BIGENDIAN
/* x86 */
#define A_VAL(p) (((uint8_t *)(p))[3])
#define R_VAL(p) (((uint8_t *)(p))[2])
#define G_VAL(p) (((uint8_t *)(p))[1])
#define B_VAL(p) (((uint8_t *)(p))[0])
#define AR_VAL(p) ((uint16_t *)(p)[1])
#define GB_VAL(p) ((uint16_t *)(p)[0])
#else
/* ppc */
#define A_VAL(p) (((uint8_t *)(p))[0])
#define R_VAL(p) (((uint8_t *)(p))[1])
#define G_VAL(p) (((uint8_t *)(p))[2])
#define B_VAL(p) (((uint8_t *)(p))[3])
#define AR_VAL(p) ((uint16_t *)(p)[0])
#define GB_VAL(p) ((uint16_t *)(p)[1])
#endif
#endif

#define ARGB_JOIN(a,r,g,b) \
        (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))


typedef Svg_Node *(*Factory_Method)(Svg_Node *parent, const char *buf, unsigned buflen);

typedef Svg_Style_Gradient *(*Gradient_Factory_Method)(const char *buf, unsigned buflen);

typedef struct _Evas_SVG_Loader Evas_SVG_Loader;
struct _Evas_SVG_Loader
{
   Eina_Array *stack;
   Svg_Node *doc;
   Svg_Node *def;
   Svg_Style_Gradient *gradient;
   int level;
   Eina_Bool result:1;
};

char *
_skip_space(const char *str, const char *end)
{
   while (((end != NULL && str < end) || (end == NULL && *str != '\0')) && isspace(*str))
     ++str;
   return (char *)str;
}

static inline char *
_copy_id(const char* str)
{
   return strdup(str);
}

static const char *
_skipcomma(const char *content)
{
   content = _skip_space(content, NULL);
   if (*content == ',') return content + 1;
   return content;
}

static inline Eina_Bool
_parse_number(const char **content, double *number)
{
   char *end = NULL;

   *number = strtod(*content, &end);
   // if the start of string is not number
   if ((*content) == end) return EINA_FALSE;
   //skip comma if any
   *content = _skipcomma(end);
   return EINA_TRUE;
}

static inline double
_to_double(const char *str)
{
   char *end = NULL;

   return strtod(str, &end);
}

static inline int
_to_opacity(const char *str)
{
   char *end = NULL;
   int a = 0;
   double opacity = strtod(str, &end);

   if (*end == '\0')
     a = lrint(opacity * 255);
   return a;
}

#define _PARSE_TAG(Type, Short_Name, Tags_Array, Default)               \
  static Type _to_##Short_Name(const char *str)                         \
  {                                                                     \
     unsigned int i;                                                    \
                                                                        \
     for (i = 0; i < sizeof (Tags_Array) / sizeof (Tags_Array[0]); i++) \
       if (!strcmp(str, Tags_Array[i].tag))                             \
         return Tags_Array[i].Short_Name;                               \
     return Default;                                                    \
  }
/* parse the line cap used during stroking a path.
 * Value:    butt | round | square | inherit
 * Initial:    butt
 * https://www.w3.org/TR/SVG/painting.html
 */
static struct {
   Efl_Gfx_Cap line_cap;
   const char *tag;
} line_cap_tags[] = {
  { EFL_GFX_CAP_BUTT, "butt" },
  { EFL_GFX_CAP_ROUND, "round" },
  { EFL_GFX_CAP_SQUARE, "square" }
};

_PARSE_TAG(Efl_Gfx_Cap, line_cap, line_cap_tags, EFL_GFX_CAP_LAST);

/* parse the line join used during stroking a path.
 * Value:   miter | round | bevel | inherit
 * Initial:    miter
 * https://www.w3.org/TR/SVG/painting.html
 */
static struct {
   Efl_Gfx_Join line_join;
   const char *tag;
} line_join_tags[] = {
  { EFL_GFX_JOIN_MITER, "miter" },
  { EFL_GFX_JOIN_ROUND, "round" },
  { EFL_GFX_JOIN_BEVEL, "bevel" }
};

_PARSE_TAG(Efl_Gfx_Join, line_join, line_join_tags, EFL_GFX_JOIN_LAST);

/* parse the fill rule used during filling a path.
 * Value:   nonzero | evenodd | inherit
 * Initial:    nonzero
 * https://www.w3.org/TR/SVG/painting.html
 */

static struct {
   Efl_Gfx_Fill_Rule fill_rule;
   const char *tag;
} fill_rule_tags[] = {
  { EFL_GFX_FILL_RULE_ODD_EVEN, "evenodd" }
};

_PARSE_TAG(Efl_Gfx_Fill_Rule, fill_rule, fill_rule_tags, EFL_GFX_FILL_RULE_WINDING);

#if 0
// unused at the moment
/* parse the dash pattern used during stroking a path.
 * Value:   none | <dasharray> | inherit
 * Initial:    none
 * https://www.w3.org/TR/SVG/painting.html
 */
static inline void
_parse_dash_array(const char *str, Efl_Gfx_Dash** dash, int *length)
{
   double tmp[30];
   char *end = NULL;
   int leni, gapi, count = 0, index = 0;

   while (*str)
     {
        // skip white space, comma
        str = _skipcomma(str);
        tmp[count++] = strtod(str, &end);
        str = _skipcomma(end);
     }

   if (count & 0x1)
     { // odd case.
        *length = count;
        *dash = calloc(*length, sizeof(Efl_Gfx_Dash));
        while (index < count)
          {
             leni = (2 * index) % count;
             gapi = (2 * index + 1) % count;
             (*dash)[index].length = tmp[leni];
             (*dash)[index].gap = tmp[gapi];
          }
     }
   else
     { // even case
        *length = count/2;
        *dash = calloc(*length, sizeof(Efl_Gfx_Dash));
        while (index < count)
          {
             (*dash)[index].length = tmp[2 * index];
             (*dash)[index].gap = tmp[2 * index + 1];
          }
     }
}
#endif

static char *
 _id_from_url(const char *url)
{
   char tmp[50];
   int i = 0;

   url = _skip_space(url, NULL);
   if ((*url) == '(')
     ++url;
   url = _skip_space(url, NULL);
   if ((*url) == '#')
     ++url;

   while ((*url) != ')')
     {
        tmp[i++] = *url;
        ++url;
     }
   tmp[i] = '\0';

   return strdup(tmp);
}

static unsigned char
_color_parser(const char *value, char **end)
{
   double r;

   r = strtod(value + 4, end);
   *end = _skip_space(*end, NULL);
   if (**end == '%')
     r = 255 * r / 100;
   *end = _skip_space(*end, NULL);

   if (r < 0 || r > 255)
     {
        *end = NULL;
        return 0;
     }

   return lrint(r);
}

static inline void
_to_color(const char *str, int *r, int *g, int *b, char** ref)
{
   unsigned int len = strlen(str);
   char *red, *green, *blue;
   unsigned char tr, tg, tb;

   if (len == 4 && str[0] == '#')
     {
        // case for "#456" should be interprete as "#445566"
        if (isxdigit(str[1]) &&
            isxdigit(str[2]) &&
            isxdigit(str[3]))
          {
             char tmp[3] = { '\0', '\0', '\0' };
             tmp[0] = str[1]; tmp[1] = str[1]; *r = strtol(tmp, NULL, 16);
             tmp[0] = str[2]; tmp[1] = str[2]; *g = strtol(tmp, NULL, 16);
             tmp[0] = str[3]; tmp[1] = str[3]; *b = strtol(tmp, NULL, 16);
          }
     }
   else if (len == 7 && str[0] == '#')
     {
        if (isxdigit(str[1]) &&
            isxdigit(str[2]) &&
            isxdigit(str[3]) &&
            isxdigit(str[4]) &&
            isxdigit(str[5]) &&
            isxdigit(str[6]))
          {
             char tmp[3] = { '\0', '\0', '\0' };
             tmp[0] = str[1]; tmp[1] = str[2]; *r = strtol(tmp, NULL, 16);
             tmp[0] = str[3]; tmp[1] = str[4]; *g = strtol(tmp, NULL, 16);
             tmp[0] = str[5]; tmp[1] = str[6]; *b = strtol(tmp, NULL, 16);
          }
     }
   else if (len >= 10 &&
            (str[0] == 'r' || str[0] == 'R') &&
            (str[1] == 'g' || str[1] == 'G') &&
            (str[2] == 'b' || str[2] == 'B') &&
            str[3] == '(' &&
            str[len - 1] == ')')
     {
        tr = _color_parser(str + 4, &red);
        if (red && *red == ',')
          {
             tg = _color_parser(red + 1, &green);
             if (green && *green == ',')
               {
                  tb = _color_parser(green + 1, &blue);
                  if (blue && blue[0] == ')' && blue[1] == '\0')
                    {
                       *r = tr; *g = tg; *b = tb;
                    }
               }
          }
     }
   else if (len >= 3 && !strncmp(str, "url",3))
     {
        *ref = _id_from_url(str+3);
     }
   else
     {
      // TODO handle named color
     }
}

static inline char *
parse_numbers_array(char *str, double *points, int *pt_count)
{
   int count = 0;
   char *end = NULL;

   str = _skip_space(str, NULL);
   while (isdigit(*str) ||
          *str == '-' ||
          *str == '+' ||
          *str == '.')
     {
        points[count++] = strtod(str, &end);
        str = end;
        str = _skip_space(str, NULL);
        if (*str == ',')
          ++str;
        //eat the rest of space
        str = _skip_space(str, NULL);
     }
   *pt_count = count;
   return str;
}

typedef enum _Matrix_State
{
  SVG_MATRIX_UNKNOWN,
  SVG_MATRIX_MATRIX,
  SVG_MATRIX_TRANSLATE,
  SVG_MATRIX_ROTATE,
  SVG_MATRIX_SCALE,
  SVG_MATRIX_SKEWX,
  SVG_MATRIX_SKEWY
} Matrix_State;

#define MATRIX_DEF(Name, Value)                 \
  { #Name, sizeof (#Name), Value}

static const struct {
   const char *tag;
   int sz;
   Matrix_State state;
} matrix_tags[] = {
  MATRIX_DEF(matrix, SVG_MATRIX_MATRIX),
  MATRIX_DEF(translate, SVG_MATRIX_TRANSLATE),
  MATRIX_DEF(rotate, SVG_MATRIX_ROTATE),
  MATRIX_DEF(scale, SVG_MATRIX_SCALE),
  MATRIX_DEF(skewX, SVG_MATRIX_SKEWX),
  MATRIX_DEF(skewY, SVG_MATRIX_SKEWY)
};

/* parse transform attribute
 * https://www.w3.org/TR/SVG/coords.html#TransformAttribute
 */
static Eina_Matrix3 *
_parse_transformation_matrix(const char *value)
{
   unsigned int i;
   double points[8];
   int sz, pt_count = 0;
   double sx, sy;
   Matrix_State state = SVG_MATRIX_UNKNOWN;
   Eina_Matrix3 *matrix = calloc(1, sizeof(Eina_Matrix3));
   char *str = (char *)value;
   char *end = str + strlen(str);

   eina_matrix3_identity(matrix);
   while (str < end)
     {
        if (isspace(*str) || (*str == ','))
          {
             ++str;
             continue;
          }
        sz = end - str;
        for (i = 0; i < sizeof (matrix_tags) / sizeof(matrix_tags[0]); i++)
          if (matrix_tags[i].sz - 1 == sz && !strncmp(matrix_tags[i].tag, str, sz))
            {
               state = matrix_tags[i].state;
               str += (matrix_tags[i].sz -1);
            }
        if ( state == SVG_MATRIX_UNKNOWN)
          goto error;

        str = _skip_space(str, end);
        if (*str != '(')
          goto error;
        ++str;
        str = parse_numbers_array(str, points, &pt_count);
        if (*str != ')')
          goto error;
        ++str;

        if (state == SVG_MATRIX_MATRIX)
          {
             Eina_Matrix3 tmp;

             if (pt_count != 6) goto error;

             eina_matrix3_identity(&tmp);
             eina_matrix3_values_set(&tmp,
                                     points[0], points[2], points[4],
                                     points[1], points[3], points[5],
                                     0, 0, 1);
             eina_matrix3_compose(matrix, &tmp, matrix);
          }
        else if (state == SVG_MATRIX_TRANSLATE)
          {
             if (pt_count == 1)
               eina_matrix3_translate(matrix, points[0], 0);
             else if (pt_count == 2)
               eina_matrix3_translate(matrix, points[0], points[1]);
             else
               goto error;
          }
        else if (state == SVG_MATRIX_ROTATE)
          {
             if (pt_count == 1)
               {
                  eina_matrix3_rotate(matrix, points[0]);
               }
             else if (pt_count == 3)
               {
                  eina_matrix3_translate(matrix, points[1], points[2]);
                  eina_matrix3_rotate(matrix, points[0]);
                  eina_matrix3_translate(matrix, -points[1], -points[2]);
               }
             else
               {
                  goto error;
               }
          }
        else if (state == SVG_MATRIX_SCALE)
          {
             if (pt_count < 1 || pt_count > 2) goto error;

             sx = points[0];
             sy = sx;
             if (pt_count == 2)
               sy = points[1];
             eina_matrix3_scale(matrix, sx, sy);
          }
     }
 error:
   return matrix;
}

#define LENGTH_DEF(Name, Value)                 \
  { #Name, sizeof (#Name), Value}

static const struct {
   const char *tag;
   int sz;
   Svg_Length_Type type;
} length_tags[] = {
  LENGTH_DEF(%, SVG_LT_PERCENT),
  LENGTH_DEF(px, SVG_LT_PX),
  LENGTH_DEF(pc, SVG_LT_PC),
  LENGTH_DEF(pt, SVG_LT_PT),
  LENGTH_DEF(mm, SVG_LT_MM),
  LENGTH_DEF(cm, SVG_LT_CM),
  LENGTH_DEF(in, SVG_LT_IN)
};

static double
parse_length(const char *str, Svg_Length_Type *type)
{
   unsigned int i;
   double value;
   int sz = strlen(str);

   *type = SVG_LT_PX;
   for (i = 0; i < sizeof (length_tags) / sizeof(length_tags[0]); i++)
     if (length_tags[i].sz - 1 == sz && !strncmp(length_tags[i].tag, str, sz))
       {
          *type = length_tags[i].type;
       }
   value = strtod(str, NULL);
   return value;
}

static Eina_Bool
_attr_parse_svg_node(void *data, const char *key, const char *value)
{
   Svg_Doc_Node *node = data;
   Svg_Length_Type type;

   // @TODO handle lenght unit.
   if (!strcmp(key, "width"))
     {
        node->width = parse_length(value, &type);
     }
   else if (!strcmp(key, "height"))
     {
        node->height = parse_length(value, &type);
     }
   else if (!strcmp(key, "viewBox"))
     {
        if (_parse_number(&value, &node->vx))
          if (_parse_number(&value, &node->vy))
            if (_parse_number(&value, &node->vw))
              _parse_number(&value, &node->vh);
     }
   return EINA_TRUE;
}

static void
_handle_color_attr(Svg_Node* node, const char *value)
{
   Svg_Style_Property *style = node->style;
   char *use = NULL;

   _to_color(value, &style->fill.r, &style->fill.g, &style->fill.b, &use);
   if (use)
     {
        // update the gradient field
     }
}

static void
_handle_fill_attr(Svg_Node* node, const char *value)
{
   Svg_Style_Property *style = node->style;
   char *use = NULL;

   _to_color(value, &style->fill.r, &style->fill.g, &style->fill.b, &use);
   if (use)
     {
        // update the gradient field
     }
}

static void
_handle_stroke_attr(Svg_Node* node, const char *value)
{
   Svg_Style_Property *style = node->style;
   char *use = NULL;

   _to_color(value, &style->stroke.r, &style->stroke.g, &style->stroke.b, &use);
   if (use)
     {
        // update the gradient field
     }
}

static void
_handle_stroke_opacity_attr(Svg_Node* node, const char *value)
{
   node->style->stroke.a = _to_opacity(value);
}

static void
_handle_stroke_width_attr(Svg_Node* node, const char *value)
{
   node->style->stroke.width = _to_double(value);
}

static void
_handle_stroke_linecap_attr(Svg_Node* node, const char *value)
{
   node->style->stroke.cap = _to_line_cap(value);
}

static void
_handle_stroke_linejoin_attr(Svg_Node* node, const char *value)
{
   node->style->stroke.join = _to_line_join(value);
}

static void
_handle_color_opacity_attr(Svg_Node* node, const char *value)
{
   node->style->fill.a = _to_opacity(value);
}

static void
_handle_fill_rule_attr(Svg_Node* node, const char *value)
{
   node->style->fill.fill_rule = _to_fill_rule(value);
}

static void
_handle_fill_opacity_attr(Svg_Node* node, const char *value)
{
   node->style->fill.a = _to_opacity(value);
}

static void
_handle_transform_attr(Svg_Node* node, const char *value)
{
   node->transform = _parse_transformation_matrix(value);
}


typedef void (*Style_Method)(Svg_Node *node, const char *value);

#define STYLE_DEF(Name, Name1)       \
  { #Name, sizeof (#Name), _handle_##Name1##_attr}

static const struct {
   const char *tag;
   int sz;
   Style_Method tag_handler;;
} style_tags[] = {
  STYLE_DEF(color, color),
  STYLE_DEF(color-opacity, color_opacity),
  STYLE_DEF(fill, fill),
  STYLE_DEF(fill-rule, fill_rule),
  STYLE_DEF(fill-opacity, fill_opacity),
  STYLE_DEF(stroke, stroke),
  STYLE_DEF(stroke-width, stroke_width),
  STYLE_DEF(stroke-linejoin, stroke_linejoin),
  STYLE_DEF(stroke-linecap, stroke_linecap),
  STYLE_DEF(stroke-opacity, stroke_opacity),
  STYLE_DEF(transform, transform)
};

static Eina_Bool
_parse_style_attr(void *data, const char *key, const char *value)
{
   Svg_Node* node = data;
   unsigned int i;
   int sz;

   // trim the white space
   key = _skip_space(key, NULL);

   value = _skip_space(value, NULL);

   sz = strlen(key);
   for (i = 0; i < sizeof (style_tags) / sizeof(style_tags[0]); i++)
     if (style_tags[i].sz - 1 == sz && !strncmp(style_tags[i].tag, key, sz))
       {
          style_tags[i].tag_handler(node, value);
          return EINA_TRUE;
       }

   return EINA_TRUE;
}

static Eina_Bool
_attr_style_node(void *data, const char *str)
{
   eina_simple_xml_attribute_w3c_parse(str,
                                       _parse_style_attr, data);
   return EINA_TRUE;
}

/* parse g node
 * https://www.w3.org/TR/SVG/struct.html#Groups
 */
static Eina_Bool
_attr_parse_g_node(void *data, const char *key, const char *value)
{
   Svg_Node *node = data;

   if (!strcmp(key, "style"))
     {
        return _attr_style_node(node, value);
     }
   else if (!strcmp(key, "transform"))
     {
        node->transform = _parse_transformation_matrix(value);
     }
   else if (!strcmp(key, "id"))
     {
        node->id = _copy_id(value);
     }
   return EINA_TRUE;
}


static Svg_Node *
_create_node(Svg_Node *parent, Svg_Node_Type type)
{
   Svg_Node *node = calloc(1, sizeof(Svg_Node));

   // default fill property
   node->style = calloc(1, sizeof(Svg_Style_Property));
   node->style->fill.a = 255;
   node->style->fill.fill_rule = EFL_GFX_FILL_RULE_WINDING;

   //default stroke property
   node->style->stroke.cap = EFL_GFX_CAP_BUTT;
   node->style->stroke.join = EFL_GFX_JOIN_MITER;
   node->style->stroke.join = EFL_GFX_JOIN_MITER;
   node->style->stroke.scale = 1.0;

   node->parent = parent;
   node->type = type;
   node->child = NULL;

   if (parent)
     parent->child = eina_list_append(parent->child, node);
   return node;
}

static Svg_Node *
_create_defs_node(Svg_Node *parent EINA_UNUSED, const char *buf EINA_UNUSED, unsigned buflen EINA_UNUSED)
{
   Svg_Node *node = _create_node(NULL, SVG_NODE_DEFS);

   return node;
}

static Svg_Node *
_create_g_node(Svg_Node *parent, const char *buf, unsigned buflen)
{
   Svg_Node *node = _create_node(parent, SVG_NODE_G);

   eina_simple_xml_attributes_parse(buf, buflen,
                                    _attr_parse_g_node, node);
   return node;
}

static Svg_Node *
_create_svg_node(Svg_Node *parent, const char *buf, unsigned buflen)
{
   Svg_Node *node = _create_node(parent, SVG_NODE_DOC);

   eina_simple_xml_attributes_parse(buf, buflen,
                                    _attr_parse_svg_node, &node->node.doc);
   return node;
}

static Svg_Node *
_create_switch_node(Svg_Node *parent EINA_UNUSED, const char *buf EINA_UNUSED, unsigned buflen EINA_UNUSED)
{
   return NULL;
}

static Eina_Bool
_attr_parse_path_node(void *data, const char *key, const char *value)
{
   Svg_Node *node = data;
   Svg_Path_Node *path = &(node->node.path);

   if (!strcmp(key, "d"))
     {
        path->path = malloc(strlen(value));
        strcpy(path->path, value);
     }
   else if (!strcmp(key, "style"))
     {
        _attr_style_node(node, value);
     }
   else if (!strcmp(key, "id"))
     {
        node->id = _copy_id(value);
     }
   else
     {
        _parse_style_attr(node, key, value);
     }
   return EINA_TRUE;
}

static Svg_Node *
_create_path_node(Svg_Node *parent, const char *buf, unsigned buflen)
{
   Svg_Node *node = _create_node(parent, SVG_NODE_PATH);

   eina_simple_xml_attributes_parse(buf, buflen,
                                    _attr_parse_path_node, node);
   return node;
}

#define CIRCLE_DEF(Name, Field)       \
  { #Name, sizeof (#Name), offsetof(Svg_Circle_Node, Field)}

static const struct {
   const char *tag;
   int sz;
   size_t offset;
} circle_tags[] = {
  CIRCLE_DEF(cx, cx),
  CIRCLE_DEF(cy, cy),
  CIRCLE_DEF(r, r)
};

/* parse the attributes for a circle element.
 * https://www.w3.org/TR/SVG/shapes.html#CircleElement
 */
static Eina_Bool
_attr_parse_circle_node(void *data, const char *key, const char *value)
{
   Svg_Node *node = data;
   Svg_Circle_Node *circle = &(node->node.circle);
   unsigned int i;
   unsigned char *array;
   int sz = strlen(key);

   array = (unsigned char*) circle;
   for (i = 0; i < sizeof (circle_tags) / sizeof(circle_tags[0]); i++)
     if (circle_tags[i].sz - 1 == sz && !strncmp(circle_tags[i].tag, key, sz))
       {
          *((double*) (array + circle_tags[i].offset)) = _to_double(value);
          return EINA_TRUE;
       }

   if (!strcmp(key, "style"))
     {
        _attr_style_node(node, value);
     }
   else if (!strcmp(key, "id"))
     {
        node->id = _copy_id(value);
     }
   else
     {
        _parse_style_attr(node, key, value);
     }
   return EINA_TRUE;
}

static Svg_Node *
_create_circle_node(Svg_Node *parent, const char *buf, unsigned buflen)
{
   Svg_Node *node = _create_node(parent, SVG_NODE_CIRCLE);

   eina_simple_xml_attributes_parse(buf, buflen,
                                    _attr_parse_circle_node, node);
   return node;
}

#define ELLIPSE_DEF(Name, Field)       \
  { #Name, sizeof (#Name), offsetof(Svg_Ellipse_Node, Field)}

static const struct {
   const char *tag;
   int sz;
   size_t offset;
} ellipse_tags[] = {
  ELLIPSE_DEF(cx,cx),
  ELLIPSE_DEF(cy,cy),
  ELLIPSE_DEF(rx,rx),
  ELLIPSE_DEF(ry,ry)
};

/* parse the attributes for an ellipse element.
 * https://www.w3.org/TR/SVG/shapes.html#EllipseElement
 */
static Eina_Bool
_attr_parse_ellipse_node(void *data, const char *key, const char *value)
{
   Svg_Node *node = data;
   Svg_Ellipse_Node *ellipse = &(node->node.ellipse);
   unsigned int i;
   unsigned char *array;
   int sz = strlen(key);

   array = (unsigned char*) ellipse;
   for (i = 0; i < sizeof (ellipse_tags) / sizeof(ellipse_tags[0]); i++)
     if (ellipse_tags[i].sz - 1 == sz && !strncmp(ellipse_tags[i].tag, key, sz))
       {
          *((double*) (array + ellipse_tags[i].offset)) = _to_double(value);
          return EINA_TRUE;
       }

   if (!strcmp(key, "id"))
     {
        node->id = _copy_id(value);
     }
   else if (!strcmp(key, "style"))
     {
        _attr_style_node(node, value);
     }
   else
     {
        _parse_style_attr(node, key, value);
     }
   return EINA_TRUE;
}

static Svg_Node *
_create_ellipse_node(Svg_Node *parent, const char *buf, unsigned buflen)
{
   Svg_Node *node = _create_node(parent, SVG_NODE_ELLIPSE);

   eina_simple_xml_attributes_parse(buf, buflen,
                                    _attr_parse_ellipse_node, node);
   return node;
}

static void
_attr_parse_polygon_points(const char *str, double **points, int *point_count)
{
   double tmp[50];
   int tmp_count=0;
   int count = 0;
   double num;
   double *point_array = NULL;

   while (_parse_number(&str, &num))
     {
        tmp[tmp_count++] = num;
        if (tmp_count == 50)
          {
             point_array = realloc(point_array, (count + tmp_count) * sizeof(double));
             memcpy(&point_array[count], tmp, tmp_count * sizeof(double));
             count += tmp_count;
          }
     }

   if (tmp_count > 0)
     {
        point_array = realloc(point_array, (count + tmp_count) * sizeof(double));
        memcpy(&point_array[count], tmp, tmp_count * sizeof(double));
        count += tmp_count;
     }
   *point_count = count;
   *points = point_array;
}

/* parse the attributes for a polygon element.
 * https://www.w3.org/TR/SVG/shapes.html#PolylineElement
 */
static Eina_Bool
_attr_parse_polygon_node(void *data, const char *key, const char *value)
{
   Svg_Node *node = data;
   Svg_Polygon_Node *polygon = &(node->node.polygon);

   if (!strcmp(key, "points"))
     {
        _attr_parse_polygon_points(value, &polygon->points, &polygon->points_count);
     }
   else if (!strcmp(key, "style"))
     {
        _attr_style_node(node, value);
     }
   else if (!strcmp(key, "id"))
     {
        node->id = _copy_id(value);
     }
   else
     {
        _parse_style_attr(node, key, value);
     }
   return EINA_TRUE;
}

static Svg_Node *
_create_polygon_node(Svg_Node *parent, const char *buf, unsigned buflen)
{
   Svg_Node *node = _create_node(parent, SVG_NODE_POLYGON);

   eina_simple_xml_attributes_parse(buf, buflen,
                                    _attr_parse_polygon_node, node);
   return node;
}

#define RECT_DEF(Name, Field)       \
  { #Name, sizeof (#Name), offsetof(Svg_Rect_Node, Field)}

static const struct {
   const char *tag;
   int sz;
   size_t offset;
} rect_tags[] = {
  RECT_DEF(x,x),
  RECT_DEF(y, y),
  RECT_DEF(width, w),
  RECT_DEF(height, h),
  RECT_DEF(rx, rx),
  RECT_DEF(ry, ry)
};

/* parse the attributes for a rect element.
 * https://www.w3.org/TR/SVG/shapes.html#RectElement
 */
static Eina_Bool
_attr_parse_rect_node(void *data, const char *key, const char *value)
{
   Svg_Node *node = data;
   Svg_Rect_Node *rect = & (node->node.rect);
   unsigned int i;
   unsigned char *array;
   int sz = strlen(key);

   array = (unsigned char*) rect;
   for (i = 0; i < sizeof (rect_tags) / sizeof(rect_tags[0]); i++)
     if (rect_tags[i].sz - 1 == sz && !strncmp(rect_tags[i].tag, key, sz))
       {
          *((double*) (array + rect_tags[i].offset)) = _to_double(value);
          return EINA_TRUE;
       }

   if (!strcmp(key, "id"))
     {
        node->id = _copy_id(value);
     }
   else if (!strcmp(key, "style"))
     {
        _attr_style_node(node, value);
     }
   else
     {
        _parse_style_attr(node, key, value);
     }

   if (rect->rx != 0 && rect->ry == 0) rect->ry = rect->rx;
   if (rect->ry != 0 && rect->rx == 0) rect->rx = rect->ry;

   return EINA_TRUE;
}

static Svg_Node *
_create_rect_node(Svg_Node *parent, const char *buf, unsigned buflen)
{
   Svg_Node *node = _create_node(parent, SVG_NODE_RECT);

   eina_simple_xml_attributes_parse(buf, buflen,
                                    _attr_parse_rect_node, node);
   return node;
}

#define TAG_DEF(Name)                                   \
  { #Name, sizeof (#Name), _create_##Name##_node }

static const struct {
   const char *tag;
   int sz;
   Factory_Method tag_handler;
} graphics_tags[] = {
  TAG_DEF(circle),
  TAG_DEF(ellipse),
  TAG_DEF(path),
  TAG_DEF(polygon),
  TAG_DEF(rect)
};

static const struct {
   const char *tag;
   int sz;
   Factory_Method tag_handler;
} group_tags[] = {
  TAG_DEF(defs),
  TAG_DEF(g),
  TAG_DEF(svg),
  TAG_DEF(switch)
};

#define FIND_FACTORY(Short_Name, Tags_Array)                            \
  static Factory_Method                                                 \
  _find_##Short_Name##_factory(const char  *name)                       \
  {                                                                     \
     unsigned int i;                                                    \
     int sz = strlen(name);                                             \
                                                                        \
     for (i = 0; i < sizeof (Tags_Array) / sizeof(Tags_Array[0]); i++)  \
       if (Tags_Array[i].sz - 1 == sz && !strncmp(Tags_Array[i].tag, name, sz)) \
         {                                                              \
            return Tags_Array[i].tag_handler;                           \
         }                                                              \
     return NULL;                                                       \
  }

FIND_FACTORY(group, group_tags);
FIND_FACTORY(graphics, graphics_tags);

static void
_handle_radial_cx_attr(Svg_Radial_Gradient* radial, const char *value)
{
   radial->cx = _to_double(value);
}

static void
_handle_radial_cy_attr(Svg_Radial_Gradient* radial, const char *value)
{
   radial->cy = _to_double(value);
}

static void
_handle_radial_fx_attr(Svg_Radial_Gradient* radial, const char *value)
{
   radial->fx = _to_double(value);
}

static void
_handle_radial_fy_attr(Svg_Radial_Gradient* radial, const char *value)
{
   radial->fy = _to_double(value);
}

static void
_handle_radial_r_attr(Svg_Radial_Gradient* radial, const char *value)
{
   radial->r = _to_double(value);
}


typedef void (*Radial_Method)(Svg_Radial_Gradient *radial, const char *value);

#define RADIAL_DEF(Name)       \
  { #Name, sizeof (#Name), _handle_radial_##Name##_attr}

static const struct {
   const char *tag;
   int sz;
   Radial_Method tag_handler;;
} radial_tags[] = {
  RADIAL_DEF(cx),
  RADIAL_DEF(cy),
  RADIAL_DEF(fx),
  RADIAL_DEF(fy),
  RADIAL_DEF(r)
};

static Eina_Bool
_attr_parse_radial_gradient_node(void *data, const char *key, const char *value)
{
   Svg_Style_Gradient *grad = data;
   Svg_Radial_Gradient *radial = grad->radial;
   unsigned int i;
   int sz = strlen(key);

   for (i = 0; i < sizeof (radial_tags) / sizeof(radial_tags[0]); i++)
     if (radial_tags[i].sz - 1 == sz && !strncmp(radial_tags[i].tag, key, sz))
       {
          radial_tags[i].tag_handler(radial, value);
          return EINA_TRUE;
       }

   if (!strcmp(key, "id"))
     grad->id = _copy_id(value);

   return EINA_TRUE;
}

static Svg_Style_Gradient *
_create_radialGradient(const char *buf, unsigned buflen)
{
   Svg_Style_Gradient *grad = calloc(1, sizeof(Svg_Style_Gradient));

   grad->type = SVG_RADIAL_GRADIENT;
   grad->radial = calloc(1, sizeof(Svg_Radial_Gradient));
   eina_simple_xml_attributes_parse(buf, buflen,
                                    _attr_parse_radial_gradient_node, grad);
   return grad;

}

static Eina_Bool
_attr_parse_stops(void *data, const char *key, const char *value)
{
   Efl_Gfx_Gradient_Stop *stop = data;

   if (!strcmp(key, "offset"))
     {
        stop->offset = _to_double(value);
     }
   else if (!strcmp(key, "stop-opacity"))
     {
        stop->a = _to_opacity(value);
     }
   else if (!strcmp(key, "stop-color"))
     {
        _to_color(value, &stop->r, &stop->g, &stop->b, NULL);
     }
   return EINA_TRUE;
}

static void
_handle_linear_x1_attr(Svg_Linear_Gradient* linear, const char *value)
{
   linear->x1 = _to_double(value);
}

static void
_handle_linear_y1_attr(Svg_Linear_Gradient* linear, const char *value)
{
   linear->y1 = _to_double(value);
}

static void
_handle_linear_x2_attr(Svg_Linear_Gradient* linear, const char *value)
{
   linear->x2 = _to_double(value);
}

static void
_handle_linear_y2_attr(Svg_Linear_Gradient* linear, const char *value)
{
   linear->y2 = _to_double(value);
}


typedef void (*Linear_Method)(Svg_Linear_Gradient *linear, const char *value);

#define LINEAR_DEF(Name)       \
  { #Name, sizeof (#Name), _handle_linear_##Name##_attr}

static const struct {
   const char *tag;
   int sz;
   Linear_Method tag_handler;;
} linear_tags[] = {
  LINEAR_DEF(x1),
  LINEAR_DEF(y1),
  LINEAR_DEF(x2),
  LINEAR_DEF(y2)
};

static Eina_Bool
_attr_parse_linear_gradient_node(void *data, const char *key, const char *value)
{
   Svg_Style_Gradient *grad = data;
   Svg_Linear_Gradient *linear = grad->linear;
   unsigned int i;
   int sz = strlen(key);

   for (i = 0; i < sizeof (radial_tags) / sizeof(linear_tags[0]); i++)
     if (linear_tags[i].sz - 1 == sz && !strncmp(linear_tags[i].tag, key, sz))
       {
          linear_tags[i].tag_handler(linear, value);
          return EINA_TRUE;
       }

   if (!strcmp(key, "id"))
     {
        grad->id = _copy_id(value);
     }

   return EINA_TRUE;
}

static Svg_Style_Gradient *
_create_linearGradient(const char *buf, unsigned buflen)
{
   Svg_Style_Gradient *grad = calloc(1, sizeof(Svg_Style_Gradient));

   grad->type = SVG_LINEAR_GRADIENT;
   grad->linear = calloc(1, sizeof(Svg_Linear_Gradient));
   eina_simple_xml_attributes_parse(buf, buflen,
                                    _attr_parse_linear_gradient_node, grad);
   return grad;

}

#define GRADIENT_DEF(Name)                                   \
  { #Name, sizeof (#Name), _create_##Name }

static const struct {
   const char *tag;
   int sz;
   Gradient_Factory_Method tag_handler;
} gradient_tags[] = {
  GRADIENT_DEF(linearGradient),
  GRADIENT_DEF(radialGradient)
};

static Gradient_Factory_Method
_find_gradient_factory(const char  *name)
{
   unsigned int i;
   int sz = strlen(name);

   for (i = 0; i < sizeof (gradient_tags) / sizeof(gradient_tags[0]); i++)
     if (gradient_tags[i].sz - 1 == sz && !strncmp(gradient_tags[i].tag, name, sz))
       {
          return gradient_tags[i].tag_handler;
       }
   return NULL;
}

static void
_evas_svg_loader_xml_open_parser(Evas_SVG_Loader *loader,
                                 const char *content, unsigned int length)
{
   const char *attrs = NULL;
   int attrs_length = 0;
   int sz = length;
   char tag_name[20];
   Factory_Method method;
   Gradient_Factory_Method gradient_method;
   Svg_Node *node = NULL, *parent;
   loader->level++;
   attrs = eina_simple_xml_tag_attributes_find(content, length);

   // find out the tag name starting from content till sz length
   if (attrs)
     {
        sz = attrs - content;
        attrs_length = length - sz;
        while ((sz > 0) && (isspace(content[sz - 1])))
          sz--;
        strncpy(tag_name, content, sz);
        tag_name[sz] = '\0';
     }

   if ((method = _find_group_factory(tag_name)))
     {
        //group
        if (!loader->doc)
          {
             if (strcmp(tag_name, "svg"))
               return; // Not a valid svg document
             node = method(NULL, attrs, attrs_length);
             loader->doc = node;
          }
        else
          {
             parent = eina_array_data_get(loader->stack, eina_array_count(loader->stack) - 1);
             node = method(parent, attrs, attrs_length);
          }
        eina_array_push(loader->stack, node);

        if (node->type == SVG_NODE_DEFS)
          loader->def = node;
     }
   else if ((method = _find_graphics_factory(tag_name)))
     {
        parent = eina_array_data_get(loader->stack, eina_array_count(loader->stack) - 1);
        node = method(parent, attrs, attrs_length);
     }
   else if ((gradient_method = _find_gradient_factory(tag_name)))
     {
        Svg_Style_Gradient *gradient;
        gradient = gradient_method(attrs, attrs_length);
        if (loader->def)
          {
             loader->def->node.defs.gradients = eina_list_append(loader->def->node.defs.gradients, gradient);
          }
        loader->gradient = gradient;
     }
   else if (!strcmp(tag_name, "stop"))
     {
        Efl_Gfx_Gradient_Stop *stop = calloc(1, sizeof(Efl_Gfx_Gradient_Stop));
        eina_simple_xml_attributes_parse(attrs, attrs_length,
                                    _attr_parse_stops, stop);
        if (loader->gradient)
          loader->gradient->stops = eina_list_append(loader->gradient->stops, stop);
     }

}

#define POP_TAG(Tag)                            \
  { #Tag, sizeof (#Tag) }

static const struct {
   const char *tag;
   size_t sz;
} pop_array[] = {
  POP_TAG(g),
  POP_TAG(svg),
  POP_TAG(defs)
};

static void
_evas_svg_loader_xml_close_parser(Evas_SVG_Loader *loader,
                                  const char *content,
                                  unsigned int length EINA_UNUSED)
{
   unsigned int i;

   content = _skip_space(content, NULL);

   for (i = 0; i < sizeof (pop_array) / sizeof (pop_array[0]); i++)
     if (!strncmp(content, pop_array[i].tag, pop_array[i].sz - 1))
       {
          eina_array_pop(loader->stack);
          break ;
       }

   if (!strncmp(content, "linearGradient", 13))
     {
        //TODO
     }

   loader->level--;
}

static Eina_Bool
_evas_svg_loader_parser(void *data, Eina_Simple_XML_Type type,
                        const char *content,
                        unsigned int offset EINA_UNUSED, unsigned int length)
{
   Evas_SVG_Loader *loader = data;

   switch (type)
     {
      case EINA_SIMPLE_XML_OPEN:
         _evas_svg_loader_xml_open_parser(loader, content, length);
         break;
      case EINA_SIMPLE_XML_OPEN_EMPTY:
         _evas_svg_loader_xml_open_parser(loader, content, length);
      case EINA_SIMPLE_XML_CLOSE:
         _evas_svg_loader_xml_close_parser(loader, content, length);
         break;
      case EINA_SIMPLE_XML_DATA:
      case EINA_SIMPLE_XML_CDATA:
      case EINA_SIMPLE_XML_DOCTYPE_CHILD:
         break;
      case EINA_SIMPLE_XML_IGNORED:
      case EINA_SIMPLE_XML_COMMENT:
      case EINA_SIMPLE_XML_DOCTYPE:
         break;

      default:
         break;
     }

   return EINA_TRUE;
}

EAPI Svg_Node *
_svg_load(Eina_File *f, const char *key EINA_UNUSED)
{
   Evas_SVG_Loader loader = {
     NULL, NULL, NULL, NULL, 0, EINA_FALSE
   };
   const char *content;
   unsigned int length;

   if (!f) return NULL;

   length = eina_file_size_get(f);
   content = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (content)
     {
       loader.stack = eina_array_new(8);
       eina_simple_xml_parse(content, length, EINA_TRUE,
                                 _evas_svg_loader_parser, &loader);

       eina_array_free(loader.stack);
       eina_file_map_free(f, (void*) content);
     }
   return loader.doc;
}
