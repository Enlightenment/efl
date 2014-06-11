#include "evas_filter_private.h"
#include <stdarg.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#if LUA_VERSION_NUM == 502
# define LUA52 1
#endif

#ifdef DEBUG
# define FILTERS_DEBUG
#endif

#define FILTERS_LEGACY_COMPAT

#define EVAS_FILTER_MODE_GROW   (EVAS_FILTER_MODE_LAST+1)
#define EVAS_FILTER_MODE_BUFFER (EVAS_FILTER_MODE_LAST+2)

/* Note on the documentation:
 * To keep it simple, I'm not using any fancy features, only <ul>/<li> lists
 * and @a, @b, @c flags from Doxygen.
 * Let's keep it that way.
 *
 * This is a REFERENCE documentation, not supposed to contain tons of examples,
 * but each filter command should have one simple copy and pasteable example.
 */

/**
  @page evasfiltersref Evas filters reference

  The Evas filters are a combination of filters used to apply specific effects
  to an @ref Evas_Object "Evas Object". For the moment, these effects are
  specific to the @ref Evas_Object_Text "Text Objects".

  The filters can be applied to an object using simple Lua scripts. A script
  will contain a series of buffer declarations and filter commands to apply
  to these buffers. The Lua programming language reference can be found
  <a href="http://www.lua.org/manual/5.1/manual.html">here</a>.

  Basically, when applying an effect to a @ref Evas_Object_Text "Text Object",
  an alpha-only @c input buffer is created, where the text is rendered, and
  an RGBA @c output buffer is created, where the text with effects shall be
  finally rendered.

  The script language being Lua, it respects the usual Lua syntax and concepts.
  As these are simple filters, the scripts should be kept as small and simple
  as possible.

  Note: Lua has been used since 1.10. The previous filters syntax is not
        garanteed to be compatible with 1.10 and newer versions.

  Here are the available commands:
  <ul>
    <li> @ref sec_syntax "Syntax" </li>
    <li> @ref sec_buffers "Buffer management" </li>
    <ul>
      <li> @ref sec_buffers_cspace "Colorspaces" </li>
      <li> @ref sec_buffers_auto "Automatic buffers" </li>
      <li> @ref sec_buffers_cmd "@c buffer command" </li>
    </ul>
    <li> @ref sec_commands "Commands" </li>
    <ul>
      <li> @ref sec_commands_blend "@c blend command"</li>
      <li> @ref sec_commands_blur "@c blur command"</li>
      <li> @ref sec_commands_grow "@c grow command"</li>
      <li> @ref sec_commands_curve "@c curve command"</li>
      <li> @ref sec_commands_fill "@c fill command"</li>
      <li> @ref sec_commands_mask "@c mask command"</li>
      <li> @ref sec_commands_bump "@c bump command"</li>
      <li> @ref sec_commands_displace "@c displace command"</li>
      <li> @ref sec_commands_transform "@c transform command"</li>
    </ul>
  </ul>

  All the examples in this page can (should) be directly used in
  @ref evas_obj_text_filter_program_set.

  Note that most of the text effects work better with larger font sizes (> 50px),
  and so do the examples in this page (embedded devices in mind).
 */

/**
  @page evasfiltersref
  @section sec_syntax Syntax

  Here is a simple example illustrating the syntax:

  @verbinclude filter_example_1.lua

  This example will display a cyan and dark blue glow surrounding the
  main text (its color depends on the object's theme).

  <center>
  @image html filter_example_1.png
  </center>

  The syntax is pretty simple and follows a small set of rules:
  <ul>
    <li>All dimensions are in pixels</li>
    <li>The commands will be executed in sequential order</li>
    <li>Most commands have default values</li>
    <li>A command argument can either be set by name, or sequentially omitting the name. So that:<br>
    @verbatim function (arg1, arg2, arg3) @endverbatim</li>
    <li>is equivalent to:<br>
    @verbatim function ({ arg1, arg2, arg2 }) @endverbatim</li>
    <li>or even (considering opt1, opt2 and opt3 are the first 3 arguments):<br>
    @verbatim function ({ opt1 = arg1, opt2 = arg2, opt3 = arg3 }) @endverbatim</li>
    <li>and since this is Lua, we can also write it as:<br>
    @verbatim function { arg1, opt3 = arg3, opt2 = arg2 } @endverbatim</li>
    <li>Boolean values are @c true/@c false but 1/0 and special string values are also accepted: 'yes'/'no', 'enabled'/'disabled'</li>
  </ul>

  <h3>Special keywords and their values</h3>

  Some options accept a certain set of values (like enums):
  <ul>
    <li>@c color</li>
    @anchor evasfilters_color
    <ul>
      <li>Colors can be referred to by strings or integers:</li>
      <li>An integer can refer to any RGB or ARGB values:
          @c 0xRRGGBB or @c 0xAARRGGBB. If alpha is zero, the color
          will be opaque (alpha = @c 0xFF), unless R=G=B=0 (invisible).
          These colors are <b>not</b> premultiplied.
      </li>
      <li>Hexademical values: @c '#RRGGBB', @c '#RRGGBBAA', @c '#RGB', @c '#RGBA'</li>
      <li>The following string values are also accepted:</li>
      <tt><ul>
        <li>'white' == '#FFFFFF'</li>
        <li>'black' == '#000000'</li>
        <li>'red' == '#FF0000'</li>
        <li>'green' == '#008000'</li>
        <li>'blue' == '#0000FF'</li>
        <li>'darkblue' == '#0000A0'</li>
        <li>'yellow' == '#FFFF00'</li>
        <li>'magenta' == '#FF00FF'</li>
        <li>'cyan' == '#00FFFF'</li>
        <li>'orange' == '#FFA500'</li>
        <li>'purple' == '#800080'</li>
        <li>'brown' == '#A52A2A'</li>
        <li>'maroon' == '#800000'</li>
        <li>'lime' == '#00FF00'</li>
        <li>'gray' == '#808080'</li>
        <li>'grey' == '#808080'</li>
        <li>'silver' == '#C0C0C0'</li>
        <li>'olive' == '#808000'</li>
        <li>'invisible', 'transparent' == '#0000' -- (alpha is zero)</li>
      </ul></tt>
    </ul>
    <li>@c fillmode</li>
    @anchor evasfilter_fillmode
    <tt><ul>
      <li>'none'</li>
      <li>'stretch_x'</li>
      <li>'stretch_y'</li>
      <li>'repeat_x'</li>
      <li>'repeat_y'</li>
      <li>'repeat_x_stretch_y', 'stretch_y_repeat_x'</li>
      <li>'repeat_y_stretch_x', 'stretch_x_repeat_y'</li>
      <li>'repeat', 'repeat_xy'</li>
      <li>'stretch', 'stretch_xy'</li>
    </ul></tt>
  </ul>
 */

/**
  @page evasfiltersref
  @section sec_buffers Buffer management

  The Evas filters subsystem is based on the concept of using various
  buffers as image layers and drawing or applying filters to these buffers.
  Think of it as how image drawing tools like The Gimp can combine multiple
  layers and apply operations on them.

  Most of the buffers are allocated automatically at runtime, depending on the
  various inputs and commands used (eg. 2-D blur will require a temporary
  intermediate buffer).

  @subsection sec_buffers_cspace Colorspaces and size

  The buffers' size will be automatically defined at runtime, based on the
  content of the input and the series of operations to apply (eg. blur adds
  some necessary margins).

  The buffers can be either ALPHA (1 color channel only) or RGBA (full color).
  Some operations might require specifically an ALPHA buffer, some others RGBA.

  Most buffers will have the same size, except those specified by an external
  source.


  @subsection sec_buffers_auto Automatic buffers

  The two most important buffers, input and output, are statically defined and
  always present when running a filter. input is an ALPHA buffer, containing
  the @ref Evas_Object_Text "Text Object"'s rendered text, and output is the
  final target on which to render as RGBA.

  Some operations, like 2-D blur might require temporary intermediate buffers,
  that will be allocated automatically. Those buffers are internal only and
  can't be used from the script.

  Finally, if a buffer is created using another Evas Object as source (see
  @ref sec_buffers_cmd "buffer" for more details), its pixel data will be filled
  by rendering the Evas Object into this buffer. This is how it will be
  possible to load external images, textures and even animations into a buffer.

  @since 1.9
 */

// Map of the most common HTML color names
static struct
{
   const char *name;
   DATA32 value;
} color_map[] =
{
   { "white", 0xFFFFFFFF },
   { "black", 0xFF000000 },
   { "red", 0xFFFF0000 },
   { "green", 0xFF008000 },
   { "blue", 0xFF0000FF },
   { "darkblue", 0xFF0000A0 },
   { "yellow", 0xFFFFFF00 },
   { "magenta", 0xFFFF00FF },
   { "cyan", 0xFF00FFFF },
   { "orange", 0xFFFFA500 },
   { "purple", 0xFF800080 },
   { "brown", 0xFFA52A2A },
   { "maroon", 0xFF800000 },
   { "lime", 0xFF00FF00 },
   { "gray", 0xFF808080 },
   { "grey", 0xFF808080 },
   { "silver", 0xFFC0C0C0 },
   { "olive", 0xFF808000 },
   { "invisible", 0x00000000 },
   { "transparent", 0x00000000 }
};

static struct
{
   const char *name;
   Evas_Filter_Fill_Mode value;
} fill_modes[] =
{
   { "none", EVAS_FILTER_FILL_MODE_NONE },
   { "stretch_x", EVAS_FILTER_FILL_MODE_STRETCH_X },
   { "stretch_y", EVAS_FILTER_FILL_MODE_STRETCH_Y },
   { "repeat_x", EVAS_FILTER_FILL_MODE_REPEAT_X },
   { "repeat_y", EVAS_FILTER_FILL_MODE_REPEAT_Y },
   { "repeat_x_stretch_y", EVAS_FILTER_FILL_MODE_REPEAT_X_STRETCH_Y },
   { "repeat_y_stretch_x", EVAS_FILTER_FILL_MODE_REPEAT_Y_STRETCH_X },
   { "stretch_y_repeat_x", EVAS_FILTER_FILL_MODE_REPEAT_X_STRETCH_Y }, // alias
   { "stretch_x_repeat_y", EVAS_FILTER_FILL_MODE_REPEAT_Y_STRETCH_X }, // alias
   { "repeat", EVAS_FILTER_FILL_MODE_REPEAT_XY }, // alias
   { "repeat_xy", EVAS_FILTER_FILL_MODE_REPEAT_XY },
   { "stretch", EVAS_FILTER_FILL_MODE_STRETCH_XY }, // alias
   { "stretch_xy", EVAS_FILTER_FILL_MODE_STRETCH_XY }
};

static Evas_Filter_Fill_Mode _fill_mode_get(Evas_Filter_Instruction *instr);

typedef enum
{
   VT_NONE,
   VT_BOOL,
   VT_INT,
   VT_REAL,
   VT_STRING,
   VT_COLOR,
   VT_BUFFER
} Value_Type;

typedef struct _Buffer
{
   EINA_INLIST;
   Eina_Stringshare *name;
   Eina_Stringshare *proxy;
   int cid; // Transient value
   struct {
      int l, r, t, b; // Used for padding calculation. Can change over time.
   } pad;
   Eina_Bool alpha : 1;
} Buffer;

