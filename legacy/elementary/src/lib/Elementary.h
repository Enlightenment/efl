/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef ELEMENTARY_H
#define ELEMENTARY_H

/* What is this?
 * 
 * This is a VERY SIMPLE toolkit - VERY SIMPLE. It is not meant for writing
 * extensive applications. Small simple ones with simple needs. It is meant
 * to make the programmers work almost brainless.
 * 
 * I'm toying with backing this with etk or ewl - or not. right now I am
 * unsure as some of the widgets will be faily complex edje creations. Do
 * not consider any choices permanent - but the API should stay unbroken.
 * 
 */

#include "elementary_config.h"

/* Standard headers for standard system calls etc. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <dlfcn.h>
#include <math.h>
#include <fnmatch.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <glob.h>
#include <locale.h>
#include <libintl.h>
#include <errno.h>
#include <signal.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

/* EFL headers */
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Evas.h>
#include <Ecore_Job.h>
#include <Ecore_Txt.h>
#include <Ecore_File.h>
#include <Eet.h>
#include <Edje.h>

/* allow usage from c++ */
#ifdef __cplusplus
extern "C" {
#endif

// FIXME: need to be able to enable/disable widgets
// FIXME: need to determine scale from dpi   
   /* Types here */
   typedef enum _Elm_Obj_Type
     {
	  ELM_OBJ_OBJ,
	  ELM_OBJ_CB,	
	  ELM_OBJ_WIDGET,
	  ELM_OBJ_WIN,
	  ELM_OBJ_BG,
	  ELM_OBJ_SCROLLER,
	  ELM_OBJ_LABEL,
	  ELM_OBJ_BOX,
	  ELM_OBJ_TABLE,
	  ELM_OBJ_BUTTON,
	  ELM_OBJ_ICON,
	  ELM_OBJ_TOGGLE,
	  ELM_OBJ_CLOCK,
	  ELM_OBJ_FRAME,
	  ELM_OBJ_PAD
//	  ELM_OBJ_CHECK, // FIXME: do
//	  ELM_OBJ_RADIO, // FIXME: do
//        ELM_OBJ_SEP, // FIXME: do (separator horiz or vert)
//	  ELM_OBJ_EXPANDER // FIXME: do (like a paned but slides open/closed)
//	  ELM_OBJ_SPIN, // FIXME: do
//	  ELM_OBJ_SLIDER, // FIXME: do
//	  ELM_OBJ_ENTRY, // FIXME: do
//	  ELM_OBJ_EDITOR, // FIXME: do
//	  ELM_OBJ_LISTITEM, // FIXME: do
// 	  ELM_OBJ_BUSY, // FIXME: do
//	  // FIXME: list more widgets to do here like:
//	  // CONTACT, SELECTOR, FILES, PREVIEW, SIGNALINFO, CALLINFO,
//	  // CELLEDIT (csv - maybe later xls or some other cell format),
//	  // COLORSEL, TACHO ... 
//	  
//	  wrap other basic ecore things:
//	  ELM_OBJ_TIMER,
//	  ELM_OBJ_ANIMATOR,
//	  ELM_OBJ_JOB,
    } Elm_Obj_Type;
   
   typedef enum _Elm_Cb_Type
     {
	  ELM_CB_DEL,
	  ELM_CB_CHILD_ADD,
	  ELM_CB_CHILD_DEL,
	  ELM_CB_UNPARENT,
	  ELM_CB_PARENT,
	  ELM_CB_DEL_REQ,
	  ELM_CB_RESIZE,
	  ELM_CB_CHANGED,
	  ELM_CB_ACTIVATED
     } Elm_Cb_Type;
   
   typedef enum _Elm_Win_Type
     {
	ELM_WIN_BASIC,
	  ELM_WIN_DIALOG_BASIC
     } Elm_Win_Type;
   
   typedef struct _Elm_Obj_Class       Elm_Obj_Class;
   typedef struct _Elm_Obj             Elm_Obj;
   typedef struct _Elm_Cb_Class        Elm_Cb_Class;
   typedef struct _Elm_Cb              Elm_Cb;
   typedef struct _Elm_Win_Class       Elm_Win_Class;
   typedef struct _Elm_Win             Elm_Win;
   typedef struct _Elm_Widget_Class    Elm_Widget_Class;
   typedef struct _Elm_Widget          Elm_Widget;
   typedef struct _Elm_Bg_Class        Elm_Bg_Class;
   typedef struct _Elm_Bg              Elm_Bg;
   typedef struct _Elm_Scroller_Class  Elm_Scroller_Class;
   typedef struct _Elm_Scroller        Elm_Scroller;
   typedef struct _Elm_Label_Class     Elm_Label_Class;
   typedef struct _Elm_Label           Elm_Label;
   typedef struct _Elm_Box_Class       Elm_Box_Class;
   typedef struct _Elm_Box             Elm_Box;
   typedef struct _Elm_Table_Class     Elm_Table_Class;
   typedef struct _Elm_Table           Elm_Table;
   typedef struct _Elm_Button_Class    Elm_Button_Class;
   typedef struct _Elm_Button          Elm_Button;
   typedef struct _Elm_Icon_Class      Elm_Icon_Class;
   typedef struct _Elm_Icon            Elm_Icon;
   typedef struct _Elm_Toggle_Class    Elm_Toggle_Class;
   typedef struct _Elm_Toggle          Elm_Toggle;
   typedef struct _Elm_Clock_Class     Elm_Clock_Class;
   typedef struct _Elm_Clock           Elm_Clock;
   typedef struct _Elm_Frame_Class     Elm_Frame_Class;
   typedef struct _Elm_Frame           Elm_Frame;
   typedef struct _Elm_Pad_Class       Elm_Pad_Class;
   typedef struct _Elm_Pad             Elm_Pad;
   
   typedef void (*Elm_Cb_Func) (void *data, Elm_Obj *obj, Elm_Cb_Type type, void *info);
   
   /* API calls here */

/**************************************************************************/   
   /* General calls */
   EAPI void elm_init(int argc, char **argv);
   EAPI void elm_shutdown(void);
   EAPI void elm_run(void);
   EAPI void elm_exit(void);

/**************************************************************************/   
   /* Generic Elm Object */
#define Elm_Obj_Class_Methods \
   void (*del)                        (Elm_Obj *obj); \
   void (*ref)                        (Elm_Obj *obj); \
   void (*unref)                      (Elm_Obj *obj); \
   Elm_Cb *(*cb_add)                  (Elm_Obj *obj, Elm_Cb_Type type, Elm_Cb_Func func, void *data); \
   void (*child_add)                  (Elm_Obj *obj, Elm_Obj *child); \
   void (*unparent)                   (Elm_Obj *obj); \
   int  (*hastype)                    (Elm_Obj *obj, Elm_Obj_Type type)
#define Elm_Obj_Class_All Elm_Obj_Class_Methods; \
   Elm_Obj_Type   type; \
   void          *clas; /* the obj class and parent classes */ \
   Elm_Obj       *parent; \
   Evas_List     *children; \
   Evas_List     *cbs; \
   int            refs; \
   unsigned char  delete_me : 1; \
   unsigned char  delete_deferred : 1
   
   struct _Elm_Obj_Class
     {
	void *parent;
        Elm_Obj_Type type;
	Elm_Obj_Class_Methods;
     };
   struct _Elm_Obj
     {
	Elm_Obj_Class_All;
     };
#define ELM_OBJ(o) ((Elm_Obj *)o)   
   
/**************************************************************************/   
   /* Callback Object */
#define Elm_Cb_Class_Methods
#define Elm_Cb_Class_All Elm_Obj_Class_All; Elm_Cb_Class_Methods; \
   Elm_Cb_Class_Methods; \
   Elm_Cb_Type  cb_type; \
   Elm_Cb_Func  func; \
   void              *data;
   struct _Elm_Cb_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Cb_Class_Methods;
     };
   struct _Elm_Cb
     {
	Elm_Cb_Class_All;
     };

