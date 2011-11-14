#include "edje_private.h"
#include <ctype.h>

#ifdef _WIN32
# define FMT_SIZE_T "%Iu"
#else
# define FMT_SIZE_T "%zu"
#endif

//--------------------------------------------------------------------------//
#define MAX_LUA_MEM (4 * (1024 * 1024))
#define ELO "|-ELO"

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

To get you started, here's an example:
@ref lua_script.edc

@section args Lua function argument and return syntax

Some of the lua functions can accept a table as well as separate arguments.
Some of them return tables.

@section classes Lua classes

 */

//--------------------------------------------------------------------------//
typedef struct _Edje_Lua_Alloc       Edje_Lua_Alloc;
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

struct _Edje_Lua_Obj
{
   EINA_INLIST;

   Edje         *ed;
   void        (*free_func) (void *obj);
   const char   *meta;
};

struct _Edje_Lua_Animator
{
   Edje_Lua_Obj     obj;
   Ecore_Animator  *animator;
   int              fn_ref;
};

struct _Edje_Lua_Timer
{
   Edje_Lua_Obj     obj;
   Ecore_Timer     *timer;
   int              fn_ref;
};

struct _Edje_Lua_Transition
{
   Edje_Lua_Obj     obj;
   Ecore_Animator  *animator;
   double           transition, start;
   int              fn_ref;
};

struct _Edje_Lua_Evas_Object
{
   Edje_Lua_Obj     obj;
   Evas_Object     *evas_obj;
   int              x, y;
};

struct _Edje_Lua_Map
{
   Edje_Lua_Obj     obj;
   Evas_Map        *map;
};


static void _elua_add_functions(lua_State *L, const char *api, const luaL_Reg *funcs, const char *meta, const char *parent, const char *base);
static Eina_Bool _elua_isa(Edje_Lua_Obj *obj, const char *type);

//--------------------------------------------------------------------------//
static lua_State *lstate = NULL;
static const char *_elua_key = "key";
static const char *_elua_objs = "objs";
static jmp_buf panic_jmp;
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
static const char *_elua_evas_line_meta = "vas_line_meta";
static const char *_elua_evas_map_meta = "evas_map_meta";
static const char *_elua_evas_polygon_meta = "evas_polygon_meta";
static const char *_elua_evas_text_meta = "evas_text_meta";


static int _elua_obj_gc(lua_State *L);

static const struct luaL_reg _elua_edje_gc_funcs [] =
{
     {"__gc", _elua_obj_gc}, // garbage collector func for edje objects

     {NULL, NULL} // end
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

     {NULL, NULL} // end
};

//--------------------------------------------------------------------------//
static void *
_elua_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
   Edje_Lua_Alloc *ela = ud;
   void *ptr2;

   ela->cur += nsize - osize;
   if (ela->cur > ela->max)
     {
        ERR("Lua memory limit of " FMT_SIZE_T " bytes reached (" FMT_SIZE_T  " allocated)",
            ela->max, ela->cur);
        return NULL;
     }
   if (nsize == 0)
     {
        free(ptr);
        return NULL;
     }

   ptr2 = realloc(ptr, nsize);
   if (ptr2) return ptr2;
   ERR("Lua cannot re-allocate " FMT_SIZE_T " bytes", nsize);
   return ptr2;
}

static int
_elua_custom_panic(lua_State *L __UNUSED__)
{
   ERR("Lua Panic!!!!");
   return 1;
}

// Really only used to manage the pointer to our edje.
static void
_elua_table_ptr_set(lua_State *L, const void *key, const void *val)
{
   lua_pushlightuserdata(L, (void *)key);
   lua_pushlightuserdata(L, (void *)val);
   lua_settable(L, LUA_REGISTRYINDEX);
}

static const void *
_elua_table_ptr_get(lua_State *L, const void *key)
{
   const void *ptr;
   lua_pushlightuserdata(L, (void *)key);
   lua_gettable(L, LUA_REGISTRYINDEX);
   ptr = lua_topointer(L, -1);
   lua_pop(L, 1);
   return ptr;
}

/* XXX: not used
static void
_elua_table_ptr_del(lua_State *L, const void *key)
{
   lua_pushlightuserdata(L, (void *)key);
   lua_pushnil(L);
   lua_settable(L, LUA_REGISTRYINDEX);
}
*/

/*
 * Cori: Assumes object to be saved on top of stack
 */
static void
_elua_ref_set(lua_State *L, void *key)
{
   lua_pushlightuserdata(L, &_elua_objs);
   lua_rawget(L, LUA_REGISTRYINDEX);
   lua_pushlightuserdata(L, key);
   lua_pushvalue(L,-3); // key & obj table & obj
   lua_rawset(L, -3);
   lua_pop(L, 1); // pop obj table
}

/*
 * Cori: Get an object from the object table
 */
static void *
_elua_ref_get(lua_State *L, void *key)
{
   lua_pushlightuserdata(L, &_elua_objs);
   lua_rawget(L, LUA_REGISTRYINDEX);
   lua_pushlightuserdata(L, key);
   lua_rawget(L, -2);
   lua_remove(L, -2); // kill obj table
   return lua_touserdata(L, -2);
}

static Edje_Lua_Obj *
_elua_obj_new(lua_State *L, Edje *ed, int size, const char *metatable)
{
   Edje_Lua_Obj *obj;

   obj = (Edje_Lua_Obj *)lua_newuserdata(L, size);
   memset(obj, 0, size);
   ed->lua_objs = eina_inlist_append(ed->lua_objs, EINA_INLIST_GET(obj));

   luaL_getmetatable(L, metatable);
   lua_setmetatable(L, -2);
   obj->ed = ed;
   obj->meta = metatable;

   _elua_ref_set(L, obj);
   return obj;
}

static void
_elua_obj_free(lua_State *L __UNUSED__, Edje_Lua_Obj *obj)
{
   if (!obj->free_func) return;
   obj->free_func(obj);
   obj->ed->lua_objs = eina_inlist_remove(obj->ed->lua_objs, EINA_INLIST_GET(obj));
   obj->free_func = NULL;
   obj->ed = NULL;
}

static int
_elua_obj_gc(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   if (!obj) return 0;
   _elua_obj_free(L, obj);
   return 0;
}

static int
_elua_obj_del(lua_State *L)
{
   return _elua_obj_gc(L);
}

