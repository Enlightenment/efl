// FIXME: Some error checking would be nice.

#include "edje_private.h"
#include <ctype.h>

#define RASTER_FORGOT_WHY "this is here."

//--------------------------------------------------------------------------//
#define MAX_LUA_MEM       (4 * (1024 * 1024))
#define ELO               "|-ELO"

#define LC(...) EINA_LOG_DOM_CRIT(_log_domain, __VA_ARGS__)
#define LE(...) EINA_LOG_DOM_ERR(_log_domain, __VA_ARGS__)
#define LW(...) EINA_LOG_DOM_WARN(_log_domain, __VA_ARGS__)
#define LI(...) EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)
#define LD(...) EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)

/**
   @page luaref Edje Lua scripting

   @section intro Introduction

   Lua is intended for script-only objects at this point (with embryo left
   for augmenting standard programs). Since script-only objects effectively
   define objects entirely via Lua script (resize handling, event handling
   etc. etc.) this places many more demands on them, and thus a more powerful
   language is in order. Lua is that language.

   To get you started, here's an example that uses most of this lua API:
   @ref lua_script.edc

   Most of these lua functions are wrappers around various evas, ecore, and edje C
   functions.  Refer to their documentation for more in depth details and up to
   date documentation.  A lot of this documentation is simple copied from the C
   functions it wraps.

   @section args Lua function argument and return syntax

   Some of the lua functions can accept a table as well as separate arguments.
   Some of them return tables.

   @section classes Lua classes

 */

/*
   Lua functions stack usage.

   In the definition of the lua functions provided, always mention the stack usage,
   using the same notation that is used in the Lua 5.1 Reference Manual.
   http://www.lua.org/manual/5.1/manual.html#3.7 describes that notation.

   On the other hand, lua discards excess stack entries when control passes back to
   it, but it's good to maintain proper discipline.

   Should do the same for the support functions.  These ARE more important to check.
 */

//--------------------------------------------------------------------------//
typedef struct _Edje_Lua_Alloc       Edje_Lua_Alloc;
typedef struct _Edje_Lua_Allocator   Edje_Lua_Allocator;
typedef struct _Edje_Lua_Obj         Edje_Lua_Obj;
typedef struct _Edje_Lua_Animator    Edje_Lua_Animator;
typedef struct _Edje_Lua_Timer       Edje_Lua_Timer;
typedef struct _Edje_Lua_Transition  Edje_Lua_Transition;
typedef struct _Edje_Lua_Evas_Object Edje_Lua_Evas_Object;
typedef struct _Edje_Lua_Map         Edje_Lua_Map;

struct _Edje_Lua_Alloc
{
   size_t max, cur;
};

struct _Edje_Lua_Allocator
{
   Edje_Lua_Alloc *ela;
   void           *(*func)(void *ud, void *ptr, size_t osize, size_t nsize);
   void           *ud;
   int             ref;
};

struct _Edje_Lua_Obj
{
   EINA_INLIST;

   Edje       *ed;
   void        (*free_func)(void *obj);
   const char *meta;
};

struct _Edje_Lua_Animator
{
   Edje_Lua_Obj    obj;
   Ecore_Animator *animator;
   int             fn_ref;
};

struct _Edje_Lua_Timer
{
   Edje_Lua_Obj obj;
   Ecore_Timer *timer;
   int          fn_ref;
};

struct _Edje_Lua_Transition
{
   Edje_Lua_Obj    obj;
   Ecore_Animator *animator;
   double          transition, start;
   int             fn_ref;
};

struct _Edje_Lua_Evas_Object
{
   Edje_Lua_Obj obj;
   Evas_Object *evas_obj;
   int          x, y;
};

struct _Edje_Lua_Map
{
   Edje_Lua_Obj obj;
   Evas_Map    *map;
};

static void      _elua_add_functions(lua_State *L, const char *api, const luaL_Reg *funcs, const char *meta, const char *parent, const char *base);
static Eina_Bool _elua_isa(Edje_Lua_Obj *obj, const char *type);

//--------------------------------------------------------------------------//
#ifndef RASTER_FORGOT_WHY
static lua_State *lstate = NULL;
#endif
static const char *_elua_key = "key";
static const char *_elua_objs = "objs";
/* This is not needed, pcalls don't longjmp(), that's why they are protected.
   static jmp_buf panic_jmp;
 */
static int panics = 0;
static int _log_domain = -1;
static int _log_count = 0;

// FIXME: methods lua script can provide that edje will call (not done yet):
// // scale set
// // key down
// // key up
// // get dragable pos
// // set dragable pos
// // set drag size, step, page
// // get drag size, step, page
// // dragable step
// // dragable page
// // get part text
// // set part text
// // get swallow part
// // set swallow part
// // unswallow part
// // textclass change
// // colorclass change
// // min size get <- ?? maybe set fn
// // max size get <- ?? maybe set fn
// // min size caclc (min/max restriction)
// // preload
// // preload cancel
// // play set
// // animation set
// // parts extends calc
// // part object get
// // part geometry get
//
// // LATER: all the entry calls
// // LATER: box and table calls
// // LATER: perspective stuff change
//

// Grumble, pre-declare these.
static const char *_elua_edje_meta = "edje_meta";
static const char *_elua_evas_meta = "evas_meta";
static const char *_elua_evas_edje_meta = "evas_edje_meta";
static const char *_elua_evas_image_meta = "evas_image_meta";
static const char *_elua_evas_line_meta = "evas_line_meta";
static const char *_elua_evas_map_meta = "evas_map_meta";
static const char *_elua_evas_polygon_meta = "evas_polygon_meta";
static const char *_elua_evas_text_meta = "evas_text_meta";
static const char *_elua_ecore_animator_meta = "ecore_animator_meta";
static const char *_elua_ecore_timer_meta = "ecore_timer_meta";

static int _elua_obj_gc(lua_State *L);

static const struct luaL_Reg _elua_edje_gc_funcs [] =
{
   {"__gc", _elua_obj_gc},   // garbage collector func for edje objects

   {NULL, NULL}   // end
};

static const luaL_Reg _elua_libs[] =
{
   {"", luaopen_base},
//     {LUA_LOADLIBNAME, luaopen_package}, // disable this lib - don't want
   {LUA_TABLIBNAME, luaopen_table},
//     {LUA_IOLIBNAME, luaopen_io}, // disable this lib - don't want
//     {LUA_OSLIBNAME, luaopen_os}, // FIXME: audit os lib - maybe not provide or only provide specific calls
   {LUA_STRLIBNAME, luaopen_string},
   {LUA_MATHLIBNAME, luaopen_math},
//     {LUA_DBLIBNAME, luaopen_debug}, // disable this lib - don't want

   {NULL, NULL}   // end
};

//--------------------------------------------------------------------------//
static void *
_elua_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
   size_t dif;
   Edje_Lua_Allocator *al = ud;
   Edje_Lua_Alloc *ela = al->ela;

   // in lua 5.2 osize encodes the type of data allocted if ptr is NULL
   // LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, or LUA_TTHREAD
   if (ptr == NULL) osize = 0;

   if (nsize > osize)
     {
        dif = nsize - osize;
        ela->cur += dif;
     }
   else
     {
        dif = osize - nsize;
        if (ela->cur < dif)
          {
             ERR("Lua allloc cur size %i < diff %i\n", (int)ela->cur, (int)dif);
             dif = ela->cur;
          }
        ela->cur -= dif;
     }

   if (ela->cur > ela->max)
     {
        ERR("Lua memory limit of %i bytes reached (%i allocated)",
            (int)ela->max, (int)ela->cur);
        return NULL;
     }

   ptr = al->func(al->ud, ptr, osize, nsize);
   if (nsize == 0 || ptr) return ptr;

   ERR("Lua cannot re-allocate %i bytes", (int)nsize);
   return NULL;
}

static int
_elua_custom_panic(lua_State *L) // Stack usage [-0, +0, m]
{
   // If we somehow manage to have multiple panics, it's likely due to being out
   // of memory in the following lua_tostring() call.
   panics++;
   if (panics)
     {
        EINA_LOG_DOM_CRIT(_edje_default_log_dom, "Lua PANICS!!!!!");
     }
   else
     {
        EINA_LOG_DOM_CRIT(_edje_default_log_dom,
                          "Lua PANIC!!!!!: %s", lua_tostring(L, -1)); // Stack usage [-0, +0, m]
     }
   // The docs say that this will cause an exit(EXIT_FAILURE) if we return,
   // and that we we should long jump some where to avoid that.  This is only
   // called for things not called from a protected environment.  We always
   // use pcalls though, except for the library load calls.  If we can't load
   // the standard libraries, then perhaps a crash is the right thing.
   return 0;
}

// Really only used to manage the pointer to our edje.
static void
_elua_table_ptr_set(lua_State *L, const void *key, const void *val)  // Stack usage [-2, +2, e]
{
   lua_pushlightuserdata(L, (void *)key);  // Stack usage [-0, +1, -]
   lua_pushlightuserdata(L, (void *)val);  // Stack usage [-0, +1, -]
   lua_settable(L, LUA_REGISTRYINDEX);     // Stack usage [-2, +0, e]
}

static const void *
_elua_table_ptr_get(lua_State *L, const void *key)  // Stack usage [-2, +2, e]
{
   const void *ptr;
   lua_pushlightuserdata(L, (void *)key);  // Stack usage [-0, +1, -]
   lua_gettable(L, LUA_REGISTRYINDEX);     // Stack usage [-1, +1, e]
   ptr = lua_topointer(L, -1); // Stack usage [-0, +0, -]
   lua_pop(L, 1); // Stack usage [-n, +0, -]
   return ptr;
}

/* XXX: not used
   static void
   _elua_table_ptr_del(lua_State *L, const void *key)  // Stack usage [-2, +2, e]
   {
   lua_pushlightuserdata(L, (void *)key);  // Stack usage [-0, +1, -]
   lua_pushnil(L);                         // Stack usage [-0, +1, -]
   lua_settable(L, LUA_REGISTRYINDEX);     // Stack usage [-2, +0, e]
   }
 */

/*
 * Cori: Assumes object to be saved on top of stack
 */
static void
_elua_ref_set(lua_State *L, void *key)     // Stack usage [-4, +4, m]
{
   lua_pushlightuserdata(L, &_elua_objs);  // Stack usage [-0, +1, -]
   lua_rawget(L, LUA_REGISTRYINDEX); // Stack usage [-1, +1, -]
   lua_pushlightuserdata(L, key); // Stack usage [-0, +1, -]
   lua_pushvalue(L, -3); // Stack usage [-0, +1, -]
   lua_rawset(L, -3); // Stack usage [-2, +0, m]
   lua_pop(L, 1); // Stack usage [-n, +0, -]
}

/*
 * Cori: Get an object from the object table
 */
static void *
_elua_ref_get(lua_State *L, void *key)     // Stack usage [-3, +4, -]
{
   lua_pushlightuserdata(L, &_elua_objs);  // Stack usage [-0, +1, -]
   lua_rawget(L, LUA_REGISTRYINDEX); // Stack usage [-1, +1, -]
   lua_pushlightuserdata(L, key); // Stack usage [-0, +1, -]
   lua_rawget(L, -2); // Stack usage [-1, +1, -]
   lua_remove(L, -2); // Stack usage [-1, +0, -]
   return lua_touserdata(L, -2); // Stack usage [-0, +0, -]
}

static Edje_Lua_Obj *
_elua_obj_new(lua_State *L, Edje *ed, int size, const char *metatable)  // Stack usage [-5, +6, m]
{
   Edje_Lua_Obj *obj;

   obj = (Edje_Lua_Obj *)lua_newuserdata(L, size);  // Stack usage [-0, +1, m]
   memset(obj, 0, size);
   ed->lua_objs = eina_inlist_append(ed->lua_objs, EINA_INLIST_GET(obj));

   luaL_getmetatable(L, metatable); // Stack usage [-0, +1, -]
   lua_setmetatable(L, -2); // Stack usage [-1, +0, -]
   obj->ed = ed;
   obj->meta = metatable;

   _elua_ref_set(L, obj); // Stack usage [-4, +4, m]
   return obj;
}

static void
_elua_obj_free(lua_State *L, Edje_Lua_Obj *obj)
{
   if (!obj->free_func) return;
   // Free the reference, so it will actually get gc'd.
   // It seems that being a completely weak table isn't enough.
   lua_pushnil(L); // Stack usage [-0, +1, -]
   _elua_ref_set(L, obj); // Stack usage [-4, +4, m]
   obj->free_func(obj);
   obj->ed->lua_objs = eina_inlist_remove(obj->ed->lua_objs, EINA_INLIST_GET(obj));
   obj->free_func = NULL;
   obj->ed = NULL;
}

static int
_elua_obj_gc(lua_State *L)  // Stack usage [-0, +0, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);  // Stack usage [-0, +0, -]
   if (!obj) return 0;
   _elua_obj_free(L, obj);
   return 0;
}

static int
_elua_obj_del(lua_State *L)  // Stack usage [-0, +0, -]
{
   return _elua_obj_gc(L);   // Stack usage [-0, +0, -]
}

static void
_elua_gc(lua_State *L)  // Stack usage [-0, +0, e]
{
   lua_gc(L, LUA_GCCOLLECT, 0);  // Stack usage [-0, +0, e]
}

// These are what the various symbols are for each type -
//  int      %
//  num      #
//  str      $
//  bool     !
// FIXME: Still to do, if we ever use them -
//  func     &
//  userdata +
//  lightuserdata *
//  table    @
//  thread   ^
//  nil      ~

static char *
_elua_push_name(lua_State *L, char *q, int idx)  // Stack usage [-0, +1, e or m]
{
   char *p = q;
   char temp = '\0';

   // A simplistic scan through an identifier, it's wrong, but it's quick,
   // and we don't mind that it's wrong, coz this is only internal.
   while (isalnum((int)*q))
     q++;
   temp = *q;
   *q = '\0';
   if (idx > 0)
     lua_getfield(L, idx, p);  // Stack usage [-0, +1, e]
   else
     lua_pushstring(L, p);  // Stack usage [-0, +1, m]
   *q = temp;

   return q;
}

static int
_elua_scan_params(lua_State *L, int i, char *params, ...) // Stack usage -
                                                                         // if i is a table
     //   [-n, +n, e]
                                                                         // else
     //   [-0, +0, -]
{
   va_list vl;
   char *f = strdup(params);
   char *p = f;
   int n = 0, j = i, count = 0;
   Eina_Bool table = EINA_FALSE;

   if (!f) return -1;
   va_start(vl, params);

   if (lua_istable(L, i)) // Stack usage [-0, +0, -]
     {
        j = -1;
        table = EINA_TRUE;
     }

   while (*p)
     {
        char *q;
        Eina_Bool get = EINA_TRUE;

        while (isspace((int)*p))
          p++;
        q = p + 1;
        switch (*p)
          {
           case '%':
           {
              if (table)
                {
                   q = _elua_push_name(L, q, i); // Stack usage [-0, +1, e]
                }
              if (lua_isnumber(L, j)) // Stack usage [-0, +0, -]
                {
                   int *v = va_arg(vl, int *);
                   *v = lua_tointeger(L, j); // Stack usage [-0, +0, -]
                   n++;
                }
              break;
           }

           case '#':
           {
              if (table)
                {
                   q = _elua_push_name(L, q, i); // Stack usage [-0, +1, e]
                }
              if (lua_isnumber(L, j)) // Stack usage [-0, +0, -]
                {
                   double *v = va_arg(vl, double *);
                   *v = lua_tonumber(L, j); // Stack usage [-0, +0, -]
                   n++;
                }
              break;
           }

           case '$':
           {
              if (table)
                {
                   q = _elua_push_name(L, q, i); // Stack usage [-0, +1, e]
                }
              if (lua_isstring(L, j)) // Stack usage [-0, +0, -]
                {
                   char **v = va_arg(vl, char **);
                   size_t len;
                   char *temp = (char *)lua_tolstring(L, j, &len);       // Stack usage [-0, +0, m]

                   len++;      // Cater for the null at the end.
                   *v = malloc(len);
                   if (*v)
                     {
                        memcpy(*v, temp, len);
                        n++;
                     }
                }
              break;
           }

           case '!':
           {
              if (table)
                {
                   q = _elua_push_name(L, q, i); // Stack usage [-0, +1, e]
                }
              if (lua_isboolean(L, j)) // Stack usage [-0, +0, -]
                {
                   int *v = va_arg(vl, int *);
                   *v = lua_toboolean(L, j); // Stack usage [-0, +0, -]
                   n++;
                }
              break;
           }

           default:
           {
              get = EINA_FALSE;
              break;
           }
          }

        if (get)
          {
             if (table)
               {
                  // If this is a table, then we pushed a value on the stack, pop it off.
                  lua_pop(L, 1); // Stack usage [-n, +0, -]
               }
             else
               j++;
             count++;
          }
        p = q;
     }

   free(f);
   va_end(vl);
   if (count > n)
     n = 0;
   else if (table)
     n = 1;
   return n;
}

