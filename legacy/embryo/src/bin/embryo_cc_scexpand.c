/* expand.c -- Byte Pair Encoding decompression */
/* Copyright 1996 Philip Gage */

/* Byte Pair Compression appeared in the September 1997
 * issue of C/C++ Users Journal. The original source code
 * may still be found at the web site of the magazine
 * (www.cuj.com).
 *
 * The decompressor has been modified by me (Thiadmer
 * Riemersma) to accept a string as input, instead of a
 * complete file.
 */

/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "embryo_cc_sc.h"

#define STACKSIZE 16

int
strexpand(char *dest, unsigned char *source, int maxlen, unsigned char pairtable[128][2])
{
   unsigned char       stack[STACKSIZE];
   short               c, top = 0;
   int                 len;

   len = 1;			/* already 1 byte for '\0' */
   for (;;)
     {
	/* Pop byte from stack or read byte from the input string */
	if (top)
	  c = stack[--top];
	else if ((c = *(unsigned char *)source++) == '\0')
	  break;

	/* Push pair on stack or output byte to the output string */
	if (c > 127)
	  {
	     stack[top++] = pairtable[c - 128][1];
	     stack[top++] = pairtable[c - 128][0];
	  }
	else
	  {
	     len++;
	     if (maxlen > 1)
	       {
		  *dest++ = (char)c;
		  maxlen--;
	       }
	  }
     }
   *dest = '\0';
   return len;
}
