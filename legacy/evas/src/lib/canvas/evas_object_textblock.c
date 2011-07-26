/**
 * @internal
 * @section Evas_Object_Textblock_Internal Internal Textblock Object Tutorial
 *
 * This explains the internal design of the Evas Textblock Object, it's assumed
 * that the reader of this section has already read @ref Evas_Object_Textblock_Tutorial "Textblock's usage docs.".
 *
 * @subsection textblock_internal_intro Introduction
 * There are two main parts to the textblock object, the first being the node
 * system, and the second being the layout system. The former is just an
 * internal representation of the markup text, while the latter is the internal
 * visual representation of the text (i.e positioning, sizing, fonts and etc).
 *
 * @subsection textblock_nodes The Nodes system
 * The nodes mechanism consists of two main data types:
 * ::Evas_Object_Textblock_Node_Text and ::Evas_Object_Textblock_Node_Format
 * the former is for Text nodes and the latter is for format nodes.
 * There's always at least one text node, even if there are only formats.
 *
 * @subsection textblock_nodes_text Text nodes
 * Each text node is essentially a paragraph, it includes an @ref Eina_UStrbuf
 * that stores the actual paragraph text, a utf8 string to store the paragraph
 * text in utf8 (which is not used internally at all), A pointer to it's
 * main @ref textblock_nodes_format_internal "Format Node" and the paragraph's
 * @ref evas_bidi_props "BiDi properties". The pointer to the format node may be
 * NULL if there's no format node anywhere before the end of the text node,
 * not even in previous text nodes. If not NULL, it points to the first format
 * node pointing to text inside of the text node, or if there is none, it points
 * to the previous's text nodes format node. Each paragraph has a format node
 * representing a paragraph separator pointing to it's last position except
 * for the last paragraph, which has no such constraint. This constraint
 * happens because text nodes are paragraphs and paragraphs are delimited by
 * paragraph separators.
 *
 * @subsection textblock_nodes_format_internal Format Nodes - Internal
 * Each format node stores a group of format information, for example the
 * markup: \<font=Vera,Kochi font_size=10 align=left\> will all be inserted
 * inside the same format node, altohugh it consists of different formatting
 * commands.
 * Each node has a pointer to it's text node, this pointer is NEVER NULL, even
 * if there's only one format, and no text, a text node is created. Each format
 * node includes an offset from the last format node of the same text node. For
 * example, the markup "0<b>12</b>" will create two format nodes, the first
 * having an offset of 1 and the second an offset of 2. Each format node also
 * includes a @ref Eina_Strbuf that includes the textual representation of the
 * format, and a boolean stating if the format is a visible format or not, see
 * @ref textblock_nodes_format_visible
 *
 * @subsection textblock_nodes_format_visible Visible Format Nodes
 * There are two types of format nodes, visible and invisible. They are the same
 * in every way, except for the representation in the text node. While invisible
 * format nodes have no representation in the text node, the visible ones do.
 * The Uniceode object replacement character (0xFFFC) is inserted to every place
 * a visible format node points to. This makes it very easy to treat visible
 * formats as items in the text, both for BiDi purposes and cursor handling
 * purposes.
 * Here are a few example visible an invisible formats:
 * Visible: newline char, tab, paragraph separator and an embedded item.
 * Invisible: setting the color, font or alignment of the text.
 *
 * @subsection textblock_layout The layout system
 * @todo write @ref textblock_layout
 */
#include <stdlib.h>

#include "evas_common.h"
#include "evas_private.h"

#ifdef HAVE_LINEBREAK
#include "linebreak.h"
#endif

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

/* private magic number for textblock objects */
static const char o_type[] = "textblock";

/* The char to be inserted instead of visible formats */
#define EVAS_TEXTBLOCK_REPLACEMENT_CHAR 0xFFFC
#define _PARAGRAPH_SEPARATOR 0x2029
#define EVAS_TEXTBLOCK_IS_VISIBLE_FORMAT_CHAR(ch) \
   (((ch) == EVAS_TEXTBLOCK_REPLACEMENT_CHAR) || \
    ((ch) == '\n') || \
    ((ch) == '\t') || \
    ((ch) == _PARAGRAPH_SEPARATOR))

/* private struct for textblock object internal data */
/**
 * @internal
 * @typedef Evas_Object_Textblock
 * The actual textblock object.
 */
typedef struct _Evas_Object_Textblock             Evas_Object_Textblock;
/**
 * @internal
 * @typedef Evas_Object_Style_Tag
 * The structure used for finding style tags.
 */
typedef struct _Evas_Object_Style_Tag             Evas_Object_Style_Tag;
/**
 * @internal
 * @typedef Evas_Object_Textblock_Node_Text
 * A text node.
 */
typedef struct _Evas_Object_Textblock_Node_Text   Evas_Object_Textblock_Node_Text;
/*
 * Defined in Evas.h
typedef struct _Evas_Object_Textblock_Node_Format Evas_Object_Textblock_Node_Format;
*/

/**
 * @internal
 * @typedef Evas_Object_Textblock_Paragraph
 * A layouting paragraph.
 */
typedef struct _Evas_Object_Textblock_Paragraph   Evas_Object_Textblock_Paragraph;
/**
 * @internal
 * @typedef Evas_Object_Textblock_Line
 * A layouting line.
 */
typedef struct _Evas_Object_Textblock_Line        Evas_Object_Textblock_Line;
/**
 * @internal
 * @typedef Evas_Object_Textblock_Item
 * A layouting item.
 */
typedef struct _Evas_Object_Textblock_Item        Evas_Object_Textblock_Item;
/**
 * @internal
 * @typedef Evas_Object_Textblock_Item
 * A layouting text item.
 */
typedef struct _Evas_Object_Textblock_Text_Item        Evas_Object_Textblock_Text_Item;
/**
 * @internal
 * @typedef Evas_Object_Textblock_Format_Item
 * A layouting format item.
 */
typedef struct _Evas_Object_Textblock_Format_Item Evas_Object_Textblock_Format_Item;
/**
 * @internal
 * @typedef Evas_Object_Textblock_Format
 * A textblock format.
 */
typedef struct _Evas_Object_Textblock_Format      Evas_Object_Textblock_Format;

/**
 * @internal
 * @def IS_AT_END(ti, ind)
 * Return true if ind is at the end of the text item, false otherwise.
 */
#define IS_AT_END(ti, ind) (ind == ti->text_props.text_len)

/**
 * @internal
 * @def MOVE_PREV_UNTIL(limit, ind)
 * This decrements ind as long as ind > limit.
 */
#define MOVE_PREV_UNTIL(limit, ind) \
   do \
     { \
        if ((limit) < (ind)) \
           (ind)--; \
     } \
   while (0)

/**
 * @internal
 * @def MOVE_NEXT_UNTIL(limit, ind)
 * This increments ind as long as ind < limit
 */
#define MOVE_NEXT_UNTIL(limit, ind) \
   do \
     { \
        if ((ind) < (limit)) \
           (ind)++; \
     } \
   while (0)

/**
 * @internal
 * @def GET_ITEM_TEXT(ti)
 * Returns a const reference to the text of the ti (not null terminated).
 */
#define GET_ITEM_TEXT(ti) \
   (((ti)->parent.text_node) ? \
    (eina_ustrbuf_string_get((ti)->parent.text_node->unicode) + \
      (ti)->parent.text_pos) : EINA_UNICODE_EMPTY_STRING)
/**
 * @internal
 * @def _FORMAT_IS_CLOSER_OF(base, closer, closer_len)
 * Returns true if closer is the closer of base.
 */
#define _FORMAT_IS_CLOSER_OF(base, closer, closer_len) \
   (!strncmp(base + 1, closer, closer_len) && \
    (!base[closer_len + 1] || \
     (base[closer_len + 1] == '=') || \
     _is_white(base[closer_len + 1])))

/*FIXME: document the structs and struct items. */
struct _Evas_Object_Style_Tag
{
   EINA_INLIST;
   char *tag;
   char *replace;
   size_t tag_len;
   size_t replace_len;
};

struct _Evas_Object_Textblock_Node_Text
{
   EINA_INLIST;
   Eina_UStrbuf                       *unicode;
   char                               *utf8;
   Evas_Object_Textblock_Node_Format  *format_node;
   Evas_Object_Textblock_Paragraph    *par;
   Eina_Bool                           dirty : 1;
   Eina_Bool                           new : 1;
};

struct _Evas_Object_Textblock_Node_Format
{
   EINA_INLIST;
   const char                         *format;
   const char                         *orig_format;
   Evas_Object_Textblock_Node_Text    *text_node;
   size_t                              offset;
   unsigned char                       anchor : 2;
   Eina_Bool                           visible : 1;
   Eina_Bool                           format_change : 1;
   Eina_Bool                           new : 1;
};

#define ANCHOR_NONE 0
#define ANCHOR_A 1
#define ANCHOR_ITEM 2

/**
 * @internal
 * @def _NODE_TEXT(x)
 * A convinience macro for casting to a text node.
 */
#define _NODE_TEXT(x)  ((Evas_Object_Textblock_Node_Text *) (x))
/**
 * @internal
 * @def _NODE_FORMAT(x)
 * A convinience macro for casting to a format node.
 */
#define _NODE_FORMAT(x)  ((Evas_Object_Textblock_Node_Format *) (x))
/**
 * @internal
 * @def _ITEM(x)
 * A convinience macro for casting to a generic item.
 */
#define _ITEM(x)  ((Evas_Object_Textblock_Item *) (x))
/**
 * @internal
 * @def _ITEM_TEXT(x)
 * A convinience macro for casting to a text item.
 */
#define _ITEM_TEXT(x)  ((Evas_Object_Textblock_Text_Item *) (x))
/**
 * @internal
 * @def _ITEM_FORMAT(x)
 * A convinience macro for casting to a format item.
 */
#define _ITEM_FORMAT(x)  ((Evas_Object_Textblock_Format_Item *) (x))

struct _Evas_Object_Textblock_Paragraph
{
   EINA_INLIST;
   EINA_RBTREE;
   Evas_Object_Textblock_Line        *lines;
   Evas_Object_Textblock_Node_Text   *text_node;
   Eina_List                         *logical_items;
   Evas_BiDi_Paragraph_Props         *bidi_props; /* Only valid during layout */
   Evas_BiDi_Direction                direction;
   Evas_Coord                         y, w, h;
   int                                line_no;
   Eina_Bool                          is_bidi : 1;
   Eina_Bool                          visible : 1;
   Eina_Bool                          indexed : 1;
   Eina_Bool                          rendered : 1;
};

struct _Evas_Object_Textblock_Line
{
   EINA_INLIST;
   Evas_Object_Textblock_Item        *items;
   Evas_Object_Textblock_Paragraph   *par;
   Evas_Coord                         x, y, w, h;
   int                                baseline;
   int                                line_no;
};

typedef enum _Evas_Textblock_Item_Type
{
   EVAS_TEXTBLOCK_ITEM_TEXT,
   EVAS_TEXTBLOCK_ITEM_FORMAT,
} Evas_Textblock_Item_Type;

struct _Evas_Object_Textblock_Item
{
   EINA_INLIST;
   Evas_Textblock_Item_Type             type;
   Evas_Object_Textblock_Node_Text     *text_node;
   Evas_Object_Textblock_Format        *format;
   size_t                               text_pos;
#ifdef BIDI_SUPPORT
   size_t                               visual_pos;
#endif
   Evas_Coord                           adv, x, w, h;
   Eina_Bool                            merge : 1; /* Indicates whether this
                                                      item should merge to the
                                                      previous item or not */
   Eina_Bool                            visually_deleted : 1;
                                                   /* Indicates whether this
                                                      item is used in the visual
                                                      layout or not. */
};

struct _Evas_Object_Textblock_Text_Item
{
   Evas_Object_Textblock_Item       parent;
   Evas_Text_Props                  text_props;
   Evas_Coord                       inset;
   Evas_Coord                       x_adjustment; /* Used to indicate by how
                                                     much we adjusted sizes */
};

struct _Evas_Object_Textblock_Format_Item
{
   Evas_Object_Textblock_Item           parent;
   Evas_BiDi_Direction                  bidi_dir;
   const char                          *item;
   int                                  y;
   unsigned char                        vsize : 2;
   unsigned char                        size : 2;
   Eina_Bool                            formatme : 1;
};

struct _Evas_Object_Textblock_Format
{
   Evas_Object_Textblock_Node_Format *fnode;
   double               halign;
   double               valign;
   struct {
      const char       *name;
      const char       *source;
      const char       *fallbacks;
      Evas_Font_Set    *font;
      int               size;
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
   int                  ref;
   int                  tabstops;
   int                  linesize;
   int                  linegap;
   double               linerelsize;
   double               linerelgap;
   double               linefill;
   double               ellipsis;
   unsigned char        style;
   Eina_Bool            wrap_word : 1;
   Eina_Bool            wrap_char : 1;
   Eina_Bool            wrap_mixed : 1;
   Eina_Bool            underline : 1;
   Eina_Bool            underline2 : 1;
   Eina_Bool            strikethrough : 1;
   Eina_Bool            backing : 1;
   Eina_Bool            password : 1;
   Eina_Bool            halign_auto : 1;
};

struct _Evas_Textblock_Style
{
   const char            *style_text;
   char                  *default_tag;
   Evas_Object_Style_Tag *tags;
   Eina_List             *objects;
   Eina_Bool              delete_me : 1;
};

struct _Evas_Textblock_Cursor
{
   Evas_Object                     *obj;
   size_t                           pos;
   Evas_Object_Textblock_Node_Text *node;
};

struct _Evas_Object_Textblock
{
   DATA32                              magic;
   Evas_Textblock_Style               *style;
   Evas_Textblock_Cursor              *cursor;
   Eina_List                          *cursors;
   Evas_Object_Textblock_Node_Text    *text_nodes;
   Evas_Object_Textblock_Node_Format  *format_nodes;
   Evas_Object_Textblock_Paragraph    *paragraphs;
   Evas_Object_Textblock_Text_Item    *ellip_ti;
   Eina_Rbtree                        *par_index;
   Eina_List                          *anchors_a;
   Eina_List                          *anchors_item;
   int                                 last_w, last_h;
   struct {
      int                              l, r, t, b;
   } style_pad;
   double                              valign;
   char                               *markup_text;
   void                               *engine_data;
   const char                         *repch;
   const char                         *bidi_delimiters;
   struct {
      int                              w, h;
      Eina_Bool                        valid : 1;
   } formatted, native;
   Eina_Bool                           redraw : 1;
   Eina_Bool                           changed : 1;
   Eina_Bool                           content_changed : 1;
   Eina_Bool                           format_changed : 1;
   Eina_Bool                           have_ellipsis : 1;
   Eina_Bool                           newline_is_ps : 1;
};

/* private methods for textblock objects */
static void evas_object_textblock_init(Evas_Object *obj);
static void *evas_object_textblock_new(void);
static void evas_object_textblock_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_textblock_free(Evas_Object *obj);
static void evas_object_textblock_render_pre(Evas_Object *obj);
static void evas_object_textblock_render_post(Evas_Object *obj);

static unsigned int evas_object_textblock_id_get(Evas_Object *obj);
static unsigned int evas_object_textblock_visual_id_get(Evas_Object *obj);
static void *evas_object_textblock_engine_data_get(Evas_Object *obj);

static int evas_object_textblock_is_opaque(Evas_Object *obj);
static int evas_object_textblock_was_opaque(Evas_Object *obj);

static void evas_object_textblock_coords_recalc(Evas_Object *obj);

static void evas_object_textblock_scale_update(Evas_Object *obj);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_textblock_free,
     evas_object_textblock_render,
     evas_object_textblock_render_pre,
     evas_object_textblock_render_post,
     evas_object_textblock_id_get,
     evas_object_textblock_visual_id_get,
     evas_object_textblock_engine_data_get,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_textblock_is_opaque,
     evas_object_textblock_was_opaque,
     NULL,
     NULL,
     evas_object_textblock_coords_recalc,
     evas_object_textblock_scale_update,
     NULL,
     NULL,
     NULL
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



static Eina_Bool _evas_textblock_cursor_is_at_the_end(const Evas_Textblock_Cursor *cur);
static void _evas_textblock_node_text_remove(Evas_Object_Textblock *o, Evas_Object_Textblock_Node_Text *n);
static void _evas_textblock_node_text_remove_formats_between(Evas_Object_Textblock *o, Evas_Object_Textblock_Node_Text *n, int start, int end);
static Evas_Object_Textblock_Node_Format *_evas_textblock_cursor_node_format_before_or_at_pos_get(const Evas_Textblock_Cursor *cur);
static size_t _evas_textblock_node_format_pos_get(const Evas_Object_Textblock_Node_Format *fmt);
static void _evas_textblock_node_format_remove(Evas_Object_Textblock *o, Evas_Object_Textblock_Node_Format *n, int visual_adjustment);
static void _evas_textblock_node_format_free(Evas_Object_Textblock *o, Evas_Object_Textblock_Node_Format *n);
static void _evas_textblock_node_text_free(Evas_Object_Textblock_Node_Text *n);
static void _evas_textblock_changed(Evas_Object_Textblock *o, Evas_Object *obj);
static void _evas_textblock_invalidate_all(Evas_Object_Textblock *o);
static void _evas_textblock_cursors_update_offset(const Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Text *n, size_t start, int offset);
static void _evas_textblock_cursors_set_node(Evas_Object_Textblock *o, const Evas_Object_Textblock_Node_Text *n, Evas_Object_Textblock_Node_Text *new_node);

/* styles */
/**
 * @internal
 * Clears the textblock style passed except for the style_text which is replaced.
 * @param ts The ts to be cleared. Must not be NULL.
 * @param style_text the style's text.
 */
static void
_style_replace(Evas_Textblock_Style *ts, const char *style_text)
{
   eina_stringshare_replace(&ts->style_text, style_text);
   if (ts->default_tag) free(ts->default_tag);
   while (ts->tags)
     {
	Evas_Object_Style_Tag *tag;

	tag = (Evas_Object_Style_Tag *)ts->tags;
	ts->tags = (Evas_Object_Style_Tag *)eina_inlist_remove(EINA_INLIST_GET(ts->tags), EINA_INLIST_GET(tag));
	free(tag->tag);
	free(tag->replace);
	free(tag);
     }
   ts->default_tag = NULL;
   ts->tags = NULL;
}

/**
 * @internal
 * Clears the textblock style passed.
 * @param ts The ts to be cleared. Must not be NULL.
 */
static void
_style_clear(Evas_Textblock_Style *ts)
{
   _style_replace(ts, NULL);
}

/**
 * @internal
 * Searches inside the tags stored in the style for the tag matching s.
 * @param ts The ts to be cleared. Must not be NULL.
 * @param s The tag to be matched.
 * @param tag_len the length of the tag string.
 * @param[out] replace_len The length of the replcaement found. - Must not be NULL.
 * @return The replacement string found.
 */
static inline const char *
_style_match_tag(Evas_Textblock_Style *ts, const char *s, size_t tag_len, size_t *replace_len)
{
   Evas_Object_Style_Tag *tag;

   EINA_INLIST_FOREACH(ts->tags, tag)
     {
	if (tag->tag_len != tag_len) continue;
	if (!strcmp(tag->tag, s))
	  {
	     *replace_len = tag->replace_len;
	     return tag->replace;
	  }
     }
   *replace_len = 0;
   return NULL;
}

/**
 * @internal
 * Clears all the nodes (text and format) of the textblock object.
 * @param obj The evas object, must not be NULL.
 */
static void
_nodes_clear(const Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   while (o->text_nodes)
     {
	Evas_Object_Textblock_Node_Text *n;

	n = o->text_nodes;
        o->text_nodes = _NODE_TEXT(eina_inlist_remove(
                 EINA_INLIST_GET(o->text_nodes), EINA_INLIST_GET(n)));
        _evas_textblock_node_text_free(n);
     }
   while (o->format_nodes)
     {
        Evas_Object_Textblock_Node_Format *n;

        n = o->format_nodes;
        o->format_nodes = _NODE_FORMAT(eina_inlist_remove(EINA_INLIST_GET(o->format_nodes), EINA_INLIST_GET(n)));
        _evas_textblock_node_format_free(o, n);
     }
}

/**
 * @internal
 * Unrefs and frees (if needed) a textblock format.
 * @param obj The Evas_Object, Must not be NULL.
 * @param fmt the format to be cleaned, must not be NULL.
 */
static void
_format_unref_free(const Evas_Object *obj, Evas_Object_Textblock_Format *fmt)
{
   fmt->ref--;
   if (fmt->ref > 0) return;
   if (fmt->font.name) eina_stringshare_del(fmt->font.name);
   if (fmt->font.fallbacks) eina_stringshare_del(fmt->font.fallbacks);
   if (fmt->font.source) eina_stringshare_del(fmt->font.source);
   evas_font_free(obj->layer->evas, fmt->font.font);
   free(fmt);
}

/**
 * @internal
 * Free a layout item
 * @param obj The evas object, must not be NULL.
 * @param ln the layout line on which the item is in, must not be NULL.
 * @param it the layout item to be freed
 */
static void
_item_free(const Evas_Object *obj, Evas_Object_Textblock_Line *ln, Evas_Object_Textblock_Item *it)
{
   if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
     {
        Evas_Object_Textblock_Text_Item *ti = _ITEM_TEXT(it);

        evas_common_text_props_content_unref(&ti->text_props);
     }
   else
     {
        Evas_Object_Textblock_Format_Item *fi = _ITEM_FORMAT(it);

        if (fi->item) eina_stringshare_del(fi->item);
     }
   _format_unref_free(obj, it->format);
   if (ln)
     {
        ln->items = (Evas_Object_Textblock_Item *) eina_inlist_remove(
              EINA_INLIST_GET(ln->items), EINA_INLIST_GET(ln->items));
     }
   free(it);
}

/**
 * @internal
 * Free a layout line.
 * @param obj The evas object, must not be NULL.
 * @param ln the layout line to be freed, must not be NULL.
 */
static void
_line_free(Evas_Object_Textblock_Line *ln)
{
   /* Items are freed from the logical list, except for the ellip item */
   if (ln) free(ln);
}

/* table of html escapes (that i can find) this should be ordered with the
 * most common first as it's a linear search to match - no hash for this.
 *
 * these are stored as one large string and one additional array that
 * contains the offsets to the tokens for space efficiency.
 */
/**
 * @internal
 * @var escape_strings[]
 * This string consists of NULL terminated pairs of strings, the first of
 * every pair is an escape and the second is the value of the escape.
 */
static const char escape_strings[] =
/* most common escaped stuff */
"&quot;\0"     "\x22\0"
"&amp;\0"      "\x26\0"
"&lt;\0"       "\x3c\0"
"&gt;\0"       "\x3e\0"
/* all the rest */
"&nbsp;\0"     "\xc2\xa0\0"
"&iexcl;\0"    "\xc2\xa1\0"
"&cent;\0"     "\xc2\xa2\0"
"&pound;\0"    "\xc2\xa3\0"
"&curren;\0"   "\xc2\xa4\0"
"&yen;\0"      "\xc2\xa5\0"
"&brvbar;\0"   "\xc2\xa6\0"
"&sect;\0"     "\xc2\xa7\0"
"&uml;\0"      "\xc2\xa8\0"
"&copy;\0"     "\xc2\xa9\0"
"&ordf;\0"     "\xc2\xaa\0"
"&laquo;\0"    "\xc2\xab\0"
"&not;\0"      "\xc2\xac\0"
"&reg;\0"      "\xc2\xae\0"
"&macr;\0"     "\xc2\xaf\0"
"&deg;\0"      "\xc2\xb0\0"
"&plusmn;\0"   "\xc2\xb1\0"
"&sup2;\0"     "\xc2\xb2\0"
"&sup3;\0"     "\xc2\xb3\0"
"&acute;\0"    "\xc2\xb4\0"
"&micro;\0"    "\xc2\xb5\0"
"&para;\0"     "\xc2\xb6\0"
"&middot;\0"   "\xc2\xb7\0"
"&cedil;\0"    "\xc2\xb8\0"
"&sup1;\0"     "\xc2\xb9\0"
"&ordm;\0"     "\xc2\xba\0"
"&raquo;\0"    "\xc2\xbb\0"
"&frac14;\0"   "\xc2\xbc\0"
"&frac12;\0"   "\xc2\xbd\0"
"&frac34;\0"   "\xc2\xbe\0"
"&iquest;\0"   "\xc2\xbf\0"
"&Agrave;\0"   "\xc3\x80\0"
"&Aacute;\0"   "\xc3\x81\0"
"&Acirc;\0"    "\xc3\x82\0"
"&Atilde;\0"   "\xc3\x83\0"
"&Auml;\0"     "\xc3\x84\0"
"&Aring;\0"    "\xc3\x85\0"
"&Aelig;\0"    "\xc3\x86\0"
"&Ccedil;\0"   "\xc3\x87\0"
"&Egrave;\0"   "\xc3\x88\0"
"&Eacute;\0"   "\xc3\x89\0"
"&Ecirc;\0"    "\xc3\x8a\0"
"&Euml;\0"     "\xc3\x8b\0"
"&Igrave;\0"   "\xc3\x8c\0"
"&Iacute;\0"   "\xc3\x8d\0"
"&Icirc;\0"    "\xc3\x8e\0"
"&Iuml;\0"     "\xc3\x8f\0"
"&Eth;\0"      "\xc3\x90\0"
"&Ntilde;\0"   "\xc3\x91\0"
"&Ograve;\0"   "\xc3\x92\0"
"&Oacute;\0"   "\xc3\x93\0"
"&Ocirc;\0"    "\xc3\x94\0"
"&Otilde;\0"   "\xc3\x95\0"
"&Ouml;\0"     "\xc3\x96\0"
"&times;\0"    "\xc3\x97\0"
"&Oslash;\0"   "\xc3\x98\0"
"&Ugrave;\0"   "\xc3\x99\0"
"&Uacute;\0"   "\xc3\x9a\0"
"&Ucirc;\0"    "\xc3\x9b\0"
"&Yacute;\0"   "\xc3\x9d\0"
"&Thorn;\0"    "\xc3\x9e\0"
"&szlig;\0"    "\xc3\x9f\0"
"&agrave;\0"   "\xc3\xa0\0"
"&aacute;\0"   "\xc3\xa1\0"
"&acirc;\0"    "\xc3\xa2\0"
"&atilde;\0"   "\xc3\xa3\0"
"&auml;\0"     "\xc3\xa4\0"
"&aring;\0"    "\xc3\xa5\0"
"&aelig;\0"    "\xc3\xa6\0"
"&ccedil;\0"   "\xc3\xa7\0"
"&egrave;\0"   "\xc3\xa8\0"
"&eacute;\0"   "\xc3\xa9\0"
"&ecirc;\0"    "\xc3\xaa\0"
"&euml;\0"     "\xc3\xab\0"
"&igrave;\0"   "\xc3\xac\0"
"&iacute;\0"   "\xc3\xad\0"
"&icirc;\0"    "\xc3\xae\0"
"&iuml;\0"     "\xc3\xaf\0"
"&eth;\0"      "\xc3\xb0\0"
"&ntilde;\0"   "\xc3\xb1\0"
"&ograve;\0"   "\xc3\xb2\0"
"&oacute;\0"   "\xc3\xb3\0"
"&ocirc;\0"    "\xc3\xb4\0"
"&otilde;\0"   "\xc3\xb5\0"
"&ouml;\0"     "\xc3\xb6\0"
"&divide;\0"   "\xc3\xb7\0"
"&oslash;\0"   "\xc3\xb8\0"
"&ugrave;\0"   "\xc3\xb9\0"
"&uacute;\0"   "\xc3\xba\0"
"&ucirc;\0"    "\xc3\xbb\0"
"&uuml;\0"     "\xc3\xbc\0"
"&yacute;\0"   "\xc3\xbd\0"
"&thorn;\0"    "\xc3\xbe\0"
"&yuml;\0"     "\xc3\xbf\0"
"&alpha;\0"    "\xce\x91\0"
"&beta;\0"     "\xce\x92\0"
"&gamma;\0"    "\xce\x93\0"
"&delta;\0"    "\xce\x94\0"
"&epsilon;\0"  "\xce\x95\0"
"&zeta;\0"     "\xce\x96\0"
"&eta;\0"      "\xce\x97\0"
"&theta;\0"    "\xce\x98\0"
"&iota;\0"     "\xce\x99\0"
"&kappa;\0"    "\xce\x9a\0"
"&lambda;\0"   "\xce\x9b\0"
"&mu;\0"       "\xce\x9c\0"
"&nu;\0"       "\xce\x9d\0"
"&xi;\0"       "\xce\x9e\0"
"&omicron;\0"  "\xce\x9f\0"
"&pi;\0"       "\xce\xa0\0"
"&rho;\0"      "\xce\xa1\0"
"&sigma;\0"    "\xce\xa3\0"
"&tau;\0"      "\xce\xa4\0"
"&upsilon;\0"  "\xce\xa5\0"
"&phi;\0"      "\xce\xa6\0"
"&chi;\0"      "\xce\xa7\0"
"&psi;\0"      "\xce\xa8\0"
"&omega;\0"    "\xce\xa9\0"
"&hellip;\0"   "\xe2\x80\xa6\0"
"&euro;\0"     "\xe2\x82\xac\0"
"&larr;\0"     "\xe2\x86\x90\0"
"&uarr;\0"     "\xe2\x86\x91\0"
"&rarr;\0"     "\xe2\x86\x92\0"
"&darr;\0"     "\xe2\x86\x93\0"
"&harr;\0"     "\xe2\x86\x94\0"
"&larr;\0"     "\xe2\x87\x90\0"
"&rarr;\0"     "\xe2\x87\x92\0"
"&forall;\0"   "\xe2\x88\x80\0"
"&exist;\0"    "\xe2\x88\x83\0"
"&nabla;\0"    "\xe2\x88\x87\0"
"&prod;\0"     "\xe2\x88\x8f\0"
"&sum;\0"      "\xe2\x88\x91\0"
"&and;\0"      "\xe2\x88\xa7\0"
"&or;\0"       "\xe2\x88\xa8\0"
"&int;\0"      "\xe2\x88\xab\0"
"&ne;\0"       "\xe2\x89\xa0\0"
"&equiv;\0"    "\xe2\x89\xa1\0"
"&oplus;\0"    "\xe2\x8a\x95\0"
"&perp;\0"     "\xe2\x8a\xa5\0"
"&dagger;\0"   "\xe2\x80\xa0\0"
"&Dagger;\0"   "\xe2\x80\xa1\0"
"&bull;\0"     "\xe2\x80\xa2\0"
;

EVAS_MEMPOOL(_mp_obj);

/**
 * @internal
 * Checks if a char is a whitespace.
 * @param c the unicode codepoint.
 * @return EINA_TRUE if the unicode codepoint is a whitespace, EINA_FALSE otherwise.
 */
static Eina_Bool
_is_white(Eina_Unicode c)
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
     return EINA_TRUE;
   return EINA_FALSE;
}

/**
 * @internal
 * Prepends the text between s and p to the main cursor of the object.
 *
 * @param cur the cursor to prepend to.
 * @param[in] s start of the string
 * @param[in] p end of the string
 */
static void
_prepend_text_run(Evas_Textblock_Cursor *cur, const char *s, const char *p)
{
   if ((s) && (p > s))
     {
        char *ts;

        ts = alloca(p - s + 1);
        strncpy(ts, s, p - s);
        ts[p - s] = 0;
        evas_textblock_cursor_text_prepend(cur, ts);
     }
}


/**
 * @internal
 * Returns the numeric value of HEX chars for example for ch = 'A'
 * the function will return 10.
 *
 * @param ch The HEX char.
 * @return numeric value of HEX.
 */
static int
_hex_string_get(char ch)
{
   if ((ch >= '0') && (ch <= '9')) return (ch - '0');
   else if ((ch >= 'A') && (ch <= 'F')) return (ch - 'A' + 10);
   else if ((ch >= 'a') && (ch <= 'f')) return (ch - 'a' + 10);
   return 0;
}

/**
 * @internal
 * Parses a string of one of the formas:
 * 1. "#RRGGBB"
 * 2. "#RRGGBBAA"
 * 3. "#RGB"
 * 4. "#RGBA"
 * To the rgba values.
 *
 * @param[in] str The string to parse - NOT NULL.
 * @param[out] r The Red value - NOT NULL.
 * @param[out] g The Green value - NOT NULL.
 * @param[out] b The Blue value - NOT NULL.
 * @param[out] a The Alpha value - NOT NULL.
 */
static void
_format_color_parse(const char *str, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
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
   *r = (*r * *a) / 255;
   *g = (*g * *a) / 255;
   *b = (*b * *a) / 255;
}

/* The refcount for the formats. */
static int format_refcount = 0;
/* Holders for the stringshares */
static const char *fontstr = NULL;
static const char *font_fallbacksstr = NULL;
static const char *font_sizestr = NULL;
static const char *font_sourcestr = NULL;
static const char *colorstr = NULL;
static const char *underline_colorstr = NULL;
static const char *underline2_colorstr = NULL;
static const char *outline_colorstr = NULL;
static const char *shadow_colorstr = NULL;
static const char *glow_colorstr = NULL;
static const char *glow2_colorstr = NULL;
static const char *backing_colorstr = NULL;
static const char *strikethrough_colorstr = NULL;
static const char *alignstr = NULL;
static const char *valignstr = NULL;
static const char *wrapstr = NULL;
static const char *left_marginstr = NULL;
static const char *right_marginstr = NULL;
static const char *underlinestr = NULL;
static const char *strikethroughstr = NULL;
static const char *backingstr = NULL;
static const char *stylestr = NULL;
static const char *tabstopsstr = NULL;
static const char *linesizestr = NULL;
static const char *linerelsizestr = NULL;
static const char *linegapstr = NULL;
static const char *linerelgapstr = NULL;
static const char *itemstr = NULL;
static const char *linefillstr = NULL;
static const char *ellipsisstr = NULL;
static const char *passwordstr = NULL;

