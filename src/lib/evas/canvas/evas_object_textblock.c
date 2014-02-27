/**
 * @internal
 * @subsection Evas_Object_Textblock_Internal Internal Textblock Object Tutorial
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
 * inside the same format node, although it consists of different formatting
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
#include "evas_common_private.h"
#include "evas_private.h"
#include <stdlib.h>

#include "Eo.h"

//#define LYDBG(f, args...) printf(f, ##args)
#define LYDBG(f, args...)

#define MY_CLASS EVAS_OBJ_TEXTBLOCK_CLASS

#define MY_CLASS_NAME "Evas_Textblock"

#include "linebreak.h"
#include "wordbreak.h"

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

/* private magic number for textblock objects */
static const char o_type[] = "textblock";

/* The char to be inserted instead of visible formats */
#define _REPLACEMENT_CHAR 0xFFFC
#define _PARAGRAPH_SEPARATOR 0x2029
#define _NEWLINE '\n'
#define _TAB '\t'

#define _REPLACEMENT_CHAR_UTF8 "\xEF\xBF\xBC"
#define _PARAGRAPH_SEPARATOR_UTF8 "\xE2\x80\xA9"
#define _NEWLINE_UTF8 "\n"
#define _TAB_UTF8 "\t"
#define EVAS_TEXTBLOCK_IS_VISIBLE_FORMAT_CHAR(ch) \
   (((ch) == _REPLACEMENT_CHAR) || \
    ((ch) ==  _NEWLINE) || \
    ((ch) == _TAB) || \
    ((ch) == _PARAGRAPH_SEPARATOR))

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(EINA_LOG_DOMAIN_DEFAULT, __VA_ARGS__)

#define TB_NULL_CHECK(null_check, ...) \
   do \
     { \
        if (!null_check) \
          { \
             ERR("%s is NULL while it shouldn't be, please notify developers.", #null_check); \
             return __VA_ARGS__; \
          } \
     } \
   while(0)

/* private struct for textblock object internal data */
/**
 * @internal
 * @typedef Evas_Textblock_Data
 * The actual textblock object.
 */
typedef struct _Evas_Object_Textblock             Evas_Textblock_Data;
/**
 * @internal
 * @typedef Evas_Object_Style_Tag
 * The structure used for finding style tags.
 */
typedef struct _Evas_Object_Style_Tag             Evas_Object_Style_Tag;
/**
 * @internal
 * @typedef Evas_Object_Style_Tag
 * The structure used for finding style tags.
 */
typedef struct _Evas_Object_Style_Tag_Base        Evas_Object_Style_Tag_Base;
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
   (!strncmp(base, closer, closer_len) && \
    (!base[closer_len] || \
     (base[closer_len] == '=') || \
     _is_white(base[closer_len])))

/*FIXME: document the structs and struct items. */
struct _Evas_Object_Style_Tag_Base
{
   char *tag;
   char *replace;
   size_t tag_len;
   size_t replace_len;
};

struct _Evas_Object_Style_Tag
{
   EINA_INLIST;
   Evas_Object_Style_Tag_Base tag;
};

struct _Evas_Object_Textblock_Node_Text
{
   EINA_INLIST;
   Eina_UStrbuf                       *unicode;
   char                               *utf8;
   Evas_Object_Textblock_Node_Format  *format_node;
   Evas_Object_Textblock_Paragraph    *par;
   Eina_Bool                           dirty : 1;
   Eina_Bool                           is_new : 1;
};

struct _Evas_Object_Textblock_Node_Format
{
   EINA_INLIST;
   const char                         *format;
   const char                         *orig_format;
   Evas_Object_Textblock_Node_Text    *text_node;
   size_t                              offset;
   struct {
      unsigned char l, r, t, b;
   } pad;
   unsigned char                       anchor : 2;
   Eina_Bool                           opener : 1;
   Eina_Bool                           own_closer : 1;
   Eina_Bool                           visible : 1;
   Eina_Bool                           format_change : 1;
   Eina_Bool                           is_new : 1;
};

/* The default tags to use */
static const Evas_Object_Style_Tag_Base default_tags[] = {
          { "b", "+ font_weight=Bold", 1, 18 },
          { "i", "+ font_style=Italic", 1, 19 }};

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
   Evas_Object_Textblock_Line        *lines;
   Evas_Object_Textblock_Node_Text   *text_node;
   Eina_List                         *logical_items;
   Evas_BiDi_Paragraph_Props         *bidi_props; /* Only valid during layout */
   Evas_BiDi_Direction                direction;
   Evas_Coord                         y, w, h;
   int                                line_no;
   Eina_Bool                          is_bidi : 1;
   Eina_Bool                          visible : 1;
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
   Evas_Object_Textblock_Node_Text     *text_node;
   Evas_Object_Textblock_Format        *format;
   Evas_Object_Textblock_Line          *ln;
   size_t                               text_pos;
#ifdef BIDI_SUPPORT
   size_t                               visual_pos;
#endif
   Evas_Textblock_Item_Type             type;

   Evas_Coord                           adv, x, w, h;
   Evas_Coord                           yoff;
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
      Evas_Font_Description *fdesc;
      const char       *source;
      Evas_Font_Set    *font;
      Evas_Font_Size    size;
   } font;
   struct {
      struct {
	 unsigned char  r, g, b, a;
      } normal, underline, underline2, underline_dash, outline, shadow, glow, glow2, backing,
	strikethrough;
   } color;
   struct {
      int               l, r;
   } margin;
   int                  ref;
   int                  tabstops;
   int                  linesize;
   int                  linegap;
   int                  underline_dash_width;
   int                  underline_dash_gap;
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
   Eina_Bool            underline_dash : 1;
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

/* Size of the index array */
#define TEXTBLOCK_PAR_INDEX_SIZE 10
struct _Evas_Object_Textblock
{
   DATA32                              magic;
   Evas_Textblock_Style               *style;
   Evas_Textblock_Style               *style_user;
   Evas_Textblock_Cursor              *cursor;
   Eina_List                          *cursors;
   Evas_Object_Textblock_Node_Text    *text_nodes;
   Evas_Object_Textblock_Node_Format  *format_nodes;

   int                                 num_paragraphs;
   Evas_Object_Textblock_Paragraph    *paragraphs;
   Evas_Object_Textblock_Paragraph    *par_index[TEXTBLOCK_PAR_INDEX_SIZE];

   Evas_Object_Textblock_Text_Item    *ellip_ti;
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
      int                              w, h, oneline_h;
      Eina_Bool                        valid : 1;
   } formatted, native;
   Eina_Bool                           redraw : 1;
   Eina_Bool                           changed : 1;
   Eina_Bool                           content_changed : 1;
   Eina_Bool                           format_changed : 1;
   Eina_Bool                           have_ellipsis : 1;
   Eina_Bool                           legacy_newline : 1;
};

/* private methods for textblock objects */
static void evas_object_textblock_init(Evas_Object *eo_obj);
static void evas_object_textblock_render(Evas_Object *eo_obj,
					 Evas_Object_Protected_Data *obj,
					 void *type_private_data,
					 void *output, void *context, void *surface,
					 int x, int y, Eina_Bool do_async);
static void evas_object_textblock_free(Evas_Object *eo_obj);
static void evas_object_textblock_render_pre(Evas_Object *eo_obj,
					     Evas_Object_Protected_Data *obj,
					     void *type_private_data);
static void evas_object_textblock_render_post(Evas_Object *eo_obj,
					      Evas_Object_Protected_Data *obj,
					      void *type_private_data);
static Evas_Object_Textblock_Node_Text *_evas_textblock_node_text_new(void);

static unsigned int evas_object_textblock_id_get(Evas_Object *eo_obj);
static unsigned int evas_object_textblock_visual_id_get(Evas_Object *eo_obj);
static void *evas_object_textblock_engine_data_get(Evas_Object *eo_obj);

static int evas_object_textblock_is_opaque(Evas_Object *eo_obj,
					   Evas_Object_Protected_Data *obj,
					   void *type_private_data);
static int evas_object_textblock_was_opaque(Evas_Object *eo_obj,
					    Evas_Object_Protected_Data *obj,
					    void *type_private_data);
static void evas_object_textblock_coords_recalc(Evas_Object *eo_obj,
						Evas_Object_Protected_Data *obj,
						void *type_private_data);
static void evas_object_textblock_scale_update(Evas_Object *eo_obj,
					       Evas_Object_Protected_Data *obj,
					       void *type_private_data);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
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
     NULL, /*evas_object_textblock_coords_recalc, <- disable - not useful. */
     evas_object_textblock_scale_update,
     NULL,
     NULL,
     NULL
};

/* the actual api call to add a textblock */

#define TB_HEAD() \
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ); \
   return; \
   MAGIC_CHECK_END(); \
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

#define TB_HEAD_RETURN(x) \
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ); \
   return (x); \
   MAGIC_CHECK_END();

