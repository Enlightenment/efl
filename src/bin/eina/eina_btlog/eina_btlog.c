/* EINA - EFL data type library
 * Copyright (C) 2015 Carsten Haitzler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#include <Eina.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// right now this is quick and dirty and may have some parsing ... frailty,
// so don't put malicious data through it... :) but cat in eina bt's through
// this to get a nicely clean and readable bt with filenames of binaries,
// shared objects, source files, and line numbers. even nicely colored and
// columnated. this is more the start of a bunch of debug tools for efl to make
// it easier to identify issues.
//
// how to use:
//
// cat mybacktrace.txt | eina_btlog
//
// (or just run it and copy & paste in on stdin - what i do mostly, and out
// pops a nice backtrace, hit ctrl+d to end)

#if defined (__MacOSX__) || (defined (__MACH__) && defined (__APPLE__))
# define ATOS_COMPATIBLE
#endif

typedef struct _Bt Bt;

struct _Bt
{
   char *bin_dir;
   char *bin_name;
   char *file_dir;
   char *file_name;
   char *func_name;
   char *comment;
   int line;
};

typedef Eina_Bool (*Translate_Func)(const char *prog,
                                    const char *bin_dir,
                                    const char *bin_name,
                                    unsigned long long addr,
                                    char **file_dir,
                                    char **file_name,
                                    char **func_name,
                                    int *file_line);

typedef struct _Translation_Desc Translation_Desc;

struct _Translation_Desc
{
   const char *name;
   const char *test;
   Translate_Func func;
   const char *prog;
};

static Translate_Func _translate = NULL;
static const char *_prog = NULL;
static Eina_Bool color = EINA_TRUE;
static Eina_Bool show_comments = EINA_TRUE;
static Eina_Bool show_compact = EINA_FALSE;

static void
path_split(const char *path, char **dir, char **file)
{
   const char *p;

   if (!path)
     {
        *dir = NULL;
        *file = NULL;
        return;
     }
   p = strrchr(path, '/');
   if (!p)
     {
        *dir = NULL;
        *file = strdup(path);
        return;
     }
   *dir = malloc(p - path + 1);
   if (!*dir)
     {
        *dir = NULL;
        *file = NULL;
        return;
     }
   strncpy(*dir, path, p - path);
   (*dir)[p - path] = 0;
   *file = strdup(p + 1);
}

static Eina_Bool
_addr2line(const char *prog, const char *bin_dir, const char *bin_name, unsigned long long addr,
           char **file_dir, char **file_name, char **func_name, int *file_line)
{
   char buf[4096], func[4096], *f1 = NULL, *f2 = NULL;
   Eina_Bool ok = EINA_FALSE;
   int line;
   FILE *p;

   snprintf(buf, sizeof(buf), "%s -f -e %s/%s -C -a 0x%llx",
            prog, bin_dir, bin_name, addr);
   p = popen(buf, "r");
   if (!p) return EINA_FALSE;
   if ((fscanf(p, "%4095s\n", buf) == 1) &&
       (fscanf(p, "%4095s\n", func) == 1))
     {
        if (fscanf(p, "%[^:]:%i\n", buf, &line) == 2)
          {
             path_split(buf, &(f1), &(f2));
             if ((!f1) || (!f2))
               {
                  free(f1);
                  free(f2);
                  pclose(p);
                  return EINA_FALSE;
               }
          }
        else
          {
             f1 = strdup("??");
             f2 = strdup("??");
          }
        *file_dir = f1;
        *file_name = f2;
        *func_name = strdup(func);
        *file_line = line;
        ok = EINA_TRUE;
     }
   pclose(p);
   return ok;
}

#ifdef ATOS_COMPATIBLE
static Eina_Bool
_atos(const char *prog, const char *bin_dir, const char *bin_name, unsigned long long addr,
      char **file_dir, char **file_name, char **func_name, int *file_line)
{
   char buf[4096];
   FILE *p = NULL;
   char *f1 = NULL, *s;
   Eina_Bool ret = EINA_FALSE;
   unsigned int count = 0, len;
   Eina_Bool func_done = EINA_FALSE;
   unsigned int spaces = 0, func_space_count;

   // Example of what we want to parse
   // $ atos -o /usr/local/lib/libevas.1.dylib 0xa82d
   // evas_object_clip_recalc (in libevas.1.dylib) (evas_inline.x:353)
   //
   // WARNING! Sometimes:
   // tlv_load_notification (in libdyld.dylib) + 382
   //
   // WARNING! Objective-C methods:
   // -[EcoreCocoaWindow windowDidResize:] (in libecore_cocoa.1.dylib) (ecore_cocoa_window.m:97)

   snprintf(buf, sizeof(buf), "%s -o %s/%s 0x%llx", prog, bin_dir, bin_name, addr);
   p = popen(buf, "r");
   if (!p) goto end;

   s = fgets(buf, sizeof(buf), p);
   if (!s) goto end;

   /* Default value, used as a fallback when cannot be determined */
   *file_line = -1;

   if ((*s == '-') || (*s == '+')) /* objc methods... will contain an extra space */
     func_space_count = 2;
   else
     func_space_count = 1;

   do
     {
        if (*s == ' ') spaces++;

        if ((spaces == func_space_count) && (func_done == EINA_FALSE))
          {
             *s = '\0';
             *func_name = strndup(buf, (int)(s - &(buf[0])));
             func_done = EINA_TRUE;
          }
        else if (*s == '(')
          {
             count++;
             if ((count == 2) && (f1 == NULL))
               {
                  f1 = s + 1; /* skip the leading '(' */
               }
          }
        else if ((*s == ':') && (func_done == EINA_TRUE))
          {
             *s = '\0';
             *file_name = strndup(f1, (int)(s - f1));
             s++;
             len = strlen(s);
             s[len - 1] = '\0'; /* Remove the closing parenthesis */
             *file_line = atoi(s);
             break; /* Done */
          }
     }
   while (*(++s) != '\0');

   /* Cannot be determined */
   *file_dir = strdup("??");

   if (!*func_name) *func_name = strdup("??");
   if (!*file_name) *file_name = strdup("??");

   ret = EINA_TRUE;