/**
 * @internal
 * Init the format strings.
 */
static void
_format_command_init(void)
{
   if (format_refcount == 0)
     {
        fontstr = eina_stringshare_add("font");
        font_fallbacksstr = eina_stringshare_add("font_fallbacks");
        font_sizestr = eina_stringshare_add("font_size");
        font_sourcestr = eina_stringshare_add("font_source");
        colorstr = eina_stringshare_add("color");
        underline_colorstr = eina_stringshare_add("underline_color");
        underline2_colorstr = eina_stringshare_add("underline2_color");
        outline_colorstr = eina_stringshare_add("outline_color");
        shadow_colorstr = eina_stringshare_add("shadow_color");
        glow_colorstr = eina_stringshare_add("glow_color");
        glow2_colorstr = eina_stringshare_add("glow2_color");
        backing_colorstr = eina_stringshare_add("backing_color");
        strikethrough_colorstr = eina_stringshare_add("strikethrough_color");
        alignstr = eina_stringshare_add("align");
        valignstr = eina_stringshare_add("valign");
        wrapstr = eina_stringshare_add("wrap");
        left_marginstr = eina_stringshare_add("left_margin");
        right_marginstr = eina_stringshare_add("right_margin");
        underlinestr = eina_stringshare_add("underline");
        strikethroughstr = eina_stringshare_add("strikethrough");
        backingstr = eina_stringshare_add("backing");
        stylestr = eina_stringshare_add("style");
        tabstopsstr = eina_stringshare_add("tabstops");
        linesizestr = eina_stringshare_add("linesize");
        linerelsizestr = eina_stringshare_add("linerelsize");
        linegapstr = eina_stringshare_add("linegap");
        linerelgapstr = eina_stringshare_add("linerelgap");
        itemstr = eina_stringshare_add("item");
        linefillstr = eina_stringshare_add("linefill");
        ellipsisstr = eina_stringshare_add("ellipsis");
        passwordstr = eina_stringshare_add("password");
     }
   format_refcount++;
}

/**
 * @internal
 * Shutdown the format strings.
 */
static void
_format_command_shutdown(void)
{
   if (--format_refcount > 0) return;

   eina_stringshare_del(fontstr);
   eina_stringshare_del(font_fallbacksstr);
   eina_stringshare_del(font_sizestr);
   eina_stringshare_del(font_sourcestr);
   eina_stringshare_del(colorstr);
   eina_stringshare_del(underline_colorstr);
   eina_stringshare_del(underline2_colorstr);
   eina_stringshare_del(outline_colorstr);
   eina_stringshare_del(shadow_colorstr);
   eina_stringshare_del(glow_colorstr);
   eina_stringshare_del(glow2_colorstr);
   eina_stringshare_del(backing_colorstr);
   eina_stringshare_del(strikethrough_colorstr);
   eina_stringshare_del(alignstr);
   eina_stringshare_del(valignstr);
   eina_stringshare_del(wrapstr);
   eina_stringshare_del(left_marginstr);
   eina_stringshare_del(right_marginstr);
   eina_stringshare_del(underlinestr);
   eina_stringshare_del(strikethroughstr);
   eina_stringshare_del(backingstr);
   eina_stringshare_del(stylestr);
   eina_stringshare_del(tabstopsstr);
   eina_stringshare_del(linesizestr);
   eina_stringshare_del(linerelsizestr);
   eina_stringshare_del(linegapstr);
   eina_stringshare_del(linerelgapstr);
   eina_stringshare_del(itemstr);
   eina_stringshare_del(linefillstr);
   eina_stringshare_del(ellipsisstr);
   eina_stringshare_del(passwordstr);
}

/**
 * @internal
 * Copies str to dst while removing the \\ char, i.e unescape the escape sequences.
 *
 * @param[out] dst the destination string - Should not be NULL.
 * @param[in] src the source string - Should not be NULL.
 */
static void
_format_clean_param(char *dst, const char *src)
{
   const char *ss;
   char *ds;

   ds = dst;
   for (ss = src; *ss; ss++, ds++)
     {
        if ((*ss == '\\') && *(ss + 1)) ss++;
        *ds = *ss;
     }
   *ds = 0;
}

/**
 * @internal
 * Parses the cmd and parameter and adds the parsed format to fmt.
 *
 * @param obj the evas object - should not be NULL.
 * @param fmt The format to populate - should not be NULL.
 * @param[in] cmd the command to process, should be stringshared.
 * @param[in] param the parameter of the command.
 */
static void
_format_command(Evas_Object *obj, Evas_Object_Textblock_Format *fmt, const char *cmd, const char *param)
{
   int new_font = 0;
   int len;
   char *tmp_param;

   len = strlen(param);
   tmp_param = alloca(len + 1);

   _format_clean_param(tmp_param, param);
   if (cmd == fontstr)
     {
        if ((!fmt->font.name) ||
              ((fmt->font.name) && (strcmp(fmt->font.name, tmp_param))))
          {
             if (fmt->font.name) eina_stringshare_del(fmt->font.name);
             fmt->font.name = eina_stringshare_add(tmp_param);
             new_font = 1;
          }
     }
   else if (cmd == font_fallbacksstr)
     {
        if ((!fmt->font.fallbacks) ||
              ((fmt->font.fallbacks) && (strcmp(fmt->font.fallbacks, tmp_param))))
          {
             /* policy - when we say "fallbacks" do we prepend and use prior
              * fallbacks... or should we replace. for now we replace
              */
             if (fmt->font.fallbacks) eina_stringshare_del(fmt->font.fallbacks);
             fmt->font.fallbacks = eina_stringshare_add(tmp_param);
             new_font = 1;
          }
     }
   else if (cmd == font_sizestr)
     {
        int v;

        v = atoi(tmp_param);
        if (v != fmt->font.size)
          {
             fmt->font.size = v;
             new_font = 1;
          }
     }
   else if (cmd == font_sourcestr)
     {
        if ((!fmt->font.source) ||
              ((fmt->font.source) && (strcmp(fmt->font.source, tmp_param))))
          {
             if (fmt->font.source) eina_stringshare_del(fmt->font.source);
             fmt->font.source = eina_stringshare_add(tmp_param);
             new_font = 1;
          }
     }
   else if (cmd == colorstr)
     _format_color_parse(tmp_param,
           &(fmt->color.normal.r), &(fmt->color.normal.g),
           &(fmt->color.normal.b), &(fmt->color.normal.a));
   else if (cmd == underline_colorstr)
     _format_color_parse(tmp_param,
           &(fmt->color.underline.r), &(fmt->color.underline.g),
           &(fmt->color.underline.b), &(fmt->color.underline.a));
   else if (cmd == underline2_colorstr)
     _format_color_parse(tmp_param,
           &(fmt->color.underline2.r), &(fmt->color.underline2.g),
           &(fmt->color.underline2.b), &(fmt->color.underline2.a));
   else if (cmd == outline_colorstr)
     _format_color_parse(tmp_param,
           &(fmt->color.outline.r), &(fmt->color.outline.g),
           &(fmt->color.outline.b), &(fmt->color.outline.a));
   else if (cmd == shadow_colorstr)
     _format_color_parse(tmp_param,
           &(fmt->color.shadow.r), &(fmt->color.shadow.g),
           &(fmt->color.shadow.b), &(fmt->color.shadow.a));
   else if (cmd == glow_colorstr)
     _format_color_parse(tmp_param,
           &(fmt->color.glow.r), &(fmt->color.glow.g),
           &(fmt->color.glow.b), &(fmt->color.glow.a));
   else if (cmd == glow2_colorstr)
     _format_color_parse(tmp_param,
           &(fmt->color.glow2.r), &(fmt->color.glow2.g),
           &(fmt->color.glow2.b), &(fmt->color.glow2.a));
   else if (cmd == backing_colorstr)
     _format_color_parse(tmp_param,
           &(fmt->color.backing.r), &(fmt->color.backing.g),
           &(fmt->color.backing.b), &(fmt->color.backing.a));
   else if (cmd == strikethrough_colorstr)
     _format_color_parse(tmp_param,
           &(fmt->color.strikethrough.r), &(fmt->color.strikethrough.g),
           &(fmt->color.strikethrough.b), &(fmt->color.strikethrough.a));
   else if (cmd == alignstr)
     {
        if (!strcmp(tmp_param, "auto"))
          {
             fmt->halign_auto = EINA_TRUE;
          }
        else
          {
             if (!strcmp(tmp_param, "middle")) fmt->halign = 0.5;
             else if (!strcmp(tmp_param, "center")) fmt->halign = 0.5;
             else if (!strcmp(tmp_param, "left")) fmt->halign = 0.0;
             else if (!strcmp(tmp_param, "right")) fmt->halign = 1.0;
             else
               {
                  char *endptr = NULL;
                  double val = strtod(tmp_param, &endptr);
                  if (endptr)
                    {
                       while (*endptr && _is_white(*endptr))
                         endptr++;
                       if (*endptr == '%')
                         val /= 100.0;
                    }
                  fmt->halign = val;
                  if (fmt->halign < 0.0) fmt->halign = 0.0;
                  else if (fmt->halign > 1.0) fmt->halign = 1.0;
               }
             fmt->halign_auto = EINA_FALSE;
          }
     }
   else if (cmd == valignstr)
     {
        if (!strcmp(tmp_param, "top")) fmt->valign = 0.0;
        else if (!strcmp(tmp_param, "middle")) fmt->valign = 0.5;
        else if (!strcmp(tmp_param, "center")) fmt->valign = 0.5;
        else if (!strcmp(tmp_param, "bottom")) fmt->valign = 1.0;
        else if (!strcmp(tmp_param, "baseline")) fmt->valign = -1.0;
        else if (!strcmp(tmp_param, "base")) fmt->valign = -1.0;
        else
          {
             char *endptr = NULL;
             double val = strtod(tmp_param, &endptr);
             if (endptr)
               {
                  while (*endptr && _is_white(*endptr))
                    endptr++;
                  if (*endptr == '%')
                    val /= 100.0;
               }
             fmt->valign = val;
             if (fmt->valign < 0.0) fmt->valign = 0.0;
             else if (fmt->valign > 1.0) fmt->valign = 1.0;
          }
     }
   else if (cmd == wrapstr)
     {
        if (!strcmp(tmp_param, "word"))
          {
             fmt->wrap_word = 1;
             fmt->wrap_char = fmt->wrap_mixed = 0;
          }
        else if (!strcmp(tmp_param, "char"))
          {
             fmt->wrap_word = fmt->wrap_mixed = 0;
             fmt->wrap_char = 1;
          }
        else if (!strcmp(tmp_param, "mixed"))
          {
             fmt->wrap_word = fmt->wrap_char = 0;
             fmt->wrap_mixed = 1;
          }
        else
          {
             fmt->wrap_word = fmt->wrap_mixed = fmt->wrap_char = 0;
          }
     }
   else if (cmd == left_marginstr)
     {
        if (!strcmp(tmp_param, "reset"))
          fmt->margin.l = 0;
        else
          {
             if (tmp_param[0] == '+')
               fmt->margin.l += atoi(&(tmp_param[1]));
             else if (tmp_param[0] == '-')
               fmt->margin.l -= atoi(&(tmp_param[1]));
             else
               fmt->margin.l = atoi(tmp_param);
             if (fmt->margin.l < 0) fmt->margin.l = 0;
          }
     }
   else if (cmd == right_marginstr)
     {
        if (!strcmp(tmp_param, "reset"))
          fmt->margin.r = 0;
        else
          {
             if (tmp_param[0] == '+')
               fmt->margin.r += atoi(&(tmp_param[1]));
             else if (tmp_param[0] == '-')
               fmt->margin.r -= atoi(&(tmp_param[1]));
             else
               fmt->margin.r = atoi(tmp_param);
             if (fmt->margin.r < 0) fmt->margin.r = 0;
          }
     }
   else if (cmd == underlinestr)
     {
        if (!strcmp(tmp_param, "off"))
          {
             fmt->underline = 0;
             fmt->underline2 = 0;
          }
        else if ((!strcmp(tmp_param, "on")) ||
              (!strcmp(tmp_param, "single")))
          {
             fmt->underline = 1;
             fmt->underline2 = 0;
          }
        else if (!strcmp(tmp_param, "double"))
          {
             fmt->underline = 1;
             fmt->underline2 = 1;
          }
     }
   else if (cmd == strikethroughstr)
     {
        if (!strcmp(tmp_param, "off"))
          fmt->strikethrough = 0;
        else if (!strcmp(tmp_param, "on"))
          fmt->strikethrough = 1;
     }
   else if (cmd == backingstr)
     {
        if (!strcmp(tmp_param, "off"))
          fmt->backing = 0;
        else if (!strcmp(tmp_param, "on"))
          fmt->backing = 1;
     }
   else if (cmd == stylestr)
     {
        char *p1, *p2, *p, *pp;

        p1 = alloca(len + 1);
        *p1 = 0;
        p2 = alloca(len + 1);
        *p2 = 0;
        /* no comma */
        if (!strstr(tmp_param, ",")) p1 = tmp_param;
        else
          {
             /* split string "str1,str2" into p1 and p2 (if we have more than
              * 1 str2 eg "str1,str2,str3,str4" then we don't care. p2 just
              * ends up being the last one as right now it's only valid to have
              * 1 comma and 2 strings */
             pp = p1;
             for (p = tmp_param; *p; p++)
               {
                  if (*p == ',')
                    {
                       *pp = 0;
                       pp = p2;
                       continue;
                    }
                  *pp = *p;
                  pp++;
               }
             *pp = 0;
          }
        if      (!strcmp(p1, "off"))                 fmt->style = EVAS_TEXT_STYLE_PLAIN;
        else if (!strcmp(p1, "none"))                fmt->style = EVAS_TEXT_STYLE_PLAIN;
        else if (!strcmp(p1, "plain"))               fmt->style = EVAS_TEXT_STYLE_PLAIN;
        else if (!strcmp(p1, "shadow"))              fmt->style = EVAS_TEXT_STYLE_SHADOW;
        else if (!strcmp(p1, "outline"))             fmt->style = EVAS_TEXT_STYLE_OUTLINE;
        else if (!strcmp(p1, "soft_outline"))        fmt->style = EVAS_TEXT_STYLE_SOFT_OUTLINE;
        else if (!strcmp(p1, "outline_shadow"))      fmt->style = EVAS_TEXT_STYLE_OUTLINE_SHADOW;
        else if (!strcmp(p1, "outline_soft_shadow")) fmt->style = EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW;
        else if (!strcmp(p1, "glow"))                fmt->style = EVAS_TEXT_STYLE_GLOW;
        else if (!strcmp(p1, "far_shadow"))          fmt->style = EVAS_TEXT_STYLE_FAR_SHADOW;
        else if (!strcmp(p1, "soft_shadow"))         fmt->style = EVAS_TEXT_STYLE_SOFT_SHADOW;
        else if (!strcmp(p1, "far_soft_shadow"))     fmt->style = EVAS_TEXT_STYLE_FAR_SOFT_SHADOW;
        else                                         fmt->style = EVAS_TEXT_STYLE_PLAIN;
        
        if (*p2)
          {
             if      (!strcmp(p2, "bottom_right")) EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT);
             else if (!strcmp(p2, "bottom"))       EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM);
             else if (!strcmp(p2, "bottom_left"))  EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT);
             else if (!strcmp(p2, "left"))         EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT);
             else if (!strcmp(p2, "top_left"))     EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT);
             else if (!strcmp(p2, "top"))          EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP);
             else if (!strcmp(p2, "top_right"))    EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT);
             else if (!strcmp(p2, "right"))        EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT);
             else                                  EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET(fmt->style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT);
          }
     }
   else if (cmd == tabstopsstr)
     {
        fmt->tabstops = atoi(tmp_param);
        if (fmt->tabstops < 1) fmt->tabstops = 1;
     }
   else if (cmd == linesizestr)
     {
        fmt->linesize = atoi(tmp_param);
        fmt->linerelsize = 0.0;
     }
   else if (cmd == linerelsizestr)
     {
        char *endptr = NULL;
        double val = strtod(tmp_param, &endptr);
        if (endptr)
          {
             while (*endptr && _is_white(*endptr))
               endptr++;
             if (*endptr == '%')
               {
                  fmt->linerelsize = val / 100.0;
                  fmt->linesize = 0;
                  if (fmt->linerelsize < 0.0) fmt->linerelsize = 0.0;
               }
          }
     }
   else if (cmd == linegapstr)
     {
        fmt->linegap = atoi(tmp_param);
        fmt->linerelgap = 0.0;
     }
   else if (cmd == linerelgapstr)
     {
        char *endptr = NULL;
        double val = strtod(tmp_param, &endptr);
        if (endptr)
          {
             while (*endptr && _is_white(*endptr))
               endptr++;
             if (*endptr == '%')
               {
                  fmt->linerelgap = val / 100.0;
                  fmt->linegap = 0;
                  if (fmt->linerelgap < 0.0) fmt->linerelgap = 0.0;
               }
          }
     }
   else if (cmd == itemstr)
     {
        // itemstr == replacement object items in textblock - inline imges
        // for example
     }
   else if (cmd == linefillstr)
     {
        char *endptr = NULL;
        double val = strtod(tmp_param, &endptr);
        if (endptr)
          {
             while (*endptr && _is_white(*endptr))
               endptr++;
             if (*endptr == '%')
               {
                  fmt->linefill = val / 100.0;
                  if (fmt->linefill < 0.0) fmt->linefill = 0.0;
               }
          }
     }
   else if (cmd == ellipsisstr)
     {
        char *endptr = NULL;
        fmt->ellipsis = strtod(tmp_param, &endptr);
        if ((fmt->ellipsis < 0.0) || (fmt->ellipsis > 1.0))
          fmt->ellipsis = -1.0;
        else
          {
             Evas_Object_Textblock *o;
             
             o = (Evas_Object_Textblock *)(obj->object_data);
             o->have_ellipsis = 1;
          }
     }
   else if (cmd == passwordstr)
     {
        if (!strcmp(tmp_param, "off"))
          fmt->password = 0;
        else if (!strcmp(tmp_param, "on"))
          fmt->password = 1;
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
              buf, fmt->font.source,
              (int)(((double)fmt->font.size) * obj->cur.scale));
        if (buf) free(buf);
        if (of) evas_font_free(obj->layer->evas, of);
     }
}

/**
 * @internal
 * Returns #EINA_TRUE if the item is a format parameter, #EINA_FALSE otherwise.
 *
 * @param[in] item the item to check - Not NULL.
 */
static Eina_Bool
_format_is_param(const char *item)
{
   if (strchr(item, '=')) return EINA_TRUE;
   return EINA_FALSE;
}

/**
 * @internal
 * Parse the format item and populate key and val with the stringshares that
 * corrospond to the formats parsed.
 * It expects item to be of the structure:
 * "key=val"
 *
 * @param[in] item the item to parse - Not NULL.
 * @param[out] key where to store the key at - Not NULL.
 * @param[out] val where to store the value at - Not NULL.
 */
static void
_format_param_parse(const char *item, const char **key, const char **val)
{
   const char *equal, *end;

   equal = strchr(item, '=');
   *key = eina_stringshare_add_length(item, equal - item);
   equal++; /* Advance after the '=' */
   /* Null terminate before the spaces */
   end = strchr(equal, ' ');
   if (end)
     {
        *val = eina_stringshare_add_length(equal, end - equal);
     }
   else
     {
        *val = eina_stringshare_add(equal);
     }
}

/**
 * @internal
 * FIXME: comment.
 */
static const char *
_format_parse(const char **s)
{
   const char *p, *item;
   const char *s1 = NULL, *s2 = NULL;

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
             item = s1;

             *s = s2;
             return item;
          }
     }
   *s = p;
   return NULL;
}

/**
 * @internal
 * Parse the format str and populate fmt with the formats found.
 *
 * @param obj The evas object - Not NULL.
 * @param[out] fmt The format to populate - Not NULL.
 * @param[in] str the string to parse.- Not NULL.
 */
static void
_format_fill(Evas_Object *obj, Evas_Object_Textblock_Format *fmt, const char *str)
{
   const char *s;
   const char *item;

   s = str;

   /* get rid of anything +s or -s off the start of the string */
   while ((*s == ' ') || (*s == '+') || (*s == '-')) s++;

   while ((item = _format_parse(&s)))
     {
        if (_format_is_param(item))
          {
             const char *key = NULL, *val = NULL;

             _format_param_parse(item, &key, &val);
             _format_command(obj, fmt, key, val);
             eina_stringshare_del(key);
             eina_stringshare_del(val);
          }
        else
          {
             /* immediate - not handled here */
          }
     }
}

/**
 * @internal
 * Duplicate a format and return the duplicate.
 *
 * @param obj The evas object - Not NULL.
 * @param[in] fmt The format to duplicate - Not NULL.
 * @return the copy of the format.
 */
static Evas_Object_Textblock_Format *
_format_dup(Evas_Object *obj, const Evas_Object_Textblock_Format *fmt)
{
   Evas_Object_Textblock_Format *fmt2;
   char *buf = NULL;

   fmt2 = calloc(1, sizeof(Evas_Object_Textblock_Format));
   memcpy(fmt2, fmt, sizeof(Evas_Object_Textblock_Format));
   fmt2->ref = 1;
   if (fmt->font.name) fmt2->font.name = eina_stringshare_add(fmt->font.name);
   if (fmt->font.fallbacks) fmt2->font.fallbacks = eina_stringshare_add(fmt->font.fallbacks);
   if (fmt->font.source) fmt2->font.source = eina_stringshare_add(fmt->font.source);

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
         buf, fmt2->font.source,
         (int)(((double)fmt2->font.size) * obj->cur.scale));
   if (buf) free(buf);
   return fmt2;
}




/**
 * @internal
 * @typedef Ctxt
 *
 * A pack of information that needed to be passed around in the layout engine,
 * packed for easier access.
 */
typedef struct _Ctxt Ctxt;

struct _Ctxt
{
   Evas_Object *obj;
   Evas_Object_Textblock *o;

   Evas_Object_Textblock_Paragraph *paragraphs;
   Evas_Object_Textblock_Paragraph *par;
   Evas_Object_Textblock_Line *ln;


   Eina_List *format_stack;
   Evas_Object_Textblock_Format *fmt;

   int x, y;
   int w, h;
   int wmax, hmax;
   int maxascent, maxdescent;
   int marginl, marginr;
   int line_no;
   int underline_extend;
   int have_underline, have_underline2;
   double align, valign;
   Eina_Bool align_auto : 1;
   Eina_Bool width_changed : 1;
};

static void _layout_text_add_logical_item(Ctxt *c, Evas_Object_Textblock_Text_Item *ti, Eina_List *rel);
static void _text_item_update_sizes(Ctxt *c, Evas_Object_Textblock_Text_Item *ti);
static void _layout_do_format(const Evas_Object *obj, Ctxt *c, Evas_Object_Textblock_Format **_fmt, Evas_Object_Textblock_Node_Format *n, int *style_pad_l, int *style_pad_r, int *style_pad_t, int *style_pad_b, Eina_Bool create_item);
/**
 * @internal
 * Adjust the ascent/descent of the format and context.
 *
 * @param maxascent The ascent to update - Not NUL.
 * @param maxdescent The descent to update - Not NUL.
 * @param fmt The format to adjust - NOT NULL.
 */
static void
_layout_format_ascent_descent_adjust(const Evas_Object *obj,
      Evas_Coord *maxascent, Evas_Coord *maxdescent,
      Evas_Object_Textblock_Format *fmt)
{
   int ascent, descent;

   if (fmt->font.font)
     {
        //	ascent = c->ENFN->font_max_ascent_get(c->ENDT, fmt->font.font);
        //	descent = c->ENFN->font_max_descent_get(c->ENDT, fmt->font.font);
        ascent = ENFN->font_ascent_get(ENDT, fmt->font.font);
        descent = ENFN->font_descent_get(ENDT, fmt->font.font);
        if (fmt->linesize > 0)
          {
             if ((ascent + descent) < fmt->linesize)
               {
                  ascent = ((fmt->linesize * ascent) / (ascent + descent));
                  descent = fmt->linesize - ascent;
               }
          }
        else if (fmt->linerelsize > 0.0)
          {
             descent = descent * fmt->linerelsize;
             ascent = ascent * fmt->linerelsize;
          }
        descent += fmt->linegap;
        descent += ((ascent + descent) * fmt->linerelgap);
        if (*maxascent < ascent) *maxascent = ascent;
        if (*maxdescent < descent) *maxdescent = descent;
        if (fmt->linefill > 0.0)
          {
             int dh;

             dh = obj->cur.geometry.h - (*maxascent + *maxdescent);
             if (dh < 0) dh = 0;
             dh = fmt->linefill * dh;
             *maxdescent += dh / 2;
             *maxascent += dh - (dh / 2);
             // FIXME: set flag that says "if heigh changes - reformat"
          }
     }
}

/**
 * @internal
 * Create a new line using the info from the format and update the format
 * and context.
 *
 * @param c The context to work on - Not NULL.
 * @param fmt The format to use info from - NOT NULL.
 */
static void
_layout_line_new(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   c->ln = calloc(1, sizeof(Evas_Object_Textblock_Line));
   c->align = fmt->halign;
   c->align_auto = fmt->halign_auto;
   c->marginl = fmt->margin.l;
   c->marginr = fmt->margin.r;
   c->par->lines = (Evas_Object_Textblock_Line *)eina_inlist_append(EINA_INLIST_GET(c->par->lines), EINA_INLIST_GET(c->ln));
   c->x = 0;
   c->maxascent = c->maxdescent = 0;
   c->ln->line_no = -1;
   c->ln->par = c->par;
}

/* par index functions */
static Eina_Rbtree_Direction
_par_index_node_cmp(const Eina_Rbtree *left, const Eina_Rbtree *right,
      void *data __UNUSED__)
{
   Evas_Object_Textblock_Paragraph *lpar, *rpar;
   lpar = EINA_RBTREE_CONTAINER_GET(left, Evas_Object_Textblock_Paragraph);
   rpar = EINA_RBTREE_CONTAINER_GET(right, Evas_Object_Textblock_Paragraph);
   /* Because they can't be equal or overlap, we don't need to compare
    * anything except for the y position */
   return (lpar->y < rpar->y) ? EINA_RBTREE_LEFT : EINA_RBTREE_RIGHT;
}

static int
_par_index_y_key_cmp(const Eina_Rbtree *node, const void *key,
      int length __UNUSED__, void *data __UNUSED__)
{
   Evas_Coord y = *((const Evas_Coord *) key);
   Evas_Object_Textblock_Paragraph *par;
   par = EINA_RBTREE_CONTAINER_GET(node, Evas_Object_Textblock_Paragraph);

   if (y < par->y)
      return 1;
   else if ((par->y <= y) && (y < par->y + par->h))
      return 0;
   else
      return -1;
}

static int
_par_index_line_no_key_cmp(const Eina_Rbtree *node, const void *key,
      int length __UNUSED__, void *data __UNUSED__)
{
   int line_no = *((const int *) key);
   Evas_Object_Textblock_Paragraph *par, *npar;
   par = EINA_RBTREE_CONTAINER_GET(node, Evas_Object_Textblock_Paragraph);
   npar = (Evas_Object_Textblock_Paragraph *) EINA_INLIST_GET(par)->next;

   if (line_no < par->line_no)
      return 1;
   else if ((par->line_no <= par->line_no) &&
         (!npar || (line_no < npar->line_no)))
      return 0;
   else return -1;
}

static inline Evas_Object_Textblock_Paragraph *
_layout_find_paragraph_by_y(Evas_Object_Textblock *o, Evas_Coord y)
{
   Eina_Rbtree *tmp = eina_rbtree_inline_lookup(o->par_index, &y, 0,
         _par_index_y_key_cmp, NULL);

   return (tmp) ?
      EINA_RBTREE_CONTAINER_GET(tmp, Evas_Object_Textblock_Paragraph) :
      NULL;
}

static inline Evas_Object_Textblock_Paragraph *
_layout_find_paragraph_by_line_no(Evas_Object_Textblock *o, int line_no)
{
   Eina_Rbtree *tmp = eina_rbtree_inline_lookup(o->par_index,
         &line_no, 0, _par_index_line_no_key_cmp, NULL);

   return (tmp) ?
      EINA_RBTREE_CONTAINER_GET(tmp, Evas_Object_Textblock_Paragraph) :
      NULL;
}
/* End of rbtree index functios */

/**
 * @internal
 * Create a new layout paragraph.
 * If c->par is not NULL, the paragraph is appended/prepended according
 * to the append parameter. If it is NULL, the paragraph is appended at
 * the end of the list.
 *
 * @param c The context to work on - Not NULL.
 * @param n the associated text node
 * @param append true to append, false to prpend.
 */
static void
_layout_paragraph_new(Ctxt *c, Evas_Object_Textblock_Node_Text *n,
      Eina_Bool append)
{
   Evas_Object_Textblock_Paragraph *rel_par = c->par;
   c->par = calloc(1, sizeof(Evas_Object_Textblock_Paragraph));
   if (append || !rel_par)
      c->paragraphs = (Evas_Object_Textblock_Paragraph *)
         eina_inlist_append_relative(EINA_INLIST_GET(c->paragraphs),
               EINA_INLIST_GET(c->par),
               EINA_INLIST_GET(rel_par));
   else
      c->paragraphs = (Evas_Object_Textblock_Paragraph *)
         eina_inlist_prepend_relative(EINA_INLIST_GET(c->paragraphs),
               EINA_INLIST_GET(c->par),
               EINA_INLIST_GET(rel_par));

   c->ln = NULL;
   c->par->text_node = n;
   if (n)
      n->par = c->par;
   c->par->line_no = -1;
   c->par->visible = 1;
}

#ifdef BIDI_SUPPORT
/**
 * @internal
 * Update bidi paragraph props.
 *
 * @param par The paragraph to update
 */
static inline void
_layout_update_bidi_props(const Evas_Object_Textblock *o,
      Evas_Object_Textblock_Paragraph *par)
{
   if (par->text_node)
     {
        const Eina_Unicode *text;
        int *segment_idxs = NULL;
        text = eina_ustrbuf_string_get(par->text_node->unicode);

        if (o->bidi_delimiters)
           segment_idxs = evas_bidi_segment_idxs_get(text, o->bidi_delimiters);

        evas_bidi_paragraph_props_unref(par->bidi_props);
        par->bidi_props = evas_bidi_paragraph_props_get(text,
              eina_ustrbuf_length_get(par->text_node->unicode),
              segment_idxs);
        par->direction = EVAS_BIDI_PARAGRAPH_DIRECTION_IS_RTL(par->bidi_props) ?
           EVAS_BIDI_DIRECTION_RTL : EVAS_BIDI_DIRECTION_LTR;
        par->is_bidi = !!par->bidi_props;
        if (segment_idxs) free(segment_idxs);
     }
}
#endif


/**
 * @internal
 * Free the visual lines in the paragraph (logical items are kept)
 */
static void
_paragraph_clear(const Evas_Object *obj __UNUSED__,
      Evas_Object_Textblock_Paragraph *par)
{
   while (par->lines)
     {
        Evas_Object_Textblock_Line *ln;

        ln = (Evas_Object_Textblock_Line *) par->lines;
        par->lines = (Evas_Object_Textblock_Line *)eina_inlist_remove(EINA_INLIST_GET(par->lines), EINA_INLIST_GET(par->lines));
        _line_free(ln);
     }
}

/**
 * @internal
 * Free the layout paragraph and all of it's lines and logical items.
 */
static void
_paragraph_free(const Evas_Object *obj, Evas_Object_Textblock_Paragraph *par)
{
   Evas_Object_Textblock *o;
   o = (Evas_Object_Textblock *)(obj->object_data);
   _paragraph_clear(obj, par);

     {
        Eina_List *i, *i_prev;
        Evas_Object_Textblock_Item *it;
        EINA_LIST_FOREACH_SAFE(par->logical_items, i, i_prev, it)
          {
             _item_free(obj, NULL, it);
          }
        eina_list_free(par->logical_items);
     }
#ifdef BIDI_SUPPORT
   if (par->bidi_props)
      evas_bidi_paragraph_props_unref(par->bidi_props);
#endif
   /* If we are the active par of the text node, set to NULL */
   if (par->text_node && (par->text_node->par == par))
      par->text_node->par = NULL;
   o->par_index = eina_rbtree_inline_remove(o->par_index,
            EINA_RBTREE_GET(par), _par_index_node_cmp, NULL);

   free(par);
}

/**
 * @internal
 * Clear all the paragraphs from the inlist pars.
 *
 * @param obj the evas object - Not NULL.
 * @param pars the paragraphs to clean - Not NULL.
 */
static void
_paragraphs_clear(const Evas_Object *obj, Evas_Object_Textblock_Paragraph *pars)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Paragraph *par;
   o = (Evas_Object_Textblock *)(obj->object_data);

   EINA_INLIST_FOREACH(EINA_INLIST_GET(pars), par)
     {
        _paragraph_clear(obj, par);
     }
}

