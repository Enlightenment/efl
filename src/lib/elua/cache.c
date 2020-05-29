#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "elua_private.h"

/* bytecode caching */

#if LUA_VERSION_NUM > 501
#  define elua_load(L, reader, data, chunkname) lua_load(L, reader, data, chunkname, NULL)
#else
#  define elua_load(L, reader, data, chunkname) lua_load(L, reader, data, chunkname)
#endif

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
open_src(const char *fname, Eina_Bool *bc, Eina_Bool allow_bc)
{
   Eina_File  *f   = NULL;
   const char *ext = strstr(fname, ".lua");
   if (ext && !ext[4] && allow_bc)
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

static FILE *
bc_tmp_open(const char *fname, char *buf, size_t buflen)
{
   Eina_Tmpstr *tmp_file;
   int fd;
   snprintf(buf, buflen, "%s.XXXXXX.cache", fname);
   fd = eina_file_mkstemp(buf, &tmp_file);
   if (fd < 0)
     return NULL;
   eina_strlcpy(buf, tmp_file, buflen);
   eina_tmpstr_del(tmp_file);
   return fdopen(fd, "wb");
}

static void
write_bc(lua_State *L, const char *fname)
{
   FILE *f;
   char buf[PATH_MAX];
   if ((f = bc_tmp_open(fname, buf, sizeof(buf))))
     {
        char buf2[PATH_MAX];
        if (lua_dump(L, writef, f))
          {
             fclose(f);
             /* there really is nothing to handle here */
             (void)!!remove(buf);
             return;
          }
        else fclose(f);
        snprintf(buf2, sizeof(buf2), "%sc", fname);
        if (rename(buf, buf2))
          {
             /* a futile attempt at cleanup */
             (void)!!remove(buf);
             (void)!!remove(buf2);
          }
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
   int status = elua_load(L, getf, &buff, "=stdin");
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
   /* gotta null it - tell lua to terminate reading */
   s->fmap = NULL;
   return fmap;
}

EAPI int
elua_io_loadfile(const Elua_State *es, const char *fname)
{
   Map_Stream s;
   int status;
   Eina_File *f;
   const char *chname;
   Eina_Bool bcache = EINA_FALSE;
   lua_State *L;
   if (!es || !es->luastate) return -1;
   L = es->luastate;
   if (!fname)
     {
        return elua_loadstdin(L);
     }
   if (!(f = open_src(fname, &bcache, EINA_TRUE)))
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
   status = elua_load(L, getf_map, &s, chname);
   eina_file_map_free(f, s.fmap);
   eina_file_close(f);
   if (status)
     {
        /* we loaded bytecode and that failed; try loading source instead */
        if (!bcache)
          {
             /* can't open real file, so return original error */
             if (!(f = open_src(fname, &bcache, EINA_FALSE)))
               {
                  lua_remove(L, -2);
                  return status;
               }
             s.flen = eina_file_size_get(f);
             /* can't read real file, so return original error */
             if (!(s.fmap = eina_file_map_all(f, EINA_FILE_RANDOM)))
               {
                  lua_remove(L, -2);
                  return status;
               }
             /* loaded original file, pop old error and load again */
             lua_pop(L, 1);
             status = elua_load(L, getf_map, &s, chname);
             eina_file_map_free(f, s.fmap);
             eina_file_close(f);
             /* force write new bytecode */
             if (!status)
               write_bc(L, fname);
          }
        /* whatever happened here, proceed to the end... */
     }
   else if (bcache)
     write_bc(L, fname); /* success and bytecode write */
   lua_remove(L, -2);
   return status;
}

/* lua function */

static int
loadfile(lua_State *L)
{
   Elua_State *es = elua_state_from_lua_state_get(L);
   const char *fname = luaL_optstring(L, 1, NULL);
   int status = elua_io_loadfile(es, fname),
       hasenv = (lua_gettop(L) >= 3);
   if (!status)
     {
        if (hasenv)
          {
             lua_pushvalue(L, 3);
#if LUA_VERSION_NUM < 502
             lua_setfenv(L, -2);
#else
             if (!lua_setupvalue(L, -2, 1))
               lua_pop(L, 1);
#endif
          }
        return 1;
     }
   lua_pushnil(L);
   lua_insert(L, -2);
   return 2;
}

Eina_Bool
_elua_state_io_setup(const Elua_State *es)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(es && es->luastate, EINA_FALSE);
   lua_pushcfunction(es->luastate, loadfile);
   lua_setglobal(es->luastate, "loadfile");
   return EINA_TRUE;
}
