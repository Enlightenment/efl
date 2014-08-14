/* Parse C expressions for CCCP.
 * Copyright (C) 1987, 1992, 1994, 1995 Free Software Foundation.
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
 * along with this program; if not, write to the Free Software
 * Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Help stamp out software-hoarding!
 * 
 * Written by Per Bothner 1994. */

/* Parse a C expression from text in a string  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef __EMX__
# include <strings.h>
#endif

#ifdef MULTIBYTE_CHARS
# include <locale.h>
#endif

#include <Eina.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cpplib.h"
#include "cpphash.h"

#ifdef _WIN32
# ifdef ERROR
#  undef ERROR
# endif
#endif

/* This is used for communicating lists of keywords with cccp.c.  */
struct arglist {
   struct arglist     *next;
   unsigned char      *name;
   int                 length;
   int                 argno;
};

/* Define a generic NULL if one hasn't already been defined.  */

#ifndef NULL
#define NULL 0
#endif

#ifndef GENERIC_PTR
#if defined (USE_PROTOTYPES) ? USE_PROTOTYPES : defined (__STDC__)
#define GENERIC_PTR void *
#else
#define GENERIC_PTR char *
#endif
#endif

#ifndef NULL_PTR
#define NULL_PTR ((GENERIC_PTR)0)
#endif

#ifndef CHAR_TYPE_SIZE
#define CHAR_TYPE_SIZE BITS_PER_UNIT
#endif

#ifndef INT_TYPE_SIZE
#define INT_TYPE_SIZE BITS_PER_WORD
#endif

#ifndef LONG_TYPE_SIZE
#define LONG_TYPE_SIZE BITS_PER_WORD
#endif

#ifndef WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE INT_TYPE_SIZE
#endif

#ifndef MAX_CHAR_TYPE_SIZE
#define MAX_CHAR_TYPE_SIZE CHAR_TYPE_SIZE
#endif

#ifndef MAX_INT_TYPE_SIZE
#define MAX_INT_TYPE_SIZE INT_TYPE_SIZE
#endif

#ifndef MAX_LONG_TYPE_SIZE
#define MAX_LONG_TYPE_SIZE LONG_TYPE_SIZE
#endif

#ifndef MAX_WCHAR_TYPE_SIZE
#define MAX_WCHAR_TYPE_SIZE WCHAR_TYPE_SIZE
#endif

/* Yield nonzero if adding two numbers with A's and B's signs can yield a
 * number with SUM's sign, where A, B, and SUM are all C integers.  */
#define possible_sum_sign(a, b, sum) ((((a) ^ (b)) | ~ ((a) ^ (sum))) < 0)

#define ERROR 299
#define OROR 300
#define ANDAND 301
#define EQUAL 302
#define NOTEQUAL 303
#define LEQ 304
#define GEQ 305
#define LSH 306
#define RSH 307
#define NAME 308
#define INT 309
#define CHAR 310

#define LEFT_OPERAND_REQUIRED 1
#define RIGHT_OPERAND_REQUIRED 2
#define HAVE_VALUE 4
/* SKIP_OPERAND is set for '&&' '||' '?' and ':' when the
 * following operand should be short-circuited instead of evaluated. */
#define SKIP_OPERAND 8
/*#define UNSIGNEDP 16 */

struct operation {
   short               op;
   char                rprio;	/* Priority of op (relative to it right operand). */
   char                flags;
   char                unsignedp;	/* true if value should be treated as unsigned */
   HOST_WIDE_INT       value;	/* The value logically "right" of op. */
};

/* Take care of parsing a number (anything that starts with a digit).
 * LEN is the number of characters in it.  */

/* maybe needs to actually deal with floating point numbers */

