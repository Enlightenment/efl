/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_private.h"

/* FIXME:
 *
 * this is a rewrite of textblock - for now api calsl are just textblock2
 * instead of textblock
 *
 */

/* LEAK: lines!!!! lines allocated in _layout_line_new() */

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

/* private magic number for textblock objects */
static const char o_type[] = "textblock";

/* private struct for textblock object internal data */
typedef struct _Evas_Object_Textblock             Evas_Object_Textblock;
typedef struct _Evas_Object_Style_Tag             Evas_Object_Style_Tag;
typedef struct _Evas_Object_Textblock_Node        Evas_Object_Textblock_Node;
typedef struct _Evas_Object_Textblock_Line        Evas_Object_Textblock_Line;
typedef struct _Evas_Object_Textblock_Item        Evas_Object_Textblock_Item;
typedef struct _Evas_Object_Textblock_Format_Item Evas_Object_Textblock_Format_Item;
typedef struct _Evas_Object_Textblock_Format      Evas_Object_Textblock_Format;

/* the current state of the formatting */

#define  NODE_TEXT   0
#define  NODE_FORMAT 1

#define STYLE_PLAIN 0
#define STYLE_SHADOW 1
#define STYLE_OUTLINE 2
#define STYLE_GLOW 3
#define STYLE_OUTLINE_SHADOW 4
#define STYLE_FAR_SHADOW 5
#define STYLE_OUTLINE_SOFT_SHADOW 6
#define STYLE_SOFT_SHADOW 7
#define STYLE_FAR_SOFT_SHADOW 8

struct _Evas_Object_Style_Tag
{  Evas_Object_List _list_data;
   char *tag;
   char *replace;
};

struct _Evas_Object_Textblock_Node
{  Evas_Object_List _list_data;
   int   type;
   char *text;
   int   len, alloc;
};

struct _Evas_Object_Textblock_Line
{  Evas_Object_List _list_data;
   Evas_Object_Textblock_Item        *items;
   Evas_Object_Textblock_Format_Item *format_items;
   int                                x, y, w, h;
   int                                baseline;
   int                                line_no;
};

struct _Evas_Object_Textblock_Item
{  Evas_Object_List _list_data;
   unsigned char                 type;
   char                         *text;
   int                           x, w, h;
   int                           inset, baseline;
   Evas_Object_Textblock_Format *format;
   Evas_Object_Textblock_Node   *source_node;
   int                           source_pos;
};

struct _Evas_Object_Textblock_Format_Item
{  Evas_Object_List _list_data;
   char                         *item;
   Evas_Object_Textblock_Node   *source_node;
   int                           x, w;
};

struct _Evas_Object_Textblock_Format
{
   int                  ref;
   double               halign;
   double               valign;
   struct {
      char             *name;
      char             *source;
      char             *fallbacks;
      int               size;
      void             *font;
   } font;
   struct {
      struct {
	 unsigned char  r, g, b, a;
      } normal, underline, underline2, outline, shadow, glow, glow2, backing,
	strikethrough;
   } color;
   struct {
      int               l, r;
   } margin;
   int                  tabstops;
   unsigned char        style;
   unsigned char        wrap_word : 1;
   unsigned char        wrap_char : 1;
   unsigned char        underline : 1;
   unsigned char        underline2 : 1;
   unsigned char        strikethrough : 1;
   unsigned char        backing : 1;
};

struct _Evas_Textblock_Style
{
   char                  *style_text;
   char                  *default_tag;
   Evas_Object_Style_Tag *tags;
   Evas_List             *objects;
   unsigned char          delete_me : 1;
};

struct _Evas_Textblock_Cursor
{
   Evas_Object                *obj;
   int                         pos;
   Evas_Object_Textblock_Node *node;
};

struct _Evas_Object_Textblock
{
   DATA32                       magic;
   Evas_Textblock_Style        *style;
   Evas_Textblock_Cursor       *cursor;
   Evas_List                   *cursors;
   Evas_Object_Textblock_Node  *nodes;
   Evas_Object_Textblock_Line  *lines;
   int                          last_w;
   struct {
      int                       l, r, t, b;
   } style_pad;
   char                        *markup_text;
   char                         changed : 1;
   void                        *engine_data;
   struct {
      int                       w, h;
      unsigned char             valid : 1;
   } formatted, native;
   unsigned char                redraw : 1;
};

/* private methods for textblock objects */
static void evas_object_textblock_init(Evas_Object *obj);
static void *evas_object_textblock_new(void);
static void evas_object_textblock_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_textblock_free(Evas_Object *obj);
static void evas_object_textblock_render_pre(Evas_Object *obj);
static void evas_object_textblock_render_post(Evas_Object *obj);

static int evas_object_textblock_is_opaque(Evas_Object *obj);
static int evas_object_textblock_was_opaque(Evas_Object *obj);

static void evas_object_textblock_coords_recalc(Evas_Object *obj);

static Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_textblock_free,
     evas_object_textblock_render,
     evas_object_textblock_render_pre,
     evas_object_textblock_render_post,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_textblock_is_opaque,
     evas_object_textblock_was_opaque,
     NULL,
     NULL,
     evas_object_textblock_coords_recalc
};

/* the actual api call to add a textblock */

#define TB_HEAD() \
   Evas_Object_Textblock *o; \
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ); \
   return; \
   MAGIC_CHECK_END(); \
   o = (Evas_Object_Textblock *)(obj->object_data); \
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK); \
   return; \
   MAGIC_CHECK_END();

#define TB_HEAD_RETURN(x) \
   Evas_Object_Textblock *o; \
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ); \
   return (x); \
   MAGIC_CHECK_END(); \
   o = (Evas_Object_Textblock *)(obj->object_data); \
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK); \
   return (x); \
   MAGIC_CHECK_END();



/* styles */
static void
_style_clear(Evas_Textblock_Style *ts)
{
   if (ts->style_text) free(ts->style_text);
   if (ts->default_tag) free(ts->default_tag);
   while (ts->tags)
     {
	Evas_Object_Style_Tag *tag;
	
	tag = (Evas_Object_Style_Tag *)ts->tags;
	ts->tags = evas_object_list_remove(ts->tags, tag);
	free(tag->tag);
	free(tag->replace);
	free(tag);
     }
   ts->style_text = NULL;
   ts->default_tag = NULL;
   ts->tags = NULL;
}

/* setting a textblock via markup */
static char *
_style_match_replace(Evas_Textblock_Style *ts, char *s)
{
   Evas_Object_List *l;
   
   for (l = (Evas_Object_List *)ts->tags; l; l = l->next)
     {
	Evas_Object_Style_Tag *tag;
	
	tag = (Evas_Object_Style_Tag *)l;
	if (!strcmp(tag->replace, s)) return tag->tag;
     }
   return NULL;
}

static char *
_style_match_tag(Evas_Textblock_Style *ts, char *s)
{
   Evas_Object_List *l;

   for (l = (Evas_Object_List *)ts->tags; l; l = l->next)
     {
	Evas_Object_Style_Tag *tag;
	
	tag = (Evas_Object_Style_Tag *)l;
	if (!strcmp(tag->tag, s)) return tag->replace;
     }
   return NULL;
}

static char *
_strbuf_append(char *s, char *s2, int *len, int *alloc)
{
   int l2;
   int tlen;
   
   if (!s2) return s;
   l2 = strlen(s2);
   tlen = *len + l2;
   if (tlen > *alloc)
     {
	char *ts;
	int talloc;
	  
	talloc = ((tlen + 31) >> 5) << 5;
	ts = realloc(s, talloc + 1);
	if (!ts) return s;
	s = ts;
	*alloc = talloc;
     }
   strcpy(s + *len, s2);
   *len = tlen;
   return s;
}

static char *
_strbuf_append_n(char *s, char *s2, int n, int *len, int *alloc)
{
   int l2;
   int tlen;
   
   if (!s2) return s;
   l2 = 0;
   if (n < 1) return s;
   else
     {
	char *p;
	for (p = s2; (l2 < n) && (*p != 0); p++, l2++);
     }
   tlen = *len + l2;
   if (tlen > *alloc)
     {
	char *ts;
	int talloc;
	  
	talloc = ((tlen + 31) >> 5) << 5;
	ts = realloc(s, talloc + 1);
	if (!ts) return s;
	s = ts;
	*alloc = talloc;
     }
   strncpy(s + *len, s2, l2);
   *len = tlen;
   s[tlen] = 0;
   return s;
}

static char *
_strbuf_insert(char *s, char *s2, int pos, int *len, int *alloc)
{
   int l2;
   int tlen;
   char *tbuf;
   
   if (!s2) return s;
   else if (pos < 0) pos = 0;
   else if (pos > *len) pos = *len;
   l2 = strlen(s2);
   tlen = *len + l2;
   if (tlen > *alloc)
     {
	char *ts;
	int talloc;
	  
	talloc = ((tlen + 31) >> 5) << 5;
	ts = realloc(s, talloc + 1);
	if (!ts) return s;
	s = ts;
	*alloc = talloc;
     }
   tbuf = malloc(*len - pos);   
   if (tbuf)
     {
	strncpy(tbuf, s + pos, *len - pos);
	strncpy(s + pos, s2, l2);   
	strncpy(s + pos + l2, tbuf, *len - pos);
	free(tbuf);
     }
   *len = tlen;
   s[tlen] = 0;
   return s;
}

static char *
_strbuf_remove(char *s, int p, int p2, int *len, int *alloc)
{
   int l2;
   int tlen;
   char *tbuf;
   
   if ((p == 0) && (p2 ==  *len))
     {
	free(s);
	*len = 0;
	*alloc = 0;
	return NULL;
     }
   tbuf = malloc(*len - p2 + 1);
   strcpy(tbuf, s + p2);
   strcpy(s + p, tbuf);
   free(tbuf);
   tlen = *len - (p2 - p);
   if (tlen < ((*alloc >> 5) << 15))
     {
	char *ts;
	int talloc;
	  
	talloc = ((tlen + 31) >> 5) << 5;
	ts = realloc(s, talloc + 1);
	if (!ts) return s;
	s = ts;
	*alloc = talloc;
     }
   *len = tlen;
   return s;
}

static void
_nodes_clear(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   while (o->nodes)
     {
	Evas_Object_Textblock_Node *n;
	
	n = (Evas_Object_Textblock_Node *)o->nodes;
	o->nodes = evas_object_list_remove(o->nodes, n);
	if (n->text) free(n->text);
	free(n);
     }
}

static void
_format_free(Evas_Object *obj, Evas_Object_Textblock_Format *fmt)
{
   fmt->ref--;
   if (fmt->ref > 0) return;
   if (fmt->font.name) free(fmt->font.name);
   if (fmt->font.fallbacks) free(fmt->font.fallbacks);
   if (fmt->font.source) free(fmt->font.source);
   evas_font_free(obj->layer->evas, fmt->font.font);
   free(fmt);
}

static void
_line_free(Evas_Object *obj, Evas_Object_Textblock_Line *ln)
{
   while (ln->items)
     {
	Evas_Object_Textblock_Item *it;
	
	it = (Evas_Object_Textblock_Item *)ln->items;
	ln->items = evas_object_list_remove(ln->items, ln->items);
	if (it->text) free(it->text);
	_format_free(obj, it->format);
	free(it);
     }
   while (ln->format_items)
     {
	Evas_Object_Textblock_Format_Item *fi;
	
	fi = (Evas_Object_Textblock_Format_Item *)ln->format_items;
	ln->format_items = evas_object_list_remove(ln->format_items, ln->format_items);
	if (fi->item) free(fi->item);
	free(fi);
     }
   free(ln);
}

static void
_lines_clear(Evas_Object *obj, Evas_Object_Textblock_Line *lines)
{
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   while (lines)
     {
	Evas_Object_Textblock_Line *ln;
	
	ln = (Evas_Object_Textblock_Line *)lines;
	lines = evas_object_list_remove(lines, ln);
	_line_free(obj, ln);
     }
}

/* table of html escapes (that i can find) this shoudl be ordered with the
 * most common first as it's a linear search to match - no hash for this
 */
static const char *_escapes[] = 
{
/* most common escaped stuff */
     "&lt;", "<",
     "&gt;", ">",
     "&amp;", "&",
     "&nbsp;", " ", /* NOTE: we will allow nbsp's to break as we map early - maybe map to ascii 0x01 and then make the rendring code think 0x01 -> 0x20 */
     "&quot;", "\"",
     /* all the rest */
     "&copy;", "©",
     "&reg;", "®",
     "&Ntilde;", "Ñ",
     "&ntilde;", "ñ",
     "&Ccedil;", "Ç",
     "&ccedil;", "ç",
     "&szlig;", "ß",
     "&THORN;", "Þ",
     "&thorn;", "þ",
     "&ETH;", "Ð",
     "&eth;", "ð",
     "&acute;", "´",
     "&cedil;", "¸",
     "&deg;", "°",
     "&uml;", "¨",
     "&cent;", "¢",
     "&pound;", "£",
     "&curren;", "¤",
     "&yen;", "¥",
     "&sect;", "§",
     "&para;", "¶",
     "&laquo;", "«",
     "&raquo;", "»",
     "&iexcl;", "¡",
     "&iquest;", "¿",
     "&brvbar;", "¦",
     "&ordf;", "ª",
     "&ordm;", "º",
     "&micro;", "µ",
     "&macr;", "¯",
     "&oplus;", "⊕",
     "&int;", "∫",
     "&sum;", "∑",
     "&prod;", "∏",
     "&perp;", "⊥",
     "&or;", "∨",
     "&and;", "∧",
     "&equiv;", "≡",
     "&ne;", "≠",
     "&forall;", "∀",
     "&exist;", "∃",
     "&nabla;", "∇",
     "&larr;", "←",
     "&rarr;", "→",
     "&uarr;", "↑",
     "&darr;", "↓",
     "&harr;", "↔",
     "&lArr;", "⇐",
     "&rArr;", "⇒",
     "&plusmn;", "±",
     "&middot;", "·",
     "&times;", "×",
     "&divide;", "÷",
     "&sup1;", "¹",
     "&sup2;", "²",
     "&sup3;", "³",
     "&frac14;", "¼",
     "&frac12;", "½",
     "&frac34;", "¾",
     "&not;", "¬",
     "&Aacute;", "Á",
     "&Eacute;", "É",
     "&Iacute;", "Í",
     "&Oacute;", "Ó",
     "&Uacute;", "Ú",
     "&Yacute;", "Ý",
     "&aacute;", "á",
     "&eacute;", "é",
     "&iacute;", "í",
     "&oacute;", "ó",
     "&uacute;", "ú",
     "&yacute;", "ý",
     "&Acirc;", "Â",
     "&Ecirc;", "Ê",
     "&Icirc;", "Î",
     "&Ocirc;", "Ô",
     "&Ucirc;", "Û",
     "&acirc;", "â",
     "&ecirc;", "ê",
     "&icirc;", "î",
     "&ocirc;", "ô",
     "&ucirc;", "û",
     "&Agrave;", "À",
     "&Egrave;", "È",
     "&Igrave;", "Ì",
     "&Ograve;", "Ò",
     "&Ugrave;", "Ù",
     "&agrave;", "à",
     "&egrave;", "è",
     "&igrave;", "ì",
     "&ograve;", "ò",
     "&ugrave;", "ù",
     "&Auml;", "Ä",
     "&Euml;", "Ë",
     "&Iuml;", "Ï",
     "&Ouml;", "Ö",
     "&auml;", "ä",
     "&euml;", "ë",
     "&iuml;", "ï",
     "&ouml;", "ö",
     "&uuml;", "ü",
     "&yuml;", "ÿ",
     "&Atilde;", "Ã",
     "&atilde;", "ã",
     "&Otilde;", "Õ",
     "&otilde;", "õ",
     "&aring;", "å",
     "&Aring;", "Å",
     "&Oslash;", "Ø",
     "&oslash;", "ø",
     "&AElig;", "Æ",
     "&aelig;", "æ",
     "&Ntilde;", "Ñ",
     "&ntilde;", "ñ",
     "&Ccedil;", "Ç",
     "&ccedil;", "ç",
     "&szlig;", "ß",
     "&THORN;", "Þ",
     "&thorn;", "þ",
     "&ETH;", "Ð",
     "&eth;", "ð",
     "&alpha;", "α",
     "&beta;", "β",
     "&gamma;", "γ",
     "&delta;", "δ",
     "&epsilon;", "ε",
     "&zeta;", "ζ",
     "&eta;", "η",
     "&theta;", "θ",
     "&iota;", "ι",
     "&kappa;", "κ",
     "&mu;", "μ",
     "&nu;", "ν",
     "&omicron;", "ο",
     "&xi;", "ξ",
     "&pi;", "π",
     "&rho;", "ρ",
     "&sigma;", "σ",
     "&tau;", "τ",
     "&upsilon;", "υ",
     "&phi;", "φ",
     "&chi;", "χ",
     "&psi;", "ψ",
     "&omega;", "ω"
};

