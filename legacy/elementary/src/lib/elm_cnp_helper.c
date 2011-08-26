#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include "elm_priv.h"

#include <sys/mman.h>

#ifdef HAVE_ELEMENTARY_X

#define ARRAYINIT(foo)  [foo] =

//#define DEBUGON 1

#ifdef DEBUGON
# define cnp_debug(x...) fprintf(stderr, __FILE__": " x)
#else
# define cnp_debug(x...)
#endif

typedef struct _Cnp_Selection Cnp_Selection;
typedef struct _Escape        Escape;
typedef struct _Tmp_Info      Tmp_Info;
typedef struct _Cnp_Atom      Cnp_Atom;
typedef struct _Saved_Type    Saved_Type;
typedef struct _Dropable      Dropable;

typedef Eina_Bool (*Converter_Fn_Cb)     (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
typedef int       (*Response_Handler_Cb) (Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *);
typedef int       (*Notify_Handler_Cb)   (Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *);

enum
{
   CNP_ATOM_TARGETS = 0,
   CNP_ATOM_text_uri,
   CNP_ATOM_text_urilist,
   CNP_ATOM_text_x_vcard,
   CNP_ATOM_image_png,
   CNP_ATOM_image_jpeg,
   CNP_ATOM_image_bmp,
   CNP_ATOM_image_gif,
   CNP_ATOM_image_tiff,
   CNP_ATOM_image_svg,
   CNP_ATOM_image_xpm,
   CNP_ATOM_image_tga,
   CNP_ATOM_image_ppm,
   CNP_ATOM_XELM,
   CNP_ATOM_text_html_utf8,
   CNP_ATOM_text_html,
   CNP_ATOM_UTF8STRING,
   CNP_ATOM_STRING,
   CNP_ATOM_TEXT,
   CNP_ATOM_text_plain_utf8,
   CNP_ATOM_text_plain,

   CNP_N_ATOMS,
};

struct _Cnp_Selection
{
   const char      *debug;
   Evas_Object     *widget;
   char            *selbuf;
   Evas_Object     *requestwidget;
   void            *udata;
   Elm_Sel_Format   requestformat;
   Elm_Drop_Cb      datacb;
   Eina_Bool      (*set)     (Ecore_X_Window, const void *data, int size);
   Eina_Bool      (*clear)   (void);
   void           (*request) (Ecore_X_Window, const char *target);

   Elm_Sel_Format    format;
   Ecore_X_Selection ecore_sel;

   Eina_Bool         active : 1;
};

struct _Escape
{
   const char *escape;
   const char  *value;
};

struct _Tmp_Info
{
   char *filename;
   void *map;
   int   fd;
   int   len;
};

struct _Cnp_Atom
{
   const char          *name;
   Elm_Sel_Format       formats;
   /* Called by ecore to do conversion */
   Converter_Fn_Cb      converter;
   Response_Handler_Cb  response;
   Notify_Handler_Cb    notify;
   /* Atom */
   Ecore_X_Atom         atom;
};

struct _Saved_Type
{
   const char  **types;
   char         *imgfile;
   int           ntypes;
   int           x, y;
   Eina_Bool     textreq: 1;
};

struct _Dropable
{
   Evas_Object     *obj;
   /* FIXME: Cache window */
   Elm_Sel_Format   types;
   Elm_Drop_Cb      dropcb;
   void            *cbdata;
};

static Tmp_Info *elm_cnp_tempfile_create(int size);
static int tmpinfo_free(Tmp_Info *tmp);

static Eina_Bool _elm_cnp_init(void);
static Eina_Bool selection_clear(void *udata __UNUSED__, int type, void *event);
static Eina_Bool selection_notify(void *udata __UNUSED__, int type, void *event);
static char *remove_tags(const char *p, int *len);
static char *mark_up(const char *start, int inlen, int *lenp);

static Eina_Bool targets_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool text_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool general_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool image_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool vcard_send(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);

static int response_handler_targets(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *);

static int notify_handler_targets(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int notify_handler_text(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int notify_handler_image(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int notify_handler_uri(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int notify_handler_html(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int vcard_receive(Cnp_Selection *sed, Ecore_X_Event_Selection_Notify *notify);

static Eina_Bool pasteimage_append(char *file, Evas_Object *entry);

#define _PARAGRAPH_SEPARATOR "\xE2\x80\xA9"

/* Optimisation: Turn this into a 256 byte table:
 *	then can lookup in one index, not N checks */
static const Escape escapes[] = {
  { "<ps>",  _PARAGRAPH_SEPARATOR },
  { "<br>",  "\n" },
  { "<\t>",  "\t" },
  { "gt;",   ">" },
  { "lt;",    "<" },
  { "amp;",   "&" },
  { "quot;",  "\'" },
  { "dquot;", "\"" }
};
#define N_ESCAPES ((int)(sizeof(escapes) / sizeof(escapes[0])))

static Cnp_Atom atoms[CNP_N_ATOMS] = {
     [CNP_ATOM_TARGETS] = {
          "TARGETS",
          (Elm_Sel_Format) -1, // everything
          targets_converter,
          response_handler_targets,
          notify_handler_targets,
          0
     },
     [CNP_ATOM_XELM] =  {
          "application/x-elementary-markup",
          ELM_SEL_FORMAT_MARKUP,
          general_converter,
          NULL,
          NULL,
          0
     },
     [CNP_ATOM_text_uri] = {
          "text/uri",
          ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE, /* Either images or entries */
          general_converter,
          NULL,
          notify_handler_uri,
          0
     },
     [CNP_ATOM_text_urilist] = {
          "text/uri-list",
          ELM_SEL_FORMAT_IMAGE,
          general_converter,
          NULL,
          notify_handler_uri,
          0
     },
     [CNP_ATOM_text_x_vcard] = {
          "text/x-vcard",
          ELM_SEL_FORMAT_VCARD,
          vcard_send, NULL,
          vcard_receive, 0
     },
     [CNP_ATOM_image_png] = {
          "image/png",
          ELM_SEL_FORMAT_IMAGE,
          image_converter,
          NULL,
          notify_handler_image,
          0
     },
     [CNP_ATOM_image_jpeg] = {
          "image/jpeg",
          ELM_SEL_FORMAT_IMAGE,
          image_converter,
          NULL,
          notify_handler_image,/* Raw image data is the same */
          0
     },
     [CNP_ATOM_image_bmp] = {
          "image/x-ms-bmp",
          ELM_SEL_FORMAT_IMAGE,
          image_converter,
          NULL,
          notify_handler_image,/* Raw image data is the same */
          0
     },
     [CNP_ATOM_image_gif] = {
          "image/gif",
          ELM_SEL_FORMAT_IMAGE,
          image_converter,
          NULL,
          notify_handler_image,/* Raw image data is the same */
          0
     },
     [CNP_ATOM_image_tiff] = {
          "image/tiff",
          ELM_SEL_FORMAT_IMAGE,
          image_converter,
          NULL,
          notify_handler_image,/* Raw image data is the same */
          0
     },
     [CNP_ATOM_image_svg] = {
          "image/svg+xml",
          ELM_SEL_FORMAT_IMAGE,
          image_converter,
          NULL,
          notify_handler_image,/* Raw image data is the same */
          0
     },
     [CNP_ATOM_image_xpm] = {
          "image/x-xpixmap",
          ELM_SEL_FORMAT_IMAGE,
          image_converter,
          NULL,
          notify_handler_image,/* Raw image data is the same */
          0
     },
     [CNP_ATOM_image_tga] = {
          "image/x-tga",
          ELM_SEL_FORMAT_IMAGE,
          image_converter,
          NULL,
          notify_handler_image,/* Raw image data is the same */
          0
     },
     [CNP_ATOM_image_ppm] = {
          "image/x-portable-pixmap",
          ELM_SEL_FORMAT_IMAGE,
          image_converter,
          NULL,
          notify_handler_image,/* Raw image data is the same */
          0
     },
     [CNP_ATOM_text_html_utf8] = {
          "text/html;charset=utf-8",
          ELM_SEL_FORMAT_HTML,
          general_converter,
          NULL,
          notify_handler_html,
          0
     },
     [CNP_ATOM_text_html] = {
          "text/html",
          ELM_SEL_FORMAT_HTML,
          general_converter,
          NULL,
          notify_handler_html, /* No encoding: Webkit only */
          0
     },
     [CNP_ATOM_UTF8STRING] = {
          "UTF8_STRING",
          ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
          text_converter,
          NULL,
          notify_handler_text,
          0
     },
     [CNP_ATOM_STRING] = {
          "STRING",
          ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
          text_converter,
          NULL,
          notify_handler_text,
          0
     },
     [CNP_ATOM_TEXT] = {
          "TEXT",
          ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
          text_converter,
          NULL,
          NULL,
          0
     },
     [CNP_ATOM_text_plain_utf8] = {
          "text/plain;charset=utf-8",
          ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
          text_converter,
          NULL,
          NULL,
          0
     },
     [CNP_ATOM_text_plain] = {
          "text/plain",
          ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
          text_converter,
          NULL,
          NULL,
          0
     },
};

static Cnp_Selection selections[ELM_SEL_MAX] = {
     ARRAYINIT(ELM_SEL_PRIMARY) {
          .debug = "Primary",
          .ecore_sel = ECORE_X_SELECTION_PRIMARY,
          .set = ecore_x_selection_primary_set,
          .clear = ecore_x_selection_primary_clear,
          .request = ecore_x_selection_primary_request,
     },
     ARRAYINIT(ELM_SEL_SECONDARY) {
          .debug = "Secondary",
          .ecore_sel = ECORE_X_SELECTION_SECONDARY,
          .set = ecore_x_selection_secondary_set,
          .clear = ecore_x_selection_secondary_clear,
          .request = ecore_x_selection_secondary_request,
     },
     ARRAYINIT(ELM_SEL_CLIPBOARD) {
          .debug = "Clipboard",
          .ecore_sel = ECORE_X_SELECTION_CLIPBOARD,
          .set = ecore_x_selection_clipboard_set,
          .clear = ecore_x_selection_clipboard_clear,
          .request = ecore_x_selection_clipboard_request,
     },
     ARRAYINIT(ELM_SEL_XDND) {
          .debug = "XDnD",
          .ecore_sel = ECORE_X_SELECTION_XDND,
          .request = ecore_x_selection_xdnd_request,
     },
};

/* Data for DND in progress */
static Saved_Type savedtypes =  { NULL, NULL, 0, 0, 0, EINA_FALSE };

static void (*dragdonecb) (void *data, Evas_Object *obj) = NULL;
static void *dragdonedata = NULL;

static int _elm_cnp_init_count = 0;

/**
 * Drag & Drop functions
 */

/* FIXME: Way too many globals */
static Eina_List *drops = NULL;
static Evas_Object *dragwin = NULL;
static int _dragx = 0, _dragy = 0;
static Ecore_Event_Handler *handler_pos = NULL;
static Ecore_Event_Handler *handler_drop = NULL;
static Ecore_Event_Handler *handler_enter = NULL;
static Ecore_Event_Handler *handler_status = NULL;

#endif

/* Stringshared, so I can just compare pointers later */
static const char *text_uri;

Eina_Bool
elm_selection_selection_has_owner(void)
{
#ifdef HAVE_ELEMENTARY_X
   return !!ecore_x_selection_owner_get(ECORE_X_ATOM_SELECTION_CLIPBOARD);
#else
   return EINA_FALSE;
#endif
}

Eina_Bool
elm_selection_set(Elm_Sel_Type selection, Evas_Object *widget, Elm_Sel_Format format, const char *selbuf)
{
#ifdef HAVE_ELEMENTARY_X
   Evas_Object *top = elm_widget_top_get(widget);
   Ecore_X_Window xwin;
   Cnp_Selection *sel;

   if (top) xwin = elm_win_xwindow_get(top);
   else xwin = elm_win_xwindow_get(widget);
   if (!xwin) return EINA_FALSE;
   if ((unsigned int)selection >= (unsigned int)ELM_SEL_MAX) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();
   if ((!selbuf) && (format != ELM_SEL_FORMAT_IMAGE))
     return elm_selection_clear(selection, widget);

   sel = selections + selection;

   sel->active = 1;
   sel->widget = widget;

   sel->set(xwin, &selection, sizeof(Elm_Sel_Type));
   sel->format = format;
   sel->selbuf = selbuf ? strdup(selbuf) : NULL;

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

Eina_Bool
elm_selection_clear(Elm_Sel_Type selection, Evas_Object *widget)
{
#ifdef HAVE_ELEMENTARY_X
   Cnp_Selection *sel;

   if ((unsigned int)selection >= (unsigned int)ELM_SEL_MAX) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();

   sel = selections + selection;

   /* No longer this selection: Consider it gone! */
   if ((!sel->active) || (sel->widget != widget)) return EINA_TRUE;

   sel->active = 0;
   sel->widget = NULL;
   sel->clear();

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

Eina_Bool
elm_selection_get(Elm_Sel_Type selection, Elm_Sel_Format format,
                  Evas_Object *widget, Elm_Drop_Cb datacb, void *udata)
{
#ifdef HAVE_ELEMENTARY_X
   Evas_Object *top;
   Cnp_Selection *sel;

   if ((unsigned int)selection >= (unsigned int)ELM_SEL_MAX) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();

   sel = selections + selection;
   top = elm_widget_top_get(widget);
   if (!top) return EINA_FALSE;

   sel->requestformat = format;
   sel->requestwidget = widget;
   sel->request(elm_win_xwindow_get(top), ECORE_X_SELECTION_TARGET_TARGETS);
   sel->datacb = datacb;
   sel->udata = udata;

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

#ifdef HAVE_ELEMENTARY_X

static Eina_Bool
_elm_cnp_init(void)
{
   int i;

   if (_elm_cnp_init_count++) return EINA_TRUE;
   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        atoms[i].atom = ecore_x_atom_get(atoms[i].name);
        ecore_x_selection_converter_atom_add(atoms[i].atom,
                                             atoms[i].converter);
     }

   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR, selection_clear, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, selection_notify, NULL);

   text_uri = eina_stringshare_add("text/uri-list");
   return EINA_TRUE;
}

static Eina_Bool
selection_clear(void *udata __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_X_Event_Selection_Clear *ev = event;
   Cnp_Selection *sel;
   int i;

   for (i = 0; i < ELM_SEL_MAX; i++)
     {
        if (selections[i].ecore_sel == ev->selection) break;
     }
   cnp_debug("selection %d clear\n", i);
   /* Not me... Don't care */
   if (i == ELM_SEL_MAX) return ECORE_CALLBACK_PASS_ON;

   sel = selections + i;
   sel->active = 0;
   sel->widget = NULL;
   sel->selbuf = NULL;

   return ECORE_CALLBACK_PASS_ON;
}


/*
 * Response to a selection notify:
 *	- So we have asked for the selection list.
 *	- If it's the targets list, parse it, and fire of what we want,
 *	else it's the data we want.
 */
static Eina_Bool
selection_notify(void *udata __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_X_Event_Selection_Notify *ev = event;
   Cnp_Selection *sel;
   int i;

   cnp_debug("selection notify callback: %d\n",ev->selection);
   switch (ev->selection)
     {
      case ECORE_X_SELECTION_CLIPBOARD:
         sel = selections + ELM_SEL_CLIPBOARD;
         break;
      case ECORE_X_SELECTION_PRIMARY:
         sel = selections + ELM_SEL_PRIMARY;
         break;
      case ECORE_X_SELECTION_SECONDARY:
         sel = selections + ELM_SEL_SECONDARY;
         break;
      case ECORE_X_SELECTION_XDND:
         sel = selections + ELM_SEL_XDND;
         break;
      default:
         return ECORE_CALLBACK_PASS_ON;
     }
   cnp_debug("Target is %s\n", ev->target);

   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        if (!strcmp(ev->target, atoms[i].name))
          {
             if (atoms[i].notify)
               {
                  cnp_debug("Found something: %s\n", atoms[i].name);
                  atoms[i].notify(sel, ev);
               }
             else
               {
                  cnp_debug("Ignored: No handler!\n");
               }
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}



static Eina_Bool
targets_converter(char *target __UNUSED__, void *data, int size __UNUSED__, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize)
{
   int i,count;
   Ecore_X_Atom *aret;
   Cnp_Selection *sel;

   if (!data_ret) return EINA_FALSE;

   sel = selections + *((int *)data);

   for (i = 0, count = 0; i < CNP_N_ATOMS ; i++)
     {
        if (sel->format & atoms[i].formats) count++;
     }

   aret = malloc(sizeof(Ecore_X_Atom) * count);
   for (i = 0, count = 0; i < CNP_N_ATOMS; i++)
     {
        if (sel->format & atoms[i].formats) aret[count ++] = atoms[i].atom;
     }

   *data_ret = aret;
   if (typesize) *typesize = 32 /* urk */;
   if (ttype) *ttype = ECORE_X_ATOM_ATOM;
   if (size_ret) *size_ret = count;

   return EINA_TRUE;
}

static Eina_Bool
image_converter(char *target __UNUSED__, void *data __UNUSED__, int size __UNUSED__, void **data_ret __UNUSED__, int *size_ret __UNUSED__, Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   cnp_debug("Image converter called\n");
   return EINA_TRUE;
}

static Eina_Bool
vcard_send(char *target __UNUSED__, void *data __UNUSED__, int size __UNUSED__, void **data_ret, int *size_ret, Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   Cnp_Selection *sel;

   cnp_debug("Vcard send called\n");

   sel = selections + *((int *)data);

   if (data_ret) *data_ret = strdup(sel->selbuf);
   if (size_ret) *size_ret = strlen(sel->selbuf);

   return EINA_TRUE;
}

static Eina_Bool
is_uri_type_data(Cnp_Selection *sel __UNUSED__, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   char *p;

   data = notify->data;
   cnp_debug("data->format is %d %p %p\n", data->format, notify, data);
   if (data->content == ECORE_X_SELECTION_CONTENT_FILES) return EINA_TRUE;
   else p = (char *)data->data;

   if (!p) return EINA_TRUE;
   cnp_debug("Got %s\n", p);
   if (strncmp(p, "file://", 7))
     {
        if (*p != '/') return EINA_FALSE;
     }

   return EINA_TRUE;
}

/*
 * Callback to handle a targets response on a selection request:
 * So pick the format we'd like; and then request it.
 */
static int
notify_handler_targets(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data_Targets *targets;
   Ecore_X_Atom *atomlist;
   Evas_Object *top;
   int i, j;

   targets = notify->data;
   atomlist = (Ecore_X_Atom *)(targets->data.data);

   for (j = 1; j < CNP_N_ATOMS; j++)
     {
        cnp_debug("\t%s %d\n", atoms[j].name, atoms[j].atom);
        if (!(atoms[j].formats & sel->requestformat)) continue;
        for (i = 0; i < targets->data.length; i++)
          {
             if ((atoms[j].atom == atomlist[i]) && (atoms[j].notify))
               {
                  if ((j == CNP_ATOM_text_uri) ||
                      (j == CNP_ATOM_text_urilist))
                    {
                      if(!is_uri_type_data(sel, notify)) continue;
                    }
                  cnp_debug("Atom %s matches\n",atoms[j].name);
                  goto done;
               }
          }
     }

   cnp_debug("Couldn't find anything that matches\n");
   return ECORE_CALLBACK_PASS_ON;

done:
   top = elm_widget_top_get(sel->requestwidget);
   if (!top) top = sel->requestwidget;
   cnp_debug("Sending request for %s\n", atoms[j].name);
   sel->request(elm_win_xwindow_get(top), atoms[j].name);

   return ECORE_CALLBACK_PASS_ON;
}

static int
response_handler_targets(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data_Targets *targets;
   Ecore_X_Atom *atomlist;
   Evas_Object *top;
   int i,j;

   targets = notify->data;
   atomlist = (Ecore_X_Atom *)(targets->data.data);

   /* Start from 1: Skip targets */
   for (j = 1 ; j < CNP_N_ATOMS ; j ++)
     {
        if (!(atoms[j].formats & sel->requestformat)) continue;
        for (i = 0 ; i < targets->data.length ; i ++)
          {
             if ((atoms[j].atom == atomlist[i]) && (atoms[j].response))
               {
                  /* Found a match: Use it */
                  goto found;
               }
          }
     }
found:
   if (j == CNP_N_ATOMS)
     {
        cnp_debug("No matching type found\n");
        return 0;
     }

   top = elm_widget_top_get(sel->requestwidget);
   if (!top) return 0;

   sel->request(elm_win_xwindow_get(top), atoms[j].name);
   return 0;
}


static int
notify_handler_text(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   char *str;

   data = notify->data;

   if (sel->datacb)
     {
        Elm_Selection_Data ddata;

        ddata.x = ddata.y = 0;
        ddata.format = ELM_SEL_FORMAT_TEXT;
        ddata.data = data->data;
        ddata.len = data->length;
        sel->datacb(sel->udata, sel->widget, &ddata);
        return 0;
     }

   cnp_debug("Notify handler text %d %d %p\n", data->format,data->length, data->data);
   str = mark_up((char *)data->data, data->length, NULL);
   cnp_debug("String is %s (from %s)\n", str, data->data);
   _elm_entry_entry_paste(sel->requestwidget, str);
   free(str);
   return 0;
}


/**
 * So someone is pasting an image into my entry or widget...
 */
static int
notify_handler_uri(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   Ecore_X_Selection_Data_Files *files;
   char *p;

   data = notify->data;
   cnp_debug("data->format is %d %p %p\n", data->format, notify, data);
   if (data->content == ECORE_X_SELECTION_CONTENT_FILES)
     {
        cnp_debug("got a files list\n");
        files = notify->data;
        if (files->num_files > 1)
          {
             /* Don't handle many items */
             cnp_debug("more then one file: Bailing\n");
             return 0;
          }
        p = files->files[0];
     }
   else
     {
        p = (char *)data->data;
     }

   if (!p)
     {
        cnp_debug("Couldn't find a file\n");
        return 0;
     }
   cnp_debug("Got %s\n",p);
   if (strncmp(p, "file://", 7))
     {
        /* Try and continue if it looks sane */
        if (*p != '/') return 0;
     }
   else
     {
        p += strlen("file://");
     }

   if (savedtypes.imgfile) free((void*)savedtypes.imgfile);
   if (savedtypes.textreq)
     {
        savedtypes.textreq = 0;
        savedtypes.imgfile = strdup(p);
     }
   else
     {
        pasteimage_append(p, sel->requestwidget);
     }
   return 0;
}

/**
 * Just receieved an vcard, either through cut and paste, or dnd.
 */
static int
vcard_receive(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Dropable *dropable;
   Eina_List *l;
   Ecore_X_Selection_Data *data;

   data = notify->data;
   cnp_debug("vcard receive\n");

   if (sel == (selections + ELM_SEL_XDND))
     {
        Elm_Selection_Data ddata;

        cnp_debug("drag & drop\n");
        /* FIXME: this needs to be generic: Used for all receives */
        EINA_LIST_FOREACH(drops, l, dropable)
          {
             if (dropable->obj == sel->requestwidget) break;
          }
        if (!dropable)
          {
             cnp_debug("Unable to find drop object");
             ecore_x_dnd_send_finished();
             return 0;
          }
        dropable = eina_list_data_get(l);
        ddata.x = savedtypes.x;
        ddata.y = savedtypes.y;
        ddata.format = ELM_SEL_FORMAT_VCARD;
        ddata.data = data->data;
        ddata.len = data->length;
        dropable->dropcb(dropable->cbdata, dropable->obj, &ddata);
        ecore_x_dnd_send_finished();
     }
   else if (sel->datacb)
     {
        Elm_Selection_Data ddata;
        ddata.x = ddata.y = 0;
        ddata.format = ELM_SEL_FORMAT_IMAGE;
        ddata.data = data->data;
        ddata.len = data->length;
        sel->datacb(sel->udata, sel->widget, &ddata);
     }
   else
     {
        cnp_debug("Paste request\n");
     }

   return 0;

}


static int
notify_handler_image(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   Tmp_Info *tmp;

   cnp_debug("got a png (or a jpeg)!\n");
   data = notify->data;

   cnp_debug("Size if %d\n", data->length);

   if (sel->datacb)
     {
        Elm_Selection_Data ddata;

        ddata.x = ddata.y = 0;
        ddata.format = ELM_SEL_FORMAT_IMAGE;
        ddata.data = data->data;
        ddata.len = data->length;
        sel->datacb(sel->udata, sel->widget, &ddata);
        return 0;
     }

   /* generate tmp name */
   tmp = elm_cnp_tempfile_create(data->length);
   memcpy(tmp->map, data->data, data->length);
   munmap(tmp->map,data->length);

   /* FIXME: Add to paste image data to clean up */
   pasteimage_append(tmp->filename, sel->requestwidget);

   tmpinfo_free(tmp);
   return 0;
}


/**
 *    Warning: Generic text/html can';t handle it sanely.
 *    Firefox sends ucs2 (i think).
 *       chrome sends utf8... blerg
 */
static int
notify_handler_html(Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;

   cnp_debug("Got some HTML: Checking encoding is useful\n");
   data = notify->data;

   if (sel->datacb)
     {
        Elm_Selection_Data ddata;
        ddata.x = ddata.y = 0;
        ddata.format = ELM_SEL_FORMAT_HTML;
        ddata.data = data->data;
        ddata.len = data->length;
        sel->datacb(sel->udata, sel->widget, &ddata);
        return 0;
     }

   char *stripstr = NULL;
   stripstr = malloc(sizeof(char) * (data->length + 1));
   strncpy(stripstr, (char *)data->data, data->length);
   stripstr[data->length] = '\0';
   cnp_debug("String is %s (%d bytes)\n", stripstr, data->length);
   _elm_entry_entry_paste(sel->requestwidget, stripstr);
   free(stripstr);
   return 0;
}


static Eina_Bool
text_converter(char *target __UNUSED__, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   Cnp_Selection *sel;

   cnp_debug("text converter\n");
   if (size != sizeof(int))
     {
        if (data_ret) *data_ret = strndup(data, size - 1);
        if (size_ret) *size_ret = size - 1;
        return EINA_TRUE;
     }
   sel = selections + *((int *)data);
   if (!sel->active) return EINA_TRUE;

   if ((sel->format & ELM_SEL_FORMAT_MARKUP) ||
       (sel->format & ELM_SEL_FORMAT_HTML))
     {
        *data_ret = remove_tags(sel->selbuf, size_ret);
     }
   else if (sel->format & ELM_SEL_FORMAT_TEXT)
     {
        *data_ret = strdup(sel->selbuf);
        *size_ret = strlen(sel->selbuf);
     }
   else if (sel->format & ELM_SEL_FORMAT_IMAGE)
     {
        cnp_debug("Image %s\n", evas_object_type_get(sel->widget));
        cnp_debug("Elm type: %s\n", elm_object_widget_type_get(sel->widget));
        evas_object_image_file_get(elm_photocam_internal_image_get(sel->widget), (const char **)data_ret, NULL);
        if (!*data_ret) *data_ret = strdup("No file");
        else *data_ret = strdup(*data_ret);
        *size_ret = strlen(*data_ret);
     }
   return EINA_TRUE;
}

static Eina_Bool
general_converter(char *target __UNUSED__, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   if (size == sizeof(int))
     {
        Cnp_Selection *sel;

        sel = selections + *((int *)data);
        if (data_ret) *data_ret = strdup(sel->selbuf);
        if (size_ret) *size_ret = strlen(sel->selbuf);
     }
   else if (size)
     {
        if (data_ret) *data_ret = strndup(data, size - 1);
        if (size_ret) *size_ret = size - 1;
     }
   return EINA_TRUE;
}

static Eina_Bool
pasteimage_append(char *file, Evas_Object *entry)
{
   char *entrytag;
   int len;
   static const char *tagstring = "<item absize=240x180 href=file://%s></item>";

   if (!file) return EINA_FALSE;
   if (!entry) return EINA_FALSE;

   len = strlen(tagstring)+strlen(file);

   entrytag = alloca(len + 1);
   snprintf(entrytag, len + 1, tagstring, file);
   _elm_entry_entry_paste(entry, entrytag);

   return EINA_TRUE;
}

static char *
remove_tags(const char *p, int *len)
{
   char *q,*ret;
   int i;
   if (!p) return NULL;

   q = malloc(strlen(p)+1);
   if (!q) return NULL;
   ret = q;

   while (*p)
     {
        if ((*p != '<') && (*p != '&')) *q++ = *p++;
        else if (*p == '<')
          {
             if ((p[1] == 'b') && (p[2] == 'r') &&
                 ((p[3] == ' ') || (p[3] == '/') || (p[3] == '>')))
               *q++ = '\n';
             else if ((p[1] == 'p') && (p[2] == 's') && (p[3] == '>'))
               {
                  strcpy(q, _PARAGRAPH_SEPARATOR);
                  q += (sizeof(_PARAGRAPH_SEPARATOR)-1);
               }
             while ((*p) && (*p != '>')) p++;
             p++;
          }
        else if (*p == '&')
          {
             p++;
             for (i = 0 ; i < N_ESCAPES ; i++)
               {
                  if (!strncmp(p,escapes[i].escape, strlen(escapes[i].escape)))
                    {
                       p += strlen(escapes[i].escape);
                       strcpy(q, escapes[i].value);
                       q += strlen(escapes[i].value);
                       break;
                    }
               }
             if (i == N_ESCAPES) *q ++= '&';
          }
     }
   *q = 0;
   if (len) *len = q - ret;
   return ret;
}

/* Mark up */
static char *
mark_up(const char *start, int inlen, int *lenp)
{
   int l, i;
   const char *p;
   char *q, *ret;
   const char *endp = NULL;

   if (!start) return NULL;
   if (inlen >= 0) endp = start + inlen;
   /* First pass: Count characters */
   for (l = 0, p = start; ((!endp) || (p < endp)) && (*p); p++)
     {
        for (i = 0 ; i < N_ESCAPES ; i ++)
          {
             if (*p == escapes[i].value[0])
               {
                  if (!strncmp(p, escapes[i].value, strlen(escapes[i].value)))
                    l += strlen(escapes[i].escape);
                  break;
               }
          }
        if (i == N_ESCAPES) l++;
     }

   q = ret = malloc(l + 1);

   /* Second pass: Change characters */
   for (p = start; ((!endp) || (p < endp)) && (*p); )
     {
        for (i = 0; i < N_ESCAPES; i++)
          {
             if (*p == escapes[i].value[0])
               {
                  if (!strncmp(p, escapes[i].value, strlen(escapes[i].value)))
                    {
                       strcpy(q, escapes[i].escape);
                       q += strlen(escapes[i].escape);
                       p += strlen(escapes[i].value);
                    }
                  break;
               }
          }
        if (i == N_ESCAPES) *q++ = *p++;
     }
   *q = 0;

   if (lenp) *lenp = l;
   return ret;
}


static Eina_Bool
_dnd_enter(void *data __UNUSED__, int etype __UNUSED__, void *ev)
{
   Ecore_X_Event_Xdnd_Enter *enter = ev;
   int i;

   /* Skip it */
   if ((!enter) || (!enter->num_types) || (!enter->types)) return EINA_TRUE;

   cnp_debug("Types\n");
   savedtypes.ntypes = enter->num_types;
   if (savedtypes.types) free(savedtypes.types);
   savedtypes.types = malloc(sizeof(char *) * enter->num_types);
   if (!savedtypes.types) return EINA_FALSE;

   for (i = 0; i < enter->num_types; i++)
     {
        savedtypes.types[i] = eina_stringshare_add(enter->types[i]);
        cnp_debug("Type is %s %p %p\n", enter->types[i],
                  savedtypes.types[i],text_uri);
        if (savedtypes.types[i] == text_uri)
          {
             /* Request it, so we know what it is */
             cnp_debug("Sending uri request\n");
             savedtypes.textreq = 1;
             if (savedtypes.imgfile) free((void*)savedtypes.imgfile);
             savedtypes.imgfile = NULL;
             ecore_x_selection_xdnd_request(enter->win, text_uri);
          }
     }

   /* FIXME: Find an object and make it current */
   return EINA_TRUE;
}

static Eina_Bool
_dnd_drop(void *data __UNUSED__, int etype __UNUSED__, void *ev)
{
   struct _Ecore_X_Event_Xdnd_Drop *drop;
   Dropable *dropable;
   Eina_List *l;
   Ecore_Evas *ee;
   Ecore_X_Window xwin;
   Elm_Selection_Data ddata;
   int x, y, w, h;
   int i, j;

   drop = ev;

   // check we still have something to drop
   if (!drops) return EINA_TRUE;

   /* Find any widget in our window; then work out geometry rel to our window */
   for (l = drops; l; l = l->next)
     {
        dropable = l->data;
        xwin = (Ecore_X_Window)ecore_evas_window_get
           (ecore_evas_ecore_evas_get(evas_object_evas_get
                                      (dropable->obj)));
        if (xwin == drop->win) break;
     }
   /* didn't find a window */
   if (!l) return EINA_TRUE;

   /* Calculate real (widget relative) position */
   // - window position
   // - widget position
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(dropable->obj));
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   savedtypes.x = drop->position.x - x;
   savedtypes.y = drop->position.y - y;

   cnp_debug("Drop position is %d,%d\n", savedtypes.x, savedtypes.y);

   for (; l; l = l->next)
     {
        dropable = l->data;
        evas_object_geometry_get(dropable->obj, &x, &y, &w, &h);
        if ((savedtypes.x >= x) && (savedtypes.y >= y) &&
            (savedtypes.x < x + w) && (savedtypes.y < y + h))
          break; /* found! */
     }

   if (!l) return EINA_TRUE; /* didn't find one */

   evas_object_geometry_get(dropable->obj, &x, &y, NULL, NULL);
   savedtypes.x -= x;
   savedtypes.y -= y;

   /* Find our type from the previous list */
   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        for (j = 0; j < savedtypes.ntypes; j++)
          {
             if (!strcmp(savedtypes.types[j], atoms[i].name)) goto found;
          }
     }

   cnp_debug("Didn't find a target\n");
   return EINA_TRUE;

found:
   cnp_debug("Found a target we'd like: %s\n", atoms[i].name);
   cnp_debug("0x%x\n",xwin);

   if (i == CNP_ATOM_text_urilist)
     {
        cnp_debug("We found a URI... (%scached) %s\n",
                  savedtypes.imgfile ? "" : "not ",
                  savedtypes.imgfile);
        if (savedtypes.imgfile)
          {
             char *entrytag;
             static const char *tagstring = "<item absize=240x180 href="
                "file://%s></item>";
             ddata.x = savedtypes.x;
             ddata.y = savedtypes.y;

             /* If it's markup that also supports images */
             if ((dropable->types & ELM_SEL_FORMAT_MARKUP) &&
                 (dropable->types & ELM_SEL_FORMAT_IMAGE))
               {
                  int len;
                  ddata.format = ELM_SEL_FORMAT_MARKUP;

                  len = strlen(tagstring) + strlen(savedtypes.imgfile);
                  entrytag = alloca(len + 1);
                  snprintf(entrytag, len + 1, tagstring, savedtypes.imgfile);
                  ddata.data = entrytag;
                  cnp_debug("Insert %s\n", (char *)ddata.data);
                  dropable->dropcb(dropable->cbdata, dropable->obj, &ddata);
                  ecore_x_dnd_send_finished();
                  if (savedtypes.imgfile) free((void*)savedtypes.imgfile);
                  savedtypes.imgfile = NULL;

                  return EINA_TRUE;
               }
             else if (dropable->types & ELM_SEL_FORMAT_IMAGE)
               {
                  cnp_debug("Doing image insert (%s)\n", savedtypes.imgfile);
                  ddata.format = ELM_SEL_FORMAT_IMAGE;
                  ddata.data = (char *)savedtypes.imgfile;
                  dropable->dropcb(dropable->cbdata, dropable->obj, &ddata);
                  ecore_x_dnd_send_finished();
                  if (savedtypes.imgfile) free((void*)savedtypes.imgfile);
                  savedtypes.imgfile = NULL;

                  return EINA_TRUE;
               }
             else
               {
                  cnp_debug("Item doesn't support images... passing\n");
                  return EINA_TRUE;
               }
          }
        else if (savedtypes.textreq)
          {
             /* Already asked: Pretend we asked now, and paste immediately when
              * it comes in */
             savedtypes.textreq = 0;
             ecore_x_dnd_send_finished();
             return EINA_TRUE;
          }
     }

   cnp_debug("doing a request then\n");
   selections[ELM_SEL_XDND].requestwidget = dropable->obj;
   selections[ELM_SEL_XDND].requestformat = ELM_SEL_FORMAT_MARKUP;
   selections[ELM_SEL_XDND].active = EINA_TRUE;

   ecore_x_selection_xdnd_request(xwin, atoms[i].name);

   return EINA_TRUE;
}
static Eina_Bool
_dnd_position(void *data __UNUSED__, int etype __UNUSED__, void *ev)
{
   struct _Ecore_X_Event_Xdnd_Position *pos;
   Ecore_X_Rectangle rect;

   pos = ev;

   /* Need to send a status back */
   /* FIXME: Should check I can drop here */
   /* FIXME: Should highlight widget */
   rect.x = pos->position.x - 5;
   rect.y = pos->position.y - 5;
   rect.width = 10;
   rect.height = 10;
   ecore_x_dnd_send_status(EINA_TRUE, EINA_FALSE, rect, pos->action);

   return EINA_TRUE;
}

/**
 * When dragging this is callback response from the destination.
 * The important thing we care about: Can we drop; thus update cursor
 * appropriately.
 */
static Eina_Bool
_dnd_status(void *data __UNUSED__, int etype __UNUSED__, void *ev)
{
   struct _Ecore_X_Event_Xdnd_Status *status = ev;

   if (!status) return EINA_TRUE;

   /* Only thing we care about: will accept */
   if (status->will_accept)
     {
        cnp_debug("Will accept\n");
     }
   else
     { /* Won't accept */
        cnp_debug("Won't accept accept\n");
     }
   return EINA_TRUE;
}

/**
 * Add a widget as drop target.
 */
Eina_Bool
elm_drop_target_add(Evas_Object *obj, Elm_Sel_Type format, Elm_Drop_Cb dropcb, void *cbdata)
{
   Dropable *drop;
   Ecore_X_Window xwin;
   Eina_List *item;
   int first;
   Evas_Object *top;

   if (!obj) return EINA_FALSE;
   top = elm_widget_top_get(obj);
   if (!top || !elm_win_xwindow_get(top)) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();

   /* Is this the first? */
   first = (!drops) ? 1 : 0;

   EINA_LIST_FOREACH(drops, item, drop)
     {
        if (drop->obj == obj)
          {
             /* Update: Not a new one */
             drop->dropcb = dropcb;
             drop->cbdata = cbdata;
             drop->types = format;
             return EINA_TRUE;
          }
     }

   /* Create new drop */
   drop = calloc(1, sizeof(Dropable));
   if (!drop) return EINA_FALSE;
   /* FIXME: Check for eina's deranged error method */
   drops = eina_list_append(drops, drop);

   if (!drops/* || or other error */)
     {
        free(drop);
        return EINA_FALSE;
     }
   drop->dropcb = dropcb;
   drop->cbdata = cbdata;
   drop->types = format;
   drop->obj = obj;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  /* I love C and varargs */
                                  (Evas_Object_Event_Cb)elm_drop_target_del,
                                  obj);
   /* FIXME: Handle resizes */

   /* If not the first: We're done */
   if (!first) return EINA_TRUE;

   xwin = (Ecore_X_Window)ecore_evas_window_get
      (ecore_evas_ecore_evas_get(evas_object_evas_get(obj)));

   ecore_x_dnd_aware_set(xwin, EINA_TRUE);

   cnp_debug("Adding drop target calls\n");
   handler_enter = ecore_event_handler_add(ECORE_X_EVENT_XDND_ENTER,
                                           _dnd_enter, NULL);
   handler_pos = ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION,
                                         _dnd_position, NULL);
   handler_drop = ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP,
                                          _dnd_drop, NULL);

   return EINA_TRUE;
}

Eina_Bool
elm_drop_target_del(Evas_Object *obj)
{
   Dropable *drop,*del;
   Eina_List *item;
   Ecore_X_Window xwin;

   del = NULL;
   EINA_LIST_FOREACH(drops, item, drop)
     {
        if (drop->obj == obj)
          {
             drops = eina_list_remove_list(drops, item);
             del = drop;
             break;
          }
     }
   if (!del) return EINA_FALSE;

   evas_object_event_callback_del(obj, EVAS_CALLBACK_FREE,
                                  (Evas_Object_Event_Cb)elm_drop_target_del);
   free(drop);
   /* If still drops there: All fine.. continue */
   if (drops) return EINA_TRUE;

   cnp_debug("Disabling DND\n");
   xwin = (Ecore_X_Window)ecore_evas_window_get
      (ecore_evas_ecore_evas_get(evas_object_evas_get(obj)));
   ecore_x_dnd_aware_set(xwin, EINA_FALSE);

   ecore_event_handler_del(handler_pos);
   ecore_event_handler_del(handler_drop);
   ecore_event_handler_del(handler_enter);

   if (savedtypes.imgfile) free((void*)savedtypes.imgfile);
   savedtypes.imgfile = NULL;

   return EINA_TRUE;
}


static void
_drag_mouse_up(void *un __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *data __UNUSED__)
{
   evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP, _drag_mouse_up);
   ecore_x_dnd_drop();
   if (dragdonecb)
     {
        dragdonecb(dragdonecb,selections[ELM_SEL_XDND].widget);
        dragdonecb = NULL;
     }
   if (dragwin)
     {
        evas_object_del(dragwin);
        dragwin = NULL;
     }
}

static void
_drag_move(void *data __UNUSED__, Ecore_X_Xdnd_Position *pos)
{
   evas_object_move(dragwin,
                    pos->position.x - _dragx,
                    pos->position.y - _dragy);
}


Eina_Bool
elm_drag_start(Evas_Object *obj, Elm_Sel_Format format, const char *data, void (*dragdone) (void *data, Evas_Object *), void *donecbdata)
{
   Ecore_X_Window xwin;
   Cnp_Selection *sel;
   Elm_Sel_Type xdnd = ELM_SEL_XDND;
   Ecore_Evas *ee;
   int x, y, x2, y2, x3, y3;
   Evas_Object *icon;
   int w, h;

   if (!_elm_cnp_init_count) _elm_cnp_init();

   xwin = elm_win_xwindow_get(obj);

   cnp_debug("starting drag...\n");

   ecore_x_dnd_type_set(xwin, "text/uri-list", 1);
   sel = selections + ELM_SEL_XDND;
   sel->active = 1;
   sel->widget = obj;
   sel->format = format;
   sel->selbuf = data ? strdup(data) : NULL;
   dragdonecb = dragdone;
   dragdonedata = donecbdata;

   ecore_x_dnd_callback_pos_update_set(_drag_move, NULL);
   ecore_x_dnd_begin(xwin, (unsigned char *)&xdnd, sizeof(Elm_Sel_Type));
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                  _drag_mouse_up, NULL);

   handler_status = ecore_event_handler_add(ECORE_X_EVENT_XDND_STATUS,
                                            _dnd_status, NULL);

   dragwin = elm_win_add(NULL, "Elm Drag Object", ELM_WIN_UTILITY);
   elm_win_override_set(dragwin, 1);

   /* FIXME: Images only */
   icon = elm_icon_add(dragwin);
   elm_icon_file_set(icon, data + 7, NULL); /* 7!? "file://" */
   elm_win_resize_object_add(dragwin,icon);
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);

   /* Position subwindow appropriately */
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   evas_object_geometry_get(obj, &x2, &y2, &w, &h);
   x += x2;
   y += y2;
   evas_object_move(dragwin, x, y);
   evas_object_resize(icon, w, h);
   evas_object_resize(dragwin, w, h);

   evas_object_show(icon);
   evas_object_show(dragwin);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x3, &y3);
   _dragx = x3 - x2;
   _dragy = y3 - y2;

   return EINA_TRUE;
}

