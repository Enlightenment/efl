/**
 * @addtogroup Evas_Object_Textblock
 *
 * @{
 */

/**
 * @section Evas_Object_Textblock_Tutorial Textblock Object Tutorial
 *
 * This part explains about the textblock object's API and proper usage.
 * If you want to develop textblock, you should also refer to @ref Evas_Object_Textblock_Internal.
 * The main user of the textblock object is the edje entry object in Edje, so
 * that's a good place to learn from, but I think this document is more than
 * enough, if it's not, please request for me info and I'll update it.
 *
 * @subsection textblock_intro Introduction
 * The textblock objects is, as implied, an object that can show big chunks of
 * text. Textblock supports many features including: Text formatting, automatic
 * and manual text alignment, embedding items (for example icons) and more.
 * Textblock has three important parts, the text paragraphs, the format nodes
 * and the cursors.
 *
 * @subsection textblock_cursors Textblock Object Cursors
 * A textblock Cursor @ref Evas_Textblock_Cursor is data type that represents
 * a position in a textblock. Each cursor contains information about the
 * paragraph it points to, the position in that paragraph and the object itself.
 * Cursors register to textblock objects upon creation, this means that once
 * you created a cursor, it belongs to a specific obj and you can't for example
 * copy a cursor "into" a cursor of a different object. Registered cursors
 * also have the added benefit of updating automatically upon textblock changes,
 * this means that if you have a cursor pointing to a specific character, it'll
 * still point to it even after you change the whole object completely (as long
 * as the char was not deleted), this is not possible without updating, because
 * as mentioned, each cursor holds a character position. There are many
 * functions that handle cursors, just check out the evas_textblock_cursor*
 * functions. For creation and deletion of cursors check out:
 * @see evas_object_textblock_cursor_new()
 * @see evas_textblock_cursor_free()
 * @note Cursors are generally the correct way to handle text in the textblock object, and there are enough functions to do everything you need with them (no need to get big chunks of text and processing them yourself).
 *
 * @subsection textblock_paragraphs Textblock Object Paragraphs
 * The textblock object is made out of text splitted to paragraphs (delimited
 * by the paragraph separation character). Each paragraph has many (or none)
 * format nodes associated with it which are responsible for the formatting
 * of that paragraph.
 *
 * @subsection textblock_format_nodes Textblock Object Format Nodes
 * As explained in @ref textblock_paragraphs each one of the format nodes
 * is associated with a paragraph.
 * There are two types of format nodes, visible and invisible:
 * Visible: formats that a cursor can point to, i.e formats that
 * occupy space, for example: newlines, tabs, items and etc. Some visible items
 * are made of two parts, in this case, only the opening tag is visible.
 * A closing tag (i.e a </tag> tag) should NEVER be visible.
 * Invisible: formats that don't occupy space, for example: bold and underline.
 * Being able to access format nodes is very important for some uses. For
 * example, edje uses the "<a>" format to create links in the text (and pop
 * popups above them when clicked). For the textblock object a is just a
 * formatting instruction (how to color the text), but edje utilizes the access
 * to the format nodes to make it do more.
 * For more information, take a look at all the evas_textblock_node_format_*
 * functions.
 * The translation of "<tag>" tags to actual format is done according to the
 * tags defined in the style, see @ref evas_textblock_style_set
 *
 * @subsection textblock_special_formats Special Formats
 * This section is not yet written. If you want some info about styles/formats
 * and how to use them, expedite's textblock_basic test is a great start.
 * @todo Write @textblock_special_formats
 */

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

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

/* private magic number for textblock objects */
static const char o_type[] = "textblock";

/* The char to be inserted instead of visible formats */
#define EVAS_TEXTBLOCK_REPLACEMENT_CHAR 0xFFFC

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

/* the current state of the formatting */
/**
 * @internal
 * @def GET_PREV(text, ind)
 * Gets the index of the previous char in the text text, this simply returns
 * the current char pointed to and decrements ind but ensures it stays in
 * the text range.
 */
#define GET_PREV(text, ind) (text ? (((ind) > 0) ? (text[(ind)--]) : (text[ind])) : 0)
/**
 * @internal
 * @def GET_NEXT(text, ind)
 * Gets the index of the next in the text text, this simply returns
 * the current char pointed to and increments indd but ensures it stays in
 * the text range.
 */
#define GET_NEXT(text, ind) (text ? ((text[ind]) ? (text[(ind)++]) : (text[ind])) : 0)

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
   Evas_BiDi_Paragraph_Props          *bidi_props;
   Eina_Bool                           dirty : 1;
};

struct _Evas_Object_Textblock_Node_Format
{
   EINA_INLIST;
   Eina_Strbuf                        *format;
   Evas_Object_Textblock_Node_Text    *text_node;
   size_t                              offset;
   Eina_Bool                           visible;
};

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
   Evas_Object_Textblock_Line        *lines;
   Evas_Object_Textblock_Node_Text   *text_node;
   Eina_List                         *logical_items;
   int                                x, y, w, h;
   int                                line_no;
};

struct _Evas_Object_Textblock_Line
{
   EINA_INLIST;
   Evas_Object_Textblock_Item        *items;
   Evas_Object_Textblock_Paragraph   *par;
   Evas_Object_Textblock_Text_Item   *ellip_ti;
   int                                x, y, w, h;
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
   Eina_Unicode                    *text;
   Evas_Text_Props                  text_props;
   int                              inset, baseline;
   Evas_Coord                       x_adjustment; /* Used to indicate by how
                                                     much we adjusted sizes */
};

struct _Evas_Object_Textblock_Format_Item
{
   Evas_Object_Textblock_Item           parent;
   Evas_BiDi_Direction                  bidi_dir;
   const char                          *item;
   Evas_Object_Textblock_Node_Format   *source_node;
   int                                  y, ascent, descent;
   unsigned char                        vsize : 2;
   unsigned char                        size : 2;
   unsigned char                        formatme : 1;
};