static int
_elua_ret(lua_State *L, char *params, ...) // Stack usage [-(2*n), +(2*n+1), em]
{
   va_list vl;
   char *f = strdup(params);
   char *p = f;
   int n = 0;

   if (!f) return -1;

   lua_newtable(L); // Stack usage [-0, +1, m]
   va_start(vl, params);

   while (*p)
     {
        char *q;
        Eina_Bool set = EINA_TRUE;

        while (isspace((int)*p))
          p++;
        q = p + 1;
        switch (*p)
          {
           case '%':
           {
              q = _elua_push_name(L, q, -1); // Stack usage [-0, +1, m]
              lua_pushinteger(L, va_arg(vl, int));        // Stack usage [-0, +1, -]
              break;
           }

           case '#':
           {
              q = _elua_push_name(L, q, -1); // Stack usage [-0, +1, m]
              lua_pushnumber(L, va_arg(vl, double));      // Stack usage [-0, +1, -]
              break;
           }

           case '$':
           {
              q = _elua_push_name(L, q, -1); // Stack usage [-0, +1, m]
              lua_pushstring(L, va_arg(vl, char *));      // Stack usage [-0, +1, m]
              break;
           }

           case '!':
           {
              q = _elua_push_name(L, q, -1); // Stack usage [-0, +1, m]
              lua_pushboolean(L, va_arg(vl, int));        // Stack usage [-0, +1, -]
              break;
           }

           default:
           {
              set = EINA_FALSE;
              break;
           }
          }

        if (set)
          {
             lua_settable(L, -3); // Stack usage [-2, +0, e]
             n++;
          }
        p = q;
     }

   free(f);
   va_end(vl);
   return n;
}

static void
_elua_color_fix(int *r, int *g, int *b, int *a)
{
   if (*r > *a) *r = *a;
   if (*g > *a) *g = *a;
   if (*b > *a) *b = *a;
}

//--------------------------------------------------------------------------//

/**
   @page luaref
   @subsection edje Edje class.

   The lua edje class includes functions for dealing with the lua script only group
   as an edje object, basic functions, and functions to create other objects.

   In the following, "edje" is the actual global table used to access these edje functions.
 */

static int _elua_echo(lua_State *L);

static int _elua_date(lua_State *L);
static int _elua_looptime(lua_State *L);
static int _elua_seconds(lua_State *L);
static int _elua_version(lua_State *L);

static int _elua_objgeom(lua_State *L);
static int _elua_objpos(lua_State *L);
static int _elua_objsize(lua_State *L);

static int _elua_emit(lua_State *L);
static int _elua_messagesend(lua_State *L);

static int _elua_animator(lua_State *L);
static int _elua_timer(lua_State *L);
static int _elua_transition(lua_State *L);

static int _elua_color_class(lua_State *L);
static int _elua_text_class(lua_State *L);

static int _elua_edje(lua_State *L);
static int _elua_image(lua_State *L);
static int _elua_line(lua_State *L);
static int _elua_map(lua_State *L);
static int _elua_polygon(lua_State *L);
static int _elua_rect(lua_State *L);
static int _elua_text(lua_State *L);
//static int _elua_textblock(lua_State *L);  /* XXX: disabled until there are enough textblock functions implemented to make it actually useful

static const char *_elua_edje_api = "edje";
static const struct luaL_Reg _elua_edje_funcs [] =
{
   // add an echo too to make it more shelly
   {"echo", _elua_echo},           // test func - echo (i know we have print. test)
   // FIXME: add logging functions here, probably to it's own domain, or even a script defined domain.

   // system information (time, date blah blah)
   {"date", _elua_date},           // get date in a table
   {"looptime", _elua_looptime},       // get loop time
   {"seconds", _elua_seconds},        // get seconds
   {"version", _elua_version},        // edje version

   // query edje - size, pos
   {"geom", _elua_objgeom},           // get while edje object geometry in canvas
   {"pos", _elua_objpos},            // get while edje object pos in canvas
   {"size", _elua_objsize},           // get while edje object pos in canvas

   // talk to application/caller
   {"emit", _elua_emit},           // emit signal + src
   {"messagesend", _elua_messagesend},    // send a structured message

   // time based "callback" systems
   {"animator", _elua_animator},       // add animator
   {"timer", _elua_timer},          // add timer
   {"transition", _elua_transition},     // add transition
   // FIXME: need poller

   // set and query color / text class
   {"color_class", _elua_color_class},
   {"text_class", _elua_text_class},

   // create new objects
   {"edje", _elua_edje},
   {"image", _elua_image},           // defaults to a filled image.
   {"line", _elua_line},
   {"map", _elua_map},
   {"polygon", _elua_polygon},
   {"rect", _elua_rect},
   {"text", _elua_text},
//     {"textblock",    _elua_textblock},  /* XXX: disabled until there are enough textblock functions implemented to make it actually useful

   // FIXME: add the new sound stuff.

   {NULL, NULL}   // end
};

/**
   @page luaref
   @subsubsection edje_echo edje.echo(text)

   Make lua a bit shelly.  Prints a string to the console

   @param text The string to print.
 */
static int
_elua_echo(lua_State *L) // Stack usage [-0, +0, v]
{
   const char *string = luaL_checkstring(L, 1);  // Stack usage [-0, +0, v]
   LD("%s", string);
   return 0;
}

//-------------
/**
   @page luaref
   @subsubsection edje_date edje.date()

   Retrieves the current time and date.

   Wraps gettimeofday(), as passed through localtime().

   @return A table with these fields:
   - integer year: Year.
   - integer month: Month of the year.
   - integer day: Day of the month.
   - integer yearday: Day of the year.
   - integer weekday: Day of the week.
   - integer hour: Hour of the day (24 hour format).
   - integer min: Minute of the hour.
   - number sec: Seconds as a number.

 */
static int
_elua_date(lua_State *L)  // Stack usage [-16, +17, em]
{
   static time_t last_tzset = 0;
   struct timeval timev;
   struct tm *tm;
   time_t tt;

   gettimeofday(&timev, NULL);
   tt = (time_t)(timev.tv_sec);
   if ((tt > (last_tzset + 1)) || (tt < (last_tzset - 1)))
     {
        last_tzset = tt;
        tzset();
     }
   tm = localtime(&tt);
   if (tm) // Stack usage [-16, +17, em]
     {
        _elua_ret(L, "%year %month %day %yearday %weekday %hour %min #sec",
                  (int)(tm->tm_year + 1900),
                  (int)(tm->tm_mon + 1),
                  (int)(tm->tm_mday),
                  (int)(tm->tm_yday),
                  (int)((tm->tm_wday + 6) % 7),
                  (int)(tm->tm_hour),
                  (int)(tm->tm_min),
                  (double)((double)tm->tm_sec + (((double)timev.tv_usec) / 1000000))
                  );
     }
   return 1;
}

/**
   @page luaref
   @subsubsection edje_looptime edje.looptime()

   Retrieves the time at which the last loop stopped waiting for timeouts or events.

   This gets the time that the main loop ceased waiting for timouts and/or events
   to come in or for signals or any other interrupt source. This should be
   considered a reference point for all time based activity that should calculate
   its timepoint from the return of edje.looptime(). Use this UNLESS you absolutely
   must get the current actual timepoint - then use edje.seconds(). Note that this
   time is meant to be used as relative to other times obtained on this run.

   Wraps ecore_loop_time_get().

   @returns A number of seconds.
 */
static int
_elua_looptime(lua_State *L)  // Stack usage [-0, +1, -]
{
   double t = ecore_loop_time_get();
   lua_pushnumber(L, t); // Stack usage [-0, +1, -]
   return 1;
}

/**
   @page luaref
   @subsubsection edje_seconds edje.seconds()

   Retrieves the current system time as a floating point value in seconds.

   This uses a monotonic clock and thus never goes back in time while machine is
   live (even if user changes time or timezone changes, however it may be reset
   whenever the machine is restarted).

   Wraps ecore_time_get().

   @returns A number of seconds.
 */
static int
_elua_seconds(lua_State *L)  // Stack usage [-0, +1, -]
{
   double t = ecore_time_get();
   lua_pushnumber(L, t);     // Stack usage [-0, +1, -]
   return 1;
}

/**
   @page luaref
   @subsubsection edje_version edje.version()

   Retrieves the current edje version number.

   @returns A table with these fields:
    - integer major: The edje version major number.
    - integer minor: The edje version minor number.

   @since 1.2.0
 */
static int
_elua_version(lua_State *L) // Stack usage [-4, +5, em]
{
   _elua_ret(L, "%major %minor", EDJE_VERSION_MAJOR, EDJE_VERSION_MINOR);  // Stack usage [-4, +5, em]
   return 1;
}

//-------------
/**
   @page luaref
   @subsubsection edje_geom edje.geom()

   Retrieves the position and size of the edje object that this lua group is in.

   @returns A table with these fields:
   - integer x: The edjes X position.
   - integer y: The edjes Y position.
   - integer w: The edjes width.
   - integer h: The edjes height.
 */
static int
_elua_objgeom(lua_State *L) // Stack usage [-10, +11, em]
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     // Stack usage [-2, +2, e]
   _elua_ret(L, "%x %y %w %h", ed->x, ed->y, ed->w, ed->h);  // Stack usage [-8, +9, em]
   return 1;
}

/**
   @page luaref
   @subsubsection edje_pos edje.pos()


   Retrieves the position of the edje object that this lua group is in.

   @returns A table with these fields:
   - integer x: The edjes X position.
   - integer y: The edjes Y position.
 */
static int
_elua_objpos(lua_State *L) // Stack usage [-6, +7, em]
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     // Stack usage [-2, +2, e]
   _elua_ret(L, "%x %y", ed->x, ed->y); // Stack usage [-4, +5, em]
   return 1;
}

/**
   @page luaref
   @subsubsection edje_size edje.size()


   Retrieves the size of the edje object that this lua group is in.

   @returns A table with these fields:
   - integer w: The edjes width.
   - integer h: The edjes height.
 */
static int
_elua_objsize(lua_State *L) // Stack usage [-6, +7, em]
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     // Stack usage [-2, +2, e]
   _elua_ret(L, "%w %h", ed->w, ed->h); // Stack usage [-4, +5, em]
   return 1;
}

//-------------
/**
   @page luaref
   @subsubsection edje_emit edje.emit(signal, source)

   Emit a signal.

   Wraps edje_object_signal_emit().

   @param signal The signal string to send.
   @param source The source string of the signal.

   NOTE: The source string will have a name and a colon prepended to in when it is
   delivered to things that are not this edje, like C and other edje groups.
   If this edje is a top level edje, then it will be the name of the group (I think).
   If this edje is swallowed into some other part, then it will be the name of the
   part:

   group_name:source

   FIXME: I actually have no idea what happens if it's swallowed into another lua
   edje group.
 */
static int
_elua_emit(lua_State *L) // Stack usage [-2, +2, ev]
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     // Stack usage [-2, +2, e]
   const char *sig = luaL_checkstring(L, 1); // Stack usage [-0, +0, v]
   const char *src = luaL_checkstring(L, 2); // Stack usage [-0, +0, v]
   if ((!sig) || (!src)) return 0;
   _edje_emit(ed, sig, src);
   return 0;
}

/**
   @page luaref
   @subsubsection edje_message_send edje.messagesend(id, type, ...)

   Send a message to this edje, and all it's child objects.

   Wraps edje_object_message_send().

   @param id   An identification integer for the message.
   @param type The type of message to send.
   @param ...  Zero or more things to send as part of the message, depending on the type.

   The type can be one of:
   - none: No msg.
   - sig: The msg is two strings (signal, source), sent as a signal.
   - str: The msg is a C string.
   - int: The message is a C integer.
   - float: The message is a C float.
   - strset: The message is an array of C strings.
   - intset: The message is an array of C integers.
   - floatset: The message is an array of C floats.
   - strint: The message is a C stnring and a C integer.
   - strfloat: The message is a C string and a C float.
   - strintset: The message is a C string and an array of C integers.
   - strfloatset: The message is a G string and an array of C floats.

   For the array types, the lua caller passes a table.
 */
static int
_elua_messagesend(lua_State *L)  // Stack usage [-2, +2, ev] plus [-2, +2] for every element if it's an array message.
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     // Stack usage [-2, +2, e]
   int id = luaL_checkinteger(L, 1); // Stack usage [-0, +0, v]
   const char *type = luaL_checkstring(L, 2); // Stack usage [-0, +0, v]
   if (!type) return 0;
   if (!strcmp(type, "none"))
     {
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_NONE, id, NULL);
     }
   else if (!strcmp(type, "sig"))
     {
        const char *sig = luaL_checkstring(L, 3); // Stack usage [-0, +0, v]
        const char *src = luaL_checkstring(L, 4); // Stack usage [-0, +0, v]
        _edje_emit(ed, sig, src);
     }
   else if (!strcmp(type, "str"))
     {
        Edje_Message_String *emsg;
        const char *str = luaL_checkstring(L, 3); // Stack usage [-0, +0, v]
        emsg = alloca(sizeof(Edje_Message_String));
        emsg->str = (char *)str;
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING, id, emsg);
     }
   else if (!strcmp(type, "int"))
     {
        Edje_Message_Int *emsg;
        int val = luaL_checkinteger(L, 3); // Stack usage [-0, +0, v]
        emsg = alloca(sizeof(Edje_Message_Int));
        emsg->val = val;
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_INT, id, emsg);
     }
   else if (!strcmp(type, "float"))
     {
        Edje_Message_Float *emsg;
        float val = luaL_checknumber(L, 3); // Stack usage [-0, +0, v]
        emsg = alloca(sizeof(Edje_Message_Float));
        emsg->val = val;
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_FLOAT, id, emsg);
     }
   else if (!strcmp(type, "strset"))
     {
        Edje_Message_String_Set *emsg;
        int i, n;
        const char *str;
        luaL_checktype(L, 3, LUA_TTABLE); // Stack usage [-0, +0, v]
#if LUA_VERSION_NUM >= 502
        n = lua_rawlen(L, 3); // Stack usage [-0, +0, -]
#else
        n = lua_objlen(L, 3); // Stack usage [-0, +0, -]
#endif
        emsg = alloca(sizeof(Edje_Message_String_Set) + ((n - 1) * sizeof(char *)));
        emsg->count = n;
        for (i = 1; i <= n; i++)
          {
             lua_pushinteger(L, i); // Stack usage [-0, +1, -]
             lua_gettable(L, 3); // Stack usage [-1, +1, e]
             str = lua_tostring(L, -1); // Stack usage [-0, +0, m]
             lua_pop(L, 1); // Stack usage [-n, +0, -]
             emsg->str[i - 1] = (char *)str;
          }
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING_SET, id, emsg);
     }
   else if (!strcmp(type, "intset"))
     {
        Edje_Message_Int_Set *emsg;
        int i, n;
        luaL_checktype(L, 3, LUA_TTABLE); // Stack usage [-0, +0, v]
#if LUA_VERSION_NUM >= 502
        n = lua_rawlen(L, 3); // Stack usage [-0, +0, -]
#else
        n = lua_objlen(L, 3); // Stack usage [-0, +0, -]
#endif
        emsg = alloca(sizeof(Edje_Message_Int_Set) + ((n - 1) * sizeof(int)));
        emsg->count = n;
        for (i = 1; i <= n; i++)
          {
             lua_pushinteger(L, i); // Stack usage [-0, +1, -]
             lua_gettable(L, 3); // Stack usage [-1, +1, e]
             emsg->val[i - 1] = lua_tointeger(L, -1); // Stack usage [-0, +0, -]
             lua_pop(L, 1); // Stack usage [-n, +0, -]
          }
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_INT_SET, id, emsg);
     }
   else if (!strcmp(type, "floatset"))
     {
        Edje_Message_Float_Set *emsg;
        int i, n;
        luaL_checktype(L, 3, LUA_TTABLE); // Stack usage [-0, +0, v]
#if LUA_VERSION_NUM >= 502
        n = lua_rawlen(L, 3); // Stack usage [-0, +0, -]
#else
        n = lua_objlen(L, 3); // Stack usage [-0, +0, -]
#endif
        emsg = alloca(sizeof(Edje_Message_Float_Set) + ((n - 1) * sizeof(double)));
        emsg->count = n;
        for (i = 1; i <= n; i++)
          {
             lua_pushinteger(L, i); // Stack usage [-0, +1, -]
             lua_gettable(L, 3); // Stack usage [-1, +1, e]
             emsg->val[i - 1] = lua_tonumber(L, -1); // Stack usage [-0, +0, -]
             lua_pop(L, 1); // Stack usage [-n, +0, -]
          }
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_FLOAT_SET, id, emsg);
     }
   else if (!strcmp(type, "strint"))
     {
        Edje_Message_String_Int *emsg;
        const char *str = luaL_checkstring(L, 3); // Stack usage [-0, +0, v]
        emsg = alloca(sizeof(Edje_Message_String_Int));
        emsg->str = (char *)str;
        emsg->val = luaL_checkinteger(L, 4); // Stack usage [-0, +0, v]
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING_INT, id, emsg);
     }
   else if (!strcmp(type, "strfloat"))
     {
        Edje_Message_String_Float *emsg;
        const char *str = luaL_checkstring(L, 3); // Stack usage [-0, +0, v]
        emsg = alloca(sizeof(Edje_Message_String_Float));
        emsg->str = (char *)str;
        emsg->val = luaL_checknumber(L, 4); // Stack usage [-0, +0, v]
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING_FLOAT, id, emsg);
     }
   else if (!strcmp(type, "strintset"))
     {
        Edje_Message_String_Int_Set *emsg;
        int i, n;
        const char *str = luaL_checkstring(L, 3); // Stack usage [-0, +0, v]
        if (!str) return 0;
        luaL_checktype(L, 4, LUA_TTABLE); // Stack usage [-0, +0, v]
#if LUA_VERSION_NUM >= 502
        n = lua_rawlen(L, 4); // Stack usage [-0, +0, -]
#else
        n = lua_objlen(L, 4); // Stack usage [-0, +0, -]
#endif
        emsg = alloca(sizeof(Edje_Message_String_Int_Set) + ((n - 1) * sizeof(int)));
        emsg->str = (char *)str;
        emsg->count = n;
        for (i = 1; i <= n; i++)
          {
             lua_pushinteger(L, i); // Stack usage [-0, +1, -]
             lua_gettable(L, 4); // Stack usage [-1, +1, e]
             emsg->val[i - 1] = lua_tointeger(L, -1); // Stack usage [-0, +0, -]
             lua_pop(L, 1); // Stack usage [-n, +0, -]
          }
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING_INT_SET, id, emsg);
     }
   else if (!strcmp(type, "strfloatset"))
     {
        Edje_Message_String_Float_Set *emsg;
        int i, n;
        const char *str = luaL_checkstring(L, 3); // Stack usage [-0, +0, v]
        if (!str) return 0;
        luaL_checktype(L, 4, LUA_TTABLE); // Stack usage [-0, +0, v]
#if LUA_VERSION_NUM >= 502
        n = lua_rawlen(L, 4); // Stack usage [-0, +0, -]
#else
        n = lua_objlen(L, 4);
#endif
        emsg = alloca(sizeof(Edje_Message_String_Float_Set) + ((n - 1) * sizeof(double)));
        emsg->str = (char *)str;
        emsg->count = n;
        for (i = 1; i <= n; i++)
          {
             lua_pushinteger(L, i); // Stack usage [-0, +1, -]
             lua_gettable(L, 4); // Stack usage [-1, +1, e]
             emsg->val[i - 1] = lua_tonumber(L, -1); // Stack usage [-0, +0, -]
             lua_pop(L, 1); // Stack usage [-n, +0, -]
          }
        _edje_util_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING_FLOAT_SET, id, emsg);
     }
   return 0;
}

