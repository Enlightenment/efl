/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iconv.h>

#include "Ecore_Txt.h"
#include "ecore_txt_private.h"


/**
 * To be documented.
 * 
 * FIXME: Finish this.
 */
EAPI char *
ecore_txt_convert(const char *enc_from, const char *enc_to, const char *text)
{
   iconv_t ic;
   char *new_txt, *inp, *outp;
   size_t inb, outb, outlen, tob, outalloc;
   
   if (!text) return NULL;
   ic = iconv_open(enc_to, enc_from);
   if (ic == (iconv_t)(-1)) return NULL;
   new_txt  = malloc(64);
   inb      = strlen(text);
   outb     = 64;
   inp      = (char*)text;
   outp     = new_txt;
   outalloc = 64;
   outlen   = 0;
   tob      = 0;

   for (;;)
     {
	size_t count;

	tob = outb;
	count = iconv(ic, &inp, &inb, &outp, &outb);
	outlen += tob - outb;
	if (count == (size_t)(-1))
	  {
	     if (errno == E2BIG)
	       {
		  new_txt = realloc(new_txt, outalloc + 64);
		  outp = new_txt + outlen;
		  outalloc += 64;
		  outb += 64;
	       }
	     else if (errno == EILSEQ)
	       {
		  if (new_txt) free(new_txt);
		  new_txt = NULL;
		  break;
	       }
	     else if (errno == EINVAL)
	       {
		  if (new_txt) free(new_txt);
		  new_txt = NULL;
		  break;
	       }
	     else
	       {
		  if (new_txt) free(new_txt);
		  new_txt = NULL;
		  break;
	       }
	  }
	if (inb == 0)
	  {
	     if (outalloc == outlen) new_txt = realloc(new_txt, outalloc + 1);
	     new_txt[outlen] = 0;
	     break;
	  }
     }
   iconv_close(ic);
   return new_txt;
}
