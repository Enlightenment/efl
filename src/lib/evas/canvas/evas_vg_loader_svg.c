#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ctype.h>

#include <Eina.h>
#include <Evas.h>
#include "evas_common_private.h"

typedef struct _Evas_SVG_Loader Evas_SVG_Loader;
struct _Evas_SVG_Loader
{
   Eina_Array *stack;
   Eina_Hash *definition;
   Evas_Object *vg;
   Evas *e;

   unsigned int level;

   Eina_Bool svg : 1;
   Eina_Bool defs : 1;
   Eina_Bool result : 1;
};

static Eina_Bool
_evas_svg_loader_xml_attrs_parser(void *data, const char *key, const char *value)
{
   Evas_SVG_Loader *loader = data;

   fprintf(stderr, "{%s = %s}\n", key, value);

   return EINA_TRUE;
}

static Eina_Bool
_attrs_id_parser(void *data, const char *key, const char *value)
{
   const char **id = data;

   if (!strcmp(key, "id")) *id = eina_stringshare_add(value);
   return EINA_TRUE;
}

static Eina_Bool
_attrs_size_parser(void *data, const char *key, const char *value)
{
   int width, height;
   Evas_Object *vg = data;
   Eina_Bool get_w = EINA_FALSE, get_h = EINA_FALSE;

   eo_do(vg, efl_gfx_view_size_get(&width, &height));
   if (!strcmp(key, "width"))
     get_w = EINA_TRUE;
   else if (!strcmp(key, "height"))
     get_h = EINA_TRUE;

   if (get_w || get_h)
     {
        const char *end = NULL;
        unsigned int r;

        r = strtol(value, &end, 10);
        if (value != end)
          {
             if (get_w) width = r;
             else if (get_h) height = r;
          }
        eo_do(vg, efl_gfx_view_size_set(width, height));
     }

   return EINA_TRUE;
}

static Eina_Bool
_tag_svg_handler(Evas_SVG_Loader *loader,
                 const char *attrs EINA_UNUSED,
                 unsigned int attrs_length EINA_UNUSED)
{
   Eo *node;

   if (loader->level != 1) return EINA_FALSE;

   eina_simple_xml_attributes_parse(attrs, attrs_length,
                                    _attrs_size_parser, loader->vg);

   eo_do(loader->vg, node = evas_obj_vg_root_node_get());
   eina_array_push(loader->stack, node);

   loader->svg = EINA_TRUE;
   return EINA_TRUE;
}

static Eina_Bool
_tag_defs_handler(Evas_SVG_Loader *loader,
                  const char *attrs EINA_UNUSED,
                  unsigned int attrs_length EINA_UNUSED)
{
   if (loader->level != 1) return EINA_FALSE;

   loader->defs = EINA_TRUE;
   return EINA_TRUE;
}

static Eina_Bool
_tag_linearGradient_handler(Evas_SVG_Loader *loader,
                            const char *attrs,
                            unsigned int attrs_length)
{
   const char *id = NULL;
   Eo *node;

   if (loader->level < 2) return EINA_FALSE;

   eina_simple_xml_attributes_parse(attrs, attrs_length,
                                    _attrs_id_parser, &id);

   if (!id) return EINA_FALSE;
   node = eo_add(EFL_VG_GRADIENT_LINEAR_CLASS, NULL);
   if (!node) return EINA_FALSE;

   eina_hash_direct_add(loader->definition, id, node);
   eina_array_push(loader->stack, node);

   return EINA_TRUE;
}

static double
_attr_percent_parser(const char *value)
{
   char *tmp = NULL;
   double r;
   int neg = 1;

   r = strtod(value, &tmp);
   while(tmp && *tmp == '%') tmp++;

   if (tmp && *tmp == '%')
     r = r / 100;

   return r * neg;
}

