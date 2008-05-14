/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "embryo_private.h"
#include <fnmatch.h>

#define STRGET(ep, str, par) { \
   Embryo_Cell *___cptr; \
   str = NULL; \
   if ((___cptr = embryo_data_address_get(ep, par))) { \
	int ___l; \
	___l = embryo_data_string_length_get(ep, ___cptr); \
	(str) = alloca(___l + 1); \
	if (str) embryo_data_string_get(ep, ___cptr, str); \
     } }
#define STRSET(ep, par, str) { \
   Embryo_Cell *___cptr; \
   if ((___cptr = embryo_data_address_get(ep, par))) { \
      embryo_data_string_set(ep, str, ___cptr); \
   } }

/* exported string api */

static Embryo_Cell
_embryo_str_atoi(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1;

   /* params[1] = str */
   if (params[0] != (1 * sizeof(Embryo_Cell))) return 0;
   STRGET(ep, s1, params[1]);
   if (!s1) return 0;
   return (Embryo_Cell)atoi(s1);
}

static Embryo_Cell
_embryo_str_fnmatch(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2;

   /* params[1] = glob */
   /* params[2] = str */
   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   STRGET(ep, s1, params[1]);
   STRGET(ep, s2, params[2]);
   if ((!s1) || (!s2)) return -1;
   return (Embryo_Cell)fnmatch(s1, s2, 0);
}

static Embryo_Cell
_embryo_str_strcmp(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2;

   /* params[1] = str1 */
   /* params[2] = str2 */
   if (params[0] != (2 * sizeof(Embryo_Cell))) return -1;
   STRGET(ep, s1, params[1]);
   STRGET(ep, s2, params[2]);
   if ((!s1) || (!s2)) return -1;
   return (Embryo_Cell)strcmp(s1, s2);
}

static Embryo_Cell
_embryo_str_strncmp(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2;

   /* params[1] = str1 */
   /* params[2] = str2 */
   /* params[3] = n */
   if (params[0] != (3 * sizeof(Embryo_Cell))) return 0;
   if (params[3] < 0) params[3] = 0;
   STRGET(ep, s1, params[1]);
   STRGET(ep, s2, params[2]);
   if ((!s1) || (!s2)) return -1;
   return (Embryo_Cell)strncmp(s1, s2, (size_t)params[3]);
}

static Embryo_Cell
_embryo_str_strcpy(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1;

   /* params[1] = dst */
   /* params[2] = str */
   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   STRGET(ep, s1, params[2]);
   if (!s1) return 0;
   STRSET(ep, params[1], s1);
   return 0;
}

static Embryo_Cell
_embryo_str_strncpy(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1;
   int l;

   /* params[1] = dst */
   /* params[2] = str */
   /* params[3] = n */
   if (params[0] != (3 * sizeof(Embryo_Cell))) return 0;
   if (params[3] < 0) params[3] = 0;
   STRGET(ep, s1, params[2]);
   if (!s1) return 0;
   l = strlen(s1);
   if (l > params[3]) s1[params[3]] = 0;
   STRSET(ep, params[1], s1);
   return 0;
}

static Embryo_Cell
_embryo_str_strlen(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1;

   /* params[1] = str */
   if (params[0] != (1 * sizeof(Embryo_Cell))) return 0;
   STRGET(ep, s1, params[1]);
   if (!s1) return 0;
   return (Embryo_Cell)strlen(s1);
}

static Embryo_Cell
_embryo_str_strcat(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2, *s3;

   /* params[1] = dsr */
   /* params[2] = str */
   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   STRGET(ep, s1, params[1]);
   STRGET(ep, s2, params[2]);
   if ((!s1) || (!s2)) return 0;
   s3 = alloca(strlen(s1) + strlen(s2) + 1);
   if (!s3) return 0;
   strcpy(s3, s1);
   strcat(s3, s2);
   STRSET(ep, params[1], s3);
   return 0;
}

static Embryo_Cell
_embryo_str_strncat(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2, *s3;
   int l1, l2;

   /* params[1] = dst */
   /* params[2] = str */
   /* params[3] = n */
   if (params[0] != (3 * sizeof(Embryo_Cell))) return 0;
   if (params[3] < 0) params[3] = 0;
   STRGET(ep, s1, params[1]);
   STRGET(ep, s2, params[2]);
   if ((!s1) || (!s2)) return 0;
   l1 = strlen(s1);
   l2 = strlen(s2);
   s3 = alloca(l1 + l2 + 1);
   if (!s3) return 0;
   strcpy(s3, s1);
   strncat(s3, s2, params[3]);
   if (l2 >= params[3]) s3[l1 + params[3]] = 0;
   STRSET(ep, params[1], s3);
   return 0;
}

