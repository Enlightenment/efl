/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* FIXME:
 * 
 * things to add:
 * 
 * * font path support
 * * font sources support
 * * word wrap
 * * underline support
 * * double underline support
 * * solid bg behind text
 * * if a word (or char) doesnt fit at all do something sensible
 * * styles (outline, glow, etxra glow, shadow, soft shadow, etc.)
 * * anchors (to query text extents)
 * * inline objects (queryable)
 * * tabs (indents)
 * * left and right margins
 * * api to query current extents as well as the native extents
 * * overflow objects (overflow from this textblock can go into another)
 * * obstacle objects to wrap around
 * * on change figure out what node the change is in and figure out what line (nodes) it affects and only modify those nodes on that line or maybe others until changes dont happen further down
 * * right to left text
 * 
 */

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

/* private magic number for textblock objects */
static const char o_type[] = "textblock";

/* private struct for textblock object internal data */
typedef struct _Evas_Object_Textblock      Evas_Object_Textblock;
typedef struct _Layout                     Layout;
typedef struct _Node                       Node;
typedef struct _Layout_Node                Layout_Node;

/* the current state of the formatting */
struct _Layout
{
   struct {
      char             *name;
      char             *source;
      Evas_Font_Size    size;
      void             *font;
   } font;
   struct {
      unsigned char     r, g, b, a;
   } color, underline_color, outline_color, shadow_color;
   struct {
      int               inset, x, y, ascent, descent, mascent, mdescent;
   } line;
   double               align, valign;
};

/* a node of formatting data */
struct _Node
{
   Evas_Object_List _list_data;

   char *format; /* format data */
   char *text; /* text data until the next node */
   int   text_len; /* length of the text */
};

/* a node of formatting data */
struct _Layout_Node
{
   Evas_Object_List _list_data;

   /* the current state */
   Layout layout;
   char *text; /* text data until the next node */
   int w, h;
};

struct _Evas_Object_Textblock
{
   DATA32                 magic;
   struct {
      int                 dummy;
   } cur, prev;
   char                   changed : 1;
   
   int                    pos, len;
   Evas_Format_Direction  format_dir;
   Node                  *nodes;
   Layout_Node           *layout_nodes;
   Evas_Coord             last_w, last_h;
   struct {
      unsigned char       dirty : 1;
      Evas_Coord          w, h;
   } native;
   Evas_List             *font_hold;
   void                  *engine_data;
};

static void
evas_object_textblock_layout_init(Layout *layout)
{
   layout->font.name = NULL;
   layout->font.source = NULL;
   layout->font.size = 0;
   layout->font.font = NULL;
   layout->color.r = 255;
   layout->color.g = 255;
   layout->color.b = 255;
   layout->color.a = 255;
   layout->underline_color.r = 255;
   layout->underline_color.g = 255;
   layout->underline_color.b = 255;
   layout->underline_color.a = 255;
   layout->outline_color.r = 255;
   layout->outline_color.g = 255;
   layout->outline_color.b = 255;
   layout->outline_color.a = 255;
   layout->shadow_color.r = 255;
   layout->shadow_color.g = 255;
   layout->shadow_color.b = 255;
   layout->shadow_color.a = 255;
   layout->line.inset = 0;
   layout->line.x = 0;
   layout->line.y = 0;
   layout->line.ascent = 0;
   layout->line.descent = 0;
   layout->line.mascent = 0;
   layout->line.mdescent = 0;
   layout->align = 0.0;
   layout->valign = -1.0;
}

static char *
evas_object_textblock_format_merge(char *ofmt, char *fmt)
{
   return strdup(fmt);
}

static int
evas_object_textblock_hex_string_get(char ch)
{
   if ((ch >= '0') && (ch <= '9')) return (ch - '0');
   else if ((ch >= 'A') && (ch <= 'F')) return (ch - 'A' + 10);
   else if ((ch >= 'a') && (ch <= 'f')) return (ch - 'a' + 10);
   return 0;
}

