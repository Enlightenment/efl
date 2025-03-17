/*  Small compiler - File input, preprocessing and lexical analysis functions
 *
 *  Copyright (c) ITB CompuPhase, 1997-2003
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 *
 *  Version: $Id$
 */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "embryo_cc_sc.h"
#include "Embryo.h"

static int          match(char *st, int end);
static cell         litchar(char **lptr, int rawmode);
static int          alpha(char c);

static int          icomment;	/* currently in multiline comment? */
static int          iflevel;	/* nesting level if #if/#else/#endif */
static int          skiplevel;	/* level at which we started skipping */
static int          elsedone;	/* level at which we have seen an #else */
static char         term_expr[] = "";
static int          listline = -1;	/* "current line" for the list file */

/*  pushstk & popstk
 *
 *  Uses a LIFO stack to store information. The stack is used by doinclude(),
 *  doswitch() (to hold the state of "swactive") and some other routines.
 *
 *  Porting note: I made the bold assumption that an integer will not be
 *  larger than a pointer (it may be smaller). That is, the stack element
 *  is typedef'ed as a pointer type, but I also store integers on it. See
 *  SC.H for "stkitem"
 *
 *  Global references: stack,stkidx (private to pushstk() and popstk())
 */
static stkitem      stack[sSTKMAX];
static int          stkidx;
void
pushstk(stkitem val)
{
   if (stkidx >= sSTKMAX)
      error(102, "parser stack");	/* stack overflow (recursive include?) */
   stack[stkidx] = val;
   stkidx += 1;
}

stkitem
popstk(void)
{
   if (stkidx == 0)
      return (stkitem) - 1;	/* stack is empty */
   stkidx -= 1;
   return stack[stkidx];
}

int
plungequalifiedfile(char *name)
{
   static char        *extensions[] = { ".inc", ".sma", ".small" };
   FILE               *fp;
   char               *ext;
   int                 ext_idx;

   ext_idx = 0;
   do
     {
	fp = (FILE *) sc_opensrc(name);
	ext = strchr(name, '\0');	/* save position */
	if (!fp)
	  {
	     /* try to append an extension */
	     strcpy(ext, extensions[ext_idx]);
	     fp = (FILE *) sc_opensrc(name);
	     if (!fp)
		*ext = '\0';	/* on failure, restore filename */
	  }			/* if */
	ext_idx++;
     }
   while ((!fp) &&
          (ext_idx < (int)(sizeof extensions / sizeof extensions[0])));
   if (!fp)
     {
	*ext = '\0';		/* restore filename */
	return FALSE;
     }				/* if */
   pushstk((stkitem) inpf);
   pushstk((stkitem) inpfname);	/* pointer to current file name */
   pushstk((stkitem) curlibrary);
   pushstk((stkitem) iflevel);
   assert(skiplevel == 0);
   pushstk((stkitem) icomment);
   pushstk((stkitem) fcurrent);
   pushstk((stkitem) fline);
   inpfname = strdup(name);	/* set name of include file */
   if (!inpfname)
      error(103);		/* insufficient memory */
   inpf = fp;			/* set input file pointer to include file */
   fnumber++;
   fline = 0;			/* set current line number to 0 */
   fcurrent = fnumber;
   icomment = FALSE;
   setfile(inpfname, fcurrent);
   listline = -1;		/* force a #line directive when changing the file */
   setactivefile(fcurrent);
   return TRUE;
}

int
plungefile(char *name, int try_currentpath, int try_includepaths)
{
   int                 result = FALSE;
   int                 i;
   char               *ptr;

   if (try_currentpath)
      result = plungequalifiedfile(name);

   if (try_includepaths && name[0] != DIRSEP_CHAR)
     {
	for (i = 0; !result && (ptr = get_path(i)); i++)
	  {
	     char                path[PATH_MAX + PATH_MAX + 128];

             snprintf(path, sizeof (path), "%s/%s", ptr, name);
	     path[sizeof path - 1] = '\0';	/* force '\0' termination */
	     result = plungequalifiedfile(path);
	  }			/* while */
     }				/* if */
   return result;
}

static void
check_empty(const char *sptr)
{
   /* verifies that the string contains only whitespace */
   while (*sptr <= ' ' && *sptr != '\0')
      sptr++;
   if (*sptr != '\0')
      error(38);		/* extra characters on line */
}

/*  doinclude
 *
 *  Gets the name of an include file, pushes the old file on the stack and
 *  sets some options. This routine doesn't use lex(), since lex() doesn't
 *  recognize file names (and directories).
 *
 *  Global references: inpf     (altered)
 *                     inpfname (altered)
 *                     fline    (altered)
 *                     lptr     (altered)
 */
static void
doinclude(void)
{
   char                name[PATH_MAX], c;
   int                 i, result;

   while (*lptr <= ' ' && *lptr != 0)	/* skip leading whitespace */
      lptr++;
   if (*lptr == '<' || *lptr == '\"')
     {
	c = (char)((*lptr == '\"') ? '\"' : '>');	/* termination character */
	lptr++;
	while (*lptr <= ' ' && *lptr != 0)	/* skip whitespace after quote */
	   lptr++;
     }
   else
     {
	c = '\0';
     }				/* if */

   i = 0;
   while ((*lptr != c) && (*lptr != '\0') && (i < (int)(sizeof(name) - 1))) /* find the end of the string */
      name[i++] = *lptr++;
   while (i > 0 && name[i - 1] <= ' ')
      i--;			/* strip trailing whitespace */
   assert((i >= 0) && (i < (int)(sizeof(name))));
   name[i] = '\0';		/* zero-terminate the string */

   if (*lptr != c)
     {				/* verify correct string termination */
	error(37);		/* invalid string */
	return;
     }				/* if */
   if (c != '\0')
      check_empty(lptr + 1);	/* verify that the rest of the line is whitespace */

   /* Include files between "..." or without quotes are read from the current
    * directory, or from a list of "include directories". Include files
    * between <...> are only read from the list of include directories.
    */
   result = plungefile(name, (c != '>'), TRUE);
   if (!result)
      error(100, name);		/* cannot read from ... (fatal error) */
}

/*  readline
 *
 *  Reads in a new line from the input file pointed to by "inpf". readline()
 *  concatenates lines that end with a \ with the next line. If no more data
 *  can be read from the file, readline() attempts to pop off the previous file
 *  from the stack. If that fails too, it sets "freading" to 0.
 *
 *  Global references: inpf,fline,inpfname,freading,icomment (altered)
 */
static void
readline(char *line)
{
   int                 i, num, cont;

   if (lptr == term_expr)
      return;
   num = sLINEMAX;
   cont = FALSE;
   do
     {
	if (!inpf || sc_eofsrc(inpf))
	  {
	     if (cont)
		error(49);	/* invalid line continuation */
	     if (inpf && inpf != inpf_org)
		sc_closesrc(inpf);
	     i = (int)(long)popstk();
	     if (i == -1)
	       {		/* All's done; popstk() returns "stack is empty" */
		  freading = FALSE;
		  *line = '\0';
		  /* when there is nothing more to read, the #if/#else stack should
		   * be empty and we should not be in a comment
		   */
		  assert(iflevel >= 0);
		  if (iflevel > 0)
		     error(1, "#endif", "-end of file-");
		  else if (icomment)
		     error(1, "*/", "-end of file-");
		  return;
	       }		/* if */
	     fline = i;
	     fcurrent = (int)(long)popstk();
	     icomment = (int)(long)popstk();
	     assert(skiplevel == 0);	/* skiplevel was not stored on stack, because it should always be zero at this point */
	     iflevel = (int)(long)popstk();
	     curlibrary = (constvalue *) popstk();
	     free(inpfname);	/* return memory allocated for the include file name */
	     inpfname = (char *)popstk();
	     inpf = (FILE *) popstk();
	     setactivefile(fcurrent);
	     listline = -1;	/* force a #line directive when changing the file */
	     elsedone = 0;
	  }			/* if */

	if (!sc_readsrc(inpf, line, num))
	  {
	     *line = '\0';	/* delete line */
	     cont = FALSE;
	  }
	else
	  {
             char *ptr;
	     /* check whether to erase leading spaces */
	     if (cont)
	       {
		  char *sptr = line;

		  while (*sptr == ' ' || *sptr == '\t')
		     sptr++;
		  if (sptr != line)
		     memmove(line, sptr, strlen(sptr) + 1);
	       }		/* if */
	     cont = FALSE;
	     /* check whether a full line was read */
	     if (!strchr(line, '\n') && !sc_eofsrc(inpf))
		error(75);	/* line too long */
	     /* check if the next line must be concatenated to this line */
	     if ((ptr = strchr(line, '\n')) && ptr > line)
	       {
		  assert(*(ptr + 1) == '\0');	/* '\n' should be last in the string */
		  while (ptr > line
			 && (*ptr == '\n' || *ptr == ' ' || *ptr == '\t'))
		     ptr--;	/* skip trailing whitespace */
		  if (*ptr == '\\')
		    {
		       cont = TRUE;
		       /* set '\a' at the position of '\\' to make it possible to check
		        * for a line continuation in a single line comment (error 49)
		        */
		       *ptr++ = '\a';
		       *ptr = '\0';	/* erase '\n' (and any trailing whitespace) */
		    }		/* if */
	       }		/* if */
	     num -= strlen(line);
	     line += strlen(line);
	  }			/* if */
	fline += 1;
     }
   while (num >= 0 && cont);
}

/*  stripcom
 *
 *  Replaces all comments from the line by space characters. It updates
 *  a global variable ("icomment") for multiline comments.
 *
 *  This routine also supports the C++ extension for single line comments.
 *  These comments are started with "//" and end at the end of the line.
 *
 *  Global references: icomment  (private to "stripcom")
 */
