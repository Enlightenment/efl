#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include "elm_priv.h"
#ifdef HAVE_MMAN_H
# include <sys/mman.h>
#endif

//#define DEBUGON 1
#ifdef DEBUGON
# define cnp_debug(fmt, args...) fprintf(stderr, __FILE__":%s : " fmt , __FUNCTION__, ##args)
#else
# define cnp_debug(x...) do { } while (0)
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
   CNP_ATOM_COMPOUND_TEXT,
   CNP_ATOM_TEXT,
   CNP_ATOM_text_plain_utf8,
   CNP_ATOM_text_plain,

   CNP_N_ATOMS,
};

typedef struct _Tmp_Info      Tmp_Info;
typedef struct _Saved_Type    Saved_Type;
typedef struct _Cnp_Escape    Cnp_Escape;
typedef struct _Dropable      Dropable;
typedef struct _Dropable_Cbs  Dropable_Cbs;
static Eina_Bool doaccept = EINA_FALSE;

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

struct _Cnp_Escape
{
   const char *escape;
   const char *value;
};

struct _Dropable_Cbs
{
   EINA_INLIST;
   Elm_Sel_Format  types;
   Elm_Drag_State  entercb;
   Elm_Drag_State  leavecb;
   Elm_Drag_Pos    poscb;
   Elm_Drop_Cb     dropcb;
   void           *enterdata;
   void           *leavedata;
   void           *posdata;
   void           *dropdata;
};

struct _Dropable
{
   Evas_Object    *obj;
   /* FIXME: Cache window */
   Eina_Inlist    *cbs_list; /* List of Dropable_Cbs * */
   struct {
      Evas_Coord      x, y;
      Eina_Bool       in : 1;
      const char     *type;
      Elm_Sel_Format  format;
   } last;
};

struct _Item_Container_Drop_Info
{  /* Info kept for containers to support drop */
   Evas_Object *obj;
   Elm_Xy_Item_Get_Cb itemgetcb;
   Elm_Drop_Item_Container_Cb dropcb;
   Elm_Drag_Item_Container_Pos poscb;
};
typedef struct _Item_Container_Drop_Info Item_Container_Drop_Info;

struct _Anim_Icon
{
   int start_x;
   int start_y;
   int start_w;
   int start_h;
   Evas_Object *o;
};
typedef struct _Anim_Icon Anim_Icon;

struct _Item_Container_Drag_Info
{  /* Info kept for containers to support drag */
   Evas_Object *obj;
   Ecore_Timer *tm;    /* When this expires, start drag */
   double anim_tm;  /* Time period to set tm         */
   double tm_to_drag;  /* Time period to set tm         */
   Elm_Xy_Item_Get_Cb itemgetcb;
   Elm_Item_Container_Data_Get_Cb data_get;

   Evas_Coord x_down;  /* Mouse down x cord when drag starts */
   Evas_Coord y_down;  /* Mouse down y cord when drag starts */

   /* Some extra information needed to impl default anim */
   Evas *e;
   Eina_List *icons;   /* List of icons to animate (Anim_Icon) */
   int final_icon_w; /* We need the w and h of the final icon for the animation */
   int final_icon_h;
   Ecore_Animator *ea;

   Elm_Drag_User_Info user_info;
};
typedef struct _Item_Container_Drag_Info Item_Container_Drag_Info;

static int _elm_cnp_init_count = 0;
/* Stringshared, so I can just compare pointers later */
static const char *text_uri;

/* Data for DND in progress */
static Saved_Type savedtypes =  { NULL, NULL, 0, 0, 0, EINA_FALSE };

/* Drag & Drop functions */
/* FIXME: Way too many globals */
static Eina_List *drops = NULL;
static Evas_Object *dragwin = NULL;
static int dragwin_x_start, dragwin_y_start;
static int dragwin_x_end, dragwin_y_end;
static int _dragx = 0, _dragy = 0;
static Ecore_Event_Handler *handler_pos = NULL;
static Ecore_Event_Handler *handler_drop = NULL;
static Ecore_Event_Handler *handler_enter = NULL;
static Ecore_Event_Handler *handler_status = NULL;
static Ecore_Event_Handler *handler_leave = NULL;
static Ecore_Event_Handler *handler_up = NULL;

/* TODO BUG: should NEVER have these as globals! They should be per context (window). */
static Elm_Drag_Pos dragposcb = NULL;
static Elm_Drag_Accept dragacceptcb = NULL;
static Elm_Drag_State dragdonecb = NULL;
static void *dragposdata = NULL;
static void *dragacceptdata = NULL;
static void *dragdonedata = NULL;
static Evas_Object *dragwidget = NULL;
static Elm_Xdnd_Action dragaction = ELM_XDND_ACTION_UNKNOWN;

static Eina_List *cont_drop_tg = NULL; /* List of Item_Container_Drop_Info */
static Eina_List *cont_drag_tg = NULL; /* List of Item_Container_Drag_Info */