static void
evas_object_textblock_layout_format_apply(Layout *layout, char *key, char *data)
{
   if (!strcmp(key, "font"))
     {
	if (layout->font.name) free(layout->font.name);
	layout->font.name = strdup(data);
     }
   else if (!strcmp(key, "font_source"))
     {
	if (layout->font.source) free(layout->font.source);
	layout->font.source = strdup(data);
     }
   else if (!strcmp(key, "size"))
     {
	layout->font.size = atoi(data);
     }
   else if (!strcmp(key, "align"))
     {
	if (!strcmp(data, "left")) layout->align = 0.0;
	else if (!strcmp(data, "middle")) layout->align = 0.5;
	else if (!strcmp(data, "center")) layout->align = 0.5;
	else if (!strcmp(data, "right")) layout->align = 1.0;
	else
	  {
	     layout->align = atof(data);
	     if (layout->align < 0.0) layout->align = 0.0;
	     else if (layout->align > 1.0) layout->align = 1.0;
	  }
     }
   else if (!strcmp(key, "valign"))
     {
	if (!strcmp(data, "top")) layout->valign = 0.0;
	else if (!strcmp(data, "middle")) layout->valign = 0.5;
	else if (!strcmp(data, "center")) layout->valign = 0.5;
	else if (!strcmp(data, "bottom")) layout->valign = 1.0;
	else if (!strcmp(data, "baseline")) layout->valign = -1.0;
	else if (!strcmp(data, "base")) layout->valign = -1.0;
	else
	  {
	     layout->valign = atof(data);
	     if (layout->valign < 0.0) layout->valign = 0.0;
	     else if (layout->valign > 1.0) layout->valign = 1.0;
	  }
     }
   else if (!strcmp(key, "color"))
     {
	/* #RRGGBB[AA] or #RGB[A] */
	if (data[0] == '#')
	  {
	     int r, g, b, a;
	     
	     if (strlen(data) == 7) /* #RRGGBB */
	       {
		  r = (evas_object_textblock_hex_string_get(data[1]) << 4) |
		    (evas_object_textblock_hex_string_get(data[2]));
		  g = (evas_object_textblock_hex_string_get(data[3]) << 4) |
		    (evas_object_textblock_hex_string_get(data[4]));
		  b = (evas_object_textblock_hex_string_get(data[5]) << 4) |
		    (evas_object_textblock_hex_string_get(data[6]));
		  a = 0xff;
	       }
	     else if (strlen(data) == 9) /* #RRGGBBAA */
	       {
		  r = (evas_object_textblock_hex_string_get(data[1]) << 4) |
		    (evas_object_textblock_hex_string_get(data[2]));
		  g = (evas_object_textblock_hex_string_get(data[3]) << 4) |
		    (evas_object_textblock_hex_string_get(data[4]));
		  b = (evas_object_textblock_hex_string_get(data[5]) << 4) |
		    (evas_object_textblock_hex_string_get(data[6]));
		  a = (evas_object_textblock_hex_string_get(data[7]) << 4) |
		    (evas_object_textblock_hex_string_get(data[8]));
	       }
	     else if (strlen(data) == 4) /* #RGB */
	       {
		  r = evas_object_textblock_hex_string_get(data[1]);
		  r = (r << 4) | r;
		  g = evas_object_textblock_hex_string_get(data[2]);
		  g = (g << 4) | g;
		  b = evas_object_textblock_hex_string_get(data[3]);
		  b = (b << 4) | b;
		  a = 0xff;
	       }
	     else if (strlen(data) == 5) /* #RGBA */
	       {
		  r = evas_object_textblock_hex_string_get(data[1]);
		  r = (r << 4) | r;
		  g = evas_object_textblock_hex_string_get(data[2]);
		  g = (g << 4) | g;
		  b = evas_object_textblock_hex_string_get(data[3]);
		  b = (b << 4) | b;
		  a = evas_object_textblock_hex_string_get(data[4]);
		  a = (a << 4) | a;
	       }
	     layout->color.r = r;
	     layout->color.g = g;
	     layout->color.b = b;
	     layout->color.a = a;
	  }
     }
}