static void
_elua_gc(lua_State *L)
{
   lua_gc(L, LUA_GCCOLLECT, 0);
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
_elua_push_name(lua_State *L, char *q, int index)
{
   char *p = q;
   char temp = '\0';

   // A simplistic scan through an identifier, it's wrong, but it's quick,
   // and we don't mind that it's wrong, coz this is only internal.
   while (isalnum(*q))
      q++;
   temp = *q;
   *q = '\0';
   if (index > 0)
      lua_getfield(L, index, p);
   else
      lua_pushstring(L, p);
   *q = temp;

   return q;
}

static int
_elua_scan_params(lua_State *L, int i, Eina_Bool tr, char *params, ...)
{
   va_list vl;
   char *f = strdup(params);
   char *p = f;
   int n = 0, j = i, count = 0;
   Eina_Bool table = EINA_FALSE;

   if (!f) return -1;
   va_start(vl, params);

   if (lua_istable(L, i))
     {
        j = -1;
        table = EINA_TRUE;
     }

   while (*p)
     {
        char *q;
        Eina_Bool get = EINA_TRUE;

        while (isspace(*p))
           p++;
        q = p + 1;
        switch (*p)
          {
             case '%':
               {
                  if (table)
                    {
                       q = _elua_push_name(L, q, i);
                    }
                  if (lua_isnumber(L, j))
                    {
                       int *v = va_arg(vl, int *);
                       *v = lua_tointeger(L, j);
                       n++;
                    }
                  break;
               }
             case '#':
               {
                  if (table)
                    {
                       q = _elua_push_name(L, q, i);
                    }
                  if (lua_isnumber(L, j))
                    {
                       double *v = va_arg(vl, double *);
                       *v = lua_tonumber(L, j);
                       n++;
                    }
                  break;
               }
             case '$':
               {
                  if (table)
                    {
                       q = _elua_push_name(L, q, i);
                    }
                  if (lua_isstring(L, j))
                    {
                       char **v = va_arg(vl, char **);
                       size_t len;
                       char *temp = (char *) lua_tolstring(L, j, &len);

                       len++;  // Cater for the null at the end.
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
                       q = _elua_push_name(L, q, i);
                    }
                  if (lua_isboolean(L, j))
                    {
                       int *v = va_arg(vl, int *);
                       *v = lua_toboolean(L, j);
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
               }
            else
                j++;
            count++;
          }
        p = q;
     }

   if (tr)
     {
        if (table)
           lua_settop(L, i);
        else
           lua_newtable(L);
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
_elua_ret(lua_State *L, char *params, ...)
{
   va_list vl;
   char *f = strdup(params);
   char *p = f;
   int n = 0;

   if (!f) return -1;
   va_start(vl, params);

   while (*p)
     {
        char *q;
        Eina_Bool set = EINA_TRUE;

        while (isspace(*p))
           p++;
        q = p + 1;
        switch (*p)
          {
             case '%':
               {
                  q = _elua_push_name(L, q, -1);
                  lua_pushinteger(L, va_arg(vl, int));
                  break;
               }
             case '#':
               {
                  q = _elua_push_name(L, q, -1);
                  lua_pushnumber(L, va_arg(vl, double));
                  break;
               }
             case '$':
               {
                  q = _elua_push_name(L, q, -1);
                  lua_pushstring(L, va_arg(vl, char *));
                  break;
               }
             case '!':
               {
                  q = _elua_push_name(L, q, -1);
                  lua_pushboolean(L, va_arg(vl, int));
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
             lua_settable(L, -3);
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
*/

static int _elua_echo(lua_State *L);

static int _elua_date(lua_State *L);
static int _elua_looptime(lua_State *L);
static int _elua_seconds(lua_State *L);

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
static const struct luaL_reg _elua_edje_funcs [] =
{
   // add an echo too to make it more shelly
     {"echo",         _elua_echo}, // test func - echo (i know we have print. test)
   // FIXME: add logging functions here, probably to it's own domain, or even a script defined domain.

   // system information (time, date blah blah)
     {"date",         _elua_date}, // get date in a table
     {"looptime",     _elua_looptime}, // get loop time
     {"seconds",      _elua_seconds}, // get seconds

   // query edje - size, pos
     {"geom",         _elua_objgeom}, // get while edje object geometry in canvas
     {"pos",          _elua_objpos}, // get while edje object pos in canvas
     {"size",         _elua_objsize}, // get while edje object pos in canvas

   // talk to application/caller
     {"emit",         _elua_emit}, // emit signal + src
     {"messagesend",  _elua_messagesend}, // send a structured message

   // time based "callback" systems
     {"animator",     _elua_animator}, // add animator
     {"timer",        _elua_timer}, // add timer
     {"transition",   _elua_transition}, // add transition
   // FIXME: need poller

   // set and query color / text class
     {"color_class",  _elua_color_class},
     {"text_class",   _elua_text_class},

   // create new objects
     {"edje",         _elua_edje},
     {"image",        _elua_image},  // defaults to a filled image.
     {"line",         _elua_line},
     {"map",          _elua_map},
     {"polygon",      _elua_polygon},
     {"rect",         _elua_rect},
     {"text",         _elua_text},
//     {"textblock",    _elua_textblock},  /* XXX: disabled until there are enough textblock functions implemented to make it actually useful

   // FIXME: add the new sound stuff.

     {NULL, NULL} // end
};

/**
@page luaref
@subsubsection echo edje:echo()

Make lua a bit shelly.

Param - a string to print to the console.
*/
static int
_elua_echo(lua_State *L)
{
   const char *string = luaL_checkstring(L, 1);
   printf("%s\n", string);
   return 0;
}

//-------------
/**
@page luaref
@subsubsection date edje:date()

Gives us the current time and date.

Returns a table with these members -
year - Year.
month - Month as an integer.
day - Day of the month.
yearday - Day of the year.
weekday - Day of the week as an integer.
hour -
min -
sec - Seconds as a number.

*/
static int
_elua_date(lua_State *L)
{
   static time_t       last_tzset = 0;
   struct timeval      timev;
   struct tm          *tm;
   time_t              tt;

   lua_newtable(L);
   gettimeofday(&timev, NULL);
   tt = (time_t)(timev.tv_sec);
   if ((tt > (last_tzset + 1)) || (tt < (last_tzset - 1)))
     {
        last_tzset = tt;
        tzset();
     }
   tm = localtime(&tt);
   if (tm)
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

static int
_elua_looptime(lua_State *L)
{
   double t = ecore_loop_time_get();
   lua_pushnumber(L, t);
   return 1;
}

static int
_elua_seconds(lua_State *L)
{
   double t = ecore_time_get();
   lua_pushnumber(L, t);
   return 1;
}

//-------------
static int
_elua_objgeom(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   if (!lua_istable(L, 1)) lua_newtable(L);
   _elua_ret(L, "%x %y %w %h", ed->x, ed->y, ed->w, ed->h);
   return 1;
}
static int

_elua_objpos(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   if (!lua_istable(L, 1)) lua_newtable(L);
   _elua_ret(L, "%x %y", ed->x, ed->y);
   return 1;
}

static int
_elua_objsize(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   if (!lua_istable(L, 1)) lua_newtable(L);
   _elua_ret(L, "%w %h", ed->w, ed->h);
   return 1;
}

//-------------
static int
_elua_emit(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   const char *sig = luaL_checkstring(L, 1);
   const char *src = luaL_checkstring(L, 2);
   if ((!sig) || (!src)) return 0;
   _edje_emit(ed, sig, src);
   return 0;
}

static int
_elua_messagesend(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   int id = luaL_checkinteger(L, 1);
   const char *type = luaL_checkstring(L, 2);
   if (!type) return 0;
   if (!strcmp(type, "none"))
     {
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_NONE, id, NULL);
     }
   else if (!strcmp(type, "sig"))
     {
        const char *sig = luaL_checkstring(L, 3);
        const char *src = luaL_checkstring(L, 4);
        _edje_emit(ed, sig, src);
     }
   else if (!strcmp(type, "str"))
     {
        Edje_Message_String *emsg;
        const char *str = luaL_checkstring(L, 3);
        emsg = alloca(sizeof(Edje_Message_String));
        emsg->str = (char *)str;
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING, id, emsg);
     }
   else if (!strcmp(type, "int"))
     {
        Edje_Message_Int *emsg;
        int val = luaL_checkinteger(L, 3);
        emsg = alloca(sizeof(Edje_Message_Int));
        emsg->val = val;
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_INT, id, emsg);
     }
   else if (!strcmp(type, "float"))
     {
        Edje_Message_Float *emsg;
        float val = luaL_checknumber(L, 3);
        emsg = alloca(sizeof(Edje_Message_Float));
        emsg->val = val;
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_FLOAT, id, emsg);
     }
   else if (!strcmp(type, "strset"))
     {
        Edje_Message_String_Set *emsg;
        int i, n;
        const char *str;
        luaL_checktype(L, 3, LUA_TTABLE);
        n = lua_objlen(L, 3);
        emsg = alloca(sizeof(Edje_Message_String_Set) + ((n - 1) * sizeof(char *)));
        emsg->count = n;
        for (i = 1; i <= n; i ++)
          {
             lua_rawgeti(L, 3, i);
             str = lua_tostring(L, -1);
             emsg->str[i - 1] = (char *)str;
          }
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING_SET, id, emsg);
     }
   else if (!strcmp(type, "intset"))
     {
        Edje_Message_Int_Set *emsg;
        int i, n;
        luaL_checktype(L, 3, LUA_TTABLE);
        n = lua_objlen(L, 3);
        emsg = alloca(sizeof(Edje_Message_Int_Set) + ((n - 1) * sizeof(int)));
        emsg->count = n;
        for (i = 1; i <= n; i ++)
          {
             lua_rawgeti(L, 3, i);
             emsg->val[i - 1] = lua_tointeger(L, -1);
          }
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_INT_SET, id, emsg);
     }
   else if (!strcmp(type, "floatset"))
     {
        Edje_Message_Float_Set *emsg;
        int i, n;
        luaL_checktype(L, 3, LUA_TTABLE);
        n = lua_objlen(L, 3);
        emsg = alloca(sizeof(Edje_Message_Float_Set) + ((n - 1) * sizeof(double)));
        emsg->count = n;
        for (i = 1; i <= n; i ++)
          {
             lua_rawgeti(L, 3, i);
             emsg->val[i - 1] = lua_tonumber(L, -1);
          }
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_FLOAT_SET, id, emsg);
     }
   else if (!strcmp(type, "strint"))
     {
        Edje_Message_String_Int *emsg;
        const char *str = luaL_checkstring(L, 3);
        emsg = alloca(sizeof(Edje_Message_String_Int));
        emsg->str = (char *)str;
        emsg->val =  luaL_checkinteger(L, 4);
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING_INT, id, emsg);
     }
   else if (!strcmp(type, "strfloat"))
     {
        Edje_Message_String_Float *emsg;
        const char *str = luaL_checkstring(L, 3);
        emsg = alloca(sizeof(Edje_Message_String_Float));
        emsg->str = (char *)str;
        emsg->val =  luaL_checknumber(L, 4);
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING_FLOAT, id, emsg);
     }
   else if (!strcmp(type, "strintset"))
     {
        Edje_Message_String_Int_Set *emsg;
        int i, n;
        const char *str = luaL_checkstring(L, 3);
        if (!str) return 0;
        luaL_checktype(L, 4, LUA_TTABLE);
        n = lua_objlen(L, 4);
        emsg = alloca(sizeof(Edje_Message_String_Int_Set) + ((n - 1) * sizeof(int)));
        emsg->str = (char *)str;
        emsg->count = n;
        for (i = 1; i <= n; i ++)
          {
             lua_rawgeti(L, 4, i);
             emsg->val[i - 1] = lua_tointeger(L, -1);
          }
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING_INT_SET, id, emsg);
     }
   else if (!strcmp(type, "strfloatset"))
     {
        Edje_Message_String_Float_Set *emsg;
        int i, n;
        const char *str = luaL_checkstring(L, 3);
        if (!str) return 0;
        luaL_checktype(L, 4, LUA_TTABLE);
        n = lua_objlen(L, 4);
        emsg = alloca(sizeof(Edje_Message_String_Float_Set) + ((n - 1) * sizeof(double)));
        emsg->str = (char *)str;
        emsg->count = n;
        for (i = 1; i <= n; i ++)
          {
             lua_rawgeti(L, 4, i);
             emsg->val[i - 1] = lua_tonumber(L, -1);
          }
        _edje_message_send(ed, EDJE_QUEUE_APP, EDJE_MESSAGE_STRING_FLOAT_SET, id, emsg);
     }
   return 0;
}