static void
stripcom(char *line)
{
   char                c;

   while (*line)
     {
	if (icomment)
	  {
	     if (*line == '*' && *(line + 1) == '/')
	       {
		  icomment = FALSE;	/* comment has ended */
		  *line = ' ';	/* replace '*' and '/' characters by spaces */
		  *(line + 1) = ' ';
		  line += 2;
	       }
	     else
	       {
		  if (*line == '/' && *(line + 1) == '*')
		     error(216);	/* nested comment */
		  *line = ' ';	/* replace comments by spaces */
		  line += 1;
	       }		/* if */
	  }
	else
	  {
	     if (*line == '/' && *(line + 1) == '*')
	       {
		  icomment = TRUE;	/* start comment */
		  *line = ' ';	/* replace '/' and '*' characters by spaces */
		  *(line + 1) = ' ';
		  line += 2;
	       }
	     else if (*line == '/' && *(line + 1) == '/')
	       {		/* comment to end of line */
		  if (strchr(line, '\a'))
		     error(49);	/* invalid line continuation */
		  *line++ = '\n';	/* put "newline" at first slash */
		  *line = '\0';	/* put "zero-terminator" at second slash */
	       }
	     else
	       {
		  if (*line == '\"' || *line == '\'')
		    {		/* leave literals unaltered */
		       c = *line;	/* ending quote, single or double */
		       line += 1;
		       while ((*line != c || *(line - 1) == '\\')
			      && *line != '\0')
			  line += 1;
		       line += 1;	/* skip final quote */
		    }
		  else
		    {
		       line += 1;
		    }		/* if */
	       }		/* if */
	  }			/* if */
     }				/* while */
}

/*  btoi
 *
 *  Attempts to interpret a numeric symbol as a boolean value. On success
 *  it returns the number of characters processed (so the line pointer can be
 *  adjusted) and the value is stored in "val". Otherwise it returns 0 and
 *  "val" is garbage.
 *
 *  A boolean value must start with "0b"
 */
static int
btoi(cell * val, char *curptr)
{
   char               *ptr;

   *val = 0;
   ptr = curptr;
   if (*ptr == '0' && *(ptr + 1) == 'b')
     {
	ptr += 2;
	while (*ptr == '0' || *ptr == '1' || *ptr == '_')
	  {
	     if (*ptr != '_')
		*val = (*val << 1) | (*ptr - '0');
	     ptr++;
	  }			/* while */
     }
   else
     {
	return 0;
     }				/* if */
   if (alphanum(*ptr))		/* number must be delimited by non-alphanumeric char */
      return 0;
   else
      return (int)(ptr - curptr);
}

/*  dtoi
 *
 *  Attempts to interpret a numeric symbol as a decimal value. On success
 *  it returns the number of characters processed and the value is stored in
 *  "val". Otherwise it returns 0 and "val" is garbage.
 */
static int
dtoi(cell * val, char *curptr)
{
   char               *ptr;

   *val = 0;
   ptr = curptr;
   if (!sc_isdigit(*ptr))		/* should start with digit */
      return 0;
   while (sc_isdigit(*ptr) || *ptr == '_')
     {
	if (*ptr != '_')
	   *val = (*val * 10) + (*ptr - '0');
	ptr++;
     }				/* while */
   if (alphanum(*ptr))		/* number must be delimited by non-alphanumerical */
      return 0;
   if (*ptr == '.' && sc_isdigit(*(ptr + 1)))
      return 0;			/* but a fractional part must not be present */
   return (int)(ptr - curptr);
}

/*  htoi
 *
 *  Attempts to interpret a numeric symbol as a hexadecimal value. On
 *  success it returns the number of characters processed and the value is
 *  stored in "val". Otherwise it return 0 and "val" is garbage.
 */
static int
htoi(cell * val, char *curptr)
{
   char               *ptr;

   *val = 0;
   ptr = curptr;
   if (!sc_isdigit(*ptr))		/* should start with digit */
      return 0;
   if (*ptr == '0' && *(ptr + 1) == 'x')
     {				/* C style hexadecimal notation */
	ptr += 2;
	while (sc_isxdigit(*ptr) || *ptr == '_')
	  {
	     if (*ptr != '_')
	       {
		  assert(sc_isxdigit(*ptr));
		  *val = *val << 4;
		  if (sc_isdigit(*ptr))
		     *val += (*ptr - '0');
		  else
		     *val += (tolower(*ptr) - 'a' + 10);
	       }		/* if */
	     ptr++;
	  }			/* while */
     }
   else
     {
	return 0;
     }				/* if */
   if (alphanum(*ptr))
      return 0;
   else
      return (int)(ptr - curptr);
}

#if defined LINUX
static double
pow10(int value)
{
   double              res = 1.0;

   while (value >= 4)
     {
	res *= 10000.0;
	value -= 5;
     }				/* while */
   while (value >= 2)
     {
	res *= 100.0;
	value -= 2;
     }				/* while */
   while (value >= 1)
     {
	res *= 10.0;
	value -= 1;
     }				/* while */
   return res;
}
#endif

/*  ftoi
 *
 *  Attempts to interpret a numeric symbol as a rational number, either as
 *  IEEE 754 single precision floating point or as a fixed point integer.
 *  On success it returns the number of characters processed and the value is
 *  stored in "val". Otherwise it returns 0 and "val" is unchanged.
 *
 *  Small has stricter definition for floating point numbers than most:
 *  o  the value must start with a digit; ".5" is not a valid number, you
 *     should write "0.5"
 *  o  a period must appear in the value, even if an exponent is given; "2e3"
 *     is not a valid number, you should write "2.0e3"
 *  o  at least one digit must follow the period; "6." is not a valid number,
 *     you should write "6.0"
 */
static int
ftoi(cell * val, char *curptr)
{
   char               *ptr;
   double              fnum, ffrac, fmult;
   unsigned long       dnum, dbase;
   int                 i, ignore;

   assert(rational_digits >= 0 && rational_digits < 9);
   for (i = 0, dbase = 1; i < rational_digits; i++)
      dbase *= 10;
   fnum = 0.0;
   dnum = 0L;
   ptr = curptr;
   if (!sc_isdigit(*ptr))		/* should start with digit */
      return 0;
   while (sc_isdigit(*ptr) || *ptr == '_')
     {
	if (*ptr != '_')
	  {
	     fnum = (fnum * 10.0) + (*ptr - '0');
	     dnum = (dnum * 10L) + (*ptr - '0') * dbase;
	  }			/* if */
	ptr++;
     }				/* while */
   if (*ptr != '.')
      return 0;			/* there must be a period */
   ptr++;
   if (!sc_isdigit(*ptr))		/* there must be at least one digit after the dot */
      return 0;
   ffrac = 0.0;
   fmult = 1.0;
   ignore = FALSE;
   while (sc_isdigit(*ptr) || *ptr == '_')
     {
	if (*ptr != '_')
	  {
	     ffrac = (ffrac * 10.0) + (*ptr - '0');
	     fmult = fmult / 10.0;
	     dbase /= 10L;
	     dnum += (*ptr - '0') * dbase;
	     if (dbase == 0L && sc_rationaltag && rational_digits > 0
		 && !ignore)
	       {
		  error(222);	/* number of digits exceeds rational number precision */
		  ignore = TRUE;
	       }		/* if */
	  }			/* if */
	ptr++;
     }				/* while */
   fnum += ffrac * fmult;	/* form the number so far */
   if (*ptr == 'e')
     {				/* optional fractional part */
	int                 exp, sign;

	ptr++;
	if (*ptr == '-')
	  {
	     sign = -1;
	     ptr++;
	  }
	else
	  {
	     sign = 1;
	  }			/* if */
	if (!sc_isdigit(*ptr))	/* 'e' should be followed by a digit */
	   return 0;
	exp = 0;
	while (sc_isdigit(*ptr))
	  {
	     exp = (exp * 10) + (*ptr - '0');
	     ptr++;
	  }			/* while */
#if defined LINUX
	fmult = pow10(exp * sign);
#else
	fmult = pow(10, exp * sign);
#endif
	fnum *= fmult;
	dnum *= (unsigned long)(fmult + 0.5);
     }				/* if */

   /* decide how to store the number */
   if (sc_rationaltag == 0)
     {
	error(70);		/* rational number support was not enabled */
	*val = 0;
     }
   else if (rational_digits == 0)
     {
	float f = (float) fnum;
	/* floating point */
      *val = EMBRYO_FLOAT_TO_CELL(f);
#if !defined NDEBUG
	/* I assume that the C/C++ compiler stores "float" values in IEEE 754
	 * format (as mandated in the ANSI standard). Test this assumption anyway.
	 */
	{
	   float test1 = 0.0, test2 = 50.0;
	   Embryo_Cell c1 = EMBRYO_FLOAT_TO_CELL(test1);
	   Embryo_Cell c2 = EMBRYO_FLOAT_TO_CELL(test2);

	   if (c1 != 0x00000000L)
	     {
		fprintf(stderr,
			"embryo_cc: WARNING! you compiler does not SEEM to interpret IEEE floating\n"
			"point math as embryo expects. this could be bad.\n"
			"\n"
			"(float 0.0 != 0x00000000 bitpattern, 0x%08x instead)\n"
			"\n"
			"this could be an issue with you compiling embryo with gcc 3.2.x that seems\n"
			"to trigger this sanity check. we are not sure why yet, but gcc 3.3.x works\n"
			, c1);
	     }
	  else if (c2 != 0x42480000L)
	     {
		fprintf(stderr,
			"embryo_cc: WARNING! you compiler does not SEEM to interpret IEEE floating\n"
			"point math as embryo expects. This could be bad.\n"
			"\n"
			"(float 50.0 != 0x42480000 bitpattern, 0x%08x instead)\n"
			"\n"
			"this could be an issue with you compiling embryo with gcc 3.2.x that seems\n"
			"to trigger this sanity check. we are not sure why yet, but gcc 3.3.x works\n"
			, c2);
	     }
	}
#endif
     }
   else
     {
	/* fixed point */
	*val = (cell) dnum;
     }				/* if */

   return (int)(ptr - curptr);
}

/*  number
 *
 *  Reads in a number (binary, decimal or hexadecimal). It returns the number
 *  of characters processed or 0 if the symbol couldn't be interpreted as a
 *  number (in this case the argument "val" remains unchanged). This routine
 *  relies on the 'early dropout' implementation of the logical or (||)
 *  operator.
 *
 *  Note: the routine doesn't check for a sign (+ or -). The - is checked
 *        for at "hier2()" (in fact, it is viewed as an operator, not as a
 *        sign) and the + is invalid (as in K&R C, and unlike ANSI C).
 */
