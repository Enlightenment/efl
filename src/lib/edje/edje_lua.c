#include "edje_private.h"

#if 0
/////////////////////////////////////////////////////////////////////////////
// the below is deprecated and here for reference only until removed. look
// at edje_lua2.c for the active workign code
/////////////////////////////////////////////////////////////////////////////

#include <lauxlib.h>
#include <lualib.h>

#define EDJE_LUA_GET 1
#define EDJE_LUA_SET 2
#define EDJE_LUA_FN  3

typedef struct _Edje_Lua_Alloc                 Edje_Lua_Alloc;

typedef struct _Edje_Lua_Ref                   Edje_Lua_Ref;

typedef struct _Edje_Lua_Reg                   Edje_Lua_Reg;

typedef struct _Edje_Lua_Timer                 Edje_Lua_Timer;

typedef struct _Edje_Lua_Animator              Edje_Lua_Animator;

typedef struct _Edje_Lua_Poller                Edje_Lua_Poller;

typedef struct _Edje_Lua_Transform             Edje_Lua_Transform;

typedef struct _Edje_Lua_Transition            Edje_Lua_Transition;

typedef struct _Edje_Lua_Evas_Object           Edje_Lua_Evas_Object;

typedef struct _Edje_Lua_Edje_Part_Object      Edje_Lua_Edje_Part_Object;

typedef struct _Edje_Lua_Edje_Part_Description Edje_Lua_Edje_Part_Description;

struct _Edje_Lua_Alloc
{
   size_t max, cur; /* maximal and current memory used by Lua */
};

struct _Edje_Lua_Ref
{
   int        id;
   lua_State *L;
};

struct _Edje_Lua_Reg
{
   const luaL_Reg *mt, *get, *set, *fn;
};

struct _Edje_Lua_Timer
{
   lua_State    *L;
   Ecore_Timer  *et;
   Edje_Lua_Ref *cb;
};

struct _Edje_Lua_Animator
{
   lua_State      *L;
   Ecore_Animator *ea;
   Edje_Lua_Ref   *cb;
};

struct _Edje_Lua_Poller
{
   lua_State    *L;
   Ecore_Poller *ep;
   Edje_Lua_Ref *cb;
};

struct _Edje_Lua_Transform
{
   lua_State     *L;
   Evas_Transform et;
};

struct _Edje_Lua_Transition
{
   lua_State    *L;
   Ecore_Timer  *et;
   Edje_Lua_Ref *trans;
   Edje_Lua_Ref *cb;
   Edje_Lua_Ref *ref;
   double        dur;
};

struct _Edje_Lua_Evas_Object
{
   lua_State   *L;
   Edje        *ed;
   Evas_Object *eo;
   Eina_Bool    mouse_events;
   Eina_List   *cb;
};

struct _Edje_Lua_Edje_Part_Object
{
   lua_State      *L;
   Edje           *ed;
   Evas_Object    *eo;
   Edje_Real_Part *rp;
   const char     *key;
};

struct _Edje_Lua_Edje_Part_Description
{
   lua_State             *L;
   Edje                  *ed;
   Evas_Object           *eo;
   Edje_Real_Part        *rp;
   Edje_Part_Description *pd;
};

jmp_buf _edje_lua_panic_jmp;

static int
_edje_lua_custom_panic(EINA_UNUSED lua_State *L)
{
   CRI("PANIC");
   longjmp(_edje_lua_panic_jmp, 1);
   return 1; /* longjmp() never returns, but this keep gcc happy */
}

void
__edje_lua_error(const char *file, const char *fnc, int line, lua_State *L, int err_code)
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
     (_edje_default_log_dom, EINA_LOG_LEVEL_ERR, file, fnc, line,
     "Lua %s error: %s", err_type, lua_tostring(L, -1));
   // don't exit. this is BAD. lua script bugs will cause thngs like e to
   // exit mysteriously ending your x session. bad!
   // exit(-1);
}

lua_State *
_edje_lua_new_thread(Edje *ed, lua_State *L)
{
#if 1 // newlua
   lua_newtable(L);
   ed->lua_ref = luaL_ref(L, LUA_REGISTRYINDEX);
   /* inherit new environment from global environment */
   lua_createtable(L, 1, 0);
   lua_pushvalue(L, LUA_GLOBALSINDEX);
   lua_setfield(L, -2, "__index");
   lua_setmetatable(L, -2);
   lua_setfenv(L, -2);
   return L;
#else
   /* create new thread */
   lua_State *thread = lua_newthread(L);
   //printf ("new thread %d->%d\n", L, thread);
   /* create new environment for new thread */
   lua_newtable(L);
   /* inherit new environment from global environment */
   lua_createtable(L, 1, 0);
   lua_pushvalue(L, LUA_GLOBALSINDEX);
   lua_setfield(L, -2, "__index");
   lua_setmetatable(L, -2);
   lua_setfenv(L, -2);
   return thread;
#endif
}

void
_edje_lua_free_thread(Edje *ed, lua_State *L)
{
#if 1 // newlua
   luaL_unref(L, LUA_REGISTRYINDEX, ed->lua_ref);
   lua_gc(L, LUA_GCCOLLECT, 0);
#else
   lua_pushthread(L);
   lua_getfenv(L, -1);
   lua_pushnil(L);
   while (lua_next(L, -2))
     {
        // key at -2, value at -1
        lua_pop(L, 1);
        lua_pushvalue(L, -1);
        lua_pushnil(L);
        lua_rawset(L, -4);
     }
   lua_settop(L, 0);
   lua_gc(L, LUA_GCCOLLECT, 0);
#endif
}

/*
 * only for debug, returns number of objects in registry
 */
static int
_edje_lua_reg_count(lua_State *L)
{
   int count = 0;
   lua_pushvalue(L, LUA_REGISTRYINDEX);
   lua_pushnil(L);
   while (lua_next(L, -2))
     {
        // key at -2, value at -1
        lua_pop(L, 1);
        count++;
     }
   lua_pop(L, 1);
   return count;
}

static Edje_Lua_Ref *
_edje_lua_new_ref(lua_State *L, int index)
{
   lua_pushvalue(L, index);
   Edje_Lua_Ref *ref = malloc(sizeof(Edje_Lua_Ref));
   ref->id = luaL_ref(L, LUA_REGISTRYINDEX);
   ref->L = L;
   return ref;
}

static void
_edje_lua_get_ref(lua_State *L, Edje_Lua_Ref *ref)
{
   lua_rawgeti(L, LUA_REGISTRYINDEX, ref->id);
}

static void
_edje_lua_free_ref(lua_State *L, Edje_Lua_Ref *ref)
{
   //printf ("_edje_lua_free_ref %d %d %d\n", L, lua_objlen(L, LUA_REGISTRYINDEX), _edje_lua_reg_count(L));
   luaL_unref(L, LUA_REGISTRYINDEX, ref->id);
   free(ref);
   lua_gc(L, LUA_GCCOLLECT, 0);
}

void
_edje_lua_new_reg(lua_State *L, int index, void *ptr)
{
   //printf ("_edje_lua_new_reg %d %d %d\n", L, ptr, _edje_lua_reg_count(L));
   lua_pushvalue(L, index);
   lua_pushlightuserdata(L, ptr);
   lua_insert(L, -2);
   lua_rawset(L, LUA_REGISTRYINDEX); /* freed in _edje_lua_free_reg */
}

void
_edje_lua_get_reg(lua_State *L, void *ptr)
{
   //printf ("_edje_lua_get_reg %d %d\n", L, ptr);
   lua_pushlightuserdata(L, ptr);
   lua_rawget(L, LUA_REGISTRYINDEX);
}

void
_edje_lua_free_reg(lua_State *L, void *ptr)
{
   //printf ("_edje_lua_free_reg %d %d %d\n", L, ptr, _edje_lua_reg_count(L));
   lua_pushlightuserdata(L, ptr);
   lua_pushnil(L);
   lua_rawset(L, LUA_REGISTRYINDEX); /* created in _edje_lua_new_reg */
   lua_gc(L, LUA_GCCOLLECT, 0);
}

static void
_edje_lua_rawsetfield(lua_State *L, int index, const char *key)
{
   lua_pushstring(L, key);
   lua_insert(L, -2);
   if (index < 0)
     lua_rawset(L, index - 1);
   else
     lua_rawset(L, index);
}

static void
_edje_lua_rawgetfield(lua_State *L, int index, const char *key)
{
   lua_pushstring(L, key);
   if (index < 0)
     lua_rawget(L, index - 1);
   else
     lua_rawget(L, index);
}

static void
_edje_lua_new_const(lua_State *L, const char *id, int val)
{
   lua_pushnumber(L, val);
   lua_setglobal(L, id);
}

static void
_edje_lua_new_metatable(lua_State *L, const Edje_Lua_Reg **class)
{
   lua_newtable(L);
   lua_pushlightuserdata(L, class);
   lua_pushvalue(L, -2);
   lua_rawset(L, LUA_REGISTRYINDEX); /* freed in _edje_lua_free_metatable */
   lua_pushvalue(L, -1);
   lua_pushlightuserdata(L, class);
   lua_rawset(L, LUA_REGISTRYINDEX); /* freed in _edje_lua_free_metatable */
}

static void
_edje_lua_get_metatable(lua_State *L, const Edje_Lua_Reg **class)
{
   lua_pushlightuserdata(L, class);
   lua_rawget(L, LUA_REGISTRYINDEX);
}

static void
_edje_lua_free_metatable(lua_State *L, const Edje_Lua_Reg **class)
{
   lua_pushlightuserdata(L, class);
   lua_rawget(L, LUA_REGISTRYINDEX);
   lua_pushnil(L);
   lua_rawset(L, LUA_REGISTRYINDEX); /* created in _edje_lua_new_metatable */
   lua_pushlightuserdata(L, class);
   lua_pushnil(L);
   lua_rawset(L, LUA_REGISTRYINDEX); /* created in _edje_lua_new_metatable */
   lua_gc(L, LUA_GCCOLLECT, 0);
}

static void *
_edje_lua_checkudata(lua_State *L, int pos, const Edje_Lua_Reg *module)
{
   luaL_checktype(L, pos, LUA_TUSERDATA);
   lua_getmetatable(L, pos);
   lua_rawget(L, LUA_REGISTRYINDEX);
   Edje_Lua_Reg **class = lua_touserdata(L, -1);
   lua_pop(L, 1); // class
   int flag = 0;
   int ptr = 0;
   while (class[ptr] && !flag)
     if (class[ptr++] == module)
       flag = 1;
   if (!flag)
     {
        lua_pushstring(L, "class type mismatch");
        lua_error(L);
     }
   return lua_touserdata(L, pos);
}

static void
_edje_lua_new_class(lua_State *L, const Edje_Lua_Reg **class)
{
   int n = 0;
   _edje_lua_new_metatable(L, class);
   while (class && (class[n] != NULL))
     {
        luaL_register(L, NULL, class[n]->mt);
        lua_pushstring(L, "hands off, it's none of your business!");
        _edje_lua_rawsetfield(L, -2, "__metatable");

        if (n == 0)
          {
             lua_newtable(L);
             luaL_register(L, NULL, class[n]->set);
             lua_rawseti(L, -2, EDJE_LUA_SET);

             lua_newtable(L);
             luaL_register(L, NULL, class[n]->get);
             lua_rawseti(L, -2, EDJE_LUA_GET);

             lua_newtable(L);
             luaL_register(L, NULL, class[n]->fn);
             lua_rawseti(L, -2, EDJE_LUA_FN);
          }
        else
          {
             lua_rawgeti(L, -1, EDJE_LUA_SET);
             luaL_register(L, NULL, class[n]->set);
             lua_pop(L, 1);

             lua_rawgeti(L, -1, EDJE_LUA_GET);
             luaL_register(L, NULL, class[n]->get);
             lua_pop(L, 1);

             lua_rawgeti(L, -1, EDJE_LUA_FN);
             luaL_register(L, NULL, class[n]->fn);
             lua_pop(L, 1);
          }
        n += 1;
     }
}

static void
_edje_lua_set_class(lua_State *L, int index, const Edje_Lua_Reg **class)
{
   lua_newtable(L);
   if (index < 0)
     lua_setfenv(L, index - 1);
   else
     lua_setfenv(L, index);

   _edje_lua_get_metatable(L, class);
   if (index < 0)
     lua_setmetatable(L, index - 1);
   else
     lua_setmetatable(L, index);
}

static int
_edje_lua_look_fn(lua_State *L)
{
   lua_rawgeti(L, -1, EDJE_LUA_FN);
   lua_pushvalue(L, 2); // key
   lua_rawget(L, -2); // .fn[key]
   if (lua_iscfunction(L, -1))
     return 1;
   else
     {
        lua_pop(L, 2); // .fn[key], .fn
        return 0;
     }
}

static int
_edje_lua_look_get(lua_State *L)
{
   lua_rawgeti(L, -1, EDJE_LUA_GET);
   lua_pushvalue(L, 2); // key
   lua_rawget(L, -2); // .get[key]
   if (lua_iscfunction(L, -1))
     {
        int err_code;

        lua_pushvalue(L, 1);

        if ((err_code = lua_pcall(L, 1, 1, 0)))
          _edje_lua_error(L, err_code);
        return 1;
     }
   else
     {
        lua_pop(L, 2); // .get[key], .get
        return 0;
     }
}

static int
_edje_lua_look_set(lua_State *L)
{
   lua_rawgeti(L, -1, EDJE_LUA_SET);
   lua_pushvalue(L, 2); // key
   lua_rawget(L, -2); // .set[key]
   if (lua_iscfunction(L, -1))
     {
        int err_code;

        lua_pushvalue(L, 1);    // obj
        lua_pushvalue(L, 3);    // value

        if ((err_code = lua_pcall(L, 2, 0, 0))) // .set[key](obj,key,value)
          _edje_lua_error(L, err_code);
        return 1;
     }
   else
     {
        lua_pop(L, 2); // .set[key], .set
        return 0;
     }
}

/*
 * Lua Class bindings
 */

const luaL_Reg lNil[] = {
   {NULL, NULL} // sentinel
};

const luaL_Reg lClass_mt[];

const luaL_Reg lClass_fn[];

const Edje_Lua_Reg mClass = {
   lClass_mt,
   lNil,
   lNil,
   lClass_fn
};

static int
_edje_lua_class_mt_index(lua_State *L)
{
   _edje_lua_checkudata(L, 1, &mClass);
   lua_getmetatable(L, 1);
   if (!_edje_lua_look_fn(L)) // look in lClass_fn
     if (!_edje_lua_look_get(L)) // look in lClass_get
       {                        // look in obj ref hash
         lua_getfenv(L, 1);
         lua_pushvalue(L, 2);   // key
         lua_rawget(L, -2);
       }
   return 1;
}

static int
_edje_lua_class_mt_newindex(lua_State *L)
{
   _edje_lua_checkudata(L, 1, &mClass);
   lua_getmetatable(L, 1);
   if (!_edje_lua_look_set(L))  // look in lClass_set
     {                          // look in obj ref hash
       lua_getfenv(L, 1);
       lua_pushvalue(L, 2);
       lua_pushvalue(L, 3);
       lua_rawset(L, -3);
     }
   return 0;
}

static int
_edje_lua_class_mt_gc(lua_State *L)
{
   _edje_lua_checkudata(L, 1, &mClass);
   //printf("_edje_lua_class_mt_gc\n");
   /* FIXME has to be commented to not raise an error, solve differently
      lua_getfield(L, 1, "del");
      if (!lua_isnil(L, -1))
      {
        lua_pushvalue(L, 1);
        int err_code;

        if (err_code = lua_pcall(L, 1, 0, 0))
           _edje_lua_error(L, err_code);
      }
      lua_pop(L, 1);
    */
   return 0;
}

static int
_edje_lua_class_fn_set(lua_State *L)
{
   _edje_lua_checkudata(L, 1, &mClass);
   /*
    * for k,v in pairs(table) do
    *   obj[k] = v
    * end
    */
   lua_pushnil(L);
   while (lua_next(L, 2))
     {
        // key at -2, value at -1
        lua_pushvalue(L, -2);
        lua_insert(L, -2);
        lua_settable(L, 1);
     }
   return 0;
}