//-------------
static Eina_Bool
_elua_animator_cb(void *data) // Stack usage [-2, +2, em]
{
   Edje_Lua_Animator *ela = data;
   lua_State *L;
   int ret = 0, err = 0;

   if (!ela->obj.ed) return 0;
   L = ela->obj.ed->L;
   if (!L) return 0;
   /* This is not needed, pcalls don't longjmp(), that's why they are protected.
      if (setjmp(panic_jmp) == 1)
      {
        LE("Animator callback panic");
        _edje_lua2_error(L, err);                            // Stack usage [-0, +0, m]
        _elua_obj_free(L, (Edje_Lua_Obj *)ela);
        _elua_gc(L);                                         // Stack usage [-0, +0, e]
        return 0;
      }
    */
   lua_rawgeti(L, LUA_REGISTRYINDEX, ela->fn_ref); // Stack usage [-0, +1, -]
   if ((err = lua_pcall(L, 0, 1, 0))) // Stack usage [-1, +1, -]
     {
        _edje_lua2_error(L, err); // Stack usage [-0, +0, m]
        _elua_obj_free(L, (Edje_Lua_Obj *)ela);
        _elua_gc(L); // Stack usage [-0, +0, e]
        return 0;
     }
   ret = lua_toboolean(L, -1); // Stack usage [-0, +0, -]
   lua_pop(L, 1); // Stack usage [-n, +0, -]
   if (ret == 0) _elua_obj_free(L, (Edje_Lua_Obj *)ela);
   _elua_gc(L); // Stack usage [-0, +0, e]
   return ret;
}

static void
_elua_animator_free(void *obj) // Stack usage [-0, +0, -]
{
   Edje_Lua_Animator *ela = obj;
   lua_State *L;
   if (!ela->obj.ed) return;
   L = ela->obj.ed->L;
   luaL_unref(L, LUA_REGISTRYINDEX, ela->fn_ref); // Stack usage [-0, +0, -]
   ela->fn_ref = 0;
   ecore_animator_del(ela->animator);
   ela->animator = NULL;
}

/**
   @page luaref
   @subsubsection edje_animator edje.animator(func)

   This function adds an animator and returns its handle on success and NULL on
   failure. The function func will be called every frame tick.  Note that setting
   the frame tick is not available as a lua function, so has to be done from C.
   The default tick is 1/30 second.

   When the animator func is called, it must return a value of either true or false.
   If it returns true it will be called again at the next tick, or if it returns
   false it will be deleted automatically making any references/handles for it
   invalid.

   Wraps ecore_animator_add().

   @param func The function to call when the animator triggers.

   @returns A userdata that is an ecore animator.
 */
static int
_elua_animator(lua_State *L) // Stack usage [-8, +9, emv]
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     // Stack usage [-2, +2, e]
   Edje_Lua_Animator *ela;

   luaL_checkany(L, 1); // Stack usage [-0, +0, v]

   // FIXME: Allow lua to set a data to be sent back with the callback.
   ela = (Edje_Lua_Animator *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Animator), _elua_ecore_animator_meta);
   // Stack usage [-5, +6, m]
   ela->obj.free_func = _elua_animator_free;
   ela->animator = ecore_animator_add(_elua_animator_cb, ela);
   lua_pushvalue(L, 1); // Stack usage [-0, +1, -]
   ela->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX); // Stack usage [-1, +0, m]
   _elua_gc(L); // Stack usage [-0, +0, e]
   return 1;
}

static Eina_Bool
_elua_timer_cb(void *data) // Stack usage [-2, +2, em]
{
   Edje_Lua_Timer *elt = data;
   lua_State *L;
   int ret = 0, err = 0;

   if (!elt->obj.ed) return 0;
   L = elt->obj.ed->L;
   if (!L) return 0;
   /* This is not needed, pcalls don't longjmp(), that's why they are protected.
      if (setjmp(panic_jmp) == 1)
      {
        LE("Timer callback panic");
        _edje_lua2_error(L, err);                            // Stack usage [-0, +0, m]
        _elua_obj_free(L, (Edje_Lua_Obj *)elt);
        _elua_gc(L);                                         // Stack usage [-0, +0, e]
        return 0;
      }
    */
   lua_rawgeti(L, LUA_REGISTRYINDEX, elt->fn_ref); // Stack usage [-0, +1, -]
   if ((err = lua_pcall(L, 0, 1, 0))) // Stack usage [-1, +1, -]
     {
        _edje_lua2_error(L, err);
        _elua_obj_free(L, (Edje_Lua_Obj *)elt); // Stack usage [-0, +0, m]
        _elua_gc(L); // Stack usage [-0, +0, e]
        return 0;
     }
   ret = lua_toboolean(L, -1); // Stack usage [-0, +0, -]
   lua_pop(L, 1); // Stack usage [-n, +0, -]
   if (ret == 0) _elua_obj_free(L, (Edje_Lua_Obj *)elt);
   _elua_gc(L); // Stack usage [-0, +0, e]
   return ret;
}

static void
_elua_timer_free(void *obj) // Stack usage [-0, +0, -]
{
   Edje_Lua_Timer *elt = obj;
   lua_State *L;
   if (!elt->obj.ed) return;
   L = elt->obj.ed->L;
   luaL_unref(L, LUA_REGISTRYINDEX, elt->fn_ref); // Stack usage [-0, +0, -]
   elt->fn_ref = 0;
   ecore_timer_del(elt->timer);
   elt->timer = NULL;
}

/**
   @page luaref
   @subsubsection edje_timer edje.timer(tick, func)

   This function adds a timer and returns its handle on success and NULL on failure.
   The function func will be called every tick seconds.

   When the timer func is called, it must return a value of either true or false.
   If it returns true, it will be called again at the next tick, or if it returns
   false it will be deleted automatically making any references/handles for it
   invalid.

   Wraps ecore_timer_add().

   @param tick How often, in seconds, to call the function.
   @param func The function to call when the timer triggers.

   @returns A userdata that is an ecore timer.
 */
static int
_elua_timer(lua_State *L) // Stack usage [-8, +9, emv]
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     // Stack usage [-2, +2, e]
   Edje_Lua_Timer *elt;
   double val;

   val = luaL_checknumber(L, 1); // Stack usage [-0, +0, v]
   luaL_checkany(L, 2); // Stack usage [-0, +0, v]

   elt = (Edje_Lua_Timer *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Timer), _elua_ecore_timer_meta);
   // Stack usage [-5, +6, m]
   elt->obj.free_func = _elua_timer_free;
   elt->timer = ecore_timer_add(val, _elua_timer_cb, elt);
   lua_pushvalue(L, 2); // Stack usage [-0, +1, -]
   elt->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX); // Stack usage [-1, +0, m]
   _elua_gc(L); // Stack usage [-0, +0, e]
   return 1;
}

static Eina_Bool
_elua_transition_cb(void *data) // Stack usage [-3, +3, em]
{
   Edje_Lua_Transition *elt = data;
   lua_State *L;
   int ret = 0, err = 0;
   double t;

   if (!elt->obj.ed) return 0;
   L = elt->obj.ed->L;
   if (!L) return 0;
   t = (ecore_loop_time_get() - elt->start) / elt->transition;
   if (t > 1.0) t = 1.0;
   /* This is not needed, pcalls don't longjmp(), that's why they are protected.
      if (setjmp(panic_jmp) == 1)
      {
        LE("Transition callback panic");
        _edje_lua2_error(L, err);                            // Stack usage [-0, +0, m]
        _elua_obj_free(L, (Edje_Lua_Obj *)elt);
        _elua_gc(L);                                         // Stack usage [-0, +0, e]
        return 0;
      }
    */
   lua_rawgeti(L, LUA_REGISTRYINDEX, elt->fn_ref); // Stack usage [-0, +1, -]
   lua_pushnumber(L, t); // Stack usage [-0, +1, -]
   if ((err = lua_pcall(L, 1, 1, 0))) // Stack usage [-2, +1, -]
     {
        _edje_lua2_error(L, err);
        _elua_obj_free(L, (Edje_Lua_Obj *)elt); // Stack usage [-0, +0, m]
        _elua_gc(L); // Stack usage [-0, +0, e]
        return 0;
     }
   ret = lua_toboolean(L, -1); // Stack usage [-0, +0, -]
   lua_pop(L, 1); // Stack usage [-n, +0, -]
   if (t >= 1.0) ret = 0;
   if (ret == 0) _elua_obj_free(L, (Edje_Lua_Obj *)elt);
   _elua_gc(L); // Stack usage [-0, +0, e]
   return ret;
}

static void
_elua_transition_free(void *obj) // Stack usage [-0, +0, -]
{
   Edje_Lua_Transition *elt = obj;
   lua_State *L;
   if (!elt->obj.ed) return;
   L = elt->obj.ed->L;
   luaL_unref(L, LUA_REGISTRYINDEX, elt->fn_ref); // Stack usage [-0, +0, -]
   elt->fn_ref = 0;
   ecore_animator_del(elt->animator);
   elt->animator = NULL;
}

/**
   @page luaref
   @subsubsection edje_transition edje.transition(div, func)

   Just like edje.animator(), except that the callback function gets called with an
   argument.  The argument is the amount of time since the transition was created,
   divided by the div parameter.

   @param div A number to divide the time since creation by.
   @param func The function to call when the transition triggers.

   @returns A userdata that is a transition (ecore animator, plus other info).
 */
static int
_elua_transition(lua_State *L) // Stack usage [-8, +9, emv]
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     // Stack usage [-2, +2, e]
   Edje_Lua_Transition *elt;
   double val;

   val = luaL_checknumber(L, 1); // Stack usage [-0, +0, v]
   luaL_checkany(L, 2); // Stack usage [-0, +0, v]

   elt = (Edje_Lua_Transition *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Transition), _elua_ecore_animator_meta);
   // Stack usage [-5, +6, m]
   elt->obj.free_func = _elua_transition_free;
   elt->animator = ecore_animator_add(_elua_transition_cb, elt);
   if (val < 0.0000001) val = 0.0000001;
   elt->transition = val;
   elt->start = ecore_loop_time_get();
   lua_pushvalue(L, 2); // Stack usage [-0, +1, -]
   elt->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX); // Stack usage [-1, +0, m]
   _elua_gc(L); // Stack usage [-0, +0, e]
   return 1;
}

//-------------
/**
   @page luaref
   @subsubsection edje_colour_class edje.color_class(class, r, g, b, a)

   Gets, (and optionally sets) the colours for a color class.

   Wraps edje_object_color_class_set().

   @param class A color class name.
   @param r The new red value.
   @param g The new green value.
   @param b The new blue value.
   @param a The new alpha value.

   Note that the r, g, b, and a arguments are optional, without them this function
   just queries the current values.  The r, g, b, and a arguments can be separate
   values, or named fields in a table.

   @return A table with these fields:
   - integer r: The red value.
   - integer g: The green value.
   - integer b: The blue value.
   - integer a: The alpha value.

   @since 1.1.0
 */
static int
_elua_color_class(lua_State *L) // Stack usage [-(10|14), +(11|15), ?]
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     // Stack usage [-2, +2, e]
   Edje_Color_Class *c_class;
   const char *class = luaL_checkstring(L, 1); // Stack usage [-0, +0, v]
   int r, g, b, a;

   if (!class) return 0;

   if (_elua_scan_params(L, 2, "%r %g %b %a", &r, &g, &b, &a) > 0) // Stack usage [-0, +0, m] unless it's in a table [-4, +4, e]
     {
        _elua_color_fix(&r, &g, &b, &a);
        // This is the way that embryo does it -
        //edje_object_color_class_set(ed->obj, class, r, g, b, a, r, g, b, a, r, g, b, a);
        // But that deals with object scope, which is currently useless in lua,
        // since we have no objects that can use color_class yet.
        // So we do it at global scope instead.
        // LATER - Should do both?
        edje_color_class_set(class, r, g, b, a, r, g, b, a, r, g, b, a);
     }

   c_class = _edje_color_class_find(ed, class);
   if (!c_class) return 0;

   _elua_ret(L, "%r %g %b %a", c_class->r, c_class->g, c_class->b, c_class->a);
   // Stack usage [-8, +9, em]
   return 1;
}

/**
   @page luaref
   @subsubsection edje_text_class edje.text_class(class, font, size)

   Gets, (and optionally sets) the details for a text class.

   Wraps edje_object_text_class_set().

   @param class A text class name.
   @param font The new font name.
   @param size The new font size.

   Note that the font and size arguments are optional, without them this function
   just queries the current values.  The font and size arguments can be separate
   values, or named fields in a table.  The font name can refer to a font in the
   edje file, or an external font.

   @return A table with these fields:
   - string font: The font name.
   - integer size: The font size.

   @since 1.1.0
 */
static int
_elua_text_class(lua_State *L) // Stack usage [-(6|8), +(7|9), emv]
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     // Stack usage [-2, +2, e]
   Edje_Text_Class *t_class;
   const char *class = luaL_checkstring(L, 1); // Stack usage [-0, +0, v]
   char *font = NULL;
   Evas_Font_Size size = 0;

   if (!class) return 0;

   // Just like color_class above, this does things differently from embryo,
   // for the same reason.
   if (_elua_scan_params(L, 2, "$font %size", &font, &size) > 0)
     // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
     edje_text_class_set(class, font, size);

   t_class = _edje_text_class_find(ed, class);
   if (!t_class) return 0;

   _elua_ret(L, "$font %size", t_class->font, t_class->size);
   // Stack usage [-4, +5, em]
   return 1;
}

//-------------
static void
_elua_efl_canvas_object_free(void *obj)
{
   Edje_Lua_Evas_Object *elo = obj;

   if (!elo->obj.ed) return;
   evas_object_del(elo->evas_obj);
   elo->evas_obj = NULL;
}

// Stack usage [-7, +8, em]
#define _ELUA_PLANT_EVAS_OBJECT(type, meta, free)         \
  Edje * ed = (Edje *)_elua_table_ptr_get(L, _elua_key);  \
  type *elo;                                              \
  elo = (type *)_elua_obj_new(L, ed, sizeof(type), meta); \
  elo->obj.free_func = free;
// Stack usage [-2, +2, e]
// Stack usage [-5, +6, m]

static void
_elua_polish_evas_object(Edje *ed, Edje_Lua_Evas_Object *elo)
{
   evas_object_smart_member_add(elo->evas_obj, ed->obj);
   evas_object_clip_set(elo->evas_obj, ed->base.clipper);
   evas_object_move(elo->evas_obj, ed->x, ed->y);
   evas_object_resize(elo->evas_obj, 0, 0);
   evas_object_data_set(elo->evas_obj, ELO, elo);
}

/**
   @page luaref
   @subsubsection edje_edje edje.edje()

   Create an edje object, and add it to the edje.

   Wraps edje_object_add().

   @returns A userdata that is an edje object.

   @since 1.1.0
 */
static int
_elua_edje(lua_State *L) // Stack usage [-7, +8, em]
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_edje_meta, _elua_efl_canvas_object_free)
   // Stack usage [-7, +8, em]
   elo->evas_obj = edje_object_add(evas_object_evas_get(ed->obj));
   _edje_subobj_register(ed, elo->evas_obj);
   _elua_polish_evas_object(ed, elo);
   return 1;
}

/**
   @page luaref
   @subsubsection edje_image edje.image()

   Create an evas image, and add it to the edje.

   Wraps evas_object_image_add().

   @returns A userdata that is an evas image.

   @since 1.1.0
 */
static int
_elua_image(lua_State *L) // Stack usage [-7, +8, em]
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_image_meta, _elua_efl_canvas_object_free)
   // Stack usage [-7, +8, em]
   elo->evas_obj = evas_object_image_filled_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
}

/**
   @page luaref
   @subsubsection edje_line edje.line()

   Create an evas line, and add it to the edje.

   Wraps evas_object_line_add().

   @returns A userdata that is an evas line.

   @since 1.1.0
 */