static int
_is_white(int c)
{
   /*
    * unicode list of whitespace chars
    *
    * 0009..000D <control-0009>..<control-000D>
    * 0020 SPACE
    * 0085 <control-0085>
    * 00A0 NO-BREAK SPACE
    * 1680 OGHAM SPACE MARK
    * 180E MONGOLIAN VOWEL SEPARATOR
    * 2000..200A EN QUAD..HAIR SPACE
    * 2028 LINE SEPARATOR
    * 2029 PARAGRAPH SEPARATOR
    * 202F NARROW NO-BREAK SPACE
    * 205F MEDIUM MATHEMATICAL SPACE
    * 3000 IDEOGRAPHIC SPACE
    */
   if (
       (c == 0x20) ||
       ((c >= 0x9) && (c <= 0xd)) ||
       (c == 0x85) ||
       (c == 0xa0) ||
       (c == 0x1680) ||
       (c == 0x180e) ||
       ((c >= 0x2000) && (c <= 0x200a)) ||
       (c == 0x2028) ||
       (c == 0x2029) ||
       (c == 0x202f) ||
       (c == 0x205f) ||
       (c == 0x3000)
       )
     return 1;
   return 0;
}

static char *
_clean_white(int clean_start, int clean_end, char *str)
{
   char *p, *p2, *str2 = NULL;
   int white, pwhite, start, ok;

   return str;
   str2 = malloc(strlen(str) + 2);
   p = str;
   p2 = str2;
   white = 0;
   pwhite = 0;
   start = 1;
   ok = 1;
   while (*p != 0)
     {
	pwhite = white;
	if (isspace(*p) || _is_white(*p)) white = 1;
	else white = 0;
	if ((pwhite) && (white)) ok = 0;
	else
	  {
	     if (!clean_start)
	       {
		  if ((start) && (pwhite) && (!white))
		    {
//		       *p2 = ' ';
//		       p2++;
		    }
	       }
	     ok = 1;
	     if (!white) start = 0;
	  }
	if (clean_start)
	  {
	     if ((start) && (ok)) ok = 0;
	  }
	if (ok)
	  {
	     *p2 = *p;
	     p2++;
	  }
	p++;
     }
   *p2 = 0;
   if (clean_end)
     {
	while (p2 > str2)
	  {
	     p2--;
	     if (!(isspace(*p2) || _is_white(*p2))) break;
	     *p2 = 0;
	  }
     }
   free(str);
   return str2;
}

static void
_append_text_run(Evas_Object_Textblock *o, char *s, char *p)
{
   if ((s) && (p > s))
     {
	char *ts;
	
	ts = malloc(p - s + 1);
	if (ts)
	  {
	     strncpy(ts, s, p - s);
	     ts[p - s] = 0;
	     ts = _clean_white(0, 0, ts);
	     evas_textblock2_cursor_text_append(o->cursor, ts);
	     free(ts);
	  }
     }
}


static int
_hex_string_get(char ch)
{
   if ((ch >= '0') && (ch <= '9')) return (ch - '0');
   else if ((ch >= 'A') && (ch <= 'F')) return (ch - 'A' + 10);
   else if ((ch >= 'a') && (ch <= 'f')) return (ch - 'a' + 10);
   return 0;
}

static void
_format_color_parse(char *str, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
   int slen;
   
   slen = strlen(str);
   *r = *g = *b = *a = 0;
   
   if (slen == 7) /* #RRGGBB */
     {
	*r = (_hex_string_get(str[1]) << 4) | (_hex_string_get(str[2]));
	*g = (_hex_string_get(str[3]) << 4) | (_hex_string_get(str[4]));
	*b = (_hex_string_get(str[5]) << 4) | (_hex_string_get(str[6]));
	*a = 0xff;
     }
   else if (slen == 9) /* #RRGGBBAA */
     {
	*r = (_hex_string_get(str[1]) << 4) | (_hex_string_get(str[2]));
	*g = (_hex_string_get(str[3]) << 4) | (_hex_string_get(str[4]));
	*b = (_hex_string_get(str[5]) << 4) | (_hex_string_get(str[6]));
	*a = (_hex_string_get(str[7]) << 4) | (_hex_string_get(str[8]));
     }
   else if (slen == 4) /* #RGB */
     {
	*r = _hex_string_get(str[1]);
	*r = (*r << 4) | *r;
	*g = _hex_string_get(str[2]);
	*g = (*g << 4) | *g;
	*b = _hex_string_get(str[3]);
	*b = (*b << 4) | *b;
	*a = 0xff;
     }
   else if (slen == 5) /* #RGBA */
     {
	*r = _hex_string_get(str[1]);
	*r = (*r << 4) | *r;
	*g = _hex_string_get(str[2]);
	*g = (*g << 4) | *g;
	*b = _hex_string_get(str[3]);
	*b = (*b << 4) | *b;
	*a = _hex_string_get(str[4]);
	*a = (*a << 4) | *a;
     }
}

static void
_format_command(Evas_Object *obj, Evas_Object_Textblock_Format *fmt, char *cmd, char *param)
{
   int new_font = 0;
   
   if (!strcmp(cmd, "font"))
     {
	if ((!fmt->font.name) ||
	    ((fmt->font.name) && (strcmp(fmt->font.name, param))))
	  {
	     if (fmt->font.name) free(fmt->font.name);
	     fmt->font.name = strdup(param);
	     new_font = 1;
	  }
     }
   else if (!strcmp(cmd, "font_fallbacks"))
     {
	if ((!fmt->font.fallbacks) ||
	    ((fmt->font.fallbacks) && (strcmp(fmt->font.fallbacks, param))))
	  {
	     /* policy - when we say "fallbacks" do we prepend and use prior
	      * fallbacks... or shoudl we replace. for nwo we replace
	      */
	     if (fmt->font.fallbacks) free(fmt->font.fallbacks);
	     fmt->font.fallbacks = strdup(param);
	     new_font = 1;
	  }
     }
   else if (!strcmp(cmd, "font_size"))
     {
	int v;
	
	v = atoi(param);
	if (v != fmt->font.size)
	  {
	     fmt->font.size = v;
	     new_font = 1;
	  }
     }
   else if (!strcmp(cmd, "font_source"))
     {
	if ((!fmt->font.source) ||
	    ((fmt->font.source) && (strcmp(fmt->font.source, param))))
	  {
	     if (fmt->font.source) free(fmt->font.source);
	     fmt->font.source = strdup(param);
	     new_font = 1;
	  }
     }
   else if (!strcmp(cmd, "color"))
     _format_color_parse(param,
			 &(fmt->color.normal.r), &(fmt->color.normal.g),
			 &(fmt->color.normal.b), &(fmt->color.normal.a));
   else if (!strcmp(cmd, "underline_color"))
     _format_color_parse(param,
			 &(fmt->color.underline.r), &(fmt->color.underline.g),
			 &(fmt->color.underline.b), &(fmt->color.underline.a));
   else if (!strcmp(cmd, "underline2_color"))
     _format_color_parse(param,
			 &(fmt->color.underline2.r), &(fmt->color.underline2.g),
			 &(fmt->color.underline2.b), &(fmt->color.underline2.a));
   else if (!strcmp(cmd, "outline_color"))
     _format_color_parse(param,
			 &(fmt->color.outline.r), &(fmt->color.outline.g),
			 &(fmt->color.outline.b), &(fmt->color.outline.a));
   else if (!strcmp(cmd, "shadow_color"))
     _format_color_parse(param,
			 &(fmt->color.shadow.r), &(fmt->color.shadow.g),
			 &(fmt->color.shadow.b), &(fmt->color.shadow.a));
   else if (!strcmp(cmd, "glow_color"))
     _format_color_parse(param,
			 &(fmt->color.glow.r), &(fmt->color.glow.g),
			 &(fmt->color.glow.b), &(fmt->color.glow.a));
   else if (!strcmp(cmd, "glow2_color"))
     _format_color_parse(param,
			 &(fmt->color.glow2.r), &(fmt->color.glow2.g),
			 &(fmt->color.glow2.b), &(fmt->color.glow2.a));
   else if (!strcmp(cmd, "backing_color"))
     _format_color_parse(param,
			 &(fmt->color.backing.r), &(fmt->color.backing.g),
			 &(fmt->color.backing.b), &(fmt->color.backing.a));
   else if (!strcmp(cmd, "strikethrough_color"))
     _format_color_parse(param,
			 &(fmt->color.strikethrough.r), &(fmt->color.strikethrough.g),
			 &(fmt->color.strikethrough.b), &(fmt->color.strikethrough.a));
   else if (!strcmp(cmd, "align"))
     {
	if (!strcmp(param, "middle")) fmt->halign = 0.5;
	else if (!strcmp(param, "center")) fmt->halign = 0.5;
	else if (!strcmp(param, "left")) fmt->halign = 0.0;
	else if (!strcmp(param, "right")) fmt->halign = 1.0;
	else if (strchr(param, '%'))
	  {
	     char *ts, *p;
	     
	     ts = strdup(param);
	     if (ts)
	       {
		  p = strchr(ts, '%');
		  *p = 0;
		  fmt->halign = ((double)atoi(ts)) / 100.0;
		  free(ts);
	       }
	     if (fmt->halign < 0.0) fmt->halign = 0.0;
	     else if (fmt->halign > 1.0) fmt->halign = 1.0;
	  }
	else
	  {
	     fmt->halign = atof(param);
	     if (fmt->halign < 0.0) fmt->halign = 0.0;
	     else if (fmt->halign > 1.0) fmt->halign = 1.0;
	  }
     }
   else if (!strcmp(cmd, "valign"))
     {
        if (!strcmp(param, "top")) fmt->valign = 0.0;
	else if (!strcmp(param, "middle")) fmt->valign = 0.5;
	else if (!strcmp(param, "center")) fmt->valign = 0.5;
	else if (!strcmp(param, "bottom")) fmt->valign = 1.0;
	else if (!strcmp(param, "baseline")) fmt->valign = -1.0;
	else if (!strcmp(param, "base")) fmt->valign = -1.0;
	else if (strchr(param, '%'))
	  {
	     char *ts, *p;
	     
	     ts = strdup(param);
	     if (ts)
	       {
		  p = strchr(ts, '%');
		  *p = 0;
		  fmt->valign = ((double)atoi(ts)) / 100.0;
		  free(ts);
	       }
	     if (fmt->valign < 0.0) fmt->valign = 0.0;
	     else if (fmt->valign > 1.0) fmt->valign = 1.0;
	  }
	else
	  {
	     fmt->valign = atof(param);
	     if (fmt->valign < 0.0) fmt->valign = 0.0;
	     else if (fmt->valign > 1.0) fmt->valign = 1.0;
	  }
     }
   else if (!strcmp(cmd, "wrap"))
     {
	if (!strcmp(param, "word"))
	  {
	     fmt->wrap_word = 1;
	     fmt->wrap_char = 0;
	  }
	else if (!strcmp(param, "char"))
	  {
	     fmt->wrap_word = 0;
	     fmt->wrap_char = 1;
	  }
	else
	  {
	     fmt->wrap_word = 0;
	     fmt->wrap_char = 0;
	  }
     }
   else if (!strcmp(cmd, "left_margin"))
     {
	if (!strcmp(param, "reset"))
	  fmt->margin.l = 0;
	else
	  {
	     if (param[0] == '+')
	       fmt->margin.l += atoi(&(param[1]));
	     else if (param[0] == '-')
	       fmt->margin.l -= atoi(&(param[1]));
	     else
	       fmt->margin.l = atoi(param);
	     if (fmt->margin.l < 0) fmt->margin.l = 0;
	  }
     }
   else if (!strcmp(cmd, "right_margin"))
     {
	if (!strcmp(param, "reset"))
	  fmt->margin.r = 0;
	else
	  {
	     if (param[0] == '+')
	       fmt->margin.r += atoi(&(param[1]));
	     else if (param[0] == '-')
	       fmt->margin.r -= atoi(&(param[1]));
	     else
	       fmt->margin.r = atoi(param);
	     if (fmt->margin.r < 0) fmt->margin.r = 0;
	  }
     }
   else if (!strcmp(cmd, "underline"))
     {
	if (!strcmp(param, "off"))
	  {
	     fmt->underline = 0;
	     fmt->underline2 = 0;
	  }
	else if ((!strcmp(param, "on")) ||
		 (!strcmp(param, "single")))
	  {
	     fmt->underline = 1;
	     fmt->underline2 = 0;
	  }
	else if (!strcmp(param, "double"))
	  {
	     fmt->underline = 1;
	     fmt->underline2 = 1;
	  }
     }
   else if (!strcmp(cmd, "strikethrough"))
     {
	if (!strcmp(param, "off"))
	  fmt->strikethrough = 0;
	else if (!strcmp(param, "on"))
	  fmt->strikethrough = 1;
     }
   else if (!strcmp(cmd, "backing"))
     {
	if (!strcmp(param, "off"))
	  fmt->backing = 0;
	else if (!strcmp(param, "on"))
	  fmt->backing = 1;
     }
   else if (!strcmp(cmd, "style"))
     {
	if (!strcmp(param, "off")) fmt->style = STYLE_PLAIN;
	else if (!strcmp(param, "none")) fmt->style = STYLE_PLAIN;
	else if (!strcmp(param, "plain")) fmt->style = STYLE_PLAIN;
	else if (!strcmp(param, "shadow")) fmt->style = STYLE_SHADOW;
	else if (!strcmp(param, "outline")) fmt->style = STYLE_OUTLINE;
	else if (!strcmp(param, "outline_shadow")) fmt->style = STYLE_OUTLINE_SHADOW;
	else if (!strcmp(param, "outline_soft_shadow")) fmt->style = STYLE_OUTLINE_SOFT_SHADOW;
	else if (!strcmp(param, "glow")) fmt->style = STYLE_GLOW;
	else if (!strcmp(param, "far_shadow")) fmt->style = STYLE_FAR_SHADOW;
	else if (!strcmp(param, "soft_shadow")) fmt->style = STYLE_SOFT_SHADOW;
	else if (!strcmp(param, "far_soft_shadow")) fmt->style = STYLE_FAR_SOFT_SHADOW;
	else fmt->style = STYLE_PLAIN;
     }
   else if (!strcmp(cmd, "tabstops"))
     {
	fmt->tabstops = atoi(param);
	if (fmt->tabstops < 1) fmt->tabstops = 1;
     }
   
   if (new_font)
     {
	void *of;
	char *buf = NULL;

	of = fmt->font.font;
	if ((fmt->font.name) && (fmt->font.fallbacks))
	  {
	     buf = malloc(strlen(fmt->font.name) + 1 + strlen(fmt->font.fallbacks) + 1);
	     strcpy(buf, fmt->font.name);
	     strcat(buf, ",");
	     strcat(buf, fmt->font.fallbacks);
	  }
	else if (fmt->font.name)
	  buf = strdup(fmt->font.name);
	fmt->font.font = evas_font_load(obj->layer->evas, 
					fmt->font.name, fmt->font.source, 
					fmt->font.size);
	if (buf) free(buf);
	if (of) evas_font_free(obj->layer->evas, of);
     }
}

static int
_format_is_param(char *item)
{
   if (strchr(item, '=')) return 1;
   return 0;
}

static void
_format_param_parse(char *item, char **key, char **val)
{
   char *p, *pv;
   char *k, *v;
   int qoute = 0;
   
   p = strchr(item, '=');
   k = malloc(p - item + 1);
   strncpy(k, item, p - item);
   k[p - item] = 0;
   *key = k;
   p++;
   v = strdup(p);
   pv = v;
   for (;;)
     {
	if (*p == 0)
	  {
	     *pv = 0;
	     break;
	  }
	else if (*p != '"')
	  {
	     *pv = *p;
	  }
	pv++;
	p++;
     }
   *val = v;
}

static char *
_format_parse(char **s)
{
   char *p, *item, *ss, *ds;
   char *s1 = NULL, *s2 = NULL;
   
   p = *s;
   if (*p == 0) return NULL;
   for (;;)
     {
	if (!s1)
	  {
	     if (*p != ' ') s1 = p;
	     if (*p == 0) break;
	  }
	else if (!s2)
	  {
	     if ((p > *s) && (p[-1] != '\\'))
	       {
		  if (*p == ' ') s2 = p;
	       }
	     if (*p == 0) s2 = p;
	  }
	p++;
	if (s1 && s2)
	  {
	     item = malloc(s2 - s1 + 1);
	     if (item)
	       {
		  ds = item;
		  for (ds = item, ss = s1; ss < s2; ss++)
		    {
		       if ((*ss == '\\') && (ss < (s2 - 1))) ss++;
		       *ds = *ss;
		       ds++;
		    }
		  *ds = 0;
	       }
	     *s = s2;
	     printf("ITEM: %s\n", item);
	     return item;
	  }
     }
   *s = p;
   return NULL;
}