typedef struct _Instruction_Param
{
   EINA_INLIST;
   Eina_Stringshare *name;
   Value_Type type;
   Eina_Value *value;
   Eina_Bool set : 1;
   Eina_Bool allow_seq : 1;
   Eina_Bool allow_any_string : 1;
} Instruction_Param;

struct _Evas_Filter_Instruction
{
   EINA_INLIST;
   Eina_Stringshare *name;
   int /*Evas_Filter_Mode*/ type;
   Eina_Inlist /* Instruction_Param */ *params;
   int return_count;
   Eina_Bool (* parse_run) (lua_State *L, Evas_Filter_Program *, Evas_Filter_Instruction *);
   struct
   {
      void (* update) (Evas_Filter_Program *, Evas_Filter_Instruction *, int *, int *, int *, int *);
   } pad;
   Eina_Bool valid : 1;
};

struct _Evas_Filter_Program
{
   Eina_Stringshare *name; // Optional for now
   Eina_Hash /* const char * : Evas_Filter_Proxy_Binding */ *proxies;
   Eina_Inlist /* Evas_Filter_Instruction */ *instructions;
   Eina_Inlist /* Buffer */ *buffers;
   struct {
      int l, r, t, b;
   } pad;
   Eina_Bool valid : 1;
   Eina_Bool padding_calc : 1; // Padding has been calculated
   Eina_Bool padding_set : 1; // Padding has been forced
};

/* Instructions */
static Evas_Filter_Instruction *
_instruction_new(const char *name)
{
   Evas_Filter_Instruction *instr;

   instr = calloc(1, sizeof(Evas_Filter_Instruction));
   instr->name = eina_stringshare_add(name);

   return instr;
}

static Eina_Bool
_instruction_param_addv(Evas_Filter_Instruction *instr, const char *name,
                        Value_Type format, Eina_Bool sequential, va_list args)
{
   const Eina_Value_Type *type = NULL;
   Instruction_Param *param;

   switch (format)
     {
      case VT_BOOL:
      case VT_INT:
        type = EINA_VALUE_TYPE_INT;
        break;
      case VT_REAL:
        type = EINA_VALUE_TYPE_DOUBLE;
        break;
      case VT_STRING:
      case VT_BUFFER:
        type = EINA_VALUE_TYPE_STRING;
        break;
      case VT_COLOR:
        type = EINA_VALUE_TYPE_UINT;
        break;
      case VT_NONE:
      default:
        return EINA_FALSE;
     }

   param = calloc(1, sizeof(Instruction_Param));
   param->name = eina_stringshare_add(name);
   param->type = format;
   param->value = eina_value_new(type);
   param->allow_seq = sequential;
   eina_value_vset(param->value, args);
   instr->params = eina_inlist_append(instr->params, EINA_INLIST_GET(param));

   return EINA_TRUE;
}

static Eina_Bool
_instruction_param_adda(Evas_Filter_Instruction *instr, const char *name,
                        Value_Type format, Eina_Bool sequential,
                        /* default value */ ...)
{
   Eina_Bool ok;
   va_list args;

   va_start(args, sequential);
   ok = _instruction_param_addv(instr, name, format, sequential, args);
   va_end(args);

   return ok;
}
#define _instruction_param_seq_add(a,b,c,d) _instruction_param_adda((a),(b),(c),1,(d))
#define _instruction_param_name_add(a,b,c,d) _instruction_param_adda((a),(b),(c),0,(d))

static void
_instruction_del(Evas_Filter_Instruction *instr)
{
   Instruction_Param *param;

   if (!instr) return;
   EINA_INLIST_FREE(instr->params, param)
     {
        eina_value_free(param->value);
        eina_stringshare_del(param->name);
        instr->params = eina_inlist_remove(instr->params, EINA_INLIST_GET(param));
        free(param);
     }
   eina_stringshare_del(instr->name);
   free(instr);
}

static Instruction_Param *
_instruction_param_get(Evas_Filter_Instruction *instr, const char *name)
{
   Instruction_Param *param;

   EINA_INLIST_FOREACH(instr->params, param)
     if (!strcasecmp(name, param->name))
       return param;

   return NULL;
}

static int
_instruction_param_geti(Evas_Filter_Instruction *instr, const char *name,
                        Eina_Bool *isset)
{
   Instruction_Param *param;
   int i = 0;

   EINA_INLIST_FOREACH(instr->params, param)
     if (!strcasecmp(name, param->name))
       {
          if (eina_value_get(param->value, &i))
            {
               if (isset) *isset = param->set;
               return i;
            }
          else return -1;
       }

   if (isset) *isset = EINA_FALSE;
   return -1;
}

static double
_instruction_param_getd(Evas_Filter_Instruction *instr, const char *name,
                        Eina_Bool *isset)
{
   Instruction_Param *param;
   double i = 0;

   EINA_INLIST_FOREACH(instr->params, param)
     if (!strcasecmp(name, param->name))
       {
          if (eina_value_get(param->value, &i))
            {
               if (isset) *isset = param->set;
               return i;
            }
          else return 0.0;
       }

   if (isset) *isset = EINA_FALSE;
   return 0.0;
}

static DATA32
_instruction_param_getc(Evas_Filter_Instruction *instr, const char *name,
                        Eina_Bool *isset)
{
   Instruction_Param *param;
   DATA32 i = 0;

   EINA_INLIST_FOREACH(instr->params, param)
     if (!strcasecmp(name, param->name))
       {
          if (eina_value_get(param->value, &i))
            {
               if (isset) *isset = param->set;
               return i;
            }
          else return 0;
       }

   if (isset) *isset = EINA_FALSE;
   return 0;
}

static const char *
_instruction_param_gets(Evas_Filter_Instruction *instr, const char *name,
                        Eina_Bool *isset)
{
   Instruction_Param *param;
   const char *str = NULL;

   EINA_INLIST_FOREACH(instr->params, param)
     if (!strcasecmp(name, param->name))
       {
          if (eina_value_get(param->value, &str))
            {
               if (isset) *isset = param->set;
               return str;
            }
          else return NULL;
       }

   if (isset) *isset = EINA_FALSE;
   return NULL;
}

static Eina_Bool
_bool_parse(const char *str, Eina_Bool *b)
{
   if (!str || !*str) return EINA_FALSE;
   if (!strcmp(str, "1") ||
       !strcasecmp(str, "yes") ||
       !strcasecmp(str, "on") ||
       !strcasecmp(str, "enable") ||
       !strcasecmp(str, "enabled") ||
       !strcasecmp(str, "true"))
     {
        if (b) *b = EINA_TRUE;
        return EINA_TRUE;
     }
   else if (!strcmp(str, "0") ||
            !strcasecmp(str, "no") ||
            !strcasecmp(str, "off") ||
            !strcasecmp(str, "disable") ||
            !strcasecmp(str, "disabled") ||
            !strcasecmp(str, "false"))
     {
        if (b) *b = EINA_FALSE;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

#define PARSE_ABORT() do {} while (0)
//#define PARSE_ABORT() abort()

#define PARSE_CHECK(a) do { if (!(a)) { ERR("Parsing failed because '%s' is false at %s:%d", #a, __FUNCTION__, __LINE__); PARSE_ABORT(); goto end; } } while (0)

static Eina_Bool
_color_parse(const char *word, DATA32 *color)
{
   DATA32 value;
   Eina_Bool success = EINA_FALSE;

   PARSE_CHECK(word && *word);

   errno = 0;
   if (*word == '#')
     {
        unsigned char a, r, g, b;
        int slen = strlen(word);
        PARSE_CHECK(evas_common_format_color_parse(word, slen, &r, &g, &b, &a));
        value = ARGB_JOIN(a, r, g, b);
     }
   else
     {
        unsigned int k;
        for (k = 0; k < (sizeof(color_map) / sizeof(color_map[0])); k++)
          {
             if (!strcasecmp(word, color_map[k].name))
               {
                  if (color) *color = color_map[k].value;
                  return EINA_TRUE;
               }
          }
        PARSE_CHECK(!"color name not found");
     }

   if ((value & 0xFF000000) == 0 && (value != 0))
     value |= 0xFF000000;

   if (color) *color = value;
   success = EINA_TRUE;

end:
   return success;
}

/* Buffers */
static Buffer *
_buffer_get(Evas_Filter_Program *pgm, const char *name)
{
   Buffer *buf;
   Evas_Object *source;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pgm, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   EINA_INLIST_FOREACH(pgm->buffers, buf)
     if (!strcmp(buf->name, name))
       return buf;

   // Auto proxies
   if (pgm->proxies)
     {
        source = eina_hash_find(pgm->proxies, name);
        if (!source) return NULL;

        buf = calloc(1, sizeof(Buffer));
        if (!buf) return NULL;

        buf->name = eina_stringshare_add(name);
        buf->proxy = eina_stringshare_add(name);
        buf->alpha = EINA_FALSE;
        pgm->buffers = eina_inlist_append(pgm->buffers, EINA_INLIST_GET(buf));
        return buf;
     }

   return NULL;
}

static Eina_Bool
_buffer_add(Evas_Filter_Program *pgm, const char *name, Eina_Bool alpha,
            const char *src)
{
   Buffer *buf;

   if (_buffer_get(pgm, name))
     {
        ERR("Buffer '%s' already exists", name);
        return EINA_FALSE;
     }

   if (alpha && src)
     {
        ERR("Can not set proxy buffer as alpha!");
        return EINA_FALSE;
     }

   buf = calloc(1, sizeof(Buffer));
   if (!buf) return EINA_FALSE;

   buf->name = eina_stringshare_add(name);
   buf->proxy = eina_stringshare_add(src);
   buf->alpha = alpha;
   pgm->buffers = eina_inlist_append(pgm->buffers, EINA_INLIST_GET(buf));

   return EINA_TRUE;
}

static void
_buffer_del(Buffer *buf)
{
   if (!buf) return;
   eina_stringshare_del(buf->name);
   eina_stringshare_del(buf->proxy);
   free(buf);
}

/* Instruction definitions */

/**
  @page evasfiltersref

  @subsection sec_buffers_cmd Buffer command

  Create a new buffer.

  @verbatim
  name1 = buffer()
  name2 = buffer("alpha")
  name3 = buffer("rgba")
  name4 = buffer({ type = "rgba" })
  name5 = buffer({ src = "partname" })
  @endverbatim

  @param type   Buffer type: @c rgba (default) or @c alpha
  @param src    An optional source. If set, @a type will be @c rgba.
  @return A new buffer. This value must not be saved to a variable.

  This creates a new named buffer, specify its colorspace or source. Possible options:
    @li @c alpha: Create an alpha-only buffer (1 channel, no color)
    @li @c rgba: Create an RGBA buffer (4 channels, full color)
    @li <tt>{src = "partname"}</tt>: Use another <tt>Evas Object</tt> as source for this
      buffer's pixels. The name can either be an Edje part name or the one
      specified in @c evas_obj_text_filter_source_set.

  If no option is given, an RGBA buffer will be created. All buffers have the
  same size, unless they are based on an external source.

  @see evas_obj_text_filter_source_set

  @since 1.10
 */

static Eina_Bool
_buffer_instruction_parse_run(lua_State *L,
                              Evas_Filter_Program *pgm,
                              Evas_Filter_Instruction *instr)
{
   char bufname[32] = {0};
   const char *src, *rgba;
   Eina_Bool ok, alpha = EINA_FALSE;
   int cnt;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pgm, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);

   // FIXME: Buffers are still referred to internally by name.
   // This is pretty bad with the switch to Lua.

   rgba = _instruction_param_gets(instr, "type", NULL);
   src = _instruction_param_gets(instr, "src", NULL);

   alpha = (rgba && !strcasecmp(rgba, "alpha"));

   cnt = (pgm->buffers ? eina_inlist_count(pgm->buffers) : 0) + 1;
   snprintf(bufname, sizeof(bufname), "__buffer%02d", cnt);
   ok = _buffer_add(pgm, bufname, alpha, src);

   if (!ok) return EINA_FALSE;

   lua_pushstring(L, bufname);
   instr->return_count = 1;

   return ok;
}

static Eina_Bool
_buffer_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcmp(instr->name, "buffer"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_BUFFER;
   instr->parse_run = _buffer_instruction_parse_run;
   _instruction_param_seq_add(instr, "type", VT_BUFFER, "rgba");
   _instruction_param_seq_add(instr, "src", VT_BUFFER, NULL);

   return EINA_TRUE;
}

static void
_blend_padding_update(Evas_Filter_Program *pgm, Evas_Filter_Instruction *instr,
                      int *padl, int *padr, int *padt, int *padb)
{
   const char *inbuf, *outbuf;
   Evas_Filter_Fill_Mode fillmode;
   Buffer *in, *out;
   int ox, oy, l = 0, r = 0, t = 0, b = 0;

   ox = _instruction_param_geti(instr, "ox", NULL);
   oy = _instruction_param_geti(instr, "oy", NULL);

   inbuf = _instruction_param_gets(instr, "src", NULL);
   in = _buffer_get(pgm, inbuf);
   EINA_SAFETY_ON_NULL_RETURN(in);

   outbuf = _instruction_param_gets(instr, "dst", NULL);
   out = _buffer_get(pgm, outbuf);
   EINA_SAFETY_ON_NULL_RETURN(out);

   fillmode = _fill_mode_get(instr);
   if (fillmode & (EVAS_FILTER_FILL_MODE_STRETCH_X | EVAS_FILTER_FILL_MODE_REPEAT_X)) ox = 0;
   if (fillmode & (EVAS_FILTER_FILL_MODE_STRETCH_Y | EVAS_FILTER_FILL_MODE_REPEAT_Y)) oy = 0;

   if (ox < 0) l = (-ox) + in->pad.l;
   else r = ox + in->pad.r;

   if (oy < 0) t = (-oy) + in->pad.t;
   else b = oy + in->pad.b;

   if (out->pad.l < l) out->pad.l = l;
   if (out->pad.r < r) out->pad.r = r;
   if (out->pad.t < t) out->pad.t = t;
   if (out->pad.b < b) out->pad.b = b;

   if (padl) *padl = l;
   if (padr) *padr = r;
   if (padt) *padt = t;
   if (padb) *padb = b;
}

/**
  @page evasfiltersref
  @section sec_commands Filter commands
  @page evasfiltersref

  This section will present the various filter instructions, their syntax
  and their effects.
 */

/**
  @page evasfiltersref

  @subsection sec_commands_blend Blend

  Blend a buffer onto another. This is the simplest filter, as it just
  renders one buffer on another, potentially using a color, an
  offset and fill options.

  @verbatim
  blend ({ src = input, dst = output, ox = 0, oy = 0, color = 'white', fillmode = 'none' })
  @endverbatim

  @param src Source buffer to blend.
  @param dst Destination buffer for blending.
  @param ox  X offset. Moves the buffer to the right (ox > 0) or to the left (ox < 0) by N pixels.
  @param oy  Y offset. Moves the buffer to the bottom (oy > 0) or to the top (oy < 0) by N pixels.
  @param color A color to use for alpha to RGBA conversion. See @ref evasfilters_color "colors". <br>
                 If the input is an alpha buffer and the output is RGBA, this will
                 draw the buffer in this color. If both buffers are RGBA, this will
                 have no effect.
  @param fillmode Map the input onto the whole surface of the output by stretching or
                 repeating it. See @ref evasfilter_fillmode "fillmodes".

  If @a src is an alpha buffer and @a dst is an RGBA buffer, then the @a color option should be set.

  @verbinclude filter_blend.lua

  <center>
  @image html filter_blend.png
  </center>

  @since 1.9
 */

static Eina_Bool
_blend_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "blend"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_BLEND;
   instr->pad.update = _blend_padding_update;
   _instruction_param_seq_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_seq_add(instr, "dst", VT_BUFFER, "output");
   _instruction_param_seq_add(instr, "ox", VT_INT, 0);
   _instruction_param_seq_add(instr, "oy", VT_INT, 0);
   _instruction_param_name_add(instr, "color", VT_COLOR, 0xFFFFFFFF);
   _instruction_param_name_add(instr, "fillmode", VT_STRING, "none");

   return EINA_TRUE;
}