static int
_elua_line(lua_State *L) // Stack usage [-7, +8, em]
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_line_meta, _elua_efl_canvas_object_free)
   // Stack usage [-7, +8, em]
   elo->evas_obj = evas_object_line_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
}

static void
_elua_map_free(void *obj)
{
   Edje_Lua_Map *elm = obj;
   if (!elm->obj.ed) return;
   evas_map_free(elm->map);
   elm->map = NULL;
}

/**
   @page luaref
   @subsubsection edje_map edje.map()

   Create an evas map.

   Wraps evas_map_new().

   @returns A userdata that is an evas map.

   @since 1.1.0
 */
static int
_elua_map(lua_State *L) // Stack usage [-7, +8, emv]
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Map, _elua_evas_map_meta, _elua_map_free)
   // Stack usage [-7, +8, em]
   elo->map = evas_map_new(luaL_checkinteger(L, 1)); // Stack usage [-0, +0, v]
   return 1;
}

/**
   @page luaref
   @subsubsection edje_polygon edje.polygon()

   Create an evas polygon, and add it to the edje.

   Wraps evas_object_polygon_add().

   @returns A userdata that is an evas polygon.

   @since 1.1.0
 */
static int
_elua_polygon(lua_State *L) // Stack usage [-7, +8, em]
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_polygon_meta, _elua_efl_canvas_object_free)
   // Stack usage [-7, +8, em]
   elo->evas_obj = evas_object_polygon_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
}

/**
   @page luaref
   @subsubsection edje_rect edje.rect()

   Create an evas rectangle, and add it to the edje.

   Wraps evas_object_rectangle_add().

   @returns A userdata that is an evas rectangle.
 */
static int
_elua_rect(lua_State *L) // Stack usage [-7, +8, em]
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_meta, _elua_efl_canvas_object_free)
   // Stack usage [-7, +8, em]
   elo->evas_obj = evas_object_rectangle_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
}

/**
   @page luaref
   @subsubsection edje_text edje.text()

   Create an evas text object, and add it to the edje.

   Wraps evas_object_text_add().

   @returns A userdata that is an evas text object.

   @since 1.1.0
 */
static int
_elua_text(lua_State *L) // Stack usage [-7, +8, em]
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_text_meta, _elua_efl_canvas_object_free)
   // Stack usage [-7, +8, em]
   elo->evas_obj = evas_object_text_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
}

/* XXX: disabled until there are enough textblock functions implemented to make it actually useful
   _elua_textblock(lua_State *L)                               // Stack usage [-7, +8, em]
   {
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_textblock_meta, _elua_efl_canvas_object_free)
                                                            // Stack usage [-7, +8, em]
   elo->evas_obj = evas_object_textblock_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
   }
 */

//-------------
//-------------

/**
   @page luaref
   @subsection evas Evas class.

   The lua evas class includes functions for dealing with evas objects.  The evas
   objects must have been previously created by lua using one of the lua evas
   object creation functions from the lua edje class.

   In the following, "evas_object" is a place holder for any lua variable that
   holds a reference to an evas object.
 */

static int _elua_hide(lua_State *L);
static int _elua_show(lua_State *L);
static int _elua_visible(lua_State *L);

static int _elua_above(lua_State *L);
static int _elua_below(lua_State *L);
static int _elua_bottom(lua_State *L);
static int _elua_lower(lua_State *L);
static int _elua_raise(lua_State *L);
static int _elua_top(lua_State *L);

static int _elua_geom(lua_State *L);
static int _elua_move(lua_State *L);
static int _elua_pos(lua_State *L);
static int _elua_resize(lua_State *L);
static int _elua_size(lua_State *L);

static int _elua_clip(lua_State *L);
static int _elua_clipees(lua_State *L);
static int _elua_unclip(lua_State *L);

static int _elua_type(lua_State *L);

static int _elua_pass(lua_State *L);
static int _elua_precise(lua_State *L);
static int _elua_repeat(lua_State *L);

static int _elua_color(lua_State *L);

static int _elua_obj_map(lua_State *L);
static int _elua_obj_map_enable(lua_State *L);

static const char *_elua_evas_api = "evas";
static const struct luaL_Reg _elua_evas_funcs [] =
{
   {"del", _elua_obj_del},            // generic del any object created for edje (evas objects, timers, animators, transitions... everything)

   {"hide", _elua_hide},           // hide, return current visibility
   {"show", _elua_show},           // show, return current visibility
   {"visible", _elua_visible},        // get object visibility

   {"above", _elua_above},          // get object above or stack obj above given obj
   {"below", _elua_below},          // get object below or stack obj below given obj
   {"bottom", _elua_bottom},         // get bottom
   {"lower", _elua_lower},          // lower to bottom
   {"raise", _elua_raise},          // raise to top
   {"top", _elua_top},            // get top

   {"geom", _elua_geom},           // move and resize and return current geometry
   {"move", _elua_move},           // move, return current position
   {"pos", _elua_pos},            // move, return current position
   {"resize", _elua_resize},         // resize, return current size
   {"size", _elua_size},           // resize, return current size

   {"clip", _elua_clip},           // set clip obj, return clip object
   {"clipees", _elua_clipees},        // get clip children
   {"unclip", _elua_unclip},         // clear clip obj

   {"type", _elua_type},           // get object type

   {"pass", _elua_pass},           // set pass events, get pass events
   {"precise", _elua_precise},        // set precise inside flag, get precise
   {"repeat", _elua_repeat},         // set repeat events, get repeat events

   {"color", _elua_color},          // set color, return color
//     {"color_class",  _elua_object_color_class}, // get or set object color class

   // FIXME: set callbacks (mouse down, up, blah blah blah)
   //
   // FIXME: set scale (explicit value)
   // FIXME: need to set auto-scale (same as scale: 1)

   // FIXME: later - set render op, anti-alias, pointer mode (autograb, nograb)

   // map api here
   {"map", _elua_obj_map},
   {"map_enable", _elua_obj_map_enable},

   {NULL, NULL}   // end
};

//-------------
/**
   @page luaref
   @subsubsection evas_hide evas_object:hide()

   Hides the object.

   Wraps evas_object_hide().

   @returns A boolean representing the current visibility.
 */
static int
_elua_hide(lua_State *L) // Stack usage [-0, +1, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_hide(elo->evas_obj);
   lua_pushboolean(L, evas_object_visible_get(elo->evas_obj));  // Stack usage [-0, +1, -]
   return 1;
}

/**
   @page luaref
   @subsubsection evas_show evas_object:show()

   Shows the object.

   Wraps evas_object_show().

   @returns A boolean representing the current visibility.
 */
static int
_elua_show(lua_State *L) // Stack usage [-0, +1, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_show(elo->evas_obj);
   lua_pushboolean(L, evas_object_visible_get(elo->evas_obj));  // Stack usage [-0, +1, -]
   return 1;
}

/**
   @page luaref
   @subsubsection evas_visible evas_object:visible(visibility)

   Gets (and optionally sets) this objects visibility.

   Wraps evas_object_hide() or evas_object_show().

   @param visibility The new visibility you want to change it to.

   Note that the argument is optional, without it this function just queries the
   current value.

   @returns A boolean representing the current visibility.
 */
static int
_elua_visible(lua_State *L) // Stack usage [-0, +1, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (n == 2)
     {
        if (lua_isboolean(L, 2)) // Stack usage [-0, +0, -]
          {
             if (lua_toboolean(L, 2)) evas_object_show(elo->evas_obj);
             // Stack usage [-0, +0, -]
             else evas_object_hide(elo->evas_obj);
          }
     }
   lua_pushboolean(L, evas_object_visible_get(elo->evas_obj));  // Stack usage [-0, +1, -]
   return 1;
}

//-------------
/**
   @page luaref
   @subsubsection evas_above evas_object:above()

   Figure out what, if anything, is above us.

   Wraps evas_object_above_get().

   Note that it may not return any value.

   @returns A reference to the object above this one.
 */
static int
_elua_above(lua_State *L) // Stack usage [-3, +4, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Edje_Lua_Evas_Object *elo2;
   Evas_Object *o;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   if (!(o = evas_object_above_get(elo->evas_obj))) return 0;
   if (!(elo2 = evas_object_data_get(o, ELO))) return 0;
   _elua_ref_get(L, elo2); // Stack usage [-3, +4, -]
   return 1;
}

/**
   @page luaref
   @subsubsection evas_below evas_object:below()

   Figure out what, if anything, is below us.

   Wraps evas_object_below_get().

   Note that it may not return any value.

   @returns A reference to the object below this one.
 */
static int
_elua_below(lua_State *L) // Stack usage [-3, +4, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Edje_Lua_Evas_Object *elo2;
   Evas_Object *o;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   if (!(o = evas_object_below_get(elo->evas_obj))) return 0;
   if (!(elo2 = evas_object_data_get(o, ELO))) return 0;
   _elua_ref_get(L, elo2); // Stack usage [-3, +4, -]
   return 1;
}

/**
   @page luaref
   @subsubsection evas_bottom evas_object:bottom()

   Figure out what, if anything, is waaaay below us.

   Note that it may not return any value.

   @returns A reference to the object at the bottom.
 */
static int
_elua_bottom(lua_State *L) // Stack usage [-(0|3), +(0|4), -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo2;
   Evas_Object *o;
   Eina_List *list, *l;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   if (!(list = (Eina_List *)evas_object_smart_members_get(obj->ed->obj))) return 0;
   for (l = list; l; l = l->next)
     {
        o = l->data;
        if ((elo2 = evas_object_data_get(o, ELO)))
          {
             _elua_ref_get(L, elo2); // Stack usage [-3, +4, -]
             return 1;
          }
     }
   return 0;
}

/**
   @page luaref
   @subsubsection evas_lower evas_object:lower()

   Lower this object to the bottom.

   Wraps evas_object_lower().
 */
static int
_elua_lower(lua_State *L) // Stack usage [-0, +0, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_lower(elo->evas_obj);
   return 0;
}

/**
   @page luaref
   @subsubsection evas_raise evas_object:raise()

   Raise this object to the top.

   Wraps evas_object_raise().
 */
static int
_elua_raise(lua_State *L) // Stack usage [-0, +0, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_raise(elo->evas_obj);
   return 0;
}

/**
   @page luaref
   @subsubsection evas_top evas_object:top()

   Figure out what, if anything, is waaaay above us.

   Note that it may not return any value.

   @returns A reference to the object at the top.
 */
static int
_elua_top(lua_State *L) // Stack usage [-(0|3), +(0|4), -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-(0, +0, -]
   Edje_Lua_Evas_Object *elo2;
   Evas_Object *o;
   Eina_List *list, *l;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   list = evas_object_smart_members_get(obj->ed->obj);
   if (!list) return 0;
   for (l = eina_list_last(list); l; l = l->prev)
     {
        o = l->data;
        if ((elo2 = evas_object_data_get(o, ELO)))
          {
             _elua_ref_get(L, elo2); // Stack usage [-3, +4, -]
             return 1;
          }
     }
   return 0;
}

//-------------
/**
   @page luaref
   @subsubsection evas_geom evas_object:geom(x, y, w, h)

   Gets (and optionally sets) this objects geometry.

   Wraps evas_object_move() and evas_object_resize.

   @param x The new X coordinate.
   @param y The new Y coordinate.
   @param w The new width.
   @param h The new height.

   Note that the arguments are optional, without them this function just queries
   the current values.  The arguments can be separate values, or named fields in a
   table.

   @return A table with these fields:
   - integer x: X coordinate.
   - integer x: Y coordinate.
   - integer w: Width.
   - integer w: Height.
 */
static int
_elua_geom(lua_State *L) // Stack usage [-(8|12), +(9|13), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord ox, oy, ow, oh;
   int x, y, w, h;

   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_geometry_get(elo->evas_obj, &ox, &oy, &ow, &oh);
   if (_elua_scan_params(L, 2, "%x %y %w %h", &x, &y, &w, &h) > 0) // Stack usage [-0, +0, m] unless it's in a table [-4, +4, e]
     {
        if ((x != (ox - obj->ed->x)) || (y != (oy - obj->ed->y)))
          {
             evas_object_move(elo->evas_obj,
                              obj->ed->x + x,
                              obj->ed->y + y);
          }
        if ((w != ow) || (h != oh))
          {
             evas_object_resize(elo->evas_obj, w, h);
          }
        evas_object_geometry_get(elo->evas_obj, &ox, &oy, &ow, &oh);
        elo->x = ox - obj->ed->x;
        elo->y = oy - obj->ed->y;
     }
   _elua_ret(L, "%x %y %w %h", elo->x, elo->y, ow, oh);
   // Stack usage [-8, +9, em]
   return 1;
}

/**
   @page luaref
   @subsubsection evas_move evas_object:move(x, y)

   Gets (and optionally sets) this objects position.

   Wraps evas_object_move().

   @param x The new X coordinate.
   @param y The new Y coordinate.

   Note that the arguments are optional, without them this function just queries
   the current values.  The arguments can be separate values, or named fields in a
   table.

   @return A table with these fields:
   - integer x: X coordinate.
   - integer x: Y coordinate.
 */
static int
_elua_move(lua_State *L) // Stack usage [-(4|6), +(5|7), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord ox, oy;
   int x, y;

   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_geometry_get(elo->evas_obj, &ox, &oy, NULL, NULL);
   if (_elua_scan_params(L, 2, "%x %y", &x, &y) > 0) // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
     {
        if ((x != (ox - obj->ed->x)) || (y != (oy - obj->ed->y)))
          {
             evas_object_move(elo->evas_obj,
                              obj->ed->x + x,
                              obj->ed->y + y);
             evas_object_geometry_get(elo->evas_obj, &ox, &oy, NULL, NULL);
          }
        elo->x = ox - obj->ed->x;
        elo->y = oy - obj->ed->y;
     }
   _elua_ret(L, "%x %y", elo->x, elo->y);
   // Stack usage [-4, +5, em]
   return 1;
}

/**
   @page luaref
   @subsubsection evas_pos evas_object:pos(x, y)

   An alias for evas_object:move().
 */
static int
_elua_pos(lua_State *L) // Stack usage [-(4|6), +(5|7), em]
{
   return _elua_move(L);
}

/**
   @page luaref
   @subsubsection evas_resize evas_object:resize(w, h)

   Gets (and optionally sets) this objects size.

   Wraps evas_object_resize().

   @param w The new width.
   @param h The new height.

   Note that the arguments are optional, without them this function just queries
   the current values.  The arguments can be separate values, or named fields in a
   table.

   @return A table with these fields:
   - integer w: Width.
   - integer w: Height.
 */
static int
_elua_resize(lua_State *L) // Stack usage [-(4|6), +(5|7), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord ow, oh;
   int w, h;

   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_geometry_get(elo->evas_obj, NULL, NULL, &ow, &oh);
   if (_elua_scan_params(L, 2, "%w %h", &w, &h) > 0) // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
     {
        if ((w != ow) || (h != oh))
          {
             evas_object_resize(elo->evas_obj, w, h);
             evas_object_geometry_get(elo->evas_obj, NULL, NULL, &ow, &oh);
          }
     }
   _elua_ret(L, "%w %h", ow, oh);
   // Stack usage [-4, +5, em]
   return 1;
}

/**
   @page luaref
   @subsubsection evas_size evas_object:size()

   An alias for evas_object:resize().
 */
static int
_elua_size(lua_State *L) // Stack usage [-(4|6), +(5|7), em]
{
   return _elua_resize(L);
}

//-------------
/**
   @page luaref
   @subsubsection evas_clip evas_object:clip(evas_object2)

   Get (and optionally set) the object that clips this object.

   Note that the argument is optional, without it this function just queries the
   current value.

   Wraps evas_object_clip_set().

   @param evas_object2 A reference to the object to clip this object with.

   @returns A reference to the object clipping this object, if any.
 */
static int
_elua_clip(lua_State *L) // Stack usage [-3, +4, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1); // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo2, *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Object *o;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (n == 2)
     {
        Edje_Lua_Obj *obj2 = (Edje_Lua_Obj *)lua_touserdata(L, 2);  // Stack usage [-0, +0, -]
        elo2 = (Edje_Lua_Evas_Object *)obj2;
        if (!_elua_isa(obj2, _elua_evas_meta)) return 0;
        evas_object_clip_set(elo->evas_obj, elo2->evas_obj);
     }
   o = evas_object_clip_get(elo->evas_obj);
   if (!o) return 0;
   if (!(elo2 = evas_object_data_get(o, ELO))) return 0;
   _elua_ref_get(L, elo2); // Stack usage [-3, +4, -]
   return 1;
}

/**
   @page luaref
   @subsubsection evas_clipees evas_object:clipees()

   Gets the list of objects this objects clips.

   Wraps evas_object_clipees_get().

   @return A table, that holds all the objects this clips, if any,
         otherwise an empty table.
 */
static int
_elua_clipees(lua_State *L) // Stack usage [-0, +1, me] plus [-5, +5] for each clipee.
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo2, *elo = (Edje_Lua_Evas_Object *)obj;
   Eina_List *list, *l;
   Evas_Object *o;
   int n = 0;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   list = (Eina_List *)evas_object_clipees_get(elo->evas_obj);
   lua_newtable(L); // Stack usage [-0, +1, m]
   EINA_LIST_FOREACH(list, l, o)
     {
        if (!(elo2 = evas_object_data_get(o, ELO))) continue;
        lua_pushinteger(L, n + 1); // Stack usage [-0, +1, -]
        _elua_ref_get(L, elo2); // Stack usage [-3, +4, -]
        lua_settable(L, -3); // Stack usage [-2, +0, e]
        n++;
     }
   return 1;
}