static void _cont_obj_mouse_up( void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _cont_obj_mouse_move( void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _all_drop_targets_cbs_del(void *data, Evas *e, Evas_Object *obj, void *info);
static Ecore_X_Window _x11_elm_widget_xwin_get(const Evas_Object *obj);

static Eina_Bool
_drag_cancel_animate(void *data EINA_UNUSED, double pos)
{  /* Animation to "move back" drag-window */
   if (pos >= 0.99)
     {
        Ecore_X_Window xdragwin = _x11_elm_widget_xwin_get(data);
        ecore_x_window_ignore_set(xdragwin, 0);
        evas_object_del(data);
        return ECORE_CALLBACK_CANCEL;
     }
   else
     {
        int x, y;
        x = dragwin_x_end - (pos * (dragwin_x_end - dragwin_x_start));
        y = dragwin_y_end - (pos * (dragwin_y_end - dragwin_y_start));
        evas_object_move(data, x, y);
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_all_drop_targets_cbs_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *info EINA_UNUSED)
{
   Dropable *dropable = NULL;
   eo_do(obj, eo_base_data_get("__elm_dropable", (void **)&dropable));
   if (dropable)
     {
        Dropable_Cbs *cbs;
        while (dropable->cbs_list)
          {
             cbs = EINA_INLIST_CONTAINER_GET(dropable->cbs_list, Dropable_Cbs);
             elm_drop_target_del(obj, cbs->types,
                                 cbs->entercb, cbs->enterdata, cbs->leavecb, cbs->leavedata,
                                 cbs->poscb, cbs->posdata, cbs->dropcb, cbs->dropdata);
             // If elm_drop_target_del() happened to delete dropabale, then
             // re-fetch it each loop to make sure it didn't
             eo_do(obj, eo_base_data_get("__elm_dropable", (void **)&dropable));
             if (!dropable) break;
          }
     }
}

#ifdef HAVE_ELEMENTARY_X
static Tmp_Info  *_tempfile_new      (int size);
static int        _tmpinfo_free      (Tmp_Info *tmp);
static Eina_Bool  _pasteimage_append (char *file, Evas_Object *entry);
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
   Elm_Xdnd_Action    action;

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

static void           _x11_sel_obj_del              (void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED);
static void           _x11_sel_obj_del2             (void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED);
static Eina_Bool      _x11_selection_clear          (void *udata EINA_UNUSED, int type, void *event);
static Eina_Bool      _x11_selection_notify         (void *udata EINA_UNUSED, int type, void *event);
static Eina_Bool      _x11_targets_converter        (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool      _x11_text_converter           (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool      _x11_general_converter        (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool      _x11_image_converter          (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool      _x11_vcard_send               (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
static Eina_Bool      _x11_is_uri_type_data         (X11_Cnp_Selection *sel EINA_UNUSED, Ecore_X_Event_Selection_Notify *notify);
static int            _x11_response_handler_targets (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *);
static int            _x11_notify_handler_targets   (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int            _x11_notify_handler_text      (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int            _x11_notify_handler_image     (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int            _x11_notify_handler_uri       (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
//static int            _x11_notify_handler_html      (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static int            _x11_vcard_receive            (X11_Cnp_Selection *sed, Ecore_X_Event_Selection_Notify *notify);
static Eina_Bool      _x11_dnd_enter                (void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev);
static Eina_Bool      _x11_dnd_drop                 (void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev);
static Eina_Bool      _x11_dnd_position             (void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev);
static Eina_Bool      _x11_dnd_status               (void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev);
static Eina_Bool      _x11_dnd_leave                (void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev);
static Eina_Bool      _x11_drag_mouse_up            (void *data, int etype EINA_UNUSED, void *event);
static void           _x11_drag_move                (void *data EINA_UNUSED, Ecore_X_Xdnd_Position *pos);

static Ecore_X_Window _x11_elm_widget_xwin_get           (const Evas_Object *obj);

static Eina_Bool _x11_elm_cnp_init                       (void);
static Eina_Bool _x11_elm_cnp_selection_set              (Ecore_X_Window xwin, Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, const void *selbuf, size_t buflen);
static void      _x11_elm_cnp_selection_loss_callback_set(Evas_Object *obj EINA_UNUSED, Elm_Sel_Type selection, Elm_Selection_Loss_Cb func, const void *data);
static Eina_Bool _x11_elm_object_cnp_selection_clear     (Evas_Object *obj, Elm_Sel_Type selection);
static Eina_Bool _x11_elm_cnp_selection_get              (Ecore_X_Window xwin, Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, Elm_Drop_Cb datacb, void *udata);
static Eina_Bool _x11_elm_drop_target_add                (Evas_Object *obj, Elm_Sel_Format format,
                                                          Elm_Drag_State entercb, void *enterdata,
                                                          Elm_Drag_State leavecb, void *leavedata,
                                                          Elm_Drag_Pos poscb, void *posdata,
                                                          Elm_Drop_Cb dropcb, void *dropdata);
static Eina_Bool _x11_elm_drop_target_del                (Evas_Object *obj, Elm_Sel_Format format,
                                                          Elm_Drag_State entercb, void *enterdata,
                                                          Elm_Drag_State leavecb, void *leavedata,
                                                          Elm_Drag_Pos poscb, void *posdata,
                                                          Elm_Drop_Cb dropcb, void *dropdata);
static Eina_Bool _x11_elm_selection_selection_has_owner  (Evas_Object *obj EINA_UNUSED);

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
   [CNP_ATOM_COMPOUND_TEXT] = {
      "COMPOUND_TEXT",
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
      _x11_notify_handler_text,
      0
   },
   [CNP_ATOM_text_plain_utf8] = {
      "text/plain;charset=utf-8",
      ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
      _x11_text_converter,
      NULL,
      _x11_notify_handler_text,
      0
   },
   [CNP_ATOM_text_plain] = {
      "text/plain",
      ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
      _x11_text_converter,
      NULL,
      _x11_notify_handler_text,
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
_x11_sel_obj_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   X11_Cnp_Selection *sel = data;
   if (sel->widget == obj) sel->widget = NULL;
   if (dragwidget == obj) dragwidget = NULL;
}

static void
_x11_sel_obj_del2(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   X11_Cnp_Selection *sel = data;
   if (sel->requestwidget == obj) sel->requestwidget = NULL;
}

static Eina_Bool
_x11_selection_clear(void *udata EINA_UNUSED, int type EINA_UNUSED, void *event)
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
   ELM_SAFE_FREE(sel->selbuf, free);
   return ECORE_CALLBACK_PASS_ON;
}

/*
 * Response to a selection notify:
 *  - So we have asked for the selection list.
 *  - If it's the targets list, parse it, and fire of what we want,
 *    else it's the data we want.
 */
static Eina_Bool
_x11_selection_notify(void *udata EINA_UNUSED, int type EINA_UNUSED, void *event)
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
_x11_targets_converter(char *target EINA_UNUSED, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize)
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
_x11_image_converter(char *target EINA_UNUSED, void *data EINA_UNUSED, int size EINA_UNUSED, void **data_ret EINA_UNUSED, int *size_ret EINA_UNUSED, Ecore_X_Atom *ttype EINA_UNUSED, int *typesize EINA_UNUSED)
{
   cnp_debug("Image converter called\n");
   return EINA_TRUE;
}

static Eina_Bool
_x11_vcard_send(char *target EINA_UNUSED, void *data EINA_UNUSED, int size EINA_UNUSED, void **data_ret, int *size_ret, Ecore_X_Atom *ttype EINA_UNUSED, int *typesize EINA_UNUSED)
{
   X11_Cnp_Selection *sel;

   cnp_debug("Vcard send called\n");
   sel = _x11_selections + *((int *)data);
   if (data_ret) *data_ret = strdup(sel->selbuf);
   if (size_ret) *size_ret = strlen(sel->selbuf);
   return EINA_TRUE;
}

static Eina_Bool
_x11_is_uri_type_data(X11_Cnp_Selection *sel EINA_UNUSED, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data *data;
   char *p;

   data = notify->data;
   cnp_debug("data->format is %d %p %p\n", data->format, notify, data);
   if (data->content == ECORE_X_SELECTION_CONTENT_FILES) return EINA_TRUE;
   p = (char *)data->data;
   if (!p) return EINA_TRUE;
   cnp_debug("Got %s\n", p);
   if (strncmp(p, "file:/", 6))
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
   Eina_List *l;
   Dropable *dropable;

   data = notify->data;
   if (sel == (_x11_selections + ELM_SEL_TYPE_XDND))
     {
        Elm_Selection_Data ddata;

        cnp_debug("drag & drop\n");
        /* FIXME: this needs to be generic: Used for all receives */
        EINA_LIST_FOREACH(drops, l, dropable)
          {
             if (dropable->obj == sel->requestwidget) break;
             dropable = NULL;
          }
        if (dropable)
          {
             Dropable_Cbs *cbs;
             Eina_Inlist *itr;
             ddata.x = savedtypes.x;
             ddata.y = savedtypes.y;
             ddata.format = ELM_SEL_FORMAT_TEXT;
             ddata.data = data->data;
             ddata.len = data->length;
             ddata.action = sel->action;
             EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                if (cbs->dropcb)
                  cbs->dropcb(cbs->dropdata, dropable->obj, &ddata);
             goto end;
          }
     }
   if (sel->datacb)
     {
        Elm_Selection_Data ddata;

        ddata.x = ddata.y = 0;
        ddata.format = ELM_SEL_FORMAT_TEXT;
        ddata.data = data->data;
        ddata.len = data->length;
        ddata.action = sel->action;
        sel->datacb(sel->udata, sel->widget, &ddata);
     }
   else
     {
        char *stripstr, *mkupstr;

        stripstr = malloc(data->length + 1);
        if (!stripstr) goto end;
        strncpy(stripstr, (char *)data->data, data->length);
        stripstr[data->length] = '\0';
        cnp_debug("Notify handler text %d %d %p\n", data->format,
                  data->length, data->data);
        mkupstr = _elm_util_text_to_mkup((const char *)stripstr);
        cnp_debug("String is %s (from %s)\n", stripstr, data->data);
        /* TODO BUG: should never NEVER assume it's an elm_entry! */
        _elm_entry_entry_paste(sel->requestwidget, mkupstr);
        free(stripstr);
        free(mkupstr);
     }
end:
   if (sel == (_x11_selections + ELM_SEL_TYPE_XDND))
     ecore_x_dnd_send_finished();
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
   char *p, *s, *stripstr = NULL;

   data = notify->data;
   cnp_debug("data->format is %d %p %p\n", data->format, notify, data);
   if (data->content == ECORE_X_SELECTION_CONTENT_FILES)
     {
        int i, len = 0;

        cnp_debug("got a files list\n");
        files = notify->data;
        /*
        if (files->num_files > 1)
          {
             // Don't handle many items <- this makes mr bigglesworth sad :(
             cnp_debug("more then one file: Bailing\n");
             return 0;
          }
        stripstr = p = strdup(files->files[0]);
         */
        for (i = 0; i < files->num_files ; i++)
          {
             p = files->files[i];
             if ((strncmp(p, "file:/", 6)) && (p[0] != '/')) continue;
             len += strlen(files->files[i]) + 1;
          }
        p = NULL;
        if (len > 0)
          {
             s = stripstr = malloc(len + 1);
             for (i = 0; i < files->num_files ; i++)
               {
                  p = files->files[i];
                  if (!strncmp(p, "file:/", 6)) p += 5;
                  else if (p[0] != '/') continue;
                  len = strlen(p);
                  strcpy(s, p);
                  if (i < (files->num_files - 1))
                    {
                       s[len] = '\n';
                       s[len + 1] = 0;
                       s += len + 1;
                    }
                  else
                    {
                       s[len] = 0;
                       s += len;
                    }
               }
          }
     }
   else
     {
        p = (char *)data->data;
        if ((!strncmp(p, "file:/", 6)) || (p[0] == '/'))
          {
             int len = data->length;
             if (!strncmp(p, "file:/", 6))
               {
                  p += 5;
                  len -= 5;
               }
             stripstr = malloc(len + 1);
             if (!stripstr) return 0;
             memcpy(stripstr, p, len);
             stripstr[len] = 0;
          }
     }
   if (!stripstr)
     {
        cnp_debug("Couldn't find a file\n");
        return 0;
     }
   free(savedtypes.imgfile);
   if (savedtypes.textreq)
     {
        savedtypes.textreq = 0;
        savedtypes.imgfile = stripstr;
     }
   else
     {
        savedtypes.imgfile = NULL;
        _pasteimage_append(p, sel->requestwidget);
        free(stripstr);
     }
   return 0;
}

/**
 * Just received an vcard, either through cut and paste, or dnd.
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
        Dropable_Cbs *cbs;
        Eina_Inlist *itr;
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
        ddata.action = sel->action;
        EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
           if (cbs->dropcb)
             cbs->dropcb(cbs->dropdata, dropable->obj, &ddata);
        ecore_x_dnd_send_finished();
     }
   else if (sel->datacb)
     {
        Elm_Selection_Data ddata;
        ddata.x = ddata.y = 0;
        ddata.format = ELM_SEL_FORMAT_VCARD;
        ddata.data = data->data;
        ddata.len = data->length;
        ddata.action = sel->action;
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
        ddata.action = sel->action;
        sel->datacb(sel->udata, sel->widget, &ddata);
        return 0;
     }
   /* generate tmp name */
   tmp = _tempfile_new(data->length);
   if (!tmp) return 0;
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
        ddata.action = ELM_XDND_ACTION_UNKNOWN;
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
_x11_text_converter(char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize)
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
        if (size_ret && *data_ret) *size_ret = strlen(*data_ret);
     }
   else if (sel->format & ELM_SEL_FORMAT_TEXT)
     {
        ecore_x_selection_converter_text(target, sel->selbuf,
                                         strlen(sel->selbuf),
                                         data_ret, size_ret,
                                         ttype, typesize);
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
_x11_general_converter(char *target EINA_UNUSED, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype EINA_UNUSED, int *typesize EINA_UNUSED)
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
        if (sel->selbuf)
          {
             if (data_ret) *data_ret = strdup(sel->selbuf);
             if (size_ret) *size_ret = strlen(sel->selbuf);
          }
        else
          {
             if (data_ret) *data_ret = NULL;
             if (size_ret) *size_ret = 0;
          }
     }
   return EINA_TRUE;
}

static Dropable *
_x11_dropable_find(Ecore_X_Window win)
{
   Eina_List *l;
   Dropable *dropable;

   if (!drops) return NULL;
   EINA_LIST_FOREACH(drops, l, dropable)
     {
        if (_x11_elm_widget_xwin_get(dropable->obj) == win) return dropable;
     }
   return NULL;
}

static Eina_List *
_x11_dropable_list_geom_find(Ecore_X_Window win, Evas_Coord px, Evas_Coord py)
{
   Eina_List *itr, *top_objects_list = NULL, *dropable_list = NULL;
   Evas *evas = NULL;
   Evas_Object *top_obj;
   Dropable *dropable = NULL;

   if (!drops) return NULL;
   /* Find the Evas connected to the window */
   EINA_LIST_FOREACH(drops, itr, dropable)
     {
        if (_x11_elm_widget_xwin_get(dropable->obj) == win)
          {
             evas = evas_object_evas_get(dropable->obj);
             break;
          }
     }
   if (!evas) return NULL;

   /* We retrieve the (non-smart) objects pointed by (px, py) */
   top_objects_list = evas_tree_objects_at_xy_get(evas, NULL, px, py);
   /* We walk on this list from the last because if the list contains more than one
    * element, all but the last will repeat events. The last one can repeat events
    * or not. Anyway, this last one is the first that has to be taken into account
    * for the determination of the drop target.
    */
   EINA_LIST_REVERSE_FOREACH(top_objects_list, itr, top_obj)
     {
        Evas_Object *object = top_obj;
        /* We search for the dropable data into the object. If not found, we search into its parent.
         * For example, if a button is a drop target, the first object will be an (internal) image.
         * The drop target is attached to the button, i.e to image's parent. That's why we need to
         * walk on the parents until NULL.
         * If we find this dropable data, we found our drop target.
         */
        while (object)
          {
             eo_do(object, eo_base_data_get("__elm_dropable", (void **)&dropable));
             if (dropable)
               {
                  Eina_Bool exist = EINA_FALSE;
                  Eina_List *l;
                  Dropable *d = NULL;
                  EINA_LIST_FOREACH(dropable_list, l, d)
                    {
                       if (d == dropable)
                         {
                            exist = EINA_TRUE;
                            break;
                         }
                    }
                  if (!exist)
                    dropable_list = eina_list_append(dropable_list, dropable);
                  object = evas_object_smart_parent_get(object);
                  if (dropable)
                    cnp_debug("Drop target %p of type %s found\n",
                              dropable->obj, eo_class_name_get(eo_class_get(dropable->obj)));
               }
             else
                object = evas_object_smart_parent_get(object);
          }
     }
   eina_list_free(top_objects_list);
   return dropable_list;
}

static void
_x11_dropable_coords_adjust(Dropable *dropable, Evas_Coord *x, Evas_Coord *y)
{
   Ecore_Evas *ee;
   int ex = 0, ey = 0, ew = 0, eh = 0;
   Evas_Object *win;
   Evas_Coord x2, y2;

   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(dropable->obj));
   ecore_evas_geometry_get(ee, &ex, &ey, &ew, &eh);
   *x = *x - ex;
   *y = *y - ey;

   if (elm_widget_is(dropable->obj))
     {
        win = elm_widget_top_get(dropable->obj);
        if (win && !strcmp(evas_object_type_get(win), "elm_win"))
          {
             int rot = elm_win_rotation_get(win);
             switch (rot)
               {
                case 90:
                   x2 = ew - *y;
                   y2 = *x;
                   break;
                case 180:
                   x2 = ew - *x;
                   y2 = eh - *y;
                   break;
                case 270:
                   x2 = *y;
                   y2 = eh - *x;
                   break;
                default:
                   x2 = *x;
                   y2 = *y;
                   break;
               }
             cnp_debug("rotation %d, w %d, h %d - x:%d->%d, y:%d->%d\n",
                       rot, ew, eh, *x, x2, *y, y2);
             *x = x2;
             *y = y2;
          }
     }
}

static Eina_Bool
_x11_dnd_enter(void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev)
{
   Ecore_X_Event_Xdnd_Enter *enter = ev;
   int i;
   Dropable *dropable;

   if (!enter) return EINA_TRUE;
   dropable = _x11_dropable_find(enter->win);
   if (dropable)
     {
        cnp_debug("Enter %x\n", enter->win);
     }
   /* Skip it */
   cnp_debug("enter types=%p (%d)\n", enter->types, enter->num_types);
   if ((!enter->num_types) || (!enter->types)) return EINA_TRUE;

   cnp_debug("Types\n");
   savedtypes.ntypes = enter->num_types;
   free(savedtypes.types);
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
             ELM_SAFE_FREE(savedtypes.imgfile, free);
             ecore_x_selection_xdnd_request(enter->win, text_uri);
          }
     }

   /* FIXME: Find an object and make it current */
   return EINA_TRUE;
}

static void
_x11_dnd_dropable_handle(Dropable *dropable, Evas_Coord x, Evas_Coord y, Elm_Xdnd_Action action)
{
   Dropable *d, *last_dropable = NULL;
   Eina_List *l;
   Dropable_Cbs *cbs;
   Eina_Inlist *itr;

   EINA_LIST_FOREACH(drops, l, d)
     {
        if (d->last.in)
          {
             last_dropable = d;
             break;
          }
     }
   if (last_dropable)
     {
        if (last_dropable == dropable) // same
          {
             cnp_debug("same obj dropable %p\n", dropable->obj);
             EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                if (cbs->poscb)
                  cbs->poscb(cbs->posdata, dropable->obj, x, y, action);
          }
        else
          {
             if (dropable) // leave last obj and enter new one
               {
                  cnp_debug("leave %p\n", last_dropable->obj);
                  cnp_debug("enter %p\n", dropable->obj);
                  last_dropable->last.in = EINA_FALSE;
                  last_dropable->last.type = NULL;
                  dropable->last.in = EINA_TRUE;
                  EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                     if (cbs->entercb)
                       cbs->entercb(cbs->enterdata, dropable->obj);
                  EINA_INLIST_FOREACH_SAFE(last_dropable->cbs_list, itr, cbs)
                     if (cbs->leavecb)
                       cbs->leavecb(cbs->leavedata, last_dropable->obj);
               }
             else // leave last obj
               {
                  cnp_debug("leave %p\n", last_dropable->obj);
                  last_dropable->last.in = EINA_FALSE;
                  last_dropable->last.type = NULL;
                  EINA_INLIST_FOREACH_SAFE(last_dropable->cbs_list, itr, cbs)
                     if (cbs->leavecb)
                       cbs->leavecb(cbs->leavedata, last_dropable->obj);
               }
          }
     }
   else
     {
        if (dropable) // enter new obj
          {
             cnp_debug("enter %p\n", dropable->obj);
             dropable->last.in = EINA_TRUE;
             EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
               {
                if (cbs->entercb)
                  cbs->entercb(cbs->enterdata, dropable->obj);
                if (cbs->poscb)
                  cbs->poscb(cbs->posdata, dropable->obj, x, y, action);
               }
          }
        else
          {
             cnp_debug("both dropable & last_dropable are null\n");
          }
     }
}

static Elm_Xdnd_Action
_x11_dnd_action_map(Ecore_X_Atom action)
{
   Elm_Xdnd_Action act = ELM_XDND_ACTION_UNKNOWN;

   if (action == ECORE_X_ATOM_XDND_ACTION_COPY)
     act = ELM_XDND_ACTION_COPY;
   else if (action == ECORE_X_ATOM_XDND_ACTION_MOVE)
     act = ELM_XDND_ACTION_MOVE;
   else if (action == ECORE_X_ATOM_XDND_ACTION_PRIVATE)
     act = ELM_XDND_ACTION_PRIVATE;
   else if (action == ECORE_X_ATOM_XDND_ACTION_ASK)
     act = ELM_XDND_ACTION_ASK;
   else if (action == ECORE_X_ATOM_XDND_ACTION_LIST)
     act = ELM_XDND_ACTION_LIST;
   else if (action == ECORE_X_ATOM_XDND_ACTION_LINK)
     act = ELM_XDND_ACTION_LINK;
   else if (action == ECORE_X_ATOM_XDND_ACTION_DESCRIPTION)
     act = ELM_XDND_ACTION_DESCRIPTION;
   return act;
}

static Ecore_X_Atom
_x11_dnd_action_rev_map(Elm_Xdnd_Action action)
{
   Ecore_X_Atom act = ECORE_X_ATOM_XDND_ACTION_MOVE;

   if (action == ELM_XDND_ACTION_COPY)
     act = ECORE_X_ATOM_XDND_ACTION_COPY;
   else if (action == ELM_XDND_ACTION_MOVE)
     act = ECORE_X_ATOM_XDND_ACTION_MOVE;
   else if (action == ELM_XDND_ACTION_PRIVATE)
     act = ECORE_X_ATOM_XDND_ACTION_PRIVATE;
   else if (action == ELM_XDND_ACTION_ASK)
     act = ECORE_X_ATOM_XDND_ACTION_ASK;
   else if (action == ELM_XDND_ACTION_LIST)
     act = ECORE_X_ATOM_XDND_ACTION_LIST;
   else if (action == ELM_XDND_ACTION_LINK)
     act = ECORE_X_ATOM_XDND_ACTION_LINK;
   else if (action == ELM_XDND_ACTION_DESCRIPTION)
     act = ECORE_X_ATOM_XDND_ACTION_DESCRIPTION;
   return act;
}

static int
_x11_dnd_types_get(Elm_Sel_Format format, const char **types)
{
   int i;
   int types_no = 0;
   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        if (_x11_atoms[i].formats == ELM_SEL_FORMAT_TARGETS)
          {
             if (format == ELM_SEL_FORMAT_TARGETS)
               if (types)
                 types[types_no++] = _x11_atoms[i].name;
          }
        else if (_x11_atoms[i].formats & format)
          {
             if (types)
               types[types_no++] = _x11_atoms[i].name;
          }
     }
   return types_no;
}

static Eina_Bool
_x11_dnd_position(void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev)
{
   Ecore_X_Event_Xdnd_Position *pos = ev;
   Ecore_X_Rectangle rect = { 0, 0, 0, 0 };
   Dropable *dropable;
   Elm_Xdnd_Action act;

   /* Need to send a status back */
   /* FIXME: Should check I can drop here */
   /* FIXME: Should highlight widget */
   dropable = _x11_dropable_find(pos->win);
   if (dropable)
     {
        Eina_List *dropable_list;
        Evas_Coord x, y, ox = 0, oy = 0;

        act = _x11_dnd_action_map(pos->action);
        x = pos->position.x;
        y = pos->position.y;
        _x11_dropable_coords_adjust(dropable, &x, &y);
        dropable_list = _x11_dropable_list_geom_find(pos->win, x, y);
        /* check if there is dropable (obj) can accept this drop */
        if (dropable_list)
          {
             Eina_List *l;
             Eina_Bool found = EINA_FALSE;
             int i, j;

             EINA_LIST_FOREACH(dropable_list, l, dropable)
               {
                  Dropable_Cbs *cbs;
                  Eina_Inlist *itr;
                  EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                    {
                       int types_no;
                       const char *types[CNP_N_ATOMS];
                       types_no = _x11_dnd_types_get(cbs->types, types);
                       for (j = 0; j < types_no; j++)
                         {
                            for (i = 0; i < savedtypes.ntypes; i++)
                              {
                                 if (!strcmp(types[j], savedtypes.types[i]))
                                   {
                                      found = EINA_TRUE;
                                      dropable->last.type = savedtypes.types[i];
                                      dropable->last.format = cbs->types;
                                      break;
                                   }
                              }
                            if (found) break;
                         }
                       if (found) break;
                    }
                  if (found) break;
               }
             if (found)
               {
                  Dropable *d = NULL;
                  Eina_Rectangle inter_rect = {0, 0, 0, 0};
                  int idx = 0;
                  EINA_LIST_FOREACH(dropable_list, l, d)
                    {
                       if (idx == 0)
                         {
                            evas_object_geometry_get(d->obj, &inter_rect.x, &inter_rect.y,
                                                     &inter_rect.w, &inter_rect.h);
                         }
                       else
                         {
                            Eina_Rectangle cur_rect;
                            evas_object_geometry_get(d->obj, &cur_rect.x, &cur_rect.y,
                                                     &cur_rect.w, &cur_rect.h);
                            if (!eina_rectangle_intersection(&inter_rect, &cur_rect)) continue;
                         }
                       idx++;
                    }
                  rect.x = inter_rect.x;
                  rect.y = inter_rect.y;
                  rect.width = inter_rect.w;
                  rect.height = inter_rect.h;
                  ecore_x_dnd_send_status(EINA_TRUE, EINA_FALSE, rect, pos->action);
                  cnp_debug("dnd position %i %i %p\n", x - ox, y - oy, dropable);
                  _x11_dnd_dropable_handle(dropable, x - ox, y - oy, act);
                  // CCCCCCC: call dnd exit on last obj if obj != last
                  // CCCCCCC: call drop position on obj
               }
             else
               {
                  //if not: send false status
                  ecore_x_dnd_send_status(EINA_FALSE, EINA_FALSE, rect, pos->action);
                  cnp_debug("dnd position (%d, %d) not in obj\n", x, y);
                  _x11_dnd_dropable_handle(NULL, 0, 0, act);
                  // CCCCCCC: call dnd exit on last obj
               }
          }
        else
          {
             ecore_x_dnd_send_status(EINA_FALSE, EINA_FALSE, rect, pos->action);
             cnp_debug("dnd position (%d, %d) has no drop\n", x, y);
             _x11_dnd_dropable_handle(NULL, 0, 0, act);
          }
     }
   else
     {
        ecore_x_dnd_send_status(EINA_FALSE, EINA_FALSE, rect, pos->action);
     }
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_leave(void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev)
{
#ifdef DEBUGON
   cnp_debug("Leave %x\n", ((Ecore_X_Event_Xdnd_Leave *)ev)->win);
#else
   (void)ev;
#endif
   _x11_dnd_dropable_handle(NULL, 0, 0, ELM_XDND_ACTION_UNKNOWN);
   // CCCCCCC: call dnd exit on last obj if there was one
   // leave->win leave->source
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_drop(void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev)
{
   Ecore_X_Event_Xdnd_Drop *drop;
   Dropable *dropable = NULL;
   Elm_Selection_Data ddata;
   Evas_Coord x = 0, y = 0;
   Elm_Xdnd_Action act = ELM_XDND_ACTION_UNKNOWN;
   Eina_List *l;
   Dropable_Cbs *cbs;
   Eina_Inlist *itr;

   drop = ev;

   cnp_debug("drops %p (%d)\n", drops, eina_list_count(drops));
   if (!(dropable = _x11_dropable_find(drop->win))) return EINA_TRUE;

   /* Calculate real (widget relative) position */
   // - window position
   // - widget position
   savedtypes.x = drop->position.x;
   savedtypes.y = drop->position.y;
   _x11_dropable_coords_adjust(dropable, &savedtypes.x, &savedtypes.y);

   cnp_debug("Drop position is %d,%d\n", savedtypes.x, savedtypes.y);

   EINA_LIST_FOREACH(drops, l, dropable)
     {
        if (dropable->last.in)
          {
             evas_object_geometry_get(dropable->obj, &x, &y, NULL, NULL);
             savedtypes.x -= x;
             savedtypes.y -= y;
             goto found;
          }
     }

   cnp_debug("Didn't find a target\n");
   return EINA_TRUE;

found:
   cnp_debug("0x%x\n", drop->win);

   act = _x11_dnd_action_map(drop->action);

   if ((!strcmp(dropable->last.type, text_uri)))
     {
        cnp_debug("We found a URI... (%scached) %s\n",
                  savedtypes.imgfile ? "" : "not ",
                  savedtypes.imgfile);
        if (savedtypes.imgfile)
          {
             char *entrytag;
             static const char *tagstring =
               "<item absize=240x180 href=file://%s></item>";
             ddata.x = savedtypes.x;
             ddata.y = savedtypes.y;
             ddata.action = act;

             EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
               {
                  /* If it's markup that also supports images */
                  if ((cbs->types & ELM_SEL_FORMAT_MARKUP) &&
                      (cbs->types & ELM_SEL_FORMAT_IMAGE))
                    {
                       int len;
                       ddata.format = ELM_SEL_FORMAT_MARKUP;

                       len = strlen(tagstring) + strlen(savedtypes.imgfile);
                       entrytag = alloca(len + 1);
                       snprintf(entrytag, len + 1, tagstring, savedtypes.imgfile);
                       ddata.data = entrytag;
                       cnp_debug("Insert %s\n", (char *)ddata.data);
                       if (cbs->dropcb)
                         cbs->dropcb(cbs->dropdata, dropable->obj, &ddata);
                    }
                  else if (cbs->types & ELM_SEL_FORMAT_IMAGE)
                    {
                       cnp_debug("Doing image insert (%s)\n", savedtypes.imgfile);
                       ddata.format = ELM_SEL_FORMAT_IMAGE;
                       ddata.data = (char *)savedtypes.imgfile;
                       if (cbs->dropcb)
                         cbs->dropcb(cbs->dropdata, dropable->obj, &ddata);
                    }
                  else
                    {
                       cnp_debug("Item doesn't support images... passing\n");
                    }
               }
             ecore_x_dnd_send_finished();
             ELM_SAFE_FREE(savedtypes.imgfile, free);
             return EINA_TRUE;
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

   if (dropable->last.type)
     {
        cnp_debug("doing a request then: %s\n", dropable->last.type);
        _x11_selections[ELM_SEL_TYPE_XDND].xwin = drop->win;
        _x11_selections[ELM_SEL_TYPE_XDND].requestwidget = dropable->obj;
        _x11_selections[ELM_SEL_TYPE_XDND].requestformat = dropable->last.format;
        _x11_selections[ELM_SEL_TYPE_XDND].active = EINA_TRUE;
        _x11_selections[ELM_SEL_TYPE_XDND].action = act;

        ecore_x_selection_xdnd_request(drop->win, dropable->last.type);
     }
   else
     {
        cnp_debug("cannot match format\n");
     }
   return EINA_TRUE;
}

static Eina_Bool
_x11_dnd_status(void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev)
{
   Ecore_X_Event_Xdnd_Status *status = ev;
   doaccept = EINA_FALSE;

   /* Only thing we care about: will accept */
   if ((status) && (status->will_accept))
     {
        cnp_debug("Will accept\n");
        doaccept = EINA_TRUE;
     }
   /* Won't accept */
   else
     {
        cnp_debug("Won't accept accept\n");
     }
   if (dragacceptcb)
     dragacceptcb(dragacceptdata, _x11_selections[ELM_SEL_TYPE_XDND].widget,
                  doaccept);
   return EINA_TRUE;
}

static Eina_Bool
_x11_drag_mouse_up(void *data, int etype EINA_UNUSED, void *event)
{
   Ecore_X_Window xwin = (Ecore_X_Window)(long)data;
   Ecore_Event_Mouse_Button *ev = event;

   if ((ev->buttons == 1) &&
       (ev->event_window == xwin))
     {
        Eina_Bool have_drops = EINA_FALSE;
        Eina_List *l;
        Dropable *dropable;

        ecore_x_pointer_ungrab();
        ELM_SAFE_FREE(handler_up, ecore_event_handler_del);
        ELM_SAFE_FREE(handler_status, ecore_event_handler_del);
        ecore_x_dnd_self_drop();

        cnp_debug("mouse up, xwin=%#llx\n", (unsigned long long)xwin);

        EINA_LIST_FOREACH(drops, l, dropable)
          {
             if (xwin == _x11_elm_widget_xwin_get(dropable->obj))
               {
                  have_drops = EINA_TRUE;
                  break;
               }
          }
        if (!have_drops) ecore_x_dnd_aware_set(xwin, EINA_FALSE);
        if (dragdonecb) dragdonecb(dragdonedata, dragwidget);
        if (dragwin)
          {
             if (!doaccept)
               {  /* Commit animation when drag cancelled */
                  /* Record final position of dragwin, then do animation */
                  ecore_animator_timeline_add(0.3,
                        _drag_cancel_animate, dragwin);
               }
             else
               {  /* No animation drop was committed */
                  Ecore_X_Window xdragwin = _x11_elm_widget_xwin_get(dragwin);
                  ecore_x_window_ignore_set(xdragwin, 0);
                  evas_object_del(dragwin);
               }

             dragwin = NULL;  /* if not freed here, free in end of anim */
          }

        dragdonecb = NULL;
        dragacceptcb = NULL;
        dragposcb = NULL;
        dragwidget = NULL;
        doaccept = EINA_FALSE;
        /*  moved to _drag_cancel_animate
        if (dragwin)
          {
             evas_object_del(dragwin);
             dragwin = NULL;
          }
          */
     }
   return EINA_TRUE;
}

static void
_x11_drag_move(void *data EINA_UNUSED, Ecore_X_Xdnd_Position *pos)
{
   evas_object_move(dragwin,
                    pos->position.x - _dragx, pos->position.y - _dragy);
   dragwin_x_end = pos->position.x - _dragx;
   dragwin_y_end = pos->position.y - _dragy;
   cnp_debug("dragevas: %p -> %p\n",
          dragwidget,
          evas_object_evas_get(dragwidget));
   if (dragposcb)
     dragposcb(dragposdata, dragwidget, pos->position.x, pos->position.y,
               dragaction);
}

static Ecore_X_Window
_x11_elm_widget_xwin_get(const Evas_Object *obj)
{
   Evas_Object *top, *par;
   Ecore_X_Window xwin = 0;

   if (elm_widget_is(obj))
     {
        top = elm_widget_top_get(obj);
        if (!top)
          {
             par = elm_widget_parent_widget_get(obj);
             if (par) top = elm_widget_top_get(par);
          }
        if (top && (eo_isa(top, ELM_OBJ_WIN_CLASS)))
            xwin = elm_win_xwindow_get(top);
     }
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
   //XXX delete handlers?
   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR, _x11_selection_clear, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, _x11_selection_notify, NULL);
   return EINA_TRUE;
}

static Eina_Bool
_x11_elm_cnp_selection_set(Ecore_X_Window xwin, Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, const void *selbuf, size_t buflen)
{
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
   if (sel->set) sel->set(xwin, &selection, sizeof(Elm_Sel_Type));
   sel->format = format;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;

   evas_object_event_callback_add
     (sel->widget, EVAS_CALLBACK_DEL, _x11_sel_obj_del, sel);

   ELM_SAFE_FREE(sel->selbuf, free);
   if (selbuf)
     {
        if (format == ELM_SEL_FORMAT_IMAGE)
          {
             // selbuf is actual image data, not text/string
             sel->selbuf = malloc(buflen + 1);
             if (!sel->selbuf)
               {
                  elm_object_cnp_selection_clear(obj, selection);
                  return EINA_FALSE;
               }
             memcpy(sel->selbuf, selbuf, buflen);
             sel->selbuf[buflen] = 0;
          }
        else
          sel->selbuf = strdup((char*)selbuf);
     }

   return EINA_TRUE;
}

static void
_x11_elm_cnp_selection_loss_callback_set(Evas_Object *obj EINA_UNUSED, Elm_Sel_Type selection, Elm_Selection_Loss_Cb func, const void *data)
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
   ELM_SAFE_FREE(sel->selbuf, free);
   sel->clear();

   return EINA_TRUE;
}

static Eina_Bool
_x11_elm_cnp_selection_get(Ecore_X_Window xwin, Evas_Object *obj, Elm_Sel_Type selection,
                           Elm_Sel_Format format, Elm_Drop_Cb datacb,
                           void *udata)
{
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
_x11_elm_drop_target_add(Evas_Object *obj, Elm_Sel_Format format,
                         Elm_Drag_State entercb, void *enterdata,
                         Elm_Drag_State leavecb, void *leavedata,
                         Elm_Drag_Pos poscb, void *posdata,
                         Elm_Drop_Cb dropcb, void *dropdata)
{
   Dropable *dropable = NULL;
   Dropable_Cbs *cbs = NULL;
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   Eina_List *l;
   Eina_Bool first = !drops;
   Eina_Bool have_drops = EINA_FALSE;

   _x11_elm_cnp_init();

   /* Is this the first? */
   EINA_LIST_FOREACH(drops, l, dropable)
     {
        if (xwin == _x11_elm_widget_xwin_get(dropable->obj))
          {
             have_drops = EINA_TRUE;
             break;
          }
     }
   dropable = NULL; // In case of error, we don't want to free it

   cbs = calloc(1, sizeof(*cbs));
   if (!cbs) return EINA_FALSE;

   cbs->entercb = entercb;
   cbs->enterdata = enterdata;
   cbs->leavecb = leavecb;
   cbs->leavedata = leavedata;
   cbs->poscb = poscb;
   cbs->posdata = posdata;
   cbs->dropcb = dropcb;
   cbs->dropdata = dropdata;
   cbs->types = format;

   eo_do(obj, eo_base_data_get("__elm_dropable", (void **)&dropable));
   if (!dropable)
     {
        /* Create new drop */
        dropable = calloc(1, sizeof(Dropable));
        if (!dropable) goto error;
        dropable->last.in = EINA_FALSE;
        drops = eina_list_append(drops, dropable);
        if (!drops) goto error;
        dropable->obj = obj;
        eo_do(obj, eo_base_data_set("__elm_dropable", dropable, NULL));
     }
   dropable->cbs_list = eina_inlist_append(dropable->cbs_list, EINA_INLIST_GET(cbs));

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
         _all_drop_targets_cbs_del, obj);
   if (!have_drops) ecore_x_dnd_aware_set(xwin, EINA_TRUE);

   /* TODO BUG: should handle dnd-aware per window, not just the first
    * window that requested it! */
   /* If not the first: We're done */
   if (!first) return EINA_TRUE;

   cnp_debug("Adding drop target calls xwin=%#llx\n", (unsigned long long)xwin);
   handler_enter = ecore_event_handler_add(ECORE_X_EVENT_XDND_ENTER,
                                           _x11_dnd_enter, NULL);
   handler_leave = ecore_event_handler_add(ECORE_X_EVENT_XDND_LEAVE,
                                           _x11_dnd_leave, NULL);
   handler_pos = ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION,
                                         _x11_dnd_position, NULL);
   handler_drop = ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP,
                                          _x11_dnd_drop, NULL);
   return EINA_TRUE;
error:
   free(cbs);
   free(dropable);
   return EINA_FALSE;
}

static Eina_Bool
_x11_elm_drop_target_del(Evas_Object *obj, Elm_Sel_Format format,
                         Elm_Drag_State entercb, void *enterdata,
                         Elm_Drag_State leavecb, void *leavedata,
                         Elm_Drag_Pos poscb, void *posdata,
                         Elm_Drop_Cb dropcb, void *dropdata)
{
   Dropable *dropable;
   Eina_List *l;
   Ecore_X_Window xwin;
   Eina_Bool have_drops = EINA_FALSE;

   _x11_elm_cnp_init();

   eo_do(obj, eo_base_data_get("__elm_dropable", (void **)&dropable));
   if (dropable)
     {
        Eina_Inlist *itr;
        Dropable_Cbs *cbs_info;
        /* Look for the callback in the list */
        EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs_info)
           if (cbs_info->entercb == entercb && cbs_info->enterdata == enterdata &&
                 cbs_info->leavecb == leavecb && cbs_info->leavedata == leavedata &&
                 cbs_info->poscb == poscb && cbs_info->posdata == posdata &&
                 cbs_info->dropcb == dropcb && cbs_info->dropdata == dropdata &&
                 cbs_info->types == format)
             {
                dropable->cbs_list = eina_inlist_remove(dropable->cbs_list,
                      EINA_INLIST_GET(cbs_info));
                free(cbs_info);
             }
        /* In case no more callbacks are listed for the object */
        if (!dropable->cbs_list)
          {
             drops = eina_list_remove(drops, dropable);
             eo_do(obj, eo_base_data_del("__elm_dropable"));
             free(dropable);
             dropable = NULL;
             evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
                   _all_drop_targets_cbs_del);
          }
     }
   else return EINA_FALSE;

   /* TODO BUG: we should handle dnd-aware per window, not just the last that released it */

   /* If still drops there: All fine.. continue */
   if (drops) return EINA_TRUE;

   cnp_debug("Disabling DND\n");
   xwin = _x11_elm_widget_xwin_get(obj);
   EINA_LIST_FOREACH(drops, l, dropable)
     {
        if (xwin == _x11_elm_widget_xwin_get(dropable->obj))
          {
             have_drops = EINA_TRUE;
             break;
          }
     }
   if (!have_drops) ecore_x_dnd_aware_set(xwin, EINA_FALSE);

   if (!drops)
     {
        ELM_SAFE_FREE(handler_pos, ecore_event_handler_del);
        ELM_SAFE_FREE(handler_drop, ecore_event_handler_del);
        ELM_SAFE_FREE(handler_enter, ecore_event_handler_del);
        ELM_SAFE_FREE(handler_leave, ecore_event_handler_del);
     }

   ELM_SAFE_FREE(savedtypes.imgfile, free);

   return EINA_TRUE;
}

static void
_x11_drag_target_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *info EINA_UNUSED)
{
   X11_Cnp_Selection *sel = _x11_selections + ELM_SEL_TYPE_XDND;

   if (dragwidget == obj)
     {
        sel->widget = NULL;
        dragwidget = NULL;
     }
}

static  Eina_Bool
_x11_elm_drag_start(Evas_Object *obj, Elm_Sel_Format format, const char *data,
                    Elm_Xdnd_Action action,
                    Elm_Drag_Icon_Create_Cb createicon, void *createdata,
                    Elm_Drag_Pos dragpos, void *dragdata,
                    Elm_Drag_Accept acceptcb, void *acceptdata,
                    Elm_Drag_State dragdone, void *donecbdata)
{
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   Ecore_X_Window xdragwin;
   X11_Cnp_Selection *sel;
   Elm_Sel_Type xdnd = ELM_SEL_TYPE_XDND;
   Ecore_Evas *ee;
   int x, y, x2 = 0, y2 = 0, x3, y3;
   Evas_Object *icon = NULL;
   int w = 0, h = 0;
   Ecore_X_Atom actx;
   int i;

   _x11_elm_cnp_init();

   cnp_debug("starting drag... %p\n", obj);

   if (dragwin)
     {
        cnp_debug("another obj is dragging...\n");
        return EINA_FALSE;
     }

   ecore_x_dnd_types_set(xwin, NULL, 0);
   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        if (_x11_atoms[i].formats == ELM_SEL_FORMAT_TARGETS)
          {
             if (format == ELM_SEL_FORMAT_TARGETS)
               {
                  ecore_x_dnd_type_set(xwin, _x11_atoms[i].name, EINA_TRUE);
                  cnp_debug("set dnd type: %s\n", _x11_atoms[i].name);
               }
          }
        else if (_x11_atoms[i].formats & format)
          {
             ecore_x_dnd_type_set(xwin, _x11_atoms[i].name, EINA_TRUE);
             cnp_debug("set dnd type: %s\n", _x11_atoms[i].name);
          }
     }

   sel = _x11_selections + ELM_SEL_TYPE_XDND;
   sel->active = EINA_TRUE;
   sel->widget = obj;
   sel->format = format;
   sel->selbuf = data ? strdup(data) : NULL;
   sel->action = action;
   dragwidget = obj;
   dragaction = action;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _x11_drag_target_del, obj);
   /* TODO BUG: should NEVER have these as globals! They should be per context (window). */
   dragposcb = dragpos;
   dragposdata = dragdata;
   dragacceptcb = acceptcb;
   dragacceptdata = acceptdata;
   dragdonecb = dragdone;
   dragdonedata = donecbdata;
   /* TODO BUG: should increase dnd-awareness, in case it's drop target as well. See _x11_drag_mouse_up() */
   ecore_x_dnd_aware_set(xwin, EINA_TRUE);
   ecore_x_dnd_callback_pos_update_set(_x11_drag_move, NULL);
   ecore_x_dnd_self_begin(xwin, (unsigned char *)&xdnd, sizeof(Elm_Sel_Type));
   actx = _x11_dnd_action_rev_map(dragaction);
   ecore_x_dnd_source_action_set(actx);
   ecore_x_pointer_grab(xwin);
   handler_up = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP,
                                        _x11_drag_mouse_up,
                                        (void *)(long)xwin);
   handler_status = ecore_event_handler_add(ECORE_X_EVENT_XDND_STATUS,
                                            _x11_dnd_status, NULL);
   dragwin = elm_win_add(NULL, "Elm-Drag", ELM_WIN_UTILITY);
   elm_win_alpha_set(dragwin, EINA_TRUE);
   elm_win_override_set(dragwin, EINA_TRUE);
   xdragwin = _x11_elm_widget_xwin_get(dragwin);
   ecore_x_window_ignore_set(xdragwin, 1);

   /* dragwin has to be rotated as the main window is */
   if (elm_widget_is(obj))
     {
        Evas_Object *win = elm_widget_top_get(obj);
        if (win && eo_isa(win, ELM_OBJ_WIN_CLASS))
          elm_win_rotation_set(dragwin, elm_win_rotation_get(win));
     }

   if (createicon)
     {
        Evas_Coord xoff = 0, yoff = 0;

        icon = createicon(createdata, dragwin, &xoff, &yoff);
        if (icon)
          {
             x2 = xoff;
             y2 = yoff;
             evas_object_geometry_get(icon, NULL, NULL, &w, &h);
          }
     }
   else
     {
        icon = elm_icon_add(dragwin);
        evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
        // need to resize
     }
   elm_win_resize_object_add(dragwin, icon);

   /* Position subwindow appropriately */
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   x += x2;
   y += y2;
   dragwin_x_start = dragwin_x_end = x;
   dragwin_y_start = dragwin_y_end = y;
   evas_object_resize(dragwin, w, h);

   evas_object_show(icon);
   evas_object_show(dragwin);
   evas_object_move(dragwin, x, y);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x3, &y3);
   _dragx = x3 - x2;
   _dragy = y3 - y2;

   return EINA_TRUE;
}