//-------------
static Eina_Bool
_elua_animator_cb(void *data)
{
   Edje_Lua_Animator *ela = data;
   lua_State *L;
   int ret = 0, err = 0;

   if (!ela->obj.ed) return 0;
   L = ela->obj.ed->L;
   if (!L) return 0;
   lua_rawgeti(L, LUA_REGISTRYINDEX, ela->fn_ref);
   if (setjmp(panic_jmp) == 1)
     {
        LE("Animator callback panic");
        _edje_lua2_error(L, err);
        _elua_obj_free(L, (Edje_Lua_Obj *)ela);
        _elua_gc(L);
        return 0;
     }
   if ((err = lua_pcall(L, 0, 1, 0)))
     {
        _edje_lua2_error(L, err);
        _elua_obj_free(L, (Edje_Lua_Obj *)ela);
        _elua_gc(L);
        return 0;
     }
   ret = lua_toboolean(L, -1);
   lua_pop(L, 1);
   if (ret == 0) _elua_obj_free(L, (Edje_Lua_Obj *)ela);
   _elua_gc(L);
   return ret;
}

static void
_elua_animator_free(void *obj)
{
   Edje_Lua_Animator *ela = obj;
   lua_State *L;
   if (!ela->obj.ed) return;
   L = ela->obj.ed->L;
   luaL_unref(L, LUA_REGISTRYINDEX, ela->fn_ref);
   ela->fn_ref  = 0;
   ecore_animator_del(ela->animator);
   ela->animator = NULL;
}

static int
_elua_animator(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   Edje_Lua_Animator *ela;

   luaL_checkany(L, 1);

   // FIXME: This, and the other two timer thingies, should be it's own class I think.  But that might be API change, so wait until after the freeze.
   ela = (Edje_Lua_Animator *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Animator), _elua_evas_meta);
   ela->obj.free_func = _elua_animator_free;
   ela->animator = ecore_animator_add(_elua_animator_cb, ela);
   lua_pushvalue(L, 1);
   ela->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
   _elua_gc(L);
   return 1;
}

static Eina_Bool
_elua_timer_cb(void *data)
{
   Edje_Lua_Timer *elt = data;
   lua_State *L;
   int ret = 0, err = 0;

   if (!elt->obj.ed) return 0;
   L = elt->obj.ed->L;
   if (!L) return 0;
   lua_rawgeti(L, LUA_REGISTRYINDEX, elt->fn_ref);
   if (setjmp(panic_jmp) == 1)
     {
        LE("Timer callback panic");
        _edje_lua2_error(L, err);
        _elua_obj_free(L, (Edje_Lua_Obj *)elt);
        _elua_gc(L);
        return 0;
     }
   if ((err = lua_pcall(L, 0, 1, 0)))
     {
        _edje_lua2_error(L, err);
        _elua_obj_free(L, (Edje_Lua_Obj *)elt);
        _elua_gc(L);
        return 0;
     }
   ret = lua_toboolean(L, -1);
   lua_pop(L, 1);
   if (ret == 0) _elua_obj_free(L, (Edje_Lua_Obj *)elt);
   _elua_gc(L);
   return ret;
}

static void
_elua_timer_free(void *obj)
{
   Edje_Lua_Timer *elt = obj;
   lua_State *L;
   if (!elt->obj.ed) return;
   L = elt->obj.ed->L;
   luaL_unref(L, LUA_REGISTRYINDEX, elt->fn_ref); //0
   elt->fn_ref  = 0;
   ecore_timer_del(elt->timer);
   elt->timer = NULL;
}

static int
_elua_timer(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   Edje_Lua_Timer *elt;
   double val;

   val = luaL_checknumber(L, 1);
   luaL_checkany(L, 2);

   elt = (Edje_Lua_Timer *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Timer), _elua_evas_meta);
   elt->obj.free_func = _elua_timer_free;
   elt->timer = ecore_timer_add(val, _elua_timer_cb, elt);
   lua_pushvalue(L, 2);
   elt->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
   _elua_gc(L);
   return 1;
}

static Eina_Bool
_elua_transition_cb(void *data)
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
   lua_rawgeti(L, LUA_REGISTRYINDEX, elt->fn_ref);
   lua_pushnumber(L, t);
   if (setjmp(panic_jmp) == 1)
     {
        LE("Transition callback panic");
        _edje_lua2_error(L, err);
        _elua_obj_free(L, (Edje_Lua_Obj *)elt);
        _elua_gc(L);
        return 0;
     }
   if ((err = lua_pcall(L, 1, 1, 0)))
     {
        _edje_lua2_error(L, err);
        _elua_obj_free(L, (Edje_Lua_Obj *)elt);
        _elua_gc(L);
        return 0;
     }
   ret = lua_toboolean(L, -1);
   lua_pop(L, 1);
   if (t >= 1.0) ret = 0;
   if (ret == 0) _elua_obj_free(L, (Edje_Lua_Obj *)elt);
   _elua_gc(L);
   return ret;
}

static void
_elua_transition_free(void *obj)
{
   Edje_Lua_Transition *elt = obj;
   lua_State *L;
   if (!elt->obj.ed) return;
   L = elt->obj.ed->L;
   luaL_unref(L, LUA_REGISTRYINDEX, elt->fn_ref); //0
   elt->fn_ref  = 0;
   ecore_animator_del(elt->animator);
   elt->animator = NULL;
}

static int
_elua_transition(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   Edje_Lua_Transition *elt;
   double val;

   val = luaL_checknumber(L, 1);
   luaL_checkany(L, 2);

   elt = (Edje_Lua_Transition *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Transition), _elua_evas_meta);
   elt->obj.free_func = _elua_transition_free;
   elt->animator = ecore_animator_add(_elua_transition_cb, elt);
   if (val < 0.0000001) val = 0.0000001;
   elt->transition = val;
   elt->start = ecore_loop_time_get();
   lua_pushvalue(L, 2);
   elt->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
   _elua_gc(L);
   return 1;
}

//-------------
static int
_elua_color_class(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   Edje_Color_Class *c_class;
   const char *class = luaL_checkstring(L, 1);
   int r, g, b, a;

   if (!class) return 0;

   if (_elua_scan_params(L, 2, EINA_TRUE, "%r %g %b %a", &r, &g, &b, &a) > 0)
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
   return 1;
}

