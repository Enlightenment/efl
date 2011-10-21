#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Eina.h>

#include "Eet.h"
#include "Eet_private.h"

Eet_Dictionary *
eet_dictionary_add(void)
{
   Eet_Dictionary *new;

   new = calloc(1, sizeof (Eet_Dictionary));
   if (!new)
     return NULL;

   memset(new->hash, -1, sizeof (int) * 256);

   return new;
} /* eet_dictionary_add */

void
eet_dictionary_free(Eet_Dictionary *ed)
{
   if (ed)
     {
        int i;

        for (i = 0; i < ed->count; ++i)
          if (ed->all[i].allocated)
            eina_stringshare_del(ed->all[i].str);

        if (ed->all)
          free(ed->all);

        if (ed->converts) eina_hash_free(ed->converts);

        free(ed);
     }
} /* eet_dictionary_free */

static int
_eet_dictionary_lookup(Eet_Dictionary *ed,
                       const char     *string,
                       int             len,
                       int             hash)
{
   Eina_Bool found = EINA_FALSE;
   int prev = -1;
   int current;

   current = ed->hash[hash];

   while (current != -1)
     {
        if (ed->all[current].len == len)
          {
             if (ed->all[current].str &&
                 (ed->all[current].str == string || strcmp(ed->all[current].str, string) == 0))
               {
                  found = EINA_TRUE;
                  break;
               }
          }

        prev = current;
        current = ed->all[current].next;
     }

   if (current == -1 && found)
     return prev;

   return current;
} /* _eet_dictionary_lookup */

int
eet_dictionary_string_add(Eet_Dictionary *ed,
                          const char     *string)
{
   Eet_String *current;
   const char *str;
   int hash;
   int idx;
   int len;

   if (!ed)
     return -1;

   hash = _eet_hash_gen(string, 8);
   len = strlen(string) + 1;

   idx = _eet_dictionary_lookup(ed, string, len, hash);

   if (idx != -1)
     {
        if (ed->all[idx].str && (ed->all[idx].str == string || strcmp(ed->all[idx].str, string) == 0))
          return idx;
     }

   if (ed->total == ed->count)
     {
        Eet_String *new;
        int total;

        total = ed->total + 8;

        new = realloc(ed->all, sizeof (Eet_String) * total);
        if (!new)
          return -1;

        ed->all = new;
        ed->total = total;
     }

   str = eina_stringshare_add(string);
   if (!str)
     return -1;

   current = ed->all + ed->count;

   current->allocated = EINA_TRUE;

   current->hash = hash;

   current->str = str;
   current->len = len;

   if (idx == -1)
     {
        current->next = ed->hash[hash];
        current->prev = -1;
        ed->hash[hash] = ed->count;
     }
   else
     {
        current->next = idx;
        current->prev = ed->all[idx].prev;

        if (current->next != -1)
          ed->all[current->next].prev = ed->count;

        if (current->prev != -1)
          ed->all[current->prev].next = ed->count;
        else
          ed->hash[hash] = ed->count;
     }

   return ed->count++;
} /* eet_dictionary_string_add */

int
eet_dictionary_string_get_size(const Eet_Dictionary *ed,
                               int                   idx)
{
   if (!ed)
     return 0;

   if (idx < 0)
     return 0;

   if (idx < ed->count)
     return ed->all[idx].len;

   return 0;
} /* eet_dictionary_string_get_size */

int
eet_dictionary_string_get_hash(const Eet_Dictionary *ed,
                               int                   idx)
{
   if (!ed)
     return -1;

   if (idx < 0)
     return -1;

   if (idx < ed->count)
     return ed->all[idx].hash;

   return -1;
} /* eet_dictionary_string_get_hash */

const char *
eet_dictionary_string_get_char(const Eet_Dictionary *ed,
                               int                   idx)
{
   if (!ed)
     return NULL;

   if (idx < 0)
     return NULL;

   if (idx < ed->count)
     {
#ifdef _WIN32
        /* Windows file system could change the mmaped file when replacing a file. So we need to copy all string in memory to avoid bugs. */
        if (!ed->all[idx].allocated)
          {
             ed->all[idx].str = eina_stringshare_add(ed->all[idx].str);
             ed->all[idx].allocated = EINA_TRUE;
          }
#endif /* ifdef _WIN32 */
        return ed->all[idx].str;
     }

   return NULL;
} /* eet_dictionary_string_get_char */

static inline Eina_Bool
_eet_dictionary_string_get_me_cache(const char *s,
                                    int         len,
                                    int        *mantisse,
                                    int        *exponent)
{
   if ((len == 6) && (s[0] == '0') && (s[1] == 'x') && (s[3] == 'p'))
     {
        *mantisse = (s[2] >= 'a') ? (s[2] - 'a' + 10) : (s[2] - '0');
        *exponent = (s[5] - '0');

        return EINA_TRUE;
     }

   return EINA_FALSE;
} /* _eet_dictionary_string_get_me_cache */