static void
parse_number(struct operation *op, cpp_reader * pfile, const char *start,
	     int olen)
{
   const char         *p = start;
   int                 c;
   unsigned long       n = 0, nd, ULONG_MAX_over_base;
   int                 base = 10;
   int                 len = olen;
   int                 overflow = 0;
   int                 digit, largest_digit = 0;
   int                 spec_long = 0;

   op->unsignedp = 0;

   for (c = 0; c < len; c++)
      if (p[c] == '.')
	{
	   /* It's a float since it contains a point.  */
	   cpp_error(pfile,
		     "floating point numbers not allowed in #if expressions");
	   op->op = ERROR;
	   return;
	}
   if (len >= 3 && (!strncmp(p, "0x", 2) || !strncmp(p, "0X", 2)))
     {
	p += 2;
	base = 16;
	len -= 2;
     }
   else if (*p == '0')
      base = 8;

   /* Some buggy compilers (e.g. MPW C) seem to need both casts. */
   ULONG_MAX_over_base = ((unsigned long)-1) / ((unsigned long)base);

   for (; len > 0; len--)
     {
	c = *p++;

	if (c >= '0' && c <= '9')
	   digit = c - '0';
	else if (base == 16 && c >= 'a' && c <= 'f')
	   digit = c - 'a' + 10;
	else if (base == 16 && c >= 'A' && c <= 'F')
	   digit = c - 'A' + 10;
	else
	  {
	     /* `l' means long, and `u' means unsigned.  */
	     while (1)
	       {
		  if (c == 'l' || c == 'L')
		    {
		       if (spec_long)
			  cpp_error(pfile, "two `l's in integer constant");
		       spec_long = 1;
		    }
		  else if (c == 'u' || c == 'U')
		    {
		       if (op->unsignedp)
			  cpp_error(pfile, "two `u's in integer constant");
		       op->unsignedp = 1;
		    }
		  else
		     break;

		  if (--len == 0)
		     break;
		  c = *p++;
	       }
	     /* Don't look for any more digits after the suffixes.  */
	     break;
	  }
	if (largest_digit < digit)
	   largest_digit = digit;
	nd = n * base + digit;
	overflow |= (ULONG_MAX_over_base < n) | (nd < n);
	n = nd;
     }

   if (len != 0)
     {
	cpp_error(pfile, "Invalid number in #if expression");
	op->op = ERROR;
	return;
     }
   if (base <= largest_digit)
      cpp_warning(pfile, "integer constant contains digits beyond the radix");

   if (overflow)
      cpp_warning(pfile, "integer constant out of range");

   /* If too big to be signed, consider it unsigned.  */
   if ((long)n < 0 && !op->unsignedp)
     {
	if (base == 10)
	   cpp_warning(pfile,
		       "integer constant is so large that it is unsigned");
	op->unsignedp = 1;
     }
   op->value = n;
   op->op = INT;
}

struct token {
   const char         *oper;
   int                 token;
};

static struct token tokentab2[] = {
   {"&&", ANDAND},
   {"||", OROR},
   {"<<", LSH},
   {">>", RSH},
   {"==", EQUAL},
   {"!=", NOTEQUAL},
   {"<=", LEQ},
   {">=", GEQ},
   {"++", ERROR},
   {"--", ERROR},
   {NULL, ERROR}
};

/* Read one token. */