static void
_blur_padding_update(Evas_Filter_Program *pgm, Evas_Filter_Instruction *instr,
                     int *padl, int *padr, int *padt, int *padb)
{
   Eina_Bool yset = EINA_FALSE;
   int rx, ry, ox, oy, l, r, t, b, count;
   const char *inbuf, *outbuf, *typestr;
   Evas_Filter_Blur_Type type = EVAS_FILTER_BLUR_DEFAULT;
   Buffer *in, *out;

   rx = _instruction_param_geti(instr, "rx", NULL);
   ry = _instruction_param_geti(instr, "ry", &yset);
   ox = _instruction_param_geti(instr, "ox", NULL);
   oy = _instruction_param_geti(instr, "oy", NULL);
   inbuf = _instruction_param_gets(instr, "src", NULL);
   outbuf = _instruction_param_gets(instr, "dst", NULL);
   count = _instruction_param_geti(instr, "count", NULL);
   typestr = _instruction_param_gets(instr, "type", NULL);

   if (typestr && !strcasecmp(typestr, "box"))
     type = EVAS_FILTER_BLUR_BOX;

   in = _buffer_get(pgm, inbuf);
   out = _buffer_get(pgm, outbuf);
   EINA_SAFETY_ON_NULL_RETURN(in);
   EINA_SAFETY_ON_NULL_RETURN(out);

   if (!yset) ry = rx;
   if (rx < 0) rx = 0;
   if (ry < 0) ry = 0;

   if (type == EVAS_FILTER_BLUR_BOX)
     {
        if (count < 1) count = 1;
        if (count > 6) count = 3;
     }
   else
     count = 1;

   rx *= count;
   ry *= count;

   l = rx + in->pad.l + ((ox < 0) ? (-ox) : 0);
   r = rx + in->pad.r + ((ox > 0) ? ox : 0);
   t = ry + in->pad.t + ((oy < 0) ? (-oy) : 0);
   b = ry + in->pad.b + ((oy > 0) ? oy : 0);

   if (out->pad.l < l) out->pad.l = l;
   if (out->pad.r < r) out->pad.r = r;
   if (out->pad.t < t) out->pad.t = t;
   if (out->pad.b < b) out->pad.b = b;

   if (padl) *padl = l;
   if (padr) *padr = r;
   if (padt) *padt = t;
   if (padb) *padb = b;
}

/**
  @page evasfiltersref

  @subsection sec_commands_blur Blur

  Apply blur effect on a buffer (box or gaussian).

  @verbatim
  blur ({ rx = 3, ry = nil, type = 'default', ox = 0, oy = 0, color = 'white', src = input, dst = output })
  @endverbatim

  @param rx    X radius. Specifies the radius of the blurring kernel (X direction).
  @param ry    Y radius. Specifies the radius of the blurring kernel (Y direction). If -1 is used, then @a ry = @a rx.
  @param type  Blur type to apply. One of @c default, @c box or @c gaussian. See below for details about @c default.
  @param ox    X offset. Moves the buffer to the right (@a ox > 0) or to the left (@a ox < 0) by N pixels.
  @param oy    Y offset. Moves the buffer to the bottom (@a oy > 0) or to the top (@a oy < 0) by N pixels.
  @param color A color to use for alpha to RGBA conversion. See @ref evasfilters_color "colors". <br>
                 If the input is an alpha buffer and the output is RGBA, this will
                 draw the buffer in this color.
  @param src   Source buffer to blur.
  @param dst   Destination buffer for blending.
  @param count Number of times to repeat the blur. Only valid with @c box blur. Valid range is: 1 to 6.

  The blur type @c default is <b>recommended in all situations</b> as it will select the smoothest
  and fastest operation possible depending on the kernel size. Instead of running a real
  gaussian blur, 2 or 3 box blurs may be chained to produce a similar effect at a much
  higher speed. The value @a count can be set to a value from 1 to 6 if blur type @c box
  has been specified.

  The speedups of @c box over @c gaussian are of orders of 4x to more than 20x faster.

  If @a src is an alpha buffer and @a dst is an RGBA buffer, then the color option should be set.

  @a ox and @a oy can be used to move the blurry output by a few pixels, like a drop shadow. Example:
  @verbinclude filter_blur.lua

  <center>
  @image html filter_blur.png
  </center>

  @since 1.9
 */

static Eina_Bool
_blur_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "blur"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_BLUR;
   instr->pad.update = _blur_padding_update;
   _instruction_param_seq_add(instr, "rx", VT_INT, 3);
   _instruction_param_seq_add(instr, "ry", VT_INT, -1);
   _instruction_param_seq_add(instr, "type", VT_STRING, "default");
   _instruction_param_seq_add(instr, "ox", VT_INT, 0);
   _instruction_param_seq_add(instr, "oy", VT_INT, 0);
   _instruction_param_name_add(instr, "color", VT_COLOR, 0xFFFFFFFF);
   _instruction_param_name_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_name_add(instr, "dst", VT_BUFFER, "output");
   _instruction_param_name_add(instr, "count", VT_INT, 0);

   return EINA_TRUE;
}

/**
  @page evasfiltersref

  @subsection sec_commands_bump Bump

  Apply a light effect (ambient light, specular reflection and shadows) based on a bump map.

  This can be used to give a relief effect on the object.

  @verbatim
  bump ({ map, azimuth = 135.0, elevation = 45.0, depth = 8.0, specular = 0.0,
          color = 'white', compensate = false, src = input, dst = output,
          black = 'black', white = 'white', fillmode = 'repeat' })
  @endverbatim

  @param map        An alpha buffer treated like a Z map for the light effect (bump map). Must be specified.
  @param azimuth    The angle between the light vector and the X axis in the XY plane (Z = 0). 135.0 means 45 degrees from the top-left. Counter-clockwise notation.
  @param elevation  The angle between the light vector and the Z axis. 45.0 means 45 degrees to the screen's plane. Ranges from 0 to 90 only.
  @param depth      The depth of the object in an arbitrary unit. More depth means the shadows will be stronger. Default is 8.0.
  @param specular   An arbitrary unit for the specular light effect. Default is 0.0, but a common value would be 40.0.
  @param color      The main color of the object if src is an alpha buffer. This represents the light's normal color. See @ref evasfilters_color "colors".
  @param compensate If set to true, compensate for whitening or darkening on flat surfaces. Default is false but it is recommended if specular light is wanted.
  @param src        Source buffer. This should be an alpha buffer.
  @param dst        Destination buffer. This should be an RGBA buffer (although alpha is supported). Must be of the same size as @a src.
  @param black      The shadows' color. Usually this will be black (@c #000).
  @param white      The specular light's color. Usually this will be white (@c #FFF).
  @param fillmode   This specifies how to handle @a map when its dimensions don't match those of @a src and @a dst. Default is to @c repeat. See @ref evasfilter_fillmode "fillmodes".

  @note As of 2014/02/11, the ALPHA to RGBA support is of much better quality than ALPHA only, but @b very slow. RGBA sources are not supported yet.

  Here is a full example of a very simple bevel effect:
  @verbinclude filter_bump.lua

  <center>
  @image html filter_bump.png
  </center>

  @since 1.9
 */