/**
 * @internal
 * Free the paragraphs from the inlist pars, the difference between this and
 * _paragraphs_clear is that the latter keeps the logical items and the par
 * items, while the former frees them as well.
 *
 * @param obj the evas object - Not NULL.
 * @param pars the paragraphs to clean - Not NULL.
 */
static void
_paragraphs_free(const Evas_Object *obj, Evas_Object_Textblock_Paragraph *pars)
{
   Evas_Object_Textblock *o;
   o = (Evas_Object_Textblock *)(obj->object_data);
   o->par_index = NULL;

   while (pars)
     {
        Evas_Object_Textblock_Paragraph *par;

        par = (Evas_Object_Textblock_Paragraph *) pars;
        pars = (Evas_Object_Textblock_Paragraph *)eina_inlist_remove(EINA_INLIST_GET(pars), EINA_INLIST_GET(par));
        _paragraph_free(obj, par);
     }
}

/**
 * @internal
 * Push fmt to the format stack, if fmt is NULL, will fush a default item.
 *
 * @param c the context to work on - Not NULL.
 * @param fmt the format to push.
 * @see _layout_format_pop()
 */
static Evas_Object_Textblock_Format *
_layout_format_push(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      Evas_Object_Textblock_Node_Format *fnode)
{
   if (fmt)
     {
        fmt = _format_dup(c->obj, fmt);
        c->format_stack  = eina_list_prepend(c->format_stack, fmt);
        fmt->fnode = fnode;
     }
   else
     {
        fmt = calloc(1, sizeof(Evas_Object_Textblock_Format));
        c->format_stack  = eina_list_prepend(c->format_stack, fmt);
        fmt->ref = 1;
        fmt->halign = 0.0;
        fmt->halign_auto = EINA_TRUE;
        fmt->valign = -1.0;
        fmt->style = EVAS_TEXT_STYLE_PLAIN;
        fmt->tabstops = 32;
        fmt->linesize = 0;
        fmt->linerelsize = 0.0;
        fmt->linegap = 0;
        fmt->linerelgap = 0.0;
        fmt->password = 1;
     }
   return fmt;
}

/**
 * @internal
 * Pop fmt to the format stack, if there's something in the stack free fmt
 * and set it to point to the next item instead, else return fmt.
 *
 * @param c the context to work on - Not NULL.
 * @param format - the text of the format to free (assured to start with '-').
 * @return the next format in the stack, or format if there's none.
 * @see _layout_format_push()
 */
static Evas_Object_Textblock_Format *
_layout_format_pop(Ctxt *c, const char *format)
{
   Evas_Object_Textblock_Format *fmt = eina_list_data_get(c->format_stack);

   if ((c->format_stack) && (c->format_stack->next))
     {
        Eina_List *redo_nodes = NULL;
        format++; /* Skip the '-' */

        /* Generic pop, should just pop. */
        if (((format[0] == ' ') && !format[1]) ||
              !format[0])
          {
             _format_unref_free(c->obj, fmt);
             c->format_stack =
                eina_list_remove_list(c->format_stack, c->format_stack);
          }
        else
          {
             size_t len = strlen(format);
             Eina_List *i, *i_next;
             /* Remove only the matching format. */
             EINA_LIST_FOREACH_SAFE(c->format_stack, i, i_next, fmt)
               {
                  /* Stop when we reach the base item */
                  if (!i_next)
                     break;

                  c->format_stack =
                     eina_list_remove_list(c->format_stack, c->format_stack);

                  /* Make sure the ending tag matches the starting tag.
                   * I.e whole of the ending tag matches the start of the
                   * starting tag, and the starting tag's next char is either
                   * NULL or white. Skip the starting '+'. */
                  if (_FORMAT_IS_CLOSER_OF(
                           fmt->fnode->orig_format, format, len))
                    {
                       _format_unref_free(c->obj, fmt);
                       break;
                    }
                  else
                    {
                       redo_nodes = eina_list_prepend(redo_nodes, fmt->fnode);
                       _format_unref_free(c->obj, fmt);
                    }
               }
          }

        /* Redo all the nodes needed to be redone */
          {
             Evas_Object_Textblock_Node_Format *fnode;
             Eina_List *i, *i_next;

             EINA_LIST_FOREACH_SAFE(redo_nodes, i, i_next, fnode)
               {
                  /* FIXME: Actually do something with the new acquired padding,
                   * the can be different and affect our padding! */
                  Evas_Coord style_pad_l, style_pad_r, style_pad_t, style_pad_b;
                  redo_nodes = eina_list_remove_list(redo_nodes, i);
                  fmt = eina_list_data_get(c->format_stack);
                  _layout_do_format(c->obj, c, &fmt, fnode,
                        &style_pad_l, &style_pad_r,
                        &style_pad_t, &style_pad_b, EINA_FALSE);
               }
          }

        fmt = eina_list_data_get(c->format_stack);
     }
   return fmt;
}

/**
 * @internal
 * Parse item and fill fmt with the item.
 *
 * @param c the context to work on - Not NULL.
 * @param fmt the format to fill - not null.
 */
static void
_layout_format_value_handle(Ctxt *c, Evas_Object_Textblock_Format *fmt, const char *item)
{
   const char *key = NULL, *val = NULL;
   char *tmp;
   tmp = alloca(strlen(item) + 1);
   strcpy(tmp, item);

   _format_param_parse(tmp, &key, &val);
   if ((key) && (val)) _format_command(c->obj, fmt, key, val);
   if (key) eina_stringshare_del(key);
   if (val) eina_stringshare_del(val);
   c->align = fmt->halign;
   c->align_auto = fmt->halign_auto;
   c->marginl = fmt->margin.l;
   c->marginr = fmt->margin.r;
}

#define VSIZE_FULL 0
#define VSIZE_ASCENT 1

#define SIZE 0
#define SIZE_ABS 1
#define SIZE_REL 2

/**
 * @internal
 * Get the current line's alignment from the context.
 *
 * @param c the context to work on - Not NULL.
 */
static inline double
_layout_line_align_get(Ctxt *c)
{
#ifdef BIDI_SUPPORT
   if (c->align_auto && c->ln)
     {
        if (c->ln->items && c->ln->items->text_node &&
              (c->ln->par->direction == EVAS_BIDI_DIRECTION_RTL))
          {
             /* Align right*/
             return 1.0;
          }
        else
          {
             /* Align left */
             return 0.0;
          }
     }
#endif
   return c->align;
}

#ifdef BIDI_SUPPORT
/**
 * @internal
 * Reorder the items in visual order
 *
 * @param line the line to reorder
 */
static void
_layout_line_reorder(Evas_Object_Textblock_Line *line)
{
   /*FIXME: do it a bit more efficient - not very efficient ATM. */
   Evas_Object_Textblock_Item *it;
   EvasBiDiStrIndex *v_to_l = NULL;
   Evas_Coord x;
   size_t start, end;
   size_t len;

   if (line->items && line->items->text_node &&
         line->par->bidi_props)
     {
        Evas_BiDi_Paragraph_Props *props;
        props = line->par->bidi_props;
        start = end = line->items->text_pos;

        /* Find the first and last positions in the line */

        EINA_INLIST_FOREACH(line->items, it)
          {
             if (it->text_pos < start)
               {
                  start = it->text_pos;
               }
             else
               {
                  int tlen;
                  tlen = (it->type == EVAS_TEXTBLOCK_ITEM_TEXT) ?
                     _ITEM_TEXT(it)->text_props.text_len : 1;
                  if (it->text_pos + tlen > end)
                    {
                       end = it->text_pos + tlen;
                    }
               }
          }

        len = end - start;
        evas_bidi_props_reorder_line(NULL, start, len, props, &v_to_l);

        /* Update visual pos */
          {
             Evas_Object_Textblock_Item *i;
             i = line->items;
             while (i)
               {
                  i->visual_pos = evas_bidi_position_logical_to_visual(
                        v_to_l, len, i->text_pos - start);
                  i = (Evas_Object_Textblock_Item *) EINA_INLIST_GET(i)->next;
               }
          }

        /*FIXME: not very efficient, sort the items arrays. Anyhow, should only
         * reorder if it's a bidi paragraph */
          {
             Evas_Object_Textblock_Item *i, *j, *min;
             i = line->items;
             while (i)
               {
                  min = i;
                  EINA_INLIST_FOREACH(i, j)
                    {
                       if (j->visual_pos < min->visual_pos)
                         {
                            min = j;
                         }
                    }
                  if (min != i)
                    {
                       line->items = (Evas_Object_Textblock_Item *) eina_inlist_remove(EINA_INLIST_GET(line->items), EINA_INLIST_GET(min));
                       line->items = (Evas_Object_Textblock_Item *) eina_inlist_prepend_relative(EINA_INLIST_GET(line->items), EINA_INLIST_GET(min), EINA_INLIST_GET(i));
                    }

                  i = (Evas_Object_Textblock_Item *) EINA_INLIST_GET(min)->next;
               }
          }
     }

   if (v_to_l) free(v_to_l);
   x = 0;
   EINA_INLIST_FOREACH(line->items, it)
     {
        it->x = x;
        x += it->adv;
     }
}
#endif

/* FIXME: doc */
static void
_layout_calculate_format_item_size(const Evas_Object *obj,
      const Evas_Object_Textblock_Format_Item *fi,
      Evas_Coord *maxascent, Evas_Coord *maxdescent,
      Evas_Coord *_y, Evas_Coord *_w, Evas_Coord *_h)
{
   /* Adjust sizes according to current line height/scale */
   Evas_Coord w, h;
   const char *p, *s;

   s = fi->item;
   w = fi->parent.w;
   h = fi->parent.h;
   switch (fi->size)
     {
      case SIZE:
         p = strstr(s, " size=");
         if (p)
           {
              p += 6;
              if (sscanf(p, "%ix%i", &w, &h) == 2)
                {
                   w = w * obj->cur.scale;
                   h = h * obj->cur.scale;
                }
           }
         break;
      case SIZE_REL:
         p = strstr((char *) s, " relsize=");
         p += 9;
         if (sscanf(p, "%ix%i", &w, &h) == 2)
           {
              int sz = 1;
              if (fi->vsize == VSIZE_FULL)
                {
                   sz = *maxdescent + *maxascent;
                }
              else if (fi->vsize == VSIZE_ASCENT)
                {
                   sz = *maxascent;
                }
              w = (w * sz) / h;
              h = sz;
           }
         break;
      case SIZE_ABS:
         /* Nothing to do */
      default:
         break;
     }

   switch (fi->size)
     {
      case SIZE:
      case SIZE_ABS:
         switch (fi->vsize)
           {
            case VSIZE_FULL:
               if (h > (*maxdescent + *maxascent))
                 {
                    *maxascent += h - (*maxdescent + *maxascent);
                    *_y = -*maxascent;
                 }
               else
                  *_y = -(h - *maxdescent);
               break;
            case VSIZE_ASCENT:
               if (h > *maxascent)
                 {
                    *maxascent = h;
                    *_y = -h;
                 }
               else
                  *_y = -h;
               break;
            default:
               break;
           }
         break;
      case SIZE_REL:
         switch (fi->vsize)
           {
            case VSIZE_FULL:
            case VSIZE_ASCENT:
               *_y = -*maxascent;
               break;
            default:
               break;
           }
         break;
      default:
         break;
     }

   *_w = w;
   *_h = h;
}

/**
 * @internal
 * Order the items in the line, update it's properties and update it's
 * corresponding paragraph.
 *
 * @param c the context to work on - Not NULL.
 * @param fmt the format to use.
 * @param add_line true if we should create a line, false otherwise.
 */
static void
_layout_line_finalize(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   Evas_Object_Textblock_Item *it;
   Evas_Coord x = 0;

   /* If there are no text items yet, calc ascent/descent
    * according to the current format. */
   if (c->maxascent + c->maxdescent == 0)
      _layout_format_ascent_descent_adjust(c->obj, &c->maxascent,
            &c->maxdescent, fmt);

   /* Adjust all the item sizes according to the final line size,
    * and update the x positions of all the items of the line. */
   EINA_INLIST_FOREACH(c->ln->items, it)
     {
        if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
          {
             Evas_Object_Textblock_Format_Item *fi = _ITEM_FORMAT(it);
             if (!fi->formatme) goto loop_advance;
             _layout_calculate_format_item_size(c->obj, fi, &c->maxascent,
                   &c->maxdescent, &fi->y, &fi->parent.w, &fi->parent.h);
             fi->parent.adv = fi->parent.w;
          }

loop_advance:
        it->x = x;
        x += it->adv;

        if ((it->x + it->adv) > c->ln->w) c->ln->w = it->x + it->adv;
     }

   c->ln->y = (c->y - c->par->y) + c->o->style_pad.t;
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
   c->ln->line_no = c->line_no - c->ln->par->line_no;
   c->line_no++;
   c->y += c->maxascent + c->maxdescent;
   if (c->w >= 0)
     {
        c->ln->x = c->marginl + c->o->style_pad.l +
           ((c->w - c->ln->w -
             c->o->style_pad.l - c->o->style_pad.r -
             c->marginl - c->marginr) * _layout_line_align_get(c));
     }
   else
     {
        c->ln->x = c->marginl + c->o->style_pad.l;
     }

   c->par->h = c->ln->y + c->ln->h;
   if (c->ln->w > c->par->w)
     c->par->w = c->ln->w;

     {
        Evas_Coord new_wmax = c->ln->w +
           c->marginl + c->marginr - (c->o->style_pad.l + c->o->style_pad.r);
        if (new_wmax > c->wmax)
           c->wmax = new_wmax;
     }
}

/**
 * @internal
 * Create a new line and append it to the lines in the context.
 *
 * @param c the context to work on - Not NULL.
 * @param fmt the format to use.
 * @param add_line true if we should create a line, false otherwise.
 */
static void
_layout_line_advance(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   _layout_line_finalize(c, fmt);
   _layout_line_new(c, fmt);
}

/**
 * @internal
 * Create a new text layout item from the string and the format.
 *
 * @param c the context to work on - Not NULL.
 * @param fmt the format to use.
 * @param str the string to use.
 * @param len the length of the string.
 */
static Evas_Object_Textblock_Text_Item *
_layout_text_item_new(Ctxt *c __UNUSED__, Evas_Object_Textblock_Format *fmt)
{
   Evas_Object_Textblock_Text_Item *ti;

   ti = calloc(1, sizeof(Evas_Object_Textblock_Text_Item));
   ti->parent.format = fmt;
   ti->parent.format->ref++;
   ti->parent.type = EVAS_TEXTBLOCK_ITEM_TEXT;
   return ti;
}

/**
 * @internal
 * Return the cutoff of the text in the text item.
 *
 * @param c the context to work on - Not NULL.
 * @param fmt the format to use. - Not NULL.
 * @param it the item to check - Not null.
 * @return -1 if there is no cutoff (either because there is really none,
 * or because of an error), cutoff index on success.
 */
static int
_layout_text_cutoff_get(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Text_Item *ti)
{
   if (fmt->font.font)
     {
        Evas_Coord x;
        x = c->w - c->o->style_pad.l - c->o->style_pad.r - c->marginl -
           c->marginr - c->x - ti->x_adjustment;
        if (x < 0)
          x = 0;
        return c->ENFN->font_last_up_to_pos(c->ENDT, fmt->font.font,
              &ti->text_props, x, 0);
     }
   return -1;
}

/**
 * @internal
 * Split before cut, and strip if str[cut - 1] is a whitespace.
 *
 * @param c the context to work on - Not NULL.
 * @param ti the item to cut - not null.
 * @param lti the logical list item of the item.
 * @param cut the cut index.
 * @return the second (newly created) item.
 */
static Evas_Object_Textblock_Text_Item *
_layout_item_text_split_strip_white(Ctxt *c,
      Evas_Object_Textblock_Text_Item *ti, Eina_List *lti, size_t cut)
{
   const Eina_Unicode *ts;
   Evas_Object_Textblock_Text_Item *new_ti = NULL, *white_ti = NULL;

   ts = GET_ITEM_TEXT(ti);

   if (!IS_AT_END(ti, cut) && (ti->text_props.text_len > 0))
     {
        new_ti = _layout_text_item_new(c, ti->parent.format);
        new_ti->parent.text_node = ti->parent.text_node;
        new_ti->parent.text_pos = ti->parent.text_pos + cut;
        new_ti->parent.merge = EINA_TRUE;

        evas_common_text_props_split(&ti->text_props,
                                     &new_ti->text_props, cut);
        _layout_text_add_logical_item(c, new_ti, lti);
     }

   /* Strip the previous white if needed */
   if ((cut >= 1) && _is_white(ts[cut - 1]) && (ti->text_props.text_len > 0))
     {
        if (cut - 1 > 0)
          {
             size_t white_cut = cut - 1;
             white_ti = _layout_text_item_new(c, ti->parent.format);
             white_ti->parent.text_node = ti->parent.text_node;
             white_ti->parent.text_pos = ti->parent.text_pos + white_cut;
             white_ti->parent.merge = EINA_TRUE;
             white_ti->parent.visually_deleted = EINA_TRUE;

             evas_common_text_props_split(&ti->text_props,
                   &white_ti->text_props, white_cut);
             _layout_text_add_logical_item(c, white_ti, lti);
          }
        else
          {
             /* Mark this one as the visually deleted. */
             ti->parent.visually_deleted = EINA_TRUE;
          }
     }

   if (new_ti || white_ti)
     {
        _text_item_update_sizes(c, ti);
     }
   return new_ti;
}

/**
 * @internal
 * Merge item2 into item1 and free item2.
 *
 * @param c the context to work on - Not NULL.
 * @param item1 the item to copy to
 * @param item2 the item to copy from
 */
static void
_layout_item_merge_and_free(Ctxt *c,
      Evas_Object_Textblock_Text_Item *item1,
      Evas_Object_Textblock_Text_Item *item2)
{
   evas_common_text_props_merge(&item1->text_props,
         &item2->text_props);

   _text_item_update_sizes(c, item1);

   item1->parent.merge = EINA_FALSE;
   item1->parent.visually_deleted = EINA_FALSE;

   _item_free(c->obj, NULL, _ITEM(item2));
}

/**
 * @internal
 * Calculates an item's size.
 *
 * @param c the context
 * @param it the item itself.
 */
static void
_text_item_update_sizes(Ctxt *c, Evas_Object_Textblock_Text_Item *ti)
{
   int tw, th, inset, advw;
   const Evas_Object_Textblock_Format *fmt = ti->parent.format;
   int shad_sz = 0, shad_dst = 0, out_sz = 0;
   int dx = 0, minx = 0, maxx = 0, shx1, shx2;

   tw = th = 0;
   if (fmt->font.font)
     c->ENFN->font_string_size_get(c->ENDT, fmt->font.font,
           &ti->text_props, &tw, &th);
   inset = 0;
   if (fmt->font.font)
     inset = c->ENFN->font_inset_get(c->ENDT, fmt->font.font,
           &ti->text_props);
   advw = 0;
   if (fmt->font.font)
      advw = c->ENFN->font_h_advance_get(c->ENDT, fmt->font.font,
           &ti->text_props);


   /* These adjustments are calculated and thus heavily linked to those in
    * textblock_render!!! Don't change one without the other. */

   switch (ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC)
     {
      case EVAS_TEXT_STYLE_SHADOW:
        shad_dst = 1;
        break;
      case EVAS_TEXT_STYLE_OUTLINE_SHADOW:
      case EVAS_TEXT_STYLE_FAR_SHADOW:
        shad_dst = 2;
        out_sz = 1;
        break;
      case EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW:
        shad_dst = 1;
        shad_sz = 2;
        out_sz = 1;
        break;
      case EVAS_TEXT_STYLE_FAR_SOFT_SHADOW:
        shad_dst = 2;
        shad_sz = 2;
        break;
      case EVAS_TEXT_STYLE_SOFT_SHADOW:
        shad_dst = 1;
        shad_sz = 2;
        break;
      case EVAS_TEXT_STYLE_GLOW:
      case EVAS_TEXT_STYLE_SOFT_OUTLINE:
        out_sz = 2;
        break;
      case EVAS_TEXT_STYLE_OUTLINE:
        out_sz = 1;
        break;
      default:
        break;
     }
   switch (ti->parent.format->style & EVAS_TEXT_STYLE_MASK_SHADOW_DIRECTION)
     {
      case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT:
      case EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT:
      case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT:
        dx = -1;
        break;
      case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT:
      case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT:
      case EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT:
        dx = 1;
      case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP:
      case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM:
      default:
        dx = 0;
        break;
     }
   minx = -out_sz;
   maxx = out_sz;
   shx1 = dx * shad_dst;
   shx1 -= shad_sz;
   shx2 = dx * shad_dst;
   shx2 += shad_sz;
   if (shx1 < minx) minx = shx1;
   if (shx2 > maxx) maxx = shx2;
   inset += -minx;
   ti->x_adjustment = maxx - minx;
   
   ti->inset = inset;
   ti->parent.w = tw + ti->x_adjustment;
   ti->parent.h = th;
   ti->parent.adv = advw;
   ti->parent.x = 0;
}

/**
 * @internal
 * Adds the item to the list, updates the item's properties (e.g, x,w,h)
 *
 * @param c the context
 * @param it the item itself.
 * @param rel item ti will be appened after, NULL = last.
 */
static void
_layout_text_add_logical_item(Ctxt *c, Evas_Object_Textblock_Text_Item *ti,
      Eina_List *rel)
{
   _text_item_update_sizes(c, ti);

   c->par->logical_items = eina_list_append_relative_list(
         c->par->logical_items, ti, rel);
}

/**
 * @internal
 * Appends the text from node n starting at start ending at off to the layout.
 * It uses the fmt for the formatting.
 *
 * @param c the current context- NOT NULL.
 * @param fmt the format to use.
 * @param n the text node. - Not null.
 * @param start the start position. - in range.
 * @param off the offset - start + offset in range. if offset is -1, it'll add everything to the end of the string if offset = 0 it'll return with doing nothing.
 * @param repch a replacement char to print instead of the original string, for example, * when working with passwords.
 */
static void
_layout_text_append(Ctxt *c, Evas_Object_Textblock_Format *fmt, Evas_Object_Textblock_Node_Text *n, int start, int off, const char *repch)
{
   int new_line, empty_item;
   const Eina_Unicode *str = EINA_UNICODE_EMPTY_STRING;
   const Eina_Unicode *tbase;
   Evas_Object_Textblock_Text_Item *ti;
   size_t cur_len = 0;
   Eina_Unicode urepch = 0;

   /* prepare a working copy of the string, either filled by the repch or
    * filled with the true values */
   if (n)
     {
        int len;
        int orig_off = off;

        /* Figure out if we want to bail, work with an empty string,
         * or continue with a slice of the passed string */
        len = eina_ustrbuf_length_get(n->unicode);
        if (off == 0) return;
        else if (off < 0) off = len - start;

        if (start < 0)
          {
             start = 0;
          }
        else if ((start == 0) && (off == 0) && (orig_off == -1))
          {
             /* Special case that means that we need to add an empty
              * item */
             str = EINA_UNICODE_EMPTY_STRING;
             goto skip;
          }
        else if ((start >= len) || (start + off > len))
          {
             return;
          }

        /* If we work with a replacement char, create a string which is the same
         * but with replacement chars instead of regular chars. */
        if ((fmt->password) && (repch) && (eina_ustrbuf_length_get(n->unicode)))
          {
             int i, ind;
             Eina_Unicode *ptr;

             tbase = str = ptr = alloca((off + 1) * sizeof(Eina_Unicode));
             ind = 0;
             urepch = eina_unicode_utf8_get_next(repch, &ind);
             for (i = 0 ; i < off; ptr++, i++)
               *ptr = urepch;
             *ptr = 0;
          }
        /* Use the string, just cut the relevant parts */
        else
          {
             str = eina_ustrbuf_string_get(n->unicode) + start;
          }

        cur_len = off;
     }

skip:
   tbase = str;
   new_line = 0;
   empty_item = 0;


   /* If there's no parent text node, only create an empty item */
   if (!n)
     {
        ti = _layout_text_item_new(c, fmt);
        ti->parent.text_node = NULL;
        ti->parent.text_pos = 0;
        _layout_text_add_logical_item(c, ti, NULL);

        return;
     }

   while (cur_len > 0)
     {
        Evas_Font_Instance *script_fi = NULL;
        int script_len, tmp_cut;
        Evas_Script_Type script;

        script_len = cur_len;

        tmp_cut = evas_common_language_script_end_of_run_get(str,
              c->par->bidi_props, start + str - tbase, script_len);
        if (tmp_cut > 0)
          {
             script_len = tmp_cut;
          }
        cur_len -= script_len;

        script = evas_common_language_script_type_get(str, script_len);


        while (script_len > 0)
          {
             Evas_Font_Instance *cur_fi = NULL;
             int run_len = script_len;
             ti = _layout_text_item_new(c, fmt);
             ti->parent.text_node = n;
             ti->parent.text_pos = start + str - tbase;

             if (ti->parent.format->font.font)
               {
                  run_len = c->ENFN->font_run_end_get(c->ENDT,
                        ti->parent.format->font.font, &script_fi, &cur_fi,
                        script, str, script_len);
               }

             evas_common_text_props_bidi_set(&ti->text_props,
                   c->par->bidi_props, ti->parent.text_pos);
             evas_common_text_props_script_set(&ti->text_props, script);

             if (cur_fi)
               {
                  c->ENFN->font_text_props_info_create(c->ENDT,
                        cur_fi, str, &ti->text_props, c->par->bidi_props,
                        ti->parent.text_pos, run_len);
               }
             str += run_len;
             script_len -= run_len;

             _layout_text_add_logical_item(c, ti, NULL);
          }
     }
}

/**
 * @internal
 * Add a format item from the format node n and the item item.
 *
 * @param c the current context- NOT NULL.
 * @param n the source format node - not null.
 * @param item the format text.
 *
 * @return the new format item.
 */
static Evas_Object_Textblock_Format_Item *
_layout_format_item_add(Ctxt *c, Evas_Object_Textblock_Node_Format *n, const char *item, Evas_Object_Textblock_Format *fmt)
{
   Evas_Object_Textblock_Format_Item *fi;

   fi = calloc(1, sizeof(Evas_Object_Textblock_Format_Item));
   fi->item = eina_stringshare_add(item);
   fi->parent.type = EVAS_TEXTBLOCK_ITEM_FORMAT;
   fi->parent.format = fmt;
   fi->parent.format->ref++;
   c->par->logical_items = eina_list_append(c->par->logical_items, fi);
   if (n)
     {
        fi->parent.text_node = n->text_node;
        /* FIXME: make it more efficient */
        fi->parent.text_pos = _evas_textblock_node_format_pos_get(n);
#ifdef BIDI_SUPPORT
        fi->bidi_dir = (evas_bidi_is_rtl_char(
              c->par->bidi_props,
              0,
              fi->parent.text_pos)) ?
           EVAS_BIDI_DIRECTION_RTL : EVAS_BIDI_DIRECTION_LTR;
#else
        fi->bidi_dir = EVAS_BIDI_DIRECTION_LTR;
#endif
     }
   return fi;
}

/**
 * @internal
 * Returns true if the item is a tab
 * @def _IS_TAB(item)
 */
#define _IS_TAB(item)                                             \
   (!strcmp(item, "\t") || !strcmp(item, "\\t"))
/**
 * @internal
 * Returns true if the item is a line spearator, false otherwise
 * @def _IS_LINE_SEPARATOR(item)
 */
#define _IS_LINE_SEPARATOR(item)                                             \
   (!strcmp(item, "\n") || !strcmp(item, "\\n"))
/**
 * @internal
 * Returns true if the item is a paragraph separator, false otherwise
 * @def _IS_PARAGRAPH_SEPARATOR(item)
 */
#define _IS_PARAGRAPH_SEPARATOR(o, item)                                     \
   (!strcmp(item, "ps") ||                                                   \
    (o->newline_is_ps && _IS_LINE_SEPARATOR(item))) /* Paragraph separator */

/**
 * @internal
 * Handles a format by processing a format node. It returns the relevant format
 * through _fmt and updates the padding through style_pad_*. If needed,
 * it creates a format item.
 *
 * @param obj the evas object - NOT NULL.
 * @param c the current context- NOT NULL.
 * @param _fmt the format that holds the result.
 * @param n the source format node - not null.
 * @param style_pad_l the pad to update.
 * @param style_pad_r the pad to update.
 * @param style_pad_t the pad to update.
 * @param style_pad_b the pad to update.
 * @param create_item Create a new format item if true, only process otherwise.
 */
static void
_layout_do_format(const Evas_Object *obj __UNUSED__, Ctxt *c,
      Evas_Object_Textblock_Format **_fmt, Evas_Object_Textblock_Node_Format *n,
      int *style_pad_l, int *style_pad_r, int *style_pad_t, int *style_pad_b,
      Eina_Bool create_item)
{
   Evas_Object_Textblock_Format *fmt = *_fmt;
   /* FIXME: comment the algo */

   const char *s;
   const char *item;
   int handled = 0;

   s = n->format;
   if (!strncmp(s, "+ item ", 7))
     {
        // one of:
        //   item size=20x10 href=name
        //   item relsize=20x10 href=name
        //   item abssize=20x10 href=name
        // 
        // optional arguments:
        //   vsize=full
        //   vsize=ascent
        // 
        // size == item size (modifies line size) - can be multiplied by
        //   scale factor
        // relsize == relative size (height is current font height, width
        //   modified accordingly keeping aspect)
        // abssize == absolute size (modifies line size) - never mulitplied by
        //   scale factor
        // href == name of item - to be found and matched later and used for
        //   positioning
        Evas_Object_Textblock_Format_Item *fi;
        int w = 1, h = 1;
        int vsize = 0, size = 0;
        char *p;

        // don't care
        //href = strstr(s, " href=");
        p = strstr(s, " vsize=");
        if (p)
          {
             p += 7;
             if (!strncmp(p, "full", 4)) vsize = VSIZE_FULL;
             else if (!strncmp(p, "ascent", 6)) vsize = VSIZE_ASCENT;
          }
        p = strstr(s, " size=");
        if (p)
          {
             p += 6;
             if (sscanf(p, "%ix%i", &w, &h) == 2)
               {
                  /* this is handled somewhere else because it depends
                   * on the current scaling factor of the object which
                   * may change and break because the results of this
                   * function are cached */
                  size = SIZE;
               }
          }
        else
          {
             p = strstr(s, " absize=");
             if (p)
               {
                  p += 8;
                  if (sscanf(p, "%ix%i", &w, &h) == 2)
                    {
                       size = SIZE_ABS;
                    }
               }
             else
               {
                  p = strstr(s, " relsize=");
                  if (p)
                    {
                       /* this is handled somewhere else because it depends
                        * on the line it resides in, which is not defined
                        * at this point and will change anyway, which will
                        * break because the results of this function are
                        * cached */
                       size = SIZE_REL;
                    }
               }
          }

        if (create_item)
          {
             fi = _layout_format_item_add(c, n, s, fmt);
             fi->vsize = vsize;
             fi->size = size;
             fi->formatme = 1;
             /* For formats items it's usually
                the same, we don't handle the
                special cases yet. */
             fi->parent.w = fi->parent.adv = w;
             fi->parent.h = h;
          }
        /* Not sure if it's the best handling, but will do it for now. */
        fmt = _layout_format_push(c, fmt, n);
        handled = 1;
     }

   if (!handled)
     {
        Eina_Bool push_fmt = EINA_FALSE;
        if (s[0] == '+')
          {
             fmt = _layout_format_push(c, fmt, n);
             s++;
             push_fmt = EINA_TRUE;
          }
        else if (s[0] == '-')
          {
             fmt = _layout_format_pop(c, n->orig_format);
             s++;
          }
        while ((item = _format_parse(&s)))
          {
             if (_format_is_param(item))
               {
                  /* Only handle it if it's a push format, otherwise,
                   * don't let overwrite the format stack.. */
                  if (push_fmt)
                    {
                       _layout_format_value_handle(c, fmt, item);
                    }
               }
             else if (create_item)
               {
                  if ((_IS_PARAGRAPH_SEPARATOR(c->o, item)) ||
                        (_IS_LINE_SEPARATOR(item)))
                    {
                       Evas_Object_Textblock_Format_Item *fi;

                       fi = _layout_format_item_add(c, n, item, fmt);

                       fi->parent.w = fi->parent.adv = 0;
                    }
                  else if ((!strcmp(item, "\t")) || (!strcmp(item, "\\t")))
                    {
                       Evas_Object_Textblock_Format_Item *fi;

                       fi = _layout_format_item_add(c, n, item, fmt);
                       fi->parent.w = fi->parent.adv = fmt->tabstops;
                       fi->formatme = 1;
                    }
               }
          }
     }

     {
        Evas_Coord pad_l, pad_r, pad_t, pad_b;
        pad_l = pad_r = pad_t = pad_b = 0;
        evas_text_style_pad_get(fmt->style, &pad_l, &pad_r, &pad_t, &pad_b);
        if (pad_l > *style_pad_l) *style_pad_l = pad_l;
        if (pad_r > *style_pad_r) *style_pad_r = pad_r;
        if (pad_t > *style_pad_t) *style_pad_t = pad_t;
        if (pad_b > *style_pad_b) *style_pad_b = pad_b;
     }

   if (fmt->underline2)
     c->have_underline2 = 1;
   else if (fmt->underline)
     c->have_underline = 1;
   *_fmt = fmt;
}

