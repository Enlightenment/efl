#include "e_str.h"
#include "e_mem.h"

/* string manipulation conveience calls */

void
e_string_free_list(char **slist, int num)
{
   int                 i;

   for (i = 0; i < num; i++)
      FREE(slist[i]);
   FREE(slist);
}

char               *
e_string_dup(char *str)
{
   if (!str)
      return NULL;
   return strdup(str);
}

int
e_string_cmp(char *s1, char *s2)
{
   if ((!s1) || (!s2))
      return 0;
   if (!strcmp(s1, s2))
      return 1;
   return 0;
}

int
e_string_case_cmp(char *s1, char *s2)
{
   if ((!s1) || (!s2))
      return 0;
   if (!strcasecmp(s1, s2))
      return 1;
   return 0;
}

int
e_string_length(char *str)
{
   return strlen(str);
}

void
e_string_clear(char *str)
{
   str[0] = 0;
}

void
e_string_cat(char *str, char *cat)
{
   strcat(str, cat);
}

void
e_string_cat_n(char *str, char *cat, int start, int len)
{
   strncat(str, &(cat[start]), len);
}

char               *
e_string_escape(char *str)
{
   char                buf[8192];
   int                 i, j, len;

   e_string_clear(buf);
   len = e_string_length(str);
   for (i = 0, j = 0; i < len; i++)
     {
	switch (str[i])
	  {
	  case ' ':
	  case '"':
	  case '\'':
	  case '\\':
	  case '(':
	  case ')':
	  case '[':
	  case ']':
	  case '$':
	  case '`':
	  case '~':
	  case '!':
	  case '#':
	  case '&':
	  case '*':
	  case '?':
	  case '|':
	  case ';':
	  case '<':
	  case '>':
	     buf[j++] = '\\';
	     buf[j++] = str[i];
	     break;
	  default:
	     buf[j++] = str[i];
	     break;
	  }
     }
   buf[j] = 0;
   return e_string_dup(buf);
}

char               *
e_string_build(char *format, char **rep_list, int rep_num)
{
   char                buf[16384], *s;
   int                 i;

   e_string_clear(buf);
   s = format;
   i = 0;
   while (*s)
     {
	if (s[0] == '%')
	  {
	     if (s[1] == '%')
		e_string_cat(buf, "%");
	     else
	       {
		  int                 j;

		  for (j = 0; j < rep_num; j += 2)
		    {
		       if (s[1] == rep_list[j][0])
			 {
			    char               *ss;

			    ss = e_string_escape(rep_list[j + 1]);
			    e_string_cat(buf, ss);
			    FREE(ss);
			    break;
			 }
		    }
	       }
	     s++;
	  }
	else
	   strncat(buf, s, 1);
	s++;
     }
   return e_string_dup(buf);
}