static void
_format_fill(Evas_Object *obj, Evas_Object_Textblock_Format *fmt, char *str)
{
   char *s;
   char *item;
   
   s = str;
   while ((item = _format_parse(&s)))
     {
	if (_format_is_param(item))
	  {
	     char *key = NULL, *val = NULL;
	     
	     _format_param_parse(item, &key, &val);
	     _format_command(obj, fmt, key, val);
	     free(key);
	     free(val);
	  }
	else
	  {
	     /* immediate - not handled here */
	  }
	free(item);
     }
}

static Evas_Object_Textblock_Format *
_format_dup(Evas_Object *obj, Evas_Object_Textblock_Format *fmt)
{
   Evas_Object_Textblock_Format *fmt2;
   char *buf = NULL;
   
   fmt2 = calloc(1, sizeof(Evas_Object_Textblock_Format));
   memcpy(fmt2, fmt, sizeof(Evas_Object_Textblock_Format));
   fmt2->ref = 1;
   if (fmt->font.name) fmt2->font.name = strdup(fmt->font.name);
   if (fmt->font.fallbacks) fmt2->font.fallbacks = strdup(fmt->font.fallbacks);
   if (fmt->font.source) fmt2->font.source = strdup(fmt->font.source);

   if ((fmt2->font.name) && (fmt2->font.fallbacks))
     {
	buf = malloc(strlen(fmt2->font.name) + 1 + strlen(fmt2->font.fallbacks) + 1);
	strcpy(buf, fmt2->font.name);
	strcat(buf, ",");
	strcat(buf, fmt2->font.fallbacks);
     }
   else if (fmt2->font.name)
     buf = strdup(fmt2->font.name);
   fmt2->font.font = evas_font_load(obj->layer->evas, 
				   fmt2->font.name, fmt2->font.source,
				   fmt2->font.size);
   if (buf) free(buf);
   return fmt2;
}










typedef struct _Ctxt Ctxt;

struct _Ctxt
{
   Evas_Object *obj;
   Evas_Object_Textblock *o;
   
   Evas_Object_Textblock_Line *lines;
   Evas_Object_Textblock_Line *ln;

   Evas_List *format_stack;
   
   int x, y;
   int w, h;
   int wmax, hmax;
   int maxascent, maxdescent;
   int marginl, marginr;
   int line_no;
   int underline_extend;
   int have_underline, have_underline2;
   double align;
};

static void
_layout_format_ascent_descent_adjust(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   int ascent, descent;

   if (fmt->font.font)
     {
	ascent = c->ENFN->font_max_ascent_get(c->ENDT, fmt->font.font);
	descent = c->ENFN->font_max_descent_get(c->ENDT, fmt->font.font);
	if (c->maxascent < ascent) c->maxascent = ascent;
	if (c->maxdescent < descent) c->maxdescent = descent;
     }
}

static void
_layout_line_new(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   c->ln = calloc(1, sizeof(Evas_Object_Textblock_Line));
   c->align = fmt->halign;
   c->marginl = fmt->margin.l;
   c->marginr = fmt->margin.r;
   c->lines = evas_object_list_append(c->lines, c->ln);
   c->x = 0;
   c->maxascent = c->maxdescent = 0;
   c->ln->line_no = -1;
   _layout_format_ascent_descent_adjust(c, fmt);
}

static Evas_Object_Textblock_Format *
_layout_format_push(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   if (fmt)
     {
	fmt = _format_dup(c->obj, fmt);
	c->format_stack  = evas_list_prepend(c->format_stack, fmt);
     }
   else
     {
	fmt = calloc(1, sizeof(Evas_Object_Textblock_Format));
	c->format_stack  = evas_list_prepend(c->format_stack, fmt);
	fmt->ref = 1;
	fmt->halign = 0.0;
	fmt->valign = -1.0;
	fmt->style = STYLE_PLAIN;
	fmt->tabstops = 32;
     }
   return fmt;
}

static Evas_Object_Textblock_Format *
_layout_format_pop(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   if ((c->format_stack) && (c->format_stack->next))
     {
	_format_free(c->obj, fmt);
	c->format_stack = evas_list_remove_list(c->format_stack, c->format_stack);
	fmt = c->format_stack->data;
     }
   return fmt;
}

static void
_layout_format_value_handle(Ctxt *c, Evas_Object_Textblock_Format *fmt, char *item)
{
   char *key = NULL, *val = NULL;
   
   _format_param_parse(item, &key, &val);
   if ((key) && (val)) _format_command(c->obj, fmt, key, val);
   if (key) free(key);
   if (val) free(val);
   c->align = fmt->halign;
   c->marginl = fmt->margin.l;
   c->marginr = fmt->margin.r;
}

static void
_layout_line_advance(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   Evas_Object_Textblock_Item *it;
   Evas_Object_List *l;
   
   c->maxascent = c->maxdescent = 0;
   if (!c->ln->items)
     _layout_format_ascent_descent_adjust(c, fmt);
   for (l = (Evas_Object_List *)c->ln->items; l; l = l->next)
     {
	int endx;
	
	it = (Evas_Object_Textblock_Item *)l;
	if (it->format->font.font)
	  it->baseline = c->ENFN->font_max_ascent_get(c->ENDT, it->format->font.font);
	_layout_format_ascent_descent_adjust(c, it->format);
	endx = it->x + it->w;
	if (endx > c->ln->w) c->ln->w = endx;
     }
   c->ln->y = c->y + c->o->style_pad.t;
   c->ln->h = c->maxascent + c->maxdescent;
   c->ln->baseline = c->maxascent;
   if (c->have_underline2)
     {
	if (c->maxdescent < 4) c->underline_extend = 4 - c->maxdescent;
     }
   else if (c->have_underline)
     {
	if (c->maxdescent < 2) c->underline_extend = 2 - c->maxdescent;
     }
   c->ln->line_no = c->line_no;
   c->line_no++;
   c->y += c->maxascent + c->maxdescent;
   if (c->w >= 0)
     {
	c->ln->x = c->marginl + c->o->style_pad.l +
	  ((c->w - c->ln->w -
	    c->o->style_pad.l - c->o->style_pad.r - 
	    c->marginl - c->marginr) * c->align);
	if ((c->ln->x + c->ln->w + c->marginr - c->o->style_pad.l) > c->wmax)
	  c->wmax = c->ln->x + c->ln->w + c->marginl + c->marginr - c->o->style_pad.l;
     }
   else
     {
	c->ln->x = c->marginl + c->o->style_pad.l;
	if ((c->ln->x + c->ln->w + c->marginr - c->o->style_pad.l) > c->wmax)
	  c->wmax = c->ln->x + c->ln->w + c->marginl + c->marginr - c->o->style_pad.l;
     }
   _layout_line_new(c, fmt);
}

static Evas_Object_Textblock_Item *
_layout_item_new(Ctxt *c, Evas_Object_Textblock_Format *fmt, char *str)
{
   Evas_Object_Textblock_Item *it;
   
   it = calloc(1, sizeof(Evas_Object_Textblock_Item));
   it->format = fmt;
   it->format->ref++;
   it->text = strdup(str);
   return it;
}

static int
_layout_text_cutoff_get(Ctxt *c, Evas_Object_Textblock_Format *fmt, Evas_Object_Textblock_Item *it)
{
   int cx, cy, cw, ch;

   if (fmt->font.font)
     return c->ENFN->font_char_at_coords_get(c->ENDT, fmt->font.font, it->text,
					     c->w - 
					     c->o->style_pad.l - 
					     c->o->style_pad.r - 
					     c->marginl - 
					     c->marginr -
					     c->x,
					     0, &cx, &cy, &cw, &ch);
   return -1;
}

static void
_layout_item_text_cutoff(Ctxt *c, Evas_Object_Textblock_Item *it, int cut)
{
   char *ts;
   
   ts = it->text;
   ts[cut] = 0;
   it->text = strdup(ts);
   free(ts);
}

static int
_layout_word_start(char *str, int start)
{
   int p, tp, chr = 0;
   
   p = start;
   chr = evas_common_font_utf8_get_next((unsigned char *)(str), &p);
   if (_is_white(chr))
     {
	tp = p;
	while (_is_white(chr) && (p >= 0))
	  {
	     tp = p;
	     chr = evas_common_font_utf8_get_next((unsigned char *)(str), &p);
	  }
	return tp;
     }
   p = start;
   tp = p;
   while (p >= 0)
     {
	chr = evas_common_font_utf8_get_prev((unsigned char *)(str), &p);
	if (_is_white(chr)) break;
	tp = p;
     }
   p = tp;
   if (p < 0) p = 0;
   if ((p >= 0) && (_is_white(chr)))
     evas_common_font_utf8_get_next((unsigned char *)(str), &p);
   return p;
}

static int
_layout_ends_with_space(char *str)
{
   int p, chr;
   
   p = evas_common_font_utf8_get_last((unsigned char *)(str), strlen(str));
   if (p < 0) return 0;
   chr = evas_common_font_utf8_get_next((unsigned char *)(str), &p);
   return _is_white(chr);
}

static int
_layout_strip_trailing_whitespace(Ctxt *c, Evas_Object_Textblock_Format *fmt, Evas_Object_Textblock_Item *it)
{
   int p, tp, chr, adv, tw, th;
   
   p = evas_common_font_utf8_get_last((unsigned char *)(it->text), strlen(it->text));
   tp = p;
   if (p >= 0)
/*   while (p >= 0)*/
     {
	chr = evas_common_font_utf8_get_prev((unsigned char *)(it->text), &p);
	if (_is_white(chr))
	  {
	     _layout_item_text_cutoff(c, it, tp);
	     adv = 0;
	     if (it->format->font.font)
	       adv = c->ENFN->font_h_advance_get(c->ENDT, it->format->font.font, it->text);
	     tw = th = 0;
	     if (it->format->font.font)
	       c->ENFN->font_string_size_get(c->ENDT, it->format->font.font, it->text, &tw, &th);
	     it->w = tw;
	     it->h = th;
	     c->x = it->x + adv;
	     return 1;
	  }
/*	  
	if (!_is_white(chr))
	  {
	     evas_common_font_utf8_get_next((unsigned char *)(it->text), &tp);
	     _layout_item_text_cutoff(c, it, tp);
	     adv = c->ENFN->font_h_advance_get(c->ENDT, it->format->font.font, it->text);
	     c->ENFN->font_string_size_get(c->ENDT, it->format->font.font, it->text, &tw, &th);
	     it->w = tw;
	     it->h = th;
	     c->x = it->x + adv;
	     return;
	  }
	tp = p;
 */
     }
   return 0;
}

static int
_layout_item_abort(Ctxt *c, Evas_Object_Textblock_Format *fmt, Evas_Object_Textblock_Item *it)
{
   if (it->text) free(it->text);
   _format_free(c->obj, it->format);
   free(it);
   if (c->ln->items)
     {
	it = (Evas_Object_Textblock_Item *)((Evas_Object_List *)c->ln->items)->last;
	return _layout_strip_trailing_whitespace(c, fmt, it);
     }
   return 0;
}

static char *
_layout_next_char_jump(Ctxt *c, Evas_Object_Textblock_Item *it, char *str)
{
   int index;
   
   index = 0;
   evas_common_font_utf8_get_next((unsigned char *)str, &index);
   if (index >= 0)
     {
	str = str + index;
	_layout_item_text_cutoff(c, it, index);
     }
   else
     str = NULL;
   return str;
}

static int
_layout_last_item_ends_in_whitespace(Ctxt *c)
{
   Evas_Object_Textblock_Item *it;

   if (!c->ln->items) return 1;
   it = (Evas_Object_Textblock_Item *)((Evas_Object_List *)c->ln->items)->last;
   return _layout_ends_with_space(it->text);
}

static int
_layout_word_end(char *str, int p)
{
   int ch, tp;
   
   tp = p;
   ch = evas_common_font_utf8_get_next((unsigned char *)str, &tp);
   while ((!_is_white(ch)) && (tp >= 0) && (ch != 0))
     {
	p = tp;
	ch = evas_common_font_utf8_get_next((unsigned char *)str, &tp);
     }
   if (ch == 0) return -1;
   return p;
}

static int
_layout_word_next(char *str, int p)
{
   int ch, tp;
   
   tp = p;
   ch = evas_common_font_utf8_get_next((unsigned char *)str, &tp);
   while ((!_is_white(ch)) && (tp >= 0) && (ch != 0))
     {
	p = tp;
	ch = evas_common_font_utf8_get_next((unsigned char *)str, &tp);
     }
   if (ch == 0) return -1;
   while ((_is_white(ch)) && (tp >= 0) && (ch != 0))
     {
	p = tp;
	ch = evas_common_font_utf8_get_next((unsigned char *)str, &tp);
     }
   if (ch == 0) return -1;
   return p;
}

static void
_layout_walk_back_to_item_word_redo(Ctxt *c, Evas_Object_Textblock_Item *it)
{
   Evas_Object_Textblock_Item *pit, *new_it = NULL;
   Evas_List *remove_items = NULL, *l;
   int index, p, ch, tw, th, inset, adv;
   
   /* it is not appended yet */
   for (pit = (Evas_Object_Textblock_Item *)((Evas_Object_List *)c->ln->items)->last;
	pit;
	pit = (Evas_Object_Textblock_Item *)((Evas_Object_List *)pit)->prev)
     {
	if (_layout_ends_with_space(pit->text))
	  {
	     break;
	  }
	index = evas_common_font_utf8_get_last((unsigned char *)(pit->text), strlen(pit->text));
	index = _layout_word_start(pit->text, index);
	if (index == 0)
	  remove_items = evas_list_prepend(remove_items, pit);
	else
	  {
	     new_it = _layout_item_new(c, pit->format, pit->text + index);
	     new_it->source_node = pit->source_node;
	     new_it->source_pos = pit->source_pos + index;
	     _layout_item_text_cutoff(c, pit, index);
	     _layout_strip_trailing_whitespace(c, pit->format, pit);
/* ***	     
	     if (!white_stripped)
	       {
		  index = 0;
		  ch = evas_common_font_utf8_get_next((unsigned char *)str, &index);
		  if (_is_white(ch)) str += index;
	       }
 */
	     break;
	  }
     }
   for (l = remove_items; l; l = l->next)
     {
	c->ln->items = evas_object_list_remove(c->ln->items, l->data);
     }
   /* new line now */
   if (remove_items)
     {
	pit = remove_items->data;
	_layout_line_advance(c, pit->format);
     }
   else
     {
	_layout_line_advance(c, it->format);
     }
   if (new_it)
     {
	/* append new_it */
	tw = th = 0;
	if (new_it->format->font.font)
	  c->ENFN->font_string_size_get(c->ENDT, new_it->format->font.font, new_it->text, &tw, &th);
	new_it->w = tw;
	new_it->h = th;
	inset = 0;
	if (new_it->format->font.font)
	  inset = c->ENFN->font_inset_get(c->ENDT, new_it->format->font.font, new_it->text);
	new_it->inset = inset;
	new_it->x = c->x;
	adv = 0;
	if (new_it->format->font.font)
	  adv = c->ENFN->font_h_advance_get(c->ENDT, new_it->format->font.font, new_it->text);
	c->x += adv;
	c->ln->items = evas_object_list_append(c->ln->items, new_it);
     }
   while (remove_items)
     {
	pit = remove_items->data;
	remove_items = evas_list_remove_list(remove_items, remove_items);
	/* append pit */
	pit->x = c->x;
	adv = c->ENFN->font_h_advance_get(c->ENDT, pit->format->font.font, pit->text);
	c->x += adv;
	c->ln->items = evas_object_list_append(c->ln->items, pit);
     }
   if (it)
     {
	/* append it */
	it->x = c->x;
	adv = 0;
	if (it->format->font.font)
	  adv = c->ENFN->font_h_advance_get(c->ENDT, it->format->font.font, it->text);
	c->x += adv;
	c->ln->items = evas_object_list_append(c->ln->items, it);
     }
}