static void
evas_object_textblock_layout_format_modify(Layout *layout, const char *format)
{
   const char *p, *k1 = NULL, *k2 = NULL, *d1 = NULL, *d2 = NULL;
   int inquote = 0, inescape = 0;
   
   if (!format) return;
   p = format - 1;
   do
     {
	p++;
	/* we dont have the start of a key yet */
	if (!k1)
	  {
	     if (isalnum(*p)) k1 = p;
	  }
	else if (!k2)
	  {
	     if (*p == '=') k2 = p;
	  }
	else if (!d1)
	  {
	     if (*p == '\'') inquote = 1;
	     else d1 = p;
	  }
	else if (!d2)
	  {
	     if (inquote)
	       {
		  if (!inescape)
		    {
		       if (*p == '\\') inescape = 1;
		       else if (*p == '\'') d2 = p;
		    }
		  else
		    inescape = 0;
	       }
	     else
	       {
		  if ((isblank(*p)) || (*p == 0)) d2 = p;
	       }
	     if (d2)
	       {
		  char *key, *data;
		  
		  key = malloc(k2 - k1 + 1);
		  data = malloc(d2 - d1 + 1);
		  strncpy(key, k1, k2 - k1);
		  key[k2 - k1] = 0;
		  if (inquote)
		    {
		       const char *p2;
		       char *dst;
		       
		       inescape = 0;
		       p2 = d1;
		       dst = data;
		       while (p2 != d2)
			 {
			    if (!inescape)
			      {
				 if (*p == '\\') inescape = 1;
			      }
			    else
			      {
				 *dst = *p;
				 dst++;
				 inescape = 0;
			      }
			    p2++;
			 }
		       *dst = 0;
		    }
		  else
		    {
		       strncpy(data, d1, d2 - d1);
		       data[d2 - d1] = 0;
		    }
		  k1 = k2 = d1 = d2 = NULL;
		  inquote = 0;
		  inescape = 0;
		  evas_object_textblock_layout_format_apply(layout, key, data);
		  free(key);
		  free(data);
	       }
	  }
     }
   while (*p);
}

static void
evas_object_textblock_layout_copy(Layout *layout, Layout *layout_dst)
{
   *layout_dst = *layout;
   if (layout->font.name) layout_dst->font.name = strdup(layout->font.name);
   if (layout->font.source) layout_dst->font.source = strdup(layout->font.source);
}

static void
evas_object_textblock_layout_clear(Evas_Object *obj, Layout *layout)
{
   if (layout->font.name) free(layout->font.name);
   if (layout->font.source) free(layout->font.source);
   if (layout->font.font) ENFN->font_free(ENDT, layout->font.font);
   memset(layout, 0, sizeof(Layout));
}

static void
evas_object_textblock_layout_fonts_hold(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   for (l = (Evas_Object_List *)o->layout_nodes; l; l = l->next)
     {
	Layout_Node *lnode;
	
	lnode = (Layout_Node *)l;
	if (lnode->layout.font.font)
	  {
	     o->font_hold = evas_list_append(o->font_hold,
					     lnode->layout.font.font);
	     lnode->layout.font.font = NULL;
	  }
     }
}

static void
evas_object_textblock_layout_fonts_hold_clean(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   
   while (o->font_hold)
     {
	ENFN->font_free(ENDT, o->font_hold->data);
	o->font_hold = evas_list_remove_list(o->font_hold, o->font_hold);
     }
}

