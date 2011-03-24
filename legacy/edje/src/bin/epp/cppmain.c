/* CPP main program, using CPP Library.
 * Copyright (C) 1995 Free Software Foundation, Inc.
 * Written by Per Bothner, 1994-95.
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
 * what you give them.   Help stamp out software-hoarding!  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cpplib.h"

#define EPP_DEBUG 0

cpp_reader          parse_in;
cpp_options         options;

int
main(int argc, char **argv)
{
   char               *p;
   int                 i;
   int                 argi = 1;	/* Next argument to handle. */
   struct cpp_options *opts = &options;
   enum cpp_token      kind;
   int                 got_text;

   p = argv[0] + strlen(argv[0]);
#ifndef __EMX__
   while (p != argv[0] && p[-1] != '/')
#else
   while (p != argv[0] && p[-1] != '/' && p[-1] != '\\')
#endif
      --p;
   progname = p;

   init_parse_file(&parse_in);
   parse_in.data = opts;

   init_parse_options(opts);

   argi += cpp_handle_options(&parse_in, argc - argi, argv + argi);
   if (argi < argc)
      cpp_fatal("Invalid option `%s'", argv[argi]);
   parse_in.show_column = 1;

   i = push_parse_file(&parse_in, opts->in_fname);
   if (i != SUCCESS_EXIT_CODE)
      return i;

   /* Now that we know the input file is valid, open the output.  */

   if (!opts->out_fname || !strcmp(opts->out_fname, ""))
      opts->out_fname = "stdout";
   else if (!freopen(opts->out_fname, "w", stdout))
      cpp_pfatal_with_name(&parse_in, opts->out_fname);

   got_text = 0;
   for (i = 0;; i++)
     {
	kind = cpp_get_token(&parse_in);
#if EPP_DEBUG
	fprintf(stderr, "%03d: kind=%d len=%d out=%d text=%d\n", i,
		kind, CPP_WRITTEN(&parse_in), !opts->no_output, got_text);
#endif
	switch (kind)
	  {
	  case CPP_EOF:
	     goto done;

	  case CPP_HSPACE:
	     continue;

	  case CPP_VSPACE:
	     break;

	  default:
	  case CPP_OTHER:
	  case CPP_NAME:
	  case CPP_NUMBER:
	  case CPP_CHAR:
	  case CPP_STRING:
	  case CPP_LPAREN:
	  case CPP_RPAREN:
	  case CPP_LBRACE:
	  case CPP_RBRACE:
	  case CPP_COMMA:
	  case CPP_SEMICOLON:
	  case CPP_3DOTS:
	     got_text = 1;
	     continue;

	  case CPP_COMMENT:
	  case CPP_DIRECTIVE:
	  case CPP_POP:
	     continue;
	  }
#if EPP_DEBUG
	fprintf(stderr, "'");
	fwrite(parse_in.token_buffer, 1, CPP_WRITTEN(&parse_in), stderr);
	fprintf(stderr, "'\n");
#endif
	if (!opts->no_output)
	  {
	     size_t n;

	     n = CPP_WRITTEN(&parse_in);
	     if (fwrite(parse_in.token_buffer, 1, n, stdout) != n)
		exit(FATAL_EXIT_CODE);
	  }
        /*
      next:
	parse_in.limit = parse_in.token_buffer;
	got_text = 0;
         */
     }

 done:
   cpp_finish(&parse_in);

   if (parse_in.errors)
      exit(FATAL_EXIT_CODE);
   exit(SUCCESS_EXIT_CODE);
}
