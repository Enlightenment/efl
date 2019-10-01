/* CPP Library.
 * Copyright (C) 1986, 87, 89, 92, 93, 94, 1995 Free Software Foundation, Inc.
 * Written by Per Bothner, 1994-95.
 * Based on CCCP program by by Paul Rubin, June 1986
 * Adapted to ANSI C, Richard Stallman, Jan 1987
 * Copyright (C) 2003-2011 Kim Woelders
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 * 
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Help stamp out software-hoarding!  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#ifdef __EMX__
#include <strings.h>
#endif

#ifndef STANDARD_INCLUDE_DIR
#define STANDARD_INCLUDE_DIR "/usr/include"
#endif

#ifndef LOCAL_INCLUDE_DIR
#define LOCAL_INCLUDE_DIR "/usr/local/include"
#endif

#include "cpplib.h"
#include "cpphash.h"

/*
 * On Windows, if the file is not opened in binary mode,
 * read does not return the correct size, because of
 * CR / LF translation.
 */
#ifndef O_BINARY
# define O_BINARY 0
#endif

const char         *version_string = "0.0.0";

#ifndef STDC_VALUE
#define STDC_VALUE 1
#endif

/* By default, colon separates directories in a path.  */
#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR ':'
#endif

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#ifndef VMS
#ifndef USG
#include <time.h>
#include <sys/time.h>		/* for __DATE__ and __TIME__ */
#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif
#else
#include <sys/param.h>		/* CYGNUS LOCAL: shebs -noquiet */
#include <sys/times.h>
#include <time.h>
#include <fcntl.h>
#endif /* USG */
#endif /* not VMS */

/* This defines "errno" properly for VMS, and gives us EACCES. */
#include <errno.h>

#ifndef O_RDONLY
#define O_RDONLY 0
#endif

#undef MIN
#undef MAX
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

/* Define a generic NULL if one hasn't already been defined.  */

#ifndef GENERIC_PTR
#if defined (USE_PROTOTYPES) ? USE_PROTOTYPES : defined (__STDC__)
#define GENERIC_PTR void *
#else
#define GENERIC_PTR char *
#endif
#endif

#ifndef INCLUDE_LEN_FUDGE
#define INCLUDE_LEN_FUDGE 0
#endif

#define USE_FILE_NAME_MAPS 0

/* Symbols to predefine.  */

#ifdef CPP_PREDEFINES
static const char  *predefs = CPP_PREDEFINES;

#else
static const char  *predefs = "";

#endif

/* We let tm.h override the types used here, to handle trivial differences
 * such as the choice of unsigned int or long unsigned int for size_t.
 * When machines start needing nontrivial differences in the size type,
 * it would be best to do something here to figure out automatically
 * from other information what type to use.  */

/* The string value for __SIZE_TYPE__.  */

#ifndef SIZE_TYPE
#define SIZE_TYPE "long unsigned int"
#endif

/* The string value for __PTRDIFF_TYPE__.  */

#ifndef PTRDIFF_TYPE
#define PTRDIFF_TYPE "long int"
#endif

/* The string value for __WCHAR_TYPE__.  */

#ifndef WCHAR_TYPE
#define WCHAR_TYPE "int"
#endif
#define CPP_WCHAR_TYPE(PFILE) \
	(CPP_OPTIONS (PFILE)->cplusplus ? "__wchar_t" : WCHAR_TYPE)

/* The string value for __USER_LABEL_PREFIX__ */

#ifndef USER_LABEL_PREFIX
#define USER_LABEL_PREFIX ""
#endif

/* The string value for __REGISTER_PREFIX__ */

#ifndef REGISTER_PREFIX
#define REGISTER_PREFIX ""
#endif

struct directive {
   int                 length;
   int                 (*func) (cpp_reader * pfile, struct directive * keyword,
				unsigned char *buf, unsigned char *limit);
   const char         *name;
   enum node_type      type;
   char                command_reads_line;
   char                traditional_comments;
   char                pass_thru;
};

/* In the definition of a #assert name, this structure forms
 * a list of the individual values asserted.
 * Each value is itself a list of "tokens".
 * These are strings that are compared by name.  */

struct tokenlist_list {
   struct tokenlist_list *next;
   struct arglist     *tokens;
};

struct assertion_hashnode {
   struct assertion_hashnode *next;	/* double links for easy deletion */
   struct assertion_hashnode *prev;
   /* also, a back pointer to this node's hash
    * chain is kept, in case the node is the head
    * of the chain and gets deleted. */
   struct assertion_hashnode **bucket_hdr;
   int                 length;	/* length of token, for quick comparison */
   char               *name;	/* the actual name */
   /* List of token-sequences.  */
   struct tokenlist_list *value;
};

#define SKIP_WHITE_SPACE(p) do { while (is_hor_space[(unsigned char)(*p)]) p++; } while (0)
#define SKIP_ALL_WHITE_SPACE(p) do { while (is_space[(unsigned char)(*p)]) p++; } while (0)

#define PEEKN(N) (CPP_BUFFER (pfile)->rlimit - CPP_BUFFER (pfile)->cur >= (N) ? CPP_BUFFER (pfile)->cur[N] : EOF)
#define FORWARD(N) CPP_FORWARD (CPP_BUFFER (pfile), (N))
#define GETC() CPP_BUF_GET (CPP_BUFFER (pfile))
#define PEEKC() CPP_BUF_PEEK (CPP_BUFFER (pfile))
/* CPP_IS_MACRO_BUFFER is true if the buffer contains macro expansion.
 * (Note that it is false while we're expanding marco *arguments*.) */
#define CPP_IS_MACRO_BUFFER(PBUF) ((PBUF)->cleanup == macro_cleanup)

/* Move all backslash-newline pairs out of embarrassing places.
 * Exchange all such pairs following BP
 * with any potentially-embarrassing characters that follow them.
 * Potentially-embarrassing characters are / and *
 * (because a backslash-newline inside a comment delimiter
 * would cause it not to be recognized).  */

#define NEWLINE_FIX \
  do {while (PEEKC() == '\\' && PEEKN(1) == '\n') FORWARD(2); } while(0)

/* Same, but assume we've already read the potential '\\' into C. */
#define NEWLINE_FIX1(C) do { \
    while ((C) == '\\' && PEEKC() == '\n') { FORWARD(1); (C) = GETC(); }\
  } while(0)

/* Name under which this program was invoked.  */

char               *progname;

struct cpp_pending {
   struct cpp_pending *next;
   const char         *cmd;
   const char         *arg;
};

/* Structure returned by create_definition */
typedef struct {
   struct definition  *defn;
   char               *symnam;
   int                 symlen;
} MACRODEF;

/* Forward declarations.  */
typedef struct file_name_list file_name_list;

static void         add_import(cpp_reader * pfile, int fd, char *fname);
static int          finclude(cpp_reader * pfile, int f, const char *fname,
			     int system_header_p, file_name_list * dirptr);
static void         validate_else(cpp_reader * pfile, const char *directive);
static int          comp_def_part(int first, unsigned char *beg1, int len1,
				  unsigned char *beg2, int len2, int last);
static int          lookup_import(cpp_reader * pfile, char *filename,
				  file_name_list * searchptr);
static int          redundant_include_p(cpp_reader * pfile, char *name);

static int          is_system_include(cpp_reader * pfile, char *filename);

static int          open_include_file(cpp_reader * pfile, char *filename,
				      file_name_list * searchptr);
static int          check_macro_name(cpp_reader * pfile, unsigned char *symname,
				     const char *usage);

static int          compare_token_lists(struct arglist *l1, struct arglist *l2);
static HOST_WIDE_INT eval_if_expression(cpp_reader * pfile, unsigned char *buf,
					int length);

static int          file_size_and_mode(int fd, int *mode_pointer,
				       long int *size_pointer);
static struct arglist *read_token_list(cpp_reader * pfile, int *error_flag);
static void         free_token_list(struct arglist *tokens);
static int          safe_read(int desc, char *ptr, int len);
static void         push_macro_expansion(cpp_reader * pfile,
					 unsigned char *x,
					 int xbuf_len, HASHNODE * hp);

static struct cpp_pending *nreverse_pending(struct cpp_pending *list);
static char        *savestring(const char *input);

static void         conditional_skip(cpp_reader * pfile, int skip,
				     enum node_type type,
				     unsigned char *control_macro);
static void         skip_if_group(cpp_reader * pfile, int any);

static void         cpp_error_with_line(cpp_reader * pfile, int line,
					int column, const char *msg);
static void         cpp_pedwarn_with_line(cpp_reader * pfile, int line,
					  int column, const char *msg);
static void         cpp_pedwarn_with_file_and_line(cpp_reader * pfile,
						   const char *file, int line,
						   const char *msg,
						   const char *arg1,
						   const char *arg2,
						   const char *arg3);
static void         cpp_error_from_errno(cpp_reader * pfile, const char *name);

static cpp_buffer  *cpp_push_buffer(cpp_reader * pfile, unsigned char *buffer,
				    long length);
static cpp_buffer  *cpp_pop_buffer(cpp_reader * pfile);

/* Last arg to output_line_command.  */
enum file_change_code {
   same_file, enter_file, leave_file
};

/* These functions are declared to return int instead of void since they
 * are going to be placed in a table and some old compilers have trouble with
 * pointers to functions returning void.  */

static int          do_define(cpp_reader * pfile, struct directive *keyword,
			      unsigned char *buf, unsigned char *limit);

static int          do_line(cpp_reader * pfile, struct directive *keyword,
			    unsigned char *unused1, unsigned char *unused2);

static int          do_include(cpp_reader * pfile, struct directive *keyword,
			       unsigned char *unused1, unsigned char *unused2);

static int          do_undef(cpp_reader * pfile, struct directive *keyword,
			     unsigned char *buf, unsigned char *limit);

static int          do_error(cpp_reader * pfile, struct directive *keyword,
			     unsigned char *buf, unsigned char *limit);

static int          do_pragma(cpp_reader * pfile, struct directive *keyword,
			      unsigned char *buf, unsigned char *limit);

static int          do_ident(cpp_reader * pfile, struct directive *keyword,
			     unsigned char *buf, unsigned char *limit);

static int          do_if(cpp_reader * pfile, struct directive *keyword,
			  unsigned char *buf, unsigned char *limit);

static int          do_xifdef(cpp_reader * pfile, struct directive *keyword,
			      unsigned char *buf, unsigned char *limit);

static int          do_else(cpp_reader * pfile, struct directive *keyword,
			    unsigned char *buf, unsigned char *limit);

static int          do_elif(cpp_reader * pfile, struct directive *keyword,
			    unsigned char *buf, unsigned char *limit);

static int          do_endif(cpp_reader * pfile, struct directive *keyword,
			     unsigned char *buf, unsigned char *limit);

static int          do_assert(cpp_reader * pfile, struct directive *keyword,
			      unsigned char *buf, unsigned char *limit);

static int          do_unassert(cpp_reader * pfile, struct directive *keyword,
				unsigned char *buf, unsigned char *limit);

static int          do_warning(cpp_reader * pfile, struct directive *keyword,
			       unsigned char *buf, unsigned char *limit);

struct arglist     *reverse_token_list(struct arglist *tokens);

static int          parse_name(cpp_reader * pfile, int c);

static void         parse_set_mark(struct parse_marker *pmark,
				   cpp_reader * pfile);
static void         parse_clear_mark(struct parse_marker *pmark);
static void         parse_goto_mark(struct parse_marker *pmark,
				    cpp_reader * pfile);
static void         parse_move_mark(struct parse_marker *pmark,
				    cpp_reader * pfile);

struct file_name_list {
   file_name_list     *next;
   char               *fname;
   /* If the following is nonzero, it is a macro name.
    * Don't include the file again if that macro is defined.  */
   unsigned char      *control_macro;
   /* If the following is nonzero, it is a C-language system include
    * directory.  */
   int                 c_system_include_path;
   /* Mapping of file names for this directory.  */
   struct file_name_map *name_map;
   /* Non-zero if name_map is valid.  */
   int                 got_name_map;
};

/* If a buffer's dir field is SELF_DIR_DUMMY, it means the file was found
 * via the same directory as the file that #included it. */
#define SELF_DIR_DUMMY ((file_name_list*)(~0))

/* #include "file" looks in source file dir, then stack. */
/* #include <file> just looks in the stack. */
/* -I directories are added to the end, then the defaults are added. */
/* The */
static struct default_include {
   const char         *fname;	/* The name of the directory.  */
   int                 cplusplus;	/* Only look here if we're compiling C++.  */
   int                 cxx_aware;	/* Includes in this directory don't need to
					 * be wrapped in extern "C" when compiling
					 * C++.  */
} include_defaults_array[]
#ifdef INCLUDE_DEFAULTS
   = INCLUDE_DEFAULTS;

#else
   =
{
   /* Pick up GNU C++ specific include files.  */
   {
   GPLUSPLUS_INCLUDE_DIR, 1, 1}
   ,
#ifdef CROSS_COMPILE
      /* This is the dir for fixincludes.  Put it just before
       * the files that we fix.  */
   {
   GCC_INCLUDE_DIR, 0, 0}
   ,
      /* For cross-compilation, this dir name is generated
       * automatically in Makefile.in.  */
   {
   CROSS_INCLUDE_DIR, 0, 0}
   ,
      /* This is another place that the target system's headers might be.  */
   {
   TOOL_INCLUDE_DIR, 0, 1}
   ,
#else /* not CROSS_COMPILE */
      /* This should be /usr/local/include and should come before
       * the fixincludes-fixed header files.  */
   {
   LOCAL_INCLUDE_DIR, 0, 1}
   ,
      /* This is here ahead of GCC_INCLUDE_DIR because assert.h goes here.
       * Likewise, behind LOCAL_INCLUDE_DIR, where glibc puts its assert.h.  */
   {
   TOOL_INCLUDE_DIR, 0, 1}
   ,
      /* This is the dir for fixincludes.  Put it just before
       * the files that we fix.  */
   {
   GCC_INCLUDE_DIR, 0, 0}
   ,
      /* Some systems have an extra dir of include files.  */
#ifdef SYSTEM_INCLUDE_DIR
   {
   SYSTEM_INCLUDE_DIR, 0, 0}
   ,
#endif
   {
   STANDARD_INCLUDE_DIR, 0, 0}
   ,
#endif /* not CROSS_COMPILE */
   {
   0, 0, 0}
};

#endif /* no INCLUDE_DEFAULTS */

/* Here is the actual list of #-directives, most-often-used first.
 * The initialize_builtins function assumes #define is the very first.  */

static struct directive directive_table[] = {
   {6, do_define, "define", T_DEFINE, 0, 1, 0},
   {5, do_xifdef, "ifdef", T_IFDEF, 1, 0, 0},
   {6, do_xifdef, "ifndef", T_IFNDEF, 1, 0, 0},
   {7, do_include, "include", T_INCLUDE, 1, 0, 0},
   {12, do_include, "include_next", T_INCLUDE_NEXT, 1, 0, 0},
   {6, do_include, "import", T_IMPORT, 1, 0, 0},
   {5, do_endif, "endif", T_ENDIF, 1, 0, 0},
   {4, do_else, "else", T_ELSE, 1, 0, 0},
   {2, do_if, "if", T_IF, 1, 0, 0},
   {4, do_elif, "elif", T_ELIF, 1, 0, 0},
   {5, do_undef, "undef", T_UNDEF, 0, 0, 0},
   {5, do_error, "error", T_ERROR, 0, 0, 0},
   {7, do_warning, "warning", T_WARNING, 0, 0, 0},
   {6, do_pragma, "pragma", T_PRAGMA, 0, 0, 1},
   {4, do_line, "line", T_LINE, 1, 0, 0},
   {5, do_ident, "ident", T_IDENT, 1, 0, 1},
#ifdef SCCS_DIRECTIVE
   {4, do_sccs, "sccs", T_SCCS, 0, 0, 0},
#endif
   {6, do_assert, "assert", T_ASSERT, 1, 0, 0},
   {8, do_unassert, "unassert", T_UNASSERT, 1, 0, 0},
   {-1, 0, "", T_UNUSED, 0, 0, 0},
};

/* table to tell if char can be part of a C identifier. */
unsigned char       is_idchar[256];

/* table to tell if char can be first char of a c identifier. */
unsigned char       is_idstart[256];

/* table to tell if c is horizontal space.  */
unsigned char       is_hor_space[256];

/* table to tell if c is horizontal or vertical space.  */
static unsigned char is_space[256];

static int           annotate = 0;

/* Initialize syntactic classifications of characters.  */

static void
initialize_char_syntax(struct cpp_options *opts)
{
   int                 i;

   /*
    * Set up is_idchar and is_idstart tables.  These should be
    * faster than saying (is_alpha (c) || c == '_'), etc.
    * Set up these things before calling any routines tthat
    * refer to them.
    */
   for (i = 'a'; i <= 'z'; i++)
     {
	is_idchar[i - 'a' + 'A'] = 1;
	is_idchar[i] = 1;
	is_idstart[i - 'a' + 'A'] = 1;
	is_idstart[i] = 1;
     }
   for (i = '0'; i <= '9'; i++)
      is_idchar[i] = 1;
   is_idchar[(unsigned char)'_'] = 1;
   is_idstart[(unsigned char)'_'] = 1;
   is_idchar[(unsigned char)'$'] = opts->dollars_in_ident;
   is_idstart[(unsigned char)'$'] = opts->dollars_in_ident;

   /* horizontal space table */
   is_hor_space[(unsigned char)' '] = 1;
   is_hor_space[(unsigned char)'\t'] = 1;
   is_hor_space[(unsigned char)'\v'] = 1;
   is_hor_space[(unsigned char)'\f'] = 1;
   is_hor_space[(unsigned char)'\r'] = 1;

   is_space[(unsigned char)' '] = 1;
   is_space[(unsigned char)'\t'] = 1;
   is_space[(unsigned char)'\v'] = 1;
   is_space[(unsigned char)'\f'] = 1;
   is_space[(unsigned char)'\n'] = 1;
   is_space[(unsigned char)'\r'] = 1;
}

/* Place into PFILE a quoted string representing the string SRC.
 * Caller must reserve enough space in pfile->token_buffer. */
static void
quote_string(cpp_reader * pfile, const char *src)
{
   unsigned char       c;

   CPP_PUTC_Q(pfile, '\"');
   for (;;)
      switch ((c = *src++))
	{
	default:
	   if (isprint(c))
	      CPP_PUTC_Q(pfile, c);
	   else
	     {
		sprintf((char *)CPP_PWRITTEN(pfile), "\\%03o", c);
		CPP_ADJUST_WRITTEN(pfile, 4);
	     }
	   break;

	case '\"':
	case '\\':
	   CPP_PUTC_Q(pfile, '\\');
	   CPP_PUTC_Q(pfile, c);
	   break;

	case '\0':
	   CPP_PUTC_Q(pfile, '\"');
	   CPP_NUL_TERMINATE_Q(pfile);
	   return;
	}
}

/* Make sure PFILE->token_buffer will hold at least N more chars. */

void
cpp_grow_buffer(cpp_reader * pfile, long n)
{
   long                old_written = CPP_WRITTEN(pfile);

   pfile->token_buffer_size = n + 2 * pfile->token_buffer_size;
   pfile->token_buffer =
      (unsigned char *)xrealloc(pfile->token_buffer, pfile->token_buffer_size);
   CPP_SET_WRITTEN(pfile, old_written);
}

/*
 * process a given definition string, for initialization
 * If STR is just an identifier, define it with value 1.
 * If STR has anything after the identifier, then it should
 * be identifier=definition.
 */

void
cpp_define(cpp_reader * pfile, unsigned char *str)
{
   unsigned char      *buf, *p;

   buf = str;
   p = str;
   if (!is_idstart[*p])
     {
	cpp_error(pfile, "malformed option `-D %s'", str);
	return;
     }
   while (is_idchar[*++p])
      ;
   if (*p == 0)
     {
	buf = (unsigned char *)alloca(p - buf + 4);
	strcpy((char *)buf, (const char *)str);
	strcat((char *)buf, " 1");
     }
   else if (*p != '=')
     {
	cpp_error(pfile, "malformed option `-D %s'", str);
	return;
     }
   else
     {
	unsigned char      *q;

	/* Copy the entire option so we can modify it.  */
	buf = (unsigned char *)alloca(2 * strlen((char *)str) + 1);
	strncpy((char *)buf, (const char *)str, p - str);
	/* Change the = to a space.  */
	buf[p - str] = ' ';
	/* Scan for any backslash-newline and remove it.  */
	p++;
	q = &buf[p - str];
	while (*p)
	  {
	     if (*p == '\\' && p[1] == '\n')
		p += 2;
	     else
		*q++ = *p++;
	  }
	*q = 0;
     }

   do_define(pfile, NULL, buf, buf + strlen((char *)buf));
}

/* Process the string STR as if it appeared as the body of a #assert.
 * OPTION is the option name for which STR was the argument.  */

static void
make_assertion(cpp_reader * pfile, const char *option, const char *str)
{
   unsigned char      *buf, *p, *q;

   /* Copy the entire option so we can modify it.  */
   buf = (unsigned char *)alloca(strlen((char *)str) + 1);
   strcpy((char *)buf, (const char *)str);
   /* Scan for any backslash-newline and remove it.  */
   p = q = buf;
   while (*p)
     {
	*q++ = *p++;
     }
   *q = 0;

   p = buf;
   if (!is_idstart[*p])
     {
	cpp_error(pfile, "malformed option `%s %s'", option, str);
	return;
     }
   while (is_idchar[*++p])
      ;
   while (*p == ' ' || *p == '\t')
      p++;
   if (!(*p == 0 || *p == '('))
     {
	cpp_error(pfile, "malformed option `%s %s'", option, str);
	return;
     }
   cpp_push_buffer(pfile, buf, strlen((char *)buf));
   do_assert(pfile, NULL, NULL, NULL);
   cpp_pop_buffer(pfile);
}

/* Append a chain of `file_name_list's
 * to the end of the main include chain.
 * FIRST is the beginning of the chain to append, and LAST is the end.  */

static void
append_include_chain(cpp_reader * pfile, file_name_list * first,
		     file_name_list * last)
{
   struct cpp_options *opts = CPP_OPTIONS(pfile);
   file_name_list     *dir;

   if (!first || !last)
      return;

   if (!opts->include)
      opts->include = first;
   else
      opts->last_include->next = first;

   if (!opts->first_bracket_include)
      opts->first_bracket_include = first;

   for (dir = first;; dir = dir->next)
     {
	int                 len = strlen(dir->fname) + INCLUDE_LEN_FUDGE;

	if (len > pfile->max_include_len)
	   pfile->max_include_len = len;
	if (dir == last)
	   break;
     }

   last->next = NULL;
   opts->last_include = last;
}

/* Add output to `deps_buffer' for the -M switch.
 * STRING points to the text to be output.
 * SPACER is ':' for targets, ' ' for dependencies, zero for text
 * to be inserted literally.  */

static void
deps_output(cpp_reader * pfile, const char *string, int spacer)
{
   int                 size = strlen(string);

   if (size == 0)
      return;

#ifndef MAX_OUTPUT_COLUMNS
#define MAX_OUTPUT_COLUMNS 72
#endif
   if (spacer
       && pfile->deps_column > 0
       && (pfile->deps_column + size) > MAX_OUTPUT_COLUMNS)
     {
	deps_output(pfile, " \\\n  ", 0);
	pfile->deps_column = 0;
     }
   if (pfile->deps_size + size + 8 > pfile->deps_allocated_size)
     {
	pfile->deps_allocated_size = (pfile->deps_size + size + 50) * 2;
	pfile->deps_buffer = (char *)xrealloc(pfile->deps_buffer,
					      pfile->deps_allocated_size);
     }
   if (spacer == ' ' && pfile->deps_column > 0)
      pfile->deps_buffer[pfile->deps_size++] = ' ';
   memcpy(&pfile->deps_buffer[pfile->deps_size], string, size);
   pfile->deps_size += size;
   pfile->deps_column += size;
   if (spacer == ':')
      pfile->deps_buffer[pfile->deps_size++] = ':';
   pfile->deps_buffer[pfile->deps_size] = 0;
}

/* Given a colon-separated list of file names PATH,
 * add all the names to the search path for include files.  */

static void
path_include(cpp_reader * pfile, char *path)
{
   char               *p;

   p = path;

   if (*p)
      while (1)
	{
	   char               *q = p;
	   char               *name;
	   file_name_list     *dirtmp;

	   /* Find the end of this name.  */
	   while (*q != 0 && *q != PATH_SEPARATOR)
	      q++;
	   if (p == q)
	     {
		/* An empty name in the path stands for the current directory.  */
		name = (char *)xmalloc(2);
		name[0] = '.';
		name[1] = 0;
	     }
	   else
	     {
		/* Otherwise use the directory that is named.  */
		name = (char *)xmalloc(q - p + 1);
		memcpy(name, p, q - p);
		name[q - p] = 0;
	     }

	   dirtmp = (file_name_list *) xmalloc(sizeof(file_name_list));

	   dirtmp->next = 0;	/* New one goes on the end */
	   dirtmp->control_macro = 0;
	   dirtmp->c_system_include_path = 0;
	   dirtmp->fname = name;
	   dirtmp->got_name_map = 0;
	   append_include_chain(pfile, dirtmp, dirtmp);

	   /* Advance past this name.  */
	   p = q;
	   if (*p == 0)
	      break;
	   /* Skip the colon.  */
	   p++;
	}
}

void
init_parse_options(struct cpp_options *opts)
{
   memset((char *)opts, 0, sizeof *opts);
   opts->in_fname = NULL;
   opts->out_fname = NULL;

   /* Initialize is_idchar to allow $.  */
   opts->dollars_in_ident = 1;
   initialize_char_syntax(opts);
   opts->dollars_in_ident = DOLLARS_IN_IDENTIFIERS > 0;

   opts->no_line_commands = 0;
   opts->no_trigraphs = 1;
   opts->put_out_comments = 0;
   opts->print_include_names = 0;
   opts->dump_macros = dump_none;
   opts->no_output = 0;
   opts->cplusplus = 0;
   opts->cplusplus_comments = 1;

   opts->verbose = 0;
   opts->objc = 0;
   opts->lang_asm = 0;
   opts->for_lint = 0;
   opts->chill = 0;
   opts->pedantic_errors = 0;
   opts->inhibit_warnings = 0;
   opts->warn_comments = 0;
   opts->warn_import = 1;
   opts->warnings_are_errors = 0;
}

static enum cpp_token
null_underflow(cpp_reader * pfile EINA_UNUSED)
{
   return CPP_EOF;
}

static int
null_cleanup(cpp_buffer * pbuf EINA_UNUSED, cpp_reader * pfile EINA_UNUSED)
{
   return 0;
}

static int
macro_cleanup(cpp_buffer * pbuf, cpp_reader * pfile EINA_UNUSED)
{
   HASHNODE           *macro = (HASHNODE *) pbuf->data;

   if (macro->type == T_DISABLED)
      macro->type = T_MACRO;
   if (macro->type != T_MACRO || pbuf->buf != macro->value.defn->expansion)
      free(pbuf->buf);
   return 0;
}

static int
file_cleanup(cpp_buffer * pbuf, cpp_reader * pfile EINA_UNUSED)
{
   if (pbuf->buf)
     {
	free(pbuf->buf);
	pbuf->buf = 0;
     }
   return 0;
}

/* Assuming we have read '/'.
 * If this is the start of a comment (followed by '*' or '/'),
 * skip to the end of the comment, and return ' '.
 * Return EOF if we reached the end of file before the end of the comment.
 * If not the start of a comment, return '/'. */

static int
skip_comment(cpp_reader * pfile, long *linep)
{
   int                 c = 0;

   while (PEEKC() == '\\' && PEEKN(1) == '\n')
     {
	if (linep)
	   (*linep)++;
	FORWARD(2);
     }
   if (PEEKC() == '*')
     {
	FORWARD(1);
	for (;;)
	  {
	     int                 prev_c = c;

	     c = GETC();
	     if (c == EOF)
		return EOF;
	     while (c == '\\' && PEEKC() == '\n')
	       {
		  if (linep)
		     (*linep)++;
		  FORWARD(1), c = GETC();
	       }
	     if (prev_c == '*' && c == '/')
		return ' ';
	     if (c == '\n' && linep)
		(*linep)++;
	  }
     }
   else if (PEEKC() == '/' && CPP_OPTIONS(pfile)->cplusplus_comments)
     {
	FORWARD(1);
	for (;;)
	  {
	     c = GETC();
	     if (c == EOF)
		return ' ';	/* Allow // to be terminated by EOF. */
	     while (c == '\\' && PEEKC() == '\n')
	       {
		  FORWARD(1);
		  c = GETC();
		  if (linep)
		     (*linep)++;
	       }
	     if (c == '\n')
	       {
		  /* Don't consider final '\n' to be part of comment. */
		  FORWARD(-1);
		  return ' ';
	       }
	  }
     }
   else
      return '/';
}