static int
number(cell * val, char *curptr)
{
   int                 i;
   cell                curval;

   if ((i = btoi(&curval, curptr)) != 0	/* binary? */
       || (i = htoi(&curval, curptr)) != 0	/* hexadecimal? */
       || (i = dtoi(&curval, curptr)) != 0)	/* decimal? */
     {
	*val = curval;
	return i;
     }
   else
     {
	return 0;		/* else not a number */
     }				/* if */
}

static void
chrcat(char *str, char chr)
{
   str = strchr(str, '\0');
   *str++ = chr;
   *str = '\0';
}

static int
preproc_expr(cell * val, int *tag)
{
   int                 result;
   int                 idx;
   cell                code_index;
   char               *term;

   /* Disable staging; it should be disabled already because
    * expressions may not be cut off half-way between conditional
    * compilations. Reset the staging index, but keep the code
    * index.
    */
   if (stgget(&idx, &code_index))
     {
	error(57);		/* unfinished expression */
	stgdel(0, code_index);
	stgset(FALSE);
     }				/* if */
   /* append a special symbol to the string, so the expression
    * analyzer won't try to read a next line when it encounters
    * an end-of-line
    */
   assert(strlen(pline) < sLINEMAX);
   term = strchr(pline, '\0');
   assert(term != NULL);
   chrcat(pline, PREPROC_TERM);	/* the "DEL" code (see SC.H) */
   result = constexpr(val, tag);	/* get value (or 0 on error) */
   *term = '\0';		/* erase the token (if still present) */
   lexclr(FALSE);		/* clear any "pushed" tokens */
   return result;
}

/* getstring
 * Returns returns a pointer behind the closing quote or to the other
 * character that caused the input to be ended.
 */
static char        *
getstring(char *dest, int max)
{
   assert(dest != NULL);
   *dest = '\0';
   while (*lptr <= ' ' && *lptr != '\0')
      lptr++;			/* skip whitespace */
   if (*lptr != '"')
     {
	error(37);		/* invalid string */
     }
   else
     {
	int                 len = 0;

	lptr++;			/* skip " */
	while (*lptr != '"' && *lptr != '\0')
	  {
	     if (len < max - 1)
		dest[len++] = *lptr;
	     lptr++;
	  }			/* if */
	dest[len] = '\0';
	if (*lptr == '"')
	   lptr++;		/* skip closing " */
	else
	   error(37);		/* invalid string */
     }				/* if */
   return lptr;
}

enum
{
   CMD_NONE,
   CMD_TERM,
   CMD_EMPTYLINE,
   CMD_CONDFALSE,
   CMD_INCLUDE,
   CMD_DEFINE,
   CMD_IF,
   CMD_DIRECTIVE,
};

/*  command
 *
 *  Recognizes the compiler directives. The function returns:
 *     CMD_NONE         the line must be processed
 *     CMD_TERM         a pending expression must be completed before processing further lines
 *     Other value: the line must be skipped, because:
 *     CMD_CONDFALSE    false "#if.." code
 *     CMD_EMPTYLINE    line is empty
 *     CMD_INCLUDE      the line contains a #include directive
 *     CMD_DEFINE       the line contains a #subst directive
 *     CMD_IF           the line contains a #if/#else/#endif directive
 *     CMD_DIRECTIVE    the line contains some other compiler directive
 *
 *  Global variables: iflevel, skiplevel, elsedone (altered)
 *                    lptr      (altered)
 */