static int
_edje_lua_class_fn_get(lua_State *L)
{
   _edje_lua_checkudata(L, 1, &mClass);
   /*
    * res = {}
    * mtG = getmetatable(obj)['.get']
    * for k,v in pairs(mtG) do
    *   res[k] = obj[k]
    * end
    */
   lua_newtable(L); // res
   lua_getmetatable(L, 1); // mt
   lua_getfield(L, -1, ".get");
   lua_remove(L, -2); // mt

   lua_pushnil(L);
   while (lua_next(L, -2))
     {
        // key at -2, value at -1
        lua_pop(L, 1); // value = cfunction
        lua_pushvalue(L, -1);   // key
        lua_pushvalue(L, -1);   // key
        lua_gettable(L, 1);     // obj[key]
        lua_settable(L, 2);     // res[key]
     }
   lua_pop(L, 1); // .get
   return 1;
}

static int
_edje_lua_class_itr_call(lua_State *L, int id)
{
   int err_code;

   _edje_lua_checkudata(L, 1, &mClass);
   lua_getmetatable(L, 1); // mt
   lua_rawgeti(L, -1, id);
   lua_remove(L, -2); // mt
   lua_getglobal(L, "pairs");
   lua_insert(L, -2);

   if ((err_code = lua_pcall(L, 1, 3, 0)))
     _edje_lua_error(L, err_code);
   return 3;
}

static int
_edje_lua_class_fn_gpairs(lua_State *L)
{
   _edje_lua_checkudata(L, 1, &mClass);
   return _edje_lua_class_itr_call(L, EDJE_LUA_GET);
}

static int
_edje_lua_class_fn_spairs(lua_State *L)
{
   _edje_lua_checkudata(L, 1, &mClass);
   return _edje_lua_class_itr_call(L, EDJE_LUA_SET);
}

static int
_edje_lua_class_fn_fpairs(lua_State *L)
{
   _edje_lua_checkudata(L, 1, &mClass);
   return _edje_lua_class_itr_call(L, EDJE_LUA_FN);
}

static int
_edje_lua_class_fn_pairs(lua_State *L)
{
   int err_code;

   _edje_lua_checkudata(L, 1, &mClass);
   lua_getfenv(L, 1);
   lua_getglobal(L, "pairs");
   lua_insert(L, -2);

   if ((err_code = lua_pcall(L, 1, 3, 0)))
     _edje_lua_error(L, err_code);
   return 3;
}

static int
_edje_lua_class_fn_ipairs(lua_State *L)
{
   int err_code;

   _edje_lua_checkudata(L, 1, &mClass);
   lua_getfenv(L, 1);
   lua_getglobal(L, "ipairs");
   lua_insert(L, -2);

   if ((err_code = lua_pcall(L, 1, 3, 0)))
     _edje_lua_error(L, err_code);
   return 3;
}

const luaL_Reg lClass_mt[] = {
   {"__index", _edje_lua_class_mt_index},
   {"__newindex", _edje_lua_class_mt_newindex},
   {"__gc", _edje_lua_class_mt_gc},
   {NULL, NULL} // sentinel
};

const luaL_Reg lClass_fn[] = {
   {"get", _edje_lua_class_fn_get},
   {"set", _edje_lua_class_fn_set},
   {"gpairs", _edje_lua_class_fn_gpairs},
   {"spairs", _edje_lua_class_fn_spairs},
   {"fpairs", _edje_lua_class_fn_fpairs},
   {"pairs", _edje_lua_class_fn_pairs},
   {"ipairs", _edje_lua_class_fn_ipairs},
   {NULL, NULL} // sentinel
};

const luaL_Reg lTimer_get[];

const luaL_Reg lTimer_set[];

const luaL_Reg lTimer_fn[];

const Edje_Lua_Reg mTimer = {
   lNil,
   lTimer_get,
   lTimer_set,
   lTimer_fn
};

const Edje_Lua_Reg *cTimer[] = {
   &mClass,
   &mTimer,
   NULL // sentinel
};

static Eina_Bool
_edje_lua_timer_cb(void *data)
{
   Edje_Lua_Timer *obj = data;
   lua_State *L = obj->L;
   int err_code;
   Eina_Bool res;

   _edje_lua_get_ref(L, obj->cb); // callback function
   _edje_lua_get_reg(L, obj);

   if ((err_code = lua_pcall(L, 1, 1, 0)))
     {
        _edje_lua_error(L, err_code);
        return ECORE_CALLBACK_CANCEL;
     }

   res = luaL_optint(L, -1, ECORE_CALLBACK_CANCEL);
   lua_pop(L, 1); // -- res

/*
   if (_edje_lua_panic_here())
   printf("blahc\n");
   else
   lua_pop(L, 1);		// -- res
 */
   if (res == ECORE_CALLBACK_CANCEL)
     {
        // delete object
        _edje_lua_get_reg(L, obj);
        lua_pushvalue(L, -1);
        lua_pushstring(L, "del");
        lua_gettable(L, -2);
        lua_insert(L, -2);
        if ((err_code = lua_pcall(L, 1, 0, 0)))
          _edje_lua_error(L, err_code);
     }
   return res;
}

static int
_edje_lua_timer_get_pending(lua_State *L)
{
   Edje_Lua_Timer *obj = _edje_lua_checkudata(L, 1, &mTimer);
   if (obj->et)
     lua_pushnumber(L, ecore_timer_pending_get(obj->et));
   else
     lua_pushnil(L);
   return 1;
}

static int
_edje_lua_timer_get_precision(lua_State *L)
{
   Edje_Lua_Timer *obj = _edje_lua_checkudata(L, 1, &mTimer);

   if (obj->et)
     lua_pushnumber(L, ecore_timer_precision_get());
   else
     lua_pushnil(L);

   return 1;
}

static int
_edje_lua_timer_get_interval(lua_State *L)
{
   Edje_Lua_Timer *obj = _edje_lua_checkudata(L, 1, &mTimer);

   if (obj->et)
     lua_pushnumber(L, ecore_timer_interval_get(obj->et));
   else
     lua_pushnil(L);

   return 1;
}

const luaL_Reg lTimer_get[] = {
   {"pending", _edje_lua_timer_get_pending},
   {"precision", _edje_lua_timer_get_precision},
   {"interval", _edje_lua_timer_get_interval},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_timer_set_interval(lua_State *L)
{
   Edje_Lua_Timer *obj = _edje_lua_checkudata(L, 1, &mTimer);
   if (obj->et)
     ecore_timer_interval_set(obj->et, luaL_checknumber(L, 2));
   return 0;
}

const luaL_Reg lTimer_set[] = {
   {"interval", _edje_lua_timer_set_interval},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_timer_fn_del(lua_State *L)
{
   Edje_Lua_Timer *obj = _edje_lua_checkudata(L, 1, &mTimer);
   if (obj->et)
     {
        ecore_timer_del(obj->et);
        obj->et = NULL;
     }
   if (obj->cb)
     {
        _edje_lua_free_ref(L, obj->cb); // created in _edje_lua_group_fn_timer
        obj->cb = NULL;
     }
   _edje_lua_free_reg(L, obj); // created in _edje_lua_group_fn_timer
   return 0;
}

static int
_edje_lua_timer_fn_freeze(lua_State *L)
{
   Edje_Lua_Timer *obj = _edje_lua_checkudata(L, 1, &mTimer);
   if (obj->et)
     ecore_timer_freeze(obj->et);
   return 0;
}

static int
_edje_lua_timer_fn_thaw(lua_State *L)
{
   Edje_Lua_Timer *obj = _edje_lua_checkudata(L, 1, &mTimer);
   if (obj->et)
     ecore_timer_thaw(obj->et);
   return 0;
}

static int
_edje_lua_timer_fn_delay(lua_State *L)
{
   Edje_Lua_Timer *obj = _edje_lua_checkudata(L, 1, &mTimer);
   if (obj->et)
     ecore_timer_delay(obj->et, luaL_checknumber(L, 2));
   return 0;
}

const luaL_Reg lTimer_fn[] = {
   {"del", _edje_lua_timer_fn_del},
   {"freeze", _edje_lua_timer_fn_freeze},
   {"thaw", _edje_lua_timer_fn_thaw},
   {"delay", _edje_lua_timer_fn_delay},
   {NULL, NULL} // sentinel
};

const luaL_Reg lAnimator_get[];

const luaL_Reg lAnimator_fn[];

const Edje_Lua_Reg mAnimator = {
   lNil,
   lAnimator_get,
   lNil,
   lAnimator_fn
};

const Edje_Lua_Reg *cAnimator[] = {
   &mClass,
   &mAnimator,
   NULL // sentinel
};

static Eina_Bool
_edje_lua_animator_cb(void *data)
{
   Eina_Bool res;
   int err;
   Edje_Lua_Animator *obj = data;
   lua_State *L = obj->L;

   _edje_lua_get_ref(L, obj->cb);
   _edje_lua_get_reg(L, obj);

   if ((err = lua_pcall(L, 1, 1, 0)))
     {
        _edje_lua_error(L, err);
        return ECORE_CALLBACK_CANCEL;
     }

   res = luaL_checkint(L, -1);
   lua_pop(L, 1); // Pop res off the stack
   if (res == ECORE_CALLBACK_CANCEL)
     {
        /* delete animator */
        _edje_lua_get_reg(L, obj);
        lua_pushvalue(L, -1);
        lua_pushstring(L, "del");
        lua_gettable(L, -2);
        lua_insert(L, -2);
        if ((err = lua_pcall(L, 1, 0, 0)))
          _edje_lua_error(L, err);
     }

   return res;
}

static int
_edje_lua_animator_get_frametime(lua_State *L)
{
   Edje_Lua_Animator *obj = _edje_lua_checkudata(L, 1, &mAnimator);

   if (obj->ea)
     lua_pushnumber(L, ecore_animator_frametime_get());
   else
     lua_pushnil(L);

   return 1;
}

const luaL_Reg lAnimator_get[] = {
   {"frametime", _edje_lua_animator_get_frametime},
   {NULL, NULL}
};

static int
_edje_lua_animator_fn_del(lua_State *L)
{
   Edje_Lua_Animator *obj = _edje_lua_checkudata(L, 1, &mAnimator);
   if (obj->ea)
     {
        ecore_animator_del(obj->ea);
        obj->ea = NULL;
     }
   if (obj->cb)
     {
        _edje_lua_free_ref(L, obj->cb); // created in _edje_lua_group_fn_animator
        obj->cb = NULL;
     }
   _edje_lua_free_reg(L, obj); // created in _edje_lua_group_fn_animator
   return 0;
}

const luaL_Reg lAnimator_fn[] = {
   {"del", _edje_lua_animator_fn_del},
   {NULL, NULL} // sentinel
};

const luaL_Reg lPoller_get[];

const luaL_Reg lPoller_fn[];

const Edje_Lua_Reg mPoller = {
   lNil,
   lPoller_get,
   lNil,
   lPoller_fn
};

const Edje_Lua_Reg *cPoller[] = {
   &mClass,
   &mPoller,
   NULL // sentinel
};

static Eina_Bool
_edje_lua_poller_cb(void *data)
{
   Eina_Bool res;
   int err;
   Edje_Lua_Poller *obj = data;
   lua_State *L = obj->L;

   _edje_lua_get_ref(L, obj->cb);
   _edje_lua_get_reg(L, obj);

   if ((err = lua_pcall(L, 1, 1, 0)))
     {
        _edje_lua_error(L, err);
        return ECORE_CALLBACK_CANCEL;
     }

   res = luaL_checkint(L, -1);
   lua_pop(L, 1); // Pop res off the stack
   if (res == ECORE_CALLBACK_CANCEL)
     {
        /* delete poller */
        _edje_lua_get_reg(L, obj);
        lua_pushvalue(L, -1);
        lua_pushstring(L, "del");
        lua_gettable(L, -2);
        lua_insert(L, -2);
        if ((err = lua_pcall(L, 1, 0, 0)))
          _edje_lua_error(L, err);
     }

   return res;
}

static int
_edje_lua_poller_get_interval(lua_State *L)
{
   Edje_Lua_Poller *obj = _edje_lua_checkudata(L, 1, &mPoller);

   if (obj->ep)
     lua_pushnumber(L, ecore_poller_poll_interval_get(ECORE_POLLER_CORE));
   else
     lua_pushnil(L);

   return 1;
}

const luaL_Reg lPoller_get[] = {
   {"interval", _edje_lua_poller_get_interval},
   {NULL, NULL}
};

static int
_edje_lua_poller_fn_del(lua_State *L)
{
   Edje_Lua_Poller *obj = _edje_lua_checkudata(L, 1, &mPoller);
   if (obj->ep)
     {
        ecore_poller_del(obj->ep);
        obj->ep = NULL;
     }

   if (obj->cb)
     {
        _edje_lua_free_ref(L, obj->cb); // created in _edje_lua_group_fn_poller
        obj->cb = NULL;
     }
   _edje_lua_free_reg(L, obj); // created in _edje_lua_group_fn_poller

   return 0;
}

const luaL_Reg lPoller_fn[] = {
   {"del", _edje_lua_poller_fn_del},
   {NULL, NULL}
};

/*
 * Lua Edje Transform bindings
 */

const luaL_Reg lTransform_get[];

const luaL_Reg lTransform_set[];

const luaL_Reg lTransform_fn[];

const Edje_Lua_Reg mTransform = {
   lNil,
   lTransform_get,
   lTransform_set,
   lTransform_fn
};

const Edje_Lua_Reg *cTransform[] = {
   &mClass,
   &mTransform,
   NULL // sentinel
};

static int
_edje_lua_transform_get_matrix(lua_State *L)
{
   Edje_Lua_Transform *obj = _edje_lua_checkudata(L, 1, &mTransform);
   lua_createtable(L, 3, 0);
   lua_createtable(L, 3, 0);
   lua_pushnumber(L, obj->et.mxx); lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->et.mxy); lua_rawseti(L, -2, 2);
   lua_pushnumber(L, obj->et.mxz); lua_rawseti(L, -2, 3);
   lua_rawseti(L, -2, 1);
   lua_createtable(L, 3, 0);
   lua_pushnumber(L, obj->et.myx); lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->et.myy); lua_rawseti(L, -2, 2);
   lua_pushnumber(L, obj->et.myz); lua_rawseti(L, -2, 3);
   lua_rawseti(L, -2, 2);
   lua_createtable(L, 3, 0);
   lua_pushnumber(L, obj->et.mzx); lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->et.mzy); lua_rawseti(L, -2, 2);
   lua_pushnumber(L, obj->et.mzz); lua_rawseti(L, -2, 3);
   lua_rawseti(L, -2, 3);
   return 1;
}

