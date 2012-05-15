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

   new = eet_dictionary_calloc(1);
   if (!new)
     return NULL;

   memset(new->hash, -1, sizeof (int) * 256);
   eina_lock_new(&new->mutex);

   return new;
}

void
eet_dictionary_free(Eet_Dictionary *ed)
{
   int i;

   if (!ed) return;

   eina_lock_free(&ed->mutex);

   for (i = 0; i < ed->count; ++i)
     if (ed->all[i].allocated)
       eina_stringshare_del(ed->all[i].str);

   if (ed->all)
     free(ed->all);

   if (ed->converts) eina_hash_free(ed->converts);

   eet_dictionary_mp_free(ed);
}

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
                 ((ed->all[current].str == string) ||
                     (!strcmp(ed->all[current].str, string))))
               {
                  found = EINA_TRUE;
                  break;
               }
          }

        prev = current;
        current = ed->all[current].next;
     }

   if ((current == -1) && found)
     return prev;

   return current;
}

int
eet_dictionary_string_add(Eet_Dictionary *ed,
                          const char     *string)
{
   Eet_String *current;
   const char *str;
   int hash;
   int idx;
   int len;
   int cnt;

   if (!ed)
     return -1;

   hash = _eet_hash_gen(string, 8);
   len = strlen(string) + 1;

   eina_lock_take(&ed->mutex);

   idx = _eet_dictionary_lookup(ed, string, len, hash);

   if (idx != -1)
     {
        if (ed->all[idx].str && 
            ((ed->all[idx].str == string) ||
                (!strcmp(ed->all[idx].str, string))))
	  {
	    eina_lock_release(&ed->mutex);
	    return idx;
	  }
     }

   if (ed->total == ed->count)
     {
        Eet_String *new;
        int total;

        total = ed->total + 8;

        new = realloc(ed->all, total * sizeof(Eet_String));
        if (!new) goto on_error;

        ed->all = new;
        ed->total = total;
     }

   str = eina_stringshare_add(string);
   if (!str) goto on_error;

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

   cnt = ed->count++;
   eina_lock_release(&ed->mutex);
   return cnt;

 on_error:
   eina_lock_release(&ed->mutex);
   return -1;
}

int
eet_dictionary_string_get_size(const Eet_Dictionary *ed,
                               int                   idx)
{
   int length = 0;

   if (!ed) goto done;

   if (idx < 0) goto done;

   eina_lock_take((Eina_Lock*) &ed->mutex);

   if (idx < ed->count)
     length = ed->all[idx].len;

   eina_lock_release((Eina_Lock*) &ed->mutex);

 done:
   return length;
}

EAPI int
eet_dictionary_count(const Eet_Dictionary *ed)
{
  return ed->count;
}

int
eet_dictionary_string_get_hash(const Eet_Dictionary *ed,
                               int                   idx)
{
   int hash = -1;

   if (!ed) goto done;

   if (idx < 0) goto done;

   eina_lock_take((Eina_Lock*) &ed->mutex);

   if (idx < ed->count)
     hash = ed->all[idx].hash;

   eina_lock_release((Eina_Lock*) &ed->mutex);

 done:
   return hash;
}

const char *
eet_dictionary_string_get_char(const Eet_Dictionary *ed,
                               int                   idx)
{
   const char *s = NULL;

   if (!ed) goto done;

   if (idx < 0) goto done;

   eina_lock_take((Eina_Lock*) &ed->mutex);

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
        s = ed->all[idx].str;
     }

   eina_lock_release((Eina_Lock*) &ed->mutex);

 done:
   return s;
}

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
}

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
}

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
}

static inline Eina_Bool
_eet_dictionary_test(const Eet_Dictionary *ed,
                     int                   idx,
                     void                 *result)
{
   Eina_Bool limit = EINA_FALSE;

   if (!result) goto done;

   if (!ed) goto done;

   if (idx < 0) goto done;

   eina_lock_take((Eina_Lock*) &ed->mutex);

   if (!(idx < ed->count)) goto unlock_done;

   limit = EINA_TRUE;

 unlock_done:
   eina_lock_release((Eina_Lock*) &ed->mutex);

 done:
   return limit;
}

static Eet_Convert *
eet_dictionary_convert_get(const Eet_Dictionary *ed,
                           int                   idx,
                           const char          **str)
{
   Eet_Convert *result;

   eina_lock_take((Eina_Lock*) &ed->mutex);

   *str = ed->all[idx].str;

   if (!ed->converts)
     {
        ((Eet_Dictionary *)ed)->converts = eina_hash_int32_new(free);

        goto add_convert;
     }

   result = eina_hash_find(ed->converts, &idx);
   if (result) goto done;

 add_convert:
   result = calloc(1, sizeof (Eet_Convert));

   eina_hash_add(ed->converts, &idx, result);

 done:
   eina_lock_release((Eina_Lock*) &ed->mutex);

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
        eina_lock_take((Eina_Lock*) &ed->mutex);
        if (!_eet_dictionary_string_get_float_cache(str, ed->all[idx].len,
                                                    &convert->f))
          {
             long long mantisse = 0;
             long exponent = 0;

             if (eina_convert_atod(str, ed->all[idx].len, &mantisse,
                                   &exponent) == EINA_FALSE)
               {
                  eina_lock_release((Eina_Lock*) &ed->mutex);
                  return EINA_FALSE;
               }

             convert->f = ldexpf((float)mantisse, exponent);
          }
        eina_lock_release((Eina_Lock*) &ed->mutex);

        convert->type |= EET_D_FLOAT;
     }

   *result = convert->f;
   return EINA_TRUE;
}

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
        eina_lock_take((Eina_Lock*) &ed->mutex);

        if (!_eet_dictionary_string_get_double_cache(str, ed->all[idx].len,
                                                     &convert->d))
          {
             long long mantisse = 0;
             long exponent = 0;

             if (eina_convert_atod(str, ed->all[idx].len, &mantisse,
                                   &exponent) == EINA_FALSE)
               {
                  eina_lock_release((Eina_Lock*) &ed->mutex);
                  return EINA_FALSE;
               }

             convert->d = ldexp((double)mantisse, exponent);
          }
        eina_lock_release((Eina_Lock*) &ed->mutex);

        convert->type |= EET_D_DOUBLE;
     }

   *result = convert->d;
   return EINA_TRUE;
}

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

        eina_lock_take((Eina_Lock*) &ed->mutex);
        if (!eina_convert_atofp(str, ed->all[idx].len, &fp))
          {
             eina_lock_release((Eina_Lock*) &ed->mutex);
             return EINA_FALSE;
          }
        eina_lock_release((Eina_Lock*) &ed->mutex);

        convert->fp = fp;
        convert->type |= EET_D_FIXED_POINT;
     }

   *result = convert->fp;
   return EINA_TRUE;
}

EAPI int
eet_dictionary_string_check(Eet_Dictionary *ed,
                            const char     *string)
{
   int res = 0;
   int i;

   if ((!ed) || (!string))
     return 0;

   eina_lock_take(&ed->mutex);

   if ((ed->start <= string) && (string < ed->end))
     res = 1;

   if (!res)
     {
        for (i = 0; i < ed->count; ++i)
          if ((ed->all[i].allocated) && ed->all[i].str == string)
            {
               res = 1;
               break;
            }
     }

   eina_lock_release(&ed->mutex);

   return res;
}