static void
_layout_update_par(Ctxt *c)
{
   Evas_Object_Textblock_Paragraph *last_par;
   last_par = (Evas_Object_Textblock_Paragraph *)
      EINA_INLIST_GET(c->par)->prev;
   if (last_par)
     {
        c->par->y = last_par->y + last_par->h;
     }
   else
     {
        c->par->y = 0;
     }

   /* Insert it to the index now that we calculated it's y
    * We don't need to reinsert even if y (they key) changed, because the
    * order remains the same. */
   if (!c->par->indexed)
     {
        c->o->par_index = eina_rbtree_inline_insert(c->o->par_index,
              EINA_RBTREE_GET(c->par), _par_index_node_cmp, NULL);
        c->par->indexed = EINA_TRUE;
     }
}

/* -1 means no wrap */
static int
_layout_get_charwrap(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Text_Item *ti, size_t line_start,
      const char *breaks)
{
   int wrap;
   size_t uwrap;
   size_t len = eina_ustrbuf_length_get(ti->parent.text_node->unicode);
   /* Currently not being used, because it doesn't contain relevant
    * information */
   (void) breaks;

     {
        wrap = _layout_text_cutoff_get(c, fmt, ti);
        if (wrap < 0)
           return -1;
        uwrap = (size_t) wrap + ti->parent.text_pos;
     }


   if (uwrap == line_start)
     {
        uwrap = ti->parent.text_pos +
           (size_t) evas_common_text_props_cluster_next(&ti->text_props, wrap);
     }
   if ((uwrap <= line_start) || (uwrap > len))
      return -1;

   return uwrap;
}

/* -1 means no wrap */
#ifdef HAVE_LINEBREAK

/* Allow break means: if we can break after the current char */
#define ALLOW_BREAK(i) \
   (breaks[i] <= LINEBREAK_ALLOWBREAK)

#else

#define ALLOW_BREAK(i) \
   (_is_white(str[i]))

#endif
static int
_layout_get_word_mixwrap_common(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Text_Item *ti, Eina_Bool mixed_wrap,
      size_t line_start, const char *breaks)
{
   Eina_Bool wrap_after = EINA_FALSE;
   size_t wrap;
   size_t orig_wrap;
   const Eina_Unicode *str = eina_ustrbuf_string_get(
         ti->parent.text_node->unicode);
   int item_start = ti->parent.text_pos;
   size_t len = eina_ustrbuf_length_get(ti->parent.text_node->unicode);
#ifndef HAVE_LINEBREAK
   /* Not used without liblinebreak ATM. */
   (void) breaks;
#endif

     {
        int swrap = -1;
        swrap = _layout_text_cutoff_get(c, fmt, ti);
        /* Avoiding too small textblocks to even contain one char.
         * FIXME: This can cause breaking inside ligatures. */

        if (swrap < 0)
           return -1;

        orig_wrap = wrap = swrap + item_start;
     }

   if (wrap > line_start)
     {
        /* The wrapping point found is the first char of the next string
           the rest works on the last char of the previous string.
           If it's a whitespace, then it's ok, and no need to go back
           because we'll remove it anyway. */
        if (!_is_white(str[wrap]))
           MOVE_PREV_UNTIL(line_start, wrap);
        /* If there's a breakable point inside the text, scan backwards until
         * we find it */
        while (wrap > line_start)
          {
             if (ALLOW_BREAK(wrap))
                break;
             wrap--;
          }

        if ((wrap > line_start) ||
              ((wrap == line_start) && (ALLOW_BREAK(wrap)) && (wrap < len)))
          {
             /* We found a suitable wrapping point, break here. */
             MOVE_NEXT_UNTIL(len, wrap);
             return wrap;
          }
        else
          {
             if (mixed_wrap)
               {
                  return ((orig_wrap >= line_start) && (orig_wrap < len)) ?
                     ((int) orig_wrap) : -1;
               }
             else
               {
                  /* Scan forward to find the next wrapping point */
                  wrap = orig_wrap;
                  wrap_after = EINA_TRUE;
               }
          }
     }

   /* If we need to find the position after the cutting point */
   if ((wrap == line_start) || (wrap_after))
     {
        if (mixed_wrap)
          {
             return _layout_get_charwrap(c, fmt, ti,
                   line_start, breaks);
          }
        else
          {
             while (wrap < len)
               {
                  if (ALLOW_BREAK(wrap))
                     break;
                  wrap++;
               }


             if ((wrap < len) && (wrap > line_start))
               {
                  MOVE_NEXT_UNTIL(len, wrap);
                  return wrap;
               }
             else
               {
                  return -1;
               }
          }
     }

   return -1;
}

/* -1 means no wrap */
static int
_layout_get_wordwrap(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Text_Item *ti, size_t line_start,
      const char *breaks)
{
   return _layout_get_word_mixwrap_common(c, fmt, ti, EINA_FALSE, line_start,
         breaks);
}

/* -1 means no wrap */
static int
_layout_get_mixedwrap(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Text_Item *ti, size_t line_start,
      const char *breaks)
{
   return _layout_get_word_mixwrap_common(c, fmt, ti, EINA_TRUE, line_start,
         breaks);
}

/* Should be moved inside _layout_ellipsis_item_new once we fix the hack in
 * textblock render */
static const Eina_Unicode _ellip_str[2] = { 0x2026, '\0' };

static Evas_Object_Textblock_Text_Item *
_layout_ellipsis_item_new(Ctxt *c, const Evas_Object_Textblock_Item *cur_it)
{
   Evas_Object_Textblock_Text_Item *ellip_ti;
   Evas_Script_Type script;
   Evas_Font_Instance *script_fi = NULL, *cur_fi;
   size_t len = 1; /* The length of _ellip_str */

   /* We can free it here, cause there's only one ellipsis item per tb. */
   if (c->o->ellip_ti) _item_free(c->obj, NULL, _ITEM(c->o->ellip_ti));
   c->o->ellip_ti = ellip_ti = _layout_text_item_new(c,
         eina_list_data_get(eina_list_last(c->format_stack)));
   ellip_ti->parent.text_node = cur_it->text_node;
   ellip_ti->parent.text_pos = cur_it->text_pos;
   script = evas_common_language_script_type_get(_ellip_str, len);

   evas_common_text_props_bidi_set(&ellip_ti->text_props,
         c->par->bidi_props, ellip_ti->parent.text_pos);
   evas_common_text_props_script_set (&ellip_ti->text_props, script);

   if (ellip_ti->parent.format->font.font)
     {
        /* It's only 1 char anyway, we don't need the run end. */
        (void) c->ENFN->font_run_end_get(c->ENDT,
              ellip_ti->parent.format->font.font, &script_fi, &cur_fi,
              script, _ellip_str, len);

        c->ENFN->font_text_props_info_create(c->ENDT,
              cur_fi, _ellip_str, &ellip_ti->text_props,
              c->par->bidi_props, ellip_ti->parent.text_pos, len);
     }

   _text_item_update_sizes(c, ellip_ti);

   if (cur_it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
     {
        ellip_ti->parent.text_pos += _ITEM_TEXT(cur_it)->text_props.text_len
           - 1;
     }
   else
     {
        ellip_ti->parent.text_pos++;
     }

   return ellip_ti;
}

/**
 * @internel
 * Handle ellipsis
 */
static inline void
_layout_handle_ellipsis(Ctxt *c, Evas_Object_Textblock_Item *it, Eina_List *i)
{
   Evas_Object_Textblock_Text_Item *ellip_ti, *last_ti;
   Evas_Object_Textblock_Item *last_it;
   Evas_Coord save_cx;
   int wrap;
   ellip_ti = _layout_ellipsis_item_new(c, it);
   last_it = it;
   last_ti = _ITEM_TEXT(it);

   save_cx = c->x;
   c->w -= ellip_ti->parent.w;
   do
     {
        wrap = _layout_text_cutoff_get(c, last_it->format,
              last_ti);
        if ((wrap > 0) && !IS_AT_END(last_ti, (size_t) wrap))
          {
             _layout_item_text_split_strip_white(c, last_ti, i, wrap);
          }
        else if (wrap == 0)
          {
             if (!c->ln->items)
                break;
             /* We haven't added it yet at this point */
             if (_ITEM(last_ti) != it)
               {
                  last_it =
                     _ITEM(EINA_INLIST_GET(last_it)->prev);
                  c->ln->items = _ITEM(eina_inlist_remove(
                           EINA_INLIST_GET(c->ln->items),
                           EINA_INLIST_GET(_ITEM(last_ti))));
               }
             else
               {
                  last_it =
                     _ITEM(EINA_INLIST_GET(c->ln->items)->last);
               }
             last_ti = _ITEM_TEXT(last_it);
             if (last_it)
               {
                  c->x -= last_it->adv;
               }
          }
     }
   while (last_it && (wrap == 0));
   c->x = save_cx;
   c->w += ellip_ti->parent.w;
   /* If we should add this item, do it */
   if (last_it == it)
     {
        c->ln->items = (Evas_Object_Textblock_Item *)
           eina_inlist_append(EINA_INLIST_GET(c->ln->items),
                 EINA_INLIST_GET(it));
        if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
          {
             Evas_Object_Textblock_Format_Item *fi;
             fi = _ITEM_FORMAT(it);
             fi->y = c->y;
          }
     }
   c->ln->items = (Evas_Object_Textblock_Item *)
      eina_inlist_append(EINA_INLIST_GET(c->ln->items),
            EINA_INLIST_GET(_ITEM(ellip_ti)));
   _layout_line_finalize(c, ellip_ti->parent.format);
}

#ifdef BIDI_SUPPORT
static void
_layout_paragraph_reorder_lines(Evas_Object_Textblock_Paragraph *par)
{
   Evas_Object_Textblock_Line *ln;

   EINA_INLIST_FOREACH(EINA_INLIST_GET(par->lines), ln)
     {
        _layout_line_reorder(ln);
     }
}
#endif

static void
_layout_paragraph_render(Evas_Object_Textblock *o,
      Evas_Object_Textblock_Paragraph *par)
{
   if (par->rendered)
      return;
   par->rendered = EINA_TRUE;

#ifdef BIDI_SUPPORT
   if (par->is_bidi)
     {
        _layout_update_bidi_props(o, par);
        _layout_paragraph_reorder_lines(par);
        /* Clear the bidi props because we don't need them anymore. */
        if (par->bidi_props)
          {
             evas_bidi_paragraph_props_unref(par->bidi_props);
             par->bidi_props = NULL;
          }
     }
#endif
}

/* 0 means go ahead, 1 means break without an error, 2 means
 * break with an error, should probably clean this a bit (enum/macro)
 * FIXME ^ */
static int
_layout_par(Ctxt *c)
{
   Evas_Object_Textblock_Item *it;
   Eina_List *i;
   int ret = 0;
   int wrap = -1;
   char *line_breaks = NULL;

   if (!c->par->logical_items)
     return 2;

   /* We want to show it. */
   c->par->visible = 1;

   /* Check if we need to skip this paragraph because it's already layouted
    * correctly, and mark handled nodes as dirty. */
   c->par->line_no = c->line_no;
   if (c->par->text_node)
     {
        /* Skip this paragraph if width is the same, there is no ellipsis
         * and we aren't just calculating. */
        if (!c->par->text_node->new && !c->par->text_node->dirty &&
              !c->width_changed && c->par->lines &&
              !c->o->have_ellipsis)
          {
             Evas_Object_Textblock_Line *ln;
             /* Update c->line_no */
             ln = (Evas_Object_Textblock_Line *)
                EINA_INLIST_GET(c->par->lines)->last;
             if (ln)
                c->line_no = c->par->line_no + ln->line_no + 1;
             return 0;
          }
        c->par->text_node->dirty = EINA_FALSE;
        c->par->text_node->new = EINA_FALSE;
        c->par->rendered = EINA_FALSE;

        /* Merge back and clear the paragraph */
          {
             Eina_List *itr, *itr_next;
             Evas_Object_Textblock_Item *ititr, *prev_it = NULL;
             _paragraph_clear(c->obj, c->par);
             EINA_LIST_FOREACH_SAFE(c->par->logical_items, itr, itr_next, ititr)
               {
                  if (ititr->merge && prev_it &&
                        (prev_it->type == EVAS_TEXTBLOCK_ITEM_TEXT) &&
                        (ititr->type == EVAS_TEXTBLOCK_ITEM_TEXT))
                    {
                       _layout_item_merge_and_free(c, _ITEM_TEXT(prev_it),
                             _ITEM_TEXT(ititr));
                       c->par->logical_items =
                          eina_list_remove_list(c->par->logical_items, itr);
                    }
                  else
                    {
                       prev_it = ititr;
                    }
               }
          }
     }

   c->y = c->par->y;

   it = _ITEM(eina_list_data_get(c->par->logical_items));
   _layout_line_new(c, it->format);
   /* We walk on our own because we want to be able to add items from
    * inside the list and then walk them on the next iteration. */
   for (i = c->par->logical_items ; i ; )
     {
        int adv_line = 0;
        int redo_item = 0;
        it = _ITEM(eina_list_data_get(i));
        /* Skip visually deleted items */
        if (it->visually_deleted)
          {
             i = eina_list_next(i);
             continue;
          }

        if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
          {
             Evas_Object_Textblock_Text_Item *ti = _ITEM_TEXT(it);
             _layout_format_ascent_descent_adjust(c->obj, &c->maxascent,
                   &c->maxdescent, ti->parent.format);
          }
        else
          {
             Evas_Object_Textblock_Format_Item *fi = _ITEM_FORMAT(it);
             if (fi->formatme)
               {
                  /* If there are no text items yet, calc ascent/descent
                   * according to the current format. */
                  if (c->maxascent + c->maxdescent == 0)
                     _layout_format_ascent_descent_adjust(c->obj, &c->maxascent,
                           &c->maxdescent, it->format);

                  _layout_calculate_format_item_size(c->obj, fi, &c->maxascent,
                        &c->maxdescent, &fi->y, &fi->parent.w, &fi->parent.h);
                  fi->parent.adv = fi->parent.w;
               }
          }


        /* Check if we need to wrap, i.e the text is bigger than the width,
           or we already found a wrap point. */
        if ((c->w >= 0) &&
              (((c->x + it->adv) >
                (c->w - c->o->style_pad.l - c->o->style_pad.r -
                 c->marginl - c->marginr)) || (wrap > 0)))
          {
             /* Handle ellipsis here. If we don't have more width left
              * and no height left, or no more width left and no wrapping. */
             if ((it->format->ellipsis == 1.0) && (c->h >= 0) &&
                   ((2 * it->h + c->y >
                     c->h - c->o->style_pad.t - c->o->style_pad.b) ||
                    (!it->format->wrap_word && !it->format->wrap_char &&
                     !it->format->wrap_mixed)))
               {
                  _layout_handle_ellipsis(c, it, i);
                  ret = 1;
                  goto end;
               }
             /* If we want to wrap and it's worth checking for wrapping
              * (i.e there's actually text). */
             else if ((it->format->wrap_word || it->format->wrap_char ||
                it->format->wrap_mixed) && it->text_node)
               {
                  if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
                    {
                       /* Don't wrap if it's the only item */
                       if (c->ln->items)
                         {
                            /*FIXME: I should handle format correctly,
                              i.e verify we are allowed to break here */
                            _layout_line_advance(c, it->format);
                            wrap = -1;
                         }
                    }
                  else
                    {
                       Evas_Object_Textblock_Text_Item *ti = _ITEM_TEXT(it);
                       size_t line_start;

#ifdef HAVE_LINEBREAK
                       /* If we haven't calculated the linebreaks yet,
                        * do */
                       if (!line_breaks)
                         {
                            /* Only relevant in those cases */
                            if (it->format->wrap_word || it->format->wrap_mixed)
                              {
                                 size_t len =
                                    eina_ustrbuf_length_get(
                                          it->text_node->unicode);
                                 line_breaks = malloc(len);
                                 set_linebreaks_utf32((const utf32_t *)
                                    eina_ustrbuf_string_get(
                                       it->text_node->unicode),
                                    len, "", line_breaks);
                                 /* FIXME: "" should be text_props language */
                              }
                         }
#endif
                       if (c->ln->items)
                          line_start = c->ln->items->text_pos;
                       else
                          line_start = ti->parent.text_pos;

                       adv_line = 1;
                       /* If we don't already have a wrap point from before */
                       if (wrap < 0)
                         {
                            if (it->format->wrap_word)
                               wrap = _layout_get_wordwrap(c, it->format, ti,
                                     line_start, line_breaks);
                            else if (it->format->wrap_char)
                               wrap = _layout_get_charwrap(c, it->format, ti,
                                     line_start, line_breaks);
                            else if (it->format->wrap_mixed)
                               wrap = _layout_get_mixedwrap(c, it->format, ti,
                                     line_start, line_breaks);
                            else
                               wrap = -1;
                         }

                       /* If it's before the item, rollback and apply.
                          if it's in the item, cut.
                          If it's after the item, delay the cut */
                       if (wrap > 0)
                         {
                            size_t uwrap = (size_t) wrap;
                            if (uwrap < ti->parent.text_pos)
                              {
                                 /* Rollback latest additions, and cut that
                                    item */
                                 i = eina_list_prev(i);
                                 it = eina_list_data_get(i);
                                 while (uwrap < it->text_pos)
                                   {
                                      c->ln->items = _ITEM(
                                            eina_inlist_remove(
                                               EINA_INLIST_GET(c->ln->items),
                                               EINA_INLIST_GET(it)));
                                      i = eina_list_prev(i);
                                      it = eina_list_data_get(i);
                                   }
                                 c->x = it->x;
                                 c->ln->items = _ITEM(
                                       eina_inlist_remove(
                                          EINA_INLIST_GET(c->ln->items),
                                          EINA_INLIST_GET(it)));
                                 continue;
                              }
                            /* If it points to the end, it means the previous
                             * char is a whitespace we should remove, so this
                             * is a wanted cutting point. */
                            else if (uwrap > ti->parent.text_pos +
                                  ti->text_props.text_len)
                               wrap = -1; /* Delay the cut in a smart way
                               i.e use the item_pos as the line_start, because
                               there's already no cut before*/
                            else
                               wrap -= ti->parent.text_pos; /* Cut here */
                         }

                       if (wrap > 0)
                         {
                            _layout_item_text_split_strip_white(c, ti, i, wrap);
                         }
                       else if (wrap == 0)
                         {
                            /* Should wrap before the item */
                            adv_line = 0;
                            redo_item = 1;
                            _layout_line_advance(c, it->format);
                         }
                       /* Reset wrap */
                       wrap = -1;
                    }
               }
          }

        if (!redo_item && !it->visually_deleted)
          {
             c->ln->items = (Evas_Object_Textblock_Item *)
                eina_inlist_append(EINA_INLIST_GET(c->ln->items),
                      EINA_INLIST_GET(it));
             if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
               {
                  Evas_Object_Textblock_Format_Item *fi;
                  fi = _ITEM_FORMAT(it);
                  fi->y = c->y;
                  /* If it's a newline, and we are not in newline compat
                   * mode, or we are in newline compat mode, and this is
                   * not used as a paragraph separator, advance */
                  if (fi->item && _IS_LINE_SEPARATOR(fi->item) &&
                        (!c->o->newline_is_ps ||
                         eina_list_next(i)))
                    {
                       adv_line = 1;
                    }
               }
             c->x += it->adv;
             i = eina_list_next(i);
          }
        if (adv_line)
          {
             /* Each line is according to the first item in it, and here
              * i is already the next item (or the current if we redo it) */
             if (i)
               {
                  it = _ITEM(eina_list_data_get(i));
               }
             _layout_line_advance(c, it->format);
          }
     }
   if (c->ln->items)
     {
        /* Here 'it' is the last format used */
        _layout_line_finalize(c, it->format);
     }

end:
#ifdef HAVE_LINEBREAK
   if (line_breaks)
      free(line_breaks);
#endif

   return ret;
}

/**
 * @internal
 * Invalidate text nodes according to format changes
 * This goes through all the new format changes and marks the text nodes
 * that should be invalidated because of format changes.
 *
 * @param c the working context.
 */
static inline void
_format_changes_invalidate_text_nodes(Ctxt *c)
{
   Evas_Object_Textblock_Node_Format *fnode = c->o->format_nodes;
   Evas_Object_Textblock_Node_Text *start_n = NULL;
   Eina_List *fstack = NULL;
   int balance = 0;
   while (fnode)
     {
        if (fnode->new)
          {
             const char *fstr = fnode->orig_format;
             /* balance < 0 means we gave up and everything should be
              * invalidated */
             if (*fstr == '+')
               {
                  balance++;
                  if (!fstack)
                     start_n = fnode->text_node;
                  fstack = eina_list_prepend(fstack, fnode);
               }
             else if (*fstr == '-')
               {
                  size_t fstr_len;
                  /* Skip the '-' */
                  fstr++;
                  fstr_len = strlen(fstr);
                  /* Generic popper, just pop */
                  if (((fstr[0] == ' ') && !fstr[1]) || !fstr[0])
                    {
                       fstack = eina_list_remove_list(fstack, fstack);
                       balance--;
                    }
                  /* Find the matching format and pop it, if the matching format
                   * is out format, i.e the last one, pop and break. */
                  else
                    {
                       Eina_List *i;
                       Evas_Object_Textblock_Node_Format *fnode2;
                       EINA_LIST_FOREACH(fstack, i, fnode2)
                         {
                            if (_FORMAT_IS_CLOSER_OF(
                                     fnode2->orig_format, fstr, fstr_len))
                              {
                                 fstack = eina_list_remove_list(fstack, i);
                                 balance--;
                                 break;
                              }
                         }
                    }

                  if (!fstack)
                    {
                       Evas_Object_Textblock_Node_Text *f_tnode =
                          fnode->text_node;
                       while (start_n)
                         {
                            start_n->dirty = EINA_TRUE;
                            if (start_n == f_tnode)
                               break;
                            start_n =
                               _NODE_TEXT(EINA_INLIST_GET(start_n)->next);
                         }
                       start_n = NULL;
                    }
               }
             else if (!fnode->visible)
                balance = -1;

             if (balance < 0)
               {
                  /* if we don't already have a starting point, use the
                   * current paragraph. */
                  if (!start_n)
                     start_n = fnode->text_node;
                  break;
               }
          }
        fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
     }

   if (balance != 0)
     {
        while (start_n)
          {
             start_n->dirty = EINA_TRUE;
             start_n = _NODE_TEXT(EINA_INLIST_GET(start_n)->next);
          }
     }
}


/** FIXME: Document */
static void
_layout_pre(Ctxt *c, int *style_pad_l, int *style_pad_r, int *style_pad_t,
      int *style_pad_b)
{
   Evas_Object *obj = c->obj;
   Evas_Object_Textblock *o = c->o;
   /* Mark text nodes as dirty if format have changed. */
   if (c->o->format_changed)
     {
        _format_changes_invalidate_text_nodes(c);
     }

   if (o->content_changed)
     {
        Evas_Object_Textblock_Node_Text *n;
        c->o->have_ellipsis = 0;
        c->par = c->paragraphs = o->paragraphs;
        /* Go through all the text nodes to create the logical layout */
        EINA_INLIST_FOREACH(c->o->text_nodes, n)
          {
             Evas_Object_Textblock_Node_Format *fnode;
             size_t start;
             int off;

             /* If it's not a new paragraph, either update it or skip it.
              * Remove all the paragraphs that were deleted */
             if (!n->new)
               {
                  /* Remove all the deleted paragraphs at this point */
                  while (c->par->text_node != n)
                    {
                       Evas_Object_Textblock_Paragraph *tmp_par =
                          (Evas_Object_Textblock_Paragraph *)
                          EINA_INLIST_GET(c->par)->next;

                       c->paragraphs = (Evas_Object_Textblock_Paragraph *)
                          eina_inlist_remove(EINA_INLIST_GET(c->paragraphs),
                                EINA_INLIST_GET(c->par));
                       _paragraph_free(obj, c->par);

                       c->par = tmp_par;
                    }

                  /* If it's dirty, remove and recreate, if it's clean,
                   * skip to the next. */
                  if (n->dirty)
                    {
                       Evas_Object_Textblock_Paragraph *prev_par = c->par;

                       _layout_paragraph_new(c, n, EINA_TRUE);

                       c->paragraphs = (Evas_Object_Textblock_Paragraph *)
                          eina_inlist_remove(EINA_INLIST_GET(c->paragraphs),
                                EINA_INLIST_GET(prev_par));
                       _paragraph_free(obj, prev_par);
                    }
                  else
                    {
                       c->par = (Evas_Object_Textblock_Paragraph *)
                          EINA_INLIST_GET(c->par)->next;

                       /* Update the format stack according to the node's
                        * formats */
                       fnode = n->format_node;
                       while (fnode && (fnode->text_node == n))
                         {
                            /* Only do this if this actually changes format */
                            if (fnode->format_change)
                               _layout_do_format(obj, c, &c->fmt, fnode,
                                     style_pad_l, style_pad_r,
                                     style_pad_t, style_pad_b, EINA_FALSE);
                            fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
                         }
                       continue;
                    }
               }
             else
               {
                  /* If it's a new paragraph, just add it. */
                  _layout_paragraph_new(c, n, EINA_FALSE);
               }

#ifdef BIDI_SUPPORT
             _layout_update_bidi_props(c->o, c->par);
#endif

             /* For each text node to thorugh all of it's format nodes
              * append text from the start to the offset of the next format
              * using the last format got. if needed it also creates format
              * items this is the core algorithm of the layout mechanism.
              * Skip the unicode replacement chars when there are because
              * we don't want to print them. */
             fnode = n->format_node;
             start = off = 0;
             while (fnode && (fnode->text_node == n))
               {
                  off += fnode->offset;
                  /* No need to skip on the first run, or a non-visible one */
                  _layout_text_append(c, c->fmt, n, start, off, o->repch);
                  _layout_do_format(obj, c, &c->fmt, fnode, style_pad_l,
                        style_pad_r, style_pad_t, style_pad_b, EINA_TRUE);
                  if ((c->have_underline2) || (c->have_underline))
                    {
                       if (*style_pad_b < c->underline_extend)
                         *style_pad_b = c->underline_extend;
                       c->have_underline = 0;
                       c->have_underline2 = 0;
                       c->underline_extend = 0;
                    }
                  start += off;
                  if (fnode->visible)
                    {
                       off = -1;
                       start++;
                    }
                  else
                    {
                       off = 0;
                    }
                  fnode->new = EINA_FALSE;
                  fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
               }
             _layout_text_append(c, c->fmt, n, start, -1, o->repch);
#ifdef BIDI_SUPPORT
             /* Clear the bidi props because we don't need them anymore. */
             if (c->par->bidi_props)
               {
                  evas_bidi_paragraph_props_unref(c->par->bidi_props);
                  c->par->bidi_props = NULL;
               }
#endif
             c->par = (Evas_Object_Textblock_Paragraph *)
                EINA_INLIST_GET(c->par)->next;
          }

        /* Delete the rest of the layout paragraphs */
        while (c->par)
          {
             Evas_Object_Textblock_Paragraph *tmp_par =
                (Evas_Object_Textblock_Paragraph *)
                EINA_INLIST_GET(c->par)->next;

             c->paragraphs = (Evas_Object_Textblock_Paragraph *)
                eina_inlist_remove(EINA_INLIST_GET(c->paragraphs),
                      EINA_INLIST_GET(c->par));
             _paragraph_free(obj, c->par);

             c->par = tmp_par;
          }
        o->paragraphs = c->paragraphs;
        c->par = NULL;
     }

}

/**
 * @internal
 * Create the layout from the nodes.
 *
 * @param obj the evas object - NOT NULL.
 * @param calc_only true if should only calc sizes false if should also create the layout.. It assumes native size is being calculated, doesn't support formatted size atm.
 * @param w the object's w, -1 means no wrapping (i.e infinite size)
 * @param h the object's h, -1 means inifinte size.
 * @param w_ret the object's calculated w.
 * @param h_ret the object's calculated h.
 */
static void
_layout(const Evas_Object *obj, int w, int h, int *w_ret, int *h_ret)
{
   Evas_Object_Textblock *o;
   Ctxt ctxt, *c;
   int style_pad_l = 0, style_pad_r = 0, style_pad_t = 0, style_pad_b = 0;

   /* setup context */
   o = (Evas_Object_Textblock *)(obj->object_data);
   c = &ctxt;
   c->obj = (Evas_Object *)obj;
   c->o = o;
   c->paragraphs = c->par = NULL;
   c->format_stack = NULL;
   c->fmt = NULL;
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
   c->align_auto = EINA_TRUE;
   c->ln = NULL;
   c->width_changed = (obj->cur.geometry.w != o->last_w);

   /* Start of logical layout creation */
   /* setup default base style */
   if ((c->o->style) && (c->o->style->default_tag))
     {
        c->fmt = _layout_format_push(c, NULL, NULL);
        _format_fill(c->obj, c->fmt, c->o->style->default_tag);
     }
   if (!c->fmt)
     {
        if (w_ret) *w_ret = 0;
        if (h_ret) *h_ret = 0;
        return;
     }

   _layout_pre(c, &style_pad_l, &style_pad_r, &style_pad_t, &style_pad_b);
   c->paragraphs = o->paragraphs;

   /* If there are no paragraphs, create the minimum needed,
    * if the last paragraph has no lines/text, create that as well */
   if (!c->paragraphs)
     {
        _layout_paragraph_new(c, NULL, EINA_TRUE);
        o->paragraphs = c->paragraphs;
     }
   c->par = (Evas_Object_Textblock_Paragraph *)
      EINA_INLIST_GET(c->paragraphs)->last;
   if (!c->par->logical_items)
     {
        Evas_Object_Textblock_Text_Item *ti;
        ti = _layout_text_item_new(c, c->fmt);
        ti->parent.text_node = c->par->text_node;
        ti->parent.text_pos = 0;
        _layout_text_add_logical_item(c, ti, NULL);
     }

   /* End of logical layout creation */

   /* Start of visual layout creation */
   {
      Evas_Object_Textblock_Paragraph *last_vis_par = NULL;

      EINA_INLIST_FOREACH(c->paragraphs, c->par)
        {
           _layout_update_par(c);

           /* Break if we should stop here. */
           if (_layout_par(c))
             {
                last_vis_par = c->par;
                break;
             }
        }

      /* Mark all the rest of the paragraphs as invisible */
      if (c->par)
        {
           c->par = (Evas_Object_Textblock_Paragraph *)
              EINA_INLIST_GET(c->par)->next;
           while (c->par)
             {
                c->par->visible = 0;
                c->par = (Evas_Object_Textblock_Paragraph *)
                   EINA_INLIST_GET(c->par)->next;
             }
        }

      /* Get the last visible paragraph in the layout */
      if (!last_vis_par && c->paragraphs)
         last_vis_par = (Evas_Object_Textblock_Paragraph *)
            EINA_INLIST_GET(c->paragraphs)->last;

      if (last_vis_par)
         c->hmax = last_vis_par->y + last_vis_par->h;
   }

   /* Clean the rest of the format stack */
   while (c->format_stack)
     {
        c->fmt = c->format_stack->data;
        c->format_stack = eina_list_remove_list(c->format_stack, c->format_stack);
        _format_unref_free(c->obj, c->fmt);
     }

   if (w_ret) *w_ret = c->wmax;
   if (h_ret) *h_ret = c->hmax;

   /* Vertically align the textblock */
   if ((o->valign > 0.0) && (c->h > c->hmax))
     {
        Evas_Coord adjustment = (c->h - c->hmax) * o->valign;
        Evas_Object_Textblock_Paragraph *par;
        EINA_INLIST_FOREACH(c->paragraphs, par)
          {
             par->y += adjustment;
          }
     }

   if ((o->style_pad.l != style_pad_l) || (o->style_pad.r != style_pad_r) ||
       (o->style_pad.t != style_pad_t) || (o->style_pad.b != style_pad_b))
     {
        o->style_pad.l = style_pad_l;
        o->style_pad.r = style_pad_r;
        o->style_pad.t = style_pad_t;
        o->style_pad.b = style_pad_b;
        _paragraphs_clear(obj, c->paragraphs);
        _layout(obj, w, h, w_ret, h_ret);
     }
}

/*
 * @internal
 * Relayout the object according to current object size.
 *
 * @param obj the evas object - NOT NULL.
 */
static void
_relayout(const Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   _layout(obj, obj->cur.geometry.w, obj->cur.geometry.h,
         &o->formatted.w, &o->formatted.h);
   o->formatted.valid = 1;
   o->last_w = obj->cur.geometry.w;
   o->last_h = obj->cur.geometry.h;
   o->changed = 0;
   o->content_changed = 0;
   o->format_changed = EINA_FALSE;
   o->redraw = 1;
}

/**
 * @internal
 * Find the layout item and line that match the text node and position passed.
 *
 * @param obj the evas object - NOT NULL.
 * @param n the text node - Not null.
 * @param pos the position to look for - valid.
 * @param[out] lnr the line found - not null.
 * @param[out] tir the item found - not null.
 * @see _find_layout_format_item_line_match()
 */
static void
_find_layout_item_line_match(Evas_Object *obj, Evas_Object_Textblock_Node_Text *n, int pos, Evas_Object_Textblock_Line **lnr, Evas_Object_Textblock_Item **itr)
{
   Evas_Object_Textblock_Paragraph *found_par;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   if (!o->formatted.valid) _relayout(obj);

   found_par = n->par;
   if (found_par)
     {
        _layout_paragraph_render(o, found_par);
        EINA_INLIST_FOREACH(found_par->lines, ln)
          {
             Evas_Object_Textblock_Item *it;
             Evas_Object_Textblock_Line *lnn;

             lnn = (Evas_Object_Textblock_Line *)(((Eina_Inlist *)ln)->next);
             EINA_INLIST_FOREACH(ln->items, it)
               {
                  /* FIXME: p should be size_t, same goes for pos */
                  int p = (int) it->text_pos;

                  if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
                    {
                       Evas_Object_Textblock_Text_Item *ti =
                          _ITEM_TEXT(it);

                       p += (int) ti->text_props.text_len;
                    }
                  else
                    {
                       p++;
                    }

                  if (((pos >= (int) it->text_pos) && (pos < p)))
                    {
                       *lnr = ln;
                       *itr = it;
                       return;
                    }
                  else if (p == pos)
                    {
                       *lnr = ln;
                       *itr = it;
                    }
               }
          }
     }
}

/**
 * @internal
 * Return the line number 'line'.
 *
 * @param obj the evas object - NOT NULL.
 * @param line the line to find
 * @return the line of line number or NULL if no line found.
 */
static Evas_Object_Textblock_Line *
_find_layout_line_num(const Evas_Object *obj, int line)
{
   Evas_Object_Textblock_Paragraph *par;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);

   par = _layout_find_paragraph_by_line_no(o, line);
   if (par)
     {
        _layout_paragraph_render(o, par);
        EINA_INLIST_FOREACH(par->lines, ln)
          {
             if (par->line_no + ln->line_no == line) return ln;
          }
     }
   return NULL;
}