const luaL_Reg lTransform_get[] = {
   {"matrix", _edje_lua_transform_get_matrix},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_transform_set_matrix(lua_State *L)
{
   Edje_Lua_Transform *obj = _edje_lua_checkudata(L, 1, &mTransform);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   luaL_checktype(L, -1, LUA_TTABLE);
   lua_rawgeti(L, -1, 1); obj->et.mxx = luaL_checknumber(L, -1); lua_pop(L, 1);
   lua_rawgeti(L, -1, 2); obj->et.mxy = luaL_checknumber(L, -1); lua_pop(L, 1);
   lua_rawgeti(L, -1, 3); obj->et.mxz = luaL_checknumber(L, -1); lua_pop(L, 1);
   lua_pop(L, 1);
   lua_rawgeti(L, 2, 2);
   luaL_checktype(L, -1, LUA_TTABLE);
   lua_rawgeti(L, -1, 1); obj->et.myx = luaL_checknumber(L, -1); lua_pop(L, 1);
   lua_rawgeti(L, -1, 2); obj->et.myy = luaL_checknumber(L, -1); lua_pop(L, 1);
   lua_rawgeti(L, -1, 3); obj->et.myz = luaL_checknumber(L, -1); lua_pop(L, 1);
   lua_pop(L, 1);
   lua_rawgeti(L, 2, 3);
   luaL_checktype(L, -1, LUA_TTABLE);
   lua_rawgeti(L, -1, 1); obj->et.mzx = luaL_checknumber(L, -1); lua_pop(L, 1);
   lua_rawgeti(L, -1, 2); obj->et.mzy = luaL_checknumber(L, -1); lua_pop(L, 1);
   lua_rawgeti(L, -1, 3); obj->et.mzz = luaL_checknumber(L, -1); lua_pop(L, 1);
   lua_pop(L, 1);
   return 0;
}

const luaL_Reg lTransform_set[] = {
   {"matrix", _edje_lua_transform_set_matrix},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_transform_fn_identity(lua_State *L)
{
   Edje_Lua_Transform *obj = _edje_lua_checkudata(L, 1, &mTransform);
   evas_transform_identity_set(&(obj->et));
   return 0;
}

static int
_edje_lua_transform_fn_rotate(lua_State *L)
{
   Edje_Lua_Transform *obj = _edje_lua_checkudata(L, 1, &mTransform);
   evas_transform_rotate(luaL_checknumber(L, 2), &(obj->et));
   return 0;
}

static int
_edje_lua_transform_fn_translate(lua_State *L)
{
   Edje_Lua_Transform *obj = _edje_lua_checkudata(L, 1, &mTransform);
   evas_transform_translate(luaL_checknumber(L, 2), luaL_checknumber(L, 3), &(obj->et));
   return 0;
}

static int
_edje_lua_transform_fn_scale(lua_State *L)
{
   Edje_Lua_Transform *obj = _edje_lua_checkudata(L, 1, &mTransform);
   evas_transform_scale(luaL_checknumber(L, 2), luaL_checknumber(L, 3), &(obj->et));
   return 0;
}

static int
_edje_lua_transform_fn_shear(lua_State *L)
{
   Edje_Lua_Transform *obj = _edje_lua_checkudata(L, 1, &mTransform);
   evas_transform_shear(luaL_checknumber(L, 2), luaL_checknumber(L, 3), &(obj->et));
   return 0;
}

static int
_edje_lua_transform_fn_compose(lua_State *L)
{
   Edje_Lua_Transform *obj = _edje_lua_checkudata(L, 1, &mTransform);
   Edje_Lua_Transform *tar = _edje_lua_checkudata(L, 2, &mTransform);
   evas_transform_compose(&(tar->et), &(obj->et));
   return 0;
}

const luaL_Reg lTransform_fn[] = {
   {"identity", _edje_lua_transform_fn_identity},
   {"rotate", _edje_lua_transform_fn_rotate},
   {"translate", _edje_lua_transform_fn_translate},
   {"scale", _edje_lua_transform_fn_scale},
   {"shear", _edje_lua_transform_fn_shear},
   {"compose", _edje_lua_transform_fn_compose},
   {NULL, NULL} // sentinel
};

/*
 * Lua Edje Transition bindings
 */

const luaL_Reg lTransition_get[];

const luaL_Reg lTransition_set[];

const luaL_Reg lTransition_fn[];

const Edje_Lua_Reg mTransition = {
   lNil,
   lTransition_get,
   lTransition_set,
   lTransition_fn
};

const Edje_Lua_Reg *cTransition[] = {
   &mClass,
   &mTransition,
   NULL // sentinel
};

const luaL_Reg lTransition_get[] = {
   {NULL, NULL} // sentinel
};

const luaL_Reg lTransition_set[] = {
   {NULL, NULL} // sentinel
};

const luaL_Reg lTransition_fn[] = {
   {NULL, NULL} // sentinel
};

const luaL_Reg lObject_get[];

const luaL_Reg lObject_set[];

const luaL_Reg lObject_fn[];

const Edje_Lua_Reg mObject = {
   lNil,
   lObject_get,
   lObject_set,
   lObject_fn
};

static void
_edje_lua_object_del_cb(void *data, EINA_UNUSED Evas *e, Evas_Object *obj, EINA_UNUSED void *event_info)
{
   //printf("_edje_lua_object_delete_cb\n");
   lua_State *L = data;
   _edje_lua_get_reg(L, obj);
   Edje_Lua_Evas_Object *udata = _edje_lua_checkudata(L, -1, &mObject);
   lua_pop(L, 1);
   _edje_lua_free_reg(L, udata); // created in EDJE_LUA_SCRIPT_FN_ADD
   _edje_lua_free_reg(L, obj); // created in EDJE_LUA_SCRIPT_FN_ADD

   Edje_Lua_Ref *ref;
   EINA_LIST_FREE(udata->cb, ref)
     _edje_lua_free_ref(L, ref);
}

static int
_edje_lua_object_fn_del(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);

   if (obj->eo)
     {
        evas_object_del(obj->eo);
        obj->eo = NULL;
        obj->ed = NULL;
     }
   return 0;
}

static int
_edje_lua_object_fn_show(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_show(obj->eo);
   return 0;
}

static int
_edje_lua_object_fn_hide(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_hide(obj->eo);
   return 0;
}

static int
_edje_lua_object_fn_move(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   //printf ("%i %i %i %i\n", obj->ed->x, obj->ed->y, luaL_checkint (L, 2), luaL_checkint (L, 3));
   evas_object_move(obj->eo,
                    obj->ed->x + luaL_checkint(L, 2),
                    obj->ed->y + luaL_checkint(L, 3));
   return 0;
}

static int
_edje_lua_object_fn_resize(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_resize(obj->eo, luaL_checkint(L, 2), luaL_checkint(L, 3));
   return 0;
}

static int
_edje_lua_object_fn_raise(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_raise(obj->eo);
   return 0;
}

static int
_edje_lua_object_fn_lower(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_lower(obj->eo);
   return 0;
}

static int
_edje_lua_object_fn_stack_above(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   evas_object_stack_above(obj->eo, tar->eo);
   return 0;
}

static int
_edje_lua_object_fn_stack_below(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   evas_object_stack_below(obj->eo, tar->eo);
   return 0;
}

static int
_edje_lua_object_fn_clip_unset(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_clip_unset(obj->eo);
   return 0;
}

const luaL_Reg lObject_fn[] = {
   {"del", _edje_lua_object_fn_del},
   {"show", _edje_lua_object_fn_show},
   {"hide", _edje_lua_object_fn_hide},
   {"move", _edje_lua_object_fn_move},
   {"resize", _edje_lua_object_fn_resize},
   {"raise", _edje_lua_object_fn_raise},
   {"lower", _edje_lua_object_fn_lower},
   {"stack_above", _edje_lua_object_fn_stack_above},
   {"stack_below", _edje_lua_object_fn_stack_below},
   {"clip_unset", _edje_lua_object_fn_clip_unset},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_object_get_name(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushstring(L, evas_object_name_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_geometry(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   int x, y, w, h;
   evas_object_geometry_get(obj->eo, &x, &y, &w, &h);
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, y);
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_object_get_type(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushstring(L, evas_object_type_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_layer(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushnumber(L, evas_object_layer_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_above(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   Evas_Object *above = evas_object_above_get(obj->eo);
   _edje_lua_get_reg(L, above);
   // TODO create object if it does not already exist?
   return 1;
}

static int
_edje_lua_object_get_below(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   Evas_Object *below = evas_object_below_get(obj->eo);
   _edje_lua_get_reg(L, below);
   // TODO create object if it does not already exist?
   return 1;
}

static int
_edje_lua_object_get_size_hint_min(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   int w, h;
   evas_object_size_hint_combined_min_get(obj->eo, &w, &h);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_object_get_size_hint_max(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   int w, h;
   evas_object_size_hint_max_get(obj->eo, &w, &h);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_object_get_size_hint_request(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   int w, h;
   evas_object_size_hint_request_get(obj->eo, &w, &h);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_object_get_size_hint_aspect(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   Evas_Aspect_Control a;
   int w, h;
   evas_object_size_hint_aspect_get(obj->eo, &a, &w, &h);
   lua_createtable(L, 3, 0);
   lua_pushnumber(L, a);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 3);
   return 1;
}

static int
_edje_lua_object_get_size_hint_align(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   double w, h;
   evas_object_size_hint_align_get(obj->eo, &w, &h);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_object_get_size_hint_weight(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   double w, h;
   evas_object_size_hint_weight_get(obj->eo, &w, &h);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_object_get_size_hint_padding(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   int l, r, t, b;
   evas_object_size_hint_padding_get(obj->eo, &l, &r, &t, &b);
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, l);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, r);
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, t);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, b);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_object_get_visible(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushboolean(L, evas_object_visible_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_render_op(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushnumber(L, evas_object_render_op_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_anti_alias(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushboolean(L, evas_object_anti_alias_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_scale(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushnumber(L, evas_object_scale_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_color(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   int r, g, b, a;
   evas_object_color_get(obj->eo, &r, &g, &b, &a);
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, r);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, g);
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, b);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, a);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_object_get_color_interpolation(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushnumber(L, evas_object_color_interpolation_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_clip(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   Evas_Object *clip = evas_object_clip_get(obj->eo);
   _edje_lua_get_reg(L, clip);
   // TODO create object if it does not already exist?
   return 1;
}

static int
_edje_lua_object_get_clipees(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   Eina_List *clipees = (Eina_List *)evas_object_clipees_get(obj->eo);
   Eina_List *l;
   Evas_Object *clip;
   int i = 1;
   lua_createtable(L, eina_list_count(clipees), 0);
   EINA_LIST_FOREACH(clipees, l, clip)
     {
        _edje_lua_get_reg(L, clip);
        // TODO create object if it does not already exist?
        lua_rawseti(L, -2, i++);
     }
   return 1;
}

static int
_edje_lua_object_get_evas(lua_State *L)
{
   //Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushnil(L);
   // FIXME implement Evas class in the first place?
   return 1;
}

static int
_edje_lua_object_get_pass_events(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushboolean(L, evas_object_pass_events_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_repeat_events(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushboolean(L, evas_object_repeat_events_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_propagate_events(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushboolean(L, evas_object_propagate_events_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_focus(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushboolean(L, evas_object_focus_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_pointer_mode(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushnumber(L, evas_object_pointer_mode_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_precise_is_inside(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushboolean(L, evas_object_precise_is_inside_get(obj->eo));
   return 1;
}

static int
_edje_lua_object_get_mouse_events(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   lua_pushboolean(L, obj->mouse_events);
   return 1;
}

const luaL_Reg lObject_get[] = {
   {"name", _edje_lua_object_get_name},
   {"geometry", _edje_lua_object_get_geometry},
   {"type", _edje_lua_object_get_type},
   {"layer", _edje_lua_object_get_layer},
   {"above", _edje_lua_object_get_above},
   {"below", _edje_lua_object_get_below},
   {"size_hint_min", _edje_lua_object_get_size_hint_min},
   {"size_hint_max", _edje_lua_object_get_size_hint_max},
   {"size_hint_request", _edje_lua_object_get_size_hint_request},
   {"size_hint_aspect", _edje_lua_object_get_size_hint_aspect},
   {"size_hint_align", _edje_lua_object_get_size_hint_align},
   {"size_hint_weight", _edje_lua_object_get_size_hint_weight},
   {"size_hint_padding", _edje_lua_object_get_size_hint_padding},
   {"visible", _edje_lua_object_get_visible},
   {"render_op", _edje_lua_object_get_render_op},
   {"anti_alias", _edje_lua_object_get_anti_alias},
   {"scale", _edje_lua_object_get_scale},
   {"color", _edje_lua_object_get_color},
   {"color_interpolation", _edje_lua_object_get_color_interpolation},
   {"clip", _edje_lua_object_get_clip},
   {"clipees", _edje_lua_object_get_clipees},
   {"evas", _edje_lua_object_get_evas},
   {"pass_events", _edje_lua_object_get_pass_events},
   {"repeat_events", _edje_lua_object_get_repeat_events},
   {"propagate_events", _edje_lua_object_get_propagate_events},
   {"focus", _edje_lua_object_get_focus},
   {"pointer_mode", _edje_lua_object_get_pointer_mode},
   {"precise_is_inside", _edje_lua_object_get_precise_is_inside},
   {"mouse_events", _edje_lua_object_get_mouse_events},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_object_set_name(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_name_set(obj->eo, luaL_checkstring(L, 2));
   return 0;
}

static int
_edje_lua_object_set_layer(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_layer_set(obj->eo, luaL_checkint(L, 2));
   return 0;
}

static int
_edje_lua_object_set_size_hint_min(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   evas_object_size_hint_min_set(obj->eo,
                                 luaL_checkint(L, -2),
                                 luaL_checkint(L, -1));
   return 0;
}

static int
_edje_lua_object_set_size_hint_max(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   evas_object_size_hint_max_set(obj->eo,
                                 luaL_checkint(L, -2),
                                 luaL_checkint(L, -1));
   return 0;
}

static int
_edje_lua_object_set_size_hint_request(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   evas_object_size_hint_request_set(obj->eo,
                                     luaL_checkint(L, -2),
                                     luaL_checkint(L, -1));
   return 0;
}

static int
_edje_lua_object_set_size_hint_aspect(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   evas_object_size_hint_aspect_set(obj->eo,
                                    luaL_checkint(L, -3),
                                    luaL_checkint(L, -2),
                                    luaL_checkint(L, -1));
   return 0;
}

static int
_edje_lua_object_set_size_hint_align(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   evas_object_size_hint_align_set(obj->eo,
                                   luaL_checknumber(L, -2),
                                   luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_object_set_size_hint_weight(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   evas_object_size_hint_weight_set(obj->eo,
                                    luaL_checknumber(L, -2),
                                    luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_object_set_size_hint_padding(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   evas_object_size_hint_padding_set(obj->eo,
                                     luaL_checknumber(L, -4),
                                     luaL_checknumber(L, -3),
                                     luaL_checknumber(L, -2),
                                     luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_object_set_render_op(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_render_op_set(obj->eo, luaL_checkint(L, 2));
   return 0;
}

static int
_edje_lua_object_set_anti_alias(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_anti_alias_set(obj->eo, lua_toboolean(L, 2));
   return 0;
}

static int
_edje_lua_object_set_scale(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_scale_set(obj->eo, luaL_checknumber(L, 2));
   return 0;
}

static int
_edje_lua_object_set_color(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   evas_object_color_set(obj->eo,
                         luaL_checkint(L, -4),
                         luaL_checkint(L, -3),
                         luaL_checkint(L, -2),
                         luaL_checkint(L, -1));
   return 0;
}

static int
_edje_lua_object_set_color_interpolation(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_color_interpolation_set(obj->eo, luaL_checkint(L, 2));
   return 0;
}

static int
_edje_lua_object_set_clip(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   Edje_Lua_Evas_Object *clip = _edje_lua_checkudata(L, 2, &mObject);
   evas_object_clip_set(obj->eo, clip->eo);
   return 0;
}

static int
_edje_lua_object_set_pass_events(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_pass_events_set(obj->eo, lua_toboolean(L, 2));
   return 0;
}

static int
_edje_lua_object_set_repeat_events(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_repeat_events_set(obj->eo, lua_toboolean(L, 2));
   return 0;
}

static int
_edje_lua_object_set_propagate_events(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_propagate_events_set(obj->eo, lua_toboolean(L, 2));
   return 0;
}

static int
_edje_lua_object_set_focus(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_focus_set(obj->eo, lua_toboolean(L, 2));
   return 0;
}

static int
_edje_lua_object_set_pointer_mode(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_pointer_mode_set(obj->eo, luaL_checkint(L, 2));
   return 0;
}

static int
_edje_lua_object_set_precise_is_inside(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_precise_is_inside_set(obj->eo, lua_toboolean(L, 2));
   return 0;
}

#define OBJECT_CB_MACRO(KEY)            \
  lua_State * L = data;                 \
  _edje_lua_get_reg(L, obj);            \
  lua_getfield(L, -1, KEY);             \
  if (lua_type(L, -1) != LUA_TFUNCTION) \
    {                                   \
       lua_pop(L, 2);                   \
       return;                          \
    }                                   \
  lua_insert(L, -2);

static void
_edje_lua_object_cb_mouse_in(void *data, EINA_UNUSED Evas *e, Evas_Object *obj,
                             void *event_info)
{
   OBJECT_CB_MACRO("mouse_in");
   Evas_Event_Mouse_In *ev = event_info;
   int err_code;

   lua_pushnumber(L, ev->output.x);
   lua_pushnumber(L, ev->output.y);
   lua_pushnumber(L, ev->canvas.x);
   lua_pushnumber(L, ev->canvas.y);

   if ((err_code = lua_pcall(L, 5, 0, 0)))
     _edje_lua_error(L, err_code);
}

static void
_edje_lua_object_cb_mouse_out(void *data, EINA_UNUSED Evas *e, Evas_Object *obj,
                              void *event_info)
{
   OBJECT_CB_MACRO("mouse_out");
   Evas_Event_Mouse_In *ev = event_info;
   int err_code;

   lua_pushnumber(L, ev->output.x);
   lua_pushnumber(L, ev->output.y);
   lua_pushnumber(L, ev->canvas.x);
   lua_pushnumber(L, ev->canvas.y);

   if ((err_code = lua_pcall(L, 5, 0, 0)))
     _edje_lua_error(L, err_code);
}

static void
_edje_lua_object_cb_mouse_down(void *data, EINA_UNUSED Evas *e, Evas_Object *obj,
                               void *event_info)
{
   OBJECT_CB_MACRO("mouse_down");
   Evas_Event_Mouse_Down *ev = event_info;
   int err_code;

   lua_pushnumber(L, ev->button);
   lua_pushnumber(L, ev->output.x);
   lua_pushnumber(L, ev->output.y);
   lua_pushnumber(L, ev->canvas.x);
   lua_pushnumber(L, ev->canvas.y);

   if ((err_code = lua_pcall(L, 6, 0, 0)))
     _edje_lua_error(L, err_code);
}

static void
_edje_lua_object_cb_mouse_up(void *data, EINA_UNUSED Evas *e, Evas_Object *obj,
                             void *event_info)
{
   OBJECT_CB_MACRO("mouse_up");
   Evas_Event_Mouse_Up *ev = event_info;
   int err_code;

   lua_pushnumber(L, ev->button);
   lua_pushnumber(L, ev->output.x);
   lua_pushnumber(L, ev->output.y);
   lua_pushnumber(L, ev->canvas.x);
   lua_pushnumber(L, ev->canvas.y);

   if ((err_code = lua_pcall(L, 6, 0, 0)))
     _edje_lua_error(L, err_code);
}

static void
_edje_lua_object_cb_mouse_move(void *data, EINA_UNUSED Evas *e, Evas_Object *obj,
                               void *event_info)
{
   OBJECT_CB_MACRO("mouse_move");
   Evas_Event_Mouse_Move *ev = event_info;
   int err_code;

   lua_pushnumber(L, ev->buttons);
   lua_pushnumber(L, ev->cur.output.x);
   lua_pushnumber(L, ev->cur.output.y);
   lua_pushnumber(L, ev->cur.canvas.x);
   lua_pushnumber(L, ev->cur.canvas.y);

   if ((err_code = lua_pcall(L, 6, 0, 0)))
     _edje_lua_error(L, err_code);
}

static void
_edje_lua_object_cb_mouse_wheel(void *data, EINA_UNUSED Evas *e, Evas_Object *obj,
                                void *event_info)
{
   OBJECT_CB_MACRO("mouse_wheel");
   Evas_Event_Mouse_Wheel *ev = event_info;
   int err_code;

   lua_pushnumber(L, ev->z);
   lua_pushnumber(L, ev->output.x);
   lua_pushnumber(L, ev->output.y);
   lua_pushnumber(L, ev->canvas.x);
   lua_pushnumber(L, ev->canvas.y);

   if ((err_code = lua_pcall(L, 6, 0, 0)))
     _edje_lua_error(L, err_code);
}

static int
_edje_lua_object_set_mouse_events(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   obj->mouse_events = lua_toboolean(L, 2);
   if (obj->mouse_events)
     {
        // add all mouse events
        evas_object_event_callback_add(obj->eo, EVAS_CALLBACK_MOUSE_IN, _edje_lua_object_cb_mouse_in, L);
        evas_object_event_callback_add(obj->eo, EVAS_CALLBACK_MOUSE_OUT, _edje_lua_object_cb_mouse_out, L);
        evas_object_event_callback_add(obj->eo, EVAS_CALLBACK_MOUSE_DOWN, _edje_lua_object_cb_mouse_down, L);
        evas_object_event_callback_add(obj->eo, EVAS_CALLBACK_MOUSE_UP, _edje_lua_object_cb_mouse_up, L);
        evas_object_event_callback_add(obj->eo, EVAS_CALLBACK_MOUSE_MOVE, _edje_lua_object_cb_mouse_move, L);
        evas_object_event_callback_add(obj->eo, EVAS_CALLBACK_MOUSE_WHEEL, _edje_lua_object_cb_mouse_wheel, L);
     }
   else
     {
        // delete all mouse events
        evas_object_event_callback_del(obj->eo, EVAS_CALLBACK_MOUSE_IN, _edje_lua_object_cb_mouse_in);
        evas_object_event_callback_del(obj->eo, EVAS_CALLBACK_MOUSE_OUT, _edje_lua_object_cb_mouse_out);
        evas_object_event_callback_del(obj->eo, EVAS_CALLBACK_MOUSE_DOWN, _edje_lua_object_cb_mouse_down);
        evas_object_event_callback_del(obj->eo, EVAS_CALLBACK_MOUSE_UP, _edje_lua_object_cb_mouse_up);
        evas_object_event_callback_del(obj->eo, EVAS_CALLBACK_MOUSE_MOVE, _edje_lua_object_cb_mouse_move);
        evas_object_event_callback_del(obj->eo, EVAS_CALLBACK_MOUSE_WHEEL, _edje_lua_object_cb_mouse_wheel);
     }
   return 0;
}

const luaL_Reg lObject_set[] = {
   {"name", _edje_lua_object_set_name},
   {"layer", _edje_lua_object_set_layer},
   {"size_hint_min", _edje_lua_object_set_size_hint_min},
   {"size_hint_max", _edje_lua_object_set_size_hint_max},
   {"size_hint_request", _edje_lua_object_set_size_hint_request},
   {"size_hint_aspect", _edje_lua_object_set_size_hint_aspect},
   {"size_hint_align", _edje_lua_object_set_size_hint_align},
   {"size_hint_weight", _edje_lua_object_set_size_hint_weight},
   {"size_hint_padding", _edje_lua_object_set_size_hint_padding},
   {"render_op", _edje_lua_object_set_render_op},
   {"anti_alias", _edje_lua_object_set_anti_alias},
   {"scale", _edje_lua_object_set_scale},
   {"color", _edje_lua_object_set_color},
   {"color_interpolation", _edje_lua_object_set_color_interpolation},
   {"clip", _edje_lua_object_set_clip},
   {"pass_events", _edje_lua_object_set_pass_events},
   {"repeat_events", _edje_lua_object_set_repeat_events},
   {"propagate_events", _edje_lua_object_set_propagate_events},
   {"focus", _edje_lua_object_set_focus},
   {"pointer_mode", _edje_lua_object_get_pointer_mode},
   {"precise_is_inside", _edje_lua_object_get_precise_is_inside},
   {"mouse_events", _edje_lua_object_set_mouse_events},
   {NULL, NULL} // sentinel
};

const Edje_Lua_Reg *cRectangle[] = {
   &mClass,
   &mObject,
   NULL // sentinel
};

const luaL_Reg lImage_get[];

const luaL_Reg lImage_set[];

const Edje_Lua_Reg mImage = {
   lNil,
   lImage_get,
   lImage_set,
   lNil
};

const Edje_Lua_Reg *cImage[] = {
   &mClass,
   &mObject,
   &mImage,
   NULL // sentinel
};

static int
_edje_lua_image_get_size(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mImage);
   int w, h;
   evas_object_image_size_get(obj->eo, &w, &h);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 2);
   return 1;
}

const luaL_Reg lImage_get[] = {
   {"size", _edje_lua_image_get_size},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_image_set_file(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mImage);
   int id = edje_edit_image_id_get(obj->ed->obj, luaL_checkstring(L, 2));
   char buf[256];
   sprintf(buf, "edje/images/%i", id);
   evas_object_image_file_set(obj->eo, obj->ed->path, buf);
   return 0;
}

static int
_edje_lua_image_set_fill(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mImage);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   evas_object_image_fill_set(obj->eo,
                              luaL_checkint(L, -4),
                              luaL_checkint(L, -3),
                              luaL_checkint(L, -2), luaL_checkint(L, -1));
   return 0;
}

static int
_edje_lua_image_set_fill_transform(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mImage);
   Edje_Lua_Transform *tar = _edje_lua_checkudata(L, 2, &mTransform);
   evas_object_image_fill_transform_set(obj->eo, &(tar->et));
   return 0;
}

static int
_edje_lua_image_set_alpha(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mObject);
   evas_object_image_alpha_set(obj->eo, lua_toboolean(L, 2));
   return 0;
}

const luaL_Reg lImage_set[] = {
   {"file", _edje_lua_image_set_file},
   {"fill", _edje_lua_image_set_fill},
   {"fill_transform", _edje_lua_image_set_fill_transform},
   {"alpha", _edje_lua_image_set_alpha},
   {NULL, NULL} // sentinel
};

const luaL_Reg lLine_get[];
const luaL_Reg lLine_set[];

const Edje_Lua_Reg mLine = {
   lNil,
   lLine_get,
   lLine_set,
   lNil
};

const Edje_Lua_Reg *cLine[] = {
   &mClass,
   &mObject,
   &mLine,
   NULL // sentinel
};

static int
_edje_lua_line_get_xy(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mLine);
   int x1, y1, x2, y2;
   evas_object_line_xy_get(obj->eo, &x1, &y1, &x2, &y2);
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, x1); lua_rawseti(L, -2, 1);
   lua_pushnumber(L, y1); lua_rawseti(L, -2, 2);
   lua_pushnumber(L, x2); lua_rawseti(L, -2, 3);
   lua_pushnumber(L, y2); lua_rawseti(L, -2, 4);
   return 1;
}

const luaL_Reg lLine_get[] = {
   {"xy", _edje_lua_line_get_xy},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_line_set_xy(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mLine);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   evas_object_line_xy_set(obj->eo,
                           luaL_checkint(L, -4),
                           luaL_checkint(L, -3),
                           luaL_checkint(L, -2),
                           luaL_checkint(L, -1));
   return 0;
}

const luaL_Reg lLine_set[] = {
   {"xy", _edje_lua_line_set_xy},
   {NULL, NULL} // sentinel
};

const luaL_Reg lPolygon_fn[];

const Edje_Lua_Reg mPolygon = {
   lNil,
   lNil,
   lNil,
   lPolygon_fn
};

const Edje_Lua_Reg *cPolygon[] = {
   &mClass,
   &mObject,
   &mPolygon,
   NULL // sentinel
};

static int
_edje_lua_polygon_fn_point_add(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mPolygon);
   evas_object_polygon_point_add(obj->eo,
                                 luaL_checknumber(L, 2),
                                 luaL_checknumber(L, 3));
   return 0;
}

static int
_edje_lua_polygon_fn_points_clear(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mPolygon);
   evas_object_polygon_points_clear(obj->eo);
   return 0;
}

const luaL_Reg lPolygon_fn[] = {
   {"point_add", _edje_lua_polygon_fn_point_add},
   {"points_clear", _edje_lua_polygon_fn_points_clear},
   {NULL, NULL} // sentinel
};

const luaL_Reg lTable_get[];

const luaL_Reg lTable_set[];

const luaL_Reg lTable_fn[];

const Edje_Lua_Reg mTable = {
   lNil,
   lTable_get,
   lTable_set,
   lTable_fn
};

const Edje_Lua_Reg *cTable[] = {
   &mClass,
   &mObject,
   &mTable,
   NULL // sentinel
};

static int
_edje_lua_table_get_homogeneous(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   lua_pushnumber(L, evas_object_table_homogeneous_get(obj->eo));
   return 1;
}

static int
_edje_lua_table_get_padding(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   int x, y;
   evas_object_table_padding_get(obj->eo, &x, &y);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_table_get_align(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   double x, y;
   evas_object_table_align_get(obj->eo, &x, &y);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_table_get_col_row_size(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   int x, y;
   evas_object_table_col_row_size_get(obj->eo, &x, &y);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_table_get_children(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   Eina_List *children = (Eina_List *)evas_object_table_children_get(obj->eo);
   Eina_List *l;
   Evas_Object *child;
   int i = 1;
   lua_createtable(L, eina_list_count(children), 0);
   EINA_LIST_FOREACH(children, l, child)
     {
        _edje_lua_get_reg(L, child);
        // TODO create object if it does not already exist?
        lua_rawseti(L, -2, i++);
     }
   return 1;
}

const luaL_Reg lTable_get[] = {
   {"homogeneous", _edje_lua_table_get_homogeneous},
   {"padding", _edje_lua_table_get_padding},
   {"align", _edje_lua_table_get_align},
   {"col_row_size", _edje_lua_table_get_col_row_size},
   {"children", _edje_lua_table_get_children},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_table_set_homogeneous(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   evas_object_table_homogeneous_set(obj->eo,
                                     luaL_checkint(L, 2));
   return 0;
}

static int
_edje_lua_table_set_padding(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   evas_object_table_padding_set(obj->eo,
                                 luaL_checkint(L, -2),
                                 luaL_checkint(L, -1));
   return 0;
}

static int
_edje_lua_table_set_align(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   evas_object_table_align_set(obj->eo,
                               luaL_checknumber(L, -2),
                               luaL_checknumber(L, -1));
   return 0;
}

const luaL_Reg lTable_set[] = {
   {"homogeneous", _edje_lua_table_set_homogeneous},
   {"padding", _edje_lua_table_set_padding},
   {"align", _edje_lua_table_set_align},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_table_fn_pack(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   lua_pushboolean(L,
                   evas_object_table_pack(obj->eo, tar->eo,
                                          luaL_checkint(L, 3),
                                          luaL_checkint(L, 4),
                                          luaL_checkint(L, 5),
                                          luaL_checkint(L, 6)));
   return 1;
}

static int
_edje_lua_table_fn_unpack(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   lua_pushboolean(L,
                   evas_object_table_unpack(obj->eo, tar->eo));
   return 1;
}

static int
_edje_lua_table_fn_clear(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mTable);
   evas_object_table_clear(obj->eo, lua_toboolean(L, 2));
   return 0;
}

const luaL_Reg lTable_fn[] = {
   {"pack", _edje_lua_table_fn_pack},
   {"unpack", _edje_lua_table_fn_unpack},
   {"clear", _edje_lua_table_fn_clear},
   {NULL, NULL} // sentinel
};

const luaL_Reg lDescription_get[];

const luaL_Reg lDescription_set[];

const Edje_Lua_Reg mDescription = {
   lNil,
   lDescription_get,
   lDescription_set,
   lNil
};

const Edje_Lua_Reg *cDescription[] = {
   &mClass,
   &mObject,
   &mDescription,
   NULL // sentinel
};

static int
_edje_lua_description_get_alignment(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->align.x));
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->align.y));
   lua_rawseti(L, -2, 1);
   return 1;
}

static int
_edje_lua_description_get_min(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, obj->rp->custom->description->min.w);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->min.h);
   lua_rawseti(L, -2, 1);
   return 1;
}

static int
_edje_lua_description_get_max(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, obj->rp->custom->description->max.w);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->max.h);
   lua_rawseti(L, -2, 1);
   return 1;
}

static int
_edje_lua_description_get_step(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, obj->rp->custom->description->step.x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->step.y);
   lua_rawseti(L, -2, 1);
   return 1;
}

static int
_edje_lua_description_get_aspect(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->aspect.min));
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->aspect.max));
   lua_rawseti(L, -2, 1);
   return 1;
}

static int
_edje_lua_description_get_aspect_pref(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_pushnumber(L, obj->rp->custom->description->aspect.prefer);
   return 1;
}

static int
_edje_lua_description_get_color(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   /* check whether this part has a "custom" state */
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, obj->rp->custom->description->color.r);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->color.g);
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, obj->rp->custom->description->color.b);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, obj->rp->custom->description->color.a);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_description_get_color2(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, obj->rp->custom->description->color2.r);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->color2.g);
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, obj->rp->custom->description->color2.b);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, obj->rp->custom->description->color2.a);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_description_get_color3(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, obj->rp->custom->description->color3.r);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->color3.g);
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, obj->rp->custom->description->color3.b);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, obj->rp->custom->description->color3.a);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_description_get_color_class(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_pushstring(L, obj->rp->custom->description->color_class);
   return 1;
}

static int
_edje_lua_description_get_rel1(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->rel1.relative_x));
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->rel1.relative_y));
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_description_get_rel1_to(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, obj->rp->custom->description->rel1.id_x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->rel1.id_y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_description_get_rel1_offset(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, obj->rp->custom->description->rel1.offset_x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->rel1.offset_y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_description_get_rel2(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->rel2.relative_x));
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->rel2.relative_y));
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_description_get_rel2_to(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, obj->rp->custom->description->rel2.id_x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->rel2.id_y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_description_get_rel2_offset(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, obj->rp->custom->description->rel2.offset_x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->rel2.offset_y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_description_get_image(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   if ((obj->rp->part->type != EDJE_PART_TYPE_IMAGE))
     return 0;
   // TODO
   return 0;
}

static int
_edje_lua_description_get_border(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_IMAGE))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, obj->rp->custom->description->border.l);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->border.r);
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, obj->rp->custom->description->border.t);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, obj->rp->custom->description->border.b);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_description_get_fill_smooth(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_IMAGE))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_pushboolean(L, obj->rp->custom->description->fill.smooth);
   return 1;
}