static inline Eina_Bool
_eet_dictionary_string_get_float_cache(const char *s,
                                       int         len,
                                       float      *result)
{
   int mantisse;
   int exponent;

   if (_eet_dictionary_string_get_me_cache(s, len, &mantisse, &exponent))
     {
        if (s[4] == '+')
          *result = (float)(mantisse << exponent);
        else
          *result = (float)mantisse / (float)(1 << exponent);

        return EINA_TRUE;
     }

   return EINA_FALSE;
} /* _eet_dictionary_string_get_float_cache */

static inline Eina_Bool
_eet_dictionary_string_get_double_cache(const char *s,
                                        int         len,
                                        double     *result)
{
   int mantisse;
   int exponent;

   if (_eet_dictionary_string_get_me_cache(s, len, &mantisse, &exponent))
     {
        if (s[4] == '+')
          *result = (double)(mantisse << exponent);
        else
          *result = (double)mantisse / (float)(1 << exponent);

        return EINA_TRUE;
     }

   return EINA_FALSE;
} /* _eet_dictionary_string_get_double_cache */

static inline Eina_Bool
_eet_dictionary_test(const Eet_Dictionary *ed,
                     int                   idx,
                     void                 *result)
{
   if (!result)
     return EINA_FALSE;

   if (!ed)
     return EINA_FALSE;

   if (idx < 0)
     return EINA_FALSE;

   if (!(idx < ed->count))
     return EINA_FALSE;

   return EINA_TRUE;
} /* _eet_dictionary_test */

static Eet_Convert *
eet_dictionary_convert_get(const Eet_Dictionary *ed,
                           int                   idx,
                           const char          **str)
{
   Eet_Convert *result;

   *str = ed->all[idx].str;

   if (!ed->converts)
     {
        ((Eet_Dictionary *)ed)->converts = eina_hash_int32_new(free);

        goto add_convert;
     }

   result = eina_hash_find(ed->converts, &idx);
   if (result) return result;

add_convert:
   result = calloc(1, sizeof (Eet_Convert));

   eina_hash_add(ed->converts, &idx, result);
   return result;
}

Eina_Bool
eet_dictionary_string_get_float(const Eet_Dictionary *ed,
                                int                   idx,
                                float                *result)
{
   Eet_Convert *convert;
   const char *str;

   if (!_eet_dictionary_test(ed, idx, result))
     return EINA_FALSE;

   convert = eet_dictionary_convert_get(ed, idx, &str);
   if (!convert) return EINA_FALSE;

   if (!(convert->type & EET_D_FLOAT))
     {
        if (!_eet_dictionary_string_get_float_cache(str, ed->all[idx].len,
                                                    &convert->f))
          {
             long long mantisse = 0;
             long exponent = 0;

             if (eina_convert_atod(str, ed->all[idx].len, &mantisse,
                                   &exponent) == EINA_FALSE)
               return EINA_FALSE;

             convert->f = ldexpf((float)mantisse, exponent);
          }

        convert->type |= EET_D_FLOAT;
     }

   *result = convert->f;
   return EINA_TRUE;
} /* eet_dictionary_string_get_float */

Eina_Bool
eet_dictionary_string_get_double(const Eet_Dictionary *ed,
                                 int                   idx,
                                 double               *result)
{
   Eet_Convert *convert;
   const char *str;

   if (!_eet_dictionary_test(ed, idx, result))
     return EINA_FALSE;

   convert = eet_dictionary_convert_get(ed, idx, &str);
   if (!convert) return EINA_FALSE;

   if (!(convert->type & EET_D_DOUBLE))
     {
        if (!_eet_dictionary_string_get_double_cache(str, ed->all[idx].len,
                                                     &convert->d))
          {
             long long mantisse = 0;
             long exponent = 0;

             if (eina_convert_atod(str, ed->all[idx].len, &mantisse,
                                   &exponent) == EINA_FALSE)
               return EINA_FALSE;

             convert->d = ldexp((double)mantisse, exponent);
          }

        convert->type |= EET_D_DOUBLE;
     }

   *result = convert->d;
   return EINA_TRUE;
} /* eet_dictionary_string_get_double */

Eina_Bool
eet_dictionary_string_get_fp(const Eet_Dictionary *ed,
                             int                   idx,
                             Eina_F32p32          *result)
{
   Eet_Convert *convert;
   const char *str;

   if (!_eet_dictionary_test(ed, idx, result))
     return EINA_FALSE;

   convert = eet_dictionary_convert_get(ed, idx, &str);
   if (!convert) return EINA_FALSE;

   if (!(convert->type & EET_D_FIXED_POINT))
     {
        Eina_F32p32 fp;

        if (!eina_convert_atofp(str, ed->all[idx].len, &fp))
          return EINA_FALSE;

        convert->fp = fp;
        convert->type |= EET_D_FIXED_POINT;
     }

   *result = convert->fp;
   return EINA_TRUE;
} /* eet_dictionary_string_get_fp */

EAPI int
eet_dictionary_string_check(Eet_Dictionary *ed,
                            const char     *string)
{
   int i;

   if ((!ed) || (!string))
     return 0;

   if ((ed->start <= string) && (string < ed->end))
     return 1;

   for (i = 0; i < ed->count; ++i)
     if ((ed->all[i].allocated) && ed->all[i].str == string)
       return 1;

   return 0;
} /* eet_dictionary_string_check */