static Tmp_Info *
elm_cnp_tempfile_create(int size)
{
   Tmp_Info *info;
   const char *tmppath;
   int len;

   info = malloc(sizeof(Tmp_Info));
   if (!info) return NULL;

   tmppath = getenv("TMP");
   if (!tmppath) tmppath = P_tmpdir;
   if (!tmppath) tmppath = "/tmp";
   len = snprintf(NULL, 0, "%s/%sXXXXXX", tmppath, "elmcnpitem-");
   if (len < 0)
     {
        free(info);
        return NULL;
     }
   len++;
   info->filename = malloc(len);
   if (!info->filename)
     {
        free(info);
        return NULL;
     }
   snprintf(info->filename,len,"%s/%sXXXXXX", tmppath, "elmcnpitem-");

   info->fd = mkstemp(info->filename);

# ifdef __linux__
     {
        char *tmp;
        /* And before someone says anything see POSIX 1003.1-2008 page 400 */
        long pid;

        pid = (long)getpid();
        /* Use pid instead of /proc/self: That way if can be passed around */
        len = snprintf(NULL,0,"/proc/%li/fd/%i", pid, info->fd);
        len++;
        tmp = malloc(len);
        if (tmp)
          {
             snprintf(tmp,len, "/proc/%li/fd/%i", pid, info->fd);
             unlink(info->filename);
             free(info->filename);
             info->filename = tmp;
          }
     }
# endif

   cnp_debug("filename is %s\n", info->filename);
   if (size < 1)
     {
        /* Set map to NULL and return */
        info->map = NULL;
        info->len = 0;
        return info;
     }

   /* Map it in */
   if (ftruncate(info->fd, size))
     {
        perror("ftruncate");
        info->map = NULL;
        info->len = 0;
        return info;
     }

   eina_mmap_safety_enabled_set(EINA_TRUE);

   info->map = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, info->fd, 0);
   if (info->map == MAP_FAILED)
     {
        perror("mmap");
        info->map = NULL;
        info->len = 0;
     }

   return info;
}


static int
tmpinfo_free(Tmp_Info *info)
{
   if (!info) return 0;
   free(info->filename);
   free(info);
   return 0;
}

#else
/* Stubs for windows */
Eina_Bool
elm_drag_start(Evas_Object *o, Elm_Sel_Format f, const char *d, void (*donecb)(void *, Evas_Object *),void *cbdata)
{
   return EINA_FALSE;
}

Eina_Bool
elm_drop_target_add(Evas_Object *obj, Elm_Sel_Type format, Elm_Drop_Cb dropcb, void *cbdata)
{
   return EINA_FALSE;
}

Eina_Bool
elm_drop_target_del(Evas_Object *o)
{
   return EINA_TRUE;
}
#endif

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0 :*/
