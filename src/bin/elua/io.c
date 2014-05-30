/* elua io extras, largely taken from lua io lib source */

#include "main.h"

/* expand fname to full path name (so that PATH is ignored) plus turn
 * stuff into a command, and also verify whether the path exists */
static char *
get_cmdline_from_argv(const char *fname, const char **argv)
{
   Eina_Strbuf *buf;
   char        *ret;
   char         pbuf[PATH_MAX];

   FILE *testf = fopen(fname, "r");
   if  (!testf)
      return NULL;

   fclose(testf);

   /* for windows, we have realpath in evil, no need for GetFullPathName */
   if (!realpath(fname, pbuf))
      return NULL;

   buf = eina_strbuf_new();
   eina_strbuf_append_char(buf, '"');
   eina_strbuf_append(buf, pbuf);
   eina_strbuf_append_char(buf, '"');

   while (*argv)
     {
        const char *arg = *(argv++);
        char        c;
        eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append_char(buf, '"');

        while ((c = *(arg++)))
          {
#ifndef _WIN32
             if (c == '"' || c == '$') eina_strbuf_append_char(buf, '\\');
             eina_strbuf_append_char(buf, c);
#else
             if      (c == '"') eina_strbuf_append_char(buf, '\\');
             else if (c == '%') eina_strbuf_append_char(buf,  '"');
             eina_strbuf_append_char(buf, c);
             if (c == '%') eina_strbuf_append_char(buf,  '"');
#endif
          }

        eina_strbuf_append_char(buf, '"');
     }

   ret = strdup(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
}

static FILE *
elua_popen_c(const char *path, const char *md, const char *argv[])
{
   FILE *ret;

   char *cmdline = get_cmdline_from_argv(path, argv);
   if  (!cmdline) return NULL;

   ret = popen(cmdline, md);
   if (!ret) return NULL;

   return ret;
}

static int
push_ret(lua_State *L, int i, const char *fname)
{
   int en = errno;
   if (i)
     {
        lua_pushboolean(L, 1);
        return 1;
     }
   else
     {
        lua_pushnil(L);
        if (fname)
           lua_pushfstring(L, "%s: %s", fname, strerror(en));
        else
           lua_pushfstring(L, "%s", strerror(en));
        lua_pushinteger(L, en);
        return 3;
     }
}

static FILE *
tofile(lua_State *L)
{
   FILE **f = (FILE**)luaL_checkudata(L, 1, "ELUA_FILE*");
   if (!*f)
     {
        luaL_error(L, "attempt to use a closed file");
     }
   return *f;
}

static int
elua_close(lua_State *L)
{
   FILE **f = (FILE**)luaL_checkudata(L, 1, "ELUA_FILE*");
   int ok = (fclose(*f) == 0);
   if (ok) *f = NULL;
   return push_ret(L, ok, NULL);
}

static int
elua_flush(lua_State *L)
{
   return push_ret(L, fflush(tofile(L)) == 0, NULL);
}

static int elua_readline(lua_State *L);

static int
elua_lines(lua_State *L)
{
   lua_pushvalue(L, 1);
   lua_pushcclosure(L, elua_readline, 1);
   return 1;
}

static int
read_number(lua_State *L, FILE *f)
{
   lua_Number d;
   if (fscanf(f, LUA_NUMBER_SCAN, &d) == 1)
     {
        lua_pushnumber(L, d);
        return 1;
     }
   return 0;
}

static int
test_eof(lua_State *L, FILE *f)
{
   int c = getc(f);
   ungetc(c, f);
   lua_pushlstring(L, NULL, 0);
   return (c != EOF);
}

static int
read_line(lua_State *L, FILE *f)
{
   luaL_Buffer b;
   luaL_buffinit(L, &b);
   for (;;)
     {
        size_t l;
        char *p = luaL_prepbuffer(&b);
        if (fgets(p, LUAL_BUFFERSIZE, f) == NULL)
          {
             luaL_pushresult(&b);
             return (lua_strlen(L, -1) > 0);
          }
        l = strlen(p);
        if (!l || p[l - 1] != '\n')
           luaL_addsize(&b, l);
        else
          {
             luaL_addsize(&b, l - 1);
             luaL_pushresult(&b);
             return 1;
          }
     }
}

static int
read_chars(lua_State *L, FILE *f, size_t n)
{
   size_t rlen;
   size_t nr;
   luaL_Buffer b;
   luaL_buffinit(L, &b);
   rlen = LUAL_BUFFERSIZE;
   do
     {
        char *p = luaL_prepbuffer(&b);
        if (rlen > n) rlen = n;
        nr = fread(p, sizeof(char), rlen, f);
        luaL_addsize(&b, nr);
        n -= nr;
     } while (n > 0 && nr == rlen);
   luaL_pushresult(&b);
   return (n == 0 || lua_strlen(L, -1) > 0);
}

static int
elua_readline(lua_State *L)
{
   FILE *f = *(FILE**)lua_touserdata(L, lua_upvalueindex(1));
   int success;
   if (!f)
     {
        luaL_error(L, "file is already closed");
     }
   success = read_line(L, f);
   if (ferror(f))
      return luaL_error(L, "%s", strerror(errno));
   return success;
}

static int
elua_read(lua_State *L)
{
   FILE *f   = tofile(L);
   int nargs = lua_gettop(L) - 1;
   int first = 2;
   int success, n;
   clearerr(f);
   if (!nargs)
     {
        success = read_line(L, f);
        n = first + 1;
     }
   else
     {
        luaL_checkstack(L, nargs + LUA_MINSTACK, "too many arguments");
        success = 1;
        for (n = first; nargs-- && success; ++n)
          {
             if (lua_type(L, n) == LUA_TNUMBER)
               {
                  size_t l = (size_t)lua_tointeger(L, n);
                  success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
               }
             else
               {
                  const char *p = lua_tostring(L, n);
                  luaL_argcheck(L, p && p[0] == '*', n, "invalid option");
                  switch (p[1])
                    {
                       case 'n':
                          success = read_number(L, f);
                          break;
                       case 'l':
                          success = read_line(L, f);
                          break;
                       case 'a':
                          read_chars(L, f, ~((size_t)0));
                          success = 1;
                          break;
                       default:
                          return luaL_argerror(L, n, "invalid format");
                    }
               }
          }
     }
   if (ferror(f))
      return push_ret(L, 0, NULL);
   if (!success)
     {
        lua_pop(L, 1);
        lua_pushnil(L);
     }
   return n - first;
}

static int
elua_write(lua_State *L)
{
   FILE *f    = tofile(L);
   int nargs  = lua_gettop(L) - 1;
   int status = 1, arg = 2;
   for (; nargs--; ++arg)
     {
        if (lua_type(L, arg) == LUA_TNUMBER)
           status = status && (fprintf(f, LUA_NUMBER_FMT,
                                       lua_tonumber(L, arg)) > 0);
        else
          {
             size_t l;
             const char *s = luaL_checklstring(L, arg, &l);
             status = status && (fwrite(s, sizeof(char), l, f) == l);
          }
     }
   return push_ret(L, status, NULL);
}

static int
elua_fgc(lua_State *L)
{
   FILE **f = (FILE**)luaL_checkudata(L, 1, "ELUA_FILE*");
   if (*f)
     {
        fclose(*f);
        *f = NULL;
     }
   return 0;
}

static int
elua_ftostring(lua_State *L)
{
   FILE *f = *((FILE**)luaL_checkudata(L, 1, "ELUA_FILE*"));
   if  (!f)
      lua_pushliteral(L, "file (closed)");
   else
      lua_pushfstring(L, "file (%p)", f);
   return 1;
}

static const luaL_reg elua_popenlib[] =
{
   { "close"     , elua_close     },
   { "flush"     , elua_flush     },
   { "lines"     , elua_lines     },
   { "read"      , elua_read      },
   { "write"     , elua_write     },
   { "__gc"      , elua_fgc       },
   { "__tostring", elua_ftostring },
   { NULL        , NULL           }
};

static FILE **
elua_newfile(lua_State *L)
{
   FILE **f = (FILE**)lua_newuserdata(L, sizeof(FILE*));
   *f = NULL;
   if (luaL_newmetatable(L, "ELUA_FILE*"))
     {
        lua_pushvalue(L, -1);
        lua_setfield (L, -2, "__index");
        luaL_register(L, NULL, elua_popenlib);
     }
   lua_setmetatable(L, -2);
   return f;
}

int
elua_popen(lua_State *L)
{
   const char *fname = luaL_checkstring(L, 1);
   const char *mode  = luaL_optstring(L, 2, "r");
   int nargs = lua_gettop(L) - 2;
   FILE **pf = elua_newfile(L);
   if (nargs > 0)
     {
        const char **argv = (const char**)alloca((nargs + 1) * sizeof(char*));
        memset(argv, 0, (nargs + 1) * sizeof(char*));
        for (; nargs; --nargs)
          {
             argv[nargs - 1] = lua_tostring(L, nargs + 2);
          }
        *pf = elua_popen_c(fname, mode, argv);
     }
   else
     {
        const char **argv = { NULL };
        *pf = elua_popen_c(fname, mode, argv);
     }
   return (!*pf) ? push_ret(L, 0, fname) : 1;
}