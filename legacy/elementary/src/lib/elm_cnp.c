#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include "elm_priv.h"
#ifdef HAVE_MMAN_H
# include <sys/mman.h>
#endif

// common stuff
enum
{
   CNP_ATOM_TARGETS = 0,
   CNP_ATOM_ATOM,
   CNP_ATOM_LISTING_ATOMS = CNP_ATOM_ATOM,
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
//   CNP_ATOM_text_html_utf8,
//   CNP_ATOM_text_html,
   CNP_ATOM_UTF8STRING,
   CNP_ATOM_STRING,
   CNP_ATOM_TEXT,
   CNP_ATOM_text_plain_utf8,
   CNP_ATOM_text_plain,

   CNP_N_ATOMS,
};

typedef struct _Tmp_Info      Tmp_Info;
typedef struct _Saved_Type    Saved_Type;
typedef struct _Escape        Escape;
typedef struct _Dropable      Dropable;

struct _Tmp_Info
{
   char *filename;
   void *map;
   int   fd;
   int   len;
};

struct _Saved_Type
{
   const char  **types;
   char         *imgfile;
   int           ntypes;
   int           x, y;
   Eina_Bool     textreq: 1;
};

struct _Escape
{
   const char *escape;
   const char *value;
};

struct _Dropable
{
   Evas_Object    *obj;
   /* FIXME: Cache window */
   Elm_Sel_Format  types;
   Elm_Drop_Cb     dropcb;
   void           *cbdata;
};

static int _elm_cnp_init_count = 0;
/* Stringshared, so I can just compare pointers later */
static const char *text_uri;
/* Data for DND in progress */
static Saved_Type savedtypes =  { NULL, NULL, 0, 0, 0, EINA_FALSE };

/* TODO BUG: should NEVER have these as globals! They should be per context (window). */
static void (*dragdonecb) (void *data, Evas_Object *obj) = NULL;
static void *dragdonedata = NULL;

/* Drag & Drop functions */
/* FIXME: Way too many globals */
static Eina_List *drops = NULL;
static Evas_Object *dragwin = NULL;
static int _dragx = 0, _dragy = 0;
static Ecore_Event_Handler *handler_pos = NULL;
static Ecore_Event_Handler *handler_drop = NULL;
static Ecore_Event_Handler *handler_enter = NULL;
static Ecore_Event_Handler *handler_status = NULL;

static Tmp_Info  *_tempfile_new      (int size);
static int        _tmpinfo_free      (Tmp_Info *tmp);
static Eina_Bool  _pasteimage_append (char *file, Evas_Object *entry);

//#define DEBUGON 1
#ifdef DEBUGON
# define cnp_debug(x...) fprintf(stderr, __FILE__": " x)
#else
# define cnp_debug(x...) do { } while (0)
#endif

// x11 specific stuff
////////////////////////////////////////////////////////////////////////////
#ifdef HAVE_ELEMENTARY_X
#define ARRAYINIT(foo)  [foo] =

typedef struct _X11_Cnp_Selection X11_Cnp_Selection;
typedef struct _X11_Cnp_Atom      X11_Cnp_Atom;