static int
_edje_lua_description_get_fill_pos(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_IMAGE))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->fill.pos_rel_x));
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->fill.pos_rel_y));
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, obj->rp->custom->description->fill.pos_abs_x);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, obj->rp->custom->description->fill.pos_abs_y);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_description_get_fill_size(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_IMAGE))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->fill.rel_x));
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->fill.rel_y));
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, obj->rp->custom->description->fill.abs_x);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, obj->rp->custom->description->fill.abs_y);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_description_get_text(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT)
       && (obj->rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_pushstring(L, obj->rp->custom->description->text.text);
   return 1;
}

static int
_edje_lua_description_get_text_class(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT)
       && (obj->rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_pushstring(L, obj->rp->custom->description->text.text_class);
   return 1;
}

static int
_edje_lua_description_get_text_font(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_pushstring(L, obj->rp->custom->description->text.text_class);
   return 1;
}

static int
_edje_lua_description_get_text_style(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_pushstring(L, obj->rp->custom->description->text.style);
   return 1;
}

static int
_edje_lua_description_get_text_size(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_pushnumber(L, obj->rp->custom->description->text.size);
   return 1;
}

static int
_edje_lua_description_get_text_fit(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushboolean(L, obj->rp->custom->description->text.fit_x);
   lua_rawseti(L, -2, 1);
   lua_pushboolean(L, obj->rp->custom->description->text.fit_y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_description_get_text_min(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT)
       && (obj->rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, obj->rp->custom->description->text.min_x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->text.min_y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_description_get_text_max(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT)
       && (obj->rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, obj->rp->custom->description->text.max_x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, obj->rp->custom->description->text.max_y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_description_get_text_align(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT))
     return 0;
   if (!obj->rp->custom) return 0;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->text.align.x));
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, TO_DOUBLE(obj->rp->custom->description->text.align.y));
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_description_get_visible(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_pushboolean(L, obj->rp->custom->description->visible);
   return 1;
}