static Eina_Bool _evas_textblock_cursor_is_at_the_end(const Evas_Textblock_Cursor *cur);
static void _evas_textblock_node_text_remove(Evas_Textblock_Data *o, Evas_Object_Textblock_Node_Text *n);
static Evas_Object_Textblock_Node_Format *_evas_textblock_cursor_node_format_before_or_at_pos_get(const Evas_Textblock_Cursor *cur);
static size_t _evas_textblock_node_format_pos_get(const Evas_Object_Textblock_Node_Format *fmt);
static void _evas_textblock_node_format_remove(Evas_Textblock_Data *o, Evas_Object_Textblock_Node_Format *n, int visual_adjustment);
static void _evas_textblock_node_format_free(Evas_Textblock_Data *o, Evas_Object_Textblock_Node_Format *n);
static void _evas_textblock_node_text_free(Evas_Object_Textblock_Node_Text *n);
static void _evas_textblock_changed(Evas_Textblock_Data *o, Evas_Object *eo_obj);
static void _evas_textblock_invalidate_all(Evas_Textblock_Data *o);
static void _evas_textblock_cursors_update_offset(const Evas_Textblock_Cursor *cur, const Evas_Object_Textblock_Node_Text *n, size_t start, int offset);
static void _evas_textblock_cursors_set_node(Evas_Textblock_Data *o, const Evas_Object_Textblock_Node_Text *n, Evas_Object_Textblock_Node_Text *new_node);

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
	free(tag->tag.tag);
	free(tag->tag.replace);
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
_style_match_tag(const Evas_Textblock_Style *ts, const char *s, size_t tag_len, size_t *replace_len)
{
   Evas_Object_Style_Tag *tag;

   /* Try the style tags */
   EINA_INLIST_FOREACH(ts->tags, tag)
     {
	if (tag->tag.tag_len != tag_len) continue;
	if (!strncmp(tag->tag.tag, s, tag_len))
	  {
	     *replace_len = tag->tag.replace_len;
	     return tag->tag.replace;
	  }
     }

   /* Try the default tags */
   {
      size_t i;
      const Evas_Object_Style_Tag_Base *btag;
      for (btag = default_tags, i = 0 ;
            i < (sizeof(default_tags) / sizeof(default_tags[0])) ;
            btag++, i++)
        {
           if (btag->tag_len != tag_len) continue;
           if (!strncmp(btag->tag, s, tag_len))
             {
                *replace_len = btag->replace_len;
                return btag->replace;
             }
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
_nodes_clear(const Evas_Object *eo_obj)
{
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
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
_format_unref_free(const Evas_Object *eo_obj, Evas_Object_Textblock_Format *fmt)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   fmt->ref--;
   if (fmt->ref > 0) return;
   if (fmt->font.fdesc) evas_font_desc_unref(fmt->font.fdesc);
   if (fmt->font.source) eina_stringshare_del(fmt->font.source);
   evas_font_free(obj->layer->evas->evas, fmt->font.font);
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
_item_free(const Evas_Object *eo_obj, Evas_Object_Textblock_Line *ln, Evas_Object_Textblock_Item *it)
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
   _format_unref_free(eo_obj, it->format);
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

/**
 * @internal
 * Checks if a char is a whitespace.
 * @param c the unicode codepoint.
 * @return @c EINA_TRUE if the unicode codepoint is a whitespace, @c EINA_FALSE
 * otherwise.
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

/* The refcount for the formats. */
static int format_refcount = 0;
/* Holders for the stringshares */
static const char *fontstr = NULL;
static const char *font_fallbacksstr = NULL;
static const char *font_sizestr = NULL;
static const char *font_sourcestr = NULL;
static const char *font_weightstr = NULL;
static const char *font_stylestr = NULL;
static const char *font_widthstr = NULL;
static const char *langstr = NULL;
static const char *colorstr = NULL;
static const char *underline_colorstr = NULL;
static const char *underline2_colorstr = NULL;
static const char *underline_dash_colorstr = NULL;
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
static const char *underline_dash_widthstr = NULL;
static const char *underline_dash_gapstr = NULL;

/**
 * @page evas_textblock_style_page Evas Textblock Style Options
 *
 * @brief This page describes how to style text in an Evas Text Block.
 */

/**
 * @internal
 * Init the format strings.
 */
static void
_format_command_init(void)
{
   if (format_refcount == 0)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @section evas_textblock_style_index Index
         *
         * The following styling commands are accepted:
         * @li @ref evas_textblock_style_font
         * @li @ref evas_textblock_style_font_fallback
         * @li @ref evas_textblock_style_font_size
         * @li @ref evas_textblock_style_font_source
         * @li @ref evas_textblock_style_font_weight
         * @li @ref evas_textblock_style_font_style
         * @li @ref evas_textblock_style_font_width
         * @li @ref evas_textblock_style_lang
         * @li @ref evas_textblock_style_color
         * @li @ref evas_textblock_style_underline_color
         * @li @ref evas_textblock_style_underline2_color
         * @li @ref evas_textblock_style_underline_dash_color
         * @li @ref evas_textblock_style_outline_color
         * @li @ref evas_textblock_style_shadow_color
         * @li @ref evas_textblock_style_glow_color
         * @li @ref evas_textblock_style_glow2_color
         * @li @ref evas_textblock_style_backing_color
         * @li @ref evas_textblock_style_strikethrough_color
         * @li @ref evas_textblock_style_align
         * @li @ref evas_textblock_style_valign
         * @li @ref evas_textblock_style_wrap
         * @li @ref evas_textblock_style_left_margin
         * @li @ref evas_textblock_style_right_margin
         * @li @ref evas_textblock_style_underline
         * @li @ref evas_textblock_style_strikethrough
         * @li @ref evas_textblock_style_backing
         * @li @ref evas_textblock_style_style
         * @li @ref evas_textblock_style_tabstops
         * @li @ref evas_textblock_style_linesize
         * @li @ref evas_textblock_style_linerelsize
         * @li @ref evas_textblock_style_linegap
         * @li @ref evas_textblock_style_linerelgap
         * @li @ref evas_textblock_style_item
         * @li @ref evas_textblock_style_linefill
         * @li @ref evas_textblock_style_ellipsis
         * @li @ref evas_textblock_style_password
         * @li @ref evas_textblock_style_underline_dash_width
         * @li @ref evas_textblock_style_underline_dash_gap
         *
         * @section evas_textblock_style_contents Contents
         */
        fontstr = eina_stringshare_add("font");
        font_fallbacksstr = eina_stringshare_add("font_fallbacks");
        font_sizestr = eina_stringshare_add("font_size");
        font_sourcestr = eina_stringshare_add("font_source");
        font_weightstr = eina_stringshare_add("font_weight");
        font_stylestr = eina_stringshare_add("font_style");
        font_widthstr = eina_stringshare_add("font_width");
        langstr = eina_stringshare_add("lang");
        colorstr = eina_stringshare_add("color");
        underline_colorstr = eina_stringshare_add("underline_color");
        underline2_colorstr = eina_stringshare_add("underline2_color");
        underline_dash_colorstr = eina_stringshare_add("underline_dash_color");
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
        underline_dash_widthstr = eina_stringshare_add("underline_dash_width");
        underline_dash_gapstr = eina_stringshare_add("underline_dash_gap");
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
   eina_stringshare_del(font_weightstr);
   eina_stringshare_del(font_stylestr);
   eina_stringshare_del(font_widthstr);
   eina_stringshare_del(langstr);
   eina_stringshare_del(colorstr);
   eina_stringshare_del(underline_colorstr);
   eina_stringshare_del(underline2_colorstr);
   eina_stringshare_del(underline_dash_colorstr);
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
   eina_stringshare_del(underline_dash_widthstr);
   eina_stringshare_del(underline_dash_gapstr);
}

/**
 * @internal
 * Copies str to dst while removing the \\ char, i.e unescape the escape sequences.
 *
 * @param[out] dst the destination string - Should not be NULL.
 * @param[in] src the source string - Should not be NULL.
 */
static int
_format_clean_param(Eina_Tmpstr *s)
{
   Eina_Tmpstr *ss;
   char *ds;
   int len = 0;

   ds = (char*) s;
   for (ss = s; *ss; ss++, ds++, len++)
     {
        if ((*ss == '\\') && *(ss + 1)) ss++;
        if (ds != ss) *ds = *ss;
     }
   *ds = 0;

   return len;
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
_format_command(Evas_Object *eo_obj, Evas_Object_Textblock_Format *fmt, const char *cmd, Eina_Tmpstr *param)
{
   int len;

   len = _format_clean_param(param);

   /* If we are changing the font, create the fdesc. */
   if ((cmd == font_weightstr) || (cmd == font_widthstr) ||
         (cmd == font_stylestr) || (cmd == langstr) ||
         (cmd == fontstr) || (cmd == font_fallbacksstr))
     {
        if (!fmt->font.fdesc)
          {
             fmt->font.fdesc = evas_font_desc_new();
          }
        else if (!fmt->font.fdesc->is_new)
          {
             Evas_Font_Description *old = fmt->font.fdesc;
             fmt->font.fdesc = evas_font_desc_dup(fmt->font.fdesc);
             if (old) evas_font_desc_unref(old);
          }
     }


   if (cmd == fontstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_font Font
         *
         * This sets the name of the font to be used.
         * @code
         * font=<font name>
         * @endcode
         */
        evas_font_name_parse(fmt->font.fdesc, param);
     }
   else if (cmd == font_fallbacksstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_font_fallback Font fallback
         *
         * This sets the name of the fallback font to be used. This font will
         * be used if the primary font is not available.
         * @code
         * font_fallbacks=<font name>
         * @endcode
         */
        eina_stringshare_replace(&(fmt->font.fdesc->fallbacks), param);
     }
   else if (cmd == font_sizestr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_font_size Font size
         *
         * This sets the the size of font in points to be used.
         * @code
         * font_size=<size>
         * @endcode
         */
        int v;

        v = atoi(param);
        if (v != fmt->font.size)
          {
             fmt->font.size = v;
          }
     }
   else if (cmd == font_sourcestr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_font_source Font source
         *
         * Specify an object from which to search for the font.
         * @code
         * font_source=<source>
         * @endcode
         */
        if ((!fmt->font.source) ||
              ((fmt->font.source) && (strcmp(fmt->font.source, param))))
          {
             eina_stringshare_replace(&(fmt->font.source), param);
          }
     }
   else if (cmd == font_weightstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_font_weight Font weight
         *
         * Sets the weight of the font. The value must be one of:
         * @li "normal"
         * @li "thin"
         * @li "ultralight"
         * @li "light"
         * @li "book"
         * @li "medium"
         * @li "semibold"
         * @li "bold"
         * @li "ultrabold"
         * @li "black"
         * @li "extrablack"
         * @code
         * font_weight=<weight>
         * @endcode
         */
        fmt->font.fdesc->weight = evas_font_style_find(param,
                                                       param + len,
                                                       EVAS_FONT_STYLE_WEIGHT);
     }
   else if (cmd == font_stylestr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_font_style Font style
         *
         * Sets the style of the font. The value must be one of:
         * @li "normal"
         * @li "oblique"
         * @li "italic"
         * @code
         * font_style=<style>
         * @endcode
         */
        fmt->font.fdesc->slant = evas_font_style_find(param,
                                                      param + len,
                                                      EVAS_FONT_STYLE_SLANT);
     }
   else if (cmd == font_widthstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_font_width Font width
         *
         * Sets the width of the font. The value must be one of:
         * @li "normal"
         * @li "ultracondensed"
         * @li "extracondensed"
         * @li "condensed"
         * @li "semicondensed"
         * @li "semiexpanded"
         * @li "expanded"
         * @li "extraexpanded"
         * @li "ultraexpanded"
         * @code
         * font_width=<width>
         * @endcode
         */
        fmt->font.fdesc->width = evas_font_style_find(param,
                                                      param + len,
                                                      EVAS_FONT_STYLE_WIDTH);
     }
   else if (cmd == langstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_lang Language
         *
         * Sets the language of the text for FontConfig.
         * @code
         * lang=<language>
         * @endcode
         */
        eina_stringshare_replace(&(fmt->font.fdesc->lang), param);
     }
   else if (cmd == colorstr)
     /**
      * @page evas_textblock_style_page Evas Textblock Style Options
      *
      * @subsection evas_textblock_style_color Color
      *
      * Sets the color of the text. The following formats are accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.normal.r), &(fmt->color.normal.g),
           &(fmt->color.normal.b), &(fmt->color.normal.a));
   else if (cmd == underline_colorstr)
     /**
      * @page evas_textblock_style_page Evas Textblock Style Options
      *
      * @subsection evas_textblock_style_underline_color Underline Color
      *
      * Sets the color of the underline. The following formats are accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * underline_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.underline.r), &(fmt->color.underline.g),
           &(fmt->color.underline.b), &(fmt->color.underline.a));
   else if (cmd == underline2_colorstr)
     /**
      * @page evas_textblock_style_page Evas Textblock Style Options
      *
      * @subsection evas_textblock_style_underline2_color Second Underline Color
      *
      * Sets the color of the second line of underline(when using underline
      * mode "double"). The following formats are accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * underline2_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.underline2.r), &(fmt->color.underline2.g),
           &(fmt->color.underline2.b), &(fmt->color.underline2.a));
   else if (cmd == underline_dash_colorstr)
     /**
      * @page evas_textblock_style_page Evas Textblock Style Options
      *
      * @subsection evas_textblock_style_underline_dash_color Underline Dash Color
      *
      * Sets the color of dashed underline. The following formats are accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * underline_dash_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.underline_dash.r), &(fmt->color.underline_dash.g),
           &(fmt->color.underline_dash.b), &(fmt->color.underline_dash.a));
   else if (cmd == outline_colorstr)
     /**
      * @page evas_textblock_style_page Evas Textblock Style Options
      *
      * @subsection evas_textblock_style_outline_color Outline Color
      *
      * Sets the color of the outline of the text. The following formats are
      * accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * outline_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.outline.r), &(fmt->color.outline.g),
           &(fmt->color.outline.b), &(fmt->color.outline.a));
   else if (cmd == shadow_colorstr)
     /**
      * @page evas_textblock_style_page Evas Textblock Style Options
      *
      * @subsection evas_textblock_style_shadow_color Shadow Color
      *
      * Sets the color of the shadow of the text. The following formats are
      * accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * shadow_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.shadow.r), &(fmt->color.shadow.g),
           &(fmt->color.shadow.b), &(fmt->color.shadow.a));
   else if (cmd == glow_colorstr)
     /**
      * @page evas_textblock_style_page Evas Textblock Style Options
      *
      * @subsection evas_textblock_style_glow_color First Glow Color
      *
      * Sets the first color of the glow of text. The following formats are
      * accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * glow_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.glow.r), &(fmt->color.glow.g),
           &(fmt->color.glow.b), &(fmt->color.glow.a));
   else if (cmd == glow2_colorstr)
     /**
      * @page evas_textblock_style_page Evas Textblock Style Options
      *
      * @subsection evas_textblock_style_glow2_color Second Glow Color
      *
      * Sets the second color of the glow of text. The following formats are
      * accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * glow2_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.glow2.r), &(fmt->color.glow2.g),
           &(fmt->color.glow2.b), &(fmt->color.glow2.a));
   else if (cmd == backing_colorstr)
     /**
      * @page evas_textblock_style_page Evas Textblock Style Options
      *
      * @subsection evas_textblock_style_backing_color Backing Color
      *
      * Sets a background color for text. The following formats are
      * accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * backing_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.backing.r), &(fmt->color.backing.g),
           &(fmt->color.backing.b), &(fmt->color.backing.a));
   else if (cmd == strikethrough_colorstr)
     /**
      * @page evas_textblock_style_page Evas Textblock Style Options
      *
      * @subsection evas_textblock_style_strikethrough_color Strikethrough Color
      *
      * Sets the color of text that is striked through. The following formats
      * are accepted:
      * @li "#RRGGBB"
      * @li "#RRGGBBAA"
      * @li "#RGB"
      * @li "#RGBA"
      * @code
      * strikethrough_color=<color>
      * @endcode
      */
     evas_common_format_color_parse(param, len,
           &(fmt->color.strikethrough.r), &(fmt->color.strikethrough.g),
           &(fmt->color.strikethrough.b), &(fmt->color.strikethrough.a));
   else if (cmd == alignstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_align Horizontal Align
         *
         * Sets the horizontal alignment of the text. The value can either be
         * a number, a percentage or one of several presets:
         * @li "auto" - Respects LTR/RTL settings
         * @li "center" - Centers the text in the line
         * @li "middle" - Alias for "center"
         * @li "left" - Puts the text at the left of the line
         * @li "right" - Puts the text at the right of the line
         * @li <number> - A number between 0.0 and 1.0 where 0.0 represents
         * "left" and 1.0 represents "right"
         * @li <number>% - A percentage between 0% and 100% where 0%
         * represents "left" and 100% represents "right"
         * @code
         * align=<value or preset>
         * @endcode
         */
        if (len == 4 && !strcmp(param, "auto"))
          {
             fmt->halign_auto = EINA_TRUE;
          }
        else
          {
             static const struct {
                const char *param;
                int len;
                double halign;
             } halign_named[] = {
               { "middle", 6, 0.5 },
               { "center", 6, 0.5 },
               { "left", 4, 0.0 },
               { "right", 5, 1.0 },
               { NULL, 0, 0.0 }
             };
             unsigned int i;

             for (i = 0; halign_named[i].param; i++)
               if (len == halign_named[i].len &&
                   !strcmp(param, halign_named[i].param))
                 {
                    fmt->halign = halign_named[i].halign;
                    break;
                 }

             if (halign_named[i].param == NULL)
               {
                  char *endptr = NULL;
                  double val = strtod(param, &endptr);
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
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_valign Vertical Align
         *
         * Sets the vertical alignment of the text. The value can either be
         * a number or one of the following presets:
         * @li "top" - Puts text at the top of the line
         * @li "center" - Centers the text in the line
         * @li "middle" - Alias for "center"
         * @li "bottom" - Puts the text at the bottom of the line
         * @li "baseline" - Baseline
         * @li "base" - Alias for "baseline"
         * @li <number> - A number between 0.0 and 1.0 where 0.0 represents
         * "top" and 1.0 represents "bottom"
         * @li <number>% - A percentage between 0% and 100% where 0%
         * represents "top" and 100% represents "bottom"
         * @code
         * valign=<value or preset>
         * @endcode
         *
         * See explanation of baseline at:
         * https://en.wikipedia.org/wiki/Baseline_%28typography%29
         */
        static const struct {
           const char *param;
           int len;
           double valign;
        } valign_named[] = {
          { "top", 3, 0.0 },
          { "middle", 6, 0.5 },
          { "center", 6, 0.5 },
          { "bottom", 6, 1.0 },
          { "baseline", 8, -1.0 },
          { "base", 4, -1.0 },
          { NULL, 0, 0 }
        };
        unsigned int i;

        for (i = 0; valign_named[i].param; i++)
          if (len == valign_named[i].len &&
              !strcmp(valign_named[i].param, param))
            {
               fmt->valign = valign_named[i].valign;
               break;
            }

        if (valign_named[i].param == NULL)
          {
             char *endptr = NULL;
             double val = strtod(param, &endptr);
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
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_wrap Wrap
         *
         * Sets the wrap policy of the text. The value must be one of the
         * following:
         * @li "word" - Only wraps lines at word boundaries
         * @li "char" - Wraps at any character
         * @li "mixed" - Wrap at words if possible, if not at any character
         * @li "" - Don't wrap
         * @code
         * wrap=<value or preset>
         * @endcode
         */
        static const struct {
           const char *param;
           int len;
           Eina_Bool wrap_word;
           Eina_Bool wrap_char;
           Eina_Bool wrap_mixed;
        } wrap_named[] = {
          { "word", 4, 1, 0, 0 },
          { "char", 4, 0, 1, 0 },
          { "mixed", 5, 0, 0, 1 },
          { NULL, 0, 0, 0, 0 }
        };
        unsigned int i;

        fmt->wrap_word = fmt->wrap_mixed = fmt->wrap_char = 0;
        for (i = 0; wrap_named[i].param; i++)
          if (wrap_named[i].len == len &&
              !strcmp(wrap_named[i].param, param))
            {
               fmt->wrap_word = wrap_named[i].wrap_word;
               fmt->wrap_char = wrap_named[i].wrap_char;
               fmt->wrap_mixed = wrap_named[i].wrap_mixed;
               break;
            }
     }
   else if (cmd == left_marginstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_left_margin Left margin
         *
         * Sets the left margin of the text. The value can be a number, an
         * increment, decrement or "reset":
         * @li +<number> - Increments existing left margin by <number>
         * @li -<number> - Decrements existing left margin by <number>
         * @li <number> - Sets left margin to <number>
         * @li "reset" - Sets left margin to 0
         * @code
         * left_margin=<value or reset>
         * @endcode
         */
        if (len == 5 && !strcmp(param, "reset"))
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
   else if (cmd == right_marginstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_right_margin Right margin
         *
         * Sets the right margin of the text. The value can be a number, an
         * increment, decrement or "reset":
         * @li +<number> - Increments existing right margin by <number>
         * @li -<number> - Decrements existing right margin by <number>
         * @li <number> - Sets left margin to <number>
         * @li "reset" - Sets left margin to 0
         * @code
         * right_margin=<value or reset>
         * @endcode
         */
        if (len == 5 && !strcmp(param, "reset"))
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
   else if (cmd == underlinestr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_underline Underline
         *
         * Sets if and how a text will be underlined. The value must be one of
         * the following:
         * @li "off" - No underlining
         * @li "single" - A single line under the text
         * @li "on" - Alias for "single"
         * @li "double" - Two lines under the text
         * @li "dashed" - A dashed line under the text
         * @code
         * underline=off/single/on/double/dashed
         * @endcode
         */
        static const struct {
           const char *param;
           int len;
           Eina_Bool underline;
           Eina_Bool underline2;
           Eina_Bool underline_dash;
        } underlines_named[] = {
          { "off", 3, 0, 0, 0 },
          { "on", 2, 1, 0, 0 },
          { "single", 6, 1, 0, 0 },
          { "double", 6, 1, 1, 0 },
          { "dashed", 6, 0, 0, 1 },
          { NULL, 0, 0, 0, 0 }
        };
        unsigned int i;

        fmt->underline = fmt->underline2 = fmt->underline_dash = 0;
        for (i = 0; underlines_named[i].param; ++i)
          if (underlines_named[i].len == len &&
              !strcmp(underlines_named[i].param, param))
            {
               fmt->underline = underlines_named[i].underline;
               fmt->underline2 = underlines_named[i].underline2;
               fmt->underline_dash = underlines_named[i].underline_dash;
               break;
            }
     }
   else if (cmd == strikethroughstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_strikethrough Strikethrough
         *
         * Sets if the text will be striked through. The value must be one of
         * the following:
         * @li "off" - No strikethrough
         * @li "on" - Strikethrough
         * @code
         * strikethrough=on/off
         * @endcode
         */
        if (len == 3 && !strcmp(param, "off"))
          fmt->strikethrough = 0;
        else if (len == 2 && !strcmp(param, "on"))
          fmt->strikethrough = 1;
     }
   else if (cmd == backingstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_backing Backing
         *
         * Sets if the text will have backing. The value must be one of
         * the following:
         * @li "off" - No backing
         * @li "on" - Backing
         * @code
         * backing=on/off
         * @endcode
         */
        if (len == 3 && !strcmp(param, "off"))
          fmt->backing = 0;
        else if (len == 2 && !strcmp(param, "on"))
          fmt->backing = 1;
     }
   else if (cmd == stylestr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_style Style
         *
         * Sets the style of the text. The value must be a string composed of
         * two comma separated parts. The first part of the value sets the
         * appearance of the text, the second the position.
         *
         * The first part may be any of the following values:
         * @li "plain"
         * @li "off" - Alias for "plain"
         * @li "none" - Alias for "plain"
         * @li "shadow"
         * @li "outline"
         * @li "soft_outline"
         * @li "outline_shadow"
         * @li "outline_soft_shadow"
         * @li "glow"
         * @li "far_shadow"
         * @li "soft_shadow"
         * @li "far_soft_shadow"
         * The second part may be any of the following values:
         * @li "bottom_right"
         * @li "bottom"
         * @li "bottom_left"
         * @li "left"
         * @li "top_left"
         * @li "top"
         * @li "top_right"
         * @li "right"
         * @code
         * style=<appearance>,<position>
         * @endcode
         */
        const char *p;
        char *p1, *p2, *pp;

        p2 = alloca(len + 1);
        *p2 = 0;
        /* no comma */
        if (!strstr(param, ",")) p1 = (char*) param;
        else
          {
             p1 = alloca(len + 1);
             *p1 = 0;

             /* split string "str1,str2" into p1 and p2 (if we have more than
              * 1 str2 eg "str1,str2,str3,str4" then we don't care. p2 just
              * ends up being the last one as right now it's only valid to have
              * 1 comma and 2 strings */
             pp = p1;
             for (p = param; *p; p++)
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
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_tabstops Tabstops
         *
         * Sets the size of the tab character. The value must be a number
         * greater than one.
         * @code
         * tabstops=<number>
         * @endcode
         */
        fmt->tabstops = atoi(param);
        if (fmt->tabstops < 1) fmt->tabstops = 1;
     }
   else if (cmd == linesizestr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_linesize Line size
         *
         * Sets the size of line of text. The value should be a number.
         * @warning Setting this value sets linerelsize to 0%!
         * @code
         * linesize=<number>
         * @endcode
         */
        fmt->linesize = atoi(param);
        fmt->linerelsize = 0.0;
     }
   else if (cmd == linerelsizestr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_linerelsize Relative line size
         *
         * Sets the relative size of line of text. The value must be a
         * percentage.
         * @warning Setting this value sets linesize to 0!
         * @code
         * linerelsize=<number>%
         * @endcode
         */
        char *endptr = NULL;
        double val = strtod(param, &endptr);
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
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_linegap Line gap
         *
         * Sets the size of the line gap in text. The value should be a
         * number.
         * @warning Setting this value sets linerelgap to 0%!
         * @code
         * linegap=<number>
         * @endcode
         */
        fmt->linegap = atoi(param);
        fmt->linerelgap = 0.0;
     }
   else if (cmd == linerelgapstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_linerelgap Relative line gap
         *
         * Sets the relative size of the line gap in text. The value must be
         * a percentage.
         * @warning Setting this value sets linegap to 0!
         * @code
         * linerelgap=<number>%
         * @endcode
         */
        char *endptr = NULL;
        double val = strtod(param, &endptr);
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
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_item Item
         *
         * Not implemented! Does nothing!
         * @code
         * item=<anything>
         * @endcode
         */
        // itemstr == replacement object items in textblock - inline imges
        // for example
     }
   else if (cmd == linefillstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_linefill Line fill
         *
         * Sets the size of the line fill in text. The value must be a
         * percentage.
         * @code
         * linefill=<number>%
         * @endcode
         */
        char *endptr = NULL;
        double val = strtod(param, &endptr);
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
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_ellipsis Ellipsis
         *
         * Sets ellipsis mode. The value should be a number. Any value smaller
         * than 0.0 or greater than 1.0 disables ellipsis.
         * A value of 0 means ellipsizing the leftmost portion of the text
         * first, 1 on the other hand the rightmost portion.
         * @code
         * ellipsis=<number>
         * @endcode
         */
        char *endptr = NULL;
        fmt->ellipsis = strtod(param, &endptr);
        if ((fmt->ellipsis < 0.0) || (fmt->ellipsis > 1.0))
          fmt->ellipsis = -1.0;
        else
          {
             Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
             o->have_ellipsis = 1;
          }
     }
   else if (cmd == passwordstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_password Password
         *
         * Sets if the text is being used for passwords. Enabling this causes
         * all characters to be substituted for '*'.
         * Value must be one of the following:
         * @li "on" - Enable
         * @li "off" - Disable
         * @code
         * password=<number>
         * @endcode
         */
        if (len == 3 && !strcmp(param, "off"))
          fmt->password = 0;
        else if (len == 2 && !strcmp(param, "on"))
          fmt->password = 1;
     }
   else if (cmd == underline_dash_widthstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_underline_dash_width Underline dash width
         *
         * Sets the width of the underline dash. The value should be a number.
         * @code
         * underline_dash_width=<number>
         * @endcode
         */
        fmt->underline_dash_width = atoi(param);
        if (fmt->underline_dash_width <= 0) fmt->underline_dash_width = 1;
     }
   else if (cmd == underline_dash_gapstr)
     {
        /**
         * @page evas_textblock_style_page Evas Textblock Style Options
         *
         * @subsection evas_textblock_style_underline_dash_gap Underline dash gap
         *
         * Sets the gap of the underline dash. The value should be a number.
         * @code
         * underline_dash_gap=<number>
         * @endcode
         */
        fmt->underline_dash_gap = atoi(param);
        if (fmt->underline_dash_gap <= 0) fmt->underline_dash_gap = 1;
     }
}

/**
 * @internal
 * Returns @c EINA_TRUE if the item is a format parameter, @c EINA_FALSE
 * otherwise.
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
_format_param_parse(const char *item, const char **key, Eina_Tmpstr **val)
{
   const char *start, *end;
   char *tmp, *s, *d;
   size_t len;

   start = strchr(item, '=');
   *key = eina_stringshare_add_length(item, start - item);
   start++; /* Advance after the '=' */
   /* If we can find a quote as the first non-space char,
    * our new delimiter is a quote, not a space. */
   while (*start == ' ')
      start++;

   if (*start == '\'')
     {
        start++;
        end = strchr(start, '\'');
        while ((end) && (end > start) && (end[-1] == '\\'))
          end = strchr(end + 1, '\'');
     }
   else
     {
        end = strchr(start, ' ');
        while ((end) && (end > start) && (end[-1] == '\\'))
          end = strchr(end + 1, ' ');
     }

   /* Null terminate before the spaces */
   if (end) len = end - start;
   else len = strlen(start);

   tmp = (char*) eina_tmpstr_add_length(start, len);
   if (!tmp) goto end;

   for (d = tmp, s = tmp; *s; s++)
     {
        if (*s != '\\')
          {
             *d = *s;
             d++;
          }
     }
   *d = '\0';

end:
   *val = tmp;
}

/**
 * @internal
 * This function parses the format passed in *s and advances s to point to the
 * next format item, while returning the current one as the return value.
 * @param s The current and returned position in the format string.
 * @return the current item parsed from the string.
 */
static const char *
_format_parse(const char **s)
{
   const char *p;
   const char *s1 = NULL, *s2 = NULL;
   Eina_Bool quote = EINA_FALSE;

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
             if (*p == '\'')
               {
                  quote = !quote;
               }

             if ((p > *s) && (p[-1] != '\\') && (!quote))
               {
                  if (*p == ' ') s2 = p;
               }
             if (*p == 0) s2 = p;
          }
        p++;
        if (s1 && s2)
          {
             *s = s2;
             return s1;
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
_format_fill(Evas_Object *eo_obj, Evas_Object_Textblock_Format *fmt, const char *str)
{
   const char *s;
   const char *item;

   s = str;

   /* get rid of any spaces at the start of the string */
   while (*s == ' ') s++;

   while ((item = _format_parse(&s)))
     {
        if (_format_is_param(item))
          {
             const char *key = NULL;
             Eina_Tmpstr *val = NULL;

             _format_param_parse(item, &key, &val);
             if ((key) && (val)) _format_command(eo_obj, fmt, key, val);
             eina_stringshare_del(key);
             eina_tmpstr_del(val);
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
_format_dup(Evas_Object *eo_obj, const Evas_Object_Textblock_Format *fmt)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Object_Textblock_Format *fmt2;

   fmt2 = calloc(1, sizeof(Evas_Object_Textblock_Format));
   memcpy(fmt2, fmt, sizeof(Evas_Object_Textblock_Format));
   fmt2->ref = 1;
   if (fmt->font.fdesc) fmt2->font.fdesc = evas_font_desc_ref(fmt->font.fdesc);

   if (fmt->font.source) fmt2->font.source = eina_stringshare_add(fmt->font.source);

   /* FIXME: just ref the font here... */
   fmt2->font.font = evas_font_load(obj->layer->evas->evas, fmt2->font.fdesc,
         fmt2->font.source, (int)(((double) fmt2->font.size) * obj->cur->scale));
   return fmt2;
}




typedef enum
{
   TEXTBLOCK_POSITION_START,
   TEXTBLOCK_POSITION_END,
   TEXTBLOCK_POSITION_ELSE,
   TEXTBLOCK_POSITION_SINGLE
} Textblock_Position;

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
   Evas_Textblock_Data *o;

   Evas_Object_Textblock_Paragraph *paragraphs;
   Evas_Object_Textblock_Paragraph *par;
   Evas_Object_Textblock_Line *ln;


   Eina_List *format_stack;
   Evas_Object_Textblock_Format *fmt;

   int x, y;
   int w, h;
   int wmax, hmax;
   int ascent, descent;
   int maxascent, maxdescent;
   int marginl, marginr;
   int line_no;
   int underline_extend;
   int have_underline, have_underline2;
   double align, valign;
   Textblock_Position position;
   Eina_Bool align_auto : 1;
   Eina_Bool width_changed : 1;
};

static void _layout_text_add_logical_item(Ctxt *c, Evas_Object_Textblock_Text_Item *ti, Eina_List *rel);
static void _text_item_update_sizes(Ctxt *c, Evas_Object_Textblock_Text_Item *ti);
static Evas_Object_Textblock_Format_Item *_layout_do_format(const Evas_Object *obj EINA_UNUSED, Ctxt *c, Evas_Object_Textblock_Format **_fmt, Evas_Object_Textblock_Node_Format *n, int *style_pad_l, int *style_pad_r, int *style_pad_t, int *style_pad_b, Eina_Bool create_item);

/**
 * @internal
 * Adjust the ascent/descent of the format and context.
 *
 * @param maxascent The ascent to update - Not NUL.
 * @param maxdescent The descent to update - Not NUL.
 * @param fmt The format to adjust - NOT NULL.
 */
static void
_layout_format_ascent_descent_adjust(const Evas_Object *eo_obj,
      Evas_Coord *maxascent, Evas_Coord *maxdescent,
      Evas_Object_Textblock_Format *fmt)
{
   int ascent, descent;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   if (fmt->font.font)
     {
        ascent = *maxascent;
        descent = *maxdescent;
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

             dh = obj->cur->geometry.h - (*maxascent + *maxdescent);
             if (dh < 0) dh = 0;
             dh = fmt->linefill * dh;
             *maxdescent += dh / 2;
             *maxascent += dh - (dh / 2);
             // FIXME: set flag that says "if heigh changes - reformat"
          }
     }
}

static void
_layout_item_max_ascent_descent_calc(const Evas_Object *eo_obj,
      Evas_Coord *maxascent, Evas_Coord *maxdescent,
      Evas_Object_Textblock_Item *it, Textblock_Position position)
{
   void *fi = NULL;
   *maxascent = *maxdescent = 0;

   if (!it || !it->format || !it->format->font.font)
      return;

   if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
     {
        fi = _ITEM_TEXT(it)->text_props.font_instance;
     }

   if ((position == TEXTBLOCK_POSITION_START) ||
         (position == TEXTBLOCK_POSITION_SINGLE))
     {
        Evas_Coord asc = 0;

        if (fi)
          {
             asc = evas_common_font_instance_max_ascent_get(fi);
          }
        else
          {
             Evas_Object_Protected_Data *obj =
                eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
             asc = ENFN->font_max_ascent_get(ENDT,
                   it->format->font.font);
          }

        if (asc > *maxascent)
           *maxascent = asc;
     }

   if ((position == TEXTBLOCK_POSITION_END) ||
         (position == TEXTBLOCK_POSITION_SINGLE))
     {
        /* Calculate max descent. */
        Evas_Coord desc = 0;

        if (fi)
          {
             desc = evas_common_font_instance_max_descent_get(fi);
          }
        else
          {
             Evas_Object_Protected_Data *obj =
                eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
             desc = ENFN->font_max_descent_get(ENDT,
                   it->format->font.font);
          }

        if (desc > *maxdescent)
           *maxdescent = desc;
     }
}

/**
 * @internal
 * Adjust the ascent/descent of the item and context.
 *
 * @param ascent The ascent to update - Not NUL.
 * @param descent The descent to update - Not NUL.
 * @param it The format to adjust - NOT NULL.
 * @param position The position inside the textblock
 */
static void
_layout_item_ascent_descent_adjust(const Evas_Object *eo_obj,
      Evas_Coord *ascent, Evas_Coord *descent,
      Evas_Object_Textblock_Item *it, Evas_Object_Textblock_Format *fmt)
{
   void *fi = NULL;
   int asc = 0, desc = 0;

   if ((!it || !it->format || !it->format->font.font) &&
         (!fmt || !fmt->font.font))
     {
        return;
     }

   if (it)
     {
        fmt = it->format;

        if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
          {
             fi = _ITEM_TEXT(it)->text_props.font_instance;
          }
     }

   if (fi)
     {
        asc = evas_common_font_instance_ascent_get(fi);
        desc = evas_common_font_instance_descent_get(fi);
     }
   else
     {
        Evas_Object_Protected_Data *obj =
           eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        asc =
           ENFN->font_ascent_get(ENDT, fmt->font.font);
        desc =
           ENFN->font_descent_get(ENDT, fmt->font.font);
     }

   if (ascent && (asc > *ascent))
      *ascent = asc;
   if (descent && (desc > *descent))
      *descent = desc;

   _layout_format_ascent_descent_adjust(eo_obj, ascent, descent, fmt);
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
   c->ascent = c->descent = 0;
   c->maxascent = c->maxdescent = 0;
   c->ln->line_no = -1;
   c->ln->par = c->par;
}

static inline Evas_Object_Textblock_Paragraph *
_layout_find_paragraph_by_y(Evas_Textblock_Data *o, Evas_Coord y)
{
   Evas_Object_Textblock_Paragraph *start, *par;
   int i;

   start = o->paragraphs;

   for (i = 0 ; i < TEXTBLOCK_PAR_INDEX_SIZE ; i++)
     {
        if (!o->par_index[i] || (o->par_index[i]->y > y))
          {
             break;
          }
        start = o->par_index[i];
     }

   EINA_INLIST_FOREACH(start, par)
     {
        if ((par->y <= y) && (y < par->y + par->h))
           return par;
     }

   return NULL;
}

static inline Evas_Object_Textblock_Paragraph *
_layout_find_paragraph_by_line_no(Evas_Textblock_Data *o, int line_no)
{
   Evas_Object_Textblock_Paragraph *start, *par;
   int i;

   start = o->paragraphs;

   for (i = 0 ; i < TEXTBLOCK_PAR_INDEX_SIZE ; i++)
     {
        if (!o->par_index[i] || (o->par_index[i]->line_no > line_no))
          {
             break;
          }
        start = o->par_index[i];
     }

   EINA_INLIST_FOREACH(start, par)
     {
        Evas_Object_Textblock_Paragraph *npar =
           (Evas_Object_Textblock_Paragraph *) EINA_INLIST_GET(par)->next;
        if ((par->line_no <= line_no) &&
              (!npar || (line_no < npar->line_no)))
           return par;
     }

   return NULL;
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
   c->o->num_paragraphs++;
}

#ifdef BIDI_SUPPORT
/**
 * @internal
 * Update bidi paragraph props.
 *
 * @param par The paragraph to update
 */
static inline void
_layout_update_bidi_props(const Evas_Textblock_Data *o,
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
_paragraph_clear(const Evas_Object *obj EINA_UNUSED,
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
_paragraph_free(const Evas_Object *eo_obj, Evas_Object_Textblock_Paragraph *par)
{
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   _paragraph_clear(eo_obj, par);

     {
        Evas_Object_Textblock_Item *it;
        EINA_LIST_FREE(par->logical_items, it)
          {
             _item_free(eo_obj, NULL, it);
          }
     }
#ifdef BIDI_SUPPORT
   if (par->bidi_props)
      evas_bidi_paragraph_props_unref(par->bidi_props);
#endif
   /* If we are the active par of the text node, set to NULL */
   if (par->text_node && (par->text_node->par == par))
      par->text_node->par = NULL;

   o->num_paragraphs--;

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
_paragraphs_clear(const Evas_Object *eo_obj, Evas_Object_Textblock_Paragraph *pars)
{
   Evas_Object_Textblock_Paragraph *par;

   EINA_INLIST_FOREACH(EINA_INLIST_GET(pars), par)
     {
        _paragraph_clear(eo_obj, par);
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
_paragraphs_free(const Evas_Object *eo_obj, Evas_Object_Textblock_Paragraph *pars)
{
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

   o->num_paragraphs = 0;

   while (pars)
     {
        Evas_Object_Textblock_Paragraph *par;

        par = (Evas_Object_Textblock_Paragraph *) pars;
        pars = (Evas_Object_Textblock_Paragraph *)eina_inlist_remove(EINA_INLIST_GET(pars), EINA_INLIST_GET(par));
        _paragraph_free(eo_obj, par);
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
        fmt->underline_dash_width = 6;
        fmt->underline_dash_gap = 2;
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
                  style_pad_l = style_pad_r = style_pad_t = style_pad_b = 0;
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
   const char *key = NULL;
   Eina_Tmpstr *val = NULL;

   _format_param_parse(item, &key, &val);
   if ((key) && (val)) _format_command(c->obj, fmt, key, val);
   if (key) eina_stringshare_del(key);
   if (val) eina_tmpstr_del(val);
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
_layout_calculate_format_item_size(const Evas_Object *eo_obj,
      const Evas_Object_Textblock_Format_Item *fi,
      Evas_Coord *maxascent, Evas_Coord *maxdescent,
      Evas_Coord *_y, Evas_Coord *_w, Evas_Coord *_h)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   /* Adjust sizes according to current line height/scale */
   Evas_Coord w, h;
   const char *p, *s;

   s = fi->item;
   w = fi->parent.w;
   h = fi->parent.h;
   if (!s)
     {
        *_w = w;
        *_h = h;
        return;
     }
   switch (fi->size)
     {
      case SIZE:
         p = strstr(s, " size=");
         if (p)
           {
              p += 6;
              if (sscanf(p, "%ix%i", &w, &h) == 2)
                {
                   w = w * obj->cur->scale;
                   h = h * obj->cur->scale;
                }
           }
         break;
      case SIZE_REL:
         p = strstr(s, " relsize=");
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

static Evas_Coord
_layout_last_line_max_descent_adjust_calc(Ctxt *c, const Evas_Object_Textblock_Paragraph *last_vis_par)
{
   if (last_vis_par->lines)
     {
        Evas_Object_Textblock_Line *ln = (Evas_Object_Textblock_Line *)
           EINA_INLIST_GET(last_vis_par->lines)->last;
        Evas_Object_Textblock_Item *it;

        EINA_INLIST_FOREACH(ln->items, it)
          {
             if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
               {
                  Evas_Coord asc = 0, desc = 0;
                  Evas_Coord maxasc = 0, maxdesc = 0;
                  _layout_item_ascent_descent_adjust(c->obj, &asc, &desc,
                        it, it->format);
                  _layout_item_max_ascent_descent_calc(c->obj, &maxasc, &maxdesc,
                        it, c->position);

                  if (desc > c->descent)
                     c->descent = desc;
                  if (maxdesc > c->maxdescent)
                     c->maxdescent = maxdesc;
               }
          }

        if (c->maxdescent > c->descent)
          {
             return c->maxdescent - c->descent;
          }
     }

   return 0;
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
   if (c->ascent + c->descent == 0)
      _layout_item_ascent_descent_adjust(c->obj, &c->ascent, &c->descent,
            NULL, fmt);

   /* Adjust all the item sizes according to the final line size,
    * and update the x positions of all the items of the line. */
   EINA_INLIST_FOREACH(c->ln->items, it)
     {
        if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
          {
             Evas_Object_Textblock_Format_Item *fi = _ITEM_FORMAT(it);
             if (!fi->formatme) goto loop_advance;
             _layout_calculate_format_item_size(c->obj, fi, &c->ascent,
                   &c->descent, &fi->y, &fi->parent.w, &fi->parent.h);
             fi->parent.adv = fi->parent.w;
          }
        else
          {
             Evas_Coord asc = 0, desc = 0;
             Evas_Coord maxasc = 0, maxdesc = 0;
             _layout_item_ascent_descent_adjust(c->obj, &asc, &desc,
                   it, it->format);
             _layout_item_max_ascent_descent_calc(c->obj, &maxasc, &maxdesc,
                   it, c->position);

             if (asc > c->ascent)
                c->ascent = asc;
             if (desc > c->descent)
                c->descent = desc;
             if (maxasc > c->maxascent)
                c->maxascent = maxasc;
             if (maxdesc > c->maxdescent)
                c->maxdescent = maxdesc;
          }

loop_advance:
        it->x = x;
        x += it->adv;

        if ((it->w > 0) && ((it->x + it->w) > c->ln->w)) c->ln->w = it->x + it->w;
     }

   c->ln->y = c->y - c->par->y;
   c->ln->h = c->ascent + c->descent;

   /* Handle max ascent and descent if at the edges */
     {
        /* If it's the start, offset the line according to the max ascent. */
        if (((c->position == TEXTBLOCK_POSITION_START) ||
                 (c->position == TEXTBLOCK_POSITION_SINGLE))
              && (c->maxascent > c->ascent))
          {
             Evas_Coord ascdiff;

             ascdiff = c->maxascent - c->ascent;
             c->ln->y += ascdiff;
             c->y += ascdiff;
             c->ln->y += c->o->style_pad.t;
             c->y += c->o->style_pad.t;
          }
     }

   c->ln->baseline = c->ascent;
   /* FIXME: Actually needs to be adjusted using the actual font value.
    * Also, underline_extend is actually not being used. */
   if (c->have_underline2)
     {
        if (c->descent < 4) c->underline_extend = 4 - c->descent;
     }
   else if (c->have_underline)
     {
        if (c->descent < 2) c->underline_extend = 2 - c->descent;
     }
   c->ln->line_no = c->line_no - c->ln->par->line_no;
   c->line_no++;
   c->y += c->ascent + c->descent;
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

   if (c->position == TEXTBLOCK_POSITION_START)
      c->position = TEXTBLOCK_POSITION_ELSE;
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
_layout_text_item_new(Ctxt *c EINA_UNUSED, Evas_Object_Textblock_Format *fmt)
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
        Evas_Object_Protected_Data *obj = eo_data_scope_get(c->obj, EVAS_OBJ_CLASS);
        return ENFN->font_last_up_to_pos(ENDT, fmt->font.font,
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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(c->obj, EVAS_OBJ_CLASS);
   if (fmt->font.font)
     ENFN->font_string_size_get(ENDT, fmt->font.font,
           &ti->text_props, &tw, &th);
   inset = 0;
   if (fmt->font.font)
     inset = ENFN->font_inset_get(ENDT, fmt->font.font,
           &ti->text_props);
   advw = 0;
   if (fmt->font.font)
      advw = ENFN->font_h_advance_get(ENDT, fmt->font.font,
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
        break;
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

static void
_layout_text_append_add_logical_item(Ctxt *c, Evas_Object_Textblock_Text_Item *ti,
      Eina_List *rel)
{
   _text_item_update_sizes(c, ti);

   if (rel)
     {
        c->par->logical_items = eina_list_prepend_relative_list(
              c->par->logical_items, ti, rel);
     }
   else
     {
        c->par->logical_items = eina_list_append(
              c->par->logical_items, ti);
     }
}

typedef struct {
     EINA_INLIST;
     Evas_Object_Textblock_Format *format;
     size_t start;
     int off;
} Layout_Text_Append_Queue;

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
_layout_text_append(Ctxt *c, Layout_Text_Append_Queue *queue, Evas_Object_Textblock_Node_Text *n, int start, int off, const char *repch, Eina_List *rel)
{
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
        if ((queue->format->password) && (repch) && (eina_ustrbuf_length_get(n->unicode)))
          {
             int i, ind;
             Eina_Unicode *ptr;

             tbase = str = ptr = alloca((off + 1) * sizeof(Eina_Unicode));
             ind = 0;
             urepch = eina_unicode_utf8_next_get(repch, &ind);
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

   /* If there's no parent text node, only create an empty item */
   if (!n)
     {
        ti = _layout_text_item_new(c, queue->format);
        ti->parent.text_node = NULL;
        ti->parent.text_pos = 0;
        _layout_text_append_add_logical_item(c, ti, rel);

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

        Evas_Object_Protected_Data *obj = eo_data_scope_get(c->obj, EVAS_OBJ_CLASS);
        while (script_len > 0)
          {
             Evas_Font_Instance *cur_fi = NULL;
             size_t run_start;
             int run_len = script_len;
             ti = _layout_text_item_new(c, queue->format);
             ti->parent.text_node = n;
             ti->parent.text_pos = run_start = start + str - tbase;

             if (ti->parent.format->font.font)
               {
                  run_len = ENFN->font_run_end_get(ENDT,
                        ti->parent.format->font.font, &script_fi, &cur_fi,
                        script, str, script_len);
               }

             evas_common_text_props_bidi_set(&ti->text_props,
                   c->par->bidi_props, ti->parent.text_pos);
             evas_common_text_props_script_set(&ti->text_props, script);

             if (cur_fi)
               {
                  ENFN->font_text_props_info_create(ENDT,
                        cur_fi, str, &ti->text_props, c->par->bidi_props,
                        ti->parent.text_pos, run_len, EVAS_TEXT_PROPS_MODE_SHAPE);
               }

             while (queue &&
                   ((queue->start + queue->off) < (run_start + run_len)))
               {
                  Evas_Object_Textblock_Text_Item *new_ti;

                  /* There must be a next because of the test in the while. */
                  queue = (Layout_Text_Append_Queue *) EINA_INLIST_GET(queue)->next;

                  new_ti = _layout_text_item_new(c, queue->format);
                  new_ti->parent.text_node = ti->parent.text_node;
                  new_ti->parent.text_pos = queue->start;

                  evas_common_text_props_split(&ti->text_props, &new_ti->text_props,
                        new_ti->parent.text_pos - ti->parent.text_pos);

                  if (ti)
                    {
                       _layout_text_append_add_logical_item(c, ti, rel);
                       ti = new_ti;
                    }
               }

             if (ti)
               {
                  _layout_text_append_add_logical_item(c, ti, rel);
               }

             str += run_len;
             script_len -= run_len;
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
 * Should be call after we finish filling a format.
 * FIXME: doc.
 */
static void
_format_finalize(Evas_Object *eo_obj, Evas_Object_Textblock_Format *fmt)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   void *of;

   of = fmt->font.font;

   fmt->font.font = evas_font_load(obj->layer->evas->evas, fmt->font.fdesc,
         fmt->font.source, (int)(((double) fmt->font.size) * obj->cur->scale));
   if (of) evas_font_free(obj->layer->evas->evas, of);
}

/**
 * @internal
 * Returns true if the item is a tab
 * @def _IS_TAB(item)
 */
#define _IS_TAB(item)                                             \
   (!strcmp(item, "tab") || !strcmp(item, "\t") || !strcmp(item, "\\t"))
/**
 * @internal
 * Returns true if the item is a line spearator, false otherwise
 * @def _IS_LINE_SEPARATOR(item)
 */
#define _IS_LINE_SEPARATOR(item)                                             \
   (!strcmp(item, "br") || !strcmp(item, "\n") || !strcmp(item, "\\n"))
/**
 * @internal
 * Returns true if the item is a paragraph separator, false otherwise
 * @def _IS_PARAGRAPH_SEPARATOR(item)
 */
#define _IS_PARAGRAPH_SEPARATOR_SIMPLE(item)                                 \
   (!strcmp(item, "ps"))
/**
 * @internal
 * Returns true if the item is a paragraph separator, false otherwise
 * takes legacy mode into account.
 * @def _IS_PARAGRAPH_SEPARATOR(item)
 */
#define _IS_PARAGRAPH_SEPARATOR(o, item)                                     \
   (_IS_PARAGRAPH_SEPARATOR_SIMPLE(item) ||                                  \
    (o->legacy_newline && _IS_LINE_SEPARATOR(item))) /* Paragraph separator */

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
 *
 * @return fi if created.
 */
static Evas_Object_Textblock_Format_Item *
_layout_do_format(const Evas_Object *obj EINA_UNUSED, Ctxt *c,
      Evas_Object_Textblock_Format **_fmt, Evas_Object_Textblock_Node_Format *n,
      int *style_pad_l, int *style_pad_r, int *style_pad_t, int *style_pad_b,
      Eina_Bool create_item)
{
   Evas_Object_Textblock_Format_Item *fi = NULL;
   Evas_Object_Textblock_Format *fmt = *_fmt;
   /* FIXME: comment the algo */

   const char *s;
   const char *item;
   int handled = 0;

   s = n->format;
   if (!strncmp(s, "item ", 5))
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
        if (n->opener && !n->own_closer)
          {
             fmt = _layout_format_push(c, fmt, n);
             push_fmt = EINA_TRUE;
          }
        else if (!n->opener)
          {
             fmt = _layout_format_pop(c, n->orig_format);
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
                       fi = _layout_format_item_add(c, n, item, fmt);

                       fi->parent.w = fi->parent.adv = 0;
                    }
                  else if (_IS_TAB(item))
                    {
                       fi = _layout_format_item_add(c, n, item, fmt);
                       fi->parent.w = fi->parent.adv = fmt->tabstops;
                       fi->formatme = 1;
                    }
               }
          }
        _format_finalize(c->obj, fmt);
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
   else if (fmt->underline || fmt->underline_dash)
     c->have_underline = 1;
   *_fmt = fmt;

   return fi;
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
}

/* -1 means no wrap */
static int
_layout_get_charwrap(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Item *it, size_t line_start,
      const char *breaks)
{
   int wrap;
   size_t uwrap;
   size_t len = eina_ustrbuf_length_get(it->text_node->unicode);
   /* Currently not being used, because it doesn't contain relevant
    * information */
   (void) breaks;

     {
        if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
           wrap = 0;
        else
           wrap = _layout_text_cutoff_get(c, fmt, _ITEM_TEXT(it));

        if (wrap < 0)
           return -1;
        uwrap = (size_t) wrap + it->text_pos;
     }


   if ((uwrap == line_start) && (it->type == EVAS_TEXTBLOCK_ITEM_TEXT))
     {
        uwrap = it->text_pos +
           (size_t) evas_common_text_props_cluster_next(
                 &_ITEM_TEXT(it)->text_props, wrap);
     }
   if ((uwrap <= line_start) || (uwrap > len))
      return -1;

   return uwrap;
}

/* -1 means no wrap */
/* Allow break means: if we can break after the current char */
#define ALLOW_BREAK(i) \
   (breaks[i] <= LINEBREAK_ALLOWBREAK)

static int
_layout_get_word_mixwrap_common(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Item *it, Eina_Bool mixed_wrap,
      size_t line_start, const char *breaks)
{
   Eina_Bool wrap_after = EINA_FALSE;
   size_t wrap;
   size_t orig_wrap;
   const Eina_Unicode *str = eina_ustrbuf_string_get(
         it->text_node->unicode);
   int item_start = it->text_pos;
   size_t len = eina_ustrbuf_length_get(it->text_node->unicode);

     {
        int swrap = -1;
        if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
           swrap = 0;
        else
           swrap = _layout_text_cutoff_get(c, fmt, _ITEM_TEXT(it));
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
        if (!_is_white(str[wrap]) || (wrap + 1 == len))
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
             return _layout_get_charwrap(c, fmt, it,
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


             if ((wrap < len) && (wrap >= line_start))
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
      const Evas_Object_Textblock_Item *it, size_t line_start,
      const char *breaks)
{
   return _layout_get_word_mixwrap_common(c, fmt, it, EINA_FALSE, line_start,
         breaks);
}

/* -1 means no wrap */
static int
_layout_get_mixedwrap(Ctxt *c, Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Item *it, size_t line_start,
      const char *breaks)
{
   return _layout_get_word_mixwrap_common(c, fmt, it, EINA_TRUE, line_start,
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
   c->o->ellip_ti = ellip_ti = _layout_text_item_new(c, cur_it->format);
   ellip_ti->parent.text_node = cur_it->text_node;
   ellip_ti->parent.text_pos = cur_it->text_pos;
   script = evas_common_language_script_type_get(_ellip_str, len);

   evas_common_text_props_bidi_set(&ellip_ti->text_props,
         c->par->bidi_props, ellip_ti->parent.text_pos);
   evas_common_text_props_script_set (&ellip_ti->text_props, script);

   if (ellip_ti->parent.format->font.font)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(c->obj, EVAS_OBJ_CLASS);
        /* It's only 1 char anyway, we don't need the run end. */
        (void) ENFN->font_run_end_get(ENDT,
              ellip_ti->parent.format->font.font, &script_fi, &cur_fi,
              script, _ellip_str, len);

        ENFN->font_text_props_info_create(ENDT,
              cur_fi, _ellip_str, &ellip_ti->text_props,
              c->par->bidi_props, ellip_ti->parent.text_pos, len, EVAS_TEXT_PROPS_MODE_SHAPE);
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
   Evas_Object_Textblock_Text_Item *ellip_ti;
   Evas_Object_Textblock_Item *last_it;
   Evas_Coord save_cx;
   int wrap;
   ellip_ti = _layout_ellipsis_item_new(c, it);
   last_it = it;

   save_cx = c->x;
   c->w -= ellip_ti->parent.w;

   if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
     {
        Evas_Object_Textblock_Text_Item *ti = _ITEM_TEXT(it);

        wrap = _layout_text_cutoff_get(c, last_it->format, ti);
        if ((wrap > 0) && !IS_AT_END(ti, (size_t) wrap))
          {
             _layout_item_text_split_strip_white(c, ti, i, wrap);
          }
        else if ((wrap == 0) && (c->ln->items))
          {
             last_it = _ITEM(EINA_INLIST_GET(c->ln->items)->last);
          }
     }
   else if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
     {
        /* We don't want to add this format item. */
        last_it = NULL;
     }

   c->x = save_cx;
   c->w += ellip_ti->parent.w;
   /* If we should add this item, do it */
   if (last_it == it)
     {
        c->ln->items = (Evas_Object_Textblock_Item *)
           eina_inlist_append(EINA_INLIST_GET(c->ln->items),
                 EINA_INLIST_GET(it));
        it->ln = c->ln;
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
   _ITEM(ellip_ti)->ln = c->ln;

   c->position = (c->position == TEXTBLOCK_POSITION_START) ?
      TEXTBLOCK_POSITION_SINGLE : TEXTBLOCK_POSITION_END;
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

static inline void
_layout_paragraph_render(Evas_Textblock_Data *o,
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
#else
   (void) o;
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
        if (!c->par->text_node->is_new && !c->par->text_node->dirty &&
              !c->width_changed && c->par->lines &&
              !c->o->have_ellipsis)
          {
             Evas_Object_Textblock_Line *ln;
             /* Update c->line_no */
             ln = (Evas_Object_Textblock_Line *)
                EINA_INLIST_GET(c->par->lines)->last;
             if (ln)
                c->line_no = c->par->line_no + ln->line_no + 1;

             /* After this par we are no longer at the beginning, as there
              * must be some text in the par. */
             if (c->position == TEXTBLOCK_POSITION_START)
                c->position = TEXTBLOCK_POSITION_ELSE;

             return 0;
          }
        c->par->text_node->dirty = EINA_FALSE;
        c->par->text_node->is_new = EINA_FALSE;
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
                       ititr->visually_deleted = EINA_FALSE;
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
        Evas_Coord prevdescent = 0, prevascent = 0;
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
             _layout_item_ascent_descent_adjust(c->obj, &c->ascent,
                   &c->descent, it, it->format);
          }
        else
          {
             Evas_Object_Textblock_Format_Item *fi = _ITEM_FORMAT(it);
             if (fi->formatme)
               {
                  prevdescent = c->descent;
                  prevascent = c->ascent;
                  /* If there are no text items yet, calc ascent/descent
                   * according to the current format. */
                  if (c->ascent + c->descent == 0)
                     _layout_item_ascent_descent_adjust(c->obj, &c->ascent,
                           &c->descent, it, it->format);

                  _layout_calculate_format_item_size(c->obj, fi, &c->ascent,
                        &c->descent, &fi->y, &fi->parent.w, &fi->parent.h);
                  fi->parent.adv = fi->parent.w;
               }
          }


        /* Check if we need to wrap, i.e the text is bigger than the width,
           or we already found a wrap point. */
        if ((c->w >= 0) &&
              (((c->x + it->w) >
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
                  size_t line_start;
                  size_t it_len;

                  it_len = (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT) ?
                     1 : _ITEM_TEXT(it)->text_props.text_len;


                  /* If we haven't calculated the linebreaks yet,
                   * do */
                  if (!line_breaks)
                    {
                       /* Only relevant in those cases */
                       if (it->format->wrap_word || it->format->wrap_mixed)
                         {
                            const char *lang;
                            lang = (it->format->font.fdesc) ?
                               it->format->font.fdesc->lang : "";
                            size_t len =
                               eina_ustrbuf_length_get(
                                     it->text_node->unicode);
                            line_breaks = malloc(len);
                            set_linebreaks_utf32((const utf32_t *)
                                  eina_ustrbuf_string_get(
                                     it->text_node->unicode),
                                  len, lang, line_breaks);
                         }
                    }
                  if (c->ln->items)
                     line_start = c->ln->items->text_pos;
                  else
                     line_start = it->text_pos;

                  adv_line = 1;
                  /* If we don't already have a wrap point from before */
                  if (wrap < 0)
                    {
                       if (it->format->wrap_word)
                          wrap = _layout_get_wordwrap(c, it->format, it,
                                line_start, line_breaks);
                       else if (it->format->wrap_char)
                          wrap = _layout_get_charwrap(c, it->format, it,
                                line_start, line_breaks);
                       else if (it->format->wrap_mixed)
                          wrap = _layout_get_mixedwrap(c, it->format, it,
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
                       if (uwrap < it->text_pos)
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
                                 it->ln = c->ln;
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
                       else if (uwrap > it->text_pos + it_len)
                         {
                            /* FIXME: Should redo the ellipsis handling.
                             * If we can do ellipsis, just cut here. */
                            if (it->format->ellipsis == 1.0)
                              {
                                 _layout_handle_ellipsis(c, it, i);
                                 ret = 1;
                                 goto end;
                              }
                            else
                              {
                                 /* Delay the cut in a smart way i.e use the
                                    item_pos as the line_start, because
                                    there's already no cut before*/
                                 wrap = -1;
                                 adv_line = 0;
                              }
                         }
                       else
                         {
                            wrap -= it->text_pos; /* Cut here */
                         }
                    }

                  if (wrap > 0)
                    {
                       if (it->type == EVAS_TEXTBLOCK_ITEM_TEXT)
                         {
                            _layout_item_text_split_strip_white(c,
                                  _ITEM_TEXT(it), i, wrap);
                         }
                    }
                  else if (wrap == 0)
                    {
                       /* Should wrap before the item */

                       /* We didn't end up using the item, so revert the ascent
                        * and descent changes. */
                       c->descent = prevdescent;
                       c->ascent = prevascent;

                       adv_line = 0;
                       redo_item = 1;
                       _layout_line_advance(c, it->format);
                    }
                  /* Reset wrap */
                  wrap = -1;
               }
          }

        if (!redo_item && !it->visually_deleted)
          {
             c->ln->items = (Evas_Object_Textblock_Item *)
                eina_inlist_append(EINA_INLIST_GET(c->ln->items),
                      EINA_INLIST_GET(it));
             it->ln = c->ln;
             if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
               {
                  Evas_Object_Textblock_Format_Item *fi;
                  fi = _ITEM_FORMAT(it);
                  fi->y = c->y;
                  /* If it's a newline, and we are not in newline compat
                   * mode, or we are in newline compat mode, and this is
                   * not used as a paragraph separator, advance */
                  if (fi->item && _IS_LINE_SEPARATOR(fi->item) &&
                        (!c->o->legacy_newline ||
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
        if (c->par && !EINA_INLIST_GET(c->par)->next)
          {
             c->position = (c->position == TEXTBLOCK_POSITION_START) ?
                TEXTBLOCK_POSITION_SINGLE : TEXTBLOCK_POSITION_END;
          }

        /* Here 'it' is the last format used */
        _layout_line_finalize(c, it->format);
     }

end:
   if (line_breaks)
      free(line_breaks);

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
        if (fnode->is_new)
          {
             const char *fstr = fnode->orig_format;
             /* balance < 0 means we gave up and everything should be
              * invalidated */
             if (fnode->opener && !fnode->own_closer)
               {
                  balance++;
                  if (!fstack)
                     start_n = fnode->text_node;
                  fstack = eina_list_prepend(fstack, fnode);
               }
             else if (!fnode->opener)
               {
                  size_t fstr_len;
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
                                 break;
                              }
                         }
                       balance--;
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

   eina_list_free(fstack);
}

static Layout_Text_Append_Queue *
_layout_text_append_queue_item_append(Layout_Text_Append_Queue *queue,
      Evas_Object_Textblock_Format *format, size_t start, int off)
{
   /* Don't add empty items. */
   if (off == 0)
      return (Layout_Text_Append_Queue *) queue;

   Layout_Text_Append_Queue *item = calloc(1, sizeof(*item));
   item->format = format;
   item->start = start;
   item->off = off;
   item->format->ref++;

   return (Layout_Text_Append_Queue *) eina_inlist_append(EINA_INLIST_GET(queue), EINA_INLIST_GET(item));
}

static void
_layout_text_append_item_free(Ctxt *c, Layout_Text_Append_Queue *item)
{
   if (item->format)
      _format_unref_free(c->obj, item->format);
   free(item);
}

static void
_layout_text_append_commit(Ctxt *c, Layout_Text_Append_Queue **_queue, Evas_Object_Textblock_Node_Text *n, Eina_List *rel)
{
   Layout_Text_Append_Queue *item, *queue = *_queue;

   if (!queue)
      return;

     {
        item = (Layout_Text_Append_Queue *) EINA_INLIST_GET(queue)->last;
        int off = item->start - queue->start + item->off;
        _layout_text_append(c, queue, n, queue->start, off, c->o->repch, rel);
     }

   while (queue)
     {
        item = queue;
        queue = (Layout_Text_Append_Queue *) EINA_INLIST_GET(queue)->next;
        _layout_text_append_item_free(c, item);
     }

   *_queue = NULL;
}

static Eina_Bool
_layout_split_text_because_format(const Evas_Object_Textblock_Format *fmt,
      const Evas_Object_Textblock_Format *nfmt)
{
   if ((fmt->password != nfmt->password) ||
         memcmp(&fmt->font, &nfmt->font, sizeof(fmt->font)))
     {
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

/** FIXME: Document */
static void
_layout_pre(Ctxt *c, int *style_pad_l, int *style_pad_r, int *style_pad_t,
      int *style_pad_b)
{
   Evas_Object *eo_obj = c->obj;
   Evas_Textblock_Data *o = c->o;

   /* Mark text nodes as dirty if format have changed. */
   if (o->format_changed)
     _format_changes_invalidate_text_nodes(c);

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
             if (!n->is_new)
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
                       _paragraph_free(eo_obj, c->par);

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
                       _paragraph_free(eo_obj, prev_par);
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
                              {
                                 int pl = 0, pr = 0, pt = 0, pb = 0;
                                 _layout_do_format(eo_obj, c, &c->fmt, fnode,
                                                   &pl, &pr, &pt, &pb, EINA_FALSE);
                                 fnode->pad.l = pl;
                                 fnode->pad.r = pr;
                                 fnode->pad.t = pt;
                                 fnode->pad.b = pb;
                              }
                            if (fnode->pad.l > *style_pad_l) *style_pad_l = fnode->pad.l;
                            if (fnode->pad.r > *style_pad_r) *style_pad_r = fnode->pad.r;
                            if (fnode->pad.t > *style_pad_t) *style_pad_t = fnode->pad.t;
                            if (fnode->pad.b > *style_pad_b) *style_pad_b = fnode->pad.b;
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
             Layout_Text_Append_Queue *queue = NULL;
             fnode = n->format_node;
             start = off = 0;
             while (fnode && (fnode->text_node == n))
               {
                  Evas_Object_Textblock_Format_Item *fi = NULL;
                  Evas_Object_Textblock_Format *pfmt = c->fmt;

                  off += fnode->offset;
                  /* No need to skip on the first run, or a non-visible one */
                  queue = _layout_text_append_queue_item_append(queue, c->fmt, start, off);
                  fi = _layout_do_format(eo_obj, c, &c->fmt, fnode, style_pad_l,
                        style_pad_r, style_pad_t, style_pad_b, EINA_TRUE);

                  if (fi || _layout_split_text_because_format(pfmt, c->fmt))
                    {
                       Eina_List *rel = NULL;
                       if (fi)
                         {
                            rel = eina_list_last(c->par->logical_items);
                         }

                       _layout_text_append_commit(c, &queue, n, rel);
                    }

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
                  fnode->is_new = EINA_FALSE;
                  fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
               }
             queue = _layout_text_append_queue_item_append(queue, c->fmt, start,
                   eina_ustrbuf_length_get(n->unicode) - start);
             _layout_text_append_commit(c, &queue, n, NULL);
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
             _paragraph_free(eo_obj, c->par);

             c->par = tmp_par;
          }
        o->paragraphs = c->paragraphs;
        c->par = NULL;
     }
   else
     {
        if (o->style_pad.l > *style_pad_l) *style_pad_l = o->style_pad.l;
        if (o->style_pad.r > *style_pad_r) *style_pad_r = o->style_pad.r;
        if (o->style_pad.t > *style_pad_t) *style_pad_t = o->style_pad.t;
        if (o->style_pad.b > *style_pad_b) *style_pad_b = o->style_pad.b;
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
_layout(const Evas_Object *eo_obj, int w, int h, int *w_ret, int *h_ret)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Textblock_Data *o = eo_data_ref(eo_obj, MY_CLASS);
   Ctxt ctxt, *c;
   int style_pad_l = 0, style_pad_r = 0, style_pad_t = 0, style_pad_b = 0;

   LYDBG("ZZ: layout %p %4ix%4i | w=%4i | last_w=%4i --- '%s'\n", eo_obj, w, h, obj->cur->geometry.w, o->last_w, o->markup_text);
   /* setup context */
   c = &ctxt;
   c->obj = (Evas_Object *)eo_obj;
   c->o = o;
   c->paragraphs = c->par = NULL;
   c->format_stack = NULL;
   c->fmt = NULL;
   c->x = c->y = 0;
   c->w = w;
   c->h = h;
   c->wmax = c->hmax = 0;
   c->ascent = c->descent = 0;
   c->maxascent = c->maxdescent = 0;
   c->marginl = c->marginr = 0;
   c->have_underline = 0;
   c->have_underline2 = 0;
   c->underline_extend = 0;
   c->line_no = 0;
   c->align = 0.0;
   c->align_auto = EINA_TRUE;
   c->ln = NULL;
   c->width_changed = (obj->cur->geometry.w != o->last_w);

   /* Start of logical layout creation */
   /* setup default base style */
     {
        Eina_Bool finalize = EINA_FALSE;
        if ((c->o->style) && (c->o->style->default_tag))
          {
             c->fmt = _layout_format_push(c, NULL, NULL);
             _format_fill(c->obj, c->fmt, c->o->style->default_tag);
             finalize = EINA_TRUE;
          }

        if ((c->o->style_user) && (c->o->style_user->default_tag))
          {
             if (!c->fmt)
               {
                  c->fmt = _layout_format_push(c, NULL, NULL);
               }
             _format_fill(c->obj, c->fmt, c->o->style_user->default_tag);
             finalize = EINA_TRUE;
          }

        if (finalize)
           _format_finalize(c->obj, c->fmt);
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
      int par_index_step = o->num_paragraphs / TEXTBLOCK_PAR_INDEX_SIZE;
      int par_count = 1; /* Force it to take the first one */
      int par_index_pos = 0;

      c->position = TEXTBLOCK_POSITION_START;

      if (par_index_step == 0) par_index_step = 1;

      /* Clear all of the index */
      memset(o->par_index, 0, sizeof(o->par_index));

      EINA_INLIST_FOREACH(c->paragraphs, c->par)
        {
           _layout_update_par(c);

           /* Break if we should stop here. */
           if (_layout_par(c))
             {
                last_vis_par = c->par;
                break;
             }

           if ((par_index_pos < TEXTBLOCK_PAR_INDEX_SIZE) && (--par_count == 0))
             {
                par_count = par_index_step;

                o->par_index[par_index_pos++] = c->par;
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
        {
           c->hmax = last_vis_par->y + last_vis_par->h +
              _layout_last_line_max_descent_adjust_calc(c, last_vis_par);
        }
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
        _paragraphs_clear(eo_obj, c->paragraphs);
        LYDBG("ZZ: ... layout #2\n");
        _layout(eo_obj, w, h, w_ret, h_ret);
     }
}

/*
 * @internal
 * Relayout the object according to current object size.
 *
 * @param obj the evas object - NOT NULL.
 */
static void
_relayout(const Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   _layout(eo_obj, obj->cur->geometry.w, obj->cur->geometry.h,
         &o->formatted.w, &o->formatted.h);
   o->formatted.valid = 1;
   o->formatted.oneline_h = 0;
   o->last_w = obj->cur->geometry.w;
   LYDBG("ZZ: --------- layout %p @ %ix%i = %ix%i\n", eo_obj, obj->cur->geometry.w, obj->cur->geometry.h, o->formatted.w, o->formatted.h);
   o->last_h = obj->cur->geometry.h;
   if ((o->paragraphs) && (!EINA_INLIST_GET(o->paragraphs)->next) &&
       (o->paragraphs->lines) && (!EINA_INLIST_GET(o->paragraphs->lines)->next))
     {
        if (obj->cur->geometry.h < o->formatted.h)
          {
             LYDBG("ZZ: 1 line only... lasth == formatted h (%i)\n", o->formatted.h);
             o->formatted.oneline_h = o->formatted.h;
          }
     }
   o->changed = 0;
   o->content_changed = 0;
   o->format_changed = EINA_FALSE;
   o->redraw = 1;
}

/*
 * @internal
 * Check if the object needs a relayout, and if so, execute it.
 */
static inline void
_relayout_if_needed(Evas_Object *eo_obj, const Evas_Textblock_Data *o)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   evas_object_textblock_coords_recalc(eo_obj, obj, obj->private_data);
   if (!o->formatted.valid)
     {
        LYDBG("ZZ: relayout\n");
        _relayout(eo_obj);
     }
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
_find_layout_item_line_match(Evas_Object *eo_obj, Evas_Object_Textblock_Node_Text *n, int pos, Evas_Object_Textblock_Line **lnr, Evas_Object_Textblock_Item **itr)
{
   Evas_Object_Textblock_Paragraph *found_par;
   Evas_Object_Textblock_Line *ln;
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

   _relayout_if_needed(eo_obj, o);

   found_par = n->par;
   if (found_par)
     {
        _layout_paragraph_render(o, found_par);
        EINA_INLIST_FOREACH(found_par->lines, ln)
          {
             Evas_Object_Textblock_Item *it;

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
_find_layout_line_num(const Evas_Object *eo_obj, int line)
{
   Evas_Object_Textblock_Paragraph *par;
   Evas_Object_Textblock_Line *ln;
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

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
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *eo_obj = eo_add(EVAS_OBJ_TEXTBLOCK_CLASS, e);
   eo_unref(eo_obj);
   return eo_obj;
}

EOLIAN static void
_evas_textblock_constructor(Eo *eo_obj, Evas_Textblock_Data *class_data EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Textblock_Data *o;
   Eo *eo_parent;

   eo_do_super(eo_obj, MY_CLASS, eo_constructor());

   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = eo_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;

   o = obj->private_data;
   o->cursor = calloc(1, sizeof(Evas_Textblock_Cursor));
   _format_command_init();
   evas_object_textblock_init(eo_obj);

   eo_do(eo_obj, eo_parent_get(&eo_parent));
   evas_object_inject(eo_obj, obj, evas_object_evas_get(eo_parent));
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
   Evas_Object *eo_obj;

   if (!ts) return;
   /* If the style wasn't really changed, abort. */
   if ((!ts->style_text && !text) ||
       (ts->style_text && text && !strcmp(text, ts->style_text)))
      return;

   EINA_LIST_FOREACH(ts->objects, l, eo_obj)
     {
        Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
        _evas_textblock_invalidate_all(o);
        _evas_textblock_changed(o, eo_obj);
     }

   _style_replace(ts, text);

   if (ts->style_text)
     {
        // format MUST be KEY='VALUE'[KEY='VALUE']...
        const char *p;
        const char *key_start, *key_stop, *val_start;

        key_start = key_stop = val_start = NULL;
        p = ts->style_text;
        while (*p)
          {
             if (!key_start)
               {
		 if (!isspace((unsigned char)(*p)))
                    key_start = p;
               }
             else if (!key_stop)
               {
		 if ((*p == '=') || (isspace((unsigned char)(*p))))
                    key_stop = p;
               }
             else if (!val_start)
               {
                  if (((*p) == '\'') && (*(p + 1)))
                    {
                       val_start = ++p;
                    }
               }
             if ((key_start) && (key_stop) && (val_start))
               {
                  char *tags, *replaces = NULL;
                  Evas_Object_Style_Tag *tag;
                  const char *val_stop = NULL;
                  size_t tag_len;
                  size_t replace_len;

                    {
                       Eina_Strbuf *buf = eina_strbuf_new();
                       val_stop = val_start;
                       while(*p)
                         {
                            if (*p == '\'')
                              {
                                 /* Break if we found the tag end */
                                 if (p[-1] != '\\')
                                   {
                                      eina_strbuf_append_length(buf, val_stop,
                                            p - val_stop);
                                      break;
                                   }
                                 else
                                   {
                                      eina_strbuf_append_length(buf, val_stop,
                                            p - val_stop - 1);
                                      eina_strbuf_append_char(buf, '\'');
                                      val_stop = p + 1;
                                   }
                              }
                            p++;
                         }
                       replaces = eina_strbuf_string_steal(buf);
                       eina_strbuf_free(buf);
                    }
                  /* If we didn't find an end, just aboart. */
                  if (!*p)
                    {
                       if (replaces) free(replaces);
                       break;
                    }

                  tag_len = key_stop - key_start;
                  replace_len = val_stop - val_start;

                  tags = malloc(tag_len + 1);
                  if (tags)
                    {
                       memcpy(tags, key_start, tag_len);
                       tags[tag_len] = 0;
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
                                 tag->tag.tag = tags;
                                 tag->tag.replace = replaces;
                                 tag->tag.tag_len = tag_len;
                                 tag->tag.replace_len = replace_len;
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
                  key_start = key_stop = val_start = NULL;
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

static const char *
_textblock_format_node_from_style_tag(Evas_Textblock_Data *o, Evas_Object_Textblock_Node_Format *fnode, const char *format, size_t format_len)
{
   const char *match;
   size_t replace_len;
   if (!o->style_user || !(match = _style_match_tag(o->style_user, format,
               format_len, &replace_len)))
     {
        match = _style_match_tag(o->style, format, format_len,
              &replace_len);
     }

   if (match)
     {
        if (match[0] != '-')
          {
             fnode->opener = EINA_TRUE;
             if (match[0] != '+')
               {
                  fnode->own_closer = EINA_TRUE;
               }
          }
     }
   return match;
}

/* textblock styles */

static void
_textblock_style_generic_set(Evas_Object *eo_obj, Evas_Textblock_Style *ts,
      Evas_Textblock_Style **obj_ts)
{
   TB_HEAD();
   if (ts == *obj_ts) return;
   if ((ts) && (ts->delete_me)) return;
   if (*obj_ts)
     {
        Evas_Textblock_Style *old_ts;
        if (o->markup_text)
          {
             free(o->markup_text);
             o->markup_text = NULL;
          }

        old_ts = *obj_ts;
        old_ts->objects = eina_list_remove(old_ts->objects, eo_obj);
        if ((old_ts->delete_me) && (!old_ts->objects))
          evas_textblock_style_free(old_ts);
     }
   if (ts)
     {
        ts->objects = eina_list_append(ts->objects, eo_obj);
     }
   *obj_ts = ts;

   Evas_Object_Textblock_Node_Format *fnode = o->format_nodes;
   while (fnode)
     {
        const char *match;
        size_t format_len = eina_stringshare_strlen(fnode->orig_format);
        /* Is this safe to use alloca here? Strings might possibly get large */
        char *format = alloca(format_len + 2);

        if (!fnode->opener)
          {
             format[0] = '/';
             format[1] = '\0';
          }
        else
          {
             format[0] = '\0';
          }

        strcat(format, fnode->orig_format);

        match = _textblock_format_node_from_style_tag(o, fnode, format,
              format_len);

        if (match && fnode->format && strcmp(match, fnode->format))
          {
             if ((*match == '+') || (*match == '-'))
               {
                  match++;
                  while (*match == ' ') match++;
               }
             fnode->is_new = EINA_TRUE;
             eina_stringshare_replace(&fnode->format, match);
          }
        fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
     }

   o->format_changed = EINA_TRUE;
   _evas_textblock_invalidate_all(o);
   _evas_textblock_changed(o, eo_obj);
}

EOLIAN static void
_evas_textblock_style_set(Eo *eo_obj, Evas_Textblock_Data *o, const Evas_Textblock_Style *ts)
{
   _textblock_style_generic_set(eo_obj, (Evas_Textblock_Style *) ts, &(o->style));
}

EOLIAN static const Evas_Textblock_Style*
_evas_textblock_style_get(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o)
{
   return o->style;
}

EOLIAN static void
_evas_textblock_style_user_push(Eo *eo_obj, Evas_Textblock_Data *o, Evas_Textblock_Style *ts)
{
   _textblock_style_generic_set(eo_obj, ts, &(o->style_user));
}

EOLIAN static const Evas_Textblock_Style*
_evas_textblock_style_user_peek(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o)
{
   return o->style_user;
}

EOLIAN static void
_evas_textblock_style_user_pop(Eo *eo_obj, Evas_Textblock_Data *o)
{
   _textblock_style_generic_set(eo_obj, NULL,  &(o->style_user));
}

EOLIAN static void
_evas_textblock_replace_char_set(Eo *eo_obj, Evas_Textblock_Data *o, const char *ch)
{
   if (o->repch) eina_stringshare_del(o->repch);
   if (ch) o->repch = eina_stringshare_add(ch);
   else o->repch = NULL;
   _evas_textblock_invalidate_all(o);
   _evas_textblock_changed(o, eo_obj);
}

EOLIAN static void
_evas_textblock_legacy_newline_set(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o, Eina_Bool mode)
{
   if (o->legacy_newline == mode)
      return;

   o->legacy_newline = mode;
   /* FIXME: Should recreate all the textnodes... For now, it's just
    * for new text inserted. */
}

EOLIAN static Eina_Bool
_evas_textblock_legacy_newline_get(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o)
{
   return o->legacy_newline;
}

EOLIAN static void
_evas_textblock_valign_set(Eo *eo_obj, Evas_Textblock_Data *o, double align)
{
   if (align < 0.0) align = 0.0;
   else if (align > 1.0) align = 1.0;
   if (o->valign == align) return;
   o->valign = align;
   _evas_textblock_changed(o, eo_obj);
}

EOLIAN static double
_evas_textblock_valign_get(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o)
{
   return o->valign;
}

EOLIAN static void
_evas_textblock_bidi_delimiters_set(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o, const char *delim)
{
   eina_stringshare_replace(&o->bidi_delimiters, delim);
}

EOLIAN static const char*
_evas_textblock_bidi_delimiters_get(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o)
{
   return o->bidi_delimiters;
}

EOLIAN static const char*
_evas_textblock_replace_char_get(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o)
{
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
             if (*sc != *mc)
               {
                  match = 0;
                  break;
               }
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

        if (tolower((unsigned char)(*s)) == 'x')
          {
             s++;
             base = 16;
          }

        len = s_end - s;
        if (len > sizeof(ustr))
           len = sizeof(ustr);

        memcpy(ustr, s, len);
        ustr[len - 1] = '\0';
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
   if (!escape) return NULL;
   return _escaped_char_get(escape, escape + strlen(escape));
}

EAPI const char *
evas_textblock_escape_string_range_get(const char *escape_start, const char *escape_end)
{
   if ((!escape_start) || (!escape_end)) return NULL;
   return _escaped_char_get(escape_start, escape_end);
}

EAPI const char *
evas_textblock_string_escape_get(const char *string, int *len_ret)
{
   if ((!string) || (!len_ret)) return NULL;
   /* & -> &amp; */
   return _escaped_char_match(string, len_ret);
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


EOLIAN static void
_evas_textblock_text_markup_set(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o, const char *text)
{
   if ((text != o->markup_text) && (o->markup_text))
     {
        free(o->markup_text);
        o->markup_text = NULL;
     }

   _nodes_clear(eo_obj);

   o->cursor->node = _evas_textblock_node_text_new();
   o->text_nodes = _NODE_TEXT(eina_inlist_append(
            EINA_INLIST_GET(o->text_nodes),
            EINA_INLIST_GET(o->cursor->node)));

   if (!o->style && !o->style_user)
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
   if (!cur) return;
   Evas_Object *eo_obj = cur->obj;
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
             size_t text_len;
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
                       size_t ttag_len = tag_end - tag_start;


                       ttag = malloc(ttag_len + 1);
                       if (ttag)
                         {
                            memcpy(ttag, tag_start, ttag_len);
                            ttag[ttag_len] = 0;
                            evas_textblock_cursor_format_prepend(cur, ttag);
                            free(ttag);
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
                       tag_end = p + 1;
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
             else if (!strncmp(_REPLACEMENT_CHAR_UTF8, p,
                      text_len = strlen(_REPLACEMENT_CHAR_UTF8)) ||
                   !strncmp(_NEWLINE_UTF8, p,
                      text_len = strlen(_NEWLINE_UTF8)) ||
                   !strncmp(_TAB_UTF8, p,
                      text_len = strlen(_TAB_UTF8)) ||
                   !strncmp(_PARAGRAPH_SEPARATOR_UTF8, p,
                      text_len = strlen(_PARAGRAPH_SEPARATOR_UTF8)))
               {
                  /*FIXME: currently just remove them, maybe do something
                   * fancier in the future, atm it breaks if this char
                   * is inside <> */
                  _prepend_text_run(cur, s, p);
                  /* it's also advanced later in this loop need +text_len
                     in total*/
                  p += text_len - 1;
                  s = p + 1; /* One after the end of the replacement char */
               }
             p++;
          }
     }
   _evas_textblock_changed(o, eo_obj);
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
_markup_get_format_append(Eina_Strbuf *txt, Evas_Object_Textblock_Node_Format *fnode)
{
   eina_strbuf_append_char(txt, '<');
     {
        const char *s;

        // FIXME: need to escape
        s = fnode->orig_format;
        if (!fnode->opener && !fnode->own_closer)
           eina_strbuf_append_char(txt, '/');
        eina_strbuf_append(txt, s);
        if (fnode->own_closer)
           eina_strbuf_append_char(txt, '/');
     }
   eina_strbuf_append_char(txt, '>');
}

/**
 * @internal
 * An helper function to _markup_get_text_append and others, used for getting
 * back only the "dangerous" escapes.
 */
static void
_markup_get_text_utf8_append(Eina_Strbuf *sbuf, const char *text)
{
   int ch, pos = 0, pos2 = 0;

   for (;;)
     {
        pos = pos2;
        ch = eina_unicode_utf8_next_get(text, &pos2);
        if ((ch <= 0) || (pos2 <= 0)) break;

        if (ch == _NEWLINE)
           eina_strbuf_append(sbuf, "<br/>");
        else if (ch == _TAB)
           eina_strbuf_append(sbuf, "<tab/>");
        else if (ch == '<')
           eina_strbuf_append(sbuf, "&lt;");
        else if (ch == '>')
           eina_strbuf_append(sbuf, "&gt;");
        else if (ch == '&')
           eina_strbuf_append(sbuf, "&amp;");
        else if (ch == _PARAGRAPH_SEPARATOR)
           eina_strbuf_append(sbuf, "<ps/>");
        else if (ch == _REPLACEMENT_CHAR)
           eina_strbuf_append(sbuf, "&#xfffc;");
        else if (ch != '\r')
          {
             eina_strbuf_append_length(sbuf, text + pos, pos2 - pos);
          }
     }
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
   char *base = eina_unicode_unicode_to_utf8(text, NULL);

   if (!base) return;

   _markup_get_text_utf8_append(txt, base);

   free(base);
}
EOLIAN static const char*
_evas_textblock_text_markup_get(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o)
{
   Evas_Object_Textblock_Node_Text *n;
   Eina_Strbuf *txt = NULL;

   const char *markup;
   if (o->markup_text)
     {
        markup = (o->markup_text);
        return markup;
     }
   txt = eina_strbuf_new();
   EINA_INLIST_FOREACH(o->text_nodes, n)
     {
        Evas_Object_Textblock_Node_Format *fnode;
        Eina_Unicode *text_base, *text;
        int off;
        int len;

        /* For each text node to thorugh all of it's format nodes
         * append text from the start to the offset of the next format
         * using the last format got. if needed it also creates format items
         * this is the core algorithm of the layout mechanism.
         * Skip the unicode replacement chars when there are because
         * we don't want to print them. */
        len = (int) eina_ustrbuf_length_get(n->unicode);
        text_base = text =
           eina_unicode_strndup(eina_ustrbuf_string_get(n->unicode), len);
        fnode = n->format_node;
        off = 0;
        while (fnode && (fnode->text_node == n))
          {
             Eina_Unicode tmp_ch;
             off += fnode->offset;
             
             if (off > len) break;
             /* No need to skip on the first run */
             tmp_ch = text[off];
             text[off] = 0; /* Null terminate the part of the string */
             _markup_get_text_append(txt, text);
             _markup_get_format_append(txt, fnode);
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

   (((Evas_Textblock_Data *)o)->markup_text) = eina_strbuf_string_steal(txt);
   eina_strbuf_free(txt);
   markup = (o->markup_text);

   return markup;
}

EAPI char *
evas_textblock_text_markup_to_utf8(const Evas_Object *eo_obj, const char *text)
{
   /* FIXME: Redundant and awful, should be merged with markup_prepend */
   Eina_Strbuf *sbuf;
   char *s, *p, *ret;
   char *tag_start, *tag_end, *esc_start, *esc_end;

   if (!text) return NULL;


   tag_start = tag_end = esc_start = esc_end = NULL;
   sbuf = eina_strbuf_new();
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
                  size_t ttag_len;

                  tag_start++; /* Skip the < */
                  tag_end--; /* Skip the > */
                  if ((tag_end > tag_start) && (*(tag_end - 1) == '/'))
                    {
                       tag_end --; /* Skip the terminating '/' */
                       while (*(tag_end - 1) == ' ')
                         tag_end--; /* skip trailing ' ' */
                    }

                  ttag_len = tag_end - tag_start;

                  ttag = malloc(ttag_len + 1);
                  if (ttag)
                    {
                       const char *match = NULL;
                       size_t replace_len;
                       memcpy(ttag, tag_start, ttag_len);
                       ttag[ttag_len] = 0;


                       if (eo_obj)
                         {
                            match = _style_match_tag(
                                  evas_object_textblock_style_get(eo_obj),
                                  ttag, ttag_len, &replace_len);
                         }

                       if (!match) match = ttag;

                       if (_IS_PARAGRAPH_SEPARATOR_SIMPLE(match))
                          eina_strbuf_append(sbuf, _PARAGRAPH_SEPARATOR_UTF8);
                       else if (_IS_LINE_SEPARATOR(match))
                          eina_strbuf_append(sbuf, _NEWLINE_UTF8);
                       else if (_IS_TAB(match))
                          eina_strbuf_append(sbuf, _TAB_UTF8);
                       else if (!strncmp(match, "item", 4))
                          eina_strbuf_append(sbuf, _REPLACEMENT_CHAR_UTF8);

                       free(ttag);
                    }
                  tag_start = tag_end = NULL;
               }
             else if (esc_end)
               {
                  const char *escape;

                  escape = _escaped_char_get(esc_start, esc_end + 1);
                  if (escape) eina_strbuf_append(sbuf, escape);
                  esc_start = esc_end = NULL;
               }
             else if (*p == 0)
               {
                  if (s)
                    {
                       eina_strbuf_append_length(sbuf, s, p - s);
                       s = NULL;
                    }
                  else
                    {
                       ERR("There is a invalid markup tag at positoin '%u'. Please check the text.", (unsigned int) (p - text));
                    }
               }
             if (*p == 0)
                break;
          }
        if (*p == '<')
          {
             if (!esc_start)
               {
                  /* Append the text prior to this to the textblock and
                   * mark the start of the tag */
                  tag_start = p;
                  tag_end = NULL;
                  if (s)
                    {
                       eina_strbuf_append_length(sbuf, s, p - s);
                       s = NULL;
                    }
                  else
                    {
                       ERR("There is a invalid markup tag at positoin '%u'. Please check the text.", (unsigned int) (p - text));
                    }
               }
          }
        else if (*p == '>')
          {
             if (tag_start)
               {
                  tag_end = p + 1;
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
                  if (s)
                    {
                       eina_strbuf_append_length(sbuf, s, p - s);
                       s = NULL;
                    }
                  else
                    {
                       ERR("There is a invalid markup tag at positoin '%u'. Please check the text.", (unsigned int) (p - text));
                    }
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

   ret = eina_strbuf_string_steal(sbuf);
   eina_strbuf_free(sbuf);
   return ret;
}

EAPI char *
evas_textblock_text_utf8_to_markup(const Evas_Object *eo_obj, const char *text)
{
   Eina_Strbuf *sbuf;
   char *str = NULL;

   (void) eo_obj;

   if (!text) return NULL;

   sbuf = eina_strbuf_new();

   _markup_get_text_utf8_append(sbuf, text);

   str = eina_strbuf_string_steal(sbuf);
   eina_strbuf_free(sbuf);
   return str;

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
_evas_textblock_nodes_merge(Evas_Textblock_Data *o, Evas_Object_Textblock_Node_Text *to)
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

   to->is_new = EINA_TRUE;

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
   int len;
   if (!cur) return;

   len = eina_ustrbuf_length_get(cur->node->unicode);

   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);
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

   TB_NULL_CHECK(cur->node, NULL);

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

   TB_NULL_CHECK(cur->node, NULL);

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
 * @return @c EINA_TRUE if we matched the previous format, @c EINA_FALSE
 * otherwise.
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

EOLIAN static Evas_Textblock_Cursor*
_evas_textblock_cursor_get(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o)
{
   return o->cursor;
}

EOLIAN static Evas_Textblock_Cursor*
_evas_textblock_cursor_new(Eo *eo_obj, Evas_Textblock_Data *o)
{
   Evas_Textblock_Cursor *cur;
   {
        cur = calloc(1, sizeof(Evas_Textblock_Cursor));
        (cur)->obj = (Evas_Object *) eo_obj;
        (cur)->node = o->text_nodes;
        (cur)->pos = 0;
        o->cursors = eina_list_append(o->cursors, cur);
     }

   return cur;
}

EAPI void
evas_textblock_cursor_free(Evas_Textblock_Cursor *cur)
{
   if (!cur) return;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);
   if (cur == o->cursor) return;
   o->cursors = eina_list_remove(o->cursors, cur);
   free(cur);
}

EAPI Eina_Bool
evas_textblock_cursor_is_format(const Evas_Textblock_Cursor *cur)
{
   if (!cur || !cur->node) return EINA_FALSE;
   return (_evas_textblock_cursor_node_format_at_pos_get(cur)) ?
      EINA_TRUE : EINA_FALSE;
}

EAPI const Eina_List *
evas_textblock_node_format_list_get(const Evas_Object *eo_obj, const char *anchor)
{
   const Eina_List *list = NULL;
   eo_do((Eo *)eo_obj, evas_obj_textblock_node_format_list_get(anchor, &list));
   return list;
}

EOLIAN static void
_evas_textblock_node_format_list_get(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o, const char *anchor, const Eina_List **ret_list)
{
   if (!strcmp(anchor, "a"))
     {
        *ret_list = o->anchors_a;
        return;
     }
   else if (!strcmp(anchor, "item"))
     {
        *ret_list = o->anchors_item;
        return;
     }

   *ret_list = NULL;
}

EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_first_get(const Evas_Object *eo_obj)
{
   const Evas_Object_Textblock_Node_Format *format = NULL;
   eo_do((Eo *)eo_obj, evas_obj_textblock_node_format_first_get(&format));
   return format;
}

EOLIAN static const Evas_Object_Textblock_Node_Format*
_evas_textblock_node_format_first_get(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o)
{
   return o->format_nodes;
}

EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_last_get(const Evas_Object *eo_obj)
{
   const Evas_Object_Textblock_Node_Format *format = NULL;
   eo_do((Eo *)eo_obj, evas_obj_textblock_node_format_last_get(&format));
   return format;
}

EOLIAN static const Evas_Object_Textblock_Node_Format*
_evas_textblock_node_format_last_get(Eo *eo_obj EINA_UNUSED, Evas_Textblock_Data *o)
{
   return o->format_nodes ? _NODE_FORMAT(EINA_INLIST_GET(o->format_nodes)->last) : NULL;
}

EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_next_get(const Evas_Object_Textblock_Node_Format *n)
{
   if (!n) return NULL;
   return _NODE_FORMAT(EINA_INLIST_GET(n)->next);
}

EAPI const Evas_Object_Textblock_Node_Format *
evas_textblock_node_format_prev_get(const Evas_Object_Textblock_Node_Format *n)
{
   if (!n) return NULL;
   return _NODE_FORMAT(EINA_INLIST_GET(n)->prev);
}

EAPI void
evas_textblock_node_format_remove_pair(Evas_Object *eo_obj,
      Evas_Object_Textblock_Node_Format *n)
{
   eo_do(eo_obj, evas_obj_textblock_node_format_remove_pair(n));
}

EOLIAN static void
_evas_textblock_node_format_remove_pair(Eo *eo_obj, Evas_Textblock_Data *o, Evas_Object_Textblock_Node_Format *n)
{
   Evas_Object_Textblock_Node_Text *tnode1;
   Evas_Object_Textblock_Node_Format *fmt, *found_node = NULL;
   Eina_List *fstack = NULL;

   if (!n) return;

   fmt = n;

   do
     {
        const char *fstr = fmt->orig_format;

        if (fmt->opener && !fmt->own_closer)
          {
             fstack = eina_list_prepend(fstack, fmt);
          }
        else if (fstr && !fmt->opener)
          {
             size_t fstr_len;
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
        cur.obj = eo_obj;

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

   _evas_textblock_changed(o, eo_obj);
}

EAPI void
evas_textblock_cursor_paragraph_first(Evas_Textblock_Cursor *cur)
{
   if (!cur) return;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);
   cur->node = o->text_nodes;
   cur->pos = 0;

}

EAPI void
evas_textblock_cursor_paragraph_last(Evas_Textblock_Cursor *cur)
{
   Evas_Object_Textblock_Node_Text *node;

   if (!cur) return;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);
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
   TB_NULL_CHECK(cur->node, EINA_FALSE);
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
   TB_NULL_CHECK(cur->node, EINA_FALSE);
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
   TB_NULL_CHECK(cur->node, EINA_FALSE);
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
   TB_NULL_CHECK(cur->node, EINA_FALSE);
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

/* BREAK_AFTER: true if we can break after the current char.
 * Both macros assume str[i] is not the terminating nul */
#define BREAK_AFTER(i) \
   (breaks[i] == WORDBREAK_BREAK)

EAPI Eina_Bool
evas_textblock_cursor_word_start(Evas_Textblock_Cursor *cur)
{
   const Eina_Unicode *text;
   size_t i;
   char *breaks;

   if (!cur) return EINA_FALSE;
   TB_NULL_CHECK(cur->node, EINA_FALSE);

   size_t len = eina_ustrbuf_length_get(cur->node->unicode);

   text = eina_ustrbuf_string_get(cur->node->unicode);

     {
        const char *lang = ""; /* FIXME: get lang */
        breaks = malloc(len);
        set_wordbreaks_utf32((const utf32_t *) text, len, lang, breaks);
     }

   if ((cur->pos > 0) && (cur->pos == len))
      cur->pos--;

   for (i = cur->pos ; _is_white(text[i]) && BREAK_AFTER(i) ; i--)
     {
        if (i == 0) break;
     }

   for ( ; i > 0 ; i--)
     {
        if (BREAK_AFTER(i - 1))
          {
             break;
          }
     }

   cur->pos = i;

   free(breaks);
   return EINA_TRUE;
}

EAPI Eina_Bool
evas_textblock_cursor_word_end(Evas_Textblock_Cursor *cur)
{
   const Eina_Unicode *text;
   size_t i;
   char *breaks;

   if (!cur) return EINA_FALSE;
   TB_NULL_CHECK(cur->node, EINA_FALSE);

   size_t len = eina_ustrbuf_length_get(cur->node->unicode);

   text = eina_ustrbuf_string_get(cur->node->unicode);

     {
        const char *lang = ""; /* FIXME: get lang */
        breaks = malloc(len);
        set_wordbreaks_utf32((const utf32_t *) text, len, lang, breaks);
     }

   if (cur->pos == len)
      return EINA_TRUE;

   for (i = cur->pos; text[i] && _is_white(text[i]) && (BREAK_AFTER(i)) ; i++);

   for ( ; text[i] ; i++)
     {
        if (BREAK_AFTER(i))
          {
             /* This is the one to break after. */
             break;
          }
     }

   cur->pos = i;

   free(breaks);
   return EINA_TRUE;
}

EAPI Eina_Bool
evas_textblock_cursor_char_next(Evas_Textblock_Cursor *cur)
{
   int ind;
   const Eina_Unicode *text;

   if (!cur) return EINA_FALSE;
   TB_NULL_CHECK(cur->node, EINA_FALSE);

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
   TB_NULL_CHECK(cur->node, EINA_FALSE);

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
   TB_NULL_CHECK(cur->node);
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
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;

   if (!cur) return;
   TB_NULL_CHECK(cur->node);
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

   _relayout_if_needed(cur->obj, o);

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
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;

   if (!cur) return;
   TB_NULL_CHECK(cur->node);
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

   _relayout_if_needed(cur->obj, o);

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

   if (!fnode->own_closer)
     {
        is_opener = fnode->opener;
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
              (!strncmp(item, "br", itlen) && (itlen >= 2)) ||
              (!strncmp(item, "tab", itlen) && (itlen >= 3)) ||
              (!strncmp(item, "ps", itlen) && (itlen >= 2)) ||
              (!strncmp(item, "item", itlen) && (itlen >= 4) && is_opener))
          {
             fnode->visible = EINA_TRUE;
          }

        if (is_opener && !strncmp(item, "a", itlen))
          {
             fnode->anchor = ANCHOR_A;
          }
        else if (is_opener && !strncmp(item, "item", itlen) && (itlen >= 4))
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
static void EINA_UNUSED
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
_evas_textblock_node_format_remove_matching(Evas_Textblock_Data *o,
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


        if (fmt->opener && !fmt->own_closer)
          {
             formats = eina_list_prepend(formats, fmt);
          }
        else if (fstr && !fmt->opener)
          {
             Evas_Object_Textblock_Node_Format *fnode;
             size_t fstr_len;
             fstr_len = strlen(fstr);
             /* Generic popper, just pop (if there's anything to pop). */
             if (formats && (((fstr[0] == ' ') && !fstr[1]) || !fstr[0]))
               {
                  fnode = eina_list_data_get(formats);
                  formats = eina_list_remove_list(formats, formats);
                  _evas_textblock_node_format_remove(o, fnode, 0);
                  _evas_textblock_node_format_remove(o, fmt, 0);
               }
             /* Find the matching format and pop it, if the matching format
              * is our format, i.e the last one, pop and break. */
             else
               {
                  Eina_List *i, *next;
                  EINA_LIST_FOREACH_SAFE(formats, i, next, fnode)
                    {
                       if (_FORMAT_IS_CLOSER_OF(
                                fnode->orig_format, fstr, fstr_len))
                         {
                            fnode = eina_list_data_get(i);
                            formats = eina_list_remove_list(formats, i);
                            _evas_textblock_node_format_remove(o, fnode, 0);
                            _evas_textblock_node_format_remove(o, fmt, 0);
                            break;
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
_evas_textblock_node_format_adjust_offset(Evas_Textblock_Data *o,
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
_evas_textblock_node_format_remove(Evas_Textblock_Data *o, Evas_Object_Textblock_Node_Format *n, int visible_adjustment)
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
 * @returns @c EINA_TRUE if removed a PS, @c EINA_FALSE otherwise.
 */
static Eina_Bool
_evas_textblock_node_text_adjust_offsets_to_start(Evas_Textblock_Data *o,
      Evas_Object_Textblock_Node_Text *n, size_t start, int end)
{
   Evas_Object_Textblock_Node_Format *last_node, *itr;
   int use_end = 1;
   int delta = 0;
   int first = 1;
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
     }

   return EINA_FALSE;
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
_evas_textblock_node_text_remove(Evas_Textblock_Data *o, Evas_Object_Textblock_Node_Text *n)
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
   Evas_Object_Textblock_Node_Text *n;
   size_t npos = 0;

   if (!cur) return -1;
   TB_NULL_CHECK(cur->node, 0);
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);
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
   Evas_Object_Textblock_Node_Text *n;
   size_t pos;

   if (!cur) return;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

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
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock_Item *it;

   if (!cur) return EINA_FALSE;

   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

   _relayout_if_needed(cur->obj, o);

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
   n->is_new = EINA_TRUE;

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
   Evas_Object_Textblock_Node_Text *n;

   if (!cur) return;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

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
_evas_textblock_cursors_set_node(Evas_Textblock_Data *o,
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
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

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
_evas_textblock_changed(Evas_Textblock_Data *o, Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   LYDBG("ZZ: invalidate 1 %p\n", eo_obj);
   o->formatted.valid = 0;
   o->native.valid = 0;
   o->content_changed = 1;
   if (o->markup_text)
     {
	free(o->markup_text);
	o->markup_text = NULL;
     }

   evas_object_change(eo_obj, obj);
}

static void
_evas_textblock_invalidate_all(Evas_Textblock_Data *o)
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
   Evas_Object_Textblock_Node_Text *n;
   Evas_Object_Textblock_Node_Format *fnode = NULL;
   Eina_Unicode *text;
   int len = 0;

   if (!cur) return 0;
   text = eina_unicode_utf8_to_unicode(_text, &len);
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

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
        n = cur->node = o->text_nodes;
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
   if (len == 0) return 0;
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
_evas_textblock_node_format_free(Evas_Textblock_Data *o,
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
_evas_textblock_node_format_new(Evas_Textblock_Data *o, const char *_format)
{
   Evas_Object_Textblock_Node_Format *n;
   const char *format = _format;
   const char *pre_stripped_format = NULL;

   n = calloc(1, sizeof(Evas_Object_Textblock_Node_Format));
   /* Create orig_format and format */
   if (format[0] == '<')
     {
        const char *match;
        size_t format_len;

        format++; /* Advance after '<' */
        format_len = strlen(format);
        if ((format_len > 0) && format[format_len - 1] == '>')
          {
             format_len--; /* We don't care about '>' */
             /* Check if it closes itself. Skip the </> case. */
             if ((format_len > 1) && format[format_len - 1] == '/')
               {
                  format_len--; /* We don't care about '/' */
                  n->own_closer = EINA_TRUE;
               }
          }

        match = _textblock_format_node_from_style_tag(o, n, format, format_len);

        if (match)
          {
             pre_stripped_format = match;
          }
        else
          {
             if (format[0] == '/')
               {
                  format++;
                  format_len--;
               }
             else
               {
                  n->opener = EINA_TRUE;
               }
          }

        n->orig_format = eina_stringshare_add_length(format, format_len);

        if (!pre_stripped_format)
           pre_stripped_format = n->orig_format;
     }
   /* Just use as is, it's a special format. */
   else
     {
        const char *tmp = format;
        if (format[0] != '-')
          {
             n->opener = EINA_TRUE;
             if (format[0] != '+')
               {
                  n->own_closer = EINA_TRUE;
               }
          }
        if ((*tmp == '+') || (*tmp == '-'))
          {
             tmp++;
             while (*tmp == ' ') tmp++;
          }
        n->orig_format = eina_stringshare_add(tmp);
        pre_stripped_format = n->orig_format;
     }

   /* Strip format */
     {
        const char *tmp = pre_stripped_format;
        if ((*tmp == '+') || (*tmp == '-'))
          {
             tmp++;
             while (*tmp == ' ') tmp++;
          }
        n->format = eina_stringshare_add(tmp);
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
   n->is_new = EINA_TRUE;

   return n;
}

static Eina_Bool
_evas_textblock_cursor_is_at_the_end(const Evas_Textblock_Cursor *cur)
{
   const Eina_Unicode *text;

   if (!cur) return EINA_FALSE;
   TB_NULL_CHECK(cur->node, EINA_FALSE);
   text = eina_ustrbuf_string_get(cur->node->unicode);
   if ((cur->pos - 1) > eina_ustrbuf_length_get(cur->node->unicode)) return EINA_FALSE;
   return ((text[cur->pos] == 0) && (!EINA_INLIST_GET(cur->node)->next)) ?
              EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool
evas_textblock_cursor_format_append(Evas_Textblock_Cursor *cur, const char *format)
{
   Evas_Object_Textblock_Node_Format *n;
   Eina_Bool is_visible;

   if (!cur) return EINA_FALSE;
   if ((!format) || (format[0] == 0)) return EINA_FALSE;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);
   /* We should always have at least one text node */
   if (!o->text_nodes)
     {
        evas_textblock_cursor_text_prepend(cur, "");
     }

   n = _evas_textblock_node_format_new(o, format);
   is_visible = n->visible;
   format = n->format;
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
             fmt = _evas_textblock_node_format_last_at_off(fmt);
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
           insert_char = _NEWLINE;
        else if (_IS_TAB(format))
           insert_char = _TAB;
        else
           insert_char = _REPLACEMENT_CHAR;

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
             n->is_new = EINA_FALSE;
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
   Evas_Object_Textblock_Node_Text *n, *n2;
   const Eina_Unicode *text;
   int chr, ind, ppos;

   if (!cur || !cur->node) return;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);
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
             if (!format || last_fmt->own_closer)
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
   Evas_Object_Textblock_Node_Text *n1, *n2;
   Eina_Bool should_merge = EINA_FALSE, reset_cursor = EINA_FALSE;

   if (!cur1 || !cur1->node) return;
   if (!cur2 || !cur2->node) return;
   if (cur1->obj != cur2->obj) return;
   Evas_Textblock_Data *o = eo_data_scope_get(cur1->obj, MY_CLASS);
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
             /* Remove the whole node. */
             Evas_Object_Textblock_Node_Text *n =
                _NODE_TEXT(EINA_INLIST_GET(n1)->next);
             if (n)
               {
                  should_merge = EINA_TRUE;
               }
             _evas_textblock_node_text_adjust_offsets_to_start(o, n1, cur1->pos, -1);
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
        n = _NODE_TEXT(EINA_INLIST_GET(n1)->next);
        /* Remove all the text nodes between */
        while (n && (n != n2))
          {
             Evas_Object_Textblock_Node_Text *nnode;

             nnode = _NODE_TEXT(EINA_INLIST_GET(n)->next);
             _evas_textblock_nodes_merge(o, n1);
             n = nnode;
          }
        /* After we merged all the nodes, move the formats to the start of
         * the range. */
        _evas_textblock_node_text_adjust_offsets_to_start(o, n1, cur1->pos, -1);

        should_merge = _evas_textblock_node_text_adjust_offsets_to_start(o, n2,
              0, cur2->pos);

        /* Remove the formats and the strings in the first and last nodes */
        len = eina_ustrbuf_length_get(n1->unicode);
        eina_ustrbuf_remove(n1->unicode, cur1->pos, len);
        eina_ustrbuf_remove(n2->unicode, 0, cur2->pos);
        /* Merge the nodes because we removed the PS */
        _evas_textblock_cursors_update_offset(cur1, cur1->node, cur1->pos,
                                              -cur1->pos);
        _evas_textblock_cursors_update_offset(cur2, cur2->node, 0, -cur2->pos);
        cur2->pos = 0;
        _evas_textblock_nodes_merge(o, n1);
     }
   fnode = _evas_textblock_cursor_node_format_at_pos_get(cur1);

   n1 = cur1->node;
   n2 = cur2->node;
   n1->dirty = n2->dirty = EINA_TRUE;

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
}


EAPI char *
evas_textblock_cursor_content_get(const Evas_Textblock_Cursor *cur)
{
   if (!cur || !cur->node) return NULL;
   if (evas_textblock_cursor_format_is_visible_get(cur))
     {
        Eina_Strbuf *buf;
        Evas_Object_Textblock_Node_Format *fnode;
        char *ret;
        fnode = _evas_textblock_node_visible_at_pos_get(
                 evas_textblock_cursor_format_get(cur));

        buf = eina_strbuf_new();
        _markup_get_format_append(buf, fnode);
        ret = eina_strbuf_string_steal(buf);
        eina_strbuf_free(buf);

        return ret;
     }
   else
     {
        const Eina_Unicode *ustr;
        Eina_Unicode buf[2];
        char *s;

        ustr = eina_ustrbuf_string_get(cur->node->unicode);
        buf[0] = ustr[cur->pos];
        buf[1] = 0;
        s = eina_unicode_unicode_to_utf8(buf, NULL);

        return s;
     }
}

static char *
_evas_textblock_cursor_range_text_markup_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *_cur2)
{
   Evas_Object_Textblock_Node_Text *tnode;
   Eina_Strbuf *buf;
   Evas_Textblock_Cursor *cur2;

   if (!cur1 || !cur1->node) return NULL;
   if (!_cur2 || !_cur2->node) return NULL;
   if (cur1->obj != _cur2->obj) return NULL;
   buf = eina_strbuf_new();

   if (evas_textblock_cursor_compare(cur1, _cur2) > 0)
     {
	const Evas_Textblock_Cursor *tc;

	tc = cur1;
	cur1 = _cur2;
	_cur2 = tc;
     }
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
        int cur1_pos = 0, cur2_pos = -1;
        int off = 0;

        text_base = text =
           eina_unicode_strndup(eina_ustrbuf_string_get(tnode->unicode),
                                eina_ustrbuf_length_get(tnode->unicode));
        if (tnode == cur2->node)
          cur2_pos = cur2->pos;
        if (tnode == cur1->node)
          cur1_pos = cur1->pos;
        fnode = _evas_textblock_node_text_get_first_format_between(tnode,
                cur1_pos, cur2_pos);
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
             _markup_get_format_append(buf, fnode);
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

static char *
_evas_textblock_cursor_range_text_plain_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *_cur2)
{
   Eina_UStrbuf *buf;
   Evas_Object_Textblock_Node_Text *n1, *n2;
   Evas_Textblock_Cursor *cur2;

   if (!cur1 || !cur1->node) return NULL;
   if (!_cur2 || !_cur2->node) return NULL;
   if (cur1->obj != _cur2->obj) return NULL;
   buf = eina_ustrbuf_new();

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


   if (n1 == n2)
     {
        const Eina_Unicode *tmp;
        tmp = eina_ustrbuf_string_get(n1->unicode);
        eina_ustrbuf_append_length(buf, tmp + cur1->pos, cur2->pos - cur1->pos);
     }
   else
     {
        const Eina_Unicode *tmp;
        tmp = eina_ustrbuf_string_get(n1->unicode);
        eina_ustrbuf_append(buf, tmp + cur1->pos);
        n1 = _NODE_TEXT(EINA_INLIST_GET(n1)->next);
        while (n1 != n2)
          {
             tmp = eina_ustrbuf_string_get(n1->unicode);
             eina_ustrbuf_append_length(buf, tmp,
                   eina_ustrbuf_length_get(n1->unicode));
             n1 = _NODE_TEXT(EINA_INLIST_GET(n1)->next);
          }
        tmp = eina_ustrbuf_string_get(n2->unicode);
        eina_ustrbuf_append_length(buf, tmp, cur2->pos);
     }

   /* Free and return */
     {
        char *ret;
        ret = eina_unicode_unicode_to_utf8(eina_ustrbuf_string_get(buf), NULL);
        eina_ustrbuf_free(buf);
        return ret;
     }
}

EAPI Eina_List *
evas_textblock_cursor_range_formats_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2)
{
   Evas_Object *eo_obj;
   Eina_List *ret = NULL;
   Evas_Object_Textblock_Node_Text *n1, *n2;
   Evas_Object_Textblock_Node_Format *first, *last;
   if (!cur1 || !cur1->node) return NULL;
   if (!cur2 || !cur2->node) return NULL;
   if (cur1->obj != cur2->obj) return NULL;

   eo_obj = cur1->obj;
   TB_HEAD_RETURN(NULL);

   if (evas_textblock_cursor_compare(cur1, cur2) > 0)
     {
        const Evas_Textblock_Cursor *tc;

        tc = cur1;
        cur1 = cur2;
        cur2 = tc;
     }
   n1 = cur1->node;
   n2 = cur2->node;

   /* FIXME: Change first and last getting to format_before_or_at_pos_get */

   last = n2->format_node;

   /* If n2->format_node is NULL, we don't have formats in the tb/range. */
   if (!last)
      return NULL;
   /* If the found format is on our text node, we should go to the last
    * one, otherwise, the one we found is good enough. */
   if (last->text_node == n2)
     {
        Evas_Object_Textblock_Node_Format *fnode = last;
        while (fnode && (fnode->text_node == n2))
          {
             last = fnode;
             fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
          }
     }

   /* If the first format node is within the range (i.e points to n1) or if
    * we have other formats in the range, go through them */
   first = n1->format_node;
   if ((first->text_node == n1) || (first != last))
     {
        Evas_Object_Textblock_Node_Format *fnode = first;
        /* Go to the first one in the range */
        if (fnode->text_node != n1)
          {
             fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
          }

        while (fnode)
          {
             ret = eina_list_append(ret, fnode);
             if (fnode == last)
                break;
             fnode = _NODE_FORMAT(EINA_INLIST_GET(fnode)->next);
          }
     }

   return ret;

}

EAPI char *
evas_textblock_cursor_range_text_get(const Evas_Textblock_Cursor *cur1, const Evas_Textblock_Cursor *cur2, Evas_Textblock_Text_Type format)
{
   if (format == EVAS_TEXTBLOCK_TEXT_MARKUP)
      return _evas_textblock_cursor_range_text_markup_get(cur1, cur2);
   else if (format == EVAS_TEXTBLOCK_TEXT_PLAIN)
      return _evas_textblock_cursor_range_text_plain_get(cur1, cur2);
   else
      return NULL; /* Not yet supported */
}

EAPI const char *
evas_textblock_cursor_paragraph_text_get(const Evas_Textblock_Cursor *cur)
{
   Evas_Textblock_Cursor cur1, cur2;
   if (!cur) return NULL;
   TB_NULL_CHECK(cur->node, NULL);
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
   TB_NULL_CHECK(cur->node, -1);
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
   TB_NULL_CHECK(cur->node, NULL);
   return _evas_textblock_cursor_node_format_at_pos_get(cur);
}

EAPI const char *
evas_textblock_node_format_text_get(const Evas_Object_Textblock_Node_Format *fmt)
{
   static char *ret = NULL;
   char *tmp;

   if (!fmt) return NULL;

   if (ret) free(ret);
   ret = malloc(strlen(fmt->orig_format) + 2 + 1);
   tmp = ret;

   if (fmt->opener && !fmt->own_closer)
     {
        *(tmp++) = '+';
        *(tmp++) = ' ';
     }
   else if (!fmt->opener)
     {
        *(tmp++) = '-';
        *(tmp++) = ' ';
     }
   strcpy(tmp, fmt->orig_format);
   return ret;
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
   TB_NULL_CHECK(cur->node, EINA_FALSE);
   if (!evas_textblock_cursor_is_format(cur)) return EINA_FALSE;
   text = eina_ustrbuf_string_get(cur->node->unicode);
   return EVAS_TEXTBLOCK_IS_VISIBLE_FORMAT_CHAR(text[cur->pos]);
}

#ifdef BIDI_SUPPORT
static Evas_Object_Textblock_Line*
_find_layout_line_by_item(Evas_Object_Textblock_Paragraph *par, Evas_Object_Textblock_Item *_it)
{
   Evas_Object_Textblock_Line *ln;

   EINA_INLIST_FOREACH(par->lines, ln)
     {
        Evas_Object_Textblock_Item *it;

        EINA_INLIST_FOREACH(ln->items, it)
          {
             if (_it == it)
               return ln;
          }
     }
   return NULL;
}
#endif

EAPI Eina_Bool
evas_textblock_cursor_geometry_bidi_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_Coord *cx2, Evas_Coord *cy2, Evas_Coord *cw2, Evas_Coord *ch2, Evas_Textblock_Cursor_Type ctype)
{
   if (!cur) return EINA_FALSE;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

   _relayout_if_needed(cur->obj, o);

   if (ctype == EVAS_TEXTBLOCK_CURSOR_UNDER)
     {
        evas_textblock_cursor_pen_geometry_get(cur, cx, cy, cw, ch);
        return EINA_FALSE;
     }

#ifdef BIDI_SUPPORT
#define IS_RTL(par) ((par) % 2)
#define IS_DIFFERENT_DIR(l1, l2) (IS_RTL(l1) != IS_RTL(l2))
   else
     {
        Evas_Object_Textblock_Line *ln = NULL;
        Evas_Object_Textblock_Item *it = NULL;
        _find_layout_item_match(cur, &ln, &it);
        if (ln && it)
          {
             if (ln->par->is_bidi)
               {
                  if (cw) *cw = 0;
                  if (cw2) *cw2 = 0;

                  /* If we are at the start or the end of the item there's a chance
                   * we'll want a split cursor.  */
                  Evas_Object_Textblock_Item *previt = NULL;
                  Evas_Object_Textblock_Item *it1 = NULL, *it2 = NULL;
                  Evas_Coord adv1 = 0, adv2 = 0;

                  if (cur->pos == it->text_pos)
                    {
                       EvasBiDiLevel par_level, it_level, previt_level;

                       _layout_update_bidi_props(o, ln->par);
                       par_level = *(ln->par->bidi_props->embedding_levels);
                       it_level = ln->par->bidi_props->embedding_levels[it->text_pos];
                       /* Get the logically previous item. */
                         {
                            Eina_List *itr;
                            Evas_Object_Textblock_Item *ititr;

                            EINA_LIST_FOREACH(ln->par->logical_items, itr, ititr)
                              {
                                 if (ititr == it)
                                   break;
                                 previt = ititr;
                              }

                            if (previt)
                              {
                                 previt_level = ln->par->bidi_props->embedding_levels[previt->text_pos];
                              }
                         }

                       if (previt && (it_level != previt_level))
                         {
                            Evas_Object_Textblock_Item *curit = NULL, *curit_opp = NULL;
                            EvasBiDiLevel cur_level;

                            if (it_level > previt_level)
                              {
                                 curit = it;
                                 curit_opp = previt;
                                 cur_level = it_level;
                              }
                            else
                              {
                                 curit = previt;
                                 curit_opp = it;
                                 cur_level = previt_level;
                              }

                            if (((curit == it) && (!IS_RTL(par_level))) ||
                                ((curit == previt) && (IS_RTL(par_level))))
                              {
                                 adv1 = (IS_DIFFERENT_DIR(cur_level, par_level)) ?
                                                          curit_opp->adv : 0;
                                 adv2 = curit->adv;
                              }
                            else if (((curit == previt) && (!IS_RTL(par_level))) ||
                                     ((curit == it) && (IS_RTL(par_level))))
                              {
                                 adv1 = (IS_DIFFERENT_DIR(cur_level, par_level)) ?
                                                          0 : curit->adv;
                                 adv2 = 0;
                              }

                            if (!IS_DIFFERENT_DIR(cur_level, par_level))
                              curit_opp = curit;

                            it1 = curit_opp;
                            it2 = curit;
                         }
                       /* Clear the bidi props because we don't need them anymore. */
                       evas_bidi_paragraph_props_unref(ln->par->bidi_props);
                       ln->par->bidi_props = NULL;
                    }
                  /* Handling last char in line (or in paragraph).
                   * T.e. prev condition didn't work, so we are not standing in the beginning of item,
                   * but in the end of line or paragraph. */
                  else if (evas_textblock_cursor_eol_get(cur))
                    {
                       EvasBiDiLevel par_level, it_level;

                       _layout_update_bidi_props(o, ln->par);
                       par_level = *(ln->par->bidi_props->embedding_levels);
                       it_level = ln->par->bidi_props->embedding_levels[it->text_pos];

                       if (it_level > par_level)
                         {
                            Evas_Object_Textblock_Item *lastit = it;

                            if (IS_RTL(par_level)) /* RTL par*/
                              {
                                 /*  We know, that all the items before current are of the same or bigger embedding level.
                                  *  So search backwards for the first one. */
                                 while (EINA_INLIST_GET(lastit)->prev)
                                   {
                                      lastit = _EINA_INLIST_CONTAINER(it, EINA_INLIST_GET(lastit)->prev);
                                   }

                                 adv1 = 0;
                                 adv2 = it->adv;
                              }
                            else /* LTR par */
                              {
                                 /*  We know, that all the items after current are of bigger or same embedding level.
                                  *  So search forward for the last one. */
                                 while (EINA_INLIST_GET(lastit)->next)
                                   {
                                      lastit = _EINA_INLIST_CONTAINER(it, EINA_INLIST_GET(lastit)->next);
                                   }

                                 adv1 = lastit->adv;
                                 adv2 = 0;
                              }

                            it1 = lastit;
                            it2 = it;
                         }
                       /* Clear the bidi props because we don't need them anymore. */
                       evas_bidi_paragraph_props_unref(ln->par->bidi_props);
                       ln->par->bidi_props = NULL;
                    }

                  if (it1 && it2)
                    {
                       Evas_Object_Textblock_Line *ln1 = NULL, *ln2 = NULL;
                       ln1 = _find_layout_line_by_item(ln->par, it1);
                       if (cx) *cx = ln1->x + it1->x + adv1;
                       if (cy) *cy = ln1->par->y + ln1->y;
                       if (ch) *ch = ln1->h;

                       ln2 = _find_layout_line_by_item(ln->par, it2);
                       if (cx2) *cx2 = ln2->x + it2->x + adv2;
                       if (cy2) *cy2 = ln2->par->y + ln2->y;
                       if (ch2) *ch2 = ln2->h;

                       return EINA_TRUE;
                    }
               }
          }
     }
#undef IS_DIFFERENT_DIR
#undef IS_RTL
#else
   (void) cx2;
   (void) cy2;
   (void) cw2;
   (void) ch2;
#endif
   evas_textblock_cursor_geometry_get(cur, cx, cy, cw, ch, NULL, ctype);
   return EINA_FALSE;
}

EAPI int
evas_textblock_cursor_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch, Evas_BiDi_Direction *dir, Evas_Textblock_Cursor_Type ctype)
{
   int ret = -1;
   if (!cur) return -1;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

   _relayout_if_needed(cur->obj, o);

   if (ctype == EVAS_TEXTBLOCK_CURSOR_UNDER)
     {
        ret = evas_textblock_cursor_pen_geometry_get(cur, cx, cy, cw, ch);
     }
   else if (ctype == EVAS_TEXTBLOCK_CURSOR_BEFORE)
     {
        /* In the case of a "before cursor", we should get the coordinates
         * of just after the previous char (which in bidi text may not be
         * just before the current char). */
        Evas_Coord x, y, w, h;

        Evas_Object_Textblock_Line *ln;
        Evas_Object_Textblock_Item *it;

        ret = evas_textblock_cursor_pen_geometry_get(cur, &x, &y, &w, &h);
        _find_layout_item_match(cur, &ln, &it);
        if (ret >= 0)
          {
             Evas_BiDi_Direction itdir =
                (it->type == EVAS_TEXTBLOCK_ITEM_TEXT) ?
                _ITEM_TEXT(it)->text_props.bidi_dir :
                _ITEM_FORMAT(it)->bidi_dir;
             if (itdir == EVAS_BIDI_DIRECTION_RTL)
               {
                  if (cx) *cx = x + w;
               }
             else
               {
                  if (cx) *cx = x;
               }
             if (cy) *cy = y;
             if (cw) *cw = 0;
             if (ch) *ch = h;
             if (dir) *dir = itdir;
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
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;
   Evas_Object_Textblock_Text_Item *ti = NULL;
   Evas_Object_Textblock_Format_Item *fi = NULL;
   int x = 0, y = 0, w = 0, h = 0;
   int pos;
   Eina_Bool previous_format;

   if (!cur) return -1;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

   _relayout_if_needed(cur->obj, o);

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

        if (pos < 0) pos = 0;
        if (ti->parent.format->font.font)
          {
             Evas_Object_Protected_Data *obj = eo_data_scope_get(cur->obj, EVAS_OBJ_CLASS);
             query_func(ENDT,
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
   if (!cur) return -1;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(cur->obj, EVAS_OBJ_CLASS);
   return _evas_textblock_cursor_char_pen_geometry_common_get(
         ENFN->font_char_coords_get, cur, cx, cy, cw, ch);
}

EAPI int
evas_textblock_cursor_pen_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   if (!cur) return -1;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(cur->obj, EVAS_OBJ_CLASS);
   return _evas_textblock_cursor_char_pen_geometry_common_get(
         ENFN->font_pen_coords_get, cur, cx, cy, cw, ch);
}

EAPI int
evas_textblock_cursor_line_geometry_get(const Evas_Textblock_Cursor *cur, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Item *it = NULL;
   int x, y, w, h;

   if (!cur) return -1;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

   _relayout_if_needed(cur->obj, o);

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
evas_textblock_cursor_visible_range_get(Evas_Textblock_Cursor *start, Evas_Textblock_Cursor *end)
{
   Evas *eo_e;
   Evas_Coord cy, ch;
   Evas_Object *eo_obj = start->obj;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   TB_HEAD_RETURN(EINA_FALSE);
   eo_e = evas_object_evas_get(eo_obj);
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   cy = 0 - obj->cur->geometry.y;
   ch = e->viewport.h;
   evas_textblock_cursor_line_coord_set(start, cy);
   evas_textblock_cursor_line_coord_set(end, cy + ch);
   evas_textblock_cursor_line_char_last(end);

   return EINA_TRUE;
}

EAPI Eina_Bool
evas_textblock_cursor_char_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord x, Evas_Coord y)
{
   Evas_Object_Textblock_Paragraph *found_par;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock_Item *it = NULL;

   if (!cur) return EINA_FALSE;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

   _relayout_if_needed(cur->obj, o);

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

                  Evas_Object_Protected_Data *obj = eo_data_scope_get(cur->obj, EVAS_OBJ_CLASS);
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
                                   pos = ENFN->font_char_at_coords_get(
                                         ENDT,
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

   if (o->paragraphs)
     {
        Evas_Object_Textblock_Line *first_line = o->paragraphs->lines;
        if (y >= o->paragraphs->y + o->formatted.h)
          {
             /* If we are after the last paragraph, use the last position in the
              * text. */
             evas_textblock_cursor_paragraph_last(cur);
             return EINA_TRUE;
          }
        else if (o->paragraphs && (y < (o->paragraphs->y + first_line->y)))
          {
             evas_textblock_cursor_paragraph_first(cur);
             return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}

EAPI int
evas_textblock_cursor_line_coord_set(Evas_Textblock_Cursor *cur, Evas_Coord y)
{
   Evas_Object_Textblock_Paragraph *found_par;
   Evas_Object_Textblock_Line *ln;

   if (!cur) return -1;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

   _relayout_if_needed(cur->obj, o);

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
   else if (o->paragraphs && (y >= o->paragraphs->y + o->formatted.h))
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
            _ITEM_TEXT(it)->text_props.bidi_dir == EVAS_BIDI_DIRECTION_RTL)
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
            _ITEM_TEXT(it)->text_props.bidi_dir == EVAS_BIDI_DIRECTION_RTL)
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

   if (!cur) return NULL;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(cur->obj, EVAS_OBJ_CLASS);

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
        int ret = 0;

        ti = _ITEM_TEXT(it1);
        if (ti->parent.format->font.font)
          {
             ret = ENFN->font_pen_coords_get(ENDT,
                   ti->parent.format->font.font,
                   &ti->text_props,
                   start,
                   &x1, &y, &w1, &h);
          }
        if (!ret)
          {
             return NULL;
          }
        ret = ENFN->font_pen_coords_get(ENDT,
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
        if (ti->text_props.bidi_dir == EVAS_BIDI_DIRECTION_RTL)
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
   else if ((it1 == it2) && (it1->type != EVAS_TEXTBLOCK_ITEM_TEXT))
     {
        Evas_Coord x, w;
        x = 0;
        w = it1->w;
        _evas_textblock_range_calc_x_w(it1, &x, &w, EINA_TRUE,
                                       switch_items);
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

             ret = ENFN->font_pen_coords_get(ENDT,
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
             if (((it1->text_pos <= it->text_pos) && (it->text_pos <= it2->text_pos)) ||
                   ((it2->text_pos <= it->text_pos) && (it->text_pos <= it1->text_pos)))
               {
                  max_x = it->x + it->adv;
               }
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

             ret = ENFN->font_pen_coords_get(ENDT,
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
             if (end > 0)
               {
                  x = it2->adv;
                  w = 0;
               }
             else
               {
                  x = 0;
                  w = it2->adv;
               }
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
   Evas_Object_Textblock_Line *ln1, *ln2;
   Evas_Object_Textblock_Item *it1, *it2;
   Eina_List *rects = NULL;
   Evas_Textblock_Rectangle *tr;

   if (!cur1 || !cur1->node) return NULL;
   if (!cur2 || !cur2->node) return NULL;
   if (cur1->obj != cur2->obj) return NULL;
   Evas_Textblock_Data *o = eo_data_scope_get(cur1->obj, MY_CLASS);

   _relayout_if_needed(cur1->obj, o);

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
   Evas_Object_Textblock_Line *ln = NULL;
   Evas_Object_Textblock_Format_Item *fi;
   Evas_Object_Textblock_Item *it = NULL;
   Evas_Coord x, y, w, h;

   if (!cur || !evas_textblock_cursor_format_is_visible_get(cur)) return EINA_FALSE;
   Evas_Textblock_Data *o = eo_data_scope_get(cur->obj, MY_CLASS);

   _relayout_if_needed(cur->obj, o);

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
EOLIAN static Eina_Bool
_evas_textblock_line_number_geometry_get(Eo *eo_obj, Evas_Textblock_Data *o, int line, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{

   Evas_Object_Textblock_Line *ln;

   _relayout_if_needed(eo_obj, o);

   ln = _find_layout_line_num(eo_obj, line);
   if (!ln) return EINA_FALSE;
   if (cx) *cx = ln->x;
   if (cy) *cy = ln->par->y + ln->y;
   if (cw) *cw = ln->w;
   if (ch) *ch = ln->h;
   return EINA_TRUE;
}

static void
_evas_object_textblock_clear_all(Evas_Object *eo_obj)
{
   eo_do(eo_obj, evas_obj_textblock_clear());
}

EOLIAN static void
_evas_textblock_clear(Eo *eo_obj, Evas_Textblock_Data *o)
{
   Eina_List *l;
   Evas_Textblock_Cursor *cur;

   if (o->paragraphs)
     {
	_paragraphs_free(eo_obj, o->paragraphs);
	o->paragraphs = NULL;
     }

   _nodes_clear(eo_obj);
   o->cursor->node = NULL;
   o->cursor->pos = 0;
   EINA_LIST_FOREACH(o->cursors, l, cur)
     {
	cur->node = NULL;
	cur->pos = 0;

     }

   _evas_textblock_changed(o, eo_obj);
}

EAPI void
evas_object_textblock_clear(Evas_Object *eo_obj)
{
   TB_HEAD();
   _evas_object_textblock_clear_all(eo_obj);

   /* Force recreation of everything for textblock.
    * FIXME: We have the same thing in other places, merge it... */
   evas_textblock_cursor_paragraph_first(o->cursor);
   evas_textblock_cursor_text_append(o->cursor, "");
}

EOLIAN static void
_evas_textblock_size_formatted_get(Eo *eo_obj, Evas_Textblock_Data *o, Evas_Coord *w, Evas_Coord *h)
{
   _relayout_if_needed(eo_obj, o);

   if (w) *w = o->formatted.w;
   if (h) *h = o->formatted.h;
}

static void
_size_native_calc_line_finalize(const Evas_Object *eo_obj, Eina_List *items,
      Evas_Coord *ascent, Evas_Coord *descent, Evas_Coord *w, Textblock_Position position)
{
   Evas_Object_Textblock_Item *it, *last_it = NULL;
   Eina_List *i;

   it = eina_list_data_get(items);
   *w = 0;

   if (it)
     {
        Evas_Coord asc = 0, desc = 0;
        /* If there are no text items yet, calc ascent/descent
         * according to the current format. */
        _layout_item_ascent_descent_adjust(eo_obj, &asc, &desc,
              it, it->format);

        if (asc > *ascent)
           *ascent = asc;
        if (desc > *descent)
           *descent = desc;

        /* Add margins. */
        if (it->format)
           *w = it->format->margin.l + it->format->margin.r;
      }


   /* Adjust all the item sizes according to the final line size,
    * and update the x positions of all the items of the line. */
   EINA_LIST_FOREACH(items, i, it)
     {
        if (it->type == EVAS_TEXTBLOCK_ITEM_FORMAT)
          {
             Evas_Coord fw, fh, fy;

             Evas_Object_Textblock_Format_Item *fi = _ITEM_FORMAT(it);
             if (!fi->formatme) goto loop_advance;
             _layout_calculate_format_item_size(eo_obj, fi, ascent,
                   descent, &fy, &fw, &fh);
          }
        else
          {
             Evas_Coord maxasc = 0, maxdesc = 0;
             _layout_item_ascent_descent_adjust(eo_obj, ascent, descent,
                   it, it->format);
             _layout_item_max_ascent_descent_calc(eo_obj, &maxasc, &maxdesc,
                   it, position);

             if (maxasc > *ascent)
                *ascent = maxasc;
             if (maxdesc > *descent)
                *descent = maxdesc;
          }

loop_advance:
        *w += it->adv;

        if (!last_it || (it->visual_pos > last_it->visual_pos))
           last_it = it;
     }

   if (last_it)
      *w += last_it->w - last_it->adv;
}

/* FIXME: doc */
static void
_size_native_calc_paragraph_size(const Evas_Object *eo_obj,
      const Evas_Textblock_Data *o,
      const Evas_Object_Textblock_Paragraph *par,
      Textblock_Position *position,
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
                  _size_native_calc_line_finalize(eo_obj, line_items, &ascent,
                        &descent, &w, *position);

                  if (ascent + descent > h)
                     h = ascent + descent;

                  y += h;
                  if (w > wmax)
                     wmax = w;
                  h = 0;
                  ascent = descent = 0;
                  *position = TEXTBLOCK_POSITION_ELSE;
                  line_items = eina_list_free(line_items);
               }
             else
               {
                  Evas_Coord fw, fh, fy;
                  /* If there are no text items yet, calc ascent/descent
                   * according to the current format. */
                  if (it && (ascent + descent == 0))
                     _layout_item_ascent_descent_adjust(eo_obj, &ascent,
                           &descent, it, it->format);

                  _layout_calculate_format_item_size(eo_obj, fi, &ascent,
                        &descent, &fy, &fw, &fh);
               }
          }
        else
          {
             _layout_item_ascent_descent_adjust(eo_obj, &ascent,
                   &descent, it, it->format);
          }
     }

   if (!EINA_INLIST_GET(par)->next)
     {
        *position = (*position == TEXTBLOCK_POSITION_START) ?
           TEXTBLOCK_POSITION_SINGLE : TEXTBLOCK_POSITION_END;
     }
   _size_native_calc_line_finalize(eo_obj, line_items, &ascent, &descent, &w, *position);

   if (*position == TEXTBLOCK_POSITION_START)
      *position = TEXTBLOCK_POSITION_ELSE;

   line_items = eina_list_free(line_items);

   /* Do the last addition */
   if (ascent + descent > h)
      h = ascent + descent;

   if (w > wmax)
      wmax = w;

   *_h = y + h;
   *_w = wmax;
}

EOLIAN static void
_evas_textblock_size_native_get(Eo *eo_obj, Evas_Textblock_Data *o, Evas_Coord *w, Evas_Coord *h)
{
   if (!o->native.valid)
     {
        Evas_Coord wmax = 0, hmax = 0;
        Evas_Object_Textblock_Paragraph *par;
        Textblock_Position position = TEXTBLOCK_POSITION_START;
        /* We just want the layout objects to update, should probably
         * split that. */
        _relayout_if_needed(eo_obj, o);

        EINA_INLIST_FOREACH(o->paragraphs, par)
          {
             Evas_Coord tw, th;
             _size_native_calc_paragraph_size(eo_obj, o, par, &position, &tw, &th);
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

EOLIAN static void
_evas_textblock_style_insets_get(Eo *eo_obj, Evas_Textblock_Data *o, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b)
{
   _relayout_if_needed(eo_obj, o);

   if (l) *l = o->style_pad.l;
   if (r) *r = o->style_pad.r;
   if (t) *t = o->style_pad.t;
   if (b) *b = o->style_pad.b;
}

EOLIAN static void
_evas_textblock_eo_base_dbg_info_get(Eo *eo_obj, Evas_Textblock_Data *o EINA_UNUSED, Eo_Dbg_Info *root)
{
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   if (!root) return;
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);
   Eo_Dbg_Info *node;

   const char *style;
   const char *text;
   char shorttext[48];
   const Evas_Textblock_Style *ts;

   eo_do(eo_obj, evas_obj_textblock_style_get(&ts));
   style = evas_textblock_style_get(ts);
   eo_do(eo_obj, evas_obj_textblock_text_markup_get(&text));
   strncpy(shorttext, text, 38);
   if (shorttext[37])
     strcpy(shorttext + 37, "\xe2\x80\xa6"); /* HORIZONTAL ELLIPSIS */

   EO_DBG_INFO_APPEND(group, "Style", EINA_VALUE_TYPE_STRING, style);
   EO_DBG_INFO_APPEND(group, "Text", EINA_VALUE_TYPE_STRING, shorttext);

     {
        int w, h;
        eo_do(eo_obj, evas_obj_textblock_size_formatted_get(&w, &h));
        node = EO_DBG_INFO_LIST_APPEND(group, "Formatted size");
        EO_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, w);
        EO_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, h);
     }

     {
        int w, h;
        eo_do(eo_obj, evas_obj_textblock_size_native_get(&w, &h));
        node = EO_DBG_INFO_LIST_APPEND(group, "Native size");
        EO_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, w);
        EO_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, h);
     }
}

/* all nice and private */
static void
evas_object_textblock_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Textblock_Data *o;
   static Eina_Bool linebreak_init = EINA_FALSE;

   if (!linebreak_init)
     {
        linebreak_init = EINA_TRUE;
        init_linebreak();
        init_wordbreak();
     }

   o = obj->private_data;
   o->cursor->obj = eo_obj;
   evas_object_textblock_text_markup_set(eo_obj, "");

   o->legacy_newline = EINA_TRUE;
}

EOLIAN static void
_evas_textblock_destructor(Eo *eo_obj, Evas_Textblock_Data *o EINA_UNUSED)
{
   evas_object_textblock_free(eo_obj);
   eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

static void
evas_object_textblock_free(Evas_Object *eo_obj)
{
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

   _evas_object_textblock_clear_all(eo_obj);
   evas_object_textblock_style_set(eo_obj, NULL);
   while (evas_object_textblock_style_user_peek(eo_obj))
     {
        evas_object_textblock_style_user_pop(eo_obj);
     }
   free(o->cursor);
   while (o->cursors)
     {
	Evas_Textblock_Cursor *cur;

	cur = (Evas_Textblock_Cursor *)o->cursors->data;
	o->cursors = eina_list_remove_list(o->cursors, o->cursors);
	free(cur);
     }
   if (o->repch) eina_stringshare_del(o->repch);
   if (o->ellip_ti) _item_free(eo_obj, NULL, _ITEM(o->ellip_ti));
   o->magic = 0;
  _format_command_shutdown();
}


static void
evas_object_textblock_render(Evas_Object *eo_obj EINA_UNUSED,
			     Evas_Object_Protected_Data *obj,
			     void *type_private_data,
			     void *output, void *context, void *surface,
			     int x, int y, Eina_Bool do_async)
{
   Evas_Object_Textblock_Paragraph *par, *start = NULL;
   Evas_Object_Textblock_Item *itr;
   Evas_Object_Textblock_Line *ln;
   Evas_Textblock_Data *o = type_private_data;
   Eina_List *shadows = NULL;
   Eina_List *glows = NULL;
   Eina_List *outlines = NULL;
   int i, j;
   int cx, cy, cw, ch, clip;
   int ca, cr, cg, cb;
   int na, nr, ng, nb;
   const char vals[5][5] =
     {
	  {0, 1, 2, 1, 0},
	  {1, 3, 4, 3, 1},
	  {2, 4, 5, 4, 2},
	  {1, 3, 4, 3, 1},
	  {0, 1, 2, 1, 0}
     };

   /* render object to surface with context, and offxet by x,y */
   obj->layer->evas->engine.func->context_multiplier_unset(output,
							   context);
   ENFN->context_render_op_set(output, context, obj->cur->render_op);
   /* FIXME: This clipping is just until we fix inset handling correctly. */
   ENFN->context_clip_clip(output, context,
                              obj->cur->geometry.x + x,
                              obj->cur->geometry.y + y,
                              obj->cur->geometry.w,
                              obj->cur->geometry.h);
   clip = ENFN->context_clip_get(output, context, &cx, &cy, &cw, &ch);
   /* If there are no paragraphs and thus there are no lines,
    * there's nothing left to do. */
   if (!o->paragraphs) return;

   ENFN->context_color_set(output, context, 0, 0, 0, 0);
   ca = cr = cg = cb = 0;

#define ITEM_WALK() \
   EINA_INLIST_FOREACH(start, par) \
     { \
        if (!par->visible) continue; \
        if (clip) \
          { \
             if ((obj->cur->geometry.y + y + par->y + par->h) < (cy - 20)) \
             continue; \
             if ((obj->cur->geometry.y + y + par->y) > (cy + ch + 20)) \
             break; \
          } \
        _layout_paragraph_render(o, par); \
        EINA_INLIST_FOREACH(par->lines, ln) \
          { \
             if (clip) \
               { \
                  if ((obj->cur->geometry.y + y + par->y + ln->y + ln->h) < (cy - 20)) \
                  continue; \
                  if ((obj->cur->geometry.y + y + par->y + ln->y) > (cy + ch + 20)) \
                  break; \
               } \
             EINA_INLIST_FOREACH(ln->items, itr) \
               { \
                  Evas_Coord yoff; \
                  yoff = ln->baseline; \
                  if (itr->format->valign != -1.0) \
                    { \
                       if (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) \
                         { \
                            Evas_Object_Textblock_Text_Item *titr = \
                              (Evas_Object_Textblock_Text_Item *)itr; \
                            int ascent = 0; \
                            if (titr->text_props.font_instance) \
                              ascent = evas_common_font_instance_max_ascent_get(titr->text_props.font_instance); \
                            yoff = ascent + \
                              (itr->format->valign * (ln->h - itr->h)); \
                         } \
                       else yoff = itr->format->valign * (ln->h - itr->h); \
                    } \
                  itr->yoff = yoff;             \
                  if (clip) \
                    { \
                       if ((obj->cur->geometry.x + x + ln->x + itr->x + itr->w) < (cx - 20)) \
                       continue; \
                       if ((obj->cur->geometry.x + x + ln->x + itr->x) > (cx + cw + 20)) \
                       break; \
                    } \
                  if ((ln->x + itr->x + itr->w) <= 0) continue; \
                  if (ln->x + itr->x > obj->cur->geometry.w) break; \
                  do

#define ITEM_WALK_END() \
                  while (0); \
               } \
          } \
     } \
   do {} while(0)
#define COLOR_SET(col)                                                  \
   nr = obj->cur->cache.clip.r * ti->parent.format->color.col.r;        \
   ng = obj->cur->cache.clip.g * ti->parent.format->color.col.g;        \
   nb = obj->cur->cache.clip.b * ti->parent.format->color.col.b;        \
   na = obj->cur->cache.clip.a * ti->parent.format->color.col.a;        \
   if (na != ca || nb != cb || ng != cg || nr != cr)                    \
     {                                                                  \
        ENFN->context_color_set(output, context,                        \
                                nr / 255, ng / 255, nb / 255, na / 255); \
        cr = nr; cg = ng; cb = nb; ca = na;                             \
     }
#define COLOR_SET_AMUL(col, amul)                                       \
   nr = obj->cur->cache.clip.r * ti->parent.format->color.col.r * (amul); \
   ng = obj->cur->cache.clip.g * ti->parent.format->color.col.g * (amul); \
   nb = obj->cur->cache.clip.b * ti->parent.format->color.col.b * (amul); \
   na = obj->cur->cache.clip.a * ti->parent.format->color.col.a * (amul); \
   if (na != ca || nb != cb || ng != cg || nr != cr)                    \
     {                                                                  \
        ENFN->context_color_set(output, context,                        \
                                nr / 65025, ng / 65025, nb / 65025, na / 65025); \
        cr = nr; cg = ng; cb = nb; ca = na;                             \
     }
#define DRAW_TEXT(ox, oy)                                               \
   if (ti->parent.format->font.font)                                    \
     evas_font_draw_async_check(obj, output, context, surface,          \
        ti->parent.format->font.font,                                   \
        obj->cur->geometry.x + ln->x + ti->parent.x + x + (ox),          \
        obj->cur->geometry.y + ln->par->y + ln->y + yoff + y + (oy),     \
        ti->parent.w, ti->parent.h, ti->parent.w, ti->parent.h,         \
        &ti->text_props, do_async);

   /* backing */
#define DRAW_RECT(ox, oy, ow, oh, or, og, ob, oa)                       \
   do                                                                   \
     {                                                                  \
        nr = obj->cur->cache.clip.r * or;                               \
        ng = obj->cur->cache.clip.g * og;                               \
        nb = obj->cur->cache.clip.b * ob;                               \
        na = obj->cur->cache.clip.a * oa;                               \
        if (na != ca || nb != cb || ng != cg || nr != cr)               \
          {                                                             \
             ENFN->context_color_set(output, context,                   \
                                     nr / 255, ng / 255, nb / 255, na / 255); \
             cr = nr; cg = ng; cb = nb; ca = na;                        \
          }                                                             \
        ENFN->rectangle_draw(output,                                    \
                             context,                                   \
                             surface,                                   \
                             obj->cur->geometry.x + ln->x + x + (ox),   \
                             obj->cur->geometry.y + ln->par->y + ln->y + y + (oy), \
                             (ow),                                      \
                             (oh),                                      \
                             do_async);                                 \
     }                                                                  \
   while (0)

#define DRAW_FORMAT_DASHED(oname, oy, oh, dw, dp) \
   do \
     { \
        if (itr->format->oname) \
          { \
             unsigned char _or, _og, _ob, _oa; \
             int _ind, _dx = 0, _dn, _dr; \
             _or = itr->format->color.oname.r; \
             _og = itr->format->color.oname.g; \
             _ob = itr->format->color.oname.b; \
             _oa = itr->format->color.oname.a; \
             if (!EINA_INLIST_GET(itr)->next) \
               { \
                  _dn = itr->w / (dw + dp); \
                  _dr = itr->w % (dw + dp); \
               } \
             else \
               { \
                  _dn = itr->adv / (dw + dp); \
                  _dr = itr->adv % (dw + dp); \
               } \
             if (_dr > dw) _dr = dw; \
             for (_ind = 0 ; _ind < _dn ; _ind++) \
               { \
                  DRAW_RECT(itr->x + _dx, oy, dw, oh, _or, _og, _ob, _oa); \
                  _dx += dw + dp; \
               } \
             DRAW_RECT(itr->x + _dx, oy, _dr, oh, _or, _og, _ob, _oa); \
          } \
     } \
   while (0)

#define DRAW_FORMAT(oname, oy, oh) \
   do \
     { \
        if (itr->format->oname) \
          { \
             unsigned char _or, _og, _ob, _oa; \
             _or = itr->format->color.oname.r; \
             _og = itr->format->color.oname.g; \
             _ob = itr->format->color.oname.b; \
             _oa = itr->format->color.oname.a; \
             DRAW_RECT(itr->x, oy, itr->adv, oh, _or, _og, _ob, _oa); \
          } \
     } \
   while (0)

     {
        Evas_Coord look_for_y = 0 - (obj->cur->geometry.y + y);
        if (clip)
          {
             Evas_Coord tmp_lfy = cy - (obj->cur->geometry.y + y);
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
        /* Check which other pass are necessary to avoid useless WALK */
        Evas_Object_Textblock_Text_Item *ti;

        ti = (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? _ITEM_TEXT(itr) : NULL;
        if (ti)
          {
             if (ti->parent.format->style & (EVAS_TEXT_STYLE_SHADOW |
                                             EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW |
                                             EVAS_TEXT_STYLE_OUTLINE_SHADOW |
                                             EVAS_TEXT_STYLE_FAR_SHADOW |
                                             EVAS_TEXT_STYLE_FAR_SOFT_SHADOW |
                                             EVAS_TEXT_STYLE_SOFT_SHADOW))
               {
                  shadows = eina_list_append(shadows, itr);
               }
             if ((ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC) ==
                 EVAS_TEXT_STYLE_GLOW)
               {
                  glows = eina_list_append(glows, itr);
               }
             if (((ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC) == EVAS_TEXT_STYLE_OUTLINE) ||
                 ((ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC) == EVAS_TEXT_STYLE_OUTLINE_SHADOW) ||
                 ((ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC) == EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW) ||
                 (ti->parent.format->style == EVAS_TEXT_STYLE_SOFT_OUTLINE))
               {
                  outlines = eina_list_append(outlines, itr);
               }
          }

        /* Draw background */
        DRAW_FORMAT(backing, 0, ln->h);
     }
   ITEM_WALK_END();

   /* There are size adjustments that depend on the styles drawn here back
    * in "_text_item_update_sizes" should not modify one without the other. */

   /* prepare everything for text draw */

   /* shadows */
   EINA_LIST_FREE(shadows, itr)
     {
        int shad_dst, shad_sz, dx, dy, haveshad;
        Evas_Object_Textblock_Text_Item *ti;
        Evas_Coord yoff;

        ti = (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? _ITEM_TEXT(itr) : NULL;
        if (!ti) continue;

        yoff = itr->yoff;
        ln = itr->ln;

        shad_dst = shad_sz = dx = dy = haveshad = 0;
        switch (ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC)
          {
           case EVAS_TEXT_STYLE_SHADOW:
              shad_dst = 1;
              haveshad = 1;
              break;
           case EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW:
              shad_dst = 1;
              shad_sz = 2;
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

   /* glows */
   EINA_LIST_FREE(glows, itr)
     {
        Evas_Object_Textblock_Text_Item *ti;
        Evas_Coord yoff;

        ti = (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? _ITEM_TEXT(itr) : NULL;
        if (!ti) continue;

        yoff = itr->yoff;
        ln = itr->ln;

        if ((ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC) == EVAS_TEXT_STYLE_GLOW)
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

   /* outlines */
   EINA_LIST_FREE(outlines, itr)
     {
        Evas_Object_Textblock_Text_Item *ti;
        Evas_Coord yoff;

        ti = (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? _ITEM_TEXT(itr) : NULL;
        if (!ti) continue;

        yoff = itr->yoff;
        ln = itr->ln;

        if (((ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC) == EVAS_TEXT_STYLE_OUTLINE) ||
            ((ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC) == EVAS_TEXT_STYLE_OUTLINE_SHADOW) ||
            ((ti->parent.format->style & EVAS_TEXT_STYLE_MASK_BASIC) == EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW))
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

   /* normal text and lines */
   /* Get the thickness and position, and save them for non-text items. */
   int line_thickness =
           evas_common_font_instance_underline_thickness_get(NULL);
   int line_position =
           evas_common_font_instance_underline_position_get(NULL);
   ITEM_WALK()
     {
        Evas_Object_Textblock_Text_Item *ti;
        ti = (itr->type == EVAS_TEXTBLOCK_ITEM_TEXT) ? _ITEM_TEXT(itr) : NULL;
        /* NORMAL TEXT */
        if (ti)
          {
             void *fi = _ITEM_TEXT(itr)->text_props.font_instance;
             COLOR_SET(normal);
             DRAW_TEXT(0, 0);
             line_thickness =
                evas_common_font_instance_underline_thickness_get(fi);
             line_position =
                evas_common_font_instance_underline_position_get(fi);
          }

        /* STRIKETHROUGH */
        DRAW_FORMAT(strikethrough, (ln->h / 2), line_thickness);

        /* UNDERLINE */
        DRAW_FORMAT(underline, ln->baseline + line_position, line_thickness);

        /* UNDERLINE DASHED */
        DRAW_FORMAT_DASHED(underline_dash, ln->baseline + line_position,
                         line_thickness,
                         ti->parent.format->underline_dash_width,
                         ti->parent.format->underline_dash_gap);

        /* UNDERLINE2 */
        DRAW_FORMAT(underline2, ln->baseline + line_position + line_thickness +
              line_position, line_thickness);
     }
   ITEM_WALK_END();
}

static void
evas_object_textblock_coords_recalc(Evas_Object *eo_obj EINA_UNUSED,
                                    Evas_Object_Protected_Data *obj,
                                    void *type_private_data)
{
   Evas_Textblock_Data *o = type_private_data;

   if (
       // width changed thus we may have to re-wrap or change centering etc.
       (obj->cur->geometry.w != o->last_w) ||
       // if valign not top OR we have ellipsis, then if height changed we need to re-eval valign or ... spot
       (((o->valign != 0.0) || (o->have_ellipsis)) &&
           (
               ((o->formatted.oneline_h == 0) &&
                   (obj->cur->geometry.h != o->last_h)) ||
               ((o->formatted.oneline_h != 0) &&
                   (((obj->cur->geometry.h != o->last_h) &&
                     (o->formatted.oneline_h < obj->cur->geometry.h))))
           )
       ) ||
       // obviously if content text changed we need to reformat it
       (o->content_changed) ||
       // if format changed (eg styles) we need to re-format/match tags etc.
       (o->format_changed)
      )
     {
        LYDBG("ZZ: invalidate 2 %p ## %i != %i || %3.3f || %i && %i != %i | %i %i\n", eo_obj, obj->cur->geometry.w, o->last_w, o->valign, o->have_ellipsis, obj->cur->geometry.h, o->last_h, o->content_changed, o->format_changed);
	o->formatted.valid = 0;
	o->changed = 1;
     }
}

static void
evas_object_textblock_render_pre(Evas_Object *eo_obj,
				 Evas_Object_Protected_Data *obj,
				 void *type_private_data)
{
   Evas_Textblock_Data *o = type_private_data;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = EINA_TRUE;

   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw textblocks */

   evas_object_textblock_coords_recalc(eo_obj, obj, obj->private_data);
   if (o->changed)
     {
        LYDBG("ZZ: relayout 16\n");
        _relayout(eo_obj);
        o->redraw = 0;
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        is_v = evas_object_is_visible(eo_obj, obj);
        was_v = evas_object_was_visible(eo_obj, obj);
        goto done;
     }

   if (o->redraw)
     {
        o->redraw = 0;
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        is_v = evas_object_is_visible(eo_obj, obj);
        was_v = evas_object_was_visible(eo_obj, obj);
        goto done;
     }
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur->clipper)
     {
        if (obj->cur->cache.clip.dirty)
          evas_object_clip_recalc(obj->cur->clipper);
        obj->cur->clipper->func->render_pre(obj->cur->clipper->object,
                                            obj->cur->clipper,
                                            obj->cur->clipper->private_data);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(eo_obj, obj);
   was_v = evas_object_was_visible(eo_obj, obj);
   if (is_v != was_v)
     {
        evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes,
                                              eo_obj, is_v, was_v);
        goto done;
     }
   if (obj->changed_map || obj->changed_src_visible)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
   /* it's not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes,
                                         eo_obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
   /* if it changed color */
   if ((obj->cur->color.r != obj->prev->color.r) ||
       (obj->cur->color.g != obj->prev->color.g) ||
       (obj->cur->color.b != obj->prev->color.b) ||
       (obj->cur->color.a != obj->prev->color.a))
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur->geometry.x != obj->prev->geometry.x) ||
       (obj->cur->geometry.y != obj->prev->geometry.y) ||
       (obj->cur->geometry.w != obj->prev->geometry.w) ||
       (obj->cur->geometry.h != obj->prev->geometry.h))
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
   if (obj->cur->render_op != obj->prev->render_op)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
done:
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes,
                                         eo_obj, is_v, was_v);
}

static void
evas_object_textblock_render_post(Evas_Object *eo_obj,
                                  Evas_Object_Protected_Data *obj EINA_UNUSED,
                                  void *type_private_data EINA_UNUSED)
{
   /*   Evas_Textblock_Data *o; */

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
/*   o = (Evas_Textblock_Data *)(obj->object_data); */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(eo_obj);
   /* move cur to prev safely for object data */
   evas_object_cur_prev(eo_obj);
/*   o->prev = o->cur; */
}

static unsigned int evas_object_textblock_id_get(Evas_Object *eo_obj)
{
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_TEXTBLOCK;
}

static unsigned int evas_object_textblock_visual_id_get(Evas_Object *eo_obj)
{
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_CUSTOM;
}

static void *evas_object_textblock_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->engine_data;
}

static int
evas_object_textblock_is_opaque(Evas_Object *eo_obj EINA_UNUSED,
                                Evas_Object_Protected_Data *obj EINA_UNUSED,
                                void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object is */
   /* currently fulyl opque over the entire gradient it occupies */
   return 0;
}

static int
evas_object_textblock_was_opaque(Evas_Object *eo_obj EINA_UNUSED,
                                 Evas_Object_Protected_Data *obj EINA_UNUSED,
                                 void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object was */
   /* currently fulyl opque over the entire gradient it occupies */
   return 0;
}

static void
evas_object_textblock_scale_update(Evas_Object *eo_obj EINA_UNUSED,
                                   Evas_Object_Protected_Data *obj EINA_UNUSED,
                                   void *type_private_data)
{
   Evas_Textblock_Data *o = type_private_data;
   _evas_textblock_invalidate_all(o);
   _evas_textblock_changed(o, eo_obj);
   o->last_w = -1;
   o->last_h = -1;
}

void
_evas_object_textblock_rehint(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   Evas_Object_Textblock_Paragraph *par;
   Evas_Object_Textblock_Line *ln;

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
                            evas_font_load_hinting_set(obj->layer->evas->evas,
                                  ti->parent.format->font.font,
                                  obj->layer->evas->hinting);
                         }
                    }
               }
          }
     }
   _evas_textblock_invalidate_all(o);
   _evas_textblock_changed(o, eo_obj);
}

/**
 * @}
 */

#ifdef HAVE_TESTS
/* return EINA_FALSE on error, used in unit_testing */
EAPI Eina_Bool
_evas_textblock_check_item_node_link(Evas_Object *eo_obj)
{
   Evas_Textblock_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   Evas_Object_Textblock_Paragraph *par;
   Evas_Object_Textblock_Line *ln;
   Evas_Object_Textblock_Item *it;

   if (!o) return EINA_FALSE;

   _relayout_if_needed(eo_obj, o);

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
EAPI void
pfnode(Evas_Object_Textblock_Node_Format *n)
{
   printf("Format Node: %p\n", n);
   printf("next = %p, prev = %p, last = %p\n", EINA_INLIST_GET(n)->next, EINA_INLIST_GET(n)->prev, EINA_INLIST_GET(n)->last);
   printf("text_node = %p, offset = %u, visible = %d\n", n->text_node, (unsigned int) n->offset, n->visible);
   printf("'%s'\n", n->format);
}

EAPI void
ptnode(Evas_Object_Textblock_Node_Text *n)
{
   printf("Text Node: %p\n", n);
   printf("next = %p, prev = %p, last = %p\n", EINA_INLIST_GET(n)->next, EINA_INLIST_GET(n)->prev, EINA_INLIST_GET(n)->last);
   printf("format_node = %p\n", n->format_node);
   printf("'%ls'\n", eina_ustrbuf_string_get(n->unicode));
}

EAPI void
pitem(Evas_Object_Textblock_Item *it)
{
   Evas_Object_Textblock_Text_Item *ti;
   Evas_Object_Textblock_Format_Item *fi;
   printf("Item: %p %s\n", it, (it->visually_deleted) ? "(visually deleted)" : "");
   printf("Type: %s (%d)\n", (it->type == EVAS_TEXTBLOCK_ITEM_TEXT) ?
         "TEXT" : "FORMAT", it->type);
   printf("Text pos: %u Visual pos: %u\n", (unsigned int) it->text_pos, (unsigned int)
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
        Eina_Unicode *tmp;
        ti = _ITEM_TEXT(it);
        tmp = eina_unicode_strdup(GET_ITEM_TEXT(ti));
        tmp[ti->text_props.text_len] = '\0';
        printf("Text: '%ls'\n", tmp);
        free(tmp);
     }
   else
     {
        fi = _ITEM_FORMAT(it);
        printf("Format: '%s'\n", fi->item);
     }
}

EAPI void
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

#include "canvas/evas_textblock.eo.c"