static void
evas_object_textblock_layout_clean(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   while (o->layout_nodes)
     {
	Layout_Node *lnode;
	
	lnode = (Layout_Node *)o->layout_nodes;
	o->layout_nodes = evas_object_list_remove(o->layout_nodes, lnode);
	evas_object_textblock_layout_clear(obj, &lnode->layout);
	if (lnode->text) free(lnode->text);
	free(lnode);
     }
}

static void
evas_object_textblock_contents_clean(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   while (o->nodes)
     {
	Node *node;
	
	node = (Node *)o->nodes;
	o->nodes = evas_object_list_remove(o->nodes, node);
	if (node->format) free(node->format);
	if (node->text) free(node->text);
	free(node);
     }
}

static void
evas_object_textblock_layout(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Layout layout;
   Evas_Object_List *l, *ll;
   Evas_Coord w, h;
   Layout_Node *line_start = NULL;

   o = (Evas_Object_Textblock *)(obj->object_data);
   evas_object_textblock_layout_init(&layout);
   w = obj->cur.geometry.w;
   h = obj->cur.geometry.h;
   o->last_w = w;
   o->last_h = h;
   /* FIXME: this is a hack - seems the lowe level font stuff is off in its */
   /* size estimates of a text line */
//   w -= 2;
//   printf("RE-LAYOUT %ix%i!\n", w, h);
   for (l = (Evas_Object_List *)o->nodes; l; l = l->next)
     {
	Layout_Node *lnode;
	Node *node;
	int newline;
	
	node = (Node *)l;
//	printf("NODE: FMT:\"%s\" TXT:\"%s\"\n", node->format, node->text);
	newline = 0;
	if (node->format)
	  {
	     /* first handle newline, tab etc. etc */
	     if (!strcmp(node->format, "\n"))
	       {
		  layout.line.x = 0;
		  layout.line.y += lnode->layout.line.mascent + lnode->layout.line.mdescent;
		  layout.line.mascent = 0;
		  layout.line.mdescent = 0;
	       }
	     else
	       evas_object_textblock_layout_format_modify(&layout, node->format);
	  }
	if (node->text)
	  {
	     int inset = 0, hadvance = 0, vadvance = 0;
	     int ascent = 0, descent = 0, tw = 0, th = 0;
	     int chrpos = -1, x, y, cx, cy, cw, ch;
	     void *font = NULL;
	     char *text;
	     int adj, lastnode;

	     text = strdup(node->text);
	     new_node:
	     lnode = calloc(1, sizeof(Layout_Node));
	     evas_object_textblock_layout_copy(&layout, &(lnode->layout));
	     if (lnode->layout.font.name)
	       font = ENFN->font_load(ENDT, lnode->layout.font.name, lnode->layout.font.size);
	     lnode->layout.font.font = font;
	     if (font) ascent = ENFN->font_max_ascent_get(ENDT, font);
	     if (font) descent = ENFN->font_max_descent_get(ENDT, font);
	     lnode->layout.line.ascent = ascent;
	     lnode->layout.line.descent = descent;
	     layout.line.ascent = ascent;
	     layout.line.descent = descent;
	     if (layout.line.mascent < ascent) layout.line.mascent = ascent;
	     if (layout.line.mdescent < descent) layout.line.mdescent = descent;
	     /* if this is at the start of the line... */
	     if (layout.line.x == 0)
	       {
		  if (font) inset = ENFN->font_inset_get(ENDT, font, text);
		  layout.line.inset = inset;
		  layout.line.x = -inset;
		  line_start = lnode;
	       }
	     if (font) chrpos = ENFN->font_char_at_coords_get(ENDT, font, text, 
							      w - layout.line.x, 0, 
							      &cx, &cy, &cw, &ch);
	     /* if the text fits... just add */
	     if (chrpos < 0)
	       {
		  if (font) ENFN->font_string_size_get(ENDT, font, text, &tw, &th);
		  lnode->w = tw;
		  lnode->h = th;
		  lnode->text = text;
		  if (font) hadvance = ENFN->font_h_advance_get(ENDT, font, text);
		  o->layout_nodes = evas_object_list_append(o->layout_nodes, lnode);
		  /* and advance */
		  layout.line.x += hadvance;
		  /* fix up max ascent/descent for the line */
		  /* FIXME: fixup align */
		  /*
		  lastnode = 1;
		  for (ll = l; ll; ll = ll->next)
		    {
		       Node *tnode;
		       
		       tnode = (Node *)ll;
		       if ((tnode->format) &&
			   (!strcmp(tnode->format, "\n")))
			 break;
		       if (tnode->text)
			 {
			    lastnode = 0;
			    break;
			 }
		    }
		  if (lastnode)
		   */
		    {
		       adj = (double)(w - (lnode->layout.line.x + tw + layout.line.inset)) * layout.align;
		       adj -= line_start->layout.line.x;
		       for (ll = (Evas_Object_List *)lnode; ll; ll = ll->prev)
			 {
			    Layout_Node *lnode2;
			    
			    lnode2 = (Layout_Node *)ll;
			    lnode2->layout.line.x += adj;
			    lnode2->layout.line.mascent = layout.line.mascent;
			    lnode2->layout.line.mdescent = layout.line.mdescent;
			    if (ll == (Evas_Object_List *)line_start) break;
			 }
		    }
	       }
	     /* text doesnt fit */
	     else
	       {
		  /* if the first char in the line can't fit!!! */
		  if ((chrpos == 0) && (lnode == line_start))
		    {
		       /* the first char can't fit. put it in there anyway */
		       /* FIXME */
		       free(text);
		       if (lnode->text) free(lnode->text);
		       evas_object_textblock_layout_clear(obj, &(lnode->layout));
		       free(lnode);
		    }
		  else
		    {
		       char *text1, *text2;
		       
		       /* byte chrpos is over... so cut there */
		       text1 = malloc(chrpos + 1);
		       strncpy(text1, text, chrpos);
		       text1[chrpos] = 0;
		       text2 = strdup(text + chrpos);
		       lnode->text = text1;
		       free(text);
		       text = text1;
		       if (font) ENFN->font_string_size_get(ENDT, font, text, &tw, &th);
		       lnode->w = tw;
		       lnode->h = th;
		       o->layout_nodes = evas_object_list_append(o->layout_nodes, lnode);
		       /* fix up max ascent/descent for the line */
		       /* FIXME: fixup align */
		       adj = (double)(w - (lnode->layout.line.x + tw + layout.line.inset)) * layout.align;
		       adj -= line_start->layout.line.x;
//		       printf("\"%s\" -> %i, %i %i ++ %i\n", 
//			      lnode->text, layout.line.inset,
//			      w, tw, adj);
		       for (ll = (Evas_Object_List *)lnode; ll; ll = ll->prev)
			 {
			    Layout_Node *lnode2;
			    
			    lnode2 = (Layout_Node *)ll;
			    lnode2->layout.line.x += adj;
			    lnode2->layout.line.mascent = layout.line.mascent;
			    lnode2->layout.line.mdescent = layout.line.mdescent;
			    if (ll == (Evas_Object_List *)line_start) break;
			 }
		       layout.line.inset = 0;
		       layout.line.x = 0;
		       layout.line.y += lnode->layout.line.mascent + lnode->layout.line.mdescent;
		       layout.line.mascent = 0;
		       layout.line.mdescent = 0;
		       text = text2;
		       /* still more text to go */
		       goto new_node;
		    }
	       }
/*	     
	     inset = ENFN->font_inset_get(ENDT, font, node->text);
	     hadvance = ENFN->font_h_advance_get(ENDT, font, node->text);
	     vadvance = ENFN->font_v_advance_get(ENDT, font, node->text);
	     ascent = ENFN->font_ascent_get(ENDT, font);
	     descent = ENFN->font_descent_get(ENDT, font);
	     ENFN->font_string_size_get(ENDT, font, text, &tw, &th);
 */
	  }
     }
   evas_object_textblock_layout_clear(obj, &layout);
}

