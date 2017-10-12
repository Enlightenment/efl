#ifndef EFL_SELECTION_MANAGER_PRIVATE_H
#define EFL_SELECTION_MANAGER_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

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

   SELECTION_N_ATOMS,
};

typedef struct _Tmp_Info      Tmp_Info;
typedef struct _X11_Cnp_Selection X11_Cnp_Selection;

typedef Eina_Bool (*X11_Converter_Fn_Cb)     (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
typedef int       (*X11_Response_Handler_Cb) (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *);
typedef Eina_Bool (*X11_Data_Preparer_Cb)    (Ecore_X_Event_Selection_Notify *, Elm_Selection_Data *, Tmp_Info **);

typedef struct _Efl_Selection_Manager_Atom Efl_Selection_Manager_Atom;
typedef struct _Efl_Sel_Manager_Atom Efl_Sel_Manager_Atom;
typedef struct _Efl_Selection_Manager_Data Efl_Selection_Manager_Data;

struct _Tmp_Info
{
   char *filename;
   void *map;
   int   fd;
   int   len;
};

struct _X11_Cnp_Selection
{
   const char        *debug;
   Evas_Object       *widget;
   char              *selbuf;
   Evas_Object       *requestwidget;
   void              *udata;
   //Elm_Sel_Format     requestformat;
   //Elm_Drop_Cb        datacb;
   Eina_Bool        (*set)     (Ecore_X_Window, const void *data, int size);
   Eina_Bool        (*clear)   (void);
   void             (*request) (Ecore_X_Window, const char *target);
   //Elm_Selection_Loss_Cb  loss_cb;
   //void                  *loss_data;

   //Elm_Sel_Format     format;
   Ecore_X_Selection  ecore_sel;
   Ecore_X_Window     xwin;
   //Elm_Xdnd_Action    action;

   Eina_Bool          active : 1;
};


struct _Efl_Selection_Manager_Atom
{
    const char *name;
    Ecore_X_Atom x_atom;
};

struct _Efl_Sel_Manager_Atom
{
   const char              *name;
   Efl_Selection_Format           format;
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


struct _Efl_Selection_Manager_Data
{
    Eo *obj;
    Ecore_Event_Handler *notify_handler;
    Ecore_Event_Handler *clear_handler;
    Efl_Promise *promise;
    Efl_Selection_Type loss_type;
#ifdef HAVE_ELEMENTARY_X
    Ecore_X_Window *xwin;
#endif

    void *data_func_data;
    Efl_Selection_Data_Ready data_func;
    Eina_Free_Cb data_func_free_cb;

    Efl_Selection_Type type;
    Efl_Selection_Format format;
    Efl_Selection_Manager_Atom atom;
    void *buf;
    int len;

    Eo *sel_owner;

    Efl_Sel_Manager_Atom *atomlist;
};

#endif
