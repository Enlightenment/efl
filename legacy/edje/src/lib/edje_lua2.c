#include "edje_private.h"

//--------------------------------------------------------------------------//
#define MAX_LUA_MEM (4 * (1024 * 1024))

#ifdef _WIN32
# define FMT_SIZE_T "%Iu"
#else
# define FMT_SIZE_T "%zu"
#endif


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
   int              x, y;
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
static int _elua_color(lua_State *L);
static int _elua_clip(lua_State *L);
static int _elua_unclip(lua_State *L);
static int _elua_clipees(lua_State *L);
static int _elua_type(lua_State *L);
static int _elua_pass(lua_State *L);
static int _elua_repeat(lua_State *L);
static int _elua_precise(lua_State *L);

static int _elua_rect(lua_State *L);

#define ELO "|-ELO"

//--------------------------------------------------------------------------//
static lua_State *lstate = NULL;
static jmp_buf panic_jmp;

// FIXME: methods lua scrupt can provide that edje will call (not done yet):
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
static const struct luaL_reg _elua_edje_api [] =
{
   // add an echo too to make it more shelly
     {"echo",         _elua_echo}, // test func - echo (i know we have print. test)
   
   // time based "callback" systems
     {"timer",        _elua_timer}, // add timer
     {"animator",     _elua_animator}, // add animator
     {"transition",   _elua_transition}, // add transition
   // FIXME: need poller
   
   // system information (time, date blah blah)
     {"seconds",      _elua_seconds}, // get seconds
     {"looptime",     _elua_looptime}, // get loop time
     {"date",         _elua_date}, // get date in a table

   // talk to application/caller
     {"emit",         _elua_emit}, // emit signal + src
     {"messagesend",  _elua_messagesend}, // send a structured message

   // query edje - size, pos
     {"pos",          _elua_objpos}, // get while edje object pos in canvas
     {"size",         _elua_objsize}, // get while edje object pos in canvas
     {"geom",         _elua_objgeom}, // get while edje object geometry in canvas
   
   // FIXME: query color classes
   // FIXME: query text classes

     {"rect",         _elua_rect}, // new rect
   // FIXME: need image(filled, normal), text, textblock, edje
   
     {NULL, NULL} // end
};

static const struct luaL_reg _elua_edje_evas_obj [] =
{
   // generic object methods
     {"del",          _elua_obj_del}, // generic del any object created for edje (evas objects, timers, animators, transitions... everything)
   
   // now evas stuff (manipulate, delete etc.)
     {"show",         _elua_show}, // show, return current visibility
     {"hide",         _elua_hide}, // hide, return current visibility
     {"visible",      _elua_visible}, // get object visibility
     {"move",         _elua_move}, // move, return current position
     {"resize",       _elua_resize}, // resize, return current size
     {"pos",          _elua_pos}, // move, return current position
     {"size",         _elua_size}, // resize, return current size
     {"geom",         _elua_geom}, // move and resize and return current geometry
   
     {"raise",        _elua_raise}, // raise to top
     {"lower",        _elua_lower}, // lower to bottom
     {"above",        _elua_above}, // get object above or stack obj above given obj
     {"below",        _elua_below}, // get object below or stack obj below given obj
     {"top",          _elua_top}, // get top
     {"bottom",       _elua_bottom}, // get bottom
     {"color",        _elua_color}, // set color, return color
     {"clip",         _elua_clip}, // set clip obj, return clip object
     {"unclip",       _elua_unclip}, // clear clip obj
     {"clipees",      _elua_clipees}, // get clip children
     {"type",         _elua_type}, // get object type
     {"pass",         _elua_pass}, // set pass events, get pass events
     {"repeat",       _elua_repeat}, // set repeat events, get repeat events
     {"precise",      _elua_precise}, // set precise inside flag, get precise
   // FIXME: set callbacks (mouse down, up, blah blah blah)
   // 
   // FIXME: set scale (explicit value)
   // FIXME: need to set auto-scale (same as scale: 1)
   
   // FIXME: later - set render op, anti-alias, pointer mode (autograb, nograb)
   // FIXME: later - 
   
   // FIXME: map api here

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
//     {LUA_OSLIBNAME, luaopen_os}, // FIXME: audit os lib - maybe not provide or only provide specific calls
     {LUA_STRLIBNAME, luaopen_string},
     {LUA_MATHLIBNAME, luaopen_math},
//     {LUA_DBLIBNAME, luaopen_debug}, // disable this lib - don't want

     {NULL, NULL} // end
};
static const char *_elua_key = "key";
static const char *_elua_objs = "objs";