static void
evas_object_textblock_native_calc(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   Layout layout;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   /* FIXME: takes nodes and produce layotu nodes ignoring object size */
}

static Node *
evas_object_textblock_node_pos_get(Evas_Object *obj, int pos, int *pstart)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l;
   int p, ps;
   
   o = (Evas_Object_Textblock *)(obj->object_data);
   ps = p = 0;
   for (l = (Evas_Object_List *)o->nodes; l; l = l->next)
     {
	Node *node;
	
	node = (Node *)l;
	if (node->text)
	  {
	     ps = p;
	     p += node->text_len;
	     if (p > pos)
	       {
		  *pstart = ps;
		  return node;
	       }
	  }
     }
   return NULL;
}

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

/**
 * Adds a textblock to the given evas.
 * @param   e The given evas.
 * @return  The new textblock object.
 * @todo Find a documentation group to put this under.
 */
Evas_Object *
evas_object_textblock_add(Evas *e)
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

void
evas_object_textblock_clear(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   evas_object_textblock_contents_clean(obj);
   evas_object_textblock_layout_clean(obj);
   o->len = 0;
   o->pos = 0;
   o->changed = 1;
   evas_object_change(obj);
}

void
evas_object_textblock_cursor_pos_set(Evas_Object *obj, int pos)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   if (pos < 0) pos = 0;
   else if (pos > o->len) pos = o->len;
   o->pos = pos;
}