/**************************************************************************/   
   /* Widget Object */
#define Elm_Widget_Class_Methods \
   void (*geom_set)   (Elm_Widget *wid, int x, int y, int w, int h); \
   void (*show)       (Elm_Widget *wid); \
   void (*hide)       (Elm_Widget *wid); \
   void (*size_alloc) (Elm_Widget *wid, int w, int h); \
   void (*size_req)   (Elm_Widget *wid, Elm_Widget *child, int w, int h); \
   void (*above)      (Elm_Widget *wid, Elm_Widget *above); \
   void (*below)      (Elm_Widget *wid, Elm_Widget *below)
   
#define Elm_Widget_Class_All Elm_Obj_Class_All; Elm_Widget_Class_Methods; \
   int x, y, w, h; \
   struct { int w, h; } req; \
   Evas_Object *base; \
   double align_x, align_y; \
   unsigned char expand_x : 1; \
   unsigned char expand_y : 1; \
   unsigned char fill_x : 1; \
   unsigned char fill_y : 1
   
   /* Object specific ones */
   // FIXME: should this be a function or widget method call?
   EAPI void elm_widget_sizing_update(Elm_Widget *wid);
   struct _Elm_Widget_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Widget_Class_Methods;
     };
   struct _Elm_Widget
     {
	Elm_Widget_Class_All;
     };
   