static Embryo_Cell
_embryo_str_strprep(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2, *s3;

   /* params[1] = dst */
   /* params[2] = str */
   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   STRGET(ep, s1, params[1]);
   STRGET(ep, s2, params[2]);
   if ((!s1) || (!s2)) return 0;
   s3 = alloca(strlen(s1) + strlen(s2) + 1);
   if (!s3) return 0;
   strcpy(s3, s2);
   strcat(s3, s1);
   STRSET(ep, params[1], s3);
   return 0;
}

static Embryo_Cell
_embryo_str_strnprep(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2, *s3;
   int l1, l2;

   /* params[1] = dst */
   /* params[2] = str */
   /* params[3] = n */
   if (params[0] != (3 * sizeof(Embryo_Cell))) return 0;
   if (params[3] < 0) params[3] = 0;
   STRGET(ep, s1, params[1]);
   STRGET(ep, s2, params[2]);
   if ((!s1) || (!s2)) return 0;
   l1 = strlen(s1);
   l2 = strlen(s2);
   s3 = alloca(l1 + l2 + 1);
   if (!s3) return 0;
   strncpy(s3, s2, params[3]);
   if (params[3] <= l2) s3[params[3]] = 0;
   strcat(s3, s1);
   STRSET(ep, params[1], s3);
   return 0;
}

static Embryo_Cell
_embryo_str_strcut(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2;
   int l1;

   /* params[1] = dst */
   /* params[2] = str */
   /* params[3] = n */
   /* params[4] = n2 */
   if (params[0] != (4 * sizeof(Embryo_Cell))) return 0;
   if (params[3] < 0) params[3] = 0;
   if (params[4] < params[3]) params[4] = params[3];
   STRGET(ep, s1, params[2]);
   if (!s1) return 0;
   l1 = strlen(s1);
   if (params[3] >= l1) params[3] = l1;
   if (params[4] >= l1) params[4] = l1;
   if (params[4] == params[3])
     {
	STRSET(ep, params[1], "");
	return 0;
     }
   s2 = alloca(params[4] - params[3] + 1);
   strncpy(s2, s1 + params[3], params[4] - params[3]);
   s2[params[4] - params[3]] = 0;
   STRSET(ep, params[1], s2);
   return 0;
}

static Embryo_Cell
_embryo_str_snprintf(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2;
   int i, o;
   int inesc = 0;
   int insub = 0;
   int p, pnum;

   /* params[1] = buf */
   /* params[2] = bufsize */
   /* params[3] = format_string */
   /* params[4] = first arg ... */
   if (params[0] < (3 * sizeof(Embryo_Cell))) return 0;
   if (params[2] <= 0) return 0;
   STRGET(ep, s1, params[3]);
   if (!s1) return -1;
   s2 = alloca(params[2] + 1);
   if (!s2) return -1;
   s2[0] = 0;
   pnum = (params[0] / sizeof(Embryo_Cell)) - 3;
   for (p = 0, o = 0, i = 0; (s1[i]) && (o < (params[2] - 1)) && (p < (pnum + 1)); i++)
     {
	if ((!inesc) && (!insub))
	  {
	     if      (s1[i] == '\\') inesc = 1;
	     else if (s1[i] == '%')  insub = 1;
	     if ((!inesc) && (!insub))
	       {
		  s2[o] = s1[i];
		  o++;
	       }
	  }
	else
	  {
	     Embryo_Cell *cptr;

	     if (inesc)
	       {
		  switch (s1[i])
		    {
		     case 't':
		       s2[o] = '\t';
		       o++;
		       break;
		     case 'n':
		       s2[o] = '\n';
		       o++;
		       break;
		     default:
		       s2[o] = s1[i];
		       o++;
		       break;
		    }
		  inesc = 0;
	       }
	     if ((insub) && (s1[i] == '%')) pnum++;
	     if ((insub) && (p < pnum))
	       {
		  switch (s1[i])
		    {
		     case '%':
		       s2[o] = '%';
		       o++;
		       break;
		     case 'c':
		       cptr = embryo_data_address_get(ep, params[4 + p]);
		       if (cptr) s2[o] = (char)(*cptr);
		       p++;
		       o++;
		       break;
		     case 'i':
		     case 'd':
		     case 'x':
		     case 'X':
			 {
			    char fmt[10] = "";
			    char tmp[256] = "";
			    int l;

			    if      (s1[i] == 'i') strcpy(fmt, "%i");
			    else if (s1[i] == 'd') strcpy(fmt, "%d");
			    else if (s1[i] == 'x') strcpy(fmt, "%x");
			    else if (s1[i] == 'X') strcpy(fmt, "%08x");
			    cptr = embryo_data_address_get(ep, params[4 + p]);
			    if (cptr) snprintf(tmp, sizeof(tmp), fmt, (int)(*cptr));
			    l = strlen(tmp);
			    if ((o + l) > (params[2] - 1))
			      {
				 l = params[2] - 1 - o;
				 if (l < 0) l = 0;
				 tmp[l] = 0;
			      }
			    strcpy(s2 + o, tmp);
			    o += l;
			    p++;
			 }
		       break;
		     case 'f':
			 {
			    char tmp[256] = "";
			    int l;

			    cptr = embryo_data_address_get(ep, params[4 + p]);
			    if (cptr) snprintf(tmp, sizeof(tmp), "%f", (double)EMBRYO_CELL_TO_FLOAT(*cptr));
			    l = strlen(tmp);
			    if ((o + l) > (params[2] - 1))
			      {
				 l = params[2] - 1 - o;
				 if (l < 0) l = 0;
				 tmp[l] = 0;
			      }
			    strcpy(s2 + o, tmp);
			    o += l;
			    p++;
			 }
		       break;
		     case 's':
			 {
			    char *tmp;
			    int l;

			    STRGET(ep, tmp, params[4 + p]);
			    l = strlen(tmp);
			    if ((o + l) > (params[2] - 1))
			      {
				 l = params[2] - 1 - o;
				 if (l < 0) l = 0;
				 tmp[l] = 0;
			      }
			    strcpy(s2 + o, tmp);
			    o += l;
			    p++;
			 }
		       break;
		     default:
		       break;
		    }
		  insub = 0;
	       }
	     else if (insub)
	       insub = 0;
	  }
     }
   s2[o] = 0;

   STRSET(ep, params[1], s2);
   return o;
}