static int
command(void)
{
   int                 tok, ret;
   cell                val;
   char               *str;
   int                 idx;
   cell                code_index;

   while (*lptr <= ' ' && *lptr != '\0')
      lptr += 1;
   if (*lptr == '\0')
      return CMD_EMPTYLINE;	/* empty line */
   if (*lptr != '#')
      return skiplevel > 0 ? CMD_CONDFALSE : CMD_NONE;	/* it is not a compiler directive */
   /* compiler directive found */
   indent_nowarn = TRUE;	/* allow loose indentation" */
   lexclr(FALSE);		/* clear any "pushed" tokens */
   /* on a pending expression, force to return a silent ';' token and force to
    * re-read the line
    */
   if (!sc_needsemicolon && stgget(&idx, &code_index))
     {
	lptr = term_expr;
	return CMD_TERM;
     }				/* if */
   tok = lex(&val, &str);
   ret = skiplevel > 0 ? CMD_CONDFALSE : CMD_DIRECTIVE;	/* preset 'ret' to CMD_DIRECTIVE (most common case) */
   switch (tok)
     {
     case tpIF:		/* conditional compilation */
	ret = CMD_IF;
	iflevel += 1;
	if (skiplevel)
	   break;		/* break out of switch */
	preproc_expr(&val, NULL);	/* get value (or 0 on error) */
	if (!val)
	   skiplevel = iflevel;
	check_empty(lptr);
	break;
     case tpELSE:
	ret = CMD_IF;
	if (iflevel == 0 && skiplevel == 0)
	  {
	     error(26);		/* no matching #if */
	     errorset(sRESET);
	  }
	else
	  {
	     if (elsedone == iflevel)
		error(60);	/* multiple #else directives between #if ... #endif */
	     elsedone = iflevel;
	     if (skiplevel == iflevel)
		skiplevel = 0;
	     else if (skiplevel == 0)
		skiplevel = iflevel;
	  }			/* if */
	check_empty(lptr);
	break;
#if 0				/* ??? *really* need to use a stack here */
     case tpELSEIF:
	ret = CMD_IF;
	if (iflevel == 0 && skiplevel == 0)
	  {
	     error(26);		/* no matching #if */
	     errorset(sRESET);
	  }
	else if (elsedone == iflevel)
	  {
	     error(61);		/* #elseif directive may not follow an #else */
	     errorset(sRESET);
	  }
	else
	  {
	     preproc_expr(&val, NULL);	/* get value (or 0 on error) */
	     if (skiplevel == 0)
		skiplevel = iflevel;	/* we weren't skipping, start skipping now */
	     else if (val)
		skiplevel = 0;	/* we were skipping, condition is valid -> stop skipping */
	     /* else: we were skipping and condition is invalid -> keep skipping */
	     check_empty(lptr);
	  }			/* if */
	break;
#endif
     case tpENDIF:
	ret = CMD_IF;
	if (iflevel == 0 && skiplevel == 0)
	  {
	     error(26);
	     errorset(sRESET);
	  }
	else
	  {
	     if (skiplevel == iflevel)
		skiplevel = 0;
	     if (elsedone == iflevel)
		elsedone = 0;	/* ??? actually, should build a stack of #if/#endif and keep
				 * the state whether an #else was seen per nesting level */
	     iflevel -= 1;
	  }			/* if */
	check_empty(lptr);
	break;
     case tINCLUDE:		/* #include directive */
	ret = CMD_INCLUDE;
	if (skiplevel == 0)
	   doinclude();
	break;
     case tpFILE:
	if (skiplevel == 0)
	  {
	     char                pathname[PATH_MAX];

	     lptr = getstring(pathname, sizeof pathname);
	     if (pathname[0] != '\0')
	       {
		  free(inpfname);
		  inpfname = strdup(pathname);
		  if (!inpfname)
		     error(103);	/* insufficient memory */
	       }		/* if */
	  }			/* if */
	check_empty(lptr);
	break;
     case tpLINE:
	if (skiplevel == 0)
	  {
	     if (lex(&val, &str) != tNUMBER)
		error(8);	/* invalid/non-constant expression */
	     fline = (int)val;

	     while (*lptr == ' ')
	        lptr++;			/* skip whitespace */
	     if (*lptr == '"')
               {
		  char pathname[PATH_MAX];

		  lptr = getstring(pathname, sizeof pathname);
		  if (pathname[0] != '\0')
		    {
		       free(inpfname);
		       inpfname = strdup(pathname);
		       if (!inpfname)
		          error(103);	/* insufficient memory */
		    }		/* if */
	       }
	  }			/* if */
	check_empty(lptr);
	break;
     case tpASSERT:
	if (skiplevel == 0 && (sc_debug & sCHKBOUNDS) != 0)
	  {
	     preproc_expr(&val, NULL);	/* get constant expression (or 0 on error) */
	     if (!val)
		error(7);	/* assertion failed */
	     check_empty(lptr);
	  }			/* if */
	break;
     case tpPRAGMA:
	if (skiplevel == 0)
	  {
	     if (lex(&val, &str) == tSYMBOL)
	       {
		  if (strcmp(str, "ctrlchar") == 0)
		    {
		       if (lex(&val, &str) != tNUMBER)
			  error(27);	/* invalid character constant */
		       sc_ctrlchar = (char)val;
		    }
		  else if (strcmp(str, "compress") == 0)
		    {
		       cell                compval;

		       preproc_expr(&compval, NULL);
		       sc_compress = (int)compval;	/* switch code packing on/off */
		    }
		  else if (strcmp(str, "dynamic") == 0)
		    {
		       preproc_expr(&sc_stksize, NULL);
		    }
		  else if (strcmp(str, "library") == 0)
		    {
		       char                name[sNAMEMAX + 1];

		       while (*lptr <= ' ' && *lptr != '\0')
			  lptr++;
		       if (*lptr == '"')
			 {
			    lptr = getstring(name, sizeof name);
			 }
		       else
			 {
			    int                 i;

			    for (i = 0;
                                 (i < (int)(sizeof(name)) - 1) &&
                                 (alphanum(*lptr));
				 i++, lptr++)
			       name[i] = *lptr;
			    name[i] = '\0';
			 }	/* if */
		       if (name[0] == '\0')
			 {
			    curlibrary = NULL;
			 }
		       else
			 {
			    if (strlen(name) > sEXPMAX)
			       error(220, name, sEXPMAX);	/* exported symbol is truncated */
			    /* add the name if it does not yet exist in the table */
			    if (!find_constval(&libname_tab, name, 0))
			       curlibrary =
				  append_constval(&libname_tab, name, 0, 0);
			 }	/* if */
		    }
		  else if (strcmp(str, "pack") == 0)
		    {
		       cell                packval;

		       preproc_expr(&packval, NULL);	/* default = packed/unpacked */
		       sc_packstr = (int)packval;
		    }
		  else if (strcmp(str, "rational") == 0)
		    {
		       char                name[sNAMEMAX + 1];
		       cell                digits = 0;
		       int                 i;

		       /* first gather all information, start with the tag name */
		       while ((*lptr <= ' ') && (*lptr != '\0'))
			  lptr++;
		       for (i = 0;
                            (i < (int)(sizeof(name)) - 1) &&
                            (alphanum(*lptr));
			    i++, lptr++)
			  name[i] = *lptr;
		       name[i] = '\0';
		       /* then the precision (for fixed point arithmetic) */
		       while (*lptr <= ' ' && *lptr != '\0')
			  lptr++;
		       if (*lptr == '(')
			 {
			    preproc_expr(&digits, NULL);
			    if (digits <= 0 || digits > 9)
			      {
				 error(68);	/* invalid rational number precision */
				 digits = 0;
			      }	/* if */
			    if (*lptr == ')')
			       lptr++;
			 }	/* if */
		       /* add the tag (make it public) and check the values */
		       i = sc_addtag(name);
		       exporttag(i);
		       if (sc_rationaltag == 0
			   || (sc_rationaltag == i
			       && rational_digits == (int)digits))
			 {
			    sc_rationaltag = i;
			    rational_digits = (int)digits;
			 }
		       else
			 {
			    error(69);	/* rational number format already set, can only be set once */
			 }	/* if */
		    }
		  else if (strcmp(str, "semicolon") == 0)
		    {
		       cell                semicolval;

		       preproc_expr(&semicolval, NULL);
		       sc_needsemicolon = (int)semicolval;
		    }
		  else if (strcmp(str, "tabsize") == 0)
		    {
		       cell                tabsizeval;

		       preproc_expr(&tabsizeval, NULL);
		       sc_tabsize = (int)tabsizeval;
		    }
		  else if (strcmp(str, "align") == 0)
		    {
		       sc_alignnext = TRUE;
		    }
		  else if (strcmp(str, "unused") == 0)
		    {
		       char                name[sNAMEMAX + 1];
		       int                 i, comma;
		       symbol             *sym;

		       do
			 {
			    /* get the name */
			    while ((*lptr <= ' ') && (*lptr != '\0'))
			       lptr++;
			    for (i = 0;
                                 (i < (int)(sizeof(name)) - 1) &&
                                 (sc_isalpha(*lptr));
				 i++, lptr++)
			       name[i] = *lptr;
			    name[i] = '\0';
			    /* get the symbol */
			    sym = findloc(name);
			    if (!sym)
			       sym = findglb(name);
			    if (sym)
			      {
				 sym->usage |= uREAD;
				 if (sym->ident == iVARIABLE
				     || sym->ident == iREFERENCE
				     || sym->ident == iARRAY
				     || sym->ident == iREFARRAY)
				    sym->usage |= uWRITTEN;
			      }
			    else
			      {
				 error(17, name);	/* undefined symbol */
			      }	/* if */
			    /* see if a comma follows the name */
			    while (*lptr <= ' ' && *lptr != '\0')
			       lptr++;
			    comma = (*lptr == ',');
			    if (comma)
			       lptr++;
			 }
		       while (comma);
		    }
		  else
		    {
		       error(207);	/* unknown #pragma */
		    }		/* if */
	       }
	     else
	       {
		  error(207);	/* unknown #pragma */
	       }		/* if */
	     check_empty(lptr);
	  }			/* if */
	break;
     case tpENDINPUT:
     case tpENDSCRPT:
	if (skiplevel == 0)
	  {
	     check_empty(lptr);
	     assert(inpf != NULL);
	     if (inpf != inpf_org)
		sc_closesrc(inpf);
	     inpf = NULL;
	  }			/* if */
	break;
#if !defined NOEMIT
     case tpEMIT:
	{
	   /* write opcode to output file */
	   char                name[41];
	   int                 i;

	   while (*lptr <= ' ' && *lptr != '\0')
	      lptr++;
	   for (i = 0; i < 40 && (sc_isalpha(*lptr) || *lptr == '.'); i++, lptr++)
	      name[i] = (char)tolower(*lptr);
	   name[i] = '\0';
	   stgwrite("\t");
	   stgwrite(name);
	   stgwrite(" ");
	   code_idx += opcodes(1);
	   /* write parameter (if any) */
	   while (*lptr <= ' ' && *lptr != '\0')
	      lptr++;
	   if (*lptr != '\0')
	     {
		symbol             *sym;

		tok = lex(&val, &str);
		switch (tok)
		  {
		  case tNUMBER:
		  case tRATIONAL:
		     outval(val, FALSE);
		     code_idx += opargs(1);
		     break;
		  case tSYMBOL:
		     sym = findloc(str);
		     if (!sym)
			sym = findglb(str);
		     if (!sym || (sym->ident != iFUNCTN
			 && sym->ident != iREFFUNC
			 && (sym->usage & uDEFINE) == 0))
		       {
			  error(17, str);	/* undefined symbol */
		       }
		     else
		       {
			  outval(sym->addr, FALSE);
			  /* mark symbol as "used", unknown whether for read or write */
			  markusage(sym, uREAD | uWRITTEN);
			  code_idx += opargs(1);
		       }	/* if */
		     break;
		  default:
		     {
			char                s2[20];
			extern char        *sc_tokens[];	/* forward declaration */

                        if (tok < 256)
                          {
                             s2[0] = (char)tok;
                             s2[1] = 0;
                          }
                        else
                          {
                             strncpy(s2, sc_tokens[tok - tFIRST], 19);
                             s2[19] = 0;
                          }
			error(1, sc_tokens[tSYMBOL - tFIRST], s2);
			break;
		     }		/* case */
		  }		/* switch */
	     }			/* if */
	   stgwrite("\n");
	   check_empty(lptr);
	   break;
	}			/* case */
#endif
#if !defined NO_DEFINE
     case tpDEFINE:
	{
	   ret = CMD_DEFINE;
	   if (skiplevel == 0)
	     {
		char               *pattern, *substitution;
		char               *start, *end;
		int                 count, prefixlen;
		stringpair         *def;

		/* find the pattern to match */
		while (*lptr <= ' ' && *lptr != '\0')
		   lptr++;
		start = lptr;	/* save starting point of the match pattern */
		count = 0;
		while (*lptr > ' ' && *lptr != '\0')
		  {
		     litchar(&lptr, FALSE);	/* litchar() advances "lptr" and handles escape characters */
		     count++;
		  }		/* while */
		end = lptr;
		/* check pattern to match */
		if (!sc_isalpha(*start) && *start != '_')
		  {
		     error(74);	/* pattern must start with an alphabetic character */
		     break;
		  }		/* if */
		/* store matched pattern */
		pattern = malloc(count + 1);
		if (!pattern)
		   error(103);	/* insufficient memory */
		lptr = start;
		count = 0;
		while (lptr != end)
		  {
		     assert(lptr < end);
		     assert(*lptr != '\0');
		     pattern[count++] = (char)litchar(&lptr, FALSE);
		  }		/* while */
		pattern[count] = '\0';
		/* special case, erase trailing variable, because it could match anything */
		if (count >= 2 && sc_isdigit(pattern[count - 1])
		    && pattern[count - 2] == '%')
		   pattern[count - 2] = '\0';
		/* find substitution string */
		while (*lptr <= ' ' && *lptr != '\0')
		   lptr++;
		start = lptr;	/* save starting point of the match pattern */
		count = 0;
		end = NULL;
		while (*lptr != '\0')
		  {
		     /* keep position of the start of trailing whitespace */
		     if (*lptr <= ' ')
		       {
			  if (!end)
			     end = lptr;
		       }
		     else
		       {
			  end = NULL;
		       }	/* if */
		     count++;
		     lptr++;
		  }		/* while */
		if (!end)
		   end = lptr;
		/* store matched substitution */
		substitution = malloc(count + 1);	/* +1 for '\0' */
		if (!substitution)
		   error(103);	/* insufficient memory */
		lptr = start;
		count = 0;
		while (lptr != end)
		  {
		     assert(lptr < end);
		     assert(*lptr != '\0');
		     substitution[count++] = *lptr++;
		  }		/* while */
		substitution[count] = '\0';
		/* check whether the definition already exists */
		for (prefixlen = 0, start = pattern;
		     sc_isalpha(*start) || sc_isdigit(*start) || *start == '_';
		     prefixlen++, start++)
		   /* nothing */ ;
		assert(prefixlen > 0);
		if ((def = find_subst(pattern, prefixlen)))
		  {
		     if (strcmp(def->first, pattern) != 0
			 || strcmp(def->second, substitution) != 0)
			error(201, pattern);	/* redefinition of macro (non-identical) */
		     delete_subst(pattern, prefixlen);
		  }		/* if */
		/* add the pattern/substitution pair to the list */
		assert(pattern[0] != '\0');
		insert_subst(pattern, substitution, prefixlen);
		free(pattern);
		free(substitution);
	     }			/* if */
	   break;
	}			/* case */
     case tpUNDEF:
	if (skiplevel == 0)
	  {
	     if (lex(&val, &str) == tSYMBOL)
	       {
		  if (!delete_subst(str, strlen(str)))
		     error(17, str);	/* undefined symbol */
	       }
	     else
	       {
		  error(20, str);	/* invalid symbol name */
	       }		/* if */
	     check_empty(lptr);
	  }			/* if */
	break;
#endif
     default:
	error(31);		/* unknown compiler directive */
	ret = skiplevel > 0 ? CMD_DIRECTIVE : CMD_NONE;	/* line must be processed (if skiplevel==0) */
     }				/* switch */
   return ret;
}

#if !defined NO_DEFINE
static int
is_startstring(char *string)
{
   if (*string == '\"' || *string == '\'')
      return TRUE;		/* "..." */

   if (*string == '!')
     {
	string++;
	if (*string == '\"' || *string == '\'')
	   return TRUE;		/* !"..." */
	if (*string == sc_ctrlchar)
	  {
	     string++;
	     if (*string == '\"' || *string == '\'')
		return TRUE;	/* !\"..." */
	  }			/* if */
     }
   else if (*string == sc_ctrlchar)
     {
	string++;
	if (*string == '\"' || *string == '\'')
	   return TRUE;		/* \"..." */
	if (*string == '!')
	  {
	     string++;
	     if (*string == '\"' || *string == '\'')
		return TRUE;	/* \!"..." */
	  }			/* if */
     }				/* if */

   return FALSE;
}

