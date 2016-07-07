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
   if (str == NULL) return NULL;

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

static const struct {
   const char *name;
   unsigned int value;
} colors[] = {
  { "aliceblue", 0xfff0f8ff },
  { "antiquewhite", 0xfffaebd7 },
  { "aqua", 0xff00ffff },
  { "aquamarine", 0xff7fffd4 },
  { "azure", 0xfff0ffff },
  { "beige", 0xfff5f5dc },
  { "bisque", 0xffffe4c4 },
  { "black", 0xff000000 },
  { "blanchedalmond", 0xffffebcd },
  { "blue", 0xff0000ff },
  { "blueviolet", 0xff8a2be2 },
  { "brown", 0xffa52a2a },
  { "burlywood", 0xffdeb887 },
  { "cadetblue", 0xff5f9ea0 },
  { "chartreuse", 0xff7fff00 },
  { "chocolate", 0xffd2691e },
  { "coral", 0xffff7f50 },
  { "cornflowerblue", 0xff6495ed },
  { "cornsilk", 0xfffff8dc },
  { "crimson", 0xffdc143c },
  { "cyan", 0xff00ffff },
  { "darkblue", 0xff00008b },
  { "darkcyan", 0xff008b8b },
  { "darkgoldenrod", 0xffb8860b },
  { "darkgray", 0xffa9a9a9 },
  { "darkgrey", 0xffa9a9a9 },
  { "darkgreen", 0xff006400 },
  { "darkkhaki", 0xffbdb76b },
  { "darkmagenta", 0xff8b008b },
  { "darkolivegreen", 0xff556b2f },
  { "darkorange", 0xffff8c00 },
  { "darkorchid", 0xff9932cc },
  { "darkred", 0xff8b0000 },
  { "darksalmon", 0xffe9967a },
  { "darkseagreen", 0xff8fbc8f },
  { "darkslateblue", 0xff483d8b },
  { "darkslategray", 0xff2f4f4f },
  { "darkslategrey", 0xff2f4f4f },
  { "darkturquoise", 0xff00ced1 },
  { "darkviolet", 0xff9400d3 },
  { "deeppink", 0xffff1493 },
  { "deepskyblue", 0xff00bfff },
  { "dimgray", 0xff696969 },
  { "dimgrey", 0xff696969 },
  { "dodgerblue", 0xff1e90ff },
  { "firebrick", 0xffb22222 },
  { "floralwhite", 0xfffffaf0 },
  { "forestgreen", 0xff228b22 },
  { "fuchsia", 0xffff00ff },
  { "gainsboro", 0xffdcdcdc },
  { "ghostwhite", 0xfff8f8ff },
  { "gold", 0xffffd700 },
  { "goldenrod", 0xffdaa520 },
  { "gray", 0xff808080 },
  { "grey", 0xff808080 },
  { "green", 0xff008000 },
  { "greenyellow", 0xffadff2f },
  { "honeydew", 0xfff0fff0 },
  { "hotpink", 0xffff69b4 },
  { "indianred", 0xffcd5c5c },
  { "indigo", 0xff4b0082 },
  { "ivory", 0xfffffff0 },
  { "khaki", 0xfff0e68c },
  { "lavender", 0xffe6e6fa },
  { "lavenderblush", 0xfffff0f5 },
  { "lawngreen", 0xff7cfc00 },
  { "lemonchiffon", 0xfffffacd },
  { "lightblue", 0xffadd8e6 },
  { "lightcoral", 0xfff08080 },
  { "lightcyan", 0xffe0ffff },
  { "lightgoldenrodyellow", 0xfffafad2 },
  { "lightgray", 0xffd3d3d3 },
  { "lightgrey", 0xffd3d3d3 },
  { "lightgreen", 0xff90ee90 },
  { "lightpink", 0xffffb6c1 },
  { "lightsalmon", 0xffffa07a },
  { "lightseagreen", 0xff20b2aa },
  { "lightskyblue", 0xff87cefa },
  { "lightslategray", 0xff778899 },
  { "lightslategrey", 0xff778899 },
  { "lightsteelblue", 0xffb0c4de },
  { "lightyellow", 0xffffffe0 },
  { "lime", 0xff00ff00 },
  { "limegreen", 0xff32cd32 },
  { "linen", 0xfffaf0e6 },
  { "magenta", 0xffff00ff },
  { "maroon", 0xff800000 },
  { "mediumaquamarine", 0xff66cdaa },
  { "mediumblue", 0xff0000cd },
  { "mediumorchid", 0xffba55d3 },
  { "mediumpurple", 0xff9370d8 },
  { "mediumseagreen", 0xff3cb371 },
  { "mediumslateblue", 0xff7b68ee },
  { "mediumspringgreen", 0xff00fa9a },
  { "mediumturquoise", 0xff48d1cc },
  { "mediumvioletred", 0xffc71585 },
  { "midnightblue", 0xff191970 },
  { "mintcream", 0xfff5fffa },
  { "mistyrose", 0xffffe4e1 },
  { "moccasin", 0xffffe4b5 },
  { "navajowhite", 0xffffdead },
  { "navy", 0xff000080 },
  { "oldlace", 0xfffdf5e6 },
  { "olive", 0xff808000 },
  { "olivedrab", 0xff6b8e23 },
  { "orange", 0xffffa500 },
  { "orangered", 0xffff4500 },
  { "orchid", 0xffda70d6 },
  { "palegoldenrod", 0xffeee8aa },
  { "palegreen", 0xff98fb98 },
  { "paleturquoise", 0xffafeeee },
  { "palevioletred", 0xffd87093 },
  { "papayawhip", 0xffffefd5 },
  { "peachpuff", 0xffffdab9 },
  { "peru", 0xffcd853f },
  { "pink", 0xffffc0cb },
  { "plum", 0xffdda0dd },
  { "powderblue", 0xffb0e0e6 },
  { "purple", 0xff800080 },
  { "red", 0xffff0000 },
  { "rosybrown", 0xffbc8f8f },
  { "royalblue", 0xff4169e1 },
  { "saddlebrown", 0xff8b4513 },
  { "salmon", 0xfffa8072 },
  { "sandybrown", 0xfff4a460 },
  { "seagreen", 0xff2e8b57 },
  { "seashell", 0xfffff5ee },
  { "sienna", 0xffa0522d },
  { "silver", 0xffc0c0c0 },
  { "skyblue", 0xff87ceeb },
  { "slateblue", 0xff6a5acd },
  { "slategray", 0xff708090 },
  { "slategrey", 0xff708090 },
  { "snow", 0xfffffafa },
  { "springgreen", 0xff00ff7f },
  { "steelblue", 0xff4682b4 },
  { "tan", 0xffd2b48c },
  { "teal", 0xff008080 },
  { "thistle", 0xffd8bfd8 },
  { "tomato", 0xffff6347 },
  { "turquoise", 0xff40e0d0 },
  { "violet", 0xffee82ee },
  { "wheat", 0xfff5deb3 },
  { "white", 0xffffffff },
  { "whitesmoke", 0xfff5f5f5 },
  { "yellow", 0xffffff00 },
  { "yellowgreen", 0xff9acd32 }
};

