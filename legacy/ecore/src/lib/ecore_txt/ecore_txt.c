#include "ecore_txt_private.h"
#include "Ecore_Txt.h"

#include <iconv.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

char *
ecore_txt_convert(char *enc_from, char *enc_to, char *text)
{
   iconv_t ic;
   char *new_txt, *inp, *outp;
   size_t inb, outb, outlen, tob, outalloc;
   
   if (!text) return strdup("");
   ic = iconv_open(enc_to, enc_from);
   if (!ic) return strdup("");
   new_txt  = malloc(64);
   inb      = strlen(text);
   outb     = 64;
   inp      = text;
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
	if (count == -1)
	  {
	     if (errno == E2BIG)
	       {
		  new_txt = realloc(new_txt, outalloc + 64);
		  outalloc += 64;
		  outb += 64;
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