static void
cpp_lex(struct operation *op, cpp_reader * pfile)
{
   int                 c;
   struct token       *toktab;
   enum cpp_token      token;
   unsigned char      *tok_start, *tok_end;
   int                 old_written;

   op->value = 0;
   op->unsignedp = 0;

 retry:

   old_written = CPP_WRITTEN(pfile);
   cpp_skip_hspace(pfile);
   c = CPP_BUF_PEEK(CPP_BUFFER(pfile));
   if (c == '#')
     {
	parse_number(op, pfile, cpp_read_check_assertion(pfile) ? "1" : "0", 1);
	return;
     }

   if (c == '\n')
     {
	op->op = 0;
	return;
     }
   token = cpp_get_token(pfile);
   tok_start = pfile->token_buffer + old_written;
   tok_end = CPP_PWRITTEN(pfile);
   pfile->limit = tok_start;
   switch (token)
     {
     case CPP_EOF:		/* Should not happen ... */
	op->op = 0;
	break;

     case CPP_VSPACE:
     case CPP_POP:
	if (CPP_BUFFER(pfile)->fname)
	  {
	     op->op = 0;
	     break;
	  }
	goto retry;

     case CPP_HSPACE:
     case CPP_COMMENT:
	goto retry;

     case CPP_NUMBER:
	parse_number(op, pfile, (char *)tok_start, tok_end - tok_start);
	break;

     case CPP_STRING:
	cpp_error(pfile, "string constants not allowed in #if expressions");
	op->op = ERROR;
	break;

     case CPP_CHAR:
	/* This code for reading a character constant
	 * handles multicharacter constants and wide characters.
	 * It is mostly copied from c-lex.c.  */
	{
	   int                 result = 0;
	   int                 num_chars = 0;
	   unsigned            width = MAX_CHAR_TYPE_SIZE;
	   int                 wide_flag = 0;
	   int                 max_chars;
	   char               *ptr = (char *)tok_start;

#ifdef MULTIBYTE_CHARS
	   char                token_buffer[MAX_LONG_TYPE_SIZE /
					    MAX_CHAR_TYPE_SIZE + MB_CUR_MAX];
#endif

	   if (*ptr == 'L')
	     {
		ptr++;
		wide_flag = 1;
		width = MAX_WCHAR_TYPE_SIZE;
#ifdef MULTIBYTE_CHARS
		max_chars = MB_CUR_MAX;
#else
		max_chars = 1;
#endif
	     }
	   else
	      max_chars = MAX_LONG_TYPE_SIZE / width;

	   while (1)
	     {
		if (ptr >= (char *)CPP_PWRITTEN(pfile) || (c = *ptr++) == '\'')
		   break;

		if (c == '\\')
		  {
		     c = cpp_parse_escape(pfile, &ptr);
		     if (width < HOST_BITS_PER_INT
			 && (unsigned)c >= (unsigned)(1 << width))
			cpp_pedwarn(pfile,
				    "escape sequence out of range for character");
		  }
		num_chars++;

		/* Merge character into result; ignore excess chars.  */
		if (num_chars < max_chars + 1)
		  {
		     if (width < HOST_BITS_PER_INT)
			result = (result << width) | (c & ((1 << width) - 1));
		     else
			result = c;
#ifdef MULTIBYTE_CHARS
		     token_buffer[num_chars - 1] = c;
#endif
		  }
	     }

#ifdef MULTIBYTE_CHARS
	   token_buffer[num_chars] = 0;
#endif

	   if (c != '\'')
	      cpp_error(pfile, "malformatted character constant");
	   else if (num_chars == 0)
	      cpp_error(pfile, "empty character constant");
	   else if (num_chars > max_chars)
	     {
		num_chars = max_chars;
		cpp_error(pfile, "character constant too long");
	     }
	   else if (num_chars != 1 && !CPP_TRADITIONAL(pfile))
	      cpp_warning(pfile, "multi-character character constant");

	   /* If char type is signed, sign-extend the constant.  */
	   if (!wide_flag)
	     {
		int                 num_bits = num_chars * width;

		if ((num_bits > 0) &&
		    (cpp_lookup("__CHAR_UNSIGNED__",
			       sizeof("__CHAR_UNSIGNED__") - 1, -1)
		    || ((result >> (num_bits - 1)) & 1) == 0))
		   op->value =
		      result & ((unsigned long)~0 >>
				(HOST_BITS_PER_LONG - num_bits));
		else
		   op->value =
		      result | ~((unsigned long)~0 >>
				 (HOST_BITS_PER_LONG - num_bits));
	     }
	   else
	     {
#ifdef MULTIBYTE_CHARS
		/* Set the initial shift state and convert the next sequence.  */
		result = 0;
		/* In all locales L'\0' is zero and mbtowc will return zero,
		 * so don't use it.  */
		if (num_chars > 1
		    || (num_chars == 1 && token_buffer[0] != '\0'))
		  {
		     wchar_t             wc;

		     (void)mbtowc(NULL_PTR, NULL_PTR, 0);
		     if (mbtowc(&wc, token_buffer, num_chars) == num_chars)
			result = wc;
		     else
			cpp_warning(pfile,
				    "Ignoring invalid multibyte character");
		  }
#endif
		op->value = result;
	     }
	}

	/* This is always a signed type.  */
	op->unsignedp = 0;
	op->op = CHAR;
	break;

     case CPP_NAME:
	parse_number(op, pfile, "0", 0);
	break;

     case CPP_OTHER:
	/* See if it is a special token of length 2.  */
	if (tok_start + 2 == tok_end)
	  {
	     for (toktab = tokentab2; toktab->oper; toktab++)
		if (tok_start[0] == toktab->oper[0]
		    && tok_start[1] == toktab->oper[1])
		   break;
	     if (toktab->token == ERROR)
	       {
		  char               *buf = (char *)malloc(40);

		  memset(buf, 0, 40);

		  sprintf(buf, "`%s' not allowed in operand of `#if'",
			  tok_start);
		  cpp_error(pfile, buf);
		  free(buf);
	       }
	     op->op = toktab->token;
	     break;
	  }
	/* fall through */
     default:
	op->op = *tok_start;
	break;
     }
}