int
evas_object_textblock_cursor_pos_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return 0;
   MAGIC_CHECK_END();
   return o->pos;
}

int
evas_object_textblock_length_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return 0;
   MAGIC_CHECK_END();
   return o->len;
}

void
evas_object_textblock_text_insert(Evas_Object *obj, const char *text)
{
   Evas_Object_Textblock *o;
   Node *node;
   int ps;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   if (!text) return;
   node = evas_object_textblock_node_pos_get(obj, o->pos, &ps);
   /* at the end - just append */
   if (!node)
     {
	if (!o->nodes)
	  {
	     node = calloc(1, sizeof(Node));
	     node->text = strdup(text);
	     node->text_len = strlen(node->text);
	     o->pos = node->text_len;
	     o->len = node->text_len;
	  }
	else
	  {
	     int len;
	     char *ntext;
	     
	     node = (Node *)(((Evas_Object_List *)(o->nodes))->last);
	     len = strlen(text);
	     ntext = malloc(node->text_len + len + 1);
	     if (node->text) strcpy(ntext, node->text);
	     strcpy(ntext + node->text_len, text);
	     if (node->text) free(node->text);
	     node->text = ntext;
	     node->text_len += len;
	     o->pos += len;
	     o->len += len;
	  }
     }
   else
     {
	int len;
	char *ntext;
	
	len = strlen(text);
	ntext = malloc(node->text_len + len + 1);
	if (node->text) strncpy(ntext, node->text, o->pos - ps);
	strcpy(ntext + o->pos - ps, text);
	if (node->text) strcpy(ntext + o->pos - ps + len, node->text + o->pos - ps);
	if (node->text) free(node->text);
	node->text = ntext;
	node->text_len += len;
	o->pos += len;
	o->len += len;
     }
   o->native.dirty = 1;
   o->changed = 1;
   evas_object_change(obj);
}

/**
 * Gets length bytes from the textblock from the given start position
 * @param   obj The given textblock.
 * @param   start The position to start getting text from.
 * @param   len The number of characters to get from the textblock.
 * @return  Returns NULL on failure, or as many of len characters as were
 * available. The returned memory must be free'd by the caller.
 */