//--------------------------------------------------------------------------//
static void *
_elua_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
   Edje_Lua_Alloc *ela = ud;
   void *ptr2;
   
   ela->cur += nsize - osize;
   if (ela->cur > ela->max)
     {
        ERR("Edje Lua memory limit of " FMT_SIZE_T " bytes reached (" FMT_SIZE_T  " allocated)",
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
   ERR("Edje Lua cannot re-allocate " FMT_SIZE_T " bytes", nsize);
   return ptr2;
}

static int
_elua_custom_panic(lua_State *L __UNUSED__)
{
   ERR("Lua Panic!!!!");
   return 1;
}


//-------------
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
   
   luaL_register(L, "edje_evas_obj", _elua_edje_evas_obj);
   luaL_newmetatable(L, "edje_evas_obj");
   luaL_register(L, 0, _elua_edje_meta);

   lua_pushliteral(L, "__index");
   lua_pushvalue(L, -3);
   lua_rawset(L, -3);
   lua_pop(L, 2);

   // weak table for our objects
   lua_pushlightuserdata(L, &_elua_objs);
   lua_newtable(L);
   lua_pushstring(L, "__mode");
   lua_pushstring(L, "v");
   lua_rawset(L, -3);
   lua_rawset(L, LUA_REGISTRYINDEX);
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

/* XXX: not used
static void
_elua_table_ptr_del(lua_State *L, const void *key)
{
   lua_pushlightuserdata(L, (void *)key);
   lua_pushnil(L);
   lua_settable(L, LUA_REGISTRYINDEX);
}
*/

static void
_elua_gc(lua_State *L)
{
   lua_gc(L, LUA_GCCOLLECT, 0);
}

//-------------
/**
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
 
/**
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
_elua_obj_new(lua_State *L, Edje *ed, int size)
{
   Edje_Lua_Obj *obj;

   obj = (Edje_Lua_Obj *)lua_newuserdata(L, size);
   memset(obj, 0, size);
   ed->lua_objs = eina_inlist_append(ed->lua_objs, EINA_INLIST_GET(obj));

   luaL_getmetatable(L, "edje_evas_obj");
   lua_setmetatable(L, -2);
   obj->ed = ed;

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
   int ret = 0, err = 0;
   
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
   int ret = 0, err = 0;
   
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

/* XXX: not used
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
*/

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
   int x, y;

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
   Evas_Coord ow, oh;
   int w, h;

   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   evas_object_geometry_get(elo->evas_obj, NULL, NULL, &ow, &oh);
   if (_elua_2_int_get(L, 2, EINA_TRUE, "w", &w, "h", &h) > 0)
     {
        if ((w != ow) || (h != oh))
          {
             evas_object_resize(elo->evas_obj, w, h);
             evas_object_geometry_get(elo->evas_obj, NULL, NULL, &ow, &oh);
          }
     }
   _elua_int_ret(L, "w", ow);
   _elua_int_ret(L, "h", oh);
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
   Evas_Coord ow, oh;
   int x, y, w, h;

   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   evas_object_geometry_get(elo->evas_obj, NULL, NULL, &ow, &oh);
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
        if ((w != ow) || (h != oh))
          {
             evas_object_resize(elo->evas_obj, w, h);
             evas_object_geometry_get(elo->evas_obj, NULL, NULL, &ow, &oh);
          }
     }
   _elua_int_ret(L, "x", elo->x);
   _elua_int_ret(L, "y", elo->y);
   _elua_int_ret(L, "w", ow);
   _elua_int_ret(L, "h", oh);
   return 1;
}

static int
_elua_raise(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   evas_object_raise(elo->evas_obj);
   return 0;
}

static int
_elua_lower(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   evas_object_lower(elo->evas_obj);
   return 0;
}

static int
_elua_above(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Edje_Lua_Evas_Object *elo2;
   Evas_Object *o;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
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
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   if (!(o = evas_object_below_get(elo->evas_obj))) return 0;
   if (!(elo2 = evas_object_data_get(o, ELO))) return 0;
   _elua_ref_get(L, elo2);
   return 1;
}