static Eina_Bool
_x11_elm_drag_action_set(Evas_Object *obj, Elm_Xdnd_Action action)
{
   Ecore_X_Atom actx;

   _x11_elm_cnp_init();
   if (!dragwin) return EINA_FALSE;

   if (dragwidget != obj) return EINA_FALSE;
   if (dragaction == action) return EINA_TRUE;
   dragaction = action;
   actx = _x11_dnd_action_rev_map(dragaction);
   ecore_x_dnd_source_action_set(actx);
   return EINA_TRUE;
}

static Eina_Bool
_x11_elm_selection_selection_has_owner(Evas_Object *obj EINA_UNUSED)
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

   void *udata;
   Elm_Sel_Type seltype;
   Elm_Sel_Format requestformat;
   Elm_Drop_Cb datacb;

   Elm_Selection_Loss_Cb loss_cb;
   void *loss_data;

   Elm_Sel_Format format;
   Ecore_Wl_Window *win;
   Elm_Xdnd_Action action;

   Eina_Bool active : 1;
};

static Eina_Bool _wl_elm_cnp_init(void);

static Wl_Cnp_Selection wl_cnp_selection =
{
   0, 0, NULL, NULL,
   NULL, 0, 0, NULL, NULL, NULL,
   0, NULL, 0, EINA_FALSE
};

