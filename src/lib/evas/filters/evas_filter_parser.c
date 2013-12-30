#include "evas_filter_private.h"
#include <stdarg.h>

#define EVAS_FILTER_MODE_BUFFER (EVAS_FILTER_MODE_LAST+1)
#define EVAS_FILTER_MODE_GROW   (EVAS_FILTER_MODE_LAST+2)

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
} Instruction_Param;

struct _Evas_Filter_Instruction
{
   EINA_INLIST;
   Eina_Stringshare *name;
   int /* Evas_Filter_Mode */ type;
   Eina_Inlist /* Instruction_Param */ *params;
   struct
   {
      void (* update) (Evas_Filter_Program *, Evas_Filter_Instruction *, int *, int *, int *, int *);
   } pad;
   Eina_Bool valid : 1;
};

struct _Evas_Filter_Program
{
   Eina_Stringshare *name; // Optional for now
   Eina_Hash /* const char * : Evas_Object */ *proxies;
   Eina_Inlist /* Evas_Filter_Instruction */ *instructions;
   Eina_Inlist /* Buffer */ *buffers;
   Eina_Bool valid : 1;
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

/* Parsing format: func ( arg , arg2 , argname=val1, argname2 = val2 ) */

#define CHARS_ALPHABET "abcdefghijklmnopqrstuvwxyzABCDEFGHJIKLMNOPQRSTUVWXYZ"
#define CHARS_NUMS "0123456789"
#define CHARS_DELIMS "=-(),;#."
//static const char *allowed_chars = DELIMS;
static const char *allowed_delim = CHARS_DELIMS;

static char *
_whitespace_ignore_strdup(const char *str)
{
   Eina_Bool inword = EINA_FALSE, wasword = EINA_FALSE;
   char *dst, *ptr, *next;
   int len;

   if (!str) return NULL;
   len = strlen(str);
   dst = calloc(len + 1, 1);

   // TODO: Support quoted strings ("string" or 'string')

   ptr = dst;
   for (; *str; str++)
     {
        if (isspace(*str))
          {
             wasword = inword;
             inword = EINA_FALSE;
          }
        else if (isalpha(*str) || isdigit(*str))
          {
             if (wasword)
               {
                  ERR("Invalid space found in program code");
                  goto invalid;
               }
             inword = EINA_TRUE;
             *ptr++ = *str;
          }
        else if (*str == '/')
          {
             if (str[1] == '*')
               {
                  next = strstr(str + 2, "*/");
                  if (!next)
                    {
                       ERR("Unterminated comment section, \"*/\" was not found");
                       goto invalid;
                    }
                  str = next + 1;
               }
             else if (str[1] == '/')
               {
                  next = strchr(str + 2, '\n');
                  if (!next) break;
                  str = next;
               }
             else
               {
                  ERR("Character '/' not followed by '/' or '*' is invalid");
                  goto invalid;
               }
          }
        else
          {
             if (!strchr(allowed_delim, *str))
               {
                  ERR("Character '%1.1s' is not allowed", str);
                  goto invalid;
               }
             wasword = inword = EINA_FALSE;
             *ptr++ = *str;
          }
     }

   *ptr = 0;
   return dst;

invalid:
   free(dst);
   return NULL;
}

// "key", alphanumeric chars only, starting with a letter
static Eina_Bool
_is_valid_string(const char *str)
{
   if (!str)
     return EINA_FALSE;
   if (!isalpha(*str++))
     return EINA_FALSE;
   for (; *str; str++)
     if (!isalpha(*str) && !isdigit(*str))
       return EINA_FALSE;
   return EINA_TRUE;
}

// valid number:
static Eina_Bool
_is_valid_number(const char *str)
{
   Eina_Bool dot = EINA_FALSE;
   if (!str || !*str) return EINA_FALSE;
   for (; *str; str++)
     {
        if (!isdigit(*str))
          {
             if (dot) return EINA_FALSE;
             if (*str == '.') dot = EINA_TRUE;
          }
     }
   return EINA_TRUE;
}

// FIXME/TODO: Add support for strings with "" AND/OR ''

// "key=val"
static Eina_Bool
_is_valid_keyval(const char *str)
{
   char *equal;
   Eina_Bool ok = EINA_TRUE;

   if (!str) return EINA_FALSE;
   equal = strchr(str, '=');
   if (!equal) return EINA_FALSE;
   *equal = 0;
   if (!_is_valid_string(str))
     ok = EINA_FALSE;
   else if (!_is_valid_string(equal + 1))
     {
        if (!_is_valid_number(equal + 1))
          ok = EINA_FALSE;
     }
   *equal = '=';
   return ok;
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
   unsigned long value;
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

   if (color) *color = (DATA32) value;
   success = EINA_TRUE;

end:
   return success;
}

static Eina_Bool
_value_parse(Instruction_Param *param, const char *value)
{
   Eina_Bool b;
   DATA32 color;
   double d;
   int i;

   switch (param->type)
     {
      case VT_BOOL:
        PARSE_CHECK(_bool_parse(value, &b));
        eina_value_set(param->value, b ? 1 : 0);
        return EINA_TRUE;
      case VT_INT:
        PARSE_CHECK(sscanf(value, "%d", &i) == 1);
        eina_value_set(param->value, i);
        return EINA_TRUE;
      case VT_REAL:
        PARSE_CHECK(sscanf(value, "%lf", &d) == 1);
        eina_value_set(param->value, d);
        return EINA_TRUE;
      case VT_STRING:
      case VT_BUFFER:
        PARSE_CHECK(_is_valid_string(value));
        eina_value_set(param->value, value);
        return EINA_TRUE;
      case VT_COLOR:
        PARSE_CHECK(_color_parse(value, &color));
        eina_value_set(param->value, color);
        return EINA_TRUE;
      case VT_NONE:
      default:
        PARSE_CHECK(!"invalid value type");
     }

end:
   return EINA_FALSE;
}

static Eina_Bool
_instruction_parse(Evas_Filter_Instruction *instr, const char *string)
{
   Instruction_Param *param = NULL;
   char *str = NULL, *token = NULL, *next = NULL, *optval = NULL, *optname = NULL;
   Eina_Bool last = EINA_FALSE, namedargs = EINA_FALSE, success = EINA_FALSE;
   int seqorder = 0;

   instr->valid = EINA_FALSE;
   PARSE_CHECK(string);

   EINA_INLIST_FOREACH(instr->params, param)
     param->set = EINA_FALSE;

   // Copy and remove whitespaces now
   str = _whitespace_ignore_strdup(string);
   PARSE_CHECK(str);
   token = str;

   // Check instruction matches function name
   next = strchr(token, '(');
   PARSE_CHECK(next);
   *next++ = 0;
   PARSE_CHECK(!strcasecmp(token, instr->name));

   // Read arguments
   while (((token = strsep(&next, ",")) != NULL) && (!last))
     {
        Eina_Bool found = EINA_FALSE;

        // Last argument
        if (!next)
          {
             // ',' was not found, find ')'
             next = strchr(token, ')');
             PARSE_CHECK(next);
             last = EINA_TRUE;
             *next++ = 0;
          }

        // Named arguments
        if (_is_valid_keyval(token))
          {
             namedargs = EINA_TRUE;

             optval = strchr(token, '=');
             PARSE_CHECK(optval); // assert
             *optval++ = 0;

             optname = token;
             EINA_INLIST_FOREACH(instr->params, param)
               {
                  if (!strcasecmp(param->name, optname))
                    {
                       found = EINA_TRUE;
                       PARSE_CHECK(!param->set);
                       PARSE_CHECK(_value_parse(param, optval));
                       param->set = EINA_TRUE;
                    }
               }
             PARSE_CHECK(found);
          }
        // Sequential arguments
        else if (!namedargs &&
                 (_is_valid_string(token) || _is_valid_number(token)))
          {
             int order = 0;

             // Go to the nth argument
             EINA_INLIST_FOREACH(instr->params, param)
               {
                  if (order < seqorder)
                    order++;
                  else
                    {
                       found = EINA_TRUE;
                       break;
                    }
               }

             PARSE_CHECK(found);
             PARSE_CHECK(param->allow_seq);
             PARSE_CHECK(_value_parse(param, token));
             param->set = EINA_TRUE;
             seqorder++;
          }
        else if (!last)
          PARSE_CHECK(!"invalid argument list");
     }
   PARSE_CHECK(last);
   success = EINA_TRUE;

end:
   free(str);
   instr->valid = success;
   return success;
}

/* Buffers */
static Buffer *
_buffer_get(Evas_Filter_Program *pgm, const char *name)
{
   Buffer *buf;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pgm, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   EINA_INLIST_FOREACH(pgm->buffers, buf)
     if (!strcasecmp(buf->name, name))
       return buf;

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

static Eina_Bool
_buffer_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "buffer"), EINA_FALSE);

   /*
   * buffer [name=]NAME (alpha=BOOL) (src=OBJECT)
   * Alpha is not compatible with src, as proxy rendering implies RGBA
   */

   instr->type = EVAS_FILTER_MODE_BUFFER;
   _instruction_param_seq_add(instr, "name", VT_STRING, NULL);
   _instruction_param_name_add(instr, "alpha", VT_BOOL, EINA_FALSE);
   _instruction_param_name_add(instr, "src", VT_STRING, NULL);

   return EINA_TRUE;
}