struct _Evas_Object_Textblock_Format
{
   int                  ref;
   double               halign;
   Eina_Bool            halign_auto;
   double               valign;
   struct {
      const char       *name;
      const char       *source;
      const char       *fallbacks;
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
   int                  linesize;
   double               linerelsize;
   int                  linegap;
   double               linerelgap;
   double               linefill;
   double               ellipsis;
   unsigned char        style;
   unsigned char        wrap_word : 1;
   unsigned char        wrap_char : 1;
   unsigned char        wrap_mixed : 1;
   unsigned char        underline : 1;
   unsigned char        underline2 : 1;
   unsigned char        strikethrough : 1;
   unsigned char        backing : 1;
};

struct _Evas_Textblock_Style
{
   const char            *style_text;
   char                  *default_tag;
   Evas_Object_Style_Tag *tags;
   Eina_List             *objects;
   unsigned char          delete_me : 1;
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
   int                                 last_w, last_h;
   struct {
      int                              l, r, t, b;
   } style_pad;
   double                              valign;
   char                               *markup_text;
   void                               *engine_data;
   const char                         *repch;
   struct {
      int                              w, h;
      unsigned char                    valid : 1;
   } formatted, native;
   unsigned char                       redraw : 1;
   unsigned char                       changed : 1;
   unsigned char                       content_changed : 1;
   unsigned char                       have_ellipsis : 1;
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
static Eina_Bool _evas_textblock_format_is_visible(const char *s);
static void _evas_textblock_node_format_remove(Evas_Object_Textblock *o, Evas_Object_Textblock_Node_Format *n, int visual_adjustment);
static void _evas_textblock_node_format_free(Evas_Object_Textblock_Node_Format *n);
static void _evas_textblock_node_text_free(Evas_Object_Textblock_Node_Text *n);
static void _evas_textblock_text_node_changed(Evas_Object_Textblock *o, Evas_Object *obj, Evas_Object_Textblock_Node_Text *n);
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
 * Searches inside the tags stored in the style for the tag who's
 * replacement is s of size replace_len;
 * @param ts The ts to be cleared. Must not be NULL.
 * @param s The replace string to match.
 * @param replace_len the length of the replace string.
 * @param[out] tag_len The length of the tag found. - Must not be NULL.
 * @return The tag found.
 */
static inline const char *
_style_match_replace(Evas_Textblock_Style *ts, const char *s, size_t replace_len, size_t *tag_len)
{
   Evas_Object_Style_Tag *tag;

   EINA_INLIST_FOREACH(ts->tags, tag)
     {
	if (tag->replace_len != replace_len) continue;
	if (!strcmp(tag->replace, s))
	  {
	     *tag_len = tag->tag_len;
	     return tag->tag;
	  }
     }
   *tag_len = 0;
   return NULL;
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
        _evas_textblock_node_format_free(n);
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
        if (ti->text) free(ti->text);
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
_line_free(const Evas_Object *obj, Evas_Object_Textblock_Line *ln)
{
   /* Items are freed from the logical list, except for the ellip item */
   if (ln->ellip_ti) _item_free(obj, NULL, _ITEM(ln->ellip_ti));
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
" \0"          "\x20\0" /* NOTE: this here to avoid escaping to &nbsp */
"&nbsp;\0"     "\x20\0" /* NOTE: we allow nsbp's to break as we map early - maybe map to ascii 0x01 and then make the rendering code think 0x01 -> 0x20 */
"&quot;\0"     "\x22\0"
"&amp;\0"      "\x26\0"
"&lt;\0"       "\x3c\0"
"&gt;\0"       "\x3e\0"
/* all the rest */
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
 * Creates a copy of a string without the whitespaces between byte indexes
 * clean_start and clean_end and frees str.
 *
 * FIXME: BROKEN FOR NON-ENGLISH.
 * @param clean_start The byte index of the location to start cleaning at.
 * @param clean_end The byte index of the location to end cleaning at.
 * @param str The string to copy and free.
 * @return a copy of str cleaned of whitespaces.
 */
static char *
_clean_white(int clean_start, int clean_end, char *str)
{
   char *p, *p2, *str2 = NULL;
   int white, pwhite, start, ok;

   /*FIXME: fix this function */
   return strdup(str);
   str2 = malloc(strlen(str) + 2);
   p = str;
   p2 = str2;
   white = 0;
   start = 1;
   ok = 1;
   while (*p != 0)
     {
        pwhite = white;
        if (_is_white(*p)) white = 1;
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

/**
 * @internal
 * Appends the text between s and p to the main cursor of the object.
 *
 * @param o The textblock to append to.
 * @param[in] s start of the string
 * @param[in] p end of the string
 */
static void __UNUSED__
_append_text_run(Evas_Object_Textblock *o, const char *s, const char *p)
{
   if ((s) && (p > s))
     {
        char *ts;

        ts = alloca(p - s + 1);
        strncpy(ts, s, p - s);
        ts[p - s] = 0;
        ts = _clean_white(0, 0, ts);
        evas_textblock_cursor_text_append(o->cursor, ts);
        free(ts);
     }
}

/**
 * @internal
 * Prepends the text between s and p to the main cursor of the object.
 *
 * @param o The textblock to prepend to.
 * @param[in] s start of the string
 * @param[in] p end of the string
 */
static void
_prepend_text_run(Evas_Object_Textblock *o, const char *s, const char *p)
{
   if ((s) && (p > s))
     {
        char *ts;

        ts = alloca(p - s + 1);
        strncpy(ts, s, p - s);
        ts[p - s] = 0;
        ts = _clean_white(0, 0, ts);
        evas_textblock_cursor_text_prepend(o->cursor, ts);
        free(ts);
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
   char *tmp_param;

   tmp_param = alloca(strlen(param) + 1);

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
        if (!strcmp(tmp_param, "off")) fmt->style = EVAS_TEXT_STYLE_PLAIN;
        else if (!strcmp(tmp_param, "none")) fmt->style = EVAS_TEXT_STYLE_PLAIN;
        else if (!strcmp(tmp_param, "plain")) fmt->style = EVAS_TEXT_STYLE_PLAIN;
        else if (!strcmp(tmp_param, "shadow")) fmt->style = EVAS_TEXT_STYLE_SHADOW;
        else if (!strcmp(tmp_param, "outline")) fmt->style = EVAS_TEXT_STYLE_OUTLINE;
        else if (!strcmp(tmp_param, "soft_outline")) fmt->style = EVAS_TEXT_STYLE_SOFT_OUTLINE;
        else if (!strcmp(tmp_param, "outline_shadow")) fmt->style = EVAS_TEXT_STYLE_OUTLINE_SHADOW;
        else if (!strcmp(tmp_param, "outline_soft_shadow")) fmt->style = EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW;
        else if (!strcmp(tmp_param, "glow")) fmt->style = EVAS_TEXT_STYLE_GLOW;
        else if (!strcmp(tmp_param, "far_shadow")) fmt->style = EVAS_TEXT_STYLE_FAR_SHADOW;
        else if (!strcmp(tmp_param, "soft_shadow")) fmt->style = EVAS_TEXT_STYLE_SOFT_SHADOW;
        else if (!strcmp(tmp_param, "far_soft_shadow")) fmt->style = EVAS_TEXT_STYLE_FAR_SOFT_SHADOW;
        else fmt->style = EVAS_TEXT_STYLE_PLAIN;
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

   int x, y;
   int w, h;
   int wmax, hmax;
   int maxascent, maxdescent;
   int marginl, marginr;
   int line_no;
   int underline_extend;
   int have_underline, have_underline2;
   double align, valign;
   Eina_Bool align_auto;
};

static void _layout_text_add_logical_item(Ctxt *c, Evas_Object_Textblock_Text_Item *ti, Eina_List *rel);
static void _text_item_update_sizes(Ctxt *c, Evas_Object_Textblock_Text_Item *ti);
/**
 * @internal
 * Adjust the ascent/descent of the format and context.
 *
 * @param c The context to work on - Not NUL.
 * @param fmt The format to adjust - NOT NULL.
 */
static void
_layout_format_ascent_descent_adjust(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   int ascent, descent;

   if (fmt->font.font)
     {
        //	ascent = c->ENFN->font_max_ascent_get(c->ENDT, fmt->font.font);
        //	descent = c->ENFN->font_max_descent_get(c->ENDT, fmt->font.font);
        ascent = c->ENFN->font_ascent_get(c->ENDT, fmt->font.font);
        descent = c->ENFN->font_descent_get(c->ENDT, fmt->font.font);
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
        c->maxdescent += fmt->linegap;
        c->maxdescent += ((ascent + descent) * fmt->linerelgap);
        if (c->maxascent < ascent) c->maxascent = ascent;
        if (c->maxdescent < descent) c->maxdescent = descent;
        if (fmt->linefill > 0.0)
          {
             int dh;

             dh = c->obj->cur.geometry.h - (c->maxascent + c->maxdescent);
             if (dh < 0) dh = 0;
             dh = fmt->linefill * dh;
             c->maxdescent += dh / 2;
             c->maxascent += dh - (dh / 2);
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
   _layout_format_ascent_descent_adjust(c, fmt);
}

/**
 * @internal
 * Create a new layout paragraph.
 *
 * @param c The context to work on - Not NULL.
 */
static void
_layout_paragraph_new(Ctxt *c, Evas_Object_Textblock_Node_Text *n)
{
   c->par = calloc(1, sizeof(Evas_Object_Textblock_Paragraph));
   c->paragraphs = (Evas_Object_Textblock_Paragraph *)eina_inlist_append(EINA_INLIST_GET(c->paragraphs), EINA_INLIST_GET(c->par));
   c->ln = NULL;
   c->par->text_node = n;
   c->par->line_no = -1;
}

/**
 * @internal
 * Free the visual lines in the paragraph (logical items are kept)
 */
static void
_paragraph_clear(const Evas_Object *obj, Evas_Object_Textblock_Paragraph *par)
{
   while (par->lines)
     {
        Evas_Object_Textblock_Line *ln;

        ln = (Evas_Object_Textblock_Line *) par->lines;
        par->lines = (Evas_Object_Textblock_Line *)eina_inlist_remove(EINA_INLIST_GET(par->lines), EINA_INLIST_GET(par->lines));
        _line_free(obj, ln);
     }
   par->line_no = -1;
}

/**
 * @internal
 * Free the layout paragraph and all of it's lines and logical items.
 */
static void
_paragraph_free(const Evas_Object *obj, Evas_Object_Textblock_Paragraph *par)
{
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
_layout_format_push(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   if (fmt)
     {
        fmt = _format_dup(c->obj, fmt);
        c->format_stack  = eina_list_prepend(c->format_stack, fmt);
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
     }
   return fmt;
}

/**
 * @internal
 * Pop fmt to the format stack, if there's something in the stack free fmt
 * and set it to point to the next item instead, else return fmt.
 *
 * @param c the context to work on - Not NULL.
 * @param fmt the format to free.
 * @return the next format in the stack, or format if there's none.
 * @see _layout_format_push()
 */
static Evas_Object_Textblock_Format *
_layout_format_pop(Ctxt *c, Evas_Object_Textblock_Format *fmt)
{
   if ((c->format_stack) && (c->format_stack->next))
     {
        _format_unref_free(c->obj, fmt);
        c->format_stack = eina_list_remove_list(c->format_stack, c->format_stack);
        fmt = c->format_stack->data;
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
              EVAS_BIDI_PARAGRAPH_DIRECTION_IS_RTL(
                 c->ln->items->text_node->bidi_props))
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

/**
 * @internal
 * Reorder the items in visual order
 *
 * @param line the line to reorder
 */
static void
_layout_line_order(Ctxt *c __UNUSED__, Evas_Object_Textblock_Line *line)
{
   /*FIXME: do it a bit more efficient - not very efficient ATM. */
#ifdef BIDI_SUPPORT
   Evas_Object_Textblock_Item *it;
   EvasBiDiStrIndex *v_to_l = NULL;
   size_t start, end;
   size_t len;

   if (line->items && line->items->text_node &&
         line->items->text_node->bidi_props)
     {
        Evas_BiDi_Paragraph_Props *props;
        props = line->items->text_node->bidi_props;
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
#else
   line = NULL;
#endif
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
   Eina_Bool no_text = EINA_TRUE;
   Evas_Coord x = 0;

   _layout_line_order(c, c->ln);

   c->maxascent = c->maxdescent = 0;
   EINA_INLIST_FOREACH(c->ln->items, it)
     {
        if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
          {
             no_text = EINA_FALSE;
             break;
          }
     }

   if (no_text)
     _layout_format_ascent_descent_adjust(c, fmt);

   EINA_INLIST_FOREACH(c->ln->items, it)
     {
        if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
          {
             Evas_Object_Textblock_Text_Item *ti = _ITEM_TEXT(it);
             if (ti->parent.format->font.font)
               ti->baseline = c->ENFN->font_max_ascent_get(c->ENDT, ti->parent.format->font.font);
             _layout_format_ascent_descent_adjust(c, ti->parent.format);
          }
        else
          {
             Evas_Object_Textblock_Format_Item *fi = _ITEM_FORMAT(it);
             if (!fi->formatme) goto loop_advance;
             fi->ascent = c->maxascent;
             fi->descent = c->maxdescent;
             /* Adjust sizes according to current line height/scale */
               {
                  Evas_Coord w, h;
                  const char *p, *s;

                  s = eina_strbuf_string_get(fi->source_node->format);
                  w = fi->parent.w;
                  h = fi->parent.h;
                  switch (fi->size)
                    {
                     case SIZE:
                        if (!strncmp(s, "item", 4))
                          {
                             p = strstr(s, " size=");
                             if (p)
                               {
                                  p += 6;
                                  if (sscanf(p, "%ix%i", &w, &h) == 2)
                                    {
                                       w = w * c->obj->cur.scale;
                                       h = h * c->obj->cur.scale;
                                    }
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
                                  sz = c->maxdescent + c->maxascent;
                               }
                             else if (fi->vsize == VSIZE_ASCENT)
                               {
                                  sz = c->maxascent;
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
                  fi->parent.w = fi->parent.adv = w;
                  fi->parent.h = h;
               }

             switch (fi->size)
               {
                case SIZE:
                case SIZE_ABS:
                   switch (fi->vsize)
                     {
                      case VSIZE_FULL:
                         if (fi->parent.h > (c->maxdescent + c->maxascent))
                           {
                              c->maxascent += fi->parent.h - (c->maxdescent + c->maxascent);
                              fi->y = -c->maxascent;
                           }
                         else
                           fi->y = -(fi->parent.h - c->maxdescent);
                         break;
                      case VSIZE_ASCENT:
                         if (fi->parent.h > c->maxascent)
                           {
                              c->maxascent = fi->parent.h;
                              fi->y = -fi->parent.h;
                           }
                         else
                           fi->y = -fi->parent.h;
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
                         fi->y = -fi->ascent;
                         break;
                      default:
                         break;
                     }
                   break;
                default:
                   break;
               }
          }

loop_advance:
        it->x = x;
        x += it->adv;

        if ((it->x + it->w) > c->ln->w) c->ln->w = it->x + it->w;
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
   /* Update the paragraphs line number. */
   if (c->par->line_no == -1)
     {
        c->par->line_no = c->line_no;
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
        if ((c->par->x + c->ln->x + c->ln->w + c->marginr - c->o->style_pad.l) > c->wmax)
          c->wmax = c->par->x + c->ln->x + c->ln->w + c->marginl + c->marginr - c->o->style_pad.l;
     }
   else
     {
        c->ln->x = c->marginl + c->o->style_pad.l;
        if ((c->par->x + c->ln->x + c->ln->w + c->marginr - c->o->style_pad.l) > c->wmax)
          c->wmax = c->par->x + c->ln->x + c->ln->w + c->marginl + c->marginr - c->o->style_pad.l;
     }
   c->par->h = c->ln->y + c->ln->h;
   if (c->ln->w + c->ln->x > c->par->w)
     c->par->w = c->ln->x + c->ln->w;
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
_layout_text_item_new(Ctxt *c __UNUSED__, Evas_Object_Textblock_Format *fmt, const Eina_Unicode *str, size_t len)
{
   Evas_Object_Textblock_Text_Item *ti;

   ti = calloc(1, sizeof(Evas_Object_Textblock_Text_Item));
   ti->parent.format = fmt;
   ti->parent.format->ref++;
   ti->text = eina_unicode_strndup(str, len);
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
        return c->ENFN->font_last_up_to_pos(c->ENDT, fmt->font.font, ti->text,
              &ti->text_props, x, 0);
     }
   return -1;
}

/**
 * @internal
 * Cut the text up until cut and split
 *
 * @param c the context to work on - Not NULL.
 * @param ti the item to cut - not null.
 * @param lti the logical list item of the item.
 * @param cut the cut index.
 * @return the second (newly created) item.
 */
static Evas_Object_Textblock_Text_Item *
_layout_item_text_split_strip_white(Ctxt *c,
      Evas_Object_Textblock_Text_Item *ti, Eina_List *lti, int cut)
{
   Eina_Unicode *ts;
   Evas_Object_Textblock_Text_Item *new_ti = NULL, *white_ti = NULL;
   int cut2;

   ts = ti->text;
   if (_is_white(ts[cut]))
     cut2 = cut + 1;
   else
     cut2 = cut;

   if (ts[cut2] && (ti->text_props.text_len > 0))
     {
        new_ti = _layout_text_item_new(c, ti->parent.format, &ts[cut2],
                                       ti->text_props.text_len - cut2);
        new_ti->parent.text_node = ti->parent.text_node;
        new_ti->parent.text_pos = ti->parent.text_pos + cut2;
        new_ti->parent.merge = EINA_TRUE;
        ts[cut2] = 0;

        evas_common_text_props_split(&ti->text_props,
                                     &new_ti->text_props, cut2);
        _layout_text_add_logical_item(c, new_ti, lti);
     }

   if ((cut2 > cut) && (ti->text_props.text_len > 0))
     {
        white_ti = _layout_text_item_new(c, ti->parent.format, &ts[cut],
                                       ti->text_props.text_len - cut);
        white_ti->parent.text_node = ti->parent.text_node;
        white_ti->parent.text_pos = ti->parent.text_pos + cut;
        white_ti->parent.merge = EINA_TRUE;
        white_ti->parent.visually_deleted = EINA_TRUE;
        ts[cut] = 0;

        evas_common_text_props_split(&ti->text_props,
              &white_ti->text_props, cut);
        _layout_text_add_logical_item(c, white_ti, lti);
     }

   if (new_ti || white_ti)
     {
        _text_item_update_sizes(c, ti);

        ti->text = eina_unicode_strndup(ts, cut);
        free(ts);
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
   Eina_Unicode *tmp;
   size_t len1, len2;
   len1 = item1->text_props.text_len;
   len2 = item2->text_props.text_len;
   evas_common_text_props_merge(&item1->text_props,
         &item2->text_props);

   item1->parent.w = item1->parent.adv + item2->parent.w;
   item1->parent.adv += item2->parent.adv;

   tmp = realloc(item1->text, (len1 + len2 + 1) * sizeof(Eina_Unicode));
   eina_unicode_strncpy(tmp + len1, item2->text, len2);
   item1->text = tmp;
   item1->text[len1 + len2] = 0;

   item1->parent.merge = EINA_FALSE;
   item1->parent.visually_deleted = EINA_FALSE;

   _item_free(c->obj, NULL, _ITEM(item2));
}

/**
 * @internal
 * Return the start of the last word up until start.
 *
 * @param str the string to work on.
 * @param start the start of where to look at.
 * @return the start of the last word up until start.
 */
static int
_layout_word_start(const Eina_Unicode *str, int start)
{
   int p, tp, chr = 0;

   p = start;
   chr = GET_NEXT(str, p);
   if (_is_white(chr))
     {
        tp = p;
        while (_is_white(chr) && (p >= 0))
          {
             tp = p;
             chr = GET_NEXT(str, p);
          }
        return tp;
     }
   p = start;
   tp = p;
   while (p > 0)
     {
        chr = GET_PREV(str, p);
        if (_is_white(chr)) break;
        tp = p;
     }
   if (p < 0) p = 0;
   if ((p >= 0) && (_is_white(chr)))
     {
        GET_NEXT(str, p);
     }
   return p;
}

/**
 * @internal
 * returns the index of the words end starting from p
 *
 * @param str the str to work on - NOT NULL.
 * @param p start position - must be within strings range..
 *
 * @return the position of the end of the word. -1 on error.
 */
static int
_layout_word_end(const Eina_Unicode *str, int p)
{
   int ch, tp;

   tp = p;
   ch = GET_NEXT(str, tp);
   while ((!_is_white(ch)) && (tp >= 0) && (ch != 0))
     {
        p = tp;
        ch = GET_NEXT(str, tp);
     }
   if (ch == 0) return -1;
   return p;
}

/**
 * @internal
 * returns the index of the start of the next word.
 *
 * @param str the str to work on - NOT NULL.
 * @param p start position - must be within strings range..
 *
 * @return the position of the start of the next word. -1 on error.
 */
static int
_layout_word_next(Eina_Unicode *str, int p)
{
   int ch, tp;

   tp = p;
   ch = GET_NEXT(str, tp);
   while ((!_is_white(ch)) && (tp >= 0) && (ch != 0))
     {
        p = tp;
        ch = GET_NEXT(str, tp);
     }
   if (ch == 0) return -1;
   while ((_is_white(ch)) && (tp >= 0) && (ch != 0))
     {
        p = tp;
        ch = GET_NEXT(str, tp);
     }
   if (ch == 0) return -1;
   return p;
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
   int tw, th, inset, right_inset;
   const Evas_Object_Textblock_Format *fmt = ti->parent.format;

   tw = th = 0;
   if (fmt->font.font)
     c->ENFN->font_string_size_get(c->ENDT, fmt->font.font, ti->text,
           &ti->text_props, &tw, &th);
   inset = 0;
   if (fmt->font.font)
     inset = c->ENFN->font_inset_get(c->ENDT, fmt->font.font,
           &ti->text_props);
   right_inset = 0;
   if (fmt->font.font)
      right_inset = c->ENFN->font_right_inset_get(c->ENDT, fmt->font.font,
            &ti->text_props);

   /* These adjustments are calculated and thus heavily linked to those in
    * textblock_render!!! Don't change one without the other. */
   switch (ti->parent.format->style)
     {
        case EVAS_TEXT_STYLE_SHADOW:
           ti->x_adjustment = 1;
           break;
        case EVAS_TEXT_STYLE_OUTLINE_SHADOW:
        case EVAS_TEXT_STYLE_FAR_SHADOW:
           ti->x_adjustment = 2;
           break;
        case EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW:
        case EVAS_TEXT_STYLE_FAR_SOFT_SHADOW:
           ti->x_adjustment = 4;
           break;
        case EVAS_TEXT_STYLE_SOFT_SHADOW:
           inset += 1;
           ti->x_adjustment = 4;
           break;
        case EVAS_TEXT_STYLE_GLOW:
        case EVAS_TEXT_STYLE_SOFT_OUTLINE:
           inset += 2;
           ti->x_adjustment = 4;
           break;
        case EVAS_TEXT_STYLE_OUTLINE:
           inset += 1;
           ti->x_adjustment = 1;
           break;
        default:
           break;
     }
   ti->inset = inset;
   ti->parent.w = tw + ti->x_adjustment;
   ti->parent.h = th;
   ti->parent.adv = tw + right_inset;
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
   Eina_Unicode *alloc_str = NULL;
   const Eina_Unicode *str = EINA_UNICODE_EMPTY_STRING;
   const Eina_Unicode *tbase;
   Evas_Object_Textblock_Text_Item *ti;
   size_t cur_len = 0;

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
        if ((repch) && (eina_ustrbuf_length_get(n->unicode)))
          {
             int i, ind;
             Eina_Unicode *ptr;
             Eina_Unicode urepch;

             str = alloca((off + 1) * sizeof(Eina_Unicode));
             tbase = str;
             ind = 0;
             urepch = eina_unicode_utf8_get_next(repch, &ind);
             for (i = 0, ptr = (Eina_Unicode *)tbase; i < off; ptr++, i++)
               *ptr = urepch;
             *ptr = 0;
          }
        /* Use the string, just cut the relevant parts */
        else
          {
             str = eina_ustrbuf_string_get(n->unicode);
             alloc_str = eina_unicode_strndup(str + start, off);
             str = alloc_str;
          }

        cur_len = off;
     }

skip:
   tbase = str;
   new_line = 0;
   empty_item = 0;


   while (str)
     {
        int tmp_len;

        ti = _layout_text_item_new(c, fmt, str, cur_len);
        ti->parent.text_node = n;
        ti->parent.text_pos = start + str - tbase;
        tmp_len = off - (str - tbase);
        if (ti->parent.text_node)
          {
             int tmp_cut;
             tmp_cut = evas_common_language_script_end_of_run_get(
                   ti->text,
                   ti->parent.text_node->bidi_props,
                   ti->parent.text_pos, tmp_len);
             if (tmp_cut > 0)
               {
                  Eina_Unicode *ts;

                  ts = ti->text;
                  ts[tmp_cut] = 0;
                  ti->text = eina_unicode_strndup(ts, tmp_cut);
                  free(ts);
                  tmp_len = tmp_cut;
               }
             evas_common_text_props_bidi_set(&ti->text_props,
                   ti->parent.text_node->bidi_props, ti->parent.text_pos);
             evas_common_text_props_script_set (&ti->text_props,
                   ti->text);
             if (ti->parent.format->font.font)
               {
                  c->ENFN->font_text_props_info_create(c->ENDT,
                        ti->parent.format->font.font,
                        ti->text, &ti->text_props,
                        ti->parent.text_node->bidi_props,
                        ti->parent.text_pos, tmp_len);
               }
          }
        str += tmp_len;
        cur_len -= tmp_len;

        _layout_text_add_logical_item(c, ti, NULL);

        /* Break if we reached the end. */
        if (!*str)
          break;
     }

   if (alloc_str) free(alloc_str);
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
   fi->source_node = n;
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
              fi->parent.text_node->bidi_props,
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
 */
static void
_layout_do_format(const Evas_Object *obj __UNUSED__, Ctxt *c,
      Evas_Object_Textblock_Format **_fmt, Evas_Object_Textblock_Node_Format *n,
      int *style_pad_l, int *style_pad_r, int *style_pad_t, int *style_pad_b)
{
   Evas_Object_Textblock_Format *fmt = *_fmt;
   /* FIXME: comment the algo */

   const char *s;
   const char *item;
   int handled = 0;

   s = eina_strbuf_string_get(n->format);
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

        fi = _layout_format_item_add(c, n, NULL, fmt);
        fi->vsize = vsize;
        fi->size = size;
        fi->formatme = 1;
        fi->parent.w = fi->parent.adv = w; /* For formats items it's usually
                                              the same, we don't handle the
                                              special cases yet. */
        fi->parent.h = h;
        handled = 1;
     }
   if (!handled)
     {
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
             char *tmp = alloca(s - item + 1);
             strncpy(tmp, item, s - item);
             tmp[s - item] = '\0';
             if (_format_is_param(item))
               {
                  _layout_format_value_handle(c, fmt, item);
               }
             else
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

   evas_text_style_pad_get(fmt->style, style_pad_l, style_pad_r, style_pad_t, style_pad_b);

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
}

/* -1 means no wrap */
static int
_layout_get_charwrap(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Text_Item *ti)
{
   int wrap;

   wrap = _layout_text_cutoff_get(c, fmt, ti);
   if (wrap == 0)
     GET_NEXT(ti->text, wrap);
   if (!ti->text[wrap])
     wrap = -1;

   return wrap;
}

/* -1 means no wrap */
static int
_layout_get_word_mixwrap_common(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Text_Item *ti, Eina_Bool mixed_wrap)
{
   int wrap = -1, twrap;
   int orig_wrap;
   Eina_Unicode ch;
   const Eina_Unicode *str = ti->text;

   wrap = _layout_text_cutoff_get(c, fmt, ti);
   /* Avoiding too small textblocks to even contain one char */
   if (wrap == 0)
     GET_NEXT(str, wrap);
   orig_wrap = wrap;
   /* We need to wrap and found the position that overflows */
   if (wrap > 0)
     {
        int index = wrap;
        ch = GET_NEXT(str, index);
        if (!_is_white(ch))
          wrap = _layout_word_start(str, wrap);
        /* If we found where to cut the text at, i.e the start
         * of the word we were pointing at */
        if (wrap > 0)
          {
             twrap = wrap;
             ch = GET_PREV(str, twrap);
             /* the text intersects the wrap point on a whitespace char */
             if (_is_white(ch))
               {
                  return wrap;
               }
             /* intersects a word */
             else
               {
                  /* walk back to start of word */
                  twrap = _layout_word_start(str, wrap);
                  if (twrap != 0)
                    {
                       wrap = twrap;
                       ch = GET_PREV(str, wrap);
                       return (str[wrap]) ? wrap : -1;
                    }
               }
          }
        /* If we weren't able to find the start of the word we
         * are currently pointing at, or we were able but it's
         * the first word - the end of this word is the wrap point, o */
        else
          {
             /* wrap now is the index of the word START */
             index = wrap;
             ch = GET_NEXT(str, index);

             /* If there are already items in this line, we
              * should just try creating a new line for it */
             if (c->ln->items)
               {
                  return 0;
               }
             /* If there were no items in this line, try to do
              * our best wrapping possible since it's the middle
              * of the word */
             else
               {
                  if (mixed_wrap)
                    {
                       return (str[orig_wrap]) ? orig_wrap : -1;
                    }
                  else
                    {
                       wrap = 0;
                       twrap = _layout_word_end(ti->text, wrap);
                       wrap = twrap;
                       if (twrap >= 0)
                         {
                            ch = GET_NEXT(str, wrap);
                            return (str[wrap]) ? wrap : -1;
                         }
                       else
                         {
                            return -1;
                         }
                    }
               }
          }
     }
   /* We need to wrap, but for some reason we failed obatining the
    * overflow position. */
   else
     {
        /*FIXME: sanitize this error handling - should probably
         * never get here anyway unless something really bad
         * has happend */
        /* wrap now is the index of the word START */
        if (wrap < 0) wrap = 0;

        /* If there are already items in the line, break before. */
        if (c->ln->items)
          {
             return 0;
          }
        else
          {
             twrap = _layout_word_end(ti->text, wrap);
             wrap = _layout_word_next(ti->text, wrap);
             if (wrap >= 0)
               {
                  ch = GET_NEXT(str, wrap);
                  return (str[wrap]) ? wrap : -1;
               }
             else if (twrap >= 0)
               {
                  ch = GET_NEXT(str, twrap);
                  return (str[twrap]) ? twrap : -1;
               }
          }
     }
   return -1;
}

/* -1 means no wrap */
static int
_layout_get_wordwrap(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Text_Item *ti)
{
   return _layout_get_word_mixwrap_common(c, fmt, ti, EINA_FALSE);
}

/* -1 means no wrap */
static int
_layout_get_mixedwrap(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Text_Item *ti)
{
   return _layout_get_word_mixwrap_common(c, fmt, ti, EINA_TRUE);
}

static Evas_Object_Textblock_Text_Item *
_layout_ellipsis_item_new(Ctxt *c, const Evas_Object_Textblock_Item *cur_it)
{
   Evas_Object_Textblock_Text_Item *ellip_ti;
   const Eina_Unicode _ellip_str[2] = { 0x2026, '\0' }; /* Ellipsis char */
   size_t len = 1; /* The length of _ellip_str */
   /* We assume that the format stack has at least one time,
    * the only reason it may not have, is more </> than <>, other
    * than that, we're safe. The last item is the base format. */
   ellip_ti = _layout_text_item_new(c,
         eina_list_data_get(eina_list_last(c->format_stack)),
         _ellip_str, len);
   ellip_ti->parent.text_node = cur_it->text_node;
   ellip_ti->parent.text_pos = cur_it->text_pos;
   if (cur_it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
     {
        ellip_ti->parent.text_pos += _ITEM_TEXT(cur_it)->text_props.text_len;
     }
   else
     {
        ellip_ti->parent.text_pos++;
     }

   evas_common_text_props_bidi_set(&ellip_ti->text_props,
         ellip_ti->parent.text_node->bidi_props, ellip_ti->parent.text_pos);
   evas_common_text_props_script_set (&ellip_ti->text_props,
         ellip_ti->text);
   c->ENFN->font_text_props_info_create(c->ENDT,
         ellip_ti->parent.format->font.font,
         ellip_ti->text, &ellip_ti->text_props,
         ellip_ti->parent.text_node->bidi_props,
         ellip_ti->parent.text_pos, len);
   _text_item_update_sizes(c, ellip_ti);

   return ellip_ti;
}

/* 0 means go ahead, 1 means break without an error, 2 means
 * break with an error, should probably clean this a bit (enum/macro)
 * FIXME ^ */
static int
_layout_visualize_par(Ctxt *c)
{
   Evas_Object_Textblock_Item *it;
   Eina_List *i;
   if (!c->par->logical_items)
     return 2;

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

        /* Check if we need to wrap, i.e the text is bigger than the width */
        if ((c->w >= 0) &&
              ((c->x + it->w) >
               (c->w - c->o->style_pad.l - c->o->style_pad.r -
                c->marginl - c->marginr)))
          {
             /* Handle ellipsis here */
             if ((it->format->ellipsis == 1.0) && (c->h >= 0) &&
                   (2 * it->h + c->y >
                    c->h - c->o->style_pad.t - c->o->style_pad.b))
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
                       if ((wrap > 0) && last_ti->text[wrap])
                         {
                            _layout_item_text_split_strip_white(c, last_ti, i,
                                  wrap);
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
                  c->ln->ellip_ti = ellip_ti;
                  _layout_line_finalize(c, ellip_ti->parent.format);

                  return 1;
               }
             else if (it->format->wrap_word || it->format->wrap_char ||
                it->format->wrap_mixed)
               {
                  if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
                    {
                       /* Don't wrap if it's the only item */
                       if (c->ln->items)
                         {
                            /*FIXME: I should handle tabs correctly, i.e like
                             * spaces */
                            _layout_line_advance(c, it->format);
                         }
                    }
                  else
                    {
                       Evas_Object_Textblock_Text_Item *ti = _ITEM_TEXT(it);
                       int wrap;

                       adv_line = 1;
                       if (it->format->wrap_word)
                         wrap = _layout_get_wordwrap(c, it->format, ti);
                       else if (it->format->wrap_char)
                         wrap = _layout_get_charwrap(c, it->format, ti);
                       else if (it->format->wrap_mixed)
                         wrap = _layout_get_mixedwrap(c, it->format, ti);
                       else
                         wrap = -1;

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
                    }
               }
          }

        if (!redo_item)
          {
             c->ln->items = (Evas_Object_Textblock_Item *)
                eina_inlist_append(EINA_INLIST_GET(c->ln->items),
                      EINA_INLIST_GET(it));
             if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
               {
                  Evas_Object_Textblock_Format_Item *fi;
                  fi = _ITEM_FORMAT(it);
                  fi->y = c->y;
                  if (fi->item && _IS_LINE_SEPARATOR(fi->item))
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
   return 0;
}

/**
 * @internal
 * Create the layout from the nodes.
 *
 * @param obj the evas object - NOT NULL.
 * @param calc_only true if should only calc sizes false if should also create the layout..
 * @param w the object's w, -1 means no wrapping (i.e infinite size)
 * @param h the object's h, -1 means inifinte size.
 * @param w_ret the object's calculated w.
 * @param h_ret the object's calculated h.
 */
static void
_layout(const Evas_Object *obj, int calc_only, int w, int h, int *w_ret, int *h_ret)
{
   Evas_Object_Textblock *o;
   Ctxt ctxt, *c;
   Evas_Object_Textblock_Node_Text *n;
   Evas_Object_Textblock_Format *fmt = NULL;
   int style_pad_l = 0, style_pad_r = 0, style_pad_t = 0, style_pad_b = 0;

   /* setup context */
   o = (Evas_Object_Textblock *)(obj->object_data);
   c = &ctxt;
   c->obj = (Evas_Object *)obj;
   c->o = o;
   c->paragraphs = c->par = NULL;
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
   c->align_auto = EINA_TRUE;
   c->valign = o->valign;
   c->ln = NULL;


   /* Start of logical layout creation */

   /* setup default base style */
   if ((c->o->style) && (c->o->style->default_tag))
     {
        fmt = _layout_format_push(c, NULL);
        _format_fill(c->obj, fmt, c->o->style->default_tag);
     }
   if (!fmt)
     {
        if (w_ret) *w_ret = 0;
        if (h_ret) *h_ret = 0;
        return;
     }

   if (o->content_changed)
     {
        _paragraphs_free(obj, o->paragraphs);
        /* Go through all the text nodes to create the logical layout */
        EINA_INLIST_FOREACH(c->o->text_nodes, n)
          {
             Evas_Object_Textblock_Node_Format *fnode;
             size_t start;
             int off;

             n->dirty = 0; /* Mark as if we cleaned the paragraph, although
                              we should really use it to fine tune the
                              changes here, and not just blindly mark */
             _layout_paragraph_new(c, n); /* Each node is a paragraph */

             /* For each text node to thorugh all of it's format nodes
              * append text from the start to the offset of the next format
              * using the last format got. if needed it also creates format items
              * this is the core algorithm of the layout mechanism.
              * Skip the unicode replacement chars when there are because
              * we don't want to print them. */
             fnode = n->format_node;
             start = off = 0;
             while (fnode && (fnode->text_node == n))
               {
                  off += fnode->offset;
                  /* No need to skip on the first run, or a non-visible one */
                  _layout_text_append(c, fmt, n, start, off, o->repch);
                  _layout_do_format(obj, c, &fmt, fnode, &style_pad_l,
                        &style_pad_r, &style_pad_t, &style_pad_b);
                  if ((c->have_underline2) || (c->have_underline))
                    {
                       if (style_pad_b < c->underline_extend)
                         style_pad_b = c->underline_extend;
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
                  fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
               }
             _layout_text_append(c, fmt, n, start, -1, o->repch);
          }
        o->paragraphs = c->paragraphs;
     }
   else if (!calc_only)
     {
        _paragraphs_clear(obj, o->paragraphs);
        c->paragraphs = o->paragraphs;
        /* Merge the ones that need merging. */
        /* Go through all the paragraphs, lines, items and merge if should be
         * merged we merge backwards!!! */
        Evas_Object_Textblock_Paragraph *par;
        EINA_INLIST_FOREACH(EINA_INLIST_GET(c->paragraphs), par)
          {
             Eina_List *itr, *itr_next;
             Evas_Object_Textblock_Item *it, *prev_it = NULL;
             EINA_LIST_FOREACH_SAFE(par->logical_items, itr, itr_next, it)
               {
                  if (it->merge && prev_it &&
                        (prev_it->type == EVAS_TEXTBLOCK_ITEM_TEXT) &&
                        (it->type == EVAS_TEXTBLOCK_ITEM_TEXT))
                    {
                       _layout_item_merge_and_free(c, _ITEM_TEXT(prev_it),
                             _ITEM_TEXT(it));
                       par->logical_items =
                          eina_list_remove_list(par->logical_items, itr);
                    }
                  else
                    {
                       prev_it = it;
                    }
               }
          }
     }
   else /* Calc only and content hasn't changed */
     {
        c->paragraphs = o->paragraphs;
     }

   /* If there are no paragraphs, create the minimum needed,
    * if the last paragraph has no lines/text, create that as well */
   if (!c->paragraphs)
     {
        _layout_paragraph_new(c, NULL);
        o->paragraphs = c->paragraphs;
     }
   c->par = (Evas_Object_Textblock_Paragraph *)
      EINA_INLIST_GET(c->paragraphs)->last;
   if (!c->par->logical_items)
     {
        _layout_text_append(c, fmt, NULL, 0, 0, NULL);
     }

   /* In the case of calc only, we copy the items and the paragraphs,
    * but because we don't change OT_DATA we can use it, just copy the item
    * and ref */
   if (calc_only)
     {
        Evas_Object_Textblock_Paragraph *orig_pars, *par;
        Eina_List *itr;
        orig_pars = c->paragraphs;
        c->paragraphs = NULL;
        EINA_INLIST_FOREACH(EINA_INLIST_GET(orig_pars), par)
          {
             Evas_Object_Textblock_Item *it;
             c->par = malloc(sizeof(Evas_Object_Textblock_Paragraph));
             memcpy(c->par, par, sizeof(Evas_Object_Textblock_Paragraph));
             /* Both of these should not be copied */
             c->par->lines = NULL;
             c->par->logical_items = NULL;
             c->paragraphs = (Evas_Object_Textblock_Paragraph *)
                eina_inlist_append(EINA_INLIST_GET(c->paragraphs),
                      EINA_INLIST_GET(c->par));

             /* Copy all the items */
             EINA_LIST_FOREACH(par->logical_items, itr, it)
               {
                  Evas_Object_Textblock_Item *new_it;
                  if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
                    {
                       new_it = malloc(sizeof(Evas_Object_Textblock_Text_Item));
                       memcpy(new_it, it,
                             sizeof(Evas_Object_Textblock_Text_Item));
                    }
                  else
                    {
                       new_it = malloc(
                             sizeof(Evas_Object_Textblock_Format_Item));
                       memcpy(new_it, it,
                             sizeof(Evas_Object_Textblock_Format_Item));
                    }
                  c->par->logical_items =
                     eina_list_append(c->par->logical_items, new_it);
               }
          }
     }


   /* End of logical layout creation */

   /* Start of visual layout creation */
   /* FIXME: move away? */
   {
      Evas_Object_Textblock_Paragraph *par;
      EINA_INLIST_FOREACH(c->paragraphs, par)
        {
           c->par = par;
           _layout_update_par(c);
           /* Break if we should stop here. */
           if (_layout_visualize_par(c))
             break;
        }
   }
   /* End of visual layout creation */

   /* Clean the rest of the format stack */
   while (c->format_stack)
     {
        fmt = c->format_stack->data;
        c->format_stack = eina_list_remove_list(c->format_stack, c->format_stack);
        _format_unref_free(c->obj, fmt);
     }

   if (c->paragraphs)
     {
        Evas_Object_Textblock_Paragraph *last_par;
        last_par = (Evas_Object_Textblock_Paragraph *)
           EINA_INLIST_GET(c->paragraphs)->last;
        c->hmax = last_par->y + last_par->h;
     }

   if (w_ret) *w_ret = c->wmax;
   if (h_ret) *h_ret = c->hmax;

   /* Is this really the place? */
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
        if (!calc_only)
          {
             _paragraphs_clear(obj, c->paragraphs);
          }
        _layout(obj, calc_only, w, h, w_ret, h_ret);
        return;
     }

   if (calc_only)
     {
        Evas_Object_Textblock_Paragraph *par;
        while (c->paragraphs)
          {
             Eina_List *itr, *itrn;
             Evas_Object_Textblock_Item *it;
             par = c->paragraphs;
             /* free all the items */
             EINA_LIST_FOREACH_SAFE(par->logical_items, itr, itrn, it)
               {
                  par->logical_items =
                     eina_list_remove_list(par->logical_items, itr);
                  free(it);
               }
             c->paragraphs = (Evas_Object_Textblock_Paragraph *)
                eina_inlist_remove(EINA_INLIST_GET(c->paragraphs),
                      EINA_INLIST_GET(c->paragraphs));
             _paragraph_clear(obj, par);
             free(par);
          }
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
   o->have_ellipsis = 0;
   _layout(obj,
         0,
         obj->cur.geometry.w, obj->cur.geometry.h,
         &o->formatted.w, &o->formatted.h);
   o->formatted.valid = 1;
   o->last_w = obj->cur.geometry.w;
   o->last_h = obj->cur.geometry.h;
   o->changed = 0;
   o->content_changed = 0;
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
   Evas_Object_Textblock_Paragraph *par, *found_par = NULL;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   if (!o->formatted.valid) _relayout(obj);
   EINA_INLIST_FOREACH(o->paragraphs, par)
     {
        if (par->text_node == n)
          {
             found_par = par;
             break;
          }
     }
   if (found_par)
     {
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
   Evas_Object_Textblock_Paragraph *par, *prev_par = NULL;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock *o;

   o = (Evas_Object_Textblock *)(obj->object_data);
   EINA_INLIST_FOREACH(o->paragraphs, par)
     {
        if (prev_par && (prev_par->line_no <= line) && (line < par->line_no))
          {
             break;
          }
        prev_par = par;
     }
   if (prev_par)
     {
        EINA_INLIST_FOREACH(prev_par->lines, ln)
          {
             if (ln->par->line_no + ln->line_no == line) return ln;
          }
     }
   return NULL;
}

/**
 * Adds a textblock to the given evas.
 * @param   e The given evas.
 * @return  The new textblock object.
 */
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

/**
 * Creates a new textblock style.
 * @return  The new textblock style.
 */
EAPI Evas_Textblock_Style *
evas_textblock_style_new(void)
{
   Evas_Textblock_Style *ts;

   ts = calloc(1, sizeof(Evas_Textblock_Style));
   return ts;
}

/**
 * Destroys a textblock style.
 * @param ts The textblock style to free.
 */
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

/**
 * Sets the style ts to the style passed as text by text.
 * Expected a string consisting of many (or none) tag='format' pairs.
 *
 * @param ts  the style to set.
 * @param text the text to parse - NOT NULL.
 * @return Returns no value.
 */
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
        _evas_textblock_text_node_changed(o, obj, NULL);
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

/**
 * Return the text of the style ts.
 * @param ts  the style to get it's text.
 * @return the text of the style or null on error.
 */
EAPI const char *
evas_textblock_style_get(const Evas_Textblock_Style *ts)
{
   if (!ts) return NULL;
   return ts->style_text;
}

/* textblock styles */
/**
 * Set the objects style to ts.
 * @param obj the evas object to set the style to.
 * @param ts  the style to set.
 * @return Returns no value.
 */
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

    _evas_textblock_text_node_changed(o, obj, NULL);
}

/**
 * Return the style of an object.
 * @param obj  the object to get the style from.
 * @return the style of the object.
 */
EAPI const Evas_Textblock_Style *
evas_object_textblock_style_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   return o->style;
}

/**
 * @brief Set the "replacement character" to use for the given textblock object.
 *
 * @param obj The given textblock object.
 * @param ch The charset name.
 */
EAPI void
evas_object_textblock_replace_char_set(Evas_Object *obj, const char *ch)
{
   TB_HEAD();
   if (o->repch) eina_stringshare_del(o->repch);
   if (ch) o->repch = eina_stringshare_add(ch);
   else o->repch = NULL;
   _evas_textblock_text_node_changed(o, obj, NULL);
}

/**
 * @brief Sets newline mode. When true, newline character will behave
 * as a paragraph separator.
 *
 * @param obj The given textblock object.
 * @param mode EINA_TRUE for PS mode, EINA_FALSE otherwise.
 * @since 1.1.0
 */
EAPI void
evas_object_textblock_newline_mode_set(Evas_Object *obj, Eina_Bool mode)
{
   TB_HEAD();
   if (o->newline_is_ps == mode)
      return;

   o->newline_is_ps = mode;
    _evas_textblock_text_node_changed(o, obj, NULL);
}

/**
 * @brief Gets newline mode. When true, newline character behaves
 * as a paragraph separator.
 *
 * @param obj The given textblock object.
 * @return EINA_TRUE if in PS mode, EINA_FALSE otherwise.
 * @since 1.1.0
 */
EAPI Eina_Bool
evas_object_textblock_newline_mode_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(EINA_FALSE);
   return o->newline_is_ps;
}

/**
 * @brief Sets the vertical alignment of text within the textblock object
 * as a whole.
 *
 * Normally alignment is 0.0 (top of object). Values given should be
 * between 0.0 and 1.0 (1.0 bottom of object, 0.5 being vertically centered
 * etc.).
 * 
 * @param obj The given textblock object.
 * @param align A value between 0.0 and 1.0
 * @since 1.1.0
 */
EAPI void
evas_object_textblock_valign_set(Evas_Object *obj, double align)
{
   TB_HEAD();
   if (align < 0.0) align = 0.0;
   else if (align > 1.0) align = 1.0;
   if (o->valign == align) return;
   o->valign = align;
    _evas_textblock_text_node_changed(o, obj, NULL);
}

/**
 * @brief Gets the vertical alignment of a textblock
 *
 * @param obj The given textblock object.
 * @return The elignment set for the object
 * @since 1.1.0
 */
EAPI double
evas_object_textblock_valign_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(0.0);
   return o->valign;
}

/**
 * @brief Get the "replacement character" for given textblock object. Returns
 * NULL if no replacement character is in use.
 *
 * @param obj The given textblock object
 * @return replacement character or NULL
 */
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
   for (;((s < s_end) && (*p_m < m_end)); s++, (*p_m)++)
     {
        if (*s != **p_m)
          {
             _escaped_advance_after_end_of_string(p_m);
             return 0;
          }
     }

   if (*p_m < m_end)
     _escaped_advance_after_end_of_string(p_m);

   return s == s_end;
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
   return NULL;
}

/**
 * Returns the unescaped version of escape.
 * @param escape the string to be escaped
 * @return the unescaped version of escape
 */
EAPI const char *
evas_textblock_escape_string_get(const char *escape)
{
   /* &amp; -> & */
   return _escaped_char_get(escape, escape + strlen(escape));
}

/**
 * Return the unescaped version of the string between start and end.
 *
 * @param escape_start the start of the string.
 * @param escape_end the end of the string.
 * @return the unescaped version of the range
 */
EAPI const char *
evas_textblock_escape_string_range_get(const char *escape_start, const char *escape_end)
{
   return _escaped_char_get(escape_start, escape_end);
}

/**
 * Returns the escaped version of the string.
 * @param string to escape
 * @param len_ret the len of the new escape
 * @return the escaped string.
 */
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


/**
 * Sets the tetxblock's text to the markup text.
 *
 * @note assumes text does not include the unicode object replacement char (0xFFFC)
 *
 * @param obj  the textblock object.
 * @param text the markup text to use.
 * @return Return no value.
 */
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

/**
 * Prepends markup to the cursor cur.
 *
 * @note assumes text does not include the unicode object replacement char (0xFFFC)
 *
 * @param cur  the cursor to prepend to.
 * @param text the markup text to prepend.
 * @return Return no value.
 */
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
                            const char *match;
                            size_t replace_len;

                            memcpy(ttag, tag_start + 1, ttag_len);
                            ttag[ttag_len] = 0;
                            match = _style_match_tag(o->style, ttag, ttag_len, &replace_len);
                            if (match)
                              {
                                 evas_textblock_cursor_format_prepend(o->cursor, match);
                              }
                            else
                              {
                                 char *ttag2;

                                 ttag2 = malloc(ttag_len + 2 + 1);
                                 if (ttag2)
                                   {
                                      if (ttag[0] == '/')
                                        {
                                           strcpy(ttag2, "- ");
                                           strcat(ttag2, ttag + 1);
                                        }
                                      else
                                        {
                                           strcpy(ttag2, "+ ");
                                           strcat(ttag2, ttag);
                                        }
                                      evas_textblock_cursor_format_prepend(o->cursor, ttag2);
                                      free(ttag2);
                                   }
                              }
                            free(ttag);
                         }
                       tag_start = tag_end = NULL;
                    }
                  else if (esc_end)
                    {
                       _prepend_escaped_char(o->cursor, esc_start, esc_end);
                       esc_start = esc_end = NULL;
                    }
                  else if (*p == 0)
                    {
                       _prepend_text_run(o, s, p);
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
                       _prepend_text_run(o, s, p);
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
                       _prepend_text_run(o, s, p);
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
                  _prepend_text_run(o, s, p);
                  p += 2; /* it's also advanced later in this loop need +3
                           * in total*/
                  s = p + 1; /* One after the end of the replacement char */
               }
             p++;
          }
     }
   _evas_textblock_text_node_changed(o, obj, o->cursor->node);
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
_markup_get_format_append(Evas_Object_Textblock *o, Eina_Strbuf *txt, Evas_Object_Textblock_Node_Format *fnode)
{
   size_t replace_len;
   size_t tag_len;
   const char *tag;
   const char *replace;

   replace_len = eina_strbuf_length_get(fnode->format);
   replace = eina_strbuf_string_get(fnode->format);
   tag = _style_match_replace(o->style, replace, replace_len, &tag_len);
   eina_strbuf_append_char(txt, '<');
   if (tag)
     {
        eina_strbuf_append_length(txt, tag, tag_len);
     }
   else
     {
        const char *s;
        int push = 0;
        int pop = 0;

        // FIXME: need to escape
        s = eina_strbuf_string_get(fnode->format);
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
/**
 * Return the markup of the object.
 *
 * @param obj the evas object.
 * @return the markup text of the object.
 */
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
#ifdef BIDI_SUPPORT
   evas_bidi_paragraph_props_unref(to->bidi_props);
   to->bidi_props = evas_bidi_paragraph_props_get(
         eina_ustrbuf_string_get(to->unicode));
#endif

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

/**
 * Return the object's main cursor.
 *
 * @param obj the object.
 * @return the obj's main cursor.
 */
EAPI const Evas_Textblock_Cursor *
evas_object_textblock_cursor_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   return o->cursor;
}

/**
 * Create a new cursor, associate it to the obj and init it to point
 * to the start of the textblock. Association to the object means the cursor
 * will be updated when the object will change.
 *
 * @note if you need speed and you know what you are doing, it's slightly faster to just allocate the cursor yourself and not associate it. (only people developing the actual object, and not users of the object).
 *
 * @param obj the object to associate to.
 * @return the new cursor.
 */
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

/**
 * Free the cursor and unassociate it from the object.
 * @note do not use it to free unassociated cursors.
 *
 * @param cur the cursor to free.
 * @return Returns no value.
 */
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

/**
 * Returns true if the cursor points to a format.
 *
 * @param cur the cursor to check.
 * @return Returns #EINA_TRUE if a cursor points to a format #EINA_FALSE otherwise.
 */
EAPI Eina_Bool
evas_textblock_cursor_is_format(const Evas_Textblock_Cursor *cur)
{
   if (!cur || !cur->node) return EINA_FALSE;
   if (evas_textblock_cursor_format_is_visible_get(cur)) return EINA_TRUE;
   return (_evas_textblock_cursor_node_format_at_pos_get(cur)) ?
      EINA_TRUE : EINA_FALSE;
}

/**
 * Returns the first format node.
 *
 * @param obj The evas, must not be NULL.
 * @return Returns the first format node, may be null if there are none.
 */
EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_first_get(const Evas_Object *obj)
{
   TB_HEAD_RETURN(NULL);
   return o->format_nodes;
}
/**
 * Returns the last format node.
 *
 * @param obj The evas textblock, must not be NULL.
 * @return Returns the first format node, may be null if there are none.
 */
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

/**
 * Returns the next format node (after n)
 *
 * @param n the current format node - not null.
 * @return Returns the next format node, may be null.
 */
EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_next_get(const Evas_Object_Textblock_Node_Format *n)
{
   return _NODE_FORMAT(EINA_INLIST_GET(n)->next);
}

/**
 * Returns the prev format node (after n)
 *
 * @param n the current format node - not null.
 * @return Returns the prev format node, may be null.
 */
EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_prev_get(const Evas_Object_Textblock_Node_Format *n)
{
   return _NODE_FORMAT(EINA_INLIST_GET(n)->prev);
}

/**
 * Remove a format node and it's match. i.e, removes a <tag> </tag> pair.
 * Assumes the node is the first part of <tag> i.e, this won't work if
 * n is a closing tag.
 *
 * @param obj the evas object of the textblock - not null.
 * @param n the current format node - not null.
 */
EAPI void
evas_textblock_node_format_remove_pair(Evas_Object *obj,
      Evas_Object_Textblock_Node_Format *n)
{
   Evas_Object_Textblock_Node_Text *tnode;
   Evas_Object_Textblock_Node_Format *fmt, *pnode;
   int level;
   TB_HEAD();

   if (!n) return;

   pnode = NULL;
   fmt = n;
   tnode = fmt->text_node;
   level = 0;

   do
     {
        const char *fstr = eina_strbuf_string_get(fmt->format);

        if (fstr && (*fstr == '+'))
          {
             level++;
          }
        else if (fstr && (*fstr == '-'))
          {
             level--;
          }

        pnode = fmt;
        fmt = _NODE_FORMAT(EINA_INLIST_GET(fmt)->next);
     }
   while (fmt && (level > 0));

   if (n->visible)
     {
        size_t index = _evas_textblock_node_format_pos_get(n);
        const char *format = eina_strbuf_string_get(n->format);
        Evas_Textblock_Cursor cur;
        cur.obj = obj;

        eina_ustrbuf_remove(n->text_node->unicode, index, index + 1);
        if (format && _IS_PARAGRAPH_SEPARATOR(o, format))
          {
             evas_textblock_cursor_set_at_format(&cur, n);
             _evas_textblock_cursor_nodes_merge(&cur);
          }
        _evas_textblock_cursors_update_offset(&cur, n->text_node, index, -1);
     }
   _evas_textblock_node_format_remove(o, n, 0);
   if (pnode && (pnode != n))
     {
        /* pnode can never be visible! (it's the closing format) */
        _evas_textblock_node_format_remove(o, pnode, 0);
     }
   _evas_textblock_text_node_changed(o, obj, tnode);
}

/**
 * Sets the cursor to the start of the first text node.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void
evas_textblock_cursor_paragraph_first(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   if (!cur) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   cur->node = o->text_nodes;
   cur->pos = 0;

}

/**
 * sets the cursor to the end of the last text node.
 *
 * @param cur the cursor to set.
 * @return Returns no value.
 */
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

/**
 * Advances to the the start of the next text node
 *
 * @param cur the cursor to update
 * @return #EINA_TRUE if it managed to advance a paragraph, #EINA_FALSE otherwise.
 */
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

/**
 * Advances to the the end of the previous text node
 *
 * @param cur the cursor to update
 * @return #EINA_TRUE if it managed to advance a paragraph, #EINA_FALSE otherwise.
 */
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

/**
 * Sets the cursor to point to the place where format points to.
 *
 * @param cur the cursor to update.
 * @param n the format node to update according.
 */
EAPI void
evas_textblock_cursor_set_at_format(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *n)
{
   if (!cur || !n) return;
   cur->node = n->text_node;
   cur->pos = _evas_textblock_node_format_pos_get(n);
}

/**
 * Advances to the next format node
 *
 * @param cur the cursor to be updated.
 * @return #EINA_TRUE on success #EINA_FALSE otherwise.
 */
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

/**
 * Advances to the previous format node.
 *
 * @param cur the cursor to update.
 * @return #EINA_TRUE on success #EINA_FALSE otherwise.
 */
EAPI Eina_Bool
evas_textblock_cursor_format_prev(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock_Node_Format *node;
   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;
   /* If the current node is a format node, just get the next if any,
    * if it's a text, get the current format node out of the text and return
    * the next format node if any. */
   node = _evas_textblock_cursor_node_format_before_or_at_pos_get(cur);
   if (evas_textblock_cursor_is_format(cur))
     {
        if (node)
          {
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
             cur->pos = _evas_textblock_node_format_pos_get(node);

             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

/**
 * Advances 1 char forward.
 *
 * @param cur the cursor to advance.
 * @return #EINA_TRUE on success #EINA_FALSE otherwise.
 */
EAPI Eina_Bool
evas_textblock_cursor_char_next(Evas_Textblock_Cursor *cur)
{
   int index;
   const Eina_Unicode *text;

   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;

   index = cur->pos;
   text = eina_ustrbuf_string_get(cur->node->unicode);
   GET_NEXT(text, index);
   /* Only allow pointing a null if it's the last paragraph.
    * because we don't have a PS there. */
   if (text[index])
     {
        cur->pos = index;
        return EINA_TRUE;
     }
   else
     {
        if (!evas_textblock_cursor_paragraph_next(cur))
          {
             /* If we already were at the end, that means we don't have
              * where to go next we should return FALSE */
             if (cur->pos == (size_t) index)
               return EINA_FALSE;

             cur->pos = index;
             return EINA_TRUE;
          }
        else
          {
             return EINA_TRUE;
          }
     }
}

/**
 * Advances 1 char backward.
 *
 * @param cur the cursor to advance.
 * @return #EINA_TRUE on success #EINA_FALSE otherwise.
 */
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

/**
 * Go to the first char in the node the cursor is pointing on.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void
evas_textblock_cursor_paragraph_char_first(Evas_Textblock_Cursor *cur)
{
   if (!cur) return;
   cur->pos = 0;

}

/**
 * Go to the last char in a text node.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
EAPI void
evas_textblock_cursor_paragraph_char_last(Evas_Textblock_Cursor *cur)
{
   int index;

   if (!cur) return;
   if (!cur->node) return;
   index = eina_ustrbuf_length_get(cur->node->unicode) - 1;
   if (index >= 0)
      cur->pos = index;
   else
      cur->pos = 0;

}

/**
 * Go to the start of the current line
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
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

/**
 * Go to the end of the current line.
 *
 * @param cur the cursor to update.
 * @return Returns no value.
 */
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
        size_t index;

	cur->node = it->text_node;
	cur->pos = it->text_pos;
        if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
          {
             index = _ITEM_TEXT(it)->text_props.text_len - 1;
             GET_NEXT(_ITEM_TEXT(it)->text, index);
             cur->pos += index;
          }
        else if (!EINA_INLIST_GET(ln)->next && !EINA_INLIST_GET(ln->par)->next)
          {
             cur->pos++;
          }
     }
}

/**
 * @internal
 * checks if a format (as a string) is visible.
 *
 * @param the string.
 * @return #EINA_TRUE if it represents a visible format #EINA_FALSE otherwise.
 */
static Eina_Bool
_evas_textblock_format_is_visible(const char *s)
{
   const char *item;
   Eina_Bool is_opener = EINA_TRUE;

   if (!s) return EINA_FALSE;

   if (s[0] == '+' || s[0] == '-')
     {
        is_opener = (s[0] == '+') ? EINA_TRUE : EINA_FALSE;
        s++;
     }
   while ((item = _format_parse(&s)))
     {
        char *tmp;
        tmp = alloca(s - item + 1);
        strncpy(tmp, item, s - item);
        tmp[s - item] = '\0';
        /* We care about all of the formats even after a - except for
         * item which we don't care after a - because it's just a standard
         * closing */
        if (((!strcmp(item, "\n")) || (!strcmp(item, "\\n"))) ||
              ((!strcmp(item, "\t")) || (!strcmp(item, "\\t"))) ||
              (!strcmp(item, "ps")) ||
              (!strncmp(item, "item", 4) && is_opener)) /*FIXME: formats like item2 will break it. */
          return EINA_TRUE;
     }
   return EINA_FALSE;
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
        const char *fstr = eina_strbuf_string_get(fmt->format);

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
             fnode = eina_list_data_get(formats);
             if (fnode)
               {
                  formats = eina_list_remove(formats, fnode);
                  _evas_textblock_node_format_remove(o, fnode, 0);
                  _evas_textblock_node_format_remove(o, fmt, 0);
               }
          }
        else if (!fmt->visible)
          {
             _evas_textblock_node_format_remove(o, fmt, 0);
          }
        fmt = nnode;
     }
   while (fmt && (offset == 0) && (fmt->text_node == tnode));
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
   _evas_textblock_node_format_free(n);
}

/**
 * @internal
 * Sets all the offsets of the format nodes between start and end in the text
 * node n to 0 and sets visibility to EINA_FALSE.
 * If end == -1 end means the end of the string.
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
             new_node = _NODE_TEXT(EINA_INLIST_GET(n)->next);
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
             if (_IS_PARAGRAPH_SEPARATOR(o,
                      eina_strbuf_string_get(last_node->format)))
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

/**
 * Return the current cursor pos.
 *
 * @param cur the cursor to take the position from.
 * @return the position or -1 on error
 */
EAPI int
evas_textblock_cursor_pos_get(const Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *n;
   size_t npos = 0;

   if (!cur) return -1;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   n = o->text_nodes;
   while (n != cur->node)
     {
        npos += eina_ustrbuf_length_get(n->unicode);
        n = _NODE_TEXT(EINA_INLIST_GET(n)->next);
     }
   return npos + cur->pos;
}

/**
 * Set the cursor pos.
 *
 * @param cur the cursor to be set.
 * @param pos the pos to set.
 */
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

/**
 * Go to the start of the line passed
 *
 * @param cur cursor to update.
 * @param line numer to set.
 * @return #EINA_TRUE on success, #EINA_FALSE on error.
 */
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

/**
 * Compare two cursors.
 *
 * @param cur1 the first cursor.
 * @param cur2 the second cursor.
 * @return -1 if cur1 < cur2, 0 if cur1 == cur2 and 1 otherwise.
 */
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

/**
 * Make cur_dest point to the same place as cur. Does not work if they don't
 * point to the same object.
 *
 * @param cur the source cursor.
 * @param cur_dest destination cursor.
 * @return Returns no value.
 */
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
#ifdef BIDI_SUPPORT
   if (&n->bidi_props)
     evas_bidi_paragraph_props_unref(n->bidi_props);
#endif
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
#ifdef BIDI_SUPPORT
   n->bidi_props = evas_bidi_paragraph_props_new();
   n->bidi_props->direction = EVAS_BIDI_PARAGRAPH_NATURAL;
#endif

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
        text = eina_ustrbuf_string_get(cur->node->unicode);
        len = eina_ustrbuf_length_get(cur->node->unicode) - start;
        eina_ustrbuf_append_length(n->unicode, text + start, len);
        eina_ustrbuf_remove(cur->node->unicode, start, start + len);
#ifdef BIDI_SUPPORT
   evas_bidi_paragraph_props_unref(n->bidi_props);
   n->bidi_props = evas_bidi_paragraph_props_get(
         eina_ustrbuf_string_get(n->unicode));

   evas_bidi_paragraph_props_unref(cur->node->bidi_props);
   cur->node->bidi_props = evas_bidi_paragraph_props_get(
         eina_ustrbuf_string_get(cur->node->unicode));
#endif
     }
   else
     {
        Evas_Object_Textblock_Node_Format *fnode;
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
          }
     }
}

/**
 * @internal
 * Mark and notifiy that the textblock, and specifically a node has changed.
 *
 * @param o the textblock object.
 * @param obj the evas object.
 * @param n the paragraph that changed - NULL means all.
 */
static void
_evas_textblock_text_node_changed(Evas_Object_Textblock *o, Evas_Object *obj,
      Evas_Object_Textblock_Node_Text *n)
{
   if (!n)
     {
        Evas_Object_Textblock_Node_Text *itr;
        EINA_INLIST_FOREACH(EINA_INLIST_GET(o->text_nodes), itr)
          {
             itr->dirty = EINA_TRUE;
          }
     }
   else
     {
        n->dirty = EINA_TRUE;
     }
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

/**
 * Adds text to the current cursor position and set the cursor to *before*
 * the start of the text just added.
 *
 * @param cur the cursor to where to add text at.
 * @param _text the text to add.
 * @return Returns the len of the text added.
 * @see evas_textblock_cursor_text_prepend()
 */
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
   /* Update all the cursors after our position. */
   _evas_textblock_cursors_update_offset(cur, cur->node, cur->pos, len);

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
#ifdef BIDI_SUPPORT
   evas_bidi_paragraph_props_unref(n->bidi_props);
   n->bidi_props = evas_bidi_paragraph_props_get(eina_ustrbuf_string_get(n->unicode));
#endif
   _evas_textblock_text_node_changed(o, cur->obj, n);
   free(text);
   return len;
}

/**
 * Adds text to the current cursor position and set the cursor to *after*
 * the start of the text just added.
 *
 * @param cur the cursor to where to add text at.
 * @param _text the text to add.
 * @return Returns the len of the text added.
 * @see evas_textblock_cursor_text_append()
 */
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
 * @prama n the format node to free
 */
static void
_evas_textblock_node_format_free(Evas_Object_Textblock_Node_Format *n)
{
   if (!n) return;
   eina_strbuf_free(n->format);
   free(n);
}

/**
 * @internal
 * Create a new format node.
 *
 * @param format the text to create the format node from.
 * @return Returns the new format node
 */
static Evas_Object_Textblock_Node_Format *
_evas_textblock_node_format_new(const char *format)
{
   Evas_Object_Textblock_Node_Format *n;

   n = calloc(1, sizeof(Evas_Object_Textblock_Node_Format));
   n->format = eina_strbuf_new();
   eina_strbuf_append(n->format, format);
   n->visible = _evas_textblock_format_is_visible(format);

   return n;
}

/**
 * Check if the current cursor position points to the terminating null of the
 * last paragraph. (shouldn't be allowed to point to the terminating null of
 * any previous paragraph anyway.
 *
 * @param cur the cursor to look at.
 * @return #EINA_TRUE if the cursor points to the terminating null, #EINA_FALSE otherwise.
 */
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

/**
 * Adds format to the current cursor position. If the format being added is a
 * visible format, add it *before* the cursor position, otherwise, add it after.
 * This behavior is because visible formats are like characters and invisible
 * should be stacked in a way that the last one is added last.
 *
 * This function works with native formats, that means that style defined
 * tags like <br> won't work here. For those kind of things use markup prepend.
 *
 * @param cur the cursor to where to add format at.
 * @param format the format to add.
 * @return Returns true if a visible format was added, false otherwise.
 * @see evas_textblock_cursor_format_prepend()
 */
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

   n = _evas_textblock_node_format_new(format);
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
        eina_ustrbuf_insert_char(cur->node->unicode,
              EVAS_TEXTBLOCK_REPLACEMENT_CHAR, cur->pos);

        /* Advance all the cursors after our cursor */
        _evas_textblock_cursors_update_offset(cur, cur->node, cur->pos, 1);
        if (_IS_PARAGRAPH_SEPARATOR(o, format))
          {
             _evas_textblock_cursor_break_paragraph(cur, n);
          }
        else
          {
#ifdef BIDI_SUPPORT
             evas_bidi_paragraph_props_unref(cur->node->bidi_props);
             cur->node->bidi_props = evas_bidi_paragraph_props_get(
                   eina_ustrbuf_string_get(cur->node->unicode));
#endif
          }
     }

   _evas_textblock_text_node_changed(o, cur->obj, cur->node);

   return is_visible;
}

/**
 * Adds format to the current cursor position. If the format being added is a
 * visible format, add it *before* the cursor position, otherwise, add it after.
 * This behavior is because visible formats are like characters and invisible
 * should be stacked in a way that the last one is added last.
 * If the format is visible the cursor is advanced after it.
 *
 * This function works with native formats, that means that style defined
 * tags like <br> won't work here. For those kind of things use markup prepend.
 *
 * @param cur the cursor to where to add format at.
 * @param format the format to add.
 * @return Returns true if a visible format was added, false otherwise.
 * @see evas_textblock_cursor_format_prepend()
 */
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


/**
 * Delete the character at the location of the cursor. If there's a format
 * pointing to this position, delete it as well.
 *
 * @param cur the cursor pointing to the current location.
 * @return Returns no value.
 */
EAPI void
evas_textblock_cursor_char_delete(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *n, *n2;
   int merge_nodes = 0;
   const Eina_Unicode *text;
   int chr, index, ppos;

   if (!cur || !cur->node) return;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   n = cur->node;

   text = eina_ustrbuf_string_get(n->unicode);
   index = cur->pos;
   chr = GET_NEXT(text, index);
   if (chr == 0) return;
   ppos = cur->pos;
   /* Remove a format node if needed, and remove the char only if the
    * fmt node is not visible */
     {
        Evas_Object_Textblock_Node_Format *fmt, *fmt2;
        fmt = _evas_textblock_cursor_node_format_at_pos_get(cur);
        if (fmt)
          {
             const char *format = NULL;
             Evas_Object_Textblock_Node_Format *last_fmt;
             /* If there's a PS it must be the last become it delimits paragraphs */
             last_fmt = _evas_textblock_node_format_last_at_off(fmt);
             format = eina_strbuf_string_get(last_fmt->format);
             if (format && _IS_PARAGRAPH_SEPARATOR(o, format))
               {
                  merge_nodes = 1;
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
              -(index - cur->pos));

        _evas_textblock_node_format_remove_matching(o, fmt);
     }
   eina_ustrbuf_remove(n->unicode, cur->pos, index);
   /* If it was a paragraph separator, we should merge the current with the
    * next, there must be a next. */
   if (merge_nodes)
     {
        _evas_textblock_cursor_nodes_merge(cur);
     }
#ifdef BIDI_SUPPORT
   evas_bidi_paragraph_props_unref(n->bidi_props);
   n->bidi_props = evas_bidi_paragraph_props_get(eina_ustrbuf_string_get(n->unicode));
#endif

   if (cur->pos == eina_ustrbuf_length_get(n->unicode))
     {
	n2 = _NODE_TEXT(EINA_INLIST_GET(n)->next);
	if (n2)
	  {
	     cur->node = n2;
	     cur->pos = 0;
	  }
     }

   _evas_textblock_cursors_update_offset(cur, n, ppos, -(index - ppos));
   _evas_textblock_text_node_changed(o, cur->obj, cur->node);
}

/**
 * Delete the range between cur1 and cur2.
 *
 * @param cur1 one side of the range.
 * @param cur2 the second side of the range
 * @return Returns no value.
 */
EAPI void
evas_textblock_cursor_range_delete(Evas_Textblock_Cursor *cur1, Evas_Textblock_Cursor *cur2)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Node_Text *n1, *n2, *n;
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
   _evas_textblock_node_format_remove_matching(o,
         _evas_textblock_cursor_node_format_at_pos_get(cur1));

   if (should_merge)
     {
        /* We call this function instead of the cursor one because we already
         * updated the cursors */
        _evas_textblock_nodes_merge(o, n1);
     }

#ifdef BIDI_SUPPORT
   evas_bidi_paragraph_props_unref(n1->bidi_props);
   n1->bidi_props = evas_bidi_paragraph_props_get(
         eina_ustrbuf_string_get(n1->unicode));
#endif

   evas_textblock_cursor_copy(cur1, cur2);
   if (reset_cursor)
     evas_textblock_cursor_copy(cur1, o->cursor);

   _evas_textblock_text_node_changed(o, cur1->obj, cur1->node);
}


/**
 * Return the content of the cursor.
 *
 * @param cur the cursor
 * @return the text in the range
 */
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
        return strdup(tmp);
     }

   ustr = eina_ustrbuf_string_get(cur->node->unicode);
   buf[0] = ustr[cur->pos];
   buf[1] = 0;
   s = eina_unicode_unicode_to_utf8(buf, NULL);

   return s;
}

/**
 * Return the text in the range between cur1 and cur2
 *
 * FIXME: format is currently unused, you always get markup back.
 *
 * @param cur1 one side of the range.
 * @param cur2 the other side of the range
 * @param format to be documented
 * @return the text in the range
 * @see elm_entry_markup_to_utf8()
 */
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

/**
 * Return the text of the paragraph cur points to - returns the text in markup..
 *
 * @param cur the cursor pointing to the paragraph.
 * @return the text on success, NULL otherwise.
 */
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

/**
 * Return the length of the paragraph, cheaper the eina_unicode_strlen()
 *
 * @param cur the position of the paragraph.
 * @return the length of the paragraph on success, -1 otehrwise.
 */
EAPI int
evas_textblock_cursor_paragraph_text_length_get(const Evas_Textblock_Cursor *cur)
{
   if (!cur) return -1;
   if (!cur->node) return -1;
   return eina_ustrbuf_length_get(cur->node->unicode);
}

/**
 * Return the format node at the position pointed by cur.
 *
 * @param cur the position to look at.
 * @return the format node if found, NULL otherwise.
 * @see evas_textblock_cursor_format_is_visible_get()
 */
EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_cursor_format_get(const Evas_Textblock_Cursor *cur)
{
   if (!cur) return NULL;
   if (!cur->node) return NULL;
   return _evas_textblock_cursor_node_format_at_pos_get(cur);
}
/**
 * Get the text format representation of the format node.
 *
 * @param fmt the format node.
 * @return the textual format of the format node.
 */
EAPI const char *
evas_textblock_node_format_text_get(const Evas_Object_Textblock_Node_Format *fmt)
{
   if (!fmt) return NULL;
   return eina_strbuf_string_get(fmt->format);
}

/**
 * Set the cursor to point to the position of fmt.
 *
 * @param cur the cursor to update
 * @param fmt the format to update according to.
 */
EAPI void
evas_textblock_cursor_at_format_set(Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Format *fmt)
{
   if (!fmt || !cur) return;
   cur->node = fmt->text_node;
   cur->pos = _evas_textblock_node_format_pos_get(fmt);
}

/**
 * Check if the current cursor position is a visible format. This way is more
 * efficient than evas_textblock_cursor_format_get() to check for the existence
 * of a visible format.
 *
 * @param cur the cursor to look at.
 * @return #EINA_TRUE if the cursor points to a visible format, #EINA_FALSE otherwise.
 * @see evas_textblock_cursor_format_get()
 */
EAPI Eina_Bool
evas_textblock_cursor_format_is_visible_get(const Evas_Textblock_Cursor *cur)
{
   const Eina_Unicode *text;

   if (!cur) return EINA_FALSE;
   if (!cur->node) return EINA_FALSE;
   text = eina_ustrbuf_string_get(cur->node->unicode);
   return (text[cur->pos] == EVAS_TEXTBLOCK_REPLACEMENT_CHAR) ?
              EINA_TRUE : EINA_FALSE;
}

/**
 * Returns the geometry of the cursor. Depends on the type of cursor requested.
 * This should be used instead of char_geometry_get because there are weird
 * special cases with BiDi text.
 * in '_' cursor mode (i.e a line below the char) it's the same as char_geometry
 * get, except for the case of the last char of a line which depends on the
 * paragraph direction.
 *
 * in '|' cursor mode (i.e a line between two chars) it is very varyable.
 * For example consider the following visual string:
 * "abcCBA" (ABC are rtl chars), a cursor pointing on A should actually draw
 * a '|' between the c and the C.
 *
 * @param cur the cursor.
 * @param cx the x of the cursor
 * @param cy the y of the cursor
 * @param cw the width of the cursor
 * @param ch the height of the cursor
 * @param dir the direction of the cursor, can be NULL.
 * @param ctype the type of the cursor.
 * @return line number of the char on success, -1 on error.
 */
EAPI int
evas_textblock_cursor_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_BiDi_Direction *dir, Evas_Textblock_Cursor_Type ctype)
{
   int ret = -1;
   const Evas_Textblock_Cursor *dir_cur;
   Evas_Textblock_Cursor cur2;

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

             if (!fmt ||
                   !_IS_LINE_SEPARATOR(eina_strbuf_string_get(fmt->format)))
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
                  if ((!before_char &&
                           evas_bidi_is_rtl_char(dir_cur->node->bidi_props, 0,
                              dir_cur->pos)) ||
                        (before_char &&
                         !evas_bidi_is_rtl_char(dir_cur->node->bidi_props, 0,
                            dir_cur->pos)))

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
             /* Adjust if the char is an rtl char */
             if ((ret >= 0) && (!evas_bidi_is_rtl_char(
                         dir_cur->node->bidi_props, 0, dir_cur->pos)))
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
        if (_evas_textblock_cursor_is_at_the_end(dir_cur) && (dir_cur->pos > 0))
          {
             *dir = (evas_bidi_is_rtl_char(dir_cur->node->bidi_props, 0,
                      dir_cur->pos - 1)) ?
                EVAS_BIDI_DIRECTION_RTL : EVAS_BIDI_DIRECTION_LTR;
          }
        else if (dir_cur->pos > 0)
          {
             *dir = (evas_bidi_is_rtl_char(dir_cur->node->bidi_props, 0,
                      dir_cur->pos)) ?
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
_evas_textblock_cursor_char_pen_geometry_common_get(int (*query_func) (void *data, void *font, const Eina_Unicode *text, const Evas_Text_Props *intl_props, int pos, int *cx, int *cy, int *cw, int *ch), const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
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
   if (!cur->node)
     {
        if (!o->text_nodes)
          {
             if (!o->paragraphs) return -1;
             ln = o->paragraphs->lines;
             if (!ln) return -1;
             if (cx) *cx = ln->par->x + ln->x;
             if (cy) *cy = ln->par->y + ln->y;
             if (cw) *cw = ln->w;
             if (ch) *ch = ln->h;
             return ln->par->line_no + ln->line_no;
          }
        else
          return -1;
     }
   if (!o->formatted.valid) _relayout(cur->obj);

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
                   ti->text, &ti->text_props,
                   pos,
                   &x, &y, &w, &h);
          }

        x += ln->par->x + ln->x + _ITEM(ti)->x;

        if (x < ln->par->x + ln->x)
          {
             x = ln->par->x + ln->x;
          }
	y = ln->par->y + ln->y;
	h = ln->h;
     }
   else if (ln && fi)
     {
        if (previous_format)
          {
             if (_IS_LINE_SEPARATOR(
                      eina_strbuf_string_get(fi->source_node->format)))
               {
                  x = 0;
                  y = ln->par->y + ln->y + ln->h;
               }
             else
               {
#ifdef BIDI_SUPPORT
                  if (EVAS_BIDI_PARAGRAPH_DIRECTION_IS_RTL(
                           fi->parent.text_node->bidi_props))
                    {
                       x = ln->par->x + ln->x;
                    }
                  else
#endif
                    {
                       x = ln->par->x + ln->x + ln->w;
                    }
                  y = ln->par->y + ln->y;
               }
             w = 0;
             h = ln->h;
          }
        else
          {
             x = ln->par->x + ln->x + _ITEM(fi)->x;
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

/**
 * Returns the geometry of the char at cur.
 *
 * @param cur the position of the char.
 * @param cx the x of the char.
 * @param cy the y of the char.
 * @param cw the w of the char.
 * @param ch the h of the char.
 * @return line number of the char on success, -1 on error.
 */
EAPI int
evas_textblock_cursor_char_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   return _evas_textblock_cursor_char_pen_geometry_common_get(
         cur->ENFN->font_char_coords_get, cur, cx, cy, cw, ch);
}

/**
 * Returns the geometry of the pen at cur.
 *
 * @param cur the position of the char.
 * @param cpen_x the pen_x of the char.
 * @param cy the y of the char.
 * @param cadv the adv of the char.
 * @param ch the h of the char.
 * @return line number of the char on success, -1 on error.
 */
EAPI int
evas_textblock_cursor_pen_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   return _evas_textblock_cursor_char_pen_geometry_common_get(
         cur->ENFN->font_pen_coords_get, cur, cx, cy, cw, ch);
}

/**
 * Returns the geometry of the line at cur.
 *
 * @param cur the position of the line.
 * @param cx the x of the line.
 * @param cy the y of the line.
 * @param cw the width of the line.
 * @param ch the height of the line.
 * @return line number of the line on success, -1 on error.
 */
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
   x = ln->par->x + ln->x;
   y = ln->par->y + ln->y;
   w = ln->w;
   h = ln->h;
   if (cx) *cx = x;
   if (cy) *cy = y;
   if (cw) *cw = w;
   if (ch) *ch = h;
   return ln->par->line_no + ln->line_no;
}

/**
 * Set the position of the cursor according to the X and Y coordinates.
 *
 * @param cur the cursor to set.
 * @param x coord to set by.
 * @param y coord to set by.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
EAPI Eina_Bool
evas_textblock_cursor_char_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Paragraph *par, *found_par = NULL;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock_Item *it = NULL, *it_break = NULL;

   if (!cur) return EINA_FALSE;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   x += o->style_pad.l;
   y += o->style_pad.t;
   EINA_INLIST_FOREACH(o->paragraphs, par)
     {
        if ((par->x <= x) && (par->x + par->w > x) &&
              (par->y <= y) && (par->y + par->h > y))
          {
             found_par = par;
             break;
          }
     }
   if (found_par)
     {
        EINA_INLIST_FOREACH(found_par->lines, ln)
          {
             if (ln->par->y + ln->y > y) break;
             if ((ln->par->y + ln->y <= y) && ((ln->par->y + ln->y + ln->h) > y))
               {
                  EINA_INLIST_FOREACH(ln->items, it)
                    {
                       if ((it->x + ln->par->x + ln->x) > x)
                         {
                            it_break = it;
                            break;
                         }
                       if (((it->x + ln->par->x + ln->x) <= x) && (((it->x + ln->par->x + ln->x) + it->adv) > x))
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
                                         ti->text, &ti->text_props,
                                         x - it->x - ln->par->x - ln->x, 0,
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
                                 cur->node = fi->source_node->text_node;
                                 return EINA_TRUE;
                              }
                         }
                    }
                  if (it_break)
                    {
                       it = it_break;
                       cur->node = it->text_node;
                       cur->pos = it->text_pos;

                       /*FIXME: needs smarter handling, ATM just check, if it's
                        * the first item, then go to the end of the line, helps
                        * with rtl langs, doesn't affect ltr langs that much. */
                       if (!EINA_INLIST_GET(it)->prev)
                         {
                            evas_textblock_cursor_line_char_last(cur);
                         }

                       return EINA_TRUE;
                    }
               }
          }
     }
   return EINA_FALSE;
}