static void _wl_sel_obj_del2(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED);
static Eina_Bool _wl_elm_cnp_selection_set(Evas_Object *obj EINA_UNUSED, Elm_Sel_Type selection, Elm_Sel_Format format EINA_UNUSED, const void *selbuf, size_t buflen);
static Eina_Bool _wl_elm_cnp_selection_get(Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format EINA_UNUSED, Elm_Drop_Cb datacb EINA_UNUSED, void *udata EINA_UNUSED);
static Eina_Bool _wl_elm_cnp_selection_clear(Evas_Object *obj, Elm_Sel_Type selection);

static Eina_Bool _wl_selection_send(void *udata, int type EINA_UNUSED, void *event);
static Eina_Bool _wl_selection_receive(void *udata, int type EINA_UNUSED, void *event);

static Eina_Bool _wl_elm_dnd_init(void);
static Eina_Bool _wl_elm_drop_target_add(Evas_Object *obj, Elm_Sel_Format format, Elm_Drag_State entercb, void *enterdata, Elm_Drag_State leavecb, void *leavedata, Elm_Drag_Pos poscb, void *posdata, Elm_Drop_Cb dropcb, void *dropdata);
static Eina_Bool _wl_elm_drop_target_del(Evas_Object *obj, Elm_Sel_Format format, Elm_Drag_State entercb, void *enterdata, Elm_Drag_State leavecb, void *leavedata, Elm_Drag_Pos poscb, void *posdata, Elm_Drop_Cb dropcb, void *dropdata);

static Eina_Bool _wl_elm_drag_action_set(Evas_Object *obj, Elm_Xdnd_Action action);
static Eina_Bool _wl_elm_drag_start(Evas_Object *obj, Elm_Sel_Format format, const char *data, Elm_Xdnd_Action action, Elm_Drag_Icon_Create_Cb createicon, void *createdata, Elm_Drag_Pos dragpos, void *dragdata, Elm_Drag_Accept acceptcb, void *acceptdata, Elm_Drag_State dragdone, void *donecbdata);

static void _wl_drag_source_del(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED);

static Eina_Bool _wl_dnd_enter(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);
static Eina_Bool _wl_dnd_leave(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);
static Eina_Bool _wl_dnd_position(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);
static Eina_Bool _wl_dnd_drop(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);
/* static Eina_Bool _wl_dnd_offer(void *data EINA_UNUSED, int type EINA_UNUSED, void *event); */

static Eina_Bool _wl_dnd_send(void *data, int type EINA_UNUSED, void *event);
static Eina_Bool _wl_dnd_receive(void *data, int type EINA_UNUSED, void *event);
static Eina_Bool _wl_dnd_end(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED);
static void _wl_dropable_data_handle(Wl_Cnp_Selection *sel, char *data);

static Dropable *_wl_dropable_find(unsigned int win);
static Dropable *_wl_dropable_find_geom(unsigned int win, Evas_Coord x, Evas_Coord y);
static void _wl_dropable_handle(Dropable *drop, Evas_Coord x, Evas_Coord y, Eina_Bool have_obj);
static void _wl_dropable_all_set(unsigned int win, Evas_Coord x, Evas_Coord y, Eina_Bool set);
static Eina_Bool _wl_drops_accept(const char *type);
static unsigned int _wl_elm_widget_window_get(Evas_Object *obj);

static void
_wl_sel_obj_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Wl_Cnp_Selection *sel = data;
   if (sel->widget == obj) sel->widget = NULL;
   if (dragwidget == obj) dragwidget = NULL;
}

static void
_wl_sel_obj_del2(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Wl_Cnp_Selection *sel = data;
   if (sel->requestwidget == obj) sel->requestwidget = NULL;
}