/**
   @page luaref
   @subsubsection evas_unclip evas_object:unclip()

   Remove any clipping on this object.

   Wraps evas_object_clip_unset().
 */
static int
_elua_unclip(lua_State *L) // Stack usage [-0, +0, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_clip_unset(elo->evas_obj);
   return 0;
}

//-------------
/**
   @page luaref
   @subsubsection evas_type evas_object:type()

   Get the type of this object.  See the documentation of the evas_object_type_get()
   C function for details.

   Wraps evas_object_type_get().

   @return A string with this objects type in it.
 */
static int
_elua_type(lua_State *L) // Stack usage [-0, +1, m]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   const char *t;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   t = evas_object_type_get(elo->evas_obj);
   if (!t) return 0;
   lua_pushstring(L, t); // Stack usage [-0, +1, m]
   return 1;
}

//-------------
/**
   @page luaref
   @subsubsection evas_pass evas_object:pass(pass)

   Get (and optionally set) whether this object ignores events, passing them to the
   next object underneath it.

   Wraps evas_object_pass_events_set().

   @param pass A boolean saying if this object passes events.

   Note that the argument is optional, without it this function just queries the
   current value.

   @return A boolean saying if this object passes events.
 */
static int
_elua_pass(lua_State *L) // Stack usage [-0, +1, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (n == 2)
     {
        if (lua_isboolean(L, 2)) // Stack usage [-0, +0, -]
          {
             evas_object_pass_events_set(elo->evas_obj, lua_toboolean(L, 2));
             // Stack usage [-0, +0, -]
          }
     }
   lua_pushboolean(L, evas_object_pass_events_get(elo->evas_obj));
   // Stack usage [-0, +1, -]
   return 1;
}

/**
   @page luaref
   @subsubsection evas_precise evas_object:precise(precise)

   Get (and optionally set) whether to use precise (usually expensive) point
   collision detection for this object.

   Wraps evas_object_precise_is_inside_set().

   @param precise A boolean saying if this object is precisely detected.

   Note that the argument is optional, without it this function just queries the
   current value.

   @return A boolean saying if this object is precisely detected.
 */
static int
_elua_precise(lua_State *L) // Stack usage [-0, +1, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (n == 2)
     {
        if (lua_isboolean(L, 2)) // Stack usage [-0, +0, -]
          {
             evas_object_precise_is_inside_set(elo->evas_obj, lua_toboolean(L, 2));
             // Stack usage [-0, +0, -]
          }
     }
   lua_pushboolean(L, evas_object_precise_is_inside_get(elo->evas_obj));
   // Stack usage [-0, +1, -]
   return 1;
}

/**
   @page luaref
   @subsubsection evas_repeat evas_object:repeat(repeat)

   Get (and optionally set) whether this object repeats events.

   Wraps evas_object_repeat_events_set().

   @param repeat A boolean saying if this object repeats events to lower objects.

   Note that the argument is optional, without it this function just queries the
   current value.

   @return A boolean saying if this object repeats events.
 */
static int
_elua_repeat(lua_State *L) // Stack usage [-0, +1, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (n == 2)
     {
        if (lua_isboolean(L, 2)) // Stack usage [-0, +0, -]
          {
             evas_object_repeat_events_set(elo->evas_obj, lua_toboolean(L, 2));
             // Stack usage [-0, +0, -]
          }
     }
   lua_pushboolean(L, evas_object_repeat_events_get(elo->evas_obj));
   // Stack usage [-0, +1, -]
   return 1;
}

//-------------
/**
   @page luaref
   @subsubsection evas_colour evas_object:color(r, g, b, a)

   Gets (and optionally sets) this objects colour.

   Wraps evas_object_color_set().

   @param r The new red value.
   @param g The new green value.
   @param b The new blue value.
   @param a The new alpha value.

   Note that the arguments are optional, without them this function just queries
   the current values.  The arguments can be separate values, or named fields in a
   table.

   @return A table with these fields:
   - integer r: The red value.
   - integer g: The green value.
   - integer b: The blue value.
   - integer a: The alpha value.
 */
static int
_elua_color(lua_State *L) // Stack usage [-(8|12), +(9|13), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int r, g, b, a;

   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   if (_elua_scan_params(L, 2, "%r %g %b %a", &r, &g, &b, &a) > 0) // Stack usage [-0, +0, m] unless it's in a table [-4, +4, e]
     {
        _elua_color_fix(&r, &g, &b, &a);
        evas_object_color_set(elo->evas_obj, r, g, b, a);
     }
   evas_object_color_get(elo->evas_obj, &r, &g, &b, &a);
   _elua_ret(L, "%r %g %b %a", r, g, b, a);
   // Stack usage [-8, +9, em]
   return 1;
}

//-------------
/**
   @page luaref
   @subsubsection evas_map evas_object:map(map)

   Attach a map to this object.

   Wraps evas_object_map_set().

   @param map The map to attach.

   @since 1.1.0
 */
static int
_elua_obj_map(lua_State *L) // Stack usage [-0, +0, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Edje_Lua_Obj *obj2 = (Edje_Lua_Obj *)lua_touserdata(L, 2);   // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj2;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   if (!_elua_isa(obj2, _elua_evas_map_meta)) return 0;

   evas_object_map_set(elo->evas_obj, elm->map);

   return 0;
}

/**
   @page luaref
   @subsubsection evas_map_enable evas_object:map_enable(enable)

   Enable or disable the map attached to this object.

   Wraps evas_object_map_enable_set().

   @param enable A booleon that controls if the attached map is enabled or not.

   @return A boolean reflecting the map enabled status of this object.

   @since 1.1.0
 */
static int
_elua_obj_map_enable(lua_State *L) // Stack usage [-0, +1, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;

   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (n == 2)
     {
        evas_object_map_enable_set(elo->evas_obj, lua_toboolean(L, 2));
        // Stack usage [-0, +0, -]
     }
   lua_pushboolean(L, evas_object_map_enable_get(elo->evas_obj));
   // Stack usage [-0, +1, -]
   return 1;
}

//-------------
//-------------
/**
   @page luaref
   @subsection ecore_animator Ecore animator class.

   The lua ecore animator class includes functions for dealing with ecore animator objects.
   The ecore animator objects must have been previously created by lua using the lua
   edje object creation function edje.animator() or edje.transition().

   In the following, "animator_object" is a place holder for any lua variable that
   holds a reference to an ecore animator object.
 */
static const char *_elua_ecore_animator_api = "ecore_animator";
static const struct luaL_Reg _elua_ecore_animator_funcs [] =
{
   {NULL, NULL}   // end
};

//-------------
//-------------
/**
   @page luaref
   @subsection ecore_timer Ecore timer class.

   The lua ecore timer class includes functions for dealing with ecore timer objects.
   The ecore timer objects must have been previously created by lua using the lua
   edje object creation function edje.timer().

   In the following, "timer_object" is a place holder for any lua variable that
   holds a reference to an ecore timer object.
 */

static const char *_elua_ecore_timer_api = "ecore_timer";
static const struct luaL_Reg _elua_ecore_timer_funcs [] =
{
   {NULL, NULL}   // end
};

//-------------
//-------------
/**
   @page luaref
   @subsection evas_edje Evas edje class.

   The lua evas edje class includes functions for dealing with evas edje objects.
   The evas edje objects must have been previously created by lua using the lua
   edje object creation function edje.edje().

   In the following, "edje_object" is a place holder for any lua variable that
   holds a reference to an evas edje object.  NOT the edje class specified earlier
   though.

   @since 1.1.0
 */

static int _elua_edje_file(lua_State *L);

static const char *_elua_evas_edje_api = "evas_edje";
static const char *_elua_evas_edje_parent = "evas_edje_parent";
static const struct luaL_Reg _elua_evas_edje_funcs [] =
{
   {"file", _elua_edje_file},           // get or set edje file and group

   {NULL, NULL}   // end
};

/**
   @page luaref
   @subsubsection edje_file edje_object:file(file, group)

   Load an edje group into this edje object.

   Wraps edje_object_file_set().

   @param file An edje file name (ignored, sandboxed to the file this lua script is in).
   @param group The group within the edje file to be loaded.

   Note that the arguments are optional, without them this function just queries
   the current values.  The arguments can be separate values, or named fields in a
   table.  The file argument is optional, and ignored anyway.

   @return A table with these fields:
   - string file: The name of the edje file this edje's group is loaded from.
   - string group: The name of the group this edje is loaded from.

   @since 1.1.0
 */
static int
_elua_edje_file(lua_State *L) // Stack usage [-(4|6), +(5|7), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   const char *file = NULL, *group = NULL;
   int n = lua_gettop(L); // Stack usage [-0, +0, -]

   if (!_elua_isa(obj, _elua_evas_edje_meta)) return 0;

   n = _elua_scan_params(L, 2, "$file $group", &file, &group);
   // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
   if (0 >= n)
     {
        file = (char *)obj->ed->file->path;
        group = (char *)lua_tostring(L, 2); // Stack usage [-0, +0, m]
        n = 2;
     }

   if (1 < n)
     {
        // Sandbox lua - Only allow access to groups within the same file.
        // By the simple expedient of completely ignoring what file was requested.
        file = (char *)obj->ed->file->path;
        if (!edje_object_file_set(elo->evas_obj, file, group))
          {
             Edje_Load_Error err = edje_object_load_error_get(elo->evas_obj);

             switch (err)
               {
                case EDJE_LOAD_ERROR_NONE:                         LE("Edje file loading errer %s %s - no error happened, but you should not see this.", obj->ed->file->path, group);  break;

                case EDJE_LOAD_ERROR_GENERIC:                      LE("Edje file loading errer %s %s - generic error.", obj->ed->file->path, group);  break;

                case EDJE_LOAD_ERROR_DOES_NOT_EXIST:               LE("Edje file loading errer %s %s - file does not exist.", obj->ed->file->path, group);  break;

                case EDJE_LOAD_ERROR_PERMISSION_DENIED:            LE("Edje file loading errer %s %s - permission denied reading the file.", obj->ed->file->path, group);  break;

                case EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED:   LE("Edje file loading errer %s %s - resource allocation failed.", obj->ed->file->path, group);  break;

                case EDJE_LOAD_ERROR_CORRUPT_FILE:                 LE("Edje file loading errer %s %s - corrupt file.", obj->ed->file->path, group);  break;

                case EDJE_LOAD_ERROR_UNKNOWN_FORMAT:               LE("Edje file loading errer %s %s - unknown file format.", obj->ed->file->path, group);  break;

                case EDJE_LOAD_ERROR_INCOMPATIBLE_FILE:            LE("Edje file loading errer %s %s - incompatible file.", obj->ed->file->path, group);  break;

                case EDJE_LOAD_ERROR_UNKNOWN_COLLECTION:           LE("Edje file loading errer %s %s - unknown group.", obj->ed->file->path, group);  break;

                case EDJE_LOAD_ERROR_RECURSIVE_REFERENCE:          LE("Edje file loading errer %s %s - recursive reference in group.", obj->ed->file->path, group);  break;
               }
          }
     }
   edje_object_file_get(elo->evas_obj, &file, &group);
   _elua_ret(L, "$file $group", file, group);
   // Stack usage [-4, +5, em]
   return 1;
}

//-------------
//-------------
/**
   @page luaref
   @subsection evas_image Evas image class.

   The lua evas image class includes functions for dealing with evas image objects.
   The evas image objects must have been previously created by lua using the lua
   image object creation function edje.image().

   In the following, "image_object" is a place holder for any lua variable that
   holds a reference to an evas image object.

   @since 1.1.0
 */

static int _elua_image_fill(lua_State *L);
static int _elua_image_filled(lua_State *L);
static int _elua_image_image(lua_State *L);

static const char *_elua_evas_image_api = "evas_image";
static const char *_elua_evas_image_parent = "evas_image_parent";
static const struct luaL_Reg _elua_evas_image_funcs [] =
{
   {"fill", _elua_image_fill},             // get or set the fill parameters
   {"filled", _elua_image_filled},         // get or set the filled state (overrides fill())
   {"image", _elua_image_image},           // get or set image

   {NULL, NULL}   // end
};

/**
   @page luaref
   @subsubsection image_fill image_object:fill(x, y, w, h)

   Gets (and optionally sets) how to fill this image's drawing rectangle given the
   (real) image bound to it.

   Wraps evas_object_image_fill_set().

   @param x The x coordinate (from the top left corner of the bound image) to start drawing from.
   @param y The y coordinate (from the top left corner of the bound image) to start drawing from.
   @param w The width the bound image will be displayed at.
   @param h The height the bound image will be displayed at.

   Note that the arguments are optional, without them this function just queries
   the current values.  The arguments can be separate values, or named fields in a
   table.

   @return A table with these fields:
   - integer x: The x coordinate (from the top left corner of the bound image) to start drawing from.
   - integer y: The y coordinate (from the top left corner of the bound image) to start drawing from.
   - integer w: The width the bound image will be displayed at.
   - integer h: The height the bound image will be displayed at.

   @since 1.1.0
 */
static int
_elua_image_fill(lua_State *L) // Stack usage [-(8|12), +(9|13), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord x, y, w, h;

   if (!_elua_isa(obj, _elua_evas_image_meta)) return 0;

   if (_elua_scan_params(L, 2, "%x %y %w %h", &x, &y, &w, &h) > 0) // Stack usage [-0, +0, m] unless it's in a table [-4, +4, e]
     {
        evas_object_image_fill_set(elo->evas_obj, x, y, w, h);
     }
   evas_object_image_fill_get(elo->evas_obj, &x, &y, &w, &h);
   _elua_ret(L, "%x %y %w %h", x, y, w, h);
   // Stack usage [-8, +9, em]
   return 1;
}

/**
   @page luaref
   @subsubsection image_filled image_object:filled(filled)

   Get (and optionally set) whether this image fills the object.

   Wraps evas_object_image_filled_set().

   @param filled A boolean saying if this image fills the object.

   Note that the argument is optional, without it this function just queries the
   current value.

   @return A boolean saying if this image fills the object.

   @since 1.1.0
 */
static int
_elua_image_filled(lua_State *L) // Stack usage [-0, +0, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;

   if (!_elua_isa(obj, _elua_evas_image_meta)) return 0;

   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (n == 2)
     {
        evas_object_image_filled_set(elo->evas_obj, lua_toboolean(L, 2));
        // Stack usage [-0, +0, -]
     }
   lua_pushboolean(L, evas_object_image_filled_get(elo->evas_obj));
   // Stack usage [-0, +0, -]
   return 1;
}

/**
   @page luaref
   @subsubsection image_image image_object:image(file, key)

   Load an image into this edje object.

   Wraps evas_object_image_file_set().

   @param file An edje file name (ignored, sandboxed to the file this lua script is in).
   @param group The name of an image.

   Note that the arguments are optional, without them this function just queries
   the current values.  The arguments can be separate values, or named fields in a
   table.  The file argument is optional, and ignored anyway.

   @return A table with these fields:
   - string file: The name of the edje file the image is loaded from.
   - string key: The name of the image within the edje file.

   @since 1.1.0
 */
static int
_elua_image_image(lua_State *L) // Stack usage [-(4|6), +(5|7), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   const char *file = NULL, *key = NULL;
   int n, id = -1;

   if (!_elua_isa(obj, _elua_evas_image_meta)) return 0;

   n = _elua_scan_params(L, 2, "$file $key", &file, &key);
   // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
   if (0 >= n)
     {
        file = (char *)obj->ed->file->path;
        key = (char *)lua_tostring(L, 2); // Stack usage [-0, +0, m]
        n = 2;
     }

   if (1 < n)
     {
        if (obj->ed->file->image_dir)
          {
             Edje_Image_Directory_Entry *de;
             unsigned int i;
             char *name;

             /* Image name */
             if ((name = strrchr(key, '/'))) name++;
             else name = (char *)key;

             /* Loop through image directory to find if image exists */
             for (i = 0; i < obj->ed->file->image_dir->entries_count; ++i)
               {
                  de = obj->ed->file->image_dir->entries + i;

                  if (de->entry)
                    {
                       if (strcmp(name, de->entry) == 0)
                         {
                            char buf[32];

                            id = i;
                            // This is copied from _edje_image_recalc_apply()), dunno if it provides any benefit over sprintf().
                            /* Replace snprint("edje/images/%i") == memcpy + itoa */
#define IMAGES "edje/images/"
                            memcpy(buf, IMAGES, strlen(IMAGES));
                            eina_convert_itoa(id, buf + strlen(IMAGES)); /* No need to check length as 2³² need only 10 characters. */
                            evas_object_image_file_set(elo->evas_obj, obj->ed->file->path, buf);
                            break;
                         }
                    }
               }
          }

        if (-1 == id)
          {
             LE("Image %s not found in our edje file.", key);
             /* Sandbox lua - Only allow access to images within the same edje file.  I'm not so sure we need this level of sandboxing though.  So leaving it here, just in case.
                LI("Image %s not found in our edje file, trying external image file %s.", key, file);
                evas_object_image_file_set(elo->evas_obj, file, key);
              */
          }
     }
   evas_object_image_file_get(elo->evas_obj, &file, &key);
   _elua_ret(L, "$file $key", file, key);
   // Stack usage [-4, +5, em]
   return 1;
}

//-------------
//-------------
/**
   @page luaref
   @subsection evas_line Evas line class.

   The lua evas line class includes functions for dealing with evas line objects.
   The evas line objects must have been previously created by lua using the lua
   line object creation function edje.line().

   In the following, "line_object" is a place holder for any lua variable that
   holds a reference to an evas line object.

   @since 1.1.0
 */

static int _elua_line_xy(lua_State *L);

