/* elua bytecode caching */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "main.h"

/* bytecode caching */

static Eina_File *check_bc(Eina_File *of, const char *mode, Eina_Bool *bc) {
    const char *fname = eina_file_filename_get(of);
    const char *ext   = strstr(fname, ".lua");
    if (ext && !ext[4] && (!mode || strchr(mode, 't'))) {
        /* loading lua source file, try cached */
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "%sc", fname);
        Eina_File *f = eina_file_open(buf, EINA_FALSE);
        if (!f) {
            /* no cached bytecode */
            *bc = EINA_TRUE;
        } else {
            /* cached bytecode, check timestamps */
            if (eina_file_mtime_get(f) > eina_file_mtime_get(of)) {
                /* bytecode new enough, chunkname stays the same */
                eina_file_close(of);
                return f;
            } else {
                /* bytecode too old, remove old file */
                eina_file_close(f);
                *bc = EINA_TRUE;
            }
        }
    }
    return of;
}

static int writef(lua_State *L EINA_UNUSED, const void *p, size_t size,
void *ud) {
    FILE *f = ud;
    return ferror(f) || (fwrite(p, 1, size, f) != size);
}

static void write_bc(lua_State *L, const char *fname) {
    char buf[PATH_MAX];
    snprintf(buf, sizeof(buf), "%sc", fname);
    FILE *f = fopen(buf, "wb");
    if (f) {
        if (lua_dump(L, writef, f)) {
            fclose(f);
            remove(buf);
        } else fclose(f);
    }
}

static const char *getf(lua_State *L EINA_UNUSED, void *ud, size_t *size) {
    char *buff = *((char**)ud);
    if (feof(stdin)) return NULL;
    *size = fread(buff, 1, LUAL_BUFFERSIZE, stdin);
    return (*size > 0) ? buff : NULL;
}

static int elua_loadstdin(lua_State *L, const char *mode) {
    char buff[LUAL_BUFFERSIZE];
    int status = lua_loadx(L, getf, &buff, "=stdin", mode);
    if (ferror(stdin)) {
        lua_pop(L, 1);
        lua_pushfstring(L, "cannot read stdin: %s", strerror(errno));
        return LUA_ERRFILE;
    }
    return status;
}

typedef struct Map_Stream {
    char  *fmap;
    size_t flen;
} Map_Stream;

static const char *getf_map(lua_State *L EINA_UNUSED, void *ud, size_t *size) {
    Map_Stream *s    = ud;
    const char *fmap = s->fmap;
    *size = s->flen;
    /* gotta null it - tell luajit to terminate reading */
    s->fmap = NULL;
    return fmap;
}

int elua_loadfilex(lua_State *L, const char *fname, const char *mode) {
    Map_Stream s;
    int status;
    Eina_File *f;
    const char *chname;
    Eina_Bool bcache = EINA_FALSE;
    if (!fname) {
        return elua_loadstdin(L, mode);
    }
    if (!(f = eina_file_open(fname, EINA_FALSE))) {
        lua_pushfstring(L, "cannot open %s: %s", fname, strerror(errno));
        return LUA_ERRFILE;
    }
    chname = lua_pushfstring(L, "@%s", fname);
    f = check_bc(f, mode, &bcache);
    s.flen = eina_file_size_get(f);
    if (!(s.fmap = eina_file_map_all(f, EINA_FILE_RANDOM))) {
        lua_pushfstring(L, "cannot read %s: %s", chname + 1, strerror(errno));
        lua_remove(L, -2);
        return LUA_ERRFILE;
    }
    status = lua_loadx(L, getf_map, &s, chname, mode);
    eina_file_map_free(f, s.fmap);
    eina_file_close(f);
    if (!status && bcache) write_bc(L, fname);
    lua_remove(L, -2);
    return status;
}

int elua_loadfile(lua_State *L, const char *fname) {
    return elua_loadfilex(L, fname, NULL);
}

/* lua function */

static int loadfile(lua_State *L) {
    const char *fname = luaL_optstring(L, 1, NULL);
    const char *mode  = luaL_optstring(L, 2, NULL);
    int status = elua_loadfilex(L, fname, mode),
        hasenv = (lua_gettop(L) >= 3);
    if (!status) {
        if (hasenv) {
            lua_pushvalue(L, 3);
            lua_setfenv(L, -2);
        }
        return 1;
    }
    lua_pushnil(L);
    lua_insert(L, -2);
    return 2;
}

void elua_register_cache(lua_State *L) {
    lua_pushcfunction(L, loadfile);
    lua_setglobal(L, "loadfile");
}