static Eina_Bool
_wl_elm_cnp_selection_set(Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, const void *selbuf, size_t buflen)
{
   Ecore_Wl_Window *win;
   Wl_Cnp_Selection *sel = &wl_cnp_selection;

   if ((!selbuf) && (format != ELM_SEL_FORMAT_IMAGE))
     return elm_object_cnp_selection_clear(obj, selection);

   _wl_elm_cnp_init();

   win = elm_win_wl_window_get(obj);

   if (sel->loss_cb) sel->loss_cb(sel->loss_data, selection);

   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget,
                                         EVAS_CALLBACK_DEL,
                                         _wl_sel_obj_del, &wl_cnp_selection);
   sel->widget = NULL;

   sel->active = EINA_TRUE;
   sel->seltype = selection;
   sel->widget = obj;
   sel->win = win;
   /* sel->set(win, &selection, sizeof(Elm_Sel_Type)); */
   sel->format = format;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;

   evas_object_event_callback_add
     (sel->widget, EVAS_CALLBACK_DEL, _wl_sel_obj_del,
         &wl_cnp_selection);

   if (selbuf)
     {
        if (format & ELM_SEL_FORMAT_IMAGE)
          {
             /* FIXME */
             // selbuf is actual image data, not text/string
             sel->selbuf = malloc(buflen + 1);
             if (!sel->selbuf)
               {
                  elm_object_cnp_selection_clear(obj, selection);
                  return EINA_FALSE;
               }
             memcpy(sel->selbuf, selbuf, buflen);
             sel->selbuf[buflen] = 0;
          }
        else
          {
             const char *types[10] = {0, };
             int i = -1;

             if ((format & ELM_SEL_FORMAT_MARKUP) ||
                 (format & ELM_SEL_FORMAT_TEXT))
               {
                  types[++i] = "application/x-elementary-markup";
                  types[++i] = "text/plain";
                  types[++i] = "text/plain;charset=utf-8";
               }

             if (format & ELM_SEL_FORMAT_HTML)
               {
                  types[++i] = "text/html";
                  types[++i] = "text/html;charset=utf-8";
               }

             if (i < 0) return EINA_FALSE;

             ecore_wl_dnd_selection_set(ecore_wl_input_get(), types);

             free(sel->selbuf);
             sel->buflen = buflen;
             sel->selbuf = strdup((char*)selbuf);

             return EINA_TRUE;
          }
     }
   else
     sel->selbuf = NULL;

   return EINA_FALSE;
}

static Eina_Bool
_wl_elm_cnp_selection_get(Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, Elm_Drop_Cb datacb, void *udata)
{
   Ecore_Wl_Window *win;
   Wl_Cnp_Selection *sel = &wl_cnp_selection;

   _wl_elm_cnp_init();

   win = elm_win_wl_window_get(obj);

   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget,
                                         EVAS_CALLBACK_DEL,
                                         _wl_sel_obj_del2, &wl_cnp_selection);
   sel->requestwidget = NULL;

   sel->requestformat = format;
   sel->requestwidget = obj;
   sel->win = win;
   /* sel->request(win, ECORE_X_SELECTION_TARGET_TARGETS); */
   sel->datacb = datacb;
   sel->udata = udata;

   evas_object_event_callback_add(sel->requestwidget,
                                  EVAS_CALLBACK_DEL, _wl_sel_obj_del2,
                                  &wl_cnp_selection);

   if ((selection == ELM_SEL_TYPE_CLIPBOARD) ||
       (selection == ELM_SEL_TYPE_PRIMARY) ||
       (selection == ELM_SEL_TYPE_SECONDARY))
     {
        const char *types[10] = {0, };
        int i = -1;

        if ((format & ELM_SEL_FORMAT_MARKUP) ||
            (format & ELM_SEL_FORMAT_TEXT))
          {
             types[++i] = "application/x-elementary-markup";
             types[++i] = "text/plain";
             types[++i] = "text/plain;charset=utf-8";
          }

        if (format & ELM_SEL_FORMAT_HTML)
          {
             types[++i] = "text/html";
             types[++i] = "text/html;charset=utf-8";
          }

        if (i < 0) return EINA_FALSE;

        ecore_wl_dnd_selection_get(ecore_wl_input_get(), *types);
     }

   return EINA_TRUE;
}

static void
_wl_elm_cnp_selection_loss_callback_set(Evas_Object *obj EINA_UNUSED, Elm_Sel_Type selection EINA_UNUSED, Elm_Selection_Loss_Cb func, const void *data)
{
   Wl_Cnp_Selection *sel = &wl_cnp_selection;

   _wl_elm_cnp_init();

   sel->loss_cb = func;
   sel->loss_data = (void *)data;
}

static Eina_Bool
_wl_elm_cnp_selection_clear(Evas_Object *obj, Elm_Sel_Type selection EINA_UNUSED)
{
   Wl_Cnp_Selection *sel = &wl_cnp_selection;

   _wl_elm_cnp_init();

   if ((!sel->active) || (sel->widget != obj))
     return EINA_TRUE;

   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget,
                                         EVAS_CALLBACK_DEL,
                                         _wl_sel_obj_del, &wl_cnp_selection);
   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget,
                                         EVAS_CALLBACK_DEL,
                                         _wl_sel_obj_del2, &wl_cnp_selection);

   sel->widget = NULL;
   sel->requestwidget = NULL;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;

   sel->active = EINA_FALSE;
   ELM_SAFE_FREE(sel->selbuf, free);
   sel->buflen = 0;
   /* sel->clear(); */
   ecore_wl_dnd_selection_clear(ecore_wl_input_get());

   return EINA_TRUE;
}

static Eina_Bool
_wl_selection_send(void *udata, int type EINA_UNUSED, void *event)
{
   char *buf;
   int ret, len_remained;
   int len_written = 0;
   Wl_Cnp_Selection *sel = udata;
   Ecore_Wl_Event_Data_Source_Send *ev = event;

   _wl_elm_cnp_init();

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
_wl_selection_receive(void *udata, int type EINA_UNUSED, void *event)
{
   Wl_Cnp_Selection *sel = udata;
   Ecore_Wl_Event_Selection_Data_Ready *ev = event;

   _wl_elm_cnp_init();

   if (sel->requestwidget)
     {
        if (!ev->done)
          {
             if (sel->seltype == ELM_SEL_TYPE_XDND)
               {
                  Elm_Selection_Data sdata;
                  Eina_List *l;
                  Dropable *dropable;

                  EINA_LIST_FOREACH(drops, l, dropable)
                    {
                       if (dropable->obj == sel->requestwidget) break;
                       dropable = NULL;
                    }

                  if (dropable)
                    {
                       Dropable_Cbs *cbs;

                       sdata.x = savedtypes.x;
                       sdata.y = savedtypes.y;
                       sdata.format = ELM_SEL_FORMAT_TEXT;
                       sdata.data = ev->data;
                       sdata.len = ev->len;
                       sdata.action = sel->action;

                       EINA_INLIST_FOREACH(dropable->cbs_list, cbs)
                         if (cbs->dropcb)
                           cbs->dropcb(cbs->dropdata, dropable->obj, &sdata);

                       goto end;
                    }
               }

             if (sel->datacb)
               {
                  Elm_Selection_Data sdata;

                  sdata.x = sdata.y = 0;
                  sdata.format = ELM_SEL_FORMAT_TEXT;
                  sdata.data = ev->data;
                  sdata.len = ev->len;
                  sdata.action = sel->action;
                  sel->datacb(sel->udata, sel->widget, &sdata);
               }
             else
               {
                  char *stripstr, *mkupstr;

                  stripstr = malloc(ev->len + 1);
                  if (!stripstr) goto end;
                  strncpy(stripstr, (char *)ev->data, ev->len);
                  stripstr[ev->len] = '\0';
                  mkupstr = _elm_util_text_to_mkup((const char *)stripstr);
                  /* TODO BUG: should never NEVER assume it's an elm_entry! */
                  _elm_entry_entry_paste(sel->requestwidget, mkupstr);
                  free(stripstr);
                  free(mkupstr);
               }
          }
        else
          {
             evas_object_event_callback_del_full(sel->requestwidget,
                                                 EVAS_CALLBACK_DEL,
                                                 _wl_sel_obj_del2, sel);
             sel->requestwidget = NULL;
          }
     }

end:
   if (sel->seltype == ELM_SEL_TYPE_XDND)
     {
        /* FIXME: Send Finished ?? */
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_elm_cnp_init(void)
{
   static int _init_count = 0;

   if (_init_count > 0) return EINA_TRUE;
   _init_count++;

   ecore_event_handler_add(ECORE_WL_EVENT_DATA_SOURCE_SEND,
                           _wl_selection_send, &wl_cnp_selection);
   ecore_event_handler_add(ECORE_WL_EVENT_SELECTION_DATA_READY,
                           _wl_selection_receive, &wl_cnp_selection);

   return EINA_TRUE;
}

static Eina_Bool
_wl_elm_dnd_init(void)
{
   static int _init_count = 0;

   if (_init_count > 0) return EINA_TRUE;
   _init_count++;

   text_uri = eina_stringshare_add("text/uri-list");

   ecore_event_handler_add(ECORE_WL_EVENT_DATA_SOURCE_SEND,
                           _wl_dnd_send, &wl_cnp_selection);
   ecore_event_handler_add(ECORE_WL_EVENT_SELECTION_DATA_READY,
                           _wl_dnd_receive, &wl_cnp_selection);

   ecore_event_handler_add(ECORE_WL_EVENT_DND_END,
                           _wl_dnd_end, &wl_cnp_selection);

   return EINA_TRUE;
}

static Eina_Bool
_wl_elm_drop_target_add(Evas_Object *obj, Elm_Sel_Format format, Elm_Drag_State entercb, void *enterdata, Elm_Drag_State leavecb, void *leavedata, Elm_Drag_Pos poscb, void *posdata, Elm_Drop_Cb dropcb, void *dropdata)
{
   Dropable *dropable = NULL;
   Dropable_Cbs *cbs = NULL;

   Eina_Bool first = !drops;

   cbs = calloc(1, sizeof(*cbs));
   if (!cbs) return EINA_FALSE;

   cbs->entercb = entercb;
   cbs->enterdata = enterdata;
   cbs->leavecb = leavecb;
   cbs->leavedata = leavedata;
   cbs->poscb = poscb;
   cbs->posdata = posdata;
   cbs->dropcb = dropcb;
   cbs->dropdata = dropdata;
   cbs->types = format;

   eo_do(obj, eo_base_data_get("__elm_dropable", (void **)&dropable));
   if (!dropable)
     {
        /* Create new drop */
        dropable = calloc(1, sizeof(Dropable));
        if (!dropable) goto error;
        drops = eina_list_append(drops, dropable);
        if (!drops) goto error;
        dropable->obj = obj;
        eo_do(obj, eo_base_data_set("__elm_dropable", dropable, NULL));
     }
   dropable->cbs_list = eina_inlist_append(dropable->cbs_list, EINA_INLIST_GET(cbs));

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
         _all_drop_targets_cbs_del, obj);

   if (first)
     {
        handler_enter =
          ecore_event_handler_add(ECORE_WL_EVENT_DND_ENTER,
                                  _wl_dnd_enter, NULL);
        handler_leave =
          ecore_event_handler_add(ECORE_WL_EVENT_DND_LEAVE,
                                  _wl_dnd_leave, NULL);
        handler_pos =
          ecore_event_handler_add(ECORE_WL_EVENT_DND_POSITION,
                                  _wl_dnd_position, NULL);
        handler_drop =
          ecore_event_handler_add(ECORE_WL_EVENT_DND_DROP,
                                  _wl_dnd_drop, NULL);
     }

   return EINA_TRUE;
error:
   free(cbs);
   free(dropable);
   return EINA_FALSE;
}

static Eina_Bool
_wl_elm_drop_target_del(Evas_Object *obj, Elm_Sel_Format format,
                        Elm_Drag_State entercb, void *enterdata,
                        Elm_Drag_State leavecb, void *leavedata,
                        Elm_Drag_Pos poscb, void *posdata,
                        Elm_Drop_Cb dropcb, void *dropdata)
{
   Dropable *dropable;

   eo_do(obj, eo_base_data_get("__elm_dropable", (void **)&dropable));
   if (dropable)
     {
        Eina_Inlist *itr;
        Dropable_Cbs *cbs_info;
        /* Look for the callback in the list */
        EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs_info)
           if (cbs_info->entercb == entercb && cbs_info->enterdata == enterdata &&
                 cbs_info->leavecb == leavecb && cbs_info->leavedata == leavedata &&
                 cbs_info->poscb == poscb && cbs_info->posdata == posdata &&
                 cbs_info->dropcb == dropcb && cbs_info->dropdata == dropdata &&
                 cbs_info->types == format)
             {
                dropable->cbs_list = eina_inlist_remove(dropable->cbs_list,
                      EINA_INLIST_GET(cbs_info));
                free(cbs_info);
             }
        /* In case no more callbacks are listed for the object */
        if (!dropable->cbs_list)
          {
             drops = eina_list_remove(drops, dropable);
             eo_do(obj, eo_base_data_del("__elm_dropable"));
             ELM_SAFE_FREE(dropable, free);
             evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
                   _all_drop_targets_cbs_del);
          }
     }
   else return EINA_FALSE;

   if (!drops)
     {
        ELM_SAFE_FREE(handler_pos, ecore_event_handler_del);
        ELM_SAFE_FREE(handler_drop, ecore_event_handler_del);
        ELM_SAFE_FREE(handler_enter, ecore_event_handler_del);
        ELM_SAFE_FREE(handler_leave, ecore_event_handler_del);
     }

   return EINA_TRUE;
}

static Eina_Bool
_wl_elm_drag_action_set(Evas_Object *obj, Elm_Xdnd_Action action)
{
   if (!dragwin) return EINA_FALSE;
   if (dragwidget != obj) return EINA_FALSE;
   if (dragaction == action) return EINA_TRUE;
   dragaction = action;
   return EINA_TRUE;
}

static Eina_Bool
_wl_elm_drag_start(Evas_Object *obj, Elm_Sel_Format format EINA_UNUSED, const char *data,
                   Elm_Xdnd_Action action,
                   Elm_Drag_Icon_Create_Cb createicon, void *createdata,
                   Elm_Drag_Pos dragpos, void *dragdata,
                   Elm_Drag_Accept acceptcb, void *acceptdata,
                   Elm_Drag_State dragdone, void *donecbdata)
{
   Ecore_Evas *ee;
   Evas_Object *icon = NULL;
   int x, y, x2 = 0, y2 = 0, x3, y3, w = 0, h = 0;
   const char *types[2] = { "text/uri-list", NULL };

   _wl_elm_dnd_init();

   /* if we already have a drag, get out */
   if (dragwin) return EINA_FALSE;

   ecore_wl_dnd_drag_types_set(ecore_wl_input_get(), types);

   /* set the drag data used when a drop occurs */
   free(wl_cnp_selection.selbuf);
   wl_cnp_selection.selbuf = strdup((char*)data);
   wl_cnp_selection.buflen = strlen(wl_cnp_selection.selbuf);

   /* setup callback to notify if this object gets deleted */
   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _wl_drag_source_del, obj);

   dragwidget = obj;
   dragaction = action;
   dragposcb = dragpos;
   dragposdata = dragdata;
   dragacceptcb = acceptcb;
   dragacceptdata = acceptdata;
   dragdonecb = dragdone;
   dragdonedata = donecbdata;

   dragwin = elm_win_add(NULL, "Elm-Drag", ELM_WIN_DND);
   elm_win_alpha_set(dragwin, EINA_TRUE);
   elm_win_borderless_set(dragwin, EINA_TRUE);
   elm_win_override_set(dragwin, EINA_TRUE);

   ecore_wl_window_type_set(elm_win_wl_window_get(dragwin),
                            ECORE_WL_WINDOW_TYPE_DND);

   if (createicon)
     {
        Evas_Coord xoff = 0, yoff = 0;

        icon = createicon(createdata, dragwin, &xoff, &yoff);
        if (icon)
          {
             x2 = xoff;
             y2 = yoff;
             evas_object_geometry_get(icon, NULL, NULL, &w, &h);
          }
     }
   else
     {
        icon = elm_icon_add(dragwin);
        evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND,
                                         EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
     }

   elm_win_resize_object_add(dragwin, icon);
   evas_object_show(icon);

   /* Position subwindow appropriately */
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   x += x2;
   y += y2;
   dragwin_x_start = dragwin_x_end = x;
   dragwin_y_start = dragwin_y_end = y;

   evas_object_move(dragwin, x, y);
   evas_object_resize(dragwin, w, h);
   evas_object_show(dragwin);

   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x3, &y3);
   _dragx = x3 - x2;
   _dragy = y3 - y2;

   ecore_wl_dnd_drag_start(ecore_wl_input_get(),
                           elm_win_wl_window_get(obj),
                           elm_win_wl_window_get(dragwin),
                           x3, y3, w, h);

   return EINA_TRUE;
}