#ifdef __cplusplus
}
#endif

/**************************************************************************/   
   /* Window Object */
#define Elm_Win_Class_Methods \
   void (*name_set)  (Elm_Win *win, const char *name); \
   void (*title_set) (Elm_Win *win, const char *title)
// FIXME:   
// cover methods & state for:
// type, fullscreen, icon, activate, shaped, alpha, borderless, iconified,
// setting parent window (for dialogs)
#define Elm_Win_Class_All Elm_Widget_Class_All; Elm_Win_Class_Methods; \
   Elm_Win_Type  win_type; \
   const char   *name; \
   const char   *title; \
   unsigned char autodel : 1
   
   /* Object specific ones */
   EAPI Elm_Win *elm_win_new(void);
   struct _Elm_Win_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Win_Class_Methods;
     };
   struct _Elm_Win
     {
	Elm_Win_Class_All;
	
	Ecore_Evas     *ee; /* private */
	Evas           *evas; /* private */
	Ecore_X_Window  xwin; /* private */
	Ecore_Job      *deferred_resize_job; /* private */
	Ecore_Job      *deferred_child_eval_job; /* private */
	unsigned char   showme : 1; /* private */
     };
   
/**************************************************************************/   
   /* Background Object */
#define Elm_Bg_Class_Methods \
   void (*file_set)  (Elm_Bg *bg, const char *file, const char *group);
#define Elm_Bg_Class_All Elm_Widget_Class_All; Elm_Bg_Class_Methods; \
   const char *file; \
   const char *group
   
   /* Object specific ones */
   EAPI Elm_Bg *elm_bg_new(Elm_Win *win);
   struct _Elm_Bg_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Bg_Class_Methods;
     };
   struct _Elm_Bg
     {
	Elm_Bg_Class_All;
	
	Evas_Object *custom_bg;
     };
   
/**************************************************************************/   
   /* Scroller (scrollframe/scrolledview) Object */
#define Elm_Scroller_Class_Methods \
   void (*file_set)  (Elm_Scroller *scroller, const char *file, const char *group);
#define Elm_Scroller_Class_All Elm_Widget_Class_All; Elm_Scroller_Class_Methods; \
   const char *file; \
   const char *group
   
   /* Object specific ones */
   EAPI Elm_Scroller *elm_scroller_new(Elm_Win *win);
   struct _Elm_Scroller_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Scroller_Class_Methods;
     };
   struct _Elm_Scroller
     {
	Elm_Scroller_Class_All;
	
	Evas_Object *scroller_pan;
     };
   
/**************************************************************************/   
   /* Label Object */
#define Elm_Label_Class_Methods \
   void (*text_set)  (Elm_Label *lb, const char *text)
#define Elm_Label_Class_All Elm_Widget_Class_All; Elm_Label_Class_Methods; \
   const char *text; \
   int minw, minh
   
   /* Object specific ones */
   EAPI Elm_Label *elm_label_new(Elm_Win *win);
   struct _Elm_Label_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Label_Class_Methods;
     };
   struct _Elm_Label
     {
	Elm_Label_Class_All;
     };
   
/**************************************************************************/   
   /* Box Object */
#define Elm_Box_Class_Methods \
   void (*layout_update)  (Elm_Box *bx); \
   void (*pack_start)     (Elm_Box *bx, Elm_Widget *wid); \
   void (*pack_end)       (Elm_Box *bx, Elm_Widget *wid); \
   void (*pack_before)    (Elm_Box *bx, Elm_Widget *wid, Elm_Widget *wid_before); \
   void (*pack_after)     (Elm_Box *bx, Elm_Widget *wid, Elm_Widget *wid_after);

#define Elm_Box_Class_All Elm_Widget_Class_All; Elm_Box_Class_Methods; \
   unsigned char horizontal : 1; \
   unsigned char homogenous : 1
   
   /* Object specific ones */
   EAPI Elm_Box *elm_box_new(Elm_Win *win);
   struct _Elm_Box_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Box_Class_Methods;
     };
   struct _Elm_Box
     {
	Elm_Box_Class_All;
     };
   
/**************************************************************************/   
   /* Table Object */
#define Elm_Table_Class_Methods \
   void (*layout_update)  (Elm_Table *tb); \
   void (*pack)           (Elm_Table *tb, Elm_Widget *wid, int x, int y, int w, int h)
