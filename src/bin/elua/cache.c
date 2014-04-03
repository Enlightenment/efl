/* elua bytecode caching */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "main.h"

/* bytecode caching */

static int check_bc(const char *fname, const char *mode, Eina_Bool *bc) {
    const char *ext = strstr(fname, ".lua");
    if (ext && !ext[4] && (!mode || strchr(mode, 't'))) {
        /* loading lua source file, try cached */
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "%sc", fname);
        int fd = open(buf, O_RDONLY);
        if (fd < 0) {
            /* no cached bytecode */
            *bc = EINA_TRUE;
        } else {
            /* cached bytecode, check timestamp */
            struct stat s1, s2;
            stat(fname, &s1);
            stat(buf,   &s2);
            if (s2.st_ctime > s1.st_ctime) {
                /* bytecode new enough, chunkname stays the same */
                return fd;
            } else {
                /* bytecode too old, remove old file */
                close(fd);
                *bc = EINA_TRUE;
            }
        }
    }
    return -1;
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

/* loadfile - regular version */

typedef struct Cached_Stream {
    FILE *f;
    char buff[LUAL_BUFFERSIZE];
} Cached_Stream;

static const char *getf(lua_State *L EINA_UNUSED, void *ud, size_t *size) {
    Cached_Stream *s = ud;
    if (feof(s->f)) return NULL;
    *size = fread(s->buff, 1, sizeof(s->buff), s->f);
    return (*size > 0) ? s->buff : NULL;
}

int elua_loadfilex(lua_State *L, const char *fname, const char *mode) {
    Cached_Stream s;
    int status;
    const char *chname;
    Eina_Bool bcache = EINA_FALSE;
    if (!fname) {
        s.f = stdin;
        chname = "=stdin";
    } else {
        if (!(s.f = fopen(fname, "rb"))) {
            lua_pushfstring(L, "cannot open %s: %s", fname, strerror(errno));
            return LUA_ERRFILE;
        }
        chname = lua_pushfstring(L, "@%s", fname);
        int fd = check_bc(fname, mode, &bcache);
        if (fd >= 0) {
            fclose(s.f);
            s.f = fdopen(fd, "rb");
        }
    }
    status = lua_loadx(L, getf, &s, chname, mode);
    if (ferror(s.f)) {
        lua_pop(L, 1);
        lua_pushfstring(L, "cannot read %s: %s", chname + 1, strerror(errno));
        if (fname) {
            lua_remove(L, -2);
            fclose(s.f);
        }
        return LUA_ERRFILE;
    }
    /* trigger bytecode writing */
    if (!status && bcache) write_bc(L, fname);
    if (fname) {
        lua_remove(L, -2);
        fclose(s.f);
    }
    return status;
}

int elua_loadfile(lua_State *L, const char *fname) {
    return elua_loadfilex(L, fname, NULL);
}

/* loadfile - mmap version */

typedef struct Map_Stream {
    char  *fmap;
    size_t flen;
} Map_Stream;

static const char *getf_map(lua_State *L EINA_UNUSED, void *ud, size_t *size) {
    Map_Stream *s = ud;
    *size = s->flen;
    return  s->fmap;
}

int elua_loadfilex_mmap(lua_State *L, const char *fname, const char *mode) {
    Map_Stream s;
    int status, fd, nfd;
    const char *chname;
    Eina_Bool bcache = EINA_FALSE;
    if (!fname) {
        return elua_loadfilex(L, fname, mode);
    }
    if ((fd = open(fname, O_RDONLY, 0)) < 0) {
        lua_pushfstring(L, "cannot open %s: %s", fname, strerror(errno));
        return LUA_ERRFILE;
    }
    chname = lua_pushfstring(L, "@%s", fname);
    if ((nfd = check_bc(fname, mode, &bcache)) > 0) {
        close(fd);
        fd = nfd;
    }
    s.flen = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    if ((s.fmap = mmap(NULL, s.flen, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0))
    == MAP_FAILED) {
        lua_pushfstring(L, "cannot read %s: %s", chname + 1, strerror(errno));
        lua_remove(L, -2);
        return LUA_ERRFILE;
    }
    status = lua_loadx(L, getf_map, &s, chname, mode);
    munmap(s.fmap, s.flen);
    close(fd);
    if (!status && bcache) write_bc(L, fname);
    lua_remove(L, -2);
    return status;
}

int elua_loadfile_mmap(lua_State *L, const char *fname) {
    return elua_loadfilex_mmap(L, fname, NULL);
}

/* lua function */

static int loadfile(lua_State *L) {
    const char *fname = luaL_optstring(L, 1, NULL);
    const char *mode  = luaL_optstring(L, 2, NULL);
    int status, hasenv = (lua_gettop(L) >= 3);
    if (lua_toboolean(L, 4)) {
        status = elua_loadfilex_mmap(L, fname, mode);
    } else {
        status = elua_loadfilex(L, fname, mode);
    }
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