static Eina_Bool
_bump_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "bump"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_BUMP;
   _instruction_param_seq_add(instr, "map", VT_BUFFER, NULL);
   _instruction_param_seq_add(instr, "azimuth", VT_REAL, 135.0);
   _instruction_param_seq_add(instr, "elevation", VT_REAL, 45.0);
   _instruction_param_seq_add(instr, "depth", VT_REAL, 8.0);
   _instruction_param_seq_add(instr, "specular", VT_REAL, 0.0);
   _instruction_param_name_add(instr, "color", VT_COLOR, 0xFFFFFFFF);
   _instruction_param_name_add(instr, "compensate", VT_BOOL, EINA_FALSE);
   _instruction_param_name_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_name_add(instr, "dst", VT_BUFFER, "output");
   _instruction_param_name_add(instr, "black", VT_COLOR, 0xFF000000);
   _instruction_param_name_add(instr, "white", VT_COLOR, 0xFFFFFFFF);
   _instruction_param_name_add(instr, "fillmode", VT_STRING, "repeat");

   return EINA_TRUE;
}

/**
  @page evasfiltersref

  @subsection sec_commands_curve Curve

  Apply a color curve to a specific channel in a buffer.

  @verbatim
  curve ({ points, interpolation = 'linear', channel = 'rgb', src = input, dst = output })
  @endverbatim

  Modify the colors of a buffer. This applies a color curve y = f(x) to every pixel.

  @param points        The color curve to apply. See below for the syntax.
  @param interpolation How to interpolate between points. One of @c linear (y = ax + b) or @c none (y = Yk).
  @param channel       Target channel for the color modification. One of @c R(ed), @c G(reen), @c B(lue), @c A(lpha), @c RGB and @c RGBA. If @a src is an alpha buffer, this parameter will be ignored.
  @param src           Source buffer.
  @param dst           Destination buffer, must be of same dimensions and color space as @a src.

  The @a points argument contains a list of (X,Y) points in the range 0..255,
  describing a function <tt>f(x) = y</tt> to apply to all pixel values.

  The syntax of this @a points string is <tt>'x1:y1 - x2:y2 - x3:y3 - ... - xn:yn'</tt>
  (remember that all spaces are discarded).
  The points @c xn are in @a increasing order: <tt>x1 < x2 < x3 < ... < xn</tt>,
  and all values @c xn or @c yn are within the range 0..255.

  The identity curve is then described as <tt>'0:0-255:255'</tt>, with linear interpolation:
  @verbatim
  curve ({ points = '0:0 - 255:255', interpolation = linear })
  @endverbatim
  If ignored, y(x = 0) is 0 and y(x = 255) is 255.

  The following example will generate a 4px thick stroke around text letters:
  @verbinclude filter_curve.lua

  <center>
  @image html filter_curve.png
  </center>

  The curve command can be used to alter the output of a blur operation.

  @since 1.9
 */

static Eina_Bool
_curve_instruction_prepare(Evas_Filter_Instruction *instr)
{
   Instruction_Param *param;

   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "curve"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_CURVE;

   // TODO: Allow passing an array of 256 values as points.
   // It could be easily computed from another function in the script.
   _instruction_param_seq_add(instr, "points", VT_STRING, NULL);
   param = EINA_INLIST_CONTAINER_GET(eina_inlist_last(instr->params), Instruction_Param);
   param->allow_any_string = EINA_TRUE;

   _instruction_param_seq_add(instr, "interpolation", VT_STRING, "linear");
   _instruction_param_seq_add(instr, "channel", VT_STRING, "rgb");
   _instruction_param_name_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_name_add(instr, "dst", VT_BUFFER, "output");

   return EINA_TRUE;
}

static void
_displace_padding_update(Evas_Filter_Program *pgm,
                         Evas_Filter_Instruction *instr,
                         int *padl, int *padr, int *padt, int *padb)
{
   int intensity = 0;
   int l, r, t, b;
   const char *inbuf, *outbuf;
   Buffer *in, *out;

   intensity = _instruction_param_geti(instr, "intensity", NULL);
   inbuf = _instruction_param_gets(instr, "src", NULL);
   outbuf = _instruction_param_gets(instr, "dst", NULL);

   in = _buffer_get(pgm, inbuf);
   out = _buffer_get(pgm, outbuf);
   EINA_SAFETY_ON_NULL_RETURN(in);
   EINA_SAFETY_ON_NULL_RETURN(out);

   l = intensity + in->pad.l;
   r = intensity + in->pad.r;
   t = intensity + in->pad.t;
   b = intensity + in->pad.b;

   if (out->pad.l < l) out->pad.l = l;
   if (out->pad.r < r) out->pad.r = r;
   if (out->pad.t < t) out->pad.t = t;
   if (out->pad.b < b) out->pad.b = b;

   if (padl) *padl = l;
   if (padr) *padr = r;
   if (padt) *padt = t;
   if (padb) *padb = b;
}

/**
  @page evasfiltersref

  @subsection sec_commands_displace Displace

  Apply a displacement map on a buffer.

  @verbatim
  displace ({ map, intensity = 10, flags = 0, src = input, dst = output, fillmode = 'repeat' })
  @endverbatim

  @param map       An RGBA buffer containing a displacement map. See below for more details.
  @param intensity Maximum distance for the displacement.
                   This means 0 and 255 will represent a displacement of @c intensity pixels.
  @param flags     One of @c default, @c nearest, @c smooth, @c nearest_stretch or @c smooth_stretch.
                   This defines how pixels should be treated when going out of the @a src image bounds.
                   @c default is equivalent to @c smooth_stretch.
  @param src       Source buffer
  @param dst       Destination buffer. Must be of same color format and size as @a src.
  @param fillmode  Defines how to handle cases where the map has a different size from @a src and @a dst.
                   It should be a combination of @c stretch or @c repeat: @c none is not supported.
                   See @ref evasfilter_fillmode "fillmodes".

  <h3>Displacement map</h3>

  The @a map buffer is an RGBA image containing displacement and alpha values.
  Its size can be different from @c src or @c dst.

  The @b red channel is used for X displacements while the @b green channel is
  used for Y displacements. All subpixel values are in the range 0..255.
  A value of 128 means 0 displacement, lower means displace to the top/left
  and higher than 128 displace to the bottom/right.

  If <tt>signed char</tt> is used instead of <tt>unsigned char</tt> to represent
  these R and G values, then < 0 means displace top/left while > 0 means bottom/right.

  The @c alpha channel is used as an alpha multiplier for blending.

  Considering <tt>I(x, y)</tt> represents the pixel at position (x, y) in the
  image I, then here is how the displacement is applied to @a dst:
  @verbatim
  D = map (x, y)
  dst (x, y) = D.alpha * src (x + (D.red - 128) * intensity / 128, y + (D.green - 128) * intensity / 128) / 255 + (255 - D.alpha) * dst (x, y) / 255
  @endverbatim
  Of course, the real algorithm takes into account interpolation between pixels as well.

  @since 1.9
 */

static Eina_Bool
_displace_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "displace"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_DISPLACE;
   instr->pad.update = _displace_padding_update;
   _instruction_param_seq_add(instr, "map", VT_BUFFER, NULL);
   _instruction_param_seq_add(instr, "intensity", VT_INT, 10);
   _instruction_param_seq_add(instr, "flags", VT_STRING, "default");
   _instruction_param_name_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_name_add(instr, "dst", VT_BUFFER, "output");
   _instruction_param_name_add(instr, "fillmode", VT_STRING, "repeat");

   return EINA_TRUE;
}

/**
  @page evasfiltersref

  @subsection sec_commands_fill Fill

  Fill a buffer with a specific color.
  Not blending, can be used to clear a buffer.

  @verbatim
  fill ({ dst = output, color = 'transparent', l = 0, r = 0, t = 0, b = 0 })
  @endverbatim

  @param dst       Target buffer to fill with @a color.
  @param color     The color used to fill the buffer. All pixels within the fill area will be reset to this value. See @ref evasfilters_color "colors".
  @param l         Left padding: skip @a l pixels from the left border of the buffer
  @param r         Right padding: skip @a r pixels from the right border of the buffer
  @param t         Top padding: skip @a t pixels from the top border of the buffer
  @param b         Bottom padding: skip @a b pixels from the bottom border of the buffer

  This function should generally not be used, except for:
  <ul>
    <li>@a Testing an effect over a specific background color</li>
    <li>Clearing out a buffer with either white or transparent color</li>
  </ul>

  @since 1.9
 */

static Eina_Bool
_fill_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "fill"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_FILL;
   _instruction_param_seq_add(instr, "dst", VT_BUFFER, "output");
   _instruction_param_seq_add(instr, "color", VT_COLOR, 0x0);
   _instruction_param_seq_add(instr, "l", VT_INT, 0);
   _instruction_param_seq_add(instr, "r", VT_INT, 0);
   _instruction_param_seq_add(instr, "t", VT_INT, 0);
   _instruction_param_seq_add(instr, "b", VT_INT, 0);

   return EINA_TRUE;
}

static void
_grow_padding_update(Evas_Filter_Program *pgm, Evas_Filter_Instruction *instr,
                     int *padl, int *padr, int *padt, int *padb)
{
   const char *inbuf, *outbuf;
   Buffer *in, *out;
   int l, r, t, b;
   int radius;

   radius = _instruction_param_geti(instr, "radius", NULL);
   inbuf = _instruction_param_gets(instr, "src", NULL);
   outbuf = _instruction_param_gets(instr, "dst", NULL);

   in = _buffer_get(pgm, inbuf);
   out = _buffer_get(pgm, outbuf);
   EINA_SAFETY_ON_NULL_RETURN(in);
   EINA_SAFETY_ON_NULL_RETURN(out);

   if (radius < 0) radius = 0;

   l = radius + in->pad.l;
   r = radius + in->pad.r;
   t = radius + in->pad.t;
   b = radius + in->pad.b;

   if (padl) *padl = l;
   if (padr) *padr = r;
   if (padt) *padt = t;
   if (padb) *padb = b;

   if (out->pad.l < l) out->pad.l = l;
   if (out->pad.r < r) out->pad.r = r;
   if (out->pad.t < t) out->pad.t = t;
   if (out->pad.b < b) out->pad.b = b;
}

/**
  @page evasfiltersref

  @subsection sec_commands_grow Grow

  Grow or shrink a buffer's contents. This is not a zoom effect.

  @verbatim
  grow ({ radius, smooth = true, src = input, dst = output })
  @endverbatim

  @param radius  The radius of the grow kernel.
                 If a negative value is specified, the contents will shrink rather than grow.
  @param smooth  If @c true, use a smooth transitions between black and white (smooth blur and smoother curve).
  @param src     Source buffer to blur.
  @param dst     Destination buffer for blending. This must be of same size and colorspace as @a src.

  Example:
  @verbinclude filter_grow.lua

  This will first grow the letters in the buffer @c input by a few pixels, and
  then draw this buffer in black in the background.

  <center>
  @image html filter_grow.png
  </center>

  @since 1.9
 */

