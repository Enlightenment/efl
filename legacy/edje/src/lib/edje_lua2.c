#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
  void *alloca(size_t);
#endif

#include "edje_private.h"

//--------------------------------------------------------------------------//
#ifdef LUA2
#define MAX_LUA_MEM (4 * (1024 * 1024))


//--------------------------------------------------------------------------//
typedef struct _Edje_Lua_Alloc       Edje_Lua_Alloc;
typedef struct _Edje_Lua_Obj         Edje_Lua_Obj;
typedef struct _Edje_Lua_Timer       Edje_Lua_Timer;
typedef struct _Edje_Lua_Animator    Edje_Lua_Animator;
typedef struct _Edje_Lua_Transition  Edje_Lua_Transition;
typedef struct _Edje_Lua_Evas_Object Edje_Lua_Evas_Object;

//--------------------------------------------------------------------------//
struct _Edje_Lua_Alloc
{
   size_t max, cur;
};

struct _Edje_Lua_Obj
{
   EINA_INLIST;
   
   Edje         *ed;
   void        (*free_func) (void *obj);
   Eina_Bool    is_evas_obj : 1;
};
  
struct _Edje_Lua_Timer
{
   Edje_Lua_Obj     obj;
   Ecore_Timer     *timer;
   int              fn_ref;
};

struct _Edje_Lua_Animator
{
   Edje_Lua_Obj     obj;
   Ecore_Animator  *animator;
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
   int              x, y, w, h;
};


//--------------------------------------------------------------------------//
static int _elua_obj_gc(lua_State *L);
static int _elua_obj_del(lua_State *L);

static int _elua_echo(lua_State *L);

static int _elua_timer(lua_State *L);
static int _elua_animator(lua_State *L);
static int _elua_transition(lua_State *L);

static int _elua_seconds(lua_State *L);
static int _elua_looptime(lua_State *L);
static int _elua_date(lua_State *L);

static int _elua_emit(lua_State *L);
static int _elua_messagesend(lua_State *L);

static int _elua_objpos(lua_State *L);
static int _elua_objsize(lua_State *L);
static int _elua_objgeom(lua_State *L);

static int _elua_show(lua_State *L);
static int _elua_hide(lua_State *L);
static int _elua_visible(lua_State *L);
static int _elua_move(lua_State *L);
static int _elua_resize(lua_State *L);
static int _elua_pos(lua_State *L);
static int _elua_size(lua_State *L);
static int _elua_geom(lua_State *L);
static int _elua_raise(lua_State *L);
static int _elua_lower(lua_State *L);
static int _elua_above(lua_State *L);
static int _elua_below(lua_State *L);
static int _elua_top(lua_State *L);
static int _elua_bottom(lua_State *L);
static int _elua_aboveget(lua_State *L);
static int _elua_belowget(lua_State *L);
static int _elua_color(lua_State *L);
static int _elua_clip(lua_State *L);
static int _elua_unclip(lua_State *L);
static int _elua_clipees(lua_State *L);
static int _elua_type(lua_State *L);
static int _elua_pass(lua_State *L);
static int _elua_repeat(lua_State *L);

static int _elua_rect(lua_State *L);

//--------------------------------------------------------------------------//
static lua_State *lstate = NULL;
static jmp_buf panic_jmp;