static Eina_Bool
_blend_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "blend"), EINA_FALSE);

   /*
   * blend [src=BUFFER] [dst=BUFFER] [ox=INT] [oy=INT] (color=COLOR)
   */

   instr->type = EVAS_FILTER_MODE_BLEND;
   _instruction_param_seq_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_seq_add(instr, "dst", VT_BUFFER, "output");
   _instruction_param_seq_add(instr, "ox", VT_INT, 0);
   _instruction_param_seq_add(instr, "oy", VT_INT, 0);
   _instruction_param_name_add(instr, "color", VT_COLOR, 0xFFFFFFFF);

   return EINA_TRUE;
}

static void
_blur_padding_update(Evas_Filter_Program *pgm, Evas_Filter_Instruction *instr,
                     int *padl, int *padr, int *padt, int *padb)
{
   Eina_Bool yset;
   int rx, ry, l, r, t, b;
   const char *typestr, *inbuf, *outbuf;
   Buffer *in, *out;

   // TODO/FIXME: Add ox, oy support
   rx = _instruction_param_geti(instr, "rx", NULL);
   ry = _instruction_param_geti(instr, "ry", &yset);
   typestr = _instruction_param_gets(instr, "type", NULL);
   inbuf = _instruction_param_gets(instr, "src", NULL);
   outbuf = _instruction_param_gets(instr, "dst", NULL);

   in = _buffer_get(pgm, inbuf);
   out = _buffer_get(pgm, outbuf);
   EINA_SAFETY_ON_NULL_RETURN(in);
   EINA_SAFETY_ON_NULL_RETURN(out);

   if (typestr && !strcasecmp(typestr, "motion"))
     {
        CRI("Motion blur not implemented yet!");
        /*
        instr->pad.l = (rx < 0) ? (-rx) : 0;
        instr->pad.r = (rx > 0) ? (rx) : 0;
        instr->pad.t = (ry < 0) ? (-ry) : 0;
        instr->pad.b = (ry > 0) ? (ry) : 0;
        */
     }
   else
     {
        if (!yset) ry = rx;
        if (rx < 0) rx = 0;
        if (ry < 0) ry = 0;

        l = rx + in->pad.l;
        r = rx + in->pad.r;
        t = ry + in->pad.t;
        b = ry + in->pad.b;

        if (out->pad.l < l) out->pad.l = l;
        if (out->pad.r < r) out->pad.r = r;
        if (out->pad.t < t) out->pad.t = t;
        if (out->pad.b < b) out->pad.b = b;

        if (padl) *padl = rx;
        if (padr) *padr = rx;
        if (padt) *padt = ry;
        if (padb) *padb = ry;
     }
}