end:
   if (p) pclose(p);
   return ret;
}
#endif

static const char *
bt_input_translate(const char *line, char **comment)
{
   static char local[PATH_MAX + sizeof(" 0x1234567890123456789 0x1234567890123456789\n")];
   const char *addrstart, *addrend, *filestart, *fileend, *basestart, *baseend;

   /* new bt format is more human readable, but needs some cleanup before we bt_append()
    *
    * Example:
    *   ERR<23314>:eo_lifecycle ../src/lib/eo/efl_object.eo.c:78 efl_del()    0x00000005c7c291: __libc_start_main+0xf1 (in /usr/lib/libc.so.6 0x5c5c000)
    *   ERR<23314>:eo_lifecycle ../src/lib/eo/efl_object.eo.c:78 efl_del()    0x00000004e409aa: libeo_dbg.so+0x99aa (in src/lib/eo/.libs/libeo_dbg.so 0x4e37000)
    */
   *comment = NULL;

   addrstart = strstr(line, "0x");
   if (!addrstart) return NULL;

   addrend = strchr(addrstart, ':');
   if (!addrend) return NULL;

   filestart = strstr(addrend, "(in ");
   if (!filestart) return NULL;

   filestart += strlen("(in ");
   basestart = strstr(filestart, " 0x");
   if (!basestart) return NULL;
   fileend = basestart;
   basestart += strlen(" ");
   baseend = strchr(basestart, ')');
   if (!baseend) return NULL;

   snprintf(local, sizeof(local), "%.*s %.*s %.*s\n",
            (int)(fileend - filestart), filestart,
            (int)(addrend - addrstart), addrstart,
            (int)(baseend - basestart), basestart);
   *comment = strndup(line, addrstart - line);
   return local;
}