#define Elm_Table_Class_All Elm_Widget_Class_All; Elm_Table_Class_Methods; \
   unsigned char homogenous : 1
   
   /* Object specific ones */
   EAPI Elm_Table *elm_table_new(Elm_Win *win);
   struct _Elm_Table_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Table_Class_Methods;
     };
   struct _Elm_Table
     {
	Elm_Table_Class_All;
     };
   
/**************************************************************************/   
   /* Button Object */
#define Elm_Button_Class_Methods \
   void (*text_set)  (Elm_Button *bt, const char *text)
#define Elm_Button_Class_All Elm_Widget_Class_All; Elm_Button_Class_Methods; \
   const char *text; \
   int minw, minh
   
   /* Object specific ones */
   EAPI Elm_Button *elm_button_new(Elm_Win *win);
   struct _Elm_Button_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Button_Class_Methods;
     };
   struct _Elm_Button
     {
	Elm_Button_Class_All;
     };
   
/**************************************************************************/   
   /* Icon Object */
#define Elm_Icon_Class_Methods \
   void (*file_set)  (Elm_Icon *icon, const char *file, const char *group); \
   void (*layout_update)  (Elm_Icon *icon)
#define Elm_Icon_Class_All Elm_Widget_Class_All; Elm_Icon_Class_Methods; \
   unsigned char scale_up : 1; \
   unsigned char scale_down : 1; \
   unsigned char fill_outside : 1; \
   unsigned char smooth : 1; \
   unsigned char no_scale : 1; \
   const char *file; \
   const char *group
   
   /* Object specific ones */
   EAPI Elm_Icon *elm_icon_new(Elm_Win *win);
   struct _Elm_Icon_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Icon_Class_Methods;
     };
   struct _Elm_Icon
     {
	Elm_Icon_Class_All;
     };

/**************************************************************************/   
   /* Toggle Object */
#define Elm_Toggle_Class_Methods \
   void (*text_set)  (Elm_Toggle *tg, const char *text); \
   void (*layout_update)  (Elm_Toggle *tg); \
   void (*states_text_set) (Elm_Toggle *tg, const char *ontext, const char *offtext)
#define Elm_Toggle_Class_All Elm_Widget_Class_All; Elm_Toggle_Class_Methods; \
   const char *text; \
   int minw, minh; \
   int state; \
   int *state_ptr
   
   /* Object specific ones */
   EAPI Elm_Toggle *elm_toggle_new(Elm_Win *win);
   struct _Elm_Toggle_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Toggle_Class_Methods;
     };
   struct _Elm_Toggle
     {
	Elm_Toggle_Class_All;
     };
   
/**************************************************************************/   
   /* Clock Object */
#define Elm_Clock_Class_Methods \
   void (*time_update) (Elm_Clock *ck)
#define Elm_Clock_Class_All Elm_Widget_Class_All; Elm_Clock_Class_Methods; \
   int hrs, min, sec; \
   int minw, minh; \
   unsigned char seconds : 1; \
   unsigned char am_pm : 1; \
   unsigned char edit : 1
   
   /* Object specific ones */
   EAPI Elm_Clock *elm_clock_new(Elm_Win *win);
   struct _Elm_Clock_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Clock_Class_Methods;
     };
   struct _Elm_Clock
     {
	Elm_Clock_Class_All;
	
	Evas_Object *digit[6];
	Evas_Object *ampm;
	Ecore_Timer *ticker;
	struct {
	   int hrs, min, sec;
	   char ampm;
	   char seconds;
	   char am_pm;
	   char edit;
	} cur;
     };
   
/**************************************************************************/   
   /* Frame Object */
#define Elm_Frame_Class_Methods \
   void (*text_set)  (Elm_Frame *fr, const char *text)
#define Elm_Frame_Class_All Elm_Widget_Class_All; Elm_Frame_Class_Methods; \
   const char *text; \
   int minw, minh
   
   /* Object specific ones */
   EAPI Elm_Frame *elm_frame_new(Elm_Win *win);
   struct _Elm_Frame_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Frame_Class_Methods;
     };
   struct _Elm_Frame
     {
	Elm_Frame_Class_All;
     };
   
/**************************************************************************/   
   /* Pad Object */
#define Elm_Pad_Class_Methods
#define Elm_Pad_Class_All Elm_Widget_Class_All; Elm_Pad_Class_Methods; \
   int minw, minh
   
   /* Object specific ones */
   EAPI Elm_Pad *elm_pad_new(Elm_Win *win);
   struct _Elm_Pad_Class
     {
	void *parent;
	Elm_Obj_Type type;
	Elm_Pad_Class_Methods;
     };
   struct _Elm_Pad
     {
	Elm_Pad_Class_All;
     };
   
#endif