static int
_elua_top(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
 //  Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Edje_Lua_Evas_Object *elo2;
   Evas_Object *o;
   Eina_List *list, *l;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
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

static int
_elua_bottom(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
//   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   Edje_Lua_Evas_Object *elo2;
   Evas_Object *o;
   Eina_List *list, *l;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
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
_elua_color(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int r, g, b, a;

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
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo2, *elo = (Edje_Lua_Evas_Object *)obj;
   Evas_Object *o;
   int n;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   n = lua_gettop(L);
   if (n == 2)
     {
        Edje_Lua_Obj *obj2 = (Edje_Lua_Obj *)lua_touserdata(L, 2);
        elo2 = (Edje_Lua_Evas_Object *)obj2;
        evas_object_clip_set(elo->evas_obj, elo2->evas_obj);
     }
   o = evas_object_clip_get(elo->evas_obj);
   if (!o) return 0;
   if (!(elo2 = evas_object_data_get(o, ELO))) return 0;
   _elua_ref_get(L, elo2);
   return 1;
}

static int
_elua_unclip(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   evas_object_clip_unset(elo->evas_obj);
   return 0;
}

static int
_elua_clipees(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo2, *elo = (Edje_Lua_Evas_Object *)obj;
   Eina_List *list, *l;
   Evas_Object *o;
   int n = 0;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
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
_elua_type(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   const char *t;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   t = evas_object_type_get(elo->evas_obj);
   if (!t) return 0;
   lua_pushstring(L, t);
   return 1;
}

static int
_elua_pass(lua_State *L)
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
             evas_object_pass_events_set(elo->evas_obj, lua_toboolean(L, 2));
          }
     }
   lua_pushboolean(L, evas_object_pass_events_get(elo->evas_obj));
   return 1;
}

static int
_elua_repeat(lua_State *L)
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
             evas_object_repeat_events_set(elo->evas_obj, lua_toboolean(L, 2));
          }
     }
   lua_pushboolean(L, evas_object_repeat_events_get(elo->evas_obj));
   return 1;
}

static int
_elua_precise(lua_State *L)
{
   Edje_Lua_Obj *obj = (Edje_Lua_Obj *)lua_touserdata(L, 1);
   Edje_Lua_Evas_Object *elo = (Edje_Lua_Evas_Object *)obj;
   int n;
   if (!obj) return 0;
   if (!obj->is_evas_obj) return 0;
   n = lua_gettop(L);
   if (n == 2)
     {
        evas_object_precise_is_inside_set(elo->evas_obj, lua_toboolean(L, 2));
     }
   lua_pushboolean(L, evas_object_precise_is_inside_get(elo->evas_obj));
   return 1;
}

//-------------
static void
_elua_evas_obj_free(void *obj)
{
   Edje_Lua_Evas_Object *elo = obj;
//   lua_State *L;
   if (!elo->obj.ed) return;
//   L = elo->obj.ed->L;
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
   evas_object_clip_set(elo->evas_obj, ed->base.clipper);
   evas_object_move(elo->evas_obj, ed->x, ed->y);
   evas_object_resize(elo->evas_obj, 0, 0);
   evas_object_data_set(elo->evas_obj, ELO, elo);
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
   lua_State *L;
   
   if (ed->L) return;
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
   
   
   luaL_register(L, "edje", _elua_edje_api);
   luaL_newmetatable(L, "edje");
   luaL_register(L, 0, _elua_edje_meta);

   luaL_register(L, "edje_evas_obj", _elua_edje_evas_obj);
   luaL_newmetatable(L, "edje_evas_obj");
   luaL_register(L, 0, _elua_edje_meta);

   lua_pushliteral(L, "__index");
   lua_pushvalue(L, -3);
   lua_rawset(L, -3);
   lua_pop(L, 2);

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
               ERR("lua load syntax error: %s", 
                   lua_tostring(L, -1));
             else if (err == LUA_ERRMEM)
               ERR("lua load memory allocation error: %s",
                   lua_tostring(L, -1));
          }
        free(data);
        if (setjmp(panic_jmp) == 1)
          {
             ERR("Script init panic");
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
