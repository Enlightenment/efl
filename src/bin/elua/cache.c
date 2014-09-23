#include "config.h" 

/* elua bytecode caching */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "main.h"

/* bytecode caching */

static Eina_File *
check_bc(Eina_File *of, const char *fname, Eina_Bool *bc)
{
   if (of)
     {
        struct stat bc_stat, sc_stat;
        /* original file doesn't exist, only bytecode does, use bytecode */
        if (stat(fname, &sc_stat) < 0)
          return of;
        if (stat(eina_file_filename_get(of), &bc_stat) < 0)
          {
             /* what? */
             eina_file_close(of);
             goto generate;
          }
        /* bytecode is newer than original file, use bytecode */
        if (bc_stat.st_mtime > sc_stat.st_mtime)
          return of;
        /* bytecode is not new enough; trigger regeneration */
        eina_file_close(of);
     }
generate:
   *bc = EINA_TRUE;
   return eina_file_open(fname, EINA_FALSE);
}

static Eina_File *
open_src(const char *fname, Eina_Bool *bc)
{
   Eina_File  *f   = NULL;
   const char *ext = strstr(fname, ".lua");
   if (ext && !ext[4])
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "%sc", fname);
        f = check_bc(eina_file_open(buf, EINA_FALSE), fname, bc);
     }
   if (!f) f = eina_file_open(fname, EINA_FALSE);
   return  f;
}

static int
writef(lua_State *L EINA_UNUSED, const void *p, size_t size, void *ud)
{
   FILE *f = ud;
   return ferror(f) || (fwrite(p, 1, size, f) != size);
}

static void
write_bc(lua_State *L, const char *fname)
{
   FILE *f;
   char  buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%sc", fname);
   if ((f = fopen(buf, "wb")))
     {
        if (lua_dump(L, writef, f))
          {
             fclose(f);
             /* there really is nothing to handle here */
             (void)!!remove(buf);
          }
        else fclose(f);
     }
}

static const char *
getf(lua_State *L EINA_UNUSED, void *ud, size_t *size)
{
   char *buff = *((char**)ud);
   if (feof(stdin)) return NULL;
   *size = fread(buff, 1, LUAL_BUFFERSIZE, stdin);
   return (*size > 0) ? buff : NULL;
}

static int
elua_loadstdin(lua_State *L)
{
   char buff[LUAL_BUFFERSIZE];
   int status = lua_load(L, getf, &buff, "=stdin");
   if (ferror(stdin))
     {
        lua_pop(L, 1);
        lua_pushfstring(L, "cannot read stdin: %s", strerror(errno));
        return LUA_ERRFILE;
     }
   return status;
}

typedef struct Map_Stream
{
   char   *fmap;
   size_t  flen;
} Map_Stream;

static const char *
getf_map(lua_State *L EINA_UNUSED, void *ud, size_t *size)
{
   Map_Stream *s    = ud;
   const char *fmap = s->fmap;
   *size = s->flen;
   /* gotta null it - tell luajit to terminate reading */
   s->fmap = NULL;
   return fmap;
}

int
elua_loadfile(lua_State *L, const char *fname)
{
   Map_Stream s;
   int status;
   Eina_File *f;
   const char *chname;
   Eina_Bool bcache = EINA_FALSE;
   if (!fname)
     {
        return elua_loadstdin(L);
     }
   if (!(f = open_src(fname, &bcache)))
     {
        lua_pushfstring(L, "cannot open %s: %s", fname, strerror(errno));
        return LUA_ERRFILE;
     }
   chname = lua_pushfstring(L, "@%s", fname);
   s.flen = eina_file_size_get(f);
   if (!(s.fmap = eina_file_map_all(f, EINA_FILE_RANDOM)))
     {
        lua_pushfstring(L, "cannot read %s: %s", chname + 1, strerror(errno));
        lua_remove(L, -2);
        return LUA_ERRFILE;
     }
   status = lua_load(L, getf_map, &s, chname);
   eina_file_map_free(f, s.fmap);
   eina_file_close(f);
   if (!status && bcache) write_bc(L, fname);
   lua_remove(L, -2);
   return status;
}

/* lua function */

static int
loadfile(lua_State *L)
{
   const char *fname = luaL_optstring(L, 1, NULL);
   int status = elua_loadfile(L, fname),
       hasenv = (lua_gettop(L) >= 3);
   if (!status)
     {
        if (hasenv)
          {
             lua_pushvalue(L, 3);
             lua_setfenv(L, -2);
          }
        return 1;
     }
   lua_pushnil(L);
   lua_insert(L, -2);
   return 2;
}

void
elua_register_cache(lua_State *L)
{
   lua_pushcfunction(L, loadfile);
   lua_setglobal(L, "loadfile");
}