static inline void
_to_color(const char *str, int *r, int *g, int *b, char** ref)
{
   unsigned int i, len = strlen(str);
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
        //handle named color
        for (i = 0; i < (sizeof (colors) / sizeof (colors[0])); i++)
          if (!strcasecmp(colors[i].name, str))
            {
               *r = R_VAL(&(colors[i].value));
               *g = G_VAL(&(colors[i].value));
               *b = B_VAL(&(colors[i].value));
            }
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
   int pt_count = 0;
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
        for (i = 0; i < sizeof (matrix_tags) / sizeof(matrix_tags[0]); i++)
          if (!strncmp(matrix_tags[i].tag, str, matrix_tags[i].sz -1))
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
                  eina_matrix3_rotate(matrix, points[0] * (M_PI/180.0));
               }
             else if (pt_count == 3)
               {
                  eina_matrix3_translate(matrix, points[1], points[2]);
                  eina_matrix3_rotate(matrix, points[0] * (M_PI/180.0));
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

//https://www.w3.org/TR/SVGTiny12/painting.html#SpecifyingPaint
static void
_handle_paint_attr(Svg_Paint* paint, const char *value)
{
   if (!strcmp(value, "none"))
     {
        // no paint property
        paint->none = EINA_TRUE;
        return;
     }
   paint->none = EINA_FALSE;
   if (!strcmp(value, "currentColor"))
     {
        paint->cur_color = EINA_TRUE;
        return;
     }
   _to_color(value, &paint->r, &paint->g, &paint->b, &paint->url);
}

static void
_handle_color_attr(Svg_Node* node, const char *value)
{
   Svg_Style_Property *style = node->style;
   _to_color(value, &style->r, &style->g, &style->b, NULL);
}

static void
_handle_fill_attr(Svg_Node* node, const char *value)
{
   Svg_Style_Property *style = node->style;
   style->fill.flags |= SVG_FILL_FLAGS_PAINT;
   _handle_paint_attr(&style->fill.paint, value);
}

static void
_handle_stroke_attr(Svg_Node* node, const char *value)
{
   Svg_Style_Property *style = node->style;
   style->stroke.flags |= SVG_STROKE_FLAGS_PAINT;
   _handle_paint_attr(&style->stroke.paint, value);
}

static void
_handle_stroke_opacity_attr(Svg_Node* node, const char *value)
{
   node->style->stroke.flags |= SVG_STROKE_FLAGS_OPACITY;
   node->style->stroke.opacity = _to_opacity(value);
}

static void
_handle_stroke_width_attr(Svg_Node* node, const char *value)
{
   node->style->stroke.flags |= SVG_STROKE_FLAGS_WIDTH;
   node->style->stroke.width = _to_double(value);
}

static void
_handle_stroke_linecap_attr(Svg_Node* node, const char *value)
{
   node->style->stroke.flags |= SVG_STROKE_FLAGS_CAP;
   node->style->stroke.cap = _to_line_cap(value);
}

static void
_handle_stroke_linejoin_attr(Svg_Node* node, const char *value)
{
   node->style->stroke.flags |= SVG_STROKE_FLAGS_JOIN;
   node->style->stroke.join = _to_line_join(value);
}

static void
_handle_fill_rule_attr(Svg_Node* node, const char *value)
{
   node->style->fill.flags |= SVG_FILL_FLAGS_FILL_RULE;
   node->style->fill.fill_rule = _to_fill_rule(value);
}

static void
_handle_fill_opacity_attr(Svg_Node* node, const char *value)
{
   node->style->fill.flags |= SVG_FILL_FLAGS_OPACITY;
   node->style->fill.opacity = _to_opacity(value);
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
   else
     {
        _parse_style_attr(node, key, value);
     }
   return EINA_TRUE;
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

static Svg_Node *
_create_defs_node(Svg_Node *parent EINA_UNUSED, const char *buf EINA_UNUSED, unsigned buflen EINA_UNUSED)
{
   Svg_Node *node = _create_node(NULL, SVG_NODE_DEFS);
   eina_simple_xml_attributes_parse(buf, buflen,
                                    NULL, node);
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
        path->path = strdup(value);
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

static Svg_Node *
_create_polyline_node(Svg_Node *parent, const char *buf, unsigned buflen)
{
   Svg_Node *node = _create_node(parent, SVG_NODE_POLYLINE);

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

static char *
_id_from_href(const char *href)
{
   href = _skip_space(href, NULL);
   if ((*href) == '#')
     href++;
   return strdup(href);
}

static Svg_Node*
_get_defs_node(Svg_Node *node)
{
   if (!node) return NULL;

   while (node->parent != NULL)
     {
        node = node->parent;
     }

   if (node->type == SVG_NODE_DOC)
     return node->node.doc.defs;

   return NULL;
}

static Svg_Node*
_find_child_by_id(Svg_Node *node, char *id)
{
   Eina_List *l;
   Svg_Node *child;

   if (!node) return NULL;

   EINA_LIST_FOREACH(node->child, l, child)
     {
        if ((child->id != NULL) && !strcmp(child->id, id))
          return child;
     }
   return NULL;
}

static Eina_List *
_clone_grad_stops(Eina_List *from)
{
   Efl_Gfx_Gradient_Stop *stop;
   Eina_List *l;
   Eina_List *res = NULL;

   EINA_LIST_FOREACH(from, l, stop)
     {
        Efl_Gfx_Gradient_Stop *new_stop;

        new_stop = calloc(1, sizeof(Efl_Gfx_Gradient_Stop));
        memcpy(new_stop, stop, sizeof(Efl_Gfx_Gradient_Stop));
        res = eina_list_append(res, new_stop);
     }

   return res;
}

static Svg_Style_Gradient *
_clone_gradient(Svg_Style_Gradient *from)
{
   Svg_Style_Gradient *grad;

   if (!from) return NULL;

   grad= calloc(1, sizeof(Svg_Style_Gradient));
   grad->type = from->type;
   grad->id = _copy_id(from->id);
   grad->ref = _copy_id(from->ref);
   grad->spread = from->spread;
   grad->stops = _clone_grad_stops(from->stops);
   if (grad->type == SVG_LINEAR_GRADIENT)
     {
        grad->linear = calloc(1, sizeof(Svg_Linear_Gradient));
        memcpy(grad->linear, from->linear, sizeof(Svg_Linear_Gradient));
     }
   else if (grad->type == SVG_RADIAL_GRADIENT)
     {
        grad->radial = calloc(1, sizeof(Svg_Radial_Gradient));
        memcpy(grad->radial, from->radial, sizeof(Svg_Radial_Gradient));
     }

   return grad;
}

static void
_copy_attribute(Svg_Node *to, Svg_Node *from)
{
   // copy matrix attribute
   if (from->transform)
     {
        to->transform = calloc(1, sizeof(Eina_Matrix3));
        eina_matrix3_copy(to->transform, from->transform);
     }
   // copy style attribute;
   memcpy(to->style, from->style, sizeof(Svg_Style_Property));

   // copy node attribute
   switch (from->type)
     {
        case SVG_NODE_CIRCLE:
           to->node.circle.cx = from->node.circle.cx;
           to->node.circle.cy = from->node.circle.cy;
           to->node.circle.r = from->node.circle.r;
           break;
        case SVG_NODE_ELLIPSE:
           to->node.ellipse.cx = from->node.ellipse.cx;
           to->node.ellipse.cy = from->node.ellipse.cy;
           to->node.ellipse.rx = from->node.ellipse.rx;
           to->node.ellipse.ry = from->node.ellipse.ry;
           break;
        case SVG_NODE_RECT:
           to->node.rect.x = from->node.rect.x;
           to->node.rect.y = from->node.rect.y;
           to->node.rect.w = from->node.rect.w;
           to->node.rect.h = from->node.rect.h;
           to->node.rect.rx = from->node.rect.rx;
           to->node.rect.ry = from->node.rect.ry;
           break;
        case SVG_NODE_PATH:
           to->node.path.path = strdup(from->node.path.path);
           break;
        case SVG_NODE_POLYGON:
           to->node.polygon.points_count = from->node.polygon.points_count;
           to->node.polygon.points = calloc(to->node.polygon.points_count, sizeof(double));
           break;
        default:
           break;
     }

}

static void
_clone_node(Svg_Node *from, Svg_Node *parent)
{
   Svg_Node *new_node;
   Eina_List *l;
   Svg_Node *child;

   if (!from) return;

   new_node = _create_node(parent, from->type);
   _copy_attribute(new_node, from);

   EINA_LIST_FOREACH(from->child, l, child)
     {
         _clone_node(child, new_node);
     }

}

static Eina_Bool
_attr_parse_use_node(void *data, const char *key, const char *value)
{
   Svg_Node *defs, *node_from, *node = data;
   char *id;

   if (!strcmp(key, "xlink:href"))
     {
        id = _id_from_href(value);
        defs = _get_defs_node(node);
        node_from = _find_child_by_id(defs, id);
        _clone_node(node_from, node);
        free(id);
     }
   else
     {
        _attr_parse_g_node(data, key, value);
     }
   return EINA_TRUE;
}

static Svg_Node *
_create_use_node(Svg_Node *parent, const char *buf, unsigned buflen)
{
   Svg_Node *node = _create_node(parent, SVG_NODE_G);

   eina_simple_xml_attributes_parse(buf, buflen,
                                    _attr_parse_use_node, node);
   return node;
}

#define TAG_DEF(Name)                                   \
  { #Name, sizeof (#Name), _create_##Name##_node }

static const struct {
   const char *tag;
   int sz;
   Factory_Method tag_handler;
} graphics_tags[] = {
  TAG_DEF(use),
  TAG_DEF(circle),
  TAG_DEF(ellipse),
  TAG_DEF(path),
  TAG_DEF(polygon),
  TAG_DEF(rect),
  TAG_DEF(polyline),
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

Efl_Gfx_Gradient_Spread
_parse_spread_value(const char *value)
{
   Efl_Gfx_Gradient_Spread spread = EFL_GFX_GRADIENT_SPREAD_PAD;

   if (!strcmp(value, "reflect"))
     {
        spread = EFL_GFX_GRADIENT_SPREAD_REFLECT;
     }
   else if (!strcmp(value, "repeat"))
     {
        spread = EFL_GFX_GRADIENT_SPREAD_REPEAT;
     }

   return spread;
}

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
     {
        grad->id = _copy_id(value);
     }
   else if (!strcmp(key, "spreadMethod"))
     {
        grad->spread = _parse_spread_value(value);
     }
   else if (!strcmp(key, "xlink:href"))
     {
        grad->ref = _id_from_href(value);
     }

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
   else if (!strcmp(key, "style"))
     {
        eina_simple_xml_attribute_w3c_parse(value,
                                            _attr_parse_stops, data);
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

   for (i = 0; i < sizeof (linear_tags) / sizeof(linear_tags[0]); i++)
     if (linear_tags[i].sz - 1 == sz && !strncmp(linear_tags[i].tag, key, sz))
       {
          linear_tags[i].tag_handler(linear, value);
          return EINA_TRUE;
       }

   if (!strcmp(key, "id"))
     {
        grad->id = _copy_id(value);
     }
   else if (!strcmp(key, "spreadMethod"))
     {
        grad->spread = _parse_spread_value(value);
     }
   else if (!strcmp(key, "xlink:href"))
     {
        grad->ref = _id_from_href(value);
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

   if (!attrs)
     {
        // parse the empty tag
        attrs = content;
        while ((attrs != NULL) && *attrs != '>')
          attrs++;
     }

   if (attrs)
     {
        // find out the tag name starting from content till sz length
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
        {
          loader->doc->node.doc.defs = node;
          loader->def = node;
        }
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
        if (loader->doc->node.doc.defs)
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

static void
_inherit_style(Svg_Style_Property *child, Svg_Style_Property *parent)
{
   if (parent == NULL)
     return;
   // inherit the property of parent if not present in child. 
   // fill
   if (!(child->fill.flags & SVG_FILL_FLAGS_PAINT))
     {
        child->fill.paint.r = parent->fill.paint.r;
        child->fill.paint.g = parent->fill.paint.g;
        child->fill.paint.b = parent->fill.paint.b;
        child->fill.paint.none = parent->fill.paint.none;
        child->fill.paint.cur_color = parent->fill.paint.cur_color;
        child->fill.paint.url = _copy_id(parent->fill.paint.url);
     }
   if (!(child->fill.flags & SVG_FILL_FLAGS_OPACITY))
     {
        child->fill.opacity = parent->fill.opacity;
     }
   if (!(child->fill.flags & SVG_FILL_FLAGS_FILL_RULE))
     {
        child->fill.fill_rule = parent->fill.fill_rule;
     }
   // stroke
   if (!(child->stroke.flags & SVG_STROKE_FLAGS_PAINT))
     {
        child->stroke.paint.r = parent->stroke.paint.r;
        child->stroke.paint.g = parent->stroke.paint.g;
        child->stroke.paint.b = parent->stroke.paint.b;
        child->stroke.paint.none = parent->stroke.paint.none;
        child->stroke.paint.cur_color = parent->stroke.paint.cur_color;
        child->stroke.paint.url = _copy_id(parent->stroke.paint.url);
     }
   if (!(child->stroke.flags & SVG_STROKE_FLAGS_OPACITY))
     {
        child->stroke.opacity = parent->stroke.opacity;
     }
   if (!(child->stroke.flags & SVG_STROKE_FLAGS_WIDTH))
     {
        child->stroke.width = parent->stroke.width;
     }
   if (!(child->stroke.flags & SVG_STROKE_FLAGS_CAP))
     {
        child->stroke.cap = parent->stroke.cap;
     }
   if (!(child->stroke.flags & SVG_STROKE_FLAGS_JOIN))
     {
        child->stroke.join = parent->stroke.join;
     }
}

void
_update_style(Svg_Node *node, Svg_Style_Property *parent_style)
{
   Eina_List *l;
   Svg_Node *child;

   _inherit_style(node->style, parent_style);

   EINA_LIST_FOREACH(node->child, l, child)
     {
        _update_style(child, node->style);
     }
}

static Svg_Style_Gradient*
_dup_gradient(Eina_List *grad_list, const char *id)
{
   Svg_Style_Gradient *grad;
   Svg_Style_Gradient *result = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(grad_list, l, grad)
     {
        if (!strcmp(grad->id, id))
          {
             result = _clone_gradient(grad);
             break;
          }
     }

   if (result && result->ref)
     {
        EINA_LIST_FOREACH(grad_list, l, grad)
          {
             if (!strcmp(grad->id, result->ref))
               {
                  if (!result->stops)
                    {
                       result->stops = _clone_grad_stops(grad->stops);
                    }
                  //TODO properly inherit other property
                  break;
               }
         }
     }

   return result;
}

void
_update_gradient(Svg_Node *node, Eina_List *grad_list)
{
   Eina_List *l;
   Svg_Node *child;

   if (node->child)
     {
        EINA_LIST_FOREACH(node->child, l, child)
          {
             _update_gradient(child, grad_list);
          }
     }
   else
     {
        if (node->style->fill.paint.url)
          {
             node->style->fill.paint.gradient = _dup_gradient(grad_list, node->style->fill.paint.url);
          }
        else if (node->style->stroke.paint.url)
          {
             node->style->stroke.paint.gradient = _dup_gradient(grad_list, node->style->stroke.paint.url);
          }
     }
}

EAPI Svg_Node *
_svg_load(Eina_File *f, const char *key EINA_UNUSED)
{
   Evas_SVG_Loader loader = {
     NULL, NULL, NULL, NULL, 0, EINA_FALSE
   };
   const char *content;
   unsigned int length;
   Svg_Node *defs;

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

   if (loader.doc)
     {
        _update_style(loader.doc, NULL);
        defs = loader.doc->node.doc.defs;
        if (defs)
          _update_gradient(loader.doc, defs->node.defs.gradients);
     }

   return loader.doc;
}
