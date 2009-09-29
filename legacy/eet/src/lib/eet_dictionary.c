/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Eina.h>

#include "Eet.h"
#include "Eet_private.h"

Eet_Dictionary *
eet_dictionary_add(void)
{
   Eet_Dictionary       *new;

   new = calloc(1, sizeof (Eet_Dictionary));
   if (!new)
     return NULL;

   memset(new->hash, -1, sizeof (int) * 256);

   return new;
}

void
eet_dictionary_free(Eet_Dictionary *ed)
{
   if (ed)
     {
        int     i;

        for (i = 0; i < ed->count; ++i)
          if (ed->all[i].str)
            free(ed->all[i].str);
        if (ed->all) free(ed->all);
        free(ed);
     }
}

static int
_eet_dictionary_lookup(Eet_Dictionary *ed, const char *string, int hash)
{
   int  prev = -1;
   int  current;

   current = ed->hash[hash];

   while (current != -1)
     {
        if (ed->all[current].str)
          {
             if (strcmp(ed->all[current].str, string) >= 0)
               break ;
          }
        if (ed->all[current].mmap)
          {
             if (strcmp(ed->all[current].mmap, string) >= 0)
               break ;
          }

        prev = current;
        current = ed->all[current].next;
     }

   if (current == -1)
     return prev;

   return current;
}

int
eet_dictionary_string_add(Eet_Dictionary *ed, const char *string)
{
   Eet_String   *current;
   char         *str;
   int           hash;
   int           index;
   int           len;

   if (!ed)
     return -1;

   hash = _eet_hash_gen(string, 8);

   index = _eet_dictionary_lookup(ed, string, hash);

   if (index != -1)
     {
        if (ed->all[index].str)
          {
             if (strcmp(ed->all[index].str, string) == 0)
               return index;
          }
        if (ed->all[index].mmap)
          {
             if (strcmp(ed->all[index].mmap, string) == 0)
               return index;
          }
     }

   if (ed->total == ed->count)
     {
        Eet_String      *new;
        int              total;

        total = ed->total + 8;

        new = realloc(ed->all, sizeof (Eet_String) * total);
        if (new == NULL)
          return -1;

        ed->all = new;
        ed->total = total;
     }

   len = strlen(string) + 1;
   str = strdup(string);
   if (str == NULL)
     return -1;

   current = ed->all + ed->count;

   current->type = EET_D_NOT_CONVERTED;

   current->hash = hash;

   current->str = str;
   current->len = len;
   current->mmap = NULL;

   if (index == -1)
     {
        current->next = ed->hash[hash];
        current->prev = -1;
        ed->hash[hash] = ed->count;
     }
   else
     {
        current->next = index;
        current->prev = ed->all[index].prev;

        if (current->next != -1)
          ed->all[current->next].prev = ed->count;
        if (current->prev != -1)
          ed->all[current->prev].next = ed->count;
        else
          ed->hash[hash] = ed->count;
     }

   return ed->count++;
}

int
eet_dictionary_string_get_size(const Eet_Dictionary *ed, int index)
{
   if (!ed) return 0;
   if (index < 0) return 0;
   if (index < ed->count)
     return ed->all[index].len;
   return 0;
}

int
eet_dictionary_string_get_hash(const Eet_Dictionary *ed, int index)
{
   if (!ed) return -1;
   if (index < 0) return -1;
   if (index < ed->count)
     return ed->all[index].hash;
   return -1;
}

const char *
eet_dictionary_string_get_char(const Eet_Dictionary *ed, int index)
{
   if (!ed) return NULL;
   if (index < 0) return NULL;
   if (index < ed->count)
     {
#ifdef _WIN32
	/* Windows file system could change the mmaped file when replacing a file. So we need to copy all string in memory to avoid bugs. */
	if (ed->all[index].str == NULL)
	  {
	     ed->all[index].str = strdup(ed->all[index].mmap);
	     ed->all[index].mmap = NULL;
	  }
#else
        if (ed->all[index].mmap)
          return ed->all[index].mmap;
#endif
        return ed->all[index].str;
     }
   return NULL;
}