const luaL_Reg lDescription_get[] = {
   {"alignment", _edje_lua_description_get_alignment},
   {"min", _edje_lua_description_get_min},
   {"max", _edje_lua_description_get_max},
   {"step", _edje_lua_description_get_step},
   {"aspect", _edje_lua_description_get_aspect},
   {"aspect_pref", _edje_lua_description_get_aspect_pref},
   {"color", _edje_lua_description_get_color},
   {"color2", _edje_lua_description_get_color2},
   {"color3", _edje_lua_description_get_color3},
   {"color_class", _edje_lua_description_get_color_class},
   {"rel1", _edje_lua_description_get_rel1},
   {"rel1_to", _edje_lua_description_get_rel1_to},
   {"rel1_offset", _edje_lua_description_get_rel1_offset},
   {"rel2", _edje_lua_description_get_rel2},
   {"rel2_to", _edje_lua_description_get_rel2_to},
   {"rel2_offset", _edje_lua_description_get_rel2_offset},
   {"image", _edje_lua_description_get_image},
   {"border", _edje_lua_description_get_border},
   {"fill_smooth", _edje_lua_description_get_fill_smooth},
   {"fill_pos", _edje_lua_description_get_fill_pos},
   {"fill_size", _edje_lua_description_get_fill_size},
   {"text", _edje_lua_description_get_text},
   {"text_class", _edje_lua_description_get_text_class},
   {"text_font", _edje_lua_description_get_text_font},
   {"text_style", _edje_lua_description_get_text_style},
   {"text_size", _edje_lua_description_get_text_size},
   {"text_fit", _edje_lua_description_get_text_fit},
   {"text_min", _edje_lua_description_get_text_min},
   {"text_max", _edje_lua_description_get_text_max},
   {"text_align", _edje_lua_description_get_text_align},
   {"visible", _edje_lua_description_get_visible},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_description_set_alignment(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->align.x = FROM_DOUBLE(luaL_checknumber(L, -2));
   obj->rp->custom->description->align.y = FROM_DOUBLE(luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_description_set_min(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->min.w = luaL_checkint(L, -2);
   obj->rp->custom->description->min.h = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_max(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->max.w = luaL_checkint(L, -2);
   obj->rp->custom->description->max.h = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_step(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->step.x = luaL_checkint(L, -2);
   obj->rp->custom->description->step.y = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_aspect(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->aspect.min = FROM_DOUBLE(luaL_checknumber(L, -2));
   obj->rp->custom->description->aspect.max = FROM_DOUBLE(luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_description_set_aspect_pref(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   obj->rp->custom->description->aspect.prefer = luaL_checknumber(L, 2);
   return 0;
}

static int
_edje_lua_description_set_color(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   obj->rp->custom->description->color.r = luaL_checkint(L, -4);
   obj->rp->custom->description->color.g = luaL_checkint(L, -3);
   obj->rp->custom->description->color.b = luaL_checkint(L, -2);
   obj->rp->custom->description->color.a = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_color2(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   obj->rp->custom->description->color2.r = luaL_checkint(L, -4);
   obj->rp->custom->description->color2.g = luaL_checkint(L, -3);
   obj->rp->custom->description->color2.b = luaL_checkint(L, -2);
   obj->rp->custom->description->color2.a = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_color3(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   obj->rp->custom->description->color3.r = luaL_checkint(L, -4);
   obj->rp->custom->description->color3.g = luaL_checkint(L, -3);
   obj->rp->custom->description->color3.b = luaL_checkint(L, -2);
   obj->rp->custom->description->color3.a = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_color_class(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   obj->rp->custom->description->color_class = (char *)luaL_checkstring(L, 2);
   return 0;
}

static int
_edje_lua_description_set_rel1(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->rel1.relative_x = FROM_DOUBLE(luaL_checknumber(L, -2));
   obj->rp->custom->description->rel1.relative_y = FROM_DOUBLE(luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_description_set_rel1_to(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->rel1.id_x = luaL_checknumber(L, -2);
   obj->rp->custom->description->rel1.id_y = luaL_checknumber(L, -1);
   if (obj->rp->param1.description->rel1.id_x >= 0)
     obj->rp->param1.rel1_to_x = obj->ed->table_parts[obj->rp->param1.description->rel1.id_x % obj->ed->table_parts_size];
   if (obj->rp->param1.description->rel1.id_y >= 0)
     obj->rp->param1.rel1_to_y = obj->ed->table_parts[obj->rp->param1.description->rel1.id_y % obj->ed->table_parts_size];
   return 0;
}

static int
_edje_lua_description_set_rel1_offset(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->rel1.offset_x = luaL_checkint(L, -2);
   obj->rp->custom->description->rel1.offset_y = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_rel2(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->rel2.relative_x = FROM_DOUBLE(luaL_checknumber(L, -2));
   obj->rp->custom->description->rel2.relative_y = FROM_DOUBLE(luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_description_set_rel2_to(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->rel2.id_x = luaL_checknumber(L, -2);
   obj->rp->custom->description->rel2.id_y = luaL_checknumber(L, -1);
   if (obj->rp->param1.description->rel2.id_x >= 0)
     obj->rp->param1.rel2_to_x = obj->ed->table_parts[obj->rp->param1.description->rel2.id_x % obj->ed->table_parts_size];
   if (obj->rp->param1.description->rel2.id_y >= 0)
     obj->rp->param1.rel2_to_y = obj->ed->table_parts[obj->rp->param1.description->rel2.id_y % obj->ed->table_parts_size];
   return 0;
}

static int
_edje_lua_description_set_rel2_offset(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->rel2.offset_x = luaL_checkint(L, -2);
   obj->rp->custom->description->rel2.offset_y = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_image(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   if ((obj->rp->part->type != EDJE_PART_TYPE_IMAGE))
     return 0;
   obj->rp->custom->description->image.id =
     edje_edit_image_id_get(obj->ed->obj, (char *)luaL_checkstring(L, 2));
   return 0;
}

static int
_edje_lua_description_set_border(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_IMAGE))
     return 0;
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   obj->rp->custom->description->border.l = luaL_checkint(L, -2);
   obj->rp->custom->description->border.r = luaL_checkint(L, -1);
   obj->rp->custom->description->border.t = luaL_checkint(L, -2);
   obj->rp->custom->description->border.b = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_fill_smooth(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_IMAGE))
     return 0;
   if (!obj->rp->custom) return 0;
   obj->rp->custom->description->fill.smooth = lua_toboolean(L, 2);
   return 0;
}

static int
_edje_lua_description_set_fill_pos(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_IMAGE))
     return 0;
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   obj->rp->custom->description->fill.pos_rel_x = luaL_checkint(L, -2);
   obj->rp->custom->description->fill.pos_rel_y = luaL_checkint(L, -1);
   obj->rp->custom->description->fill.pos_abs_x = luaL_checkint(L, -2);
   obj->rp->custom->description->fill.pos_abs_y = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_fill_size(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_IMAGE))
     return 0;
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   obj->rp->custom->description->fill.rel_x = luaL_checkint(L, -2);
   obj->rp->custom->description->fill.rel_y = luaL_checkint(L, -1);
   obj->rp->custom->description->fill.abs_x = luaL_checkint(L, -2);
   obj->rp->custom->description->fill.abs_y = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_text(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT)
       && (obj->rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;
   if (!obj->rp->custom) return 0;
   obj->rp->custom->description->text.text = (char *)luaL_checkstring(L, 2);
   return 0;
}

static int
_edje_lua_description_set_text_class(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT)
       && (obj->rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;
   if (!obj->rp->custom) return 0;
   obj->rp->custom->description->text.text_class =
     (char *)luaL_checkstring(L, 2);
   return 0;
}

static int
_edje_lua_description_set_text_font(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT))
     return 0;
   if (!obj->rp->custom) return 0;
   obj->rp->custom->description->text.font = (char *)luaL_checkstring(L, 2);
   return 0;
}

static int
_edje_lua_description_set_text_style(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;
   if (!obj->rp->custom) return 0;
   obj->rp->custom->description->text.style = (char *)luaL_checkstring(L, 2);
   return 0;
}

static int
_edje_lua_description_set_text_size(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT))
     return 0;
   if (!obj->rp->custom) return 0;
   obj->rp->custom->description->text.size = luaL_checkint(L, 2);
   return 0;
}

static int
_edje_lua_description_set_text_fit(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->text.fit_x = lua_toboolean(L, -2);
   obj->rp->custom->description->text.fit_y = lua_toboolean(L, -1);
   return 0;
}

static int
_edje_lua_description_set_text_min(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT)
       && (obj->rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->text.min_x = luaL_checkint(L, -2);
   obj->rp->custom->description->text.min_y = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_text_max(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT)
       && (obj->rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
     return 0;
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->text.max_x = luaL_checkint(L, -2);
   obj->rp->custom->description->text.max_y = luaL_checkint(L, -1);
   return 0;
}

static int
_edje_lua_description_set_text_align(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if ((obj->rp->part->type != EDJE_PART_TYPE_TEXT))
     return 0;
   if (!obj->rp->custom) return 0;
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->rp->custom->description->text.align.x = FROM_DOUBLE(luaL_checknumber(L, -2));
   obj->rp->custom->description->text.align.y = FROM_DOUBLE(luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_description_set_visible(lua_State *L)
{
   Edje_Lua_Edje_Part_Description *obj =
     _edje_lua_checkudata(L, 1, &mDescription);
   if (!obj->rp->custom) return 0;
   obj->rp->custom->description->visible = lua_toboolean(L, 2);
   return 0;
}

const luaL_Reg lDescription_set[] = {
   {"alignment", _edje_lua_description_set_alignment},
   {"min", _edje_lua_description_set_min},
   {"max", _edje_lua_description_set_max},
   {"step", _edje_lua_description_set_step},
   {"aspect", _edje_lua_description_set_aspect},
   {"aspect_pref", _edje_lua_description_set_aspect_pref},
   {"color", _edje_lua_description_set_color},
   {"color2", _edje_lua_description_set_color2},
   {"color3", _edje_lua_description_set_color3},
   {"color_class", _edje_lua_description_set_color_class},
   {"rel1", _edje_lua_description_set_rel1},
   {"rel1_to", _edje_lua_description_set_rel1_to},
   {"rel1_offset", _edje_lua_description_set_rel1_offset},
   {"rel2", _edje_lua_description_set_rel2},
   {"rel2_to", _edje_lua_description_set_rel2_to},
   {"rel2_offset", _edje_lua_description_set_rel2_offset},
   {"image", _edje_lua_description_set_image},
   {"border", _edje_lua_description_set_border},
   {"fill_smooth", _edje_lua_description_set_fill_smooth},
   {"fill_pos", _edje_lua_description_set_fill_pos},
   {"fill_size", _edje_lua_description_set_fill_size},
   {"text", _edje_lua_description_set_text},
   {"text_class", _edje_lua_description_set_text_class},
   {"text_font", _edje_lua_description_set_text_font},
   {"text_style", _edje_lua_description_set_text_style},
   {"text_size", _edje_lua_description_set_text_size},
   {"text_fit", _edje_lua_description_set_text_fit},
   {"text_min", _edje_lua_description_set_text_min},
   {"text_max", _edje_lua_description_set_text_max},
   {"text_align", _edje_lua_description_set_text_align},
   {"visible", _edje_lua_description_set_visible},
   {NULL, NULL} // sentinel
};

const luaL_Reg lPart_get[];

const luaL_Reg lPart_set[];

const luaL_Reg lPart_fn[];

const Edje_Lua_Reg mPart = {
   lNil,
   lPart_get,
   lPart_set,
   lPart_fn
};

static void
_edje_lua_edje_part_del_cb(void *data, EINA_UNUSED Evas *e, Evas_Object *obj,
                           EINA_UNUSED void *event_info)
{
   //printf("_edje_lua_object_delete_cb\n");
   lua_State *L = data;
   _edje_lua_get_reg(L, obj);
   Edje_Lua_Edje_Part_Object *udata = _edje_lua_checkudata(L, -1, &mPart);
   lua_pop(L, 1);
   _edje_lua_free_reg(L, udata); // created in _edje_lua_group_mt_index
   _edje_lua_free_reg(L, obj); // created in _edje_lua_group_mt_index
}

const Edje_Lua_Reg *cPart[] = {
   &mClass,
   &mObject,
   &mPart,
   NULL // sentinel
};

static int
_edje_lua_part_get_swallow(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Evas_Object *swa = edje_object_part_swallow_get(obj->ed->obj, obj->key);
   _edje_lua_get_reg(L, swa);
   // TODO create object if it does not already exist?
   return 1;
}

static int
_edje_lua_part_get_type(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_pushnumber(L, edje_edit_part_type_get(obj->ed->obj, obj->key));
   return 1;
}

static int
_edje_lua_part_get_effect(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_pushnumber(L, edje_edit_part_effect_get(obj->ed->obj, obj->key));
   return 1;
}

static int
_edje_lua_part_get_mouse_events(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_pushboolean(L, edje_edit_part_mouse_events_get(obj->ed->obj, obj->key));
   return 1;
}

static int
_edje_lua_part_get_repeat_events(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_pushboolean(L, edje_edit_part_repeat_events_get(obj->ed->obj, obj->key));
   return 1;
}

static int
_edje_lua_part_get_states_list(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Eina_List *slist = edje_edit_part_states_list_get(obj->ed->obj, obj->key);
   Eina_List *ilist;
   lua_newtable(L);
   int i = 1;
   for (ilist = slist; ilist; ilist = eina_list_next(ilist))
     {
        lua_pushstring(L, eina_list_data_get(ilist));
        lua_rawseti(L, -2, i++);
     }
   edje_edit_string_list_free(slist);
   return 1;
}

static int
_edje_lua_part_get_state(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   const char *state;
   double val;
   state = edje_object_part_state_get(obj->ed->obj, obj->key, &val);
   lua_createtable(L, 2, 0);
   lua_pushstring(L, state);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, val);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_part_get_text(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_pushstring(L, edje_object_part_text_get(obj->ed->obj, obj->key));
   return 1;
}

static int
_edje_lua_part_get_text_selection(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_pushstring(L,
                  edje_object_part_text_selection_get(obj->ed->obj, obj->key));
   return 1;
}

static int
_edje_lua_part_get_drag_dir(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_pushnumber(L, edje_object_part_drag_dir_get(obj->ed->obj, obj->key));
   return 1;
}

static int
_edje_lua_part_get_drag_value(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   double dx, dy;
   edje_object_part_drag_value_get(obj->ed->obj, obj->key, &dx, &dy);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, dx);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, dy);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_part_get_drag_size(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   double dx, dy;
   edje_object_part_drag_size_get(obj->ed->obj, obj->key, &dx, &dy);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, dx);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, dy);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_part_get_drag_step(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   double dx, dy;
   edje_object_part_drag_step_get(obj->ed->obj, obj->key, &dx, &dy);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, dx);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, dy);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_part_get_drag_page(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   double dx, dy;
   edje_object_part_drag_page_get(obj->ed->obj, obj->key, &dx, &dy);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, dx);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, dy);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_part_get_text_cursor_geometry(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Evas_Coord x, y, w, h;
   edje_object_part_text_cursor_geometry_get(obj->ed->obj, obj->key,
                                             &x, &y, &w, &h);
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, y);
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_part_get_geometry(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Evas_Coord x, y, w, h;
   edje_object_part_geometry_get(obj->ed->obj, obj->key, &x, &y, &w, &h);
   lua_createtable(L, 4, 0);
   lua_pushnumber(L, x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, y);
   lua_rawseti(L, -2, 2);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 3);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 4);
   return 1;
}

static int
_edje_lua_part_get_table_col_row_size(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   int cols, rows;
   edje_object_part_table_col_row_size_get(obj->ed->obj, obj->key, &cols, &rows);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, cols); lua_rawseti(L, -2, 1);
   lua_pushnumber(L, rows); lua_rawseti(L, -2, 2);
   return 1;
}

static int _edje_lua_part_fn_custom_state(lua_State *L);

const luaL_Reg lPart_get[] = {
   {"custom_state", _edje_lua_part_fn_custom_state},
   {"Swallow", _edje_lua_part_get_swallow}, //TODO it the capital S correct?

   {"drag_dir", _edje_lua_part_get_drag_dir},
   {"drag_value", _edje_lua_part_get_drag_value},
   {"drag_size", _edje_lua_part_get_drag_size},
   {"drag_step", _edje_lua_part_get_drag_step},
   {"drag_page", _edje_lua_part_get_drag_page},

   {"type", _edje_lua_part_get_type},
   {"effect", _edje_lua_part_get_effect},
   {"mouse_events", _edje_lua_part_get_mouse_events},
   {"repeat_events", _edje_lua_part_get_repeat_events},
   {"states_list", _edje_lua_part_get_states_list},
   {"state", _edje_lua_part_get_state},

   {"text", _edje_lua_part_get_text},
   {"text_selection", _edje_lua_part_get_text_selection},
   {"text_cursor_geometry", _edje_lua_part_get_text_cursor_geometry},

   {"geometry", _edje_lua_part_get_geometry},
   {"part_col_row_size", _edje_lua_part_get_table_col_row_size},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_part_set_effect(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   edje_edit_part_effect_set(obj->ed->obj, obj->key, luaL_checkint(L, 2));
   return 0;
}

static int
_edje_lua_part_set_mouse_events(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   edje_edit_part_mouse_events_set(obj->ed->obj, obj->key, lua_toboolean(L, 2));
   return 0;
}

static int
_edje_lua_part_set_repeat_events(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   edje_edit_part_repeat_events_set(obj->ed->obj, obj->key,
                                    lua_toboolean(L, 2));
   return 0;
}

static int
_edje_lua_part_set_state(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   _edje_part_description_apply(obj->ed, obj->rp,
                                luaL_checkstring(L, -2), luaL_checknumber(L, -1),
                                NULL, 0.0);
   _edje_part_pos_set(obj->ed, obj->rp, EDJE_TWEEN_MODE_LINEAR, ZERO,
                      ZERO, ZERO, ZERO, ZERO);
   _edje_recalc(obj->ed);
   return 0;
}

static int
_edje_lua_part_set_tween_state(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   lua_rawgeti(L, 2, 3);
   lua_rawgeti(L, 2, 4);
   lua_rawgeti(L, 2, 5);
   _edje_part_description_apply(obj->ed, obj->rp,
                                luaL_checkstring(L, -4), luaL_checknumber(L, -3),
                                luaL_checkstring(L, -2), luaL_checknumber(L, -1));
   _edje_part_pos_set(obj->ed, obj->rp, EDJE_TWEEN_MODE_LINEAR,
                      FROM_DOUBLE(luaL_checknumber(L, -5)), ZERO, ZERO, ZERO, ZERO);
   _edje_recalc(obj->ed);
   return 0;
}

static int
_edje_lua_part_set_text(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   edje_object_part_text_set(obj->ed->obj, obj->key, luaL_checkstring(L, 2));
   return 0;
}

static int
_edje_lua_part_set_drag_value(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   edje_object_part_drag_value_set(obj->ed->obj, obj->key,
                                   luaL_checknumber(L, -2),
                                   luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_part_set_drag_size(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   edje_object_part_drag_size_set(obj->ed->obj, obj->key,
                                  luaL_checknumber(L, -2),
                                  luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_part_set_drag_step(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   edje_object_part_drag_step_set(obj->ed->obj, obj->key,
                                  luaL_checknumber(L, -2),
                                  luaL_checknumber(L, -1));
   return 0;
}

static int
_edje_lua_part_set_drag_page(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   edje_object_part_drag_page_set(obj->ed->obj, obj->key,
                                  luaL_checknumber(L, -2),
                                  luaL_checknumber(L, -1));
   return 0;
}

const luaL_Reg lPart_set[] = {
   {"drag_value", _edje_lua_part_set_drag_value},
   {"drag_size", _edje_lua_part_set_drag_size},
   {"drag_step", _edje_lua_part_set_drag_step},
   {"drag_page", _edje_lua_part_set_drag_page},

   {"effect", _edje_lua_part_set_effect},
   {"mouse_events", _edje_lua_part_set_mouse_events},
   {"repeat_events", _edje_lua_part_set_repeat_events},
   {"state", _edje_lua_part_set_state},
   {"tween_state", _edje_lua_part_set_tween_state},
   {"text", _edje_lua_part_set_text},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_part_fn_swallow(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Edje_Lua_Evas_Object *swa = _edje_lua_checkudata(L, 2, &mObject);
   edje_object_part_swallow(obj->ed->obj, obj->key, swa->eo);
   return 0;
}

static int
_edje_lua_part_fn_unswallow(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   edje_object_part_unswallow(obj->ed->obj, obj->eo);
   return 0;
}

static int
_edje_lua_part_fn_text_select_none(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   edje_object_part_text_select_none(obj->ed->obj, obj->key);
   return 0;
}

static int
_edje_lua_part_fn_text_select_all(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   edje_object_part_text_select_all(obj->ed->obj, obj->key);
   return 0;
}

static int
_edje_lua_part_fn_text_insert(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   edje_object_part_text_insert(obj->ed->obj, obj->key, luaL_checkstring(L, 2));
   return 0;
}

static int
_edje_lua_part_fn_custom_state(lua_State *L)
{
   char *name;
   float val;
   Edje_Part_Description *parent;
   Edje_Part_Image_Id *iid;
   Eina_List *l;
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   /* check whether this part already has a "custom" state */
   if (obj->rp->custom)
     {
        _edje_lua_get_reg(L, obj->rp->custom->description);
        return 1;
     }

   name = (char *)luaL_checkstring(L, 2); // state name
   val = luaL_checknumber(L, 3); // state val
   if (!(parent = _edje_part_description_find(obj->ed, obj->rp, name, val, EINA_TRUE)))
     {
        lua_pushnil(L);
        return 1;
     }
   /* now create the custom state */
   Edje_Lua_Edje_Part_Description *tar =
     lua_newuserdata(L, sizeof(Edje_Lua_Edje_Part_Description));
   tar->rp = obj->rp;
   _edje_lua_set_class(L, -1, cDescription);
   _edje_lua_new_reg(L, -1, tar); // freed in edje_load.c::_edje_file_del
   if (!(tar->pd = calloc(1, sizeof(Edje_Part_Description))))
     {
        lua_pushnil(L);
        return 1;
     }

   obj->rp->custom = eina_mempool_malloc(_edje_real_part_state_mp, sizeof (Edje_Real_Part_State));
   if (!obj->rp->custom)
     {
        free(tar->pd);
        tar->pd = NULL;
        lua_pushnil(L);
        return 1;
     }

   *(tar->pd) = *parent;
   tar->pd->state.name = (char *)eina_stringshare_add("custom");
   tar->pd->state.value = 0.0;
   /*
    * make sure all the allocated memory is getting copied,
    * not just referenced
    */
   tar->pd->image.tween_list = NULL;
   EINA_LIST_FOREACH(parent->image.tween_list, l, iid)
     {
        Edje_Part_Image_Id *iid_new;
        iid_new = calloc(1, sizeof(Edje_Part_Image_Id));
        iid_new->id = iid->id;

        tar->pd->image.tween_list =
          eina_list_append(tar->pd->image.tween_list, iid_new);
     }
#define DUP(x) x ? (char *)eina_stringshare_add(x) : NULL
   tar->pd->color_class = DUP(tar->pd->color_class);
   tar->pd->text.text = DUP(tar->pd->text.text);
   tar->pd->text.text_class = DUP(tar->pd->text.text_class);
   tar->pd->text.font = DUP(tar->pd->text.font);
   tar->pd->text.style = DUP(tar->pd->text.style);
#undef DUP
   obj->rp->custom->description = tar->pd;
   _edje_lua_new_reg(L, -1, obj->rp->custom->description); // freed in edje_load.c::_edje_file_del
   return 1;
}

static int
_edje_lua_part_fn_table_pack(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   lua_pushboolean(L,
                   edje_object_part_table_pack(obj->ed->obj, obj->key, tar->eo,
                                               luaL_checkint(L, 3),
                                               luaL_checkint(L, 4),
                                               luaL_checkint(L, 5),
                                               luaL_checkint(L, 6)));
   return 1;
}

static int
_edje_lua_part_fn_table_unpack(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   lua_pushboolean(L,
                   edje_object_part_table_unpack(obj->ed->obj, obj->key, tar->eo));
   return 1;
}

static int
_edje_lua_part_fn_table_clear(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_pushboolean(L,
                   edje_object_part_table_clear(obj->ed->obj, obj->key, lua_toboolean(L, 2)));
   return 1;
}

static int
_edje_lua_part_fn_box_append(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   lua_pushboolean(L,
                   edje_object_part_box_append(obj->ed->obj, obj->key, tar->eo));
   return 1;
}

static int
_edje_lua_part_fn_box_prepend(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   lua_pushboolean(L,
                   edje_object_part_box_prepend(obj->ed->obj, obj->key, tar->eo));
   return 1;
}

static int
_edje_lua_part_fn_box_insert_before(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   Edje_Lua_Evas_Object *ref = _edje_lua_checkudata(L, 3, &mObject);
   lua_pushboolean(L,
                   edje_object_part_box_insert_before(obj->ed->obj, obj->key, tar->eo, ref->eo));
   return 1;
}

static int
_edje_lua_part_fn_box_insert_at(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   lua_pushboolean(L,
                   edje_object_part_box_insert_at(obj->ed->obj, obj->key, tar->eo, luaL_checkint(L, 3)));
   return 1;
}

static int
_edje_lua_part_fn_box_remove(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   Edje_Lua_Evas_Object *tar = _edje_lua_checkudata(L, 2, &mObject);
   edje_object_part_box_remove(obj->ed->obj, obj->key, tar->eo);
   lua_pushvalue(L, 2);
   return 1;
}

static int
_edje_lua_part_fn_box_remove_at(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   _edje_lua_get_reg(L, edje_object_part_box_remove_at(obj->ed->obj, obj->key, luaL_checkint(L, 2)));
   return 1;
}

static int
_edje_lua_part_fn_box_remove_all(lua_State *L)
{
   Edje_Lua_Edje_Part_Object *obj = _edje_lua_checkudata(L, 1, &mPart);
   lua_pushboolean(L,
                   edje_object_part_box_remove_all(obj->ed->obj, obj->key, lua_toboolean(L, 2)));
   return 1;
}

const luaL_Reg lPart_fn[] = {
   {"swallow", _edje_lua_part_fn_swallow},
   {"unswallow", _edje_lua_part_fn_unswallow},

   {"custom_state", _edje_lua_part_fn_custom_state},

   {"text_select_none", _edje_lua_part_fn_text_select_none},
   {"text_select_all", _edje_lua_part_fn_text_select_all},
   {"text_insert", _edje_lua_part_fn_text_insert},

   {"table_pack", _edje_lua_part_fn_table_pack},
   {"table_unpack", _edje_lua_part_fn_table_unpack},
   {"table_clear", _edje_lua_part_fn_table_clear},

   {"box_append", _edje_lua_part_fn_box_append},
   {"box_prepend", _edje_lua_part_fn_box_prepend},
   {"box_insert_before", _edje_lua_part_fn_box_insert_before},
   {"box_insert_at", _edje_lua_part_fn_box_insert_at},
   {"box_remove", _edje_lua_part_fn_box_remove},
   {"box_remove_at", _edje_lua_part_fn_box_remove_at},
   {"box_remove_all", _edje_lua_part_fn_box_remove_all},
   {NULL, NULL} // sentinel
};

const luaL_Reg lGroup_mt[];

const luaL_Reg lGroup_get[];

const luaL_Reg lGroup_set[];

const luaL_Reg lGroup_fn[];

const Edje_Lua_Reg mGroup = {
   lGroup_mt,
   lGroup_get,
   lGroup_set,
   lGroup_fn
};

const Edje_Lua_Reg *cGroup[] = {
   &mClass,
   &mObject,
   &mGroup,
   NULL // sentinel
};

static int
_edje_lua_group_mt_index(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   lua_getmetatable(L, 1);
   if (!_edje_lua_look_fn(L))
     {
        // look in lGroup_fn
        if (!_edje_lua_look_get(L))
          {
             // look in lGroup_get
             const char *key = luaL_checkstring(L, 2);
             Edje *ed = _edje_fetch(obj->eo);
             Edje_Real_Part *rp = _edje_real_part_recursive_get(ed, key);
             if (rp)
               {
                  // check if lua userdata exists
                  edje_object_freeze(obj->eo);
                  Evas_Object *part = (Evas_Object *)edje_object_part_object_get(obj->eo, key);
                  edje_object_thaw(obj->eo);
                  _edje_lua_get_reg(L, part);
                  if (lua_isnil(L, -1))
                    {
                       // create it
                       lua_pop(L, 1);
                       Edje_Lua_Edje_Part_Object *tar =
                         lua_newuserdata(L, sizeof(Edje_Lua_Edje_Part_Object));
                       tar->eo = part;
                       tar->ed = ed;
                       tar->key = key;
                       tar->L = L;
                       tar->rp = rp;
                       _edje_lua_set_class(L, -1, cPart);
                       _edje_lua_new_reg(L, -1, tar); // freed in _edje_lua_edje_part_del_cb
                       _edje_lua_new_reg(L, -1, part); // freed in _edje_lua_edje_part_del_cb
                       evas_object_event_callback_add(tar->eo,
                                                      EVAS_CALLBACK_DEL,
                                                      _edje_lua_edje_part_del_cb,
                                                      L);
                    }
               }
             else
               {
                  // look in obj environment table
                  lua_getfenv(L, 1);
                  lua_pushvalue(L, 2);  // key
                  lua_rawget(L, -2);
               }
          }
     }
   return 1;
}

const luaL_Reg lGroup_mt[] = {
   {"__index", _edje_lua_group_mt_index},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_group_get_group(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   const char *file, *group;
   edje_object_file_get(obj->eo, &file, &group);
   lua_pushstring(L, group);
   return 1;
}

static int
_edje_lua_group_get_mouse(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   int x, y;
   evas_pointer_canvas_xy_get(evas_object_evas_get(obj->eo), &x, &y);
   x -= obj->ed->x;
   y -= obj->ed->y;
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, x);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, y);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_group_get_mouse_buttons(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   lua_pushnumber(L, evas_pointer_button_down_mask_get(obj->ed->evas));
   return 1;
}

static int
_edje_lua_group_get_size_min(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   int w, h;
   edje_object_size_min_get(obj->eo, &w, &h);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_group_get_size_max(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   int w, h;
   edje_object_size_max_get(obj->eo, &w, &h);
   lua_createtable(L, 2, 0);
   lua_pushnumber(L, w);
   lua_rawseti(L, -2, 1);
   lua_pushnumber(L, h);
   lua_rawseti(L, -2, 2);
   return 1;
}

static int
_edje_lua_group_get_scale(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   lua_pushnumber(L, edje_object_scale_get(obj->eo));
   return 1;
}

static int
_edje_lua_group_get_load_error(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   lua_pushnumber(L, edje_object_load_error_get(obj->eo));
   return 1;
}

static int
_edje_lua_group_get_load_error_str(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   lua_pushstring(L, edje_load_error_str(edje_object_load_error_get(obj->eo)));
   return 1;
}

static int
_edje_lua_group_get_play(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   lua_pushboolean(L, edje_object_play_get(obj->eo));
   return 1;
}

static int
_edje_lua_group_get_animation(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   lua_pushboolean(L, edje_object_animation_get(obj->eo));
   return 1;
}

static int
_edje_lua_group_get_frametime(lua_State *L)
{
   lua_pushnumber(L, edje_frametime_get());
   return 1;
}

const luaL_Reg lGroup_get[] = {
   {"group", _edje_lua_group_get_group},
   {"mouse", _edje_lua_group_get_mouse},
   {"mouse_buttons", _edje_lua_group_get_mouse_buttons},
   {"size_min", _edje_lua_group_get_size_min},
   {"size_max", _edje_lua_group_get_size_max},
   {"scale", _edje_lua_group_get_scale},
   {"load_error", _edje_lua_group_get_load_error},
   {"load_error_str", _edje_lua_group_get_load_error_str},
   {"play", _edje_lua_group_get_play},
   {"animation", _edje_lua_group_get_animation},
   {"frametime", _edje_lua_group_get_frametime},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_group_set_group(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   edje_object_file_set(obj->eo, obj->ed->file->path, luaL_checkstring(L, 2));
   return 0;
}

static int
_edje_lua_group_set_size_min(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);

   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->ed->collection->prop.min.w = luaL_checkint(L, -2);
   obj->ed->collection->prop.min.h = luaL_checkint(L, -1);
   obj->ed->dirty = EINA_TRUE;
   _edje_recalc(obj->ed);
   return 0;
}

static int
_edje_lua_group_set_size_max(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   luaL_checktype(L, 2, LUA_TTABLE);
   lua_rawgeti(L, 2, 1);
   lua_rawgeti(L, 2, 2);
   obj->ed->collection->prop.max.w = luaL_checkint(L, -2);
   obj->ed->collection->prop.max.h = luaL_checkint(L, -1);
   obj->ed->dirty = EINA_TRUE;
   _edje_recalc(obj->ed);
   return 0;
}

static int
_edje_lua_group_set_scale(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   edje_object_scale_set(obj->eo, luaL_checknumber(L, 2));
   return 0;
}

static int
_edje_lua_group_set_play(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   edje_object_play_set(obj->eo, lua_toboolean(L, 2));
   return 0;
}

static int
_edje_lua_group_set_animation(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   edje_object_animation_set(obj->eo, lua_toboolean(L, 2));
   return 0;
}

static void
_edje_lua_group_text_change_cb(void *data, Evas_Object *obj, const char *part)
{
   Edje_Lua_Ref *ref = data;
   lua_State *L = ref->L;
   int err_code;

   _edje_lua_get_ref(L, ref);
   _edje_lua_get_reg(L, obj);
   lua_pushstring(L, part);

   if ((err_code = lua_pcall(L, 2, 0, 0)))
     _edje_lua_error(L, err_code);
}

static int
_edje_lua_group_set_text_change_cb(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   edje_object_text_change_cb_set(obj->eo, _edje_lua_group_text_change_cb, _edje_lua_new_ref(L, 2));
   return 0;
}

static int
_edje_lua_group_set_frametime(lua_State *L)
{
   edje_frametime_set(luaL_checknumber(L, 2));
   return 0;
}

const luaL_Reg lGroup_set[] = {
   {"group", _edje_lua_group_set_group},
   {"size_min", _edje_lua_group_set_size_min},
   {"size_max", _edje_lua_group_set_size_max},
   {"scale", _edje_lua_group_set_scale},
   {"play", _edje_lua_group_set_play},
   {"animation", _edje_lua_group_set_animation},
   {"text_change_cb", _edje_lua_group_set_text_change_cb},
   {"frametime", _edje_lua_group_set_frametime},
   {NULL, NULL} // sentinel
};

static int
_edje_lua_group_fn_timer(lua_State *L)
{
   Edje_Lua_Timer *tar = lua_newuserdata(L, sizeof(Edje_Lua_Timer));

   _edje_lua_set_class(L, -1, cTimer);
   /* ^^^^^^^^^^^^^^^^(L, index, class)
      lua_newtable(L);
      if (index < 0)
      lua_setfenv(L, index - 1);
      else
      lua_setfenv(L, index);

      _edje_lua_get_metatable(L, class);
      if (index < 0)
      lua_setmetatable(L, index - 1);
      else
      lua_setmetatable(L, index);
    */

   tar->et = ecore_timer_add(luaL_checknumber(L, 2), _edje_lua_timer_cb, tar);
   tar->L = L;

   _edje_lua_new_reg(L, -1, tar); // freed in _edje_lua_timer_cb/del
   /* ^^^^^^^^^^^^^^(L, index, ptr)
      lua_pushvalue(L, index);
      lua_pushlightuserdata(L, ptr);
      lua_insert(L, -2);
      lua_rawset(L, LUA_REGISTRYINDEX); // freed in _edje_lua_free_reg
    */

   tar->cb = _edje_lua_new_ref(L, 3); // freed in _edje_lua_timer_cb/del
   /* ^^^^^^^^^^^^^^^^^^^^^^^^(L, index)
      lua_pushvalue(L, index);
      Edje_Lua_Ref *ref = malloc(sizeof(Edje_Lua_Ref));
      ref->id = luaL_ref(L, LUA_REGISTRYINDEX);
      ref->L = L;
      return ref;
    */
   return 1;
}

static int
_edje_lua_group_fn_animator(lua_State *L)
{
   Edje_Lua_Animator *tar = lua_newuserdata(L, sizeof(Edje_Lua_Animator));
   _edje_lua_set_class(L, -1, cAnimator);
   tar->ea = ecore_animator_add(_edje_lua_animator_cb, tar);
   tar->L = L;
   _edje_lua_new_reg(L, -1, tar); // freed in _edje_lua_animator_cb/del
   tar->cb = _edje_lua_new_ref(L, 2); // freed in _edje_lua_animator_cb/del
   return 1;
}

static int
_edje_lua_group_fn_poller(lua_State *L)
{
   int interval;
   Edje_Lua_Poller *tar;
   tar = lua_newuserdata(L, sizeof(Edje_Lua_Poller));
   _edje_lua_set_class(L, -1, cPoller);

   interval = luaL_checknumber(L, 2);
   if ((interval <= 0) || ((interval & 1) != 0))
     {
        return 0;
     }

   // Only 1 type of poller currently implemented in ecore
   tar->ep = ecore_poller_add(ECORE_POLLER_CORE, interval,
                              _edje_lua_poller_cb, tar);
   tar->L = L;
   _edje_lua_new_reg(L, -1, tar); // freed in _edje_lua_poller_cb/del
   tar->cb = _edje_lua_new_ref(L, 3); // freed in _edje_lua_poller_cb/del

   return 1;
}

static int
_edje_lua_group_fn_transform(lua_State *L)
{
   Edje_Lua_Transform *tar = lua_newuserdata(L, sizeof(Edje_Lua_Transform));
   evas_transform_identity_set(&(tar->et));
   _edje_lua_set_class(L, -1, cTransform);
   tar->L = L;
   return 1;
}

static int
_edje_lua_group_fn_signal_emit(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   edje_object_signal_emit(obj->eo,
                           luaL_checkstring(L, 2), luaL_checkstring(L, 3));
   return 0;
}

static Edje_Program *
_edje_lua_program_get_byname(Evas_Object *obj, const char *prog_name)
{
   Edje_Program *epr;
   int i;
   Edje *ed;
   ed = _edje_fetch(obj);
   if (!ed)
     return NULL;
   if (!prog_name)
     return NULL;
   for (i = 0; i < ed->table_programs_size; i++)
     {
        epr = ed->table_programs[i];
        if ((epr->name) && (strcmp(epr->name, prog_name) == 0))
          return epr;
     }
   return NULL;
}

static int
_edje_lua_group_fn_program_run(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   Edje_Program *pr =
     _edje_lua_program_get_byname(obj->eo, luaL_checkstring(L, 2));
   if (pr)
     _edje_program_run(obj->ed, pr, 0, "", "");
   return 0;
}

static int
_edje_lua_group_fn_program_stop(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   Edje_Program *pr =
     _edje_lua_program_get_byname(obj->eo, luaL_checkstring(L, 2));
   int program_id = pr->id;
   Edje_Running_Program *runp;
   Eina_List *l;
   obj->ed->walking_actions = EINA_TRUE;
   EINA_LIST_FOREACH(obj->ed->actions, l, runp)
     if (program_id == runp->program->id)
       _edje_program_end(obj->ed, runp);
   obj->ed->walking_actions = EINA_FALSE;
   return 0;
}

static int
_edje_lua_group_fn_message_send(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   int type = luaL_checkint(L, 2);
   void *msg = NULL;
   int i;
   int size;
   switch (type)
     {
      case EDJE_MESSAGE_NONE:
        break;

      case EDJE_MESSAGE_SIGNAL:
        break;

      case EDJE_MESSAGE_STRING:
      {
         Edje_Message_String *msg_string =
           malloc(sizeof(Edje_Message_String));
         msg_string->str = (char *)luaL_checkstring(L, 4);
         msg = msg_string;
      }
      break;

      case EDJE_MESSAGE_INT:
      {
         Edje_Message_Int *msg_int = malloc(sizeof(Edje_Message_Int));

         msg_int->val = luaL_checkint(L, 4);
         msg = msg_int;
      }
      break;

      case EDJE_MESSAGE_FLOAT:
      {
         Edje_Message_Float *msg_float = malloc(sizeof(Edje_Message_Float));

         msg_float->val = luaL_checknumber(L, 4);
         msg = msg_float;
      }
      break;

      case EDJE_MESSAGE_STRING_SET:
      {
         if (lua_type(L, 4) != LUA_TTABLE)
           break;
         size = lua_objlen(L, 4);
         Edje_Message_String_Set *msg_string_set =
           malloc(sizeof(Edje_Message_String_Set) +
                  (size - 1) * sizeof(char *));
         msg_string_set->count = size;
         for (i = 0; i < size; i++)
           {
              lua_rawgeti(L, 4, i + 1);
              msg_string_set->str[i] = (char *)luaL_checkstring(L, -1);
              lua_pop(L, 1);
           }
         msg = msg_string_set;
      }
      break;

      case EDJE_MESSAGE_INT_SET:
      {
         if (lua_type(L, 4) != LUA_TTABLE)
           break;
         size = lua_objlen(L, 4);
         Edje_Message_Int_Set *msg_int_set =
           malloc(sizeof(Edje_Message_Int_Set) + (size - 1) * sizeof(int));
         msg_int_set->count = size;
         for (i = 0; i < size; i++)
           {
              lua_rawgeti(L, 4, i + 1);
              msg_int_set->val[i] = luaL_checkint(L, -1);
              lua_pop(L, 1);
           }
         msg = msg_int_set;
      }
      break;

      case EDJE_MESSAGE_FLOAT_SET:
      {
         if (lua_type(L, 4) != LUA_TTABLE)
           break;
         size = lua_objlen(L, 4);
         Edje_Message_Float_Set *msg_float_set =
           malloc(sizeof(Edje_Message_Float_Set) +
                  (size - 1) * sizeof(double));
         msg_float_set->count = size;
         for (i = 0; i < size; i++)
           {
              lua_rawgeti(L, 4, i + 1);
              msg_float_set->val[i] = luaL_checknumber(L, -1);
              lua_pop(L, 1);
           }
         msg = msg_float_set;
      }
      break;

      case EDJE_MESSAGE_STRING_INT:
      {
         Edje_Message_String_Int *msg_string_int =
           malloc(sizeof(Edje_Message_String_Int));
         msg_string_int->str = (char *)luaL_checkstring(L, 4);
         msg_string_int->val = luaL_checkint(L, 5);
         msg = msg_string_int;
      }
      break;

      case EDJE_MESSAGE_STRING_FLOAT:
      {
         Edje_Message_String_Float *msg_string_float =
           malloc(sizeof(Edje_Message_String_Float));
         msg_string_float->str = (char *)luaL_checkstring(L, 4);
         msg_string_float->val = luaL_checknumber(L, 5);
         msg = msg_string_float;
      }
      break;

      case EDJE_MESSAGE_STRING_INT_SET:
      {
         if (lua_type(L, 5) != LUA_TTABLE)
           break;
         size = lua_objlen(L, 5);
         Edje_Message_String_Int_Set *msg_string_int_set =
           malloc(sizeof(Edje_Message_String_Int_Set) +
                  (size - 1) * sizeof(int));
         msg_string_int_set->str = (char *)luaL_checkstring(L, 4);
         msg_string_int_set->count = size;
         for (i = 0; i < size; i++)
           {
              lua_rawgeti(L, 5, i + 1);
              msg_string_int_set->val[i] = luaL_checkint(L, -1);
              lua_pop(L, 1);
           }
         msg = msg_string_int_set;
      }
      break;

      case EDJE_MESSAGE_STRING_FLOAT_SET:
      {
         if (lua_type(L, 5) != LUA_TTABLE)
           break;
         size = lua_objlen(L, 5);
         Edje_Message_String_Float_Set *msg_string_float_set =
           malloc(sizeof(Edje_Message_String_Float_Set) +
                  (size - 1) * sizeof(double));
         msg_string_float_set->str = (char *)luaL_checkstring(L, 4);
         msg_string_float_set->count = size;
         for (i = 0; i < size; i++)
           {
              lua_rawgeti(L, 5, i + 1);
              msg_string_float_set->val[i] = luaL_checknumber(L, -1);
              lua_pop(L, 1);
           }
         msg = msg_string_float_set;
      }
      break;

      default:
        break;
     }

   if (msg)
     {
        edje_object_message_send(obj->eo, type, luaL_checkint(L, 3), msg);
        free(msg);
     }

   return 0;
}

static void
_edje_lua_group_signal_callback(void *data, Evas_Object *edj,
                                const char *signal, const char *source)
{
   Edje_Lua_Ref *cb = data;
   lua_State *L = cb->L;
   _edje_lua_get_ref(L, cb);    // signal callback function
   if (lua_isfunction(L, -1))
     {
        int err_code;

        _edje_lua_get_reg(L, edj);
        lua_pushstring(L, signal); // signal
        lua_pushstring(L, source); // source

        if ((err_code = lua_pcall(L, 3, 0, 0)))
          _edje_lua_error(L, err_code);
     }
}

static int
_edje_lua_group_fn_signal_callback_add(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   if (lua_type(L, 4) == LUA_TFUNCTION)
     {
        Edje_Lua_Ref *ref = _edje_lua_new_ref(L, 4);
        obj->cb = eina_list_append(obj->cb, ref);
        edje_object_signal_callback_add(obj->eo,
                                        luaL_checkstring(L, 2),
                                        luaL_checkstring(L, 3),
                                        _edje_lua_group_signal_callback,
                                        ref); // freed in _edje_lua_group_fn_signal_callback_del
     }
   return 0;
}

static int
_edje_lua_group_fn_signal_callback_del(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   Edje_Lua_Ref *ref = edje_object_signal_callback_del(obj->eo,
                                                       luaL_checkstring(L, 2),
                                                       luaL_checkstring(L, 3),
                                                       _edje_lua_group_signal_callback);
   obj->cb = eina_list_remove(obj->cb, ref);
   _edje_lua_free_ref(L, ref); // created in _edje_lua_group_fn_signal_callback_add
   return 0;
}

static int
_edje_lua_group_fn_freeze(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   edje_object_freeze(obj->eo);
   return 0;
}

static int
_edje_lua_group_fn_thaw(lua_State *L)
{
   Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mGroup);
   edje_object_thaw(obj->eo);
   return 0;
}

const luaL_Reg lGroup_fn[] = {
   {"timer", _edje_lua_group_fn_timer},
   {"animator", _edje_lua_group_fn_animator},
   {"poller", _edje_lua_group_fn_poller},
   {"transform", _edje_lua_group_fn_transform},
   {"signal_emit", _edje_lua_group_fn_signal_emit},
   {"message_send", _edje_lua_group_fn_message_send},
   {"program_run", _edje_lua_group_fn_program_run},
   {"program_stop", _edje_lua_group_fn_program_stop},
   {"signal_callback_add", _edje_lua_group_fn_signal_callback_add},
   {"signal_callback_del", _edje_lua_group_fn_signal_callback_del},
   {"freeze", _edje_lua_group_fn_freeze},
   {"thaw", _edje_lua_group_fn_thaw},
   {NULL, NULL} // sentinel
};

/*
 * Lua bindings
 */

const luaL_Reg lScript_get[];

const luaL_Reg lScript_set[];

const luaL_Reg lScript_fn[];

const Edje_Lua_Reg mScript = {
   lNil,
   lScript_get,
   lScript_set,
   lScript_fn
};

const Edje_Lua_Reg *cScript[] = {
   &mClass,
   &mObject,
   &mGroup,
   &mScript,
   NULL // sentinel
};

/*
 * macro for adding an evas_object in the lua_script_only object
 */
#define _EDJE_LUA_SCRIPT_FN_ADD(DEF, CLASS, FUNC)                                            \
  static int                                                                                 \
  DEF(lua_State * L)                                                                         \
  {                                                                                          \
     int set = lua_gettop(L) == 2;                                                           \
     Edje_Lua_Evas_Object *obj = _edje_lua_checkudata(L, 1, &mScript);                       \
     Edje_Lua_Evas_Object *tar = lua_newuserdata(L, sizeof (Edje_Lua_Evas_Object));          \
     _edje_lua_set_class(L, -1, CLASS);                                                      \
     tar->eo = FUNC(obj->ed->evas);                                                          \
     tar->ed = obj->ed;                                                                      \
     tar->L = L;                                                                             \
     tar->cb = NULL;                                                                         \
     evas_object_move(tar->eo, obj->ed->x, obj->ed->y);                                      \
     _edje_lua_new_reg(L, -1, tar);  /* freed in _edje_lua_object_del_cb */                  \
     _edje_lua_new_reg(L, -1, tar->eo);  /* freed in _edje_lua_object_del_cb */              \
     evas_object_smart_member_add(tar->eo, obj->eo);                                         \
     evas_object_clip_set(tar->eo, obj->ed->clipper);                                        \
     evas_object_event_callback_add(tar->eo, EVAS_CALLBACK_DEL, _edje_lua_object_del_cb, L); \
     if (set)                                                                                \
       {                                                                                     \
          int err_code;                                                                      \
          lua_getfield(L, -1, "set");                                                        \
          lua_pushvalue(L, -2);                                                              \
          lua_pushvalue(L, 2);                                                               \
          if ((err_code = lua_pcall(L, 2, 0, 0)))                                            \
            _edje_lua_error(L, err_code);                                                    \
       }                                                                                     \
     return 1;                                                                               \
  }

_EDJE_LUA_SCRIPT_FN_ADD(_edje_lua_script_fn_rectangle,
                        cRectangle,
                        evas_object_rectangle_add)
_EDJE_LUA_SCRIPT_FN_ADD(_edje_lua_script_fn_image,
                        cImage,
                        evas_object_image_add)
_EDJE_LUA_SCRIPT_FN_ADD(_edje_lua_script_fn_table,
                        cTable,
                        evas_object_table_add)
_EDJE_LUA_SCRIPT_FN_ADD(_edje_lua_script_fn_line,
                        cLine,
                        evas_object_line_add)
_EDJE_LUA_SCRIPT_FN_ADD(_edje_lua_script_fn_polygon,
                        cPolygon,
                        evas_object_polygon_add)
_EDJE_LUA_SCRIPT_FN_ADD(_edje_lua_script_fn_group, cGroup, edje_object_add)

const luaL_Reg lScript_fn[] = {
   {"rectangle", _edje_lua_script_fn_rectangle},
   {"image", _edje_lua_script_fn_image},
   {"table", _edje_lua_script_fn_table},
   {"line", _edje_lua_script_fn_line},
   {"polygon", _edje_lua_script_fn_polygon},
   {"group", _edje_lua_script_fn_group},
   {NULL, NULL} // sentinel
};

const luaL_Reg lScript_get[] = {
   {NULL, NULL} // sentinel
};

const luaL_Reg lScript_set[] = {
   {NULL, NULL} // sentinel
};

/*
 * creates and exports a lua_script_only object
 */
void
_edje_lua_script_fn_new(Edje *ed)
{
   lua_State *L = ed->L;
   Edje_Lua_Evas_Object *tar = lua_newuserdata(L, sizeof(Edje_Lua_Evas_Object));
   _edje_lua_set_class(L, -1, cScript);
   tar->eo = ed->obj;
   tar->ed = ed;
   tar->L = L;
   tar->cb = NULL;
   _edje_lua_new_reg(L, -1, tar); // freed in _edje_lua_object_del_cb
   _edje_lua_new_reg(L, -1, ed); // freed in edje_load.c::_edje_file_del
   _edje_lua_new_reg(L, -1, ed->obj); // freed in _edje_lua_object_del_cb
   evas_object_event_callback_add(tar->eo, EVAS_CALLBACK_DEL,
                                  _edje_lua_object_del_cb, L);
}

/*
 * creates and exports an Edje group with associated Lua scripts in the parts and programs sections
 */
void
_edje_lua_group_fn_new(Edje *ed)
{
   lua_State *L = ed->L;
   Edje_Lua_Evas_Object *tar = lua_newuserdata(L, sizeof(Edje_Lua_Evas_Object));
   _edje_lua_set_class(L, -1, cGroup);
   tar->eo = ed->obj;
   tar->ed = ed;
   tar->L = L;
   tar->cb = NULL;
   _edje_lua_new_reg(L, -1, tar); // freed in _edje_lua_object_del_cb
   _edje_lua_new_reg(L, -1, ed); // freed in edje_load.c::_edje_file_del
   _edje_lua_new_reg(L, -1, ed->obj); // freed in _edje_lua_object_del_cb
   evas_object_event_callback_add(tar->eo, EVAS_CALLBACK_DEL,
                                  _edje_lua_object_del_cb, L);
}

/*
 * this function loads all the Lua bindings into the global Lua state
 */
static void
_edje_lua_open(lua_State *L)
{
   /*
    * export classes
    */
   _edje_lua_new_class(L, cTimer);
   _edje_lua_new_class(L, cAnimator);
   _edje_lua_new_class(L, cPoller);
   _edje_lua_new_class(L, cTransform);
   _edje_lua_new_class(L, cRectangle);
   _edje_lua_new_class(L, cImage);
   _edje_lua_new_class(L, cTable);
   _edje_lua_new_class(L, cLine);
   _edje_lua_new_class(L, cPolygon);
   _edje_lua_new_class(L, cGroup);
   _edje_lua_new_class(L, cDescription);
   _edje_lua_new_class(L, cPart);
   _edje_lua_new_class(L, cScript);

   /*
    * export constants
    */
   _edje_lua_new_const(L, "MESSAGE_NONE", EDJE_MESSAGE_NONE);
   _edje_lua_new_const(L, "MESSAGE_SIGNAL", EDJE_MESSAGE_SIGNAL);
   _edje_lua_new_const(L, "MESSAGE_STRING", EDJE_MESSAGE_STRING);
   _edje_lua_new_const(L, "MESSAGE_INT", EDJE_MESSAGE_INT);
   _edje_lua_new_const(L, "MESSAGE_FLOAT", EDJE_MESSAGE_FLOAT);
   _edje_lua_new_const(L, "MESSAGE_STRING_SET", EDJE_MESSAGE_STRING_SET);
   _edje_lua_new_const(L, "MESSAGE_INT_SET", EDJE_MESSAGE_INT_SET);
   _edje_lua_new_const(L, "MESSAGE_FLOAT_SET", EDJE_MESSAGE_FLOAT_SET);
   _edje_lua_new_const(L, "MESSAGE_STRING_INT", EDJE_MESSAGE_STRING_INT);
   _edje_lua_new_const(L, "MESSAGE_STRING_FLOAT", EDJE_MESSAGE_STRING_FLOAT);
   _edje_lua_new_const(L, "MESSAGE_STRING_INT_SET", EDJE_MESSAGE_STRING_INT_SET);
   _edje_lua_new_const(L, "MESSAGE_STRING_FLOAT_SET", EDJE_MESSAGE_STRING_FLOAT_SET);

   _edje_lua_new_const(L, "PART_TYPE_NONE", EDJE_PART_TYPE_NONE);
   _edje_lua_new_const(L, "PART_TYPE_RECTANGLE", EDJE_PART_TYPE_RECTANGLE);
   _edje_lua_new_const(L, "PART_TYPE_TEXT", EDJE_PART_TYPE_TEXT);
   _edje_lua_new_const(L, "PART_TYPE_IMAGE", EDJE_PART_TYPE_IMAGE);
   _edje_lua_new_const(L, "PART_TYPE_SWALLOW", EDJE_PART_TYPE_SWALLOW);
   _edje_lua_new_const(L, "PART_TYPE_TEXTBLOCK", EDJE_PART_TYPE_TEXTBLOCK);
   _edje_lua_new_const(L, "PART_TYPE_GRADIENT", EDJE_PART_TYPE_GRADIENT);
   _edje_lua_new_const(L, "PART_TYPE_GROUP", EDJE_PART_TYPE_GROUP);
   _edje_lua_new_const(L, "PART_TYPE_BOX", EDJE_PART_TYPE_BOX);
   _edje_lua_new_const(L, "PART_TYPE_VECTOR", EDJE_PART_TYPE_VECTOR);

   _edje_lua_new_const(L, "TEXT_EFFECT_NONE", EDJE_TEXT_EFFECT_NONE);
   _edje_lua_new_const(L, "TEXT_EFFECT_PLAIN", EDJE_TEXT_EFFECT_PLAIN);
   _edje_lua_new_const(L, "TEXT_EFFECT_OUTLINE", EDJE_TEXT_EFFECT_OUTLINE);
   _edje_lua_new_const(L, "TEXT_EFFECT_OTLINE", EDJE_TEXT_EFFECT_SOFT_OUTLINE);
   _edje_lua_new_const(L, "TEXT_EFFECT_SHADOW", EDJE_TEXT_EFFECT_SHADOW);
   _edje_lua_new_const(L, "TEXT_EFFECT_SOFT_SHADOW", EDJE_TEXT_EFFECT_SOFT_SHADOW);
   _edje_lua_new_const(L, "TEXT_EFFECT_OUTLINE_SHADOW", EDJE_TEXT_EFFECT_OUTLINE_SHADOW);
   _edje_lua_new_const(L, "TEXT_EFFECT_OUTLINE_SOFT_SHADOW", EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW);
   _edje_lua_new_const(L, "TEXT_EFFECT_FAR_SHADOW", EDJE_TEXT_EFFECT_FAR_SHADOW);
   _edje_lua_new_const(L, "TEXT_EFFECT_FAR_SOFT_SHADOW", EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW);
   _edje_lua_new_const(L, "TEXT_EFFECT_GLOW", EDJE_TEXT_EFFECT_GLOW);

   _edje_lua_new_const(L, "RENDER_BLEND", EVAS_RENDER_BLEND);
   _edje_lua_new_const(L, "RENDER_BLEND_REL", EVAS_RENDER_BLEND_REL);
   _edje_lua_new_const(L, "RENDER_COPY", EVAS_RENDER_COPY);
   _edje_lua_new_const(L, "RENDER_COPY_REL", EVAS_RENDER_COPY_REL);
   _edje_lua_new_const(L, "RENDER_ADD", EVAS_RENDER_ADD);
   _edje_lua_new_const(L, "RENDER_ADD_REL", EVAS_RENDER_ADD_REL);
   _edje_lua_new_const(L, "RENDER_SUB", EVAS_RENDER_SUB);
   _edje_lua_new_const(L, "RENDER_SUB_REL", EVAS_RENDER_SUB_REL);
   _edje_lua_new_const(L, "RENDER_TINT", EVAS_RENDER_TINT);
   _edje_lua_new_const(L, "RENDER_TINT_REL", EVAS_RENDER_TINT_REL);
   _edje_lua_new_const(L, "RENDER_MASK", EVAS_RENDER_MASK);
   _edje_lua_new_const(L, "RENDER_MUL", EVAS_RENDER_MUL);

   _edje_lua_new_const(L, "BORDER_FILL_NONE", EVAS_BORDER_FILL_NONE);
   _edje_lua_new_const(L, "BORDER_FILL_DEFAULT", EVAS_BORDER_FILL_DEFAULT);
   _edje_lua_new_const(L, "BORDER_FILL_SOLID", EVAS_BORDER_FILL_SOLID);

   _edje_lua_new_const(L, "POINTER_MODE_AUTOGRAB", EVAS_OBJECT_POINTER_MODE_AUTOGRAB);
   _edje_lua_new_const(L, "POINTER_MODE_NOGRAB", EVAS_OBJECT_POINTER_MODE_NOGRAB);

   _edje_lua_new_const(L, "ASPECT_CONTROL_NEITHER", EVAS_ASPECT_CONTROL_NEITHER);
   _edje_lua_new_const(L, "ASPECT_CONTROL_HORIZONTAL", EVAS_ASPECT_CONTROL_HORIZONTAL);
   _edje_lua_new_const(L, "ASPECT_CONTROL_VERTICAL", EVAS_ASPECT_CONTROL_VERTICAL);
   _edje_lua_new_const(L, "ASPECT_CONTROL_BOTH", EVAS_ASPECT_CONTROL_BOTH);

   _edje_lua_new_const(L, "CALLBACK_RENEW", ECORE_CALLBACK_RENEW);
   _edje_lua_new_const(L, "CALLBACK_CANCEL", ECORE_CALLBACK_CANCEL);
}

/*
 * main Lua state
 * created by edje_init ()
 * closed by edje_shutdown ()
 * any other private Lua state inherits from this global state
 */
static lua_State *Ledje = NULL;

lua_State *
_edje_lua_state_get(void)
{
   return Ledje;
}

/*
 * custom memory allocation function
 * raises an error, if memory usage is above the given maximum
 */
static void *
_edje_lua_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
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
        free(ptr); /* ANSI requires that free(NULL) has no effect */
        return NULL;
     }

   /* ANSI requires that realloc(NULL, size) == malloc(size) */
   ptr2 = realloc(ptr, nsize);
   if (ptr2) return ptr2;
   ERR("Edje Lua cannot re-allocate %zu bytes", nsize);
   return ptr2;
}

void
_edje_lua_init(void)
{
   if (Ledje != NULL) return;
   /*
    * create main Lua state with the custom memory allocation function
    */
   static Edje_Lua_Alloc ela = { 1e7, 0 }; // TODO make the memory limit configurable?
   Ledje = lua_newstate(_edje_lua_alloc, &ela);
   if (!Ledje)
     {
        ERR("Lua error: Lua state could not be initialized");
        exit(-1);
     }

   lua_atpanic(Ledje, _edje_lua_custom_panic);

   /*
    * configure Lua garbage collector
    * TODO optimize garbage collector for typical edje use or make it configurable
    */
   lua_gc(Ledje, LUA_GCSETPAUSE, 200);
   lua_gc(Ledje, LUA_GCSETSTEPMUL, 200);

   /*
    * sandbox Lua
    * no access to io, os and package routines
    * no loading and execution of files
    * no loading and execution of strings
    * no access to the OS environment
    */
   luaopen_base(Ledje);
   luaopen_table(Ledje);
   luaopen_string(Ledje);
   luaopen_math(Ledje);
   luaopen_os(Ledje);

   /*
    * FIXME
    * this is just for debug purposes
    * remove it in the final version
    */
   lua_pushnil(Ledje);
   lua_setglobal(Ledje, "load");
   lua_pushnil(Ledje);
   lua_setglobal(Ledje, "loadfile");
   lua_pushnil(Ledje);
   lua_setglobal(Ledje, "loadstring");
   lua_pushnil(Ledje);
   lua_setglobal(Ledje, "dofile");
   lua_pushnil(Ledje);
   lua_setglobal(Ledje, "dostring");

   lua_getglobal(Ledje, "os");
   lua_pushnil(Ledje);
   lua_setfield(Ledje, -2, "exit");
   lua_pushnil(Ledje);
   lua_setfield(Ledje, -2, "setlocale");
   lua_pushnil(Ledje);
   lua_setfield(Ledje, -2, "getenv");
   lua_pushnil(Ledje);
   lua_setfield(Ledje, -2, "remove");
   lua_pushnil(Ledje);
   lua_setfield(Ledje, -2, "tmpname");
   lua_pushnil(Ledje);
   lua_setfield(Ledje, -2, "rename");
   lua_pushnil(Ledje);
   lua_setfield(Ledje, -2, "execute");
   lua_pushnil(Ledje);

   /*
    * we need a weak value registry
    * so that deleted and unused objects can be garbage collected
    */
   lua_createtable(Ledje, 1, 0);
   //lua_pushstring(Ledje, "v");
   lua_pushstring(Ledje, "");
   lua_setfield(Ledje, -2, "__mode");
   lua_setmetatable(Ledje, LUA_REGISTRYINDEX);

   /*
    * load Lua Evas/Edje bindings
    */
   _edje_lua_open(Ledje);
}

void
_edje_lua_shutdown(void)
{
   if (Ledje == NULL) return;
   lua_close(Ledje);
   Ledje = NULL;
}

#endif