static Eina_List *
bt_append(Eina_List *btl, const char *btline)
{
   Bt *bt = calloc(1, sizeof(Bt));
   if (!bt) return btl;
   const char *translation;
   char *comment = NULL;
   char *bin = strdup(btline);
   unsigned long long offset = 0, base = 0;

   translation = bt_input_translate(btline, &comment);
   if (translation)
     btline = translation;

   // parse:
   // /usr/local/lib/libeina.so.1 0x1ec88
   // /usr/local/lib/libelementary.so.1 0x10f695
   // /usr/local/lib/libeo.so.1 0xa474
   // /usr/local/lib/libelementary.so.1 0x139bd6
   // /usr/local/bin/elementary_test 0x8196d
   // /usr/local/bin/elementary_test 0x81b6a
   if (sscanf(btline, "%s %llx %llx", bin, &offset, &base) == 3)
     {
        path_split(bin, &(bt->bin_dir), &(bt->bin_name));
        if (!bt->bin_dir) bt->bin_dir = strdup("");
        if (!bt->bin_name) bt->bin_name = strdup("");
        if (!_translate(_prog, bt->bin_dir, bt->bin_name, offset - base,
                        &(bt->file_dir), &(bt->file_name),
                        &(bt->func_name), &(bt->line)))
          {
             if (!_translate(_prog, bt->bin_dir, bt->bin_name, offset,
                             &(bt->file_dir), &(bt->file_name),
                             &(bt->func_name), &(bt->line)))
               {
                  bt->file_dir = strdup("");
                  bt->file_name = strdup("");
                  bt->func_name = strdup("");
               }
          }
        bt->comment = comment;
        btl = eina_list_append(btl, bt);
     }
   else
     {
        free(comment);
        bt->comment = strdup(btline);
        btl = eina_list_append(btl, bt);
     }
   free(bin);

   return btl;
}

static Eina_Bool
_translation_function_detect(const Translation_Desc *desc)
{
   const Translation_Desc *d = desc;
   FILE *p;
   int ret;

   while ((d->name != NULL) && (d->func != NULL) && (d->test != NULL))
     {
         p = popen(d->test, "r");
         if (p)
           {
              ret = pclose(p);
              if (ret == 0)
                {
                   _translate = d->func;
                   _prog = d->prog;
                   break;
                }
           }
         d++;
     }

   return (_translate == NULL) ? EINA_FALSE : EINA_TRUE;
}