static const struct luaL_reg _elua_edje_api [] =
{
   // add an echo too to make it more shelly
     {"echo",         _elua_echo}, // test func - echo (i know we have print. test)
   
   // generic object methods
     {"del",          _elua_obj_del}, // generic del any object created for edje (evas objects, timers, animators, transitions... everything)
   
   // time based "callback" systems
     {"timer",        _elua_timer}, // add timer
     {"animator",     _elua_animator}, // add animator
     {"transition",   _elua_transition}, // add transition
   
   // system information (time, date blah blah)
     {"seconds",      _elua_seconds}, // get seconds
     {"looptime",     _elua_looptime}, // get loop time
     {"date",         _elua_date}, // get date in a table

   // talk to application/caller
     {"emit",         _elua_emit}, // emit signal + src
     {"messagesend",  _elua_messagesend}, // send a structured message

   // query edje - size, pos
     {"objpos",       _elua_objpos}, // get while edje object pos in canvas
     {"objsize",      _elua_objsize}, // get while edje object pos in canvas
     {"objgeom",      _elua_objgeom}, // get while edje object geometry in canvas
   
   // FIXME: query color classes
   // FIXME: query text classes
   
   // now evas stuff (manipulate, delete etc.)
     {"show",         _elua_show}, // show, return current visibility
     {"hide",         _elua_hide}, // hide, return current visibility
     {"visible",      _elua_visible}, // get object visibility
     {"move",         _elua_move}, // move, return current position
     {"resize",       _elua_resize}, // resize, return current size
     {"posget",       _elua_pos}, // move, return current position
     {"sizeget",      _elua_size}, // resize, return current size
     {"geom",         _elua_geom}, // move and resize and return current geometry
     {"raise",        _elua_raise}, // raise
     {"lower",        _elua_lower}, // lower
     {"above",        _elua_above}, // stack above
     {"below",        _elua_below}, // stack below
     {"topget",       _elua_top}, // get top
     {"bottomget",    _elua_bottom}, // get bottom
     {"aboveget",     _elua_aboveget}, // get object above
     {"belowget",     _elua_belowget}, // get object below
     {"color",        _elua_color}, // set color, return color
     {"clip",         _elua_clip}, // set clip obj, return clip object
     {"unclip",       _elua_unclip}, // clear clip obj
     {"clipees",      _elua_clipees}, // get clip children
     {"type",         _elua_type}, // get object type
     {"pass",         _elua_pass}, // set pass events, get pass events
     {"repeat",       _elua_repeat}, // set repeat events, get repeat events
   // FIXME: set scale (explicit value)
   // FIXME: need to set auto-scale (same as scale: 1)
   // FIXME: set precise inside
   // FIXME: set callbacks (mouse down, up, blah blah blah)
   
   // FIXME: map api here
   
     {"rect",         _elua_rect}, // new rect
   // FIXME: need image(filled, normal), text, textblock, edje
   
   // FIXME: methods lua scrupt can provide that edje will call (not done yet):
   // // scale set
   // // key down
   // // key up
   // // get dragable pos
   // // set dragable pos
   // // get part text
   // // set part text
   // // get swallow part
   // // set swallow part
   // // textclass change
   // // colorclass change

     {NULL, NULL} // end
};
static const struct luaL_reg _elua_edje_meta [] =
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
     {LUA_OSLIBNAME, luaopen_os}, // FIXME: audit os lib - maybe not provide or only provide specific calls
     {LUA_STRLIBNAME, luaopen_string},
     {LUA_MATHLIBNAME, luaopen_math},
//     {LUA_DBLIBNAME, luaopen_debug}, // disable this lib - don't want

     {NULL, NULL} // end
};
static const char *_elua_key = "key";