/* Skip whitespace \-newline and comments.  Does not macro-expand.  */
void
cpp_skip_hspace(cpp_reader * pfile)
{
   while (1)
     {
	int                 c = PEEKC();

	if (c == EOF)
	   return;		/* FIXME */
	if (is_hor_space[c])
	  {
	     if ((c == '\f' || c == '\v') && CPP_PEDANTIC(pfile))
		cpp_pedwarn(pfile, "%s in preprocessing directive",
			    c == '\f' ? "formfeed" : "vertical tab");
	     FORWARD(1);
	  }
	else if (c == '/')
	  {
	     FORWARD(1);
	     c = skip_comment(pfile, NULL);
	     if (c == '/')
		FORWARD(-1);
	     if (c == EOF || c == '/')
		return;
	  }
	else if (c == '\\' && PEEKN(1) == '\n')
	  {
	     FORWARD(2);
	  }
	else if (c == '@' && CPP_BUFFER(pfile)->has_escapes
		 && (PEEKN(1) != EOF) && (PEEKN(1) >= 0)
		 && (PEEKN(1) < 256) && is_hor_space[PEEKN(1)])
          {
             FORWARD(1);
          }
	else
	   return;
     }
}

/* Read the rest of the current line.
 * The line is appended to PFILE's output buffer. */

static void
copy_rest_of_line(cpp_reader * pfile)
{
   struct cpp_options *opts = CPP_OPTIONS(pfile);

   for (;;)
     {
	int                 c = GETC();
	int                 nextc;

	switch (c)
	  {
	  case EOF:
	     goto end_directive;
	  case '\\':
	     if (PEEKC() == '\n')
	       {
		  FORWARD(1);
		  continue;
	       }
	  case '\'':
	  case '\"':
	     goto scan_directive_token;
	     break;
	  case '/':
	     nextc = PEEKC();
	     if (nextc == '*' || opts->cplusplus_comments)
		goto scan_directive_token;
	     break;
	  case '\f':
	  case '\v':
	     if (CPP_PEDANTIC(pfile))
		cpp_pedwarn(pfile, "%s in preprocessing directive",
			    c == '\f' ? "formfeed" : "vertical tab");
	     break;

	  case '\n':
	     FORWARD(-1);
	     goto end_directive;
	   scan_directive_token:
	     FORWARD(-1);
	     cpp_get_token(pfile);
	     continue;
	  }
	CPP_PUTC(pfile, c);
     }
 end_directive:;
   CPP_NUL_TERMINATE(pfile);
}

void
skip_rest_of_line(cpp_reader * pfile)
{
   long                old = CPP_WRITTEN(pfile);

   copy_rest_of_line(pfile);
   CPP_SET_WRITTEN(pfile, old);
}

/* Handle a possible # directive.
 * '#' has already been read.  */

static int
handle_directive(cpp_reader * pfile)
{
   int                 c;
   struct directive   *kt;
   int                 ident_length;
   long                after_ident = 0;
   unsigned char      *ident, *line_end;
   long                old_written = CPP_WRITTEN(pfile);

   cpp_skip_hspace(pfile);

   c = PEEKC();
   if (c >= '0' && c <= '9')
     {
	/* Handle # followed by a line number.  */
	if (CPP_PEDANTIC(pfile))
	   cpp_pedwarn(pfile, "`#' followed by integer");
	do_line(pfile, NULL, NULL, NULL);
	goto done_a_directive;
     }
   /* Now find the directive name. */
   CPP_PUTC(pfile, '#');
   parse_name(pfile, GETC());
   ident = pfile->token_buffer + old_written + 1;
   ident_length = CPP_PWRITTEN(pfile) - ident;
   if (ident_length == 0 && PEEKC() == '\n')
     {
	/* A line of just `#' becomes blank.  */
	goto done_a_directive;
     }
   /*
    * Decode the keyword and call the appropriate expansion
    * routine, after moving the input pointer up to the next line.
    */
   for (kt = directive_table;; kt++)
     {
	if (kt->length <= 0)
	   goto not_a_directive;
	if (kt->length == ident_length
	    && !strncmp(kt->name, (const char *)ident, ident_length))
	   break;
     }

   if (!kt->command_reads_line)
     {
	/* Nonzero means do not delete comments within the directive.
	 * #define needs this when -traditional.  */
	int                 comments = 0;
	int                 save_put_out_comments =
	   CPP_OPTIONS(pfile)->put_out_comments;

	CPP_OPTIONS(pfile)->put_out_comments = comments;
	after_ident = CPP_WRITTEN(pfile);
	copy_rest_of_line(pfile);
	CPP_OPTIONS(pfile)->put_out_comments = save_put_out_comments;
     }
   /* For #pragma and #define, we may want to pass through the directive.
    * Other directives may create output, but we don't want the directive
    * itself out, so we pop it now.  For example #include may write a #line
    * command (see comment in do_include), and conditionals may emit
    * #failed ... #endfailed stuff.  But note that popping the buffer
    * means the parameters to kt->func may point after pfile->limit
    * so these parameters are invalid as soon as something gets appended
    * to the token_buffer.  */

   line_end = CPP_PWRITTEN(pfile);
   if (!kt->pass_thru && kt->type != T_DEFINE)
      CPP_SET_WRITTEN(pfile, old_written);

   (*kt->func) (pfile, kt, pfile->token_buffer + after_ident, line_end);
   if (kt->pass_thru
       || (kt->type == T_DEFINE
	   && CPP_OPTIONS(pfile)->dump_macros == dump_definitions))
     {
	/* Just leave the entire #define in the output stack. */
     }
   else if (kt->type == T_DEFINE
	    && CPP_OPTIONS(pfile)->dump_macros == dump_names)
     {
	unsigned char      *p = pfile->token_buffer + old_written + 7;	/* Skip "#define". */

	SKIP_WHITE_SPACE(p);
	while (is_idchar[*p])
	   p++;
	pfile->limit = p;
	CPP_PUTC(pfile, '\n');
     }
   else if (kt->type == T_DEFINE)
      CPP_SET_WRITTEN(pfile, old_written);
 done_a_directive:
   return 1;

 not_a_directive:
   return 0;
}

/* Pass a directive through to the output file.
 * BUF points to the contents of the directive, as a contiguous string.
 * LIMIT points to the first character past the end of the directive.
 * KEYWORD is the keyword-table entry for the directive.  */

static void
pass_thru_directive(char *buf, char *limit, cpp_reader * pfile,
		    struct directive *keyword)
{
   unsigned            keyword_length = keyword->length;

   CPP_RESERVE(pfile, 1 + keyword_length + (limit - buf));
   CPP_PUTC_Q(pfile, '#');
   CPP_PUTS_Q(pfile, keyword->name, keyword_length);
   if (limit != buf && buf[0] != ' ')
      CPP_PUTC_Q(pfile, ' ');
   CPP_PUTS_Q(pfile, buf, limit - buf);
}

/* The arglist structure is built by do_define to tell
 * collect_definition where the argument names begin.  That
 * is, for a define like "#define f(x,y,z) foo+x-bar*y", the arglist
 * would contain pointers to the strings x, y, and z.
 * Collect_definition would then build a DEFINITION node,
 * with reflist nodes pointing to the places x, y, and z had
 * appeared.  So the arglist is just convenience data passed
 * between these two routines.  It is not kept around after
 * the current #define has been processed and entered into the
 * hash table. */

struct arglist {
   struct arglist     *next;
   char               *name;
   int                 length;
   int                 argno;
   char                rest_args;
};

/* Read a replacement list for a macro with parameters.
 * Build the DEFINITION structure.
 * Reads characters of text starting at BUF until END.
 * ARGLIST specifies the formal parameters to look for
 * in the text of the definition; NARGS is the number of args
 * in that list, or -1 for a macro name that wants no argument list.
 * MACRONAME is the macro name itself (so we can avoid recursive expansion)
 * and NAMELEN is its length in characters.
 * 
 * Note that comments, backslash-newlines, and leading white space
 * have already been deleted from the argument.  */

static DEFINITION  *
collect_expansion(cpp_reader * pfile, unsigned char *buf, unsigned char *limit,
		  int nargs, struct arglist *arglist)
{
   DEFINITION         *defn;
   unsigned char      *p, *lastp, *exp_p;
   reflist            *endpat = NULL;

   /* Pointer to first nonspace after last ## seen.  */
   unsigned char      *concat = 0;

   /* Pointer to first nonspace after last single-# seen.  */
   unsigned char      *stringify = 0;
   int                 maxsize;
   int                 expected_delimiter = '\0';

   /* Scan thru the replacement list, ignoring comments and quoted
    * strings, picking up on the macro calls.  It does a linear search
    * thru the arg list on every potential symbol.  Profiling might say
    * that something smarter should happen. */

   if (limit < buf)
      abort();

   /* Find the beginning of the trailing whitespace.  */
   p = buf;
   while (p < limit && is_space[limit[-1]])
      limit--;

   /* Allocate space for the text in the macro definition.
    * Leading and trailing whitespace chars need 2 bytes each.
    * Each other input char may or may not need 1 byte,
    * so this is an upper bound.  The extra 5 are for invented
    * leading and trailing newline-marker and final null.  */
   maxsize = (sizeof(DEFINITION) + (limit - p) + 5);
   /* Occurrences of '@' get doubled, so allocate extra space for them. */
   while (p < limit)
      if (*p++ == '@')
	 maxsize++;
   defn = (DEFINITION *) xcalloc(1, maxsize);

   defn->nargs = nargs;
   exp_p = defn->expansion = (unsigned char *)defn + sizeof(DEFINITION);
   lastp = exp_p;

   p = buf;

   /* Add one initial space escape-marker to prevent accidental
    * token-pasting (often removed by macroexpand). */
   *exp_p++ = '@';
   *exp_p++ = ' ';

   if (limit - p >= 2 && p[0] == '#' && p[1] == '#')
     {
	cpp_error(pfile, "`##' at start of macro definition");
	p += 2;
     }
   /* Process the main body of the definition.  */
   while (p < limit)
     {
	int                 skipped_arg = 0;
	unsigned char       c = *p++;

	*exp_p++ = c;

        switch (c)
          {
           case '\'':
           case '\"':
             if (expected_delimiter != '\0')
               {
                  if (c == expected_delimiter)
                     expected_delimiter = '\0';
               }
             else
                expected_delimiter = c;
             break;
             
           case '\\':
             if (p < limit && expected_delimiter)
               {
                  /* In a string, backslash goes through
                   * and makes next char ordinary.  */
                  *exp_p++ = *p++;
               }
             break;
             
           case '@':
             /* An '@' in a string or character constant stands for itself,
              * and does not need to be escaped. */
             if (!expected_delimiter)
                *exp_p++ = c;
             break;
             
           case '#':
             /* # is ordinary inside a string.  */
             if (expected_delimiter)
                break;
             if (p < limit && *p == '#')
               {
                  /* ##: concatenate preceding and following tokens.  */
                  /* Take out the first #, discard preceding whitespace.  */
                  exp_p--;
                  while (exp_p > lastp && is_hor_space[exp_p[-1]])
                     --exp_p;
                  /* Skip the second #.  */
                  p++;
                  /* Discard following whitespace.  */
                  SKIP_WHITE_SPACE(p);
                  concat = p;
                  if (p == limit)
                     cpp_error(pfile, "`##' at end of macro definition");
               }
             else if (nargs >= 0)
               {
                  /* Single #: stringify following argument ref.
                   * Don't leave the # in the expansion.  */
                  exp_p--;
                  SKIP_WHITE_SPACE(p);
                  if (p == limit || !is_idstart[*p])
                     cpp_error(pfile,
                               "`#' operator is not followed by a macro argument name");
                  else
                     stringify = p;
               }
             break;
          }

	/* Handle the start of a symbol.  */
	if (is_idchar[c] && nargs > 0)
	  {
	     unsigned char      *id_beg = p - 1;
	     int                 id_len;

	     --exp_p;
	     while (p != limit && is_idchar[*p])
		p++;
	     id_len = p - id_beg;

	     if (is_idstart[c])
	       {
		  struct arglist     *arg;

		  for (arg = arglist; arg; arg = arg->next)
		    {
		       reflist            *tpat;

		       if (arg->name[0] == c
			   && arg->length == id_len
			   && strncmp((const char *)arg->name,
				      (const char *)id_beg, id_len) == 0)
			 {
			    if (expected_delimiter
				&& CPP_OPTIONS(pfile)->warn_stringify)
			      {
                                 cpp_warning(pfile,
                                             "macro arg `%.*s' would be stringified with -traditional.",
                                             id_len, arg->name);
			      }
			    /* If ANSI, don't actually substitute inside a string.  */
			    if (expected_delimiter)
			       break;
			    /* make a pat node for this arg and append it to the end of
			     * the pat list */
			    tpat = (reflist *) xmalloc(sizeof(reflist));

			    tpat->next = NULL;
			    tpat->raw_before = concat == id_beg;
			    tpat->raw_after = 0;
			    tpat->rest_args = arg->rest_args;
			    tpat->stringify = (stringify == id_beg);

			    if (!endpat)
			       defn->pattern = tpat;
			    else
			       endpat->next = tpat;
			    endpat = tpat;

			    tpat->argno = arg->argno;
			    tpat->nchars = exp_p - lastp;
			    {
			       unsigned char      *p1 = p;

			       SKIP_WHITE_SPACE(p1);
			       if (p1 + 2 <= limit && p1[0] == '#'
				   && p1[1] == '#')
				  tpat->raw_after = 1;
			    }
			    lastp = exp_p;	/* place to start copying from next time */
			    skipped_arg = 1;
			    break;
			 }
		    }
	       }
	     /* If this was not a macro arg, copy it into the expansion.  */
	     if (!skipped_arg)
	       {
		  unsigned char      *lim1 = p;

		  p = id_beg;
		  while (p != lim1)
		     *exp_p++ = *p++;
		  if (stringify == id_beg)
		     cpp_error(pfile,
			       "`#' operator should be followed by a macro argument name");
	       }
	  }
     }

   if (expected_delimiter == 0)
     {
	/* If ANSI, put in a "@ " marker to prevent token pasting.
	 * But not if "inside a string" (which in ANSI mode
	 * happens only for -D option).  */
	*exp_p++ = '@';
	*exp_p++ = ' ';
     }
   *exp_p = '\0';

   defn->length = exp_p - defn->expansion;

   /* Crash now if we overrun the allocated size.  */
   if (defn->length + 1 > maxsize)
      abort();

   return defn;
}

/*
 * special extension string that can be added to the last macro argument to 
 * allow it to absorb the "rest" of the arguments when expanded.  Ex:
 *              #define wow(a, b...)            process (b, a, b)
 *              { wow (1, 2, 3); }      ->      { process (2, 3, 1, 2, 3); }
 *              { wow (one, two); }     ->      { process (two, one, two); }
 * if this "rest_arg" is used with the concat token '##' and if it is not
 * supplied then the token attached to with ## will not be outputted.  Ex:
 *              #define wow (a, b...)           process (b ## , a, ## b)
 *              { wow (1, 2); }         ->      { process (2, 1, 2); }
 *              { wow (one); }          ->      { process (one); {
 */
static char         rest_extension[] = "...";

#define REST_EXTENSION_LENGTH	(sizeof (rest_extension) - 1)

/* Create a DEFINITION node from a #define directive.  Arguments are 
 * as for do_define. */
static void
create_definition(MACRODEF * mdef, unsigned char *buf, unsigned char *limit,
		  cpp_reader * pfile, int predefinition)
{
   unsigned char      *bp;	/* temp ptr into input buffer */
   unsigned char      *symname;	/* remember where symbol name starts */
   int                 sym_length;	/* and how long it is */
   int                 rest_args = 0;
   long                line, col;
   const char         *file =
      CPP_BUFFER(pfile) ? CPP_BUFFER(pfile)->nominal_fname : "";
   DEFINITION         *defn;
   int                 arglengths = 0;	/* Accumulate lengths of arg names
					 * plus number of args.  */

   cpp_buf_line_and_col(CPP_BUFFER(pfile), &line, &col);

   bp = buf;

   while (is_hor_space[*bp])
      bp++;

   symname = bp;		/* remember where it starts */

   sym_length = check_macro_name(pfile, bp, "macro");
   bp += sym_length;

   /* Lossage will occur if identifiers or control keywords are broken
    * across lines using backslash.  This is not the right place to take
    * care of that. */

   if (*bp == '(')
     {
	struct arglist     *arg_ptrs = NULL;
	int                 argno = 0;

	bp++;			/* skip '(' */
	SKIP_WHITE_SPACE(bp);

	/* Loop over macro argument names.  */
	while (*bp != ')')
	  {
	     struct arglist     *temp;

	     temp = (struct arglist *)alloca(sizeof(struct arglist));

	     temp->name = (char *)bp;
	     temp->next = arg_ptrs;
	     temp->argno = argno++;
	     temp->rest_args = 0;
	     arg_ptrs = temp;

	     if (rest_args)
		cpp_pedwarn(pfile, "another parameter follows `%s'",
			    rest_extension);

	     if (!is_idstart[*bp])
		cpp_pedwarn(pfile, "invalid character in macro parameter name");

	     /* Find the end of the arg name.  */
	     while (is_idchar[*bp])
	       {
		  bp++;
		  /* do we have a "special" rest-args extension here? */
		  if ((unsigned)(limit - bp) > REST_EXTENSION_LENGTH &&
		      strncmp(rest_extension, (const char *)bp,
			      REST_EXTENSION_LENGTH) == 0)
		    {
		       rest_args = 1;
		       temp->rest_args = 1;
		       break;
		    }
	       }
	     temp->length = (char *)bp - temp->name;
	     if (rest_args == 1)
		bp += REST_EXTENSION_LENGTH;
	     arglengths += temp->length + 2;
	     SKIP_WHITE_SPACE(bp);
	     if (temp->length == 0 || (*bp != ',' && *bp != ')'))
	       {
		  cpp_error(pfile,
			    "badly punctuated parameter list in `#define'");
		  goto nope;
	       }
	     if (*bp == ',')
	       {
		  bp++;
		  SKIP_WHITE_SPACE(bp);
	       }
	     if (bp >= limit)
	       {
		  cpp_error(pfile, "unterminated parameter list in `#define'");
		  goto nope;
	       }
	     {
		struct arglist     *otemp;

		for (otemp = temp->next; otemp; otemp = otemp->next)
		   if (temp->length == otemp->length &&
		       strncmp((const char *)temp->name,
			       (const char *)otemp->name, temp->length) == 0)
		     {
			char               *name;

			name = (char *)alloca(temp->length + 1);
			strncpy(name, (const char *)temp->name, temp->length);
			name[temp->length] = '\0';
			cpp_error(pfile,
				  "duplicate argument name `%s' in `#define'",
				  name);
			goto nope;
		     }
	     }
	  }

	++bp;			/* skip paren */
	SKIP_WHITE_SPACE(bp);
	/* now everything from bp before limit is the definition. */
	defn = collect_expansion(pfile, bp, limit, argno, arg_ptrs);
	defn->rest_args = rest_args;

	/* Now set defn->args.argnames to the result of concatenating
	 * the argument names in reverse order
	 * with comma-space between them.  */
	defn->args.argnames = (unsigned char *)xmalloc(arglengths + 1);
	{
	   struct arglist     *temp;
	   int                 i = 0;

	   for (temp = arg_ptrs; temp; temp = temp->next)
	     {
		memcpy(&defn->args.argnames[i], temp->name, temp->length);
		i += temp->length;
		if (temp->next)
		  {
		     defn->args.argnames[i++] = ',';
		     defn->args.argnames[i++] = ' ';
		  }
	     }
	   defn->args.argnames[i] = 0;
	}
     }
   else
     {
	/* Simple expansion or empty definition.  */

	if (bp < limit)
	  {
	     if (is_hor_space[*bp])
	       {
		  bp++;
		  SKIP_WHITE_SPACE(bp);
	       }
	     else
	       {
		  switch (*bp)
		    {
		    case '!':
		    case '"':
		    case '#':
		    case '%':
		    case '&':
		    case '\'':
		    case ')':
		    case '*':
		    case '+':
		    case ',':
		    case '-':
		    case '.':
		    case '/':
		    case ':':
		    case ';':
		    case '<':
		    case '=':
		    case '>':
		    case '?':
		    case '[':
		    case '\\':
		    case ']':
		    case '^':
		    case '{':
		    case '|':
		    case '}':
		    case '~':
		       cpp_warning(pfile,
				   "missing white space after `#define %.*s'",
				   sym_length, symname);
		       break;

		    default:
		       cpp_pedwarn(pfile,
				   "missing white space after `#define %.*s'",
				   sym_length, symname);
		       break;
		    }
	       }
	  }
	/* now everything from bp before limit is the definition. */
	defn = collect_expansion(pfile, bp, limit, -1, NULL);
	defn->args.argnames = (unsigned char *)"";
     }

   defn->line = line;
   defn->file = file;

   /* OP is null if this is a predefinition */
   defn->predefined = predefinition;
   mdef->defn = defn;
   mdef->symnam = (char *)symname;
   mdef->symlen = sym_length;

   return;

 nope:
   mdef->defn = 0;
}

/* Check a purported macro name SYMNAME, and yield its length.
 * USAGE is the kind of name this is intended for.  */

static int
check_macro_name(cpp_reader * pfile, unsigned char *symname, const char *usage)
{
   unsigned char      *p;
   int                 sym_length;

   for (p = symname; is_idchar[*p]; p++)
      ;
   sym_length = p - symname;
   if (sym_length == 0)
     {
	cpp_error(pfile, "invalid %s name", usage);
     }
   else if (!is_idstart[*symname])
     {
	unsigned char      *msg;	/* what pain... */

	msg = (unsigned char *)alloca(sym_length + 1);
	memcpy(msg, symname, sym_length);
	msg[sym_length] = 0;
	cpp_error(pfile, "invalid %s name `%s'", usage, msg);
     }
   else
     {
	if (!strncmp((const char *)symname, "defined", 7) && sym_length == 7)
	   cpp_error(pfile, "invalid %s name `defined'", usage);
     }
   return sym_length;
}

/*
 * return zero if two DEFINITIONs are isomorphic
 */
static int
compare_defs(DEFINITION * d1, DEFINITION * d2)
{
   reflist            *a1, *a2;
   unsigned char      *p1 = d1->expansion;
   unsigned char      *p2 = d2->expansion;
   int                 first = 1;

   if (d1->nargs != d2->nargs)
      return 1;
   if (strcmp((char *)d1->args.argnames, (char *)d2->args.argnames))
      return 1;
   for (a1 = d1->pattern, a2 = d2->pattern; a1 && a2;
	a1 = a1->next, a2 = a2->next)
     {
	if (!
	    ((a1->nchars == a2->nchars
	      && !strncmp((const char *)p1, (const char *)p2, a1->nchars))
	     || !comp_def_part(first, p1, a1->nchars, p2, a2->nchars, 0))
	    || a1->argno != a2->argno || a1->stringify != a2->stringify
	    || a1->raw_before != a2->raw_before
	    || a1->raw_after != a2->raw_after)
	   return 1;
	first = 0;
	p1 += a1->nchars;
	p2 += a2->nchars;
     }
   if (a1 != a2)
      return 1;
   if (comp_def_part(first, p1, d1->length - (p1 - d1->expansion),
		     p2, d2->length - (p2 - d2->expansion), 1))
      return 1;
   return 0;
}

/* Return 1 if two parts of two macro definitions are effectively different.
 * One of the parts starts at BEG1 and has LEN1 chars;
 * the other has LEN2 chars at BEG2.
 * Any sequence of whitespace matches any other sequence of whitespace.
 * FIRST means these parts are the first of a macro definition;
 * so ignore leading whitespace entirely.
 * LAST means these parts are the last of a macro definition;
 * so ignore trailing whitespace entirely.  */

static int
comp_def_part(int first, unsigned char *beg1, int len1,
	      unsigned char *beg2, int len2, int last)
{
   unsigned char      *end1 = beg1 + len1;
   unsigned char      *end2 = beg2 + len2;

   if (first)
     {
	while (beg1 != end1 && is_space[*beg1])
	   beg1++;
	while (beg2 != end2 && is_space[*beg2])
	   beg2++;
     }
   if (last)
     {
	while (beg1 != end1 && is_space[end1[-1]])
	   end1--;
	while (beg2 != end2 && is_space[end2[-1]])
	   end2--;
     }
   while (beg1 != end1 && beg2 != end2)
     {
	if (is_space[*beg1] && is_space[*beg2])
	  {
	     while (beg1 != end1 && is_space[*beg1])
		beg1++;
	     while (beg2 != end2 && is_space[*beg2])
		beg2++;
	  }
	else if (*beg1 == *beg2)
	  {
	     beg1++;
	     beg2++;
	  }
	else
	   break;
     }
   return (beg1 != end1) || (beg2 != end2);
}

/* Process a #define command.
 * BUF points to the contents of the #define command, as a contiguous string.
 * LIMIT points to the first character past the end of the definition.
 * KEYWORD is the keyword-table entry for #define,
 * or NULL for a "predefined" macro.  */

static int
do_define(cpp_reader * pfile, struct directive *keyword,
	  unsigned char *buf, unsigned char *limit)
{
   int                 hashcode;
   MACRODEF            mdef;
   HASHNODE           *hp;

   create_definition(&mdef, buf, limit, pfile, !keyword);
   if (!mdef.defn)
      return 1;

   hashcode = hashf(mdef.symnam, mdef.symlen, HASHSIZE);

   if ((hp = cpp_lookup(mdef.symnam, mdef.symlen, hashcode)))
     {
	int                 ok = 0;

	/* Redefining a precompiled key is ok.  */
	if (hp->type == T_PCSTRING)
	   ok = 1;
	/* Redefining a macro is ok if the definitions are the same.  */
	else if (hp->type == T_MACRO)
	   ok = !compare_defs(mdef.defn, hp->value.defn);
	/* Redefining a constant is ok with -D.  */
	else if (hp->type == T_CONST)
	   ok = !CPP_OPTIONS(pfile)->done_initializing;
	/* Print the warning if it's not ok.  */
	if (!ok)
	  {
	     char               *msg;	/* what pain... */

	     /* If we are passing through #define and #undef directives, do
	      * that for this re-definition now.  */
	     if (CPP_OPTIONS(pfile)->debug_output && keyword)
		pass_thru_directive((char *)buf, (char *)limit, pfile, keyword);

	     msg = (char *)alloca(mdef.symlen + 22);
	     *msg = '`';
	     memcpy(msg + 1, mdef.symnam, mdef.symlen);
	     strcpy((msg + mdef.symlen + 1), "' redefined");
	     cpp_pedwarn(pfile, msg);
	     if (hp->type == T_MACRO)
		cpp_pedwarn_with_file_and_line(pfile, hp->value.defn->file,
					       hp->value.defn->line,
					       "this is the location of the previous definition",
					       NULL, NULL, NULL);
	  }
	/* Replace the old definition.  */
	hp->type = T_MACRO;
	hp->value.defn = mdef.defn;
     }
   else
     {
	/* If we are passing through #define and #undef directives, do
	 * that for this new definition now.  */
	if (CPP_OPTIONS(pfile)->debug_output && keyword)
	   pass_thru_directive((char *)buf, (char *)limit, pfile, keyword);
	install(mdef.symnam, mdef.symlen, T_MACRO, 0,
		(char *)mdef.defn, hashcode);
     }

   return 0;
}

/* This structure represents one parsed argument in a macro call.
 * `raw' points to the argument text as written (`raw_length' is its length).
 * `expanded' points to the argument's macro-expansion
 * (its length is `expand_length').
 * `stringified_length' is the length the argument would have
 * if stringified.
 * `use_count' is the number of times this macro arg is substituted
 * into the macro.  If the actual use count exceeds 10, 
 * the value stored is 10. */