static void
_layout_text_append(Ctxt *c, Evas_Object_Textblock_Format *fmt, Evas_Object_Textblock_Node *n)
{
   int adv, inset, tw, th, new_line, empty_item;
   int wrap, twrap, ch, index, white_stripped;
   char *str;
   Evas_Object_Textblock_Item *it, *tit;
   
   str = n->text;
   new_line = 0;
   empty_item = 0;
   while (str)
     {
	/* if this is the first line item and it starts with spaces - remove them */
	wrap = 0;
	white_stripped = 0;
	if (!c->ln->items)
	  {
	     twrap = wrap;
	     ch = evas_common_font_utf8_get_next((unsigned char *)str, &wrap);
	     while (_is_white(ch))
	       {
		  twrap = wrap;
		  ch = evas_common_font_utf8_get_next((unsigned char *)str, &wrap);
	       }
	     str = str + twrap;
	  }
	it = _layout_item_new(c, fmt, str);
	it->source_node = n;
	it->source_pos = str - n->text;
	tw = th = 0;
	if (fmt->font.font)
	  c->ENFN->font_string_size_get(c->ENDT, fmt->font.font, it->text, &tw, &th);
	if ((c->w >= 0) && 
	    ((fmt->wrap_word) || (fmt->wrap_char)) && 
	    ((c->x + tw) > 
	     (c->w - c->o->style_pad.l - c->o->style_pad.r - 
	      c->marginl - c->marginr)))
	  {
	     wrap = _layout_text_cutoff_get(c, fmt, it);
	     if (wrap > 0)
	       {
		  if (fmt->wrap_word)
		    {
		       wrap = _layout_word_start(str, wrap);
		       if (wrap > 0)
			 {
			    twrap = wrap;
			    ch = evas_common_font_utf8_get_prev((unsigned char *)str, &twrap);
//			    
			    /* the text intersects the wrap point on a whitespace char */
			    if (_is_white(ch))
			      {
				 _layout_item_text_cutoff(c, it, wrap);
				 twrap = wrap;
				 ch = evas_common_font_utf8_get_next((unsigned char *)str, &twrap);
				 str = str + twrap;
			      }
			    /* intersects a word */
			    else
			      {
				 /* walk back to start of word */
				 twrap = _layout_word_start(str, wrap);
				 if (twrap != 0)
				   {
				      wrap = twrap;
				      ch = evas_common_font_utf8_get_prev((unsigned char *)str, &twrap);
				      _layout_item_text_cutoff(c, it, twrap);
				      str = str + wrap;
				   }
				 else
				   {
				      empty_item = 1;
				      if (it->text) free(it->text);
				      _format_free(c->obj, it->format);
				      free(it);
				      it = (Evas_Object_Textblock_Item *)((Evas_Object_List *)c->ln->items)->last;
				      _layout_strip_trailing_whitespace(c, fmt, it);
				      twrap = _layout_word_end(str, wrap);
				      ch = evas_common_font_utf8_get_next((unsigned char *)str, &twrap);
				      str = str + twrap;
				   }
			      }
#if 0				
			    ch = evas_common_font_utf8_get_prev((unsigned char *)str, &twrap);
			    while (_is_white(ch) && (twrap >= 0))
			      ch = evas_common_font_utf8_get_prev((unsigned char *)str, &twrap);
			    if (!_is_white(ch))
			      {
				 if (twrap >= 0)
				   evas_common_font_utf8_get_next((unsigned char *)str, &twrap);
				 else twrap = 0;
				 evas_common_font_utf8_get_next((unsigned char *)str, &twrap);
			      }
			    else
			      twrap = 0;
			    if (twrap == 0)
			      {
				 int ptwrap;
				 
				 empty_item = 1;
				 if (it->text) free(it->text);
				 _format_free(c->obj, it->format);
				 free(it);
				 ptwrap = twrap;
				 for (;;)
				   {
				      ch = evas_common_font_utf8_get_next((unsigned char *)str, &twrap);
				      if (twrap < 0)
					{
					   ptwrap = twrap;
					   break;
					}
				      if (!_is_white(ch)) break;
				      ptwrap = twrap;
				   }
				 str = str + ptwrap;
			      }
			    else
			      {
				 _layout_item_text_cutoff(c, it, twrap);
				 str = str + wrap;
			      }
#endif				
			 }
		       else
			 {
#if 1			    
			    /* wrap now is the index of the word START */
			    index = wrap;
			    ch = evas_common_font_utf8_get_next((unsigned char *)str, &index);
			    if (!_is_white(ch) && 
				(!_layout_last_item_ends_in_whitespace(c)))
			      {
				 _layout_walk_back_to_item_word_redo(c, it);
				 return;
			      }
			    if (c->ln->items != NULL)
			      {
				 white_stripped = _layout_item_abort(c, fmt, it);
				 empty_item = 1;
			      }
			    else
			      {
				 if (wrap <= 0)
				   {
				      wrap = 0;
				      twrap = _layout_word_end(it->text, wrap);
				      wrap = twrap;
				      ch = evas_common_font_utf8_get_next((unsigned char *)str, &wrap);
				      if (twrap >= 0)
					_layout_item_text_cutoff(c, it, twrap);
				      if (wrap > 0)
					str = str + wrap;
				      else
					str = NULL;
				   }
				 else
				   str = NULL;
			      }
			 }
#endif		       
		    }
		  else if (fmt->wrap_char)
		    {
		       _layout_item_text_cutoff(c, it, wrap);
		       str = str + wrap;
		    }
		  new_line = 1;
	       }
	     else
	       {
		  /* wrap now is the index of the word START */
		  if (wrap <= 0)
		    {
		       if (wrap < 0) wrap = 0;
		       index = wrap;
		       ch = evas_common_font_utf8_get_next((unsigned char *)str, &index);
		       if (!_is_white(ch) && 
			   (!_layout_last_item_ends_in_whitespace(c)))
			 {
			    _layout_walk_back_to_item_word_redo(c, it);
			    return;
			 }
		    }
		  if (c->ln->items != NULL)
		    {
		       white_stripped = _layout_item_abort(c, fmt, it);
		       empty_item = 1;
		       new_line = 1;
		    }
		  else
		    {
		       if (wrap <= 0)
			 {
			    wrap = 0;
			    twrap = _layout_word_end(it->text, wrap);
			    wrap = _layout_word_next(it->text, wrap);
			    if (twrap >= 0)
			      _layout_item_text_cutoff(c, it, twrap);
			    if (wrap >= 0)
			      str = str + wrap;
			    else
			      str = NULL;
			 }
		       else
			 str = NULL;
		       new_line = 1;
		    }
	       }
	     if (!empty_item)
	       {
		  tw = th = 0;
		  if (fmt->font.font)
		    c->ENFN->font_string_size_get(c->ENDT, fmt->font.font, it->text, &tw, &th);
	       }
	  }
	else
	  str = NULL;
	if (empty_item) empty_item = 0;
	else
	  {
	     it->w = tw;
	     it->h = th;
	     inset = 0;
	     if (fmt->font.font)
	       inset = c->ENFN->font_inset_get(c->ENDT, fmt->font.font, it->text);
	     it->inset = inset;
	     it->x = c->x;
	     adv = 0;
	     if (fmt->font.font)
	       adv = c->ENFN->font_h_advance_get(c->ENDT, fmt->font.font, it->text);
	     c->x += adv;
	     c->ln->items = evas_object_list_append(c->ln->items, it);
	  }
	if (new_line)
	  {
	     if (!white_stripped)
	       {
		  index = 0;
		  ch = evas_common_font_utf8_get_next((unsigned char *)str, &index);
		  if (_is_white(ch)) str += index;
	       }
	     new_line = 0;
	     _layout_line_advance(c, fmt);
	  }
     }
}

static Evas_Object_Textblock_Format_Item *
_layout_format_item_add(Ctxt *c, Evas_Object_Textblock_Node *n, char *item)
{
   Evas_Object_Textblock_Format_Item *fi;
   
   fi = calloc(1, sizeof(Evas_Object_Textblock_Format_Item));
   fi->item = strdup(item);
   fi->source_node = n;
   c->ln->format_items = evas_object_list_append(c->ln->format_items, fi);
   return fi;
}

static void
_layout(Evas_Object *obj, int calc_only, int w, int h, int *w_ret, int *h_ret)
{
   Evas_Object_Textblock *o;
   Ctxt ctxt, *c;
   Evas_Object_List *l, *ll;
   Evas_List *removes = NULL;
   Evas_Object_Textblock_Format *fmt = NULL;
   int style_pad_l = 0, style_pad_r = 0, style_pad_t = 0, style_pad_b = 0;

   /* setup context */
   o = (Evas_Object_Textblock *)(obj->object_data);
   c = &ctxt;
   c->obj =obj;
   c->o = o;
   c->lines = c->ln = NULL;
   c->format_stack = NULL;
   c->x = c->y = 0;
   c->w = w;
   c->h = h;
   c->wmax = c->hmax = 0;
   c->maxascent = c->maxdescent = 0;
   c->marginl = c->marginr = 0;
   c->have_underline = 0;
   c->have_underline2 = 0;
   c->underline_extend = 0;
   c->line_no = 0;
   c->align = 0.0;

   /* setup default base style */
   if ((c->o->style) && (c->o->style->default_tag))
     {
	fmt = _layout_format_push(c, NULL);
	_format_fill(c->obj, fmt, c->o->style->default_tag);
     }
   /* run thru all text and format nodes generating lines */
   for (l = (Evas_Object_List *)c->o->nodes; l; l = l->next)
     {
	Evas_Object_Textblock_Node *n;
	
	n = (Evas_Object_Textblock_Node *)l;
	if (!c->ln) _layout_line_new(c, fmt);
	if ((n->type == NODE_FORMAT) && (n->text))
	  {
	     char *s;
	     char *item;
	     
	     s = n->text;
	     if (s[0] == '+')
	       {
		  fmt = _layout_format_push(c, fmt);
		  s++;
	       }
	     else if (s[0] == '-')
	       {
		  fmt = _layout_format_pop(c, fmt);
		  s++;
	       }
	     while ((item = _format_parse(&s)))
	       {
		  if (_format_is_param(item))
		    _layout_format_value_handle(c, fmt, item);
		  else
		    {
		       if (!strcmp(item, "\n"))
			 {
			    Evas_Object_Textblock_Format_Item *fi;
			    
			    fi = _layout_format_item_add(c, n, item);
			    fi->x = c->x;
			    fi->w = 0;
			    _layout_line_advance(c, fmt);
			 }
		       else if (!strcmp(item, "\t"))
			 { 
			    Evas_Object_Textblock_Format_Item *fi;
			    int x2;
			    
			    x2 = (fmt->tabstops * ((c->x + fmt->tabstops) / fmt->tabstops));
			    if (x2 >
				(c->w - c->o->style_pad.l - 
				 c->o->style_pad.r - 
				 c->marginl - c->marginr))
			      {
				 
				 _layout_line_advance(c, fmt);
				 x2 = (fmt->tabstops * ((c->x + fmt->tabstops) / fmt->tabstops));
			      }
			    if (c->ln->items)
			      {
				 Evas_Object_Textblock_Item *it;
				 
				 it = (Evas_Object_Textblock_Item *)((Evas_Object_List *)c->ln->items)->last;
				 _layout_strip_trailing_whitespace(c, fmt, it);
			      }
			    fi = _layout_format_item_add(c, n, item);
			    fi->x = c->x;
			    fi->w = x2 - c->x;
			    c->x = x2;
			 }
		    }
		  free(item);
	       }
	     if (fmt->style == STYLE_SHADOW)
	       {
		  if (style_pad_r < 1) style_pad_r = 1;
		  if (style_pad_b < 1) style_pad_b = 1;
	       }
	     else if (fmt->style == STYLE_OUTLINE)
	       {
		  if (style_pad_l < 1) style_pad_l = 1;
		  if (style_pad_r < 1) style_pad_r = 1;
		  if (style_pad_t < 1) style_pad_t = 1;
		  if (style_pad_b < 1) style_pad_b = 1;
	       }
	     else if (fmt->style == STYLE_GLOW)
	       {
		  if (style_pad_l < 2) style_pad_l = 2;
		  if (style_pad_r < 2) style_pad_r = 2;
		  if (style_pad_t < 2) style_pad_t = 2;
		  if (style_pad_b < 2) style_pad_b = 2;
	       }
	     else if (fmt->style == STYLE_OUTLINE_SHADOW)
	       {
		  if (style_pad_l < 1) style_pad_l = 1;
		  if (style_pad_r < 2) style_pad_r = 2;
		  if (style_pad_t < 1) style_pad_t = 1;
		  if (style_pad_b < 2) style_pad_b = 2;
	       }
	     else if (fmt->style == STYLE_FAR_SHADOW)
	       {
		  if (style_pad_r < 2) style_pad_r = 2;
		  if (style_pad_b < 2) style_pad_b = 2;
	       }
	     else if (fmt->style == STYLE_OUTLINE_SOFT_SHADOW)
	       {
		  if (style_pad_l < 1) style_pad_l = 1;
		  if (style_pad_r < 3) style_pad_r = 3;
		  if (style_pad_t < 1) style_pad_t = 1;
		  if (style_pad_b < 3) style_pad_b = 3;
	       }
	     else if (fmt->style == STYLE_SOFT_SHADOW)
	       {
		  if (style_pad_l < 1) style_pad_l = 1;
		  if (style_pad_r < 3) style_pad_r = 3;
		  if (style_pad_t < 1) style_pad_t = 1;
		  if (style_pad_b < 3) style_pad_b = 3;
	       }
	     else if (fmt->style == STYLE_FAR_SOFT_SHADOW)
	       {
		  if (style_pad_r < 4) style_pad_r = 4;
		  if (style_pad_b < 4) style_pad_b = 4;
	       }
	     if (fmt->underline2)
	       c->have_underline2 = 1;
	     else if (fmt->underline)
	       c->have_underline = 1;
	  }
	else if ((n->type == NODE_TEXT) && (n->text))
	  {
	     _layout_text_append(c, fmt, n);
	     if ((c->have_underline2) || (c->have_underline))
	       {
		  if (style_pad_b < c->underline_extend)
		    style_pad_b = c->underline_extend;
		  c->have_underline = 0;
		  c->have_underline2 = 0;
		  c->underline_extend = 0;
	       }
	  }
     }
   if ((c->ln) && (c->ln->items) && (fmt))
     _layout_line_advance(c, fmt);
   while (c->format_stack)
     {
	fmt = c->format_stack->data;
	c->format_stack = evas_list_remove_list(c->format_stack, c->format_stack);
	_format_free(c->obj, fmt);
     }
   for (l = (Evas_Object_List *)c->lines; l; l = l->next)
     {
	Evas_Object_Textblock_Line *ln;
	
	ln = (Evas_Object_Textblock_Line *)l;
	if (ln->line_no == -1)
	  {
	     removes = evas_list_append(removes, ln);
	  }
	else
	  {
	     if ((ln->y + ln->h) > c->hmax) c->hmax = ln->y + ln->h;
	  }
     }
   while (removes)
     {
	Evas_Object_Textblock_Line *ln;
	
	ln = removes->data;
	c->lines = evas_object_list_remove(c->lines, ln);
	removes = evas_list_remove_list(removes, removes);
	_line_free(obj, ln);
     }
     
   if (w_ret) *w_ret = c->wmax;
   if (h_ret) *h_ret = c->hmax;
   if ((o->style_pad.l != style_pad_l) || (o->style_pad.r != style_pad_r) ||
       (o->style_pad.t != style_pad_t) || (o->style_pad.b != style_pad_b))
     {
	Evas_Object_Textblock_Line *lines;
	
	lines = c->lines;
	c->lines = NULL;
	o->style_pad.l = style_pad_l;
	o->style_pad.r = style_pad_r;
	o->style_pad.t = style_pad_t;
	o->style_pad.b = style_pad_b;
	_layout(obj, calc_only, w, h, w_ret, h_ret);
        _lines_clear(obj, lines);
	return;
     }
   if (!calc_only)
     {
	o->lines = c->lines;
	return;
     }
   if (c->lines) _lines_clear(obj, c->lines);
}

static void
_relayout(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *lines;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   lines = o->lines;
   o->lines = NULL;
   o->formatted.valid = 0;
   o->native.valid = 0;
   _layout(obj, 
	   0,
	   obj->cur.geometry.w, obj->cur.geometry.h,
	   &o->formatted.w, &o->formatted.h);
   o->formatted.valid = 1;
   if (lines) _lines_clear(obj, lines);
   o->last_w = obj->cur.geometry.w;
   o->changed = 0;
   o->redraw = 1;
}

static void
_find_layout_item_line_match(Evas_Object *obj, Evas_Object_Textblock_Node *n, int pos, Evas_Object_Textblock_Line **lnr, Evas_Object_Textblock_Item **itr)
{
   Evas_Object_List *l, *ll;
   Evas_Object_Textblock *o;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   for (l = (Evas_Object_List *)o->lines; l; l = l->next)
     {
        Evas_Object_Textblock_Line *ln;

        ln = (Evas_Object_Textblock_Line *)l;
	for (ll = (Evas_Object_List *)ln->items; ll; ll = ll->next)
	  {
	     Evas_Object_Textblock_Item *it;

	     it = (Evas_Object_Textblock_Item *)ll;
	     if (it->source_node == n)
	       {
		  if ((it->source_pos + strlen(it->text)) > pos)
		    {
		       *lnr = ln;
		       *itr = it;
		       return;
		    }
	       }
	  }
     }
}