//--------------------------------------------------------------------------//
static void *
_elua_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
   Edje_Lua_Alloc *ela = ud;
   void *ptr2;
   
   ela->cur += nsize - osize;
   if (ela->cur > ela->max)
     {
        ERR("Edje Lua memory limit of %zu bytes reached (%zu allocated)",
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
   ERR("Edje Lua cannot re-allocate %i bytes", nsize);
   return ptr2;
}

static int
_elua_custom_panic(lua_State *L)
{
   ERR("Lua Panic!!!!");
   return 1;
}


//-------------
void
_edje_lua2_error_full(const char *file, const char *fnc, int line,
                      lua_State *L, int err_code)
{
   char *err_type;
   
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
   
   luaL_register(L, "edje", _elua_edje_api);
   luaL_newmetatable(L, "edje");
   luaL_register(L, 0, _elua_edje_meta);
   
   lua_pushliteral(L, "__index");
   lua_pushvalue(L, -3);
   lua_rawset(L, -3);
   lua_pop(L, 2);
}


//-------------
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

static void
_elua_table_ptr_del(lua_State *L, const void *key)
{
   lua_pushlightuserdata(L, (void *)key);
   lua_pushnil(L);
   lua_settable(L, LUA_REGISTRYINDEX);
}

static void
_elua_gc(lua_State *L)
{
   lua_gc(L, LUA_GCCOLLECT, 0);
}

//-------------
static Edje_Lua_Obj *
_elua_obj_new(lua_State *L, Edje *ed, int size)
{
   Edje_Lua_Obj *obj;
   
   obj = (Edje_Lua_Obj *)lua_newuserdata(L, size);
   memset(obj, 0, size);
   ed->lua_objs = eina_inlist_append(ed->lua_objs, EINA_INLIST_GET(obj));
   luaL_getmetatable(L, "edje");
   lua_setmetatable(L, -2);
   obj->ed = ed;
   return obj;
}

static void
_elua_obj_free(lua_State *L, Edje_Lua_Obj *obj)
{
   if (!obj->free_func) return;
   obj->free_func(obj);
   obj->ed->lua_objs = eina_inlist_remove(obj->ed->lua_objs, EINA_INLIST_GET(obj));
   obj->free_func = NULL;
   obj->ed = NULL;
}

//-------------
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

//----------------------------------------------------------------------------
//-------------------------------------------
//---------------------------
//-------------------
//---------------
//-------------

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

//-------------
//-------------
//-------------
static int
_elua_echo(lua_State *L)
{
   const char *string = luaL_checkstring(L, 1);
   printf("%s\n", string);
   return 0;
}

//-------------
static Eina_Bool
_elua_timer_cb(void *data)
{
   Edje_Lua_Timer *elt = data;
   lua_State *L;
   int ret = 0;
   int err;
   
   if (!elt->obj.ed) return 0;
   L = elt->obj.ed->L;
   if (!L) return 0;
   lua_rawgeti(L, LUA_REGISTRYINDEX, elt->fn_ref);
   if (setjmp(panic_jmp) == 1)
     {
        ERR("Timer callback panic");
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
//   ret = luaL_checktype(L, -1, LUA_TBOOLEAN);
//   ret = luaL_checkint(L, -1);
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
  
   elt = (Edje_Lua_Timer *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Timer));
   elt->obj.free_func = _elua_timer_free;
   elt->timer = ecore_timer_add(val, _elua_timer_cb, elt);
   lua_pushvalue(L, 2);
   elt->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
   _elua_gc(L);
   return 1;
}

//-------------
static Eina_Bool
_elua_animator_cb(void *data)
{
   Edje_Lua_Animator *ela = data;
   lua_State *L;
   int ret = 0;
   int err;
   
   if (!ela->obj.ed) return 0;
   L = ela->obj.ed->L;
   if (!L) return 0;
   lua_rawgeti(L, LUA_REGISTRYINDEX, ela->fn_ref);
   if (setjmp(panic_jmp) == 1)
     {
        ERR("Animator callback panic");
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
   luaL_unref(L, LUA_REGISTRYINDEX, ela->fn_ref); //0
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
  
   ela = (Edje_Lua_Animator *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Animator));
   ela->obj.free_func = _elua_animator_free;
   ela->animator = ecore_animator_add(_elua_animator_cb, ela);
   lua_pushvalue(L, 1);
   ela->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
   _elua_gc(L);
   return 1;
}

//-------------
static Eina_Bool
_elua_transition_cb(void *data)
{
   Edje_Lua_Transition *elt = data;
   lua_State *L;
   int ret = 0;
   int err;
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
        ERR("Transition callback panic");
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
  
   elt = (Edje_Lua_Transition *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Transition));
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
_elua_seconds(lua_State *L)
{
   double t = ecore_time_get();
   lua_pushnumber(L, t);
   return 1;
}

//-------------
static int
_elua_looptime(lua_State *L)
{
   double t = ecore_loop_time_get();
   lua_pushnumber(L, t);
   return 1;
}

//-------------
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
        double t;

        lua_pushstring(L, "year");
        lua_pushinteger(L, (int)(tm->tm_year + 1900));
        lua_settable(L, -3);
        lua_pushstring(L, "month");
        lua_pushinteger(L, (int)(tm->tm_mon + 1));
        lua_settable(L, -3);
        lua_pushstring(L, "day");
        lua_pushinteger(L, (int)(tm->tm_mday));
        lua_settable(L, -3);
        lua_pushstring(L, "yearday");
        lua_pushinteger(L, (int)(tm->tm_yday));
        lua_settable(L, -3);
        lua_pushstring(L, "weekday");
        lua_pushinteger(L, (int)((tm->tm_wday + 6) % 7));
        lua_settable(L, -3);
        lua_pushstring(L, "hour");
        lua_pushinteger(L, (int)(tm->tm_hour));
        lua_settable(L, -3);
        lua_pushstring(L, "min");
        lua_pushinteger(L, (int)(tm->tm_min));
        lua_settable(L, -3);
        t = (double)tm->tm_sec + (((double)timev.tv_usec) / 1000000);
        lua_pushstring(L, "sec");
        lua_pushnumber(L, t);
        lua_settable(L, -3);
     }
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