static const char *_elua_evas_line_api = "evas_line";
static const char *_elua_evas_line_parent = "evas_line_parent";
static const struct luaL_Reg _elua_evas_line_funcs [] =
{
   {"xy", _elua_line_xy},           // get or set line coords

   {NULL, NULL}   // end
};

/**
   @page luaref
   @subsubsection line_xy line_object:xy(x1, y1, x2, y2)

   Sets the end points of this line.

   Wraps evas_object_line_xy_set().

   @param x1 The X coordinate of the first line end.
   @param y1 The Y coordinate of the first line end.
   @param x2 The X coordinate of the other line end.
   @param y2 The Y coordinate of the other line end.

   Note that the arguments are optional, without them this function just queries
   the current values.  The arguments can be separate values, or named fields in a
   table.

   @return A table with these fields:
   - integer x1: The X coordinate of the first line end.
   - integer y1: The Y coordinate of the first line end.
   - integer x2: The X coordinate of the other line end.
   - integer y2: The Y coordinate of the other line end.

   @since 1.1.0
 */
static int
_elua_line_xy(lua_State *L) // Stack usage [-(8|12), +(9|13), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord x1, y1, x2, y2;

   if (!_elua_isa(obj, _elua_evas_line_meta)) return 0;

   if (_elua_scan_params(L, 2, "%x1 %y1 %x2 %y2", &x1, &y1, &x2, &y2) > 0) // Stack usage [-0, +0, m] unless it's in a table [-4, +4, e]
     {
        evas_object_line_xy_set(elo->evas_obj, x1, y1, x2, y2);
     }
   evas_object_line_xy_get(elo->evas_obj, &x1, &y1, &x2, &y2);
   _elua_ret(L, "%x1 %y1 %x2 %y2", x1, y1, x2, y2);
   // Stack usage [-8, +9, em]
   return 1;
}

//-------------
//-------------
/**
   @page luaref
   @subsection evas_object_map Evas map class.

   The lua evas map class includes functions for dealing with evas map objects.
   The evas map objects must have been previously created by lua using the lua
   map object creation function edje.map().  The evas map system is complex, rather
   than repeat the copious documentation here, please refer to the evas map
   documentation.  It has pictures and everything.  B-)

   In the following, "map_object" is a place holder for any lua variable that
   holds a reference to an evas map object.

   @since 1.1.0
 */

static int _elua_map_alpha(lua_State *L);
static int _elua_map_clockwise(lua_State *L);
static int _elua_map_colour(lua_State *L);
static int _elua_map_coord(lua_State *L);
static int _elua_map_lighting(lua_State *L);
static int _elua_map_perspective(lua_State *L);
static int _elua_map_populate(lua_State *L);
static int _elua_map_rotate(lua_State *L);
static int _elua_map_rotate3d(lua_State *L);
static int _elua_map_smooth(lua_State *L);
static int _elua_map_uv(lua_State *L);
static int _elua_map_zoom(lua_State *L);

static const char *_elua_evas_map_api = "evas_map";
static const struct luaL_Reg _elua_evas_map_funcs [] =
{
   {"alpha", _elua_map_alpha},
//     {"dup",           _elua_map_dup},  // not sure of proper api for this.
   {"clockwise", _elua_map_clockwise},
   {"color", _elua_map_colour},
   {"coord", _elua_map_coord},
   {"lighting", _elua_map_lighting},
   {"perspective", _elua_map_perspective},
   {"populate", _elua_map_populate},
   {"rotate", _elua_map_rotate},
   {"rotate3d", _elua_map_rotate3d},
//     {"size",          _elua_map_size},  // not sure of proper API for this
   {"smooth", _elua_map_smooth},
   {"uv", _elua_map_uv},
   {"zoom", _elua_map_zoom},

   {NULL, NULL}   // end
};

/**
   @page luaref
   @subsubsection map_alpha map_object:alpha(alpha)

   Get (and optionally set) the maps alpha mode.

   Wraps evas_map_alpha_set().

   @param alpha The alpha mode.

   Note that the argument is optional, without it this function just queries the
   current value.

   @return A boolean reflecting the alpha mode.

   @since 1.1.0
 */
static int
_elua_map_alpha(lua_State *L) // Stack usage [-0, +1, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (n == 2)
     {
        evas_map_alpha_set(elm->map, lua_toboolean(L, 2));
        // Stack usage [-0, +0, -]
     }
   lua_pushboolean(L, evas_map_alpha_get(elm->map)); // Stack usage [-0, +1, -]
   return 1;
}

/**
   @page luaref
   @subsubsection map_clockwise map_object:clockwise()

   Get the maps clockwise state.

   Wraps evas_map_util_clockwise_get().

   @return A boolean reflecting if the map is clockwise or not.

   @since 1.1.0
 */
static int
_elua_map_clockwise(lua_State *L) // Stack usage [-0, +1, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   lua_pushboolean(L, evas_map_util_clockwise_get(elm->map));   // Stack usage [-0, +1, -]
   return 1;
}

/**
   @page luaref
   @subsubsection map_colour map_object:colour(index, r, g, b, a)

   Gets or sets colour information for the map.  There are two variations, with or
   without the index.  With the index parameter it gets (and optionally sets) the
   colour of the point the index refers to, without it sets the colour for the
   entire map.

   Wraps evas_map_point_color_set() or evas_map_util_points_color_set()

   @param index Which point to change the colour of.
   @param r The new red value.
   @param g The new green value.
   @param b The new blue value.
   @param a The new alpha value.

   Note that the arguments are optional, without them this function just queries
   the current values.  The colour arguments can be separate values, or named
   fields in a table.

   @return A table with these fields:
   - integer r: The red value.
   - integer g: The green value.
   - integer b: The blue value.
   - integer a: The alpha value.

   @since 1.1.0
 */
static int
_elua_map_colour(lua_State *L) // Stack usage [-(8|12), +(9|13), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   int r, g, b, a;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]

   switch (n)
     {
      case 5:
      {
         if (_elua_scan_params(L, 2, "%r %g %b %a", &r, &g, &b, &a) > 0) // Stack usage [-0, +0, m] unless it's in a table [-4, +4, e]
           {
              evas_map_util_points_color_set(elm->map, r, g, b, a);
           }
         break;
      }

      case 1:
      case 6:
      {
         if (_elua_scan_params(L, 3, "%r %g %b %a", &r, &g, &b, &a) > 0) // Stack usage [-0, +0, m] unless it's in a table [-4, +4, e]
           {
              evas_map_point_color_set(elm->map, lua_tointeger(L, 2), r, g, b, a);
              // Stack usage [-0, +0, -]
           }
         evas_map_point_color_get(elm->map, lua_tointeger(L, 2), &r, &g, &b, &a);
         // Stack usage [-0, +0, -]
         _elua_ret(L, "%r %g %b %a", r, g, b, a);
         // Stack usage [-8, +9, em]
         return 1;
      }
     }

   return 0;
}

/**
   @page luaref
   @subsubsection map_coord map_object:coord(index, x, y, z)

   Gets (and optionally sets) the 3D coordinates of a point on the map.

   Wraps evas_map_point_coord_set().

   @param x The x coordinate of the point.
   @param y The y coordinate of the point.
   @param z The z coordinate of the point.

   Note that the arguments are optional, without them this function just queries
   the current values.  The coordinate arguments can be separate values, or named
   fields in a table.

   @return A table with these fields:
   - integer x: The x coordinate of the point.
   - integer y: The y coordinate of the point.
   - integer z: The z coordinate of the point.

   @since 1.1.0
 */
static int
_elua_map_coord(lua_State *L) // Stack usage [-(6|9), +(7|10), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   Evas_Coord x, y, z;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (2 > n) return 0;

   if (_elua_scan_params(L, 2, "%x %y %z", &x, &y, &z) > 0)     // Stack usage [-0, +0, m] unless it's in a table [-3, +3, e]
     {
        evas_map_point_coord_set(elm->map, lua_tointeger(L, 2), x, y, z);
        // Stack usage [-0, +0, -]
     }
   evas_map_point_coord_get(elm->map, lua_tointeger(L, 2), &x, &y, &z);
   // Stack usage [-0, +0, -]
   _elua_ret(L, "%x %y %z", x, y, z);
   // Stack usage [-6, +7, em]
   return 1;
}

/**
   @page luaref
   @subsubsection map_lighting map_object:lighting(x, y, z, r, g, b, ar, ag, ab)

   Set the 3D lights for the map.  The three triplets can be tables.

   Wraps evas_map_util_3d_lighting().

   @param x The x coordinate of the light point.
   @param y The y coordinate of the light point.
   @param z The z coordinate of the light point.
   @param r The new red value of the light point.
   @param g The new green value of the light point.
   @param b The new blue value of the light point.
   @param ar The new red value of the ambient light.
   @param ag The new green value of the ambient light.
   @param ab The new blue value of the ambient light.

   @since 1.1.0
 */
static int
_elua_map_lighting(lua_State *L) // Stack usage [-(0|9), +(0|9), e]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   Evas_Coord x, y, z;
   int r, g, b, r1, g1, b1;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   if ((n = _elua_scan_params(L, 2, "%x %y %z", &x, &y, &z)) > 0)
     // Stack usage [-0, +0, m] unless it's in a table [-3, +3, e]
     if (n += _elua_scan_params(L, 2 + n, "%r %g %b", &r, &g, &b) > 0)
       // Stack usage [-0, +0, m] unless it's in a table [-3, +3, e]
       if (_elua_scan_params(L, 2 + n, "%r %g %b", &r1, &g1, &b1) > 0) // Stack usage [-0, +0, m] unless it's in a table [-3, +3, e]
         {
            evas_map_util_3d_lighting(elm->map, x, y, z, r, g, b, r1, g1, b1);
         }
   return 0;
}

/**
   @page luaref
   @subsubsection map_perspective map_object:perspective(x, y, z, f)

   Apply a perspective transform to the map.

   Wraps evas_map_util_3d_perspective().

   The arguments can be separate values, or named fields in a table.

   @param x The perspective distance X coordinate
   @param y The perspective distance Y coordinate
   @param z The "0" z plane value
   @param f The focal distance

   @since 1.1.0
 */
static int
_elua_map_perspective(lua_State *L) // Stack usage [-(0|4), +(0|4), e]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   Evas_Coord x, y, z, f;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   if (_elua_scan_params(L, 2, "%x %y %z %f", &x, &y, &z, &f) > 0) // Stack usage [-0, +0, m] unless it's in a table [-4, +4, e]
     {
        evas_map_util_3d_perspective(elm->map, x, y, z, f);
     }
   return 0;
}

/**
   @page luaref
   @subsubsection map_populate map_object:populate(...)

   Populate the points in a map, in one of three different methods.

   1) Wraps evas_map_util_points_populate_from_object().

   @param source An evas object to copy points from.

   2) Wraps evas_map_util_paints_populate_from_object_full().

   @param source An evas object to copy points from.
   @param z Common Z coordinate hint for all four points.

   3) Wraps evas_map_util_points_populate_from_geometry().

   The first four arguments can be separate values, or named fields in a table.

   @param x Point X coordinate
   @param y Point Y coordinate
   @param w Width to use to calculate second and third points.
   @param h Height to use to calculate third and fourth points.
   @param z Common Z coordinate hint for all four points.

   @since 1.1.0
 */
static int
_elua_map_populate(lua_State *L) // Stack usage [-(0|4), +(0|4), e]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]

   switch (n)
     {
      case 2:
      {
         Edje_Lua_Obj *obj2 = (Edje_Lua_Obj *)lua_touserdata(L, 2);      // Stack usage [-0, +0, -]
         const Edje_Lua_Evas_Object *source = (Edje_Lua_Evas_Object *)obj2;

         if (!_elua_isa(obj2, _elua_evas_meta)) return 0;
         evas_map_util_points_populate_from_object(elm->map, source->evas_obj);
         break;
      }

      case 3:
      {
         Edje_Lua_Obj *obj2 = (Edje_Lua_Obj *)lua_touserdata(L, 2);      // Stack usage [-0, +0, -]
         const Edje_Lua_Evas_Object *source = (Edje_Lua_Evas_Object *)obj2;
         Evas_Coord z = lua_tointeger(L, 3);

         if (!_elua_isa(obj2, _elua_evas_meta)) return 0;
         evas_map_util_points_populate_from_object_full(elm->map, source->evas_obj, z);
         break;
      }

      case 6:
      {
         Evas_Coord x, y, w, h;

         if ((n = _elua_scan_params(L, 2, "%x %y %w %h", &x, &y, &w, &h)) > 0) // Stack usage [-0, +0, m] unless it's in a table [-4, +4, e]
           {
              evas_map_util_points_populate_from_geometry(elm->map, x, y, w, h, lua_tointeger(L, 2 + n));
           }
         break;
      }
     }
   return 0;
}

/**
   @page luaref
   @subsubsection map_rotate map_object:rotate(degrees, x, y)

   Rotate the maps coordinates in 2D.

   Wraps evas_map_util_rotate().

   The coordinates can be separate values, or named fields in a table.

   @param degrees Amount of degrees from 0.0 to 360.0 to rotate.
   @param x Rotation's centre horizontal position.
   @param y Rotation's centre vertical position.

   @since 1.1.0
 */
static int
_elua_map_rotate(lua_State *L) // Stack usage [-(0|2), +(0|2), e]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   double degrees;
   Evas_Coord x, y;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (4 != n) return 0;

   degrees = lua_tonumber(L, 2);
   if (_elua_scan_params(L, 3, "%x %y", &x, &y) > 0) // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
     {
        evas_map_util_rotate(elm->map, degrees, x, y);
     }
   return 0;
}

/**
   @page luaref
   @subsubsection map_rotate3d map_object:rotate3d(dx, dy, dz, x, y, z)

   Rotate the maps coordinates in 3D.

   Wraps evas_map_util_3d_rotate().

   The coordinates can be separate values, or named fields in a table.  The same
   with the rotation.

   @param dx Amount of degrees from 0.0 to 360.0 to rotate around X axis.
   @param dy Amount of degrees from 0.0 to 360.0 to rotate around Y axis.
   @param dz Amount of degrees from 0.0 to 360.0 to rotate around Z axis.
   @param x Rotation's centre horizontal position.
   @param y Rotation's centre vertical position.
   @param z Rotation's centre vertical position.

   @since 1.1.0
 */
static int
_elua_map_rotate3d(lua_State *L) // Stack usage [-(0|6), +(0|6), e]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   double zx, zy, zz;
   Evas_Coord x, y, z;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   if ((n = _elua_scan_params(L, 2, "#x #y #z", &zx, &zy, &zz)) > 0)
     // Stack usage [-0, +0, m] unless it's in a table [-3, +3, e]
     if (_elua_scan_params(L, 2 + n, "%x %y %z", &x, &y, &z) > 0) // Stack usage [-0, +0, m] unless it's in a table [-3, +3, e]
       {
          evas_map_util_3d_rotate(elm->map, zx, zy, zz, x, y, z);
       }
   return 0;
}

/**
   @page luaref
   @subsubsection map_smooth map_object:smooth(smooth)

   Get (and optionally set) the maps smooth mode.

   Wraps evas_map_smooth_set().

   @param smooth The smooth mode.

   Note that the argument is optional, without it this function just queries the
   current value.

   @return A boolean reflecting the smooth mode.

   @since 1.1.0
 */
static int
_elua_map_smooth(lua_State *L) // Stack usage [-0, +1, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (n == 2)
     {
        evas_map_smooth_set(elm->map, lua_toboolean(L, 2));
        // Stack usage [-0, +0, -]
     }
   lua_pushboolean(L, evas_map_smooth_get(elm->map)); // Stack usage [-0, +1, -]
   return 1;
}

/**
   @page luaref
   @subsubsection map_uv map_object:uv(index, u, v)

   Gets (and optionally sets) the texture U and V texture coordinates for this map.

   Wraps evas_map_point_image_uv_set().

   @param index Index of the point to change. Must be smaller than map size.
   @param u The X coordinate within the image/texture source.
   @param v The Y coordinate within the image/texture source.

   Note that the U,V arguments are optional, without them this function just queries
   the current values.  The coordinate arguments can be separate values, or named
   fields in a table.

   @return A table with these fields:
   - number u: The X coordinate within the image/texture source.
   - number v: The Y coordinate within the image/texture source.

   @since 1.1.0
 */
static int
_elua_map_uv(lua_State *L) // Stack usage [-(4|6), +(5|7), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   double u, v;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (2 > n) return 0;

   if (_elua_scan_params(L, 3, "#u #v", &u, &v) > 0) // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
     {
        evas_map_point_image_uv_set(elm->map, lua_tonumber(L, 2), u, v);
        // Stack usage [-0, +0, -]
     }
   evas_map_point_image_uv_get(elm->map, lua_tonumber(L, 2), &u, &v);
   // Stack usage [-0, +0, -]
   _elua_ret(L, "#u #v", u, v);
   // Stack usage [-4, +5, em]
   return 1;
}

/**
   @page luaref
   @subsubsection map_zoom map_object:zoom(x, y, x, y)

   Apply a zoom to the map.

   Wraps evas_map_util_zoom().

   The arguments can be two separate values, or named fields in a table.

   @param x The horizontal zoom amount.
   @param y The vertical zoom amount.
   @param x The X coordinate of the centre of the zoom.
   @param y The Y coordinate of the centre of the zoom.

   @since 1.1.0
 */