static void
_find_layout_format_item_line_match(Evas_Object *obj, Evas_Object_Textblock_Node *n, Evas_Object_Textblock_Line **lnr, Evas_Object_Textblock_Format_Item **fir)
{
   Evas_Object_List *l, *ll;
   Evas_Object_Textblock *o;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   for (l = (Evas_Object_List *)o->lines; l; l = l->next)
     {
        Evas_Object_Textblock_Line *ln;

        ln = (Evas_Object_Textblock_Line *)l;
	for (ll = (Evas_Object_List *)ln->format_items; ll; ll = ll->next)
	  {
	     Evas_Object_Textblock_Format_Item *fi;

	     fi = (Evas_Object_Textblock_Format_Item *)ll;
	     if (fi->source_node == n)
	       {
		  *lnr = ln;
		  *fir = fi;
		  return;
	       }
	  }
     }
}

static Evas_Object_Textblock_Line *
_find_layout_line_num(Evas_Object *obj, int line)
{
   Evas_Object_List *l;
   Evas_Object_Textblock *o;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   for (l = (Evas_Object_List *)o->lines; l; l = l->next)
     {
        Evas_Object_Textblock_Line *ln;

        ln = (Evas_Object_Textblock_Line *)l;
	if (ln->line_no == line) return ln;
     }
   return NULL;
}

/**
 * Adds a textblock to the given evas.
 * @param   e The given evas.
 * @return  The new textblock object.
 * @todo Find a documentation group to put this under.
 */
Evas_Object *
evas_object_textblock2_add(Evas *e)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new();
   evas_object_textblock_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

Evas_Textblock_Style *
evas_textblock2_style_new(void)
{
   Evas_Textblock_Style *ts;
   
   ts = calloc(1, sizeof(Evas_Textblock_Style));
   return ts;
}

void
evas_textblock2_style_free(Evas_Textblock_Style *ts)
{
   if (!ts) return;
   if (ts->objects)
     {
	ts->delete_me = 1;
	return;
     }
   _style_clear(ts);
   free(ts);
}

void
evas_textblock2_style_set(Evas_Textblock_Style *ts, const char *text)
{
   Evas_List *l;
   
   if (!ts) return;
   _style_clear(ts);
   if (text) ts->style_text = strdup(text);

   if (ts->style_text)
     {
	// format MUST be KEY='VALUE'[KEY='VALUE']...
	char *p;
	char *key_start, *key_stop, *val_start, *val_stop;
	
	key_start = key_stop = val_start = val_stop = NULL;
	p = ts->style_text;
	while (*p)
	  {
	     if (!key_start)
	       {
		  if (!isspace(*p))
		    key_start = p;
	       }
	     else if (!key_stop)
	       {
		  if ((*p == '=') || (isspace(*p)))
		    key_stop = p;
	       }
	     else if (!val_start)
	       {
		  if (((*p) == '\'') && (*(p + 1)))
		    val_start = p + 1;
	       }
	     else if (!val_stop)
	       {
		  if (((*p) == '\'') && (p > ts->style_text) && (p[-1] != '\\'))
		    val_stop = p;
	       }
	     if ((key_start) && (key_stop) && (val_start) && (val_stop))
	       {
		  char *tags, *replaces;
		  Evas_Object_Style_Tag *tag;
		  
		  tags = malloc(key_stop - key_start + 1);
		  if (tags)
		    {
		       tags[key_stop - key_start] = 0;
		       strncpy(tags, key_start, key_stop - key_start);
		       tags[key_stop - key_start] = 0;
		    }
		  
		  replaces = malloc(val_stop - val_start + 1);
		  if (replaces)
		    {
		       replaces[val_stop - val_start] = 0;
		       strncpy(replaces, val_start, val_stop - val_start);
		       replaces[val_stop - val_start] = 0;
		    }
		  if ((tags) && (replaces))
		    {
		       if (!strcmp(tags, "DEFAULT"))
			 {
			    ts->default_tag = replaces;
			    free(tags);
			 }
		       else
			 {
			    tag = calloc(1, sizeof(Evas_Object_Style_Tag));
			    if (tag)
			      {
				 tag->tag = tags;
				 tag->replace = replaces;
				 ts->tags = evas_object_list_append(ts->tags, tag);
			      }
			    else
			      {
				 free(tags);
				 free(replaces);
			      }
			 }
		    }
		  else
		    {
		       if (tags) free(tags);
		       if (replaces) free(replaces);
		    }
		  key_start = key_stop = val_start = val_stop = NULL;
	       }
	     p++;
	  }
     }
   
   for (l = ts->objects; l; l = l->next)
     {
	Evas_Object *obj;
	Evas_Object_Textblock *o;
	
	obj = l->data;
	o = (Evas_Object_Textblock *)(obj->object_data);
	if (o->markup_text)
	  evas_object_textblock2_text_markup_set(obj, o->markup_text);
     }
}

const char *
evas_textblock2_style_get(Evas_Textblock_Style *ts)
{
   if (!ts) return NULL;
   return ts->style_text;
}

/* textblock styles */
void
evas_object_textblock2_style_set(Evas_Object *obj, Evas_Textblock_Style *ts)
{
   TB_HEAD();
   if (ts == o->style) return;
   if ((ts) && (ts->delete_me)) return;
   if (o->style)
     {
	Evas_Textblock_Style *old_ts;
	
	old_ts = o->style;
	old_ts->objects = evas_list_remove(old_ts->objects, obj);
	if ((old_ts->delete_me) && (!old_ts->objects))
	  evas_textblock2_style_free(old_ts);
     }
   if (ts)
     {
	ts->objects = evas_list_append(ts->objects, obj);
	o->style = ts;
     }
   else
     {
	o->style = NULL;
     }
   if (o->markup_text)
     evas_object_textblock2_text_markup_set(obj, o->markup_text);
}

const Evas_Textblock_Style *
evas_object_textblock2_style_get(Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   return o->style;
}

void
evas_object_textblock2_text_markup_set(Evas_Object *obj, const char *text)
{
   TB_HEAD();
   if ((text != o->markup_text) && (o->markup_text))
     {
	free(o->markup_text);
	o->markup_text = NULL;
     }
   _nodes_clear(obj);
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->changed = 1;
   evas_object_change(obj);
   if (!o->style)
     {
	if (text != o->markup_text)
	  {
	     if (text) o->markup_text = strdup(text);
	  }
	return;
     }
   evas_textblock2_cursor_node_first(o->cursor);
   if (text)
     {
	char *s, *p;
	char *tag_start, *tag_end, *esc_start, *esc_end;
	
	tag_start = tag_end = esc_start = esc_end = NULL;
	p = (char *)text;
	s = p;
	for (;;)
	  {
	     if ((*p == 0) || 
		 (tag_end) || (esc_end) || 
		 (tag_start) || (esc_start))
	       {
		  if (tag_end)
		    {
		       char *ttag, *match;
		       
		       ttag = malloc(tag_end - tag_start);
		       if (ttag)
			 {
			    strncpy(ttag, tag_start + 1, tag_end - tag_start - 1);
			    ttag[tag_end - tag_start - 1] = 0;
			    match = _style_match_tag(o->style, ttag);
			    if (match)
			      evas_textblock2_cursor_format_append(o->cursor, match);
			    else
			      {
				 if (ttag[0] == '/')
				   evas_textblock2_cursor_format_append(o->cursor, "-");
				 else
				   {
				      char *ttag2;
				      
				      ttag2 = malloc(strlen(ttag) + 2 + 1);
				      if (ttag2)
					{
					   strcpy(ttag2, "+ ");
					   strcat(ttag2, ttag);
					   evas_textblock2_cursor_format_append(o->cursor, ttag2);
					   free(ttag2);
					}
				   }
			      }
			    free(ttag);
			 }
		       tag_start = tag_end = NULL;
		    }
		  else if (esc_end)
		    {
		       int i;
		       
		       for (i = 0; i < (sizeof(_escapes) / sizeof(char *)); i += 2)
			 {
			    if (!strncmp(_escapes[i], esc_start, 
					 esc_end - esc_start + 1))
			      {
				 evas_textblock2_cursor_text_append(o->cursor, _escapes[i + 1]);
				 break;
			      }
			 }
		       esc_start = esc_end = NULL;
		    }
		  else if (*p == 0)
		    {
		       _append_text_run(o, s, p);
		       s = NULL;
		    }
		  if (*p == 0)
		    break;
	       }
	     if (*p == '<')
	       {
		  if (!esc_start)
		    {
		       tag_start = p;
		       tag_end = NULL;
		       _append_text_run(o, s, p);
		       s = NULL;
		    }
	       }
	     else if (*p == '>')
	       {
		  if (tag_start)
		    {
		       tag_end = p;
		       s = p + 1;
		    }
	       }
	     else if (*p == '&')
	       {
		  if (!tag_start)
		    {
		       esc_start = p;
		       esc_end = NULL;
		       _append_text_run(o, s, p);
		       s = NULL;
		    }
	       }
	     else if (*p == ';')
	       {
		  if (esc_start)
		    {
		       esc_end = p;
		       s = p + 1;
		    }
	       }
	     p++;
	  }
	if (text != o->markup_text)
	  o->markup_text = strdup(text);
     }
     {
	Evas_List *l;
	
	evas_textblock2_cursor_node_first(o->cursor);
	for (l = o->cursors; l; l = l->next)
	  evas_textblock2_cursor_node_first(l->data);
     }
}

const char *
evas_object_textblock2_text_markup_get(Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   if (!o->markup_text)
     {
	Evas_Textblock_Cursor *cur;
	int slen = 0;
	int salloc = 0;
	
	cur = evas_object_textblock2_cursor_new(obj);
	evas_textblock2_cursor_node_first(cur);
	do
	  {
	     char *s, *p, *ps;
	     
	     s = (char *)evas_textblock2_cursor_node_text_get(cur);
	     if (s)
	       {
		  p = s;
		  ps = p;
		  for (;;)
		    {
		       if (*p == 0)
			 {
			    o->markup_text = _strbuf_append(o->markup_text,
							    ps,
							    &slen, &salloc);
			    break;
			 }
		       else if (*p == '<')
			 {
			    o->markup_text = _strbuf_append_n(o->markup_text,
							      ps, p - ps,
							      &slen, &salloc);
			    o->markup_text = _strbuf_append(o->markup_text,
							    "&lt;",
							    &slen, &salloc);
			    ps = p + 1;
			 }
		       else if (*p == '>')
			 {
			    o->markup_text = _strbuf_append_n(o->markup_text,
							      ps, p - ps,
							      &slen, &salloc);
			    o->markup_text = _strbuf_append(o->markup_text,
							    "&gt;",
							    &slen, &salloc);
			    ps = p + 1;
			 }
		       else if (*p == '&')
			 {
			    o->markup_text = _strbuf_append_n(o->markup_text,
							      ps, p - ps,
							      &slen, &salloc);
			    o->markup_text = _strbuf_append(o->markup_text,
							    "&amp;",
							    &slen, &salloc);
			    ps = p + 1;
			 }
		       /* FIXME: learn how to do all the other escapes */
		       /* FIXME: strip extra whitespace ala HTML */
		       p++;
		    }
	       }
	     else
	       {
		  s = (char *)evas_textblock2_cursor_node_format_get(cur);
		  if (s)
		    {
		       char *stag;
		       
		       o->markup_text = _strbuf_append(o->markup_text,
						       "<",
						       &slen, &salloc);
		       stag = _style_match_replace(o->style, s);
		       if (stag)
			 o->markup_text = _strbuf_append(o->markup_text,
							 stag,
							 &slen, &salloc);
		       else
			 o->markup_text = _strbuf_append(o->markup_text,
							 s,
							 &slen, &salloc);
		       o->markup_text = _strbuf_append(o->markup_text,
						       ">",
						       &slen, &salloc);
		    }
	       }
	     
	  }
	while (evas_textblock2_cursor_node_next(cur));
	evas_textblock2_cursor_free(cur);
     }
   return o->markup_text;
}

/* cursors */
const Evas_Textblock_Cursor *
evas_object_textblock2_cursor_get(Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   return o->cursor;
}

Evas_Textblock_Cursor *
evas_object_textblock2_cursor_new(Evas_Object *obj)
{
   Evas_Textblock_Cursor *cur;
   
   TB_HEAD_RETURN(NULL);
   cur = calloc(1, sizeof(Evas_Textblock_Cursor));
   cur->obj = obj;
   cur->node = o->nodes;
   cur->pos = 0;
   o->cursors = evas_list_append(o->cursors, cur);
   return cur;
}

void
evas_textblock2_cursor_free(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;

   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (cur == o->cursor) return;
   o->cursors = evas_list_remove(o->cursors, cur);
   free(cur);
}

void
evas_textblock2_cursor_node_first(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   
   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   cur->node = o->nodes;
   cur->pos = 0;
}

void
evas_textblock2_cursor_node_last(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   
   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (o->nodes)
     {
	cur->node = (Evas_Object_Textblock_Node *)(((Evas_Object_List *)(o->nodes))->last);
	cur->pos = 0;
	evas_textblock2_cursor_char_last(cur);
     }
   else
     {
	cur->node = NULL;
	cur->pos = 0;
     }
}

Evas_Bool
evas_textblock2_cursor_node_next(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   
   if (!cur) return 0;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!cur->node) return 0;
   if (((Evas_Object_List *)(cur->node))->next)
     {
	cur->node = (Evas_Object_Textblock_Node *)(((Evas_Object_List *)(cur->node))->next);
	cur->pos = 0;
	return 1;
     }
   return 0;
}

Evas_Bool
evas_textblock2_cursor_node_prev(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   
   if (!cur) return 0;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!cur->node) return 0;
   if (((Evas_Object_List *)(cur->node))->prev)
     {
	cur->node = (Evas_Object_Textblock_Node *)(((Evas_Object_List *)(cur->node))->prev);
	evas_textblock2_cursor_char_last(cur);
	return 1;
     }
   return 0;
}

Evas_Bool
evas_textblock2_cursor_char_next(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   int index, tindex, ch;
   
   if (!cur) return 0;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!cur->node) return 0;
   if (cur->node->type == NODE_FORMAT) return 0;
   index = cur->pos;
   ch = evas_common_font_utf8_get_next((unsigned char *)(cur->node->text), &index);
   if ((ch == 0) || (index < 0)) return 0;
   if (cur->node->text[index] == 0) return 0;
   tindex = index;
   cur->pos = index;
   return 1;
}

Evas_Bool
evas_textblock2_cursor_char_prev(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   int index, ch;
   
   if (!cur) return 0;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!cur->node) return 0;
   if (cur->node->type == NODE_FORMAT) return 0;
   index = cur->pos;
   ch = evas_common_font_utf8_get_prev((unsigned char *)(cur->node->text), &index);
   if ((ch == 0) || (index < 0)) return 0;
   cur->pos = index;
   return 1;
}

void
evas_textblock2_cursor_char_first(Evas_Textblock_Cursor *cur)
{
   if (!cur) return;
   cur->pos = 0;
}

void
evas_textblock2_cursor_char_last(Evas_Textblock_Cursor *cur)
{
   int index;
   
   if (!cur) return;
   if (!cur->node) return;
   if (cur->node->type == NODE_FORMAT)
     {
	cur->pos = 0;
	return;
     }
   index = evas_common_font_utf8_get_last((unsigned char *)cur->node->text, cur->node->len);
   if (index < 0) cur->pos = 0;
   cur->pos = index;
}

void
evas_textblock2_cursor_line_first(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;
   Evas_Object_Textblock_Format_Item *fi = NULL;
   
   if (!cur) return;
   if (!cur->node) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   if (cur->node->type == NODE_FORMAT)
     _find_layout_format_item_line_match(cur->obj, cur->node, &ln, &fi);
   else
     _find_layout_item_line_match(cur->obj, cur->node, cur->pos, &ln, &it);
   if (!ln) return;
   it = (Evas_Object_Textblock_Item *)ln->items;
   fi = (Evas_Object_Textblock_Format_Item *)ln->format_items;
   if ((it) && (fi))
     {
	if (it->x < fi->x) fi = NULL;
	else it = NULL;
     }
   if (it)
     {
	cur->pos = it->source_pos;
	cur->node = it->source_node;
     }
   else if (fi)
     {
	cur->pos = 0;
	cur->node = fi->source_node;
     }
}