/**
 * Set the cursor position according to the y coord.
 *
 * @param cur the cur to be set.
 * @param y the coord to set by.
 * @return the line number found, -1 on error.
 */
EAPI int
evas_textblock_cursor_line_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord y)
{
   Evas_Object_Textblock *o;
   Evas_Object_Textblock_Paragraph *par, *found_par = NULL;
   Evas_Object_Textblock_Line *ln;

   if (!cur) return -1;
   o = (Evas_Object_Textblock *)(cur->obj->object_data);
   if (!o->formatted.valid) _relayout(cur->obj);
   y += o->style_pad.t;

   EINA_INLIST_FOREACH(o->paragraphs, par)
     {
        if ((par->y <= y) && (par->y + par->h > y))
          {
             found_par = par;
             break;
          }
     }

   if (found_par)
     {
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
              ti->text, &ti->text_props,
              start,
              &x1, &y, &w1, &h);
        if (!ret)
          {
             return NULL;
          }
        ret = cur->ENFN->font_pen_coords_get(cur->ENDT,
              ti->parent.format->font.font,
              ti->text, &ti->text_props,
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
             tr->x = ln->par->x + ln->x + it1->x + x;
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
                   ti->text, &ti->text_props,
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
             tr->x = ln->par->x + ln->x + it1->x + x;
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
             tr->x = ln->par->x + ln->x + min_x;
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
                   ti->text, &ti->text_props,
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
             tr->x = ln->par->x + ln->x + it2->x + x;
             tr->y = ln->par->y + ln->y;
             tr->h = ln->h;
             tr->w = w;
          }
     }
   return rects;
}
/**
 * Get the geometry of a range.
 *
 * @param cur1 one side of the range.
 * @param cur2 other side of the range.
 * @return a list of Rectangles representing the geometry of the range.
 */
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
	     tr->x = lni->par->x + lni->x;
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