static int
_elua_map_zoom(lua_State *L) // Stack usage [-(0|4), +(0|4), e]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   double zx, zy;
   Evas_Coord x, y;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   if ((n = _elua_scan_params(L, 2, "#x #y", &zx, &zy)) > 0)
     // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
     if (_elua_scan_params(L, 2 + n, "%x %y", &x, &y) > 0)      // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
       {
          evas_map_util_zoom(elm->map, zx, zy, x, y);
       }
   return 0;
}

//-------------
//-------------
/**
   @page luaref
   @subsection evas_polygon Evas polygon class.

   The lua evas polygon class includes functions for dealing with evas polygon objects.
   The evas polygon objects must have been previously created by lua using the lua
   polygon object creation function edje.polygon().

   In the following, "polygon_object" is a place holder for any lua variable that
   holds a reference to an evas polygon object.

   @since 1.1.0
 */

static int _elua_polygon_clear(lua_State *L);
static int _elua_polygon_point(lua_State *L);

static const char *_elua_evas_polygon_api = "evas_polygon";
static const char *_elua_evas_polygon_parent = "evas_polygon_parent";
static const struct luaL_Reg _elua_evas_polygon_funcs [] =
{
   {"clear", _elua_polygon_clear},           // clear all polygon points
   {"point", _elua_polygon_point},           // add a polygon point

   {NULL, NULL}   // end
};

/**
   @page luaref
   @subsubsection polygon_clear polygon_object:clear()

   Clears all points from the polygon.

   Wraps evas_object_polygon_points_clear(),

   @since 1.1.0
 */
static int
_elua_polygon_clear(lua_State *L) // Stack usage [-0, +0, -]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;

   if (!_elua_isa(obj, _elua_evas_polygon_meta)) return 0;
   evas_object_polygon_points_clear(elo->evas_obj);
   return 0;
}

/**
   @page luaref
   @subsubsection polygon_point polygon_object:point(x, y)

   Adds a point to this polygon.

   Wraps evas_object_polygon_point_add().

   @param x The X coordinate of the point.
   @param y The Y coordinate of the point.

   @since 1.1.0
 */
static int
_elua_polygon_point(lua_State *L) // Stack usage [-(0|2), +(0|2), e]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord x, y;

   if (!_elua_isa(obj, _elua_evas_polygon_meta)) return 0;

   if (_elua_scan_params(L, 2, "%x %y", &x, &y) > 0) // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
     {
        evas_object_polygon_point_add(elo->evas_obj, x, y);
     }

   return 0;
}

//-------------
//-------------
/**
   @page luaref
   @subsection evas_text Evas text class.

   The lua evas text class includes functions for dealing with evas text objects.
   The evas text objects must have been previously created by lua using the lua
   text object creation function edje.text().

   In the following, "text_object" is a place holder for any lua variable that
   holds a reference to an evas text object.

   @since 1.1.0
 */

static int _elua_text_font(lua_State *L);
static int _elua_text_text(lua_State *L);

static const char *_elua_evas_text_api = "evas_text";
static const char *_elua_evas_text_parent = "evas_text_parent";
static const struct luaL_Reg _elua_evas_text_funcs [] =
{
   {"font", _elua_text_font},           // get or set text font
   {"text", _elua_text_text},           // get or set text
//     {"text_class", _elua_object_text_class}, // get or set object text class

   {NULL, NULL}   // end
};

/**
   @page luaref
   @subsubsection text_font text_object:font(font, size)

   Gets, (and optionally sets) the font for this text object.

   Wraps evas_object_text_font_set().

   @param font The new font name.
   @param size The new font size.

   Note that the font and size arguments are optional, without them this function
   just queries the current values.  The font and size arguments can be separate
   values, or named fields in a table.  The font name can refer to a font in the
   edje file, or an external font.

   @return A table with these fields:
   - string font: The font name.
   - integer size: The font size.

   @since 1.1.0
 */
static int
_elua_text_font(lua_State *L) // Stack usage [-(4|6), +(5|7), em]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   char *font, *font2 = NULL;
   Evas_Font_Size size;
   int inlined_font = 0;

   if (!_elua_isa(obj, _elua_evas_text_meta)) return 0;

   if (_elua_scan_params(L, 2, "$font %size", &font, &size) > 0) // Stack usage [-0, +0, m] unless it's in a table [-2, +2, e]
     {
        /* Check if the font is embedded in the .edj
         * This is a simple check.
         * There is a much more complicated version in edje_text.c _edje_text_recalc_apply().
         * If we need to get more complicated, we can do that later,
         * and maybe refactor things.
         */
        if (obj->ed->file->fonts)
          {
             Edje_Font_Directory_Entry *fnt = eina_hash_find(obj->ed->file->fonts, font);

             if (fnt)
               {
                  size_t len = strlen(font) + sizeof("edje/fonts/") + 1;
                  font2 = alloca(len);
                  sprintf(font2, "edje/fonts/%s", font);
                  font = font2;
                  inlined_font = 1;
                  font2 = NULL;
               }
          }

        if (inlined_font) evas_object_text_font_source_set(elo->evas_obj, obj->ed->path);
        else evas_object_text_font_source_set(elo->evas_obj, NULL);

        evas_object_text_font_set(elo->evas_obj, font, size);
     }

   // When one external API says it's gotta be const, and another one says not, then one of them's gotta be cast.  :-P
   evas_object_text_font_get(elo->evas_obj, (const char **)&font, &size);
   _elua_ret(L, "$font %size", font, size);
   // Stack usage [-4, +5, em]
   return 1;
}

/**
   @page luaref
   @subsubsection text_text text_object:text(text)

   Get (and optionally set) the actual text for this text object.

   Wraps evas_object_text_text_set().

   @param text The text to set for this text object.

   Note that the argument is optional, without it this function just queries the
   current value.

   @return A string of the text on this text object.

   @since 1.1.0
 */
static int
_elua_text_text(lua_State *L) // Stack usage [-0, +1, m]
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);    // Stack usage [-0, +0, -]
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;

   if (!_elua_isa(obj, _elua_evas_text_meta)) return 0;
   n = lua_gettop(L); // Stack usage [-0, +0, -]
   if (n == 2)
     {
        if (lua_isstring(L, 2))
          {
             const char *str;

             if ((str = lua_tostring(L, 2))) // Extra parenthesis, coz Mikes compiler has a lisp.
               // Stack usage [-0, +0, m]
               evas_object_text_text_set(elo->evas_obj, str);
          }
     }
   lua_pushstring(L, evas_object_text_text_get(elo->evas_obj)); // Stack usage [-0, +1, m]
   return 1;
}

//--------------------------------------------------------------------------//

// A metatable and functions so that calling non existant API does not crash Lua scripts.

static int _elua_bogan_nilfunc(lua_State *L);
static int _elua_bogan_index(lua_State *L);

static const struct luaL_Reg _elua_bogan_funcs [] =
{
   {"nilfunc", _elua_bogan_nilfunc},           // Just return a nil.
   {"__index", _elua_bogan_index},             // Return the above func.

   {NULL, NULL}   // end
};

static int
_elua_bogan_nilfunc(lua_State *L)
{
   lua_getglobal(L, "nil");
   return 1;
}

static int
_elua_bogan_index(lua_State *L)
{
   const char *key;

   key = lua_tostring(L, 2);
   LE("%s does not exist!", key);
   lua_pushcfunction(L, _elua_bogan_nilfunc);
   return 1;
}

static void
_elua_bogan_protect(lua_State *L) // Stack usage [-3, +3, m]
{
   lua_pushnil(L); // Stack usage [-0, +1, -]
   luaL_newmetatable(L, "bogan"); // Stack usage [-0, +1, m]
#if LUA_VERSION_NUM >= 502
   luaL_setfuncs(L, _elua_bogan_funcs, 0); // Stack usage [-0, +0, e]
#else
   luaL_register(L, 0, _elua_bogan_funcs); // Stack usage [-1, +1, m]
#endif
   lua_setmetatable(L, -2); // Stack usage [-1, +0, -]
   lua_pop(L, 1); // Stack usage [-1, +0, -]
}

//--------------------------------------------------------------------------//

// TODO - All the register / setfuncs and rlelated stuff around here should be reviewed.  Works fine for 5.1, probably works fine for 5.2, but maybe there's a better way?  It may also need to change if we start using LuaJIT.

// Brain dead inheritance thingy, built for speed.  Kinda.  Part 1.
static void
_elua_add_functions(lua_State *L, const char *api, const luaL_Reg *funcs, const char *meta, const char *parent, const char *base)  // Stack usage [-3, +5, m]  if inheriting [-6, +11, em]
{
   // Create an api table, fill it full of the methods.
#if LUA_VERSION_NUM >= 502
   lua_newtable(L); // Stack usage [-0, +1, e]
   lua_pushvalue(L, -1); // Stack usage [-0, +1, -]
   lua_setglobal(L, api); // Stack usage [-1, +0, e]
   luaL_setfuncs(L, funcs, 0); // Stack usage [-0, +0, e]
#else
   luaL_register(L, api, funcs); // Stack usage [-0, +1, m]
#endif
   // Set the api metatable to the bogan metatable.
   luaL_getmetatable(L, "bogan"); // Stack usage [-0, +1, -]
   lua_setmetatable(L, -2); // Stack usage [-1, +0, -]
   // Creat a meta metatable.
   luaL_newmetatable(L, meta); // Stack usage [-0, +1, m]
   // Put the gc functions in the metatable.
#if LUA_VERSION_NUM >= 502
   luaL_setfuncs(L, _elua_edje_gc_funcs, 0);  // Stack usage [-0, +0, e]
#else
   luaL_register(L, 0, _elua_edje_gc_funcs);  // Stack usage [-1, +1, m]
#endif
   // Create an __index entry in the metatable, make it point to the api table.
   lua_pushliteral(L, "__index"); // Stack usage [-0, +1, m]
   lua_pushvalue(L, -3); // Stack usage [-0, +1, -]
   lua_rawset(L, -3); // Stack usage [-2, +0, m]
   // Later this metatable is used as the metatable for newly created objects of this class.

   if (base && parent)
     {
        // Inherit from base
        lua_getglobal(L, base); // Stack usage [-0, +1, e]
        // Create a new parent metatable.
        luaL_newmetatable(L, parent); // Stack usage [-0, +1, m]
        // Create an __index entry in the metatable, make it point to the base table.
        lua_pushliteral(L, "__index"); // Stack usage [-0, +1, m]
        lua_pushvalue(L, -3); // Stack usage [-0, +1, -]
        lua_rawset(L, -3); // Stack usage [-2, +0, m]
        // Set the metatable for the api table to the parent metatable.
        lua_getglobal(L, api); // Stack usage [-0, +1, e]
        luaL_getmetatable(L, parent); // Stack usage [-0, +1, -]
        lua_setmetatable(L, -2); // Stack usage [-1, +0, -]
     }
}

// Brain dead inheritance thingy, built for speed.  Kinda.  Part 2.
static Eina_Bool
_elua_isa(Edje_Lua_Obj *obj, const char *type)
{
   Eina_Bool isa = EINA_FALSE;

   if (!obj) return isa;
   if (obj->meta == type)
     isa = EINA_TRUE;
   if (_elua_evas_meta == type)
     {
        if (obj->meta == _elua_evas_image_meta)
          isa = EINA_TRUE;
        else if (obj->meta == _elua_evas_text_meta)
          isa = EINA_TRUE;
        else if (obj->meta == _elua_evas_edje_meta)
          isa = EINA_TRUE;
        else if (obj->meta == _elua_evas_line_meta)
          isa = EINA_TRUE;
        else if (obj->meta == _elua_evas_polygon_meta)
          isa = EINA_TRUE;
     }
   return isa;
}

#ifndef RASTER_FORGOT_WHY
static void
_elua_init(void) // Stack usage [-16, +20, em]
{
   static Edje_Lua_Alloc ela = { MAX_LUA_MEM, 0 };
   Edje_Lua_Allocator *al;
   const luaL_Reg *l;
   lua_State *L;

   if (lstate) return;

   lstate = L = luaL_newstate();
   al = lua_newuserdata(L, sizeof(Edje_Lua_Allocator));
   al->ref = luaL_ref(L, LUA_REGISTRYINDEX);
   al->func = lua_getallocf(L, &(al->ud));
   al->ela = &ela;
   lua_setallocf(L, _elua_alloc, al); // Stack usage [-0, +0, -]
   lua_atpanic(L, _elua_custom_panic); // Stack usage [-0, +0, -]

// FIXME: figure out optimal gc settings later
//   lua_gc(L, LUA_GCSETPAUSE, 200);                                                       // Stack usage [-0, +0, e]
//   lua_gc(L, LUA_GCSETSTEPMUL, 200);                                                     // Stack usage [-0, +0, e]

   for (l = _elua_libs; l->func; l++) // Currently * 4
     {
#if LUA_VERSION_NUM >= 502
        luaL_requiref(L, l->name, l->func, 1); // Stack usage [-0, +1, e]
#else
        lua_pushcfunction(L, l->func); // Stack usage [-0, +1, m]
        lua_pushstring(L, l->name); // Stack usage [-0, +1, m]
        lua_call(L, 1, 0); // Stack usage [-2, +0, e]
#endif
     }

#if LUA_VERSION_NUM >= 502
   lua_newtable(L); // Stack usage [-0, +1, e]
   lua_pushvalue(L, -1); // Stack usage [-0, +1, -]
   lua_setglobal(L, _elua_edje_api); // Stack usage [-1, +0, e]
   luaL_setfuncs(L, _elua_edje_funcs, 0); // Stack usage [-0, +0, e]
#else
   luaL_register(L, _elua_edje_api, _elua_edje_funcs); // Stack usage [-0, +1, m]
#endif
   luaL_newmetatable(L, _elua_edje_meta); // Stack usage [-0, +1, m]
#if LUA_VERSION_NUM >= 502
   luaL_setfuncs(L, _elua_edje_gc_funcs, 0); // Stack usage [-0, +0, e]
#else
   luaL_register(L, 0, _elua_edje_gc_funcs); // Stack usage [-1, +1, m]
#endif

   _elua_add_functions(L, _elua_evas_api, _elua_evas_funcs, _elua_evas_meta, NULL, NULL);  // Stack usage [-3, +5, m]

   // weak table for our objects
   lua_pushlightuserdata(L, &_elua_objs); // Stack usage [-0, +1, -]
   lua_newtable(L); // Stack usage [-0, +1, m]
   lua_pushstring(L, "__mode"); // Stack usage [-0, +1, m]
   lua_pushstring(L, "kv"); // Stack usage [-0, +1, m]
   lua_rawset(L, -3); // Stack usage [-2, +0, m]
   lua_rawset(L, LUA_REGISTRYINDEX); // Stack usage [-2, +0, m]
}

#endif