static int
_elua_text_class(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   Edje_Text_Class *t_class;
   const char *class = luaL_checkstring(L, 1);
   char *font = NULL;
   Evas_Font_Size size = 0;

   if (!class) return 0;

   // Just like color_class above, this does things differently from embryo,
   // for the same reason.
   if (_elua_scan_params(L, 2, EINA_TRUE, "$font %size", &font, &size) > 0)
        edje_text_class_set(class, font, size);

   t_class = _edje_text_class_find(ed, class);
   if (!t_class) return 0;

   _elua_ret(L, "$font %size", t_class->font, t_class->size);
   return 1;
}

//-------------
static void
_elua_evas_obj_free(void *obj)
{
   Edje_Lua_Evas_Object *elo = obj;

   if (!elo->obj.ed) return;
   evas_object_del(elo->evas_obj);
   elo->evas_obj = NULL;
}

#define _ELUA_PLANT_EVAS_OBJECT(type, meta, free)            \
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);     \
   type *elo;                                                \
   elo = (type *)_elua_obj_new(L, ed, sizeof(type), meta);   \
   elo->obj.free_func = free;

static void
_elua_polish_evas_object(Edje *ed, Edje_Lua_Evas_Object *elo)
{
   evas_object_smart_member_add(elo->evas_obj, ed->obj);
   evas_object_clip_set(elo->evas_obj, ed->base.clipper);
   evas_object_move(elo->evas_obj, ed->x, ed->y);
   evas_object_resize(elo->evas_obj, 0, 0);
   evas_object_data_set(elo->evas_obj, ELO, elo);
}

static int
_elua_edje(lua_State *L)
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_edje_meta, _elua_evas_obj_free)
   elo->evas_obj = edje_object_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
}

static int
_elua_image(lua_State *L)
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_image_meta, _elua_evas_obj_free)
   elo->evas_obj = evas_object_image_filled_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
}

static int
_elua_line(lua_State *L)
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_line_meta, _elua_evas_obj_free)
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

static int
_elua_map(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   Edje_Lua_Map *elm;
   int count;

   count = luaL_checkinteger(L, 1);

   elm = (Edje_Lua_Map *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Map), _elua_evas_map_meta);
   elm->obj.free_func = _elua_map_free;
   elm->map = evas_map_new(count);
   lua_pushvalue(L, 2);
   _elua_gc(L);
   return 1;
}

static int
_elua_polygon(lua_State *L)
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_polygon_meta, _elua_evas_obj_free)
   elo->evas_obj = evas_object_polygon_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
}

/**
@page luaref
@subsubsection rect edje:rect()

Create a rectangle.

Returns an evas rectangle.
*/
static int
_elua_rect(lua_State *L)
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_meta, _elua_evas_obj_free)
   elo->evas_obj = evas_object_rectangle_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
}

static int
_elua_text(lua_State *L)
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_text_meta, _elua_evas_obj_free)
   elo->evas_obj = evas_object_text_add(evas_object_evas_get(ed->obj));
   _elua_polish_evas_object(ed, elo);
   return 1;
}

/* XXX: disabled until there are enough textblock functions implemented to make it actually useful
static int
_elua_textblock(lua_State *L)
{
   _ELUA_PLANT_EVAS_OBJECT(Edje_Lua_Evas_Object, _elua_evas_textblock_meta, _elua_evas_obj_free)
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

The lua evas class includes functions for dealing with evas objects.
*/

static int _elua_obj_del(lua_State *L);

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
static int _elua_obj_map_source(lua_State *L);

static const char *_elua_evas_api = "evas";
static const struct luaL_reg _elua_evas_funcs [] =
{
   // generic object methods
     {"del",          _elua_obj_del}, // generic del any object created for edje (evas objects, timers, animators, transitions... everything)

   // now evas stuff (manipulate, delete etc.)
     {"hide",         _elua_hide}, // hide, return current visibility
     {"show",         _elua_show}, // show, return current visibility
     {"visible",      _elua_visible}, // get object visibility

     {"above",        _elua_above}, // get object above or stack obj above given obj
     {"below",        _elua_below}, // get object below or stack obj below given obj
     {"bottom",       _elua_bottom}, // get bottom
     {"lower",        _elua_lower}, // lower to bottom
     {"raise",        _elua_raise}, // raise to top
     {"top",          _elua_top}, // get top

     {"geom",         _elua_geom}, // move and resize and return current geometry
     {"move",         _elua_move}, // move, return current position
     {"pos",          _elua_pos}, // move, return current position
     {"resize",       _elua_resize}, // resize, return current size
     {"size",         _elua_size}, // resize, return current size

     {"clip",         _elua_clip}, // set clip obj, return clip object
     {"clipees",      _elua_clipees}, // get clip children
     {"unclip",       _elua_unclip}, // clear clip obj

     {"type",         _elua_type}, // get object type

     {"pass",         _elua_pass}, // set pass events, get pass events
     {"precise",      _elua_precise}, // set precise inside flag, get precise
     {"repeat",       _elua_repeat}, // set repeat events, get repeat events

     {"color",        _elua_color}, // set color, return color
//     {"color_class",  _elua_object_color_class}, // get or set object color class

   // FIXME: set callbacks (mouse down, up, blah blah blah)
   //
   // FIXME: set scale (explicit value)
   // FIXME: need to set auto-scale (same as scale: 1)

   // FIXME: later - set render op, anti-alias, pointer mode (autograb, nograb)
   // FIXME: later -

   // map api here
     {"map",           _elua_obj_map},
     {"map_enable",    _elua_obj_map_enable},
     {"map_source",    _elua_obj_map_source},

     {NULL, NULL} // end
};

//-------------
static int
_elua_hide(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_hide(elo->evas_obj);
   lua_pushboolean(L, evas_object_visible_get(elo->evas_obj));
   return 1;
}

static int
_elua_show(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_show(elo->evas_obj);
   lua_pushboolean(L, evas_object_visible_get(elo->evas_obj));
   return 1;
}

static int
_elua_visible(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   n = lua_gettop(L);
   if (n == 2)
     {
        if (lua_isboolean(L, 2))
          {
             if (lua_toboolean(L, 2)) evas_object_show(elo->evas_obj);
             else evas_object_hide(elo->evas_obj);
          }
     }
   lua_pushboolean(L, evas_object_visible_get(elo->evas_obj));
   return 1;
}

//-------------
static int
_elua_above(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Edje_Lua_Evas_Object *elo2;
   Evas_Object *o;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   if (!(o = evas_object_above_get(elo->evas_obj))) return 0;
   if (!(elo2 = evas_object_data_get(o, ELO))) return 0;
   _elua_ref_get(L, elo2);
   return 1;
}

static int
_elua_below(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Edje_Lua_Evas_Object *elo2;
   Evas_Object *o;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   if (!(o = evas_object_below_get(elo->evas_obj))) return 0;
   if (!(elo2 = evas_object_data_get(o, ELO))) return 0;
   _elua_ref_get(L, elo2);
   return 1;
}

static int
_elua_bottom(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
//   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
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
             _elua_ref_get(L, elo2);
             return 1;
          }
     }
   return 0;
}

static int
_elua_lower(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_lower(elo->evas_obj);
   return 0;
}

static int
_elua_raise(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_raise(elo->evas_obj);
   return 0;
}

static int
_elua_top(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
 //  Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Edje_Lua_Evas_Object *elo2;
   Evas_Object *o;
   Eina_List *list, *l;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   if (!(list = (Eina_List *)evas_object_smart_members_get(obj->ed->obj))) return 0;
   if (!list) return 0;
   for (l = eina_list_last(list); l; l = l->prev)
     {
        o = l->data;
        if ((elo2 = evas_object_data_get(o, ELO)))
          {
             _elua_ref_get(L, elo2);
             return 1;
          }
     }
   return 0;
}

//-------------
static int
_elua_geom(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord ox, oy, ow, oh;
   int x, y, w, h;

   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_geometry_get(elo->evas_obj, &ox, &oy, &ow, &oh);
   if (_elua_scan_params(L, 2, EINA_TRUE, "%x %y %w %h", &x, &y, &w, &h) > 0)
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
   return 1;
}

static int
_elua_move(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord ox, oy;
   int x, y;

   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_geometry_get(elo->evas_obj, &ox, &oy, NULL, NULL);
   if (_elua_scan_params(L, 2, EINA_TRUE, "%x %y", &x, &y) > 0)
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
   return 1;
}

static int
_elua_pos(lua_State *L)
{
   return _elua_move(L);
}

