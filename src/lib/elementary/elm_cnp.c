#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include <Efreet.h>
#include "elm_priv.h"
#ifdef HAVE_MMAN_H
# include <sys/mman.h>
#endif

//#define DEBUGON 1
#ifdef DEBUGON
# define cnp_debug(fmt, args...) fprintf(stderr, __FILE__":%s/%d : " fmt , __FUNCTION__, __LINE__, ##args)
#else
# define cnp_debug(x...) do { } while (0)
#endif

#define ARRAYINIT(foo)  [foo] =

EAPI int ELM_CNP_EVENT_SELECTION_CHANGED = -1;

// common stuff
enum
{
   CNP_ATOM_TARGETS = 0,
   CNP_ATOM_ATOM,
   CNP_ATOM_LISTING_ATOMS = CNP_ATOM_ATOM,
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

typedef struct _Cnp_Atom      Cnp_Atom;

static int _elm_cnp_init_count = 0;
/* Stringshared, so I can just compare pointers later */
static const char *text_uri;

/* Hash table type->Elm_Sel_Format */
static Eina_Hash *_types_hash = NULL;

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
static Ecore_Event_Handler *handler_leave = NULL;
#ifdef HAVE_ELEMENTARY_X
static Ecore_Event_Handler *handler_status = NULL;
static Ecore_Event_Handler *handler_up = NULL;
#endif

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
static Eina_Bool _elm_cnp_shutdown(void);
static  Eina_Bool _local_elm_drop_target_del(Evas_Object *obj, Elm_Sel_Format format,
                         Elm_Drag_State entercb, void *enterdata,
                         Elm_Drag_State leavecb, void *leavedata,
                         Elm_Drag_Pos poscb, void *posdata,
                         Elm_Drop_Cb dropcb, void *dropdata);

static Tmp_Info  *_tempfile_new      (int size);
static int        _tmpinfo_free      (Tmp_Info *tmp);
#ifdef HAVE_ELEMENTARY_X
static Ecore_X_Window _x11_elm_widget_xwin_get(const Evas_Object *obj);

typedef struct _X11_Cnp_Selection X11_Cnp_Selection;

typedef Eina_Bool (*X11_Converter_Fn_Cb)     (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
typedef int       (*X11_Response_Handler_Cb) (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *);
typedef Eina_Bool (*X11_Data_Preparer_Cb)    (Ecore_X_Event_Selection_Notify *, Elm_Selection_Data *, Tmp_Info **);

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
static Eina_Bool      _x11_notify_handler_targets   (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static Eina_Bool      _x11_data_preparer_text       (Ecore_X_Event_Selection_Notify *notify, Elm_Selection_Data *ddata, Tmp_Info **tmp_info);
static Eina_Bool      _x11_data_preparer_markup     (Ecore_X_Event_Selection_Notify *notify, Elm_Selection_Data *ddata, Tmp_Info **tmp_info);
static Eina_Bool      _x11_data_preparer_image      (Ecore_X_Event_Selection_Notify *notify, Elm_Selection_Data *ddata, Tmp_Info **tmp_info);
static Eina_Bool      _x11_data_preparer_uri        (Ecore_X_Event_Selection_Notify *notify, Elm_Selection_Data *ddata, Tmp_Info **tmp_info);
//static int            _x11_notify_handler_html      (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify);
static Eina_Bool      _x11_data_preparer_vcard      (Ecore_X_Event_Selection_Notify *notify, Elm_Selection_Data *ddata, Tmp_Info **tmp_info);
static Eina_Bool      _x11_dnd_enter                (void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev);
static Eina_Bool      _x11_dnd_drop                 (void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev);
static Eina_Bool      _x11_dnd_position             (void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev);
static Eina_Bool      _x11_dnd_status               (void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev);
static Eina_Bool      _x11_dnd_leave                (void *data EINA_UNUSED, int etype EINA_UNUSED, void *ev);
static Eina_Bool      _x11_drag_mouse_up            (void *data, int etype EINA_UNUSED, void *event);
static void           _x11_drag_move                (void *data EINA_UNUSED, Ecore_X_Xdnd_Position *pos);

static Eina_Bool _x11_elm_cnp_init                       (void);
static Eina_Bool _x11_elm_cnp_selection_set              (Ecore_X_Window xwin, Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, const void *selbuf, size_t buflen);
static void      _x11_elm_cnp_selection_loss_callback_set(Evas_Object *obj EINA_UNUSED, Elm_Sel_Type selection, Elm_Selection_Loss_Cb func, const void *data);
static Eina_Bool _x11_elm_object_cnp_selection_clear     (Evas_Object *obj, Elm_Sel_Type selection);
static Eina_Bool _x11_elm_cnp_selection_get              (Ecore_X_Window xwin, const Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, Elm_Drop_Cb datacb, void *udata);
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

#endif

#ifdef HAVE_ELEMENTARY_WL2
typedef struct _Wl_Cnp_Selection Wl_Cnp_Selection;

typedef Eina_Bool (*Wl_Converter_Fn_Cb)     (char *target, Wl_Cnp_Selection *sel, void *data, int size, void **data_ret, int *size_ret);
static Eina_Bool _wl_targets_converter(char *target, Wl_Cnp_Selection *sel, void *data, int size, void **data_ret, int *size_ret);
static Eina_Bool _wl_general_converter(char *target, Wl_Cnp_Selection *sel, void *data, int size, void **data_ret, int *size_ret);
static Eina_Bool _wl_text_converter(char *target, Wl_Cnp_Selection *sel, void *data, int size, void **data_ret, int *size_ret);

typedef Eina_Bool       (*Wl_Data_Preparer_Cb)   (Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
static Eina_Bool _wl_data_preparer_markup(Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
static Eina_Bool _wl_data_preparer_uri(Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
static Eina_Bool _wl_data_preparer_vcard(Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
static Eina_Bool _wl_data_preparer_image(Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
static Eina_Bool _wl_data_preparer_text(Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
#endif

struct _Cnp_Atom
{
   const char              *name;
   Elm_Sel_Format           formats;
#ifdef HAVE_ELEMENTARY_X
   /* Called by ecore to do conversion */
   X11_Converter_Fn_Cb      x_converter;
   X11_Data_Preparer_Cb     x_data_preparer;
   /* Atom */
   Ecore_X_Atom             x_atom;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Wl_Converter_Fn_Cb       wl_converter;
   Wl_Data_Preparer_Cb      wl_data_preparer;
#endif

   void                    *_term;
};

static Eina_List *
_dropable_list_geom_find(Evas *evas, Evas_Coord px, Evas_Coord py)
{
   Eina_List *itr, *top_objects_list = NULL, *dropable_list = NULL;
   Evas_Object *top_obj;
   Dropable *dropable = NULL;

   if (!drops) return NULL;

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
             dropable = efl_key_data_get(object, "__elm_dropable");
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
                              dropable->obj, efl_class_name_get(efl_class_get(dropable->obj)));
               }
             else
                object = evas_object_smart_parent_get(object);
          }
     }
   eina_list_free(top_objects_list);
   return dropable_list;
}

static void
_dropable_coords_adjust(Dropable *dropable, Evas_Coord *x, Evas_Coord *y)
{
   Ecore_Evas *ee;
   Evas *evas = evas_object_evas_get(dropable->obj);
   int ex = 0, ey = 0, ew = 0, eh = 0;
   Evas_Object *win;

   ee = ecore_evas_ecore_evas_get(evas);
   ecore_evas_geometry_get(ee, &ex, &ey, &ew, &eh);
   *x = *x - ex;
   *y = *y - ey;

   /* For Wayland, frame coords have to be subtracted. */
   Evas_Coord fx, fy;
   evas_output_framespace_get(evas, &fx, &fy, NULL, NULL);
   if (fx || fy) cnp_debug("evas frame fx %d fy %d\n", fx, fy);
   *x = *x - fx;
   *y = *y - fy;

   if (elm_widget_is(dropable->obj))
     {
        win = elm_widget_top_get(dropable->obj);
        if (win && efl_isa(win, EFL_UI_WIN_CLASS))
          {
             Evas_Coord x2, y2;
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

static Elm_Sel_Format
_dnd_types_to_format(const char **types, int ntypes)
{
   Elm_Sel_Format ret_type = 0;
   int i;
   for (i = 0; i < ntypes; i++)
     {
        Cnp_Atom *atom = eina_hash_find(_types_hash, types[i]);
        if (atom) ret_type |= atom->formats;
     }
   return ret_type;
}

static Eina_Bool
_drag_cancel_animate(void *data, double pos)
{  /* Animation to "move back" drag-window */
   if (pos >= 0.99)
     {
#ifdef HAVE_ELEMENTARY_X
        Ecore_X_Window xdragwin = _x11_elm_widget_xwin_get(data);
        ecore_x_window_ignore_set(xdragwin, 0);
#endif
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
   dropable = efl_key_data_get(obj, "__elm_dropable");
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
             dropable = efl_key_data_get(obj, "__elm_dropable");
             if (!dropable) break;
          }
     }
}

static Cnp_Atom _atoms[CNP_N_ATOMS] = {
   ARRAYINIT(CNP_ATOM_TARGETS) {
        .name = "TARGETS",
        .formats = ELM_SEL_FORMAT_TARGETS,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_targets_converter,

#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_targets_converter,
#endif
   },
   ARRAYINIT(CNP_ATOM_ATOM) {
        .name = "ATOM", // for opera browser
        .formats = ELM_SEL_FORMAT_TARGETS,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_targets_converter,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_targets_converter,
#endif
   },
   ARRAYINIT(CNP_ATOM_XELM)  {
        .name = "application/x-elementary-markup",
        .formats = ELM_SEL_FORMAT_MARKUP,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_general_converter,
        .x_data_preparer = _x11_data_preparer_markup,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_general_converter,
        .wl_data_preparer = _wl_data_preparer_markup,
#endif
   },
   ARRAYINIT(CNP_ATOM_text_urilist) {
        .name = "text/uri-list",
        .formats = ELM_SEL_FORMAT_IMAGE,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_general_converter,
        .x_data_preparer = _x11_data_preparer_uri,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_general_converter,
        .wl_data_preparer = _wl_data_preparer_uri,
#endif
   },
   ARRAYINIT(CNP_ATOM_text_x_vcard) {
        .name = "text/x-vcard",
        .formats = ELM_SEL_FORMAT_VCARD,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_vcard_send,
        .x_data_preparer = _x11_data_preparer_vcard,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_data_preparer = _wl_data_preparer_vcard,
#endif
   },
   ARRAYINIT(CNP_ATOM_image_png) {
        .name = "image/png",
        .formats = ELM_SEL_FORMAT_IMAGE,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_image_converter,
        .x_data_preparer = _x11_data_preparer_image,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_data_preparer = _wl_data_preparer_image,
#endif
   },
   ARRAYINIT(CNP_ATOM_image_jpeg) {
        .name = "image/jpeg",
        .formats = ELM_SEL_FORMAT_IMAGE,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_image_converter,
        .x_data_preparer = _x11_data_preparer_image,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_data_preparer = _wl_data_preparer_image,
#endif
   },
   ARRAYINIT(CNP_ATOM_image_bmp) {
        .name = "image/x-ms-bmp",
        .formats = ELM_SEL_FORMAT_IMAGE,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_image_converter,
        .x_data_preparer = _x11_data_preparer_image,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_data_preparer = _wl_data_preparer_image,
#endif
   },
   ARRAYINIT(CNP_ATOM_image_gif) {
        .name = "image/gif",
        .formats = ELM_SEL_FORMAT_IMAGE,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_image_converter,
        .x_data_preparer = _x11_data_preparer_image,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_data_preparer = _wl_data_preparer_image,
#endif
   },
   ARRAYINIT(CNP_ATOM_image_tiff) {
        .name = "image/tiff",
        .formats = ELM_SEL_FORMAT_IMAGE,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_image_converter,
        .x_data_preparer = _x11_data_preparer_image,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_data_preparer = _wl_data_preparer_image,
#endif
   },
   ARRAYINIT(CNP_ATOM_image_svg) {
        .name = "image/svg+xml",
        .formats = ELM_SEL_FORMAT_IMAGE,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_image_converter,
        .x_data_preparer = _x11_data_preparer_image,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_data_preparer = _wl_data_preparer_image,
#endif
   },
   ARRAYINIT(CNP_ATOM_image_xpm) {
        .name = "image/x-xpixmap",
        .formats = ELM_SEL_FORMAT_IMAGE,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_image_converter,
        .x_data_preparer = _x11_data_preparer_image,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_data_preparer = _wl_data_preparer_image,
#endif
   },
   ARRAYINIT(CNP_ATOM_image_tga) {
        .name = "image/x-tga",
        .formats = ELM_SEL_FORMAT_IMAGE,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_image_converter,
        .x_data_preparer = _x11_data_preparer_image,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_data_preparer = _wl_data_preparer_image,
#endif
   },
   ARRAYINIT(CNP_ATOM_image_ppm) {
        .name = "image/x-portable-pixmap",
        .formats = ELM_SEL_FORMAT_IMAGE,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_image_converter,
        .x_data_preparer = _x11_data_preparer_image,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_data_preparer = _wl_data_preparer_image,
#endif
   },
/*
   ARRAYINIT(CNP_ATOM_text_html_utf8) {
      .name = "text/html;charset=utf-8",
      .formats = ELM_SEL_FORMAT_HTML,
#ifdef HAVE_ELEMENTARY_X
      .x_converter = _x11_general_converter,
      .x_notify = _x11_notify_handler_html,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_general_converter,
        .wl_data_preparer = _wl_data_preparer_handler_html,
#endif

   },
   ARRAYINIT(CNP_ATOM_text_html) {
      .name = "text/html",
      .formats = ELM_SEL_FORMAT_HTML,
#ifdef HAVE_ELEMENTARY_X
      .x_converter = _x11_general_converter,
      .x_notify = _x11_notify_handler_html,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_general_converter,
        .wl_data_preparer = _wl_data_preparer_handler_html,
#endif
   },
 */
   ARRAYINIT(CNP_ATOM_UTF8STRING) {
        .name = "UTF8_STRING",
        .formats = ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_text_converter,
        .x_data_preparer = _x11_data_preparer_text,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_text_converter,
        .wl_data_preparer = _wl_data_preparer_text,
#endif
   },
   ARRAYINIT(CNP_ATOM_STRING) {
        .name = "STRING",
        .formats = ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_text_converter,
        .x_data_preparer = _x11_data_preparer_text,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_text_converter,
        .wl_data_preparer = _wl_data_preparer_text,
#endif
   },
   ARRAYINIT(CNP_ATOM_COMPOUND_TEXT) {
        .name = "COMPOUND_TEXT",
        .formats = ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_text_converter,
        .x_data_preparer = _x11_data_preparer_text,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_text_converter,
        .wl_data_preparer = _wl_data_preparer_text,
#endif
   },
   ARRAYINIT(CNP_ATOM_TEXT) {
        .name = "TEXT",
        .formats = ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_text_converter,
        .x_data_preparer = _x11_data_preparer_text,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_text_converter,
        .wl_data_preparer = _wl_data_preparer_text,
#endif
   },
   ARRAYINIT(CNP_ATOM_text_plain_utf8) {
        .name = "text/plain;charset=utf-8",
        .formats = ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_text_converter,
        .x_data_preparer = _x11_data_preparer_text,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_text_converter,
        .wl_data_preparer = _wl_data_preparer_text,
#endif
   },
   ARRAYINIT(CNP_ATOM_text_plain) {
        .name = "text/plain",
        .formats = ELM_SEL_FORMAT_TEXT | ELM_SEL_FORMAT_MARKUP | ELM_SEL_FORMAT_HTML,
#ifdef HAVE_ELEMENTARY_X
        .x_converter = _x11_text_converter,
        .x_data_preparer = _x11_data_preparer_text,
#endif
#ifdef HAVE_ELEMENTARY_WL2
        .wl_converter = _wl_text_converter,
        .wl_data_preparer = _wl_data_preparer_text,
#endif
   },
};

// x11 specific stuff
////////////////////////////////////////////////////////////////////////////
#ifdef HAVE_ELEMENTARY_X
#define ARRAYINIT(foo)  [foo] =
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
   if (sel->widget == obj)
     {
        sel->loss_cb = NULL;
        sel->loss_data = NULL;
        sel->widget = NULL;
     }
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
   sel->loss_cb = NULL;
   sel->loss_data = NULL;

   sel->active = EINA_FALSE;
   ELM_SAFE_FREE(sel->selbuf, free);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_x11_fixes_selection_notify(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
{
   Elm_Cnp_Event_Selection_Changed *e;
   Ecore_X_Event_Fixes_Selection_Notify *ev = event;
   Elm_Sel_Type type;

   switch (ev->selection)
     {
      case ECORE_X_SELECTION_CLIPBOARD:
        type = ELM_SEL_TYPE_CLIPBOARD;
        break;
      case ECORE_X_SELECTION_PRIMARY:
        type = ELM_SEL_TYPE_PRIMARY;
        break;
      default: return ECORE_CALLBACK_RENEW;
     }
   e = calloc(1, sizeof(Elm_Cnp_Event_Selection_Changed));
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, ECORE_CALLBACK_RENEW);
   e->type = type;
   e->seat_id = 1; /* under x11 this is always the default seat */
   e->exists = !!ev->owner;
   ecore_event_add(ELM_CNP_EVENT_SELECTION_CHANGED, e, NULL, NULL);
   return ECORE_CALLBACK_RENEW;
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

   if (ev->selection != ECORE_X_SELECTION_XDND &&
       (!strcmp(ev->target, "TARGETS") || !strcmp(ev->target, "ATOMS")))
     {
        _x11_notify_handler_targets(sel, ev);
        return ECORE_CALLBACK_PASS_ON;
     }
   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        if (!strcmp(ev->target, _atoms[i].name))
          {
             if (_atoms[i].x_data_preparer)
               {
                  Elm_Selection_Data ddata;
                  Tmp_Info *tmp_info = NULL;
                  Eina_Bool success;
                  ddata.data = NULL;
                  cnp_debug("Found something: %s\n", _atoms[i].name);
                  success = _atoms[i].x_data_preparer(ev, &ddata, &tmp_info);
                  if (_atoms[i].formats == ELM_SEL_FORMAT_IMAGE && savedtypes.imgfile) break;
                  if (ev->selection == ECORE_X_SELECTION_XDND)
                    {
                       if (success)
                         {
                            Dropable *dropable;
                            Eina_List *l;
                            cnp_debug("drag & drop\n");
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
                                 EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                                    if ((cbs->types & dropable->last.format) && cbs->dropcb)
                                       cbs->dropcb(cbs->dropdata, dropable->obj, &ddata);
                              }
                         }
                       /* We have to finish DnD, no matter what */
                       ecore_x_dnd_send_finished();
                    }
                  else if (sel->datacb && success)
                    {
                       ddata.x = ddata.y = 0;
                       sel->datacb(sel->udata, sel->requestwidget, &ddata);
                    }
                  free(ddata.data);
                  if (tmp_info) _tmpinfo_free(tmp_info);
               }
             else cnp_debug("Ignored: No handler!\n");
             break;
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

   for (i = CNP_ATOM_LISTING_ATOMS + 1, count = 0; i < CNP_N_ATOMS ; i++)
     {
        if (seltype & _atoms[i].formats) count++;
     }
   aret = malloc(sizeof(Ecore_X_Atom) * count);
   if (!aret) return EINA_FALSE;
   for (i = CNP_ATOM_LISTING_ATOMS + 1, count = 0; i < CNP_N_ATOMS ; i++)
     {
        if (seltype & _atoms[i].formats)
          aret[count ++] = _atoms[i].x_atom;
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
static Eina_Bool
_x11_notify_handler_targets(X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *notify)
{
   Ecore_X_Selection_Data_Targets *targets;
   Ecore_X_Atom *atomlist;
   int i, j;

   targets = notify->data;
   atomlist = (Ecore_X_Atom *)(targets->data.data);
   for (j = (CNP_ATOM_LISTING_ATOMS + 1); j < CNP_N_ATOMS; j++)
     {
        cnp_debug("\t%s %d\n", _atoms[j].name, _atoms[j].x_atom);
        if (!(_atoms[j].formats & sel->requestformat)) continue;
        for (i = 0; i < targets->data.length; i++)
          {
             if ((_atoms[j].x_atom == atomlist[i]) && (_atoms[j].x_data_preparer))
               {
                  if (j == CNP_ATOM_text_urilist)
                    {
                       if (!_x11_is_uri_type_data(sel, notify)) continue;
                    }
                  cnp_debug("Atom %s matches\n", _atoms[j].name);
                  goto done;
               }
          }
     }
   cnp_debug("Couldn't find anything that matches\n");
   return ECORE_CALLBACK_PASS_ON;
done:
   cnp_debug("Sending request for %s, xwin=%#llx\n",
             _atoms[j].name, (unsigned long long)sel->xwin);
   sel->request(sel->xwin, _atoms[j].name);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_x11_data_preparer_text(Ecore_X_Event_Selection_Notify *notify,
      Elm_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   Ecore_X_Selection_Data *data = notify->data;
   ddata->format = ELM_SEL_FORMAT_TEXT;
   ddata->data = eina_memdup(data->data, data->length, EINA_TRUE);
   ddata->len = data->length;
   return EINA_TRUE;
}

static Eina_Bool
_x11_data_preparer_markup(Ecore_X_Event_Selection_Notify *notify,
      Elm_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   Ecore_X_Selection_Data *data = notify->data;
   ddata->format = ELM_SEL_FORMAT_MARKUP;
   ddata->data = eina_memdup(data->data, data->length, EINA_TRUE);
   ddata->len = data->length;
   return EINA_TRUE;
}

/**
 * So someone is pasting an image into my entry or widget...
 */
static Eina_Bool
_x11_data_preparer_uri(Ecore_X_Event_Selection_Notify *notify,
      Elm_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   Ecore_X_Selection_Data *data;
   Ecore_X_Selection_Data_Files *files;
   char *p, *stripstr = NULL;

   data = notify->data;
   cnp_debug("data->format is %d %p %p\n", data->format, notify, data);
   if (data->content == ECORE_X_SELECTION_CONTENT_FILES)
     {
        Efreet_Uri *uri;
        Eina_Strbuf *strbuf;
        int i;

        cnp_debug("got a files list\n");
        files = notify->data;
        /*
        if (files->num_files > 1)
          {
             // Don't handle many items <- this makes mr bigglesworth sad :(
             cnp_debug("more then one file: Bailing\n");
             return EINA_FALSE;
          }
        stripstr = p = strdup(files->files[0]);
         */

        strbuf = eina_strbuf_new();
        if (!strbuf)
          return EINA_FALSE;

        for (i = 0; i < files->num_files ; i++)
          {
             uri = efreet_uri_decode(files->files[i]);
             if (uri)
               {
                  eina_strbuf_append(strbuf, uri->path);
                  efreet_uri_free(uri);
               }
             else
               {
                  eina_strbuf_append(strbuf, files->files[i]);
               }
             if (i < (files->num_files - 1))
               eina_strbuf_append(strbuf, "\n");
          }
        stripstr = eina_strbuf_string_steal(strbuf);
        eina_strbuf_free(strbuf);
     }
   else
     {
        Efreet_Uri *uri;

        p = (char *)eina_memdup((unsigned char *)data->data, data->length, EINA_TRUE);
        if (!p) return EINA_FALSE;
        uri = efreet_uri_decode(p);
        if (!uri)
          {
             /* Is there any reason why we care of URI without scheme? */
             if (p[0] == '/') stripstr = p;
             else free(p);
          }
        else
          {
             free(p);
             stripstr = strdup(uri->path);
             efreet_uri_free(uri);
          }
     }

   if (!stripstr)
     {
        cnp_debug("Couldn't find a file\n");
        return EINA_FALSE;
     }
   free(savedtypes.imgfile);
   if (savedtypes.textreq)
     {
        savedtypes.textreq = 0;
        savedtypes.imgfile = stripstr;
     }
   else
     {
        ddata->format = ELM_SEL_FORMAT_IMAGE;
        ddata->data = stripstr;
        ddata->len = strlen(stripstr);
        savedtypes.imgfile = NULL;
     }
   return EINA_TRUE;
}

/**
 * Just received an vcard, either through cut and paste, or dnd.
 */
static Eina_Bool
_x11_data_preparer_vcard(Ecore_X_Event_Selection_Notify *notify,
      Elm_Selection_Data *ddata, Tmp_Info **tmp_info EINA_UNUSED)
{
   cnp_debug("vcard receive\n");
   Ecore_X_Selection_Data *data = notify->data;
   ddata->format = ELM_SEL_FORMAT_VCARD;
   ddata->data = eina_memdup(data->data, data->length, EINA_TRUE);
   ddata->len = data->length;
   return EINA_TRUE;
}

static Eina_Bool
_x11_data_preparer_image(Ecore_X_Event_Selection_Notify *notify,
      Elm_Selection_Data *ddata, Tmp_Info **tmp_info)
{
   Ecore_X_Selection_Data *data = notify->data;
   cnp_debug("got a image file!\n");
   cnp_debug("Size if %d\n", data->length);

   ddata->format = ELM_SEL_FORMAT_IMAGE;
   data = notify->data;

   Tmp_Info *tmp = _tempfile_new(data->length);
   if (!tmp) return EINA_FALSE;
   memcpy(tmp->map, data->data, data->length);
   munmap(tmp->map, data->length);
   ddata->data = strdup(tmp->filename);
   ddata->len = strlen(tmp->filename);
   *tmp_info = tmp;
   return EINA_TRUE;
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

        if (!*data_ret)
          {
             ERR("Failed to allocate memory!");
             *size_ret = 0;
             return EINA_FALSE;
          }

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

static Evas *
_x11_evas_get_from_xwin(Ecore_X_Window win)
{
   /* Find the Evas connected to the window */
   Dropable *dropable = _x11_dropable_find(win);
   return dropable ? evas_object_evas_get(dropable->obj) : NULL;
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
             Evas_Coord ox, oy;

             cnp_debug("same obj dropable %p\n", dropable->obj);
             evas_object_geometry_get(dropable->obj, &ox, &oy, NULL, NULL);
             EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                if ((cbs->types & dropable->last.format) && cbs->poscb)
                  cbs->poscb(cbs->posdata, dropable->obj, x - ox, y - oy, action);
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
                     if ((cbs->types & dropable->last.format) && cbs->entercb)
                       cbs->entercb(cbs->enterdata, dropable->obj);
                  EINA_INLIST_FOREACH_SAFE(last_dropable->cbs_list, itr, cbs)
                     if ((cbs->types & last_dropable->last.format) && cbs->leavecb)
                       cbs->leavecb(cbs->leavedata, last_dropable->obj);
               }
             else // leave last obj
               {
                  cnp_debug("leave %p\n", last_dropable->obj);
                  last_dropable->last.in = EINA_FALSE;
                  last_dropable->last.type = NULL;
                  EINA_INLIST_FOREACH_SAFE(last_dropable->cbs_list, itr, cbs)
                     if ((cbs->types & last_dropable->last.format) && cbs->leavecb)
                       cbs->leavecb(cbs->leavedata, last_dropable->obj);
               }
          }
     }
   else
     {
        if (dropable) // enter new obj
          {
             Evas_Coord ox, oy;

             cnp_debug("enter %p\n", dropable->obj);
             evas_object_geometry_get(dropable->obj, &ox, &oy, NULL, NULL);
             dropable->last.in = EINA_TRUE;
             EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
               {
                  if (cbs->types & dropable->last.format)
                    {
                       if (cbs->entercb)
                          cbs->entercb(cbs->enterdata, dropable->obj);
                       if (cbs->poscb)
                          cbs->poscb(cbs->posdata, dropable->obj,
                                     x - ox, y - oy, action);
                    }
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
        Evas_Coord x, y, ox = 0, oy = 0;

        act = _x11_dnd_action_map(pos->action);
        x = pos->position.x;
        y = pos->position.y;
        _dropable_coords_adjust(dropable, &x, &y);
        Evas *evas = _x11_evas_get_from_xwin(pos->win);
        Eina_List *dropable_list = evas ? _dropable_list_geom_find(evas, x, y) : NULL;
        /* check if there is dropable (obj) can accept this drop */
        if (dropable_list)
          {
             Elm_Sel_Format saved_format = _dnd_types_to_format(savedtypes.types, savedtypes.ntypes);
             Eina_List *l;
             Eina_Bool found = EINA_FALSE;

             EINA_LIST_FOREACH(dropable_list, l, dropable)
               {
                  Dropable_Cbs *cbs;
                  Eina_Inlist *itr;
                  EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                    {
                       Elm_Sel_Format common_fmt = saved_format & cbs->types;
                       if (common_fmt)
                         {
                            /* We found a target that can accept this type of data */
                            int i, min_index = CNP_N_ATOMS;
                            /* We have to find the first atom that corresponds to one
                             * of the supported data types. */
                            for (i = 0; i < savedtypes.ntypes; i++)
                              {
                                 Cnp_Atom *atom = eina_hash_find(_types_hash, savedtypes.types[i]);
                                 if (atom && (atom->formats & common_fmt))
                                   {
                                      int atom_idx = (atom - _atoms);
                                      if (min_index > atom_idx) min_index = atom_idx;
                                   }
                              }
                            if (min_index != CNP_N_ATOMS)
                              {
                                 cnp_debug("Found atom %s\n", _atoms[min_index].name);
                                 found = EINA_TRUE;
                                 dropable->last.type = _atoms[min_index].name;
                                 dropable->last.format = common_fmt;
                                 break;
                              }
                         }
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
             eina_list_free(dropable_list);
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
   _dropable_coords_adjust(dropable, &savedtypes.x, &savedtypes.y);

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

   dropable->last.in = EINA_FALSE;
   cnp_debug("Last type: %s - Last format: %X\n", dropable->last.type, dropable->last.format);
   if ((!strcmp(dropable->last.type, text_uri)))
     {
        cnp_debug("We found a URI... (%scached) %s\n",
                  savedtypes.imgfile ? "" : "not ",
                  savedtypes.imgfile);
        if (savedtypes.imgfile)
          {
             ddata.x = savedtypes.x;
             ddata.y = savedtypes.y;
             ddata.action = act;

             EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
               {
                  /* If it's markup that also supports images */
                  if (cbs->types & ELM_SEL_FORMAT_IMAGE)
                    {
                       cnp_debug("Doing image insert (%s)\n", savedtypes.imgfile);
                       ddata.format = ELM_SEL_FORMAT_IMAGE;
                       ddata.data = (char *)savedtypes.imgfile;
                       ddata.len = strlen(ddata.data);
                       if ((cbs->types & dropable->last.format) && cbs->dropcb)
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

   cnp_debug("doing a request then: %s\n", dropable->last.type);
   _x11_selections[ELM_SEL_TYPE_XDND].xwin = drop->win;
   _x11_selections[ELM_SEL_TYPE_XDND].requestwidget = dropable->obj;
   _x11_selections[ELM_SEL_TYPE_XDND].requestformat = dropable->last.format;
   _x11_selections[ELM_SEL_TYPE_XDND].active = EINA_TRUE;
   _x11_selections[ELM_SEL_TYPE_XDND].action = act;
   ecore_x_selection_xdnd_request(drop->win, dropable->last.type);
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

static void
_x11_win_rotation_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *win = data;
   int rot = elm_win_rotation_get(event->object);
   elm_win_rotation_set(win, rot);
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
             if (dragwidget)
               {
                  if (elm_widget_is(dragwidget))
                    {
                       Evas_Object *win = elm_widget_top_get(dragwidget);
                       if (win && efl_isa(win, EFL_UI_WIN_CLASS))
                         efl_event_callback_del(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED, _x11_win_rotation_changed_cb, dragwin);
                    }
               }

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
        if (top && (efl_isa(top, EFL_UI_WIN_CLASS)))
            xwin = elm_win_xwindow_get(top);
     }
   if (!xwin)
     {
        Ecore_Evas *ee;
        Evas *evas = evas_object_evas_get(obj);
        if (!evas) return 0;
        ee = ecore_evas_ecore_evas_get(evas);
        if (!ee) return 0;

        while(!xwin)
          {
             const char *engine_name = ecore_evas_engine_name_get(ee);
             if (!strcmp(engine_name, ELM_BUFFER))
               {
                  ee = ecore_evas_buffer_ecore_evas_parent_get(ee);
                  if (!ee) return 0;
                  xwin = _elm_ee_xwin_get(ee);
               }
             else
               {
                  /* In case the engine is not a buffer, we want to check once. */
                  xwin = _elm_ee_xwin_get(ee);
                  if (!xwin) return 0;
               }
          }
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
        _atoms[i].x_atom = ecore_x_atom_get(_atoms[i].name);
        ecore_x_selection_converter_atom_add
          (_atoms[i].x_atom, _atoms[i].x_converter);
     }
   //XXX delete handlers?
   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR, _x11_selection_clear, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, _x11_selection_notify, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_FIXES_SELECTION_NOTIFY, _x11_fixes_selection_notify, NULL);
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
   if (sel->widget != obj && sel->loss_cb) sel->loss_cb(sel->loss_data, selection);
   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget, EVAS_CALLBACK_DEL,
                                         _x11_sel_obj_del, sel);
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
_x11_elm_cnp_selection_get(Ecore_X_Window xwin, const Evas_Object *obj, Elm_Sel_Type selection,
                           Elm_Sel_Format format, Elm_Drop_Cb datacb,
                           void *udata)
{
   X11_Cnp_Selection *sel;

   _x11_elm_cnp_init();

   sel = _x11_selections + selection;

   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget, EVAS_CALLBACK_DEL,
                                         _x11_sel_obj_del2, sel);
   sel->requestformat = format;
   sel->requestwidget = (Evas_Object *)obj;
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

   dropable = efl_key_data_get(obj, "__elm_dropable");
   if (!dropable)
     {
        /* Create new drop */
        dropable = calloc(1, sizeof(Dropable));
        if (!dropable) goto error;
        dropable->last.in = EINA_FALSE;
        drops = eina_list_append(drops, dropable);
        if (!drops) goto error;
        dropable->obj = obj;
        efl_key_data_set(obj, "__elm_dropable", dropable);
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
   Dropable *dropable = NULL;
   Eina_List *l;
   Ecore_X_Window xwin;
   Eina_Bool have_drops = EINA_FALSE;

   _x11_elm_cnp_init();

   if (!_local_elm_drop_target_del(obj, format, entercb, enterdata,
           leavecb, leavedata, poscb, posdata, dropcb, dropdata))
     {
        return EINA_FALSE;
     }

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
   int ex, ey, ew, eh;
   Ecore_X_Atom actx;
   int i;
   int xr, yr, rot;

   _x11_elm_cnp_init();

   cnp_debug("starting drag... %p\n", obj);

   if (dragwin)
     {
        cnp_debug("another obj is dragging...\n");
        return EINA_FALSE;
     }

   ecore_x_dnd_types_set(xwin, NULL, 0);
   for (i = CNP_ATOM_LISTING_ATOMS + 1; i < CNP_N_ATOMS; i++)
     {
        if (format == ELM_SEL_FORMAT_TARGETS || (_atoms[i].formats & format))
          {
             ecore_x_dnd_type_set(xwin, _atoms[i].name, EINA_TRUE);
             cnp_debug("set dnd type: %s\n", _atoms[i].name);
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
   dragwin = elm_win_add(NULL, "Elm-Drag", ELM_WIN_DND);
   elm_win_alpha_set(dragwin, EINA_TRUE);
   elm_win_override_set(dragwin, EINA_TRUE);
   xdragwin = _x11_elm_widget_xwin_get(dragwin);
   ecore_x_window_ignore_set(xdragwin, 1);

   /* dragwin has to be rotated as the main window is */
   if (elm_widget_is(obj))
     {
        Evas_Object *win = elm_widget_top_get(obj);
        if (win && efl_isa(win, EFL_UI_WIN_CLASS))
          {
             elm_win_rotation_set(dragwin, elm_win_rotation_get(win));
             efl_event_callback_add(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED, _x11_win_rotation_changed_cb, dragwin);
          }
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
        // need to resize
     }
   elm_win_resize_object_add(dragwin, icon);

   /* Position subwindow appropriately */
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   ecore_evas_geometry_get(ee, &ex, &ey, &ew, &eh);
   evas_object_resize(dragwin, w, h);

   evas_object_show(icon);
   evas_object_show(dragwin);
   evas_pointer_canvas_xy_get(evas_object_evas_get(obj), &x3, &y3);

   rot = ecore_evas_rotation_get(ee);
   switch (rot)
     {
      case 90:
         xr = y3;
         yr = ew - x3;
         _dragx = y3 - y2;
         _dragy = x3 - x2;
         break;
      case 180:
         xr = ew - x3;
         yr = eh - y3;
         _dragx = x3 - x2;
         _dragy = y3 - y2;
         break;
      case 270:
         xr = eh - y3;
         yr = x3;
         _dragx = y3 - y2;
         _dragy = x3 - x2;
         break;
      default:
         xr = x3;
         yr = y3;
         _dragx = x3 - x2;
         _dragy = y3 - y2;
         break;
     }
   x = ex + xr - _dragx;
   y = ey + yr - _dragy;
   evas_object_move(dragwin, x, y);
   dragwin_x_start = dragwin_x_end = x;
   dragwin_y_start = dragwin_y_end = y;

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

#ifdef HAVE_ELEMENTARY_WL2
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
   Ecore_Wl2_Window *win;
   Elm_Xdnd_Action action;
   uint32_t selection_serial;
   uint32_t drag_serial;

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
static Eina_Bool _wl_elm_cnp_selection_get(const Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format EINA_UNUSED, Elm_Drop_Cb datacb EINA_UNUSED, void *udata EINA_UNUSED);
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

static Eina_Bool _wl_dnd_receive(void *data, int type EINA_UNUSED, void *event);
static Eina_Bool _wl_dnd_end(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED);
static void _wl_dropable_data_handle(Wl_Cnp_Selection *sel, Ecore_Wl2_Event_Offer_Data_Ready *ev);

static Dropable *_wl_dropable_find(unsigned int win);
static void _wl_dropable_handle(Dropable *drop, Evas_Coord x, Evas_Coord y);
static void _wl_dropable_all_clean(unsigned int win);
static Eina_Bool _wl_drops_accept(const char *type);
static Ecore_Wl2_Window *_wl_elm_widget_window_get(const Evas_Object *obj);
static Evas * _wl_evas_get_from_win(unsigned int win);

static Eina_Bool
_wl_is_uri_type_data(const char *data, int len)
{
   char *p;
   if (len < 6) return EINA_FALSE;

   p = (char *)data;
   if (!p) return EINA_FALSE;
   if (strncmp(p, "file:/", 6))
     {
        if (*p != '/') return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_wl_targets_converter(char *target, Wl_Cnp_Selection *sel EINA_UNUSED, void *data EINA_UNUSED, int size EINA_UNUSED, void **data_ret, int *size_ret)
{
   cnp_debug("in\n");
   if (!data_ret) return EINA_FALSE;

   const char *sep = "\n";
   char *aret;
   int len = 0;
   int i = 0;
   Elm_Sel_Format formats = ELM_SEL_FORMAT_NONE;
   Eina_Bool is_uri = EINA_FALSE;

   if (sel->format)
     {
        formats = sel->format;
        is_uri = _wl_is_uri_type_data(sel->selbuf, sel->buflen);
     }
   else
     {
        Cnp_Atom *atom = eina_hash_find(_types_hash, target);
        if (atom)
          formats = atom->formats;
     }
   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        if (formats & _atoms[i].formats)
          {
             if ((is_uri) ||
                 ((!is_uri) && strcmp(_atoms[i].name, "text/uri-list")))
               len += strlen(_atoms[i].name) + strlen(sep);
          }
     }
   len++;  //terminating null byte
   aret = calloc(1, len * sizeof(char));
   if (!aret) return EINA_FALSE;
   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        if (formats & _atoms[i].formats)
          {
             if ((is_uri) ||
                 ((!is_uri) && strcmp(_atoms[i].name, "text/uri-list")))
               {
                  aret = strcat(aret, _atoms[i].name);
                  aret = strcat(aret, sep);
               }
          }
     }
   *data_ret = aret;
   if (size_ret) *size_ret = len;

   return EINA_TRUE;
}

static Eina_Bool
_wl_general_converter(char *target, Wl_Cnp_Selection *sel EINA_UNUSED, void *data, int size, void **data_ret, int *size_ret)
{
   Elm_Sel_Format formats = ELM_SEL_FORMAT_NONE;
   Cnp_Atom *atom = NULL;

   cnp_debug("in\n");

   atom = eina_hash_find(_types_hash, target);
   if (atom)
     formats = atom->formats;
   if (formats == ELM_SEL_FORMAT_NONE)
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
        if (data)
          {
             if (data_ret) *data_ret = strdup(data);
             if (size_ret) *size_ret = strlen(data);
          }
        else
          {
             if (data_ret) *data_ret = NULL;
             if (size_ret) *size_ret = 0;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_wl_text_converter(char *target, Wl_Cnp_Selection *sel, void *data, int size, void **data_ret, int *size_ret)
{
   Elm_Sel_Format formats = ELM_SEL_FORMAT_NONE;
   Cnp_Atom *atom = NULL;

   cnp_debug("in\n");

   atom = eina_hash_find(_types_hash, target);
   if (atom)
     formats = atom->formats;
   if (formats == ELM_SEL_FORMAT_NONE)
     {
        if (data_ret)
          {
             *data_ret = malloc(size * sizeof(char) + 1);
             if (!*data_ret) return EINA_FALSE;
             memcpy(*data_ret, data, size);
             ((char**)(data_ret))[0][size] = 0;
             if (size_ret) *size_ret = size;
             return EINA_TRUE;
          }
     }
   else if ((formats & ELM_SEL_FORMAT_MARKUP) ||
            (formats & ELM_SEL_FORMAT_HTML))
     {
        *data_ret = _elm_util_mkup_to_text(data);
        if (size_ret && *data_ret) *size_ret = strlen(*data_ret);
     }
   else if (formats & ELM_SEL_FORMAT_TEXT)
     {
        *data_ret = strdup(data);
        if (size_ret && *data_ret) *size_ret = strlen(*data_ret);
     }
   else if (formats & ELM_SEL_FORMAT_IMAGE)
     {
        cnp_debug("Image %s\n", evas_object_type_get(sel->widget));
        evas_object_image_file_get(elm_photocam_internal_image_get(sel->widget),
                                   (const char **)data_ret, NULL);
        if (!*data_ret) *data_ret = strdup("No file");
        else *data_ret = strdup(*data_ret);

        if (!*data_ret)
          {
             ERR("Failed to allocate memory!");
             *size_ret = 0;
             return EINA_FALSE;
          }

        if (size_ret) *size_ret = strlen(*data_ret);
     }
   return EINA_TRUE;
}

static void
_wl_selection_parser(void *_data, int size, char ***ret_data, int *ret_count)
{
   char **files = NULL;
   int num_files = 0;
   char *data = NULL;

   data = malloc(size);
   if (data && (size > 0))
     {
        int i, is;
        char *tmp;
        char **t2;

        memcpy(data, _data, size);
        if (data[size - 1])
          {
             char *t;

             /* Isn't nul terminated */
             size++;
             t = realloc(data, size);
             if (!t) goto done;
             data = t;
             data[size - 1] = 0;
          }

        tmp = malloc(size);
        if (!tmp) goto done;
        i = 0;
        is = 0;
        while ((is < size) && (data[is]))
          {
             if ((i == 0) && (data[is] == '#'))
               for (; ((data[is]) && (data[is] != '\n')); is++) ;
             else
               {
                  if ((data[is] != '\r') && (data[is] != '\n'))
                    tmp[i++] = data[is++];
                  else
                    {
                       while ((data[is] == '\r') || (data[is] == '\n'))
                         is++;
                       tmp[i] = 0;
                       num_files++;
                       t2 = realloc(files, num_files * sizeof(char *));
                       if (t2)
                         {
                            files = t2;
                            files[num_files - 1] = strdup(tmp);
                         }
                       else
                         {
                            num_files--;
                            goto freetmp;
                         }
                       tmp[0] = 0;
                       i = 0;
                    }
               }
          }
        if (i > 0)
          {
             tmp[i] = 0;
             num_files++;
             t2 = realloc(files, num_files * sizeof(char *));
             if (t2)
               {
                  files = t2;
                  files[num_files - 1] = strdup(tmp);
               }
             else
               {
                  num_files--;
                  goto freetmp;
               }
          }
freetmp:
        free(tmp);
     }
done:
   free(data);
   if (ret_data) *ret_data = files;
   else
     {
        int i;

        for (i = 0; i < num_files; i++) free(files[i]);
        free(files);
     }
   if (ret_count) *ret_count = num_files;
}

static Eina_Bool
_wl_data_preparer_markup(Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info EINA_UNUSED)
{
   cnp_debug("In\n");

   ddata->format = ELM_SEL_FORMAT_MARKUP;
   ddata->data = eina_memdup((unsigned char *)ev->data, ev->len, EINA_TRUE);
   ddata->len = ev->len;
   ddata->action = sel->action;

   return EINA_TRUE;
}

static Eina_Bool
_wl_data_preparer_uri(Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info EINA_UNUSED)
{
   char *p, *stripstr = NULL;
   char *data = ev->data;
   Dropable *drop;
   const char *type = NULL;

   cnp_debug("In\n");

   drop = efl_key_data_get(sel->requestwidget, "__elm_dropable");
   if (drop) type = drop->last.type;

   if ((type) && (!strcmp(type, "text/uri-list")))
     {
        int num_files = 0;
        char **files = NULL;
        Efreet_Uri *uri;
        Eina_Strbuf *strbuf;
        int i;

        strbuf = eina_strbuf_new();
        if (!strbuf) return EINA_FALSE;

        _wl_selection_parser(ev->data, ev->len, &files, &num_files);
        cnp_debug("got a files list\n");

        for (i = 0; i < num_files; i++)
          {
             uri = efreet_uri_decode(files[i]);
             if (uri)
               {
                  eina_strbuf_append(strbuf, uri->path);
                  efreet_uri_free(uri);
               }
             else
               {
                  eina_strbuf_append(strbuf, files[i]);
               }
             if (i < (num_files - 1))
               eina_strbuf_append(strbuf, "\n");
             free(files[i]);
          }
        free(files);
        stripstr = eina_strbuf_string_steal(strbuf);
        eina_strbuf_free(strbuf);
     }
   else
     {
        Efreet_Uri *uri;

        p = (char *)eina_memdup((unsigned char *)data, ev->len, EINA_TRUE);
        if (!p) return EINA_FALSE;
        uri = efreet_uri_decode(p);
        if (!uri)
          {
             /* Is there any reason why we care of URI without scheme? */
             if (p[0] == '/') stripstr = p;
             else free(p);
          }
        else
          {
             free(p);
             stripstr = strdup(uri->path);
             efreet_uri_free(uri);
          }
     }

   if (!stripstr)
     {
        cnp_debug("Couldn't find a file\n");
        return EINA_FALSE;
     }
   free(savedtypes.imgfile);

   ddata->data = stripstr;
   ddata->len = strlen(stripstr);
   ddata->action = sel->action;
   ddata->format = sel->requestformat;

   return EINA_TRUE;
}

static Eina_Bool
_wl_data_preparer_vcard(Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info EINA_UNUSED)
{
   cnp_debug("In\n");

   ddata->format = ELM_SEL_FORMAT_VCARD;
   ddata->data = eina_memdup((unsigned char *)ev->data, ev->len, EINA_TRUE);
   ddata->len = ev->len;
   ddata->action = sel->action;

   return EINA_TRUE;
}

static Eina_Bool
_wl_data_preparer_image(Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info)
{
   cnp_debug("In\n");
   Tmp_Info *tmp;
   int len = 0;

   tmp = _tempfile_new(ev->len);
   if (!tmp)
     return EINA_FALSE;
   memcpy(tmp->map, ev->data, ev->len);
   munmap(tmp->map, ev->len);

   len = strlen(tmp->filename);
   ddata->format = ELM_SEL_FORMAT_IMAGE;
   ddata->data = eina_memdup((unsigned char*)tmp->filename, len, EINA_TRUE);
   ddata->len = len;
   ddata->action = sel->action;
   *tmp_info = tmp;

   return EINA_TRUE;
}

static Eina_Bool
_wl_data_preparer_text(Wl_Cnp_Selection *sel, Elm_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info EINA_UNUSED)
{
   cnp_debug("In\n");

   ddata->format = ELM_SEL_FORMAT_TEXT;
   ddata->data = eina_memdup((unsigned char *)ev->data, ev->len, EINA_TRUE);
   ddata->len = ev->len;
   ddata->action = sel->action;

   return EINA_TRUE;
}

static void
_wl_sel_obj_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Wl_Cnp_Selection *sel = data;
   if (sel->widget == obj)
     {
        sel->loss_cb = NULL;
        sel->loss_data = NULL;
        sel->widget = NULL;
     }
   if (dragwidget == obj) dragwidget = NULL;
}

static Ecore_Wl2_Input *
_wl_default_seat_get(Ecore_Wl2_Window *win, Evas_Object *obj)
{
   Eo *seat, *parent2;

   // FIXME: In wayland the default seat is not named "default" but "seat-X"
   // where X may be 0 or really anything else.
   if (!obj) return ecore_wl2_display_input_find_by_name(ecore_wl2_window_display_get(win), "default");

   // FIXME (there might be a better solution):
   // In case of inwin, we want to use the main wl2 window for cnp, but obj
   // obj belongs to the buffer canvas, so the default seat for obj does not
   // match the window win.
   parent2 = elm_widget_parent2_get(elm_widget_top_get(obj));
   if (parent2) obj = elm_widget_top_get(parent2) ?: parent2;

   seat = evas_canvas_default_device_get(evas_object_evas_get(obj), EFL_INPUT_DEVICE_TYPE_SEAT);
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);
   return ecore_wl2_display_input_find(ecore_wl2_window_display_get(win),
     evas_device_seat_id_get(seat));
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
   Ecore_Wl2_Window *win;
   Wl_Cnp_Selection *sel = &wl_cnp_selection;

   if ((!selbuf) && (format != ELM_SEL_FORMAT_IMAGE))
     return elm_object_cnp_selection_clear(obj, selection);

   if (buflen <= 0)
     return EINA_FALSE;

   _wl_elm_cnp_init();

   win = _wl_elm_widget_window_get(obj);

   if ((sel->widget != obj) && sel->loss_cb)
     sel->loss_cb(sel->loss_data, selection);

   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget, EVAS_CALLBACK_DEL,
                                         _wl_sel_obj_del, &wl_cnp_selection);
   sel->active = EINA_TRUE;
   sel->seltype = selection;
   sel->widget = obj;
   sel->win = win;
   /* sel->set(win, &selection, sizeof(Elm_Sel_Type)); */
   sel->format = format;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;

   evas_object_event_callback_add
     (sel->widget, EVAS_CALLBACK_DEL, _wl_sel_obj_del, &wl_cnp_selection);

   if (selbuf)
     {
        int i = 0, count = 0;
        Eina_Bool is_uri = EINA_FALSE;
        const char **types;

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
             free(sel->selbuf);
             sel->buflen = buflen;
             sel->selbuf = strdup((char*)selbuf);
          }

        is_uri = _wl_is_uri_type_data(selbuf, buflen);
        types = malloc(sizeof(char *));
        if (!types) return EINA_FALSE;
        for (i = 0, count = 1; i < CNP_N_ATOMS; i++)
          {
             if (format & _atoms[i].formats)
               {
                  if ((is_uri) ||
                      ((!is_uri) && strcmp(_atoms[i].name, "text/uri-list")))
                    {
                       const char **t = NULL;

                       types[count - 1] = _atoms[i].name;
                       count++;
                       t = realloc(types, sizeof(char *) * count);
                       if (!t)
                         {
                            free(types);
                            return EINA_FALSE;
                         }
                       types = t;
                    }
               }
          }
        types[count - 1] = 0;

        sel->selection_serial = ecore_wl2_dnd_selection_set(_wl_default_seat_get(win, obj), types);

        free(types);
        return EINA_TRUE;
     }
   else
     {
        sel->selbuf = NULL;
     }

   return EINA_FALSE;
}


static Elm_Xdnd_Action
_wl_to_elm(Ecore_Wl2_Drag_Action action)
{
   #define CONV(wl, elm) if (action == wl) return elm;
   CONV(ECORE_WL2_DRAG_ACTION_COPY, ELM_XDND_ACTION_COPY);
   CONV(ECORE_WL2_DRAG_ACTION_MOVE, ELM_XDND_ACTION_MOVE);
   CONV(ECORE_WL2_DRAG_ACTION_ASK, ELM_XDND_ACTION_ASK);
   #undef CONV
   return ELM_XDND_ACTION_UNKNOWN;
}

typedef struct {
   Ecore_Wl2_Offer *offer;

   Evas_Object *requestwidget;
   Elm_Drop_Cb drop_cb;
   void *drop_cb_data;
   Elm_Sel_Format format;

   Ecore_Event_Handler *handler;
} Selection_Ready;

static void
_wl_selection_receive_timeout(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Selection_Ready *ready = data;

   if (ready->requestwidget != obj) return;

   ecore_event_handler_del(ready->handler);
   free(ready);
}

static Eina_Bool
_wl_selection_receive(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Offer_Data_Ready *ev = event;
   Selection_Ready *ready = data;

   if (ready->offer != ev->offer) return ECORE_CALLBACK_PASS_ON;

   if (ready->drop_cb)
     {
        Elm_Selection_Data sdata;

        sdata.x = sdata.y = 0;
        sdata.format = ready->format;
        sdata.data = ev->data;
        sdata.len = ev->len;
        sdata.action = _wl_to_elm(ecore_wl2_offer_action_get(ready->offer));
        ready->drop_cb(ready->drop_cb_data,
                       ready->requestwidget,
                       &sdata);
     }
   else
     {
        char *stripstr, *mkupstr;

        stripstr = malloc(ev->len + 1);
        if (!stripstr) return ECORE_CALLBACK_CANCEL;
        strncpy(stripstr, (char *)ev->data, ev->len);
        stripstr[ev->len] = '\0';
        mkupstr = _elm_util_text_to_mkup((const char *)stripstr);
        /* TODO BUG: should never NEVER assume it's an elm_entry! */
        _elm_entry_entry_paste(ready->requestwidget, mkupstr);
        free(stripstr);
        free(mkupstr);
     }

   evas_object_event_callback_del_full(ready->requestwidget,
                                        EVAS_CALLBACK_DEL,
                                        _wl_selection_receive_timeout, ready);

   ecore_event_handler_del(ready->handler);
   free(data);
   return ECORE_CALLBACK_CANCEL;
}

typedef struct _Format_Translation{
  Elm_Sel_Format format;
  char **translates;
}Format_Translation;

char *markup[] = {"application/x-elementary-markup", "", NULL};
char *text[] = {"text/plain;charset=utf-8", "text/plain", NULL};
char *html[] = {"text/html;charset=utf-8", "text/html", NULL};
char *vcard[] = {"text/x-vcard", NULL};
char *image[] = {"image/", "text/uri-list", NULL};

Format_Translation convertion[] = {
  {ELM_SEL_FORMAT_MARKUP, markup},
  {ELM_SEL_FORMAT_TEXT, text},
  {ELM_SEL_FORMAT_HTML, html},
  {ELM_SEL_FORMAT_VCARD, vcard},
  {ELM_SEL_FORMAT_IMAGE, image},
  {ELM_SEL_FORMAT_NONE, NULL},
};

static Eina_Bool
_wl_elm_cnp_selection_get(const Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, Elm_Drop_Cb datacb, void *udata)
{
   Ecore_Wl2_Window *win;
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Offer *offer;
   int i = 0;
   _wl_elm_cnp_init();

   win = _wl_elm_widget_window_get(obj);

   if (selection == ELM_SEL_TYPE_XDND) return EINA_FALSE;

   input = _wl_default_seat_get(win, (void*)obj);
   offer = ecore_wl2_dnd_selection_get(input);

   //there can be no selection available
   if (!offer) return EINA_FALSE;

   for (i = 0; convertion[i].translates; i++)
     {
       int j = 0;
       if (!(format & convertion[i].format)) continue;

       for (j = 0; convertion[i].translates[j]; j++)
         {
            if (!ecore_wl2_offer_supports_mime(offer, convertion[i].translates[j])) continue;

            //we have found mathing mimetypes
            Selection_Ready *ready;

            ready = calloc(1, sizeof(Selection_Ready));

            ready->requestwidget = (Evas_Object *) obj;
            ready->drop_cb = datacb;
            ready->drop_cb_data = udata;
            ready->offer = offer;
            ready->format = convertion[i].format;

            evas_object_event_callback_add(ready->requestwidget, EVAS_CALLBACK_DEL,
                                           _wl_selection_receive_timeout, ready);
            ready->handler = ecore_event_handler_add(ECORE_WL2_EVENT_OFFER_DATA_READY, _wl_selection_receive, ready);

            ecore_wl2_offer_receive(offer, (char*)convertion[i].translates[j]);
            return EINA_TRUE;
         }
     }

   return EINA_FALSE;
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
     evas_object_event_callback_del_full(sel->widget, EVAS_CALLBACK_DEL,
                                         _wl_sel_obj_del, &wl_cnp_selection);
   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget, EVAS_CALLBACK_DEL,
                                         _wl_sel_obj_del2, &wl_cnp_selection);

   sel->widget = NULL;
   sel->requestwidget = NULL;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;

   sel->active = EINA_FALSE;
   ELM_SAFE_FREE(sel->selbuf, free);
   sel->buflen = 0;
   /* sel->clear(); */
   sel->selection_serial = ecore_wl2_dnd_selection_clear(_wl_default_seat_get(_wl_elm_widget_window_get(obj), obj));

   return EINA_TRUE;
}

static void
_wl_selection_changed_free(void *data, void *ev EINA_UNUSED)
{
   ecore_wl2_display_disconnect(data);
}

static Eina_Bool
_wl_selection_changed(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Elm_Cnp_Event_Selection_Changed *e;
   Ecore_Wl2_Event_Seat_Selection *ev = event;
   Ecore_Wl2_Input *seat;

   seat = ecore_wl2_display_input_find(ev->display, ev->seat);
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, ECORE_CALLBACK_RENEW);
   e = calloc(1, sizeof(Elm_Cnp_Event_Selection_Changed));
   EINA_SAFETY_ON_NULL_RETURN_VAL(e, ECORE_CALLBACK_RENEW);
   e->type = ELM_SEL_TYPE_CLIPBOARD;
   e->seat_id = ev->seat;
   /* connect again to add ref */
   e->display = ecore_wl2_display_connect(ecore_wl2_display_name_get(ev->display));
   e->exists = !!ecore_wl2_dnd_selection_get(seat);
   ecore_event_add(ELM_CNP_EVENT_SELECTION_CHANGED, e, _wl_selection_changed_free, ev->display);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_wl_selection_send(void *data, int type EINA_UNUSED, void *event)
{
   char *buf;
   int ret, len_remained;
   int len_written = 0;
   Wl_Cnp_Selection *sel;
   Ecore_Wl2_Event_Data_Source_Send *ev;
   void *data_ret = NULL;
   int len_ret = 0;
   int i = 0;

   _wl_elm_cnp_init();
   cnp_debug("In\n");
   ev = event;
   sel = data;
   if ((ev->serial != sel->selection_serial) &&
       (ev->serial != sel->drag_serial))
    return ECORE_CALLBACK_RENEW;

   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        if (!strcmp(_atoms[i].name, ev->type))
          {
             cnp_debug("Found a type: %s\n", _atoms[i].name);
             Dropable *drop;
             drop = efl_key_data_get(sel->requestwidget, "__elm_dropable");
             if (drop)
               drop->last.type = _atoms[i].name;
             if (_atoms[i].wl_converter)
               {
                  _atoms[i].wl_converter(ev->type, sel, sel->selbuf,
                                         sel->buflen, &data_ret, &len_ret);
               }
             else
               {
                  data_ret = strdup(sel->selbuf);
                  len_ret = sel->buflen;
               }
             break;
          }
     }

   len_remained = len_ret;
   buf = data_ret;
   cnp_debug("write: %s", buf);

   while (len_written < len_ret)
     {
        ret = write(ev->fd, buf, len_remained);
        if (ret == -1) break;
        buf += ret;
        len_written += ret;
        len_remained -= ret;
     }
   free(data_ret);

   close(ev->fd);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_elm_cnp_init(void)
{
   static int _init_count = 0;

   if (_init_count > 0) return EINA_TRUE;
   _init_count++;

   ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_SEND,
                           _wl_selection_send, &wl_cnp_selection);
   ecore_event_handler_add(ECORE_WL2_EVENT_SEAT_SELECTION,
                           _wl_selection_changed, NULL);
   return EINA_TRUE;
}

static Eina_Bool
_wl_elm_dnd_init(void)
{
   static int _init_count = 0;

   if (_init_count > 0) return EINA_TRUE;
   _init_count++;

   text_uri = eina_stringshare_add("text/uri-list");

   _wl_elm_cnp_init();
   ecore_event_handler_add(ECORE_WL2_EVENT_DATA_SOURCE_END,
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

   dropable = efl_key_data_get(obj, "__elm_dropable");
   if (!dropable)
     {
        /* Create new drop */
        dropable = calloc(1, sizeof(Dropable));
        if (!dropable) goto error;
        drops = eina_list_append(drops, dropable);
        if (!drops) goto error;
        dropable->obj = obj;
        efl_key_data_set(obj, "__elm_dropable", dropable);
     }

   dropable->cbs_list =
     eina_inlist_append(dropable->cbs_list, EINA_INLIST_GET(cbs));

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _all_drop_targets_cbs_del, obj);

   if (first)
     {
        handler_enter =
          ecore_event_handler_add(ECORE_WL2_EVENT_DND_ENTER,
                                  _wl_dnd_enter, NULL);
        handler_leave =
          ecore_event_handler_add(ECORE_WL2_EVENT_DND_LEAVE,
                                  _wl_dnd_leave, NULL);
        handler_pos =
          ecore_event_handler_add(ECORE_WL2_EVENT_DND_MOTION,
                                  _wl_dnd_position, NULL);
        handler_drop =
          ecore_event_handler_add(ECORE_WL2_EVENT_DND_DROP,
                                  _wl_dnd_drop, NULL);
        _wl_elm_dnd_init();
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

   if (!_local_elm_drop_target_del(obj, format, entercb, enterdata,
           leavecb, leavedata, poscb, posdata, dropcb, dropdata))
     {
        return EINA_FALSE;
     }

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
_wl_elm_drag_start(Evas_Object *obj, Elm_Sel_Format format, const char *data,
                   Elm_Xdnd_Action action,
                   Elm_Drag_Icon_Create_Cb createicon, void *createdata,
                   Elm_Drag_Pos dragpos, void *dragdata,
                   Elm_Drag_Accept acceptcb, void *acceptdata,
                   Elm_Drag_State dragdone, void *donecbdata)
{
   Ecore_Evas *ee;
   Evas_Object *icon = NULL;
   int x, y, x2 = 0, y2 = 0, x3, y3, w = 0, h = 0;
   const char *types[CNP_N_ATOMS + 1];
   int i, nb_types = 0;
   Ecore_Wl2_Window *parent = NULL, *win;

   _wl_elm_dnd_init();

   /* if we already have a drag, get out */
   if (dragwin) return EINA_FALSE;

   for (i = CNP_ATOM_LISTING_ATOMS + 1; i < CNP_N_ATOMS; i++)
     {
        if (format == ELM_SEL_FORMAT_TARGETS || (_atoms[i].formats & format))
          {
             types[nb_types++] = _atoms[i].name;
             cnp_debug("set dnd type: %s\n", _atoms[i].name);
          }
     }
   types[nb_types] = NULL;

   win = _wl_elm_widget_window_get(obj);
   ecore_wl2_dnd_drag_types_set(_wl_default_seat_get(win, obj), types);

   /* set the drag data used when a drop occurs */
   free(wl_cnp_selection.selbuf);
   wl_cnp_selection.buflen = 0;
   wl_cnp_selection.selbuf = eina_strdup(data);

   if (data)
     {
        wl_cnp_selection.buflen = strlen(wl_cnp_selection.selbuf);
     }

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

   win = elm_win_wl_window_get(dragwin);

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

   if (elm_widget_is(obj))
     {
        Evas_Object *top;

        top = elm_widget_top_get(obj);
        if (!top) top = elm_widget_top_get(elm_widget_parent_widget_get(obj));
        if (top && (efl_isa(top, EFL_UI_WIN_CLASS)))
          parent = elm_win_wl_window_get(top);
     }
   if (!parent)
     {
        Evas *evas;

        if (!(evas = evas_object_evas_get(obj)))
          return EINA_FALSE;
        if (!(ee = ecore_evas_ecore_evas_get(evas)))
          return EINA_FALSE;

        parent = ecore_evas_wayland2_window_get(ee);
     }

   wl_cnp_selection.drag_serial = ecore_wl2_dnd_drag_start(_wl_default_seat_get(win, obj), parent, win);

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
   Ecore_Wl2_Event_Dnd_Enter *ev;
   Eina_Array *known, *available;
   unsigned int i = 0;

   ev = event;

   available = ecore_wl2_offer_mimes_get(ev->offer);

   free(savedtypes.types);

   savedtypes.ntypes = eina_array_count(available);
   savedtypes.types = malloc(sizeof(char *) * savedtypes.ntypes);
   if (!savedtypes.types) return EINA_FALSE;

   known = eina_array_new(5);

   for (i = 0; i < eina_array_count(available); i++)
     {
        savedtypes.types[i] =
          eina_stringshare_add(eina_array_data_get(available, i));
        if (savedtypes.types[i] == text_uri)
          {
             savedtypes.textreq = 1;
             ELM_SAFE_FREE(savedtypes.imgfile, free);
          }
     }

   doaccept = EINA_FALSE;
   for (i = 0; i < eina_array_count(available); i++)
     {
        if (_wl_drops_accept(eina_array_data_get(available, i)))
          {
             eina_array_push(known, strdup(eina_array_data_get(available, i)));
          }
     }

   ecore_wl2_offer_mimes_set(ev->offer, known);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_leave(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Dnd_Leave *ev;
   Dropable *drop;
   cnp_debug("In\n");

   ev = event;
   if ((drop = _wl_dropable_find(ev->win)))
     {
        _wl_dropable_handle(NULL, 0, 0);
        _wl_dropable_all_clean(ev->win);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_position(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Dnd_Motion *ev;
   Dropable *drop;
   Eina_Bool will_accept = EINA_FALSE;

   ev = event;

   cnp_debug("mouse pos %i %i\n", ev->x, ev->y);

   dragwin_x_end = ev->x - _dragx;
   dragwin_y_end = ev->y - _dragy;

   drop = _wl_dropable_find(ev->win);

   if (drop)
     {
        Evas_Coord x = 0, y = 0;
        Evas *evas = NULL;
        Eina_List *dropable_list = NULL;

        x = ev->x;
        y = ev->y;
        _dropable_coords_adjust(drop, &x, &y);
        evas = _wl_evas_get_from_win(ev->win);
        if (evas)
          dropable_list = _dropable_list_geom_find(evas, x, y);

        /* check if there is dropable (obj) can accept this drop */
        if (dropable_list)
          {
             Elm_Sel_Format saved_format;
             Eina_List *l;
             Eina_Bool found = EINA_FALSE;
             Dropable *dropable = NULL;

             saved_format =
               _dnd_types_to_format(savedtypes.types, savedtypes.ntypes);

             EINA_LIST_FOREACH(dropable_list, l, dropable)
               {
                  Dropable_Cbs *cbs;
                  Eina_Inlist *itr;

                  EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                    {
                       Elm_Sel_Format common_fmt = saved_format & cbs->types;

                       if (common_fmt)
                         {
                            /* We found a target that can accept this type of data */
                            int i, min_index = CNP_N_ATOMS;

                            /* We have to find the first atom that corresponds to one
                             * of the supported data types. */
                            for (i = 0; i < savedtypes.ntypes; i++)
                              {
                                 Cnp_Atom *atom;

                                 atom = eina_hash_find(_types_hash,
                                                       savedtypes.types[i]);
                                 if (atom && (atom->formats & common_fmt))
                                   {
                                      int atom_idx = (atom - _atoms);

                                      if (min_index > atom_idx)
                                        min_index = atom_idx;
                                   }
                              }
                            if (min_index != CNP_N_ATOMS)
                              {
                                 cnp_debug("Found atom %s\n", _atoms[min_index].name);
                                 found = EINA_TRUE;
                                 dropable->last.type = _atoms[min_index].name;
                                 dropable->last.format = common_fmt;
                                 break;
                              }
                         }
                    }
                  if (found) break;
               }
             if (found)
               {
                  Evas_Coord ox = 0, oy = 0;

                  evas_object_geometry_get(dropable->obj, &ox, &oy, NULL, NULL);

                  cnp_debug("Candidate %p (%s)\n",
                        dropable->obj, efl_class_name_get(efl_class_get(dropable->obj)));
                  _wl_dropable_handle(dropable, x - ox, y - oy);
                  wl_cnp_selection.requestwidget = dropable->obj;
                  will_accept = EINA_TRUE;
               }
             else
               {
                  //if not: send false status
                  cnp_debug("dnd position (%d, %d) not in obj\n", x, y);
                  _wl_dropable_handle(NULL, 0, 0);
                  // CCCCCCC: call dnd exit on last obj
               }
             eina_list_free(dropable_list);
          }
     }

   doaccept = will_accept;

   if (dragacceptcb)
     dragacceptcb(dragacceptdata, wl_cnp_selection.requestwidget, will_accept);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_receive(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Offer_Data_Ready *ev;
   Ecore_Wl2_Offer *offer;
   cnp_debug("In\n");

   ev = event;
   offer = data;

   if (offer != ev->offer) return ECORE_CALLBACK_PASS_ON;

   if (wl_cnp_selection.requestwidget)
     {
           Ecore_Wl2_Drag_Action action;

           action = ecore_wl2_offer_action_get(ev->offer);
           if (action == ECORE_WL2_DRAG_ACTION_ASK)
             ecore_wl2_offer_actions_set(ev->offer, ECORE_WL2_DRAG_ACTION_COPY, ECORE_WL2_DRAG_ACTION_COPY);
           action = ecore_wl2_offer_action_get(ev->offer);

           wl_cnp_selection.action = _wl_to_elm(action);

           _wl_dropable_data_handle(&wl_cnp_selection, ev);
           evas_object_event_callback_del_full(wl_cnp_selection.requestwidget,
                                               EVAS_CALLBACK_DEL,
                                               _wl_sel_obj_del2, &wl_cnp_selection);
           wl_cnp_selection.requestwidget = NULL;

     }

   ecore_wl2_offer_finish(ev->offer);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_wl_dnd_drop(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Dnd_Drop *ev;
   Ecore_Wl2_Window *win;
   Dropable *drop;
   Eina_List *l;

   cnp_debug("In\n");
   ev = event;
   savedtypes.x = ev->x;
   savedtypes.y = ev->y;

   EINA_LIST_FOREACH(drops, l, drop)
     {
        if (drop->last.in)
          {
             cnp_debug("Request data of type %s\n", drop->last.type);
             wl_cnp_selection.requestwidget = drop->obj;
             wl_cnp_selection.requestformat = drop->last.format;
             evas_object_event_callback_add(wl_cnp_selection.requestwidget,
                                            EVAS_CALLBACK_DEL, _wl_sel_obj_del2,
                                            &wl_cnp_selection);

             win = _wl_elm_widget_window_get(drop->obj);
             ecore_wl2_offer_receive(ev->offer, (char*)drop->last.type);
             ecore_event_handler_add(ECORE_WL2_EVENT_OFFER_DATA_READY, _wl_dnd_receive, ev->offer);

             return ECORE_CALLBACK_PASS_ON;
          }
     }

   win = ecore_wl2_display_window_find(_elm_wl_display, ev->win);
   ecore_wl2_dnd_drag_end(_wl_default_seat_get(win, NULL));
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_wl_dnd_end(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Data_Source_End *ev;
   Ecore_Wl2_Window *win;

   cnp_debug("In\n");

   ev = event;
   if (ev->serial != wl_cnp_selection.drag_serial)
    return ECORE_CALLBACK_RENEW;
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

   win = ecore_wl2_display_window_find(_elm_wl_display, ev->win);
   ecore_wl2_input_ungrab(_wl_default_seat_get(win, NULL));

   return ECORE_CALLBACK_PASS_ON;
}

static void
_wl_dropable_data_handle(Wl_Cnp_Selection *sel, Ecore_Wl2_Event_Offer_Data_Ready *ev)
{
   Dropable *drop;
   Ecore_Wl2_Window *win;

   cnp_debug("In\n");
   drop = efl_key_data_get(sel->requestwidget, "__elm_dropable");
   if (drop)
     {
        Cnp_Atom *atom = NULL;

        atom = eina_hash_find(_types_hash, drop->last.type);
        if (atom && atom->wl_data_preparer)
          {
             Elm_Selection_Data ddata;
             Tmp_Info *tmp_info = NULL;
             Eina_Bool success;
             ddata.data = NULL;

             cnp_debug("Call notify for: %s\n", atom->name);
             success = atom->wl_data_preparer(sel, &ddata, ev, &tmp_info);
             if (success)
               {
                  Dropable *dropable;
                  Eina_List *l;

                  EINA_LIST_FOREACH(drops, l, dropable)
                    {
                       if (dropable->obj == sel->requestwidget) break;
                       dropable = NULL;
                    }
                  if (dropable)
                    {
                       cnp_debug("call dropcb\n");
                       Dropable_Cbs *cbs;
                       Eina_Inlist *itr;
                       ddata.x = savedtypes.x;
                       ddata.y = savedtypes.y;
                       EINA_INLIST_FOREACH_SAFE(dropable->cbs_list, itr, cbs)
                         if ((cbs->types & dropable->last.format) &&
                             cbs->dropcb)
                           cbs->dropcb(cbs->dropdata, dropable->obj, &ddata);
                    }
               }
             win = _wl_elm_widget_window_get(sel->requestwidget);
             ecore_wl2_dnd_drag_end(_wl_default_seat_get(win, NULL));
             if (tmp_info) _tmpinfo_free(tmp_info);
             free(ddata.data);
             return;
          }
     }

   win = _wl_elm_widget_window_get(sel->requestwidget);
   ecore_wl2_dnd_drag_end(_wl_default_seat_get(win, NULL));
   savedtypes.textreq = 0;
}

static Dropable *
_wl_dropable_find(unsigned int win)
{
   Eina_List *l;
   Dropable *dropable;
   Ecore_Wl2_Window *window;

   if (!drops) return NULL;

   window = ecore_wl2_display_window_find(_elm_wl_display, win);
   if (!window) return NULL;

   EINA_LIST_FOREACH(drops, l, dropable)
     if (_wl_elm_widget_window_get(dropable->obj) == window)
       return dropable;

   return NULL;
}

static Evas *
_wl_evas_get_from_win(unsigned int win)
{
   Dropable *dropable = _wl_dropable_find(win);
   return dropable ? evas_object_evas_get(dropable->obj) : NULL;
}

static void
_wl_dropable_handle(Dropable *drop, Evas_Coord x, Evas_Coord y)
{
   Dropable *d, *last_dropable = NULL;
   Dropable_Cbs *cbs;
   Eina_Inlist *itr;
   Eina_List *l;

   EINA_LIST_FOREACH(drops, l, d)
     {
        if (d->last.in)
          {
             last_dropable = d;
             break;
          }
     }

   /* If we are on the same object, just update the position */
   if ((drop) && (last_dropable == drop))
     {
        EINA_INLIST_FOREACH_SAFE(drop->cbs_list, itr, cbs)
           if (cbs->poscb)
              cbs->poscb(cbs->posdata, drop->obj, x, y, dragaction);
        return;
     }
   /* We leave the last dropable */
   if (last_dropable)
     {
        EINA_INLIST_FOREACH_SAFE(last_dropable->cbs_list, itr, cbs)
           if (cbs->leavecb)
              cbs->leavecb(cbs->leavedata, last_dropable->obj);
        last_dropable->last.in = EINA_FALSE;
     }
   /* We enter the new dropable */
   if (drop)
     {
        EINA_INLIST_FOREACH_SAFE(drop->cbs_list, itr, cbs)
           if (cbs->entercb)
              cbs->entercb(cbs->enterdata, drop->obj);
        EINA_INLIST_FOREACH_SAFE(drop->cbs_list, itr, cbs)
           if (cbs->poscb)
              cbs->poscb(cbs->posdata, drop->obj, x, y, dragaction);
        drop->last.in = EINA_TRUE;
     }
}

static void
_wl_dropable_all_clean(unsigned int win)
{
   Eina_List *l;
   Dropable *dropable;
   Ecore_Wl2_Window *window;

   window = ecore_wl2_display_window_find(_elm_wl_display, win);
   if (!window) return;

   EINA_LIST_FOREACH(drops, l, dropable)
     {
        if (_wl_elm_widget_window_get(dropable->obj) == window)
          {
             dropable->last.x = 0;
             dropable->last.y = 0;
             dropable->last.in = EINA_FALSE;
          }
     }
}

static Eina_Bool
_wl_drops_accept(const char *type)
{
   Eina_List *l;
   Dropable *drop;
   Eina_Bool will_accept = EINA_FALSE;

   if (!type) return EINA_FALSE;

   EINA_LIST_FOREACH(drops, l, drop)
     {
        Dropable_Cbs *cbs;
        EINA_INLIST_FOREACH(drop->cbs_list, cbs)
          {
             for (int i = 0; convertion[i].translates ; ++i)
               {
                  if (!(convertion[i].format & cbs->types)) continue;

                  for (int j = 0; convertion[i].translates[j]; ++j)
                    {
                       if (!strncmp(type, convertion[i].translates[j], strlen(convertion[i].translates[j])))
                         {
                            wl_cnp_selection.requestwidget = drop->obj;
                            return EINA_TRUE;
                         }
                    }
               }
          }
     }

   return will_accept;
}

static Ecore_Wl2_Window *
_wl_elm_widget_window_get(const Evas_Object *obj)
{
   Evas_Object *top;
   Ecore_Wl2_Window *win = NULL;

   if (elm_widget_is(obj))
     {
        top = elm_widget_top_get(obj);
        if (!top) top = elm_widget_top_get(elm_widget_parent_widget_get(obj));
        if (top && (efl_isa(top, EFL_UI_WIN_CLASS)))
            win = elm_win_wl_window_get(top);
     }
   if (!win)
     {
        Ecore_Evas *ee;
        Evas *evas;
        const char *engine_name;

        if (!(evas = evas_object_evas_get(obj)))
          return NULL;
        if (!(ee = ecore_evas_ecore_evas_get(evas)))
          return NULL;

        engine_name = ecore_evas_engine_name_get(ee);
        if (!strcmp(engine_name, ELM_BUFFER))
          {
             ee = ecore_evas_buffer_ecore_evas_parent_get(ee);
             if (!ee) return NULL;
             engine_name = ecore_evas_engine_name_get(ee);
          }
        if (!strncmp(engine_name, "wayland", sizeof("wayland") - 1))
          {
             /* In case the engine is not a buffer, we want to check once. */
             win = ecore_evas_wayland2_window_get(ee);
             if (!win) return NULL;
          }
     }

   return win;
}

#endif

#ifdef HAVE_ELEMENTARY_COCOA

typedef struct _Cocoa_Cnp_Selection Cocoa_Cnp_Selection;

struct _Cocoa_Cnp_Selection
{
   Ecore_Cocoa_Window *win;
   Evas_Object        *widget;
   char               *selbuf;
   int                 buflen;
   Evas_Object        *requestwidget;
   void               *udata;
   Elm_Sel_Format      requestformat;
   Elm_Drop_Cb         datacb;

   Elm_Selection_Loss_Cb  loss_cb;
   void                  *loss_data;
   int                    pb_count;

   Elm_Sel_Format     format;

   Eina_Bool          active : 1;
};

static Cocoa_Cnp_Selection _cocoa_cnp_sel;

static Ecore_Cocoa_Window *
_cocoa_elm_widget_cocoa_window_get(const Evas_Object *obj)
{
   Evas_Object *top, *par;
   Ecore_Cocoa_Window *win = NULL;

   if (elm_widget_is(obj))
     {
         top = elm_widget_top_get(obj);
         if (!top)
           {
              par = elm_widget_parent_widget_get(obj);
              if (par) top = elm_widget_top_get(par);
           }
         if ((top) && (efl_isa(top, EFL_UI_WIN_CLASS)))
           win = elm_win_cocoa_window_get(top);
     }
   if (!win)
     {
        // FIXME
        CRI("WIN has not been retrieved!!!");
     }

   return win;
}

static Ecore_Cocoa_Cnp_Type
_elm_sel_format_to_ecore_cocoa_cnp_type(Elm_Sel_Format fmt)
{
   Ecore_Cocoa_Cnp_Type type = 0;

   if ((fmt & ELM_SEL_FORMAT_TEXT) ||
       (fmt & ELM_SEL_FORMAT_VCARD))
     type |= ECORE_COCOA_CNP_TYPE_STRING;
   if (fmt & ELM_SEL_FORMAT_MARKUP)
     type |= ECORE_COCOA_CNP_TYPE_MARKUP;
   if (fmt & ELM_SEL_FORMAT_HTML)
     type |= ECORE_COCOA_CNP_TYPE_HTML;
   if (fmt & ELM_SEL_FORMAT_IMAGE)
     type |= ECORE_COCOA_CNP_TYPE_IMAGE;

   return type;
}

static void
_cocoa_sel_obj_del_req_cb(void        *data,
                          Evas        *e       EINA_UNUSED,
                          Evas_Object *obj,
                          void        *ev_info EINA_UNUSED)
{
   Cocoa_Cnp_Selection *sel = data;
   if (sel->requestwidget == obj) sel->requestwidget = NULL;
}

static void
_cocoa_sel_obj_del_cb(void        *data,
                      Evas        *e       EINA_UNUSED,
                      Evas_Object *obj,
                      void        *ev_info EINA_UNUSED)
{
   Cocoa_Cnp_Selection *sel = data;
   if (sel->widget == obj)
     {
        sel->widget = NULL;
        sel->loss_cb = NULL;
        sel->loss_data = NULL;
     }
   if (dragwidget == obj) dragwidget = NULL;
}

static void
_job_pb_cb(void *data)
{
   Cocoa_Cnp_Selection *sel = data;
   Elm_Selection_Data ddata;
   Ecore_Cocoa_Cnp_Type type, get_type;
   void *pbdata;
   int pbdata_len;

   if (sel->datacb)
     {
        ddata.x = 0;
        ddata.y = 0;

        /* Pass to cocoa clipboard */
        type = _elm_sel_format_to_ecore_cocoa_cnp_type(sel->requestformat);
        pbdata = ecore_cocoa_clipboard_get(&pbdata_len, type, &get_type);

        ddata.format = ELM_SEL_FORMAT_NONE;
        if (get_type & ECORE_COCOA_CNP_TYPE_STRING)
          ddata.format |= ELM_SEL_FORMAT_TEXT;
        if (get_type & ECORE_COCOA_CNP_TYPE_MARKUP)
          ddata.format |= ELM_SEL_FORMAT_MARKUP;
        if (get_type & ECORE_COCOA_CNP_TYPE_IMAGE)
          ddata.format |= ELM_SEL_FORMAT_IMAGE;
        if (get_type & ECORE_COCOA_CNP_TYPE_HTML)
          ddata.format |= ELM_SEL_FORMAT_HTML;

        ddata.data = pbdata;
        ddata.len = pbdata_len;
        ddata.action = ELM_XDND_ACTION_UNKNOWN;
        sel->datacb(sel->udata, sel->requestwidget, &ddata);
        free(pbdata);
     }
}

static Eina_Bool
_cocoa_elm_cnp_selection_set(Ecore_Cocoa_Window *win,
                             Evas_Object        *obj,
                             Elm_Sel_Type        selection,
                             Elm_Sel_Format      format,
                             const void         *selbuf,
                             size_t              buflen)
{
   Cocoa_Cnp_Selection *sel = &_cocoa_cnp_sel;
   Ecore_Cocoa_Cnp_Type type;
   Eina_Bool ok = EINA_TRUE;

   if ((!selbuf) && (format != ELM_SEL_FORMAT_IMAGE))
     return elm_object_cnp_selection_clear(obj, selection);
   if (buflen <= 0) return EINA_FALSE;

   if (sel->loss_cb) sel->loss_cb(sel->loss_data, selection);
   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget, EVAS_CALLBACK_DEL,
                                         _cocoa_sel_obj_del_cb, sel);

   sel->widget = obj;
   sel->win = win;
   sel->format = format;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;

   evas_object_event_callback_add(sel->widget, EVAS_CALLBACK_DEL,
                                  _cocoa_sel_obj_del_cb, sel);
   ELM_SAFE_FREE(sel->selbuf, free);
   sel->buflen = 0;
   if (selbuf)
     {
        sel->selbuf = malloc(buflen + 1);
        if (EINA_UNLIKELY(!sel->selbuf))
          {
             CRI("Failed to allocate memory!");
             elm_object_cnp_selection_clear(obj, selection);
             return EINA_FALSE;
          }
        memcpy(sel->selbuf, selbuf, buflen);
        sel->selbuf[buflen] = 0;
        sel->buflen = buflen;
        type = _elm_sel_format_to_ecore_cocoa_cnp_type(format);
        ecore_cocoa_clipboard_set(selbuf, buflen, type);
     }

   return ok;
}

static void
_cocoa_elm_cnp_selection_loss_callback_set(Evas_Object           *obj       EINA_UNUSED,
                                           Elm_Sel_Type           selection EINA_UNUSED,
                                           Elm_Selection_Loss_Cb  func      EINA_UNUSED,
                                           const void            *data      EINA_UNUSED)
{
   // Currently, we have no way to track changes in Cocoa pasteboard.
   // Therefore, don't track this...
   //sel->loss_cb = func;
   //sel->loss_data = (void *)data;
}

static Eina_Bool
_cocoa_elm_cnp_selection_clear(Evas_Object  *obj       EINA_UNUSED,
                               Elm_Sel_Type  selection EINA_UNUSED)
{
   Cocoa_Cnp_Selection *sel = &_cocoa_cnp_sel;

   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget, EVAS_CALLBACK_DEL,
                                         _cocoa_sel_obj_del_cb, sel);
   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget, EVAS_CALLBACK_DEL,
                                         _cocoa_sel_obj_del_req_cb, sel);

   sel->widget = NULL;
   sel->requestwidget = NULL;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;
   ELM_SAFE_FREE(sel->selbuf, free);
   sel->buflen = 0;
   ecore_cocoa_clipboard_clear();

   return EINA_TRUE;
}

static Eina_Bool
_cocoa_elm_cnp_selection_get(const Evas_Object  *obj,
                             Ecore_Cocoa_Window *win,
                             Elm_Sel_Type        selection EINA_UNUSED,
                             Elm_Sel_Format      format,
                             Elm_Drop_Cb         datacb,
                             void               *udata)
{
   Cocoa_Cnp_Selection *sel = &_cocoa_cnp_sel;

   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget, EVAS_CALLBACK_DEL,
                                         _cocoa_sel_obj_del_req_cb, sel);

   sel->requestformat = format;
   sel->requestwidget = (Evas_Object *)obj;
   sel->win = win;
   sel->datacb = datacb;
   sel->udata = udata;

   ecore_job_add(_job_pb_cb, sel);

   evas_object_event_callback_add(sel->requestwidget, EVAS_CALLBACK_DEL,
                                  _cocoa_sel_obj_del_req_cb, sel);

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
static Eina_Bool  _local_elm_cnp_selection_get(const Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format EINA_UNUSED, Elm_Drop_Cb datacb, void *udata);
static  Eina_Bool _local_elm_drop_target_add(Evas_Object *obj EINA_UNUSED, Elm_Sel_Format format EINA_UNUSED,
                                             Elm_Drag_State entercb EINA_UNUSED, void *enterdata EINA_UNUSED,
                                             Elm_Drag_State leavecb EINA_UNUSED, void *leavedata EINA_UNUSED,
                                             Elm_Drag_Pos poscb EINA_UNUSED, void *posdata EINA_UNUSED,
                                             Elm_Drop_Cb dropcb EINA_UNUSED, void *dropdata EINA_UNUSED);
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
   // this doesn't need to do anything as we never lose selection to anyone
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
_local_elm_cnp_selection_get(const Evas_Object *obj,
                             Elm_Sel_Type selection,
                             Elm_Sel_Format format EINA_UNUSED,
                             Elm_Drop_Cb datacb, void *udata)
{
   _local_elm_cnp_init();
   if (_local_selinfo[selection].get.job)
     ecore_job_del(_local_selinfo[selection].get.job);
   _local_selinfo[selection].get.obj = (Evas_Object *)obj;
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
_local_elm_drop_target_del(Evas_Object *obj, Elm_Sel_Format format,
                         Elm_Drag_State entercb, void *enterdata,
                         Elm_Drag_State leavecb, void *leavedata,
                         Elm_Drag_Pos poscb, void *posdata,
                         Elm_Drop_Cb dropcb, void *dropdata)
{
   Dropable *dropable = NULL;

   _local_elm_cnp_init();

   dropable = efl_key_data_get(obj, "__elm_dropable");
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
             efl_key_data_set(obj, "__elm_dropable", NULL);
             free(dropable);
             dropable = NULL;
             evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
                   _all_drop_targets_cbs_del);
          }
        if (!drops)
          {
             _elm_cnp_shutdown();
          }
        return EINA_TRUE;
     }

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

// win32 specific stuff
////////////////////////////////////////////////////////////////////////////
#ifdef HAVE_ELEMENTARY_WIN32

typedef struct _Win32_Cnp_Selection Win32_Cnp_Selection;

struct _Win32_Cnp_Selection
{
   const char            *debug;
   Evas_Object           *widget;
   char                  *selbuf;
   Evas_Object           *requestwidget;
   void                  *udata;
   Elm_Sel_Format         requestformat;
   Elm_Drop_Cb            datacb;
   Eina_Bool            (*set)(const Ecore_Win32_Window *window, const void *data, int size);
   Eina_Bool            (*clear)(const Ecore_Win32_Window *window);
   Eina_Bool            (*get)(const Ecore_Win32_Window *window , void **data, int *size);
   Elm_Selection_Loss_Cb  loss_cb;
   void                  *loss_data;

   Elm_Sel_Format         format;
   Ecore_Win32_Selection  ecore_sel;
   Ecore_Win32_Window    *win;
   Elm_Xdnd_Action        action;

   Eina_Bool              active : 1;
};

#define ARRAYINIT(foo)  [foo] =

static Win32_Cnp_Selection _win32_selections[ELM_SEL_TYPE_CLIPBOARD + 1] =
{
   ARRAYINIT(ELM_SEL_TYPE_PRIMARY)
   {
      .debug = "Primary",
      .ecore_sel = ECORE_WIN32_SELECTION_PRIMARY,
      .set = ecore_win32_clipboard_set,
      .clear = ecore_win32_clipboard_clear,
      .get = ecore_win32_clipboard_get,
   },
   ARRAYINIT(ELM_SEL_TYPE_SECONDARY)
   {
      .debug = "Secondary",
      .ecore_sel = ECORE_WIN32_SELECTION_OTHER,
   },
   ARRAYINIT(ELM_SEL_TYPE_XDND)
   {
      .debug = "XDnD",
      .ecore_sel = ECORE_WIN32_SELECTION_OTHER,
   },
   ARRAYINIT(ELM_SEL_TYPE_CLIPBOARD)
   {
      .debug = "Clipboard",
      .ecore_sel = ECORE_WIN32_SELECTION_CLIPBOARD,
      .set = ecore_win32_clipboard_set,
      .clear = ecore_win32_clipboard_clear,
      .get = ecore_win32_clipboard_get,
   }
};

static char *
_win32_text_n_to_rn(char *intext)
{
   size_t size = 0, newlines = 0;
   char *outtext = NULL, *p, *o;

   if (!intext) return NULL;
   for (p = intext; *p; p++)
     {
        if (*p == '\n') newlines++;
        size++;
     }
   outtext = malloc(size + newlines + 1);
   if (!outtext) return intext;
   for (p = intext, o = outtext; *p; p++, o++)
     {
        if (*p == '\n')
          {
             o++;
             *p = '\r';
          }
        *o = *p;
     }
   *o = '\0';
   free(intext);
   return outtext;
}

static char *
_win32_text_rn_to_n(char *intext)
{
   char *outtext = NULL, *p, *o;

   if (!intext) return NULL;
   outtext = malloc(strlen(intext) + 1);
   if (!outtext) return intext;
   for (p = intext, o = outtext; *p; p++, o++)
     {
        if ((*p == '\r') && (p[1] == '\n'))
          {
             p++;
             *p = '\n';
          }
        else *o = *p;
     }
   *o = '\0';
   free(intext);
   return outtext;
}

static void
_win32_sel_obj_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Win32_Cnp_Selection *sel = data;
   if (sel->widget == obj) sel->widget = NULL;
   if (dragwidget == obj) dragwidget = NULL;
}

static void
_win32_sel_obj_del2(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Win32_Cnp_Selection *sel = data;
   if (sel->requestwidget == obj) sel->requestwidget = NULL;
}

static Eina_Bool
_win32_elm_cnp_init(void)
{
   static int _init_count = 0;

   if (_init_count > 0) return EINA_TRUE;
   _init_count++;
   return EINA_TRUE;
}

static Ecore_Win32_Window *
_win32_elm_widget_window_get(const Evas_Object *obj)
{
   Evas_Object *top;
   Ecore_Win32_Window *win = NULL;

   if (elm_widget_is(obj))
     {
        top = elm_widget_top_get(obj);
        if (!top)
          {
             Evas_Object *par;
             par = elm_widget_parent_widget_get(obj);
             if (par) top = elm_widget_top_get(par);
          }
        if (top && (efl_isa(top, EFL_UI_WIN_CLASS)))
          win = elm_win_win32_window_get(top);
     }

   if (!win)
     {
        Ecore_Evas *ee;
        Evas *evas;
        const char *engine_name;

        evas = evas_object_evas_get(obj);
        if (!evas) return NULL;

        ee = ecore_evas_ecore_evas_get(evas);
        if (!ee) return NULL;

        engine_name = ecore_evas_engine_name_get(ee);
        if (!strcmp(engine_name, ELM_BUFFER))
          {
             ee = ecore_evas_buffer_ecore_evas_parent_get(ee);
             if (!ee) return NULL;
             win = ecore_evas_win32_window_get(ee);
          }
        else
          {
             if ((strcmp(engine_name, ELM_SOFTWARE_WIN32) == 0) ||
                 (strcmp(engine_name, ELM_SOFTWARE_DDRAW) == 0))
               return ecore_evas_win32_window_get(ee);
          }
     }

   return win;
}

static Eina_Bool
_win32_elm_cnp_selection_set(Ecore_Win32_Window *win, Evas_Object *obj, Elm_Sel_Type selection, Elm_Sel_Format format, const void *selbuf, size_t buflen)
{
   Win32_Cnp_Selection *sel;

   if (selection != ELM_SEL_TYPE_CLIPBOARD)
     return EINA_FALSE;

   _win32_elm_cnp_init();
   if ((!selbuf) && (format != ELM_SEL_FORMAT_IMAGE))
     return elm_object_cnp_selection_clear(obj, selection);

   sel = _win32_selections + selection;
   if (sel->widget != obj && sel->loss_cb) sel->loss_cb(sel->loss_data, selection);
   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget, EVAS_CALLBACK_DEL,
                                         _win32_sel_obj_del, sel);
   sel->active = EINA_TRUE;
   sel->widget = obj;
   sel->win = win;
   if (sel->set) sel->set(win, selbuf, buflen);
   sel->format = format;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;

   evas_object_event_callback_add
     (sel->widget, EVAS_CALLBACK_DEL, _win32_sel_obj_del, sel);

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
_win32_elm_cnp_selection_loss_callback_set(Evas_Object *obj EINA_UNUSED, Elm_Sel_Type selection, Elm_Selection_Loss_Cb func, const void *data)
{
   Win32_Cnp_Selection *sel;

   if (selection != ELM_SEL_TYPE_CLIPBOARD)
     return;

   _win32_elm_cnp_init();
   sel = _win32_selections + selection;
   sel->loss_cb = func;
   sel->loss_data = (void *)data;
}

static Eina_Bool
_win32_elm_object_cnp_selection_clear(Ecore_Win32_Window *win, Evas_Object *obj, Elm_Sel_Type selection)
{
   Win32_Cnp_Selection *sel;

   if (selection != ELM_SEL_TYPE_CLIPBOARD)
     return EINA_FALSE;

   _win32_elm_cnp_init();

   sel = _win32_selections + selection;

   /* No longer this selection: Consider it gone! */
   if ((!sel->active) || (sel->widget != obj))
     return EINA_TRUE;

   if (sel->widget)
     evas_object_event_callback_del_full(sel->widget, EVAS_CALLBACK_DEL,
                                         _win32_sel_obj_del, sel);
   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget, EVAS_CALLBACK_DEL,
                                         _win32_sel_obj_del2, sel);
   sel->widget = NULL;
   sel->requestwidget = NULL;
   sel->loss_cb = NULL;
   sel->loss_data = NULL;

   sel->active = EINA_FALSE;
   ELM_SAFE_FREE(sel->selbuf, free);
   /* sel->clear(win); */

   return EINA_TRUE;
}

static Eina_Bool
_win32_elm_cnp_selection_get(Ecore_Win32_Window *win,
                             const Evas_Object *obj,
                             Elm_Sel_Type selection,
                             Elm_Sel_Format format,
                             Elm_Drop_Cb datacb,
                             void *udata)
{
   Win32_Cnp_Selection *sel;
   void *data;
   int size;

   if (selection != ELM_SEL_TYPE_CLIPBOARD)
     return EINA_FALSE;

   _win32_elm_cnp_init();

   sel = _win32_selections + selection;

   if (sel->requestwidget)
     evas_object_event_callback_del_full(sel->requestwidget, EVAS_CALLBACK_DEL,
                                         _win32_sel_obj_del2, sel);
   sel->requestformat = format;
   sel->requestwidget = (Evas_Object *)obj;
   sel->win = win;
   sel->get(win, &data, &size);
   sel->datacb = datacb;
   sel->udata = udata;

   if (!data || (size <= 0))
     goto cb_add;

   if ((sel->format & ELM_SEL_FORMAT_MARKUP) ||
       (sel->format & ELM_SEL_FORMAT_HTML))
     {
        char *str;

        str = (char *)malloc(size + 1);
        if (str)
          {
             memcpy(str, data, size);
             str[size] = '\0';
             data = _win32_text_n_to_rn(_elm_util_mkup_to_text(str));
             free(str);
             if (data)
               size = strlen(data);
             else
               size = 0;
          }
        else
          {
             free(data);
             data = NULL;
          }
     }

   if (sel->datacb && data && (size > 0))
     {
        Elm_Selection_Data sdata;

        sdata.x = sdata.y = 0;
        sdata.format = ELM_SEL_FORMAT_TEXT;
        sdata.data = data;
        sdata.len = size;
        sdata.action = sel->action;
        sel->datacb(sel->udata, sel->requestwidget, &sdata);
     }

   if (data)
     free(data);

 cb_add:
   evas_object_event_callback_add
     (sel->requestwidget, EVAS_CALLBACK_DEL, _win32_sel_obj_del2, sel);

   return EINA_TRUE;
}

#endif /* HAVE_ELEMENTARY_WIN32 */

// common internal funcs
////////////////////////////////////////////////////////////////////////////
static Eina_Bool
_elm_cnp_init(void)
{
   int i;
   if (_elm_cnp_init_count > 0) return EINA_TRUE;
   _elm_cnp_init_count++;
   ELM_CNP_EVENT_SELECTION_CHANGED = ecore_event_type_new();
   text_uri = eina_stringshare_add("text/uri-list");
   _types_hash = eina_hash_string_small_new(NULL);
   for (i = 0; i < CNP_N_ATOMS; i++)
     {
        eina_hash_add(_types_hash, _atoms[i].name, &_atoms[i]);
     }
   return EINA_TRUE;
}

static Eina_Bool
_elm_cnp_shutdown(void)
{
   if (!_elm_cnp_init_count) return EINA_TRUE;
   if (--_elm_cnp_init_count > 0) return EINA_TRUE;
   ELM_CNP_EVENT_SELECTION_CHANGED = -1;
   eina_stringshare_del(text_uri);
   text_uri = NULL;
   ELM_SAFE_FREE(_types_hash, eina_hash_free);
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
#ifdef HAVE_ELEMENTARY_WL2
   if (_wl_elm_widget_window_get(obj))
      return _wl_elm_cnp_selection_set(obj, selection, format, selbuf, buflen);
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   Ecore_Cocoa_Window *win = _cocoa_elm_widget_cocoa_window_get(obj);
   if (win)
     return _cocoa_elm_cnp_selection_set(win, obj, selection, format, selbuf, buflen);
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   Ecore_Win32_Window *win;

   win = _win32_elm_widget_window_get(obj);
   if (win)
     return _win32_elm_cnp_selection_set(win, obj, selection, format, selbuf, buflen);
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
#ifdef HAVE_ELEMENTARY_WL2
   if (_wl_elm_widget_window_get(obj))
     _wl_elm_cnp_selection_loss_callback_set(obj, selection, func, data);
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   if (_cocoa_elm_widget_cocoa_window_get(obj))
     _cocoa_elm_cnp_selection_loss_callback_set(obj, selection, func, data);
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   if (_win32_elm_widget_window_get(obj))
     _win32_elm_cnp_selection_loss_callback_set(obj, selection, func, data);
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
#ifdef HAVE_ELEMENTARY_WL2
   if (_wl_elm_widget_window_get(obj))
      return _wl_elm_cnp_selection_clear(obj, selection);
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   if (_cocoa_elm_widget_cocoa_window_get(obj))
     return _cocoa_elm_cnp_selection_clear(obj, selection);
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   Ecore_Win32_Window *win;

   win = _win32_elm_widget_window_get(obj);
   if (win)
     return _win32_elm_object_cnp_selection_clear(win, obj, selection);
#endif
   return _local_elm_object_cnp_selection_clear(obj, selection);
}

EAPI Eina_Bool
elm_cnp_selection_get(const Evas_Object *obj, Elm_Sel_Type selection,
                      Elm_Sel_Format format, Elm_Drop_Cb datacb, void *udata)
{
   if (selection > ELM_SEL_TYPE_CLIPBOARD) return EINA_FALSE;
   if (!_elm_cnp_init_count) _elm_cnp_init();
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = _x11_elm_widget_xwin_get(obj);
   if (xwin)
     return _x11_elm_cnp_selection_get(xwin, obj, selection, format, datacb, udata);
#endif
#ifdef HAVE_ELEMENTARY_WL2
   if (_wl_elm_widget_window_get(obj))
      return _wl_elm_cnp_selection_get(obj, selection, format, datacb, udata);
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   Ecore_Cocoa_Window *win = _cocoa_elm_widget_cocoa_window_get(obj);
   if (win)
     return _cocoa_elm_cnp_selection_get(obj, win, selection, format, datacb, udata);
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   Ecore_Win32_Window *win;

   win = _win32_elm_widget_window_get(obj);
   if (win)
     return _win32_elm_cnp_selection_get(win, obj, selection, format, datacb, udata);
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
#ifdef HAVE_ELEMENTARY_WL2
   if (_wl_elm_widget_window_get(obj))
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
#ifdef HAVE_ELEMENTARY_WL2
   if (_wl_elm_widget_window_get(obj))
     return _wl_elm_drop_target_del(obj, format, entercb, enterdata,
           leavecb, leavedata, poscb, posdata, dropcb, dropdata);
#endif
   /* FIXME: Not the best place for an error message, but meh.
    * This code path is actually valid if running in framebuffer, but it still shouldn't
    * be getting here because the drop target shouldn't be added. This is an error
    * and it's because of some stupid handling in both the X11 and the wayland backends
    * as seen in the commit that introduced this comment.
    * Window check is probably not the best idea, you should be doing engine check instead. */
   ERR("Please contact developers, you should probably not get here.");
   return _local_elm_drop_target_del(obj, format, entercb, enterdata,
           leavecb, leavedata, poscb, posdata, dropcb, dropdata);
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
#ifdef HAVE_ELEMENTARY_WL2
   if (_wl_elm_widget_window_get(obj))
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
#ifdef HAVE_ELEMENTARY_WL2
   if (_wl_elm_widget_window_get(obj))
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
#ifdef HAVE_ELEMENTARY_WL2
   Ecore_Wl2_Window *win;

   win = _wl_elm_widget_window_get(obj);
   if (win)
     return !!ecore_wl2_dnd_selection_get(_wl_default_seat_get(win, obj));
#endif
   return _local_elm_selection_selection_has_owner(obj);
}

EAPI Eina_Bool
elm_cnp_clipboard_selection_has_owner(Evas_Object *win)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(win, EINA_FALSE);
   return elm_selection_selection_has_owner(win);
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

        evas_object_geometry_get(obj, &xo, &yo, NULL, NULL);
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

        evas_object_geometry_get(obj, &xo, &yo, NULL, NULL);
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

static void
_anim_st_free(Item_Container_Drag_Info *st)
{  /* Stops and free mem of ongoing animation */
   if (st)
     {
        ELM_SAFE_FREE(st->ea, ecore_animator_del);
        Anim_Icon *sti;
        Eo *icon;

        EINA_LIST_FREE(st->icons, sti)
          {
             evas_object_del(sti->o);
             free(sti);
          }
        st->icons = NULL;

        EINA_LIST_FREE(st->user_info.icons, icon)
          {
             evas_object_del(icon);
          }
        st->user_info.icons = NULL;
     }
}

static inline Eina_List *
_anim_icons_make(Item_Container_Drag_Info *st)
{ /* Transfer user icons to animation icons list */
   Eina_List *list = NULL;
   Evas_Object *o;

   EINA_LIST_FREE(st->user_info.icons, o)
     {  /* Now add icons to animation window */
        Anim_Icon *ast = calloc(1, sizeof(*ast));

        if (!ast)
          {
             ERR("Failed to allocate memory for icon!");
             continue;
          }

        evas_object_geometry_get(o, &ast->start_x, &ast->start_y, &ast->start_w, &ast->start_h);
        evas_object_show(o);
        ast->o = o;
        list = eina_list_append(list, ast);
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
   st->icons = _anim_icons_make(st);
   if (st->user_info.createicon)
     {
        Evas_Object *temp_win = elm_win_add(NULL, "Temp", ELM_WIN_DND);
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
        if (st->data_get(    /* Collect drag info */
                 st->obj,      /* The container object */
                 it,           /* Drag started on this item */
                 &st->user_info))
          {
             if (EINA_DBL_EQ(st->anim_tm, 0.0))
                _cont_obj_drag_start(st);  /* Start dragging, no anim */
             else
               {
                  if (st->user_info.icons)
                    {
                       _drag_anim_start(st);
                    }
                  else
                    {
                       // even if we don't manage the icons animation, we have
                       // to wait until it is finished before beginning drag.
                       st->tm = ecore_timer_add(st->anim_tm, _cont_obj_drag_start, st);
                    }
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
   st->e = e;
   st->x_down = ev->canvas.x;
   st->y_down = ev->canvas.y;

   evas_object_event_callback_add(st->obj, EVAS_CALLBACK_MOUSE_UP,
         _cont_obj_mouse_up, st);

   ecore_timer_del(st->tm);

   if (st->tm_to_drag)
     {
        st->tm = ecore_timer_add(st->tm_to_drag, _cont_obj_anim_start, st);
        evas_object_event_callback_add(st->obj, EVAS_CALLBACK_MOUSE_MOVE,
              _cont_obj_mouse_move, st);
     }
   else
      _cont_obj_anim_start(st);
}

static Eina_Bool elm_drag_item_container_del_internal(Evas_Object *obj, Eina_Bool full);

static void
_abort_drag(Evas_Object *obj, Item_Container_Drag_Info *st)
{
   evas_object_event_callback_del_full
      (st->obj, EVAS_CALLBACK_MOUSE_MOVE, _cont_obj_mouse_move, st);
   evas_object_event_callback_del_full
      (st->obj, EVAS_CALLBACK_MOUSE_UP, _cont_obj_mouse_up, st);
   elm_drag_item_container_del_internal(obj, EINA_FALSE);

   ELM_SAFE_FREE(st->tm, ecore_timer_del);

   _anim_st_free(st);
}

static void
_cont_obj_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{  /* Cancel any drag waiting to start on timeout */
   cnp_debug("In\n");
   Item_Container_Drag_Info *st = data;
   Evas_Event_Mouse_Move *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        cnp_debug("event on hold - have to cancel DnD\n");

        _abort_drag(obj, st);
        st = NULL;
     }

   if (st && evas_device_class_get(ev->dev) == EVAS_DEVICE_CLASS_TOUCH)
     {
        int dx = ev->cur.canvas.x - st->x_down, dy = ev->cur.canvas.y - st->y_down;
        int finger_size = elm_config_finger_size_get();
        if ((dx * dx + dy * dy > finger_size * finger_size))
          {
             cnp_debug("mouse moved too much - have to cancel DnD\n");

             _abort_drag(obj, st);
             st = NULL;
         }
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
        if (dragwidget)
          {
             if (elm_widget_is(dragwidget))
               {
                  Evas_Object *win = elm_widget_top_get(dragwidget);
                  if (win && efl_isa(win, EFL_UI_WIN_CLASS))
                     efl_event_callback_del(win, EFL_UI_WIN_EVENT_ROTATION_CHANGED, _x11_win_rotation_changed_cb, dragwin);
               }
          }
        goto end;
     }
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Ecore_Wl2_Window *win;

   win = _wl_elm_widget_window_get(obj);
   if (win)
     ecore_wl2_dnd_drag_end(_wl_default_seat_get(win, obj));
#endif

end:
   ELM_SAFE_FREE(dragwin, evas_object_del);
   dragdonecb = NULL;
   dragacceptcb = NULL;
   dragposcb = NULL;
   dragwidget = NULL;
   doaccept = EINA_FALSE;
   return EINA_TRUE;
}