/* raw and expanded are relative to ARG_BASE */
#define ARG_BASE ((pfile)->token_buffer)

struct argdata {
   /* Strings relative to pfile->token_buffer */
   long                raw, expanded, stringified;
   int                 raw_length, expand_length;
   int                 stringified_length;
   char                newlines;
   char                use_count;
};

cpp_buffer         *
cpp_push_buffer(cpp_reader * pfile, unsigned char *buffer, long length)
{
#ifdef STATIC_BUFFERS
   cpp_buffer         *buf = CPP_BUFFER(pfile);

   if (buf == pfile->buffer_stack)
      cpp_fatal("macro or `#include' recursion too deep");
   buf--;
   memset((char *)buf, 0, sizeof(cpp_buffer));
   CPP_BUFFER(pfile) = buf;
#else
   cpp_buffer         *buf = (cpp_buffer *) xmalloc(sizeof(cpp_buffer));

   if (!buf) return NULL;
   memset((char *)buf, 0, sizeof(cpp_buffer));
   CPP_PREV_BUFFER(buf) = CPP_BUFFER(pfile);
   CPP_BUFFER(pfile) = buf;
#endif
   buf->if_stack = pfile->if_stack;
   buf->cleanup = null_cleanup;
   buf->underflow = null_underflow;
   buf->buf = buf->cur = buffer;
   buf->alimit = buf->rlimit = buffer + length;

   return buf;
}

static cpp_buffer  *
cpp_pop_buffer(cpp_reader * pfile)
{
   cpp_buffer         *buf = CPP_BUFFER(pfile);

#ifdef STATIC_BUFFERS
   (*buf->cleanup) (buf, pfile);
   return ++CPP_BUFFER(pfile);
#else
   cpp_buffer         *next_buf = CPP_PREV_BUFFER(buf);

   (*buf->cleanup) (buf, pfile);
   CPP_BUFFER(pfile) = next_buf;
   free(buf);
   return next_buf;
#endif
}

/* Scan until CPP_BUFFER (PFILE) is exhausted into PFILE->token_buffer.
 * Pop the buffer when done. */

static void
cpp_scan_buffer(cpp_reader * pfile)
{
   cpp_buffer         *buffer = CPP_BUFFER(pfile);

   for (;;)
     {
	enum cpp_token      token = cpp_get_token(pfile);

	if (token == CPP_EOF)	/* Should not happen ... */
	   break;
	if (token == CPP_POP && CPP_BUFFER(pfile) == buffer)
	  {
	     cpp_pop_buffer(pfile);
	     break;
	  }
     }
}

/*
 * Rescan a string (which may have escape marks) into pfile's buffer.
 * Place the result in pfile->token_buffer.
 *
 * The input is copied before it is scanned, so it is safe to pass
 * it something from the token_buffer that will get overwritten
 * (because it follows CPP_WRITTEN).  This is used by do_include.
 */

static void
cpp_expand_to_buffer(cpp_reader * pfile, unsigned char *buf, int length)
{
   cpp_buffer         *ip;
   unsigned char      *limit = buf + length;
   unsigned char      *buf1;

   if (length < 0)
      abort();

   /* Set up the input on the input stack.  */

   buf1 = (unsigned char *)alloca(length + 1);
   {
      unsigned char      *p1 = buf;
      unsigned char      *p2 = buf1;
      int                 in_string = 0;
      
#if 0 /* old behavior */ 
      while (p1 != limit) *p2++ = *p1++;
#else /* new one - handle \ escapes if not in string */
      while (p1 != limit)
        {
           if (!in_string)
             {
                if (*p1 == '"') in_string = 1;
                if (*p1 == '\\')
                  {
                     p1++;
                     if (p1 != limit) *p2++ = *p1++;
                  }
                else
                   *p2++ = *p1++;
             }
           else
             {
                if ((*p1 == '"') && (p1 > buf) && (p1[-1] != '\\'))
                   in_string = 0;
                *p2++ = *p1++;
             }
        }
#endif
      *p2 = 0;
      length = p2 - buf1;
   }

   ip = cpp_push_buffer(pfile, buf1, length);
   ip->has_escapes = 1;

   /* Scan the input, create the output.  */
   cpp_scan_buffer(pfile);

   CPP_NUL_TERMINATE(pfile);
}

static void
adjust_position(unsigned char *buf, unsigned char *limit, long *linep,
		long *colp)
{
   while (buf < limit)
     {
	unsigned char       ch = *buf++;

	if (ch == '\n')
	   (*linep)++, (*colp) = 1;
	else
	   (*colp)++;
     }
}

/* Move line_base forward, updating lineno and colno. */

static void
update_position(cpp_buffer * pbuf)
{
   unsigned char      *old_pos = pbuf->buf + pbuf->line_base;
   unsigned char      *new_pos = pbuf->cur;
   struct parse_marker *mark;

   for (mark = pbuf->marks; mark; mark = mark->next)
     {
	if (pbuf->buf + mark->position < new_pos)
	   new_pos = pbuf->buf + mark->position;
     }
   pbuf->line_base += new_pos - old_pos;
   adjust_position(old_pos, new_pos, &pbuf->lineno, &pbuf->colno);
}

void
cpp_buf_line_and_col(cpp_buffer * pbuf, long *linep, long *colp)
{
   long                dummy;

   if (!colp)
      colp = &dummy;
   if (pbuf)
     {
	*linep = pbuf->lineno;
	*colp = pbuf->colno;
	adjust_position(pbuf->buf + pbuf->line_base, pbuf->cur, linep, colp);
     }
   else
     {
	*linep = 0;
	*colp = 0;
     }
}

/* Return the cpp_buffer that corresponds to a file (not a macro). */

cpp_buffer         *
cpp_file_buffer(cpp_reader * pfile)
{
   cpp_buffer         *ip = CPP_BUFFER(pfile);

   for (; ip != CPP_NULL_BUFFER(pfile); ip = CPP_PREV_BUFFER(ip))
      if (ip->fname)
	 return ip;
   return NULL;
}

static long
count_newlines(unsigned char *buf, unsigned char *limit)
{
   long                count = 0;

   while (buf < limit)
     {
	unsigned char       ch = *buf++;

	if (ch == '\n')
	   count++;
     }
   return count;
}

/*
 * write out a #line command, for instance, after an #include file.
 * If CONDITIONAL is nonzero, we can omit the #line if it would
 * appear to be a no-op, and we can output a few newlines instead
 * if we want to increase the line number by a small amount.
 * FILE_CHANGE says whether we are entering a file, leaving, or neither.
 */

static void
output_line_command(cpp_reader * pfile, int conditional,
		    enum file_change_code file_change)
{
   long                line, col;
   cpp_buffer         *ip = CPP_BUFFER(pfile);

   if (CPP_OPTIONS(pfile)->no_line_commands
       || !ip->fname || CPP_OPTIONS(pfile)->no_output)
     {
	return;
     }
   update_position(ip);
   line = CPP_BUFFER(pfile)->lineno;
   col = CPP_BUFFER(pfile)->colno;
   adjust_position(CPP_LINE_BASE(ip), ip->cur, &line, &col);

   if (conditional)
     {
	if (line == pfile->lineno)
	   return;

	/* If the inherited line number is a little too small,
	 * output some newlines instead of a #line command.  */
	if (line > pfile->lineno && line < pfile->lineno + 8)
	  {
	     CPP_RESERVE(pfile, 20);
	     while (line > pfile->lineno)
	       {
		  CPP_PUTC_Q(pfile, '\n');
		  pfile->lineno++;
	       }
	     return;
	  }
     }

   CPP_RESERVE(pfile, 4 * strlen(ip->nominal_fname) + 50);
   {
      static char         sharp_line[] = "#line ";

      CPP_PUTS_Q(pfile, sharp_line, sizeof(sharp_line) - 1);
   }

   sprintf((char *)CPP_PWRITTEN(pfile), "%d ", (int)line);
   CPP_ADJUST_WRITTEN(pfile, strlen((char *)CPP_PWRITTEN(pfile)));

   quote_string(pfile, ip->nominal_fname);
   if (file_change != same_file)
     {
	CPP_PUTC_Q(pfile, ' ');
	CPP_PUTC_Q(pfile, file_change == enter_file ? '1' : '2');
     }
   /* Tell cc1 if following text comes from a system header file.  */
   if (ip->system_header_p)
     {
	CPP_PUTC_Q(pfile, ' ');
	CPP_PUTC_Q(pfile, '3');
     }
#ifndef NO_IMPLICIT_EXTERN_C
   /* Tell cc1plus if following text should be treated as C.  */
   if (ip->system_header_p == 2 && CPP_OPTIONS(pfile)->cplusplus)
     {
	CPP_PUTC_Q(pfile, ' ');
	CPP_PUTC_Q(pfile, '4');
     }
#endif
   CPP_PUTC_Q(pfile, '\n');
   pfile->lineno = line;
}

/*
 * Parse a macro argument and append the info on PFILE's token_buffer.
 * REST_ARGS means to absorb the rest of the args.
 * Return nonzero to indicate a syntax error.
 */

static enum cpp_token
macarg(cpp_reader * pfile, int rest_args)
{
   int                 paren = 0;
   enum cpp_token      token;
   char                save_put_out_comments =
      CPP_OPTIONS(pfile)->put_out_comments;

   CPP_OPTIONS(pfile)->put_out_comments = 0;

   token = CPP_OTHER;
   /* Try to parse as much of the argument as exists at this
    * input stack level.  */
   pfile->no_macro_expand++;
   for (;;)
     {
	token = cpp_get_token(pfile);
	switch (token)
	  {
	  case CPP_EOF:
	     goto done;
	  case CPP_POP:
	     /* If we've hit end of file, it's an error (reported by caller).
	      * Ditto if it's the end of cpp_expand_to_buffer text.
	      * If we've hit end of macro, just continue.  */
	     if (!CPP_IS_MACRO_BUFFER(CPP_BUFFER(pfile)))
		goto done;
	     break;
	  case CPP_LPAREN:
	     paren++;
	     break;
	  case CPP_RPAREN:
	     if (--paren < 0)
		goto found;
	     break;
	  case CPP_COMMA:
	     /* if we've returned to lowest level and
	      * we aren't absorbing all args */
	     if (paren == 0 && rest_args == 0)
		goto found;
	     break;
	   found:
	     /* Remove ',' or ')' from argument buffer. */
	     CPP_ADJUST_WRITTEN(pfile, -1);
	     goto done;
	  default:;
	  }
     }

 done:
   CPP_OPTIONS(pfile)->put_out_comments = save_put_out_comments;
   pfile->no_macro_expand--;

   return token;
}

/* Turn newlines to spaces in the string of length LENGTH at START,
 * except inside of string constants.
 * The string is copied into itself with its beginning staying fixed.  */

static int
change_newlines(unsigned char *start, int length)
{
   unsigned char      *ibp;
   unsigned char      *obp;
   unsigned char      *limit;
   int                 c;

   ibp = start;
   limit = start + length;
   obp = start;

   while (ibp < limit)
     {
	*obp++ = c = *ibp++;
	switch (c)
	  {

	  case '\'':
	  case '\"':
	     /* Notice and skip strings, so that we don't delete newlines in them.  */
	     {
		int                 quotec = c;

		while (ibp < limit)
		  {
		     *obp++ = c = *ibp++;
		     if (c == quotec)
			break;
		     if (c == '\n' && quotec == '\'')
			break;
		  }
	     }
	     break;
	  }
     }

   return obp - start;
}

static struct tm   *
timestamp(cpp_reader * pfile)
{
   if (!pfile->timebuf)
     {
	time_t              t = time((time_t *) 0);

	pfile->timebuf = localtime(&t);
     }
   return pfile->timebuf;
}

static const char  *monthnames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

/*
 * expand things like __FILE__.  Place the expansion into the output
 * buffer *without* rescanning.
 */

static void
special_symbol(HASHNODE * hp, cpp_reader * pfile)
{
   const char         *buf;
   char               *bufx;
   int                 len;
   int                 true_indepth;
   cpp_buffer         *ip = NULL;
   struct tm          *timebuf;

   int                 paren = 0;	/* For special `defined' keyword */

   for (ip = CPP_BUFFER(pfile);; ip = CPP_PREV_BUFFER(ip))
     {
	if (!ip)
	  {
	     cpp_error(pfile, "cccp error: not in any file?!");
	     return;		/* the show must go on */
	  }
	if (ip->fname)
	   break;
     }

   switch (hp->type)
     {
     case T_FILE:
     case T_BASE_FILE:
	{
	   const char         *string;

	   if (hp->type == T_BASE_FILE)
	     {
		while (CPP_PREV_BUFFER(ip) != CPP_NULL_BUFFER(pfile))
		   ip = CPP_PREV_BUFFER(ip);
	     }
	   string = ip->nominal_fname;

	   if (!string)
	      string = "";
	   CPP_RESERVE(pfile, 3 + 4 * strlen(string));
	   quote_string(pfile, string);
	   return;
	}

     case T_INCLUDE_LEVEL:
	true_indepth = 0;
	for (ip = CPP_BUFFER(pfile); ip != CPP_NULL_BUFFER(pfile); ip = CPP_PREV_BUFFER(ip))
	   if (ip->fname)
	      true_indepth++;

	bufx = (char *)alloca(12);	// 12 bytes - more than enough
	sprintf(bufx, "%d", true_indepth - 1);
	buf = bufx;
	break;

     case T_VERSION:
	bufx = (char *)alloca(3 + strlen(version_string));
	sprintf(bufx, "\"%s\"", version_string);
	buf = bufx;
	break;

#ifndef NO_BUILTIN_SIZE_TYPE
     case T_SIZE_TYPE:
	buf = SIZE_TYPE;
	break;
#endif

#ifndef NO_BUILTIN_PTRDIFF_TYPE
     case T_PTRDIFF_TYPE:
	buf = PTRDIFF_TYPE;
	break;
#endif

     case T_WCHAR_TYPE:
	buf = CPP_WCHAR_TYPE(pfile);
	break;

     case T_USER_LABEL_PREFIX_TYPE:
	buf = USER_LABEL_PREFIX;
	break;

     case T_REGISTER_PREFIX_TYPE:
	buf = REGISTER_PREFIX;
	break;

     case T_CONST:
	bufx = (char *)alloca(4 * sizeof(int));
	sprintf(bufx, "%d", hp->value.ival);
	buf = bufx;
	break;

     case T_SPECLINE:
	{
	   long                line = ip->lineno;
	   long                col = ip->colno;

	   adjust_position(CPP_LINE_BASE(ip), ip->cur, &line, &col);

	   bufx = (char *)alloca(10);
	   sprintf(bufx, "%d", (int)line);
	   buf = bufx;
	}
	break;

     case T_DATE:
     case T_TIME:
	bufx = (char *)alloca(20);
	timebuf = timestamp(pfile);
	if (hp->type == T_DATE)
	   sprintf(bufx, "\"%s %2d %4d\"", monthnames[timebuf->tm_mon],
		   timebuf->tm_mday, timebuf->tm_year + 1900);
	else
	   sprintf(bufx, "\"%02d:%02d:%02d\"", timebuf->tm_hour,
		   timebuf->tm_min, timebuf->tm_sec);
	buf = bufx;
	break;

     case T_SPEC_DEFINED:
	buf = " 0 ";		/* Assume symbol is not defined */
	ip = CPP_BUFFER(pfile);
	SKIP_WHITE_SPACE(ip->cur);
	if (*ip->cur == '(')
	  {
	     paren++;
	     ip->cur++;		/* Skip over the paren */
	     SKIP_WHITE_SPACE(ip->cur);
	  }
	if (!is_idstart[*ip->cur])
	   goto oops;
	if (cpp_lookup((const char *)ip->cur, -1, -1))
	  {
	     buf = " 1 ";
	  }
	while (is_idchar[*ip->cur])
	   ++ip->cur;
	SKIP_WHITE_SPACE(ip->cur);
	if (paren)
	  {
	     if (*ip->cur != ')')
		goto oops;
	     ++ip->cur;
	  }
	break;

      oops:

	cpp_error(pfile, "`defined' without an identifier");
	break;

     default:
	cpp_error(pfile, "cccp error: invalid special hash type");	/* time for gdb */
	abort();
     }
   len = strlen(buf);
   CPP_RESERVE(pfile, len + 1);
   CPP_PUTS_Q(pfile, buf, len);
   CPP_NUL_TERMINATE_Q(pfile);

   return;
}

/* Initialize the built-in macros.  */

static void
initialize_builtins(cpp_reader * pfile)
{
   install("__LINE__", -1, T_SPECLINE, 0, 0, -1);
   install("__DATE__", -1, T_DATE, 0, 0, -1);
   install("__FILE__", -1, T_FILE, 0, 0, -1);
   install("__BASE_FILE__", -1, T_BASE_FILE, 0, 0, -1);
   install("__INCLUDE_LEVEL__", -1, T_INCLUDE_LEVEL, 0, 0, -1);
   install("__VERSION__", -1, T_VERSION, 0, 0, -1);
#ifndef NO_BUILTIN_SIZE_TYPE
   install("__SIZE_TYPE__", -1, T_SIZE_TYPE, 0, 0, -1);
#endif
#ifndef NO_BUILTIN_PTRDIFF_TYPE
   install("__PTRDIFF_TYPE__ ", -1, T_PTRDIFF_TYPE, 0, 0, -1);
#endif
   install("__WCHAR_TYPE__", -1, T_WCHAR_TYPE, 0, 0, -1);
   install("__USER_LABEL_PREFIX__", -1, T_USER_LABEL_PREFIX_TYPE, 0, 0, -1);
   install("__REGISTER_PREFIX__", -1, T_REGISTER_PREFIX_TYPE, 0, 0, -1);
   install("__TIME__", -1, T_TIME, 0, 0, -1);
   install("__STDC__", -1, T_CONST, STDC_VALUE, 0, -1);
   if (CPP_OPTIONS(pfile)->objc)
      install("__OBJC__", -1, T_CONST, 1, 0, -1);
/*  This is supplied using a -D by the compiler driver
 * so that it is present only when truly compiling with GNU C.  */
/*  install ("__GNUC__", -1, T_CONST, 2, 0, -1);  */

   if (CPP_OPTIONS(pfile)->debug_output)
     {
	char                directive[2048];
	struct directive   *dp = &directive_table[0];
	struct tm          *timebuf = timestamp(pfile);
	cpp_buffer         *pbuffer = CPP_BUFFER(pfile);

	while (CPP_PREV_BUFFER(pbuffer) != CPP_NULL_BUFFER(pfile))
	   pbuffer = CPP_PREV_BUFFER(pbuffer);
	sprintf(directive, " __BASE_FILE__ \"%s\"\n", pbuffer->nominal_fname);
	output_line_command(pfile, 0, same_file);
	pass_thru_directive(directive, &directive[strlen(directive)], pfile,
			    dp);

	sprintf(directive, " __VERSION__ \"%s\"\n", version_string);
	output_line_command(pfile, 0, same_file);
	pass_thru_directive(directive, &directive[strlen(directive)], pfile,
			    dp);

#ifndef NO_BUILTIN_SIZE_TYPE
	sprintf(directive, " __SIZE_TYPE__ %s\n", SIZE_TYPE);
	output_line_command(pfile, 0, same_file);
	pass_thru_directive(directive, &directive[strlen(directive)], pfile,
			    dp);
#endif

#ifndef NO_BUILTIN_PTRDIFF_TYPE
	sprintf(directive, " __PTRDIFF_TYPE__ %s\n", PTRDIFF_TYPE);
	output_line_command(pfile, 0, same_file);
	pass_thru_directive(directive, &directive[strlen(directive)], pfile,
			    dp);
#endif

	sprintf(directive, " __WCHAR_TYPE__ %s\n", CPP_WCHAR_TYPE(pfile));
	output_line_command(pfile, 0, same_file);
	pass_thru_directive(directive, &directive[strlen(directive)], pfile,
			    dp);

	sprintf(directive, " __DATE__ \"%s %2d %4d\"\n",
		monthnames[timebuf->tm_mon],
		timebuf->tm_mday, timebuf->tm_year + 1900);
	output_line_command(pfile, 0, same_file);
	pass_thru_directive(directive, &directive[strlen(directive)], pfile,
			    dp);

	sprintf(directive, " __TIME__ \"%02d:%02d:%02d\"\n",
		timebuf->tm_hour, timebuf->tm_min, timebuf->tm_sec);
	output_line_command(pfile, 0, same_file);
	pass_thru_directive(directive, &directive[strlen(directive)], pfile,
			    dp);

        sprintf(directive, " __STDC__ 1");
        output_line_command(pfile, 0, same_file);
        pass_thru_directive(directive, &directive[strlen(directive)],
                            pfile, dp);
     }
}

/* Return 1 iff a token ending in C1 followed directly by a token C2
 * could cause mis-tokenization. */

static int
unsafe_chars(int c1, int c2)
{
//   printf("unsafe %c %c ...", c1, c2);
   switch (c1)
     {
     case '+':
     case '-':
     case '.':
//        printf(" no0\n");
        return 0;
     case '0':
     case '1':
     case '2':
     case '3':
     case '4':
     case '5':
     case '6':
     case '7':
     case '8':
     case '9':
     case 'e':
     case 'E':
	if (c2 == '-' || c2 == '+')
          {
//             printf(" yes2\n");
             return 1;		/* could extend a pre-processing number */
          }
	goto letter;
     case 'L':
	if (c2 == '\'' || c2 == '\"')
          {
//             printf(" yes3\n");
             return 1;		/* Could turn into L"xxx" or L'xxx'. */
          }
	goto letter;
      letter:
     case '_':
     case 'a':
     case 'b':
     case 'c':
     case 'd':
     case 'f':
     case 'g':
     case 'h':
     case 'i':
     case 'j':
     case 'k':
     case 'l':
     case 'm':
     case 'n':
     case 'o':
     case 'p':
     case 'q':
     case 'r':
     case 's':
     case 't':
     case 'u':
     case 'v':
     case 'w':
     case 'x':
     case 'y':
     case 'z':
     case 'A':
     case 'B':
     case 'C':
     case 'D':
     case 'F':
     case 'G':
     case 'H':
     case 'I':
     case 'J':
     case 'K':
     case 'M':
     case 'N':
     case 'O':
     case 'P':
     case 'Q':
     case 'R':
     case 'S':
     case 'T':
     case 'U':
     case 'V':
     case 'W':
     case 'X':
     case 'Y':
     case 'Z':
	/* We're in the middle of either a name or a pre-processing number. */
	if (is_idchar[c2] || c2 == '.')
          {
//             printf(" yes4 %i %i\n", is_idchar[c2], c2 == '.');
             return 1;
          }
        else
          {
//             printf(" no5\n");
             return 0;
          }
     case '<':
     case '>':
     case '!':
     case '%':
     case '#':
     case ':':
     case '^':
     case '&':
     case '|':
     case '*':
     case '/':
     case '=':
	if (c2 == c1 || c2 == '=')
          {
//             printf(" yes6\n");
             return 1;
          }
        else
          {
//             printf(" no7\n");
             return 0;
          }
     }
//   printf(" no8\n");
   return 0;
}

/* Expand a macro call.
 * HP points to the symbol that is the macro being called.
 * Put the result of expansion onto the input stack
 * so that subsequent input by our caller will use it.
 * 
 * If macro wants arguments, caller has already verified that
 * an argument list follows; arguments come from the input stack.  */