static char        *
skipstring(char *string)
{
   char                endquote;
   int                 rawstring = FALSE;

   while (*string == '!' || *string == sc_ctrlchar)
     {
	rawstring = (*string == sc_ctrlchar);
	string++;
     }				/* while */

   endquote = *string;
   assert(endquote == '\"' || endquote == '\'');
   string++;			/* skip open quote */
   while (*string != endquote && *string != '\0')
      litchar(&string, rawstring);
   return string;
}

static char        *
skippgroup(char *string)
{
   int                 nest = 0;
   char                open = *string;
   char                close;

   switch (open)
     {
     case '(':
	close = ')';
	break;
     case '{':
	close = '}';
	break;
     case '[':
	close = ']';
	break;
     case '<':
	close = '>';
	break;
     default:
	assert(0);
	close = '\0';		/* only to avoid a compiler warning */
     }				/* switch */

   string++;
   while (*string != close || nest > 0)
     {
	if (*string == open)
	   nest++;
	else if (*string == close)
	   nest--;
	else if (is_startstring(string))
	   string = skipstring(string);
	if (*string == '\0')
	   break;
	string++;
     }				/* while */
   return string;
}

static char        *
strdel(char *str, size_t len)
{
   size_t              length = strlen(str);

   if (len > length)
      len = length;
   memmove(str, str + len, length - len + 1);	/* include EOS byte */
   return str;
}

static char        *
strins(char *dest, char *src, size_t srclen)
{
   size_t              destlen = strlen(dest);

   assert(srclen <= strlen(src));
   memmove(dest + srclen, dest, destlen + 1);	/* include EOS byte */
   memcpy(dest, src, srclen);
   return dest;
}

static int
substpattern(char *line, size_t buffersize, char *pattern, char *substitution)
{
   int                 prefixlen;
   char               *p, *s, *e, *args[10];
   int                 is_match, arg, len;

   memset(args, 0, sizeof args);

   /* check the length of the prefix */
   for (prefixlen = 0, s = pattern; sc_isalpha(*s) || sc_isdigit(*s) || *s == '_';
	prefixlen++, s++)
      /* nothing */ ;
   assert(prefixlen > 0);
   assert(strncmp(line, pattern, prefixlen) == 0);

   /* pattern prefix matches; match the rest of the pattern, gather
    * the parameters
    */
   s = line + prefixlen;
   p = pattern + prefixlen;
   is_match = TRUE;		/* so far, pattern matches */
   while (is_match && *s != '\0' && *p != '\0')
     {
	if (*p == '%')
	  {
	     p++;		/* skip '%' */
	     if (sc_isdigit(*p))
	       {
		  arg = *p - '0';
		  assert(arg >= 0 && arg <= 9);
		  p++;		/* skip parameter id */
		  assert(*p != '\0');
		  /* match the source string up to the character after the digit
		   * (skipping strings in the process
		   */
		  e = s;
		  while (*e != *p && *e != '\0' && *e != '\n')
		    {
		       if (is_startstring(e))	/* skip strings */
			  e = skipstring(e);
		       else if (strchr("({[", *e))	/* skip parenthized groups */
			  e = skippgroup(e);
		       if (*e != '\0')
			  e++;	/* skip non-alphapetic character (or closing quote of
				 * a string, or the closing paranthese of a group) */
		    }		/* while */
		  /* store the parameter (overrule any earlier) */
		  if (args[arg])
		     free(args[arg]);
		  len = (int)(e - s);
		  args[arg] = malloc(len + 1);
		  if (!args[arg])
		     error(103);	/* insufficient memory */
		  strncpy(args[arg], s, len);
		  args[arg][len] = '\0';
		  /* character behind the pattern was matched too */
		  if (*e == *p)
		    {
		       s = e + 1;
		    }
		  else if (*e == '\n' && *p == ';' && *(p + 1) == '\0'
			   && !sc_needsemicolon)
		    {
		       s = e;	/* allow a trailing ; in the pattern match to end of line */
		    }
		  else
		    {
		       assert(*e == '\0' || *e == '\n');
		       is_match = FALSE;
		       s = e;
		    }		/* if */
		  p++;
	       }
	     else
	       {
		  is_match = FALSE;
	       }		/* if */
	  }
	else if (*p == ';' && *(p + 1) == '\0' && !sc_needsemicolon)
	  {
	     /* source may be ';' or end of the line */
	     while (*s <= ' ' && *s != '\0')
		s++;		/* skip white space */
	     if (*s != ';' && *s != '\0')
		is_match = FALSE;
	     p++;		/* skip the semicolon in the pattern */
	  }
	else
	  {
	     cell                ch;

	     /* skip whitespace between two non-alphanumeric characters, except
	      * for two identical symbols
	      */
	     assert(p > pattern);
	     if (!alphanum(*p) && *(p - 1) != *p)
		while (*s <= ' ' && *s != '\0')
		   s++;		/* skip white space */
	     ch = litchar(&p, FALSE);	/* this increments "p" */
	     if (*s != ch)
		is_match = FALSE;
	     else
		s++;		/* this character matches */
	  }			/* if */
     }				/* while */

   if (is_match && *p == '\0')
     {
	/* if the last character to match is an alphanumeric character, the
	 * current character in the source may not be alphanumeric
	 */
	assert(p > pattern);
	if (alphanum(*(p - 1)) && alphanum(*s))
	   is_match = FALSE;
     }				/* if */

   if (is_match)
     {
	/* calculate the length of the substituted string */
	for (e = substitution, len = 0; *e != '\0'; e++)
	  {
	     if (*e == '%' && sc_isdigit(*(e + 1)))
	       {
		  arg = *(e + 1) - '0';
		  assert(arg >= 0 && arg <= 9);
		  if (args[arg])
		     len += strlen(args[arg]);
		  e++;		/* skip %, digit is skipped later */
	       }
	     else
	       {
		  len++;
	       }		/* if */
	  }			/* for */
	/* check length of the string after substitution */
	if (strlen(line) + len - (int)(s - line) > buffersize)
	  {
	     error(75);		/* line too long */
	  }
	else
	  {
	     /* substitute pattern */
	     strdel(line, (int)(s - line));
	     for (e = substitution, s = line; *e != '\0'; e++)
	       {
		  if (*e == '%' && sc_isdigit(*(e + 1)))
		    {
		       arg = *(e + 1) - '0';
		       assert(arg >= 0 && arg <= 9);
		       if (args[arg])
			 {
			    strins(s, args[arg], strlen(args[arg]));
			    s += strlen(args[arg]);
			 }	/* if */
		       e++;	/* skip %, digit is skipped later */
		    }
		  else
		    {
		       strins(s, e, 1);
		       s++;
		    }		/* if */
	       }		/* for */
	  }			/* if */
     }				/* if */

   for (arg = 0; arg < 10; arg++)
      if (args[arg])
	 free(args[arg]);

   return is_match;
}

static void
substallpatterns(char *line, int buffersize)
{
   char               *start, *end;
   int                 prefixlen;
   stringpair         *subst;

   start = line;
   while (*start != '\0')
     {
	/* find the start of a prefix (skip all non-alphabetic characters),
	 * also skip strings
	 */
	while (!sc_isalpha(*start) && *start != '_' && *start != '\0')
	  {
	     /* skip strings */
	     if (is_startstring(start))
	       {
		  start = skipstring(start);
		  if (*start == '\0')
		     break;	/* abort loop on error */
	       }		/* if */
	     start++;		/* skip non-alphapetic character (or closing quote of a string) */
	  }			/* while */
	if (*start == '\0')
	   break;		/* abort loop on error */
	/* get the prefix (length), look for a matching definition */
	prefixlen = 0;
	end = start;
	while (sc_isalpha(*end) || sc_isdigit(*end) || *end == '_')
	  {
	     prefixlen++;
	     end++;
	  }			/* while */
	assert(prefixlen > 0);
	subst = find_subst(start, prefixlen);
	if (subst)
	  {
	     /* properly match the pattern and substitute */
	     if (!substpattern
		 (start, buffersize - (start - line), subst->first,
		  subst->second))
		start = end;	/* match failed, skip this prefix */
	     /* match succeeded: do not update "start", because the substitution text
	      * may be matched by other macros
	      */
	  }
	else
	  {
	     start = end;	/* no macro with this prefix, skip this prefix */
	  }			/* if */
     }				/* while */
}
#endif

/*  preprocess
 *
 *  Reads a line by readline() into "pline" and performs basic preprocessing:
 *  deleting comments, skipping lines with false "#if.." code and recognizing
 *  other compiler directives. There is an indirect recursion: lex() calls
 *  preprocess() if a new line must be read, preprocess() calls command(),
 *  which at his turn calls lex() to identify the token.
 *
 *  Global references: lptr     (altered)
 *                     pline    (altered)
 *                     freading (referred to only)
 */
void
preprocess(void)
{
   int                 iscommand;

   if (!freading)
      return;
   do
     {
	readline(pline);
	stripcom(pline);	/* ??? no need for this when reading back from list file (in the second pass) */
	lptr = pline;		/* set "line pointer" to start of the parsing buffer */
	iscommand = command();
	if (iscommand != CMD_NONE)
	   errorset(sRESET);	/* reset error flag ("panic mode") on empty line or directive */
#if !defined NO_DEFINE
	if (iscommand == CMD_NONE)
	  {
	     assert(lptr != term_expr);
	     substallpatterns(pline, sLINEMAX);
	     lptr = pline;	/* reset "line pointer" to start of the parsing buffer */
	  }			/* if */
#endif
     }
   while (iscommand != CMD_NONE && iscommand != CMD_TERM && freading);	/* enddo */
}

static char        *
unpackedstring(char *sptr, int rawstring)
{
   while (*sptr != '\0')
     {
	/* check for doublequotes indicating the end of the string */
	if (*sptr == '\"')
	{
	   /* check whether there's another pair of quotes following.
	    * If so, paste the two strings together, thus
	    * "pants""off" becomes "pantsoff"
	    */
	   if (*(sptr + 1) == '\"')
	      sptr += 2;
	   else
	      break;
	}

	if (*sptr == '\a')
	  {			/* ignore '\a' (which was inserted at a line concatenation) */
	     sptr++;
	     continue;
	  }			/* if */
	stowlit(litchar(&sptr, rawstring));	/* litchar() alters "lptr" */
     }				/* while */
   stowlit(0);			/* terminate string */
   return sptr;
}

