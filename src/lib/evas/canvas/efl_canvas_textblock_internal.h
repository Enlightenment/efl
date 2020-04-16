#ifndef _EFL_CANVAS_TEXTBLOCK_INTERNAL_H
#define _EFL_CANVAS_TEXTBLOCK_INTERNAL_H

#include <Evas.h>

#define _REPLACEMENT_CHAR_UTF8 "\xEF\xBF\xBC"
#define _PARAGRAPH_SEPARATOR_UTF8 "\xE2\x80\xA9"
#define _NEWLINE_UTF8 "\n"
#define _TAB_UTF8 "\t"

/* private struct for textblock object internal data */
/**
 * @internal
 * @typedef Efl_Canvas_Textblock_Data
 * The actual textblock object.
 */
typedef struct _Evas_Object_Textblock             Efl_Canvas_Textblock_Data;
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
typedef struct _Evas_Textblock_Node_Format Evas_Textblock_Node_Format;

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
 * @typedef Evas_Textblock_Selection_Iterator
 * A textblock selection iterator.
 */
typedef struct _Evas_Textblock_Selection_Iterator Evas_Textblock_Selection_Iterator;
/**
 * @internal
 * @typedef Efl_Text_Attribute_Handle_Iterator
 * A textblock annotation iterator.
 */
typedef struct _Efl_Text_Attribute_Handle_Iterator Efl_Text_Attribute_Handle_Iterator;
/**
 * @internal
 * @typedef Efl_Canvas_Textblock_Filter
 * A structure holding gfx filter information for a text item
 */
typedef struct _Efl_Canvas_Textblock_Filter Efl_Canvas_Textblock_Filter;
/**
 * @internal
 * @typedef Efl_Canvas_Textblock_Filter_Post_Render
 * Post-render data for async rendering of gfx filters
 */
typedef struct _Efl_Canvas_Textblock_Filter_Post_Render Efl_Canvas_Textblock_Filter_Post_Render;
/**
 * @internal
 * @typedef Efl_Canvas_Textblock_Filter_Program
 * Filter name - code database
 */
typedef struct _Efl_Canvas_Textblock_Filter_Program Efl_Canvas_Textblock_Filter_Program;
/**
 * @internal
 * @typedef Text_Item_Filter
 * Text item filter properties (run context, output buffer, ...)
 */
typedef struct _Text_Item_Filter Text_Item_Filter;


struct _Evas_Object_Style_Tag_Base
{
   const char *tag;  /**< Format Identifier: b=Bold, i=Italic etc. */
   const char *replace;  /**< Replacement string. "font_weight=Bold", "font_style=Italic" etc. */
   size_t tag_len;  /**< Strlen of tag. */
};

struct _Evas_Object_Style_Tag
{
   EINA_INLIST;
   Evas_Object_Style_Tag_Base tag;  /**< Base style object for holding style information. */
};

struct _Evas_Object_Textblock_Node_Text
{
   EINA_INLIST;
   Eina_UStrbuf                       *unicode;  /**< Actual paragraph text. */
   char                               *utf8;  /**< Text in utf8 format. */
   Evas_Object_Textblock_Node_Format  *format_node; /**< Points to the last format node before the paragraph, or if there is none, to the first format node within the paragraph.*/
   Evas_Object_Textblock_Paragraph    *par;  /**< Points to the paragraph node of which this node is a part. */
   Eina_Bool                           dirty : 1;  /**< EINA_TRUE if already handled/format changed, else EINA_FALSE. */
   Eina_Bool                           is_new : 1;  /**< EINA_TRUE if its a new paragraph, else EINA_FALSE. */
};

struct _Evas_Textblock_Node_Format
{
   EINA_INLIST;
   const char                         *format;  /**< Cached, parsed and translated version of orig_format. */
   const char                         *orig_format;  /**< Original format information. */
   Evas_Object_Textblock_Node_Text    *text_node;  /**< The text node it's pointing to. */
   Efl_Text_Attribute_Handle          *annotation; /**< Pointer to this node's annotation handle (if exists). */
   size_t                              offset;  /**< Offset from the last format node of the same text. */
   struct {
      unsigned char l, r, t, b;
   } pad;  /**< Amount of padding required. */
   unsigned char                       anchor : 2;  /**< ANCHOR_NONE, ANCHOR_A or ANCHOR_ITEM. */
   Eina_Bool                           opener : 1;  /**< EINA_TRUE if opener, else EINA_FALSE. */
   Eina_Bool                           own_closer : 1;  /**< EINA_TRUE if own_closer, else EINA_FALSE. */
   Eina_Bool                           visible : 1;  /**< EINA_TRUE if format is visible format, else EINA_FALSE. */
   Eina_Bool                           format_change : 1;  /**< EINA_TRUE if the format of the textblock has changed, else EINA_FALSE. */
   Eina_Bool                           is_new : 1;  /**< EINA_TRUE if its a new format node, else EINA_FALSE */
};