/**
 * to be documented.
 * @param cur to be documented.
 * @param cx to be documented.
 * @param cy to be documented.
 * @param cw to be documented.
 * @param ch to be documented.
 * @return to be documented.
 */
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
   x = ln->par->x + ln->x + fi->parent.x;
   y = ln->par->y + ln->y + ln->baseline + fi->y;
   w = fi->parent.w;
   h = fi->parent.h;
   if (cx) *cx = x;
   if (cy) *cy = y;
   if (cw) *cw = w;
   if (ch) *ch = h;
   return EINA_TRUE;
}

/**
 * Checks if the cursor points to the end of the line.
 *
 * @param cur the cursor to check.
 * @return #EINA_TRUE if true, #EINA_FALSE otherwise.
 */
EAPI Eina_Bool
evas_textblock_cursor_eol_get(const Evas_Textblock_Cursor *cur)
{
   Eina_Bool ret = EINA_FALSE;
   Evas_Textblock_Cursor cur2;
   if (!cur) return EINA_FALSE;

   evas_textblock_cursor_copy(cur, &cur2);
   evas_textblock_cursor_line_char_last(&cur2);
   if (cur2.pos == cur->pos)
     {
        ret = EINA_TRUE;
     }
   return ret;
}

/* general controls */
/**
 * Get the geometry of a line number.
 *
 * @param obj the object.
 * @param line the line number.
 * @param cx x coord of the line.
 * @param cy y coord of the line.
 * @param cw w coord of the line.
 * @param ch h coord of the line.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
EAPI Eina_Bool
evas_object_textblock_line_number_geometry_get(const Evas_Object *obj, int line, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock_Line *ln;

   TB_HEAD_RETURN(0);
   ln = _find_layout_line_num(obj, line);
   if (!ln) return EINA_FALSE;
   if (cx) *cx = ln->par->x + ln->x;
   if (cy) *cy = ln->par->y + ln->y;
   if (cw) *cw = ln->w;
   if (ch) *ch = ln->h;
   return EINA_TRUE;
}

/**
 * Clear the textblock object.
 * @note Does *NOT* free the evas object itself.
 *
 * @param obj the object to clear.
 * @return nothing.
 */