static Eina_Bool
_grow_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "grow"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_GROW;
   instr->pad.update = _grow_padding_update;
   _instruction_param_seq_add(instr, "radius", VT_INT, 0);
   _instruction_param_name_add(instr, "smooth", VT_BOOL, EINA_TRUE);
   _instruction_param_name_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_name_add(instr, "dst", VT_BUFFER, "output");

   return EINA_TRUE;
}

/**
  @page evasfiltersref

  @subsection sec_commands_mask Mask

  Blend two input buffers into a third (target).

  @verbatim
  mask ({ mask, src = input, dst = output, color = 'white', fillmode = 'repeat' })
  @endverbatim

  @param mask     A mask or texture to blend with the input @a src into the target @a dst.
  @param src      Source buffer. This can also be thought of a mask if @a src is alpha and @a mask is RGBA.
  @param dst      Destination buffer for blending. This must be of same size and colorspace as @a src.
  @param color    A color to use for alpha to RGBA conversion for the blend operations. White means no change.
                  See @ref evasfilters_color "colors". This will have no effect on RGBA sources.
  @param fillmode Defines whether to stretch or repeat the @a mask if its size that of @src.
                  Should be set when masking with external textures. Default is repeat. See @ref evasfilter_fillmode "fillmodes".

  Note that @a src and @a mask are interchangeable, if they have the same dimensions.

  Example:
  @verbinclude filter_mask.lua

  This will create an inner shadow effect.

  <center>
  @image html filter_mask.png
  </center>

  @since 1.9
 */

static Eina_Bool
_mask_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "mask"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_MASK;
   _instruction_param_seq_add(instr, "mask", VT_BUFFER, NULL);
   _instruction_param_seq_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_seq_add(instr, "dst", VT_BUFFER, "output");
   _instruction_param_name_add(instr, "color", VT_COLOR, 0xFFFFFFFF);
   _instruction_param_name_add(instr, "fillmode", VT_STRING, "repeat");

   return EINA_TRUE;
}

static void
_transform_padding_update(Evas_Filter_Program *pgm,
                          Evas_Filter_Instruction *instr,
                          int *padl, int *padr, int *padt, int *padb)
{
   const char *outbuf;
   Buffer *out;
   int ox, oy, l = 0, r = 0, t = 0, b = 0;

   //ox = _instruction_param_geti(instr, "ox", NULL);
   ox = 0;
   oy = _instruction_param_geti(instr, "oy", NULL);

   outbuf = _instruction_param_gets(instr, "dst", NULL);
   out = _buffer_get(pgm, outbuf);
   EINA_SAFETY_ON_NULL_RETURN(out);

   if (ox < 0) l = (-ox) * 2;
   else r = ox * 2;

   if (oy < 0) t = (-oy) * 2;
   else b = oy * 2;

   if (out->pad.l < l) out->pad.l = l;
   if (out->pad.r < r) out->pad.r = r;
   if (out->pad.t < t) out->pad.t = t;
   if (out->pad.b < b) out->pad.b = b;

   if (padl) *padl = l;
   if (padr) *padr = r;
   if (padt) *padt = t;
   if (padb) *padb = b;
}

/**
  @page evasfiltersref

  @subsection sec_commands_transform Transform

  Apply a geometrical transformation to a buffer.

  Right now, only <b>vertical flip</b> is implemented and available.
  This operation does not blend and assumes the destination buffer is empty.

  @verbatim
  transform ({ dst, op = 'vflip', src = input, oy = 0 })
  @endverbatim

  @param dst      Destination buffer. Must be of the same colorspace as @a src. Must be specified.
  @param op       Must be @c 'vflip'. There is no other operation yet.
  @param src      Source buffer to transform.
  @param oy       Y offset.

  Example:
  @verbinclude filter_transform.lua

  This will create a mirrored text effect, for a font of 50px.

  <center>
  @image html filter_transform.png
  </center>

  @note Because of the meaning of @a oy, this effect probably needs to be
        customized for a single font size (FIXME).

  @since 1.9
 */

static Eina_Bool
_transform_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "transform"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_TRANSFORM;
   instr->pad.update = _transform_padding_update;
   _instruction_param_seq_add(instr, "dst", VT_BUFFER, NULL);
   _instruction_param_seq_add(instr, "op", VT_STRING, "vflip");
   _instruction_param_seq_add(instr, "src", VT_BUFFER, "input");
   //_instruction_param_name_add(instr, "ox", VT_INT, 0);
   _instruction_param_name_add(instr, "oy", VT_INT, 0);

   return EINA_TRUE;
}

static void
_padding_set_padding_update(Evas_Filter_Program *pgm,
                            Evas_Filter_Instruction *instr,
                            int *padl, int *padr, int *padt, int *padb)
{
   int l = 0, r = 0, t = 0, b = 0;
   Eina_Bool lset = EINA_FALSE;
   Eina_Bool rset = EINA_FALSE;
   Eina_Bool tset = EINA_FALSE;
   Eina_Bool bset = EINA_FALSE;

   l = _instruction_param_geti(instr, "l", &lset);
   r = _instruction_param_geti(instr, "r", &rset);
   t = _instruction_param_geti(instr, "t", &tset);
   b = _instruction_param_geti(instr, "b", &bset);

   if (!lset && !rset && !bset && !tset)
     DBG("padding_set() called without specifying any of l,r,t,b resets to 0");

   if (l < 0 || r < 0 || t < 0 || b < 0)
     {
        WRN("invalid padding values in padding_set(%d, %d, %d, %d), resets to 0", l, r, t, b);
        l = r = t = b = 0;
     }

   if (!rset) r = l;
   if (!tset) t = r;
   if (!bset) b = t;

   if (padl) *padl = l;
   if (padr) *padr = r;
   if (padt) *padt = t;
   if (padb) *padb = b;
   pgm->padding_set = EINA_TRUE;
}

/**
  @page evasfiltersref

  @subsection sec_commands_padding_set Padding_Set

  Forcily set a specific padding for this filter.

  @verbatim
  padding_set ({ l, r = [l], t = [r], b = [t] })
  @endverbatim

  @param l        Padding on the left side in pixels.
  @param r        Padding on the right side in pixels. If unset, defaults to @a l.
  @param t        Padding on the top in pixels. If unset, defaults to @a r.
  @param b        Padding on the bottom in pixels. If unset, defaults to @a t.

  All values must be >= 0. When filtering 'filled' images, some values may be too high
  and would result in completely hiding the image.

  It is not possible to set only one of those without forcing the others as well.
  A common use case will be when changing a blur size during an animation, or
  when applying a mask that will hide most of the (blurred) text.

  Example (the @c fill command is used for illustration purposes):
  @verbinclude filter_padding.lua

  This will set the left, right, top and bottom paddings to their respective values,
  and some effects may look like they've been "clipped" out.

  <center>
  @image html filter_padding.png
  </center>

  @since 1.10
 */

static Eina_Bool
_padding_set_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "padding_set"), EINA_FALSE);

   instr->type = EVAS_FILTER_MODE_PADDING_SET;
   instr->pad.update = _padding_set_padding_update;
   _instruction_param_seq_add(instr, "l", VT_INT, 0);
   _instruction_param_seq_add(instr, "r", VT_INT, 0);
   _instruction_param_seq_add(instr, "t", VT_INT, 0);
   _instruction_param_seq_add(instr, "b", VT_INT, 0);

   return EINA_TRUE;
}

/* Evas_Filter_Parser entry points */

#undef PARSE_CHECK
#define PARSE_CHECK(a) do { if (!(a)) { ERR("Parsing failed because '%s' is false at %s:%d", #a, __FUNCTION__, __LINE__); PARSE_ABORT(); goto end; } } while (0)

EAPI void
evas_filter_program_del(Evas_Filter_Program *pgm)
{
   Evas_Filter_Instruction *instr;
   Buffer *buf;

   if (!pgm) return;

   EINA_INLIST_FREE(pgm->buffers, buf)
     {
        pgm->buffers = eina_inlist_remove(pgm->buffers, EINA_INLIST_GET(buf));
        _buffer_del(buf);
     }

   EINA_INLIST_FREE(pgm->instructions, instr)
     {
        pgm->instructions = eina_inlist_remove(pgm->instructions, EINA_INLIST_GET(instr));
        _instruction_del(instr);
     }

   eina_stringshare_del(pgm->name);
   free(pgm);
}

static const int this_is_not_a_cat = 42;

static Evas_Filter_Program *
_lua_program_get(lua_State *L)
{
   Evas_Filter_Program *pgm;
   lua_pushlightuserdata(L, (void *) &this_is_not_a_cat);
   lua_gettable(L, LUA_REGISTRYINDEX);
   pgm = lua_touserdata(L, -1);
   lua_pop(L, 1);
   return pgm;
}

static Eina_Bool
_lua_parameter_parse(lua_State *L, Instruction_Param *param, int i)
{
   Eina_Bool ok;

   if (!param) return EINA_FALSE;
   if (param->set)
     {
        ERR("Parameter %s has already been set", param->name);
        luaL_error(L, "Parameter %s has already been set", param->name);
        return 0;
     }

   switch (param->type)
     {
      case VT_BOOL:
        if (lua_type(L, i) == LUA_TSTRING)
          {
             ok = EINA_FALSE;
             const char *str = lua_tostring(L, i);
             Eina_Bool val = _bool_parse(str, &ok);
             if (!ok) goto fail;
             eina_value_set(param->value, val);
          }
        else if (lua_isboolean(L, i) || lua_isnumber(L, i))
          eina_value_set(param->value, lua_toboolean(L, i));
        else goto fail;
        break;
      case VT_INT:
        if (!lua_isnumber(L, i)) goto fail;
        eina_value_set(param->value, lua_tointeger(L, i));
        break;
      case VT_REAL:
        if (!lua_isnumber(L, i)) goto fail;
        eina_value_set(param->value, lua_tonumber(L, i));
        break;
      case VT_STRING:
        if (lua_type(L, i) != LUA_TSTRING) goto fail;
        eina_value_set(param->value, lua_tostring(L, i));
        break;
      case VT_COLOR:
        if (lua_isnumber(L, i))
          {
             DATA32 color = (DATA32) lua_tonumber(L, i);
             int A = A_VAL(&color);
             int R = R_VAL(&color);
             int G = G_VAL(&color);
             int B = B_VAL(&color);
             if (!A && (R || B || G)) A = 0xFF;
             evas_color_argb_premul(A, &R, &G, &B);
             eina_value_set(param->value, ARGB_JOIN(A, R, G, B));
          }
        else if (lua_type(L, i) == LUA_TSTRING)
          {
             DATA32 color;
             ok = _color_parse(lua_tostring(L, i), &color);
             if (!ok) goto fail;
             eina_value_set(param->value, color);
          }
        else goto fail;
        break;
      case VT_BUFFER:
        if (lua_type(L, i) != LUA_TSTRING) goto fail;
        eina_value_set(param->value, lua_tostring(L, i));
        break;
      case VT_NONE:
      default:
        // This should not happen
        CRI("Invalid function declaration");
        goto fail;
     }

   param->set = EINA_TRUE;
   return EINA_TRUE;

fail:
   ERR("Invalid value for parameter %s", param->name);
   luaL_error(L, "Invalid value for parameter %s", param->name);
   return EINA_FALSE;
}