EAPI Evas_Object *
evas_object_textblock_add(Evas *e)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   obj = evas_object_new(e);
   evas_object_textblock_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

EAPI Evas_Textblock_Style *
evas_textblock_style_new(void)
{
   Evas_Textblock_Style *ts;

   ts = calloc(1, sizeof(Evas_Textblock_Style));
   return ts;
}

EAPI void
evas_textblock_style_free(Evas_Textblock_Style *ts)
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

EAPI void
evas_textblock_style_set(Evas_Textblock_Style *ts, const char *text)
{
   Eina_List *l;
   Evas_Object *obj;

   if (!ts) return;
   /* If the style wasn't really changed, abort. */
   if ((!ts->style_text && !text) ||
       (ts->style_text && text && !strcmp(text, ts->style_text)))
      return;

   EINA_LIST_FOREACH(ts->objects, l, obj)
     {
        Evas_Object_Textblock *o;

        o = (Evas_Object_Textblock *)(obj->object_data);
        _evas_textblock_invalidate_all(o);
        _evas_textblock_changed(o, obj);
     }

   _style_replace(ts, text);

   if (ts->style_text)
     {
        // format MUST be KEY='VALUE'[KEY='VALUE']...
        const char *p;
        const char *key_start, *key_stop, *val_start, *val_stop;

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
                  size_t tag_len = key_stop - key_start;
                  size_t replace_len = val_stop - val_start;

                  tags = malloc(tag_len + 1);
                  if (tags)
                    {
                       memcpy(tags, key_start, tag_len);
                       tags[tag_len] = 0;
                    }

                  replaces = malloc(replace_len + 1);
                  if (replaces)
                    {
                       memcpy(replaces, val_start, replace_len);
                       replaces[replace_len] = 0;
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
                                 tag->tag_len = tag_len;
                                 tag->replace_len = replace_len;
                                 ts->tags = (Evas_Object_Style_Tag *)eina_inlist_append(EINA_INLIST_GET(ts->tags), EINA_INLIST_GET(tag));
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
}

EAPI const char *
evas_textblock_style_get(const Evas_Textblock_Style *ts)
{
   if (!ts) return NULL;
   return ts->style_text;
}

/* textblock styles */
EAPI void
evas_object_textblock_style_set(Evas_Object *obj, Evas_Textblock_Style *ts)
{
   TB_HEAD();
   if (ts == o->style) return;
   if ((ts) && (ts->delete_me)) return;
   if (o->style)
     {
        Evas_Textblock_Style *old_ts;
        if (o->markup_text)
          {
             free(o->markup_text);
             o->markup_text = NULL;
          }

        old_ts = o->style;
        old_ts->objects = eina_list_remove(old_ts->objects, obj);
        if ((old_ts->delete_me) && (!old_ts->objects))
          evas_textblock_style_free(old_ts);
     }
   if (ts)
     {
        ts->objects = eina_list_append(ts->objects, obj);
     }
   o->style = ts;

   _evas_textblock_invalidate_all(o);
   _evas_textblock_changed(o, obj);
}

EAPI const Evas_Textblock_Style *
evas_object_textblock_style_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   return o->style;
}

EAPI void
evas_object_textblock_replace_char_set(Evas_Object *obj, const char *ch)
{
   TB_HEAD();
   if (o->repch) eina_stringshare_del(o->repch);
   if (ch) o->repch = eina_stringshare_add(ch);
   else o->repch = NULL;
   _evas_textblock_invalidate_all(o);
   _evas_textblock_changed(o, obj);
}

EAPI void
evas_object_textblock_newline_mode_set(Evas_Object *obj, Eina_Bool mode)
{
   TB_HEAD();
   if (o->newline_is_ps == mode)
      return;

   o->newline_is_ps = mode;
   /* FIXME: Should recreate all the textnodes... For now, it's just
    * for new text inserted. */
}

EAPI Eina_Bool
evas_object_textblock_newline_mode_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(EINA_FALSE);
   return o->newline_is_ps;
}

EAPI void
evas_object_textblock_valign_set(Evas_Object *obj, double align)
{
   TB_HEAD();
   if (align < 0.0) align = 0.0;
   else if (align > 1.0) align = 1.0;
   if (o->valign == align) return;
   o->valign = align;
   _evas_textblock_changed(o, obj);
}

EAPI double
evas_object_textblock_valign_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(0.0);
   return o->valign;
}

EAPI void
evas_object_textblock_bidi_delimiters_set(Evas_Object *obj, const char *delim)
{
   TB_HEAD();
   eina_stringshare_replace(&o->bidi_delimiters, delim);
}

EAPI const char *
evas_object_textblock_bidi_delimiters_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   return o->bidi_delimiters;
}

EAPI const char *
evas_object_textblock_replace_char_get(Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   return o->repch;
}

/**
 * @internal
 * Advance p_buff to point after the end of the string. It's used with the
 * @ref escaped_strings[] variable.
 *
 * @param p_buff the pointer to the current string.
 */
static inline void
_escaped_advance_after_end_of_string(const char **p_buf)
{
   while (**p_buf != 0) (*p_buf)++;
   (*p_buf)++;
}

/**
 * @internal
 * Advance p_buff to point after the end of the string. It's used with the
 * @ref escaped_strings[] variable. Also chec if matches.
 * FIXME: doc.
 *
 * @param p_buff the pointer to the current string.
 */
static inline int
_escaped_is_eq_and_advance(const char *s, const char *s_end,
      const char **p_m, const char *m_end)
{
   Eina_Bool reached_end;
   for (;((s < s_end) && (*p_m < m_end)); s++, (*p_m)++)
     {
        if (*s != **p_m)
          {
             _escaped_advance_after_end_of_string(p_m);
             return 0;
          }
     }

   reached_end = !**p_m;
   if (*p_m < m_end)
     _escaped_advance_after_end_of_string(p_m);

   return ((s == s_end) && reached_end);
}

/**
 * @internal
 *
 * @param s the string to match
 */
static inline const char *
_escaped_char_match(const char *s, int *adv)
{
   const char *map_itr, *map_end, *mc, *sc;

   map_itr = escape_strings;
   map_end = map_itr + sizeof(escape_strings);

   while (map_itr < map_end)
     {
        const char *escape;
        int match;

        escape = map_itr;
        _escaped_advance_after_end_of_string(&map_itr);
        if (map_itr >= map_end) break;

        mc = map_itr;
        sc = s;
        match = 1;
        while ((*mc) && (*sc))
          {
             if ((unsigned char)*sc < (unsigned char)*mc) return NULL;
             if (*sc != *mc) match = 0;
             mc++;
             sc++;
          }
        if (match)
          {
             *adv = mc - map_itr;
             return escape;
          }
        _escaped_advance_after_end_of_string(&map_itr);
     }
   return NULL;
}

/**
 * @internal
 * FIXME: TBD.
 *
 * @param s the string to match
 */
static inline const char *
_escaped_char_get(const char *s, const char *s_end)
{
   /* Handle numeric escape codes. */
   if (s[1] == '#')
     {
        static char utf8_escape[7]; /* Support up to 6 bytes utf8 */
        char ustr[10];
        Eina_Unicode uchar[2] = { 0, 0 };
        char *utf8_char;
        size_t len = 0;
        int base = 10;
        s += 2; /* Skip "&#" */

        if (tolower(*s) == 'x')
          {
             s++;
             base = 16;
          }

        len = s_end - s;
        if (len >= sizeof(ustr) + 1)
           len = sizeof(ustr);

        memcpy(ustr, s, len);
        ustr[len] = '\0';
        uchar[0] = strtol(ustr, NULL, base);

        if (uchar[0] == 0)
          return NULL;

        utf8_char = eina_unicode_unicode_to_utf8(uchar, NULL);
        strcpy(utf8_escape, utf8_char);
        free(utf8_char);

        return utf8_escape;
     }
   else
     {
        const char *map_itr, *map_end;

        map_itr = escape_strings;
        map_end = map_itr + sizeof(escape_strings);

        while (map_itr < map_end)
          {
             if (_escaped_is_eq_and_advance(s, s_end, &map_itr, map_end))
                return map_itr;
             if (map_itr < map_end)
                _escaped_advance_after_end_of_string(&map_itr);
          }
     }

   return NULL;
}

EAPI const char *
evas_textblock_escape_string_get(const char *escape)
{
   /* &amp; -> & */
   return _escaped_char_get(escape, escape + strlen(escape));
}

EAPI const char *
evas_textblock_escape_string_range_get(const char *escape_start, const char *escape_end)
{
   return _escaped_char_get(escape_start, escape_end);
}

EAPI const char *
evas_textblock_string_escape_get(const char *string, int *len_ret)
{
   /* & -> &amp; */
   return _escaped_char_match(string, len_ret);
}

/**
 * @internal
 * Appends the escaped char beteewn s and s_end to the curosr
 *
 *
 * @param s the start of the string
 * @param s_end the end of the string.
 */
static inline void
_append_escaped_char(Evas_Textblock_Cursor *cur, const char *s,
      const char *s_end)
{
   const char *escape;

   escape = _escaped_char_get(s, s_end);
   if (escape)
     evas_textblock_cursor_text_append(cur, escape);
}

/**
 * @internal
 * prepends the escaped char beteewn s and s_end to the curosr
 *
 *
 * @param s the start of the string
 * @param s_end the end of the string.
 */
static inline void
_prepend_escaped_char(Evas_Textblock_Cursor *cur, const char *s,
      const char *s_end)
{
   const char *escape;

   escape = _escaped_char_get(s, s_end);
   if (escape)
     evas_textblock_cursor_text_prepend(cur, escape);
}


EAPI void
evas_object_textblock_text_markup_set(Evas_Object *obj, const char *text)
{
   TB_HEAD();
   if ((text != o->markup_text) && (o->markup_text))
     {
        free(o->markup_text);
        o->markup_text = NULL;
     }
   _nodes_clear(obj);
   if (!o->style)
     {
        if (text != o->markup_text)
          {
             if (text) o->markup_text = strdup(text);
          }
        return;
     }
   evas_textblock_cursor_paragraph_first(o->cursor);

   evas_object_textblock_text_markup_prepend(o->cursor, text);
   /* Point all the cursors to the starrt */
     {
        Eina_List *l;
        Evas_Textblock_Cursor *data;

        evas_textblock_cursor_paragraph_first(o->cursor);
        EINA_LIST_FOREACH(o->cursors, l, data)
           evas_textblock_cursor_paragraph_first(data);
     }
}

EAPI void
evas_object_textblock_text_markup_prepend(Evas_Textblock_Cursor *cur, const char *text)
{
   Evas_Object *obj = cur->obj;
   TB_HEAD();
   if (text)
     {
        char *s, *p;
        char *tag_start, *tag_end, *esc_start, *esc_end;

        tag_start = tag_end = esc_start = esc_end = NULL;
        p = (char *)text;
        s = p;
        /* This loop goes through all of the mark up text until it finds format
         * tags, escape sequences or the terminating NULL. When it finds either
         * of those, it appends the text found up until that point to the textblock
         * proccesses whatever found. It repeats itself until the termainating
         * NULL is reached. */
        for (;;)
          {
             /* If we got to the end of string or just finished/started tag
              * or escape sequence handling. */
             if ((*p == 0) ||
                   (tag_end) || (esc_end) ||
                   (tag_start) || (esc_start))
               {
                  if (tag_end)
                    {
                       /* If we reached to a tag ending, analyze the tag */
                       char *ttag;
                       size_t ttag_len = tag_end - tag_start -1;


                       ttag = malloc(ttag_len + 1);
                       if (ttag)
                         {
                            memcpy(ttag, tag_start + 1, ttag_len);
                            ttag[ttag_len] = 0;
                            evas_textblock_cursor_format_prepend(cur, ttag);
                         }
                       tag_start = tag_end = NULL;
                    }
                  else if (esc_end)
                    {
                       _prepend_escaped_char(cur, esc_start, esc_end + 1);
                       esc_start = esc_end = NULL;
                    }
                  else if (*p == 0)
                    {
                       _prepend_text_run(cur, s, p);
                       s = NULL;
                    }
                  if (*p == 0)
                    break;
               }
             if (*p == '<')
               {
                  if (!esc_start)
                    {
                       /* Append the text prior to this to the textblock and mark
                        * the start of the tag */
                       tag_start = p;
                       tag_end = NULL;
                       _prepend_text_run(cur, s, p);
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
                       /* Append the text prior to this to the textblock and mark
                        * the start of the escape sequence */
                       esc_start = p;
                       esc_end = NULL;
                       _prepend_text_run(cur, s, p);
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
             /* Unicode object replcament char */
             else if (!strncmp("\xEF\xBF\xBC", p, 3))
               {
                  /*FIXME: currently just remove them, maybe do something
                   * fancier in the future, atm it breaks if this char
                   * is inside <> */
                  _prepend_text_run(cur, s, p);
                  p += 2; /* it's also advanced later in this loop need +3
                           * in total*/
                  s = p + 1; /* One after the end of the replacement char */
               }
             p++;
          }
     }
   _evas_textblock_changed(o, obj);
}


/**
 * @internal
 * An helper function to markup get. Appends the format from fnode to the strbugf txt.
 *
 * @param o the textblock object.
 * @param txt the strbuf to append to.
 * @param fnode the format node to process.
 */
static void
_markup_get_format_append(Evas_Object_Textblock *o __UNUSED__, Eina_Strbuf *txt, Evas_Object_Textblock_Node_Format *fnode)
{
   eina_strbuf_append_char(txt, '<');
     {
        const char *s;
        int push = 0;
        int pop = 0;

        // FIXME: need to escape
        s = fnode->orig_format;
        if (*s == '+') push = 1;
        if (*s == '-') pop = 1;
        while ((*s == ' ') || (*s == '+') || (*s == '-')) s++;
        if (pop) eina_strbuf_append_char(txt, '/');
        eina_strbuf_append(txt, s);
     }
   eina_strbuf_append_char(txt, '>');
}

/**
 * @internal
 * An helper function to markup get. Appends the text in text.
 *
 * @param txt the strbuf to append to.
 * @param text the text to process.
 */
static void
_markup_get_text_append(Eina_Strbuf *txt, const Eina_Unicode *text)
{
   char *p = eina_unicode_unicode_to_utf8(text, NULL);
   char *base = p;
   while (*p)
     {
        const char *escape;
        int adv;

        escape = _escaped_char_match(p, &adv);
        if (escape)
          {
             p += adv;
             eina_strbuf_append(txt, escape);
          }
        else
          {
             eina_strbuf_append_char(txt, *p);
             p++;
          }
     }
   free(base);
}
EAPI const char *
evas_object_textblock_text_markup_get(const Evas_Object *obj)
{
   Evas_Object_Textblock_Node_Text *n;
   Eina_Strbuf *txt = NULL;

   TB_HEAD_RETURN(NULL);
   if (o->markup_text) return(o->markup_text);
   txt = eina_strbuf_new();
   EINA_INLIST_FOREACH(o->text_nodes, n)
     {
        Evas_Object_Textblock_Node_Format *fnode;
        Eina_Unicode *text_base, *text;
        int off;

        /* For each text node to thorugh all of it's format nodes
         * append text from the start to the offset of the next format
         * using the last format got. if needed it also creates format items
         * this is the core algorithm of the layout mechanism.
         * Skip the unicode replacement chars when there are because
         * we don't want to print them. */
        text_base = text =
           eina_unicode_strndup(eina_ustrbuf_string_get(n->unicode),
                                eina_ustrbuf_length_get(n->unicode));
        fnode = n->format_node;
        off = 0;
        while (fnode && (fnode->text_node == n))
          {
             Eina_Unicode tmp_ch;
             off += fnode->offset;
             /* No need to skip on the first run */
             tmp_ch = text[off];
             text[off] = 0; /* Null terminate the part of the string */
             _markup_get_text_append(txt, text);
             _markup_get_format_append(o, txt, fnode);
             text[off] = tmp_ch; /* Restore the char */
             text += off;
             if (fnode->visible)
               {
                  off = -1;
                  text++;
               }
             else
               {
                  off = 0;
               }
             fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
          }
        /* Add the rest, skip replacement */
        _markup_get_text_append(txt, text);
        free(text_base);
     }


   o->markup_text = eina_strbuf_string_steal(txt);
   eina_strbuf_free(txt);
   return o->markup_text;
}

/* cursors */

/**
 * @internal
 * Merge the current node with the next, no need to remove PS, already
 * not there.
 *
 * @param o the text block object.
 * @param to merge into to.
 */
static void
_evas_textblock_nodes_merge(Evas_Object_Textblock *o, Evas_Object_Textblock_Node_Text *to)
{
   Evas_Object_Textblock_Node_Format *itr;
   Evas_Object_Textblock_Node_Format *pnode;
   Evas_Object_Textblock_Node_Text *from;
   const Eina_Unicode *text;
   int to_len, len;

   if (!to) return;
   from = _NODE_TEXT(EINA_INLIST_GET(to)->next);

   to_len = eina_ustrbuf_length_get(to->unicode);
   text = eina_ustrbuf_string_get(from->unicode);
   len = eina_ustrbuf_length_get(from->unicode);
   eina_ustrbuf_append_length(to->unicode, text, len);

   itr = from->format_node;
   if (itr && (itr->text_node == from))
     {
        pnode = _NODE_FORMAT(EINA_INLIST_GET(itr)->prev);
        if (pnode && (pnode->text_node == to))
          {
             itr->offset += to_len - _evas_textblock_node_format_pos_get(pnode);
          }
        else
          {
             itr->offset += to_len;
          }
     }

   while (itr && (itr->text_node == from))
     {
        itr->text_node = to;
        itr = _NODE_FORMAT(EINA_INLIST_GET(itr)->next);
     }
   if (!to->format_node || (to->format_node->text_node != to))
     {
        to->format_node = from->format_node;
     }

   /* When it comes to how we handle it, merging is like removing both nodes
    * and creating a new one, se we need to do the needed cleanups. */
   if (to->par)
      to->par->text_node = NULL;
   to->par = NULL;

   to->new = EINA_TRUE;

   _evas_textblock_cursors_set_node(o, from, to);
   _evas_textblock_node_text_remove(o, from);
}

/**
 * @internal
 * Merge the current node with the next, no need to remove PS, already
 * not there.
 *
 * @param cur the cursor that points to the current node
 */
static void
_evas_textblock_cursor_nodes_merge(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock_Node_Text *nnode;
   Evas_Object_Textblock *o;
   int len;
   if (!cur) return;

   len = eina_ustrbuf_length_get(cur->node->unicode);

   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   nnode = _NODE_TEXT(EINA_INLIST_GET(cur->node)->next);
   _evas_textblock_nodes_merge(o, cur->node);
   _evas_textblock_cursors_update_offset(cur, nnode, 0, len);
   _evas_textblock_cursors_set_node(o, nnode, cur->node);
   if (nnode == o->cursor->node)
     {
        o->cursor->node = cur->node;
        o->cursor->pos += len;
     }
}

/**
 * @internal
 * Return the format at a specific position.
 *
 * @param cur the cursor to the position.
 * @return the format node at the specific position or NULL if not found.
 */
static Evas_Object_Textblock_Node_Format *
_evas_textblock_cursor_node_format_at_pos_get(const Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock_Node_Format *node;
   Evas_Object_Textblock_Node_Format *itr;
   int position = 0;

   if (!cur->node) return NULL;

   node = cur->node->format_node;
   if (!node) return NULL;
   /* If there is no exclusive format node to this paragraph return the
    * previous's node */
   /* Find the main format node */
   EINA_INLIST_FOREACH(node, itr)
     {
        if (itr->text_node != cur->node)
          {
             return NULL;
          }
        if ((position + itr->offset) == cur->pos)
          {
             return itr;
          }
        position += itr->offset;
     }
   return NULL;
}

/**
 * @internal
 * Return the last format node at the position of the format node n.
 *
 * @param n a format node at the position.
 * @return the last format node at the position of n.
 */
static Evas_Object_Textblock_Node_Format *
_evas_textblock_node_format_last_at_off(const Evas_Object_Textblock_Node_Format *n)
{
   const Evas_Object_Textblock_Node_Format *nnode;
   const Evas_Object_Textblock_Node_Text *tnode;
   if (!n) return NULL;
   nnode = n;
   tnode = n->text_node;
   do
     {
        n = nnode;
        nnode = _NODE_FORMAT(EINA_INLIST_GET(nnode)->next);
     }
   while (nnode && (nnode->text_node == tnode) && (nnode->offset == 0));

   return (Evas_Object_Textblock_Node_Format *) n;
}

/**
 * @internal
 * Returns the visible format at a specific location.
 *
 * @param n a format at the specific position.
 * @return the format node at the specific position or NULL if not found.
 */
static Evas_Object_Textblock_Node_Format *
_evas_textblock_node_visible_at_pos_get(const Evas_Object_Textblock_Node_Format *n)
{
   const Evas_Object_Textblock_Node_Format *nnode;
   if (!n) return NULL;
   /* The visible format is the last one, because it inserts a replacement
    * char that advances the next formats. */

   nnode = n;
   do
     {
        n = nnode;
        if (n->visible) return (Evas_Object_Textblock_Node_Format *) n;
        nnode = _NODE_FORMAT(EINA_INLIST_GET(nnode)->next);
     }
   while (nnode && (nnode->offset == 0));

   return NULL;
}

/**
 * @internal
 * Return the last format that applies to a specific cursor or at the specific
 * position the cursor points to. This means either a cursor at or before the
 * position of the cursor in the text node is returned or the previous's text
 * node's format node.
 *
 * @param cur the position to look at.
 * @return the format node found.
 */
static Evas_Object_Textblock_Node_Format *
_evas_textblock_cursor_node_format_before_or_at_pos_get(const Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock_Node_Format *node, *pitr = NULL;
   Evas_Object_Textblock_Node_Format *itr;
   size_t position = 0;

   if (!cur->node) return NULL;

   node = cur->node->format_node;
   if (!node) return NULL;
   /* If there is no exclusive format node to this paragraph return the
    * previous's node */
   if (node->text_node != cur->node)
     {
        return node;
     }
   else if (node->offset > cur->pos)
     {
        return _NODE_FORMAT(EINA_INLIST_GET(node)->prev);
     }
   /* Find the main format node */
   pitr = _NODE_FORMAT(EINA_INLIST_GET(node)->prev);
   EINA_INLIST_FOREACH(node, itr)
     {
        if ((itr->text_node != cur->node) ||
            ((position + itr->offset) > cur->pos))
          {
             return pitr;
          }
        else if ((position + itr->offset) == cur->pos)
          {
             return itr;
          }
        pitr = itr;
        position += itr->offset;
     }
   return pitr;
}

/**
 * @internal
 * Find the layout item and line that match the cursor.
 *
 * @param cur the cursor we are currently at. - NOT NULL.
 * @param[out] lnr the line found - not null.
 * @param[out] itr the item found - not null.
 * @return EINA_TRUE if we matched the previous format, EINA_FALSE otherwise.
 */
static Eina_Bool
_find_layout_item_match(const Evas_Textblock_Cursor *cur, Evas_Object_Textblock_Line **lnr, Evas_Object_Textblock_Item **itr)
{
   Evas_Textblock_Cursor cur2;
   Eina_Bool previous_format = EINA_FALSE;

   cur2.obj = cur->obj;
   evas_textblock_cursor_copy(cur, &cur2);
   if (cur2.pos > 0)
     {
        cur2.pos--;
     }

   if (_evas_textblock_cursor_is_at_the_end(cur) &&
            evas_textblock_cursor_format_is_visible_get(&cur2))
     {
        _find_layout_item_line_match(cur2.obj, cur2.node, cur2.pos, lnr, itr);
        previous_format = EINA_TRUE;
     }
   else
     {
        _find_layout_item_line_match(cur->obj, cur->node, cur->pos, lnr, itr);
     }
   return previous_format;
}

EAPI Evas_Textblock_Cursor *
evas_object_textblock_cursor_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   return o->cursor;
}

EAPI Evas_Textblock_Cursor *
evas_object_textblock_cursor_new(Evas_Object *obj)
{
   Evas_Textblock_Cursor *cur;

   TB_HEAD_RETURN(NULL);
   cur = calloc(1, sizeof(Evas_Textblock_Cursor));
   cur->obj = obj;
   cur->node = o->text_nodes;
   cur->pos = 0;

   o->cursors = eina_list_append(o->cursors, cur);
   return cur;
}

EAPI void
evas_textblock_cursor_free(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;

   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (cur == o->cursor) return;
   o->cursors = eina_list_remove(o->cursors, cur);
   free(cur);
}

EAPI Eina_Bool
evas_textblock_cursor_is_format(const Evas_Textblock_Cursor *cur)
{
   if (!cur || !cur->node) return EINA_FALSE;
   if (evas_textblock_cursor_format_is_visible_get(cur)) return EINA_TRUE;
   return (_evas_textblock_cursor_node_format_at_pos_get(cur)) ?
      EINA_TRUE : EINA_FALSE;
}

EAPI const Eina_List *
evas_textblock_node_format_list_get(const Evas_Object *obj, const char *anchor)
{
   TB_HEAD_RETURN(NULL);
   if (!strcmp(anchor, "a"))
      return o->anchors_a;
   else if (!strcmp(anchor, "item"))
      return o->anchors_item;

   return NULL;
}

EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_first_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   return o->format_nodes;
}

EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_last_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   if (o->format_nodes)
     {
        return _NODE_FORMAT(EINA_INLIST_GET(o->format_nodes)->last);
     }
   return NULL;
}

EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_next_get(const Evas_Object_Textblock_Node_Format *n)
{
   return _NODE_FORMAT(EINA_INLIST_GET(n)->next);
}

EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_prev_get(const Evas_Object_Textblock_Node_Format *n)
{
   return _NODE_FORMAT(EINA_INLIST_GET(n)->prev);
}

EAPI void
evas_textblock_node_format_remove_pair(Evas_Object *obj,
      Evas_Object_Textblock_Node_Format *n)
{
   Evas_Object_Textblock_Node_Text *tnode1;
   Evas_Object_Textblock_Node_Format *fmt, *found_node = NULL;
   Eina_List *fstack = NULL;
   TB_HEAD();

   if (!n) return;

   fmt = n;

   do
     {
        const char *fstr = fmt->orig_format;

        if (fstr && (*fstr == '+'))
          {
             fstack = eina_list_prepend(fstack, fmt);
          }
        else if (fstr && (*fstr == '-'))
          {
             size_t fstr_len;
             /* Skip the '-' */
             fstr++;
             fstr_len = strlen(fstr);
             /* Generic popper, just pop */
             if (((fstr[0] == ' ') && !fstr[1]) || !fstr[0])
               {
                  fstack = eina_list_remove_list(fstack, fstack);
                  if (!fstack)
                    {
                       found_node = fmt;
                       goto found;
                    }
               }
             /* Find the matching format and pop it, if the matching format
              * is out format, i.e the last one, pop and break. */
             else
               {
                  Eina_List *i;
                  Evas_Object_Textblock_Node_Format *fnode;
                  EINA_LIST_FOREACH(fstack, i, fnode)
                    {
                       if (_FORMAT_IS_CLOSER_OF(
                                fnode->orig_format, fstr, fstr_len))
                         {
                            /* Last one, this is our item! */
                            if (!eina_list_next(i))
                              {
                                 found_node = fmt;
                                 goto found;
                              }
                            fstack = eina_list_remove_list(fstack, i);
                            break;
                         }
                    }
               }
          }

        fmt = _NODE_FORMAT(EINA_INLIST_GET(fmt)->next);
     }
   while (fmt && fstack);

found:

   fstack = eina_list_free(fstack);

   if (n->visible)
     {
        size_t ind = _evas_textblock_node_format_pos_get(n);
        const char *format = n->format;
        Evas_Textblock_Cursor cur;
        cur.obj = obj;

        eina_ustrbuf_remove(n->text_node->unicode, ind, ind + 1);
        if (format && _IS_PARAGRAPH_SEPARATOR(o, format))
          {
             evas_textblock_cursor_at_format_set(&cur, n);
             _evas_textblock_cursor_nodes_merge(&cur);
          }
        _evas_textblock_cursors_update_offset(&cur, n->text_node, ind, -1);
     }
   tnode1 = n->text_node;
   _evas_textblock_node_format_remove(o, n, 0);
   if (found_node && (found_node != n))
     {
        Evas_Object_Textblock_Node_Text *tnode2;
        tnode2 = found_node->text_node;
        /* found_node can never be visible! (it's the closing format) */
        _evas_textblock_node_format_remove(o, found_node, 0);

        /* FIXME: Should be unified in the layout, for example, added to a list
         * that checks this kind of removals. But until then, this is very fast
         * and works. */
        /* Mark all the text nodes in between the removed formats as dirty. */
        while (tnode1)
          {
             tnode1->dirty = EINA_TRUE;
             if (tnode1 == tnode2)
                break;
             tnode1 =
                _NODE_TEXT(EINA_INLIST_GET(tnode1)->next);
          }
     }

   _evas_textblock_changed(o, obj);
}

EAPI void
evas_textblock_cursor_paragraph_first(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   cur->node = o->text_nodes;
   cur->pos = 0;

}

EAPI void
evas_textblock_cursor_paragraph_last(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *node;

   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   node = o->text_nodes;
   if (node)
     {
        node = _NODE_TEXT(EINA_INLIST_GET(node)->last);
        cur->node = node;
	cur->pos = 0;

	evas_textblock_cursor_paragraph_char_last(cur);
     }
   else
     {
	cur->node = NULL;
	cur->pos = 0;

     }
}