typedef Eina_Bool (*X11_Converter_Fn_Cb)     (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
typedef int       (*X11_Response_Handler_Cb) (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *);
typedef int       (*X11_Notify_Handler_Cb)   (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *);

struct _X11_Cnp_Selection
{
   const char        *debug;
   Evas_Object       *widget;
   char              *selbuf;
   Evas_Object       *requestwidget;
   void              *udata;
   Elm_Sel_Format     requestformat;
   Elm_Drop_Cb        datacb;
   Eina_Bool        (*set)     (Ecore_X_Window, const void *data, int size);
   Eina_Bool        (*clear)   (void);
   void             (*request) (Ecore_X_Window, const char *target);
   Elm_Selection_Loss_Cb  loss_cb;
   void                  *loss_data;

   Elm_Sel_Format     format;
   Ecore_X_Selection  ecore_sel;
   Ecore_X_Window     xwin;

   Eina_Bool          active : 1;
};

struct _X11_Cnp_Atom
{
   const char              *name;
   Elm_Sel_Format           formats;
   /* Called by ecore to do conversion */
   X11_Converter_Fn_Cb      converter;
   X11_Response_Handler_Cb  response;
   X11_Notify_Handler_Cb    notify;
   /* Atom */
   Ecore_X_Atom             atom;
};

static void           _x11_sel_obj_del              (void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__);
static void           _x11_sel_obj_del2             (void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__);
static Eina_Bool      _x11_selection_clear          (void *udata __UNUSED__, int type, void *event);
static Eina_Bool      _x11_selection_notify         (void *udata __UNUSED__, int type, void *event);
static Eina_Bool      _x11_targets_converter        (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool      _x11_text_converter           (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool      _x11_general_converter        (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool      _x11_image_converter          (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool      _x11_vcard_send               (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool      _x11_is_uri_type_data         (X11_Cnp_Selection *sel __UNUSED__, Ecore_X_Event_Selection_Notify *notify);
static int            _x11_response_handler_targets (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *);
static int            _x11_notify_handler_targets   (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int            _x11_notify_handler_text      (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int            _x11_notify_handler_image     (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int            _x11_notify_handler_uri       (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
//static int            _x11_notify_handler_html      (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int            _x11_vcard_receive            (X11_Cnp_Selection *sed, Ecore_X_Event_Selection_Notify *notify);
static Eina_Bool      _x11_dnd_enter                (void *data __UNUSED__, int etype __UNUSED__, void *ev);
static Eina_Bool      _x11_dnd_drop                 (void *data __UNUSED__, int etype __UNUSED__, void *ev);
static Eina_Bool      _x11_dnd_position             (void *data __UNUSED__, int etype __UNUSED__, void *ev);
static Eina_Bool      _x11_dnd_status               (void *data __UNUSED__, int etype __UNUSED__, void *ev);
static void           _x11_drag_mouse_up            (void *un __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *data);
static void           _x11_drag_move                (void *data __UNUSED__, Ecore_X_Xdnd_Position *pos);

static Ecore_X_Window _x11_elm_widget_xwin_get           (const Evas_Object *obj);

static Eina_Bool _x11_elm_cnp_init                       (void);
static Eina_Bool _x11_elm_cnp_selection_set              (Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, const void *selbuf, size_t buflen);
static void      _x11_elm_cnp_selection_loss_callback_set(Evas_Object *obj __UNUSED__, Elm_Sel_Type selection, Elm_Selection_Loss_Cb func, const void *data);
static Eina_Bool _x11_elm_object_cnp_selection_clear     (Evas_Object *obj, Elm_Sel_Type selection);
static Eina_Bool _x11_elm_cnp_selection_get              (Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, Elm_Drop_Cb datacb, void *udata);
static Eina_Bool _x11_elm_drop_target_add                (Evas_Object *obj, Elm_Sel_Type format, Elm_Drop_Cb dropcb, void *cbdata);
static Eina_Bool _x11_elm_drop_target_del                (Evas_Object *obj);
static Eina_Bool _x11_elm_selection_selection_has_owner  (Evas_Object *obj __UNUSED__);

static X11_Cnp_Atom _x11_atoms[CNP_N_ATOMS] = {
   [CNP_ATOM_TARGETS] = {
      "TARGETS",
      ELM_SEL_FORMAT_TARGETS,
      _x11_targets_converter,
      _x11_response_handler_targets,
      _x11_notify_handler_targets,
      0
   },
   [CNP_ATOM_ATOM] = {
      "ATOM", // for opera browser
      ELM_SEL_FORMAT_TARGETS,
      _x11_targets_converter,
      _x11_response_handler_targets,
      _x11_notify_handler_targets,
      0
   },
   [CNP_ATOM_XELM] =  {
      "application/x-elementary-markup",
      ELM_SEL_FORMAT_MARKUP,
      _x11_general_converter,
      NULL,
      NULL,
      0
   },
   [CNP_ATOM_text_uri] = {
      "text/uri",
      ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_IMAGE, /* Either images or entries */
      _x11_general_converter,
      NULL,
      _x11_notify_handler_uri,
      0
   },
   [CNP_ATOM_text_urilist] = {
      "text/uri-list",
      ELM_SEL_FORMAT_IMAGE,
      _x11_general_converter,
      NULL,
      _x11_notify_handler_uri,
      0
   },
   [CNP_ATOM_text_x_vcard] = {
      "text/x-vcard",
      ELM_SEL_FORMAT_VCARD,
      _x11_vcard_send, NULL,
      _x11_vcard_receive, 0
   },
   [CNP_ATOM_image_png] = {
      "image/png",
      ELM_SEL_FORMAT_IMAGE,
      _x11_image_converter,
      NULL,
      _x11_notify_handler_image,
      0
   },
   [CNP_ATOM_image_jpeg] = {
      "image/jpeg",
      ELM_SEL_FORMAT_IMAGE,
      _x11_image_converter,
      NULL,
      _x11_notify_handler_image,/* Raw image data is the same */
      0
   },
   [CNP_ATOM_image_bmp] = {
      "image/x-ms-bmp",
      ELM_SEL_FORMAT_IMAGE,
      _x11_image_converter,
      NULL,
      _x11_notify_handler_image,/* Raw image data is the same */
      0
   },
   [CNP_ATOM_image_gif] = {
      "image/gif",
      ELM_SEL_FORMAT_IMAGE,
      _x11_image_converter,
      NULL,
      _x11_notify_handler_image,/* Raw image data is the same */
      0
   },
   [CNP_ATOM_image_tiff] = {
      "image/tiff",
      ELM_SEL_FORMAT_IMAGE,
      _x11_image_converter,
      NULL,
      _x11_notify_handler_image,/* Raw image data is the same */
      0
   },
   [CNP_ATOM_image_svg] = {
      "image/svg+xml",
      ELM_SEL_FORMAT_IMAGE,
      _x11_image_converter,
      NULL,
      _x11_notify_handler_image,/* Raw image data is the same */
      0
   },
   [CNP_ATOM_image_xpm] = {
      "image/x-xpixmap",
      ELM_SEL_FORMAT_IMAGE,
      _x11_image_converter,
      NULL,
      _x11_notify_handler_image,/* Raw image data is the same */
      0
   },
   [CNP_ATOM_image_tga] = {
      "image/x-tga",
      ELM_SEL_FORMAT_IMAGE,
      _x11_image_converter,
      NULL,
      _x11_notify_handler_image,/* Raw image data is the same */
      0
   },
   [CNP_ATOM_image_ppm] = {
      "image/x-portable-pixmap",
      ELM_SEL_FORMAT_IMAGE,
      _x11_image_converter,
      NULL,
      _x11_notify_handler_image,/* Raw image data is the same */
      0
   },
/*   
   [CNP_ATOM_text_html_utf8] = {
      "text/html;charset=utf-8",
      ELM_SEL_FORMAT_HTML,
      _x11_general_converter,
      NULL,
      _x11_notify_handler_html,
      0
   },
   [CNP_ATOM_text_html] = {
      "text/html",
      ELM_SEL_FORMAT_HTML,
      _x11_general_converter,
      NULL,
      _x11_notify_handler_html, // No encoding: Webkit only
      0
   },
 */
   [CNP_ATOM_UTF8STRING] = {
      "UTF8_STRING",
      ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
      _x11_text_converter,
      NULL,
      _x11_notify_handler_text,
      0
   },
   [CNP_ATOM_STRING] = {
      "STRING",
      ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
      _x11_text_converter,
      NULL,
      _x11_notify_handler_text,
      0
   },
   [CNP_ATOM_TEXT] = {
      "TEXT",
      ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
      _x11_text_converter,
      NULL,
      NULL,
      0
   },
   [CNP_ATOM_text_plain_utf8] = {
      "text/plain;charset=utf-8",
      ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
      _x11_text_converter,
      NULL,
      NULL,
      0
   },
   [CNP_ATOM_text_plain] = {
      "text/plain",
      ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
      _x11_text_converter,
      NULL,
      NULL,
      0
   },
};

static X11_Cnp_Selection _x11_selections[ELM_SEL_TYPE_CLIPBOARD + 1] = {
   ARRAYINIT(ELM_SEL_TYPE_PRIMARY) {
      .debug = "Primary",
        .ecore_sel = ECORE_X_SELECTION_PRIMARY,
        .set = ecore_x_selection_primary_set,
        .clear = ecore_x_selection_primary_clear,
        .request = ecore_x_selection_primary_request,
   },
   ARRAYINIT(ELM_SEL_TYPE_SECONDARY) {
      .debug = "Secondary",
        .ecore_sel = ECORE_X_SELECTION_SECONDARY,
        .set = ecore_x_selection_secondary_set,
        .clear = ecore_x_selection_secondary_clear,
        .request = ecore_x_selection_secondary_request,
   },
   ARRAYINIT(ELM_SEL_TYPE_XDND) {
      .debug = "XDnD",
        .ecore_sel = ECORE_X_SELECTION_XDND,
        .request = ecore_x_selection_xdnd_request,
   },
   ARRAYINIT(ELM_SEL_TYPE_CLIPBOARD) {
      .debug = "Clipboard",
        .ecore_sel = ECORE_X_SELECTION_CLIPBOARD,
        .set = ecore_x_selection_clipboard_set,
        .clear = ecore_x_selection_clipboard_clear,
        .request = ecore_x_selection_clipboard_request,
   },
};

static void
_x11_sel_obj_del(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   X11_Cnp_Selection *sel = data;
   if (sel->widget == obj) sel->widget = NULL;
}

static void
_x11_sel_obj_del2(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   X11_Cnp_Selection *sel = data;
   if (sel->requestwidget == obj) sel->requestwidget = NULL;
}

static Eina_Bool
_x11_selection_clear(void *udata __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_X_Event_Selection_Clear *ev = event;
   X11_Cnp_Selection *sel;
   unsigned int i;

   _x11_elm_cnp_init();
   for (i = ELM_SEL_TYPE_PRIMARY; i <= ELM_SEL_TYPE_CLIPBOARD; i++)
     {
        if (_x11_selections[i].ecore_sel == ev->selection) break;
     }
   cnp_debug("selection %d clear\n", i);
   /* Not me... Don't care */
   if (i > ELM_SEL_TYPE_CLIPBOARD) return ECORE_CALLBACK_PASS_ON;

   sel = _x11_selections + i;
   if (sel->loss_cb) sel->loss_cb(sel->loss_data, i);
   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget, EVAS_CALLBACK_DEL,
                                         _x11_sel_obj_del, sel);
   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget, EVAS_CALLBACK_DEL,
                                         _x11_sel_obj_del2, sel);
   sel->widget = NULL;
   sel->requestwidget = NULL;
   
   sel->active = EINA_FALSE;
   sel->widget = NULL;
   if (sel->selbuf)
     {
        free(sel->selbuf);
        sel->selbuf = NULL;
     }
   return ECORE_CALLBACK_PASS_ON;
}

/*
 * Response to a selection notify:
 *  - So we have asked for the selection list.
 *  - If it's the targets list, parse it, and fire of what we want,
 *    else it's the data we want.
 */
static Eina_Bool
_x11_selection_notify(void *udata __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_X_Event_Selection_Notify *ev = event;
   X11_Cnp_Selection *sel;
   int i;

   cnp_debug("selection notify callback: %d\n",ev->selection);
   switch (ev->selection)
     {
      case ECORE_X_SELECTION_PRIMARY:
        sel = _x11_selections + ELM_SEL_TYPE_PRIMARY;
        break;
      case ECORE_X_SELECTION_SECONDARY:
        sel = _x11_selections + ELM_SEL_TYPE_SECONDARY;
        break;
      case ECORE_X_SELECTION_XDND:
        sel = _x11_selections + ELM_SEL_TYPE_XDND;
        break;
      case ECORE_X_SELECTION_CLIPBOARD:
        sel = _x11_selections + ELM_SEL_TYPE_CLIPBOARD;
        break;
      default:
        return ECORE_CALLBACK_PASS_ON;
     }
   cnp_debug("Target is %s\n", ev->target);
   
   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        if (!strcmp(ev->target, _x11_atoms[i].name))
          {
             if (_x11_atoms[i].notify)
               {
                  cnp_debug("Found something: %s\n", _x11_atoms[i].name);
                  _x11_atoms[i].notify(sel, ev);
               }
             else cnp_debug("Ignored: No handler!\n");
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Elm_Sel_Format
_get_selection_type(void *data, int size)
{
   if (size == sizeof(Elm_Sel_Type))
     {
        unsigned int seltype = *((unsigned int *)data);
        if (seltype > ELM_SEL_TYPE_CLIPBOARD)
          return ELM_SEL_FORMAT_NONE;
        X11_Cnp_Selection *sel = _x11_selections + seltype;
        if (sel->active &&
            (sel->format >= ELM_SEL_FORMAT_TARGETS) &&
            (sel->format <= ELM_SEL_FORMAT_HTML))
          return sel->format;
     }
   return ELM_SEL_FORMAT_NONE;
}

static Eina_Bool
_x11_targets_converter(char *target __UNUSED__, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize)
{
   int i, count;
   Ecore_X_Atom *aret;
   X11_Cnp_Selection *sel;
   Elm_Sel_Format seltype;

   if (!data_ret) return EINA_FALSE;
   if (_get_selection_type(data, size) == ELM_SEL_FORMAT_NONE)
     {
        /* TODO : fallback into precise type */
        seltype = ELM_SEL_FORMAT_TEXT;
     }
   else
     {
        sel = _x11_selections + *((int *)data);
        seltype = sel->format;
     }

   for (i = 0, count = 0; i < CNP_N_ATOMS ; i++)
     {
        if (seltype & _x11_atoms[i].formats) count++;
     }
   aret = malloc(sizeof(Ecore_X_Atom) * count);
   if (!aret) return EINA_FALSE;
   for (i = 0, count = 0; i < CNP_N_ATOMS; i++)
     {
        if (seltype & _x11_atoms[i].formats)
          aret[count ++] = _x11_atoms[i].atom;
     }

   *data_ret = aret;
   if (typesize) *typesize = 32 /* urk */;
   if (ttype) *ttype = ECORE_X_ATOM_ATOM;
   if (size_ret) *size_ret = count;
   return EINA_TRUE;
}

static Eina_Bool
_x11_image_converter(char *target __UNUSED__, void *data __UNUSED__, int size __UNUSED__, void **data_ret __UNUSED__, int *size_ret __UNUSED__, Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   cnp_debug("Image converter called\n");
   return EINA_TRUE;
}

static Eina_Bool
_x11_vcard_send(char *target __UNUSED__, void *data __UNUSED__, int size __UNUSED__, void **data_ret, int *size_ret, Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   X11_Cnp_Selection *sel;

   cnp_debug("Vcard send called\n");
   sel = _x11_selections + *((int *)data);
   if (data_ret) *data_ret = strdup(sel->selbuf);
   if (size_ret) *size_ret = strlen(sel->selbuf);
   return EINA_TRUE;
}

static Eina_Bool
_x11_is_uri_type_data(X11_Cnp_Selection *sel __UNUSED__, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   char *p;

   data = notify->data;
   cnp_debug("data->format is %d %p %p\n", data->format, notify, data);
   if (data->content == ECORE_X_SELECTION_CONTENT_FILES) return EINA_TRUE;
   p = (char *)data->data;
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
_x11_notify_handler_targets(X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data_Targets *targets;
   Ecore_X_Atom *atomlist;
   int i, j;

   targets = notify->data;
   atomlist = (Ecore_X_Atom *)(targets->data.data);
   for (j = (CNP_ATOM_LISTING_ATOMS + 1); j < CNP_N_ATOMS; j++)
     {
        cnp_debug("\t%s %d\n", _x11_atoms[j].name, _x11_atoms[j].atom);
        if (!(_x11_atoms[j].formats & sel->requestformat)) continue;
        for (i = 0; i < targets->data.length; i++)
          {
             if ((_x11_atoms[j].atom == atomlist[i]) && (_x11_atoms[j].notify))
               {
                  if ((j == CNP_ATOM_text_uri) ||
                      (j == CNP_ATOM_text_urilist))
                    {
                       if (!_x11_is_uri_type_data(sel, notify)) continue;
                    }
                  cnp_debug("Atom %s matches\n", _x11_atoms[j].name);
                  goto done;
               }
          }
     }
   cnp_debug("Couldn't find anything that matches\n");
   return ECORE_CALLBACK_PASS_ON;
done:
   cnp_debug("Sending request for %s, xwin=%#llx\n",
             _x11_atoms[j].name, (unsigned long long)sel->xwin);
   sel->request(sel->xwin, _x11_atoms[j].name);
   return ECORE_CALLBACK_PASS_ON;
}

static int
_x11_response_handler_targets(X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data_Targets *targets;
   Ecore_X_Atom *atomlist;
   int i, j;

   targets = notify->data;
   atomlist = (Ecore_X_Atom *)(targets->data.data);

   for (j = (CNP_ATOM_LISTING_ATOMS + 1); j < CNP_N_ATOMS; j++)
     {
        if (!(_x11_atoms[j].formats & sel->requestformat)) continue;
        for (i = 0; i < targets->data.length; i++)
          {
             if ((_x11_atoms[j].atom == atomlist[i]) &&
                 (_x11_atoms[j].response))
               goto found;
          }
     }
   cnp_debug("No matching type found\n");
   return 0;
found:
   sel->request(sel->xwin, _x11_atoms[j].name);
   return 0;
}

static int
_x11_notify_handler_text(X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   char *stripstr, *mkupstr;

   data = notify->data;
   stripstr = malloc(data->length + 1);
   if (!stripstr) return 0;
   strncpy(stripstr, (char *)data->data, data->length);
   stripstr[data->length] = '\0';
   if (sel->datacb)
     {
        Elm_Selection_Data ddata;

        ddata.x = ddata.y = 0;
        ddata.format = ELM_SEL_FORMAT_TEXT;
        ddata.data = data->data;
        ddata.len = data->length;
        sel->datacb(sel->udata, sel->widget, &ddata);
        free(stripstr);
        return 0;
     }
   cnp_debug("Notify handler text %d %d %p\n", data->format,
             data->length, data->data);
   mkupstr = _elm_util_text_to_mkup((const char *)stripstr);
   cnp_debug("String is %s (from %s)\n", stripstr, data->data);
   /* TODO BUG: should never NEVER assume it's an elm_entry! */
   _elm_entry_entry_paste(sel->requestwidget, mkupstr);
   free(stripstr);
   free(mkupstr);
   return 0;
}

/**
 * So someone is pasting an image into my entry or widget...
 */
static int
_x11_notify_handler_uri(X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   Ecore_X_Selection_Data_Files *files;
   char *p, *stripstr;

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
        stripstr = p = strdup(files->files[0]);
     }
   else
     {
        stripstr = p = malloc(data->length + 1);
        if (!stripstr) return 0;
        memcpy(stripstr, data->data, data->length);
        stripstr[data->length] = 0;
     }
   if (!p)
     {
        cnp_debug("Couldn't find a file\n");
        return 0;
     }
   cnp_debug("Got %s\n", p);
   if (strncmp(p, "file://", 7))
     {
        /* Try and continue if it looks sane */
        if (*p != '/')
          {
             free(p);
             return 0;
          }
     }
   else p += strlen("file://");
   if (savedtypes.imgfile) free(savedtypes.imgfile);
   if (savedtypes.textreq)
     {
        savedtypes.textreq = 0;
        savedtypes.imgfile = strdup(p);
     }
   else _pasteimage_append(p, sel->requestwidget);
   free(stripstr);
   return 0;
}

/**
 * Just receieved an vcard, either through cut and paste, or dnd.
 */
static int
_x11_vcard_receive(X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Dropable *dropable;
   Eina_List *l;
   Ecore_X_Selection_Data *data;

   data = notify->data;
   cnp_debug("vcard receive\n");
   if (sel == (_x11_selections + ELM_SEL_TYPE_XDND))
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
        ddata.format = ELM_SEL_FORMAT_VCARD;
        ddata.data = data->data;
        ddata.len = data->length;
        sel->datacb(sel->udata, sel->widget, &ddata);
     }
   else cnp_debug("Paste request\n");
   return 0;
}

static int
_x11_notify_handler_image(X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   Tmp_Info *tmp;

   cnp_debug("got a image file!\n");
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
   tmp = _tempfile_new(data->length);
   memcpy(tmp->map, data->data, data->length);
   munmap(tmp->map, data->length);
   /* FIXME: Add to paste image data to clean up */
   _pasteimage_append(tmp->filename, sel->requestwidget);
   _tmpinfo_free(tmp);
   return 0;
}

/**
 *    Warning: Generic text/html can';t handle it sanely.
 *    Firefox sends ucs2 (i think).
 *       chrome sends utf8... blerg
 */
/*
static int
_x11_notify_handler_html(X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;

   cnp_debug("Got some HTML: Checking encoding is useful\n");
   data = notify->data;
   char *stripstr = malloc(data->length + 1);
   if (!stripstr) return 0;
   strncpy(stripstr, (char *)data->data, data->length);
   stripstr[data->length] = '\0';

   if (sel->datacb)
     {
        Elm_Selection_Data ddata;
        ddata.x = ddata.y = 0;
        ddata.format = ELM_SEL_FORMAT_HTML;
        ddata.data = stripstr;
        ddata.len = data->length;
        sel->datacb(sel->udata, sel->widget, &ddata);
        free(stripstr);
        return 0;
     }

   cnp_debug("String is %s (%d bytes)\n", stripstr, data->length);
   // TODO BUG: should never NEVER assume it's an elm_entry!
   _elm_entry_entry_paste(sel->requestwidget, stripstr);
   free(stripstr);
   return 0;
}
*/

static Eina_Bool
_x11_text_converter(char *target __UNUSED__, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   X11_Cnp_Selection *sel;

   cnp_debug("text converter\n");
   if (_get_selection_type(data, size) == ELM_SEL_FORMAT_NONE)
     {
        if (data_ret)
          {
             *data_ret = malloc(size * sizeof(char) + 1);
             if (!*data_ret) return EINA_FALSE;
             memcpy(*data_ret, data, size);
             ((char**)(data_ret))[0][size] = 0;
          }
        if (size_ret) *size_ret = size;
        return EINA_TRUE;
     }
   sel = _x11_selections + *((int *)data);
   if (!sel->active) return EINA_TRUE;

   if ((sel->format & ELM_SEL_FORMAT_MARKUP) ||
       (sel->format & ELM_SEL_FORMAT_HTML))
     {
        *data_ret = _elm_util_mkup_to_text(sel->selbuf);
        if (size_ret) *size_ret = strlen(*data_ret);
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
        evas_object_image_file_get(elm_photocam_internal_image_get(sel->widget),
                                   (const char **)data_ret, NULL);
        if (!*data_ret) *data_ret = strdup("No file");
        else *data_ret = strdup(*data_ret);
        *size_ret = strlen(*data_ret);
     }
   return EINA_TRUE;
}

static Eina_Bool
_x11_general_converter(char *target __UNUSED__, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype __UNUSED__, int *typesize __UNUSED__)
{
   if (_get_selection_type(data, size) == ELM_SEL_FORMAT_NONE)
     {
        if (data_ret)
          {
             *data_ret = malloc(size * sizeof(char) + 1);
             if (!*data_ret) return EINA_FALSE;
             memcpy(*data_ret, data, size);
             ((char**)(data_ret))[0][size] = 0;
          }
        if (size_ret) *size_ret = size;
     }
   else
     {
        X11_Cnp_Selection *sel = _x11_selections + *((int *)data);
        if (data_ret) *data_ret = strdup(sel->selbuf);
        if (size_ret) *size_ret = strlen(sel->selbuf);
     }
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_enter(void *data __UNUSED__, int etype __UNUSED__, void *ev)
{
   Ecore_X_Event_Xdnd_Enter *enter = ev;
   int i;

   /* Skip it */
   cnp_debug("enter %p\n", enter);
   cnp_debug("enter types=%p (%d)\n", enter->types, enter->num_types);
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
                  savedtypes.types[i], text_uri);
        if (savedtypes.types[i] == text_uri)
          {
             /* Request it, so we know what it is */
             cnp_debug("Sending uri request\n");
             savedtypes.textreq = 1;
             if (savedtypes.imgfile) free(savedtypes.imgfile);
             savedtypes.imgfile = NULL;
             ecore_x_selection_xdnd_request(enter->win, text_uri);
          }
     }

   /* FIXME: Find an object and make it current */
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_drop(void *data __UNUSED__, int etype __UNUSED__, void *ev)
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

   cnp_debug("drops %p (%d)\n", drops, eina_list_count(drops));

   // check we still have something to drop
   if (!drops) return EINA_TRUE;

   /* Find any widget in our window; then work out geometry rel to our window */
   for (l = drops; l; l = l->next)
     {
        dropable = l->data;
        xwin = _x11_elm_widget_xwin_get(dropable->obj);
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
             if (!strcmp(savedtypes.types[j], _x11_atoms[i].name)) goto found;
          }
     }

   cnp_debug("Didn't find a target\n");
   return EINA_TRUE;

found:
   cnp_debug("Found a target we'd like: %s\n", _x11_atoms[i].name);
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
                  if (savedtypes.imgfile) free(savedtypes.imgfile);
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
                  if (savedtypes.imgfile) free(savedtypes.imgfile);
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
   _x11_selections[ELM_SEL_TYPE_XDND].xwin = xwin;
   _x11_selections[ELM_SEL_TYPE_XDND].requestwidget = dropable->obj;
   _x11_selections[ELM_SEL_TYPE_XDND].requestformat = ELM_SEL_FORMAT_MARKUP;
   _x11_selections[ELM_SEL_TYPE_XDND].active = EINA_TRUE;

   ecore_x_selection_xdnd_request(xwin, _x11_atoms[i].name);
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_position(void *data __UNUSED__, int etype __UNUSED__, void *ev)
{
   struct _Ecore_X_Event_Xdnd_Position *pos = ev;
   Ecore_X_Rectangle rect;

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
_x11_dnd_status(void *data __UNUSED__, int etype __UNUSED__, void *ev)
{
   struct _Ecore_X_Event_Xdnd_Status *status = ev;

   if (!status) return EINA_TRUE;

   /* Only thing we care about: will accept */
   if (status->will_accept) cnp_debug("Will accept\n");
   /* Won't accept */   
   else cnp_debug("Won't accept accept\n");
   return EINA_TRUE;
}

static void
_x11_drag_mouse_up(void *un __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *data)
{
   Ecore_X_Window xwin = *((Ecore_X_Window *)data);
   evas_object_event_callback_del(obj, EVAS_CALLBACK_MOUSE_UP, _x11_drag_mouse_up);
   ecore_x_dnd_drop();

   cnp_debug("mouse up, xwin=%#llx\n", (unsigned long long)xwin);

   /* TODO BUG: should not revert to FALSE if xwin is a drop target! */
   ecore_x_dnd_aware_set(xwin, EINA_FALSE);
   if (dragdonecb)
     {
        dragdonecb(dragdonedata, _x11_selections[ELM_SEL_TYPE_XDND].widget);
        dragdonecb = NULL;
     }
   if (dragwin)
     {
        evas_object_del(dragwin);
        dragwin = NULL;
     }
}

static void
_x11_drag_move(void *data __UNUSED__, Ecore_X_Xdnd_Position *pos)
{
   evas_object_move(dragwin, pos->position.x - _dragx, pos->position.y - _dragy);
}

static Ecore_X_Window
_x11_elm_widget_xwin_get(const Evas_Object *obj)
{
   Evas_Object *top;
   Ecore_X_Window xwin = 0;
   
   top = elm_widget_top_get(obj);
   if (!top) top = elm_widget_top_get(elm_widget_parent_widget_get(obj));
   if (top) xwin = elm_win_xwindow_get(top);
   if (!xwin)
     {
        Ecore_Evas *ee;
        Evas *evas = evas_object_evas_get(obj);
        if (!evas) return 0;
        ee = ecore_evas_ecore_evas_get(evas);
        if (!ee) return 0;
        xwin = _elm_ee_xwin_get(ee);
     }
   return xwin;
}

static Eina_Bool
_x11_elm_cnp_init(void)
{
   int i;
   static int _init_count = 0;
   
   if (_init_count > 0) return EINA_TRUE;
   _init_count++;
   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        _x11_atoms[i].atom = ecore_x_atom_get(_x11_atoms[i].name);
        ecore_x_selection_converter_atom_add
          (_x11_atoms[i].atom, _x11_atoms[i].converter);
     }
   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR, _x11_selection_clear, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, _x11_selection_notify, NULL);
   return EINA_TRUE;
}

static Eina_Bool
_x11_elm_cnp_selection_set(Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, const void *selbuf, size_t buflen)
{
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   X11_Cnp_Selection *sel;
   
   _x11_elm_cnp_init();
   if ((!selbuf) && (format != ELM_SEL_FORMAT_IMAGE))
     return elm_object_cnp_selection_clear(obj, selection);

   sel = _x11_selections + selection;
   if (sel->loss_cb) sel->loss_cb(sel->loss_data, selection);
   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget, EVAS_CALLBACK_DEL,
                                         _x11_sel_obj_del, sel);
   sel->widget = NULL;
   
   sel->active = EINA_TRUE;
   sel->widget = obj;
   sel->xwin = xwin;
   sel->set(xwin, &selection, sizeof(Elm_Sel_Type));
   sel->format = format;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;

   evas_object_event_callback_add
     (sel->widget, EVAS_CALLBACK_DEL, _x11_sel_obj_del, sel);
   
   if (selbuf)
     {
        if (format == ELM_SEL_FORMAT_IMAGE)
          {
             // selbuf is actual image data, not text/string
             sel->selbuf = malloc(buflen);
             if (!sel->selbuf)
               {
                  elm_object_cnp_selection_clear(obj, selection);
                  return EINA_FALSE;
               }
             memcpy(sel->selbuf, selbuf, buflen);
          }
        else
          sel->selbuf = strdup((char*)selbuf);
     }
   else
     sel->selbuf = NULL;

   return EINA_TRUE;
}

static void
_x11_elm_cnp_selection_loss_callback_set(Evas_Object *obj __UNUSED__, Elm_Sel_Type selection, Elm_Selection_Loss_Cb func, const void *data)
{
   X11_Cnp_Selection *sel;
   
   _x11_elm_cnp_init();
   sel = _x11_selections + selection;
   sel->loss_cb = func;
   sel->loss_data = (void *)data;
}

static Eina_Bool
_x11_elm_object_cnp_selection_clear(Evas_Object *obj, Elm_Sel_Type selection)
{
   X11_Cnp_Selection *sel;

   _x11_elm_cnp_init();
   
   sel = _x11_selections + selection;

   /* No longer this selection: Consider it gone! */
   if ((!sel->active) || (sel->widget != obj)) return EINA_TRUE;

   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget, EVAS_CALLBACK_DEL,
                                         _x11_sel_obj_del, sel);
   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget, EVAS_CALLBACK_DEL,
                                         _x11_sel_obj_del2, sel);
   sel->widget = NULL;
   sel->requestwidget = NULL;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;
   
   sel->active = EINA_FALSE;
   if (sel->selbuf)
     {
        free(sel->selbuf);
        sel->selbuf = NULL;
     }
   sel->clear();

   return EINA_TRUE;
}

static Eina_Bool
_x11_elm_cnp_selection_get(Evas_Object *obj, Elm_Sel_Type selection,
                      Elm_Sel_Format format, Elm_Drop_Cb datacb, void *udata)
{
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   X11_Cnp_Selection *sel;

   _x11_elm_cnp_init();
   
   sel = _x11_selections + selection;

   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget, EVAS_CALLBACK_DEL,
                                         _x11_sel_obj_del2, sel);
   sel->requestwidget = NULL;
   
   sel->requestformat = format;
   sel->requestwidget = obj;
   sel->xwin = xwin;
   sel->request(xwin, ECORE_X_SELECTION_TARGET_TARGETS);
   sel->datacb = datacb;
   sel->udata = udata;

   evas_object_event_callback_add
     (sel->requestwidget, EVAS_CALLBACK_DEL, _x11_sel_obj_del2, sel);
   
   return EINA_TRUE;
}

static Eina_Bool
_x11_elm_drop_target_add(Evas_Object *obj, Elm_Sel_Type format,
                         Elm_Drop_Cb dropcb, void *cbdata)
{
   Dropable *drop;
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   Eina_List *item;
   int first;

   _x11_elm_cnp_init();
   
   /* TODO: check if obj is already a drop target. Do not add twice! */

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
   /* TODO BUG: should handle dnd-aware per window, not just the first
    * window that requested it! */
   /* If not the first: We're done */
   if (!first) return EINA_TRUE;

   ecore_x_dnd_aware_set(xwin, EINA_TRUE);

   cnp_debug("Adding drop target calls xwin=%#llx\n", (unsigned long long)xwin);
   handler_enter = ecore_event_handler_add(ECORE_X_EVENT_XDND_ENTER,
                                           _x11_dnd_enter, NULL);
   handler_pos = ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION,
                                         _x11_dnd_position, NULL);
   handler_drop = ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP,
                                          _x11_dnd_drop, NULL);
   return EINA_TRUE;
}

static Eina_Bool
_x11_elm_drop_target_del(Evas_Object *obj)
{
   Dropable *drop,*del;
   Eina_List *item;
   Ecore_X_Window xwin;

   _x11_elm_cnp_init();
   
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

   /* TODO BUG: we should handle dnd-aware per window, not just the last that reelased it */

   /* If still drops there: All fine.. continue */
   if (drops) return EINA_TRUE;

   cnp_debug("Disabling DND\n");
   xwin = _x11_elm_widget_xwin_get(obj);
   ecore_x_dnd_aware_set(xwin, EINA_FALSE);

   ecore_event_handler_del(handler_pos);
   ecore_event_handler_del(handler_drop);
   ecore_event_handler_del(handler_enter);

   if (savedtypes.imgfile) free(savedtypes.imgfile);
   savedtypes.imgfile = NULL;

   return EINA_TRUE;
}

static  Eina_Bool
_x11_elm_drag_start(Evas_Object *obj, Elm_Sel_Format format, const char *data, void (*dragdone) (void *data, Evas_Object *), void *donecbdata)
{
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   X11_Cnp_Selection *sel;
   Elm_Sel_Type xdnd = ELM_SEL_TYPE_XDND;
   Ecore_Evas *ee;
   int x, y, x2, y2, x3, y3;
   Evas_Object *icon;
   int w, h;

   _x11_elm_cnp_init();
   
   cnp_debug("starting drag...\n");

   if (dragwin)
     {
        cnp_debug("another obj is dragging...\n");
        return EINA_FALSE;
     }

   ecore_x_dnd_type_set(xwin, "text/uri-list", EINA_TRUE);
   sel = _x11_selections + ELM_SEL_TYPE_XDND;
   sel->active = EINA_TRUE;
   sel->widget = obj;
   sel->format = format;
   sel->selbuf = data ? strdup(data) : NULL;

   /* TODO BUG: should NEVER have these as globals! They should be per context (window). */
   dragdonecb = dragdone;
   dragdonedata = donecbdata;
   /* TODO BUG: should increase dnd-awareness, in case it's drop target as well. See _x11_drag_mouse_up() */
   ecore_x_dnd_aware_set(xwin, EINA_TRUE);
   ecore_x_dnd_callback_pos_update_set(_x11_drag_move, NULL);
   ecore_x_dnd_begin(xwin, (unsigned char *)&xdnd, sizeof(Elm_Sel_Type));
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP,
                                  _x11_drag_mouse_up, (void *)(long)xwin);
   handler_status = ecore_event_handler_add(ECORE_X_EVENT_XDND_STATUS,
                                            _x11_dnd_status, NULL);
   dragwin = elm_win_add(NULL, "Elm Drag Object", ELM_WIN_UTILITY);
   elm_win_override_set(dragwin, EINA_TRUE);

   /* FIXME: Images only */
   icon = elm_icon_add(dragwin);
   elm_image_file_set(icon, data + 7, NULL); /* 7!? "file://" */
   elm_win_resize_object_add(dragwin, icon);
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

static Eina_Bool
_x11_elm_selection_selection_has_owner(Evas_Object *obj __UNUSED__)
{
   _x11_elm_cnp_init();
   return !!ecore_x_selection_owner_get(ECORE_X_ATOM_SELECTION_CLIPBOARD);
}

#endif

#ifdef HAVE_ELEMENTARY_WAYLAND
typedef struct _Wl_Cnp_Selection Wl_Cnp_Selection;

struct _Wl_Cnp_Selection
{
   char *selbuf;
   int buflen;

   Evas_Object *widget;
   Evas_Object *requestwidget;
};

static Wl_Cnp_Selection wl_cnp_selection = {0, 0, NULL, NULL};

static void
_wl_sel_obj_del2(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Wl_Cnp_Selection *sel = data;
   if (sel->requestwidget == obj) sel->requestwidget = NULL;
}

static Eina_Bool 
_wl_elm_cnp_selection_set(Evas_Object *obj __UNUSED__, Elm_Sel_Type selection, Elm_Sel_Format format __UNUSED__, const void *selbuf, size_t buflen)
{
   const char *types[10] = {0, };

   /* TODO: other EML_SEL_TYPE and ELM_SEL_FORMAT */
   if (ELM_SEL_TYPE_CLIPBOARD == selection) 
     {
        types[0] = "text/plain;charset=utf-8";
        ecore_wl_dnd_set_selection(ecore_wl_dnd_get(), types);

        if (wl_cnp_selection.selbuf) free(wl_cnp_selection.selbuf);
        wl_cnp_selection.selbuf = strdup((char*)selbuf);
        wl_cnp_selection.buflen = buflen;
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool 
_wl_elm_cnp_selection_get(Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format __UNUSED__, Elm_Drop_Cb datacb __UNUSED__, void *udata __UNUSED__)
{
   /* For now, just avoid overlapped request */
   if (wl_cnp_selection.requestwidget) return EINA_FALSE;

   /* TODO: other EML_SEL_TYPE and ELM_SEL_FORMAT */
   if (ELM_SEL_TYPE_CLIPBOARD == selection)
     {
        wl_cnp_selection.requestwidget = obj;
        evas_object_event_callback_add(wl_cnp_selection.requestwidget, EVAS_CALLBACK_DEL,
                                       _wl_sel_obj_del2, &wl_cnp_selection);
        ecore_wl_dnd_get_selection(ecore_wl_dnd_get(), "text/plain;charset=utf-8");
     }
   return EINA_TRUE;
}

static Eina_Bool
_wl_selection_send(void *udata, int type __UNUSED__, void *event)
{
   char *buf;
   int ret, len_remained;
   int len_written = 0;
   Wl_Cnp_Selection *sel = udata;
   Ecore_Wl_Event_Data_Source_Send *ev = event;

   len_remained = sel->buflen;
   buf = sel->selbuf;

   while (len_written < sel->buflen)
     {
        ret = write(ev->fd, buf, len_remained);
        if (ret == -1) break;
        buf += ret;
        len_written += ret;
        len_remained -= ret;
     }

   close(ev->fd);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_selection_receive(void *udata, int type __UNUSED__, void *event)
{
   Wl_Cnp_Selection *sel = udata;
   Ecore_Wl_Event_Selection_Data_Ready *ev = event;

   if (sel->requestwidget)
     {
        if (!ev->done)
          {
             /* TODO BUG: should never NEVER assume it's an elm_entry! */
             _elm_entry_entry_paste(sel->requestwidget, ev->data);
          }
        else
          {
             evas_object_event_callback_del_full(sel->requestwidget, EVAS_CALLBACK_DEL,
                                                 _wl_sel_obj_del2, sel);
             sel->requestwidget = NULL;
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool _wl_elm_cnp_init(void)
{
   ecore_event_handler_add(ECORE_WL_EVENT_DATA_SOURCE_SEND, 
                           _wl_selection_send, &wl_cnp_selection);
   ecore_event_handler_add(ECORE_WL_EVENT_SELECTION_DATA_READY, 
                           _wl_selection_receive, &wl_cnp_selection);

   return EINA_TRUE;
}
#endif






////////////////////////////////////////////////////////////////////////////
// for local (Within 1 app/process) cnp (used by fb as fallback
////////////////////////////////////////////////////////////////////////////
#if 1
typedef struct _Local_Selinfo Local_Selinfo;

struct _Local_Selinfo
{
   Elm_Sel_Format format;
   struct {
      void *buf;
      size_t size;
   } sel;
   struct {
      Evas_Object *obj;
      Elm_Drop_Cb func;
      void *data;
      Ecore_Job *job;
   } get;
};

// for ELM_SEL_TYPE_PRIMARY, ELM_SEL_TYPE_SECONDARY, ELM_SEL_TYPE_XDND,
// ELM_SEL_TYPE_CLIPBOARD
static Local_Selinfo _local_selinfo[4];

static void       _local_get_job(void *data);

static Eina_Bool  _local_elm_cnp_init(void);
static Eina_Bool  _local_elm_cnp_selection_set(Evas_Object *obj __UNUSED__, Elm_Sel_Type selection, Elm_Sel_Format format, const void *selbuf, size_t buflen);
static void       _local_elm_cnp_selection_loss_callback_set(Evas_Object *obj __UNUSED__, Elm_Sel_Type selection __UNUSED__, Elm_Selection_Loss_Cb func __UNUSED__, const void *data __UNUSED__);
static Eina_Bool  _local_elm_object_cnp_selection_clear(Evas_Object *obj __UNUSED__, Elm_Sel_Type selection);
static Eina_Bool  _local_elm_cnp_selection_get(Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format __UNUSED__, Elm_Drop_Cb datacb, void *udata);
static  Eina_Bool _local_elm_drop_target_add(Evas_Object *obj __UNUSED__, Elm_Sel_Type format __UNUSED__, Elm_Drop_Cb dropcb __UNUSED__, void *cbdata __UNUSED__);
static  Eina_Bool _local_elm_drop_target_del(Evas_Object *obj __UNUSED__);
static Eina_Bool  _local_elm_drag_start(Evas_Object *obj __UNUSED__, Elm_Sel_Format format __UNUSED__, const char *data __UNUSED__, void (*dragdone) (void *data, Evas_Object *) __UNUSED__, void *donecbdata __UNUSED__);
static Eina_Bool  _local_elm_selection_selection_has_owner(Evas_Object *obj __UNUSED__);

static void
_local_get_job(void *data)
{
   Local_Selinfo *info = data;
   Elm_Selection_Data ev;
   
   info->get.job = NULL;
   ev.x = 0;
   ev.y = 0;
   ev.format = info->format;
   ev.data = info->sel.buf;
   ev.len = info->sel.size;
   if (info->get.func)
     info->get.func(info->get.data, info->get.obj, &ev);
}

static Eina_Bool
_local_elm_cnp_init(void)
{
   static int _init_count = 0;
   
   if (_init_count > 0) return EINA_TRUE;
   _init_count++;
   memset(&(_local_selinfo), 0, sizeof(_local_selinfo));
   return EINA_TRUE;
}

static Eina_Bool
_local_elm_cnp_selection_set(Evas_Object *obj __UNUSED__,
                             Elm_Sel_Type selection, Elm_Sel_Format format,
                             const void *selbuf, size_t buflen)
{
   _local_elm_cnp_init();
   if (_local_selinfo[selection].sel.buf)
     free(_local_selinfo[selection].sel.buf);
   _local_selinfo[selection].format = format;
   _local_selinfo[selection].sel.buf = malloc(buflen);
   if (_local_selinfo[selection].sel.buf)
     {
        memcpy(_local_selinfo[selection].sel.buf, selbuf, buflen);
        _local_selinfo[selection].sel.size = buflen;
     }
   else
     _local_selinfo[selection].sel.size = 0;
   return EINA_TRUE;
}

static void
_local_elm_cnp_selection_loss_callback_set(Evas_Object *obj __UNUSED__,
                                           Elm_Sel_Type selection __UNUSED__,
                                           Elm_Selection_Loss_Cb func __UNUSED__,
                                           const void *data __UNUSED__)
{
   _local_elm_cnp_init();
   // this doesnt need to do anything as we never lose selection to anyone
   // as thisis local
}

static Eina_Bool
_local_elm_object_cnp_selection_clear(Evas_Object *obj __UNUSED__,
                                      Elm_Sel_Type selection)
{
   _local_elm_cnp_init();
   if (_local_selinfo[selection].sel.buf)
     free(_local_selinfo[selection].sel.buf);
   _local_selinfo[selection].sel.buf = NULL;
   _local_selinfo[selection].sel.size = 0;
   return EINA_TRUE;
}

static Eina_Bool
_local_elm_cnp_selection_get(Evas_Object *obj,
                             Elm_Sel_Type selection,
                             Elm_Sel_Format format __UNUSED__,
                             Elm_Drop_Cb datacb, void *udata)
{
   _local_elm_cnp_init();
   if (_local_selinfo[selection].get.job)
     ecore_job_del(_local_selinfo[selection].get.job);
   _local_selinfo[selection].get.obj = obj;
   _local_selinfo[selection].get.func = datacb;
   _local_selinfo[selection].get.data = udata;
   _local_selinfo[selection].get.job = 
     ecore_job_add(_local_get_job, &(_local_selinfo[selection]));
   return EINA_TRUE;
}

static  Eina_Bool
_local_elm_drop_target_add(Evas_Object *obj __UNUSED__,
                           Elm_Sel_Type format __UNUSED__,
                           Elm_Drop_Cb dropcb __UNUSED__,
                           void *cbdata __UNUSED__)
{
   // XXX: implement me
   _local_elm_cnp_init();
   return EINA_FALSE;
}

static  Eina_Bool
_local_elm_drop_target_del(Evas_Object *obj __UNUSED__)
{
   // XXX: implement me
   _local_elm_cnp_init();
   return EINA_FALSE;
}

static Eina_Bool
_local_elm_drag_start(Evas_Object *obj __UNUSED__,
                      Elm_Sel_Format format __UNUSED__,
                      const char *data __UNUSED__,
                      void (*dragdone) (void *data, Evas_Object *) __UNUSED__,
                      void *donecbdata __UNUSED__)
{
   // XXX: implement me
   _local_elm_cnp_init();
   return EINA_FALSE;
}

static Eina_Bool
_local_elm_selection_selection_has_owner(Evas_Object *obj __UNUSED__)
{
   _local_elm_cnp_init();
   if (_local_selinfo[ELM_SEL_TYPE_CLIPBOARD].sel.buf) return EINA_TRUE;
   return EINA_FALSE;
}
#endif






// common internal funcs
////////////////////////////////////////////////////////////////////////////
static Eina_Bool
_elm_cnp_init(Evas_Object *obj)
{
   if (_elm_cnp_init_count > 0) return EINA_TRUE;
   _elm_cnp_init_count++;
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj)) _x11_elm_cnp_init();
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   if (elm_win_wl_window_get(obj)) _wl_elm_cnp_init();
#endif
   _local_elm_cnp_init();
   text_uri = eina_stringshare_add("text/uri-list");
   return EINA_TRUE;
}

/* TODO: this should not be an actual tempfile, but rather encode the object
 * as http://dataurl.net/ if it's an image or similar. Evas should support
 * decoding it as memfile. */
static Tmp_Info *
_tempfile_new(int size)
{
#ifdef HAVE_MMAN_H
   Tmp_Info *info;
   const char *tmppath;
   int len;

   info = malloc(sizeof(Tmp_Info));
   if (!info) return NULL;
   tmppath = getenv("TMP");
   if (!tmppath) tmppath = P_tmpdir;
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
#else
   (void) size;
   return NULL;
#endif
}

static int
_tmpinfo_free(Tmp_Info *info)
{
   if (!info) return 0;
   free(info->filename);
   free(info);
   return 0;
}

static Eina_Bool
_pasteimage_append(char *file, Evas_Object *entry)
{
   char *entrytag;
   int len;
   /* TODO BUG: shouldn't define absize=240x180. Prefer data:// instead of href:// -- may need support for evas. See  http://dataurl.net/ */
   static const char *tagstring = "<item absize=240x180 href=file://%s></item>";

   if ((!file) || (!entry)) return EINA_FALSE;
   len = strlen(tagstring)+strlen(file);
   entrytag = alloca(len + 1);
   snprintf(entrytag, len + 1, tagstring, file);
   /* TODO BUG: should never NEVER assume it's an elm_entry! */
   _elm_entry_entry_paste(entry, entrytag);
   return EINA_TRUE;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// common exposed funcs
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
EAPI Eina_Bool
elm_cnp_selection_set(Evas_Object *obj, Elm_Sel_Type selection,
                      Elm_Sel_Format format, const void *selbuf, size_t buflen)
{
   if (selection > ELM_SEL_TYPE_CLIPBOARD) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init(obj);
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_cnp_selection_set(obj, selection, format, selbuf, buflen);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   if (elm_win_wl_window_get(obj))
      return _wl_elm_cnp_selection_set(obj, selection, format, selbuf, buflen);
#endif
   return _local_elm_cnp_selection_set(obj, selection, format, selbuf, buflen);
}

EAPI void
elm_cnp_selection_loss_callback_set(Evas_Object *obj, Elm_Sel_Type selection,
                                    Elm_Selection_Loss_Cb func,
                                    const void *data)
{
   if (selection > ELM_SEL_TYPE_CLIPBOARD) return;
   if (!_elm_cnp_init_count) _elm_cnp_init(obj);
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     _x11_elm_cnp_selection_loss_callback_set(obj, selection, func, data);
#endif   
   _local_elm_cnp_selection_loss_callback_set(obj, selection, func, data);
}

EAPI Eina_Bool
elm_object_cnp_selection_clear(Evas_Object *obj, Elm_Sel_Type selection)
{
   if (selection > ELM_SEL_TYPE_CLIPBOARD) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init(obj);
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_object_cnp_selection_clear(obj, selection);
#endif
   return _local_elm_object_cnp_selection_clear(obj, selection);
}

EAPI Eina_Bool
elm_cnp_selection_get(Evas_Object *obj, Elm_Sel_Type selection,
                      Elm_Sel_Format format, Elm_Drop_Cb datacb, void *udata)
{
   if (selection > ELM_SEL_TYPE_CLIPBOARD) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init(obj);
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_cnp_selection_get(obj, selection, format, datacb, udata);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   if (elm_win_wl_window_get(obj))
      return _wl_elm_cnp_selection_get(obj, selection, format, datacb, udata);
#endif
   return _local_elm_cnp_selection_get(obj, selection, format, datacb, udata);
}

////////////////////////////////////////////////////////////////////////////

/**
 * Add a widget as drop target.
 */
EAPI Eina_Bool
elm_drop_target_add(Evas_Object *obj, Elm_Sel_Type format,
                    Elm_Drop_Cb dropcb, void *cbdata)
{
   if (!_elm_cnp_init_count) _elm_cnp_init(obj);
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_drop_target_add(obj, format, dropcb, cbdata);
#endif
   return _local_elm_drop_target_add(obj, format, dropcb, cbdata);
}

EAPI Eina_Bool
elm_drop_target_del(Evas_Object *obj)
{
   if (!_elm_cnp_init_count) _elm_cnp_init(obj);
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_drop_target_del(obj);
#endif
   return _local_elm_drop_target_del(obj);
}

EAPI Eina_Bool
elm_drag_start(Evas_Object *obj, Elm_Sel_Format format, const char *data, 
               void (*dragdone) (void *data, Evas_Object *), void *donecbdata)
{
   if (!_elm_cnp_init_count) _elm_cnp_init(obj);
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_drag_start(obj, format, data, dragdone, donecbdata);
#endif
   return _local_elm_drag_start(obj, format, data, dragdone, donecbdata);
}

EAPI Eina_Bool
elm_selection_selection_has_owner(Evas_Object *obj)
{
   if (!_elm_cnp_init_count) _elm_cnp_init(obj);
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_selection_selection_has_owner(obj);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   if (elm_win_wl_window_get(obj))
     return ecore_wl_dnd_selection_has_owner(ecore_wl_dnd_get());
#endif
   return _local_elm_selection_selection_has_owner(obj);
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-3f0^-2{2(0W1st0 :*/