static void
_wl_drag_source_del(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   if (dragwidget == obj)
     dragwidget = NULL;
}

static Eina_Bool
_wl_dnd_enter(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Wl_Event_Dnd_Enter *ev;
   int i = 0;

   ev = event;

   if (_wl_dropable_find(ev->win))
     _wl_dropable_all_set(ev->win, 0, 0, EINA_FALSE);

   if ((!ev->num_types) || (!ev->types)) return ECORE_CALLBACK_PASS_ON;

   savedtypes.ntypes = ev->num_types;
   free(savedtypes.types);
   savedtypes.types = malloc(sizeof(char *) * ev->num_types);
   if (!savedtypes.types) return EINA_FALSE;

   for (i = 0; i < ev->num_types; i++)
     {
        savedtypes.types[i] = eina_stringshare_add(ev->types[i]);
        if (savedtypes.types[i] == text_uri)
          {
             savedtypes.textreq = 1;
             ELM_SAFE_FREE(savedtypes.imgfile, free);
          }
     }

   doaccept = EINA_FALSE;
   for (i = 0; i < ev->num_types; i++)
     {
        doaccept = _wl_drops_accept(ev->types[i]);
        if (doaccept)
          wl_data_offer_accept(ev->offer, ev->serial, ev->types[i]);
        else
          wl_data_offer_accept(ev->offer, ev->serial, NULL);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_leave(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Wl_Event_Dnd_Leave *ev;
   Dropable *drop;

   ev = event;

   if ((drop = _wl_dropable_find(ev->win)))
     {
        _wl_dropable_handle(drop, 0, 0, EINA_FALSE);
        _wl_dropable_all_set(ev->win, 0, 0, EINA_FALSE);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_position(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Wl_Event_Dnd_Position *ev;
   Dropable *drop, *dropable_old;
   Eina_Bool will_accept = EINA_FALSE;

   ev = event;

   dragwin_x_end = ev->position.x - _dragx;
   dragwin_y_end = ev->position.y - _dragy;

   dropable_old = drop = _wl_dropable_find(ev->win);

   if (drop)
     {
        Ecore_Evas *ee;
        Evas_Coord ex = 0, ey = 0;
        Evas_Coord x = 0, y = 0;

        x = ev->position.x;
        y = ev->position.y;
        ee = ecore_evas_ecore_evas_get(evas_object_evas_get(drop->obj));
        ecore_evas_geometry_get(ee, &ex, &ey, NULL, NULL);
        x -= ex;
        y -= ey;

        if ((drop = _wl_dropable_find_geom(ev->win, x, y)))
          {
             Evas_Coord ox = 0, oy = 0;
             int i = 0;

             evas_object_geometry_get(drop->obj, &ox, &oy, NULL, NULL);

             _wl_dropable_handle(drop, x - ox, y - oy, EINA_TRUE);

             for (i = 0; i < savedtypes.ntypes; i++)
               {
                  Dropable_Cbs *cbs;
                  EINA_INLIST_FOREACH(drop->cbs_list, cbs)
                    {
                       switch (cbs->types)
                         {
                          case ELM_SEL_FORMAT_TARGETS:
                          case ELM_SEL_FORMAT_IMAGE:
                             if ((!strncmp(savedtypes.types[i], "text/uri", 8)) ||
                                   (!strncmp(savedtypes.types[i], "image/", 6)))
                               {
                                  wl_cnp_selection.requestwidget = drop->obj;
                                  will_accept = EINA_TRUE;
                               }
                             break;
                          default:
                             break;
                         }
                    }
                  if (will_accept) break;
               }
          }
        else
          _wl_dropable_handle(dropable_old, 0, 0, EINA_FALSE);

        _wl_dropable_all_set(ev->win, x, y, EINA_TRUE);
     }

   doaccept = will_accept;

   if (dragacceptcb)
     dragacceptcb(dragacceptdata, wl_cnp_selection.requestwidget, will_accept);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_drop(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Wl_Event_Dnd_Drop *ev;
   Dropable *drop;
   Ecore_Evas *ee;
   Evas_Coord ex = 0, ey = 0;
   int i = 0;

   ev = event;

   if (!(drop = _wl_dropable_find(ev->win)))
     return ECORE_CALLBACK_PASS_ON;

   savedtypes.x = ev->position.x;
   savedtypes.y = ev->position.y;
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(drop->obj));
   ecore_evas_geometry_get(ee, &ex, &ey, NULL, NULL);
   savedtypes.x -= ex;
   savedtypes.y -= ey;

   if (!(drop = _wl_dropable_find_geom(ev->win, savedtypes.x, savedtypes.y)))
     return ECORE_CALLBACK_PASS_ON;

   for (i = 0; i < savedtypes.ntypes; i++)
     {
        Dropable_Cbs *cbs;
        EINA_INLIST_FOREACH(drop->cbs_list, cbs)
          {
             if ((savedtypes.types[i] == text_uri) &&
                   (cbs->types & ELM_SEL_FORMAT_MARKUP) &&
                   (cbs->types & ELM_SEL_FORMAT_IMAGE))
               {
                  wl_cnp_selection.requestwidget = drop->obj;
                  evas_object_event_callback_add(wl_cnp_selection.requestwidget,
                        EVAS_CALLBACK_DEL,
                        _wl_sel_obj_del2,
                        &wl_cnp_selection);
                  ecore_wl_dnd_drag_get(ecore_wl_input_get(), text_uri);
                  return ECORE_CALLBACK_PASS_ON;
               }
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_send(void *data, int type EINA_UNUSED, void *event)
{
   char *buf;
   int ret, len_remained;
   int len_written = 0;
   Wl_Cnp_Selection *sel;
   Ecore_Wl_Event_Data_Source_Send *ev;

   ev = event;
   sel = data;

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
_wl_dnd_receive(void *data, int type EINA_UNUSED, void *event)
{
   Wl_Cnp_Selection *sel;
   Ecore_Wl_Event_Selection_Data_Ready *ev;

   ev = event;
   sel = data;

   if (sel->requestwidget)
     {
        if (!ev->done)
          _wl_dropable_data_handle(sel, ev->data);
        else
          {
             evas_object_event_callback_del_full(sel->requestwidget,
                                                 EVAS_CALLBACK_DEL,
                                                 _wl_sel_obj_del2, sel);
             sel->requestwidget = NULL;
          }
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_end(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   /* Ecore_Wl_Event_Dnd_End *ev; */

   /* ev = event; */

   if (dragdonecb) dragdonecb(dragdonedata, dragwidget);

   if (dragwin)
     {
        if (!doaccept)
          {
             /* Commit animation when drag cancelled */
             /* Record final position of dragwin, then do animation */
             ecore_animator_timeline_add(0.3, _drag_cancel_animate, dragwin);
          }
        else
          {
             /* No animation drop was committed */
             evas_object_del(dragwin);
          }

        dragwin = NULL;  /* if not freed here, free in end of anim */
     }

   dragdonecb = NULL;
   dragacceptcb = NULL;
   dragposcb = NULL;
   dragwidget = NULL;
   doaccept = EINA_FALSE;

   ecore_wl_input_ungrab(ecore_wl_input_get());

   return ECORE_CALLBACK_PASS_ON;
}

static void
_wl_dropable_data_handle(Wl_Cnp_Selection *sel, char *data)
{
   Dropable *drop;
   unsigned int win = 0;
   char *p, *s = NULL, *entrytag;
   Elm_Selection_Data sdata;
   static const char *tagstring =
     "<item absize=240x180 href=file://%s></item>";

   win = _wl_elm_widget_window_get(sel->requestwidget);
   drop = _wl_dropable_find_geom(win, savedtypes.x, savedtypes.y);

   p = data;
   if ((!strncmp(p, "file:/", 6)) || (p[0] == '/'))
     {
        int len = 0;

        len = sel->buflen;
        if (!strncmp(p, "file:/", 6))
          {
             p += 5;
             len -= 5;
          }
        if (!(s = malloc(len + 1))) return;
        memcpy(s, p, len);
        s[len] = 0;
     }

   if (savedtypes.textreq)
     {
        savedtypes.textreq = 0;
        savedtypes.imgfile = s;
     }

   sdata.x = savedtypes.x;
   sdata.y = savedtypes.y;

   if (!savedtypes.imgfile) return;

   Dropable_Cbs *cbs;
   Eina_Inlist *itr;
   EINA_INLIST_FOREACH_SAFE(drop->cbs_list, itr, cbs)
     {
        /* If it's markup that also supports images */
        if ((cbs->types & ELM_SEL_FORMAT_MARKUP) &&
              (cbs->types & ELM_SEL_FORMAT_IMAGE))
          {
             int l = 0;

             sdata.format = ELM_SEL_FORMAT_MARKUP;
             l = strlen(tagstring) + strlen(savedtypes.imgfile);
             entrytag = alloca(l + 1);
             snprintf(entrytag, l + 1, tagstring, savedtypes.imgfile);
             sdata.data = entrytag;
             if (cbs->dropcb) cbs->dropcb(cbs->dropdata, drop->obj, &sdata);
             ecore_wl_dnd_drag_end(ecore_wl_input_get());
          }
        else if (cbs->types & ELM_SEL_FORMAT_IMAGE)
          {
             sdata.format = ELM_SEL_FORMAT_IMAGE;
             sdata.data = (char *)savedtypes.imgfile;
             if (cbs->dropcb) cbs->dropcb(cbs->dropdata, drop->obj, &sdata);
             ecore_wl_dnd_drag_end(ecore_wl_input_get());
          }
     }
   ELM_SAFE_FREE(savedtypes.imgfile, free);
}

static Dropable *
_wl_dropable_find(unsigned int win)
{
   Eina_List *l;
   Dropable *dropable;

   if (!drops) return NULL;
   EINA_LIST_FOREACH(drops, l, dropable)
     if (_wl_elm_widget_window_get(dropable->obj) == win)
       return dropable;
   return NULL;
}

static Dropable *
_wl_dropable_find_geom(unsigned int win, Evas_Coord x, Evas_Coord y)
{
   Eina_List *l;
   Dropable *drop;

   EINA_LIST_FOREACH(drops, l, drop)
     {
        if (_wl_elm_widget_window_get(drop->obj) == win)
          {
             Evas_Coord ox, oy, ow, oh;

             evas_object_geometry_get(drop->obj, &ox, &oy, &ow, &oh);
             if ((x >= ox) && (y >= oy) && (x < (ox + ow)) && (y < (oy + oh)))
               return drop;
          }
     }

   return NULL;
}

static void
_wl_dropable_handle(Dropable *drop, Evas_Coord x, Evas_Coord y, Eina_Bool have_obj)
{
   Dropable *last = NULL;
   Dropable_Cbs *cbs;
   Eina_Inlist *itr;

   if (drop->last.in)
     last = _wl_dropable_find_geom(_wl_elm_widget_window_get(drop->obj),
                                   drop->last.x, drop->last.y);

   if ((have_obj) && (last == drop))
     {
        EINA_INLIST_FOREACH_SAFE(drop->cbs_list, itr, cbs)
           if (cbs->poscb)
              cbs->poscb(cbs->posdata, drop->obj, x, y, dragaction);
     }
   else if ((have_obj) && (!last))
     {
        EINA_INLIST_FOREACH_SAFE(drop->cbs_list, itr, cbs)
           if (cbs->entercb)
              cbs->entercb(cbs->enterdata, drop->obj);
        EINA_INLIST_FOREACH_SAFE(drop->cbs_list, itr, cbs)
           if (cbs->poscb)
              cbs->poscb(cbs->posdata, drop->obj, x, y, dragaction);
     }
   else if ((!have_obj) && (last))
     {
        EINA_INLIST_FOREACH_SAFE(drop->cbs_list, itr, cbs)
           if (cbs->leavecb)
              cbs->leavecb(cbs->leavedata, drop->obj);
     }
   else if (have_obj)
     {
        EINA_INLIST_FOREACH_SAFE(drop->cbs_list, itr, cbs)
           if (cbs->entercb)
              cbs->entercb(cbs->enterdata, drop->obj);
        if (last)
          {
             drop = last;
             EINA_INLIST_FOREACH_SAFE(drop->cbs_list, itr, cbs)
                if (cbs->leavecb)
                   cbs->leavecb(cbs->leavedata, drop->obj);
          }
     }
}

static void
_wl_dropable_all_set(unsigned int win, Evas_Coord x, Evas_Coord y, Eina_Bool set)
{
   Eina_List *l;
   Dropable *dropable;

   EINA_LIST_FOREACH(drops, l, dropable)
     {
        if (_wl_elm_widget_window_get(dropable->obj) == win)
          {
             dropable->last.x = x;
             dropable->last.y = y;
             dropable->last.in = set;
          }
     }
}

static Eina_Bool
_wl_drops_accept(const char *type)
{
   Eina_List *l;
   Dropable *drop;
   Eina_Bool will_accept = EINA_FALSE;

   EINA_LIST_FOREACH(drops, l, drop)
     {
        Dropable_Cbs *cbs;
        EINA_INLIST_FOREACH(drop->cbs_list, cbs)
          {
             switch (cbs->types)
               {
                case ELM_SEL_FORMAT_TARGETS:
                case ELM_SEL_FORMAT_IMAGE:
                   if ((!strncmp(type, "text/uri", 8)) ||
                         (!strncmp(type, "image/", 6)))
                     {
                        wl_cnp_selection.requestwidget = drop->obj;
                        return EINA_TRUE;
                     }
                   break;
                case ELM_SEL_FORMAT_NONE:
                   break;
                case ELM_SEL_FORMAT_TEXT:
                   break;
                case ELM_SEL_FORMAT_MARKUP:
                   break;
                case ELM_SEL_FORMAT_VCARD:
                   break;
                case ELM_SEL_FORMAT_HTML:
                   break;
               }
          }
     }

   return will_accept;
}

static unsigned int
_wl_elm_widget_window_get(Evas_Object *obj)
{
   Evas_Object *top;
   Ecore_Wl_Window *win = NULL;

   if (elm_widget_is(obj))
     {
        top = elm_widget_top_get(obj);
        if (!top) top = elm_widget_top_get(elm_widget_parent_widget_get(obj));
        if (top && (eo_isa(top, ELM_OBJ_WIN_CLASS)))
            win = elm_win_wl_window_get(top);
     }
   if (!win)
     {
        Ecore_Evas *ee;
        Evas *evas;

        if (!(evas = evas_object_evas_get(obj)))
          return 0;
        if (!(ee = ecore_evas_ecore_evas_get(evas)))
          return 0;

        win = ecore_evas_wayland_window_get(ee);
     }

   return ecore_wl_window_id_get(win);
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
      char *buf;
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
static Eina_Bool  _local_elm_cnp_selection_set(Evas_Object *obj EINA_UNUSED, Elm_Sel_Type selection, Elm_Sel_Format format, const void *selbuf, size_t buflen);
static void       _local_elm_cnp_selection_loss_callback_set(Evas_Object *obj EINA_UNUSED, Elm_Sel_Type selection EINA_UNUSED, Elm_Selection_Loss_Cb func EINA_UNUSED, const void *data EINA_UNUSED);
static Eina_Bool  _local_elm_object_cnp_selection_clear(Evas_Object *obj EINA_UNUSED, Elm_Sel_Type selection);
static Eina_Bool  _local_elm_cnp_selection_get(Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format EINA_UNUSED, Elm_Drop_Cb datacb, void *udata);
static  Eina_Bool _local_elm_drop_target_add(Evas_Object *obj EINA_UNUSED, Elm_Sel_Format format EINA_UNUSED,
                                             Elm_Drag_State entercb EINA_UNUSED, void *enterdata EINA_UNUSED,
                                             Elm_Drag_State leavecb EINA_UNUSED, void *leavedata EINA_UNUSED,
                                             Elm_Drag_Pos poscb EINA_UNUSED, void *posdata EINA_UNUSED,
                                             Elm_Drop_Cb dropcb EINA_UNUSED, void *dropdata EINA_UNUSED);
static  Eina_Bool _local_elm_drop_target_del(Evas_Object *obj EINA_UNUSED);
static Eina_Bool  _local_elm_drag_start(Evas_Object *obj EINA_UNUSED,
                                        Elm_Sel_Format format EINA_UNUSED,
                                        const char *data EINA_UNUSED,
                                        Elm_Xdnd_Action action EINA_UNUSED,
                                        Elm_Drag_Icon_Create_Cb createicon EINA_UNUSED,
                                        void *createdata EINA_UNUSED,
                                        Elm_Drag_Pos dragpos EINA_UNUSED,
                                        void *dragdata EINA_UNUSED,
                                        Elm_Drag_Accept acceptcb EINA_UNUSED,
                                        void *acceptdata EINA_UNUSED,
                                        Elm_Drag_State dragdone EINA_UNUSED,
                                        void *donecbdata EINA_UNUSED);
static Eina_Bool  _local_elm_selection_selection_has_owner(Evas_Object *obj EINA_UNUSED);

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
   ev.action = ELM_XDND_ACTION_UNKNOWN;
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
_local_elm_cnp_selection_set(Evas_Object *obj EINA_UNUSED,
                             Elm_Sel_Type selection, Elm_Sel_Format format,
                             const void *selbuf, size_t buflen)
{
   _local_elm_cnp_init();
   free(_local_selinfo[selection].sel.buf);
   _local_selinfo[selection].format = format;
   _local_selinfo[selection].sel.buf = malloc(buflen + 1);
   if (_local_selinfo[selection].sel.buf)
     {
        memcpy(_local_selinfo[selection].sel.buf, selbuf, buflen);
        _local_selinfo[selection].sel.buf[buflen] = 0;
        _local_selinfo[selection].sel.size = buflen;
     }
   else
     _local_selinfo[selection].sel.size = 0;
   return EINA_TRUE;
}

static void
_local_elm_cnp_selection_loss_callback_set(Evas_Object *obj EINA_UNUSED,
                                           Elm_Sel_Type selection EINA_UNUSED,
                                           Elm_Selection_Loss_Cb func EINA_UNUSED,
                                           const void *data EINA_UNUSED)
{
   _local_elm_cnp_init();
   // this doesnt need to do anything as we never lose selection to anyone
   // as thisis local
}

static Eina_Bool
_local_elm_object_cnp_selection_clear(Evas_Object *obj EINA_UNUSED,
                                      Elm_Sel_Type selection)
{
   _local_elm_cnp_init();
   ELM_SAFE_FREE(_local_selinfo[selection].sel.buf, free);
   _local_selinfo[selection].sel.size = 0;
   return EINA_TRUE;
}

static Eina_Bool
_local_elm_cnp_selection_get(Evas_Object *obj,
                             Elm_Sel_Type selection,
                             Elm_Sel_Format format EINA_UNUSED,
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
_local_elm_drop_target_add(Evas_Object *obj EINA_UNUSED,
                           Elm_Sel_Format format EINA_UNUSED,
                           Elm_Drag_State entercb EINA_UNUSED,
                           void *enterdata EINA_UNUSED,
                           Elm_Drag_State leavecb EINA_UNUSED,
                           void *leavedata EINA_UNUSED,
                           Elm_Drag_Pos poscb EINA_UNUSED,
                           void *posdata EINA_UNUSED,
                           Elm_Drop_Cb dropcb EINA_UNUSED,
                           void *dropdata EINA_UNUSED)
{
   // XXX: implement me
   _local_elm_cnp_init();
   return EINA_FALSE;
}

static  Eina_Bool
_local_elm_drop_target_del(Evas_Object *obj EINA_UNUSED)
{
   // XXX: implement me
   _local_elm_cnp_init();
   return EINA_FALSE;
}

static Eina_Bool
_local_elm_drag_start(Evas_Object *obj EINA_UNUSED,
                      Elm_Sel_Format format EINA_UNUSED,
                      const char *data EINA_UNUSED,
                      Elm_Xdnd_Action action EINA_UNUSED,
                      Elm_Drag_Icon_Create_Cb createicon EINA_UNUSED,
                      void *createdata EINA_UNUSED,
                      Elm_Drag_Pos dragpos EINA_UNUSED,
                      void *dragdata EINA_UNUSED,
                      Elm_Drag_Accept acceptcb EINA_UNUSED,
                      void *acceptdata EINA_UNUSED,
                      Elm_Drag_State dragdone EINA_UNUSED,
                      void *donecbdata EINA_UNUSED)
{
   // XXX: implement me
   _local_elm_cnp_init();
   return EINA_FALSE;
}

static Eina_Bool
_local_elm_drag_action_set(Evas_Object *obj EINA_UNUSED,
                           Elm_Xdnd_Action action EINA_UNUSED)
{
   // XXX: implement me
   _local_elm_cnp_init();
   return EINA_FALSE;
}

static Eina_Bool
_local_elm_selection_selection_has_owner(Evas_Object *obj EINA_UNUSED)
{
   _local_elm_cnp_init();
   if (_local_selinfo[ELM_SEL_TYPE_CLIPBOARD].sel.buf) return EINA_TRUE;
   return EINA_FALSE;
}
#endif

// common internal funcs
////////////////////////////////////////////////////////////////////////////
static Eina_Bool
_elm_cnp_init(void)
{
   if (_elm_cnp_init_count > 0) return EINA_TRUE;
   _elm_cnp_init_count++;
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
   const char *tmppath = NULL;
   mode_t cur_umask;
   int len;

   info = calloc(1, sizeof(Tmp_Info));
   if (!info) return NULL;
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     tmppath = getenv("TMP");
   if (!tmppath) tmppath = P_tmpdir;
   len = snprintf(NULL, 0, "%s/%sXXXXXX", tmppath, "elmcnpitem-");
   if (len < 0) goto on_error;
   len++;
   info->filename = malloc(len);
   if (!info->filename) goto on_error;
   snprintf(info->filename,len,"%s/%sXXXXXX", tmppath, "elmcnpitem-");
   cur_umask = umask(S_IRWXO | S_IRWXG);
   info->fd = mkstemp(info->filename);
   umask(cur_umask);
   if (info->fd < 0) goto on_error;
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
   if (size < 1) goto on_error;
   /* Map it in */
   if (ftruncate(info->fd, size))
     {
        perror("ftruncate");
        goto on_error;
     }
   eina_mmap_safety_enabled_set(EINA_TRUE);
   info->map = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, info->fd, 0);
   if (info->map == MAP_FAILED)
     {
        perror("mmap");
        goto on_error;
     }
   return info;

 on_error:
   if (info->fd >= 0) close(info->fd);
   info->fd = -1;
   /* Set map to NULL and return */
   info->map = NULL;
   info->len = 0;
   free(info->filename);
   free(info);
   return NULL;
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
   if (!_elm_cnp_init_count) _elm_cnp_init();
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   if (xwin)
     return _x11_elm_cnp_selection_set(xwin, obj, selection, format, selbuf, buflen);
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
   if (!_elm_cnp_init_count) _elm_cnp_init();
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     _x11_elm_cnp_selection_loss_callback_set(obj, selection, func, data);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   if (elm_win_wl_window_get(obj))
     _wl_elm_cnp_selection_loss_callback_set(obj, selection, func, data);
#endif
   _local_elm_cnp_selection_loss_callback_set(obj, selection, func, data);
}

EAPI Eina_Bool
elm_object_cnp_selection_clear(Evas_Object *obj, Elm_Sel_Type selection)
{
   if (selection > ELM_SEL_TYPE_CLIPBOARD) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_object_cnp_selection_clear(obj, selection);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   if (elm_win_wl_window_get(obj))
      return _wl_elm_cnp_selection_clear(obj, selection);
#endif
   return _local_elm_object_cnp_selection_clear(obj, selection);
}

EAPI Eina_Bool
elm_cnp_selection_get(Evas_Object *obj, Elm_Sel_Type selection,
                      Elm_Sel_Format format, Elm_Drop_Cb datacb, void *udata)
{
   if (selection > ELM_SEL_TYPE_CLIPBOARD) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   if (xwin)
     return _x11_elm_cnp_selection_get(xwin, obj, selection, format, datacb, udata);
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
elm_drop_target_add(Evas_Object *obj, Elm_Sel_Format format,
                    Elm_Drag_State entercb, void *enterdata,
                    Elm_Drag_State leavecb, void *leavedata,
                    Elm_Drag_Pos poscb, void *posdata,
                    Elm_Drop_Cb dropcb, void *dropdata)
{
   if (!_elm_cnp_init_count) _elm_cnp_init();
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_drop_target_add(obj, format, entercb, enterdata,
                                     leavecb, leavedata, poscb, posdata,
                                     dropcb, dropdata);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   return _wl_elm_drop_target_add(obj, format, entercb, enterdata,
                                  leavecb, leavedata, poscb, posdata,
                                  dropcb, dropdata);
#endif
   return _local_elm_drop_target_add(obj, format, entercb, enterdata,
                                     leavecb, leavedata, poscb, posdata,
                                     dropcb, dropdata);
}

EAPI Eina_Bool
elm_drop_target_del(Evas_Object *obj, Elm_Sel_Format format,
                    Elm_Drag_State entercb, void *enterdata,
                    Elm_Drag_State leavecb, void *leavedata,
                    Elm_Drag_Pos poscb, void *posdata,
                    Elm_Drop_Cb dropcb, void *dropdata)
{
   if (!_elm_cnp_init_count) _elm_cnp_init();
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_drop_target_del(obj, format, entercb, enterdata,
           leavecb, leavedata, poscb, posdata, dropcb, dropdata);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   return _wl_elm_drop_target_del(obj, format, entercb, enterdata,
         leavecb, leavedata, poscb, posdata, dropcb, dropdata);
#endif
   return _local_elm_drop_target_del(obj);
}

EAPI Eina_Bool
elm_drag_start(Evas_Object *obj, Elm_Sel_Format format, const char *data,
               Elm_Xdnd_Action action,
               Elm_Drag_Icon_Create_Cb createicon, void *createdata,
               Elm_Drag_Pos dragpos, void *dragdata,
               Elm_Drag_Accept acceptcb, void *acceptdata,
               Elm_Drag_State dragdone, void *donecbdata)
{
   if (!_elm_cnp_init_count) _elm_cnp_init();
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_drag_start(obj, format, data, action,
                                createicon, createdata,
                                dragpos, dragdata,
                                acceptcb, acceptdata,
                                dragdone, donecbdata);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   return _wl_elm_drag_start(obj, format, data, action,
                             createicon, createdata,
                             dragpos, dragdata,
                             acceptcb, acceptdata,
                             dragdone, donecbdata);
#endif
   return _local_elm_drag_start(obj, format, data, action,
                                createicon, createdata,
                                dragpos, dragdata,
                                acceptcb, acceptdata,
                                dragdone, donecbdata);
}

EAPI Eina_Bool
elm_drag_action_set(Evas_Object *obj, Elm_Xdnd_Action action)
{
   if (!_elm_cnp_init_count) _elm_cnp_init();
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_drag_action_set(obj, action);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   return _wl_elm_drag_action_set(obj, action);
#endif
   return _local_elm_drag_action_set(obj, action);
}

EAPI Eina_Bool
elm_selection_selection_has_owner(Evas_Object *obj)
{
   if (!_elm_cnp_init_count) _elm_cnp_init();
#ifdef HAVE_ELEMENTARY_X
   if (_x11_elm_widget_xwin_get(obj))
     return _x11_elm_selection_selection_has_owner(obj);
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   if (elm_win_wl_window_get(obj))
     return ecore_wl_dnd_selection_owner_has(ecore_wl_input_get());
#endif
   return _local_elm_selection_selection_has_owner(obj);
}

/* START - Support elm containers for Drag and Drop */
/* START - Support elm containers for Drop */
static int
_drop_item_container_cmp(const void *d1,
               const void *d2)
{
   const Item_Container_Drop_Info *st = d1;
   return (((uintptr_t) (st->obj)) - ((uintptr_t) d2));
}

static void
_elm_item_container_pos_cb(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y, Elm_Xdnd_Action action)
{  /* obj is the container pointer */
   Elm_Object_Item *it = NULL;
   int xposret = 0;
   int yposret = 0;

   Item_Container_Drop_Info *st =
      eina_list_search_unsorted(cont_drop_tg, _drop_item_container_cmp, obj);

   if (st && st->poscb)
     {  /* Call container drop func with specific item pointer */
        int xo = 0;
        int yo = 0;
        eo_do(obj, evas_obj_position_get(&xo, &yo));
        if (st->itemgetcb)
          it = st->itemgetcb(obj, x+xo, y+yo, &xposret, &yposret);

        st->poscb(data, obj, it, x, y, xposret, yposret, action);
     }
}

static Eina_Bool
_elm_item_container_drop_cb(void *data, Evas_Object *obj , Elm_Selection_Data *ev)
{  /* obj is the container pointer */
   Elm_Object_Item *it = NULL;
   int xposret = 0;
   int yposret = 0;

   Item_Container_Drop_Info *st =
      eina_list_search_unsorted(cont_drop_tg, _drop_item_container_cmp, obj);

   if (st && st->dropcb)
     {  /* Call container drop func with specific item pointer */
        int xo = 0;
        int yo = 0;
        eo_do(obj, evas_obj_position_get(&xo, &yo));
        if (st->itemgetcb)
          it = st->itemgetcb(obj, ev->x+xo, ev->y+yo, &xposret, &yposret);

        return st->dropcb(data, obj, it, ev, xposret, yposret);
     }

   return EINA_FALSE;
}

static Eina_Bool
elm_drop_item_container_del_internal(Evas_Object *obj, Eina_Bool full)
{
   Item_Container_Drop_Info *st =
      eina_list_search_unsorted(cont_drop_tg, _drop_item_container_cmp, obj);

   if (st)
     {
        // temp until st is stored inside data of obj.
        _all_drop_targets_cbs_del(NULL, NULL, obj, NULL);
        st->itemgetcb= NULL;
        st->poscb = NULL;
        st->dropcb = NULL;

        if (full)
          {
             cont_drop_tg = eina_list_remove(cont_drop_tg, st);
             free(st);
          }

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
elm_drop_item_container_del(Evas_Object *obj)
{
   return elm_drop_item_container_del_internal(obj, EINA_TRUE);
}

EAPI Eina_Bool
elm_drop_item_container_add(Evas_Object *obj,
      Elm_Sel_Format format,
      Elm_Xy_Item_Get_Cb itemgetcb,
      Elm_Drag_State entercb, void *enterdata,
      Elm_Drag_State leavecb, void *leavedata,
      Elm_Drag_Item_Container_Pos poscb, void *posdata,
      Elm_Drop_Item_Container_Cb dropcb, void *dropdata)
{
   Item_Container_Drop_Info *st;

   if (elm_drop_item_container_del_internal(obj, EINA_FALSE))
     {  /* Updating info of existing obj */
        st = eina_list_search_unsorted(cont_drop_tg, _drop_item_container_cmp, obj);
        if (!st) return EINA_FALSE;
     }
   else
     {
        st = calloc(1, sizeof(*st));
        if (!st) return EINA_FALSE;

        st->obj = obj;
        cont_drop_tg = eina_list_append(cont_drop_tg, st);
     }

   st->itemgetcb = itemgetcb;
   st->poscb = poscb;
   st->dropcb = dropcb;
   elm_drop_target_add(obj, format,
                       entercb, enterdata,
                       leavecb, leavedata,
                       _elm_item_container_pos_cb, posdata,
                       _elm_item_container_drop_cb, dropdata);

   return EINA_TRUE;
}
/* END   - Support elm containers for Drop */

/* START - Support elm containers for Drag */
static int
_drag_item_container_cmp(const void *d1,
               const void *d2)
{
   const Item_Container_Drag_Info *st = d1;
   return (((uintptr_t) (st->obj)) - ((uintptr_t) d2));
}

static void
_cont_drag_done_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Item_Container_Drag_Info *st = data;
   elm_widget_scroll_freeze_pop(st->obj);
   if (st->user_info.dragdone)
     st->user_info.dragdone(st->user_info.donecbdata, dragwidget, doaccept);
}

static Eina_Bool
_cont_obj_drag_start(void *data)
{  /* Start a drag-action when timer expires */
   cnp_debug("In\n");
   Item_Container_Drag_Info *st = data;
   st->tm = NULL;
   Elm_Drag_User_Info *info = &st->user_info;
   if (info->dragstart) info->dragstart(info->startcbdata, st->obj);
   elm_widget_scroll_freeze_push(st->obj);
   evas_object_event_callback_del_full
      (st->obj, EVAS_CALLBACK_MOUSE_MOVE, _cont_obj_mouse_move, st);
   elm_drag_start(  /* Commit the start only if data_get successful */
         st->obj, info->format,
         info->data, info->action,
         info->createicon, info->createdata,
         info->dragpos, info->dragdata,
         info->acceptcb, info->acceptdata,
         _cont_drag_done_cb, st);
   ELM_SAFE_FREE(info->data, free);

   return ECORE_CALLBACK_CANCEL;
}

void
_anim_st_free(Item_Container_Drag_Info *st)
{  /* Stops and free mem of ongoing animation */
   if (st)
     {
        ELM_SAFE_FREE(st->ea, ecore_animator_del);
        Anim_Icon *sti;

        EINA_LIST_FREE(st->icons, sti)
          {
             evas_object_del(sti->o);
             free(sti);
          }

        st->icons = NULL;
     }
}

static inline Eina_List *
_anim_icons_make(Eina_List *icons)
{  /* Make local copies of all icons, add them to list */
   Eina_List *list = NULL, *itr;
   Evas_Object *o;

   EINA_LIST_FOREACH(icons, itr, o)
     {  /* Now add icons to animation window */
        Anim_Icon *st = calloc(1, sizeof(*st));
        evas_object_geometry_get(o, &st->start_x, &st->start_y, &st->start_w, &st->start_h);
        evas_object_show(o);
        st->o = o;
        list = eina_list_append(list, st);
     }

   return list;
}

static Eina_Bool
_drag_anim_play(void *data, double pos)
{  /* Impl of the animation of icons, called on frame time */
   cnp_debug("In\n");
   Item_Container_Drag_Info *st = data;
   Eina_List *l;
   Anim_Icon *sti;

   if (st->ea)
     {
        if (pos > 0.99)
          {
             st->ea = NULL;  /* Avoid deleting on mouse up */
             EINA_LIST_FOREACH(st->icons, l, sti)
                evas_object_hide(sti->o);

             _cont_obj_drag_start(st);  /* Start dragging */
             return ECORE_CALLBACK_CANCEL;
          }

        Evas_Coord xm, ym;
        evas_pointer_canvas_xy_get(st->e, &xm, &ym);
        EINA_LIST_FOREACH(st->icons, l, sti)
          {
             int x, y, h, w;
             w = sti->start_w + ((st->final_icon_w - sti->start_w) * pos);
             h = sti->start_h + ((st->final_icon_h - sti->start_h) * pos);
             x = sti->start_x - (pos * ((sti->start_x + (w/2) - xm)));
             y = sti->start_y - (pos * ((sti->start_y + (h/2) - ym)));
             evas_object_move(sti->o, x, y);
             evas_object_resize(sti->o, w, h);
          }

        return ECORE_CALLBACK_RENEW;
     }

   return ECORE_CALLBACK_CANCEL;
}

static inline Eina_Bool
_drag_anim_start(void *data)
{  /* Start default animation */
   cnp_debug("In\n");
   Item_Container_Drag_Info *st = data;

   st->tm = NULL;
   /* Now we need to build an (Anim_Icon *) list */
   st->icons = _anim_icons_make(st->user_info.icons);
   if (st->user_info.createicon)
     {
        Evas_Object *temp_win = elm_win_add(NULL, "Temp", ELM_WIN_UTILITY);
        Evas_Object *final_icon = st->user_info.createicon(st->user_info.createdata, temp_win, NULL, NULL);
        evas_object_geometry_get(final_icon, NULL, NULL, &st->final_icon_w, &st->final_icon_h);
        evas_object_del(final_icon);
        evas_object_del(temp_win);
     }
   st->ea = ecore_animator_timeline_add(st->anim_tm, _drag_anim_play, st);

   return EINA_FALSE;
}

static Eina_Bool
_cont_obj_anim_start(void *data)
{  /* Start a drag-action when timer expires */
   cnp_debug("In\n");
   Item_Container_Drag_Info *st = data;
   int xposret, yposret;  /* Unused */
   Elm_Object_Item *it = (st->itemgetcb) ?
      (st->itemgetcb(st->obj, st->x_down, st->y_down, &xposret, &yposret))
      : NULL;

   st->tm = NULL;
   st->user_info.format = ELM_SEL_FORMAT_TARGETS; /* Default */
   st->icons = NULL;
   st->user_info.data = NULL;
   st->user_info.action = ELM_XDND_ACTION_COPY;  /* Default */

   if (!it)   /* Failed to get mouse-down item, abort drag */
     return ECORE_CALLBACK_CANCEL;

   if (st->data_get)
     {  /* collect info then start animation or start dragging */
        if(st->data_get(    /* Collect drag info */
                 st->obj,      /* The container object */
                 it,           /* Drag started on this item */
                 &st->user_info))
          {
             if (st->user_info.icons)
               _drag_anim_start(st);
             else
               {
                  if (st->anim_tm)
                    {
                       // even if we don't manage the icons animation, we have
                       // to wait until it is finished before beginning drag.
                       st->tm = ecore_timer_add(st->anim_tm, _cont_obj_drag_start, st);
                    }
                  else
                    _cont_obj_drag_start(st);  /* Start dragging, no anim */
               }
          }
     }

   return ECORE_CALLBACK_CANCEL;
}

static void
_cont_obj_mouse_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{  /* Launch a timer to start dragging */
   Evas_Event_Mouse_Down *ev = event_info;
   cnp_debug("In - event %X\n", ev->event_flags);
   if (ev->button != 1)
     return;  /* We only process left-click at the moment */

   Item_Container_Drag_Info *st = data;
   evas_object_event_callback_add(st->obj, EVAS_CALLBACK_MOUSE_MOVE,
         _cont_obj_mouse_move, st);

   evas_object_event_callback_add(st->obj, EVAS_CALLBACK_MOUSE_UP,
         _cont_obj_mouse_up, st);

   ecore_timer_del(st->tm);

   st->e = e;
   st->x_down = ev->canvas.x;
   st->y_down = ev->canvas.y;
   st->tm = ecore_timer_add(st->tm_to_drag, _cont_obj_anim_start, st);
}

static Eina_Bool elm_drag_item_container_del_internal(Evas_Object *obj, Eina_Bool full);

static void
_cont_obj_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{  /* Cancel any drag waiting to start on timeout */
   cnp_debug("In\n");
   if (((Evas_Event_Mouse_Move *)event_info)->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        cnp_debug("event on hold - have to cancel DnD\n");
        Item_Container_Drag_Info *st = data;

        evas_object_event_callback_del_full
           (st->obj, EVAS_CALLBACK_MOUSE_MOVE, _cont_obj_mouse_move, st);
        evas_object_event_callback_del_full
           (st->obj, EVAS_CALLBACK_MOUSE_UP, _cont_obj_mouse_up, st);
        elm_drag_item_container_del_internal(obj, EINA_FALSE);

        ELM_SAFE_FREE(st->tm, ecore_timer_del);

        _anim_st_free(st);
     }
   cnp_debug("Out\n");
}

static void
_cont_obj_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{  /* Cancel any drag waiting to start on timeout */
   Item_Container_Drag_Info *st = data;

   cnp_debug("In\n");
   if (((Evas_Event_Mouse_Up *)event_info)->button != 1)
     return;  /* We only process left-click at the moment */

   evas_object_event_callback_del_full
      (st->obj, EVAS_CALLBACK_MOUSE_MOVE, _cont_obj_mouse_move, st);
   evas_object_event_callback_del_full
      (st->obj, EVAS_CALLBACK_MOUSE_UP, _cont_obj_mouse_up, st);

   ELM_SAFE_FREE(st->tm, ecore_timer_del);

   _anim_st_free(st);
}

static Eina_Bool
elm_drag_item_container_del_internal(Evas_Object *obj, Eina_Bool full)
{
   Item_Container_Drag_Info *st =
      eina_list_search_unsorted(cont_drag_tg, _drag_item_container_cmp, obj);

   if (st)
     {
        ELM_SAFE_FREE(st->tm, ecore_timer_del); /* Cancel drag-start timer */

        if (st->ea)  /* Cancel ongoing default animation */
          _anim_st_free(st);

        if (full)
          {
             st->itemgetcb = NULL;
             st->data_get = NULL;
             evas_object_event_callback_del_full
                (obj, EVAS_CALLBACK_MOUSE_DOWN, _cont_obj_mouse_down, st);

             cont_drag_tg = eina_list_remove(cont_drag_tg, st);
             ELM_SAFE_FREE(st->user_info.data, free);
             free(st);
          }

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EAPI Eina_Bool
elm_drag_item_container_del(Evas_Object *obj)
{
   return elm_drag_item_container_del_internal(obj, EINA_TRUE);
}

EAPI Eina_Bool
elm_drag_item_container_add(Evas_Object *obj, double anim_tm, double tm_to_drag, Elm_Xy_Item_Get_Cb itemgetcb, Elm_Item_Container_Data_Get_Cb data_get)
{
   Item_Container_Drag_Info *st;

   if (elm_drag_item_container_del_internal(obj, EINA_FALSE))
     {  /* Updating info of existing obj */
        st = eina_list_search_unsorted(cont_drag_tg, _drag_item_container_cmp, obj);
        if (!st) return EINA_FALSE;
     }
   else
     {
        st = calloc(1, sizeof(*st));
        if (!st) return EINA_FALSE;

        st->obj = obj;
        cont_drag_tg = eina_list_append(cont_drag_tg, st);

        /* Register for mouse callback for container to start/abort drag */
        evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN,
                                       _cont_obj_mouse_down, st);
     }

   st->tm = NULL;
   st->anim_tm = anim_tm;
   st->tm_to_drag = tm_to_drag;
   st->itemgetcb = itemgetcb;
   st->data_get = data_get;

   return EINA_TRUE;
}
/* END   - Support elm containers for Drag */
/* END   - Support elm containers for Drag and Drop */

EAPI Eina_Bool
elm_drag_cancel(Evas_Object *obj)
{
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   if (xwin)
     {
        ecore_x_pointer_ungrab();
        ELM_SAFE_FREE(handler_up, ecore_event_handler_del);
        ELM_SAFE_FREE(handler_status, ecore_event_handler_del);
        ecore_x_dnd_abort(xwin);
     }
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
/* Have to complete here.
 * if (elm_win_wl_window_get(obj)) ... */
#endif

   ELM_SAFE_FREE(dragwin, evas_object_del);
   dragdonecb = NULL;
   dragacceptcb = NULL;
   dragposcb = NULL;
   dragwidget = NULL;
   doaccept = EINA_FALSE;
   return EINA_TRUE;
}