static int
_elua_resize(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord ow, oh;
   int w, h;

   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_geometry_get(elo->evas_obj, NULL, NULL, &ow, &oh);
   if (_elua_scan_params(L, 2, EINA_TRUE, "%w %h", &w, &h) > 0)
     {
        if ((w != ow) || (h != oh))
          {
             evas_object_resize(elo->evas_obj, w, h);
             evas_object_geometry_get(elo->evas_obj, NULL, NULL, &ow, &oh);
          }
     }
   _elua_ret(L, "%w %h", ow, oh);
   return 1;
}

static int
_elua_size(lua_State *L)
{
   return _elua_resize(L);
}

//-------------
static int
_elua_clip(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo2, *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Object *o;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   n = lua_gettop(L);
   if (n == 2)
     {
        Edje_Lua_Obj *obj2 = (Edje_Lua_Obj *)lua_touserdata(L, 2);
        elo2 = (Edje_Lua_Evas_Object *)obj2;
        if (!_elua_isa(obj2, _elua_evas_meta)) return 0;
        evas_object_clip_set(elo->evas_obj, elo2->evas_obj);
     }
   o = evas_object_clip_get(elo->evas_obj);
   if (!o) return 0;
   if (!(elo2 = evas_object_data_get(o, ELO))) return 0;
   _elua_ref_get(L, elo2);
   return 1;
}

static int
_elua_clipees(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo2, *elo = (Edje_Lua_Evas_Object *)obj;
   Eina_List *list, *l;
   Evas_Object *o;
   int n = 0;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   list = (Eina_List *)evas_object_clipees_get(elo->evas_obj);
   lua_newtable(L);
   EINA_LIST_FOREACH(list, l, o)
     {
        if (!(elo2 = evas_object_data_get(o, ELO))) continue;
        lua_pushinteger(L, n + 1);
        _elua_ref_get(L, elo2);
        lua_settable(L, -3);
        n++;
     }
   return 1;
}

static int
_elua_unclip(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   evas_object_clip_unset(elo->evas_obj);
   return 0;
}

//-------------
static int
_elua_type(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   const char *t;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   t = evas_object_type_get(elo->evas_obj);
   if (!t) return 0;
   lua_pushstring(L, t);
   return 1;
}

//-------------
static int
_elua_pass(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   n = lua_gettop(L);
   if (n == 2)
     {
        if (lua_isboolean(L, 2))
          {
             evas_object_pass_events_set(elo->evas_obj, lua_toboolean(L, 2));
          }
     }
   lua_pushboolean(L, evas_object_pass_events_get(elo->evas_obj));
   return 1;
}

static int
_elua_precise(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   n = lua_gettop(L);
   if (n == 2)
     {
        evas_object_precise_is_inside_set(elo->evas_obj, lua_toboolean(L, 2));
     }
   lua_pushboolean(L, evas_object_precise_is_inside_get(elo->evas_obj));
   return 1;
}

static int
_elua_repeat(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   n = lua_gettop(L);
   if (n == 2)
     {
        if (lua_isboolean(L, 2))
          {
             evas_object_repeat_events_set(elo->evas_obj, lua_toboolean(L, 2));
          }
     }
   lua_pushboolean(L, evas_object_repeat_events_get(elo->evas_obj));
   return 1;
}

//-------------
static int
_elua_color(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int r, g, b, a;

   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   if (_elua_scan_params(L, 2, EINA_TRUE, "%r %g %b %a", &r, &g, &b, &a) > 0)
     {
        _elua_color_fix(&r, &g, &b, &a);
        evas_object_color_set(elo->evas_obj, r, g, b, a);
     }
   evas_object_color_get(elo->evas_obj, &r, &g, &b, &a);
   _elua_ret(L, "%r %g %b %a", r, g, b, a);
   return 1;
}

//-------------
static int
_elua_obj_map(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Edje_Lua_Obj *obj2 = (Edje_Lua_Obj *)lua_touserdata(L, 2);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj2;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;
   if (!_elua_isa(obj2, _elua_evas_map_meta)) return 0;

   evas_object_map_set(elo->evas_obj, elm->map);

   return 0;
}

static int
_elua_obj_map_enable(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!_elua_isa(obj, _elua_evas_meta)) return 0;

   n = lua_gettop(L);
   if (n == 2)
     {
        evas_object_map_enable_set(elo->evas_obj, lua_toboolean(L, 2));
     }
   lua_pushboolean(L, evas_object_map_enable_get(elo->evas_obj));
   return 1;
}

static int
_elua_obj_map_source(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Object *o;
   Edje_Lua_Evas_Object *elo2;
   int n;

   if (!_elua_isa(obj, _elua_evas_meta)) return 0;

   n = lua_gettop(L);
   if (n == 2)
     {
        Edje_Lua_Obj *obj2 = (Edje_Lua_Obj *)lua_touserdata(L, 2);
        const Edje_Lua_Evas_Object *source = (Edje_Lua_Evas_Object *)obj2;

        if (!_elua_isa(obj2, _elua_evas_meta)) return 0;
        evas_object_map_source_set(elo->evas_obj, source->evas_obj);
     }

   if (!(o = evas_object_map_source_get(elo->evas_obj))) return 0;
   if (!(elo2 = evas_object_data_get(o, ELO))) return 0;
    _elua_ref_get(L, elo2);

   return 1;
}

//-------------
//-------------

static int _elua_edje_file(lua_State *L);

static const char *_elua_evas_edje_api = "evas_edje";
static const char *_elua_evas_edje_parent = "evas_edje_parent";
static const struct luaL_reg _elua_evas_edje_funcs [] =
{
   // edje object specific
     {"file",         _elua_edje_file}, // get or set edje file and group

     {NULL, NULL} // end
};

static int
_elua_edje_file(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   const char *file = NULL, *group = NULL;
   int n = lua_gettop(L);

   if (!_elua_isa(obj, _elua_evas_edje_meta)) return 0;

   n = _elua_scan_params(L, 2, EINA_TRUE, "$file $group", &file, &group);
   if (0 >= n)
     {
        file = (char *) obj->ed->file->path;
        group = (char *) lua_tostring(L, 2);
        n = 2;
     }

   if (1 < n)
     {
        // Sandbox lua - Only allow access to groups within the same file.
        // By the simple expedient of completely ignoring what file was requested.
        file = (char *) obj->ed->file->path;
        if (!edje_object_file_set(elo->evas_obj, file, group))
          {
             Edje_Load_Error err = edje_object_load_error_get(elo->evas_obj);

             switch (err)
               {
                  case EDJE_LOAD_ERROR_NONE :                         LE("Edje file loading errer %s %s - no error happened, but you should not see this.\n", obj->ed->file->path, group);  break;
                  case EDJE_LOAD_ERROR_GENERIC :                      LE("Edje file loading errer %s %s - generic error.\n", obj->ed->file->path, group);  break;
                  case EDJE_LOAD_ERROR_DOES_NOT_EXIST :               LE("Edje file loading errer %s %s - file does not exist.\n", obj->ed->file->path, group);  break;
                  case EDJE_LOAD_ERROR_PERMISSION_DENIED :            LE("Edje file loading errer %s %s - permission denied reading the file.\n", obj->ed->file->path, group);  break;
                  case EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED :   LE("Edje file loading errer %s %s - resource allocation failed.\n", obj->ed->file->path, group);  break;
                  case EDJE_LOAD_ERROR_CORRUPT_FILE :                 LE("Edje file loading errer %s %s - corrupt file.\n", obj->ed->file->path, group);  break;
                  case EDJE_LOAD_ERROR_UNKNOWN_FORMAT :               LE("Edje file loading errer %s %s - unknown file format.\n", obj->ed->file->path, group);  break;
                  case EDJE_LOAD_ERROR_INCOMPATIBLE_FILE :            LE("Edje file loading errer %s %s - incompatible file.\n", obj->ed->file->path, group);  break;
                  case EDJE_LOAD_ERROR_UNKNOWN_COLLECTION :           LE("Edje file loading errer %s %s - unknown group.\n", obj->ed->file->path, group);  break;
                  case EDJE_LOAD_ERROR_RECURSIVE_REFERENCE :          LE("Edje file loading errer %s %s - recursive reference in group.\n", obj->ed->file->path, group);  break;
               }
          }
     }
   edje_object_file_get(elo->evas_obj, &file, &group);
   _elua_ret(L, "$file $group", file, group);
   return 1;
}

//-------------
//-------------

static int _elua_image_fill(lua_State *L);
static int _elua_image_filled(lua_State *L);
static int _elua_image_image(lua_State *L);