void
evas_textblock2_cursor_line_last(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;
   Evas_Object_Textblock_Format_Item *fi = NULL;
   
   if (!cur) return;
   if (!cur->node) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   if (cur->node->type == NODE_FORMAT)
     _find_layout_format_item_line_match(cur->obj, cur->node, &ln, &fi);
   else
     _find_layout_item_line_match(cur->obj, cur->node, cur->pos, &ln, &it);
   if (!ln) return;
   if (ln->items)
     it = (Evas_Object_Textblock_Item *)(((Evas_Object_List *)ln->items)->last);
   else
     it = NULL;
   if (ln->format_items)
     fi = (Evas_Object_Textblock_Format_Item *)(((Evas_Object_List *)ln->format_items)->last);
   else
     fi = NULL;
   if ((it) && (fi))
     {
	if ((it->x + it->w) > (fi->x + fi->w)) fi = NULL;
	else it = NULL;
     }
   if (it)
     {
	int index;
	
	cur->pos = it->source_pos;
	cur->node = it->source_node;
	index = evas_common_font_utf8_get_last((unsigned char *)it->text, strlen(it->text));
	if (index >= 0) cur->pos += index;
     }
   else if (fi)
     {
	cur->pos = 0;
	cur->node = fi->source_node;
     }
}

int
evas_textblock2_cursor_pos_get(Evas_Textblock_Cursor *cur)
{
   if (!cur) return -1;
   return cur->pos;
}

void
evas_textblock2_cursor_pos_set(Evas_Textblock_Cursor *cur, int pos)
{
   if (!cur) return;
   if (!cur->node) return;
   if (cur->node->type == NODE_FORMAT) pos = 0;
   if (pos < 0) pos = 0;
   else if (pos > cur->node->len) pos = cur->node->len;
   cur->pos = pos;
}

Evas_Bool
evas_textblock2_cursor_line_set(Evas_Textblock_Cursor *cur, int line)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock_Item *it;
   Evas_Object_Textblock_Format_Item *fi;
   
   if (!cur) return 0;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);

   ln = _find_layout_line_num(cur->obj, line);
   if (!ln) return 0;
   it = (Evas_Object_Textblock_Item *)ln->items;
   fi = (Evas_Object_Textblock_Format_Item *)ln->format_items;
   if ((it) && (fi))
     {
	if (it->x < fi->x) fi = NULL;
	else it = NULL;
     }
   if (it)
     {
	cur->pos = it->source_pos;
	cur->node = it->source_node;
     }
   else if (fi)
     {
	cur->pos = 0;
	cur->node = fi->source_node;
     }
   return 1;
}

int
evas_textblock2_cursor_compare(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2)
{
   Evas_Object_List *l1, *l2;
   
   if (!cur1) return 0;
   if (!cur2) return 0;
   if (cur1->obj != cur2->obj) return 0;
   if ((!cur1->node) || (!cur2->node)) return 0;
   if (cur1->node == cur2->node)
     {
	if (cur1->pos < cur2->pos) return -1; /* cur1 < cur2 */
	else if (cur1->pos > cur2->pos) return 1; /* cur2 < cur1 */
	return 0; /* cur1 == cur2 */
     }
   for (l1 = (Evas_Object_List *)cur1->node, 
	l2 = (Evas_Object_List *)cur1->node; (l1) || (l2);)
     {
	if (l1 == (Evas_Object_List *)cur2->node) return 1; /* cur2 < cur 1 */
	else if (l2 == (Evas_Object_List *)cur2->node) return -1; /* cur1 < cur 2 */
	else if (!l1) return -1; /* cur1 < cur 2 */
	else if (!l2) return 1; /* cur2 < cur 1 */
	if (l1) l1 = l1->prev;
	if (l2) l2 = l2->next;
     }
   return 0;
}

void
evas_textblock2_cursor_copy(Evas_Textblock_Cursor *cur, Evas_Textblock_Cursor *cur_dest)
{
   if (!cur) return;
   if (!cur_dest) return;
   if (cur->obj != cur_dest->obj) return;
   cur_dest->pos = cur->pos;
   cur_dest->node = cur->node;
}


/* text controls */
void
evas_textblock2_cursor_text_append(Evas_Textblock_Cursor *cur, const char *text)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node *n;
   int index, ch;
   
   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
     {
	Evas_List *l;

	if (cur != o->cursor)
	  {
	     if (cur->node == o->cursor->node)
	       {
		  if (o->cursor->pos > cur->pos)
		    o->cursor->pos += strlen(text);
	       }
	  }
	for (l = o->cursors; l; l = l->next)
	  {
	     if (l->data != cur)
	       {
		  if (cur->node == ((Evas_Textblock_Cursor *)l->data)->node)
		    {
		       if (((Evas_Textblock_Cursor *)l->data)->pos > cur->pos)
			 ((Evas_Textblock_Cursor *)l->data)->pos += strlen(text);
		    }
	       }
	  }
     }
   n = cur->node;
   if ((!n) || (n->type == NODE_FORMAT))
     {
	n = calloc(1, sizeof(Evas_Object_Textblock_Node));
	n->type = NODE_TEXT;
	o->nodes = evas_object_list_append(o->nodes, n);
     }
   cur->node = n;
   index = cur->pos;
   if (n->text)
     {
	ch = evas_common_font_utf8_get_next((unsigned char *)(n->text), &index);
	if (ch != 0)
	  cur->pos = index;
     }
   if (cur->pos >= (n->len - 1))
     n->text = _strbuf_append(n->text, (char *)text, &(n->len), &(n->alloc));
   else
     n->text = _strbuf_insert(n->text, (char *)text, cur->pos, &(n->len), &(n->alloc));
   cur->pos += strlen(text);
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->changed = 1;
   evas_object_change(cur->obj);
}

void
evas_textblock2_cursor_text_prepend(Evas_Textblock_Cursor *cur, const char *text)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node *n;
   int index, ch;
   
   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
     {
	Evas_List *l;

	if (cur != o->cursor)
	  {
	     if (cur->node == o->cursor->node)
	       {
		  if ((o->cursor->node) &&
		      (o->cursor->node->type == NODE_TEXT) &&
		      (o->cursor->pos >= cur->pos))
		    o->cursor->pos += strlen(text);
	       }
	  }
	for (l = o->cursors; l; l = l->next)
	  {
	     if (l->data != cur)
	       {
		  if (cur->node == ((Evas_Textblock_Cursor *)l->data)->node)
		    {
		       if ((((Evas_Textblock_Cursor *)l->data)->node) &&
			   (((Evas_Textblock_Cursor *)l->data)->node->type == NODE_TEXT) &&
			   (((Evas_Textblock_Cursor *)l->data)->pos >= cur->pos))
			 ((Evas_Textblock_Cursor *)l->data)->pos += strlen(text);
		    }
	       }
	  }
     }
   n = cur->node;
   if ((!n) || (n->type == NODE_FORMAT))
     {
	n = calloc(1, sizeof(Evas_Object_Textblock_Node));
	n->type = NODE_TEXT;
	o->nodes = evas_object_list_append(o->nodes, n);
     }
   cur->node = n;
   index = cur->pos;
   if (cur->pos >= (n->len - 1))
     n->text = _strbuf_append(n->text, (char *)text, &(n->len), &(n->alloc));
   else
     n->text = _strbuf_insert(n->text, (char *)text, cur->pos, &(n->len), &(n->alloc));
   cur->pos += strlen(text);
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->changed = 1;
   evas_object_change(cur->obj);
}

void
evas_textblock2_cursor_format_append(Evas_Textblock_Cursor *cur, const char *format)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node *n, *nc, *n2;
   
   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   nc = cur->node;
   n = calloc(1, sizeof(Evas_Object_Textblock_Node));
   n->type = NODE_FORMAT;
   n->text = strdup(format);
   n->len = strlen(n->text);
   n->alloc = n->len + 1;
   if (!nc)
     {
	o->nodes = evas_object_list_append(o->nodes, n);
     }
   else if (nc->type == NODE_FORMAT)
     {
	o->nodes = evas_object_list_append_relative(o->nodes, n, nc);
     }
   else if (nc->type == NODE_TEXT)
     {
	int index, ch = 0;
	char *ts;
	
	index = cur->pos;
	if (nc->text)
	  {
	     ch = evas_common_font_utf8_get_next((unsigned char *)(nc->text), &index);
	     if (ch != 0)
	       cur->pos = index;
	  }
	o->nodes = evas_object_list_append_relative(o->nodes, n, nc);
	if ((ch != 0) && (cur->pos < nc->len))
	  {
	     n2 = calloc(1, sizeof(Evas_Object_Textblock_Node));
	     n2->type = NODE_TEXT;
	     n2->text = _strbuf_append(n2->text, (char *)(nc->text + cur->pos), &(n2->len), &(n2->alloc));
	     o->nodes = evas_object_list_append_relative(o->nodes, n2, n);
	     
	     *(nc->text + cur->pos) = 0;
	     nc->len = cur->pos;
	     ts = realloc(nc->text, nc->len + 1);
	     if (ts)
	       {
		  nc->text = ts;
		  nc->alloc = nc->len + 1;
	       }
	  }
     }
   cur->node = n;
   cur->pos = 0;
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->changed = 1;
   evas_object_change(cur->obj);
}

void
evas_textblock2_cursor_format_prepend(Evas_Textblock_Cursor *cur, const char *format)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node *n, *nc, *n2;
   
   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   nc = cur->node;
   n = calloc(1, sizeof(Evas_Object_Textblock_Node));
   n->type = NODE_FORMAT;
   n->text = strdup(format);
   n->len = strlen(n->text);
   n->alloc = n->len + 1;
   if (!nc)
     {
	o->nodes = evas_object_list_prepend(o->nodes, n);
     }
   else if (nc->type == NODE_FORMAT)
     {
	o->nodes = evas_object_list_prepend_relative(o->nodes, n, nc);
     }
   else if (nc->type == NODE_TEXT)
     {
	char *ts;
	
	if (cur->pos == 0)
	  o->nodes = evas_object_list_prepend_relative(o->nodes, n, nc);
	else
	  o->nodes = evas_object_list_append_relative(o->nodes, n, nc);
	if ((cur->pos < nc->len) && (cur->pos != 0))
	  {
	     n2 = calloc(1, sizeof(Evas_Object_Textblock_Node));
	     n2->type = NODE_TEXT;
	     n2->text = _strbuf_append(n2->text, (char *)(nc->text + cur->pos), &(n2->len), &(n2->alloc));
	     o->nodes = evas_object_list_append_relative(o->nodes, n2, n);
	     
	     *(nc->text + cur->pos) = 0;
	     nc->len = cur->pos;
	     ts = realloc(nc->text, nc->len + 1);
	     if (ts)
	       {
		  nc->text = ts;
		  nc->alloc = nc->len + 1;
	       }
	  }
     }
   cur->node = n;
   cur->pos = 0;
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->changed = 1;
   evas_object_change(cur->obj);
}

void
evas_textblock2_cursor_node_delete(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node *n, *n2;
   
   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   n = cur->node;
   n2 = (Evas_Object_Textblock_Node *)(((Evas_Object_List *)n)->next);
   if (n2)
     {
	cur->node = n2;
	cur->pos = 0;
     }
   else
     {
	n2 = (Evas_Object_Textblock_Node *)(((Evas_Object_List *)n)->prev);
	cur->node = n2;
	cur->pos = 0;
	evas_textblock2_cursor_char_last(cur);
     }

     {
	Evas_List *l;

	if (cur != o->cursor)
	  {
	     if (n == o->cursor->node)
	       {
		  o->cursor->node = cur->node;
		  o->cursor->pos = cur->pos;
	       }
	  }
	for (l = o->cursors; l; l = l->next)
	  {
	     if (l->data != cur)
	       {
		  if (n == ((Evas_Textblock_Cursor *)l->data)->node)
		    {
		       ((Evas_Textblock_Cursor *)l->data)->node = cur->node;
		       ((Evas_Textblock_Cursor *)l->data)->pos = cur->pos;
		    }
	       }
	  }
     }
   
   o->nodes = evas_object_list_remove(o->nodes, n);
   if (n->text) free(n->text);
   free(n);
   
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->changed = 1;
   evas_object_change(cur->obj);
}

void
evas_textblock2_cursor_char_delete(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node *n, *n2;
   int chr, index, ppos;
   
   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   n = cur->node;
   if (n->type == NODE_FORMAT)
     {
	evas_textblock2_cursor_node_delete(cur);
	return;
     }
   index = cur->pos;
   chr = evas_common_font_utf8_get_next((unsigned char *)n->text, &index);
   if (chr == 0) return;
   ppos = cur->pos;
   n->text = _strbuf_remove(n->text, cur->pos, index, &(n->len), &(n->alloc));
   if (!n->text)
     {
	evas_textblock2_cursor_node_delete(cur);
	return;
     }
   if (cur->pos == n->len)
     {
	n2 = (Evas_Object_Textblock_Node *)(((Evas_Object_List *)n)->next);
	if (n2)
	  {
	     cur->node = n2;
	     cur->pos = 0;
	  }
	else
	  {
	     cur->pos = 0;
	     evas_textblock2_cursor_char_last(cur);
	  }
     }
   
     {
	Evas_List *l;

	if (cur != o->cursor)
	  {
	     if ((n == o->cursor->node) &&
		 (o->cursor->pos > ppos))
	       {
		  o->cursor->pos -= (index - ppos);
	       }
	  }
	for (l = o->cursors; l; l = l->next)
	  {
	     if (l->data != cur)
	       {
		  if ((n == ((Evas_Textblock_Cursor *)l->data)->node) &&
		      (((Evas_Textblock_Cursor *)l->data)->pos > ppos))
		    {
		       ((Evas_Textblock_Cursor *)l->data)->pos -= (index - ppos);
		    }
	       }
	  }
     }
   
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->changed = 1;
   evas_object_change(cur->obj);
}