static char        *
packedstring(char *sptr, int rawstring)
{
   int                 i;
   ucell               val, c;

   i = sizeof(ucell) - (charbits / 8);	/* start at most significant byte */
   val = 0;
   while (*sptr != '\0')
     {
	/* check for doublequotes indicating the end of the string */
	if (*sptr == '\"')
	{
	   /* check whether there's another pair of quotes following.
	    * If so, paste the two strings together, thus
	    * "pants""off" becomes "pantsoff"
	    */
	   if (*(sptr + 1) == '\"')
	      sptr += 2;
	   else
	      break;
	}

	if (*sptr == '\a')
	  {			/* ignore '\a' (which was inserted at a line concatenation) */
	     sptr++;
	     continue;
	  }			/* if */
	c = litchar(&sptr, rawstring);	/* litchar() alters "sptr" */
	if (c >= (ucell) (1 << charbits))
	   error(43);		/* character constant exceeds range */
	val |= (c << 8 * i);
	if (i == 0)
	  {
	     stowlit(val);
	     val = 0;
	  }			/* if */
	i = (i + sizeof(ucell) - (charbits / 8)) % sizeof(ucell);
     }				/* if */
   /* save last code; make sure there is at least one terminating zero character */
   if (i != (int)(sizeof(ucell) - (charbits / 8)))
      stowlit(val);		/* at least one zero character in "val" */
   else
      stowlit(0);		/* add full cell of zeros */
   return sptr;
}

/*  lex(lexvalue,lexsym)        Lexical Analysis
 *
 *  lex() first deletes leading white space, then checks for multi-character
 *  operators, keywords (including most compiler directives), numbers,
 *  labels, symbols and literals (literal characters are converted to a number
 *  and are returned as such). If every check fails, the line must contain
 *  a single-character operator. So, lex() returns this character. In the other
 *  case (something did match), lex() returns the number of the token. All
 *  these tokens have been assigned numbers above 255.
 *
 *  Some tokens have "attributes":
 *     tNUMBER        the value of the number is return in "lexvalue".
 *     tRATIONAL      the value is in IEEE 754 encoding or in fixed point
 *                    encoding in "lexvalue".
 *     tSYMBOL        the first sNAMEMAX characters of the symbol are
 *                    stored in a buffer, a pointer to this buffer is
 *                    returned in "lexsym".
 *     tLABEL         the first sNAMEMAX characters of the label are
 *                    stored in a buffer, a pointer to this buffer is
 *                    returned in "lexsym".
 *     tSTRING        the string is stored in the literal pool, the index
 *                    in the literal pool to this string is stored in
 *                    "lexvalue".
 *
 *  lex() stores all information (the token found and possibly its attribute)
 *  in global variables. This allows a token to be examined twice. If "_pushed"
 *  is true, this information is returned.
 *
 *  Global references: lptr          (altered)
 *                     fline         (referred to only)
 *                     litidx        (referred to only)
 *                     _lextok, _lexval, _lexstr
 *                     _pushed
 */

static int          _pushed;
static int          _lextok;
static cell         _lexval;
static char         _lexstr[sLINEMAX + 1];
static int          _lexnewline;

void
lexinit(void)
{
   stkidx = 0;			/* index for pushstk() and popstk() */
   iflevel = 0;			/* preprocessor: nesting of "#if" */
   skiplevel = 0;		/* preprocessor: skipping lines or compiling lines */
   icomment = FALSE;		/* currently not in a multiline comment */
   _pushed = FALSE;		/* no token pushed back into lex */
   _lexnewline = FALSE;
}

char               *sc_tokens[] = {
   "*=", "/=", "%=", "+=", "-=", "<<=", ">>>=", ">>=", "&=", "^=", "|=",
   "||", "&&", "==", "!=", "<=", ">=", "<<", ">>>", ">>", "++", "--",
   "...", "..",
   "assert", "break", "case", "char", "const", "continue", "default",
   "defined", "do", "else", "enum", "exit", "for", "forward", "goto",
   "if", "native", "new", "operator", "public", "return", "sizeof",
   "sleep", "static", "stock", "switch", "tagof", "while",
   "#assert", "#define", "#else", "#emit", "#endif", "#endinput",
   "#endscript", "#file", "#if", "#include", "#line", "#pragma", "#undef",
   ";", ";", "-integer value-", "-rational value-", "-identifier-",
   "-label-", "-string-"
};

int
lex(cell * lexvalue, char **lexsym)
{
   int                 i, toolong, newline, rawstring;
   char              **tokptr;

   if (_pushed)
     {
	_pushed = FALSE;	/* reset "_pushed" flag */
	*lexvalue = _lexval;
	*lexsym = _lexstr;
	return _lextok;
     }				/* if */

   _lextok = 0;			/* preset all values */
   _lexval = 0;
   _lexstr[0] = '\0';
   *lexvalue = _lexval;
   *lexsym = _lexstr;
   _lexnewline = FALSE;
   if (!freading)
      return 0;

   newline = (lptr == pline);	/* does lptr point to start of line buffer */
   while (*lptr <= ' ')
     {				/* delete leading white space */
	if (*lptr == '\0')
	  {
	     preprocess();	/* preprocess resets "lptr" */
	     if (!freading)
		return 0;
	     if (lptr == term_expr)	/* special sequence to terminate a pending expression */
		return (_lextok = tENDEXPR);
	     _lexnewline = TRUE;	/* set this after preprocess(), because
					 * preprocess() calls lex() recursively */
	     newline = TRUE;
	  }
	else
	  {
	     lptr += 1;
	  }			/* if */
     }				/* while */
   if (newline)
     {
	stmtindent = 0;
	for (i = 0; i < (int)(lptr - pline); i++)
	   if (pline[i] == '\t' && sc_tabsize > 0)
	      stmtindent +=
		 (int)(sc_tabsize - (stmtindent + sc_tabsize) % sc_tabsize);
	   else
	      stmtindent++;
     }				/* if */

   i = tFIRST;
   tokptr = sc_tokens;
   while (i <= tMIDDLE)
     {				/* match multi-character operators */
	if (match(*tokptr, FALSE))
	  {
	     _lextok = i;
	     return _lextok;
	  }			/* if */
	i += 1;
	tokptr += 1;
     }				/* while */
   while (i <= tLAST)
     {				/* match reserved words and compiler directives */
	if (match(*tokptr, TRUE))
	  {
	     _lextok = i;
	     errorset(sRESET);	/* reset error flag (clear the "panic mode") */
	     return _lextok;
	  }			/* if */
	i += 1;
	tokptr += 1;
     }				/* while */

   if ((i = number(&_lexval, lptr)) != 0)
     {				/* number */
	_lextok = tNUMBER;
	*lexvalue = _lexval;
	lptr += i;
     }
   else if ((i = ftoi(&_lexval, lptr)) != 0)
     {
	_lextok = tRATIONAL;
	*lexvalue = _lexval;
	lptr += i;
     }
   else if (alpha(*lptr))
     {				/* symbol or label */
	/*  Note: only sNAMEMAX characters are significant. The compiler
	 *        generates a warning if a symbol exceeds this length.
	 */
	_lextok = tSYMBOL;
	i = 0;
	toolong = 0;
	while (alphanum(*lptr))
	  {
	     _lexstr[i] = *lptr;
	     lptr += 1;
	     if (i < sNAMEMAX)
		i += 1;
	     else
		toolong = 1;
	  }			/* while */
	_lexstr[i] = '\0';
	if (toolong)
	   error(200, _lexstr, sNAMEMAX);	/* symbol too long, truncated to sNAMEMAX chars */
	if (_lexstr[0] == PUBLIC_CHAR && _lexstr[1] == '\0')
	  {
	     _lextok = PUBLIC_CHAR;	/* '@' all alone is not a symbol, it is an operator */
	  }
	else if (_lexstr[0] == '_' && _lexstr[1] == '\0')
	  {
	     _lextok = '_';	/* '_' by itself is not a symbol, it is a placeholder */
	  }			/* if */
	if (*lptr == ':' && sc_allowtags && _lextok != PUBLIC_CHAR)
	  {
	     _lextok = tLABEL;	/* it wasn't a normal symbol, it was a label/tagname */
	     lptr += 1;		/* skip colon */
	  }			/* if */
     }
   else if (*lptr == '\"' || (*lptr == sc_ctrlchar && *(lptr + 1) == '\"'))
     {				/* unpacked string literal */
	_lextok = tSTRING;
	rawstring = (*lptr == sc_ctrlchar);
	*lexvalue = _lexval = litidx;
	lptr += 1;		/* skip double quote */
	if (rawstring)
	   lptr += 1;		/* skip "escape" character too */
	lptr =
	   sc_packstr ? packedstring(lptr, rawstring) : unpackedstring(lptr,
								       rawstring);
	if (*lptr == '\"')
	   lptr += 1;		/* skip final quote */
	else
	   error(37);		/* invalid (non-terminated) string */
     }
   else if ((*lptr == '!' && *(lptr + 1) == '\"')
	    || (*lptr == '!' && *(lptr + 1) == sc_ctrlchar && *(lptr + 2) == '\"')
	    || (*lptr == sc_ctrlchar && *(lptr + 1) == '!'
	    && *(lptr + 2) == '\"'))
     {				/* packed string literal */
	_lextok = tSTRING;
	rawstring = (*lptr == sc_ctrlchar || *(lptr + 1) == sc_ctrlchar);
	*lexvalue = _lexval = litidx;
	lptr += 2;		/* skip exclamation point and double quote */
	if (rawstring)
	   lptr += 1;		/* skip "escape" character too */
	lptr =
	   sc_packstr ? unpackedstring(lptr, rawstring) : packedstring(lptr,
								       rawstring);
	if (*lptr == '\"')
	   lptr += 1;		/* skip final quote */
	else
	   error(37);		/* invalid (non-terminated) string */
     }
   else if (*lptr == '\'')
     {				/* character literal */
	lptr += 1;		/* skip quote */
	_lextok = tNUMBER;
	*lexvalue = _lexval = litchar(&lptr, FALSE);
	if (*lptr == '\'')
	   lptr += 1;		/* skip final quote */
	else
	   error(27);		/* invalid character constant (must be one character) */
     }
   else if (*lptr == ';')
     {				/* semicolumn resets "error" flag */
	_lextok = ';';
	lptr += 1;
	errorset(sRESET);	/* reset error flag (clear the "panic mode") */
     }
   else
     {
	_lextok = *lptr;	/* if every match fails, return the character */
	lptr += 1;		/* increase the "lptr" pointer */
     }				/* if */
   return _lextok;
}