static const char *_elua_evas_image_api = "evas_image";
static const char *_elua_evas_image_parent = "evas_image_parent";
static const struct luaL_reg _elua_evas_image_funcs [] =
{
   // image object specific
     {"fill",         _elua_image_fill},   // get or set the fill parameters
     {"filled",       _elua_image_filled}, // get or set the filled state (overrides fill())
     {"image",        _elua_image_image},  // get or set image

     {NULL, NULL} // end
};

static int
_elua_image_fill(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord x, y, w, h;

   if (!_elua_isa(obj, _elua_evas_image_meta)) return 0;

   if (_elua_scan_params(L, 2, EINA_TRUE, "%x %y %w %h", &x, &y, &w, &h) > 0)
     {
        evas_object_image_fill_set(elo->evas_obj, x, y, w, h);
     }
   evas_object_image_fill_get(elo->evas_obj, &x, &y, &w, &h);
   _elua_ret(L, "%x %y %w %h", x, y, w, h);

   return 1;
}

static int
_elua_image_filled(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;

   if (!_elua_isa(obj, _elua_evas_image_meta)) return 0;

   n = lua_gettop(L);
   if (n == 2)
     {
        evas_object_image_filled_set(elo->evas_obj, lua_toboolean(L, 2));
     }
   lua_pushboolean(L, evas_object_image_filled_get(elo->evas_obj));
   return 1;
}

static int
_elua_image_image(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   const char *file = NULL, *key = NULL;
   int n, id = -1;

   if (!_elua_isa(obj, _elua_evas_image_meta)) return 0;

   n = _elua_scan_params(L, 2, EINA_TRUE, "$file $key", &file, &key);
   if (0 >= n)
     {
        file = (char *) obj->ed->file->path;
        key = (char *) lua_tostring(L, 2);
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

        /* Sandbox lua - Only allow access to images within the same edje file.  I'm not so sure we need this level of sandboxing though.  So leaving it here, just in case.
        if (-1 == id)
          {
             LI("Image %s not found in our edje file, trying external image file %s.\n", key, file);
             evas_object_image_file_set(elo->evas_obj, file, key);
          }
        */
     }
   evas_object_image_file_get(elo->evas_obj, &file, &key);
   _elua_ret(L, "$file $key", file, key);
   return 1;
}

//-------------
//-------------

static int _elua_line_xy(lua_State *L);

static const char *_elua_evas_line_api = "evas_line";
static const char *_elua_evas_line_parent = "evas_line_parent";
static const struct luaL_reg _elua_evas_line_funcs [] =
{
   // line object specific
     {"xy",         _elua_line_xy}, // get or set line coords

     {NULL, NULL} // end
};

static int _elua_line_xy(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord x1, y1, x2, y2;

   if (!_elua_isa(obj, _elua_evas_line_meta)) return 0;

   if (_elua_scan_params(L, 2, EINA_TRUE, "%x1 %y1 %x2 %y2", &x1, &y1, &x2, &y2) > 0)
     {
        evas_object_line_xy_set(elo->evas_obj, x1, y1, x2, y2);
     }
   evas_object_line_xy_get(elo->evas_obj, &x1, &y1, &x2, &y2);
   _elua_ret(L, "%x1 %y1 %x2 %y2", x1, y1, x2, y2);
   return 1;
}

//-------------
//-------------

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

static const char *_elua_evas_map_api = "ewas_map";
static const struct luaL_reg _elua_evas_map_funcs [] =
{
     {"alpha",         _elua_map_alpha},
//     {"dup",           _elua_map_dup},  // not sure of proper api for this.
     {"clockwise",     _elua_map_clockwise},
     {"color",         _elua_map_colour},
     {"coord",         _elua_map_coord},
     {"lighting",      _elua_map_lighting},
     {"perspective",   _elua_map_perspective},
     {"populate",      _elua_map_populate},
     {"rotate",        _elua_map_rotate},
     {"rotate3d",      _elua_map_rotate3d},
//     {"size",          _elua_map_size},  // not sure of proper API for this
     {"smooth",        _elua_map_smooth},
     {"uv",            _elua_map_uv},
     {"zoom",          _elua_map_zoom},

     {NULL, NULL} // end
};

static int
_elua_map_alpha(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   n = lua_gettop(L);
   if (n == 2)
     {
        evas_map_alpha_set(elm->map, lua_toboolean(L, 2));
     }
   lua_pushboolean(L, evas_map_alpha_get(elm->map));
   return 1;
}

static int
_elua_map_clockwise(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   lua_pushboolean(L, evas_map_util_clockwise_get(elm->map));
   return 1;
}

static int
_elua_map_colour(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   int r, g, b, a;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;
   n = lua_gettop(L);

   switch (n)
    {
       case 5 :
        {
           if (_elua_scan_params(L, 2, EINA_FALSE, "%r %g %b %a", &r, &g, &b, &a) > 0)
             {
                evas_map_util_points_color_set(elm->map, r, g, b, a);
             }
           break;
        }

       case 1 :
       case 6 :
        {
           if (_elua_scan_params(L, 3, EINA_TRUE, "%r %g %b %a", &r, &g, &b, &a) > 0)
             {
                evas_map_point_color_set(elm->map, lua_tointeger(L, 2), r, g, b, a);
             }
           evas_map_point_color_get(elm->map, lua_tointeger(L, 2), &r, &g, &b, &a);
           _elua_ret(L, "%r %g %b %a", r, g, b, a);
           return 1;
        }
    }

   return 0;
}

static int
_elua_map_coord(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   Evas_Coord x, y, z;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;
   n = lua_gettop(L);
   if (2 > n) return 0;

   if (_elua_scan_params(L, 2, EINA_TRUE, "%x %y %z", &x, &y, &z) > 0)
     {
        evas_map_point_coord_set(elm->map, lua_tointeger(L, 2), x, y, z);
     }
   evas_map_point_coord_get(elm->map, lua_tointeger(L, 2), &x, &y, &z);
   _elua_ret(L, "%x %y %z", x, y, z);
   return 1;
}

static int
_elua_map_lighting(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   Evas_Coord x, y, z;
   int r, g, b, r1, g1, b1;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   if ((n = _elua_scan_params(L, 2, EINA_FALSE, "%x %y %z", &x, &y, &z)) > 0)
     if (n += _elua_scan_params(L, 2 + n, EINA_FALSE, "%r %g %b", &r, &g, &b) > 0)
        if (_elua_scan_params(L, 2 + n, EINA_FALSE, "%r %g %b", &r1, &g1, &b1) > 0)
           {
              evas_map_util_3d_lighting(elm->map, x, y, z, r, g, b, r1, g1, b1);
           }
   return 0;
}

static int
_elua_map_perspective(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   Evas_Coord x, y, z, f;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   if (_elua_scan_params(L, 2, EINA_FALSE, "%x %y %z %f", &x, &y, &z, &f) > 0)
     {
        evas_map_util_3d_perspective(elm->map, x, y, z, f);
     }
   return 0;
}

static int
_elua_map_populate(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;
   n = lua_gettop(L);

   switch (n)
    {
       case 2 :
        {
           Edje_Lua_Obj *obj2 = (Edje_Lua_Obj *)lua_touserdata(L, 2);
           const Edje_Lua_Evas_Object *source = (Edje_Lua_Evas_Object *)obj2;

           if (!_elua_isa(obj2, _elua_evas_meta)) return 0;
           evas_map_util_points_populate_from_object(elm->map, source->evas_obj);
           break;
        }

       case 3 :
        {
           Edje_Lua_Obj *obj2 = (Edje_Lua_Obj *)lua_touserdata(L, 2);
           const Edje_Lua_Evas_Object *source = (Edje_Lua_Evas_Object *)obj2;
           Evas_Coord z = lua_tointeger(L, 3);

           if (!_elua_isa(obj2, _elua_evas_meta)) return 0;
           evas_map_util_points_populate_from_object_full(elm->map, source->evas_obj, z);
           break;
        }

       case 6 :
        {
           Evas_Coord x, y, w, h;

           if ((n = _elua_scan_params(L, 2, EINA_FALSE, "%x %y %w %h", &x, &y, &w, &h)) > 0)
             {
                evas_map_util_points_populate_from_geometry(elm->map, x, y, w, h, lua_tointeger(L, 2 + n));
             }
           break;
        }
    }
   return 0;
}