EAPI void
evas_object_textblock_clear(Evas_Object *obj)
{
   Eina_List *l;
   Evas_Textblock_Cursor *cur;

   TB_HEAD();
   _nodes_clear(obj);
   o->cursor->node = NULL;
   o->cursor->pos = 0;
   EINA_LIST_FOREACH(o->cursors, l, cur)
     {
	cur->node = NULL;
	cur->pos = 0;

     }
   if (o->paragraphs)
     {
	_paragraphs_free(obj, o->paragraphs);
	o->paragraphs = NULL;
     }
   _evas_textblock_text_node_changed(o, obj, NULL);
}

/**
 * Get the formatted width and height. This calculates the actual size after restricting
 * the textblock to the current size of the object.
 * The main difference between this and @ref evas_object_textblock_size_native_get
 * is that the "native" function does not wrapping into account
 * it just calculates the real width of the object if it was placed on an
 * infinite canvas, while this function gives the size after wrapping
 * according to the size restrictions of the object.
 *
 * For example for a textblock containing the text: "You shall not pass!"
 * with no margins or padding and assuming a monospace font and a size of
 * 7x10 char widths (for simplicity) has a native size of 19x1
 * and a formatted size of 5x4.
 *
 *
 * @param obj the evas object.
 * @param w[out] the width of the object.
 * @param h[out] the height of the object
 * @return Returns no value.
 * @see evas_object_textblock_size_native_get
 */