/* Parse a C escape sequence.  STRING_PTR points to a variable
 * containing a pointer to the string to parse.  That pointer
 * is updated past the characters we use.  The value of the
 * escape sequence is returned.
 * 
 * A negative value means the sequence \ newline was seen,
 * which is supposed to be equivalent to nothing at all.
 * 
 * If \ is followed by a null character, we return a negative
 * value and leave the string pointer pointing at the null character.
 * 
 * If \ is followed by 000, we return 0 and leave the string pointer
 * after the zeros.  A value of 0 does not mean end of string.  */

int
cpp_parse_escape(cpp_reader * pfile, char **string_ptr)
{
   int                 c = *(*string_ptr)++;

   switch (c)
     {
     case 'a':
	return TARGET_BELL;
     case 'b':
	return TARGET_BS;
     case 'e':
     case 'E':
	if (CPP_PEDANTIC(pfile))
	   cpp_pedwarn(pfile, "non-ANSI-standard escape sequence, `\\%c'", c);
	return 033;
     case 'f':
	return TARGET_FF;
     case 'n':
	return TARGET_NEWLINE;
     case 'r':
	return TARGET_CR;
     case 't':
	return TARGET_TAB;
     case 'v':
	return TARGET_VT;
     case '\n':
	return -2;
     case 0:
	(*string_ptr)--;
	return 0;

     case '0':
     case '1':
     case '2':
     case '3':
     case '4':
     case '5':
     case '6':
     case '7':
	{
	   int                 i = c - '0';
	   int                 count = 0;

	   while (++count < 3)
	     {
		c = *(*string_ptr)++;
		if (c >= '0' && c <= '7')
		   i = (i << 3) + c - '0';
		else
		  {
		     (*string_ptr)--;
		     break;
		  }
	     }
	   if ((i & ~((1 << MAX_CHAR_TYPE_SIZE) - 1)) != 0)
	     {
		i &= (1 << MAX_CHAR_TYPE_SIZE) - 1;
		cpp_warning(pfile,
			    "octal character constant does not fit in a byte");
	     }
	   return i;
	}
     case 'x':
	{
	   unsigned            i = 0, overflow = 0, digits_found = 0, digit;

	   for (;;)
	     {
		c = *(*string_ptr)++;
		if (c >= '0' && c <= '9')
		   digit = c - '0';
		else if (c >= 'a' && c <= 'f')
		   digit = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
		   digit = c - 'A' + 10;
		else
		  {
		     (*string_ptr)--;
		     break;
		  }
		overflow |= i ^ (i << 4 >> 4);
		i = (i << 4) + digit;
		digits_found = 1;
	     }
	   if (!digits_found)
	      cpp_error(pfile, "\\x used with no following hex digits");
	   if (overflow | (i & ~((1 << BITS_PER_UNIT) - 1)))
	     {
		i &= (1 << BITS_PER_UNIT) - 1;
		cpp_warning(pfile,
			    "hex character constant does not fit in a byte");
	     }
	   return i;
	}
     default:
	return c;
     }
}