/*  lexpush
 *
 *  Pushes a token back, so the next call to lex() will return the token
 *  last examined, instead of a new token.
 *
 *  Only one token can be pushed back.
 *
 *  In fact, lex() already stores the information it finds into global
 *  variables, so all that is to be done is set a flag that informs lex()
 *  to read and return the information from these variables, rather than
 *  to read in a new token from the input file.
 */
void
lexpush(void)
{
   assert(_pushed == FALSE);
   _pushed = TRUE;
}

/*  lexclr
 *
 *  Sets the variable "_pushed" to 0 to make sure lex() will read in a new
 *  symbol (a not continue with some old one). This is required upon return
 *  from Assembler mode.
 */
void
lexclr(int clreol)
{
   _pushed = FALSE;
   if (clreol)
     {
	lptr = strchr(pline, '\0');
	assert(lptr != NULL);
     }				/* if */
}

/*  matchtoken
 *
 *  This routine is useful if only a simple check is needed. If the token
 *  differs from the one expected, it is pushed back.
 */
int
matchtoken(int token)
{
   cell                val;
   char               *str;
   int                 tok;

   tok = lex(&val, &str);
   if (tok == token || (token == tTERM && (tok == ';' || tok == tENDEXPR)))
     {
	return 1;
     }
   else if (!sc_needsemicolon && token == tTERM && (_lexnewline || !freading))
     {
	lexpush();		/* push "tok" back, we use the "hidden" newline token */
	return 1;
     }
   else
     {
	lexpush();
	return 0;
     }				/* if */
}

/*  tokeninfo
 *
 *  Returns additional information of a token after using "matchtoken()"
 *  or needtoken(). It does no harm using this routine after a call to
 *  "lex()", but lex() already returns the same information.
 *
 *  The token itself is the return value. Normally, this one is already known.
 */
int
tokeninfo(cell * val, char **str)
{
   /* if the token was pushed back, tokeninfo() returns the token and
    * parameters of the *next* token, not of the *current* token.
    */
   assert(!_pushed);
   *val = _lexval;
   *str = _lexstr;
   return _lextok;
}

/*  needtoken
 *
 *  This routine checks for a required token and gives an error message if
 *  it isn't there (and returns FALSE in that case).
 *
 *  Global references: _lextok;
 */
int
needtoken(int token)
{
   char                s1[20], s2[20];

   if (matchtoken(token))
     {
	return TRUE;
     }
   else
     {
	/* token already pushed back */
	assert(_pushed);
        if (token < 256)
          {
             s1[0] = (char)token; /* single character token */
             s1[1] = 0;
          }
        else
          {
             strncpy(s1, sc_tokens[token - tFIRST], 19); /* multi-character symbol */
             s1[19] = 0;
          }

        if (!freading)
          {
             strncpy(s2, "-end of file-", 19);
          }
        else if (_lextok < 256)
          {
             s2[0] = (char)_lextok;
             s2[1] = 0;
          }
        else
          {
             strncpy(s2, sc_tokens[_lextok - tFIRST], 19);
          }
        s2[19] = 0;
	error(1, s1, s2);	/* expected ..., but found ... */
	return FALSE;
     }				/* if */
}

/*  match
 *
 *  Compares a series of characters from the input file with the characters
 *  in "st" (that contains a token). If the token on the input file matches
 *  "st", the input file pointer "lptr" is adjusted to point to the next
 *  token, otherwise "lptr" remains unaltered.
 *
 *  If the parameter "end: is true, match() requires that the first character
 *  behind the recognized token is non-alphanumeric.
 *
 *  Global references: lptr   (altered)
 */
static int
match(char *st, int end)
{
   int                 k;
   char               *ptr;

   k = 0;
   ptr = lptr;
   while (st[k])
     {
	if (st[k] != *ptr)
	   return 0;
	k += 1;
	ptr += 1;
     }				/* while */
   if (end)
     {				/* symbol must terminate with non-alphanumeric char */
	if (alphanum(*ptr))
	   return 0;
     }				/* if */
   lptr = ptr;			/* match found, skip symbol */
   return 1;
}

/*  stowlit
 *
 *  Stores a value into the literal queue. The literal queue is used for
 *  literal strings used in functions and for initializing array variables.
 *
 *  Global references: litidx  (altered)
 *                     litq    (altered)
 */
void
stowlit(cell val)
{
   if (litidx >= litmax)
     {
	cell               *p;

	litmax += sDEF_LITMAX;
	p = (cell *) realloc(litq, litmax * sizeof(cell));
	if (!p)
	   error(102, "literal table");	/* literal table overflow (fatal error) */
	litq = p;
     }				/* if */
   assert(litidx < litmax);
   litq[litidx++] = val;
}

/*  litchar
 *
 *  Return current literal character and increase the pointer to point
 *  just behind this literal character.
 *
 *  Note: standard "escape sequences" are supported, but the backslash may be
 *        replaced by another character; the syntax '\ddd' is supported,
 *        but ddd must be decimal!
 */
static cell
litchar(char **p_str, int rawmode)
{
   cell                c = 0;
   unsigned char      *cptr;

   cptr = (unsigned char *)*p_str;
   if (rawmode || *cptr != sc_ctrlchar)
     {				/* no escape character */
	c = *cptr;
	cptr += 1;
     }
   else
     {
	cptr += 1;
	if (*cptr == sc_ctrlchar)
	  {
	     c = *cptr;		/* \\ == \ (the escape character itself) */
	     cptr += 1;
	  }
	else
	  {
	     switch (*cptr)
	       {
	       case 'a':	/* \a == audible alarm */
		  c = 7;
		  cptr += 1;
		  break;
	       case 'b':	/* \b == backspace */
		  c = 8;
		  cptr += 1;
		  break;
	       case 'e':	/* \e == escape */
		  c = 27;
		  cptr += 1;
		  break;
	       case 'f':	/* \f == form feed */
		  c = 12;
		  cptr += 1;
		  break;
	       case 'n':	/* \n == NewLine character */
		  c = 10;
		  cptr += 1;
		  break;
	       case 'r':	/* \r == carriage return */
		  c = 13;
		  cptr += 1;
		  break;
	       case 't':	/* \t == horizontal TAB */
		  c = 9;
		  cptr += 1;
		  break;
	       case 'v':	/* \v == vertical TAB */
		  c = 11;
		  cptr += 1;
		  break;
	       case '\'':	/* \' == ' (single quote) */
	       case '"':	/* \" == " (single quote) */
	       case '%':	/* \% == % (percent) */
		  c = *cptr;
		  cptr += 1;
		  break;
	       default:
		  if (sc_isdigit(*cptr))
		    {		/* \ddd */
		       c = 0;
		       while (*cptr >= '0' && *cptr <= '9')	/* decimal! */
			  c = c * 10 + *cptr++ - '0';
		       if (*cptr == ';')
			  cptr++;	/* swallow a trailing ';' */
		    }
		  else
		    {
		       error(27);	/* invalid character constant */
		    }		/* if */
	       }		/* switch */
	  }			/* if */
     }				/* if */
   *p_str = (char *)cptr;
   assert(c >= 0 && c < 256);
   return c;
}

/*  alpha
 *
 *  Test if character "c" is alphabetic ("a".."z"), an underscore ("_")
 *  or an "at" sign ("@"). The "@" is an extension to standard C.
 */
static int
alpha(char c)
{
   return (sc_isalpha(c) || c == '_' || c == PUBLIC_CHAR);
}

/*  alphanum
 *
 *  Test if character "c" is alphanumeric ("a".."z", "0".."9", "_" or "@")
 */
int
alphanum(char c)
{
   return (alpha(c) || sc_isdigit(c));
}

/* The local variable table must be searched backwards, so that the deepest
 * nesting of local variables is searched first. The simplest way to do
 * this is to insert all new items at the head of the list.
 * In the global list, the symbols are kept in sorted order, so that the
 * public functions are written in sorted order.
 */
static symbol      *
add_symbol(symbol * root, symbol * entry, int sort)
{
   symbol             *newsym;

   if (sort)
      while (root->next && strcmp(entry->name, root->next->name) > 0)
	 root = root->next;

   if (!(newsym = (symbol *)malloc(sizeof(symbol))))
     {
	error(103);
	return NULL;
     }				/* if */
   memcpy(newsym, entry, sizeof(symbol));
   newsym->next = root->next;
   root->next = newsym;
   return newsym;
}

static void
free_symbol(symbol * sym)
{
   arginfo            *arg;

   /* free all sub-symbol allocated memory blocks, depending on the
    * kind of the symbol
    */
   assert(sym != NULL);
   if (sym->ident == iFUNCTN)
     {
	/* run through the argument list; "default array" arguments
	 * must be freed explicitly; the tag list must also be freed */
	assert(sym->dim.arglist != NULL);
	for (arg = sym->dim.arglist; arg->ident != 0; arg++)
	  {
	     if (arg->ident == iREFARRAY && arg->hasdefault)
		free(arg->defvalue.array.data);
	     else if (arg->ident == iVARIABLE
		      && ((arg->hasdefault & uSIZEOF) != 0
			  || (arg->hasdefault & uTAGOF) != 0))
		free(arg->defvalue.size.symname);
	     assert(arg->tags != NULL);
	     free(arg->tags);
	  }			/* for */
	free(sym->dim.arglist);
     }				/* if */
   assert(sym->refer != NULL);
   free(sym->refer);
   free(sym);
}

void
delete_symbol(symbol * root, symbol * sym)
{
   /* find the symbol and its predecessor
    * (this function assumes that you will never delete a symbol that is not
    * in the table pointed at by "root")
    */
   assert(root != sym);
   while (root->next != sym)
     {
	root = root->next;
	assert(root != NULL);
     }				/* while */

   /* unlink it, then free it */
   root->next = sym->next;
   free_symbol(sym);
}