static void
macroexpand(cpp_reader * pfile, HASHNODE * hp)
{
   int                 nargs;
   DEFINITION         *defn = hp->value.defn;
   unsigned char      *xbuf;
   long                start_line, start_column;
   int                 xbuf_len;
   struct argdata     *args;
   long                old_written = CPP_WRITTEN(pfile);

   int                 rest_args, rest_zero;
   int                 i;

   pfile->output_escapes++;
   cpp_buf_line_and_col(cpp_file_buffer(pfile), &start_line, &start_column);

   rest_zero = 0;
   args = NULL;
   nargs = defn->nargs;

   if (nargs >= 0)
     {
	enum cpp_token      token;

	token = CPP_OTHER;

	args = (struct argdata *)alloca((nargs + 1) * sizeof(struct argdata));

	for (i = 0; i < nargs; i++)
	  {
	     args[i].raw = args[i].expanded = 0;
	     args[i].raw_length = 0;
	     args[i].expand_length = args[i].stringified_length = -1;
	     args[i].use_count = 0;
	  }

	/* Parse all the macro args that are supplied.  I counts them.
	 * The first NARGS args are stored in ARGS.
	 * The rest are discarded.  If rest_args is set then we assume
	 * macarg absorbed the rest of the args. */
	i = 0;
	rest_args = 0;
	FORWARD(1);		/* Discard the open-parenthesis before the first arg.  */
	do
	  {
	     if (rest_args)
		continue;
	     if (i < nargs || (nargs == 0 && i == 0))
	       {
                  unsigned char *bp;
                  
		  /* if we are working on last arg which absorbs rest of args... */
		  if (i == nargs - 1 && defn->rest_args)
		     rest_args = 1;
		  args[i].raw = CPP_WRITTEN(pfile);
		  token = macarg(pfile, rest_args);
		  args[i].raw_length = CPP_WRITTEN(pfile) - args[i].raw;
		  args[i].newlines = 0;	/* FIXME */
                  bp = ARG_BASE + args[i].raw;
                  while (is_space[(unsigned char)(*bp)]) { bp++; }
                  args[i].raw_length -= bp - (ARG_BASE + args[i].raw);
                  args[i].raw = bp - ARG_BASE;
                  if (args[i].raw_length > 0)
                    {
                       bp = ARG_BASE + args[i].raw + args[i].raw_length - 1;
                       while (is_space[(unsigned char)(*bp)])
                         {
                            bp--; 
                            args[i].raw_length--;
                            if (args[i].raw_length < 1) break;
                         }
                    }
	       }
	     else
		token = macarg(pfile, 0);
	     if (token == CPP_EOF || token == CPP_POP)
	       {
		  cpp_error_with_line(pfile, start_line, start_column,
				      "unterminated macro call");
		  return;
	       }
	     i++;
	  }
	while (token == CPP_COMMA);

	/* If we got one arg but it was just whitespace, call that 0 args.  */
	if (i == 1)
	  {
	     unsigned char      *bp = ARG_BASE + args[0].raw;
	     unsigned char      *lim = bp + args[0].raw_length;

	     /* cpp.texi says for foo ( ) we provide one argument.
	      * However, if foo wants just 0 arguments, treat this as 0.  */
	     if (nargs == 0)
		while (bp != lim && is_space[*bp])
		   bp++;
	     if (bp == lim)
		i = 0;
	  }
	/* Don't output an error message if we have already output one for
	 * a parse error above.  */
	if (nargs == 0 && i > 0)
	  {
	     cpp_error(pfile, "arguments given to macro `%s'", hp->name);
	  }
	else if (i < nargs)
	  {
	     if (i == nargs - 1 && defn->rest_args)
		rest_zero = 1;
	     else if (i == 0)
		cpp_error(pfile, "macro `%s' used without args", hp->name);
	     else if (i == 1)
		cpp_error(pfile, "macro `%s' used with just one arg", hp->name);
	     else
		cpp_error(pfile, "macro `%s' used with only %d args",
			  hp->name, i);
	  }
	else if (i > nargs)
	  {
	     cpp_error(pfile,
		       "macro `%s' used with too many (%d) args", hp->name, i);
	  }
     }
   /* If macro wants zero args, we parsed the arglist for checking only.
    * Read directly from the macro definition.  */
   if (nargs <= 0)
     {
	xbuf = defn->expansion;
	xbuf_len = defn->length;
     }
   else
     {
	unsigned char      *exp = defn->expansion;
	int                 offset;	/* offset in expansion,
					 * copied a piece at a time */
	int                 totlen;	/* total amount of exp buffer filled so far */
	reflist            *ap, *last_ap;

	/* Macro really takes args.  Compute the expansion of this call.  */

	/* Compute length in characters of the macro's expansion.
	 * Also count number of times each arg is used.  */
	xbuf_len = defn->length;
	for (ap = defn->pattern; ap; ap = ap->next)
	  {
	     if (ap->stringify)
	       {
		  struct argdata     *arg = &args[ap->argno];

		  /* Stringify it it hasn't already been */
		  if (arg->stringified_length < 0)
		    {
		       int                 arglen = arg->raw_length;
		       int                 escaped = 0;
		       int                 in_string = 0;
		       int                 c;

		       /* Initially need_space is -1.  Otherwise, 1 means the
		        * previous character was a space, but we suppressed it;
		        * 0 means the previous character was a non-space. */
		       int                 need_space = -1;

		       i = 0;
		       arg->stringified = CPP_WRITTEN(pfile);
                       CPP_PUTC(pfile, '\"');	/* insert beginning quote */
		       for (; i < arglen; i++)
			 {
			    c = (ARG_BASE + arg->raw)[i];

			    if (!in_string)
			      {
				 /* Internal sequences of whitespace are replaced by
				  * one space except within an string or char token. */
				 if (is_space[c])
				   {
				      if (CPP_WRITTEN(pfile) > arg->stringified
					  && (CPP_PWRITTEN(pfile))[-1] == '@')
					{
					   /* "@ " escape markers are removed */
					   CPP_ADJUST_WRITTEN(pfile, -1);
					   continue;
					}
				      if (need_space == 0)
					 need_space = 1;
				      continue;
				   }
				 else if (need_space > 0)
				    CPP_PUTC(pfile, ' ');
				 need_space = 0;
			      }
			    if (escaped)
			       escaped = 0;
			    else
			      {
				 if (c == '\\')
				    escaped = 1;
				 if (in_string)
				   {
				      if (c == in_string)
					 in_string = 0;
				   }
				 else if (c == '\"' || c == '\'')
				    in_string = c;
			      }

			    /* Escape these chars */
			    if (c == '\"' || (in_string && c == '\\'))
			       CPP_PUTC(pfile, '\\');
			    if (isprint(c))
			       CPP_PUTC(pfile, c);
			    else
			      {
				 CPP_RESERVE(pfile, 4);
				 sprintf((char *)CPP_PWRITTEN(pfile), "\\%03o",
					 (unsigned int)c);
				 CPP_ADJUST_WRITTEN(pfile, 4);
			      }
			 }
                       CPP_PUTC(pfile, '\"');	/* insert ending quote */
		       arg->stringified_length
			  = CPP_WRITTEN(pfile) - arg->stringified;
		    }
		  xbuf_len += args[ap->argno].stringified_length;
	       }
	     else if (ap->raw_before || ap->raw_after)
               {
                  /* Add 4 for two newline-space markers to prevent
                   * token concatenation.  */
                  xbuf_len += args[ap->argno].raw_length + 4;
               }
	     else
	       {
		  /* We have an ordinary (expanded) occurrence of the arg.
		   * So compute its expansion, if we have not already.  */
		  if (args[ap->argno].expand_length < 0)
		    {
		       args[ap->argno].expanded = CPP_WRITTEN(pfile);
		       cpp_expand_to_buffer(pfile,
					    ARG_BASE + args[ap->argno].raw,
					    args[ap->argno].raw_length);

		       args[ap->argno].expand_length
			  = CPP_WRITTEN(pfile) - args[ap->argno].expanded;
		    }
		  /* Add 4 for two newline-space markers to prevent
		   * token concatenation.  */
		  xbuf_len += args[ap->argno].expand_length + 4;
	       }
	     if (args[ap->argno].use_count < 10)
		args[ap->argno].use_count++;
	  }

	xbuf = (unsigned char *)xmalloc(xbuf_len + 1);

	/* Generate in XBUF the complete expansion
	 * with arguments substituted in.
	 * TOTLEN is the total size generated so far.
	 * OFFSET is the index in the definition
	 * of where we are copying from.  */
	offset = totlen = 0;
	for (last_ap = NULL, ap = defn->pattern; ap;
	     last_ap = ap, ap = ap->next)
	  {
	     struct argdata     *arg = &args[ap->argno];
	     int                 count_before = totlen;

	     /* Add chars to XBUF.  */
	     for (i = 0; i < ap->nchars; i++, offset++)
		xbuf[totlen++] = exp[offset];

	     /* If followed by an empty rest arg with concatenation,
	      * delete the last run of nonwhite chars.  */
	     if (rest_zero && totlen > count_before
		 && ((ap->rest_args && ap->raw_before)
		     || (last_ap && last_ap->rest_args && last_ap->raw_after)))
	       {
		  /* Delete final whitespace.  */
		  while (totlen > count_before && is_space[xbuf[totlen - 1]])
		     totlen--;

		  /* Delete the nonwhites before them.  */
		  while (totlen > count_before && !is_space[xbuf[totlen - 1]])
		     totlen--;
	       }
	     if (ap->stringify != 0)
	       {
		  memcpy(xbuf + totlen, ARG_BASE + arg->stringified,
			 arg->stringified_length);
		  totlen += arg->stringified_length;
	       }
	     else if (ap->raw_before || ap->raw_after)
	       {
		  unsigned char      *p1 = ARG_BASE + arg->raw;
		  unsigned char      *l1 = p1 + arg->raw_length;

		  if (ap->raw_before)
		    {
		       while (p1 != l1 && is_space[*p1])
			  p1++;
		       while (p1 != l1 && is_idchar[*p1])
			  xbuf[totlen++] = *p1++;
		    }
		  if (ap->raw_after)
		    {
		       /* Arg is concatenated after: delete trailing whitespace,
		        * whitespace markers, and no-reexpansion markers.  */
		       while (p1 != l1)
			 {
			    if (is_space[l1[-1]])
			       l1--;
			    else if (l1[-1] == '-')
			      {
				 unsigned char      *p2 = l1 - 1;

				 /* If a `-' is preceded by an odd number of newlines then it
				  * and the last newline are a no-reexpansion marker.  */
				 while (p2 != p1 && p2[-1] == '\n')
				    p2--;
				 if ((l1 - 1 - p2) & 1)
				   {
				      l1 -= 2;
				   }
				 else
				    break;
			      }
			    else
			       break;
			 }
		    }
		  memcpy(xbuf + totlen, p1, l1 - p1);
		  totlen += l1 - p1;
	       }
	     else
	       {
		  unsigned char      *expanded = ARG_BASE + arg->expanded;

		  if (!ap->raw_before && totlen > 0 && arg->expand_length
		      && unsafe_chars(xbuf[totlen - 1], expanded[0]))
		    {
		       xbuf[totlen++] = '@';
		       xbuf[totlen++] = ' ';
		    }
		  memcpy(xbuf + totlen, expanded, arg->expand_length);
		  totlen += arg->expand_length;

		  if (!ap->raw_after && totlen > 0 && offset < defn->length
		      && unsafe_chars(xbuf[totlen - 1], exp[offset]))
		    {
		       xbuf[totlen++] = '@';
		       xbuf[totlen++] = ' ';
		    }
		  /* If a macro argument with newlines is used multiple times,
		   * then only expand the newlines once.  This avoids creating
		   * output lines which don't correspond to any input line,
		   * which confuses gdb and gcov.  */
		  if (arg->use_count > 1 && arg->newlines > 0)
		    {
		       /* Don't bother doing change_newlines for subsequent
		        * uses of arg.  */
		       arg->use_count = 1;
		       arg->expand_length
			  = change_newlines(expanded, arg->expand_length);
		    }
	       }

	     if (totlen > xbuf_len)
		abort();
	  }
	/* if there is anything left of the definition
	 * after handling the arg list, copy that in too. */

	for (i = offset; i < defn->length; i++)
	  {
	     /* if we've reached the end of the macro */
	     if (exp[i] == ')')
		rest_zero = 0;
	     if (!(rest_zero && last_ap && last_ap->rest_args
		   && last_ap->raw_after))
		xbuf[totlen++] = exp[i];
	  }

	xbuf[totlen] = 0;
	xbuf_len = totlen;
     }

   pfile->output_escapes--;

   /* Now put the expansion on the input stack
    * so our caller will commence reading from it.  */
   push_macro_expansion(pfile, xbuf, xbuf_len, hp);
   CPP_BUFFER(pfile)->has_escapes = 1;

   /* Pop the space we've used in the token_buffer for argument expansion. */
   CPP_SET_WRITTEN(pfile, old_written);

   /* Recursive macro use sometimes works traditionally.
    * #define foo(x,y) bar (x (y,0), y)
    * foo (foo, baz)  */

   hp->type = T_DISABLED;
}

static void
push_macro_expansion(cpp_reader * pfile, unsigned char *xbuf, int xbuf_len,
		     HASHNODE * hp)
{
   cpp_buffer         *mbuf = cpp_push_buffer(pfile, xbuf, xbuf_len);

   mbuf->cleanup = macro_cleanup;
   mbuf->data = hp;

   /* The first chars of the expansion should be a "@ " added by
    * collect_expansion.  This is to prevent accidental token-pasting
    * between the text preceding the macro invocation, and the macro
    * expansion text.
    * 
    * We would like to avoid adding unneeded spaces (for the sake of
    * tools that use cpp, such as imake).  In some common cases we can
    * tell that it is safe to omit the space.
    * 
    * The character before the macro invocation cannot have been an
    * idchar (or else it would have been pasted with the idchars of
    * the macro name).  Therefore, if the first non-space character
    * of the expansion is an idchar, we do not need the extra space
    * to prevent token pasting.
    * 
    * Also, we don't need the extra space if the first char is '(',
    * or some other (less common) characters.  */

   if (xbuf[0] == '@' && xbuf[1] == ' '
       && (is_idchar[xbuf[2]] || xbuf[2] == '(' || xbuf[2] == '\''
	   || xbuf[2] == '\"'))
      mbuf->cur += 2;
}

/* Like cpp_get_token, except that it does not read past end-of-line.
 * Also, horizontal space is skipped, and macros are popped.  */

static enum cpp_token
get_directive_token(cpp_reader * pfile)
{
   for (;;)
     {
	long                old_written = CPP_WRITTEN(pfile);
	enum cpp_token      token;

	cpp_skip_hspace(pfile);
	if (PEEKC() == '\n')
	   return CPP_VSPACE;
	token = cpp_get_token(pfile);
	switch (token)
	  {
	  case CPP_POP:
	     if (!CPP_IS_MACRO_BUFFER(CPP_BUFFER(pfile)))
		return token;
#ifndef __has_attribute
# define __has_attribute(x) 0
#endif
#if __has_attribute(fallthrough)
             __attribute__((fallthrough));
#endif
             /* ... else fall though ... */
	  case CPP_HSPACE:
	  case CPP_COMMENT:
	     CPP_SET_WRITTEN(pfile, old_written);
	     break;
	  default:
	     return token;
	  }
     }
}

/* Handle #include and #import.
 * This function expects to see "fname" or <fname> on the input.
 * 
 * The input is normally in part of the output_buffer following
 * CPP_WRITTEN, and will get overwritten by output_line_command.
 * I.e. in input file specification has been popped by handle_directive.
 * This is safe. */

static int
do_include(cpp_reader * pfile, struct directive *keyword,
	   unsigned char *unused1 EINA_UNUSED, unsigned char *unused2 EINA_UNUSED)
{
   int                 importing = (keyword->type == T_IMPORT);
   int                 skip_dirs = (keyword->type == T_INCLUDE_NEXT);
   char               *fname;	/* Dynamically allocated fname buffer */
   unsigned char      *fbeg, *fend;	/* Beginning and end of fname */
   enum cpp_token      token;

   /* Chain of dirs to search */
   file_name_list     *search_start = CPP_OPTIONS(pfile)->include;
   file_name_list      dsp[1];	/* First in chain, if #include "..." */
   file_name_list     *searchptr = 0;
   long                old_written = CPP_WRITTEN(pfile);
   int                 flen;
   int                 f;	/* file number */
   int                 angle_brackets = 0;	/* 0 for "...", 1 for <...> */

   f = -1;			/* JF we iz paranoid! */

   if (importing && CPP_OPTIONS(pfile)->warn_import
       && !CPP_OPTIONS(pfile)->inhibit_warnings
       && !CPP_BUFFER(pfile)->system_header_p && !pfile->import_warning)
     {
	pfile->import_warning = 1;
	cpp_warning(pfile, "using `#import' is not recommended");
	fprintf(stderr,
		"The fact that a certain header file need not be processed more than once\n");
	fprintf(stderr,
		"should be indicated in the header file, not where it is used.\n");
	fprintf(stderr,
		"The best way to do this is with a conditional of this form:\n\n");
	fprintf(stderr, "  #ifndef _FOO_H_INCLUDED\n");
	fprintf(stderr, "  #define _FOO_H_INCLUDED\n");
	fprintf(stderr, "  ... <real contents of file> ...\n");
	fprintf(stderr, "  #endif /* Not _FOO_H_INCLUDED */\n\n");
	fprintf(stderr, "Then users can use `#include' any number of times.\n");
	fprintf(stderr,
		"GNU C automatically avoids processing the file more than once\n");
	fprintf(stderr, "when it is equipped with such a conditional.\n");
     }
   pfile->parsing_include_directive++;
   token = get_directive_token(pfile);
   pfile->parsing_include_directive--;

   if (token == CPP_STRING)
     {
	/* FIXME - check no trailing garbage */
	fbeg = pfile->token_buffer + old_written + 1;
	fend = CPP_PWRITTEN(pfile) - 1;
	if (fbeg[-1] == '<')
	  {
	     angle_brackets = 1;
	     /* If -I-, start with the first -I dir after the -I-.  */
	     if (CPP_OPTIONS(pfile)->first_bracket_include)
		search_start = CPP_OPTIONS(pfile)->first_bracket_include;
	  }
	/* If -I- was specified, don't search current dir, only spec'd ones. */
	else if (!CPP_OPTIONS(pfile)->ignore_srcdir)
	  {
	     cpp_buffer         *fp;

	     /* We have "filename".  Figure out directory this source
	      * file is coming from and put it on the front of the list. */

	     for (fp = CPP_BUFFER(pfile); fp != CPP_NULL_BUFFER(pfile); fp = CPP_PREV_BUFFER(fp))
	       {
		  int                 n;
		  const char         *ep, *nam;

		  if ((nam = fp->nominal_fname))
		    {
		       /* Found a named file.  Figure out dir of the file,
		        * and put it in front of the search list.  */
		       dsp[0].next = search_start;
		       search_start = dsp;
#ifndef VMS
		       ep = strrchr(nam, '/');
#else /* VMS */
		       ep = strrchr(nam, ']');
		       if (!ep)
			  ep = strrchr(nam, '>');
		       if (!ep)
			  ep = strrchr(nam, ':');
		       if (ep)
			  ep++;
#endif /* VMS */
		       if (ep)
			 {
			    n = ep - nam;
			    dsp[0].fname = (char *)alloca(n + 1);
			    strncpy(dsp[0].fname, nam, n);
			    dsp[0].fname[n] = '\0';
			    if (n + INCLUDE_LEN_FUDGE > pfile->max_include_len)
			       pfile->max_include_len = n + INCLUDE_LEN_FUDGE;
			 }
		       else
			 {
			    dsp[0].fname = 0;	/* Current directory */
			 }
		       dsp[0].got_name_map = 0;
		       break;
		    }
	       }
	  }
     }
   else
     {
	cpp_error(pfile,
		  "`#%s' expects \"FILENAME\" or <FILENAME>", keyword->name);
	CPP_SET_WRITTEN(pfile, old_written);
	skip_rest_of_line(pfile);
	return 0;
     }

   *fend = 0;

   token = get_directive_token(pfile);
   if (token != CPP_VSPACE)
     {
	cpp_error(pfile, "junk at end of `#include'");
	while (token != CPP_VSPACE && token != CPP_EOF && token != CPP_POP)
	   token = get_directive_token(pfile);
     }
   /* For #include_next, skip in the search path
    * past the dir in which the containing file was found.  */
   if (skip_dirs)
     {
	cpp_buffer         *fp;

	for (fp = CPP_BUFFER(pfile); fp != CPP_NULL_BUFFER(pfile); fp = CPP_PREV_BUFFER(fp))
	   if (fp->fname)
	     {
		/* fp->dir is null if the containing file was specified with
		 * an absolute file name.  In that case, don't skip anything.  */
		if (fp->dir == SELF_DIR_DUMMY)
		   search_start = CPP_OPTIONS(pfile)->include;
		else if (fp->dir)
		   search_start = fp->dir->next;
		break;
	     }
     }
   CPP_SET_WRITTEN(pfile, old_written);

   flen = fend - fbeg;

   if (flen == 0)
     {
	cpp_error(pfile, "empty file name in `#%s'", keyword->name);
	return 0;
     }
   /* Allocate this permanently, because it gets stored in the definitions
    * of macros.  */
   fname = (char *)xmalloc(pfile->max_include_len + flen + 4);
   /* + 2 above for slash and terminating null.  */
   /* + 2 added for '.h' on VMS (to support '#include filename') */

   /* If specified file name is absolute, just open it.  */

#ifndef __EMX__
   if (*fbeg == '/')
#else
   if (_fnisabs(fbeg))
#endif
     {
	strncpy(fname, (const char *)fbeg, flen);
	fname[flen] = 0;
        if (redundant_include_p(pfile, fname))
          {
             free(fname);
             return 0;
          }
	if (importing)
	   f = lookup_import(pfile, fname, NULL);
	else
	   f = open_include_file(pfile, fname, NULL);
        /* Already included this file */
        if (f == -2)
          {
             free(fname);
             return 0;
          }
     }
   else
     {
	/* Search directory path, trying to open the file.
	 * Copy each filename tried into FNAME.  */

	for (searchptr = search_start; searchptr; searchptr = searchptr->next)
	  {
	     if (searchptr->fname)
	       {
		  /* The empty string in a search path is ignored.
		   * This makes it possible to turn off entirely
		   * a standard piece of the list.  */
		  if (searchptr->fname[0] == 0)
		     continue;
		  strcpy(fname, searchptr->fname);
		  strcat(fname, "/");
		  fname[strlen(fname) + flen] = 0;
	       }
	     else
	       {
		  fname[0] = 0;
	       }
	     strncat(fname, (const char *)fbeg, flen);
#ifdef VMS
	     /* Change this 1/2 Unix 1/2 VMS file specification into a
	      * full VMS file specification */
	     if (searchptr->fname && (searchptr->fname[0] != 0))
	       {
		  /* Fix up the filename */
		  hack_vms_include_specification(fname);
	       }
	     else
	       {
		  /* This is a normal VMS filespec, so use it unchanged.  */
		  strncpy(fname, fbeg, flen);
		  fname[flen] = 0;
		  /* if it's '#include filename', add the missing .h */
		  if (!strchr(fname, '.'))
		    {
		       strcat(fname, ".h");
		    }
	       }
#endif /* VMS */
	     /* ??? There are currently 3 separate mechanisms for avoiding processing
	      * of redundant include files: #import, #pragma once, and
	      * redundant_include_p.  It would be nice if they were unified.  */
	     if (redundant_include_p(pfile, fname))
           {
              free(fname);
              return 0;
           }
	     if (importing)
		f = lookup_import(pfile, fname, searchptr);
	     else
		f = open_include_file(pfile, fname, searchptr);
	     if (f == -2)
           {
              free(fname);
              return 0;	/* Already included this file */
           }
#ifdef EACCES
	     else if (f == -1 && errno == EACCES)
		cpp_warning(pfile, "Header file %s exists, but is not readable",
			    fname);
#endif
	     if (f >= 0)
		break;
	  }
     }

   if (f < 0)
     {
	/* A file that was not found.  */
	strncpy(fname, (const char *)fbeg, flen);
	fname[flen] = 0;
	/* If generating dependencies and -MG was specified, we assume missing
	 * files are leaf files, living in the same directory as the source file
	 * or other similar place; these missing files may be generated from
	 * other files and may not exist yet (eg: y.tab.h).  */

	if (CPP_OPTIONS(pfile)->print_deps_missing_files
	    && CPP_PRINT_DEPS(pfile)
	    > (angle_brackets || (pfile->system_include_depth > 0)))
	  {
	     /* If it was requested as a system header file,
	      * then assume it belongs in the first place to look for such.  */
	     if (angle_brackets)
	       {
		  for (searchptr = search_start; searchptr;
		       searchptr = searchptr->next)
		    {
		       if (searchptr->fname)
			 {
			    char               *p;

			    if (searchptr->fname[0] == 0)
			       continue;
			    p = (char *)alloca(strlen(searchptr->fname)
					       + strlen(fname) + 2);
			    strcpy(p, searchptr->fname);
			    strcat(p, "/");
			    strcat(p, fname);
			    deps_output(pfile, p, ' ');
			    break;
			 }
		    }
	       }
	     else
	       {
		  /* Otherwise, omit the directory, as if the file existed
		   * in the directory with the source.  */
		  deps_output(pfile, fname, ' ');
	       }
	  }
	/* If -M was specified, and this header file won't be added to the
	 * dependency list, then don't count this as an error, because we can
	 * still produce correct output.  Otherwise, we can't produce correct
	 * output, because there may be dependencies we need inside the missing
	 * file, and we don't know what directory this missing file exists in. */
	else if (CPP_PRINT_DEPS(pfile)
		 && (CPP_PRINT_DEPS(pfile)
		     <= (angle_brackets || (pfile->system_include_depth > 0))))
	   cpp_warning(pfile, "No include path in which to find %s", fname);
	else if (search_start)
	   cpp_error_from_errno(pfile, fname);
	else
	   cpp_error(pfile, "No include path in which to find %s", fname);
     }
   else
     {
	/* Check to see if this include file is a once-only include file.
	 * If so, give up.  */

	file_name_list     *ptr;

	for (ptr = pfile->dont_repeat_files; ptr; ptr = ptr->next)
	  {
	     if (!strcmp(ptr->fname, fname))
	       {
		  close(f);
          free(fname);
		  return 0;	/* This file was once'd. */
	       }
	  }

	for (ptr = pfile->all_include_files; ptr; ptr = ptr->next)
	  {
	     if (!strcmp(ptr->fname, fname))
		break;		/* This file was included before. */
	  }

	if (!ptr)
	  {
	     /* This is the first time for this file.  */
	     /* Add it to list of files included.  */

	     ptr = (file_name_list *) xmalloc(sizeof(file_name_list));

	     ptr->control_macro = 0;
	     ptr->c_system_include_path = 0;
	     ptr->next = pfile->all_include_files;
	     pfile->all_include_files = ptr;
	     ptr->fname = savestring(fname);
	     ptr->got_name_map = 0;

	     /* For -M, add this file to the dependencies.  */
	     if (CPP_PRINT_DEPS(pfile)
		 > (angle_brackets || (pfile->system_include_depth > 0)))
		deps_output(pfile, fname, ' ');
	  }
	/* Handle -H option.  */
	if (CPP_OPTIONS(pfile)->print_include_names)
	  {
	     cpp_buffer         *buf = CPP_BUFFER(pfile);

	     while ((buf = CPP_PREV_BUFFER(buf)) != CPP_NULL_BUFFER(pfile))
		putc('.', stderr);
	     fprintf(stderr, "%s\n", fname);
	  }
	if (angle_brackets)
	   pfile->system_include_depth++;

	/* Actually process the file.  */

	/* Record file on "seen" list for #import. */
	add_import(pfile, f, fname);

	/* Actually process the file */
	cpp_push_buffer(pfile, NULL, 0);
	if (finclude(pfile, f, fname, is_system_include(pfile, fname),
		     searchptr != dsp ? searchptr : SELF_DIR_DUMMY))
	  {
	     output_line_command(pfile, 0, enter_file);
	     pfile->only_seen_white = 2;
	  }
	if (angle_brackets)
	   pfile->system_include_depth--;
     }

   // We are leaking fname here. This is intended as it may still be used later
   // on. It would be better on to track that correctly and fix it, but as it
   // only affect recursive include this leak is not important and time will be
   // better spent somewhere else.
   return 0;
}

/* Return nonzero if there is no need to include file NAME
 * because it has already been included and it contains a conditional
 * to make a repeated include do nothing.  */

static int
redundant_include_p(cpp_reader * pfile, char *name)
{
   file_name_list     *l = pfile->all_include_files;

   for (; l; l = l->next)
      if (!strcmp(name, l->fname)
	  && l->control_macro
	  && cpp_lookup((const char *)l->control_macro, -1, -1))
	 return 1;
   return 0;
}

/* Return nonzero if the given FILENAME is an absolute pathname which
 * designates a file within one of the known "system" include file
 * directories.  We assume here that if the given FILENAME looks like
 * it is the name of a file which resides either directly in a "system"
 * include file directory, or within any subdirectory thereof, then the
 * given file must be a "system" include file.  This function tells us
 * if we should suppress pedantic errors/warnings for the given FILENAME.
 * 
 * The value is 2 if the file is a C-language system header file
 * for which C++ should (on most systems) assume `extern "C"'.  */

static int
is_system_include(cpp_reader * pfile, char *filename)
{
   file_name_list     *searchptr;

   for (searchptr = CPP_OPTIONS(pfile)->first_system_include; searchptr;
	searchptr = searchptr->next)
      if (searchptr->fname)
	{
	   char               *sys_dir = searchptr->fname;
	   unsigned            length = strlen(sys_dir);

	   if (!strncmp(sys_dir, filename, length) && filename[length] == '/')
	     {
		if (searchptr->c_system_include_path)
		   return 2;
		else
		   return 1;
	     }
	}
   return 0;
}

/*
 * Install a name in the assertion hash table.
 *
 * If LEN is >= 0, it is the length of the name.
 * Otherwise, compute the length by scanning the entire name.
 *
 * If HASH is >= 0, it is the precomputed hash code.
 * Otherwise, compute the hash code.
 */
static ASSERTION_HASHNODE *
assertion_install(cpp_reader * pfile, const char *name, int len, int hash)
{
   ASSERTION_HASHNODE *hp;
   int                 i, bucket;

   i = sizeof(ASSERTION_HASHNODE) + len + 1;
   hp = (ASSERTION_HASHNODE *) xmalloc(i);
   bucket = hash;
   hp->bucket_hdr = &pfile->assertion_hashtab[bucket];
   hp->next = pfile->assertion_hashtab[bucket];
   pfile->assertion_hashtab[bucket] = hp;
   hp->prev = NULL;
   if (hp->next)
      hp->next->prev = hp;
   hp->length = len;
   hp->value = 0;
   hp->name = ((char *)hp) + sizeof(ASSERTION_HASHNODE);
   memcpy(hp->name, name, len);
   hp->name[len] = 0;
   return hp;
}
/*
 * find the most recent hash node for name name (ending with first
 * non-identifier char) installed by install
 *
 * If LEN is >= 0, it is the length of the name.
 * Otherwise, compute the length by scanning the entire name.
 *
 * If HASH is >= 0, it is the precomputed hash code.
 * Otherwise, compute the hash code.
 */

static ASSERTION_HASHNODE *
assertion_lookup(cpp_reader * pfile, const char *name, int len, int hash)
{
   ASSERTION_HASHNODE *bucket;

   bucket = pfile->assertion_hashtab[hash];
   while (bucket)
     {
	if (bucket->length == len && strncmp(bucket->name, name, len) == 0)
	   return bucket;
	bucket = bucket->next;
     }
   return NULL;
}

static void
delete_assertion(ASSERTION_HASHNODE * hp)
{
   struct tokenlist_list *tail;

   if (hp->prev)
      hp->prev->next = hp->next;
   if (hp->next)
      hp->next->prev = hp->prev;

   for (tail = hp->value; tail;)
     {
	struct tokenlist_list *next = tail->next;

	free_token_list(tail->tokens);
	free(tail);
	tail = next;
     }

   /* make sure that the bucket chain header that
    * the deleted guy was on points to the right thing afterwards. */
   if (hp == *hp->bucket_hdr)
      *hp->bucket_hdr = hp->next;

   free(hp);
}

/* Convert a character string literal into a nul-terminated string.
 * The input string is [IN ... LIMIT).
 * The result is placed in RESULT.  RESULT can be the same as IN.
 * The value returned in the end of the string written to RESULT,
 * or NULL on error.  */

static char        *
convert_string(cpp_reader * pfile, char *result, char *in, char *limit,
	       int handle_escapes)
{
   unsigned char       c;

   c = *in++;
   if (c != '\"')
      return NULL;
   while (in < limit)
     {
	c = *in++;

	switch (c)
	  {
	  case '\0':
	     return NULL;
	  case '\"':
	     limit = in;
	     break;
	  case '\\':
	     if (handle_escapes)
	       {
		  char               *bpc = in;
		  int                 i =
		     (unsigned char)cpp_parse_escape(pfile, &bpc);

		  in = bpc;
		  if (i >= 0)
		     *result++ = (unsigned char)c;
		  break;
	       }
	     /* else fall through */
	  default:
	     *result++ = c;
	  }
     }
   *result = 0;
   return result;
}

