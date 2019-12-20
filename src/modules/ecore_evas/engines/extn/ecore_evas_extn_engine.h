#ifndef _ECORE_EVAS_EXTN_ENGINE_H_
#define _ECORE_EVAS_EXTN_ENGINE_H_

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
# include <evil_private.h> /* mmap */
#else
# include <sys/mman.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include <Evas.h>
#include <Evas_Engine_Buffer.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Input.h>
#include <Ecore_Ipc.h>

#include "ecore_private.h" // FIXME: Because of ECORE_MAGIC
#include "ecore_evas_private.h"
#include "ecore_evas_buffer.h"
#include "ecore_evas_extn.h"

typedef struct _Extnbuf Extnbuf;

Extnbuf    *_extnbuf_new(const char *base, int id, Eina_Bool sys, int num,
                         int w, int h, Eina_Bool owner);
void        _extnbuf_free(Extnbuf *b);
void       *_extnbuf_data_get(Extnbuf *b, int *w, int *h, int *stride);
void       *_extnbuf_lock(Extnbuf *b, int *w, int *h, int *stride);
void        _extnbuf_unlock(Extnbuf *b);
const char *_extnbuf_lock_file_get(const Extnbuf *b);
Eina_Bool   _extnbuf_lock_file_set(Extnbuf *b, const char *file);
Eina_Bool   _extnbuf_lock_get(const Extnbuf *b);

// procotol version - change this as needed
#define MAJOR 0x2011

enum // opcodes
{
   OP_RESIZE,
   OP_SHOW,
   OP_HIDE,
   OP_FOCUS,
   OP_UNFOCUS,
   OP_UPDATE,
   OP_UPDATE_DONE,
   OP_SHM_REF0,
   OP_SHM_REF1,
   OP_SHM_REF2,
   OP_PROFILE_CHANGE_REQUEST,
   OP_PROFILE_CHANGE_DONE,
   OP_EV_MOUSE_IN,
   OP_EV_MOUSE_OUT,
   OP_EV_MOUSE_UP,
   OP_EV_MOUSE_DOWN,
   OP_EV_MOUSE_MOVE,
   OP_EV_MOUSE_WHEEL,
   OP_EV_MULTI_UP,
   OP_EV_MULTI_DOWN,
   OP_EV_MULTI_MOVE,
   OP_EV_KEY_UP,
   OP_EV_KEY_DOWN,
   OP_EV_HOLD,
   OP_MSG_PARENT,
   OP_MSG
};

enum
{
   MOD_SHIFT  = (1 << 0),
   MOD_CTRL   = (1 << 1),
   MOD_ALT    = (1 << 2),
   MOD_META   = (1 << 3),
   MOD_HYPER  = (1 << 4),
   MOD_SUPER  = (1 << 5),
   MOD_CAPS   = (1 << 6),
   MOD_NUM    = (1 << 7),
   MOD_SCROLL = (1 << 8),
};

typedef struct _Ipc_Data_Resize Ipc_Data_Resize;
typedef struct _Ipc_Data_Update Ipc_Data_Update;
typedef struct _Ipc_Data_Ev_Mouse_In Ipc_Data_Ev_Mouse_In;
typedef struct _Ipc_Data_Ev_Mouse_Out Ipc_Data_Ev_Mouse_Out;
typedef struct _Ipc_Data_Ev_Mouse_Up Ipc_Data_Ev_Mouse_Up;
typedef struct _Ipc_Data_Ev_Mouse_Down Ipc_Data_Ev_Mouse_Down;
typedef struct _Ipc_Data_Ev_Mouse_Move Ipc_Data_Ev_Mouse_Move;
typedef struct _Ipc_Data_Ev_Mouse_Wheel Ipc_Data_Ev_Mouse_Wheel;
typedef struct _Ipc_Data_Ev_Hold Ipc_Data_Ev_Hold;
typedef struct _Ipc_Data_Ev_Multi_Up Ipc_Data_Ev_Multi_Up;
typedef struct _Ipc_Data_Ev_Multi_Down Ipc_Data_Ev_Multi_Down;
typedef struct _Ipc_Data_Ev_Multi_Move Ipc_Data_Ev_Multi_Move;
typedef struct _Ipc_Data_Ev_Key_Up Ipc_Data_Ev_Key_Up;
typedef struct _Ipc_Data_Ev_Key_Down Ipc_Data_Ev_Key_Down;

struct _Ipc_Data_Resize
{
   int w, h;
};

struct _Ipc_Data_Update
{
   int x, w, y, h;
};

struct _Ipc_Data_Ev_Mouse_In
{
   unsigned int timestamp;
   int mask;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Mouse_Out
{
   unsigned int timestamp;
   int mask;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Mouse_Up
{
   int b;
   Evas_Button_Flags flags;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Mouse_Down
{
   int b;
   Evas_Button_Flags flags;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Mouse_Move
{
   int x, y;
   Evas_Button_Flags flags;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Mouse_Wheel
{
   int direction, z;
   Evas_Button_Flags flags;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Hold
{
   int hold;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Multi_Up
{
   Evas_Button_Flags flags;
   int d, x, y;
   double rad, radx, rady, pres, ang, fx, fy;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Multi_Down
{
   Evas_Button_Flags flags;
   int d, x, y;
   double rad, radx, rady, pres, ang, fx, fy;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Multi_Move
{
   int d, x, y;
   double rad, radx, rady, pres, ang, fx, fy;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Key_Up
{
   const char *keyname, *key, *string, *compose;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

struct _Ipc_Data_Ev_Key_Down
{
   const char *keyname, *key, *string, *compose;
   int mask;
   unsigned int timestamp;
   Evas_Event_Flags event_flags;
};

#endif /*_ECORE_EVAS_EXTN_ENGINE_H_*/