void
_edje_lua2_script_init(Edje *ed) // Stack usage [-63, +99, em]
{
   static Edje_Lua_Alloc ela = { MAX_LUA_MEM, 0 };
   Edje_Lua_Allocator *al;
   const luaL_Reg *l;
   char buf[256];
   void *data;
   int size;
   lua_State *L;

   if (ed->L) return;
   if (0 > _log_domain)
     _log_domain = eina_log_domain_register("lua", NULL);
   if (0 <= _log_domain)
     {
        _log_count++;
        eina_log_domain_level_set("lua", EINA_LOG_LEVEL_WARN);
     }

#ifndef RASTER_FORGOT_WHY
   _elua_init(); // This is actually truly pointless, even if raster remembers.
#endif
   L = ed->L = luaL_newstate();
   if (!L)
     {
        ERR("Lua state assign failed");
        return;
     }
   al = lua_newuserdata(L, sizeof(Edje_Lua_Allocator));
   al->ref = luaL_ref(L, LUA_REGISTRYINDEX);
   al->func = lua_getallocf(L, &(al->ud));
   al->ela = &ela;
   lua_setallocf(L, _elua_alloc, al); // Stack usage [-0, +0, -]
   lua_atpanic(L, _elua_custom_panic); // Stack usage [-0, +0, -]

// FIXME: figure out optimal gc settings later
//   lua_gc(L, LUA_GCSETPAUSE, 200);                              // Stack usage [-0, +0, e]
//   lua_gc(L, LUA_GCSETSTEPMUL, 200);                            // Stack usage [-0, +0, e]

   for (l = _elua_libs; l->func; l++) // Currently * 4
     {
#if LUA_VERSION_NUM >= 502
        luaL_requiref(L, l->name, l->func, 1); // Stack usage [-0, +1, e]
#else
        lua_pushcfunction(L, l->func); // Stack usage [-0, +1, m]
        lua_pushstring(L, l->name); // Stack usage [-0, +1, m]
        lua_call(L, 1, 0); // Stack usage [-2, +0, m]
#endif
     }

   _elua_bogan_protect(L); // Stack usage [+3, -3, m]

#if LUA_VERSION_NUM >= 502
   lua_newtable(L); // Stack usage [-0, +1, e]
   lua_pushvalue(L, -1); // Stack usage [-0, +1, -]
   lua_setglobal(L, _elua_edje_api); // Stack usage [-1, +0, e]
   luaL_setfuncs(L, _elua_edje_funcs, 0); // Stack usage [-0, +0, e]
#else
   luaL_register(L, _elua_edje_api, _elua_edje_funcs); // Stack usage [-0, +1, m]
#endif
   luaL_getmetatable(L, "bogan"); // Stack usage [-0, +1, -]
   lua_setmetatable(L, -2); // Stack usage [-1, +0, -]
   luaL_newmetatable(L, _elua_edje_meta); // Stack usage [-0, +1, m]
#if LUA_VERSION_NUM >= 502
   luaL_setfuncs(L, _elua_edje_gc_funcs, 0); // Stack usage [-0, +0, e]
#else
   luaL_register(L, 0, _elua_edje_gc_funcs); // Stack usage [-1, +1, m]
#endif

   lua_pop(L, 2); // Stack usage [-n, +0, -]

   _elua_add_functions(L, _elua_evas_api, _elua_evas_funcs, _elua_evas_meta, NULL, NULL);
   // Stack usage [-3, +5, m]
   _elua_add_functions(L, _elua_ecore_timer_api, _elua_ecore_timer_funcs, _elua_ecore_timer_meta, NULL, NULL);
   // Stack usage [-3, +5, m]
   _elua_add_functions(L, _elua_ecore_animator_api, _elua_ecore_animator_funcs, _elua_ecore_animator_meta, NULL, NULL);
   // Stack usage [-6, +11, m]
   _elua_add_functions(L, _elua_evas_edje_api, _elua_evas_edje_funcs, _elua_evas_edje_meta, _elua_evas_edje_parent, _elua_evas_api);
   // Stack usage [-6, +11, em]
   _elua_add_functions(L, _elua_evas_image_api, _elua_evas_image_funcs, _elua_evas_image_meta, _elua_evas_image_parent, _elua_evas_api);
   // Stack usage [-6, +11, em]
   _elua_add_functions(L, _elua_evas_line_api, _elua_evas_line_funcs, _elua_evas_line_meta, _elua_evas_line_parent, _elua_evas_api);
   // Stack usage [-6, +11, em]
   _elua_add_functions(L, _elua_evas_map_api, _elua_evas_map_funcs, _elua_evas_map_meta, NULL, NULL);
   // Stack usage [-3, +5, m]
   _elua_add_functions(L, _elua_evas_polygon_api, _elua_evas_polygon_funcs, _elua_evas_polygon_meta, _elua_evas_polygon_parent, _elua_evas_api);
   // Stack usage [-6, +11, em]
   _elua_add_functions(L, _elua_evas_text_api, _elua_evas_text_funcs, _elua_evas_text_meta, _elua_evas_text_parent, _elua_evas_api);
   // Stack usage [-6, +11, em]

   // weak table for our objects
   lua_pushlightuserdata(L, &_elua_objs); // Stack usage [-0, +1, -]
   lua_newtable(L); // Stack usage [-0, +1, m]
   lua_pushstring(L, "__mode"); // Stack usage [-0, +1, m]
   lua_pushstring(L, "kv"); // Stack usage [-0, +1, m]
   lua_rawset(L, -3); // Stack usage [-2, +0, m]
   lua_rawset(L, LUA_REGISTRYINDEX); // Stack usage [-2, +0, m]

   _elua_table_ptr_set(L, _elua_key, ed); // Stack usage [-2, +2, e]

   snprintf(buf, sizeof(buf), "edje/scripts/lua/%i", ed->collection->id);
   data = eet_read(ed->file->ef, buf, &size);

   if (data)
     {
        int err;

        /* This ends up pushing a function onto the stack for the lua_pcall() below to use.
         * The function is the compiled code. */
        err = luaL_loadbuffer(L, data, size, "edje_lua_script");  // Stack usage [-0, +1, m]
        if (err)
          {
             if (err == LUA_ERRSYNTAX)
               ERR("Lua load syntax error: %s",
                   lua_tostring(L, -1));  // Stack usage [-0, +0, m]
             else if (err == LUA_ERRMEM)
               ERR("Lua load memory allocation error: %s",
                   lua_tostring(L, -1));  // Stack usage [-0, +0, m]
          }
        free(data);
        /* This is not needed, pcalls don't longjmp(), that's why they are protected.
           if (setjmp(panic_jmp) == 1)
           {
             ERR("Lua script init panic");
             return;
           }
         */
        if ((err = lua_pcall(L, 0, 0, 0))) // Stack usage [-1, +0, -]
          _edje_lua2_error(L, err);  // Stack usage [-0, +0, m]
     }
}

void
_edje_lua2_script_shutdown(Edje *ed)
{
   Edje_Lua_Allocator *al;
   void *ud;
   if (!ed->L) return;
   lua_getallocf(ed->L, &ud);
   al = ud;
   // restore old allocator to close the state
   lua_setallocf(ed->L, al->func, al->ud);
   luaL_unref(ed->L, LUA_REGISTRYINDEX, al->ref);
   lua_close(ed->L);  // Stack usage irrelevant, as it's all gone now.
   ed->L = NULL;
   while (ed->lua_objs)
     {
        Edje_Lua_Obj *obj = (Edje_Lua_Obj *)ed->lua_objs;
        if (obj->free_func)
          {
             ERR("uncollected Lua object %p", obj);
             ed->lua_objs = eina_inlist_remove(ed->lua_objs, ed->lua_objs);
          }
        else
          {
             ERR("dangling Lua object %p", obj);
             ed->lua_objs = eina_inlist_remove(ed->lua_objs, ed->lua_objs);
          }
     }

   if (0 <= _log_domain)
     {
        _log_count--;
        if (0 >= _log_count)
          {
             eina_log_domain_unregister(_log_domain);
             _log_domain = -1;
          }
     }
}

void
_edje_lua2_script_load(Edje_Part_Collection *edc EINA_UNUSED, void *data EINA_UNUSED, int size EINA_UNUSED)  // Stack usage [-16, +20, em]
{
#ifndef RASTER_FORGOT_WHY
   _elua_init();  // Stack usage [-16, +20, em]
#endif
}

void
_edje_lua2_script_unload(Edje_Part_Collection *edc EINA_UNUSED)  // Stack usage [-0, +0, e]
{
#ifndef RASTER_FORGOT_WHY
   lua_State *L;

   if (!lstate) return;
   L = lstate;
   lua_gc(L, LUA_GCCOLLECT, 0);  // Stack usage [-0, +0, e]
#endif
}

void
_edje_lua2_error_full(const char *file, const char *fnc, int line,
                      lua_State *L, int err_code) // Stack usage [-0, +0, m]
{
   const char *err_type;

   switch (err_code)
     {
      case LUA_ERRRUN:
        err_type = "runtime";
        break;

      case LUA_ERRSYNTAX:
        err_type = "syntax";
        break;

      case LUA_ERRMEM:
        err_type = "memory allocation";
        break;

      case LUA_ERRERR:
        err_type = "error handler";
        break;

      default:
        err_type = "unknown";
        break;
     }
   eina_log_print
     (_edje_default_log_dom, EINA_LOG_LEVEL_ERR, file, fnc, line,
     "Lua %s error: %s", err_type, lua_tostring(L, -1));   // Stack usage [-0, +0, m]
}

/**
   @page luaref
   @section callbacks Lua callbacks

   These are lua functions that are called by the lua edje system when certain
   events occur.  If the functions don't exist in the lua group, they don't get
   called.

 */

/**
   @page luaref
   @subsection edje_shutdown Edje shutdown() callback.

   If a function called "shutdown" exists in a lua edje group, then it is called when
   that edje gets deleted.
 */
void
_edje_lua2_script_func_shutdown(Edje *ed) // Stack usage [-1, +1, em]
{
   int err;

   lua_getglobal(ed->L, "shutdown"); // Stack usage [-0, +1, e]
   if (!lua_isnil(ed->L, -1)) // Stack usage [-0, +0, -]
     {
        if ((err = lua_pcall(ed->L, 0, 0, 0))) // Stack usage [-1, +0, -]
          _edje_lua2_error(ed->L, err);  // Stack usage [-0, +0, m]
     }
   else
     lua_pop(ed->L, 1);  // Stack usage [-n, +0, -]
   _edje_lua2_script_shutdown(ed);
}

/**
   @page luaref
   @subsection edje_show Edje show() callback.

   If a function called "show" exists in a lua edje group, then it is called when
   that edje gets shown.
 */
void
_edje_lua2_script_func_show(Edje *ed)  // Stack usage [-1, +1, e]
{
   int err;

   lua_getglobal(ed->L, "show");
   if (!lua_isnil(ed->L, -1))
     {
        if ((err = lua_pcall(ed->L, 0, 0, 0)))
          _edje_lua2_error(ed->L, err);
     }
   else
     lua_pop(ed->L, 1);
}

/**
   @page luaref
   @subsection edje_hide Edje hide() callback.

   If a function called "hide" exists in a lua edje group, then it is called when
   that edje gets hidden.
 */
void
_edje_lua2_script_func_hide(Edje *ed)  // Stack usage [-1, +1, e]
{
   int err;

   lua_getglobal(ed->L, "hide");
   if (!lua_isnil(ed->L, -1))
     {
        if ((err = lua_pcall(ed->L, 0, 0, 0)))
          _edje_lua2_error(ed->L, err);
     }
   else
     lua_pop(ed->L, 1);
}

/**
   @page luaref
   @subsection edje_move Edje move(x, y) callback.

   If a function called "move" exists in a lua edje group, then it is called when
   that edje gets moved, with the new position passed to it.
 */
void
_edje_lua2_script_func_move(Edje *ed)  // Stack usage [-3, +3, e] or [-1, +1, e] if no matching function.
{
   int err;

   // FIXME: move all objects created by script
   lua_getglobal(ed->L, "move"); // Stack usage [-0, +1, e]
   if (!lua_isnil(ed->L, -1)) // Stack usage [-0, +0, -]
     {
        lua_pushinteger(ed->L, ed->x); // Stack usage [-0, +1, -]
        lua_pushinteger(ed->L, ed->y); // Stack usage [-0, +1, -]
        if ((err = lua_pcall(ed->L, 2, 0, 0))) // Stack usage [-3, +0, -]
          _edje_lua2_error(ed->L, err);
     }
   else
     lua_pop(ed->L, 1);  // Stack usage [-n, +0, -]
}

/**
   @page luaref
   @subsection edje_resize Edje resize(w, h) callback.

   If a function called "resize" exists in a lua edje group, then it is called when
   that edje gets resized, with the new size passed to it.
 */
void
_edje_lua2_script_func_resize(Edje *ed)  // Stack usage [-3, +3, e] or [-1, +1, e] if no matching function.
{
   int err;

   lua_getglobal(ed->L, "resize");
   if (!lua_isnil(ed->L, -1))
     {
        lua_pushinteger(ed->L, ed->w);
        lua_pushinteger(ed->L, ed->h);
        if ((err = lua_pcall(ed->L, 2, 0, 0)))
          _edje_lua2_error(ed->L, err);
     }
   else
     lua_pop(ed->L, 1);
}

/**
   @page luaref
   @subsection edje_message Edje message(id, type, ...) callback.

   If a function called "message" exists in a lua edje group, then it is called when
   that edje gets gets a message sent to it, with the message details passed to it.
   See edje.messagesend() for details of what each type means.  The arrays are
   passed as a table.
 */
void
_edje_lua2_script_func_message(Edje *ed, Edje_Message *em)  // Stack usage [-?, +?, em]  It's complicated, but it's even at least.
{
   int err, n, c, i;

   lua_getglobal(ed->L, "message"); // Stack usage [-0, +1, e]
   if (!lua_isnil(ed->L, -1)) // Stack usage [-0, +0, -]
     {
        n = 2;
        lua_pushinteger(ed->L, em->id); // Stack usage [-0, +1, -]
        switch (em->type)
          {
           case EDJE_MESSAGE_NONE:
             lua_pushstring(ed->L, "none"); // Stack usage [-0, +1, m]
             break;

           case EDJE_MESSAGE_SIGNAL:
             break;

           case EDJE_MESSAGE_STRING:
             lua_pushstring(ed->L, "str"); // Stack usage [-0, +1, m]
             lua_pushstring(ed->L, ((Edje_Message_String *)em->msg)->str);
             // Stack usage [-0, +1, m]
             n += 1;
             break;

           case EDJE_MESSAGE_INT:
             lua_pushstring(ed->L, "int"); // Stack usage [-0, +1, m]
             lua_pushinteger(ed->L, ((Edje_Message_Int *)em->msg)->val);
             // Stack usage [-0, +1, -]
             n += 1;
             break;

           case EDJE_MESSAGE_FLOAT:
             lua_pushstring(ed->L, "float"); // Stack usage [-0, +1, m]
             lua_pushnumber(ed->L, ((Edje_Message_Float *)em->msg)->val);
             // Stack usage [-0, +1, -]
             n += 1;
             break;

           case EDJE_MESSAGE_STRING_SET:
             lua_pushstring(ed->L, "strset"); // Stack usage [-0, +1, m]
             c = ((Edje_Message_String_Set *)em->msg)->count;
             lua_createtable(ed->L, c, 0); // Stack usage [-0, +1, m]
             for (i = 0; i < c; i++)
               {
                  lua_pushstring(ed->L, ((Edje_Message_String_Set *)em->msg)->str[i]);
                  // Stack usage [-0, +1, m]
                  // It's OK to bypass the metatable in these cases,
                  // we create the table, and know there is no metatable.  B-)
                  lua_rawseti(ed->L, -2, i + 1); // Stack usage [-1, +0, m]
               }
             n += 1;
             break;

           case EDJE_MESSAGE_INT_SET:
             lua_pushstring(ed->L, "intset"); // Stack usage [-0, +1, m]
             c = ((Edje_Message_Int_Set *)em->msg)->count;
             lua_createtable(ed->L, c, 0); // Stack usage [-0, +1, m]
             for (i = 0; i < c; i++)
               {
                  lua_pushinteger(ed->L, ((Edje_Message_Int_Set *)em->msg)->val[i]);
                  // Stack usage [-0, +1, -]
                  lua_rawseti(ed->L, -2, i + 1); // Stack usage [-1, +0, m]
               }
             n += 1;
             break;

           case EDJE_MESSAGE_FLOAT_SET:
             lua_pushstring(ed->L, "floatset"); // Stack usage [-0, +1, m]
             c = ((Edje_Message_Float_Set *)em->msg)->count;
             lua_createtable(ed->L, c, 0); // Stack usage [-0, +1, m]
             for (i = 0; i < c; i++)
               {
                  lua_pushnumber(ed->L, ((Edje_Message_Float_Set *)em->msg)->val[i]);
                  // Stack usage [-0, +1, -]
                  lua_rawseti(ed->L, -2, i + 1); // Stack usage [-1, +0, m]
               }
             n += 1;
             break;

           case EDJE_MESSAGE_STRING_INT:
             lua_pushstring(ed->L, "strint"); // Stack usage [-0, +1, m]
             lua_pushstring(ed->L, ((Edje_Message_String_Int *)em->msg)->str);
             // Stack usage [-0, +1, m]
             lua_pushinteger(ed->L, ((Edje_Message_String_Int *)em->msg)->val);
             // Stack usage [-0, +1, -]
             n += 2;
             break;

           case EDJE_MESSAGE_STRING_FLOAT:
             lua_pushstring(ed->L, "strfloat"); // Stack usage [-0, +1, m]
             lua_pushstring(ed->L, ((Edje_Message_String_Float *)em->msg)->str);
             // Stack usage [-0, +1, m]
             lua_pushnumber(ed->L, ((Edje_Message_String_Float *)em->msg)->val);
             // Stack usage [-0, +1, -]
             n += 2;
             break;

           case EDJE_MESSAGE_STRING_INT_SET:
             lua_pushstring(ed->L, "strintset"); // Stack usage [-0, +1, m]
             lua_pushstring(ed->L, ((Edje_Message_String_Int_Set *)em->msg)->str);
             // Stack usage [-0, +1, m]
             c = ((Edje_Message_String_Int_Set *)em->msg)->count;
             lua_createtable(ed->L, c, 0); // Stack usage [-0, +1, m]
             for (i = 0; i < c; i++)
               {
                  lua_pushinteger(ed->L, ((Edje_Message_String_Int_Set *)em->msg)->val[i]);
                  // Stack usage [-0, +1, -]
                  lua_rawseti(ed->L, -2, i + 1); // Stack usage [-1, +0, m]
               }
             n += 2;
             break;

           case EDJE_MESSAGE_STRING_FLOAT_SET:
             lua_pushstring(ed->L, "strfloatset"); // Stack usage [-0, +1, m]
             lua_pushstring(ed->L, ((Edje_Message_String_Float_Set *)em->msg)->str);
             // Stack usage [-0, +1, m]
             c = ((Edje_Message_String_Float_Set *)em->msg)->count;
             lua_createtable(ed->L, c, 0); // Stack usage [-0, +1, m]
             for (i = 0; i < c; i++)
               {
                  lua_pushnumber(ed->L, ((Edje_Message_String_Float_Set *)em->msg)->val[i]);
                  // Stack usage [-0, +1, -]
                  lua_rawseti(ed->L, -2, i + 1); // Stack usage [-1, +0, m]
               }
             n += 2;
             break;

           default:
             break;
          }
        if ((err = lua_pcall(ed->L, n, 0, 0))) // Stack usage [-n+1, +0, -]
          _edje_lua2_error(ed->L, err);
     }
   else
     lua_pop(ed->L, 1);  // Stack usage [-n, +0, -]
}

/**
   @page luaref
   @subsection edje_signal Edje signal(signal, source) callback.

   If a function called "signal" exists in a lua edje group, then it is called when
   ever a signal arrives, with the signal details passed to it.

 */
void
_edje_lua2_script_func_signal(Edje *ed, const char *sig, const char *src)  // Stack usage [-3, +3, em] or [-1, +1, e] if no matching function.
{
   int err;

   lua_getglobal(ed->L, "signal");
   if (!lua_isnil(ed->L, -1))
     {
        lua_pushstring(ed->L, sig);
        lua_pushstring(ed->L, src);
        if ((err = lua_pcall(ed->L, 2, 0, 0)))
          _edje_lua2_error(ed->L, err);
     }
   else
     lua_pop(ed->L, 1);
}