static void
integer_overflow(cpp_reader * pfile)
{
   if (CPP_PEDANTIC(pfile))
      cpp_pedwarn(pfile, "integer overflow in preprocessor expression");
}

static long
left_shift(cpp_reader * pfile, long a, int unsignedp, unsigned long b)
{
   if (b >= HOST_BITS_PER_LONG)
     {
	if (!unsignedp && a != 0)
	   integer_overflow(pfile);
	return 0;
     }
   else if (unsignedp)
      return (unsigned long)a << b;
   else
     {
	long                l = a << b;

	if (l >> b != a)
	   integer_overflow(pfile);
	return l;
     }
}

static long
right_shift(cpp_reader * pfile EINA_UNUSED, long a, int unsignedp,
	    unsigned long b)
{
   if (b >= HOST_BITS_PER_LONG)
     {
	return unsignedp ? 0 : a >> (HOST_BITS_PER_LONG - 1);
     }
   else if (unsignedp)
     {
	return (unsigned long)a >> b;
     }
   else
     {
	return a >> b;
     }
}

/* These priorities are all even, so we can handle associatively. */
#define PAREN_INNER_PRIO 0
#define COMMA_PRIO 4
#define COND_PRIO (COMMA_PRIO+2)
#define OROR_PRIO (COND_PRIO+2)
#define ANDAND_PRIO (OROR_PRIO+2)
#define OR_PRIO (ANDAND_PRIO+2)
#define XOR_PRIO (OR_PRIO+2)
#define AND_PRIO (XOR_PRIO+2)
#define EQUAL_PRIO (AND_PRIO+2)
#define LESS_PRIO (EQUAL_PRIO+2)
#define SHIFT_PRIO (LESS_PRIO+2)
#define PLUS_PRIO (SHIFT_PRIO+2)
#define MUL_PRIO (PLUS_PRIO+2)
#define UNARY_PRIO (MUL_PRIO+2)
#define PAREN_OUTER_PRIO (UNARY_PRIO+2)

#define COMPARE(OP) \
  top->unsignedp = 0;\
  top->value = (unsigned1 || unsigned2) ? (unsigned long) v1 OP (unsigned long) v2 : (v1 OP v2)

/* Parse and evaluate a C expression, reading from PFILE.
 * Returns the value of the expression.  */