static Eina_Bool
_lua_instruction_run(lua_State *L, Evas_Filter_Instruction *instr)
{
   const unsigned int argc = lua_gettop(L);
   Evas_Filter_Program *pgm = _lua_program_get(L);
   Instruction_Param *param;
   unsigned int i = 0;

   if (!instr) return EINA_FALSE;

   if (eina_inlist_count(instr->params) < argc)
     {
        ERR("Too many arguments passed to the instruction %s", instr->name);
        goto fail;
     }

   if (lua_istable(L, 1))
     {
        Eina_Bool seqmode = EINA_TRUE;

        if (argc > 1)
          {
             ERR("Too many arguments passed to the instruction %s (in table mode)", instr->name);
             goto fail;
          }
        lua_pushnil(L);
        param = EINA_INLIST_CONTAINER_GET(instr->params, Instruction_Param);

        while (lua_next(L, 1))
          {
             if (!lua_isnumber(L, -2) && (lua_type(L, -2) == LUA_TSTRING))
               {
                  const char *name = lua_tostring(L, -2);
                  seqmode = EINA_FALSE;
                  param = _instruction_param_get(instr, name);
                  if (!param)
                    {
                       ERR("Parameter %s does not exist", name);
                       goto fail;
                    }
               }
             else if (!seqmode)
               {
                  ERR("Unnamed parameter can not come after a named parameter");
                  goto fail;
               }
             else if (param && !param->allow_seq)
               {
                  ERR("The parameter %s must be refered to by name", param->name);
                  goto fail;
               }

             if (!_lua_parameter_parse(L, param, -1))
               goto fail;
             lua_pop(L, 1);

             if (seqmode)
               param = EINA_INLIST_GET(param)->next ?
                        _EINA_INLIST_CONTAINER(param, EINA_INLIST_GET(param)->next) :
                        NULL;
          }
     }
   else
     {
        EINA_INLIST_FOREACH(instr->params, param)
          {
             if ((++i) > argc) break;
             if (!_lua_parameter_parse(L, param, i))
               goto fail;
          }
     }

   if (instr->parse_run)
     {
        if (!instr->parse_run(L, pgm, instr))
          {
             ERR("Failed to run instruction '%s'", instr->name);
             return EINA_FALSE;
          }
     }

   return EINA_TRUE;

fail:
   ERR("Invalid parameters for instruction %s", instr->name);
   luaL_error(L, "Invalid parameters for instruction %s", instr->name);
   return EINA_FALSE;
}

static int
_lua_generic_function(lua_State *L, const char *name,
                      Eina_Bool (* prepare) (Evas_Filter_Instruction *))
{
   Evas_Filter_Program *pgm = _lua_program_get(L);
   Evas_Filter_Instruction *instr;
   Eina_Bool ok;

   instr = _instruction_new(name);
   prepare(instr);
   ok = _lua_instruction_run(L, instr);

   if (!ok)
     {
        ERR("Instruction parsing failed");
        _instruction_del(instr);
        lua_error(L);
        return 0;
     }
   else
     {
        pgm->instructions = eina_inlist_append(pgm->instructions, EINA_INLIST_GET(instr));
     }

   return instr->return_count;
}

#define LUA_GENERIC_FUNCTION(name) \
static int \
_lua_##name(lua_State *L) \
{ \
   return _lua_generic_function(L, #name, _##name##_instruction_prepare); \
}

#define PUSH_LUA_FUNCTION(name) \
   lua_pushcfunction(L, _lua_##name); \
   lua_setglobal(L, #name);

LUA_GENERIC_FUNCTION(buffer)
LUA_GENERIC_FUNCTION(blend)
LUA_GENERIC_FUNCTION(blur)
LUA_GENERIC_FUNCTION(bump)
LUA_GENERIC_FUNCTION(curve)
LUA_GENERIC_FUNCTION(displace)
LUA_GENERIC_FUNCTION(fill)
LUA_GENERIC_FUNCTION(grow)
LUA_GENERIC_FUNCTION(mask)
LUA_GENERIC_FUNCTION(padding_set)
LUA_GENERIC_FUNCTION(transform)

static lua_State *
_lua_state_create(Evas_Filter_Program *pgm)
{
   lua_State *L;

   L = luaL_newstate();
   if (!L)
     {
        ERR("Could not create a new Lua state");
        return NULL;
     }

   luaopen_base(L);
   luaopen_table(L);
   luaopen_string(L);
   luaopen_math(L);

   // Store program
   lua_pushlightuserdata(L, (void *) &this_is_not_a_cat);
   lua_pushlightuserdata(L, pgm);
   lua_settable(L, LUA_REGISTRYINDEX);

   // Register functions
   PUSH_LUA_FUNCTION(buffer)
   PUSH_LUA_FUNCTION(blend)
   PUSH_LUA_FUNCTION(blur)
   PUSH_LUA_FUNCTION(bump)
   PUSH_LUA_FUNCTION(curve)
   PUSH_LUA_FUNCTION(displace)
   PUSH_LUA_FUNCTION(fill)
   PUSH_LUA_FUNCTION(grow)
   PUSH_LUA_FUNCTION(mask)
   PUSH_LUA_FUNCTION(padding_set)
   PUSH_LUA_FUNCTION(transform)

   // Register special variables
   for (unsigned k = 0; k < (sizeof(color_map) / sizeof(color_map[0])); k++)
     {
        lua_pushnumber(L, color_map[k].value);
        lua_setglobal(L, color_map[k].name);
     }

   for (unsigned k = 0; k < (sizeof(fill_modes) / sizeof(fill_modes[0])); k++)
     {
        if (strcmp("repeat", fill_modes[k].name))
          {
             lua_pushstring(L, fill_modes[k].name);
             lua_setglobal(L, fill_modes[k].name);
          }
     }

   lua_pushstring(L, "rgba");
   lua_setglobal(L, "rgba");

   lua_pushstring(L, "alpha");
   lua_setglobal(L, "alpha");

   static const struct { Eina_Bool b; const char *name; } booleans[] =
   {
      { EINA_TRUE, "on" },
      { EINA_TRUE, "yes" },
      { EINA_TRUE, "enable" },
      { EINA_TRUE, "enabled" },
      { EINA_FALSE, "off" },
      { EINA_FALSE, "no" },
      { EINA_FALSE, "disable" },
      { EINA_FALSE, "disabled" }
   };

   for (unsigned k = 0; k < (sizeof(booleans) / sizeof(booleans[0])); k++)
     {
        lua_pushnumber(L, booleans[k].b);
        lua_setglobal(L, booleans[k].name);
     }

   // Buffers. Should be input & output only.
   {
      Buffer *buf;

      EINA_INLIST_FOREACH(pgm->buffers, buf)
        {
           lua_pushstring(L, buf->name);
           lua_setglobal(L, buf->name);
        }
   }

   // Register proxies
   if (pgm->proxies)
     {
        Eina_Iterator *it = eina_hash_iterator_key_new(pgm->proxies);
        const char *source;

        EINA_ITERATOR_FOREACH(it, source)
          if (_buffer_get(pgm, source))
            {
               lua_pushstring(L, source);
               lua_setglobal(L, source);
            }

        eina_iterator_free(it);
     }

   return L;
}

#ifdef FILTERS_LEGACY_COMPAT
// This function is here to avoid breaking the ABI too much.
// It should not stay here long, only until all client apps have changed the filters' code to Lua.
static char *
_legacy_strdup(const char *str)
{
   static Eina_Strbuf *dst = NULL;

   if (!dst) dst = eina_strbuf_new();
   for (const char *ptr = str; ptr && *ptr; ptr++)
     {
        if (ptr[0] == '/' && ptr[1] == '/')
          {
             // Comments
             ptr = strchr(ptr, '\n');
             if (!ptr) break;
          }
        else if (ptr[0] == '/' && ptr[1] == '*')
          {
             /* Comments */
             ptr = strstr(ptr + 2, "*/");
             if (!ptr) break;
             ptr++;
          }
        else if (*ptr == '(')
          eina_strbuf_append_char(dst, '{');
        else if (*ptr == ')')
          eina_strbuf_append_char(dst, '}');
        else if (*ptr == '#')
          {
             // Colors: #RGBA becomes "#RGBA"
             ptr++;
             eina_strbuf_append_length(dst, "\"#", 2);
             while (*ptr && *ptr != ',' && *ptr != ')')
               eina_strbuf_append_char(dst, *ptr++);
             eina_strbuf_append_char(dst, '"');
             ptr--;
          }
        else if (!strncasecmp("buffer", ptr, 6))
          {
             // Buffers: "buffer : a (rgba)" into "local a = buffer (rgba)"
             ptr = strchr(ptr, ':');
             if (!ptr) break;
             eina_strbuf_append(dst, "local ");
             for (ptr++; ptr && *ptr; ptr++)
               {
                  if (*ptr != '(')
                    eina_strbuf_append_char(dst, *ptr);
                  else
                    {
                       eina_strbuf_append(dst, " = buffer{");
                       break;
                    }
               }
          }
        else if (!strncasecmp("points", ptr, 6))
          {
             // Color curves: points = 0:0 - 255:255 becomes points = "0:0-255:255"
             ptr = strchr(ptr, '=');
             if (!ptr) break;
             ptr++;
             eina_strbuf_append(dst, "points = \"");
             while (*ptr && *ptr != ',' && *ptr != ')')
               {
                  if (isspace(*ptr))
                    {
                       ptr++;
                       continue;
                    }
                  eina_strbuf_append_char(dst, *ptr++);
               }
             eina_strbuf_append_char(dst, '"');
             ptr--;
          }
        else if (!strncasecmp("curve", ptr, 5))
          {
             // Color curves: curve (0:0 - 255:255, becomes curve { points = "0:0-255:255",
             const char *end = strchr(ptr, ')');
             const char *points = strstr(ptr, "points");
             if (!end || (points > end)) break;
             if (!points)
               {
                  while (*ptr != '(') ptr++;
                  ptr++;
                  eina_strbuf_append(dst, "curve { points = \"");
                  while (*ptr && *ptr != ',' && *ptr != ')')
                    {
                       if (isspace(*ptr))
                         {
                            ptr++;
                            continue;
                         }
                       eina_strbuf_append_char(dst, *ptr++);
                    }
                  eina_strbuf_append_char(dst, '"');
                  ptr--;
               }
             else
               {
                  eina_strbuf_append_length(dst, "curve", 5);
                  ptr += 4;
               }
          }
        else if (!strncasecmp("repeat", ptr, 6))
          {
             // repeat is a Lua keyword, replace all occurences by "repeat_xy"
             if (ptr[-1] != '_' && ptr[6] != '_')
               {
                  eina_strbuf_append(dst, "\"repeat_xy\"");
                  ptr += 5;
               }
             else
               eina_strbuf_append_char(dst, *ptr);
          }
        else
          eina_strbuf_append_char(dst, *ptr);
     }

   return eina_strbuf_string_steal(dst);
}
#endif

/** Parse a style program */

EAPI Eina_Bool
evas_filter_program_parse(Evas_Filter_Program *pgm, const char *str)
{
   lua_State *L;
   Eina_Bool ok;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pgm, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(*str != 0, EINA_FALSE);

   L = _lua_state_create(pgm);
   if (!L) return EINA_FALSE;

   ok = !luaL_loadstring(L, str);

#ifdef FILTERS_LEGACY_COMPAT
   if (!ok)
     {
        char *code = _legacy_strdup(str);
        DBG("Fallback to transformed legacy code:\n%s", code);
        ok = !luaL_loadstring(L, code);
        free(code);
     }
#endif

   if (ok)
     ok = !lua_pcall(L, 0, LUA_MULTRET, 0);
   else
     {
        const char *msg = lua_tostring(L, -1);
        ERR("Lua parsing failed: %s", msg);
     }
   lua_close(L);

   ok &= (pgm->instructions != NULL);
   pgm->valid = ok;
   pgm->padding_calc = EINA_FALSE;

   return ok;
}

/** Evaluate required padding to correctly apply an effect */

EAPI Eina_Bool
evas_filter_program_padding_get(Evas_Filter_Program *pgm,
                                int *l, int *r, int *t, int *b)
{
   Evas_Filter_Instruction *instr;
   int pl = 0, pr = 0, pt = 0, pb = 0;
   int maxl = 0, maxr = 0, maxt = 0, maxb = 0;
   Buffer *buf;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pgm, EINA_FALSE);

   if (pgm->padding_calc || pgm->padding_set)
     {
        if (l) *l = pgm->pad.l;
        if (r) *r = pgm->pad.r;
        if (t) *t = pgm->pad.t;
        if (b) *b = pgm->pad.b;
        return EINA_TRUE;
     }

   // Reset all paddings
   EINA_INLIST_FOREACH(pgm->buffers, buf)
     buf->pad.l = buf->pad.r = buf->pad.t = buf->pad.b = 0;

   // Accumulate paddings
   EINA_INLIST_FOREACH(pgm->instructions, instr)
     {
        if (instr->type == EVAS_FILTER_MODE_PADDING_SET)
          {
             instr->pad.update(pgm, instr, &maxl, &maxr, &maxt, &maxb);
             break;
          }
        else if (instr->pad.update)
          {
             instr->pad.update(pgm, instr, &pl, &pr, &pt, &pb);
             if (pl > maxl) maxl = pl;
             if (pr > maxr) maxr = pr;
             if (pt > maxt) maxt = pt;
             if (pb > maxb) maxb = pb;
          }
     }

   pgm->pad.l = maxl;
   pgm->pad.r = maxr;
   pgm->pad.t = maxt;
   pgm->pad.b = maxb;
   pgm->padding_calc = EINA_TRUE;

   if (l) *l = maxl;
   if (r) *r = maxr;
   if (t) *t = maxt;
   if (b) *b = maxb;

   return EINA_TRUE;
}

/** Create an empty filter program for style parsing */

EAPI Evas_Filter_Program *
evas_filter_program_new(const char *name, Eina_Bool input_alpha)
{
   Evas_Filter_Program *pgm;

   pgm = calloc(1, sizeof(Evas_Filter_Program));
   if (!pgm) return NULL;
   pgm->name = eina_stringshare_add(name);
   _buffer_add(pgm, "input", input_alpha, NULL);
   _buffer_add(pgm, "output", EINA_FALSE, NULL);

   return pgm;
}

/** Bind objects for proxy rendering */
EAPI void
evas_filter_program_source_set_all(Evas_Filter_Program *pgm,
                                   Eina_Hash *proxies)
{
   if (!pgm) return;
   pgm->proxies = proxies;
}

/** Glue with Evas' filters */

#define CA(color) ((color >> 24) & 0xFF)
#define CR(color) ((color >> 16) & 0xFF)
#define CG(color) ((color >> 8) & 0xFF)
#define CB(color) ((color) & 0xFF)

#define SETCOLOR(c) do { ENFN->context_color_get(ENDT, dc, &R, &G, &B, &A); \
   ENFN->context_color_set(ENDT, dc, CR(c), CG(c), CB(c), CA(c)); } while (0)
#define RESETCOLOR() do { ENFN->context_color_set(ENDT, dc, R, G, B, A); } while (0)

#define SETCLIP(l, r, t, b) int _l = 0, _r = 0, _t = 0, _b = 0; \
   do { ENFN->context_clip_get(ENDT, dc, &_l, &_r, &_t, &_b); \
   ENFN->context_clip_set(ENDT, dc, l, r, t, b); } while (0)
#define RESETCLIP() do { ENFN->context_clip_set(ENDT, dc, _l, _r, _t, _b); } while (0)

static Evas_Filter_Fill_Mode
_fill_mode_get(Evas_Filter_Instruction *instr)
{
   const char *fill;
   unsigned k;

   if (!instr) return EVAS_FILTER_FILL_MODE_NONE;
   fill = _instruction_param_gets(instr, "fillmode", NULL);
   if (!fill) return EVAS_FILTER_FILL_MODE_NONE;

   for (k = 0; k < sizeof(fill_modes) / sizeof(fill_modes[0]); k++)
     {
        if (!strcasecmp(fill_modes[k].name, fill))
          return fill_modes[k].value;
     }

   return EVAS_FILTER_FILL_MODE_NONE;
}

static int
_instr2cmd_blend(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                 Evas_Filter_Instruction *instr, void *dc)
{
   Eina_Bool isset = EINA_FALSE;
   const char *src, *dst;
   DATA32 color;
   Buffer *in, *out;
   Evas_Filter_Fill_Mode fillmode;
   int cmdid, ox, oy, A, R, G, B;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   ox = _instruction_param_geti(instr, "ox", NULL);
   oy = _instruction_param_geti(instr, "oy", NULL);
   color = _instruction_param_getc(instr, "color", &isset);
   fillmode = _fill_mode_get(instr);
   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, -1);

   if (isset) SETCOLOR(color);
   cmdid = evas_filter_command_blend_add(ctx, dc, in->cid, out->cid, ox, oy,
                                         fillmode);
   if (isset) RESETCOLOR();
   if (cmdid < 0) return cmdid;

   return cmdid;
}