static int
_elua_map_rotate(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   double degrees;
   Evas_Coord x, y;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;
   n = lua_gettop(L);
   if (4 != n) return 0;

   degrees = lua_tonumber(L, 2);
   if (_elua_scan_params(L, 3, EINA_TRUE, "%x %y", &x, &y) > 0)
     {
        evas_map_util_rotate(elm->map, degrees, x, y);
     }
   return 0;
}

static int
_elua_map_rotate3d(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   double zx, zy, zz;
   Evas_Coord x, y, z;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   if ((n = _elua_scan_params(L, 2, EINA_FALSE, "#x #y #z", &zx, &zy, &zz)) > 0)
      if (_elua_scan_params(L, 2 + n, EINA_FALSE, "%x %y %z", &x, &y, &z) > 0)
        {
           evas_map_util_3d_rotate(elm->map, zx, zy, zz, x, y, z);
        }
   return 0;
}

static int
_elua_map_smooth(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   n = lua_gettop(L);
   if (n == 2)
     {
        evas_map_smooth_set(elm->map, lua_toboolean(L, 2));
     }
   lua_pushboolean(L, evas_map_smooth_get(elm->map));
   return 1;
}

static int
_elua_map_uv(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   double u, v;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;
   n = lua_gettop(L);
   if (2 > n) return 0;

   if (_elua_scan_params(L, 3, EINA_TRUE, "#u #v", &u, &v) > 0)
     {
        evas_map_point_image_uv_set(elm->map, lua_tonumber(L, 2), u, v);
     }
   evas_map_point_image_uv_get(elm->map, lua_tonumber(L, 2), &u, &v);
   _elua_ret(L, "#u #v", u, v);
   return 1;
}

static int
_elua_map_zoom(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Map *elm = (Edje_Lua_Map *)obj;
   double zx, zy;
   Evas_Coord x, y;
   int n;

   if (!_elua_isa(obj, _elua_evas_map_meta)) return 0;

   if ((n = _elua_scan_params(L, 2, EINA_FALSE, "#x #y", &zx, &zy)) > 0)
      if (_elua_scan_params(L, 2 + n, EINA_FALSE, "%x %y", &x, &y) > 0)
        {
           evas_map_util_zoom(elm->map, zx, zy, x, y);
        }
   return 0;
}

//-------------
//-------------

static int _elua_polygon_clear(lua_State *L);
static int _elua_polygon_point(lua_State *L);

static const char *_elua_evas_polygon_api = "evas_polygon";
static const char *_elua_evas_polygon_parent = "evas_polygon_parent";
static const struct luaL_reg _elua_evas_polygon_funcs [] =
{
   // polygon object specific
     {"clear",         _elua_polygon_clear}, // clear all polygon points
     {"point",         _elua_polygon_point}, // add a polygon point

     {NULL, NULL} // end
};

static int _elua_polygon_clear(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;

   if (!_elua_isa(obj, _elua_evas_polygon_meta)) return 0;
   evas_object_polygon_points_clear(elo->evas_obj);
   return 0;
}

static int _elua_polygon_point(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Coord x, y;

   if (!_elua_isa(obj, _elua_evas_polygon_meta)) return 0;

   if (_elua_scan_params(L, 2, EINA_FALSE, "%x %y", &x, &y) > 0)
     {
        evas_object_polygon_point_add(elo->evas_obj, x, y);
     }

   return 0;
}

//-------------
//-------------

static int _elua_text_font(lua_State *L);
static int _elua_text_text(lua_State *L);

static const char *_elua_evas_text_api = "evas_text";
static const char *_elua_evas_text_parent = "evas_text_parent";
static const struct luaL_reg _elua_evas_text_funcs [] =
{
   // text object specific
     {"font",         _elua_text_font}, // get or set text font
     {"text",         _elua_text_text}, // get or set text
//     {"text_class", _elua_object_text_class}, // get or set object text class

     {NULL, NULL} // end
};

static int
_elua_text_font(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   char *font, *font2 = NULL;
   Evas_Font_Size   size;
   int     inlined_font = 0;

   if (!_elua_isa(obj, _elua_evas_text_meta)) return 0;

   if (_elua_scan_params(L, 2, EINA_TRUE, "$font %size", &font, &size) > 0)
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
   evas_object_text_font_get(elo->evas_obj, (const char **) &font, &size);
   _elua_ret(L, "$font %size", font, size);
   return 1;
}

static int
_elua_text_text(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;

   if (!_elua_isa(obj, _elua_evas_text_meta)) return 0;
   n = lua_gettop(L);
   if (n == 2)
     {
        if (lua_isstring(L, 2))
          {
             const char *str;

             if ((str = lua_tostring(L, 2)))  // Extra parenthesis, coz Mikes compiler has a lisp.
                evas_object_text_text_set(elo->evas_obj, str);
          }
     }
   lua_pushstring(L, evas_object_text_text_get(elo->evas_obj));
   return 1;
}