static Eina_Bool
_blur_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "blur"), EINA_FALSE);

   /*
   * blur [rx=]REAL [ry=REAL] [type=STRING] [ox=INT] [oy=INT] \
   *      (color=COLOR) (src=BUFFER) (dst=BUFFER)
   */

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

   return EINA_TRUE;
}

static Eina_Bool
_bump_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "bump"), EINA_FALSE);

   /*
   * bump [map=]ABUFFER [azimuth=REAL] [elevation=REAL] [depth=REAL] \
   *      [specular-factor=REAL] (color=COLOR) (compensate=BOOL) \
   *      (src=BUFFER) (dst=BUFFER) (black=COLOR) (white=COLOR);
   */

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

   return EINA_TRUE;
}

static Eina_Bool
_curve_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "curve"), EINA_FALSE);

   /*
   * curve TODO
   * This one is a bit trickier: need interpolation functions to describe
   * the curve.
   */

   //instr->type = EVAS_FILTER_MODE_CURVE;
   CRI("Not implemented yet");
   return EINA_FALSE;
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

static Eina_Bool
_displace_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "displace"), EINA_FALSE);

   /*
   * displace [map=]BUFFER [intensity=]INT [flags=]STRING \
   *          [src=BUFFER] [dst=BUFFER]
   *
   * flags can be: (FIXME TBD)
   *  alpha
   *  RG/redgreen
   *  XY
   */

   instr->type = EVAS_FILTER_MODE_DISPLACE;
   instr->pad.update = _displace_padding_update;
   _instruction_param_seq_add(instr, "map", VT_BUFFER, NULL);
   _instruction_param_seq_add(instr, "intensity", VT_INT, 10);
   _instruction_param_seq_add(instr, "flags", VT_INT, 0x0); // FIXME
   _instruction_param_name_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_name_add(instr, "dst", VT_BUFFER, "output");

   return EINA_TRUE;
}