char *
evas_object_textblock_text_get(Evas_Object *obj, int start, int len)
{
   Evas_Object_Textblock *o;
   char *ret = NULL;
   int my_len = len;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return NULL;
   MAGIC_CHECK_END();

   if (len <= 0) return NULL;
   if (start > o->len) return NULL;
   if (len > (o->len - start)) my_len = o->len - start;

   ret = malloc(sizeof(char) * (my_len + 1));
   if (ret)
     {
	Node *node;
	int ps = 0;

	node = evas_object_textblock_node_pos_get(obj, start, &ps);
	if (node)
	  {
	     if ((node->text_len - (start - ps)) >= my_len)
	       memcpy(ret, node->text + (start - ps), my_len);
	     else
	       {
		  int remaining = my_len - (node->text_len - (start - ps));
		  int count = (node->text_len - (start - ps));

		  memcpy(ret, node->text + (start - ps), node->text_len - (start - ps));

		  while(remaining > 0)
		    {
		       node = evas_object_textblock_node_pos_get(obj, start + count, &ps);
		       if (node)
			 {
			    int amt = 0;
			    if (node->text_len >= remaining)
			      amt = remaining;
			    else
			      amt = node->text_len;

			    memcpy(ret + count, node->text, amt);
			    remaining -= amt;
			    count += amt;
			 }
		       else
			 { 
			    /* we ran out of nodes ... */
			    break;
			 }
		    }
	       }
	  }
        ret[my_len] = '\0';
     }
   return ret;
}

void
evas_object_textblock_text_del(Evas_Object *obj, int len)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   o->native.dirty = 1;
   o->changed = 1;
   evas_object_change(obj);
   /* FIXME: delete len bytes of string starting at pos */
}

void
evas_object_textblock_format_insert(Evas_Object *obj, const char *format)
{
   Evas_Object_Textblock *o;
   Node *node;
   int ps;
   char *nformat;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   node = evas_object_textblock_node_pos_get(obj, o->pos, &ps);
   /* at the end - just append */
   if (!node)
     {
	nformat = evas_object_textblock_format_merge(NULL, (char *)format);
	if (nformat)
	  {
	     node = calloc(1, sizeof(Node));
	     node->format = nformat;
	     o->nodes = evas_object_list_append(o->nodes, node);
	  }
     }
   else
     {
	char *ntext1, *ntext2;
	
	ntext1 = malloc(o->pos - ps + 1);
	ntext2 = malloc(node->text_len - (o->pos - ps) + 1);
	strncpy(ntext1, node->text, o->pos - ps);
	ntext1[o->pos - ps] = 0;
	strcpy(ntext2, node->text + o->pos - ps);
	free(node->text);
	node->text = ntext1;
	node->text_len = o->pos - ps;
	nformat = evas_object_textblock_format_merge(NULL, (char *)format);
	if (nformat)
	  {
	     node = calloc(1, sizeof(Node));
	     node->format = nformat;
	     o->nodes = evas_object_list_append(o->nodes, node);
	  }
     }
   o->native.dirty = 1;
   o->changed = 1;
   evas_object_change(obj);
}

int
evas_object_textblock_format_next_pos_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return -1;
   MAGIC_CHECK_END();
   return -1;
}

int
evas_object_textblock_format_prev_pos_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return -1;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return -1;
   MAGIC_CHECK_END();
   return -1;
}

char *
evas_object_textblock_format_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return NULL;
   MAGIC_CHECK_END();
   return NULL;
}

void
evas_object_textblock_format_del(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   o->native.dirty = 1;
   o->changed = 1;
   evas_object_change(obj);
}

void
evas_object_textblock_format_direction_set(Evas_Object *obj, Evas_Format_Direction dir)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   if (o->format_dir == dir) return;
   o->native.dirty = 1;
   o->changed = 1;
   evas_object_change(obj);
}

Evas_Format_Direction
evas_object_textblock_format_direction_get(Evas_Object *obj)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_FORMAT_DIRECTION_VERTICAL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return EVAS_FORMAT_DIRECTION_VERTICAL;
   MAGIC_CHECK_END();
   return o->format_dir;
}