static unsigned char
_attr_color_component_parser(const char *value, char **end)
{
   double r;

   r = strtod(value + 4, end);
   while (isspace(**end)) (*end)++;
   if (**end == '%')
     r = 255 * r / 100;
   while (isspace(**end)) (*end)++;

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

static Eina_Bool
_attr_color_parser(void *data, const char *key, const char *value)
{
   unsigned int *color = data;
   unsigned char a = A_VAL(color);
   unsigned char r = R_VAL(color);
   unsigned char g = G_VAL(color);
   unsigned char b = B_VAL(color);

   if (!strcmp(key, "stop-color"))
     {
        unsigned int len = strlen(value);

        if (len == 4 && value[0] == '#')
          {
             if (isxdigit(value[1]) &&
                 isxdigit(value[2]) &&
                 isxdigit(value[3]))
               {
                  char tmp[2] = { '\0', '\0' };

                  tmp[0] = value[1]; r = strtol(tmp, NULL, 16);
                  tmp[0] = value[2]; g = strtol(tmp, NULL, 16);
                  tmp[0] = value[3]; b = strtol(tmp, NULL, 16);
               }
          }
        else if (len == 7 && value[0] == '#')
          {
             if (isxdigit(value[1]) &&
                 isxdigit(value[2]) &&
                 isxdigit(value[3]) &&
                 isxdigit(value[4]) &&
                 isxdigit(value[5]) &&
                 isxdigit(value[6]))
               {
                  char tmp[3] = { '\0', '\0', '\0' };

                  tmp[0] = value[1]; tmp[1] = value[2]; r = strtol(tmp, NULL, 16);
                  tmp[0] = value[3]; tmp[1] = value[4]; g = strtol(tmp, NULL, 16);
                  tmp[0] = value[5]; tmp[1] = value[6]; b = strtol(tmp, NULL, 16);
               }
          }
        else if (len >= 10 &&
                 (value[0] == 'r' || value[0] == 'R') &&
                 (value[1] == 'g' || value[1] == 'G') &&
                 (value[2] == 'b' || value[2] == 'B') &&
                 value[3] == '(' &&
                 value[len - 1] == ')')
          {
             char *red, *green, *blue;
             unsigned char tr, tg, tb;

             tr = _attr_color_component_parser(value + 4, &red);
             if (red && *red == ',')
               {
                  tg = _attr_color_component_parser(red + 1, &green);
                  if (green && *green == ',')
                    {
                       tb = _attr_color_component_parser(green + 1, &blue);
                       if (blue && blue[0] == ')' && blue[1] == '\0')
                         {
                            r = tr; g = tg; b = tb;
                         }
                    }
               }
          }
        else
          {
             unsigned int i;

             for (i = 0; i < (sizeof (colors) / sizeof (colors[0])); i++)
               if (!strcasecmp(colors[i].name, value))
                 {
                    r = R_VAL(&(colors[i].value));
                    g = G_VAL(&(colors[i].value));
                    b = B_VAL(&(colors[i].value));
                 }
          }
     }
   else if (!strcmp(key, "stop-opacity"))
     {
        char *tmp = NULL;
        double opacity = strtod(value, &tmp);

        if (*tmp == '\0')
          a = lrint(opacity * 255);
     }

   *color = ARGB_JOIN(a, r, g, b);

   return EINA_TRUE;
}

static Eina_Bool
_attrs_stop_parser(void *data, const char *key, const char *value)
{
   Efl_Gfx_Gradient_Stop *stop = data;

   if (!strcmp(key, "style"))
     {
        unsigned int color = 0xFF000000;

        eina_simple_xml_attribute_w3c_parse(value, _attr_color_parser, &color);

        stop->r = R_VAL(&color);
        stop->g = G_VAL(&color);
        stop->b = B_VAL(&color);
        stop->a = A_VAL(&color);
     }
   else if (!strcmp(key, "offset"))
     {
        stop->offset = _attr_percent_parser(value);
     }

   return EINA_TRUE;
}

static Eina_Bool
_tag_stop_handler(Evas_SVG_Loader *loader,
                  const char *attrs,
                  unsigned int attrs_length)
{
   Efl_Gfx_Gradient_Stop stop = { -1, 0, 0, 0, 0xFF };
   const Efl_Gfx_Gradient_Stop *old = NULL;
   Efl_Gfx_Gradient_Stop *new;
   unsigned int length = 0;
   Eo *node;

   if (loader->level < 3) return EINA_FALSE;
   if (((int)eina_array_count(loader->stack) - 1) < 0) return EINA_FALSE;

   node = eina_array_data_get(loader->stack, eina_array_count(loader->stack) - 1);
   if (!eo_isa(node, EFL_GFX_GRADIENT_BASE_INTERFACE))
     return EINA_FALSE;

   eina_simple_xml_attributes_parse(attrs, attrs_length,
                                    _attrs_stop_parser, &stop);

   if (stop.offset < 0) return EINA_FALSE;

   eo_do(node,
         efl_gfx_gradient_stop_get(&old, &length);
         length++;
         new = malloc(sizeof (Efl_Gfx_Gradient_Stop) * length);
         if (new)
           {
              if (length > 1)
                memcpy(new, old, sizeof (Efl_Gfx_Gradient_Stop) * (length - 1));
              new[length - 1] = stop;

              efl_gfx_gradient_stop_set(new, length);
           });

   return EINA_TRUE;
}

static Eina_Bool
_tag_g_handler(Evas_SVG_Loader *loader,
               const char *attrs,
               unsigned int attrs_length)
{
   Eo *node, *parent;

   if (loader->level < 1) return EINA_FALSE;

   parent = eina_array_data_get(loader->stack, eina_array_count(loader->stack) - 1);

   node = eo_add(EFL_VG_CONTAINER_CLASS, parent);
   eina_array_push(loader->stack, node);

   return EINA_TRUE;
}

static Eina_Bool
_tag_rect_handler(Evas_SVG_Loader *loader,
                  const char *attrs,
                  unsigned int attrs_length)
{
   Eo *node, *parent;

   if (loader->level < 1) return EINA_FALSE;

   parent = eina_array_data_get(loader->stack, eina_array_count(loader->stack) - 1);

   fprintf(stderr, "recty !\n");
   eina_simple_xml_attributes_parse(attrs, attrs_length,
                                    _evas_svg_loader_xml_attrs_parser, loader);

   return EINA_TRUE;
}

#define TAG_DEF(Name)                                   \
  { #Name, sizeof (#Name), _tag_##Name##_handler }

static struct {
   const char *tag;
   int sz;
   Eina_Bool (*tag_handler)(Evas_SVG_Loader *loader, const char *attrs, unsigned int attrs_length);
} open_tags[] = {
  TAG_DEF(svg),
  TAG_DEF(defs),
  TAG_DEF(linearGradient),
  TAG_DEF(stop),
  TAG_DEF(g),
  TAG_DEF(rect)
};

static void
_evas_svg_loader_xml_open_parser(Evas_SVG_Loader *loader,
                                 const char *content, unsigned int length)
{
   const char *attrs = NULL;
   unsigned int i;
   int attrs_length = 0;
   int sz = length;

   loader->level++;

   attrs = eina_simple_xml_tag_attributes_find(content, length);
   if (attrs)
     {
        sz = attrs - content;
        attrs_length = length - sz;
        while ((sz > 0) && (isspace(content[sz - 1])))
          sz--;
     }

   for (i = 0; i < sizeof (open_tags) / sizeof(open_tags[0]); i++)
     if (open_tags[i].sz - 1 == sz && !strncmp(open_tags[i].tag, content, sz))
       {
          if (!open_tags[i].tag_handler(loader, attrs, attrs_length))
            goto on_error;
          return ;
       }

   fprintf(stderr, "[%s]\n", strndupa(content, sz));

   eina_simple_xml_attributes_parse(attrs, attrs_length,
                                    _evas_svg_loader_xml_attrs_parser, loader);

   return ;

 on_error:
   loader->result = EINA_FALSE;
   return ;
}

static const char *poping[] = {
  "linearGradient",
  "svg",
  "g"
};

static void
_evas_svg_loader_xml_close_parser(Evas_SVG_Loader *loader,
                                  const char *content, unsigned int length)
{
   unsigned int i;
   Eina_Bool found = EINA_FALSE;

   loader->level--;

   for (i = 0; i < sizeof (poping) / sizeof (poping[0]); i++)
     {
        unsigned int l = strlen(poping[i]);

        if (!strncmp(content, poping[i], l) &&
            (content[l] == '\0' ||
             content[l] == '>' ||
             isspace(content[l])))
          {
             found = EINA_TRUE;
             eina_array_pop(loader->stack);
             break;
          }
     }
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

Eina_Bool
evas_vg_loader_svg(Evas_Object *vg,
                   const Eina_File *f, const char *key EINA_UNUSED)
// For now we don't handle eet section filled with SVG, that's for later
{
   Evas_SVG_Loader loader = {
     NULL, NULL, NULL, NULL,
     EINA_FALSE, EINA_FALSE, EINA_TRUE
   };
   const char *content;
   Eina_File *tmp;
   unsigned int length;
   Eina_Bool r = EINA_FALSE;

   tmp = eina_file_dup(f);
   if (!f || !tmp) return EINA_FALSE;

   length = eina_file_size_get(tmp);
   content = eina_file_map_all(tmp, EINA_FILE_SEQUENTIAL);

   if (!content) goto on_error;

   loader.stack = eina_array_new(8);
   loader.definition = eina_hash_stringshared_new(eo_del);
   loader.vg = vg;
   loader.e = evas_object_evas_get(vg);

   r = eina_simple_xml_parse(content, length, EINA_TRUE,
                             _evas_svg_loader_parser, &loader);

   eina_array_free(loader.stack);
   eina_hash_free(loader.definition);
   eina_file_map_free(tmp, (void*) content);

 on_error:
   eina_file_close(tmp);

   return loader.result && r;
}