/*
 * interpret #line command.  Remembers previously seen fnames
 * in its very own hash table.
 */
#define FNAME_HASHSIZE 37

static int
do_line(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
	unsigned char *unused1 EINA_UNUSED, unsigned char *unused2 EINA_UNUSED)
{
   cpp_buffer         *ip = CPP_BUFFER(pfile);
   int                 new_lineno;
   long                old_written = CPP_WRITTEN(pfile);
   enum file_change_code file_change = same_file;
   enum cpp_token      token;

   token = get_directive_token(pfile);

   if (token != CPP_NUMBER || !isdigit(pfile->token_buffer[old_written]))
     {
	cpp_error(pfile, "invalid format `#line' command");
	goto bad_line_directive;
     }
   /* The Newline at the end of this line remains to be processed.
    * To put the next line at the specified line number,
    * we must store a line number now that is one less.  */
   new_lineno = atoi((char *)(pfile->token_buffer + old_written)) - 1;
   CPP_SET_WRITTEN(pfile, old_written);

   /* NEW_LINENO is one less than the actual line number here.  */
   if (CPP_PEDANTIC(pfile) && new_lineno < 0)
      cpp_pedwarn(pfile, "line number out of range in `#line' command");

   token = get_directive_token(pfile);

   if (token == CPP_STRING)
     {
	char               *fname = (char *)pfile->token_buffer + old_written;
	char               *end_name;
	static HASHNODE    *fname_table[FNAME_HASHSIZE];
	HASHNODE           *hp, **hash_bucket;
	unsigned char      *p;
	long                num_start;
	int                 fname_length;

	/* Turn the file name, which is a character string literal,
	 * into a null-terminated string.  Do this in place.  */
	end_name =
	   convert_string(pfile, fname, fname, (char *)CPP_PWRITTEN(pfile), 1);
	if (!end_name)
	  {
	     cpp_error(pfile, "invalid format `#line' command");
	     goto bad_line_directive;
	  }
	fname_length = end_name - fname;

	num_start = CPP_WRITTEN(pfile);
	token = get_directive_token(pfile);
	if (token != CPP_VSPACE && token != CPP_EOF && token != CPP_POP)
	  {
	     p = pfile->token_buffer + num_start;
	     if (CPP_PEDANTIC(pfile))
		cpp_pedwarn(pfile, "garbage at end of `#line' command");

	     if (token != CPP_NUMBER || *p < '0' || *p > '4' || p[1] != '\0')
	       {
		  cpp_error(pfile, "invalid format `#line' command");
		  goto bad_line_directive;
	       }
	     if (*p == '1')
		file_change = enter_file;
	     else if (*p == 2)
		file_change = leave_file;
	     else if (*p == 3)
		ip->system_header_p = 1;
	     else		/* if (*p == 4) */
		ip->system_header_p = 2;

	     CPP_SET_WRITTEN(pfile, num_start);
	     token = get_directive_token(pfile);
	     p = pfile->token_buffer + num_start;
	     if (token == CPP_NUMBER && p[1] == '\0'
		 && (*p == '3' || *p == '4'))
	       {
		  ip->system_header_p = *p == 3 ? 1 : 2;
		  token = get_directive_token(pfile);
	       }
	     if (token != CPP_VSPACE)
	       {
		  cpp_error(pfile, "invalid format `#line' command");
		  goto bad_line_directive;
	       }
	  }
	hash_bucket = &fname_table[hashf(fname, fname_length, FNAME_HASHSIZE)];
	for (hp = *hash_bucket; hp; hp = hp->next)
	   if (hp->length == fname_length &&
	       strncmp(hp->value.cpval, fname, fname_length) == 0)
	     {
		ip->nominal_fname = hp->value.cpval;
		break;
	     }
	if (!hp)
	  {
	     /* Didn't find it; cons up a new one.  */
	     hp = (HASHNODE *) xcalloc(1, sizeof(HASHNODE) + fname_length + 1);
	     hp->next = *hash_bucket;
	     *hash_bucket = hp;

	     hp->length = fname_length;
	     ip->nominal_fname = hp->value.cpval =
		((char *)hp) + sizeof(HASHNODE);
	     memcpy(hp->value.cpval, fname, fname_length);
	  }
     }
   else if (token != CPP_VSPACE && token != CPP_EOF)
     {
	cpp_error(pfile, "invalid format `#line' command");
	goto bad_line_directive;
     }
   ip->lineno = new_lineno;
 bad_line_directive:
   skip_rest_of_line(pfile);
   CPP_SET_WRITTEN(pfile, old_written);
   output_line_command(pfile, 0, file_change);
   return 0;
}

/*
 * remove the definition of a symbol from the symbol table.
 * according to un*x /lib/cpp, it is not an error to undef
 * something that has no definitions, so it isn't one here either.
 */

static int
do_undef(cpp_reader * pfile, struct directive *keyword, unsigned char *buf,
	 unsigned char *limit)
{
   int                 sym_length;
   HASHNODE           *hp;
   unsigned char      *orig_buf = buf;

   SKIP_WHITE_SPACE(buf);
   sym_length = check_macro_name(pfile, buf, "macro");

   while ((hp = cpp_lookup((const char *)buf, sym_length, -1)))
     {
	/* If we are generating additional info for debugging (with -g) we
	 * need to pass through all effective #undef commands.  */
	if (CPP_OPTIONS(pfile)->debug_output && keyword)
	   pass_thru_directive((char *)orig_buf, (char *)limit, pfile, keyword);
	if (hp->type != T_MACRO)
	   cpp_warning(pfile, "undefining `%s'", hp->name);
	delete_macro(hp);
     }

   if (CPP_PEDANTIC(pfile))
     {
	buf += sym_length;
	SKIP_WHITE_SPACE(buf);
	if (buf != limit)
	   cpp_pedwarn(pfile, "garbage after `#undef' directive");
     }
   return 0;
}

/*
 * Report an error detected by the program we are processing.
 * Use the text of the line in the error message.
 * (We use error because it prints the filename & line#.)
 */

static int
do_error(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
	 unsigned char *buf, unsigned char *limit)
{
   int                 length = limit - buf;
   unsigned char      *copy = (unsigned char *)xmalloc(length + 1);
   unsigned char      *msg = copy;

   memcpy(copy, buf, length);
   copy[length] = 0;
   SKIP_WHITE_SPACE(msg);
   cpp_error(pfile, "#error %s", msg);
   free(copy);
   return 0;
}

/*
 * Report a warning detected by the program we are processing.
 * Use the text of the line in the warning message, then continue.
 * (We use error because it prints the filename & line#.)
 */

static int
do_warning(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
	   unsigned char *buf, unsigned char *limit)
{
   int                 length = limit - buf;
   unsigned char      *copy = (unsigned char *)xmalloc(length + 1);
   unsigned char      *msg = copy;

   memcpy(copy, buf, length);
   copy[length] = 0;
   SKIP_WHITE_SPACE(msg);
   cpp_warning(pfile, "#warning %s", msg);
   free(copy);
   return 0;
}

/* Remember the name of the current file being read from so that we can
 * avoid ever including it again.  */

static int
do_once(cpp_reader * pfile)
{
   cpp_buffer         *ip = NULL;
   file_name_list     *new_;

   for (ip = CPP_BUFFER(pfile);; ip = CPP_PREV_BUFFER(ip))
     {
	if (!ip)
	   return 0;
	if (ip->fname)
	   break;
     }

   new_ = (file_name_list *) xmalloc(sizeof(file_name_list));

   new_->next = pfile->dont_repeat_files;
   pfile->dont_repeat_files = new_;
   new_->fname = savestring(ip->fname);
   new_->control_macro = 0;
   new_->got_name_map = 0;
   new_->c_system_include_path = 0;

   return 0;
}

/* #ident has already been copied to the output file, so just ignore it.  */

static int
do_ident(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
	 unsigned char *buf EINA_UNUSED, unsigned char *limit EINA_UNUSED)
{
/*  long old_written = CPP_WRITTEN (pfile); */

   /* Allow #ident in system headers, since that's not user's fault.  */
   if (CPP_PEDANTIC(pfile) && !CPP_BUFFER(pfile)->system_header_p)
      cpp_pedwarn(pfile, "ANSI C does not allow `#ident'");

   /* Leave rest of line to be read by later calls to cpp_get_token. */

   return 0;
}

/* #pragma and its argument line have already been copied to the output file.
 * Just check for some recognized pragmas that need validation here.  */

static int
do_pragma(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
	  unsigned char *buf, unsigned char *limit EINA_UNUSED)
{
   while (*buf == ' ' || *buf == '\t')
      buf++;

   if (!strncmp((const char *)buf, "once", 4))
     {
	/* Allow #pragma once in system headers, since that's not the user's
	 * fault.  */
	if (!CPP_BUFFER(pfile)->system_header_p)
	   cpp_warning(pfile, "`#pragma once' is obsolete");
	do_once(pfile);
     }
   if (!strncmp((const char *)buf, "implementation", 14))
     {
	/* Be quiet about `#pragma implementation' for a file only if it hasn't
	 * been included yet.  */
	file_name_list     *ptr;
	char               *p = (char *)buf + 14, *fname, *inc_fname;
	int                 fname_len;

	SKIP_WHITE_SPACE(p);
	if (*p == '\n' || *p != '\"')
	   return 0;

	fname = p + 1;
	p = strchr(fname, '\"');
	fname_len = (p) ? (int)(p - fname) : (int)strlen(fname);

	for (ptr = pfile->all_include_files; ptr; ptr = ptr->next)
	  {
	     inc_fname = strrchr(ptr->fname, '/');
	     inc_fname = inc_fname ? inc_fname + 1 : (char *)ptr->fname;
	     if (inc_fname && !strncmp(inc_fname, fname, fname_len))
		cpp_warning(pfile,
			    "`#pragma implementation' for `%s' appears after file is included",
			    fname);
	  }
     }
   return 0;
}

/* Just ignore #sccs, on systems where we define it at all.  */

/*
 * handle #if command by
 *   1) inserting special `defined' keyword into the hash table
 *      that gets turned into 0 or 1 by special_symbol (thus,
 *      if the luser has a symbol called `defined' already, it won't
 *      work inside the #if command)
 *   2) rescan the input into a temporary output buffer
 *   3) pass the output buffer to the yacc parser and collect a value
 *   4) clean up the mess left from steps 1 and 2.
 *   5) call conditional_skip to skip til the next #endif (etc.),
 *      or not, depending on the value from step 3.
 */

static int
do_if(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
      unsigned char *buf, unsigned char *limit)
{
   HOST_WIDE_INT       value = eval_if_expression(pfile, buf, limit - buf);

   conditional_skip(pfile, value == 0, T_IF, NULL);
   return 0;
}

/*
 * handle a #elif directive by not changing  if_stack  either.
 * see the comment above do_else.
 */

static int
do_elif(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
	unsigned char *buf, unsigned char *limit)
{
   if (pfile->if_stack == CPP_BUFFER(pfile)->if_stack)
     {
	cpp_error(pfile, "`#elif' not within a conditional");
	return 0;
     }
   else
     {
	if (pfile->if_stack->type != T_IF && pfile->if_stack->type != T_ELIF)
	  {
	     cpp_error(pfile, "`#elif' after `#else'");
	     if (pfile->if_stack->fname
		 && CPP_BUFFER(pfile)->fname
		 && strcmp(pfile->if_stack->fname,
			   CPP_BUFFER(pfile)->nominal_fname) != 0)
		fprintf(stderr, ", file %s", pfile->if_stack->fname);
	     fprintf(stderr, ")\n");
	  }
	pfile->if_stack->type = T_ELIF;
     }

   if (pfile->if_stack->if_succeeded)
      skip_if_group(pfile, 0);
   else
     {
	HOST_WIDE_INT       value = eval_if_expression(pfile, buf, limit - buf);

	if (value == 0)
	   skip_if_group(pfile, 0);
	else
	  {
	     ++pfile->if_stack->if_succeeded;	/* continue processing input */
	     output_line_command(pfile, 1, same_file);
	  }
     }
   return 0;
}

/*
 * evaluate a #if expression in BUF, of length LENGTH,
 * then parse the result as a C expression and return the value as an int.
 */
static              HOST_WIDE_INT
eval_if_expression(cpp_reader * pfile, unsigned char *buf EINA_UNUSED,
		   int length EINA_UNUSED)
{
   HASHNODE           *save_defined;
   HOST_WIDE_INT       value;
   long                old_written = CPP_WRITTEN(pfile);

   save_defined = install("defined", -1, T_SPEC_DEFINED, 0, 0, -1);
   pfile->pcp_inside_if = 1;

   value = cpp_parse_expr(pfile);
   pfile->pcp_inside_if = 0;
   delete_macro(save_defined);	/* clean up special symbol */

   CPP_SET_WRITTEN(pfile, old_written);	/* Pop */

   return value;
}

/*
 * routine to handle ifdef/ifndef.  Try to look up the symbol,
 * then do or don't skip to the #endif/#else/#elif depending
 * on what directive is actually being processed.
 */

static int
do_xifdef(cpp_reader * pfile, struct directive *keyword,
	  unsigned char *unused1 EINA_UNUSED, unsigned char *unused2 EINA_UNUSED)
{
   int                 skip;
   cpp_buffer         *ip = CPP_BUFFER(pfile);
   char               *ident;
   int                 ident_length;
   enum cpp_token      token;
   int                 start_of_file = 0;
   unsigned char      *control_macro = 0;
   int                 old_written = CPP_WRITTEN(pfile);
   int                 c;

   /* Detect a #ifndef at start of file (not counting comments).  */
   if (ip->fname != 0 && keyword->type == T_IFNDEF)
      start_of_file = pfile->only_seen_white == 2;

   pfile->no_macro_expand++;
   token = get_directive_token(pfile);
   pfile->no_macro_expand--;

   ident = (char *)pfile->token_buffer + old_written;
   ident_length = CPP_WRITTEN(pfile) - old_written;
   CPP_SET_WRITTEN(pfile, old_written);	/* Pop */

   if (token == CPP_VSPACE || token == CPP_POP || token == CPP_EOF)
     {
	skip = (keyword->type == T_IFDEF);
        cpp_pedwarn(pfile, "`#%s' with no argument", keyword->name);
     }
   else if (token == CPP_NAME)
     {
	HASHNODE           *hp = cpp_lookup(ident, ident_length, -1);

	skip = (!hp) ^ (keyword->type == T_IFNDEF);
	if (start_of_file && !skip)
	  {
	     control_macro = (unsigned char *)xmalloc(ident_length + 1);
	     memcpy(control_macro, ident, ident_length + 1);
	  }
     }
   else
     {
	skip = (keyword->type == T_IFDEF);
        cpp_error(pfile, "`#%s' with invalid argument", keyword->name);
     }

   cpp_skip_hspace(pfile);
   c = PEEKC();
   if (c != EOF && c != '\n')
      cpp_pedwarn(pfile, "garbage at end of `#%s' argument",
                  keyword->name);
   skip_rest_of_line(pfile);

   conditional_skip(pfile, skip, T_IF, control_macro);
   return 0;
}

/* Push TYPE on stack; then, if SKIP is nonzero, skip ahead.
 * If this is a #ifndef starting at the beginning of a file,
 * CONTROL_MACRO is the macro name tested by the #ifndef.
 * Otherwise, CONTROL_MACRO is 0.  */

static void
conditional_skip(cpp_reader * pfile, int skip, enum node_type type,
		 unsigned char *control_macro)
{
   IF_STACK_FRAME     *temp;

   temp = (IF_STACK_FRAME *) xcalloc(1, sizeof(IF_STACK_FRAME));
   temp->fname = CPP_BUFFER(pfile)->nominal_fname;
   temp->next = pfile->if_stack;
   temp->control_macro = control_macro;
   pfile->if_stack = temp;

   pfile->if_stack->type = type;

   if (skip != 0)
     {
	skip_if_group(pfile, 0);
	return;
     }
   else
     {
	++pfile->if_stack->if_succeeded;
	output_line_command(pfile, 1, same_file);
     }
}

/*
 * skip to #endif, #else, or #elif.  adjust line numbers, etc.
 * leaves input ptr at the sharp sign found.
 * If ANY is nonzero, return at next directive of any sort.
 */

static void
skip_if_group(cpp_reader * pfile, int any)
{
   int                 c;
   struct directive   *kt;
   IF_STACK_FRAME     *save_if_stack = pfile->if_stack;	/* don't pop past here */

   int                 ident_length;
   char               *ident;
   struct parse_marker line_start_mark;

   parse_set_mark(&line_start_mark, pfile);

   if (CPP_OPTIONS(pfile)->output_conditionals)
     {
	static char         failed[] = "#failed\n";

	CPP_PUTS(pfile, failed, sizeof(failed) - 1);
	pfile->lineno++;
	output_line_command(pfile, 1, same_file);
     }
 beg_of_line:
   if (CPP_OPTIONS(pfile)->output_conditionals)
     {
	cpp_buffer         *pbuf = CPP_BUFFER(pfile);
	unsigned char      *start_line = pbuf->buf + line_start_mark.position;

	CPP_PUTS(pfile, start_line, pbuf->cur - start_line);
     }
   parse_move_mark(&line_start_mark, pfile);
   cpp_skip_hspace(pfile);
   c = GETC();
   if (c == '#')
     {
	int                 old_written = CPP_WRITTEN(pfile);

	cpp_skip_hspace(pfile);

	parse_name(pfile, GETC());
	ident_length = CPP_WRITTEN(pfile) - old_written;
	ident = (char *)pfile->token_buffer + old_written;
	pfile->limit = (unsigned char *)ident;

	for (kt = directive_table; kt->length >= 0; kt++)
	  {
	     IF_STACK_FRAME     *temp;

	     if (ident_length == kt->length
		 && strncmp(ident, kt->name, kt->length) == 0)
	       {
		  /* If we are asked to return on next directive, do so now.  */
		  if (any)
		     goto done;

		  switch (kt->type)
		    {
		    case T_IF:
		    case T_IFDEF:
		    case T_IFNDEF:
		       temp
			  =
			  (IF_STACK_FRAME *) xcalloc(1, sizeof(IF_STACK_FRAME));
		       temp->next = pfile->if_stack;
		       pfile->if_stack = temp;
		       temp->fname = CPP_BUFFER(pfile)->nominal_fname;
		       temp->type = kt->type;
		       break;
		    case T_ELSE:
		    case T_ENDIF:
		       if (CPP_PEDANTIC(pfile)
			   && pfile->if_stack != save_if_stack)
			  validate_else(pfile,
					kt->type ==
					T_ELSE ? "#else" : "#endif");
#ifndef __has_attribute
# define __has_attribute(x) 0
#endif
#if __has_attribute(fallthrough)
                       __attribute__((fallthrough));
#endif
                       /* this fall through is intened */
		    case T_ELIF:
		       if (pfile->if_stack == CPP_BUFFER(pfile)->if_stack)
			 {
			    cpp_error(pfile,
				      "`#%s' not within a conditional",
				      kt->name);
			    break;
			 }
		       else if (pfile->if_stack == save_if_stack)
			  goto done;	/* found what we came for */

		       if (kt->type != T_ENDIF)
			 {
			    if (pfile->if_stack->type == T_ELSE)
			       cpp_error(pfile,
					 "`#else' or `#elif' after `#else'");
			    pfile->if_stack->type = kt->type;
			    break;
			 }
		       temp = pfile->if_stack;
		       pfile->if_stack = temp->next;
		       free(temp);
		       break;
		    default:;
		    }
		  break;
	       }
	     /* Don't let erroneous code go by.  */
	     if (kt->length < 0 && !CPP_OPTIONS(pfile)->lang_asm
		 && CPP_PEDANTIC(pfile))
		cpp_pedwarn(pfile, "invalid preprocessor directive name");
	  }
	c = GETC();
     }
   /* We're in the middle of a line.  Skip the rest of it. */
   for (;;)
     {
	switch (c)
	  {
	     long                old;

	  case EOF:
	     goto done;
	  case '/':		/* possible comment */
	     c = skip_comment(pfile, NULL);
	     if (c == EOF)
		goto done;
	     break;
	  case '\"':
	  case '\'':
	     FORWARD(-1);
	     old = CPP_WRITTEN(pfile);
	     cpp_get_token(pfile);
	     CPP_SET_WRITTEN(pfile, old);
	     break;
	  case '\\':
	     /* Char after backslash loses its special meaning.  */
	     if (PEEKC() == '\n')
		FORWARD(1);
	     break;
	  case '\n':
	     goto beg_of_line;
	     break;
	  }
	c = GETC();
     }
 done:
   if (CPP_OPTIONS(pfile)->output_conditionals)
     {
	static char         end_failed[] = "#endfailed\n";

	CPP_PUTS(pfile, end_failed, sizeof(end_failed) - 1);
	pfile->lineno++;
     }
   pfile->only_seen_white = 1;
   parse_goto_mark(&line_start_mark, pfile);
   parse_clear_mark(&line_start_mark);
}

/*
 * handle a #else directive.  Do this by just continuing processing
 * without changing  if_stack ;  this is so that the error message
 * for missing #endif's etc. will point to the original #if.  It
 * is possible that something different would be better.
 */

static int
do_else(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
	unsigned char *buf EINA_UNUSED, unsigned char *limit EINA_UNUSED)
{
   cpp_buffer         *ip = CPP_BUFFER(pfile);

   if (CPP_PEDANTIC(pfile))
      validate_else(pfile, "#else");
   skip_rest_of_line(pfile);

   if (pfile->if_stack == CPP_BUFFER(pfile)->if_stack)
     {
	cpp_error(pfile, "`#else' not within a conditional");
	return 0;
     }
   else
     {
	/* #ifndef can't have its special treatment for containing the whole file
	 * if it has a #else clause.  */
	pfile->if_stack->control_macro = 0;

	if (pfile->if_stack->type != T_IF && pfile->if_stack->type != T_ELIF)
	  {
	     cpp_error(pfile, "`#else' after `#else'");
	     fprintf(stderr, " (matches line %d", pfile->if_stack->lineno);
	     if (strcmp(pfile->if_stack->fname, ip->nominal_fname) != 0)
		fprintf(stderr, ", file %s", pfile->if_stack->fname);
	     fprintf(stderr, ")\n");
	  }
	pfile->if_stack->type = T_ELSE;
     }

   if (pfile->if_stack->if_succeeded)
      skip_if_group(pfile, 0);
   else
     {
	++pfile->if_stack->if_succeeded;	/* continue processing input */
	output_line_command(pfile, 1, same_file);
     }
   return 0;
}

/*
 * unstack after #endif command
 */

static int
do_endif(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
	 unsigned char *buf EINA_UNUSED, unsigned char *limit EINA_UNUSED)
{
   if (CPP_PEDANTIC(pfile))
      validate_else(pfile, "#endif");
   skip_rest_of_line(pfile);

   if (pfile->if_stack == CPP_BUFFER(pfile)->if_stack)
     {
	cpp_error(pfile, "unbalanced `#endif'");
     }
   else
     {
	IF_STACK_FRAME     *temp = pfile->if_stack;

	pfile->if_stack = temp->next;
	if (temp->control_macro)
	  {
	     /* This #endif matched a #ifndef at the start of the file.
	      * See if it is at the end of the file.  */
	     struct parse_marker start_mark;
	     int                 c;

	     parse_set_mark(&start_mark, pfile);

	     for (;;)
	       {
		  cpp_skip_hspace(pfile);
		  c = GETC();
		  if (c != '\n')
		     break;
	       }
	     parse_goto_mark(&start_mark, pfile);
	     parse_clear_mark(&start_mark);

	     if (c == EOF)
	       {
		  /* If we get here, this #endif ends a #ifndef
		   * that contains all of the file (aside from whitespace).
		   * Arrange not to include the file again
		   * if the macro that was tested is defined.
		   * 
		   * Do not do this for the top-level file in a -include or any
		   * file in a -imacros.  */
		  {
		     file_name_list     *ifile = pfile->all_include_files;

		     for (; ifile; ifile = ifile->next)
		       {
			  if (!strcmp(ifile->fname, CPP_BUFFER(pfile)->fname))
			    {
			       ifile->control_macro = temp->control_macro;
			       break;
			    }
		       }
		  }
	       }
	  }
	free(temp);
	output_line_command(pfile, 1, same_file);
     }
   return 0;
}

/* When an #else or #endif is found while skipping failed conditional,
 * if -pedantic was specified, this is called to warn about text after
 * the command name.  P points to the first char after the command name.  */

static void
validate_else(cpp_reader * pfile, const char *directive)
{
   int                 c;

   cpp_skip_hspace(pfile);
   c = PEEKC();
   if (c != EOF && c != '\n')
      cpp_pedwarn(pfile,
		  "text following `%s' violates ANSI standard", directive);
}

/* Get the next token, and add it to the text in pfile->token_buffer.
 * Return the kind of token we got. */