static Eina_Bool
_fill_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "fill"), EINA_FALSE);

   /*
   * fill [dst=]BUFFER [color=COLOR]
   * Works with both Alpha and RGBA.
   */

   instr->type = EVAS_FILTER_MODE_BUFFER;
   _instruction_param_seq_add(instr, "dst", VT_BUFFER, NULL);
   _instruction_param_seq_add(instr, "color", VT_COLOR, 0x0);

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

static Eina_Bool
_grow_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "grow"), EINA_FALSE);

   /*
   * grow [radius=]INT (smooth=BOOL) (src=BUFFER) (dst=BUFFER)
   */

   instr->type = EVAS_FILTER_MODE_GROW;
   instr->pad.update = _grow_padding_update;
   _instruction_param_seq_add(instr, "radius", VT_INT, 0);
   _instruction_param_name_add(instr, "smooth", VT_BOOL, EINA_TRUE);
   _instruction_param_name_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_name_add(instr, "dst", VT_BUFFER, "output");

   return EINA_TRUE;
}

static Eina_Bool
_mask_instruction_prepare(Evas_Filter_Instruction *instr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(instr->name, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(!strcasecmp(instr->name, "mask"), EINA_FALSE);

   /*
   * mask [mask=]BUFFER [input=BUFFER] [output=BUFFER] [color=COLOR]
   */

   instr->type = EVAS_FILTER_MODE_MASK;
   _instruction_param_seq_add(instr, "mask", VT_BUFFER, NULL);
   _instruction_param_seq_add(instr, "src", VT_BUFFER, "input");
   _instruction_param_seq_add(instr, "dst", VT_BUFFER, "output");
   _instruction_param_name_add(instr, "color", VT_COLOR, 0xFFFFFFFF);

   return EINA_TRUE;
}

static Evas_Filter_Instruction *
_instruction_create(const char *name)
{
   Evas_Filter_Instruction *instr;
   Eina_Bool (* prepare) (Evas_Filter_Instruction *) = NULL;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(name && *name, EINA_FALSE);

   if (!strcasecmp(name, "buffer"))
     prepare = _buffer_instruction_prepare;
   else if (!strcasecmp(name, "blend"))
     prepare = _blend_instruction_prepare;
   else if (!strcasecmp(name, "blur"))
     prepare = _blur_instruction_prepare;
   else if (!strcasecmp(name, "bump"))
     prepare = _bump_instruction_prepare;
   else if (!strcasecmp(name, "curve"))
     prepare = _curve_instruction_prepare;
   else if (!strcasecmp(name, "displace"))
     prepare = _displace_instruction_prepare;
   else if (!strcasecmp(name, "fill"))
     prepare = _fill_instruction_prepare;
   else if (!strcasecmp(name, "grow"))
     prepare = _grow_instruction_prepare;
   else if (!strcasecmp(name, "mask"))
     prepare = _mask_instruction_prepare;

   if (!prepare)
     {
        ERR("Invalid instruction name '%s'", name);
        return NULL;
     }

   instr = _instruction_new(name);
   if (!instr) return NULL;

   prepare(instr);
   return instr;
}

/* Evas_Filter_Parser entry points */

#undef PARSE_CHECK
#define PARSE_CHECK(a) do { if (!(a)) { ERR("Parsing failed because '%s' is false at %s:%d", #a, __FUNCTION__, __LINE__); PARSE_ABORT(); goto end; } } while (0)

void
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
   eina_hash_free(pgm->proxies);
   free(pgm);
}

