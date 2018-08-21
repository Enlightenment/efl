#ifndef EFL_SELECTION_MANAGER_PRIVATE_H
#define EFL_SELECTION_MANAGER_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

enum
{
   SELECTION_ATOM_TARGETS = 0,
   SELECTION_ATOM_ATOM,
   SELECTION_ATOM_LISTING_ATOMS = SELECTION_ATOM_ATOM,
   SELECTION_ATOM_TEXT_URILIST,
   SELECTION_ATOM_TEXT_X_VCARD,
   SELECTION_ATOM_IMAGE_PNG,
   SELECTION_ATOM_IMAGE_JPEG,
   SELECTION_ATOM_IMAGE_BMP,
   SELECTION_ATOM_IMAGE_GIF,
   SELECTION_ATOM_IMAGE_TIFF,
   SELECTION_ATOM_IMAGE_SVG,
   SELECTION_ATOM_IMAGE_XPM,
   SELECTION_ATOM_IMAGE_TGA,
   SELECTION_ATOM_IMAGE_PPM,
   SELECTION_ATOM_ELM,
   SELECTION_ATOM_UTF8STRING,
   SELECTION_ATOM_STRING,
   SELECTION_ATOM_COMPOUND_TEXT,
   SELECTION_ATOM_TEXT,
   SELECTION_ATOM_TEXT_PLAIN_UTF8,
   SELECTION_ATOM_TEXT_PLAIN,

   SELECTION_N_ATOMS,
};

typedef struct _Efl_Selection_Manager_Data Efl_Selection_Manager_Data;
typedef struct _Sel_Manager_Seat_Selection Sel_Manager_Seat_Selection;
typedef struct _Sel_Manager_Atom Sel_Manager_Atom;
typedef struct _Sel_Manager_Dropable Sel_Manager_Dropable;
typedef struct _Anim_Icon Anim_Icon;
typedef struct _Sel_Manager_Drag_Container Sel_Manager_Drag_Container;
typedef struct _Drop_Format Drop_Format;
typedef struct _Item_Container_Drop_Info Item_Container_Drop_Info;
typedef struct _Sel_Manager_Selection Sel_Manager_Selection;
typedef struct _Tmp_Info      Tmp_Info;
typedef struct _Saved_Type    Saved_Type;