static int
_instr2cmd_blur(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                Evas_Filter_Instruction *instr, void *dc)
{
   Eina_Bool colorset = EINA_FALSE, yset = EINA_FALSE, cntset = EINA_FALSE;
   Evas_Filter_Blur_Type type = EVAS_FILTER_BLUR_DEFAULT;
   const char *src, *dst, *typestr;
   DATA32 color;
   Buffer *in, *out;
   int cmdid, ox, oy, rx, ry, A, R, G, B, count;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   ox = _instruction_param_geti(instr, "ox", NULL);
   oy = _instruction_param_geti(instr, "oy", NULL);
   rx = _instruction_param_geti(instr, "rx", NULL);
   ry = _instruction_param_geti(instr, "ry", &yset);
   color = _instruction_param_getc(instr, "color", &colorset);
   typestr = _instruction_param_gets(instr, "type", NULL);
   count = _instruction_param_geti(instr, "count", &cntset);
   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, -1);

   if (typestr)
     {
        if (!strcasecmp(typestr, "gaussian"))
          type = EVAS_FILTER_BLUR_GAUSSIAN;
        else if (!strcasecmp(typestr, "box"))
          type = EVAS_FILTER_BLUR_BOX;
        else if (!strcasecmp(typestr, "default"))
          type = EVAS_FILTER_BLUR_DEFAULT;
        else
          ERR("Unknown blur type '%s'. Using default blur.", typestr);
     }

   if (type == EVAS_FILTER_BLUR_BOX)
     {
        if (count < 1) count = 1;
        if (count > 6)
          {
             WRN("Box blur count should be below 6, defaults to 3.");
             count = 3;
          }
     }
   else
     {
        if (cntset) WRN("Blur count can only be used with BOX blur.");
        count = 1;
     }

   if (!yset) ry = rx;
   if (colorset) SETCOLOR(color);
   cmdid = evas_filter_command_blur_add(ctx, dc, in->cid, out->cid, type,
                                        rx, ry, ox, oy, count);
   if (colorset) RESETCOLOR();

   return cmdid;
}

static int
_instr2cmd_bump(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                Evas_Filter_Instruction *instr, void *dc)
{
   Evas_Filter_Bump_Flags flags = EVAS_FILTER_BUMP_NORMAL;
   Evas_Filter_Fill_Mode fillmode;
   const char *src, *dst, *map;
   DATA32 color, black, white;
   Buffer *in, *out, *bump;
   double azimuth, elevation, depth, specular;
   int cmdid, compensate;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   map = _instruction_param_gets(instr, "map", NULL);
   color = _instruction_param_getc(instr, "color", NULL);
   white = _instruction_param_getc(instr, "white", NULL);
   black = _instruction_param_getc(instr, "black", NULL);
   azimuth = _instruction_param_getd(instr, "azimuth", NULL);
   elevation = _instruction_param_getd(instr, "elevation", NULL);
   depth = _instruction_param_getd(instr, "depth", NULL);
   specular = _instruction_param_getd(instr, "specular", NULL);
   compensate = _instruction_param_geti(instr, "compensate", NULL);
   fillmode = _fill_mode_get(instr);
   if (compensate) flags |= EVAS_FILTER_BUMP_COMPENSATE;

   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   bump = _buffer_get(pgm, map);
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(bump, -1);

   cmdid = evas_filter_command_bump_map_add(ctx, dc, in->cid, bump->cid, out->cid,
                                            azimuth, elevation, depth, specular,
                                            black, color, white, flags,
                                            fillmode);

   return cmdid;
}

static int
_instr2cmd_displace(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                    Evas_Filter_Instruction *instr, void *dc)
{
   Evas_Filter_Fill_Mode fillmode;
   Evas_Filter_Displacement_Flags flags =
         EVAS_FILTER_DISPLACE_STRETCH | EVAS_FILTER_DISPLACE_LINEAR;
   const char *src, *dst, *map, *flagsstr;
   Buffer *in, *out, *mask;
   int cmdid, intensity;
   Eina_Bool isset = EINA_FALSE;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   map = _instruction_param_gets(instr, "map", NULL);
   intensity = _instruction_param_geti(instr, "intensity", NULL);
   flagsstr = _instruction_param_gets(instr, "flags", &isset);
   fillmode = _fill_mode_get(instr);

   if (!flagsstr) flagsstr = "default";
   if (!strcasecmp(flagsstr, "nearest"))
     flags = EVAS_FILTER_DISPLACE_NEAREST;
   else if (!strcasecmp(flagsstr, "smooth"))
     flags = EVAS_FILTER_DISPLACE_LINEAR;
   else if (!strcasecmp(flagsstr, "nearest_stretch"))
     flags = EVAS_FILTER_DISPLACE_NEAREST | EVAS_FILTER_DISPLACE_STRETCH;
   else if (!strcasecmp(flagsstr, "default") || !strcasecmp(flagsstr, "smooth_stretch"))
     flags = EVAS_FILTER_DISPLACE_STRETCH | EVAS_FILTER_DISPLACE_LINEAR;
   else if (isset)
     WRN("Invalid flags '%s' in displace operation. Using default instead", flagsstr);

   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   mask = _buffer_get(pgm, map);
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(mask, -1);

   cmdid = evas_filter_command_displacement_map_add(ctx, dc, in->cid, out->cid,
                                                    mask->cid, flags, intensity,
                                                    fillmode);

   return cmdid;
}