static Eina_Bool
_instruction_buffer_parse(Evas_Filter_Program *pgm,
                          Evas_Filter_Instruction *instr)
{
   Instruction_Param *param;
   Eina_Bool success = EINA_FALSE;
   const char *bufname = NULL, *src = NULL;
   int found = 0;
   int alpha = -1;

   EINA_INLIST_FOREACH(instr->params, param)
     {
        if (!bufname && !strcasecmp(param->name, "name"))
          {
             PARSE_CHECK(eina_value_get(param->value, &bufname));
             found++;
          }
        else if ((alpha == -1) && !strcasecmp(param->name, "alpha"))
          {
             PARSE_CHECK(eina_value_get(param->value, &alpha));
             found++;
          }
        else if (param->set && !strcasecmp(param->name, "src"))
          {
             PARSE_CHECK(eina_value_get(param->value, &src));
             found++;
          }
     }

   PARSE_CHECK(found >= 2);
   PARSE_CHECK(_buffer_add(pgm, bufname, (alpha != 0), src));
   success = EINA_TRUE;

end:
   return success;
}

/** Parse a style program */

Eina_Bool
evas_filter_program_parse(Evas_Filter_Program *pgm, const char *str)
{
   Evas_Filter_Instruction *instr = NULL;
   Instruction_Param *param;
   Eina_Bool success = EINA_FALSE, ok;
   char *token, *next, *code, *instrname, *options;
   int count = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pgm, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(*str != 0, EINA_FALSE);

   code = _whitespace_ignore_strdup(str);
   EINA_SAFETY_ON_NULL_RETURN_VAL(code, EINA_FALSE);

   // FIXME: Comments and strings will be broken by strsep if they contain ';'

   next = code;
   while ((token = strsep(&next, ";")) != NULL)
     {
        if (!next)
          {
             // Semicolon is mandatory.
             DBG("End of processing");
             PARSE_CHECK(!*token);
             break;
          }

        // Empty command
        if (next == token + 1) continue;

        // Parse "instrname(options)"
        options = token;
        instrname = strsep(&options, "(");
        PARSE_CHECK(options);

        instr = _instruction_create(instrname);
        PARSE_CHECK(instr);

        options[-1] = '(';
        ok = _instruction_parse(instr, token);
        PARSE_CHECK(ok);

        if (!strcasecmp(instr->name, "buffer"))
          PARSE_CHECK(_instruction_buffer_parse(pgm, instr));
        else
          {
             // Check buffers validity
             EINA_INLIST_FOREACH(instr->params, param)
               {
                  const char *bufname = NULL;

                  if (param->type != VT_BUFFER) continue;
                  PARSE_CHECK(eina_value_get(param->value, &bufname));
                  if (!_buffer_get(pgm, bufname))
                    {
                       ERR("Buffer '%s' does not exist!", bufname);
                       goto end;
                    }
               }

             // Add to the queue
             pgm->instructions = eina_inlist_append(pgm->instructions, EINA_INLIST_GET(instr));
             instr = NULL;
          }

        count++;
     }
   success = EINA_TRUE;

   DBG("Program successfully compiled with %d instruction(s)", count);

end:
   if (!success)
     {
        ERR("Failed to parse program");
        _instruction_del(instr);
     }
   free(code);

   pgm->valid = success;
   return success;
}

/** Evaluate required padding to correctly apply an effect */

Eina_Bool
evas_filter_program_padding_get(Evas_Filter_Program *pgm,
                                int *l, int *r, int *t, int *b)
{
   Evas_Filter_Instruction *instr;
   int pl = 0, pr = 0, pt = 0, pb = 0;
   int maxl = 0, maxr = 0, maxt = 0, maxb = 0;
   Buffer *buf;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pgm, EINA_FALSE);

   // Reset all paddings
   EINA_INLIST_FOREACH(pgm->buffers, buf)
     buf->pad.l = buf->pad.r = buf->pad.t = buf->pad.b = 0;

   // Accumulate paddings
   EINA_INLIST_FOREACH(pgm->instructions, instr)
     if (instr->pad.update)
       {
          instr->pad.update(pgm, instr, &pl, &pr, &pt, &pb);
          if (pl > maxl) maxl = pl;
          if (pr > maxr) maxr = pr;
          if (pt > maxt) maxt = pt;
          if (pb > maxb) maxb = pb;
       }

   if (l) *l = maxl;
   if (r) *r = maxr;
   if (t) *t = maxt;
   if (b) *b = maxb;

   return EINA_TRUE;
}