void
evas_textblock2_cursor_range_delete(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node *n1, *n2, *n, *tn;
   Evas_Object_List *l;
   
   int chr, index;
   
   if (!cur1) return;
   if (!cur2) return;
   if (cur1->obj != cur2->obj) return;
   o = (Evas_Object_Textblock *)(cur1->obj->object_data);
   if (evas_textblock2_cursor_compare(cur1, cur2) > 0)
     {
	Evas_Textblock_Cursor *tc;
	
	tc = cur1;
	cur1 = cur2;
	cur2 = tc;
     }
   n1 = cur1->node;
   n2 = cur2->node;
   index = cur2->pos;
   chr = evas_common_font_utf8_get_next((unsigned char *)n2->text, &index);
   if (chr == 0) return;
   if (n1 == n2)
     {
	if (cur1->pos == cur2->pos)
	  {
	     evas_textblock2_cursor_char_delete(cur1);
	     evas_textblock2_cursor_copy(cur1, cur2);
	     return;
	  }
	n1->text = _strbuf_remove(n1->text, cur1->pos, index, &(n1->len), &(n1->alloc));
	if (!n1->text)
	  {
	     evas_textblock2_cursor_node_delete(cur1);
	     evas_textblock2_cursor_copy(cur1, cur2);
	     return;
	  }
	if (cur1->pos >= n1->len)
	  {
	     n2 = (Evas_Object_Textblock_Node *)(((Evas_Object_List *)n1)->next);
	     if (n2)
	       {
		  cur1->node = n2;
		  cur1->pos = 0;
	       }
	     else
	       {
		  cur1->pos = 0;
		  evas_textblock2_cursor_char_last(cur1);
	       }
	  }
	evas_textblock2_cursor_copy(cur1, cur2);
     }
   else
     {
	Evas_List *removes, *format_hump;
	Evas_Textblock_Cursor tcur;
	
	tcur.node = n2;
	tcur.pos = 0;
	index = cur2->pos;
	chr = evas_common_font_utf8_get_next((unsigned char *)n2->text, &index);
	if ((chr == 0) || (index >= n2->len))
	  {
	     tcur.node = (Evas_Object_Textblock_Node *)((Evas_Object_List *)n2)->next;
	     tcur.pos = 0;
	     if (!tcur.node)
	       {
		  if (cur1->pos != 0)
		    {
		       tcur.node = n1;
		       index = cur1->pos;
		       chr = evas_common_font_utf8_get_prev((unsigned char *)n2->text, &index);
		       tcur.pos = index;
		    }
		  else
		    {
		       tcur.node = (Evas_Object_Textblock_Node *)((Evas_Object_List *)n1)->prev;
		       if (tcur.node->type == NODE_TEXT)
			 tcur.pos = evas_common_font_utf8_get_last((unsigned char *)tcur.node->text, tcur.node->len);
		       else
			 tcur.pos = 0;
		    }
	       }
	  }
	n1->text = _strbuf_remove(n1->text, cur1->pos, n1->len, &(n1->len), &(n1->alloc));
	removes = NULL;
	for (l = ((Evas_Object_List *)n1)->next; l != (Evas_Object_List *)n2; l = l->next)
	  removes = evas_list_append(removes, l);
	if (n1->type == NODE_TEXT)
	  {
	     if (!n1->text)
	       evas_textblock2_cursor_node_delete(cur1);
	  }
	else
	  {
	     if (n1->text[0] == '+')
	       format_hump = evas_list_append(format_hump, n1);
	     else
	       {
		  o->nodes = evas_object_list_remove(o->nodes, n1);
		  if (n1->text) free(n1->text);
		  free(n1);
	       }
	  }
	format_hump = NULL;
	while (removes)
	  {
	     n = removes->data;
	     if (n->type == NODE_TEXT)
	       {
		  o->nodes = evas_object_list_remove(o->nodes, n);
		  if (n->text) free(n->text);
		  free(n);
	       }
	     else
	       {
		  if (n->text[0] == '+')
		    {
		       format_hump = evas_list_append(format_hump, n);
		    }
		  else if (n->text[0] == '-')
		    {
		       tn = evas_list_data(evas_list_last(format_hump));
		       if (tn)
			 {
			    format_hump = evas_list_remove_list(format_hump, evas_list_last(format_hump));
			    o->nodes = evas_object_list_remove(o->nodes, tn);
			    if (tn->text) free(tn->text);
			    free(tn);
			    o->nodes = evas_object_list_remove(o->nodes, n);
			    if (n->text) free(n->text);
			    free(n);
			 }
		    }
		  else
		    {
		       o->nodes = evas_object_list_remove(o->nodes, n);
		       if (n->text) free(n->text);
		       free(n);
		    }
	       }
	     removes = evas_list_remove_list(removes, removes);
	  }
        if (n2->type == NODE_TEXT)
	  {
	     n2->text = _strbuf_remove(n2->text, 0, index, &(n2->len), &(n2->alloc));
	     if (!n2->text)
	       evas_textblock2_cursor_node_delete(cur2);
	  }
	else
	  {
	     if (n2->text[0] == '-')
	       {
		  o->nodes = evas_object_list_remove(o->nodes, n2);
		  if (n2->text) free(n2->text);
		  free(n2);
		  n = evas_list_data(evas_list_last(format_hump));
		  if (n)
		    {
		       o->nodes = evas_object_list_remove(o->nodes, n);
		       if (n->text) free(n->text);
		       free(n);
		    }
	       }
	     else
	       {
		  o->nodes = evas_object_list_remove(o->nodes, n2);
		  if (n2->text) free(n2->text);
		  free(n2);
	       }
	  }
	if (format_hump) format_hump = evas_list_free(format_hump);
	cur1->node = tcur.node;
	cur1->pos = tcur.pos;
	cur2->node = tcur.node;
	cur2->pos = tcur.pos;
     }
   
   /* FIXME: adjust cursors that are affected by the change */
   /* this is temporary just avoiding segv's - it sets all other cursors to
    * the same pos as cur1 and cur2
    */
     {
	Evas_List *l;

	if ((cur1 != o->cursor) && (cur2 != o->cursor))
	  {
	     evas_textblock2_cursor_copy(cur1, o->cursor);
	  }
	for (l = o->cursors; l; l = l->next)
	  {
	     if ((l->data != cur1) && (l->data != cur2))
	       {
		  evas_textblock2_cursor_copy(cur1, l->data);
	       }
	  }
     }
   
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->changed = 1;
   evas_object_change(cur1->obj);
}

const char *
evas_textblock2_cursor_node_text_get(Evas_Textblock_Cursor *cur)
{
   if (!cur) return NULL;
   if (!cur->node) return NULL;
   if (cur->node->type == NODE_TEXT)
     {
	return cur->node->text;
     }
   return NULL;
}

int
evas_textblock2_cursor_node_text_length_get(Evas_Textblock_Cursor *cur)
{
   if (!cur) return 0;
   if (!cur->node) return 0;
   if (cur->node->type == NODE_TEXT)
     {
	return cur->node->len;
     }
   return 0;
}

const char *
evas_textblock2_cursor_node_format_get(Evas_Textblock_Cursor *cur)
{
   if (!cur) return NULL;
   if (!cur->node) return NULL;
   if (cur->node->type == NODE_FORMAT)
     {
	return cur->node->text;
     }
   return NULL;
}

char *
evas_textblock2_cursor_range_text_get(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2, Evas_Textblock_Text_Type format)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node *n1, *n2, *n;
   Evas_Object_List *l;
   char *str = NULL, *s;
   int len = 0, alloc = 0, chr, index;
   
   if (!cur1) return NULL;
   if (!cur2) return NULL;
   if (cur1->obj != cur2->obj) return NULL;
   o = (Evas_Object_Textblock *)(cur1->obj->object_data);
   if (evas_textblock2_cursor_compare(cur1, cur2) > 0)
     {
	Evas_Textblock_Cursor *tc;
	
	tc = cur1;
	cur1 = cur2;
	cur2 = tc;
     }
   n1 = cur1->node;
   n2 = cur2->node;
   index = cur2->pos;
   chr = evas_common_font_utf8_get_next((unsigned char *)n2->text, &index);
   for (l = (Evas_Object_List *)n1; ;l = l->next)
     {
	n = (Evas_Object_Textblock_Node *)l;
	if (n->type == NODE_TEXT)
	  {
	     s = n->text;
	     if ((n == n1) && (n == n2))
	       {
		  s += cur1->pos;
		  str = _strbuf_append_n(str, s, index - cur1->pos, &len, &alloc);
		  
	       }
	     else if (n == n1)
	       {
		  s += cur1->pos;
		  str = _strbuf_append(str, s, &len, &alloc);
	       }
	     else if (n == n2)
	       {
		  str = _strbuf_append_n(str, s, index, &len, &alloc);
	       }
	     else
	       {
		  str = _strbuf_append(str, s, &len, &alloc);
	       }
	  }
	else
	  {
	     if (format == EVAS_TEXTBLOCK_TEXT_PLAIN)
	       {
		  s = n->text;
		  while (*s)
		    {
		       if (*s == '\n')
			 str = _strbuf_append(str, "\n", &len, &alloc);
		       else if (*s == '\t')
			 str = _strbuf_append(str, "\t", &len, &alloc);
		       s++;
		    }
	       }
	  }
	if (l == (Evas_Object_List *)n2) break;
     }
   return str;
}

int
evas_textblock2_cursor_char_geometry_get(Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;
   Evas_Object_Textblock_Format_Item *fi = NULL;
   int x = 0, y = 0, w = 0, h = 0;
   int pos, ret;
   
   if (!cur) return -1;
   if (!cur->node) return -1;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   if (cur->node->type == NODE_FORMAT)
     _find_layout_format_item_line_match(cur->obj, cur->node, &ln, &fi);
   else
     _find_layout_item_line_match(cur->obj, cur->node, cur->pos, &ln, &it);
   if (!ln) return -1;
   if (it)
     {
	pos = cur->pos - it->source_pos;
	ret = -1;
	if (it->format->font.font)
	  ret = cur->ENFN->font_char_coords_get(cur->ENDT, it->format->font.font,
						it->text,
						pos,
						&x, &y, &w, &h);
	if (ret <= 0) return -1;
	x = ln->x + it->x - it->inset + x;
	if (x < ln->x)
	  {
	     x = ln->x;
	     w -= (ln->x - x);
	  }
	y = ln->y;
	h = ln->h;
     }
   else if (fi)
     {
	x = ln->x + fi->x;
	y = ln->y;
	w = fi->w;
	h = ln->h;
     }
   else
     return -1;
   if (cx) *cx = x;
   if (cy) *cy = y;
   if (cw) *cw = w;
   if (ch) *ch = h;
   return ln->line_no;
}

int
evas_textblock2_cursor_line_geometry_get(Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;
   Evas_Object_Textblock_Format_Item *fi = NULL;
   int x, y, w, h;
   int pos, ret;
   
   if (!cur) return -1;
   if (!cur->node) return -1;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   if (cur->node->type == NODE_FORMAT)
     _find_layout_format_item_line_match(cur->obj, cur->node, &ln, &fi);
   else
     _find_layout_item_line_match(cur->obj, cur->node, cur->pos, &ln, &it);
   if (!ln) return -1;
   x = ln->x;
   y = ln->y;
   w = ln->w;
   h = ln->h;
   if (cx) *cx = x;
   if (cy) *cy = y;
   if (cw) *cw = w;
   if (ch) *ch = h;
   return ln->line_no;
}

Evas_Bool
evas_textblock2_cursor_char_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l, *ll;
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL, *it_break = NULL;
   Evas_Object_Textblock_Format_Item *fi = NULL;
   
   if (!cur) return 0;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   x += o->style_pad.l;
   y += o->style_pad.t;
   for (l = (Evas_Object_List *)o->lines; l; l = l->next)
     {
        Evas_Object_Textblock_Line *ln;

        ln = (Evas_Object_Textblock_Line *)l;
	if (ln->y > y) break;
	if ((ln->y <= y) && ((ln->y + ln->h) > y))
	  {
	     for (ll = (Evas_Object_List *)ln->items; ll; ll = ll->next)
	       {
		  it = (Evas_Object_Textblock_Item *)ll;
		  if ((it->x +ln->x) > x)
		    {
		       it_break = it;
		       break;
		    }
		  if (((it->x + ln->x) <= x) && (((it->x + ln->x) + it->w) > x))
		    {
		       int pos;
		       int cx, cy, cw, ch;
		       
		       pos = -1;
		       if (it->format->font.font)
			 pos = cur->ENFN->font_char_at_coords_get(cur->ENDT,
								  it->format->font.font,
								  it->text,
								  x - it->x - ln->x, 0,
								  &cx, &cy, &cw, &ch);
		       if (pos < 0)
			 return 0;
		       cur->pos = pos + it->source_pos;
		       cur->node = it->source_node;
		       return 1;
		    }
	       }
	     for (ll = (Evas_Object_List *)ln->format_items; ll; ll = ll->next)
	       {
		  fi = (Evas_Object_Textblock_Format_Item *)ll;
		  if ((fi->x + ln->x) > x) break;
		  if (((fi->x + ln->x) <= x) && (((fi->x + ln->x) + fi->w) > x))
		    {
		       cur->pos = 0;
		       cur->node = fi->source_node;
		       return 1;
		    }
	       }
	     if (it_break)
	       {
		  it = it_break;
		  cur->pos = it->source_pos;
		  cur->node = it->source_node;
		  return 1;
	       }
	  }
     }
   return 0;
}

int
evas_textblock2_cursor_line_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord y)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l;
   Evas_Object_Textblock_Line *ln = NULL;
   
   if (!cur) return -1;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   y += o->style_pad.t;
   for (l = (Evas_Object_List *)o->lines; l; l = l->next)
     {
        Evas_Object_Textblock_Line *ln;

        ln = (Evas_Object_Textblock_Line *)l;
	if (ln->y > y) break;
	if ((ln->y <= y) && ((ln->y + ln->h) > y))
	  {
	     evas_textblock2_cursor_line_set(cur, ln->line_no);
	     return ln->line_no;
	  }
     }
   return -1;
}

Evas_List *
evas_textblock2_cursor_range_geometry_get(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2)
{
   Evas_Object_Textblock *o;
   Evas_List *rects = NULL;
   Evas_Coord cx, cy, cw, ch, lx, ly, lw, lh;
   Evas_Textblock_Rectangle *tr;
   int i, line, line2;
   
   if (!cur1) return NULL;
   if (!cur2) return NULL;
   if (cur1->obj != cur2->obj) return NULL;
   o = (Evas_Object_Textblock *)(cur1->obj->object_data);
   if (evas_textblock2_cursor_compare(cur1, cur2) > 0)
     {
	Evas_Textblock_Cursor *tc;
	
	tc = cur1;
	cur1 = cur2;
	cur2 = tc;
     }
   line = evas_textblock2_cursor_char_geometry_get(cur1, &cx, &cy, &cw, &ch);
   if (line < 0) return NULL;
   line = evas_textblock2_cursor_line_geometry_get(cur1, &lx, &ly, &lw, &lh);
   if (line < 0) return NULL;
   line2 = evas_textblock2_cursor_line_geometry_get(cur2, NULL, NULL, NULL, NULL);
   if (line2 < 0) return NULL;
   if (line == line2)
     {
	tr = calloc(1, sizeof(Evas_Textblock_Rectangle));
	rects = evas_list_append(rects, tr);
	tr->x = cx;
	tr->y = ly;
	tr->h = lh;
	line = evas_textblock2_cursor_char_geometry_get(cur2, &cx, &cy, &cw, &ch);
	if (line < 0)
	  {
	     while (rects)
	       {
		  free(rects->data);
		  rects = evas_list_remove_list(rects, rects);
	       }
	     return NULL;
	  }
	tr->w = cx + cw - tr->x;
     }
   else
     {
	tr = calloc(1, sizeof(Evas_Textblock_Rectangle));
	rects = evas_list_append(rects, tr);
	tr->x = cx;
	tr->y = ly;
	tr->h = lh;
	tr->w = lx + lw - cx;
	for (i = line +1; i < line2; i++)
	  {
	     evas_object_textblock2_line_number_geometry_get(cur1->obj, i, &lx, &ly, &lw, &lh);
	     tr = calloc(1, sizeof(Evas_Textblock_Rectangle));
	     rects = evas_list_append(rects, tr);
	     tr->x = lx;
	     tr->y = ly;
	     tr->h = lh;
	     tr->w = lw;
	  }
	line = evas_textblock2_cursor_char_geometry_get(cur2, &cx, &cy, &cw, &ch);
	if (line < 0)
	  {
	     while (rects)
	       {
		  free(rects->data);
		  rects = evas_list_remove_list(rects, rects);
	       }
	     return NULL;
	  }
	line = evas_textblock2_cursor_line_geometry_get(cur2, &lx, &ly, &lw, &lh);
	if (line < 0)
	  {
	     while (rects)
	       {
		  free(rects->data);
		  rects = evas_list_remove_list(rects, rects);
	       }
	     return NULL;
	  }
	tr = calloc(1, sizeof(Evas_Textblock_Rectangle));
	rects = evas_list_append(rects, tr);
	tr->x = lx;
	tr->y = ly;
	tr->h = lh;
	tr->w = cx + cw - lx;
     }
   return rects;
}

/* general controls */
Evas_Bool
evas_object_textblock2_line_number_geometry_get(Evas_Object *obj, int line, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock_Line *ln;
   
   TB_HEAD();
   ln = _find_layout_line_num(obj, line);
   if (!ln) return 0;
   if (cx) *cx = ln->x;
   if (cy) *cy = ln->y;
   if (cw) *cw = ln->w;
   if (ch) *ch = ln->h;
   return 1;
}

void
evas_object_textblock2_clear(Evas_Object *obj)
{
   Evas_List *l;
   
   TB_HEAD();
   _nodes_clear(obj);
   o->cursor->node = NULL;
   o->cursor->pos = 0;
   for (l = o->cursors; l; l = l->next)
     {
	Evas_Textblock_Cursor *cur;
	
	cur = (Evas_Textblock_Cursor *)l->data;
	cur->node = NULL;
	cur->pos = 0;
     }
   if (o->markup_text)
     {
	free(o->markup_text);
	o->markup_text = NULL;
     }
   if (o->lines)
     {
	_lines_clear(obj, o->lines);
	o->lines = NULL;
     }
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->changed = 1;
   evas_object_change(obj);
   /* FIXME: adjust cursors that are affected by the change */
}

void
evas_object_textblock2_size_formatted_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   TB_HEAD();
   if (!o->formatted.valid) _relayout(obj);
   if (w) *w = o->formatted.w;
   if (h) *h = o->formatted.h;
}

void
evas_object_textblock2_size_native_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   TB_HEAD();
   if (!o->native.valid)
     {
	_layout(obj, 
		1,
		-1, -1,
		&o->native.w, &o->native.h);
	o->native.valid = 1;
     }
   if (w) *w = o->native.w;
   if (h) *h = o->native.h;
}

void
evas_object_textblock2_style_insets_get(Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b)
{
   TB_HEAD();
   if (!o->formatted.valid) _relayout(obj);
   if (l) *l = o->style_pad.l;
   if (r) *r = o->style_pad.r;
   if (t) *t = o->style_pad.t;
   if (b) *b = o->style_pad.b;
}

/* all nice and private */
static void
evas_object_textblock_init(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   /* alloc image ob, setup methods and default values */
   obj->object_data = evas_object_textblock_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0.0;
   obj->cur.geometry.y = 0.0;
   obj->cur.geometry.w = 0.0;
   obj->cur.geometry.h = 0.0;
   obj->cur.layer = 0;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;

   o = (Evas_Object_Textblock *)(obj->object_data);
   o->cursor->obj = obj;
}