static inline Eina_Bool
_eet_dictionary_string_get_me_cache(const char *s, int len, int *mantisse, int *exponent)
{
   if ((len == 6) && (s[0] == '0') && (s[1] == 'x') && (s[3] == 'p'))
     {
        *mantisse = (s[2] >= 'a') ? (s[2] - 'a' + 10) : (s[2] - '0');
        *exponent = (s[5] - '0');

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static inline Eina_Bool
_eet_dictionary_string_get_float_cache(const char *s, int len, float *result)
{
   int  mantisse;
   int  exponent;

   if (_eet_dictionary_string_get_me_cache(s, len, &mantisse, &exponent))
     {
        if (s[4] == '+')        *result = (float) (mantisse << exponent);
        else                    *result = (float) mantisse / (float) (1 << exponent);

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static inline Eina_Bool
_eet_dictionary_string_get_double_cache(const char *s, int len, double *result)
{
   int  mantisse;
   int  exponent;

   if (_eet_dictionary_string_get_me_cache(s, len, &mantisse, &exponent))
     {
        if (s[4] == '+')        *result = (double) (mantisse << exponent);
        else                    *result = (double) mantisse / (float) (1 << exponent);

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static inline Eina_Bool
_eet_dictionary_test(const Eet_Dictionary *ed, int index, void *result)
{
   if (!result) return EINA_FALSE;
   if (!ed) return EINA_FALSE;
   if (index < 0) return EINA_FALSE;
   if (!(index < ed->count)) return EINA_FALSE;
   return EINA_TRUE;
}

Eina_Bool
eet_dictionary_string_get_float(const Eet_Dictionary *ed, int index, float *result)
{
   if (!_eet_dictionary_test(ed, index, result)) return EINA_FALSE;

   if (ed->all[index].type != EET_D_FLOAT)
     {
	const char      *str;

	str = ed->all[index].str ? ed->all[index].str : ed->all[index].mmap;

	if (!_eet_dictionary_string_get_float_cache(str, ed->all[index].len, &ed->all[index].convert.f))
	  {
	     long long    mantisse = 0;
	     long         exponent = 0;

	     if (eina_convert_atod(str, ed->all[index].len, &mantisse, &exponent) == EINA_FALSE)
	       return EINA_FALSE;

	     ed->all[index].convert.f = ldexpf((float) mantisse, exponent);
	  }

	ed->all[index].type = EET_D_FLOAT;
     }

   *result = ed->all[index].convert.f;
   return EINA_TRUE;
}

Eina_Bool
eet_dictionary_string_get_double(const Eet_Dictionary *ed, int index, double *result)
{
   if (!_eet_dictionary_test(ed, index, result)) return EINA_FALSE;

   if (ed->all[index].type != EET_D_DOUBLE)
     {
	const char      *str;

	str = ed->all[index].str ? ed->all[index].str : ed->all[index].mmap;

	if (!_eet_dictionary_string_get_double_cache(str, ed->all[index].len, &ed->all[index].convert.d))
	  {
	     long long    mantisse = 0;
	     long         exponent = 0;

	     if (eina_convert_atod(str, ed->all[index].len, &mantisse, &exponent) == EINA_FALSE)
	       return EINA_FALSE;

	     ed->all[index].convert.d = ldexp((double) mantisse, exponent);
	  }

	ed->all[index].type = EET_D_DOUBLE;
     }

   *result = ed->all[index].convert.d;
   return EINA_TRUE;
}

Eina_Bool
eet_dictionary_string_get_fp(const Eet_Dictionary *ed, int index, Eina_F32p32 *result)
{
   if (!_eet_dictionary_test(ed, index, result)) return EINA_FALSE;

   if (ed->all[index].type != EET_D_FIXED_POINT)
     {
	const char *str;
	Eina_F32p32 fp;

	str = ed->all[index].str ? ed->all[index].str : ed->all[index].mmap;

	if (!eina_convert_atofp(str,  ed->all[index].len, &fp))
	  return EINA_FALSE;

	ed->all[index].convert.fp = fp;
	ed->all[index].type = EET_D_FIXED_POINT;
     }

   *result = ed->all[index].convert.fp;
   return EINA_TRUE;
}

EAPI int
eet_dictionary_string_check(Eet_Dictionary *ed, const char *string)
{
   int	i;

   if (ed == NULL
       || string == NULL)
     return 0;

   if (ed->start <= string
       && string < ed->end)
     return 1;

   for (i = 0; i < ed->count; ++i)
     if (ed->all[i].str == string)
       return 1;

   return 0;
}