EAPI Eina_Bool
evas_textblock_cursor_paragraph_next(Evas_Textblock_Cursor *cur)
{
   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;
   /* If there is a current text node, return the next text node (if exists)
    * otherwise, just return False. */
   if (cur->node)
     {
        Evas_Object_Textblock_Node_Text *nnode;
        nnode = _NODE_TEXT(EINA_INLIST_GET(cur->node)->next);
        if (nnode)
          {
             cur->node = nnode;
             cur->pos = 0;

             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
evas_textblock_cursor_paragraph_prev(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock_Node_Text *node;
   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;
   /* If the current node is a text node, just get the prev if any,
    * if it's a format, get the current text node out of the format and return
    * the prev text node if any. */
   node = cur->node;
   /* If there is a current text node, return the prev text node
    * (if exists) otherwise, just return False. */
   if (node)
     {
        Evas_Object_Textblock_Node_Text *pnode;
        pnode = _NODE_TEXT(EINA_INLIST_GET(cur->node)->prev);
        if (pnode)
          {
             cur->node = pnode;
             evas_textblock_cursor_paragraph_char_last(cur);
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

EAPI void
evas_textblock_cursor_set_at_format(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *n)
{
   evas_textblock_cursor_at_format_set(cur, n);
}

EAPI Eina_Bool
evas_textblock_cursor_format_next(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock_Node_Format *node;

   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;
   /* If the current node is a format node, just get the next if any,
    * if it's a text, get the current format node out of the text and return
    * the next format node if any. */
   node = _evas_textblock_cursor_node_format_before_or_at_pos_get(cur);
   node = _evas_textblock_node_format_last_at_off(node);
   if (!node)
     {
        if (cur->node->format_node)
          {
             cur->pos = _evas_textblock_node_format_pos_get(node);
             return EINA_TRUE;
          }
     }
   /* If there is a current text node, return the next format node (if exists)
    * otherwise, just return False. */
   else
     {
        Evas_Object_Textblock_Node_Format *nnode;
        nnode = _NODE_FORMAT(EINA_INLIST_GET(node)->next);
        if (nnode)
          {
             cur->node = nnode->text_node;
             cur->pos = _evas_textblock_node_format_pos_get(nnode);

             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
evas_textblock_cursor_format_prev(Evas_Textblock_Cursor *cur)
{
   const Evas_Object_Textblock_Node_Format *node;
   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;
   node = evas_textblock_cursor_format_get(cur);
   if (!node)
     {
        node = _evas_textblock_cursor_node_format_before_or_at_pos_get(cur);
        if (node)
          {
             cur->node = node->text_node;
             cur->pos = _evas_textblock_node_format_pos_get(node);

             return EINA_TRUE;
          }
     }
   /* If there is a current text node, return the next text node (if exists)
    * otherwise, just return False. */
   if (node)
     {
        Evas_Object_Textblock_Node_Format *pnode;
        pnode = _NODE_FORMAT(EINA_INLIST_GET(node)->prev);
        if (pnode)
          {
             cur->node = pnode->text_node;
             cur->pos = _evas_textblock_node_format_pos_get(pnode);

             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
evas_textblock_cursor_char_next(Evas_Textblock_Cursor *cur)
{
   int ind;
   const Eina_Unicode *text;

   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;

   ind = cur->pos;
   text = eina_ustrbuf_string_get(cur->node->unicode);
   if (text[ind]) ind++;
   /* Only allow pointing a null if it's the last paragraph.
    * because we don't have a PS there. */
   if (text[ind])
     {
        cur->pos = ind;
        return EINA_TRUE;
     }
   else
     {
        if (!evas_textblock_cursor_paragraph_next(cur))
          {
             /* If we already were at the end, that means we don't have
              * where to go next we should return FALSE */
             if (cur->pos == (size_t) ind)
               return EINA_FALSE;

             cur->pos = ind;
             return EINA_TRUE;
          }
        else
          {
             return EINA_TRUE;
          }
     }
}

EAPI Eina_Bool
evas_textblock_cursor_char_prev(Evas_Textblock_Cursor *cur)
{
   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;

   if (cur->pos != 0)
     {
        cur->pos--;
        return EINA_TRUE;
     }
   return evas_textblock_cursor_paragraph_prev(cur);
}

EAPI void
evas_textblock_cursor_paragraph_char_first(Evas_Textblock_Cursor *cur)
{
   if (!cur) return;
   cur->pos = 0;

}

EAPI void
evas_textblock_cursor_paragraph_char_last(Evas_Textblock_Cursor *cur)
{
   int ind;

   if (!cur) return;
   if (!cur->node) return;
   ind = eina_ustrbuf_length_get(cur->node->unicode);
   /* If it's not the last paragraph, go back one, because we want to point
    * to the PS, not the NULL */
   if (EINA_INLIST_GET(cur->node)->next)
      ind--;

   if (ind >= 0)
      cur->pos = ind;
   else
      cur->pos = 0;

}

EAPI void
evas_textblock_cursor_line_char_first(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;

   if (!cur) return;
   if (!cur->node) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);

   _find_layout_item_match(cur, &ln, &it);

   if (!ln) return;
   if (ln->items)
     {
        Evas_Object_Textblock_Item *i;
        it = ln->items;
        EINA_INLIST_FOREACH(ln->items, i)
          {
             if (it->text_pos > i->text_pos)
               {
                  it = i;
               }
          }
     }
   if (it)
     {
	cur->pos = it->text_pos;
	cur->node = it->text_node;
     }
}

EAPI void
evas_textblock_cursor_line_char_last(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;

   if (!cur) return;
   if (!cur->node) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);

   _find_layout_item_match(cur, &ln, &it);

   if (!ln) return;
   if (ln->items)
     {
        Evas_Object_Textblock_Item *i;
        it = ln->items;
        EINA_INLIST_FOREACH(ln->items, i)
          {
             if (it->text_pos < i->text_pos)
               {
                  it = i;
               }
          }
     }
   if (it)
     {
        size_t ind;

	cur->node = it->text_node;
	cur->pos = it->text_pos;
        if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
          {
             ind = _ITEM_TEXT(it)->text_props.text_len - 1;
             if (!IS_AT_END(_ITEM_TEXT(it), ind)) ind++;
             cur->pos += ind;
          }
        else if (!EINA_INLIST_GET(ln)->next && !EINA_INLIST_GET(ln->par)->next)
          {
             cur->pos++;
          }
     }
}

/**
 * @internal
 * checks if a format (as a string) is visible/changes format and sets the
 * fnode properties accordingly.
 *
 * @param fnode the format node
 * @param s the string.
 */
static void
_evas_textblock_format_is_visible(Evas_Object_Textblock_Node_Format *fnode,
      const char *s)
{
   const char *item;
   Eina_Bool is_opener = EINA_TRUE;

   fnode->visible = fnode->format_change = EINA_FALSE;
   fnode->anchor = ANCHOR_NONE;
   if (!s) return;

   if (s[0] == '+' || s[0] == '-')
     {
        is_opener = (s[0] == '+');
        s++;
        fnode->format_change = EINA_TRUE;
     }

   while ((item = _format_parse(&s)))
     {
        int itlen = s - item;
        /* We care about all of the formats even after a - except for
         * item which we don't care after a - because it's just a standard
         * closing */
        if ((!strncmp(item, "\n", itlen) || !strncmp(item, "\\n", itlen)) ||
              (!strncmp(item, "\t", itlen) || !strncmp(item, "\\t", itlen)) ||
              (!strncmp(item, "ps", itlen) && (itlen >= 2)) ||
              (!strncmp(item, "item", itlen) && (itlen >= 4) && is_opener))
          {
             fnode->visible = EINA_TRUE;
          }

        if (is_opener && !strncmp(item, "a", itlen))
          {
             fnode->anchor = ANCHOR_A;
          }
        else if (is_opener && !strncmp(item, "item", itlen))
          {
             fnode->anchor = ANCHOR_ITEM;
          }
     }
}

/**
 * Sets the cursor to the position of where the fmt points to.
 *
 * @param cur the cursor to update.
 * @param fmt the format to set according to.
 * @return nothing.
 */
static void __UNUSED__
_evas_textblock_cursor_node_text_at_format(Evas_Textblock_Cursor *cur, Evas_Object_Textblock_Node_Format *fmt)
{
   Evas_Object_Textblock_Node_Text *text;
   Evas_Object_Textblock_Node_Format *base_format;
   Evas_Object_Textblock_Node_Format *itr;
   size_t position = 0;

   if (!cur || !fmt) return;
   /* Find the main format node */
   text = fmt->text_node;
   cur->node = text;
   base_format = text->format_node;
   EINA_INLIST_FOREACH(base_format, itr)
     {
        if (itr == fmt)
          {
             break;
          }
        position += itr->offset;
     }
   cur->pos = position;

}


/**
 * @internal
 * Remove pairs of + and - formats and also remove formats without + or -
 * i.e formats that pair to themselves. Only removes invisible formats
 * that pair themselves, if you want to remove invisible formats that pair
 * themselves, please first change fmt->visible to EINA_FALSE.
 *
 * @param o the textblock object.
 * @param fmt the current format.
 */
static void
_evas_textblock_node_format_remove_matching(Evas_Object_Textblock *o,
      Evas_Object_Textblock_Node_Format *fmt)
{
   Evas_Object_Textblock_Node_Text *tnode;
   Eina_List *formats = NULL;
   size_t offset = 0;

   if (!fmt) return;

   tnode = fmt->text_node;

   do
     {
        Evas_Object_Textblock_Node_Format *nnode;
        const char *fstr = fmt->orig_format;

        nnode = _NODE_FORMAT(EINA_INLIST_GET(fmt)->next);
        if (nnode)
          {
             offset = nnode->offset;
          }


        if (fstr && (*fstr == '+'))
          {
             formats = eina_list_prepend(formats, fmt);
          }
        else if (fstr && (*fstr == '-'))
          {
             Evas_Object_Textblock_Node_Format *fnode;
             size_t fstr_len;
             /* Skip the '-' */
             fstr++;
             fstr_len = strlen(fstr);
             /* Generic popper, just pop */
             if (((fstr[0] == ' ') && !fstr[1]) || !fstr[0])
               {
                  fnode = eina_list_data_get(formats);
                  formats = eina_list_remove_list(formats, formats);
                  _evas_textblock_node_format_remove(o, fnode, 0);
                  _evas_textblock_node_format_remove(o, fmt, 0);
               }
             /* Find the matching format and pop it, if the matching format
              * is out format, i.e the last one, pop and break. */
             else
               {
                  Eina_List *i;
                  EINA_LIST_FOREACH(formats, i, fnode)
                    {
                       if (_FORMAT_IS_CLOSER_OF(
                                fnode->orig_format, fstr, fstr_len))
                         {
                            fnode = eina_list_data_get(i);
                            formats = eina_list_remove_list(formats, i);
                            _evas_textblock_node_format_remove(o, fnode, 0);
                            _evas_textblock_node_format_remove(o, fmt, 0);
                         }
                    }
               }
          }
        else if (!fmt->visible)
          {
             _evas_textblock_node_format_remove(o, fmt, 0);
          }
        fmt = nnode;
     }
   while (fmt && (offset == 0) && (fmt->text_node == tnode));
   eina_list_free(formats);
}
/**
 * @internal
 * Add the offset (may be negative) to the first node after fmt which is
 * pointing to the text node tnode or to o->format_nodes if fmt is null
 * and it points to tnode.
 *
 * @param o the textblock object.
 * @param tnode the text node the format should point to.
 * @param fmt the current format.
 * @param offset the offest to add (may be negative).
 */
static void
_evas_textblock_node_format_adjust_offset(Evas_Object_Textblock *o,
      Evas_Object_Textblock_Node_Text *tnode,
      Evas_Object_Textblock_Node_Format *fmt, int offset)
{
   if (fmt)
     {
        fmt = _NODE_FORMAT(EINA_INLIST_GET(fmt)->next);
     }
   else
     {
        fmt = o->format_nodes;
     }
   if (fmt && (tnode == fmt->text_node))
     {
        fmt->offset += offset;
     }
}

/**
 * @internal
 * Removes a format node updating the offset of the next format node and the
 * text nodes pointing to this node.
 *
 * @param o the textblock object.
 * @param n the fromat node to remove
 */
static void
_evas_textblock_node_format_remove(Evas_Object_Textblock *o, Evas_Object_Textblock_Node_Format *n, int visible_adjustment)
{
   /* Update the text nodes about the change */
     {
        Evas_Object_Textblock_Node_Format *nnode;
        nnode = _NODE_FORMAT(EINA_INLIST_GET(n)->next);
        /* If there's a next node that belongs to the same text node
         * and the curret node was the main one, advance the format node */
        if (nnode && (nnode->text_node == n->text_node))
          {
             if (nnode->text_node->format_node == n)
               {
                  nnode->text_node->format_node = nnode;
               }
          }
        else
          {
             Evas_Object_Textblock_Node_Text *tnode;
             /* If there's no next one update the text nodes */
             nnode = _NODE_FORMAT(EINA_INLIST_GET(n)->prev);
             tnode = n->text_node;
             /* Even if it's not the current text_node's main node
              * it can still be the next's. */
             if (tnode && (tnode->format_node != n))
               {
                  tnode = _NODE_TEXT(EINA_INLIST_GET(tnode)->next);
               }
             while (tnode && (tnode->format_node == n))
               {
                  tnode->format_node = nnode;
                  tnode = _NODE_TEXT(EINA_INLIST_GET(tnode)->next);
               }
          }
     }
   _evas_textblock_node_format_adjust_offset(o, n->text_node, n,
         n->offset - visible_adjustment);

   o->format_nodes = _NODE_FORMAT(eina_inlist_remove(
           EINA_INLIST_GET(o->format_nodes), EINA_INLIST_GET(n)));
   _evas_textblock_node_format_free(o, n);
}

/**
 * @internal
 * Sets all the offsets of the format nodes between start and end in the text
 * node n to 0 and sets visibility to EINA_FALSE.
 * If end == -1 end means the end of the string.
 * Assumes there is a prev node or the current node will be preserved.
 *
 * @param n the text node the positinos refer to.
 * @param start the start of where to delete from.
 * @param end the end of the section to delete, if end == -1 it means the end of the string.
 * @returns #EINA_TRUE if removed a PS, false otherwise.
 */
static Eina_Bool
_evas_textblock_node_text_adjust_offsets_to_start(Evas_Object_Textblock *o,
      Evas_Object_Textblock_Node_Text *n, size_t start, int end)
{
   Evas_Object_Textblock_Node_Format *last_node, *itr;
   Evas_Object_Textblock_Node_Text *new_node;
   int use_end = 1;
   int delta = 0;
   int first = 1;
   int update_format_node;
   size_t pos = 0;
   int orig_end;

   itr = n->format_node;
   if (!itr || (itr->text_node != n)) return EINA_FALSE;

   orig_end = end;
   if ((end < 0) || ((size_t) end == eina_ustrbuf_length_get(n->unicode)))
     {
        use_end = 0;
     }
   else if (end > 0)
     {
        /* We don't want the last one */
        end--;
     }

   /* If we are not removing the text node, all should stay in this text
    * node, otherwise, everything should move to the previous node */
   if ((start == 0) && !use_end)
     {
        new_node = _NODE_TEXT(EINA_INLIST_GET(n)->prev);
        if (!new_node)
          {
             new_node = n;
          }
     }
   else
     {
        new_node = n;
     }

   /* Find the first node after start */
   while (itr && (itr->text_node == n))
     {
        pos += itr->offset;
        if (pos >= start)
          {
             break;
          }
        itr = _NODE_FORMAT(EINA_INLIST_GET(itr)->next);
     }

   if (!itr || (itr->text_node != n))
     {
        return EINA_FALSE;
     }

   update_format_node = ((itr == n->format_node) && (new_node != n));
   delta = orig_end - pos;
   itr->offset -= pos - start;

   while (itr && (itr->text_node == n))
     {
        last_node = itr;
        itr = _NODE_FORMAT(EINA_INLIST_GET(itr)->next);

        if (!first)
          {
             pos += last_node->offset;
          }

        /* start is negative when this gets relevant */
        if (use_end && (pos > (size_t) end))
          {
             last_node->offset -= delta;
             break;
          }

        delta = orig_end - pos;
        if (!first)
          {
             last_node->offset = 0;
          }
        else
          {
             first = 0;
          }
        last_node->visible = EINA_FALSE;

        if (!itr || (itr && (itr->text_node != n)))
          {
             /* Remove the PS, and return since it's the end of the node */
             if (_IS_PARAGRAPH_SEPARATOR(o, last_node->format))
               {
                  _evas_textblock_node_format_remove(o, last_node, 0);
                  return EINA_TRUE;
               }

          }
        last_node->text_node = new_node;
        if (update_format_node)
          {
             n->format_node = last_node;
          }
     }

   return EINA_FALSE;
}

/**
 * @internal
 * Removes all the format nodes between start and end in the text node n.
 * This function updates the offset of the next format node and the
 * text nodes pointing to it. if end == -1 end means the end of the string.
 *
 * @param o the textblock object.
 * @param n the text node the positinos refer to.
 * @param start the start of where to delete from.
 * @param end the end of the section to delete, if end == -1 it means the end of the string.
 */
static void
_evas_textblock_node_text_remove_formats_between(Evas_Object_Textblock *o,
      Evas_Object_Textblock_Node_Text *n, int start, int end)
{
   Evas_Object_Textblock_Node_Format *itr;
   int use_end = 1;
   int offset = end - start;
   itr = n->format_node;

   if (itr)
     start -= itr->offset;
   if (offset < 0) offset = 0;
   if (end < 0) use_end = 0;
   while (itr && (itr->text_node == n))
     {
        Evas_Object_Textblock_Node_Format *nnode;
        int tmp_offset = 0;

        /* start is negative when this gets relevant */
        if ((offset + start < 0) && use_end)
          {
             break;
          }
        nnode = _NODE_FORMAT(EINA_INLIST_GET(itr)->next);
        if (nnode)
          {
             tmp_offset = nnode->offset;
          }
        if (start <= 0)
          {
             /* Don't do visible adjustments because we are removing the visual
              * chars anyway and taking those into account */
             _evas_textblock_node_format_remove(o, itr, 0);
          }
        start -= tmp_offset;
        itr = nnode;
     }
}

/**
 * @internal
 * Returns the first format in the range between start and end in the textblock
 * n.
 *
 * @param o the textblock object.
 * @param n the text node the positinos refer to.
 * @param start the start of where to delete from.
 * @param end the end of the section to delete, if end == -1 it means the end of the string.
 */
static Evas_Object_Textblock_Node_Format *
_evas_textblock_node_text_get_first_format_between(
      Evas_Object_Textblock_Node_Text *n, int start, int end)
{
   Evas_Object_Textblock_Node_Format *itr;
   int use_end = 1;
   itr = n->format_node;
   if (end < 0) use_end = 0;
   while (itr && (itr->text_node == n))
     {
        start -= itr->offset;
        end -= itr->offset;
        if ((end <= 0) && use_end)
          {
             break;
          }
        if (start <= 0)
          {
             return itr;
          }
        itr = _NODE_FORMAT(EINA_INLIST_GET(itr)->next);
     }
   return NULL;
}

/**
 * Removes a text node and the corresponding format nodes.
 *
 * @param o the textblock objec.t
 * @param n the node to remove.
 */
static void
_evas_textblock_node_text_remove(Evas_Object_Textblock *o, Evas_Object_Textblock_Node_Text *n)
{
   _evas_textblock_node_text_adjust_offsets_to_start(o, n, 0, -1);

   o->text_nodes = _NODE_TEXT(eina_inlist_remove(
           EINA_INLIST_GET(o->text_nodes), EINA_INLIST_GET(n)));
   _evas_textblock_node_text_free(n);
}

/**
 * @internal
 * Return the position where the formats starts at.
 *
 * @param fmt the format to return the position of.
 * @return the position of the format in the text node it points to.
 */
static size_t
_evas_textblock_node_format_pos_get(const Evas_Object_Textblock_Node_Format *fmt)
{
   Evas_Object_Textblock_Node_Text *text;
   Evas_Object_Textblock_Node_Format *base_format;
   Evas_Object_Textblock_Node_Format *itr;
   size_t position = 0;

   if (!fmt) return 0;
   /* Find the main format node */
   text = fmt->text_node;
   base_format = text->format_node;
   EINA_INLIST_FOREACH(base_format, itr)
     {
        if (itr == fmt)
          {
             break;
          }
        position += itr->offset;
     }
   return position + fmt->offset;
}

EAPI int
evas_textblock_cursor_pos_get(const Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *n;
   size_t npos = 0;

   if (!cur) return -1;
   if (!cur->node) return 0;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   n = o->text_nodes;
   while (n != cur->node)
     {
        npos += eina_ustrbuf_length_get(n->unicode);
        n = _NODE_TEXT(EINA_INLIST_GET(n)->next);
     }
   return npos + cur->pos;
}

EAPI void
evas_textblock_cursor_pos_set(Evas_Textblock_Cursor *cur, int _pos)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *n;
   size_t pos;

   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);

   if (_pos < 0)
     {
        pos = 0;
     }
   else
     {
        pos = (size_t) _pos;
     }

   n = o->text_nodes;
   while (n && (pos >= eina_ustrbuf_length_get(n->unicode)))
     {
        pos -= eina_ustrbuf_length_get(n->unicode);
        n = _NODE_TEXT(EINA_INLIST_GET(n)->next);
     }

   if (n)
     {
        cur->node = n;
        cur->pos = pos;
     }
   else if (o->text_nodes)
     {
        /* In case we went pass the last node, we need to put the cursor
         * at the absolute end. */
        Evas_Object_Textblock_Node_Text *last_n;

        last_n = _NODE_TEXT(EINA_INLIST_GET(o->text_nodes)->last);
        pos = eina_ustrbuf_length_get(last_n->unicode);

        cur->node = last_n;
        cur->pos = pos;
     }

}

EAPI Eina_Bool
evas_textblock_cursor_line_set(Evas_Textblock_Cursor *cur, int line)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock_Item *it;

   if (!cur) return EINA_FALSE;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);

   ln = _find_layout_line_num(cur->obj, line);
   if (!ln) return EINA_FALSE;
   it = (Evas_Object_Textblock_Item *)ln->items;
   if (it)
     {
	cur->pos = it->text_pos;
	cur->node = it->text_node;
     }
   else
     {
        cur->pos = 0;

        cur->node = o->text_nodes;
     }
   return EINA_TRUE;
}

EAPI int
evas_textblock_cursor_compare(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2)
{
   Eina_Inlist *l1, *l2;

   if (!cur1) return 0;
   if (!cur2) return 0;
   if (cur1->obj != cur2->obj) return 0;
   if ((!cur1->node) || (!cur2->node)) return 0;
   if (cur1->node == cur2->node)
     {
	if (cur1->pos < cur2->pos) return -1; /* cur1 < cur2 */
	else if (cur1->pos > cur2->pos) return 1; /* cur2 < cur1 */
	return 0;
     }
   for (l1 = EINA_INLIST_GET(cur1->node),
        l2 = EINA_INLIST_GET(cur1->node); (l1) || (l2);)
     {
	if (l1 == EINA_INLIST_GET(cur2->node)) return 1; /* cur2 < cur 1 */
	else if (l2 == EINA_INLIST_GET(cur2->node)) return -1; /* cur1 < cur 2 */
	else if (!l1) return -1; /* cur1 < cur 2 */
	else if (!l2) return 1; /* cur2 < cur 1 */
	l1 = l1->prev;
	l2 = l2->next;
     }
   return 0;
}

EAPI void
evas_textblock_cursor_copy(const Evas_Textblock_Cursor *cur, Evas_Textblock_Cursor *cur_dest)
{
   if (!cur) return;
   if (!cur_dest) return;
   if (cur->obj != cur_dest->obj) return;
   cur_dest->pos = cur->pos;
   cur_dest->node = cur->node;

}


/* text controls */
/**
 * @internal
 * Free a text node. Shouldn't be used usually, it's better to use
 * @ref _evas_textblock_node_text_remove for most cases .
 *
 * @param n the text node to free
 * @see _evas_textblock_node_text_remove
 */
static void
_evas_textblock_node_text_free(Evas_Object_Textblock_Node_Text *n)
{
   if (!n) return;
   eina_ustrbuf_free(n->unicode);
   if (n->utf8)
     free(n->utf8);
   if (n->par)
      n->par->text_node = NULL;
   free(n);
}

/**
 * @internal
 * Create a new text node
 *
 * @return the new text node.
 */
static Evas_Object_Textblock_Node_Text *
_evas_textblock_node_text_new(void)
{
   Evas_Object_Textblock_Node_Text *n;

   n = calloc(1, sizeof(Evas_Object_Textblock_Node_Text));
   n->unicode = eina_ustrbuf_new();
   /* We want to layout each paragraph at least once. */
   n->dirty = EINA_TRUE;
   n->new = EINA_TRUE;

   return n;
}

/**
 * @internal
 * Break a paragraph. This does not add a PS but only splits the paragraph
 * where a ps was just added!
 *
 * @param cur the cursor to break at.
 * @param fnode the format node of the PS just added.
 * @return Returns no value.
 */
static void
_evas_textblock_cursor_break_paragraph(Evas_Textblock_Cursor *cur,
                              Evas_Object_Textblock_Node_Format *fnode)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *n;

   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);

   n = _evas_textblock_node_text_new();
   o->text_nodes = _NODE_TEXT(eina_inlist_append_relative(
                   EINA_INLIST_GET(o->text_nodes),
                   EINA_INLIST_GET(n),
                   EINA_INLIST_GET(cur->node)));
   /* Handle text and format changes. */
   if (cur->node)
     {
        Evas_Object_Textblock_Node_Format *nnode;
        size_t len, start;
        const Eina_Unicode *text;

        /* If there was a format node in the delete range,
         * make it our format and update the text_node fields,
         * otherwise, use the paragraph separator
         * of the previous paragraph. */
        nnode  = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
        if (nnode && (nnode->text_node == cur->node))
          {
             n->format_node = nnode;
             nnode->offset--; /* We don't have to take the replacement char
                                 into account anymore */
             while (nnode && (nnode->text_node == cur->node))
               {
                  nnode->text_node = n;
                  nnode = _NODE_FORMAT(EINA_INLIST_GET(nnode)->next);
               }
          }
        else
          {
             n->format_node = fnode;
          }

        /* cur->pos now points to the PS, move after. */
        start = cur->pos + 1;
        len = eina_ustrbuf_length_get(cur->node->unicode) - start;
        if (len > 0)
          {
             text = eina_ustrbuf_string_get(cur->node->unicode);
             eina_ustrbuf_append_length(n->unicode, text + start, len);
             eina_ustrbuf_remove(cur->node->unicode, start, start + len);
             cur->node->dirty = EINA_TRUE;
          }
     }
   else
     {
        fnode = o->format_nodes;
        if (fnode)
          {
             fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->last);
          }
        n->format_node = fnode;
     }
}

/**
 * @internal
 * Set the node and offset of all the curs after cur.
 *
 * @param cur the cursor.
 * @param n the current textblock node.
 * @param new_node the new node to set.
 */
static void
_evas_textblock_cursors_set_node(Evas_Object_Textblock *o,
      const Evas_Object_Textblock_Node_Text *n,
      Evas_Object_Textblock_Node_Text *new_node)
{
   Eina_List *l;
   Evas_Textblock_Cursor *data;

   if (n == o->cursor->node)
     {
        o->cursor->pos = 0;
        o->cursor->node = new_node;
     }
   EINA_LIST_FOREACH(o->cursors, l, data)
     {
        if (n == data->node)
          {
             data->pos = 0;
             data->node = new_node;
          }
     }
}

/**
 * @internal
 * Update the offset of all the cursors after cur.
 *
 * @param cur the cursor.
 * @param n the current textblock node.
 * @param start the starting pos.
 * @param offset how much to adjust (can be negative).
 */
static void
_evas_textblock_cursors_update_offset(const Evas_Textblock_Cursor *cur,
      const Evas_Object_Textblock_Node_Text *n,
      size_t start, int offset)
{
   Eina_List *l;
   Evas_Textblock_Cursor *data;
   Evas_Object_Textblock *o;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);

   if (cur != o->cursor)
     {
        if ((n == o->cursor->node) &&
              (o->cursor->pos > start))
          {
             if ((offset < 0) && (o->cursor->pos <= (size_t) (-1 * offset)))
               {
                  o->cursor->pos = 0;
               }
             else
               {
                  o->cursor->pos += offset;
               }
          }
     }
   EINA_LIST_FOREACH(o->cursors, l, data)
     {
        if (data != cur)
          {
             if ((n == data->node) &&
                   (data->pos > start))
               {
                  if ((offset < 0) && (data->pos <= (size_t) (-1 * offset)))
                    {
                       data->pos = 0;
                    }
                  else
                    {
                       data->pos += offset;
                    }
               }
             else if (!data->node)
               {
                  data->node = o->text_nodes;
                  data->pos = 0;
               }
          }
     }
}

/**
 * @internal
 * Mark that the textblock has changed.
 *
 * @param o the textblock object.
 * @param obj the evas object.
 */
static void
_evas_textblock_changed(Evas_Object_Textblock *o, Evas_Object *obj)
{
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->content_changed = 1;
   if (o->markup_text)
     {
	free(o->markup_text);
	o->markup_text = NULL;
     }

   evas_object_change(obj);
}

static void
_evas_textblock_invalidate_all(Evas_Object_Textblock *o)
{
   Evas_Object_Textblock_Node_Text *n;

   EINA_INLIST_FOREACH(o->text_nodes, n)
     {
        n->dirty = EINA_TRUE;
     }
}

EAPI int
evas_textblock_cursor_text_append(Evas_Textblock_Cursor *cur, const char *_text)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *n;
   Evas_Object_Textblock_Node_Format *fnode = NULL;
   Eina_Unicode *text;
   int len = 0;

   if (!cur) return 0;
   text = eina_unicode_utf8_to_unicode(_text, &len);
   o = (Evas_Object_Textblock *)(cur->obj->object_data);

   n = cur->node;
   if (n)
     {
        Evas_Object_Textblock_Node_Format *nnode;
        fnode = _evas_textblock_cursor_node_format_before_or_at_pos_get(cur);
        fnode = _evas_textblock_node_format_last_at_off(fnode);
        /* find the node after the current in the same paragraph
         * either we find one and then take the next, or we try to get
         * the first for the paragraph which must be after our position  */
        if (fnode)
          {
             if (!evas_textblock_cursor_format_is_visible_get(cur))
               {
                  nnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
                  if (nnode && (nnode->text_node == n))
                    {
                       fnode = nnode;
                    }
                  else
                    {
                       fnode = NULL;
                    }
               }
          }
        else
          {
             fnode = n->format_node;
          }
     }
   else if (o->text_nodes)
     {
        cur->node = o->text_nodes;
        cur->pos = 0;
     }
   else
     {
        n = _evas_textblock_node_text_new();
        o->text_nodes = _NODE_TEXT(eina_inlist_append(
                   EINA_INLIST_GET(o->text_nodes),
                   EINA_INLIST_GET(n)));
        cur->node = n;
     }

   eina_ustrbuf_insert_length(n->unicode, text, len, cur->pos);
   /* Advance the formats */
   if (fnode && (fnode->text_node == cur->node))
     fnode->offset += len;

   /* Update all the cursors after our position. */
   _evas_textblock_cursors_update_offset(cur, cur->node, cur->pos, len);

   _evas_textblock_changed(o, cur->obj);
   n->dirty = EINA_TRUE;
   free(text);

   if (!o->cursor->node)
      o->cursor->node = o->text_nodes;
   return len;
}

EAPI int
evas_textblock_cursor_text_prepend(Evas_Textblock_Cursor *cur, const char *_text)
{
   int len;
   /*append is essentially prepend without advancing */
   len = evas_textblock_cursor_text_append(cur, _text);
   cur->pos += len; /*Advance */
   return len;
}

/**
 * @internal
 * Free a format node
 *
 * @param o the textblock object
 * @param n the format node to free
 */
static void
_evas_textblock_node_format_free(Evas_Object_Textblock *o,
      Evas_Object_Textblock_Node_Format *n)
{
   if (!n) return;
   eina_stringshare_del(n->format);
   eina_stringshare_del(n->orig_format);
   if (n->anchor == ANCHOR_ITEM)
      o->anchors_item = eina_list_remove(o->anchors_item, n);
   else if (n->anchor == ANCHOR_A)
      o->anchors_a = eina_list_remove(o->anchors_a, n);
   free(n);
}

/**
 * @internal
 * Create a new format node.
 *
 * @param format the text to create the format node from.
 * @param o the textblock object.
 * @return Returns the new format node
 */
static Evas_Object_Textblock_Node_Format *
_evas_textblock_node_format_new(Evas_Object_Textblock *o, const char *format)
{
   Evas_Object_Textblock_Node_Format *n;

   n = calloc(1, sizeof(Evas_Object_Textblock_Node_Format));
   /* Create orig_format and format */
     {
        const char *match;
        size_t format_len = strlen(format);
        size_t replace_len;

        match = _style_match_tag(o->style, format, format_len, &replace_len);
        if (match)
          {
             if ((match[0] == '+') || (match[0] == '-'))
               {
                  char *norm_format;
                  norm_format = malloc(format_len + 2 + 1);
                  memcpy(norm_format, match, 2);
                  memcpy(norm_format + 2, format, format_len);
                  norm_format[format_len + 2] = '\0';
                  n->orig_format =
                     eina_stringshare_add_length(norm_format, format_len + 2);
                  free(norm_format);
               }
             else
               {
                  n->orig_format =
                     eina_stringshare_add_length(format, format_len);
               }
             n->format = eina_stringshare_add(match);
          }
        else
          {
             char *norm_format;

             norm_format = malloc(format_len + 2 + 1);
             if (norm_format)
               {
                  if (format[0] == '/')
                    {
                       memcpy(norm_format, "- ", 2);
                       memcpy(norm_format + 2, format + 1, format_len - 1);
                       norm_format[format_len + 2 - 1] = '\0';
                    }
                  else
                    {
                       memcpy(norm_format, "+ ", 2);
                       memcpy(norm_format + 2, format, format_len);
                       norm_format[format_len + 2] = '\0';
                    }
                  n->orig_format = eina_stringshare_add(norm_format);
                  free(norm_format);
               }
             n->format = eina_stringshare_ref(n->orig_format);
          }
     }

   format = n->format;

   _evas_textblock_format_is_visible(n, format);
   if (n->anchor == ANCHOR_A)
     {
        o->anchors_a = eina_list_append(o->anchors_a, n);
     }
   else if (n->anchor == ANCHOR_ITEM)
     {
        o->anchors_item = eina_list_append(o->anchors_item, n);
     }
   n->new = EINA_TRUE;

   return n;
}

static Eina_Bool
_evas_textblock_cursor_is_at_the_end(const Evas_Textblock_Cursor *cur)
{
   const Eina_Unicode *text;

   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;
   text = eina_ustrbuf_string_get(cur->node->unicode);
   return ((text[cur->pos] == 0) && (!EINA_INLIST_GET(cur->node)->next)) ?
              EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool
evas_textblock_cursor_format_append(Evas_Textblock_Cursor *cur, const char *format)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Format *n;
   Eina_Bool is_visible;

   if (!cur) return EINA_FALSE;
   if ((!format) || (format[0] == 0)) return EINA_FALSE;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   /* We should always have at least one text node */
   if (!o->text_nodes)
     {
        evas_textblock_cursor_text_prepend(cur, "");
     }

   n = _evas_textblock_node_format_new(o, format);
   is_visible = n->visible;
   if (!cur->node)
     {
	o->format_nodes = _NODE_FORMAT(eina_inlist_append(
                 EINA_INLIST_GET(o->format_nodes),
                 EINA_INLIST_GET(n)));
	cur->pos = 0;
        n->text_node = (EINA_INLIST_GET(n)->prev) ?
           _NODE_FORMAT(EINA_INLIST_GET(n)->prev)->text_node :
           o->text_nodes;
	cur->node = n->text_node;
     }
   else
     {
        Evas_Object_Textblock_Node_Format *fmt;
        fmt = _evas_textblock_cursor_node_format_before_or_at_pos_get(cur);
        n->text_node = cur->node;
        if (!fmt)
          {
             o->format_nodes = _NODE_FORMAT(eina_inlist_prepend(
                      EINA_INLIST_GET(o->format_nodes),
                      EINA_INLIST_GET(n)));
             n->offset = cur->pos;
          }
        else
          {
             if (evas_textblock_cursor_format_is_visible_get(cur))
               {
                  o->format_nodes = _NODE_FORMAT(eina_inlist_prepend_relative(
                           EINA_INLIST_GET(o->format_nodes),
                           EINA_INLIST_GET(n),
                           EINA_INLIST_GET(fmt)
                           ));
                  n->offset = fmt->offset;
                  if (fmt->text_node->format_node == fmt)
                    {
                       fmt->text_node->format_node = n;
                    }
               }
             else
               {
                  fmt = _evas_textblock_node_format_last_at_off(fmt);
                  o->format_nodes = _NODE_FORMAT(eina_inlist_append_relative(
                           EINA_INLIST_GET(o->format_nodes),
                           EINA_INLIST_GET(n),
                           EINA_INLIST_GET(fmt)
                           ));
                  if (fmt->text_node != cur->node)
                    {
                       n->offset = cur->pos;
                    }
                  else
                    {
                       n->offset = cur->pos -
                          _evas_textblock_node_format_pos_get(fmt);
                    }
               }
          }
        /* Adjust differently if we insert a format char */
        if (is_visible)
          {
             _evas_textblock_node_format_adjust_offset(o, cur->node, n,
                   -(n->offset - 1));
          }
        else
          {
             _evas_textblock_node_format_adjust_offset(o, cur->node, n,
                   -n->offset);
          }

        if (!fmt || (fmt->text_node != cur->node))
          {
             cur->node->format_node = n;
          }
     }
   if (is_visible && cur->node)
     {
        Eina_Unicode insert_char;
        /* Insert a visual representation according to the type of the
           format */
        if (_IS_PARAGRAPH_SEPARATOR(o, format))
           insert_char = _PARAGRAPH_SEPARATOR;
        else if (_IS_LINE_SEPARATOR(format))
           insert_char = '\n';
        else if (_IS_TAB(format))
           insert_char = '\t';
        else
           insert_char = EVAS_TEXTBLOCK_REPLACEMENT_CHAR;

        eina_ustrbuf_insert_char(cur->node->unicode, insert_char, cur->pos);

        /* Advance all the cursors after our cursor */
        _evas_textblock_cursors_update_offset(cur, cur->node, cur->pos, 1);
        if (_IS_PARAGRAPH_SEPARATOR(o, format))
          {
             _evas_textblock_cursor_break_paragraph(cur, n);
          }
        else
          {
             /* Handle visible format nodes here */
             cur->node->dirty = EINA_TRUE;
             n->new = EINA_FALSE;
          }
     }
   else
     {
        o->format_changed = EINA_TRUE;
     }

   _evas_textblock_changed(o, cur->obj);

   if (!o->cursor->node)
      o->cursor->node = o->text_nodes;
   return is_visible;
}

EAPI Eina_Bool
evas_textblock_cursor_format_prepend(Evas_Textblock_Cursor *cur, const char *format)
{
   Eina_Bool is_visible;
   /* append is essentially prepend without advancing */
   is_visible = evas_textblock_cursor_format_append(cur, format);
   if (is_visible)
     {
        /* Advance after the replacement char */
        evas_textblock_cursor_char_next(cur);
     }

   return is_visible;
}


EAPI void
evas_textblock_cursor_char_delete(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *n, *n2;
   const Eina_Unicode *text;
   int chr, ind, ppos;

   if (!cur || !cur->node) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   n = cur->node;

   text = eina_ustrbuf_string_get(n->unicode);
   ind = cur->pos;
   if (text[ind])
      chr = text[ind++];
   else
      chr = 0;

   if (chr == 0) return;
   ppos = cur->pos;
   eina_ustrbuf_remove(n->unicode, cur->pos, ind);
   /* Remove a format node if needed, and remove the char only if the
    * fmt node is not visible */
     {
        Eina_Bool should_merge = EINA_FALSE;
        Evas_Object_Textblock_Node_Format *fmt, *fmt2;
        fmt = _evas_textblock_cursor_node_format_at_pos_get(cur);
        if (fmt)
          {
             const char *format = NULL;
             Evas_Object_Textblock_Node_Format *last_fmt;
             /* If there's a PS it must be the last become it delimits paragraphs */
             last_fmt = _evas_textblock_node_format_last_at_off(fmt);
             format = last_fmt->format;
             if (format && _IS_PARAGRAPH_SEPARATOR(o, format))
               {
                  /* If it was a paragraph separator, we should merge the
                   * current with the next, there must be a next. */
                  should_merge = EINA_TRUE;
               }
             /* If a singnular, mark as invisible, so we'll delete it. */
             if (!format || ((*format != '+') && (*format != '-')))
               {
                  last_fmt->visible = EINA_FALSE;
               }
          }

        fmt2 = _evas_textblock_cursor_node_format_before_or_at_pos_get(cur);
        fmt2 = _evas_textblock_node_format_last_at_off(fmt2);
        _evas_textblock_node_format_adjust_offset(o, cur->node, fmt2,
              -(ind - cur->pos));

        if (should_merge)
          {
             _evas_textblock_cursor_nodes_merge(cur);
          }

        _evas_textblock_node_format_remove_matching(o, fmt);
     }

   if (cur->pos == eina_ustrbuf_length_get(n->unicode))
     {
	n2 = _NODE_TEXT(EINA_INLIST_GET(n)->next);
	if (n2)
	  {
	     cur->node = n2;
	     cur->pos = 0;
	  }
     }

   _evas_textblock_cursors_update_offset(cur, n, ppos, -(ind - ppos));
   _evas_textblock_changed(o, cur->obj);
   cur->node->dirty = EINA_TRUE;
}

EAPI void
evas_textblock_cursor_range_delete(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2)
{
   Evas_Object_Textblock_Node_Format *fnode = NULL;
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *n1, *n2;
   Eina_Bool should_merge = EINA_FALSE, reset_cursor = EINA_FALSE;

   if (!cur1 || !cur1->node) return;
   if (!cur2 || !cur2->node) return;
   if (cur1->obj != cur2->obj) return;
   o = (Evas_Object_Textblock *)(cur1->obj->object_data);
   if (evas_textblock_cursor_compare(cur1, cur2) > 0)
     {
	Evas_Textblock_Cursor *tc;

	tc = cur1;
	cur1 = cur2;
	cur2 = tc;
     }
   n1 = cur1->node;
   n2 = cur2->node;
   if ((evas_textblock_cursor_compare(o->cursor, cur1) >= 0) &&
         (evas_textblock_cursor_compare(cur2, o->cursor) >= 0))
     {
        reset_cursor = EINA_TRUE;
     }


   if (n1 == n2)
     {
        if ((cur1->pos == 0) &&
              (cur2->pos == eina_ustrbuf_length_get(n1->unicode)))
          {
             _evas_textblock_node_text_remove_formats_between(o, n1, 0, -1);
          }
        else
          {
             should_merge = _evas_textblock_node_text_adjust_offsets_to_start(o,
                   n1, cur1->pos, cur2->pos);
          }
        eina_ustrbuf_remove(n1->unicode, cur1->pos, cur2->pos);
        _evas_textblock_cursors_update_offset(cur1, cur1->node, cur1->pos, - (cur2->pos - cur1->pos));
     }
   else
     {
        Evas_Object_Textblock_Node_Text *n;
        int len;
        _evas_textblock_node_text_adjust_offsets_to_start(o, n1, cur1->pos, -1);
        n = _NODE_TEXT(EINA_INLIST_GET(n1)->next);
        /* Remove all the text nodes between */
        while (n && (n != n2))
          {
             Evas_Object_Textblock_Node_Text *nnode;

             nnode = _NODE_TEXT(EINA_INLIST_GET(n)->next);
             _evas_textblock_cursors_set_node(o, n, n1);
             _evas_textblock_node_text_remove(o, n);
             n = nnode;
          }
        should_merge = _evas_textblock_node_text_adjust_offsets_to_start(o, n2,
              0, cur2->pos);

        /* Remove the formats and the strings in the first and last nodes */
        len = eina_ustrbuf_length_get(n1->unicode);
        eina_ustrbuf_remove(n1->unicode, cur1->pos, len);
        eina_ustrbuf_remove(n2->unicode, 0, cur2->pos);
        /* Merge the nodes because we removed the PS */
        _evas_textblock_cursors_update_offset(cur1, cur1->node, cur1->pos,
              - cur1->pos);
        _evas_textblock_cursors_update_offset(cur2, cur2->node, 0, - cur2->pos);
        _evas_textblock_nodes_merge(o, n1);
     }
   fnode = _evas_textblock_cursor_node_format_at_pos_get(cur1);

   if (should_merge)
     {
        /* We call this function instead of the cursor one because we already
         * updated the cursors */
        _evas_textblock_nodes_merge(o, n1);
     }
   _evas_textblock_node_format_remove_matching(o, fnode);

   evas_textblock_cursor_copy(cur1, cur2);
   if (reset_cursor)
     evas_textblock_cursor_copy(cur1, o->cursor);

   _evas_textblock_changed(o, cur1->obj);
   n1->dirty = n2->dirty = EINA_TRUE;
}


EAPI char *
evas_textblock_cursor_content_get(const Evas_Textblock_Cursor *cur)
{
   const Eina_Unicode *ustr;
   Eina_Unicode buf[2];
   char *s;
   if (!cur || !cur->node) return NULL;
   if (evas_textblock_cursor_format_is_visible_get(cur))
     {
        const char *tmp;
        tmp  = evas_textblock_node_format_text_get(
              _evas_textblock_node_visible_at_pos_get(
                 evas_textblock_cursor_format_get(cur)));
        return (tmp) ? strdup(tmp) : NULL;
     }

   ustr = eina_ustrbuf_string_get(cur->node->unicode);
   buf[0] = ustr[cur->pos];
   buf[1] = 0;
   s = eina_unicode_unicode_to_utf8(buf, NULL);

   return s;
}

EAPI char *
evas_textblock_cursor_range_text_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *_cur2, Evas_Textblock_Text_Type format __UNUSED__)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *n1, *n2, *tnode;
   Eina_Strbuf *buf;
   Evas_Textblock_Cursor *cur2;
   buf = eina_strbuf_new();

   if (!cur1 || !cur1->node) return NULL;
   if (!_cur2 || !_cur2->node) return NULL;
   if (cur1->obj != _cur2->obj) return NULL;
   o = (Evas_Object_Textblock *)(cur1->obj->object_data);
   if (evas_textblock_cursor_compare(cur1, _cur2) > 0)
     {
	const Evas_Textblock_Cursor *tc;

	tc = cur1;
	cur1 = _cur2;
	_cur2 = tc;
     }
   n1 = cur1->node;
   n2 = _cur2->node;
   /* Work on a local copy of the cur */
   cur2 = alloca(sizeof(Evas_Textblock_Cursor));
   cur2->obj = _cur2->obj;
   evas_textblock_cursor_copy(_cur2, cur2);

   /* Parse the text between the cursors. */
   for (tnode = cur1->node ; tnode ;
         tnode = _NODE_TEXT(EINA_INLIST_GET(tnode)->next))
     {
        Evas_Object_Textblock_Node_Format *fnode;
        Eina_Unicode *text_base, *text;
        int off = 0;

        text_base = text =
           eina_unicode_strndup(eina_ustrbuf_string_get(tnode->unicode),
                                eina_ustrbuf_length_get(tnode->unicode));
        if (tnode == cur2->node)
          {
             fnode = _evas_textblock_node_text_get_first_format_between(tnode,
                   cur1->pos, cur2->pos);
          }
        else if (tnode == cur1->node)
          {
             fnode = _evas_textblock_node_text_get_first_format_between(tnode,
                   cur1->pos, -1);
          }
        else
          {
             fnode = _evas_textblock_node_text_get_first_format_between(tnode,
                   0, -1);
          }
        /* Init the offset so the first one will count starting from cur1->pos
         * and not the previous format node */
        if (tnode == cur1->node)
          {
             if (fnode)
               {
                  off = _evas_textblock_node_format_pos_get(fnode) -
                     cur1->pos - fnode->offset;
               }
             text += cur1->pos;
          }
        else
          {
             off = 0;
          }
        while (fnode && (fnode->text_node == tnode))
          {
             Eina_Unicode tmp_ch;
             off += fnode->offset;
             if ((tnode == cur2->node) &&
                   ((size_t) (text - text_base + off) >= cur2->pos))
               {
                  break;
               }
             /* No need to skip on the first run */
             tmp_ch = text[off];
             text[off] = 0; /* Null terminate the part of the string */
             _markup_get_text_append(buf, text);
             _markup_get_format_append(o, buf, fnode);
             text[off] = tmp_ch; /* Restore the char */
             text += off;
             if (fnode->visible)
               {
                  off = -1;
                  text++;
               }
             else
               {
                  off = 0;
               }
             fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
          }
        /* If we got to the last node, stop and add the rest outside */
        if (cur2->node == tnode)
          {
             /* Add the rest, skip replacement */
             /* Don't go past the second cursor pos */
             text_base[cur2->pos] = '\0';
             _markup_get_text_append(buf, text);
             free(text_base);
             break;
          }
        else
          {
             /* Add the rest, skip replacement */
             _markup_get_text_append(buf, text);
             free(text_base);
          }
     }
   /* return the string */
     {
        char *ret;
        ret = eina_strbuf_string_steal(buf);
        eina_strbuf_free(buf);
        return ret;
     }
}