struct _Efl_Text_Cursor_Handle
{
   Evas_Object                     *obj;
   Eina_List                       *cur_objs;
   size_t                           pos;
   Evas_Object_Textblock_Node_Text *node;
   unsigned int                     ref_count;
   Eina_Bool                        changed : 1;
};

struct _Efl_Text_Attribute_Handle
{
   EINA_INLIST;
   Evas_Object                       *obj;
   Evas_Object_Textblock_Node_Format *start_node, *end_node;
   Eina_Bool                         is_item : 1; /**< indicates it is an item/object placeholder */
};

void evas_textblock_cursor_line_jump_by(Efl_Text_Cursor_Handle *cur, int by);
int _cursor_text_append(Efl_Text_Cursor_Handle *cur, const char *text);
void evas_textblock_async_block(Evas_Object *eo_object);


// Used in Efl.Text.Cursor, where multible objects can have same handle.
Efl_Text_Cursor_Handle *
evas_textblock_cursor_ref(Efl_Text_Cursor_Handle *cursor, Eo * cursor_obj);

// Used in Efl.Text.Cursor, where multible objects can have same handle.
void
evas_textblock_cursor_unref(Efl_Text_Cursor_Handle *cursor, Eo * cursor_obj);
void _evas_textblock_cursor_init(Efl_Text_Cursor_Handle *cur, const Evas_Object *tb);

/*Annoation Functions*/
/**
  * @internal
  * Returns the value of the current data of list node,
  * and goes to the next list node.
  *
  * @param it the iterator.
  * @param data the data of the current list node.
  * @return EINA_FALSE if unsuccessful. Otherwise, returns EINA_TRUE.
  */
Eina_Bool
_evas_textblock_annotation_iterator_next(Efl_Text_Attribute_Handle_Iterator *it, void **data);

/**
  * @internal
  * Frees the annotation iterator.
  * @param it the iterator to free
  * @return EINA_FALSE if unsuccessful. Otherwise, returns EINA_TRUE.
  */
void
_evas_textblock_annotation_iterator_free(Efl_Text_Attribute_Handle_Iterator *it);


/**
  * @internal
  * Creates newly allocated  iterator associated to a list.
  * @param list The list.
  * @return If the memory cannot be allocated, NULL is returned.
  * Otherwise, a valid iterator is returned.
  */
Eina_Iterator *
_evas_textblock_annotation_iterator_new(Eina_List *list);



void
_textblock_cursor_pos_at_fnode_set(Efl_Text_Cursor_Handle *cur,
      Evas_Object_Textblock_Node_Format *fnode);


Eina_Bool
_evas_textblock_annotations_set(Evas_Object *eo_obj,
      Efl_Text_Attribute_Handle *an,
      Efl_Text_Cursor_Handle *start, Efl_Text_Cursor_Handle *end,
      const char *format, Eina_Bool is_item);

void
_evas_textblock_annotation_remove(Evas_Object *eo_obj, Efl_Canvas_Textblock_Data *o,
      Efl_Text_Attribute_Handle *an, Eina_Bool remove_nodes, Eina_Bool invalidate);

void
_evas_textblock_annotations_clear(const Evas_Object *eo_obj);


Efl_Text_Attribute_Handle *
_evas_textblock_annotations_insert(Eo *eo_obj,
      Efl_Text_Cursor_Handle *start, Efl_Text_Cursor_Handle *end,
      const char *format, Eina_Bool is_item);


Eina_Inlist *
_evas_textblock_annotations_get(Evas_Object *o);

void
_evas_textblock_annotations_node_format_remove(Evas_Object *o, Evas_Object_Textblock_Node_Format *n, int visual_adjustment);

void
_evas_textblock_relayout_if_needed(Evas_Object *o);

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

/**
 * Internally sets given text_object into cursor object.
 *
 * @param canvas_text_obj  the cursor object.
 * @param canvas_text_obj  the canvas text object, where cursor methods will take effect.
 * @param text_obj         the text_object that user can get using cursor text_object property.
 */
EAPI void efl_text_cursor_object_text_object_set(Eo *cursor, Eo *canvas_text_obj, Eo *text_obj);


/**
 * Internally create instance of cursor object.
 *
 * @param parent  the parent of the cursor object.
 */
EAPI Eo* efl_text_cursor_object_create(Eo *parent);


/**
 * Internally sets cursor handle(legacy textblock cursor) into cursor object.
 *
 * @param obj     the cursor object.
 * @param handle  the text cursor handle.
 */
EAPI void efl_text_cursor_object_handle_set(Eo *obj, Efl_Text_Cursor_Handle *handle);

/**
 * Internally gets cursor handle(legacy textblock cursor) from cursor object.
 *
 * @param obj     the cursor object.
 * @return        the internal text cursor handle.
 */
EAPI Efl_Text_Cursor_Handle *efl_text_cursor_object_handle_get(const Eo *obj);

#undef EAPI
#define EAPI

#endif//#ifndef _EFL_CANVAS_TEXTBLOCK_INTERNAL_H