#ifdef HAVE_ELEMENTARY_X
typedef Eina_Bool (*X11_Converter_Fn_Cb)     (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
typedef int       (*X11_Response_Handler_Cb) (Sel_Manager_Selection *sel, Ecore_X_Event_Selection_Notify *);
typedef Eina_Bool (*X11_Data_Preparer_Cb)    (Sel_Manager_Seat_Selection *seat_sel, Ecore_X_Event_Selection_Notify *, Efl_Selection_Data *, Tmp_Info **);
#endif
#ifdef HAVE_ELEMENTARY_WL2
typedef Eina_Bool (*Wl_Converter_Fn_Cb)     (char *target, Sel_Manager_Selection *sel, void *data, int size, void **data_ret, int *size_ret);
static Eina_Bool _wl_targets_converter(char *target, Sel_Manager_Selection *sel, void *data, int size, void **data_ret, int *size_ret);
static Eina_Bool _wl_general_converter(char *target, Sel_Manager_Selection *sel, void *data, int size, void **data_ret, int *size_ret);
static Eina_Bool _wl_text_converter(char *target, Sel_Manager_Selection *sel, void *data, int size, void **data_ret, int *size_ret);

typedef Eina_Bool (*Wl_Data_Preparer_Cb)    (Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
static Eina_Bool _wl_data_preparer_markup(Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
static Eina_Bool _wl_data_preparer_uri(Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
static Eina_Bool _wl_data_preparer_vcard(Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
static Eina_Bool _wl_data_preparer_image(Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
static Eina_Bool _wl_data_preparer_text(Sel_Manager_Selection *sel, Efl_Selection_Data *ddata, Ecore_Wl2_Event_Offer_Data_Ready *ev, Tmp_Info **tmp_info);
#endif


struct _Tmp_Info
{
   char *filename;
   void *map;
   int   fd;
   int   len;
};

struct _Saved_Type
{
   const char      **types;
   char             *imgfile;
   int               ntypes;
   Eina_Position2D   pos;
   Eina_Bool         textreq: 1;
};

struct _Sel_Manager_Selection
{
   const char               *debug;
   Eina_Rw_Slice             data;
   Efl_Selection_Format      request_format;
#ifdef HAVE_ELEMENTARY_X
   Eina_Bool               (*set) (Ecore_X_Window, const void *data, int size);
   Eina_Bool               (*clear) (void);
   void                    (*request) (Ecore_X_Window, const char *target);

   Ecore_X_Selection         ecore_sel;
   Ecore_X_Window            xwin;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   uint32_t                  selection_serial;
   uint32_t                  drag_serial;
   Ecore_Wl2_Offer          *sel_offer;
   Ecore_Wl2_Offer          *dnd_offer;
   Ecore_Event_Handler      *offer_handler;
   Ecore_Wl2_Window         *win;
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   Ecore_Cocoa_Window       *win;
   int                       pb_count;
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   Eina_Bool               (*set)(const Ecore_Win32_Window *window, const void *data, int size);
   Eina_Bool               (*clear)(const Ecore_Win32_Window *window);
   Eina_Bool               (*get)(const Ecore_Win32_Window *window , void **data, int *size);
   Ecore_Win32_Selection     ecore_sel;
   Ecore_Win32_Window       *win;
#endif

   Efl_Selection_Format      want_format;
   Efl_Selection_Format      format;
   Efl_Selection_Action      action;
   Eina_Bool                 active : 1;

   unsigned char             asked : 2;

   Efl_Object               *owner;
   Efl_Object               *request_obj;
   void                     *data_func_data;
   Efl_Selection_Data_Ready  data_func;
   Eina_Free_Cb              data_func_free_cb;

   Sel_Manager_Seat_Selection *seat_sel;
};

typedef struct _Sel_Manager_Selection_Lost Sel_Manager_Selection_Lost;

struct _Sel_Manager_Selection_Lost
{
    Efl_Object *request;
    Eina_Promise *promise;
    Efl_Selection_Type type;
};

struct _Sel_Manager_Seat_Selection
{
   unsigned int seat;
#ifdef HAVE_ELEMENTARY_X
   Sel_Manager_Selection *sel_list;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Sel_Manager_Selection *sel;
#endif
#ifdef HAVE_ELEMENTARY_COCOA
   Sel_Manager_Selection *sel;
#endif
#ifdef HAVE_ELEMENTARY_WIN32
   Sel_Manager_Selection *sel_list;
#endif

   Eina_List *sel_lost_list;
   //drag
   Eo *drag_obj;
   Efl_Selection_Action drag_action;
   Eo *drag_win;
   Ecore_Event_Handler *mouse_up_handler, *dnd_status_handler;
   Eina_Bool accept;
   Ecore_X_Window xwin;
   Eina_Position2D drag_pos;
   Eina_Position2D drag_win_start;
   Eina_Position2D drag_win_end;
   Efl_Selection_Type active_type;

   Saved_Type *saved_types;
   Ecore_Event_Handler *enter_handler;
   Ecore_Event_Handler *leave_handler;
   Ecore_Event_Handler *pos_handler;
   Ecore_Event_Handler *drop_handler;

   Efl_Selection_Manager_Data *pd;
};

struct _Anim_Icon
{
   Eina_Rectangle  start;
   Evas_Object    *obj;
};

struct _Sel_Manager_Drag_Container
{
   Evas                          *e;
   Efl_Object                    *cont;
   Efl_Selection_Format           format;
   Eina_Rw_Slice                     data;
   Efl_Selection_Action           action;
   Eina_List                     *icons;
   Eina_Size2D                    final_icon;
   Eina_Position2D                down;
   Ecore_Timer                   *timer;
   Ecore_Animator                *animator;
   double                         time_to_drag;
   double                         anim_duration;
   void                          *drag_data_func_data;
   Efl_Dnd_Drag_Data_Get          drag_data_func;
   Eina_Free_Cb                   drag_data_func_free_cb;
   void                          *item_get_func_data;
   Efl_Dnd_Item_Get               item_get_func;
   Eina_Free_Cb                   item_get_func_free_cb;
   void                          *icon_func_data;
   Efl_Dnd_Drag_Icon_Create       icon_func;
   Eina_Free_Cb                   icon_func_free_cb;
   void                          *icon_list_func_data;
   Efl_Dnd_Drag_Icon_List_Create  icon_list_func;
   Eina_Free_Cb                   icon_list_func_free_cb;
   unsigned int                   seat;

   Efl_Selection_Manager_Data    *pd;
};

struct _Sel_Manager_Atom
{
   const char              *name;
   Efl_Selection_Format     format;
#ifdef HAVE_ELEMENTARY_X
   // Called by ecore to do conversion
   X11_Converter_Fn_Cb      x_converter;
   X11_Data_Preparer_Cb     x_data_preparer;
   // Atom
   Ecore_X_Atom             x_atom;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Wl_Converter_Fn_Cb       wl_converter;
   Wl_Data_Preparer_Cb      wl_data_preparer;
#endif

   void                    *_term;
};

struct _Drop_Format
{
   EINA_INLIST;
   Efl_Selection_Format format;
};

struct _Sel_Manager_Dropable
{
   Evas_Object             *obj;
   Eina_Inlist             *format_list;
   unsigned int             seat;
   struct {
      Eina_Position2D       pos;
      Eina_Bool             in : 1;
      const char           *type;
      Efl_Selection_Format  format;
   } last;

   //for container
   Efl_Dnd_Item_Get         item_func;
   void                    *item_func_data;
   Eina_Bool                is_container : 1;
};

struct _Item_Container_Drop_Info
{
   Efl_Object       *obj;
   void             *item_func_data;
   Efl_Dnd_Item_Get  item_func;
};

#ifdef HAVE_ELEMENTARY_WL2
typedef struct _Wl_Format_Translation
{
  Efl_Selection_Format   format;
  char                 **translates;
} Sel_Manager_Wl_Format_Translation;

char *sm_wl_markup[] = {"application/x-elementary-markup", "", NULL};
char *sm_wl_text[] = {"text/plain;charset=utf-8", "text/plain", NULL};
char *sm_wl_html[] = {"text/html;charset=utf-8", "text/html", NULL};
char *sm_wl_vcard[] = {"text/x-vcard", NULL};
char *sm_wl_image[] = {"image/", "text/uri-list", NULL};

Sel_Manager_Wl_Format_Translation sm_wl_convertion[] = {
  {EFL_SELECTION_FORMAT_MARKUP | EFL_SELECTION_FORMAT_TEXT, sm_wl_markup},
  {EFL_SELECTION_FORMAT_TEXT, sm_wl_text},
  {EFL_SELECTION_FORMAT_HTML, sm_wl_html},
  {EFL_SELECTION_FORMAT_VCARD, sm_wl_vcard},
  {EFL_SELECTION_FORMAT_IMAGE, sm_wl_image},
  {EFL_SELECTION_FORMAT_NONE, NULL},
};
#endif

struct _Efl_Selection_Manager_Data
{
   Eo                 *sel_man;
#ifdef HAVE_ELEMENTARY_X
   Ecore_Event_Handler *notify_handler;
   Ecore_Event_Handler *clear_handler;
   Ecore_Event_Handler *fix_handler;
#endif
#ifdef HAVE_ELEMENTARY_WL2
   Ecore_Event_Handler *send_handler;
   Ecore_Event_Handler *changed_handler;
   Ecore_Event_Handler *end_handler;
   Ecore_Wl2_Display   *wl_display;
#endif
   Efl_Selection_Type  loss_type;

   Sel_Manager_Atom   *atom_list;
   Eina_List          *seat_list;

   Eina_List          *drag_cont_list;

   //drop
   Eina_List          *drop_list;
   Eina_Hash          *type_hash;
   const char         *text_uri;
   Eina_List          *drop_cont_list;
};

#endif