/** Create an empty filter program for style parsing */

Evas_Filter_Program *
evas_filter_program_new(const char *name)
{
   Evas_Filter_Program *pgm;

   pgm = calloc(1, sizeof(Evas_Filter_Program));
   if (!pgm) return NULL;
   pgm->name = eina_stringshare_add(name);
   pgm->proxies = eina_hash_string_small_new(EINA_FREE_CB(evas_object_unref));
   _buffer_add(pgm, "input", EINA_TRUE, NULL);
   _buffer_add(pgm, "output", EINA_FALSE, NULL);

   return pgm;
}

/** Bind an object for proxy rendering */

void
evas_filter_program_proxy_source_bind(Evas_Filter_Program *pgm,
                                      const char *name, Evas_Object *object)
{
   Evas_Object *old;

   old = eina_hash_find(pgm->proxies, name);
   if (old) eina_hash_del(pgm->proxies, name, old);

   evas_object_ref(object);
   eina_hash_add(pgm->proxies, name, object);
}

/** Get object used for proxy rendering */

Evas_Object *
evas_filter_program_proxy_source_get(Evas_Filter_Program *pgm, const char *name)
{
   return (Evas_Object *) eina_hash_find(pgm->proxies, name);
}

/** Glue with Evas' filters */

#define CA(color) ((color >> 24) & 0xFF)
#define CR(color) ((color >> 16) & 0xFF)
#define CG(color) ((color >> 8) & 0xFF)
#define CB(color) ((color) & 0xFF)

#define SETCOLOR(c) do { ENFN->context_color_get(ENDT, dc, &R, &G, &B, &A); \
   ENFN->context_color_set(ENDT, dc, CR(c), CG(c), CB(c), CA(c)); } while (0)
#define RESETCOLOR() do { ENFN->context_color_set(ENDT, dc, R, G, B, A); } while (0)
int A, R, G, B;

static int
_instr2cmd_blend(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                 Evas_Filter_Instruction *instr, void *dc)
{
   Eina_Bool isset = EINA_FALSE;
   const char *src, *dst;
   DATA32 color;
   Buffer *in, *out;
   int cmdid, ox, oy;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   ox = _instruction_param_geti(instr, "ox", NULL);
   oy = _instruction_param_geti(instr, "oy", NULL);
   color = _instruction_param_getc(instr, "color", &isset);
   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);

   if (isset) SETCOLOR(color);
   cmdid = evas_filter_command_blend_add(ctx, dc, in->cid, out->cid, ox, oy);
   if (isset) RESETCOLOR();

   return cmdid;
}

static int
_instr2cmd_blur(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                Evas_Filter_Instruction *instr, void *dc)
{
   Eina_Bool isset = EINA_FALSE, yset = EINA_FALSE;
   Evas_Filter_Blur_Type type = EVAS_FILTER_BLUR_DEFAULT;
   const char *src, *dst, *typestr;
   DATA32 color;
   Buffer *in, *out;
   int cmdid, ox, oy, rx, ry;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   ox = _instruction_param_geti(instr, "ox", NULL);
   oy = _instruction_param_geti(instr, "oy", NULL);
   rx = _instruction_param_geti(instr, "rx", NULL);
   ry = _instruction_param_geti(instr, "ry", &yset);
   color = _instruction_param_getc(instr, "color", &isset);
   typestr = _instruction_param_gets(instr, "type", NULL);
   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);

   if (typestr)
     {
        if (!strcasecmp(typestr, "gaussian"))
          type = EVAS_FILTER_BLUR_GAUSSIAN;
        else if (!strcasecmp(typestr, "box"))
          type = EVAS_FILTER_BLUR_BOX;
        else if (!strcasecmp(typestr, "motion"))
          type = EVAS_FILTER_BLUR_MOTION;
        else if (!strcasecmp(typestr, "default"))
          type = EVAS_FILTER_BLUR_DEFAULT;
        else
          ERR("Unknown blur type '%s'. Using default blur.", typestr);
     }

   if (!yset) ry = rx;
   if (isset) SETCOLOR(color);
   cmdid = evas_filter_command_blur_add(ctx, dc, in->cid, out->cid, type,
                                        rx, ry, ox, oy);
   if (isset) RESETCOLOR();

   return cmdid;
}