static int
_instr2cmd_fill(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                Evas_Filter_Instruction *instr, void *dc)
{
   const char *bufname;
   Buffer *buf;
   int R, G, B, A, l, r, t, b;
   Evas_Filter_Command *cmd;
   Eina_Inlist *il;
   DATA32 color;
   int cmdid;

   bufname = _instruction_param_gets(instr, "dst", NULL);
   color = _instruction_param_getc(instr, "color", NULL);
   l = _instruction_param_geti(instr, "l", NULL);
   r = _instruction_param_geti(instr, "r", NULL);
   t = _instruction_param_geti(instr, "t", NULL);
   b = _instruction_param_geti(instr, "b", NULL);

   buf = _buffer_get(pgm, bufname);
   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, -1);

   SETCOLOR(color);
   cmdid = evas_filter_command_fill_add(ctx, dc, buf->cid);
   RESETCOLOR();

   if (cmdid < 0) return -1;
   il = eina_inlist_last(ctx->commands);
   if (!il) return -1;

   cmd = EINA_INLIST_CONTAINER_GET(il, Evas_Filter_Command);
   cmd->draw.clip.l = l;
   cmd->draw.clip.r = r;
   cmd->draw.clip.t = t;
   cmd->draw.clip.b = b;
   cmd->draw.clip_mode_lrtb = EINA_TRUE;

   return cmdid;
}

static int
_instr2cmd_grow(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                Evas_Filter_Instruction *instr, void *dc)
{
   Evas_Filter_Command *cmd;
   const char *src, *dst;
   Buffer *in, *out;
   Eina_Bool smooth;
   int cmdid, radius;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   radius = _instruction_param_geti(instr, "radius", NULL);
   smooth = _instruction_param_geti(instr, "smooth", NULL);

   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, -1);

   cmdid = evas_filter_command_grow_add(ctx, dc, in->cid, out->cid,
                                        radius, smooth);

   cmd = _evas_filter_command_get(ctx, cmdid);
   if (cmd) cmd->draw.need_temp_buffer = EINA_TRUE;

   return cmdid;
}

static int
_instr2cmd_mask(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                Evas_Filter_Instruction *instr, void *dc)
{
   Evas_Filter_Fill_Mode fillmode;
   const char *src, *dst, *msk;
   Buffer *in, *out, *mask;
   DATA32 color;
   int R, G, B, A, cmdid;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   msk = _instruction_param_gets(instr, "mask", NULL);
   color = _instruction_param_getc(instr, "color", NULL);
   fillmode = _fill_mode_get(instr);

   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   mask = _buffer_get(pgm, msk);
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(mask, -1);

   SETCOLOR(color);
   cmdid = evas_filter_command_mask_add(ctx, dc, in->cid, mask->cid, out->cid, fillmode);
   RESETCOLOR();
   if (cmdid < 0) return cmdid;

   if (!in->alpha && !mask->alpha && !out->alpha)
     {
        Evas_Filter_Command *cmd;

        cmd = _evas_filter_command_get(ctx, cmdid);
        cmd->draw.need_temp_buffer = EINA_TRUE;
     }

   return cmdid;
}

static int
_instr2cmd_curve(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                 Evas_Filter_Instruction *instr, void *dc)
{
   Evas_Filter_Interpolation_Mode mode = EVAS_FILTER_INTERPOLATION_MODE_LINEAR;
   Evas_Filter_Channel channel = EVAS_FILTER_CHANNEL_RGB;
   const char *src, *dst, *points_str, *interpolation, *channel_name;
   DATA8 values[256] = {0}, points[512];
   int cmdid, point_count = 0;
   char *token, *copy = NULL;
   Buffer *in, *out;
   Eina_Bool parse_ok = EINA_FALSE;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   points_str = _instruction_param_gets(instr, "points", NULL);
   interpolation = _instruction_param_gets(instr, "interpolation", NULL);
   channel_name = _instruction_param_gets(instr, "channel", NULL);

   if (channel_name)
     {
        if (tolower(*channel_name) == 'r')
          {
             if (!strcasecmp(channel_name, "rgb"))
               channel = EVAS_FILTER_CHANNEL_RGB;
             else
               channel = EVAS_FILTER_CHANNEL_RED;
          }
        else if (tolower(*channel_name) == 'g')
          channel = EVAS_FILTER_CHANNEL_GREEN;
        else if (tolower(*channel_name) == 'b')
          channel = EVAS_FILTER_CHANNEL_BLUE;
        else if (tolower(*channel_name) == 'a')
          channel = EVAS_FILTER_CHANNEL_ALPHA;
     }

   if (interpolation && !strcasecmp(interpolation, "none"))
     mode = EVAS_FILTER_INTERPOLATION_MODE_NONE;

   if (!points_str) goto interpolated;
   copy = strdup(points_str);
   token = strtok(copy, "-");
   if (!token) goto interpolated;

   while (token)
     {
        int x, y, r, maxx = 0;
        r = sscanf(token, "%u:%u", &x, &y);
        if (r != 2) goto interpolated;
        if (x < maxx || x >= 256) goto interpolated;
        points[point_count * 2 + 0] = x;
        points[point_count * 2 + 1] = y;
        point_count++;
        token = strtok(NULL, "-");
     }

   parse_ok = evas_filter_interpolate(values, points, point_count, mode);

interpolated:
   free(copy);
   if (!parse_ok)
     {
        int x;
        ERR("Failed to parse the interpolation chain");
        for (x = 0; x < 256; x++)
          values[x] = x;
     }

   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, -1);

   cmdid = evas_filter_command_curve_add(ctx, dc, in->cid, out->cid, values, channel);

   return cmdid;
}

static int
_instr2cmd_transform(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                     Evas_Filter_Instruction *instr, void *dc)
{
   Evas_Filter_Transform_Flags flags;
   const char *src, *dst, *op;
   Buffer *in, *out;
   int ox = 0, oy;

   op = _instruction_param_gets(instr, "op", NULL);
   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   // ox = _instruction_param_geti(instr, "ox", NULL);
   oy = _instruction_param_geti(instr, "oy", NULL);

   if (!strcasecmp(op, "vflip"))
     flags = EVAS_FILTER_TRANSFORM_VFLIP;
   else
     {
        ERR("Invalid transform '%s'", op);
        return -1;
     }

   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(out, -1);

   return evas_filter_command_transform_add(ctx, dc, in->cid, out->cid, flags, ox, oy);
}

static int
_command_from_instruction(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                          Evas_Filter_Instruction *instr, void *dc)
{
   int (* instr2cmd) (Evas_Filter_Context *, Evas_Filter_Program *,
                      Evas_Filter_Instruction *, void *);

   switch (instr->type)
     {
      case EVAS_FILTER_MODE_BLEND:
        instr2cmd = _instr2cmd_blend;
        break;
      case EVAS_FILTER_MODE_BLUR:
        instr2cmd = _instr2cmd_blur;
        break;
      case EVAS_FILTER_MODE_BUMP:
        instr2cmd = _instr2cmd_bump;
        break;
      case EVAS_FILTER_MODE_DISPLACE:
        instr2cmd = _instr2cmd_displace;
        break;
      case EVAS_FILTER_MODE_FILL:
        instr2cmd = _instr2cmd_fill;
        break;
      case EVAS_FILTER_MODE_GROW:
        instr2cmd = _instr2cmd_grow;
        break;
      case EVAS_FILTER_MODE_MASK:
        instr2cmd = _instr2cmd_mask;
        break;
      case EVAS_FILTER_MODE_CURVE:
        instr2cmd = _instr2cmd_curve;
        break;
      case EVAS_FILTER_MODE_TRANSFORM:
        instr2cmd = _instr2cmd_transform;
        break;
      case EVAS_FILTER_MODE_PADDING_SET:
      case EVAS_FILTER_MODE_BUFFER:
        return EINA_TRUE;
      default:
        CRI("Invalid instruction type: %d", instr->type);
        return -1;
     }

   return instr2cmd(ctx, pgm, instr, dc);
}

#ifdef FILTERS_DEBUG
static void
_instruction_dump(Evas_Filter_Instruction *instr)
{
   Eina_Strbuf *str;
   const char *comma = "";
   Instruction_Param *param;

   if (!instr) return;

   str = eina_strbuf_new();
   eina_strbuf_append(str, instr->name);
   eina_strbuf_append(str, "({ ");
   EINA_INLIST_FOREACH(instr->params, param)
     {
        int i;
        DATA32 c;
        const char *s;
        double d;

        switch (param->type)
          {
           case VT_BOOL:
           case VT_INT:
             eina_value_get(param->value, &i);
             eina_strbuf_append_printf(str, "%s%s = %d", comma, param->name, i);
             break;
           case VT_COLOR:
             eina_value_get(param->value, &c);
             eina_strbuf_append_printf(str, "%s%s = 0x%08x", comma, param->name, c);
             break;
           case VT_REAL:
             eina_value_get(param->value, &d);
             eina_strbuf_append_printf(str, "%s%s = %f", comma, param->name, d);
             break;
           case VT_STRING:
           case VT_BUFFER:
             eina_value_get(param->value, &s);
             if (s) eina_strbuf_append_printf(str, "%s%s = \"%s\"", comma, param->name, s);
             else eina_strbuf_append_printf(str, "%s%s = nil", comma, param->name);
             break;
           case VT_NONE:
           default:
             eina_strbuf_append_printf(str, "%s%s = <INVALID>", comma, param->name);
             break;
          }

        comma = ", ";
     }
   eina_strbuf_append(str, "})");
   DBG("%s", eina_strbuf_string_get(str));
   eina_strbuf_free(str);
}
#else
# define _instruction_dump(a) do {} while(0)
#endif

Eina_Bool
evas_filter_context_program_use(Evas_Filter_Context *ctx,
                                Evas_Filter_Program *pgm)
{
   Buffer *buf;
   Evas_Filter_Instruction *instr;
   Eina_Bool success = EINA_FALSE;
   void *dc = NULL;
   int cmdid;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pgm, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pgm->valid, EINA_FALSE);

   DBG("Using program '%s' for context %p", pgm->name, ctx);

   // Create empty context with all required buffers
   evas_filter_context_clear(ctx);
   EINA_INLIST_FOREACH(pgm->buffers, buf)
     {
        buf->cid = evas_filter_buffer_empty_new(ctx, buf->alpha);
        if (buf->proxy)
          {
             Evas_Filter_Proxy_Binding *pb;
             Evas_Filter_Buffer *fb;

             pb = eina_hash_find(pgm->proxies, buf->proxy);
             if (!pb) continue;

             fb = _filter_buffer_get(ctx, buf->cid);
             fb->proxy = pb->eo_proxy;
             fb->source = pb->eo_source;
             fb->source_name = eina_stringshare_ref(pb->name);
             fb->ctx->has_proxies = EINA_TRUE;
          }
     }

   // Compute and save padding info
   evas_filter_program_padding_get(pgm, &ctx->padl, &ctx->padr, &ctx->padt, &ctx->padb);

   dc = ENFN->context_new(ENDT);
   ENFN->context_color_set(ENDT, dc, 255, 255, 255, 255);

   // Apply all commands
   EINA_INLIST_FOREACH(pgm->instructions, instr)
     {
        _instruction_dump(instr);
        cmdid = _command_from_instruction(ctx, pgm, instr, dc);
        if (cmdid <= 0)
          goto end;
     }

   success = EINA_TRUE;

end:
   if (!success) evas_filter_context_clear(ctx);
   if (dc) ENFN->context_free(ENDT, dc);
   return success;
}