int
main(int argc, char **argv)
{
   Eina_List *btl = NULL, *l;
   char buf[4096];
   Bt *bt;
   int cols[6] = { 0 }, len, i;
   const char *func_color = "";
   const char *dir_color = "";
   const char *sep_color = "";
   const char *file_color = "";
   const char *line_color = "";
   const char *reset_color = "";
   const Translation_Desc desc[] = {
#ifdef ATOS_COMPATIBLE
        { /* Mac OS X */
           .name = "atos",
           .test = "atos --help &> /dev/null",
           .func = _atos,
           .prog = "atos"
        },
        { /* Mac OS X */
           .name = "atos (old)",
           .test = "xcrun atos --help &> /dev/null",
           .func = _atos,
           .prog = "xcrun atos"
        },
#endif
        { /* GNU binutils */
           .name = "addr2line",
           .test = "addr2line --help &> /dev/null",
           .func = _addr2line,
           .prog = "addr2line"
        },
        { /* For imported GNU binutils */
           .name = "GNU addr2line",
           .test = "gaddr2line --help &> /dev/null",
           .func = _addr2line,
           .prog = "addr2line"
        },
        { NULL, NULL, NULL, NULL } /* Sentinel */
   };

   eina_init();

   for (i = 1; i < argc; i++)
     {
        if (!strcmp(argv[i], "-h"))
          {
             printf("Usage: eina_btlog [-n]\n"
                    "  -n   Do not use color escape codes\n"
                    "  -C   Do not show comments (non-bt fragments)\n"
                    "  -c   Show compact output format\n"
                    "\n"
                    "Provide addresses logged from EFL applications to stdin.\n"
                    "Example:\n\n"
                    "\tcat log.txt | eina_btlog\n"
                    "\n");
             eina_shutdown();
             return 0;
          }
        else if (!strcmp(argv[i], "-n")) color = EINA_FALSE;
        else if (!strcmp(argv[i], "-C")) show_comments = EINA_FALSE;
        else if (!strcmp(argv[i], "-c")) show_compact = EINA_TRUE;
     }

   if (color)
     {
        func_color = EINA_COLOR_GREEN;
        dir_color = EINA_COLOR_BLUE;
        sep_color = EINA_COLOR_CYAN;
        file_color = EINA_COLOR_WHITE;
        line_color = EINA_COLOR_YELLOW;
        reset_color = EINA_COLOR_RESET;
     }

   if (!_translation_function_detect(desc))
     {
        EINA_LOG_CRIT("Fail to determine a program to translate backtrace "
                      "into human-readable text");
        return 1;
     }

 repeat:
   while (fgets(buf, sizeof(buf) - 1, stdin))
     {
        btl = bt_append(btl, buf);
        if (show_compact) goto do_show;
        bt = eina_list_last_data_get(btl);
        if (bt && !bt->bin_dir) break; /* flush once first non-bt is found */
     }

   /* compute columns for expanded display */
   for (i = 0; i < 6; i++) cols[i] = 0;
   EINA_LIST_FOREACH(btl, l, bt)
     {
        if (!bt->bin_dir) continue;
        len = strlen(bt->bin_dir);
        if (len > cols[0]) cols[0] = len;
        len = strlen(bt->bin_name);
        if (len > cols[1]) cols[1] = len;

        len = strlen(bt->file_dir);
        if (len > cols[2]) cols[2] = len;
        len = strlen(bt->file_name);
        if (len > cols[3]) cols[3] = len;

        snprintf(buf, sizeof(buf), "%i", bt->line);
        len = strlen(buf);
        if (len > cols[4]) cols[4] = len;

        len = strlen(bt->func_name);
        if (len > cols[5]) cols[5] = len;
     }

 do_show:
   EINA_LIST_FOREACH(btl, l, bt)
     {
        if (bt->comment && show_comments)
          fputs(bt->comment, stdout);
        if (!bt->bin_dir) continue;

        if (show_compact)
          {
             printf("%s%s%s (in %s%s%s:%s%d%s)\n",
                    func_color, bt->func_name, reset_color,
                    file_color, bt->file_name, reset_color,
                    line_color, bt->line, reset_color);
             fflush(stdout);
             continue;
          }

        printf("    "
               "%s%*s%s/%s%-*s%s" /* bin info */
               "| "
               "%s%*s%s/%s%-*s%s" /* file info */
               ": "
               "%s%*i%s" /* line info */
               " @ "
               "%s%s%s()%s\n", /* func info */
               /* bin info */
               dir_color, cols[0], bt->bin_dir, sep_color,
               file_color, cols[1], bt->bin_name,
               reset_color,
               /* file info */
               dir_color, cols[2], bt->file_dir, sep_color,
               file_color, cols[3], bt->file_name,
               reset_color,
               /* line info */
               line_color, cols[4], bt->line,
               reset_color,
               /* func info */
               func_color, bt->func_name,
               sep_color,
               reset_color);
     }
   fflush(stdout);
   EINA_LIST_FREE(btl, bt)
     {
        free(bt->bin_dir);
        free(bt->bin_name);
        free(bt->file_dir);
        free(bt->file_name);
        free(bt->func_name);
        free(bt->comment);
        free(bt);
     }
   /* if not EOF, then we just flushed due non-bt line, try again */
   if (!feof(stdin)) goto repeat;

   eina_shutdown();

   return 0;
}