static int
_instr2cmd_bump(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                Evas_Filter_Instruction *instr, void *dc)
{
   Evas_Filter_Bump_Flags flags = EVAS_FILTER_BUMP_NORMAL;
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
   if (compensate) flags |= EVAS_FILTER_BUMP_COMPENSATE;

   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   bump = _buffer_get(pgm, map);

   cmdid = evas_filter_command_bump_map_add(ctx, dc, in->cid, bump->cid, out->cid,
                                            azimuth, elevation, depth, specular,
                                            black, color, white, flags);

   return cmdid;
}

static int
_instr2cmd_displace(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                    Evas_Filter_Instruction *instr, void *dc)
{
   //Evas_Filter_Displacement_Flags flags = EVAS_FILTER_DISPLACE_RG;
   const char *src, *dst, *map;
   Buffer *in, *out, *mask;
   int cmdid, intensity, flags;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   map = _instruction_param_gets(instr, "map", NULL);
   intensity = _instruction_param_geti(instr, "intensity", NULL);
   flags = _instruction_param_geti(instr, "flags", NULL);

   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   mask = _buffer_get(pgm, map);

   cmdid = evas_filter_command_displacement_map_add(ctx, dc, in->cid, out->cid,
                                                    mask->cid, flags, intensity);

   return cmdid;
}

static int
_instr2cmd_fill(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                Evas_Filter_Instruction *instr, void *dc)
{
   const char *bufname;
   Buffer *buf;
   int R, G, B, A;
   DATA32 color;
   int cmdid;

   bufname = _instruction_param_gets(instr, "dst", NULL);
   color = _instruction_param_getc(instr, "color", NULL);
   // TODO/FIXME: Add clip info

   buf = _buffer_get(pgm, bufname);

   ENFN->context_color_get(ENDT, dc, &R, &G, &B, &A);
   ENFN->context_color_set(ENDT, dc, R_VAL(&color), G_VAL(&color), B_VAL(&color), A_VAL(&color));
   cmdid = evas_filter_command_fill_add(ctx, dc, buf->cid);
   ENFN->context_color_set(ENDT, dc, R, G, B, A);

   return cmdid;
}

static int
_instr2cmd_grow(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                Evas_Filter_Instruction *instr, void *dc)
{
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

   cmdid = evas_filter_command_grow_add(ctx, dc, in->cid, out->cid,
                                        radius, smooth);

   return cmdid;
}

static int
_instr2cmd_mask(Evas_Filter_Context *ctx, Evas_Filter_Program *pgm,
                Evas_Filter_Instruction *instr, void *dc)
{
   const char *src, *dst, *msk;
   Buffer *in, *out, *mask;
   DATA32 color;
   int R, G, B, A;
   int cmdid;

   src = _instruction_param_gets(instr, "src", NULL);
   dst = _instruction_param_gets(instr, "dst", NULL);
   msk = _instruction_param_gets(instr, "mask", NULL);
   color = _instruction_param_getc(instr, "color", NULL);

   in = _buffer_get(pgm, src);
   out = _buffer_get(pgm, dst);
   mask = _buffer_get(pgm, msk);

   ENFN->context_color_get(ENDT, dc, &R, &G, &B, &A);
   ENFN->context_color_set(ENDT, dc, R_VAL(&color), G_VAL(&color), B_VAL(&color), A_VAL(&color));
   cmdid = evas_filter_command_mask_add(ctx, dc, in->cid, mask->cid, out->cid);
   ENFN->context_color_set(ENDT, dc, R, G, B, A);

   return cmdid;
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
#if 0
      case EVAS_FILTER_MODE_CURVE:
        instr2cmd = _instr2cmd_curve;
        break;
#endif
      case EVAS_FILTER_MODE_CURVE:
        CRI("Not implemented yet");
        return -1;
      case EVAS_FILTER_MODE_BUFFER:
      default:
        CRI("Invalid instruction type: %d", instr->type);
        return -1;
     }

   return instr2cmd(ctx, pgm, instr, dc);
}

Eina_Bool
evas_filter_context_program_use(Evas_Filter_Context *ctx, Evas_Object *eo_obj,
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
             Eo *eo_source = evas_filter_program_proxy_source_get(pgm, buf->proxy);
             evas_filter_context_proxy_bind(ctx, eo_obj, eo_source, buf->cid);
          }
     }

   dc = ENFN->context_new(ENDT);
   ENFN->context_color_set(ENDT, dc, 255, 255, 255, 255);

   // Apply all commands
   EINA_INLIST_FOREACH(pgm->instructions, instr)
     {
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