EAPI const char *
evas_textblock_cursor_paragraph_text_get(const Evas_Textblock_Cursor *cur)
{
   Evas_Textblock_Cursor cur1, cur2;
   if (!cur) return NULL;
   if (!cur->node) return NULL;
   if (cur->node->utf8)
     {
        free(cur->node->utf8);
     }
   cur1.obj = cur2.obj = cur->obj;
   cur1.node = cur2.node = cur->node;
   evas_textblock_cursor_paragraph_char_first(&cur1);
   evas_textblock_cursor_paragraph_char_last(&cur2);

   cur->node->utf8 = evas_textblock_cursor_range_text_get(&cur1, &cur2,
         EVAS_TEXTBLOCK_TEXT_MARKUP);
   return cur->node->utf8;
}

EAPI int
evas_textblock_cursor_paragraph_text_length_get(const Evas_Textblock_Cursor *cur)
{
   int len;
   if (!cur) return -1;
   if (!cur->node) return -1;
   len = eina_ustrbuf_length_get(cur->node->unicode);

   if (EINA_INLIST_GET(cur->node)->next)
      return len - 1; /* Remove the paragraph separator */
   else
      return len;
}

EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_cursor_format_get(const Evas_Textblock_Cursor *cur)
{
   if (!cur) return NULL;
   if (!cur->node) return NULL;
   return _evas_textblock_cursor_node_format_at_pos_get(cur);
}

EAPI const char *
evas_textblock_node_format_text_get(const Evas_Object_Textblock_Node_Format *fmt)
{
   if (!fmt) return NULL;
   return fmt->orig_format;
}

EAPI void
evas_textblock_cursor_at_format_set(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *fmt)
{
   if (!fmt || !cur) return;
   cur->node = fmt->text_node;
   cur->pos = _evas_textblock_node_format_pos_get(fmt);
}

EAPI Eina_Bool
evas_textblock_cursor_format_is_visible_get(const Evas_Textblock_Cursor *cur)
{
   const Eina_Unicode *text;

   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;
   text = eina_ustrbuf_string_get(cur->node->unicode);
   return EVAS_TEXTBLOCK_IS_VISIBLE_FORMAT_CHAR(text[cur->pos]);
}

EAPI int
evas_textblock_cursor_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_BiDi_Direction *dir, Evas_Textblock_Cursor_Type ctype)
{
   int ret = -1;
   const Evas_Textblock_Cursor *dir_cur;
   Evas_Textblock_Cursor cur2;
   Evas_Object_Textblock *o;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);

   dir_cur = cur;
   if (ctype == EVAS_TEXTBLOCK_CURSOR_UNDER)
     {
        ret = evas_textblock_cursor_pen_geometry_get(cur, cx, cy, cw, ch);
     }
   else if (ctype == EVAS_TEXTBLOCK_CURSOR_BEFORE)
     {
        /* In the case of a "before cursor", we should get the coordinates
         * of just after the previous char (which in bidi text may not be
         * just before the current char). */
        Evas_Coord x, y, h, w;
        Evas_Object_Textblock_Node_Format *fmt;

        /* If it's at the end of the line, we want to get the position, not
         * the position of the previous */
        if ((cur->pos > 0) && !_evas_textblock_cursor_is_at_the_end(cur))
          {
             Eina_Bool before_char = EINA_FALSE;
             cur2.obj = cur->obj;
             evas_textblock_cursor_copy(cur, &cur2);
             evas_textblock_cursor_char_prev(&cur2);

             fmt = _evas_textblock_cursor_node_format_at_pos_get(&cur2);

             if (!fmt || !_IS_LINE_SEPARATOR(fmt->format))
               {
                  dir_cur = &cur2;
                  before_char = EINA_FALSE;
               }
             else
               {
                  before_char = EINA_TRUE;
               }
             ret = evas_textblock_cursor_pen_geometry_get(
                   dir_cur, &x, &y, &w, &h);
#ifdef BIDI_SUPPORT
             /* Adjust if the char is an rtl char */
             if (ret >= 0)
               {
                  Eina_Bool is_rtl = EINA_FALSE;
                  if (dir_cur->node->par->is_bidi)
                    {
                       Evas_Object_Textblock_Line *ln;
                       Evas_Object_Textblock_Item *it;
                       _find_layout_item_match(dir_cur, &ln, &it);
                       if ((it->type == EVAS_TEXTBLOCK_ITEM_TEXT) &&
                             (_ITEM_TEXT(it)->text_props.bidi.dir ==
                              EVAS_BIDI_DIRECTION_RTL))
                          is_rtl = EINA_TRUE;
                       else if ((it->type == EVAS_TEXTBLOCK_ITEM_FORMAT) &&
                             (_ITEM_FORMAT(it)->bidi_dir ==
                              EVAS_BIDI_DIRECTION_RTL))
                          is_rtl = EINA_TRUE;
                    }

                  if ((!before_char && is_rtl) ||
                        (before_char && !is_rtl))
                    {
                       /* Just don't advance the width */
                       w = 0;
                    }
               }
#endif
          }
        else if (cur->pos == 0)
          {
             ret = evas_textblock_cursor_pen_geometry_get(
                   dir_cur, &x, &y, &w, &h);
#ifdef BIDI_SUPPORT
             Eina_Bool is_rtl = EINA_FALSE;
             if (dir_cur->node->par->is_bidi)
               {
                  Evas_Object_Textblock_Line *ln;
                  Evas_Object_Textblock_Item *it;
                  _find_layout_item_match(dir_cur, &ln, &it);
                  if ((it->type == EVAS_TEXTBLOCK_ITEM_TEXT) &&
                        (_ITEM_TEXT(it)->text_props.bidi.dir ==
                         EVAS_BIDI_DIRECTION_RTL))
                     is_rtl = EINA_TRUE;
                  else if ((it->type == EVAS_TEXTBLOCK_ITEM_FORMAT) &&
                        (_ITEM_FORMAT(it)->bidi_dir ==
                         EVAS_BIDI_DIRECTION_RTL))
                     is_rtl = EINA_TRUE;
               }

             /* Adjust if the char is an rtl char */
             if ((ret >= 0) && (!is_rtl))
               {
                  /* Just don't advance the width */
                  w = 0;
               }
#endif
          }
        else
          {
             ret = evas_textblock_cursor_pen_geometry_get(
                   dir_cur, &x, &y, &w, &h);
          }
        if (ret >= 0)
          {
             if (cx) *cx = x + w;
             if (cy) *cy = y;
             if (cw) *cw = 0;
             if (ch) *ch = h;
          }
     }

   if (dir && dir_cur && dir_cur->node)
     {
#ifdef BIDI_SUPPORT
        Eina_Bool is_rtl = EINA_FALSE;
        if (dir_cur->node->par->is_bidi)
          {
             Evas_Object_Textblock_Line *ln;
             Evas_Object_Textblock_Item *it;
             _find_layout_item_match(dir_cur, &ln, &it);
             if ((it->type == EVAS_TEXTBLOCK_ITEM_TEXT) &&
                   (_ITEM_TEXT(it)->text_props.bidi.dir ==
                    EVAS_BIDI_DIRECTION_RTL))
                is_rtl = EINA_TRUE;
             else if ((it->type == EVAS_TEXTBLOCK_ITEM_FORMAT) &&
                   (_ITEM_FORMAT(it)->bidi_dir ==
                    EVAS_BIDI_DIRECTION_RTL))
                is_rtl = EINA_TRUE;
          }

        if (_evas_textblock_cursor_is_at_the_end(dir_cur) && (dir_cur->pos > 0))
          {
             *dir = (is_rtl) ?
                EVAS_BIDI_DIRECTION_RTL : EVAS_BIDI_DIRECTION_LTR;
          }
        else if (dir_cur->pos > 0)
          {
             *dir = (is_rtl) ?
                EVAS_BIDI_DIRECTION_RTL : EVAS_BIDI_DIRECTION_LTR;
          }
        else
#endif
          {
             *dir = EVAS_BIDI_DIRECTION_LTR;
          }
     }
   return ret;
}

/**
 * @internal
 * Returns the geometry/pen position (depending on query_func) of the char
 * at pos.
 *
 * @param cur the position of the char.
 * @param query_func the query function to use.
 * @param cx the x of the char (or pen_x in the case of pen position).
 * @param cy the y of the char.
 * @param cw the w of the char (or advance in the case pen position).
 * @param ch the h of the char.
 * @return line number of the char on success, -1 on error.
 */
static int
_evas_textblock_cursor_char_pen_geometry_common_get(int (*query_func) (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int pos, int *cx, int *cy, int *cw, int *ch), const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;
   Evas_Object_Textblock_Text_Item *ti = NULL;
   Evas_Object_Textblock_Format_Item *fi = NULL;
   int x = 0, y = 0, w = 0, h = 0;
   int pos, ret;
   Eina_Bool previous_format;

   if (!cur) return -1;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);

   if (!cur->node)
     {
        if (!o->text_nodes)
          {
             if (!o->paragraphs) return -1;
             ln = o->paragraphs->lines;
             if (!ln) return -1;
             if (cx) *cx = ln->x;
             if (cy) *cy = ln->par->y + ln->y;
             if (cw) *cw = ln->w;
             if (ch) *ch = ln->h;
             return ln->par->line_no + ln->line_no;
          }
        else
          return -1;
     }

   previous_format = _find_layout_item_match(cur, &ln, &it);
   if (!it)
     {
        return -1;
     }
   if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
     {
        ti = _ITEM_TEXT(it);
     }
   else
     {
        fi = _ITEM_FORMAT(it);
     }

   if (ln && ti)
     {
        pos = cur->pos - ti->parent.text_pos;
        ret = -1;

        if (pos < 0) pos = 0;
        if (ti->parent.format->font.font)
          {
             ret = query_func(cur->ENDT,
                   ti->parent.format->font.font,
                   &ti->text_props,
                   pos,
                   &x, &y, &w, &h);
          }

        x += ln->x + _ITEM(ti)->x;

        if (x < ln->x)
          {
             x = ln->x;
          }
	y = ln->par->y + ln->y;
	h = ln->h;
     }
   else if (ln && fi)
     {
        if (previous_format)
          {
             if (_IS_LINE_SEPARATOR(fi->item))
               {
                  x = 0;
                  y = ln->par->y + ln->y + ln->h;
               }
             else
               {
#ifdef BIDI_SUPPORT
                  if (ln->par->direction == EVAS_BIDI_DIRECTION_RTL)
                    {
                       x = ln->x;
                    }
                  else
#endif
                    {
                       x = ln->x + ln->w;
                    }
                  y = ln->par->y + ln->y;
               }
             w = 0;
             h = ln->h;
          }
        else
          {
             x = ln->x + _ITEM(fi)->x;
             y = ln->par->y + ln->y;
             w = _ITEM(fi)->w;
             h = ln->h;
          }
     }
   else
     {
	return -1;
     }
   if (cx) *cx = x;
   if (cy) *cy = y;
   if (cw) *cw = w;
   if (ch) *ch = h;
   return ln->par->line_no + ln->line_no;
}

EAPI int
evas_textblock_cursor_char_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   return _evas_textblock_cursor_char_pen_geometry_common_get(
         cur->ENFN->font_char_coords_get, cur, cx, cy, cw, ch);
}

EAPI int
evas_textblock_cursor_pen_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   return _evas_textblock_cursor_char_pen_geometry_common_get(
         cur->ENFN->font_pen_coords_get, cur, cx, cy, cw, ch);
}

EAPI int
evas_textblock_cursor_line_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;
   int x, y, w, h;

   if (!cur) return -1;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   if (!cur->node)
     {
        ln = o->paragraphs->lines;
     }
   else
     {
        _find_layout_item_match(cur, &ln, &it);
     }
   if (!ln) return -1;
   x = ln->x;
   y = ln->par->y + ln->y;
   w = ln->w;
   h = ln->h;
   if (cx) *cx = x;
   if (cy) *cy = y;
   if (cw) *cw = w;
   if (ch) *ch = h;
   return ln->par->line_no + ln->line_no;
}

EAPI Eina_Bool
evas_textblock_cursor_char_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Paragraph *found_par;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock_Item *it = NULL;

   if (!cur) return EINA_FALSE;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   x += o->style_pad.l;
   y += o->style_pad.t;

   found_par = _layout_find_paragraph_by_y(o, y);
   if (found_par)
     {
        _layout_paragraph_render(o, found_par);
        EINA_INLIST_FOREACH(found_par->lines, ln)
          {
             if (ln->par->y + ln->y > y) break;
             if ((ln->par->y + ln->y <= y) && ((ln->par->y + ln->y + ln->h) > y))
               {
                  /* If before or after the line, go to start/end according
                   * to paragraph direction. */
                  if (x < ln->x)
                    {
                       cur->pos = ln->items->text_pos;
                       cur->node = found_par->text_node;
                       if (found_par->direction == EVAS_BIDI_DIRECTION_RTL)
                         {
                            evas_textblock_cursor_line_char_last(cur);
                         }
                       else
                         {
                            evas_textblock_cursor_line_char_first(cur);
                         }
                       return EINA_TRUE;
                    }
                  else if (x >= ln->x + ln->w)
                    {
                       cur->pos = ln->items->text_pos;
                       cur->node = found_par->text_node;
                       if (found_par->direction == EVAS_BIDI_DIRECTION_RTL)
                         {
                            evas_textblock_cursor_line_char_first(cur);
                         }
                       else
                         {
                            evas_textblock_cursor_line_char_last(cur);
                         }
                       return EINA_TRUE;
                    }

                  EINA_INLIST_FOREACH(ln->items, it)
                    {
                       if (((it->x + ln->x) <= x) && (((it->x + ln->x) + it->adv) > x))
                         {
                            if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
                              {
                                 int pos;
                                 int cx, cy, cw, ch;
                                 Evas_Object_Textblock_Text_Item *ti;
                                 ti = _ITEM_TEXT(it);

                                 pos = -1;
                                 if (ti->parent.format->font.font)
                                   pos = cur->ENFN->font_char_at_coords_get(
                                         cur->ENDT,
                                         ti->parent.format->font.font,
                                         &ti->text_props,
                                         x - it->x - ln->x, 0,
                                         &cx, &cy, &cw, &ch);
                                 if (pos < 0)
                                   return EINA_FALSE;
                                 cur->pos = pos + it->text_pos;
                                 cur->node = it->text_node;
                                 return EINA_TRUE;
                              }
                            else
                              {
                                 Evas_Object_Textblock_Format_Item *fi;
                                 fi = _ITEM_FORMAT(it);
                                 cur->pos = fi->parent.text_pos;
                                 cur->node = found_par->text_node;
                                 return EINA_TRUE;
                              }
                         }
                    }
               }
          }
     }
   else if (o->paragraphs && (y > o->paragraphs->y + o->formatted.h))
     {
        /* If we are after the last paragraph, use the last position in the
         * text. */
        evas_textblock_cursor_paragraph_last(cur);
        return EINA_TRUE;
     }
   else if (o->paragraphs && (y < o->paragraphs->y))
     {
        evas_textblock_cursor_paragraph_first(cur);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI int
evas_textblock_cursor_line_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord y)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Paragraph *found_par;
   Evas_Object_Textblock_Line *ln;

   if (!cur) return -1;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   y += o->style_pad.t;

   found_par = _layout_find_paragraph_by_y(o, y);

   if (found_par)
     {
        _layout_paragraph_render(o, found_par);
        EINA_INLIST_FOREACH(found_par->lines, ln)
          {
             if (ln->par->y + ln->y > y) break;
             if ((ln->par->y + ln->y <= y) && ((ln->par->y + ln->y + ln->h) > y))
               {
                  evas_textblock_cursor_line_set(cur, ln->par->line_no +
                        ln->line_no);
                  return ln->par->line_no + ln->line_no;
               }
          }
     }
   else if (o->paragraphs && (y > o->paragraphs->y + o->formatted.h))
     {
        int line_no = 0;
        /* If we are after the last paragraph, use the last position in the
         * text. */
        evas_textblock_cursor_paragraph_last(cur);
        if (cur->node && cur->node->par)
          {
             line_no = cur->node->par->line_no;
             if (cur->node->par->lines)
               {
                  line_no += ((Evas_Object_Textblock_Line *)
                        EINA_INLIST_GET(cur->node->par->lines)->last)->line_no;
               }
          }
        return line_no;
     }
   else if (o->paragraphs && (y < o->paragraphs->y))
     {
        int line_no = 0;
        evas_textblock_cursor_paragraph_first(cur);
        if (cur->node && cur->node->par)
          {
             line_no = cur->node->par->line_no;
          }
        return line_no;
     }
   return -1;
}

/**
 * @internal
 * Updates x and w according to the text direction, position in text and
 * if it's a special case switch
 *
 * @param ti the text item we are working on
 * @param x the current x (we get) and the x we return
 * @param w the current w (we get) and the w we return
 * @param start if this is the first item or not
 * @param switch_items toogles item switching (rtl cases)
 */
static void
_evas_textblock_range_calc_x_w(const Evas_Object_Textblock_Item *it,
      Evas_Coord *x, Evas_Coord *w, Eina_Bool start, Eina_Bool switch_items)
{
   if ((start && !switch_items) || (!start && switch_items))
     {
#ifdef BIDI_SUPPORT
        if (((it->type == EVAS_TEXTBLOCK_ITEM_TEXT) &&
            _ITEM_TEXT(it)->text_props.bidi.dir == EVAS_BIDI_DIRECTION_RTL)
            ||
            ((it->type == EVAS_TEXTBLOCK_ITEM_FORMAT) &&
             _ITEM_FORMAT(it)->bidi_dir == EVAS_BIDI_DIRECTION_RTL))
          {
             *w = *x + *w;
             *x = 0;
          }
        else
#endif
          {
             *w = it->adv - *x;
          }
     }
   else
     {
#ifdef BIDI_SUPPORT
        if (((it->type == EVAS_TEXTBLOCK_ITEM_TEXT) &&
            _ITEM_TEXT(it)->text_props.bidi.dir == EVAS_BIDI_DIRECTION_RTL)
            ||
            ((it->type == EVAS_TEXTBLOCK_ITEM_FORMAT) &&
             _ITEM_FORMAT(it)->bidi_dir == EVAS_BIDI_DIRECTION_RTL))
          {
             *x = *x + *w;
             *w = it->adv - *x;
          }
        else
#endif
          {
             *w = *x;
             *x = 0;
          }
     }

}

/**
 * @internal
 * Returns the geometry of the range in line ln. Cur1 is the start cursor,
 * cur2 is the end cursor, NULL means from the start or to the end accordingly.
 * Assumes that ln is valid, and that at least one of cur1 and cur2 is not NULL.
 *
 * @param ln the line to work on.
 * @param cur1 the start cursor
 * @param cur2 the end cursor
 * @return Returns the geometry of the range
 */