HOST_WIDE_INT
cpp_parse_expr(cpp_reader * pfile)
{
   /* The implementation is an operator precedence parser,
    * i.e. a bottom-up parser, using a stack for not-yet-reduced tokens.
    * 
    * The stack base is 'stack', and the current stack pointer is 'top'.
    * There is a stack element for each operator (only),
    * and the most recently pushed operator is 'top->op'.
    * An operand (value) is stored in the 'value' field of the stack
    * element of the operator that precedes it.
    * In that case the 'flags' field has the HAVE_VALUE flag set.  */

#define INIT_STACK_SIZE 20
   struct operation    init_stack[INIT_STACK_SIZE];
   struct operation   *stack = init_stack;
   struct operation   *limit = stack + INIT_STACK_SIZE;
   struct operation   *top = stack;
   int                 lprio = 0, rprio = 0;
   int                 skip_evaluation = 0;

   top->rprio = 0;
   top->flags = 0;
   for (;;)
     {
	struct operation    op;
	char                flags = 0;

	/* Read a token */
	cpp_lex(&op, pfile);

	/* See if the token is an operand, in which case go to set_value.
	 * If the token is an operator, figure out its left and right
	 * priorities, and then goto maybe_reduce. */

	switch (op.op)
	  {
	  case NAME:
	     top->value = 0, top->unsignedp = 0;
	     goto set_value;
	  case INT:
	  case CHAR:
	     top->value = op.value;
	     top->unsignedp = op.unsignedp;
	     goto set_value;
	  case 0:
	     lprio = 0;
	     goto maybe_reduce;
	  case '+':
	  case '-':
	     /* Is this correct if unary ? FIXME */
	     flags = RIGHT_OPERAND_REQUIRED;
	     lprio = PLUS_PRIO;
	     rprio = lprio + 1;
	     goto maybe_reduce;
	  case '!':
	  case '~':
	     flags = RIGHT_OPERAND_REQUIRED;
	     rprio = UNARY_PRIO;
	     lprio = rprio + 1;
	     goto maybe_reduce;
	  case '*':
	  case '/':
	  case '%':
	     lprio = MUL_PRIO;
	     goto binop;
	  case '<':
	  case '>':
	  case LEQ:
	  case GEQ:
	     lprio = LESS_PRIO;
	     goto binop;
	  case EQUAL:
	  case NOTEQUAL:
	     lprio = EQUAL_PRIO;
	     goto binop;
	  case LSH:
	  case RSH:
	     lprio = SHIFT_PRIO;
	     goto binop;
	  case '&':
	     lprio = AND_PRIO;
	     goto binop;
	  case '^':
	     lprio = XOR_PRIO;
	     goto binop;
	  case '|':
	     lprio = OR_PRIO;
	     goto binop;
	  case ANDAND:
	     lprio = ANDAND_PRIO;
	     goto binop;
	  case OROR:
	     lprio = OROR_PRIO;
	     goto binop;
	  case ',':
	     lprio = COMMA_PRIO;
	     goto binop;
	  case '(':
	     lprio = PAREN_OUTER_PRIO;
	     rprio = PAREN_INNER_PRIO;
	     goto maybe_reduce;
	  case ')':
	     lprio = PAREN_INNER_PRIO;
	     rprio = PAREN_OUTER_PRIO;
	     goto maybe_reduce;
	  case ':':
	     lprio = COND_PRIO;
	     rprio = COND_PRIO;
	     goto maybe_reduce;
	  case '?':
	     lprio = COND_PRIO + 1;
	     rprio = COND_PRIO;
	     goto maybe_reduce;
	   binop:
	     flags = LEFT_OPERAND_REQUIRED | RIGHT_OPERAND_REQUIRED;
	     rprio = lprio + 1;
	     goto maybe_reduce;
	  default:
	     cpp_error(pfile, "invalid character in #if");
	     goto syntax_error;
	  }

      set_value:
	/* Push a value onto the stack. */
	if (top->flags & HAVE_VALUE)
	  {
	     cpp_error(pfile, "syntax error in #if");
	     goto syntax_error;
	  }
	top->flags |= HAVE_VALUE;
	continue;

      maybe_reduce:
	/* Push an operator, and check if we can reduce now. */
	while (top->rprio > lprio)
	  {
	     long                v1 = top[-1].value, v2 = top[0].value;
	     int                 unsigned1 = top[-1].unsignedp, unsigned2 =
		top[0].unsignedp;

	     top--;
	     if ((top[1].flags & LEFT_OPERAND_REQUIRED)
		 && !(top[0].flags & HAVE_VALUE))
	       {
		  cpp_error(pfile, "syntax error - missing left operand");
		  goto syntax_error;
	       }
	     if ((top[1].flags & RIGHT_OPERAND_REQUIRED)
		 && !(top[1].flags & HAVE_VALUE))
	       {
		  cpp_error(pfile, "syntax error - missing right operand");
		  goto syntax_error;
	       }
	     /* top[0].value = (top[1].op)(v1, v2); */
	     switch (top[1].op)
	       {
	       case '+':
		  if (!(top->flags & HAVE_VALUE))
		    {		/* Unary '+' */
		       top->value = v2;
		       top->unsignedp = unsigned2;
		       top->flags |= HAVE_VALUE;
		    }
		  else
		    {
		       top->value = v1 + v2;
		       top->unsignedp = unsigned1 || unsigned2;
		       if (!top->unsignedp && !skip_evaluation
			   && !possible_sum_sign(v1, v2, top->value))
			  integer_overflow(pfile);
		    }
		  break;
	       case '-':
		  if (skip_evaluation);	/* do nothing */
		  else if (!(top->flags & HAVE_VALUE))
		    {		/* Unary '-' */
		       top->value = -v2;
		       if ((top->value & v2) < 0 && !unsigned2)
			  integer_overflow(pfile);
		       top->unsignedp = unsigned2;
		       top->flags |= HAVE_VALUE;
		    }
		  else
		    {		/* Binary '-' */
		       top->value = v1 - v2;
		       top->unsignedp = unsigned1 || unsigned2;
		       if (!top->unsignedp
			   && !possible_sum_sign(top->value, v2, v1))
			  integer_overflow(pfile);
		    }
		  break;
	       case '*':
		  top->unsignedp = unsigned1 || unsigned2;
		  if (top->unsignedp)
		     top->value = (unsigned long)v1 *v2;

		  else if (!skip_evaluation)
		    {
		       top->value = v1 * v2;
		       if (v1
			   && (top->value / v1 != v2
			       || (top->value & v1 & v2) < 0))
			  integer_overflow(pfile);
		    }
		  break;
	       case '/':
		  if (skip_evaluation)
		     break;
		  if (v2 == 0)
		    {
		       cpp_error(pfile, "division by zero in #if");
		       v2 = 1;
		    }
		  top->unsignedp = unsigned1 || unsigned2;
		  if (top->unsignedp)
		     top->value = (unsigned long)v1 / v2;
		  else
		    {
		       top->value = v1 / v2;
		       if ((top->value & v1 & v2) < 0)
			  integer_overflow(pfile);
		    }
		  break;
	       case '%':
		  if (skip_evaluation)
		     break;
		  if (v2 == 0)
		    {
		       cpp_error(pfile, "division by zero in #if");
		       v2 = 1;
		    }
		  top->unsignedp = unsigned1 || unsigned2;
		  if (top->unsignedp)
		     top->value = (unsigned long)v1 % v2;
		  else
		     top->value = v1 % v2;
		  break;
	       case '!':
		  if (top->flags & HAVE_VALUE)
		    {
		       cpp_error(pfile, "syntax error");
		       goto syntax_error;
		    }
		  top->value = !v2;
		  top->unsignedp = 0;
		  top->flags |= HAVE_VALUE;
		  break;
	       case '~':
		  if (top->flags & HAVE_VALUE)
		    {
		       cpp_error(pfile, "syntax error");
		       goto syntax_error;
		    }
		  top->value = ~v2;
		  top->unsignedp = unsigned2;
		  top->flags |= HAVE_VALUE;
		  break;
	       case '<':
		  COMPARE(<);
		  break;
	       case '>':
		  COMPARE(>);
		  break;
	       case LEQ:
		  COMPARE(<=);
		  break;
	       case GEQ:
		  COMPARE(>=);
		  break;
	       case EQUAL:
		  top->value = (v1 == v2);
		  top->unsignedp = 0;
		  break;
	       case NOTEQUAL:
		  top->value = (v1 != v2);
		  top->unsignedp = 0;
		  break;
	       case LSH:
		  if (skip_evaluation)
		     break;
		  top->unsignedp = unsigned1;
		  if (v2 < 0 && !unsigned2)
		     top->value = right_shift(pfile, v1, unsigned1, -v2);
		  else
		     top->value = left_shift(pfile, v1, unsigned1, v2);
		  break;
	       case RSH:
		  if (skip_evaluation)
		     break;
		  top->unsignedp = unsigned1;
		  if (v2 < 0 && !unsigned2)
		     top->value = left_shift(pfile, v1, unsigned1, -v2);
		  else
		     top->value = right_shift(pfile, v1, unsigned1, v2);
		  break;
#define LOGICAL(OP) \
	      top->value = v1 OP v2;\
	      top->unsignedp = unsigned1 || unsigned2;
	       case '&':
		  LOGICAL(&);
		  break;
	       case '^':
		  LOGICAL(^);
		  break;
	       case '|':
		  LOGICAL(|);
		  break;
	       case ANDAND:
		  top->value = v1 && v2;
		  top->unsignedp = 0;
		  if (!v1)
		     skip_evaluation--;
		  break;
	       case OROR:
		  top->value = v1 || v2;
		  top->unsignedp = 0;
		  if (v1)
		     skip_evaluation--;
		  break;
	       case ',':
		  if (CPP_PEDANTIC(pfile))
		     cpp_pedwarn(pfile, "comma operator in operand of `#if'");
		  top->value = v2;
		  top->unsignedp = unsigned2;
		  break;
	       case '(':
	       case '?':
		  cpp_error(pfile, "syntax error in #if");
		  goto syntax_error;
	       case ':':
		  if (top[0].op != '?')
		    {
		       cpp_error(pfile,
				 "syntax error ':' without preceding '?'");
		       goto syntax_error;
		    }
		  else if (!(top[1].flags & HAVE_VALUE)
			   || !(top[-1].flags & HAVE_VALUE)
			   || !(top[0].flags & HAVE_VALUE))
		    {
		       cpp_error(pfile, "bad syntax for ?: operator");
		       goto syntax_error;
		    }
		  else
		    {
		       top--;
		       if (top->value)
			  skip_evaluation--;
		       top->value = top->value ? v1 : v2;
		       top->unsignedp = unsigned1 || unsigned2;
		    }
		  break;
	       case ')':
		  if ((top[1].flags & HAVE_VALUE)
		      || !(top[0].flags & HAVE_VALUE)
		      || top[0].op != '(' || (top[-1].flags & HAVE_VALUE))
		    {
		       cpp_error(pfile, "mismatched parentheses in #if");
		       goto syntax_error;
		    }
		  else
		    {
		       top--;
		       top->value = v1;
		       top->unsignedp = unsigned1;
		       top->flags |= HAVE_VALUE;
		    }
		  break;
	       default:
		  fprintf(stderr,
			  top[1].op >= ' ' && top[1].op <= '~'
			  ? "unimplemented operator '%c'\n"
			  : "unimplemented operator '\\%03o'\n", top[1].op);
	       }
	  }
	if (op.op == 0)
	  {
	     if (top != stack)
		cpp_error(pfile, "internal error in #if expression");
	     if (stack != init_stack)
		free(stack);
	     return top->value;
	  }
	top++;

	/* Check for and handle stack overflow. */
	if (top == limit)
	  {
	     struct operation   *new_stack;
	     int                 old_size = (char *)limit - (char *)stack;
	     int                 new_size = 2 * old_size;

	     if (stack != init_stack)
		new_stack = (struct operation *)xrealloc(stack, new_size);
	     else
	       {
		  new_stack = (struct operation *)xmalloc(new_size);
		  memcpy((char *)new_stack, (char *)stack, old_size);
	       }
	     stack = new_stack;
	     top = (struct operation *)((char *)new_stack + old_size);
	     limit = (struct operation *)((char *)new_stack + new_size);
	  }
	top->flags = flags;
	top->rprio = rprio;
	top->op = op.op;
	if ((op.op == OROR && top[-1].value)
	    || (op.op == ANDAND && !top[-1].value)
	    || (op.op == '?' && !top[-1].value))
	  {
	     skip_evaluation++;
	  }
	else if (op.op == ':')
	  {
	     if (top[-2].value)	/* Was condition true? */
		skip_evaluation++;
	     else
		skip_evaluation--;
	  }
     }
 syntax_error:
   if (stack != init_stack)
      free(stack);
   skip_rest_of_line(pfile);
   return 0;
}