EAPI void
evas_object_textblock_size_formatted_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   TB_HEAD();
   if (!o->formatted.valid) _relayout(obj);
   if (w) *w = o->formatted.w;
   if (h) *h = o->formatted.h;
}

/**
 * Get the native width and height. This calculates the actual size without taking account
 * the current size of the object.
 * The main difference between this and @ref evas_object_textblock_size_formatted_get
 * is that the "native" function does not take wrapping into account
 * it just calculates the real width of the object if it was placed on an
 * infinite canvas, while the "formatted" function gives the size after
 * wrapping text according to the size restrictions of the object.
 *
 * For example for a textblock containing the text: "You shall not pass!"
 * with no margins or padding and assuming a monospace font and a size of
 * 7x10 char widths (for simplicity) has a native size of 19x1
 * and a formatted size of 5x4.
 *
 * @param obj the evas object of the textblock
 * @param w[out] the width returned
 * @param h[out] the height returned
 * @return Returns no value.
 */
EAPI void
evas_object_textblock_size_native_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   TB_HEAD();
   if (!o->native.valid)
     {
	_layout(obj,
		1,
		-1, -1,
		&o->native.w, &o->native.h);
	o->native.valid = 1;
        o->content_changed = 0;
     }
   if (w) *w = o->native.w;
   if (h) *h = o->native.h;
}