static Embryo_Cell
_embryo_str_strstr(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2, *p;

   /* params[1] = str */
   /* params[2] = ndl */
   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   STRGET(ep, s1, params[1]);
   STRGET(ep, s2, params[2]);
   if ((!s1) || (!s2)) return -1;
   p = strstr(s1, s2);
   if (p == NULL) return -1;
   return (Embryo_Cell)(p - s1);
}

static Embryo_Cell
_embryo_str_strchr(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2, *p;

   /* params[1] = str */
   /* params[2] = ch */
   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   STRGET(ep, s1, params[1]);
   STRGET(ep, s2, params[2]);
   p = strchr(s1, s2[0]);
   if (p == NULL) return -1;
   return (Embryo_Cell)(p - s1);
}

static Embryo_Cell
_embryo_str_strrchr(Embryo_Program *ep, Embryo_Cell *params)
{
   char *s1, *s2, *p;

   /* params[1] = str */
   /* params[2] = ch */
   if (params[0] != (2 * sizeof(Embryo_Cell))) return 0;
   STRGET(ep, s1, params[1]);
   STRGET(ep, s2, params[2]);
   p = strrchr(s1, s2[0]);
   if (p == NULL) return -1;
   return (Embryo_Cell)(p - s1);
}

/* functions used by the rest of embryo */

void
_embryo_str_init(Embryo_Program *ep)
{
   embryo_program_native_call_add(ep, "atoi",     _embryo_str_atoi);
   embryo_program_native_call_add(ep, "fnmatch",  _embryo_str_fnmatch);
   embryo_program_native_call_add(ep, "strcmp",   _embryo_str_strcmp);
   embryo_program_native_call_add(ep, "strncmp",  _embryo_str_strncmp);
   embryo_program_native_call_add(ep, "strcpy",   _embryo_str_strcpy);
   embryo_program_native_call_add(ep, "strncpy",  _embryo_str_strncpy);
   embryo_program_native_call_add(ep, "strlen",   _embryo_str_strlen);
   embryo_program_native_call_add(ep, "strcat",   _embryo_str_strcat);
   embryo_program_native_call_add(ep, "strncat",  _embryo_str_strncat);
   embryo_program_native_call_add(ep, "strprep",  _embryo_str_strprep);
   embryo_program_native_call_add(ep, "strnprep", _embryo_str_strnprep);
   embryo_program_native_call_add(ep, "strcut",   _embryo_str_strcut);
   embryo_program_native_call_add(ep, "snprintf", _embryo_str_snprintf);
   embryo_program_native_call_add(ep, "strstr",   _embryo_str_strstr);
   embryo_program_native_call_add(ep, "strchr",   _embryo_str_strchr);
   embryo_program_native_call_add(ep, "strrchr",  _embryo_str_strrchr);
}