//--------------------------------------------------------------------------//
// Brain dead inheritance thingy, built for speed.  Kinda.  Part 1.
static void
_elua_add_functions(lua_State *L, const char *api, const luaL_Reg *funcs, const char *meta, const char *parent, const char *base)
{
   luaL_register(L, api, funcs);
   luaL_newmetatable(L, meta);
   luaL_register(L, 0, _elua_edje_gc_funcs);
   lua_pushliteral(L, "__index");
   lua_pushvalue(L, -3);
   lua_rawset(L, -3);

   if (base && parent)
     {
        // Inherit from base
        lua_getglobal(L, base);
        luaL_newmetatable(L, parent);
        lua_pushliteral(L, "__index");
        lua_pushvalue(L, -3);
        lua_rawset(L, -3);
        lua_getglobal(L, api);
        luaL_getmetatable(L, parent);
        lua_setmetatable(L, -2);
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

static void
_elua_init(void)
{
   static Edje_Lua_Alloc ela = { MAX_LUA_MEM, 0 };
   const luaL_Reg *l;
   lua_State *L;

   if (lstate) return;

   lstate = L = lua_newstate(_elua_alloc, &ela);
   lua_atpanic(L, _elua_custom_panic);

// FIXME: figure out optimal gc settings later
//   lua_gc(L, LUA_GCSETPAUSE, 200);
//   lua_gc(L, LUA_GCSETSTEPMUL, 200);

   for (l = _elua_libs; l->func; l++)
     {
        lua_pushcfunction(L, l->func);
        lua_pushstring(L, l->name);
        lua_call(L, 1, 0);
     }

   luaL_register(L, _elua_edje_api, _elua_edje_funcs);
   luaL_newmetatable(L, _elua_edje_meta);
   luaL_register(L, 0, _elua_edje_gc_funcs);

   _elua_add_functions(L, _elua_evas_api, _elua_evas_funcs, _elua_evas_meta, NULL, NULL);

   // weak table for our objects
   lua_pushlightuserdata(L, &_elua_objs);
   lua_newtable(L);
   lua_pushstring(L, "__mode");
   lua_pushstring(L, "v");
   lua_rawset(L, -3);
   lua_rawset(L, LUA_REGISTRYINDEX);
}

void
_edje_lua2_script_init(Edje *ed)
{
   static Edje_Lua_Alloc ela = { MAX_LUA_MEM, 0 };
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
        // FIXME: Change this to WARN before release.
        eina_log_domain_level_set("lua", EINA_LOG_LEVEL_INFO);
     }

   _elua_init();
   L = ed->L = lua_newstate(_elua_alloc, &ela);
   lua_atpanic(L, _elua_custom_panic);

// FIXME: figure out optimal gc settings later
//   lua_gc(L, LUA_GCSETPAUSE, 200);
//   lua_gc(L, LUA_GCSETSTEPMUL, 200);

   for (l = _elua_libs; l->func; l++)
     {
        lua_pushcfunction(L, l->func);
        lua_pushstring(L, l->name);
        lua_call(L, 1, 0);
     }

   luaL_register(L, _elua_edje_api, _elua_edje_funcs);
   luaL_newmetatable(L, _elua_edje_meta);
   luaL_register(L, 0, _elua_edje_gc_funcs);

   lua_pop(L, 2);

   _elua_add_functions(L, _elua_evas_api, _elua_evas_funcs, _elua_evas_meta, NULL, NULL);
   _elua_add_functions(L, _elua_evas_edje_api, _elua_evas_edje_funcs, _elua_evas_edje_meta, _elua_evas_edje_parent, _elua_evas_api);
   _elua_add_functions(L, _elua_evas_image_api, _elua_evas_image_funcs, _elua_evas_image_meta, _elua_evas_image_parent, _elua_evas_api);
   _elua_add_functions(L, _elua_evas_line_api, _elua_evas_line_funcs, _elua_evas_line_meta, _elua_evas_line_parent, _elua_evas_api);
   _elua_add_functions(L, _elua_evas_map_api, _elua_evas_map_funcs, _elua_evas_map_meta, NULL, NULL);
   _elua_add_functions(L, _elua_evas_polygon_api, _elua_evas_polygon_funcs, _elua_evas_polygon_meta, _elua_evas_polygon_parent, _elua_evas_api);
   _elua_add_functions(L, _elua_evas_text_api, _elua_evas_text_funcs, _elua_evas_text_meta, _elua_evas_text_parent, _elua_evas_api);

   // weak table for our objects
   lua_pushlightuserdata(L, &_elua_objs);
   lua_newtable(L);
   lua_pushstring(L, "__mode");
   lua_pushstring(L, "v");
   lua_rawset(L, -3);
   lua_rawset(L, LUA_REGISTRYINDEX);

   _elua_table_ptr_set(L, _elua_key, ed);

   snprintf(buf, sizeof(buf), "edje/scripts/lua/%i", ed->collection->id);
   data = eet_read(ed->file->ef, buf, &size);

   if (data)
     {
        int err;

        err = luaL_loadbuffer(L, data, size, "edje_lua_script");
        if (err)
          {
             if (err == LUA_ERRSYNTAX)
               ERR("Lua load syntax error: %s",
                   lua_tostring(L, -1));
             else if (err == LUA_ERRMEM)
               ERR("Lua load memory allocation error: %s",
                   lua_tostring(L, -1));
          }
        free(data);
        if (setjmp(panic_jmp) == 1)
          {
             ERR("Lua script init panic");
             return;
          }
        if ((err = lua_pcall(L, 0, 0, 0)))
          _edje_lua2_error(L, err);
     }
}

void
_edje_lua2_script_shutdown(Edje *ed)
{
   if (!ed->L) return;
   lua_close(ed->L);
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
_edje_lua2_script_load(Edje_Part_Collection *edc __UNUSED__, void *data __UNUSED__, int size __UNUSED__)
{
   _elua_init();
}

void
_edje_lua2_script_unload(Edje_Part_Collection *edc __UNUSED__)
{
   lua_State *L;

   if (!lstate) return;
   L = lstate;
   lua_gc(L, LUA_GCCOLLECT, 0);
}

void
_edje_lua2_error_full(const char *file, const char *fnc, int line,
                      lua_State *L, int err_code)
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
     (_edje_default_log_dom, EINA_LOG_LEVEL_ERR,  file, fnc, line,
      "Lua %s error: %s", err_type, lua_tostring(L, -1));
}

void
_edje_lua2_script_func_shutdown(Edje *ed)
{
   int err;

   lua_getglobal(ed->L, "shutdown");
   if (!lua_isnil(ed->L, -1))
     {
        if ((err = lua_pcall(ed->L, 0, 0, 0)))
          _edje_lua2_error(ed->L, err);
     }
   else
     lua_pop(ed->L, 1);
   _edje_lua2_script_shutdown(ed);
}

void
_edje_lua2_script_func_show(Edje *ed)
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

void
_edje_lua2_script_func_hide(Edje *ed)
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

void
_edje_lua2_script_func_move(Edje *ed)
{
   int err;

   // FIXME: move all objects created by script
   lua_getglobal(ed->L, "move");
   if (!lua_isnil(ed->L, -1))
     {
        lua_pushinteger(ed->L, ed->x);
        lua_pushinteger(ed->L, ed->y);
        if ((err = lua_pcall(ed->L, 2, 0, 0)))
          _edje_lua2_error(ed->L, err);
     }
   else
     lua_pop(ed->L, 1);
}

void
_edje_lua2_script_func_resize(Edje *ed)
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

void
_edje_lua2_script_func_message(Edje *ed, Edje_Message *em)
{
   int err, n, c, i;

   lua_getglobal(ed->L, "message");
   if (!lua_isnil(ed->L, -1))
     {
        n = 2;
        lua_pushinteger(ed->L, em->id);
        switch (em->type)
          {
          case EDJE_MESSAGE_NONE:
             lua_pushstring(ed->L, "none");
             break;
          case EDJE_MESSAGE_SIGNAL:
             break;
          case EDJE_MESSAGE_STRING:
             lua_pushstring(ed->L, "str");
             lua_pushstring(ed->L, ((Edje_Message_String *)em->msg)->str);
             n += 1;
            break;
          case EDJE_MESSAGE_INT:
             lua_pushstring(ed->L, "int");
             lua_pushinteger(ed->L, ((Edje_Message_Int *)em->msg)->val);
             n += 1;
             break;
          case EDJE_MESSAGE_FLOAT:
             lua_pushstring(ed->L, "float");
             lua_pushnumber(ed->L, ((Edje_Message_Float *)em->msg)->val);
             n += 1;
             break;
          case EDJE_MESSAGE_STRING_SET:
             lua_pushstring(ed->L, "strset");
             c = ((Edje_Message_String_Set *)em->msg)->count;
             lua_createtable(ed->L, c, 0);
             for (i = 0; i < c; i++)
               {
                  lua_pushstring(ed->L, ((Edje_Message_String_Set *)em->msg)->str[i]);
                  lua_rawseti(ed->L, -2, i + 1);
               }
             n += 1;
             break;
          case EDJE_MESSAGE_INT_SET:
             lua_pushstring(ed->L, "intset");
             c = ((Edje_Message_Int_Set *)em->msg)->count;
             lua_createtable(ed->L, c, 0);
             for (i = 0; i < c; i++)
               {
                  lua_pushinteger(ed->L, ((Edje_Message_Int_Set *)em->msg)->val[i]);
                  lua_rawseti(ed->L, -2, i + 1);
               }
             n += 1;
             break;
          case EDJE_MESSAGE_FLOAT_SET:
             lua_pushstring(ed->L, "floatset");
             c = ((Edje_Message_Float_Set *)em->msg)->count;
             lua_createtable(ed->L, c, 0);
             for (i = 0; i < c; i++)
               {
                  lua_pushnumber(ed->L, ((Edje_Message_Float_Set *)em->msg)->val[i]);
                  lua_rawseti(ed->L, -2, i + 1);
               }
             n += 1;
             break;
          case EDJE_MESSAGE_STRING_INT:
             lua_pushstring(ed->L, "strint");
             lua_pushstring(ed->L, ((Edje_Message_String_Int *)em->msg)->str);
             lua_pushinteger(ed->L, ((Edje_Message_String_Int *)em->msg)->val);
             n += 2;
             break;
          case EDJE_MESSAGE_STRING_FLOAT:
             lua_pushstring(ed->L, "strfloat");
             lua_pushstring(ed->L, ((Edje_Message_String_Float *)em->msg)->str);
             lua_pushnumber(ed->L, ((Edje_Message_String_Float *)em->msg)->val);
             n += 2;
             break;
          case EDJE_MESSAGE_STRING_INT_SET:
             lua_pushstring(ed->L, "strintset");
             lua_pushstring(ed->L, ((Edje_Message_String_Int_Set *)em->msg)->str);
             c = ((Edje_Message_String_Int_Set *)em->msg)->count;
             lua_createtable(ed->L, c, 0);
             for (i = 0; i < c; i++)
               {
                  lua_pushinteger(ed->L, ((Edje_Message_String_Int_Set *)em->msg)->val[i]);
                  lua_rawseti(ed->L, -2, i + 1);
               }
             n += 2;
             break;
          case EDJE_MESSAGE_STRING_FLOAT_SET:
             lua_pushstring(ed->L, "strfloatset");
             lua_pushstring(ed->L, ((Edje_Message_String_Float_Set *)em->msg)->str);
             c = ((Edje_Message_String_Float_Set *)em->msg)->count;
             lua_createtable(ed->L, c, 0);
             for (i = 0; i < c; i++)
               {
                  lua_pushnumber(ed->L, ((Edje_Message_String_Float_Set *)em->msg)->val[i]);
                  lua_rawseti(ed->L, -2, i + 1);
               }
             n += 2;
             break;
          default:
             break;
          }
        if ((err = lua_pcall(ed->L, n, 0, 0)))
          _edje_lua2_error(ed->L, err);
     }
   else
     lua_pop(ed->L, 1);
}

void
_edje_lua2_script_func_signal(Edje *ed, const char *sig, const char *src)
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