static Eina_List *
_evas_textblock_cursor_range_in_line_geometry_get(
      const Evas_Object_Textblock_Line *ln, const Evas_Textblock_Cursor *cur1,
      const Evas_Textblock_Cursor *cur2)
{
   Evas_Object_Textblock_Item *it;
   Evas_Object_Textblock_Item *it1, *it2;
   Eina_List *rects = NULL;
   Evas_Textblock_Rectangle *tr;
   size_t start, end;
   Eina_Bool switch_items;
   const Evas_Textblock_Cursor *cur;

   cur = (cur1) ? cur1 : cur2;

   /* Find the first and last items */
   it1 = it2 = NULL;
   start = end = 0;
   EINA_INLIST_FOREACH(ln->items, it)
     {
        size_t item_len;
        item_len = (it->type == EVAS_TEXTBLOCK_ITEM_TEXT) ?
           _ITEM_TEXT(it)->text_props.text_len
           : 1;
        if ((!cur1 || (cur1->pos < it->text_pos + item_len)) &&
              (!cur2 || (cur2->pos >= it->text_pos)))
          {
             if (!it1)
               {
                  it1 = it;
                  start = item_len; /* start stores the first item_len */
               }
             it2 = it;
             end = item_len; /* end stores the last item_len */
          }
     }

   /* If we couldn't find even one item, return */
   if (!it1) return NULL;

   /* If the first item is logically before or equal the second item
    * we have to set start and end differently than in the other case */
   if (it1->text_pos <= it2->text_pos)
     {
        start = (cur1) ? (cur1->pos - it1->text_pos) : 0;
        end = (cur2) ? (cur2->pos - it2->text_pos) : end;
        switch_items = EINA_FALSE;
     }
   else
     {
        start = (cur2) ? (cur2->pos - it1->text_pos) : start;
        end = (cur1) ? (cur1->pos - it2->text_pos) : 0;
        switch_items = EINA_TRUE;
     }

   /* IMPORTANT: Don't use cur1/cur2 past this point (because they probably
    * don't make sense anymore. That's why there are start and end),
    * unless you know what you are doing */

   /* Special case when they share the same item and it's a text item */
   if ((it1 == it2) && (it1->type == EVAS_TEXTBLOCK_ITEM_TEXT))
     {
        Evas_Coord x1, w1, x2, w2;
        Evas_Coord x, w, y, h;
        Evas_Object_Textblock_Text_Item *ti;
        int ret;

        ti = _ITEM_TEXT(it1);
        ret = cur->ENFN->font_pen_coords_get(cur->ENDT,
              ti->parent.format->font.font,
              &ti->text_props,
              start,
              &x1, &y, &w1, &h);
        if (!ret)
          {
             return NULL;
          }
        ret = cur->ENFN->font_pen_coords_get(cur->ENDT,
              ti->parent.format->font.font,
              &ti->text_props,
              end,
              &x2, &y, &w2, &h);
        if (!ret)
          {
             return NULL;
          }

        /* Make x2 the one on the right */
        if (x2 < x1)
          {
             Evas_Coord tmp;
             tmp = x1;
             x1 = x2;
             x2 = tmp;

             tmp = w1;
             w1 = w2;
             w2 = tmp;
          }

#ifdef BIDI_SUPPORT
        if (ti->text_props.bidi.dir == EVAS_BIDI_DIRECTION_RTL)
          {
             x = x1 + w1;
             w = x2 + w2 - x;
          }
        else
#endif
          {
             x = x1;
             w = x2 - x1;
          }
        if (w > 0)
          {
             tr = calloc(1, sizeof(Evas_Textblock_Rectangle));
             rects = eina_list_append(rects, tr);
             tr->x = ln->x + it1->x + x;
             tr->y = ln->par->y + ln->y;
             tr->h = ln->h;
             tr->w = w;
          }
     }
   else if (it1 != it2)
     {
        /* Get the middle items */
        Evas_Coord min_x, max_x;
        Evas_Coord x, w;
        it = _ITEM(EINA_INLIST_GET(it1)->next);
        min_x = max_x = it->x;

        if (it1->type == EVAS_TEXTBLOCK_ITEM_TEXT)
          {
             Evas_Coord y, h;
             Evas_Object_Textblock_Text_Item *ti;
             int ret;
             ti = _ITEM_TEXT(it1);

             ret = cur->ENFN->font_pen_coords_get(cur->ENDT,
                   ti->parent.format->font.font,
                   &ti->text_props,
                   start,
                   &x, &y, &w, &h);
             if (!ret)
               {
                  /* BUG! Skip the first item */
                  x = w = 0;
               }
             else
               {
                  _evas_textblock_range_calc_x_w(it1, &x, &w, EINA_TRUE,
                        switch_items);
               }
          }
        else
          {
             x = 0;
             w = it1->w;
             _evas_textblock_range_calc_x_w(it1, &x, &w, EINA_TRUE,
                   switch_items);
          }
        if (w > 0)
          {
             tr = calloc(1, sizeof(Evas_Textblock_Rectangle));
             rects = eina_list_append(rects, tr);
             tr->x = ln->x + it1->x + x;
             tr->y = ln->par->y + ln->y;
             tr->h = ln->h;
             tr->w = w;
          }

        while (it && (it != it2))
          {
             max_x = it->x + it->adv;
             it = (Evas_Object_Textblock_Item *) EINA_INLIST_GET(it)->next;
          }
        if (min_x != max_x)
          {
             tr = calloc(1, sizeof(Evas_Textblock_Rectangle));
             rects = eina_list_append(rects, tr);
             tr->x = ln->x + min_x;
             tr->y = ln->par->y + ln->y;
             tr->h = ln->h;
             tr->w = max_x - min_x;
          }
        if (it2->type == EVAS_TEXTBLOCK_ITEM_TEXT)
          {
             Evas_Coord y, h;
             Evas_Object_Textblock_Text_Item *ti;
             int ret;
             ti = _ITEM_TEXT(it2);

             ret = cur->ENFN->font_pen_coords_get(cur->ENDT,
                   ti->parent.format->font.font,
                   &ti->text_props,
                   end,
                   &x, &y, &w, &h);
             if (!ret)
               {
                  /* BUG! skip the last item */
                  x = w = 0;
               }
             else
               {
                  _evas_textblock_range_calc_x_w(it2, &x, &w, EINA_FALSE,
                        switch_items);
               }
          }
        else
          {
             x = 0;
             w = it2->w;
             _evas_textblock_range_calc_x_w(it2, &x, &w, EINA_FALSE,
                        switch_items);
          }
        if (w > 0)
          {
             tr = calloc(1, sizeof(Evas_Textblock_Rectangle));
             rects = eina_list_append(rects, tr);
             tr->x = ln->x + it2->x + x;
             tr->y = ln->par->y + ln->y;
             tr->h = ln->h;
             tr->w = w;
          }
     }
   return rects;
}
EAPI Eina_List *
evas_textblock_cursor_range_geometry_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln1, *ln2;
   Evas_Object_Textblock_Item *it1, *it2;
   Eina_List *rects = NULL;
   Evas_Textblock_Rectangle *tr;

   if (!cur1 || !cur1->node) return NULL;
   if (!cur2 || !cur2->node) return NULL;
   if (cur1->obj != cur2->obj) return NULL;
   o = (Evas_Object_Textblock *)(cur1->obj->object_data);
   if (!o->formatted.valid) _relayout(cur1->obj);
   if (evas_textblock_cursor_compare(cur1, cur2) > 0)
     {
	const Evas_Textblock_Cursor *tc;

	tc = cur1;
	cur1 = cur2;
	cur2 = tc;
     }

   ln1 = ln2 = NULL;
   it1 = it2 = NULL;
   _find_layout_item_match(cur1, &ln1, &it1);
   if (!ln1 || !it1) return NULL;
   _find_layout_item_match(cur2, &ln2, &it2);
   if (!ln2 || !it2) return NULL;

   if (ln1 == ln2)
     {
        rects = _evas_textblock_cursor_range_in_line_geometry_get(ln1,
              cur1, cur2);
     }
   else
     {
        Evas_Object_Textblock_Line *plni, *lni;
        Eina_List *rects2 = NULL;
        /* Handle the first line */
        rects = _evas_textblock_cursor_range_in_line_geometry_get(ln1,
              cur1, NULL);

        /* Handle the lines between the first and the last line */
        lni = (Evas_Object_Textblock_Line *) EINA_INLIST_GET(ln1)->next;
        if (!lni && (ln1->par != ln2->par))
          {
             lni = ((Evas_Object_Textblock_Paragraph *)
                    EINA_INLIST_GET(ln1->par)->next)->lines;
          }
        while (lni && (lni != ln2))
          {
	     tr = calloc(1, sizeof(Evas_Textblock_Rectangle));
	     rects = eina_list_append(rects, tr);
	     tr->x = lni->x;
	     tr->y = lni->par->y + lni->y;
	     tr->h = lni->h;
	     tr->w = lni->w;
             plni = lni;
             lni = (Evas_Object_Textblock_Line *) EINA_INLIST_GET(lni)->next;
             if (!lni && (plni->par != ln2->par))
               {
                  lni = ((Evas_Object_Textblock_Paragraph *)
                     EINA_INLIST_GET(plni->par)->next)->lines;
               }
          }
        rects2 = _evas_textblock_cursor_range_in_line_geometry_get(ln2,
              NULL, cur2);
        rects = eina_list_merge(rects, rects2);
     }
   return rects;
}

EAPI Eina_Bool
evas_textblock_cursor_format_item_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Format_Item *fi;
   Evas_Object_Textblock_Item *it = NULL;
   Evas_Coord x, y, w, h;

   if (!cur || !evas_textblock_cursor_format_is_visible_get(cur)) return EINA_FALSE;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   if (!evas_textblock_cursor_format_is_visible_get(cur)) return EINA_FALSE;
   _find_layout_item_line_match(cur->obj, cur->node, cur->pos, &ln, &it);
   fi = _ITEM_FORMAT(it);
   if ((!ln) || (!fi)) return EINA_FALSE;
   x = ln->x + fi->parent.x;
   y = ln->par->y + ln->y + ln->baseline + fi->y;
   w = fi->parent.w;
   h = fi->parent.h;
   if (cx) *cx = x;
   if (cy) *cy = y;
   if (cw) *cw = w;
   if (ch) *ch = h;
   return EINA_TRUE;
}

EAPI Eina_Bool
evas_textblock_cursor_eol_get(const Evas_Textblock_Cursor *cur)
{
   Eina_Bool ret = EINA_FALSE;
   Evas_Textblock_Cursor cur2;
   if (!cur) return EINA_FALSE;

   cur2.obj = cur->obj;
   evas_textblock_cursor_copy(cur, &cur2);
   evas_textblock_cursor_line_char_last(&cur2);
   if (cur2.pos == cur->pos)
     {
        ret = EINA_TRUE;
     }
   return ret;
}

/* general controls */
EAPI Eina_Bool
evas_object_textblock_line_number_geometry_get(const Evas_Object *obj, int line, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock_Line *ln;

   TB_HEAD_RETURN(0);
   ln = _find_layout_line_num(obj, line);
   if (!ln) return EINA_FALSE;
   if (cx) *cx = ln->x;
   if (cy) *cy = ln->par->y + ln->y;
   if (cw) *cw = ln->w;
   if (ch) *ch = ln->h;
   return EINA_TRUE;
}

EAPI void
evas_object_textblock_clear(Evas_Object *obj)
{
   Eina_List *l;
   Evas_Textblock_Cursor *cur;

   TB_HEAD();
   if (o->paragraphs)
     {
	_paragraphs_free(obj, o->paragraphs);
	o->paragraphs = NULL;
     }

   _nodes_clear(obj);
   o->cursor->node = NULL;
   o->cursor->pos = 0;
   EINA_LIST_FOREACH(o->cursors, l, cur)
     {
	cur->node = NULL;
	cur->pos = 0;

     }
   _evas_textblock_changed(o, obj);
}

EAPI void
evas_object_textblock_size_formatted_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   TB_HEAD();
   if (!o->formatted.valid) _relayout(obj);
   if (w) *w = o->formatted.w;
   if (h) *h = o->formatted.h;
}

static void
_size_native_calc_line_finalize(const Evas_Object *obj, Eina_List *items,
      Evas_Coord *ascent, Evas_Coord *descent, Evas_Coord *w)
{
   Evas_Object_Textblock_Item *it;
   Eina_List *i;

   it = eina_list_data_get(items);
   /* If there are no text items yet, calc ascent/descent
    * according to the current format. */
   if (it && (*ascent + *descent == 0))
      _layout_format_ascent_descent_adjust(obj, ascent, descent, it->format);

   *w = 0;
   /* Adjust all the item sizes according to the final line size,
    * and update the x positions of all the items of the line. */
   EINA_LIST_FOREACH(items, i, it)
     {
        if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
          {
             Evas_Coord fw, fh, fy;

             Evas_Object_Textblock_Format_Item *fi = _ITEM_FORMAT(it);
             if (!fi->formatme) goto loop_advance;
             _layout_calculate_format_item_size(obj, fi, ascent,
                   descent, &fy, &fw, &fh);
          }

loop_advance:
        *w += it->adv;
     }
}

/* FIXME: doc */
static void
_size_native_calc_paragraph_size(const Evas_Object *obj,
      const Evas_Object_Textblock *o,
      const Evas_Object_Textblock_Paragraph *par,
      Evas_Coord *_w, Evas_Coord *_h)
{
   Eina_List *i;
   Evas_Object_Textblock_Item *it;
   Eina_List *line_items = NULL;
   Evas_Coord w = 0, y = 0, wmax = 0, h = 0, ascent = 0, descent = 0;

   EINA_LIST_FOREACH(par->logical_items, i, it)
     {
        line_items = eina_list_append(line_items, it);
        if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
          {
             Evas_Object_Textblock_Format_Item *fi = _ITEM_FORMAT(it);
             if (fi->item && (_IS_LINE_SEPARATOR(fi->item) ||
                      _IS_PARAGRAPH_SEPARATOR(o, fi->item)))
               {
                  _size_native_calc_line_finalize(obj, line_items, &ascent,
                        &descent, &w);

                  if (ascent + descent > h)
                     h = ascent + descent;

                  y += h;
                  if (w > wmax)
                     wmax = w;
                  h = 0;
                  ascent = descent = 0;
                  line_items = eina_list_free(line_items);
               }
             else
               {
                  Evas_Coord fw, fh, fy;
                  /* If there are no text items yet, calc ascent/descent
                   * according to the current format. */
                  if (it && (ascent + descent == 0))
                     _layout_format_ascent_descent_adjust(obj, &ascent,
                           &descent, it->format);

                  _layout_calculate_format_item_size(obj, fi, &ascent,
                        &descent, &fy, &fw, &fh);
               }
          }
        else
          {
             _layout_format_ascent_descent_adjust(obj, &ascent,
                   &descent, it->format);
          }
     }

   _size_native_calc_line_finalize(obj, line_items, &ascent, &descent, &w);

   line_items = eina_list_free(line_items);

   /* Do the last addition */
   if (ascent + descent > h)
      h = ascent + descent;

   if (w > wmax)
      wmax = w;

   *_h = y + h;
   *_w = wmax;
}

EAPI void
evas_object_textblock_size_native_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   TB_HEAD();
   if (!o->native.valid)
     {
        Evas_Coord wmax = 0, hmax = 0;
        Evas_Object_Textblock_Paragraph *par;
        /* We just want the layout objects to update, should probably
         * split that. */
        if (!o->formatted.valid) _relayout(obj);
        EINA_INLIST_FOREACH(o->paragraphs, par)
          {
             Evas_Coord tw, th;
             _size_native_calc_paragraph_size(obj, o, par, &tw, &th);
             if (tw > wmax)
                wmax = tw;
             hmax += th;
          }

        o->native.w = wmax;
        o->native.h = hmax;

	o->native.valid = 1;
        o->content_changed = 0;
        o->format_changed = EINA_FALSE;
     }
   if (w) *w = o->native.w;
   if (h) *h = o->native.h;
}

EAPI void
evas_object_textblock_style_insets_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b)
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
#ifdef HAVE_LINEBREAK
   static Eina_Bool linebreak_init = EINA_FALSE;
   if (!linebreak_init)
     {
        linebreak_init = EINA_TRUE;
        init_linebreak();
     }
#endif

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
   o->newline_is_ps = EINA_TRUE;
}

static void *
evas_object_textblock_new(void)
{
   Evas_Object_Textblock *o;

   /* alloc obj private data */
   EVAS_MEMPOOL_INIT(_mp_obj, "evas_object_textblock", Evas_Object_Textblock, 64, NULL);
   o = EVAS_MEMPOOL_ALLOC(_mp_obj, Evas_Object_Textblock);
   if (!o) return NULL;
   EVAS_MEMPOOL_PREP(_mp_obj, o, Evas_Object_Textblock);
   o->magic = MAGIC_OBJ_TEXTBLOCK;
   o->cursor = calloc(1, sizeof(Evas_Textblock_Cursor));
   _format_command_init();
   return o;
}

static void
evas_object_textblock_free(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   evas_object_textblock_clear(obj);
   evas_object_textblock_style_set(obj, NULL);
   o = (Evas_Object_Textblock *)(obj->object_data);
   free(o->cursor);
   while (o->cursors)
     {
	Evas_Textblock_Cursor *cur;

	cur = (Evas_Textblock_Cursor *)o->cursors->data;
	o->cursors = eina_list_remove_list(o->cursors, o->cursors);
	free(cur);
     }
   if (o->repch) eina_stringshare_del(o->repch);
   if (o->ellip_ti) _item_free(obj, NULL, _ITEM(o->ellip_ti));
   o->magic = 0;
   EVAS_MEMPOOL_FREE(_mp_obj, o);
  _format_command_shutdown();
}


static void
evas_object_textblock_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Textblock_Paragraph *par, *start = NULL;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock *o;
   int i, j;
   unsigned char r = 0, g = 0, b = 0, a = 0;
   unsigned char r2 = 0, g2 = 0, b2 = 0, a2 = 0;
   unsigned char r3 = 0, g3 = 0, b3 = 0, a3 = 0;
   int cx, cy, cw, ch, clip;
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
   /* FIXME: This clipping is just until we fix inset handling correctly. */
   ENFN->context_clip_clip(output, context,
                              obj->cur.geometry.x + x,
                              obj->cur.geometry.y + y,
                              obj->cur.geometry.w,
                              obj->cur.geometry.h);
   clip = ENFN->context_clip_get(output, context, &cx, &cy, &cw, &ch);
   /* If there are no paragraphs and thus there are no lines,
    * there's nothing left to do. */
   if (!o->paragraphs) return;

#define ITEM_WALK() \
   EINA_INLIST_FOREACH(start, par) \
     { \
        if (!par->visible) continue; \
        if (clip) \
          { \
             if ((obj->cur.geometry.y + y + par->y + par->h) < (cy - 20)) \
             continue; \
             if ((obj->cur.geometry.y + y + par->y) > (cy + ch + 20)) \
             break; \
          } \
        _layout_paragraph_render(o, par); \
        EINA_INLIST_FOREACH(par->lines, ln) \
          { \
             Evas_Object_Textblock_Item *itr; \
             \
             if (clip) \
               { \
                  if ((obj->cur.geometry.y + y + par->y + ln->y + ln->h) < (cy - 20)) \
                  continue; \
                  if ((obj->cur.geometry.y + y + par->y + ln->y) > (cy + ch + 20)) \
                  break; \
               } \
             EINA_INLIST_FOREACH(ln->items, itr) \
               { \
                  Evas_Coord yoff; \
                  yoff = ln->baseline; \
                  if (itr->format->valign != -1.0) \
                    { \
                       yoff += itr->format->valign * (ln->h - itr->h); \
                    } \
                  if (clip) \
                    { \
                       if ((obj->cur.geometry.x + x + ln->x + itr->x + itr->w) < (cx - 20)) \
                       continue; \
                       if ((obj->cur.geometry.x + x + ln->x + itr->x) > (cx + cw + 20)) \
                       break; \
                    } \
                  if ((ln->x + itr->x + itr->w) <= 0) continue; \
                  if (ln->x + itr->x > obj->cur.geometry.w) break; \
                  do

#define ITEM_WALK_END() \
                  while (0); \
               } \
          } \
     } \
   do {} while(0)
#define COLOR_SET(col) \
   ENFN->context_color_set(output, context, \
         (obj->cur.cache.clip.r * ti->parent.format->color.col.r) / 255, \
         (obj->cur.cache.clip.g * ti->parent.format->color.col.g) / 255, \
         (obj->cur.cache.clip.b * ti->parent.format->color.col.b) / 255, \
         (obj->cur.cache.clip.a * ti->parent.format->color.col.a) / 255);
#define COLOR_SET_AMUL(col, amul) \
   ENFN->context_color_set(output, context, \
         (obj->cur.cache.clip.r * ti->parent.format->color.col.r * (amul)) / 65025, \
         (obj->cur.cache.clip.g * ti->parent.format->color.col.g * (amul)) / 65025, \
         (obj->cur.cache.clip.b * ti->parent.format->color.col.b * (amul)) / 65025, \
         (obj->cur.cache.clip.a * ti->parent.format->color.col.a * (amul)) / 65025);
#define DRAW_TEXT(ox, oy) \
   if (ti->parent.format->font.font) ENFN->font_draw(output, context, surface, ti->parent.format->font.font, \
         obj->cur.geometry.x + ln->x + ti->parent.x + x + (ox), \
         obj->cur.geometry.y + ln->par->y + ln->y + yoff + y + (oy), \
         ti->parent.w, ti->parent.h, ti->parent.w, ti->parent.h, \
         &ti->text_props);

   /* backing */
#define DRAW_RECT(ox, oy, ow, oh, or, og, ob, oa) \
   do \
     { \
        ENFN->context_color_set(output, \
              context, \
              (obj->cur.cache.clip.r * or) / 255, \
              (obj->cur.cache.clip.g * og) / 255, \
              (obj->cur.cache.clip.b * ob) / 255, \
              (obj->cur.cache.clip.a * oa) / 255); \
        ENFN->rectangle_draw(output, \
              context, \
              surface, \
              obj->cur.geometry.x + ln->x + x + (ox), \
              obj->cur.geometry.y + ln->par->y + ln->y + y + (oy), \
              (ow), \
              (oh)); \
     } \
   while (0)

#define DRAW_FORMAT(oname, oy, oh, or, og, ob, oa) \
   do \
     { \
        if (itr->format->oname) \
          { \
             or = itr->format->color.oname.r; \
             og = itr->format->color.oname.g; \
             ob = itr->format->color.oname.b; \
             oa = itr->format->color.oname.a; \
             if (!EINA_INLIST_GET(itr)->next) \
               { \
                  DRAW_RECT(itr->x, oy, itr->w, oh, or, og, ob, oa); \
               } \
             else \
               { \
                  DRAW_RECT(itr->x, oy, itr->adv, oh, or, og, ob, oa); \
               } \
          } \
     } \
   while (0)

     {
        Evas_Coord look_for_y = 0 - (obj->cur.geometry.y + y);
        if (clip)
          {
             Evas_Coord tmp_lfy = cy - (obj->cur.geometry.y + y);
             if (tmp_lfy > look_for_y)
                look_for_y = tmp_lfy;
          }

        if (look_for_y >= 0)
           start = _layout_find_paragraph_by_y(o, look_for_y);

        if (!start)
           start = o->paragraphs;
     }

   ITEM_WALK()
     {
        DRAW_FORMAT(backing, 0, ln->h, r, g, b, a);
     }
   ITEM_WALK_END();

   /* There are size adjustments that depend on the styles drawn here back
    * in "_text_item_update_sizes" should not modify one without the other. */

   /* prepare everything for text draw */

   /* shadows */
   ITEM_WALK()
     {
        int shad_dst, shad_sz, dx, dy, haveshad;
        Evas_Object_Textblock_Text_Item *ti;
        ti = (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? _ITEM_TEXT(itr) : NULL;
        if (!ti) continue;

        shad_dst = shad_sz = dx = dy = haveshad = 0;
        switch (ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC)
          {
           case EVAS_TEXT_STYLE_SHADOW:
           case EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW:
             shad_dst = 1;
             haveshad = 1;
             break;
           case EVAS_TEXT_STYLE_OUTLINE_SHADOW:
           case EVAS_TEXT_STYLE_FAR_SHADOW:
             shad_dst = 2;
             haveshad = 1;
             break;
           case EVAS_TEXT_STYLE_FAR_SOFT_SHADOW:
             shad_dst = 2;
             shad_sz = 2;
             haveshad = 1;
             break;
           case EVAS_TEXT_STYLE_SOFT_SHADOW:
             shad_dst = 1;
             shad_sz = 2;
             haveshad = 1;
             break;
           default:
             break;
          }
        if (haveshad)
          {
             if (shad_dst > 0)
               {
                  switch (ti->parent.format->style & EVAS_TEXT_STYLE_MASK_SHADOW_DIRECTION)
                    {
                     case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT:
                       dx = 1;
                       dy = 1;
                       break;
                     case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM:
                       dx = 0;
                       dy = 1;
                       break;
                     case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT:
                       dx = -1;
                       dy = 1;
                       break;
                     case EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT:
                       dx = -1;
                       dy = 0;
                       break;
                     case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT:
                       dx = -1;
                       dy = -1;
                       break;
                     case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP:
                       dx = 0;
                       dy = -1;
                       break;
                     case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT:
                       dx = 1;
                       dy = -1;
                       break;
                     case EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT:
                       dx = 1;
                       dy = 0;
                     default:
                       break;
                    }
                  dx *= shad_dst;
                  dy *= shad_dst;
               }
             switch (shad_sz)
               {
                case 0:
                  COLOR_SET(shadow);
                  DRAW_TEXT(dx, dy);
                  break;
                case 2:
                  for (j = 0; j < 5; j++)
                    {
                       for (i = 0; i < 5; i++)
                         {
                            if (vals[i][j] != 0)
                              {
                                 COLOR_SET_AMUL(shadow, vals[i][j] * 50);
                                 DRAW_TEXT(i - 2 + dx, j - 2 + dy);
                              }
                         }
                    }
                  break;
                default:
                  break;
               }
          }
     }
   ITEM_WALK_END();

   /* glows */
   ITEM_WALK()
     {
        Evas_Object_Textblock_Text_Item *ti;
        ti = (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? _ITEM_TEXT(itr) : NULL;
        if (!ti) continue;

        if (ti->parent.format->style == EVAS_TEXT_STYLE_GLOW)
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
     }
   ITEM_WALK_END();

   /* outlines */
   ITEM_WALK()
     {
        Evas_Object_Textblock_Text_Item *ti;
        ti = (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? _ITEM_TEXT(itr) : NULL;
        if (!ti) continue;

        if ((ti->parent.format->style == EVAS_TEXT_STYLE_OUTLINE) ||
              (ti->parent.format->style == EVAS_TEXT_STYLE_OUTLINE_SHADOW) ||
              (ti->parent.format->style == EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW))
          {
             COLOR_SET(outline);
             DRAW_TEXT(-1, 0);
             DRAW_TEXT(1, 0);
             DRAW_TEXT(0, -1);
             DRAW_TEXT(0, 1);
          }
        else if (ti->parent.format->style == EVAS_TEXT_STYLE_SOFT_OUTLINE)
          {
             for (j = 0; j < 5; j++)
               {
                  for (i = 0; i < 5; i++)
                    {
                       if (((i != 2) || (j != 2)) && (vals[i][j] != 0))
                         {
                            COLOR_SET_AMUL(outline, vals[i][j] * 50);
                            DRAW_TEXT(i - 2, j - 2);
                         }
                    }
               }
          }
     }
   ITEM_WALK_END();

   /* normal text and lines */
   ITEM_WALK()
     {
        Evas_Object_Textblock_Text_Item *ti;
        ti = (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? _ITEM_TEXT(itr) : NULL;
        /* NORMAL TEXT */
        if (ti)
          {
             COLOR_SET(normal);
             DRAW_TEXT(0, 0);
          }

        /* STRIKETHROUGH */
        DRAW_FORMAT(strikethrough, (ln->h / 2), 1, r, g, b, a);

        /* UNDERLINE */
        DRAW_FORMAT(underline, ln->baseline + 1, 1, r2, g2, b2, a2);

        /* UNDERLINE2 */
        DRAW_FORMAT(underline2, ln->baseline + 3, 1, r3, g3, b3, a3);
     }
   ITEM_WALK_END();
}

static void
evas_object_textblock_render_pre(Evas_Object *obj)
{
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
   if ((o->changed) || (o->content_changed) || (o->format_changed) ||
       ((obj->cur.geometry.w != o->last_w) ||
           (((o->valign != 0.0) || (o->have_ellipsis)) &&
               (obj->cur.geometry.h != o->last_h))))
     {
        _relayout(obj);
	o->redraw = 0;
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	is_v = evas_object_is_visible(obj);
	was_v = evas_object_was_visible(obj);
	goto done;
     }
   if (o->redraw)
     {
	o->redraw = 0;
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
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
	evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes, obj, is_v, was_v);
	goto done;
     }
   if ((obj->cur.map != obj->prev.map) ||
       (obj->cur.usemap != obj->prev.usemap))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes, obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	goto done;
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur.geometry.x != obj->prev.geometry.x) ||
       (obj->cur.geometry.y != obj->prev.geometry.y) ||
       (obj->cur.geometry.w != obj->prev.geometry.w) ||
       (obj->cur.geometry.h != obj->prev.geometry.h))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	goto done;
     }
   done:
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes, obj, is_v, was_v);
}

static void
evas_object_textblock_render_post(Evas_Object *obj)
{
/*   Evas_Object_Textblock *o; */

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
/*   o = (Evas_Object_Textblock *)(obj->object_data); */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
/*   o->prev = o->cur; */
/*   o->changed = 0; */
}

static unsigned int evas_object_textblock_id_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_TEXTBLOCK;
}

static unsigned int evas_object_textblock_visual_id_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_CUSTOM;
}

static void *evas_object_textblock_engine_data_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   if (!o) return NULL;
   return o->engine_data;
}

static int
evas_object_textblock_is_opaque(Evas_Object *obj __UNUSED__)
{
   /* this returns 1 if the internal object data implies that the object is */
   /* currently fulyl opque over the entire gradient it occupies */
   return 0;
}

static int
evas_object_textblock_was_opaque(Evas_Object *obj __UNUSED__)
{
   /* this returns 1 if the internal object data implies that the object was */
   /* currently fulyl opque over the entire gradient it occupies */
   return 0;
}

static void
evas_object_textblock_coords_recalc(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   if ((obj->cur.geometry.w != o->last_w) ||
       (((o->valign != 0.0) || (o->have_ellipsis)) &&
           (obj->cur.geometry.h != o->last_h)))
     {
	o->formatted.valid = 0;
	o->changed = 1;
     }
}

static void
evas_object_textblock_scale_update(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   _evas_textblock_invalidate_all(o);
   _evas_textblock_changed(o, obj);
}

void
_evas_object_textblock_rehint(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Paragraph *par;
   Evas_Object_Textblock_Line *ln;

   o = (Evas_Object_Textblock *)(obj->object_data);
   EINA_INLIST_FOREACH(o->paragraphs, par)
     {
        EINA_INLIST_FOREACH(par->lines, ln)
          {
             Evas_Object_Textblock_Item *it;

             EINA_INLIST_FOREACH(ln->items, it)
               {
                  if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
                    {
                       Evas_Object_Textblock_Text_Item *ti = _ITEM_TEXT(it);
                       if (ti->parent.format->font.font)
                         {  
#ifdef EVAS_FRAME_QUEUING
                            evas_common_pipe_op_text_flush((RGBA_Font *) ti->parent.format->font.font);
#endif
                            evas_font_load_hinting_set(obj->layer->evas,
                                  ti->parent.format->font.font,
                                  obj->layer->evas->hinting);
                         }
                    }
               }
          }
     }
   _evas_textblock_invalidate_all(o);
   _evas_textblock_changed(o, obj);
}

/**
 * @}
 */

#ifdef HAVE_TESTS
/* return EINA_FALSE on error, used in unit_testing */
EAPI Eina_Bool
_evas_textblock_check_item_node_link(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Paragraph *par;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock_Item *it;

   o = (Evas_Object_Textblock *)(obj->object_data);
   if (!o) return EINA_FALSE;

   if (!o->formatted.valid) _relayout(obj);

   EINA_INLIST_FOREACH(o->paragraphs, par)
     {
        EINA_INLIST_FOREACH(par->lines, ln)
          {
             EINA_INLIST_FOREACH(ln->items, it)
               {
                  if (it->text_node != par->text_node)
                     return EINA_FALSE;
               }
          }
     }
   return EINA_TRUE;
}

EAPI int
_evas_textblock_format_offset_get(const Evas_Object_Textblock_Node_Format *n)
{
   return n->offset;
}
#endif

#if 0
/* Good for debugging */
void
pfnode(Evas_Object_Textblock_Node_Format *n)
{
   printf("Format Node: %p\n", n);
   printf("next = %p, prev = %p, last = %p\n", EINA_INLIST_GET(n)->next, EINA_INLIST_GET(n)->prev, EINA_INLIST_GET(n)->last);
   printf("text_node = %p, offset = %u, visible = %d\n", n->text_node, n->offset, n->visible);
   printf("'%s'\n", eina_strbuf_string_get(n->format));
}

void
ptnode(Evas_Object_Textblock_Node_Text *n)
{
   printf("Text Node: %p\n", n);
   printf("next = %p, prev = %p, last = %p\n", EINA_INLIST_GET(n)->next, EINA_INLIST_GET(n)->prev, EINA_INLIST_GET(n)->last);
   printf("format_node = %p\n", n->format_node);
   printf("'%ls'\n", eina_ustrbuf_string_get(n->unicode));
}

void
pitem(Evas_Object_Textblock_Item *it)
{
   Evas_Object_Textblock_Text_Item *ti;
   Evas_Object_Textblock_Format_Item *fi;
   printf("Item: %p\n", it);
   printf("Type: %s (%d)\n", (it->type == EVAS_TEXTBLOCK_ITEM_TEXT) ?
         "TEXT" : "FORMAT", it->type);
   printf("Text pos: %d Visual pos: %d\n", it->text_pos,
#ifdef BIDI_SUPPORT
         it->visual_pos
#else
         it->text_pos
#endif
         );
   printf("Coords: x = %d w = %d adv = %d\n", (int) it->x, (int) it->w,
         (int) it->adv);
   if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
     {
        ti = _ITEM_TEXT(it);
        printf("Text: '%*ls'\n", ti->text_props.text_len, GET_ITEM_TEXT(ti));
     }
   else
     {
        fi = _ITEM_FORMAT(it);
        printf("Format: '%s'\n", fi->item);
     }
}

void
ppar(Evas_Object_Textblock_Paragraph *par)
{
   Evas_Object_Textblock_Item *it;
   Eina_List *i;
   EINA_LIST_FOREACH(par->logical_items, i, it)
     {
        printf("***********************\n");
        pitem(it);
     }
}

#endif