void
evas_object_textblock_native_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Textblock *o;
   
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   if (o->native.dirty)
     {
	evas_object_textblock_native_calc(obj);
	o->native.dirty = 0;
     }
   if (w) *w = o->native.w;
   if (h) *h = o->native.h;
}

/* all nice and private */
static void
evas_object_textblock_init(Evas_Object *obj)
{
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
}

static void *
evas_object_textblock_new(void)
{
   Evas_Object_Textblock *o;
   
   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Textblock));
   o->magic = MAGIC_OBJ_TEXTBLOCK;
   return o;
}

static void
evas_object_textblock_free(Evas_Object *obj)
{
   Evas_Object_Textblock *o;

   /* frees private object data. very simple here */
   o = (Evas_Object_Textblock *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textblock, MAGIC_OBJ_TEXTBLOCK);
   return;
   MAGIC_CHECK_END();
   /* free obj */
   evas_object_textblock_layout_clean(obj);
   evas_object_textblock_contents_clean(obj);
   o->magic = 0;
   free(o);
}

static void
evas_object_textblock_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Textblock *o;
   Evas_Object_List *l;
   
   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Textblock *)(obj->object_data);
   obj->layer->evas->engine.func->context_multiplier_unset(output,
							   context);
   if (o->changed)
     {
	evas_object_textblock_layout_fonts_hold(obj);
	evas_object_textblock_layout_clean(obj);
	evas_object_textblock_layout(obj);
	evas_object_textblock_layout_fonts_hold_clean(obj);
	o->changed = 0;
     }
/*   
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
 */
#if 1
   for (l = (Evas_Object_List *)o->layout_nodes; l; l = l->next)
     {
	Layout_Node *lnode;
	
	lnode = (Layout_Node *)l;
	ENFN->context_color_set(output,
				context,
				(obj->cur.cache.clip.r * lnode->layout.color.r) / 255,
				(obj->cur.cache.clip.g * lnode->layout.color.g) / 255,
				(obj->cur.cache.clip.b * lnode->layout.color.b) / 255,
				(obj->cur.cache.clip.a * lnode->layout.color.a) / 255);
	if ((lnode->layout.font.font) && (lnode->text))
	  {
	     if (lnode->layout.valign < 0.0)
	       ENFN->font_draw(output,
			       context,
			       surface,
			       lnode->layout.font.font,
			       obj->cur.cache.geometry.x + 
			       lnode->layout.line.x + x,
			       obj->cur.cache.geometry.y + 
			       lnode->layout.line.y + y + lnode->layout.line.mascent,
			       lnode->w,
			       lnode->h,
			       lnode->w,
			       lnode->h,
			       lnode->text);
	     else
	       ENFN->font_draw(output,
			       context,
			       surface,
			       lnode->layout.font.font,
			       obj->cur.cache.geometry.x +
			       lnode->layout.line.x + x,
			       obj->cur.cache.geometry.y + 
			       lnode->layout.line.y + y + 
			       ((double)(((lnode->layout.line.mascent + lnode->layout.line.mdescent) -
					 (lnode->layout.line.ascent + lnode->layout.line.descent)) * lnode->layout.valign)) +
			       lnode->layout.line.ascent,
			       lnode->w,
			       lnode->h,
			       lnode->w,
			       lnode->h,
			       lnode->text);
	  }
     }
#endif   
/*   
   if (o->engine_data)
     {
	
     }
 */
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
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
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
	evas_object_textblock_layout_fonts_hold(obj);
	evas_object_textblock_layout_clean(obj);
	evas_object_textblock_layout(obj);
	evas_object_textblock_layout_fonts_hold_clean(obj);
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
   o->prev = o->cur;
   o->changed = 0;
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
   if ((obj->cur.geometry.w != o->last_w) ||
       (obj->cur.geometry.h != o->last_h))
     {
	o->changed = 1;
     }
}