enum cpp_token
cpp_get_token(cpp_reader * pfile)
{
   int                 c, c2, c3;
   long                old_written = 0;
   long                start_line = 0, start_column = 0;
   enum cpp_token      token;
   struct cpp_options *opts = CPP_OPTIONS(pfile);

   CPP_BUFFER(pfile)->prev = CPP_BUFFER(pfile)->cur;
 get_next:
   c = GETC();
   if (c == EOF)
     {
      handle_eof:
	if (CPP_BUFFER(pfile)->seen_eof)
	  {
	     if (cpp_pop_buffer(pfile) != CPP_NULL_BUFFER(pfile))
		goto get_next;
	     else
		return CPP_EOF;
	  }
	else
	  {
	     cpp_buffer         *next_buf = CPP_PREV_BUFFER(CPP_BUFFER(pfile));

	     CPP_BUFFER(pfile)->seen_eof = 1;
	     if (CPP_BUFFER(pfile)->nominal_fname && next_buf)
	       {
		  /* We're about to return from an #include file.
		   * Emit #line information now (as part of the CPP_POP) result.
		   * But the #line refers to the file we will pop to. */
		  cpp_buffer         *cur_buffer = CPP_BUFFER(pfile);

		  CPP_BUFFER(pfile) = next_buf;
		  pfile->input_stack_listing_current = 0;
		  output_line_command(pfile, 0, leave_file);
		  CPP_BUFFER(pfile) = cur_buffer;
	       }
	     return CPP_POP;
	  }
     }
   else
     {
	long                newlines = 0;
	struct parse_marker start_mark;
	switch (c)
	  {

	  case '/':
	     if (PEEKC() == '=')
		goto op2;
	     if (opts->put_out_comments)
		parse_set_mark(&start_mark, pfile);
	     newlines = 0;
	     cpp_buf_line_and_col(cpp_file_buffer(pfile),
				  &start_line, &start_column);
	     c = skip_comment(pfile, &newlines);
	     if (opts->put_out_comments && (c == '/' || c == EOF))
		parse_clear_mark(&start_mark);
	     if (c == '/')
		goto randomchar;
	     if (c == EOF)
	       {
		  cpp_error_with_line(pfile, start_line, start_column,
				      "unterminated comment");
		  goto handle_eof;
	       }
	     c = '/';		/* Initial letter of comment. */
	   return_comment:
	     /* Comments are equivalent to spaces.
	      * For -traditional, a comment is equivalent to nothing.  */
	     if (opts->put_out_comments)
	       {
		  cpp_buffer         *pbuf = CPP_BUFFER(pfile);
		  unsigned char      *start = pbuf->buf + start_mark.position;
		  int                 len = pbuf->cur - start;

		  CPP_RESERVE(pfile, 1 + len);
		  CPP_PUTC_Q(pfile, c);
		  CPP_PUTS_Q(pfile, start, len);
		  pfile->lineno += newlines;
		  parse_clear_mark(&start_mark);
		  return CPP_COMMENT;
	       }
	     else if (newlines > 0)
	       {
		 output_line_command(pfile, 0, same_file);
		 CPP_RESERVE(pfile, 1);
		 CPP_PUTC_Q(pfile, ' ');
		 return CPP_VSPACE;
	       }
	     else
	       {
		  CPP_RESERVE(pfile, 1);
		  CPP_PUTC_Q(pfile, ' ');
		  return CPP_HSPACE;
	       }

	  case '#':
	     if (!pfile->only_seen_white)
		goto randomchar;
	     if (handle_directive(pfile))
		return CPP_DIRECTIVE;
	     pfile->only_seen_white = 0;
	     return CPP_OTHER;

	  case '\"':
	  case '\'':
	     /* A single quoted string is treated like a double -- some
	      * programs (e.g., troff) are perverse this way */
	     cpp_buf_line_and_col(cpp_file_buffer(pfile),
				  &start_line, &start_column);
	     old_written = CPP_WRITTEN(pfile);
	   string:
	     CPP_PUTC(pfile, c);
	     while (1)
	       {
		  int                 cc = GETC();

		  if (cc == EOF)
		    {
		       if (CPP_IS_MACRO_BUFFER(CPP_BUFFER(pfile)))
			 {
			    /* try harder: this string crosses a macro expansion
			     * boundary.  This can happen naturally if -traditional.
			     * Otherwise, only -D can make a macro with an unmatched
			     * quote.  */
			    cpp_buffer         *next_buf
			       = CPP_PREV_BUFFER(CPP_BUFFER(pfile));

			    (*CPP_BUFFER(pfile)->cleanup)
			       (CPP_BUFFER(pfile), pfile);
			    CPP_BUFFER(pfile) = next_buf;
			    continue;
			 }
                       cpp_error_with_line(pfile, start_line, start_column,
                                           "unterminated string or character constant");
                       if (pfile->multiline_string_line != start_line
                           && pfile->multiline_string_line != 0)
                          cpp_error_with_line(pfile,
                                              pfile->multiline_string_line,
                                              -1,
                                              "possible real start of unterminated constant");
                       pfile->multiline_string_line = 0;
		       break;
		    }
		  CPP_PUTC(pfile, cc);
		  switch (cc)
		    {
		    case '\n':
		       /* Traditionally, end of line ends a string constant with
		        * no error.  So exit the loop and record the new line.  */
		       if (c == '\'')
			 {
			    cpp_error_with_line(pfile, start_line, start_column,
						"unterminated character constant");
			    goto while2end;
			 }
		       if (CPP_PEDANTIC(pfile)
			   && pfile->multiline_string_line == 0)
			 {
			    cpp_pedwarn_with_line(pfile, start_line,
						  start_column,
						  "string constant runs past end of line");
			 }
		       if (pfile->multiline_string_line == 0)
			  pfile->multiline_string_line = start_line;
		       break;

		    case '\\':
		       cc = GETC();
		       if (cc == '\n')
			 {
			    /* Backslash newline is replaced by nothing at all. */
			    CPP_ADJUST_WRITTEN(pfile, -1);
			    pfile->lineno++;
			 }
		       else
			 {
			    /* ANSI stupidly requires that in \\ the second \
			     * is *not* prevented from combining with a newline.  */
			    NEWLINE_FIX1(cc);
			    if (cc != EOF)
			       CPP_PUTC(pfile, cc);
			 }
		       break;

		    case '\"':
		    case '\'':
		       if (cc == c)
			  goto while2end;
		       break;
		    }
	       }
	   while2end:
	     pfile->lineno += count_newlines(pfile->token_buffer + old_written,
					     CPP_PWRITTEN(pfile));
	     pfile->only_seen_white = 0;
	     return c == '\'' ? CPP_CHAR : CPP_STRING;

	  case '$':
	     if (!opts->dollars_in_ident)
		goto randomchar;
	     goto letter;

	  case ':':
	     if (opts->cplusplus && PEEKC() == ':')
		goto op2;
	     goto randomchar;

	  case '&':
	  case '+':
	  case '|':
	     NEWLINE_FIX;
	     c2 = PEEKC();
	     if (c2 == c || c2 == '=')
		goto op2;
	     goto randomchar;

	  case '*':
	  case '!':
	  case '%':
	  case '=':
	  case '^':
	     NEWLINE_FIX;
	     if (PEEKC() == '=')
		goto op2;
	     goto randomchar;

	  case '-':
	     NEWLINE_FIX;
	     c2 = PEEKC();
	     if (c2 == '-' && opts->chill)
	       {
		  /* Chill style comment */
		  if (opts->put_out_comments)
		     parse_set_mark(&start_mark, pfile);
		  FORWARD(1);	/* Skip second '-'. */
		  for (;;)
		    {
		       c = GETC();
		       if (c == EOF)
			  break;
		       if (c == '\n')
			 {
			    /* Don't consider final '\n' to be part of comment. */
			    FORWARD(-1);
			    break;
			 }
		    }
		  c = '-';
		  goto return_comment;
	       }
	     if (c2 == '-' || c2 == '=' || c2 == '>')
		goto op2;
	     goto randomchar;

	  case '<':
	     if (pfile->parsing_include_directive)
	       {
		  for (;;)
		    {
		       CPP_PUTC(pfile, c);
		       if (c == '>')
			  break;
		       c = GETC();
		       NEWLINE_FIX1(c);
		       if (c == '\n' || c == EOF)
			 {
			    cpp_error(pfile,
				      "missing '>' in `#include <FILENAME>'");
			    break;
			 }
		    }
		  return CPP_STRING;
	       }
	     /* else fall through */
	  case '>':
	     NEWLINE_FIX;
	     c2 = PEEKC();
	     if (c2 == '=')
		goto op2;
	     if (c2 != c)
		goto randomchar;
	     FORWARD(1);
	     CPP_RESERVE(pfile, 4);
	     CPP_PUTC(pfile, c);
	     CPP_PUTC(pfile, c2);
	     NEWLINE_FIX;
	     c3 = PEEKC();
	     if (c3 == '=')
		CPP_PUTC_Q(pfile, GETC());
	     CPP_NUL_TERMINATE_Q(pfile);
	     pfile->only_seen_white = 0;
	     return CPP_OTHER;

	  case '@':
	     if (CPP_BUFFER(pfile)->has_escapes)
	       {
		  c = GETC();
                  // fix macro expansions starting with - losing the -
 		  if (c == '-')
		    {
                       CPP_PUTS(pfile, "-", 1);
		       return CPP_OTHER;
		    }
                  // fix macro expansions starting with - losing the +
		  else if (c == '+')
		    {
                       CPP_PUTS(pfile, "+", 1);
		       return CPP_OTHER;
		    }
                  // fix macro expansions starting with - losing the .
		  else if (c == '.')
		    {
                       CPP_PUTS(pfile, ".", 1);
		       return CPP_OTHER;
		    }
		  else if (is_space[c])
		    {
		       CPP_RESERVE(pfile, 1);
		       if (pfile->output_escapes)
			  CPP_PUTC_Q(pfile, '@');
		       return CPP_HSPACE;
		    }
	       }
	     if (pfile->output_escapes)
	       {
		  CPP_PUTS(pfile, "@@", 2);
		  return CPP_OTHER;
	       }
	     goto randomchar;

	  case '.':
	     NEWLINE_FIX;
	     c2 = PEEKC();
	     if (isdigit(c2))
	       {
		  CPP_RESERVE(pfile, 2);
		  CPP_PUTC_Q(pfile, '.');
		  c = GETC();
		  goto number;
	       }
	     /* FIXME - misses the case "..\\\n." */
	     if (c2 == '.' && PEEKN(1) == '.')
	       {
		  CPP_RESERVE(pfile, 4);
		  CPP_PUTC_Q(pfile, '.');
		  CPP_PUTC_Q(pfile, '.');
		  CPP_PUTC_Q(pfile, '.');
		  FORWARD(2);
		  CPP_NUL_TERMINATE_Q(pfile);
		  pfile->only_seen_white = 0;
		  return CPP_3DOTS;
	       }
	     goto randomchar;

	   op2:
	     token = CPP_OTHER;
	     pfile->only_seen_white = 0;
	   op2any:
	     CPP_RESERVE(pfile, 3);
	     CPP_PUTC_Q(pfile, c);
	     CPP_PUTC_Q(pfile, GETC());
	     CPP_NUL_TERMINATE_Q(pfile);
	     return token;

	  case 'L':
	     NEWLINE_FIX;
	     c2 = PEEKC();
	     if ((c2 == '\'' || c2 == '\"'))
	       {
		  CPP_PUTC(pfile, c);
		  c = GETC();
		  goto string;
	       }
	     goto letter;

	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
	   number:
	     c2 = '.';
	     for (;;)
	       {
		  CPP_RESERVE(pfile, 2);
		  CPP_PUTC_Q(pfile, c);
		  NEWLINE_FIX;
		  c = PEEKC();
		  if (c == EOF)
		     break;
		  if (!is_idchar[c] && c != '.'
		      && ((c2 != 'e' && c2 != 'E') || (c != '+' && c != '-')))
		     break;
		  FORWARD(1);
		  c2 = c;
	       }
	     CPP_NUL_TERMINATE_Q(pfile);
	     pfile->only_seen_white = 0;
	     return CPP_NUMBER;
	  case 'b':
	  case 'c':
	  case 'd':
	  case 'h':
	  case 'o':
	  case 'B':
	  case 'C':
	  case 'D':
	  case 'H':
	  case 'O':
	     if (opts->chill && PEEKC() == '\'')
	       {
		  pfile->only_seen_white = 0;
		  CPP_RESERVE(pfile, 2);
		  CPP_PUTC_Q(pfile, c);
		  CPP_PUTC_Q(pfile, '\'');
		  FORWARD(1);
		  for (;;)
		    {
		       c = GETC();
		       if (c == EOF)
			  goto chill_number_eof;
		       if (!is_idchar[c])
			 {
			    if (c == '\\' && PEEKC() == '\n')
			      {
				 FORWARD(2);
				 continue;
			      }
			    break;
			 }
		       CPP_PUTC(pfile, c);
		    }
		  if (c == '\'')
		    {
		       CPP_RESERVE(pfile, 2);
		       CPP_PUTC_Q(pfile, c);
		       CPP_NUL_TERMINATE_Q(pfile);
		       return CPP_STRING;
		    }
		  else
		    {
		       FORWARD(-1);
		     chill_number_eof:
		       CPP_NUL_TERMINATE(pfile);
		       return CPP_NUMBER;
		    }
	       }
	     else
		goto letter;
	  case '_':
	  case 'a':
	  case 'e':
	  case 'f':
	  case 'g':
	  case 'i':
	  case 'j':
	  case 'k':
	  case 'l':
	  case 'm':
	  case 'n':
	  case 'p':
	  case 'q':
	  case 'r':
	  case 's':
	  case 't':
	  case 'u':
	  case 'v':
	  case 'w':
	  case 'x':
	  case 'y':
	  case 'z':
	  case 'A':
	  case 'E':
	  case 'F':
	  case 'G':
	  case 'I':
	  case 'J':
	  case 'K':
	  case 'M':
	  case 'N':
	  case 'P':
	  case 'Q':
	  case 'R':
	  case 'S':
	  case 'T':
	  case 'U':
	  case 'V':
	  case 'W':
	  case 'X':
	  case 'Y':
	  case 'Z':
	   letter:
	     {
		HASHNODE           *hp;
		char               *ident;
		int                 before_name_written = CPP_WRITTEN(pfile);
		int                 ident_len;

		parse_name(pfile, c);
		pfile->only_seen_white = 0;
		if (pfile->no_macro_expand)
		   return CPP_NAME;
		ident = (char *)pfile->token_buffer + before_name_written;
		ident_len = CPP_PWRITTEN(pfile) - (unsigned char *)ident;
		hp = cpp_lookup(ident, ident_len, -1);
		if (!hp)
		   return CPP_NAME;
		if (hp->type == T_DISABLED)
		  {
		     if (pfile->output_escapes)
		       {	/* Return "@-IDENT", followed by '\0'. */
			  int                 i;

			  CPP_RESERVE(pfile, 3);
			  ident =
			     (char *)pfile->token_buffer + before_name_written;
			  CPP_ADJUST_WRITTEN(pfile, 2);
			  for (i = ident_len; i >= 0; i--)
			     ident[i + 2] = ident[i];
			  ident[0] = '@';
			  ident[1] = '-';
		       }
		     return CPP_NAME;
		  }
		/* If macro wants an arglist, verify that a '(' follows.
		 * first skip all whitespace, copying it to the output
		 * after the macro name.  Then, if there is no '(',
		 * decide this is not a macro call and leave things that way.  */
		if (hp->type == T_MACRO && hp->value.defn->nargs >= 0)
		  {
		     struct parse_marker macro_mark;
		     int                 is_macro_call;

		     while (CPP_IS_MACRO_BUFFER(CPP_BUFFER(pfile)))
		       {
			  cpp_buffer         *next_buf;

			  cpp_skip_hspace(pfile);
			  if (PEEKC() != EOF)
			     break;
			  next_buf = CPP_PREV_BUFFER(CPP_BUFFER(pfile));
			  (*CPP_BUFFER(pfile)->cleanup) (CPP_BUFFER(pfile),
							 pfile);
			  CPP_BUFFER(pfile) = next_buf;
		       }
		     parse_set_mark(&macro_mark, pfile);
		     for (;;)
		       {
			  cpp_skip_hspace(pfile);
			  c = PEEKC();
			  is_macro_call = c == '(';
			  if (c != '\n')
			     break;
			  FORWARD(1);
		       }
		     if (!is_macro_call)
			parse_goto_mark(&macro_mark, pfile);
		     parse_clear_mark(&macro_mark);
		     if (!is_macro_call)
			return CPP_NAME;
		  }
		/* This is now known to be a macro call. */

		/* it might not actually be a macro.  */
		if (hp->type != T_MACRO)
		  {
		     int                 xbuf_len;
		     unsigned char      *xbuf;

		     CPP_SET_WRITTEN(pfile, before_name_written);
		     special_symbol(hp, pfile);
		     xbuf_len = CPP_WRITTEN(pfile) - before_name_written;
		     xbuf = (unsigned char *)xmalloc(xbuf_len + 1);
		     CPP_SET_WRITTEN(pfile, before_name_written);
		     memcpy(xbuf, CPP_PWRITTEN(pfile), xbuf_len + 1);
		     push_macro_expansion(pfile, xbuf, xbuf_len, hp);
		  }
		else
		  {
		     /* Expand the macro, reading arguments as needed,
		      * and push the expansion on the input stack.  */
		     macroexpand(pfile, hp);
		     CPP_SET_WRITTEN(pfile, before_name_written);
		  }

		/* An extra "@ " is added to the end of a macro expansion
		 * to prevent accidental token pasting.  We prefer to avoid
		 * unneeded extra spaces (for the sake of cpp-using tools like
		 * imake).  Here we remove the space if it is safe to do so. */
		if (pfile->buffer->rlimit - pfile->buffer->cur >= 3
		    && pfile->buffer->rlimit[-2] == '@'
		    && pfile->buffer->rlimit[-1] == ' ')
		  {
		     int                 c1 = pfile->buffer->rlimit[-3];

		     c2 = CPP_BUF_PEEK(CPP_PREV_BUFFER(CPP_BUFFER(pfile)));

		     if (c2 == EOF || !unsafe_chars(c1, c2))
			pfile->buffer->rlimit -= 2;
		  }
	     }
	     goto get_next;

	  case ' ':
	  case '\t':
	  case '\v':
	  case '\r':
	     for (;;)
	       {
		  CPP_PUTC(pfile, c);
		  c = PEEKC();
		  if (c == EOF || !is_hor_space[c])
		     break;
		  FORWARD(1);
	       }
	     return CPP_HSPACE;

	  case '\\':
	     c2 = PEEKC();
	     if (c2 != '\n')
		goto randomchar;
	     token = CPP_HSPACE;
	     goto op2any;

	  case '\n':
	     CPP_PUTC(pfile, c);
	     if (pfile->only_seen_white == 0)
		pfile->only_seen_white = 1;
	     pfile->lineno++;
	     output_line_command(pfile, 1, same_file);
	     return CPP_VSPACE;

	  case '(':
	     token = CPP_LPAREN;
	     goto char1;
	  case ')':
	     token = CPP_RPAREN;
	     goto char1;
	  case '{':
	     token = CPP_LBRACE;
	     goto char1;
	  case '}':
	     token = CPP_RBRACE;
	     goto char1;
	  case ',':
	     token = CPP_COMMA;
	     goto char1;
	  case ';':
	     token = CPP_SEMICOLON;
	     goto char1;

	   randomchar:
	  default:
	     token = CPP_OTHER;
	   char1:
	     pfile->only_seen_white = 0;
	     CPP_PUTC(pfile, c);
	     return token;
	  }
     }
}

#if 0				/* Unused */
/* Like cpp_get_token, but skip spaces and comments. */
enum cpp_token
cpp_get_non_space_token(cpp_reader * pfile)
{
   int                 old_written = CPP_WRITTEN(pfile);

   for (;;)
     {
	enum cpp_token      token = cpp_get_token(pfile);

	if (token != CPP_COMMENT && token != CPP_POP
	    && token != CPP_HSPACE && token != CPP_VSPACE)
	   return token;
	CPP_SET_WRITTEN(pfile, old_written);
     }
}
#endif

/* Parse an identifier starting with C. */

int
parse_name(cpp_reader * pfile, int c)
{
   for (;;)
     {
	if (!is_idchar[c])
	  {
	     if (c == '\\' && PEEKC() == '\n')
	       {
		  FORWARD(2);
		  continue;
	       }
	     FORWARD(-1);
	     break;
	  }
	CPP_RESERVE(pfile, 2);	/* One more for final NUL. */
	CPP_PUTC_Q(pfile, c);
	c = GETC();
	if (c == EOF)
	   break;
     }
   CPP_NUL_TERMINATE_Q(pfile);
   return 1;
}

/* Maintain and search list of included files, for #import.  */

/* Hash a file name for import_hash_table.  */

static int
import_hash(char *f)
{
   int                 val = 0;

   while (*f)
      val += *f++;
   return (val % IMPORT_HASH_SIZE);
}

/* Search for file FILENAME in import_hash_table.
 * Return -2 if found, either a matching name or a matching inode.
 * Otherwise, open the file and return a file descriptor if successful
 * or -1 if unsuccessful.  */

static int
lookup_import(cpp_reader * pfile, char *filename, file_name_list * searchptr)
{
   struct import_file *i;
   int                 h;
   int                 hashval;
   struct stat         sb;
   int                 fd;

   hashval = import_hash(filename);

   /* Attempt to find file in list of already included files */
   i = pfile->import_hash_table[hashval];

   while (i)
     {
	if (!strcmp(filename, i->name))
	   return -2;		/* return found */
	i = i->next;
     }
   /* Open it and try a match on inode/dev */
   fd = open_include_file(pfile, filename, searchptr);
   if (fd < 0)
      return fd;
   if (fstat(fd, &sb) < 0)
     {
        close(fd);
        return -2;
     }
   for (h = 0; h < IMPORT_HASH_SIZE; h++)
     {
	i = pfile->import_hash_table[h];
	while (i)
	  {
	     /* Compare the inode and the device.
	      * Supposedly on some systems the inode is not a scalar.  */
	     if (!memcmp
		 ((char *)&i->inode, (char *)&sb.st_ino, sizeof(sb.st_ino))
		 && i->dev == sb.st_dev)
	       {
		  close(fd);
		  return -2;	/* return found */
	       }
	     i = i->next;
	  }
     }
   return fd;			/* Not found, return open file */
}

/* Add the file FNAME, open on descriptor FD, to import_hash_table.  */

static void
add_import(cpp_reader * pfile, int fd, char *fname)
{
   struct import_file *i;
   int                 hashval;
   struct stat         sb;

   hashval = import_hash(fname);
   if (fstat(fd, &sb) < 0) return;
   i = (struct import_file *)xmalloc(sizeof(struct import_file));

   i->name = (char *)xmalloc(strlen(fname) + 1);
   strcpy(i->name, fname);
   memcpy((char *)&i->inode, (char *)&sb.st_ino, sizeof(sb.st_ino));
   i->dev = sb.st_dev;
   i->next = pfile->import_hash_table[hashval];
   pfile->import_hash_table[hashval] = i;
}

/* The file_name_map structure holds a mapping of file names for a
 * particular directory.  This mapping is read from the file named
 * FILE_NAME_MAP_FILE in that directory.  Such a file can be used to
 * map filenames on a file system with severe filename restrictions,
 * such as DOS.  The format of the file name map file is just a series
 * of lines with two tokens on each line.  The first token is the name
 * to map, and the second token is the actual name to use.  */

struct file_name_map {
   struct file_name_map *map_next;
   char               *map_from;
   char               *map_to;
};

#if USE_FILE_NAME_MAPS

#define FILE_NAME_MAP_FILE "header.gcc"

/* Read a space delimited string of unlimited length from a stdio
 * file.  */

static char        *
read_filename_string(int ch, FILE * f)
{
   char               *alloc, *set;
   int                 len;

   len = 20;
   set = alloc = (char *)xmalloc(len + 1);
   if (!is_space[ch])
     {
	*set++ = ch;
	while ((ch = getc(f)) != EOF && !is_space[ch])
	  {
	     if (set - alloc == len)
	       {
		  len *= 2;
		  alloc = (char *)xrealloc(alloc, len + 1);
		  set = alloc + len / 2;
	       }
	     *set++ = ch;
	  }
     }
   *set = '\0';
   ungetc(ch, f);
   return alloc;
}

/* This structure holds a linked list of file name maps, one per directory. */
struct file_name_map_list {
   struct file_name_map_list *map_list_next;
   char               *map_list_name;
   struct file_name_map *map_list_map;
};

/* Read the file name map file for DIRNAME.  */

static struct file_name_map *
read_name_map(cpp_reader * pfile, const char *dirname)
{
   struct file_name_map_list *map_list_ptr;
   char               *name;
   FILE               *f;

   for (map_list_ptr = CPP_OPTIONS(pfile)->map_list; map_list_ptr;
	map_list_ptr = map_list_ptr->map_list_next)
      if (!strcmp(map_list_ptr->map_list_name, dirname))
	 return map_list_ptr->map_list_map;

   map_list_ptr =
      ((struct file_name_map_list *)xmalloc(sizeof(struct file_name_map_list)));

   map_list_ptr->map_list_name = savestring(dirname);
   map_list_ptr->map_list_map = NULL;

   name = (char *)alloca(strlen(dirname) + strlen(FILE_NAME_MAP_FILE) + 2);
   strcpy(name, dirname);
   if (*dirname)
      strcat(name, "/");
   strcat(name, FILE_NAME_MAP_FILE);
#ifndef __EMX__
   f = fopen(name, "rb");
#else
   f = fopen(name, "rtb");
#endif
   if (!f)
      map_list_ptr->map_list_map = NULL;
   else
     {
	int                 ch;
	int                 dirlen = strlen(dirname);

	while ((ch = getc(f)) != EOF)
	  {
	     char               *from, *to;
	     struct file_name_map *ptr;

	     if (is_space[ch])
		continue;
	     from = read_filename_string(ch, f);
	     while ((ch = getc(f)) != EOF && is_hor_space[ch]);
	     to = read_filename_string(ch, f);

	     ptr =
		((struct file_name_map *)xmalloc(sizeof(struct file_name_map)));

	     ptr->map_from = from;

	     /* Make the real filename absolute.  */
	     if (*to == '/')
		ptr->map_to = to;
	     else
	       {
		  ptr->map_to = (char *)xmalloc(dirlen + strlen(to) + 2);
		  strcpy(ptr->map_to, dirname);
		  ptr->map_to[dirlen] = '/';
		  strcpy(ptr->map_to + dirlen + 1, to);
		  free(to);
	       }

	     ptr->map_next = map_list_ptr->map_list_map;
	     map_list_ptr->map_list_map = ptr;

	     while ((ch = getc(f)) != '\n')
		if (ch == EOF)
		   break;
	  }
	fclose(f);
     }

   map_list_ptr->map_list_next = CPP_OPTIONS(pfile)->map_list;
   CPP_OPTIONS(pfile)->map_list = map_list_ptr;

   return map_list_ptr->map_list_map;
}

/* Try to open include file FILENAME.  SEARCHPTR is the directory
 * being tried from the include file search path.  This function maps
 * filenames on file systems based on information read by
 * read_name_map.  */

static int
open_include_file(cpp_reader * pfile, char *filename,
		  file_name_list * searchptr)
{
   struct file_name_map *map;
   const char         *from;
   const char         *p, *dir;

   if (searchptr && !searchptr->got_name_map)
     {
	searchptr->name_map = read_name_map(pfile,
					    searchptr->fname
					    ? searchptr->fname : ".");
	searchptr->got_name_map = 1;
     }
   /* First check the mapping for the directory we are using.  */
   if (searchptr && searchptr->name_map)
     {
	from = filename;
	if (searchptr->fname)
	   from += strlen(searchptr->fname) + 1;
	for (map = searchptr->name_map; map; map = map->map_next)
	  {
	     if (!strcmp(map->map_from, from))
	       {
		  /* Found a match.  */
		  return open(map->map_to, O_RDONLY | O_BINARY, 0666);
	       }
	  }
     }
   /* Try to find a mapping file for the particular directory we are
    * looking in.  Thus #include <sys/types.h> will look up sys/types.h
    * in /usr/include/header.gcc and look up types.h in
    * /usr/include/sys/header.gcc.  */
   p = strrchr(filename, '/');
   if (!p)
      p = filename;
   if (searchptr
       && searchptr->fname
       && strlen(searchptr->fname) == (unsigned)(p - filename)
       && !strncmp(searchptr->fname, filename, p - filename))
     {
	/* FILENAME is in SEARCHPTR, which we've already checked.  */
        using_file(filename, 'E');
	return open(filename, O_RDONLY | O_BINARY, 0666);
     }
   if (p == filename)
     {
	dir = ".";
	from = filename;
     }
   else
     {
	char               *s;

	s = (char *)alloca(p - filename + 1);
	memcpy(s, filename, p - filename);
	s[p - filename] = '\0';
	from = p + 1;
	dir = s;
     }
   for (map = read_name_map(pfile, dir); map; map = map->map_next)
      if (!strcmp(map->map_from, from))
        {
           using_file(map->map_to, 'E');
           return open(map->map_to, O_RDONLY | O_BINARY, 0666);
        }

   using_file(filename, 'E');
   return open(filename, O_RDONLY | O_BINARY, 0666);
}

#else

static int
open_include_file(cpp_reader * pfile EINA_UNUSED, char *filename,
		  file_name_list * searchptr EINA_UNUSED)
{
   using_file(filename, 'E');
   return open(filename, O_RDONLY | O_BINARY, 0666);
}

#endif /* USE_FILE_NAME_MAPS */

static int
dos2unix(cpp_buffer *fp, int length)
{
   unsigned char *tbuf;
   int nlen = 0, i;
   
   tbuf = xmalloc(length + 4);
   if (!tbuf) return length;
   for (i = 0; i < length; i++)
     {
        if ((fp->buf[i] == '\r') &&
            (fp->buf[i + 1] == '\n'))
          {
             // skip \r in \r\n
             continue;
          }
        tbuf[nlen] = fp->buf[i];
        nlen++;
     }
   tbuf[nlen] = 0;
   
   free(fp->buf);
   fp->buf = tbuf;
   return nlen;
}

/* Process the contents of include file FNAME, already open on descriptor F,
 * with output to OP.
 * SYSTEM_HEADER_P is 1 if this file resides in any one of the known
 * "system" include directories (as decided by the `is_system_include'
 * function above).
 * DIRPTR is the link in the dir path through which this file was found,
 * or 0 if the file name was absolute or via the current directory.
 * Return 1 on success, 0 on failure.
 * 
 * The caller is responsible for the cpp_push_buffer.  */