void
delete_symbols(symbol * root, int level, int delete_labels,
	       int delete_functions)
{
   symbol             *sym;

   /* erase only the symbols with a deeper nesting level than the
    * specified nesting level */
   while (root->next)
     {
	sym = root->next;
	if (sym->compound < level)
	   break;
	if ((delete_labels || sym->ident != iLABEL)
	    && (delete_functions || sym->ident != iFUNCTN
		|| (sym->usage & uNATIVE) != 0) && (delete_functions
						    || sym->ident != iCONSTEXPR
						    || (sym->usage & uPREDEF) ==
						    0) && (delete_functions
							   || (sym->ident !=
							       iVARIABLE
							       && sym->ident !=
							       iARRAY)))
	  {
	     root->next = sym->next;
	     free_symbol(sym);
	  }
	else
	  {
	     /* if the function was prototyped, but not implemented in this source,
	      * mark it as such, so that its use can be flagged
	      */
	     if (sym->ident == iFUNCTN && (sym->usage & uDEFINE) == 0)
		sym->usage |= uMISSING;
	     if (sym->ident == iFUNCTN || sym->ident == iVARIABLE
		 || sym->ident == iARRAY)
		sym->usage &= ~uDEFINE;	/* clear "defined" flag */
	     /* for user defined operators, also remove the "prototyped" flag, as
	      * user-defined operators *must* be declared before use
	      */
	     if (sym->ident == iFUNCTN && !sc_isalpha(*sym->name)
		 && *sym->name != '_' && *sym->name != PUBLIC_CHAR)
		sym->usage &= ~uPROTOTYPED;
	     root = sym;	/* skip the symbol */
	  }			/* if */
     }				/* if */
}

/* The purpose of the hash is to reduce the frequency of a "name"
 * comparison (which is costly). There is little interest in avoiding
 * clusters in similar names, which is why this function is plain simple.
 */
unsigned int
namehash(char *name)
{
   unsigned char      *ptr = (unsigned char *)name;
   int                 len = strlen(name);

   if (len == 0)
      return 0L;
   assert(len < 256);
   return (len << 24Lu) + (ptr[0] << 16Lu) + (ptr[len - 1] << 8Lu) +
      (ptr[len >> 1Lu]);
}

static symbol      *
find_symbol(symbol * root, char *name, int fnum)
{
   symbol             *ptr = root->next;
   unsigned long       hash = namehash(name);

   while (ptr)
     {
	if (hash == ptr->hash && strcmp(name, ptr->name) == 0
	    && !ptr->parent && (ptr->fnumber < 0
				       || ptr->fnumber == fnum))
	   return ptr;
	ptr = ptr->next;
     }				/* while */
   return NULL;
}

static symbol      *
find_symbol_child(symbol * root, symbol * sym)
{
   symbol             *ptr = root->next;

   while (ptr)
     {
	if (ptr->parent == sym)
	   return ptr;
	ptr = ptr->next;
     }				/* while */
   return NULL;
}

/* Adds "bywhom" to the list of referrers of "entry". Typically,
 * bywhom will be the function that uses a variable or that calls
 * the function.
 */
int
refer_symbol(symbol * entry, symbol * bywhom)
{
   int                 count;

   assert(bywhom != NULL);	/* it makes no sense to add a "void" referrer */
   assert(entry != NULL);
   assert(entry->refer != NULL);

   /* see if it is already there */
   for (count = 0; count < entry->numrefers && entry->refer[count] != bywhom;
	count++)
      /* nothing */ ;
   if (count < entry->numrefers)
     {
	assert(entry->refer[count] == bywhom);
	return TRUE;
     }				/* if */

   /* see if there is an empty spot in the referrer list */
   for (count = 0; count < entry->numrefers && entry->refer[count];
	count++)
      /* nothing */ ;
   assert(count <= entry->numrefers);
   if (count == entry->numrefers)
     {
	symbol            **refer;
	int                 newsize = 2 * entry->numrefers;

	assert(newsize > 0);
	/* grow the referrer list */
	refer = (symbol **) realloc(entry->refer, newsize * sizeof(symbol *));
	if (!refer)
	   return FALSE;	/* insufficient memory */
	/* initialize the new entries */
	entry->refer = refer;
	for (count = entry->numrefers; count < newsize; count++)
	   entry->refer[count] = NULL;
	count = entry->numrefers;	/* first empty spot */
	entry->numrefers = newsize;
     }				/* if */

   /* add the referrer */
   assert(entry->refer[count] == NULL);
   entry->refer[count] = bywhom;
   return TRUE;
}

void
markusage(symbol * sym, int usage)
{
   sym->usage |= (char)usage;
   /* check if (global) reference must be added to the symbol */
   if ((usage & (uREAD | uWRITTEN)) != 0)
     {
	/* only do this for global symbols */
	if (sym->vclass == sGLOBAL)
	  {
	     /* "curfunc" should always be valid, since statements may not occurs
	      * outside functions; in the case of syntax errors, however, the
	      * compiler may arrive through this function
	      */
	     if (curfunc)
		refer_symbol(sym, curfunc);
	  }			/* if */
     }				/* if */
}

/*  findglb
 *
 *  Returns a pointer to the global symbol (if found) or NULL (if not found)
 */
symbol     *
findglb(char *name)
{
   return find_symbol(&glbtab, name, fcurrent);
}

/*  findloc
 *
 *  Returns a pointer to the local symbol (if found) or NULL (if not found).
 *  See add_symbol() how the deepest nesting level is searched first.
 */
symbol     *
findloc(char *name)
{
   return find_symbol(&loctab, name, -1);
}

symbol     *
findconst(char *name)
{
   symbol             *sym;

   sym = find_symbol(&loctab, name, -1);	/* try local symbols first */
   if (!sym || sym->ident != iCONSTEXPR)	/* not found, or not a constant */
      sym = find_symbol(&glbtab, name, fcurrent);
   if (!sym || sym->ident != iCONSTEXPR)
      return NULL;
   assert(sym->parent == NULL);	/* constants have no hierarchy */
   return sym;
}

symbol     *
finddepend(symbol * parent)
{
   symbol             *sym;

   sym = find_symbol_child(&loctab, parent);	/* try local symbols first */
   if (!sym)		/* not found */
      sym = find_symbol_child(&glbtab, parent);
   return sym;
}

/*  addsym
 *
 *  Adds a symbol to the symbol table (either global or local variables,
 *  or global and local constants).
 */
symbol     *
addsym(char *name, cell addr, int ident, int vclass, int tag, int usage)
{
   symbol              entry, **refer;

   /* global variables/constants/functions may only be defined once */
   assert(!(ident == iFUNCTN || ident == iCONSTEXPR) || vclass != sGLOBAL
	  || findglb(name) == NULL);
   /* labels may only be defined once */
   assert(ident != iLABEL || findloc(name) == NULL);

   /* create an empty referrer list */
   if (!(refer = (symbol **)malloc(sizeof(symbol *))))
     {
	error(103);		/* insufficient memory */
	return NULL;
     }				/* if */
   *refer = NULL;

   /* first fill in the entry */
   strncpy(entry.name, name, sizeof(entry.name) - 1);
   entry.name[sizeof(entry.name) - 1] = 0;
   entry.hash = namehash(name);
   entry.addr = addr;
   entry.vclass = (char)vclass;
   entry.ident = (char)ident;
   entry.tag = tag;
   entry.usage = (char)usage;
   entry.compound = 0;		/* may be overridden later */
   entry.fnumber = -1;		/* assume global visibility (ignored for local symbols) */
   entry.numrefers = 1;
   entry.refer = refer;
   entry.parent = NULL;

   /* then insert it in the list */
   if (vclass == sGLOBAL)
      return add_symbol(&glbtab, &entry, TRUE);
   else
      return add_symbol(&loctab, &entry, FALSE);
}

symbol     *
addvariable(char *name, cell addr, int ident, int vclass, int tag,
	    int dim[], int numdim, int idxtag[])
{
   symbol             *sym, *parent, *top;
   int                 level;

   sym = findglb(name);
   /* global variables may only be defined once */
   assert(vclass != sGLOBAL || sym  == NULL
	  || (sym->usage & uDEFINE) == 0);

   if (ident == iARRAY || ident == iREFARRAY)
     {
	parent = NULL;
	sym = NULL;		/* to avoid a compiler warning */
	for (level = 0; level < numdim; level++)
	  {
	     top = addsym(name, addr, ident, vclass, tag, uDEFINE);
	     top->dim.array.length = dim[level];
	     top->dim.array.level = (short)(numdim - level - 1);
	     top->x.idxtag = idxtag[level];
	     top->parent = parent;
	     parent = top;
	     if (level == 0)
		sym = top;
	  }			/* for */
     }
   else
     {
	sym = addsym(name, addr, ident, vclass, tag, uDEFINE);
     }				/* if */
   return sym;
}

/*  getlabel
 *
 *  Return next available internal label number.
 */
int
getlabel(void)
{
   return labnum++;
}

/*  itoh
 *
 *  Converts a number to a hexadecimal string and returns a pointer to that
 *  string.
 */
char       *
itoh(ucell val)
{
   static char         itohstr[15];	/* hex number is 10 characters long at most */
   char               *ptr;
   int                 i, nibble[8];	/* a 32-bit hexadecimal cell has 8 nibbles */
   int                 max;

#if defined(BIT16)
   max = 4;
#else
   max = 8;
#endif
   ptr = itohstr;
   for (i = 0; i < max; i += 1)
     {
	nibble[i] = (int)(val & 0x0f);	/* nibble 0 is lowest nibble */
	val >>= 4;
     }				/* endfor */
   i = max - 1;
   while (nibble[i] == 0 && i > 0)	/* search for highest non-zero nibble */
      i -= 1;
   while (i >= 0)
     {
	if (nibble[i] >= 10)
	   *ptr++ = (char)('a' + (nibble[i] - 10));
	else
	   *ptr++ = (char)('0' + nibble[i]);
	i -= 1;
     }				/* while */
   *ptr = '\0';			/* and a zero-terminator */
   return itohstr;
}