//-------------
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
        int i, n, len;
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
static int
_elua_objpos(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   if (!lua_istable(L, 1)) lua_newtable(L);
   lua_pushstring(L, "x");
   lua_pushinteger(L, ed->x);
   lua_settable(L, -3);
   lua_pushstring(L, "y");
   lua_pushinteger(L, ed->y);
   lua_settable(L, -3);
   return 1;
}

static int
_elua_objsize(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   if (!lua_istable(L, 1)) lua_newtable(L);
   lua_pushstring(L, "w");
   lua_pushinteger(L, ed->w);
   lua_settable(L, -3);
   lua_pushstring(L, "h");
   lua_pushinteger(L, ed->h);
   lua_settable(L, -3);
   return 1;
}

static int
_elua_objgeom(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   if (!lua_istable(L, 1)) lua_newtable(L);
   lua_pushstring(L, "x");
   lua_pushinteger(L, ed->x);
   lua_settable(L, -3);
   lua_pushstring(L, "y");
   lua_pushinteger(L, ed->y);
   lua_settable(L, -3);
   lua_pushstring(L, "w");
   lua_pushinteger(L, ed->w);
   lua_settable(L, -3);
   lua_pushstring(L, "h");
   lua_pushinteger(L, ed->h);
   lua_settable(L, -3);
   return 1;
}

//-------------
//-------------
static int
_elua_2_int_get(lua_State *L, int i, Eina_Bool tr,
                const char *n1, int *v1,
                const char *n2, int *v2
               )
{
   int n = 0;

   if (lua_istable(L, i))
     {
        lua_getfield(L, i, n1);
        if (lua_isnil(L, -1))
          {
             lua_pop(L, 1);
             lua_rawgeti(L, i, 1);
             lua_rawgeti(L, i, 2);
          }
        else
          lua_getfield(L, i, n2);
        if ((!lua_isnil(L, -1)) && (!lua_isnil(L, -2)))
          {
             *v1 = lua_tointeger(L, -2);
             *v2 = lua_tointeger(L, -1);
             n = 1;
          }
        if (tr) lua_settop(L, i);
     }
   else
     {
        if ((lua_isnumber(L, i + 0)) && (lua_isnumber(L, i + 1)))
          {
             *v1 = lua_tointeger(L, i + 0);
             *v2 = lua_tointeger(L, i + 1);
             n = 2;
          }
        if (tr) lua_newtable(L);
     }
   return n;
}