static int
finclude(cpp_reader * pfile, int f, const char *fname, int system_header_p,
	 file_name_list * dirptr)
{
   int                 st_mode;
   long                st_size;
   long                i;
   int                 length;
   cpp_buffer         *fp;	/* For input stack frame */

   if (file_size_and_mode(f, &st_mode, &st_size) < 0)
     {
	cpp_perror_with_name(pfile, fname);
	close(f);
	cpp_pop_buffer(pfile);
	return 0;
     }
   fp = CPP_BUFFER(pfile);
   fp->nominal_fname = fp->fname = fname;
   fp->dir = dirptr;
   fp->system_header_p = system_header_p;
   fp->lineno = 1;
   fp->colno = 1;
   fp->cleanup = file_cleanup;

   if (S_ISREG(st_mode))
     {
	fp->buf = (unsigned char *)xmalloc(st_size + 2);
	/* Read the file contents, knowing that st_size is an upper bound
	 * on the number of bytes we can read.  */
	length = safe_read(f, (char *)fp->buf, st_size);
        length = dos2unix(fp, length);
        
	fp->alimit = fp->buf + st_size + 2;
	fp->cur = fp->buf;
	fp->rlimit = fp->buf + length;
	if (length < 0)
	   goto nope;
     }
   else if (S_ISDIR(st_mode))
     {
	cpp_error(pfile, "directory `%s' specified in #include", fname);
	close(f);
	return 0;
     }
   else
     {
	/* Cannot count its file size before reading.
	 * First read the entire file into heap and
	 * copy them into buffer on stack. */

	int                 bsize = 2000;

	st_size = 0;
	fp->buf = (unsigned char *)xmalloc(bsize + 2);

	for (;;)
	  {
	     i = safe_read(f, (char *)(fp->buf + st_size), bsize - st_size);
	     if (i < 0)
		goto nope;	/* error! */
	     st_size += i;
	     if (st_size != bsize)
		break;		/* End of file */
	     bsize *= 2;
	     fp->buf = (unsigned char *)xrealloc(fp->buf, bsize + 2);
	  }
	length = st_size;
        length = dos2unix(fp, length);
   }

   if ((length > 0 && fp->buf[length - 1] != '\n')
       /* Backslash-newline at end is not good enough.  */
       || (length > 1 && fp->buf[length - 2] == '\\'))
     {
	fp->buf[length++] = '\n';
     }
   fp->buf[length] = '\0';
     
   fp->rlimit = fp->buf + length;

   /* Close descriptor now, so nesting does not use lots of descriptors.  */
   close(f);

   /* Must do this before calling trigraph_pcp, so that the correct file name
    * will be printed in warning messages.  */

   pfile->input_stack_listing_current = 0;

   return 1;

 nope:

   cpp_perror_with_name(pfile, fname);
   close(f);
   free(fp->buf);
   return 1;
}

int
push_parse_file(cpp_reader * pfile, const char *fname)
{
   struct cpp_options *opts = CPP_OPTIONS(pfile);
   struct cpp_pending *pend;
   char               *p;
   int                 f;
   cpp_buffer         *fp;
   char               *epath = 0;

   /* The code looks at the defaults through this pointer, rather than through
    * the constant structure above.  This pointer gets changed if an environment
    * variable specifies other defaults.  */
   struct default_include *include_defaults = include_defaults_array;

   /* Add dirs from CPATH after dirs from -I.  */
   /* There seems to be confusion about what CPATH should do,
    * so for the moment it is not documented.  */
   /* Some people say that CPATH should replace the standard include dirs,
    * but that seems pointless: it comes before them, so it overrides them
    * anyway.  */
   p = (char *)getenv("CPATH");
   if (p && !opts->no_standard_includes)
      path_include(pfile, p);

   /* Now that dollars_in_ident is known, initialize is_idchar.  */
   initialize_char_syntax(opts);

   /* Do partial setup of input buffer for the sake of generating
    * early #line directives (when -g is in effect).  */
   fp = cpp_push_buffer(pfile, NULL, 0);
   if (!opts->in_fname)
      opts->in_fname = "";
   fp->nominal_fname = fp->fname = opts->in_fname;
   fp->lineno = 0;

   /* Install __LINE__, etc.  Must follow initialize_char_syntax
    * and option processing.  */
   initialize_builtins(pfile);

   /* Do standard #defines and assertions
    * that identify system and machine type.  */

   if (!opts->inhibit_predefs)
     {
	p = (char *)alloca(strlen(predefs) + 1);

	strcpy(p, predefs);
	while (*p)
	  {
	     char               *q;

	     while (*p == ' ' || *p == '\t')
		p++;
	     /* Handle -D options.  */
	     if (p[0] == '-' && p[1] == 'D')
	       {
		  q = &p[2];
		  while (*p && *p != ' ' && *p != '\t')
		     p++;
		  if (*p != 0)
		     *p++ = 0;
		  if (opts->debug_output)
		     output_line_command(pfile, 0, same_file);
		  cpp_define(pfile, (unsigned char *)q);
		  while (*p == ' ' || *p == '\t')
		     p++;
	       }
	     else if (p[0] == '-' && p[1] == 'A')
	       {
		  /* Handle -A options (assertions).  */
		  char               *assertion;
		  char               *past_name;
		  char               *value;
		  char               *past_value;
		  char               *termination;
		  int                 save_char;

		  assertion = &p[2];
		  past_name = assertion;
		  /* Locate end of name.  */
		  while (*past_name && *past_name != ' '
			 && *past_name != '\t' && *past_name != '(')
		     past_name++;
		  /* Locate `(' at start of value.  */
		  value = past_name;
		  while (*value && (*value == ' ' || *value == '\t'))
		     value++;
		  if (*value++ != '(')
		     abort();
		  while (*value && (*value == ' ' || *value == '\t'))
		     value++;
		  past_value = value;
		  /* Locate end of value.  */
		  while (*past_value && *past_value != ' '
			 && *past_value != '\t' && *past_value != ')')
		     past_value++;
		  termination = past_value;
		  while (*termination
			 && (*termination == ' ' || *termination == '\t'))
		     termination++;
		  if (*termination++ != ')')
		     abort();
		  if (*termination && *termination != ' '
		      && *termination != '\t')
		     abort();
		  /* Temporarily null-terminate the value.  */
		  save_char = *termination;
		  *termination = '\0';
		  /* Install the assertion.  */
		  make_assertion(pfile, "-A", assertion);
		  *termination = (char)save_char;
		  p = termination;
		  while (*p == ' ' || *p == '\t')
		     p++;
	       }
	     else
	       {
		  abort();
	       }
	  }
     }
   /* Now handle the command line options.  */

   /* Do -U's, -D's and -A's in the order they were seen.  */
   /* First reverse the list. */
   opts->pending = nreverse_pending(opts->pending);

   for (pend = opts->pending; pend; pend = pend->next)
     {
	if (pend->cmd && pend->cmd[0] == '-')
	  {
	     switch (pend->cmd[1])
	       {
	       case 'U':
		  if (opts->debug_output)
		     output_line_command(pfile, 0, same_file);
		  do_undef(pfile, NULL, (unsigned char *)pend->arg,
			   (unsigned char *)pend->arg + strlen(pend->arg));
		  break;
	       case 'D':
		  if (opts->debug_output)
		     output_line_command(pfile, 0, same_file);
		  cpp_define(pfile, (unsigned char *)pend->arg);
		  break;
	       case 'A':
		  make_assertion(pfile, "-A", pend->arg);
		  break;
	       }
	  }
     }

   opts->done_initializing = 1;

   {				/* read the appropriate environment variable and if it exists
				 * replace include_defaults with the listed path. */
      switch ((opts->objc << 1) + opts->cplusplus)
	{
	case 0:
	   epath = getenv("C_INCLUDE_PATH");
	   break;
	case 1:
	   epath = getenv("CPLUS_INCLUDE_PATH");
	   break;
	case 2:
	   epath = getenv("OBJC_INCLUDE_PATH");
	   break;
	case 3:
	   epath = getenv("OBJCPLUS_INCLUDE_PATH");
	   break;
	}
      /* If the environment var for this language is set,
       * add to the default list of include directories.  */
      if (epath)
	{
	   char               *nstore = (char *)alloca(strlen(epath) + 2);
	   int                 num_dirs;
	   char               *startp, *endp;

	   for (num_dirs = 1, startp = epath; *startp; startp++)
	      if (*startp == PATH_SEPARATOR)
		 num_dirs++;
	   include_defaults
	      = (struct default_include *)xmalloc((num_dirs
						   *
						   sizeof(struct
							  default_include)) +
						  sizeof
						  (include_defaults_array));

	   startp = endp = epath;
	   num_dirs = 0;
	   while (1)
	     {
		/* Handle cases like c:/usr/lib:d:/gcc/lib */
		if ((*endp == PATH_SEPARATOR) || *endp == 0)
		  {
		     strncpy(nstore, startp, endp - startp);
		     if (endp == startp)
			strcpy(nstore, ".");
		     else
			nstore[endp - startp] = '\0';

		     include_defaults[num_dirs].fname = savestring(nstore);
		     include_defaults[num_dirs].cplusplus = opts->cplusplus;
		     include_defaults[num_dirs].cxx_aware = 1;
		     num_dirs++;
		     if (*endp == '\0')
			break;
		     endp = startp = endp + 1;
		  }
		else
		   endp++;
	     }
	   /* Put the usual defaults back in at the end.  */
	   memcpy((char *)&include_defaults[num_dirs],
		  (char *)include_defaults_array,
		  sizeof(include_defaults_array));
	}
   }

   append_include_chain(pfile, opts->before_system, opts->last_before_system);
   opts->first_system_include = opts->before_system;

   /* Unless -fnostdinc,
    * tack on the standard include file dirs to the specified list */
   if (!opts->no_standard_includes)
     {
	struct default_include *di = include_defaults;
	char               *specd_prefix = opts->include_prefix;
	char               *default_prefix = savestring(GCC_INCLUDE_DIR);
	int                 default_len = 0;

	/* Remove the `include' from /usr/local/lib/gcc.../include.  */
	if (!strcmp(default_prefix + strlen(default_prefix) - 8, "/include"))
	  {
	     default_len = strlen(default_prefix) - 7;
	     default_prefix[default_len] = 0;
	  }
	/* Search "translated" versions of GNU directories.
	 * These have /usr/local/lib/gcc... replaced by specd_prefix.  */
	if (specd_prefix && default_len != 0)
	   for (di = include_defaults; di->fname; di++)
	     {
		/* Some standard dirs are only for C++.  */
		if (!di->cplusplus
		    || (opts->cplusplus
			&& !opts->no_standard_cplusplus_includes))
		  {
		     /* Does this dir start with the prefix?  */
		     if (!strncmp(di->fname, default_prefix, default_len))
		       {
			  /* Yes; change prefix and add to search list.  */
			  file_name_list     *new_
			     =
			     (file_name_list *) xmalloc(sizeof(file_name_list));
			  int                 this_len =
			     strlen(specd_prefix) + strlen(di->fname) -
			     default_len;
			  char               *str =
			     (char *)xmalloc(this_len + 1);

			  strcpy(str, specd_prefix);
			  strcat(str, di->fname + default_len);
			  new_->fname = str;
			  new_->control_macro = 0;
			  new_->c_system_include_path = !di->cxx_aware;
			  new_->got_name_map = 0;
			  append_include_chain(pfile, new_, new_);
			  if (!opts->first_system_include)
			     opts->first_system_include = new_;
		       }
		  }
	     }
	free(default_prefix);
	/* Search ordinary names for GNU include directories.  */
	for (di = include_defaults; di->fname; di++)
	  {
	     /* Some standard dirs are only for C++.  */
	     if (!di->cplusplus
		 || (opts->cplusplus && !opts->no_standard_cplusplus_includes))
	       {
		  file_name_list     *new_
		     = (file_name_list *) xmalloc(sizeof(file_name_list));

		  new_->control_macro = 0;
		  new_->c_system_include_path = !di->cxx_aware;
		  new_->fname = (char *)di->fname;
		  new_->got_name_map = 0;
		  append_include_chain(pfile, new_, new_);
		  if (!opts->first_system_include)
		     opts->first_system_include = new_;
	       }
	  }
     }
   /* Tack the after_include chain at the end of the include chain.  */
   append_include_chain(pfile, opts->after_include, opts->last_after_include);
   if (!opts->first_system_include)
      opts->first_system_include = opts->after_include;

   /* With -v, print the list of dirs to search.  */
   if (opts->verbose)
     {
	file_name_list     *fl;

	fprintf(stderr, "#include \"...\" search starts here:\n");
	for (fl = opts->include; fl; fl = fl->next)
	  {
	     if (fl == opts->first_bracket_include)
		fprintf(stderr, "#include <...> search starts here:\n");
	     fprintf(stderr, " %s\n", fl->fname);
	  }
	fprintf(stderr, "End of search list.\n");
     }
   /* Scan the -imacros files before the main input.
    * Much like #including them, but with no_output set
    * so that only their macro definitions matter.  */

   opts->no_output++;
   pfile->no_record_file++;
   for (pend = opts->pending; pend; pend = pend->next)
     {
	if (pend->cmd && strcmp(pend->cmd, "-imacros") == 0)
	  {
	     int                 fd = open(pend->arg, O_RDONLY | O_BINARY, 0666);

	     if (fd < 0)
	       {
		  cpp_perror_with_name(pfile, pend->arg);
          if (epath) free(include_defaults);
		  return FATAL_EXIT_CODE;
	       }
	     cpp_push_buffer(pfile, NULL, 0);
	     finclude(pfile, fd, pend->arg, 0, NULL);
	     cpp_scan_buffer(pfile);
	  }
     }
   opts->no_output--;
   pfile->no_record_file--;

   /* Copy the entire contents of the main input file into
    * the stacked input buffer previously allocated for it.  */
   if (!fname || *fname == 0)
     {
	fname = "";
	f = 0;
     }
   else if ((f = open(fname, O_RDONLY | O_BINARY, 0666)) < 0)
      cpp_pfatal_with_name(pfile, fname);

   /* -MG doesn't select the form of output and must be specified with one of
    * -M or -MM.  -MG doesn't make sense with -MD or -MMD since they don't
    * inhibit compilation.  */
   if (opts->print_deps_missing_files
       && (opts->print_deps == 0 || !opts->no_output))
      cpp_fatal("-MG must be specified with one of -M or -MM");

   /* Either of two environment variables can specify output of deps.
    * Its value is either "OUTPUT_FILE" or "OUTPUT_FILE DEPS_TARGET",
    * where OUTPUT_FILE is the file to write deps info to
    * and DEPS_TARGET is the target to mention in the deps.  */

   if (opts->print_deps == 0
       && (getenv("SUNPRO_DEPENDENCIES") != 0
	   || getenv("DEPENDENCIES_OUTPUT") != 0))
     {
	char               *spec = getenv("DEPENDENCIES_OUTPUT");
	char               *s;
	char               *output_file;

	if (!spec)
	  {
	     spec = getenv("SUNPRO_DEPENDENCIES");
	     opts->print_deps = 2;
	  }
	else
	   opts->print_deps = 1;

	s = spec;
	/* Find the space before the DEPS_TARGET, if there is one.  */
	/* This should use index.  (mrs) */
	while (*s != 0 && *s != ' ')
	   s++;
	if (*s != 0)
	  {
	     opts->deps_target = s + 1;
	     output_file = (char *)xmalloc(s - spec + 1);
	     memcpy(output_file, spec, s - spec);
	     output_file[s - spec] = 0;
	  }
	else
	  {
	     opts->deps_target = 0;
	     output_file = spec;
	  }

	opts->deps_file = output_file;
	opts->print_deps_append = 1;
     }
   /* For -M, print the expected object file name
    * as the target of this Make-rule.  */
   if (opts->print_deps)
     {
	pfile->deps_allocated_size = 200;
	pfile->deps_buffer = (char *)xmalloc(pfile->deps_allocated_size);
	pfile->deps_buffer[0] = 0;
	pfile->deps_size = 0;
	pfile->deps_column = 0;

	if (opts->deps_target)
	   deps_output(pfile, opts->deps_target, ':');
	else if (*opts->in_fname == 0)
	   deps_output(pfile, "-", ':');
	else
	  {
	     char               *q;
	     int                 len;

	     /* Discard all directory prefixes from filename.  */
	     if ((q = (char *)strrchr(opts->in_fname, '/'))
#ifdef DIR_SEPARATOR
		 && (q = strrchr(opts->in_fname, DIR_SEPARATOR))
#endif
		)
		++q;
	     else
		q = (char *)opts->in_fname;

	     /* Copy remainder to mungable area.  */
	     p = (char *)alloca(strlen(q) + 8);
	     strcpy(p, q);

	     /* Output P, but remove known suffixes.  */
	     len = strlen(p);
	     q = p + len;
	     if (len >= 2 && p[len - 2] == '.' && strchr("cCsSm", p[len - 1]))
		q = p + (len - 2);
	     else if (len >= 3
		      && p[len - 3] == '.'
		      && p[len - 2] == 'c' && p[len - 1] == 'c')
		q = p + (len - 3);
	     else if (len >= 4
		      && p[len - 4] == '.'
		      && p[len - 3] == 'c'
		      && p[len - 2] == 'x' && p[len - 1] == 'x')
		q = p + (len - 4);
	     else if (len >= 4
		      && p[len - 4] == '.'
		      && p[len - 3] == 'c'
		      && p[len - 2] == 'p' && p[len - 1] == 'p')
		q = p + (len - 4);

	     /* Supply our own suffix.  */
	     strcpy(q, ".o");

	     deps_output(pfile, p, ':');
	     deps_output(pfile, opts->in_fname, ' ');
	  }
     }

   /* Scan the -include files before the main input.
    * We push these in reverse order, so that the first one is handled first.  */

   pfile->no_record_file++;
   opts->pending = nreverse_pending(opts->pending);
   for (pend = opts->pending; pend; pend = pend->next)
     {
	if (pend->cmd && strcmp(pend->cmd, "-include") == 0)
	  {
	     int                 fd = open(pend->arg, O_RDONLY | O_BINARY, 0666);

	     if (fd < 0)
	       {
		  cpp_perror_with_name(pfile, pend->arg);
                  if (f) close(f);
          if (epath) free(include_defaults);
		  return FATAL_EXIT_CODE;
	       }
	     cpp_push_buffer(pfile, NULL, 0);
	     finclude(pfile, fd, pend->arg, 0, NULL);
	  }
     }
   pfile->no_record_file--;

   /* Free the pending list. */
   for (pend = opts->pending; pend;)
     {
	struct cpp_pending *next = pend->next;

	free(pend);
	pend = next;
     }
   opts->pending = NULL;

   if (finclude(pfile, f, fname, 0, NULL))
      output_line_command(pfile, 0, same_file);
   if (epath) free(include_defaults);
   return SUCCESS_EXIT_CODE;
}

void
init_parse_file(cpp_reader * pfile)
{
   memset((char *)pfile, 0, sizeof(cpp_reader));
   pfile->get_token = cpp_get_token;

   pfile->token_buffer_size = 200;
   pfile->token_buffer = (unsigned char *)xmalloc(pfile->token_buffer_size);
   CPP_SET_WRITTEN(pfile, 0);

   pfile->system_include_depth = 0;
   pfile->dont_repeat_files = 0;
   pfile->all_include_files = 0;
   pfile->max_include_len = 0;
   pfile->timebuf = NULL;
   pfile->only_seen_white = 1;
   pfile->buffer = CPP_NULL_BUFFER(pfile);
}

static struct cpp_pending *
nreverse_pending(struct cpp_pending *list)
{
   struct cpp_pending *prev = 0, *next, *pend;

   for (pend = list; pend; pend = next)
     {
	next = pend->next;
	pend->next = prev;
	prev = pend;
     }
   return prev;
}

static void
push_pending(cpp_reader * pfile, const char *cmd, const char *arg)
{
   struct cpp_pending *pend
      = (struct cpp_pending *)xmalloc(sizeof(struct cpp_pending));

   pend->cmd = cmd;
   pend->arg = arg;
   pend->next = CPP_OPTIONS(pfile)->pending;
   CPP_OPTIONS(pfile)->pending = pend;
}

/* Handle command-line options in (argc, argv).
 * Can be called multiple times, to handle multiple sets of options.
 * Returns if an unrecognized option is seen.
 * Returns number of handled arguments.  */

int
cpp_handle_options(cpp_reader * pfile, int argc, char **argv)
{
   int                 i;
   struct cpp_options *opts = CPP_OPTIONS(pfile);

   for (i = 0; i < argc; i++)
     {
	if (argv[i][0] != '-')
	  {
	     if (opts->out_fname)
		cpp_fatal("Usage: %s [switches] input output", argv[0]);
	     else if (opts->in_fname)
		opts->out_fname = argv[i];
	     else
		opts->in_fname = argv[i];
	  }
	else
	  {
	     switch (argv[i][1])
	       {

	       case 'i':
		  if (!strcmp(argv[i], "-include")
		      || !strcmp(argv[i], "-imacros"))
		    {
		       if (i + 1 == argc)
			  cpp_fatal("Filename missing after `%s' option",
				    argv[i]);
		       else
			  push_pending(pfile, argv[i], argv[i + 1]), i++;
		    }
		  if (!strcmp(argv[i], "-iprefix"))
		    {
		       if (i + 1 == argc)
			  cpp_fatal("Filename missing after `-iprefix' option");
		       else
			  opts->include_prefix = argv[++i];
		    }
		  if (!strcmp(argv[i], "-ifoutput"))
		    {
		       opts->output_conditionals = 1;
		    }
		  if (!strcmp(argv[i], "-isystem"))
		    {
		       file_name_list     *dirtmp;

		       if (i + 1 == argc)
			  cpp_fatal("Filename missing after `-isystem' option");

		       dirtmp =
			  (file_name_list *) xmalloc(sizeof(file_name_list));

		       dirtmp->next = 0;
		       dirtmp->control_macro = 0;
		       dirtmp->c_system_include_path = 1;
		       dirtmp->fname = (char *)xmalloc(strlen(argv[i + 1]) + 1);
		       strcpy(dirtmp->fname, argv[++i]);
		       dirtmp->got_name_map = 0;

		       if (!opts->before_system)
			  opts->before_system = dirtmp;
		       else
			  opts->last_before_system->next = dirtmp;
		       opts->last_before_system = dirtmp;	/* Tail follows the last one */
		    }
		  /* Add directory to end of path for includes,
		   * with the default prefix at the front of its name.  */
		  if (!strcmp(argv[i], "-iwithprefix"))
		    {
		       file_name_list     *dirtmp;
		       char               *prefix;
             int                 is_prefix_alloc = 0;

		       if (opts->include_prefix)
			  prefix = opts->include_prefix;
		       else
			 {
			    prefix = savestring(GCC_INCLUDE_DIR);
             is_prefix_alloc++;
			    /* Remove the `include' from /usr/local/lib/gcc.../include.  */
			    if (!strcmp
				(prefix + strlen(prefix) - 8, "/include"))
			       prefix[strlen(prefix) - 7] = 0;
			 }

		       dirtmp =
			  (file_name_list *) xmalloc(sizeof(file_name_list));

		       dirtmp->next = 0;	/* New one goes on the end */
		       dirtmp->control_macro = 0;
		       dirtmp->c_system_include_path = 0;
		       if (i + 1 == argc)
			  cpp_fatal
			     ("Directory name missing after `-iwithprefix' option");

		       dirtmp->fname = (char *)xmalloc(strlen(argv[i + 1])
						       + strlen(prefix) + 1);
		       strcpy(dirtmp->fname, prefix);
		       strcat(dirtmp->fname, argv[++i]);
		       dirtmp->got_name_map = 0;

		       if (!opts->after_include)
			  opts->after_include = dirtmp;
		       else
			  opts->last_after_include->next = dirtmp;
		       opts->last_after_include = dirtmp;	/* Tail follows the last one */
           if (is_prefix_alloc) free(prefix);
		    }
		  /* Add directory to main path for includes,
		   * with the default prefix at the front of its name.  */
		  if (!strcmp(argv[i], "-iwithprefixbefore"))
		    {
		       file_name_list     *dirtmp;
		       char               *prefix;
             int                 is_prefix_alloc = 0;

		       if (opts->include_prefix)
			  prefix = opts->include_prefix;
		       else
			 {
			    prefix = savestring(GCC_INCLUDE_DIR);
             is_prefix_alloc++;
			    /* Remove the `include' from /usr/local/lib/gcc.../include.  */
			    if (!strcmp
				(prefix + strlen(prefix) - 8, "/include"))
			       prefix[strlen(prefix) - 7] = 0;
			 }

		       dirtmp =
			  (file_name_list *) xmalloc(sizeof(file_name_list));

		       dirtmp->next = 0;	/* New one goes on the end */
		       dirtmp->control_macro = 0;
		       dirtmp->c_system_include_path = 0;
		       if (i + 1 == argc)
			  cpp_fatal
			     ("Directory name missing after `-iwithprefixbefore' option");

		       dirtmp->fname = (char *)xmalloc(strlen(argv[i + 1])
						       + strlen(prefix) + 1);
		       strcpy(dirtmp->fname, prefix);
		       strcat(dirtmp->fname, argv[++i]);
		       dirtmp->got_name_map = 0;

		       append_include_chain(pfile, dirtmp, dirtmp);
             if (is_prefix_alloc) free(prefix);
		    }
		  /* Add directory to end of path for includes.  */
		  if (!strcmp(argv[i], "-idirafter"))
		    {
		       file_name_list     *dirtmp;

		       dirtmp =
			  (file_name_list *) xmalloc(sizeof(file_name_list));

		       dirtmp->next = 0;	/* New one goes on the end */
		       dirtmp->control_macro = 0;
		       dirtmp->c_system_include_path = 0;
		       if (i + 1 == argc)
			  cpp_fatal
			     ("Directory name missing after `-idirafter' option");
		       else
			  dirtmp->fname = argv[++i];
		       dirtmp->got_name_map = 0;

		       if (!opts->after_include)
			  opts->after_include = dirtmp;
		       else
			  opts->last_after_include->next = dirtmp;
		       opts->last_after_include = dirtmp;	/* Tail follows the last one */
		    }
		  break;

	       case 'o':
		  if (opts->out_fname)
		     cpp_fatal("Output filename specified twice");
		  if (i + 1 == argc)
		     cpp_fatal("Filename missing after -o option");
		  opts->out_fname = argv[++i];
		  if (!strcmp(opts->out_fname, "-"))
		     opts->out_fname = "";
		  break;

	       case 'p':
		  if (!strcmp(argv[i], "-pedantic"))
		     CPP_PEDANTIC(pfile) = 1;
		  else if (!strcmp(argv[i], "-pedantic-errors"))
		    {
		       CPP_PEDANTIC(pfile) = 1;
		       opts->pedantic_errors = 1;
		    }
		  break;

	       case 't':
		  if (!strcmp(argv[i], "-trigraphs"))
		    {
		       if (!opts->chill)
			  opts->no_trigraphs = 0;
		    }
		  break;

	       case 'l':
		  if (!strcmp(argv[i], "-lang-c"))
		     opts->cplusplus = 0, opts->cplusplus_comments =
			0, opts->objc = 0;
		  if (!strcmp(argv[i], "-lang-c++"))
		     opts->cplusplus = 1, opts->cplusplus_comments =
			1, opts->objc = 0;
		  if (!strcmp(argv[i], "-lang-c-c++-comments"))
		     opts->cplusplus = 0, opts->cplusplus_comments =
			1, opts->objc = 0;
		  if (!strcmp(argv[i], "-lang-objc"))
		     opts->objc = 1, opts->cplusplus =
			0, opts->cplusplus_comments = 1;
		  if (!strcmp(argv[i], "-lang-objc++"))
		     opts->objc = 1, opts->cplusplus =
			1, opts->cplusplus_comments = 1;
		  if (!strcmp(argv[i], "-lang-asm"))
		     opts->lang_asm = 1;
		  if (!strcmp(argv[i], "-lint"))
		     opts->for_lint = 1;
		  if (!strcmp(argv[i], "-lang-chill"))
		     opts->objc = 0, opts->cplusplus = 0, opts->chill = 1,
			opts->traditional = 1, opts->no_trigraphs = 1;
		  break;

	       case '+':
		  opts->cplusplus = 1, opts->cplusplus_comments = 1;
		  break;

	       case 'w':
		  opts->inhibit_warnings = 1;
		  break;

	       case 'W':
		  if (!strcmp(argv[i], "-Wtrigraphs"))
		     opts->warn_trigraphs = 1;
		  else if (!strcmp(argv[i], "-Wno-trigraphs"))
		     opts->warn_trigraphs = 0;
		  else if (!strcmp(argv[i], "-Wcomment"))
		     opts->warn_comments = 1;
		  else if (!strcmp(argv[i], "-Wno-comment"))
		     opts->warn_comments = 0;
		  else if (!strcmp(argv[i], "-Wcomments"))
		     opts->warn_comments = 1;
		  else if (!strcmp(argv[i], "-Wno-comments"))
		     opts->warn_comments = 0;
		  else if (!strcmp(argv[i], "-Wtraditional"))
		     opts->warn_stringify = 1;
		  else if (!strcmp(argv[i], "-Wno-traditional"))
		     opts->warn_stringify = 0;
		  else if (!strcmp(argv[i], "-Wimport"))
		     opts->warn_import = 1;
		  else if (!strcmp(argv[i], "-Wno-import"))
		     opts->warn_import = 0;
		  else if (!strcmp(argv[i], "-Werror"))
		     opts->warnings_are_errors = 1;
		  else if (!strcmp(argv[i], "-Wno-error"))
		     opts->warnings_are_errors = 0;
		  else if (!strcmp(argv[i], "-Wall"))
		    {
		       opts->warn_trigraphs = 1;
		       opts->warn_comments = 1;
		    }
		  break;

	       case 'M':
		  /* The style of the choices here is a bit mixed.
		   * The chosen scheme is a hybrid of keeping all options in one string
		   * and specifying each option in a separate argument:
		   * -M|-MM|-MT file|-MD file|-MMD file [-MG].  An alternative is:
		   * -M|-MM|-MT file|-MD file|-MMD file|-MG|-MMG; or more concisely:
		   * -M[M][G][D file][T file].  This is awkward to handle in specs, and is
		   * not as extensible.  */
		  /* ??? -MG must be specified in addition to one of -M or -MM.
		   * This can be relaxed in the future without breaking anything.
		   * The converse isn't true.  */

		  /* -MG isn't valid with -MD or -MMD.  This is checked for later.  */
		  if (!strcmp(argv[i], "-MG"))
		    {
		       opts->print_deps_missing_files = 1;
		       break;
		    }
		  if (!strcmp(argv[i], "-M"))
		     opts->print_deps = 2;
		  else if (!strcmp(argv[i], "-MM"))
		     opts->print_deps = 1;
		  else if (!strcmp(argv[i], "-MD"))
		     opts->print_deps = 2;
		  else if (!strcmp(argv[i], "-MMD"))
		     opts->print_deps = 1;
		  /* For -MD and -MMD options, write deps on file named by next arg.  */
		  if (!strcmp(argv[i], "-MD") || !strcmp(argv[i], "-MMD"))
		    {
		       if (i + 1 == argc)
			  cpp_fatal("Filename missing after %s option",
				    argv[i]);
		       opts->deps_file = argv[++i];
		    }
		  /* For MT option, use file named by next arg as Target-name to write
		   * with the dependency information.  */
		  else if (!strcmp(argv[i], "-MT"))
		    {
		       if (i + 1 == argc)
			  cpp_fatal("Filename missing after %s option",
				    argv[i]);
		       opts->deps_target = argv[++i];
		    }
		  else
		    {
		       /* For -M and -MM, write deps on standard output
		        * and suppress the usual output.  */
		       opts->no_output = 1;
		    }
		  break;

	       case 'd':
		  {
		     char               *p = argv[i] + 2;
		     char                c;

		     while ((c = *p++) != 0)
		       {
			  /* Arg to -d specifies what parts of macros to dump */
			  switch (c)
			    {
			    case 'M':
			       opts->dump_macros = dump_only;
			       opts->no_output = 1;
			       break;
			    case 'N':
			       opts->dump_macros = dump_names;
			       break;
			    case 'D':
			       opts->dump_macros = dump_definitions;
			       break;
			    }
		       }
		  }
		  break;

	       case 'g':
		  if (argv[i][2] == '3')
		     opts->debug_output = 1;
		  break;

	       case 'v':
		  fprintf(stderr, "GNU CPP version %s", version_string);
#ifdef TARGET_VERSION
		  TARGET_VERSION;
#endif
		  fprintf(stderr, "\n");
		  opts->verbose = 1;
		  break;

	       case 'H':
		  opts->print_include_names = 1;
		  break;

	       case 'D':
		  if (argv[i][2] != 0)
		     push_pending(pfile, "-D", argv[i] + 2);
		  else if (i + 1 == argc)
		     cpp_fatal("Macro name missing after -D option");
		  else
		     i++, push_pending(pfile, "-D", argv[i]);
		  break;

	       case 'a':
                  {
                     if (!strcmp(argv[i], "-a"))
                       {
                          if (i + 1 == argc)
                            cpp_fatal("Filename missing after `-a` option");
                          else if (strcmp(argv[++i], "/dev/null"))
                            {
                               opts->watchfile = argv[i];
                            }
                       }
                     else if (!strcmp(argv[i], "-annotate"))
                       {
                          annotate = 1;
                       }
		     break;
                  }

	       case 'A':
		  {
		     char               *p = NULL;

		     if (argv[i][2] != 0)
			p = argv[i] + 2;
		     else if (i + 1 == argc)
			cpp_fatal("Assertion missing after -A option");
		     else
			p = argv[++i];

                     if (!p) break;
		     if (!strcmp(p, "-"))
		       {
			  struct cpp_pending **ptr;

			  /* -A- eliminates all predefined macros and assertions.
			   * Let's include also any that were specified earlier
			   * on the command line.  That way we can get rid of any
			   * that were passed automatically in from GCC.  */

			  opts->inhibit_predefs = 1;
			  for (ptr = &opts->pending; *ptr;)
			    {
			       struct cpp_pending *pend = *ptr;

			       if (pend->cmd && pend->cmd[0] == '-'
				   && (pend->cmd[1] == 'D'
				       || pend->cmd[1] == 'A'))
				 {
				    *ptr = pend->next;
				    free(pend);
				 }
			       else
				  ptr = &pend->next;
			    }
		       }
		     else
		       {
			  push_pending(pfile, "-A", p);
		       }
		  }
		  break;

	       case 'U':	/* JF #undef something */
		  if (argv[i][2] != 0)
		     push_pending(pfile, "-U", argv[i] + 2);
		  else if (i + 1 == argc)
		     cpp_fatal("Macro name missing after -U option");
		  else
		     push_pending(pfile, "-U", argv[i + 1]), i++;
		  break;

	       case 'C':
		  opts->put_out_comments = 1;
		  break;

	       case 'E':	/* -E comes from cc -E; ignore it.  */
		  break;

	       case 'P':
		  opts->no_line_commands = 1;
		  break;

	       case '$':	/* Don't include $ in identifiers.  */
		  opts->dollars_in_ident = 0;
		  break;

	       case 'I':	/* Add directory to path for includes.  */
		  {
		     file_name_list     *dirtmp;

		     if (!CPP_OPTIONS(pfile)->ignore_srcdir
			 && !strcmp(argv[i] + 2, "-"))
		       {
			  CPP_OPTIONS(pfile)->ignore_srcdir = 1;
			  /* Don't use any preceding -I directories for #include <...>.  */
			  CPP_OPTIONS(pfile)->first_bracket_include = 0;
		       }
		     else
		       {
			  dirtmp =
			     (file_name_list *) xmalloc(sizeof(file_name_list));

			  dirtmp->next = 0;	/* New one goes on the end */
			  dirtmp->control_macro = 0;
			  dirtmp->c_system_include_path = 0;
			  if (argv[i][2] != 0)
			     dirtmp->fname = argv[i] + 2;
			  else if (i + 1 == argc)
			     cpp_fatal
				("Directory name missing after -I option");
			  else
			     dirtmp->fname = argv[++i];
			  dirtmp->got_name_map = 0;
			  append_include_chain(pfile, dirtmp, dirtmp);
		       }
		  }
		  break;

	       case 'n':
		  if (!strcmp(argv[i], "-nostdinc"))
		     /* -nostdinc causes no default include directories.
		      * You must specify all include-file directories with -I.  */
		     opts->no_standard_includes = 1;
		  else if (!strcmp(argv[i], "-nostdinc++"))
		     /* -nostdinc++ causes no default C++-specific include directories. */
		     opts->no_standard_cplusplus_includes = 1;
		  break;

	       case 'u':
		  /* Sun compiler passes undocumented switch "-undef".
		   * Let's assume it means to inhibit the predefined symbols.  */
		  opts->inhibit_predefs = 1;
		  break;

	       case '\0':	/* JF handle '-' as file name meaning stdin or stdout */
		  if (!opts->in_fname)
		    {
		       opts->in_fname = "";
		       break;
		    }
		  else if (!opts->out_fname)
		    {
		       opts->out_fname = "";
		       break;
		    }
#ifndef __has_attribute
# define __has_attribute(x) 0
#endif
#if __has_attribute(fallthrough)
                  __attribute__((fallthrough));
#endif
                  /* else fall through into error */
	       default:
		  return i;
	       }
	  }
     }
   return i;
}