/**
 * to be documented.
 * @param obj to be documented.
 * @param l to be documented.
 * @param r to be documented.
 * @param t to be documented.
 * @param b to be documented.
 * @return Returns no value.
 */
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
   o->magic = 0;
   EVAS_MEMPOOL_FREE(_mp_obj, o);
  _format_command_shutdown();
}


static void
evas_object_textblock_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Textblock_Paragraph *par;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock *o;
   int i, j;
   int pbacking = 0, backingx = 0;
   int punderline = 0, underlinex = 0;
   int punderline2 = 0, underline2x = 0;
   int pstrikethrough = 0, strikethroughx = 0;
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
   EINA_INLIST_FOREACH(o->paragraphs, par) \
     { \
        EINA_INLIST_FOREACH(par->lines, ln) \
          { \
             Evas_Object_Textblock_Item *itr; \
             \
             backingx = underlinex = underline2x = strikethroughx = 0; \
             pbacking = punderline = punderline2 = pstrikethrough = 0; \
             if (clip) \
               { \
                  if ((obj->cur.geometry.y + y + ln->par->y + ln->y + ln->h) < (cy - 20)) \
                  continue; \
                  if ((obj->cur.geometry.y + y + ln->par->y + ln->y) > (cy + ch + 20)) \
                  break; \
               } \
             EINA_INLIST_FOREACH(ln->items, itr) \
               { \
                  int yoff; \
                  yoff = ln->baseline; \
                  if (itr->format->valign != -1.0) \
                     yoff = (itr->format->valign * (double)(ln->h - itr->h)) + \
                     (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? \
                     _ITEM_TEXT(itr)->baseline : ln->baseline; \
                  if (clip) \
                    { \
                       if ((obj->cur.geometry.x + x + ln->par->x + ln->x + itr->x + itr->w) < (cx - 20)) \
                       continue; \
                       if ((obj->cur.geometry.x + x + ln->par->x + ln->x + itr->x) > (cx + cw + 20)) \
                       break; \
                    } \
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
         obj->cur.geometry.x + ln->par->x + ln->x + ti->parent.x + x + (ox), \
         obj->cur.geometry.y + ln->par->y + ln->y + yoff + y + (oy), \
         ti->parent.w, ti->parent.h, ti->parent.w, ti->parent.h, ti->text, &ti->text_props);
#define ITEM_WALK_LINE_SKIP_DROP() \
   if ((ln->par->y + ln->y + ln->h) <= 0) continue; \
   if (ln->par->y + ln->y > obj->cur.geometry.h) break


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
              obj->cur.geometry.x + ln->par->x + ln->x + x + (ox), \
              obj->cur.geometry.y + ln->par->y + ln->y + y + (oy), \
              (ow), \
              (oh)); \
     } \
   while (0)

#define DRAW_FORMAT(oname, oy, oh, or, og, ob, oa) \
   do \
     { \
        if ((p ## oname) && (itr->x > oname ## x)) \
          { \
             DRAW_RECT(oname ## x, oy, itr->x - oname ## x, oh, or, og, ob, \
                   oa); \
          } \
        \
        if (itr->format->oname) \
          { \
             p ## oname = 1; \
             or = itr->format->color.oname.r; \
             og = itr->format->color.oname.g; \
             ob = itr->format->color.oname.b; \
             oa = itr->format->color.oname.a; \
          } \
        else \
          { \
             p ## oname = 0; \
          } \
        \
        if (p ## oname && !EINA_INLIST_GET(itr)->next) \
          { \
             DRAW_RECT(itr->x, oy, itr->w, oh, or, og, ob, oa); \
          } \
        p ## oname = itr->format->oname; \
        oname ## x = itr->x; \
     } \
   while (0)


   pbacking = punderline = punderline2 = pstrikethrough = 0;
   ITEM_WALK()
     {
        ITEM_WALK_LINE_SKIP_DROP();

        DRAW_FORMAT(backing, 0, ln->h, r, g, b, a);
     }
   ITEM_WALK_END();

   /* There are size adjustments that depend on the styles drawn here back
    * in "_text_item_update_sizes" should not modify one without the other. */

   /* prepare everything for text draw */

   /* shadows */
   ITEM_WALK()
     {
        Evas_Object_Textblock_Text_Item *ti;
        ITEM_WALK_LINE_SKIP_DROP();
        ti = (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? _ITEM_TEXT(itr) : NULL;
        if (!ti) continue;

        if (ti->parent.format->style == EVAS_TEXT_STYLE_SHADOW)
          {
             COLOR_SET(shadow);
             DRAW_TEXT(1, 1);
          }
        else if ((ti->parent.format->style == EVAS_TEXT_STYLE_OUTLINE_SHADOW) ||
              (ti->parent.format->style == EVAS_TEXT_STYLE_FAR_SHADOW))
          {
             COLOR_SET(shadow);
             DRAW_TEXT(2, 2);
          }
        else if ((ti->parent.format->style == EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW) ||
              (ti->parent.format->style == EVAS_TEXT_STYLE_FAR_SOFT_SHADOW))
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
        else if (ti->parent.format->style == EVAS_TEXT_STYLE_SOFT_SHADOW)
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
     }
   ITEM_WALK_END();

   /* glows */
   ITEM_WALK()
     {
        Evas_Object_Textblock_Text_Item *ti;
        ITEM_WALK_LINE_SKIP_DROP();
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
        ITEM_WALK_LINE_SKIP_DROP();
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
        ITEM_WALK_LINE_SKIP_DROP();
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
   if ((o->changed) || (o->content_changed) ||
       ((obj->cur.geometry.w != o->last_w) ||
           (((o->valign != 0.0) || (o->have_ellipsis)) &&
               (obj->cur.geometry.h != o->last_h))))
     {
	o->formatted.valid = 0;
	_layout(obj,
		0,
		obj->cur.geometry.w, obj->cur.geometry.h,
		&o->formatted.w, &o->formatted.h);
	o->formatted.valid = 1;
	o->last_w = obj->cur.geometry.w;
	o->last_h = obj->cur.geometry.h;
	o->redraw = 0;
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	o->changed = 0;
	o->content_changed = 0;
	is_v = evas_object_is_visible(obj);
	was_v = evas_object_was_visible(obj);
	goto done;
     }
   if (o->redraw)
     {
	o->redraw = 0;
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	o->changed = 0;
	o->content_changed = 0;
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
   if (o->changed || o->content_changed)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	o->changed = 0;
	o->content_changed = 0;
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
   _relayout(obj);
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
                            evas_common_pipe_op_text_flush(ti->parent.format->font.font);
#endif
                            evas_font_load_hinting_set(obj->layer->evas,
                                  ti->parent.format->font.font,
                                  obj->layer->evas->hinting);
                         }
                    }
               }
          }
     }
   _evas_textblock_text_node_changed(o, obj, NULL);
}

/**
 * @}
 */


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
        printf("Text: '%ls'\n", ti->text);
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

