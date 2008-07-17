/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>

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

   current->flags.converted = 0;
   current->flags.is_float = 0;

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

static inline int
_eet_dictionary_string_get_me_cache(const char *s, int len, int *mantisse, int *exponent)
{
   if ((len == 6) && (s[0] == '0') && (s[1] == 'x') && (s[3] == 'p'))
     {
        *mantisse = (s[2] >= 'a') ? (s[2] - 'a' + 10) : (s[2] - '0');
        *exponent = (s[5] - '0');

        return -1;
     }
   return 0;
}

static inline int
_eet_dictionary_string_get_float_cache(const char *s, int len, float *result)
{
   int  mantisse;
   int  exponent;

   if (_eet_dictionary_string_get_me_cache(s, len, &mantisse, &exponent))
     {
        if (s[4] == '+')        *result = (float) (mantisse << exponent);
        else                    *result = (float) mantisse / (float) (1 << exponent);

        return -1;
     }
   return 0;
}

static inline int
_eet_dictionary_string_get_double_cache(const char *s, int len, double *result)
{
   int  mantisse;
   int  exponent;

   if (_eet_dictionary_string_get_me_cache(s, len, &mantisse, &exponent))
     {
        if (s[4] == '+')        *result = (double) (mantisse << exponent);
        else                    *result = (double) mantisse / (float) (1 << exponent);

        return -1;
     }
   return 0;
}

int
eet_dictionary_string_get_float(const Eet_Dictionary *ed, int index, float *result)
{
   if (!result) return 0;
   if (!ed) return 0;
   if (index < 0) return 0;
   if (index < ed->count)
     {
        if (!(ed->all[index].flags.converted
              && ed->all[index].flags.is_float))
          {
             const char      *str;

             str = ed->all[index].str ? ed->all[index].str : ed->all[index].mmap;

             if (!_eet_dictionary_string_get_float_cache(str, ed->all[index].len, &ed->all[index].convert.f))
               {
                  long long    mantisse = 0;
                  long         exponent = 0;

                  if (!_eet_string_to_double_convert(str, &mantisse, &exponent))
                    return 0;

                  ed->all[index].convert.f = ldexpf((float) mantisse, exponent);
               }

             ed->all[index].flags.is_float = 1;
          }

        *result = ed->all[index].convert.f;
        return -1;
     }
   return 0;
}

int
eet_dictionary_string_get_double(const Eet_Dictionary *ed, int index, double *result)
{
   if (!result) return 0;
   if (!ed) return 0;
   if (index < 0) return 0;
   if (index < ed->count)
     {
        if (!(ed->all[index].flags.converted
              && !ed->all[index].flags.is_float))
          {
             const char      *str;

             str = ed->all[index].str ? ed->all[index].str : ed->all[index].mmap;

             if (!_eet_dictionary_string_get_double_cache(str, ed->all[index].len, &ed->all[index].convert.d))
               {
                  long long    mantisse = 0;
                  long         exponent = 0;

                  if (!_eet_string_to_double_convert(str, &mantisse, &exponent))
                    return 0;

                  ed->all[index].convert.d = ldexp((double) mantisse, exponent);
               }

             ed->all[index].flags.is_float = 0;
          }

        *result = ed->all[index].convert.d;
        return -1;
     }
   return 0;
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
