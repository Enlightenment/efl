#ifndef EFL_SELECTION_PRIVATE_H
#define EFL_SELECTION_PRIVATE_H


//try to make common stuff for selection & dnd, so that we can separate them
#if 0
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
   SELECTION_ATOM_XELM,
   SELECTION_ATOM_UTF8STRING,
   SELECTION_ATOM_STRING,
   SELECTION_ATOM_COMPOUND_TEXT,
   SELECTION_ATOM_TEXT,
   SELECTION_ATOM_TEXT_PLAIN_UTF8,
   SELECTION_ATOM_TEXT_PLAIN,

   SELECTION_N_ATOMS,
};

typedef struct _Tmp_Info      Tmp_Info;
typedef struct _Saved_Type    Saved_Type;
typedef struct _X11_Cnp_Selection X11_Cnp_Selection;

typedef Eina_Bool (*X11_Converter_Fn_Cb)     (char *target, void *data, int size, void **data_ret, int *size_ret, Ecore_X_Atom *ttype, int *typesize);
typedef int       (*X11_Response_Handler_Cb) (X11_Cnp_Selection *sel, Ecore_X_Event_Selection_Notify *);
typedef Eina_Bool (*X11_Data_Preparer_Cb)    (Efl_Selection_Manager_Data *pd, Ecore_X_Event_Selection_Notify *, Efl_Selection_Data *, Tmp_Info **);

typedef struct _Efl_Sel_Atom Efl_Sel_Atom;

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


struct _X11_Cnp_Selection
{
   const char        *debug;
   Evas_Object       *widget;
   char              *selbuf;
   unsigned int       len;
   Efl_Selection_Format     request_format;
   //Elm_Drop_Cb        datacb;
   Eina_Bool        (*set)     (Ecore_X_Window, const void *data, int size);
   Eina_Bool        (*clear)   (void);
   void             (*request) (Ecore_X_Window, const char *target);
   //Elm_Selection_Loss_Cb  loss_cb;
   //void                  *loss_data;

   Efl_Selection_Format     format;
   Ecore_X_Selection  ecore_sel;
   Ecore_X_Window     xwin;
   //Elm_Xdnd_Action    action;

   Eo *owner;

   Eo *request_obj;
   void *data_func_data;
   Efl_Selection_Data_Ready data_func;
   Eina_Free_Cb data_func_free_cb;
};

typedef struct _Seat_Selection Seat_Selection;

struct _Seat_Selection
{
   const char *seat_name;
   X11_Cnp_Selection *sellist;
};


struct _Efl_Sel_Atom
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
#endif



#endif