static int
_elua_3_int_get(lua_State *L, int i, Eina_Bool tr,
                const char *n1, int *v1,
                const char *n2, int *v2,
                const char *n3, int *v3
               )
{
   int n = 0;

   if (lua_istable(L, i))
     {
        lua_getfield(L, i, n1);
        if (lua_isnil(L, -1))
          {
             lua_pop(L, 1);
             lua_rawgeti(L, i, 1);
             lua_rawgeti(L, i, 2);
             lua_rawgeti(L, i, 3);
          }
        else
          {
             lua_getfield(L, i, n2);
             lua_getfield(L, i, n3);
          }
        if ((!lua_isnil(L, -1)) && (!lua_isnil(L, -2)) && 
            (!lua_isnil(L, -3)))
          {
             *v1 = lua_tointeger(L, -3);
             *v2 = lua_tointeger(L, -2);
             *v3 = lua_tointeger(L, -1);
             n = 1;
          }
        if (tr) lua_settop(L, i);
     }
   else
     {
        if ((lua_isnumber(L, i + 0)) && (lua_isnumber(L, i + 1)) &&
            (lua_isnumber(L, i + 2)))
          {
             *v1 = lua_tointeger(L, i + 0);
             *v2 = lua_tointeger(L, i + 1);
             *v3 = lua_tointeger(L, i + 2);
             n = 3;
          }
        if (tr) lua_newtable(L);
     }
   return n;
}

static int
_elua_4_int_get(lua_State *L, int i, Eina_Bool tr,
                const char *n1, int *v1,
                const char *n2, int *v2,
                const char *n3, int *v3,
                const char *n4, int *v4
               )
{
   int n = 0;

   if (lua_istable(L, i))
     {
        lua_getfield(L, i, n1);
        if (lua_isnil(L, -1))
          {
             lua_pop(L, 1);
             lua_rawgeti(L, i, 1);
             lua_rawgeti(L, i, 2);
             lua_rawgeti(L, i, 3);
             lua_rawgeti(L, i, 4);
          }
        else
          {
             lua_getfield(L, i, n2);
             lua_getfield(L, i, n3);
             lua_getfield(L, i, n4);
          }
        if ((!lua_isnil(L, -1)) && (!lua_isnil(L, -2)) && 
            (!lua_isnil(L, -3)) && (!lua_isnil(L, -4)))
          {
             *v1 = lua_tointeger(L, -4);
             *v2 = lua_tointeger(L, -3);
             *v3 = lua_tointeger(L, -2);
             *v4 = lua_tointeger(L, -1);
             n = 1;
          }
        if (tr) lua_settop(L, i);
     }
   else
     {
        if ((lua_isnumber(L, i + 0)) && (lua_isnumber(L, i + 1)) &&
            (lua_isnumber(L, i + 2)) && (lua_isnumber(L, i + 3)))
          {
             *v1 = lua_tointeger(L, i + 0);
             *v2 = lua_tointeger(L, i + 1);
             *v3 = lua_tointeger(L, i + 2);
             *v4 = lua_tointeger(L, i + 3);
             n = 4;
          }
        if (tr) lua_newtable(L);
     }
   return n;
}

static void
_elua_int_ret(lua_State *L, const char *n, int v)
{
   lua_pushstring(L, n);
   lua_pushinteger(L, v);
   lua_settable(L, -3);
}

static void
_elua_color_fix(int *r, int *g, int *b, int *a)
{
   if (*r > *a) *r = *a;
   if (*g > *a) *g = *a;
   if (*b > *a) *b = *a;
}

//-------------
//-------------

static int
_elua_show(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   evas_object_show(elo->evas_obj);
   lua_pushboolean(L, evas_object_visible_get(elo->evas_obj));
   return 1;
}