void
cpp_finish(cpp_reader * pfile)
{
   struct cpp_options *opts = CPP_OPTIONS(pfile);

   if (opts->print_deps)
     {
	/* Stream on which to print the dependency information.  */
	FILE               *deps_stream;

	/* Don't actually write the deps file if compilation has failed.  */
	if (pfile->errors == 0)
	  {
	     const char         *deps_mode =
		opts->print_deps_append ? "ab" : "wb";

	     if (!opts->deps_file)
		deps_stream = stdout;
	     else if (!(deps_stream = fopen(opts->deps_file, deps_mode)))
		cpp_pfatal_with_name(pfile, opts->deps_file);
	     fputs(pfile->deps_buffer, deps_stream);
	     if (opts->deps_file)
	       {
		  if (ferror(deps_stream) || fclose(deps_stream) != 0)
		     cpp_fatal("I/O error on output");
	       }
	  }
     }
}

static int
do_assert(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
	  unsigned char *buf EINA_UNUSED, unsigned char *limit EINA_UNUSED)
{
   long                symstart;	/* remember where symbol name starts */
   int                 c;
   int                 sym_length;	/* and how long it is */
   struct arglist     *tokens = NULL;

   if (CPP_PEDANTIC(pfile) && CPP_OPTIONS(pfile)->done_initializing
       && !CPP_BUFFER(pfile)->system_header_p)
      cpp_pedwarn(pfile, "ANSI C does not allow `#assert'");

   cpp_skip_hspace(pfile);
   symstart = CPP_WRITTEN(pfile);	/* remember where it starts */
   parse_name(pfile, GETC());
   sym_length = check_macro_name(pfile, pfile->token_buffer + symstart,
				 "assertion");

   cpp_skip_hspace(pfile);
   if (PEEKC() != '(')
     {
	cpp_error(pfile, "missing token-sequence in `#assert'");
	goto error;
     }
   {
      int                 error_flag = 0;

      tokens = read_token_list(pfile, &error_flag);
      if (error_flag)
	 goto error;
      if (!tokens)
	{
	   cpp_error(pfile, "empty token-sequence in `#assert'");
	   goto error;
	}
      cpp_skip_hspace(pfile);
      c = PEEKC();
      if (c != EOF && c != '\n')
	 cpp_pedwarn(pfile, "junk at end of `#assert'");
      skip_rest_of_line(pfile);
   }

   /* If this name isn't already an assertion name, make it one.
    * Error if it was already in use in some other way.  */

   {
      ASSERTION_HASHNODE *hp;
      const char         *symname = (char *)pfile->token_buffer + symstart;
      int                 hashcode =
	 hashf(symname, sym_length, ASSERTION_HASHSIZE);
      struct tokenlist_list *value =
	 (struct tokenlist_list *)xmalloc(sizeof(struct tokenlist_list));

      hp = assertion_lookup(pfile, symname, sym_length, hashcode);
      if (!hp)
	{
	   if (sym_length == 7 && !strncmp(symname, "defined", sym_length))
	      cpp_error(pfile, "`defined' redefined as assertion");
	   hp = assertion_install(pfile, symname, sym_length, hashcode);
	}
      /* Add the spec'd token-sequence to the list of such.  */
      value->tokens = tokens;
      value->next = hp->value;
      hp->value = value;
   }
   CPP_SET_WRITTEN(pfile, symstart);	/* Pop */
   return 0;
 error:
   CPP_SET_WRITTEN(pfile, symstart);	/* Pop */
   skip_rest_of_line(pfile);
   return 1;
}

static int
do_unassert(cpp_reader * pfile, struct directive *keyword EINA_UNUSED,
	    unsigned char *buf EINA_UNUSED, unsigned char *limit EINA_UNUSED)
{
   long                symstart;	/* remember where symbol name starts */
   int                 sym_length;	/* and how long it is */
   int                 c;

   struct arglist     *tokens = NULL;
   int                 tokens_specified = 0;

   if (CPP_PEDANTIC(pfile) && CPP_OPTIONS(pfile)->done_initializing
       && !CPP_BUFFER(pfile)->system_header_p)
      cpp_pedwarn(pfile, "ANSI C does not allow `#unassert'");

   cpp_skip_hspace(pfile);

   symstart = CPP_WRITTEN(pfile);	/* remember where it starts */
   parse_name(pfile, GETC());
   sym_length = check_macro_name(pfile, pfile->token_buffer + symstart,
				 "assertion");

   cpp_skip_hspace(pfile);
   if (PEEKC() == '(')
     {
	int                 error_flag = 0;

	tokens = read_token_list(pfile, &error_flag);
	if (error_flag)
	   goto error;
	if (!tokens)
	  {
	     cpp_error(pfile, "empty token list in `#unassert'");
	     goto error;
	  }
	tokens_specified = 1;
     }
   cpp_skip_hspace(pfile);
   c = PEEKC();
   if (c != EOF && c != '\n')
      cpp_error(pfile, "junk at end of `#unassert'");
   skip_rest_of_line(pfile);

   {
      ASSERTION_HASHNODE *hp;
      const char         *symname = (char *)pfile->token_buffer + symstart;
      int                 hashcode =
	 hashf(symname, sym_length, ASSERTION_HASHSIZE);
      struct tokenlist_list *tail, *prev;

      hp = assertion_lookup(pfile, symname, sym_length, hashcode);
      if (!hp)
	 return 1;

      /* If no token list was specified, then eliminate this assertion
       * entirely.  */
      if (!tokens_specified)
	 delete_assertion(hp);
      else
	{
	   /* If a list of tokens was given, then delete any matching list.  */

	   tail = hp->value;
	   prev = 0;
	   while (tail)
	     {
		struct tokenlist_list *next = tail->next;

		if (compare_token_lists(tail->tokens, tokens))
		  {
		     if (prev)
			prev->next = next;
		     else
			hp->value = tail->next;
		     free_token_list(tail->tokens);
		     free(tail);
		  }
		else
		  {
		     prev = tail;
		  }
		tail = next;
	     }
	}
   }

   CPP_SET_WRITTEN(pfile, symstart);	/* Pop */
   return 0;
 error:
   CPP_SET_WRITTEN(pfile, symstart);	/* Pop */
   skip_rest_of_line(pfile);
   return 1;
}

/* Test whether there is an assertion named NAME
 * and optionally whether it has an asserted token list TOKENS.
 * NAME is not null terminated; its length is SYM_LENGTH.
 * If TOKENS_SPECIFIED is 0, then don't check for any token list.  */

static int
check_assertion(cpp_reader * pfile, const char *name, int sym_length,
		int tokens_specified, struct arglist *tokens)
{
   ASSERTION_HASHNODE *hp;
   int                 hashcode = hashf(name, sym_length, ASSERTION_HASHSIZE);

   if (CPP_PEDANTIC(pfile) && !CPP_BUFFER(pfile)->system_header_p)
      cpp_pedwarn(pfile, "ANSI C does not allow testing assertions");

   hp = assertion_lookup(pfile, name, sym_length, hashcode);
   if (!hp)
      /* It is not an assertion; just return false.  */
      return 0;

   /* If no token list was specified, then value is 1.  */
   if (!tokens_specified)
      return 1;

   {
      struct tokenlist_list *tail;

      tail = hp->value;

      /* If a list of tokens was given,
       * then succeed if the assertion records a matching list.  */

      while (tail)
	{
	   if (compare_token_lists(tail->tokens, tokens))
	      return 1;
	   tail = tail->next;
	}

      /* Fail if the assertion has no matching list.  */
      return 0;
   }
}

/* Compare two lists of tokens for equality including order of tokens.  */

static int
compare_token_lists(struct arglist *l1, struct arglist *l2)
{
   while (l1 && l2)
     {
	if (l1->length != l2->length)
	   return 0;
	if (strncmp(l1->name, l2->name, l1->length))
	   return 0;
	l1 = l1->next;
	l2 = l2->next;
     }

   /* Succeed if both lists end at the same time.  */
   return l1 == l2;
}

struct arglist     *
reverse_token_list(struct arglist *tokens)
{
   struct arglist     *prev = 0, *cur, *next;

   for (cur = tokens; cur; cur = next)
     {
	next = cur->next;
	cur->next = prev;
	prev = cur;
     }
   return prev;
}

/* Read a space-separated list of tokens ending in a close parenthesis.
 * Return a list of strings, in the order they were written.
 * (In case of error, return 0 and store -1 in *ERROR_FLAG.) */

static struct arglist *
read_token_list(cpp_reader * pfile, int *error_flag)
{
   struct arglist     *token_ptrs = 0;
   int                 depth = 1;
   int                 length;

   *error_flag = 0;
   FORWARD(1);			/* Skip '(' */

   /* Loop over the assertion value tokens.  */
   while (depth > 0)
     {
	struct arglist     *temp;
	long                name_written = CPP_WRITTEN(pfile);
	int                 c;

	cpp_skip_hspace(pfile);

	c = GETC();

	/* Find the end of the token.  */
	if (c == '(')
	  {
	     CPP_PUTC(pfile, c);
	     depth++;
	  }
	else if (c == ')')
	  {
	     depth--;
	     if (depth == 0)
		break;
	     CPP_PUTC(pfile, c);
	  }
	else if (c == '"' || c == '\'')
	  {
	     FORWARD(-1);
	     cpp_get_token(pfile);
	  }
	else if (c == '\n')
	   break;
	else
	  {
	     while (c != EOF && !is_space[c] && c != '(' && c != ')'
		    && c != '"' && c != '\'')
	       {
		  CPP_PUTC(pfile, c);
		  c = GETC();
	       }
	     if (c != EOF)
		FORWARD(-1);
	  }

	length = CPP_WRITTEN(pfile) - name_written;
	temp = (struct arglist *)xmalloc(sizeof(struct arglist) + length + 1);

	temp->name = (char *)(temp + 1);
	memcpy(temp->name, (char *)(pfile->token_buffer + name_written),
	       length);
	temp->name[length] = 0;
	token_ptrs = temp;
	temp->length = length;

	CPP_ADJUST_WRITTEN(pfile, -length);	/* pop */

	if (c == EOF || c == '\n')
	  {			/* FIXME */
	     cpp_error(pfile,
		       "unterminated token sequence following  `#' operator");
             free(temp);
	     return 0;
	  }
	temp->next = token_ptrs;
     }

   /* We accumulated the names in reverse order.
    * Now reverse them to get the proper order.  */
   return reverse_token_list(token_ptrs);
}

static void
free_token_list(struct arglist *tokens)
{
   while (tokens)
     {
	struct arglist     *next = tokens->next;

	free(tokens->name);
	free(tokens);
	tokens = next;
     }
}

/* Get the file-mode and data size of the file open on FD
 * and store them in *MODE_POINTER and *SIZE_POINTER.  */

static int
file_size_and_mode(int fd, int *mode_pointer, long int *size_pointer)
{
   struct stat         sbuf;

   if (fstat(fd, &sbuf) < 0)
      return (-1);
   if (mode_pointer)
      *mode_pointer = sbuf.st_mode;
   if (size_pointer)
      *size_pointer = sbuf.st_size;
   return 0;
}

/* Read LEN bytes at PTR from descriptor DESC, for file FILENAME,
 * retrying if necessary.  Return a negative value if an error occurs,
 * otherwise return the actual number of bytes read,
 * which must be LEN unless end-of-file was reached.  */

static int
safe_read(int desc, char *ptr, int len)
{
   int                 left = len;

   while (left > 0)
     {
	int                 nchars = read(desc, ptr, left);

	if (nchars < 0)
	  {
#ifdef EINTR
	     if (errno == EINTR)
		continue;
#endif
	     return nchars;
	  }
	if (nchars == 0)
	   break;
	ptr += nchars;
	left -= nchars;
     }
   return len - left;
}

static char        *
savestring(const char *input)
{
   unsigned            size = strlen(input);
   char               *output = (char *)xmalloc(size + 1);

   strcpy(output, input);
   return output;
}

/* Initialize PMARK to remember the current position of PFILE. */
void
parse_set_mark(struct parse_marker *pmark, cpp_reader * pfile)
{
   cpp_buffer         *pbuf = CPP_BUFFER(pfile);

   pmark->next = pbuf->marks;
   pbuf->marks = pmark;
   pmark->buf = pbuf;
   pmark->position = pbuf->cur - pbuf->buf;
}

/* Cleanup PMARK - we no longer need it. */
static void
parse_clear_mark(struct parse_marker *pmark)
{
   struct parse_marker **pp = &pmark->buf->marks;

   for (;; pp = &(*pp)->next)
     {
	if (!*pp)
	   cpp_fatal("internal error", "in parse_set_mark");
	if (*pp == pmark)
	   break;
     }
   *pp = pmark->next;
}

/* Backup the current position of PFILE to that saved in PMARK. */

static void
parse_goto_mark(struct parse_marker *pmark, cpp_reader * pfile)
{
   cpp_buffer         *pbuf = CPP_BUFFER(pfile);

   if (pbuf != pmark->buf)
      cpp_fatal("internal error %s", "parse_goto_mark");
   pbuf->cur = pbuf->buf + pmark->position;
}

/* Reset PMARK to point to the current position of PFILE.  (Same
 * as parse_clear_mark (PMARK), parse_set_mark (PMARK, PFILE) but faster. */

static void
parse_move_mark(struct parse_marker *pmark, cpp_reader * pfile)
{
   cpp_buffer         *pbuf = CPP_BUFFER(pfile);

   if (pbuf != pmark->buf)
      cpp_fatal("internal error %s", "parse_move_mark");
   pmark->position = pbuf->cur - pbuf->buf;
}

int
cpp_read_check_assertion(cpp_reader * pfile)
{
   int                 name_start = CPP_WRITTEN(pfile);
   int                 name_length, name_written;
   int                 result;

   FORWARD(1);			/* Skip '#' */
   cpp_skip_hspace(pfile);
   parse_name(pfile, GETC());
   name_written = CPP_WRITTEN(pfile);
   name_length = name_written - name_start;
   cpp_skip_hspace(pfile);
   if (CPP_BUF_PEEK(CPP_BUFFER(pfile)) == '(')
     {
	int                 error_flag;
	struct arglist     *token_ptrs = read_token_list(pfile, &error_flag);

	result = check_assertion(pfile,
				 (char *)pfile->token_buffer + name_start,
				 name_length, 1, token_ptrs);
     }
   else
      result = check_assertion(pfile,
			       (char *)pfile->token_buffer + name_start,
			       name_length, 0, NULL);
   CPP_ADJUST_WRITTEN(pfile, -name_length);	/* pop */
   return result;
}

static void
cpp_print_file_and_line(cpp_reader * pfile)
{
   cpp_buffer         *ip = cpp_file_buffer(pfile);

   if (ip)
     {
	long                line, col;

	cpp_buf_line_and_col(ip, &line, &col);
	cpp_file_line_for_message(pfile, ip->nominal_fname,
				  line, pfile->show_column ? col : -1);
     }
}

static void
cpp_error_v(cpp_reader * pfile, const char *msg, va_list args)
{
   cpp_print_containing_files(pfile);
   cpp_print_file_and_line(pfile);
   cpp_message_v(pfile, 1, msg, args);
}

void
cpp_error(cpp_reader * pfile, const char *msg, ...)
{
   va_list             args;

   va_start(args, msg);

   cpp_error_v(pfile, msg, args);

   va_end(args);
}

/* Print error message but don't count it.  */

static void
cpp_warning_v(cpp_reader * pfile, const char *msg, va_list args)
{
   if (CPP_OPTIONS(pfile)->inhibit_warnings)
      return;

   if (CPP_OPTIONS(pfile)->warnings_are_errors)
      pfile->errors++;

   cpp_print_containing_files(pfile);
   cpp_print_file_and_line(pfile);
   cpp_message_v(pfile, 0, msg, args);
}

void
cpp_warning(cpp_reader * pfile, const char *msg, ...)
{
   va_list             args;

   va_start(args, msg);

   cpp_warning_v(pfile, msg, args);

   va_end(args);
}

/* Print an error message and maybe count it.  */

void
cpp_pedwarn(cpp_reader * pfile, const char *msg, ...)
{
   va_list             args;

   va_start(args, msg);

   if (CPP_OPTIONS(pfile)->pedantic_errors)
      cpp_error_v(pfile, msg, args);
   else
      cpp_warning_v(pfile, msg, args);

   va_end(args);
}

static void
cpp_error_with_line(cpp_reader * pfile, int line, int column, const char *msg)
{
   cpp_buffer         *ip = cpp_file_buffer(pfile);

   cpp_print_containing_files(pfile);

   if (ip)
      cpp_file_line_for_message(pfile, ip->nominal_fname, line, column);

   cpp_message(pfile, 1, msg, NULL, NULL, NULL);
}

static void
cpp_warning_with_line(cpp_reader * pfile, int line, int column, const char *msg)
{
   cpp_buffer         *ip;

   if (CPP_OPTIONS(pfile)->inhibit_warnings)
      return;

   if (CPP_OPTIONS(pfile)->warnings_are_errors)
      pfile->errors++;

   cpp_print_containing_files(pfile);

   ip = cpp_file_buffer(pfile);

   if (ip)
      cpp_file_line_for_message(pfile, ip->nominal_fname, line, column);

   cpp_message(pfile, 0, msg, NULL, NULL, NULL);
}

static void
cpp_pedwarn_with_line(cpp_reader * pfile, int line, int column, const char *msg)
{
   if (CPP_OPTIONS(pfile)->pedantic_errors)
      cpp_error_with_line(pfile, line, column, msg);
   else
      cpp_warning_with_line(pfile, line, column, msg);
}

/* Report a warning (or an error if pedantic_errors)
 * giving specified file name and line number, not current.  */

void
cpp_pedwarn_with_file_and_line(cpp_reader * pfile,
			       const char *file, int line,
			       const char *msg, const char *arg1,
			       const char *arg2, const char *arg3)
{
   if (!CPP_OPTIONS(pfile)->pedantic_errors
       && CPP_OPTIONS(pfile)->inhibit_warnings)
      return;
   if (file)
      cpp_file_line_for_message(pfile, file, line, -1);
   cpp_message(pfile, CPP_OPTIONS(pfile)->pedantic_errors,
	       msg, arg1, arg2, arg3);
}

/* This defines "errno" properly for VMS, and gives us EACCES. */
#include <errno.h>
#ifndef errno
extern int          errno;

#endif

#ifndef HAVE_STRERROR
extern int          sys_nerr;

#if defined(bsd4_4)
extern const char  *const sys_errlist[];

#else
extern char        *sys_errlist[];

#endif
#endif /* HAVE_STRERROR */

/*
 * my_strerror - return the descriptive text associated with an `errno' code.
 */

static const char  *
my_strerror(int errnum)
{
   const char         *result;

#ifndef HAVE_STRERROR
   result = ((errnum < sys_nerr) ? sys_errlist[errnum] : 0);
#else
   result = strerror(errnum);
#endif

   if (!result)
      result = "undocumented I/O error";

   return result;
}

/* Error including a message from `errno'.  */

static void
cpp_error_from_errno(cpp_reader * pfile, const char *name)
{
   cpp_buffer         *ip = cpp_file_buffer(pfile);

   cpp_print_containing_files(pfile);

   if (ip)
      cpp_file_line_for_message(pfile, ip->nominal_fname, ip->lineno, -1);

   cpp_message(pfile, 1, "%s: %s", name, my_strerror(errno));
}

void
cpp_perror_with_name(cpp_reader * pfile, const char *name)
{
   cpp_message(pfile, 1, "%s: %s: %s", progname, name, my_strerror(errno));
}

/* TODO:
 * No pre-compiled header file support.
 *
 * Possibly different enum token codes for each C/C++ token.
 *
 * Should clean up remaining directives to that do_XXX functions
 *   only take two arguments and all have command_reads_line.
 *
 * Find and cleanup remaining uses of static variables,
 *
 * Support for trigraphs.
 *
 * Support -dM flag (dump_all_macros).
 *
 * Support for_lint flag.
 */

extern cpp_options         options;

void
using_file(const char *filename, const char type)
{
   FILE *f = NULL;

   if (!options.watchfile) return;
   f = fopen(options.watchfile, "ab");
   if (!f) return;
   if (annotate)
     {
        fprintf(f, "%c: %s\n", type, filename);
     }
   else
     {
        fputs(filename, f);
        fputc('\n', f);
     }
   fclose(f);
}