static void *
evas_object_textblock_new(void)
{
   Evas_Object_Textblock *o;

   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Textblock));
   o->magic = MAGIC_OBJ_TEXTBLOCK;
   o->cursor = calloc(1, sizeof(Evas_Textblock_Cursor));
   return o;
}

static void
evas_object_textblock_free(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   evas_object_textblock2_clear(obj);
   evas_object_textblock2_style_set(obj, NULL);
   o = (Evas_Object_Textblock *)(obj->object_data);
   free(o->cursor);
   while (o->cursors)
     {
	Evas_Textblock_Cursor *cur;
	
	cur = (Evas_Textblock_Cursor *)o->cursors->data;
	o->cursors = evas_list_remove_list(o->cursors, o->cursors);
	free(cur);
     }
   o->magic = 0;
   free(o);
}

static void
evas_object_textblock_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l, *ll;
   int i, j;
   int pback = 0, backx = 0;
   int pline = 0, linex = 0;
   int pline2 = 0, line2x = 0;
   int pstrike = 0, strikex = 0;
   int x2;
   unsigned char r = 0, g = 0, b = 0, a = 0;
   unsigned char r2 = 0, g2 = 0, b2 = 0, a2 = 0;
   unsigned char r3 = 0, g3 = 0, b3 = 0, a3 = 0;
   const char vals[5][5] =
     {
	  {0, 1, 2, 1, 0},
	  {1, 3, 4, 3, 1},
	  {2, 4, 5, 4, 2},
	  {1, 3, 4, 3, 1},
	  {0, 1, 2, 1, 0}
     };

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Textblock *)(obj->object_data);
   obj->layer->evas->engine.func->context_multiplier_unset(output,
							   context);
#if 0 /* using for some debugging. will go soon */
    obj->layer->evas->engine.func->context_color_set(output,
                                                     context,
                                                     230, 160, 30, 100);
    obj->layer->evas->engine.func->rectangle_draw(output,
                                                  context,
                                                  surface,
                                                  obj->cur.cache.geometry.x + x,
                                                  obj->cur.cache.geometry.y + y,
                                                  obj->cur.cache.geometry.w,
                                                  obj->cur.cache.geometry.h);
#endif
#define ITEM_WALK() \
   for (l = (Evas_Object_List *)o->lines; l; l = l->next) \
     { \
	Evas_Object_Textblock_Line *ln; \
	\
	ln = (Evas_Object_Textblock_Line *)l; \
        pback = 0; \
        pline = 0; \
        pline2 = 0; \
	pstrike = 0; \
	for (ll = (Evas_Object_List *)ln->items; ll; ll = ll->next) \
	  { \
	     Evas_Object_Textblock_Item *it; \
	     int yoff; \
	     \
	     it = (Evas_Object_Textblock_Item *)ll; \
	     yoff = ln->baseline; \
	     if (it->format->valign != -1.0) \
	       yoff = (it->format->valign * (double)(ln->h - it->h)) + it->baseline;
#define ITEM_WALK_END() \
	  } \
     }
#define COLOR_SET(col) \
	ENFN->context_color_set(output, context, \
				it->format->color.col.r, \
				it->format->color.col.g, \
				it->format->color.col.b, \
				it->format->color.col.a);
#define COLOR_SET_AMUL(col, amul) \
	ENFN->context_color_set(output, context, \
				it->format->color.col.r, \
				it->format->color.col.g, \
				it->format->color.col.b, \
				((int)it->format->color.col.a * (amul)) / 255);
#define DRAW_TEXT(ox, oy) \
   if (it->format->font.font) ENFN->font_draw(output, context, surface, it->format->font.font, \
						 obj->cur.cache.geometry.x + ln->x + it->x - it->inset + x + (ox), \
						 obj->cur.cache.geometry.y + ln->y + yoff + y + (oy), \
						 it->w, it->h, it->w, it->h, it->text);
   
   pback = 0;
   /* backing */
   ITEM_WALK();
   if ((it->format->backing) && (!pback) && (ll->next))
     {
	pback = 1;
	backx = it->x;
	r = it->format->color.backing.r;
	g = it->format->color.backing.g;
	b = it->format->color.backing.b;
	a = it->format->color.backing.a;
     }
   else if (((pback) && (!it->format->backing)) ||
	    (!ll->next) ||
	    (it->format->color.backing.r != r) ||
	    (it->format->color.backing.g != g) ||
	    (it->format->color.backing.b != b) ||
	    (it->format->color.backing.a != a))
     {
	if ((it->format->backing) && (!pback))
	  {
	     backx = it->x;
	     r = it->format->color.backing.r;
	     g = it->format->color.backing.g;
	     b = it->format->color.backing.b;
	     a = it->format->color.backing.a;
	  }
	x2 = it->x + it->w;
	if (!it->format->backing)
	  {
	     x2 = it->x;
	     pback = 0;
	  }
	if (x2 > backx)
	  {
	     ENFN->context_color_set(output,
				     context,
				     (obj->cur.cache.clip.r * r) / 255,
				     (obj->cur.cache.clip.g * g) / 255,
				     (obj->cur.cache.clip.b * b) / 255,
				     (obj->cur.cache.clip.a * a) / 255);
	     ENFN->rectangle_draw(output,
				  context,
				  surface,
				  obj->cur.cache.geometry.x + ln->x + backx + x,
				  obj->cur.cache.geometry.y + ln->y + y,
				  x2 - backx,
				  ln->h);
	  }
	if (it->format->backing) pback = 1;
	backx = it->x;
	r = it->format->color.backing.r;
	g = it->format->color.backing.g;
	b = it->format->color.backing.b;
	a = it->format->color.backing.a;
     }
   ITEM_WALK_END();
   
   /* shadows */
   ITEM_WALK();
   if (it->format->style == STYLE_SHADOW)
     {
	COLOR_SET(shadow);
	DRAW_TEXT(1, 1);
     }
   else if ((it->format->style == STYLE_OUTLINE_SHADOW) ||
	    (it->format->style == STYLE_FAR_SHADOW))
     {
	COLOR_SET(shadow);
	DRAW_TEXT(2, 2);
     }
   else if ((it->format->style == STYLE_OUTLINE_SOFT_SHADOW) ||
	    (it->format->style == STYLE_FAR_SOFT_SHADOW))
     {
	for (j = 0; j < 5; j++)
	  {
	     for (i = 0; i < 5; i++)
	       {
		  if (vals[i][j] != 0)
		    {
		       COLOR_SET_AMUL(shadow, vals[i][j] * 50);
		       DRAW_TEXT(i, j);
		    }
	       }
	  }
     }
   else if (it->format->style == STYLE_SOFT_SHADOW)
     {
	for (j = 0; j < 5; j++)
	  {
	     for (i = 0; i < 5; i++)
	       {
		  if (vals[i][j] != 0)
		    {
		       COLOR_SET_AMUL(shadow, vals[i][j] * 50);
		       DRAW_TEXT(i - 1, j - 1);
		    }
	       }
	  }
     }
   ITEM_WALK_END();
   
   /* glows */
   ITEM_WALK();
   if (it->format->style == STYLE_GLOW)
     {
	for (j = 0; j < 5; j++)
	  {
	     for (i = 0; i < 5; i++)
	       {
		  if (vals[i][j] != 0)
		    {
		       COLOR_SET_AMUL(glow, vals[i][j] * 50);
		       DRAW_TEXT(i - 2, j - 2);
		    }
	       }
	  }
	COLOR_SET(glow2);
	DRAW_TEXT(-1, 0);
	DRAW_TEXT(1, 0);
	DRAW_TEXT(0, -1);
	DRAW_TEXT(0, 1);
     }
   ITEM_WALK_END();
   
   /* outlines */
   ITEM_WALK();
   if ((it->format->style == STYLE_OUTLINE) ||
       (it->format->style == STYLE_OUTLINE_SHADOW) ||
       (it->format->style == STYLE_OUTLINE_SOFT_SHADOW))
     {
	COLOR_SET(outline);
	DRAW_TEXT(-1, 0);
	DRAW_TEXT(1, 0);
	DRAW_TEXT(0, -1);
	DRAW_TEXT(0, 1);
     }
   ITEM_WALK_END();
   
   /* normal text */
   ITEM_WALK();
   COLOR_SET(normal);
   DRAW_TEXT(0, 0);
   if ((it->format->strikethrough) && (!pstrike) && (ll->next))
     {
	pstrike = 1;
	strikex = it->x;
	r3 = it->format->color.strikethrough.r;
	g3 = it->format->color.strikethrough.g;
	b3 = it->format->color.strikethrough.b;
	a3 = it->format->color.strikethrough.a;
     }
   else if (((pstrike) && (!it->format->strikethrough)) ||
	    (!ll->next) ||
	    (it->format->color.strikethrough.r != r3) ||
	    (it->format->color.strikethrough.g != g3) ||
	    (it->format->color.strikethrough.b != b3) ||
	    (it->format->color.strikethrough.a != a3))
     {
	if ((it->format->strikethrough) && (!pstrike))
	  {
	     strikex = it->x;
	     r3 = it->format->color.strikethrough.r;
	     g3 = it->format->color.strikethrough.g;
	     b3 = it->format->color.strikethrough.b;
	     a3 = it->format->color.strikethrough.a;
	  }
	x2 = it->x + it->w;
	if (!it->format->strikethrough)
	  {
	     x2 = it->x;
	     pstrike = 0;
	  }
	if (x2 > strikex)
	  {
	     ENFN->context_color_set(output,
				     context,
				     (obj->cur.cache.clip.r * r3) / 255,
				     (obj->cur.cache.clip.g * g3) / 255,
				     (obj->cur.cache.clip.b * b3) / 255,
				     (obj->cur.cache.clip.a * a3) / 255);
	     ENFN->rectangle_draw(output,
				  context,
				  surface,
				  obj->cur.cache.geometry.x + ln->x + strikex + x,
				  obj->cur.cache.geometry.y + ln->y + y + (ln->h / 2),
				  x2 - strikex,
				  1);
	  }
	if (it->format->strikethrough) pstrike = 1;
	strikex = it->x;
	r3 = it->format->color.strikethrough.r;
	g3 = it->format->color.strikethrough.g;
	b3 = it->format->color.strikethrough.b;
	a3 = it->format->color.strikethrough.a;
     }
   if ((it->format->underline) && (!pline) && (ll->next))
     {
	pline = 1;
	linex = it->x;
	r = it->format->color.underline.r;
	g = it->format->color.underline.g;
	b = it->format->color.underline.b;
	a = it->format->color.underline.a;
     }
   else if (((pline) && (!it->format->underline)) ||
	    (!ll->next) ||
	    (it->format->color.underline.r != r) ||
	    (it->format->color.underline.g != g) ||
	    (it->format->color.underline.b != b) ||
	    (it->format->color.underline.a != a))
     {
	if ((it->format->underline) && (!pline))
	  {
	     linex = it->x;
	     r = it->format->color.underline.r;
	     g = it->format->color.underline.g;
	     b = it->format->color.underline.b;
	     a = it->format->color.underline.a;
	  }
	x2 = it->x + it->w;
	if (!it->format->underline)
	  {
	     x2 = it->x;
	     pline = 0;
	  }
	if (x2 > linex)
	  {
	     ENFN->context_color_set(output,
				     context,
				     (obj->cur.cache.clip.r * r) / 255,
				     (obj->cur.cache.clip.g * g) / 255,
				     (obj->cur.cache.clip.b * b) / 255,
				     (obj->cur.cache.clip.a * a) / 255);
	     ENFN->rectangle_draw(output,
				  context,
				  surface,
				  obj->cur.cache.geometry.x + ln->x + linex + x,
				  obj->cur.cache.geometry.y + ln->y + y + ln->baseline + 1,
				  x2 - linex,
				  1);
	  }
	if (it->format->underline) pline = 1;
	linex = it->x;
	r = it->format->color.underline.r;
	g = it->format->color.underline.g;
	b = it->format->color.underline.b;
	a = it->format->color.underline.a;
     }
   if ((it->format->underline2) && (!pline2) && (ll->next))
     {
	pline2 = 1;
	line2x = it->x;
	r2 = it->format->color.underline2.r;
	g2 = it->format->color.underline2.g;
	b2 = it->format->color.underline2.b;
	a2 = it->format->color.underline2.a;
     }
   else if (((pline2) && (!it->format->underline2)) ||
	    (!ll->next) ||
	    (it->format->color.underline2.r != r2) ||
	    (it->format->color.underline2.g != g2) ||
	    (it->format->color.underline2.b != b2) ||
	    (it->format->color.underline2.a != a2))
     {
	if ((it->format->underline2) && (!pline2))
	  {
	     line2x = it->x;
	     r2 = it->format->color.underline2.r;
	     g2 = it->format->color.underline2.g;
	     b2 = it->format->color.underline2.b;
	     a2 = it->format->color.underline2.a;
	  }
	x2 = it->x + it->w;
	if (!it->format->underline2)
	  {
	     x2 = it->x;
	     pline2 = 0;
	  }
	if (x2 > line2x)
	  {
	     ENFN->context_color_set(output,
				     context,
				     (obj->cur.cache.clip.r * r2) / 255,
				     (obj->cur.cache.clip.g * g2) / 255,
				     (obj->cur.cache.clip.b * b2) / 255,
				     (obj->cur.cache.clip.a * a2) / 255);
	     ENFN->rectangle_draw(output,
				  context,
				  surface,
				  obj->cur.cache.geometry.x + ln->x + line2x + x,
				  obj->cur.cache.geometry.y + ln->y + y + ln->baseline + 3,
				  x2 - line2x,
				  1);
	  }
	if (it->format->underline2) pline2 = 1;
	line2x = it->x;
	r2 = it->format->color.underline2.r;
	g2 = it->format->color.underline2.g;
	b2 = it->format->color.underline2.b;
	a2 = it->format->color.underline2.a;
     }
   ITEM_WALK_END();
}

static void
evas_object_textblock_render_pre(Evas_Object *obj)
{
   Evas_List *updates = NULL;
   Evas_Object_Textblock *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw textblocks */
   o = (Evas_Object_Textblock *)(obj->object_data);
   if ((o->changed) ||
       (o->last_w != obj->cur.geometry.w))
     {
	Evas_Object_Textblock_Line *lines;

	lines = o->lines;
	o->lines = NULL;
	o->formatted.valid = 0;
	o->native.valid = 0;
	_layout(obj, 
		0,
		obj->cur.geometry.w, obj->cur.geometry.h,
		&o->formatted.w, &o->formatted.h);
	o->formatted.valid = 1;
	if (lines) _lines_clear(obj, lines);
	o->last_w = obj->cur.geometry.w;
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	o->changed = 0;
	is_v = evas_object_is_visible(obj);
	was_v = evas_object_was_visible(obj);
	goto done;
     }
   if (o->redraw)
     {
	o->redraw = 0;
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	o->changed = 0;
	is_v = evas_object_is_visible(obj);
	was_v = evas_object_was_visible(obj);
	goto done;
     }
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	if (obj->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	updates = evas_object_render_pre_visible_change(updates, obj, is_v, was_v);
	goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   updates = evas_object_render_pre_clipper_change(updates, obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	goto done;
     }
   if (o->changed)
     {
/*
	Evas_Rectangle *r;

	r = malloc(sizeof(Evas_Rectangle));
	r->x = 0; r->y = 0;
	r->w = obj->cur.geometry.w;
	r->h = obj->cur.geometry.h;
	updates = evas_list_append(updates, r);
*/
	updates = evas_object_render_pre_prev_cur_add(updates, obj);
	o->changed = 0;
     }
   done:
   evas_object_render_pre_effect_updates(updates, obj, is_v, was_v);
}

static void
evas_object_textblock_render_post(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Textblock *)(obj->object_data);
   /* remove those pesky changes */
   while (obj->clip.changes)
     {
	Evas_Rectangle *r;

	r = (Evas_Rectangle *)obj->clip.changes->data;
	obj->clip.changes = evas_list_remove(obj->clip.changes, r);
	free(r);
     }
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
//   o->prev = o->cur;
/*   o->changed = 0; */
}

static int
evas_object_textblock_is_opaque(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   /* this returns 1 if the internal object data implies that the object is */
   /* currently fulyl opque over the entire gradient it occupies */
   o = (Evas_Object_Textblock *)(obj->object_data);
   return 0;
}

static int
evas_object_textblock_was_opaque(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   /* this returns 1 if the internal object data implies that the object was */
   /* currently fulyl opque over the entire gradient it occupies */
   o = (Evas_Object_Textblock *)(obj->object_data);
   return 0;
}

static void
evas_object_textblock_coords_recalc(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   if (obj->cur.geometry.w != o->last_w)
     {
	o->formatted.valid = 0;
	o->native.valid = 0;
	o->changed = 1;
     }
}