static int
_elua_hide(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   evas_object_hide(elo->evas_obj);
   lua_pushboolean(L, evas_object_visible_get(elo->evas_obj));
   return 1;
}

static int
_elua_visible(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
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

static int
_elua_move(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n, x, y;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   if (_elua_2_int_get(L, 2, EINA_TRUE, "x", &x, "y", &y) > 0)
     {
        if ((x != elo->x) || (y != elo->y))
          {
             elo->x = x;
             elo->y = y;
             evas_object_move(elo->evas_obj, 
                              obj->ed->x + elo->x, 
                              obj->ed->y + elo->y);
          }
     }
   _elua_int_ret(L, "x", elo->x);
   _elua_int_ret(L, "y", elo->y);
   return 1;
}

static int
_elua_resize(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n, w, h;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   if (_elua_2_int_get(L, 2, EINA_TRUE, "w", &w, "h", &h) > 0)
     {
        if ((w != elo->w) || (h != elo->h))
          {
             elo->w = w;
             elo->h = h;
             evas_object_resize(elo->evas_obj, elo->w, elo->h);
          }
     }
   _elua_int_ret(L, "w", elo->w);
   _elua_int_ret(L, "h", elo->h);
   return 1;
}

static int
_elua_pos(lua_State *L)
{
   return _elua_move(L);
}

static int
_elua_size(lua_State *L)
{
   return _elua_resize(L);
}

static int
_elua_geom(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n, x, y, w, h;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   if (_elua_4_int_get(L, 2, EINA_TRUE, "x", &x, "y", &y, "w", &w, "h", &h) > 0)
     {
        if ((x != elo->x) || (y != elo->y))
          {
             elo->x = x;
             elo->y = y;
             evas_object_move(elo->evas_obj, 
                              obj->ed->x + elo->x, 
                              obj->ed->y + elo->y);
          }
        if ((w != elo->w) || (h != elo->h))
          {
             elo->w = w;
             elo->h = h;
             evas_object_resize(elo->evas_obj, elo->w, elo->h);
          }
     }
   _elua_int_ret(L, "x", elo->x);
   _elua_int_ret(L, "y", elo->y);
   _elua_int_ret(L, "w", elo->w);
   _elua_int_ret(L, "h", elo->h);
   return 1;
}

static int
_elua_raise(lua_State *L)
{
   return 0;
}

static int
_elua_lower(lua_State *L)
{
   return 0;
}

static int
_elua_above(lua_State *L)
{
   return 0;
}

static int
_elua_below(lua_State *L)
{
   return 0;
}

static int
_elua_top(lua_State *L)
{
   return 0;
}

static int
_elua_bottom(lua_State *L)
{
   return 0;
}

static int
_elua_aboveget(lua_State *L)
{
   return 0;
}

static int
_elua_belowget(lua_State *L)
{
   return 0;
}

static int
_elua_color(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n, r, g, b, a;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   if (_elua_4_int_get(L, 2, EINA_TRUE, "r", &r, "g", &g, "b", &b, "a", &a) > 0)
     {
        _elua_color_fix(&r, &g, &b, &a);
        evas_object_color_set(elo->evas_obj, r, g, b, a);
     }
   evas_object_color_get(elo->evas_obj, &r, &g, &b, &a);
   _elua_int_ret(L, "r", r);
   _elua_int_ret(L, "g", g);
   _elua_int_ret(L, "b", b);
   _elua_int_ret(L, "a", a);
   return 1;
}

static int
_elua_clip(lua_State *L)
{
   return 0;
}

static int
_elua_unclip(lua_State *L)
{
   return 0;
}

static int
_elua_clipees(lua_State *L)
{
   return 0;
}

static int
_elua_type(lua_State *L)
{
   return 0;
}

static int
_elua_pass(lua_State *L)
{
   return 0;
}

static int
_elua_repeat(lua_State *L)
{
   return 0;
}

//-------------
static void
_elua_evas_obj_free(void *obj)
{
   Edje_Lua_Evas_Object *elo = obj;
   lua_State *L;
   if (!elo->obj.ed) return;
   L = elo->obj.ed->L;
   evas_object_del(elo->evas_obj);
   elo->evas_obj = NULL;
}

static int
_elua_rect(lua_State *L)
{
   Edje *ed = (Edje *)_elua_table_ptr_get(L, _elua_key);
   Edje_Lua_Evas_Object *elo;

   elo = (Edje_Lua_Evas_Object *)_elua_obj_new(L, ed, sizeof(Edje_Lua_Evas_Object));
   elo->obj.free_func = _elua_evas_obj_free;
   elo->obj.is_evas_obj = 1;
   elo->evas_obj = evas_object_rectangle_add(evas_object_evas_get(ed->obj));
   evas_object_smart_member_add(elo->evas_obj, ed->obj);
   evas_object_clip_set(elo->evas_obj, ed->clipper);
   evas_object_move(elo->evas_obj, ed->x, ed->y);
   evas_object_resize(elo->evas_obj, 0, 0);
   evas_object_data_set(elo->evas_obj, "elo", elo);
   return 1;
}

//-------------
//---------------
//-------------------
//---------------------------
//-------------------------------------------
//----------------------------------------------------------------------------

//-------------
void
_edje_lua2_script_init(Edje *ed)
{
   static Edje_Lua_Alloc ela = { MAX_LUA_MEM, 0 };
   const luaL_Reg *l;
   char buf[256];
   void *data;
   int size;
   
   if (ed->L) return;
   _elua_init();
   ed->L = lua_newstate(_elua_alloc, &ela);
   lua_atpanic(ed->L, _elua_custom_panic);

// FIXME: figure out optimal gc settings later   
//   lua_gc(ed->L, LUA_GCSETPAUSE, 200);
//   lua_gc(ed->L, LUA_GCSETSTEPMUL, 200);

   for (l = _elua_libs; l->func; l++)
     {
        lua_pushcfunction(ed->L, l->func);
        lua_pushstring(ed->L, l->name);
        lua_call(ed->L, 1, 0);
     }
   
   luaL_register(ed->L, "edje", _elua_edje_api);
   luaL_newmetatable(ed->L, "edje");
   luaL_register(ed->L, 0, _elua_edje_meta);

   lua_pushliteral(ed->L, "__index");
   lua_pushvalue(ed->L, -3);
   lua_rawset(ed->L, -3);
   lua_pop(ed->L, 2);
   
   _elua_table_ptr_set(ed->L, _elua_key, ed);
   
   snprintf(buf, sizeof(buf), "lua_scripts/%i", ed->collection->id);
   data = eet_read(ed->file->ef, buf, &size);
   
   if (data)
     {
        int err;
   
        err = luaL_loadbuffer(ed->L, data, size, "edje_lua_script");
        if (err)
          {
             if (err == LUA_ERRSYNTAX)
               ERR("lua load syntax error: %s", 
                   lua_tostring(ed->L, -1));
             else if (err == LUA_ERRMEM)
               ERR("lua load memory allocation error: %s",
                   lua_tostring(ed->L, -1));
          }
        free(data);
        if (setjmp(panic_jmp) == 1)
          {
             ERR("Script init panic");
             return;
          }
        if ((err = lua_pcall(ed->L, 0, 0, 0)))
          _edje_lua2_error(ed->L, err);
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
}

void
_edje_lua2_script_load(Edje_Part_Collection *edc, void *data, int size)
{
   _elua_init();
}

void
_edje_lua2_script_unload(Edje_Part_Collection *edc)
{
   lua_State *L;
   if (!lstate) return;
   L = lstate;
   lua_gc(L, LUA_GCCOLLECT, 0);
}

#endif
