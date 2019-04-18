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
   Eet_Dictionary *ed;

   ed = eet_dictionary_calloc(1);
   if (!ed) return NULL;
   memset(ed->hash, -1, sizeof(int) * 256);
   eina_rwlock_new(&ed->rwlock);
   return ed;
}

void
eet_dictionary_free(Eet_Dictionary *ed)
{
   int i;

   if (!ed) return;
   eina_rwlock_free(&ed->rwlock);

   for (i = 0; i < ed->count; i++)
     {
        if (ed->all_allocated[i >> 3] & (1 << (i & 0x7)))
          {
             eina_stringshare_del(ed->all[i].str);
          }
     }
   free(ed->all);
   free(ed->all_hash);
   free(ed->all_allocated);

   if (ed->converts) eina_hash_free(ed->converts);
   eet_dictionary_mp_free(ed);
}

static int
_eet_dictionary_lookup(Eet_Dictionary *ed,
                       const char     *string,
                       int             len,
                       int             hash,
                       int            *previous)
{
   int prev = -1, current;

   current = ed->hash[hash];
   while (current != -1)
     {
        if ((ed->all[current].str) &&
            ((ed->all[current].str == string) ||
             ((ed->all[current].len == len) &&
              (!strcmp(ed->all[current].str, string)))))
          {
             break;
          }
        prev = current;
        current = ed->all[current].next;
     }
   if (previous) *previous = prev;
   return current;
}

int
eet_dictionary_string_add(Eet_Dictionary *ed,
                          const char     *string)
{
   Eet_String *current;
   const char *str;
   int hash, idx, pidx, len, cnt;

   if (!ed) return -1;

   hash = _eet_hash_gen(string, 8);
   len = strlen(string) + 1;

   eina_rwlock_take_read(&ed->rwlock);

   idx = _eet_dictionary_lookup(ed, string, len, hash, &pidx);
   if (idx != -1)
     {
        eina_rwlock_release(&ed->rwlock);
        return idx;
     }

   str = eina_stringshare_add(string);
   if (!str) goto on_error;

   eina_rwlock_release(&ed->rwlock);
   eina_rwlock_take_write(&ed->rwlock);
   if (ed->total == ed->count)
     {
        Eet_String *s;
        unsigned char *new_hash, *new_allocated;
        int total;

        total = ed->total + 64;

        s = realloc(ed->all, total * sizeof(Eet_String));
        if (!s) goto on_error;
        ed->all = s;

        new_hash = realloc(ed->all_hash, total);
        if (!new_hash) goto on_error;
        ed->all_hash = new_hash;

        new_allocated = realloc(ed->all_allocated, ((total >> 3) + 1));
        if (!new_allocated) goto on_error;
        ed->all_allocated = new_allocated;

        ed->total = total;
     }

   current = ed->all + ed->count;

   ed->all_allocated[ed->count >> 3] |= (1 << (ed->count & 0x7));
   ed->all_hash[ed->count] = hash;

   current->str = str;
   current->len = len;

   current->next = ed->hash[hash];
   ed->hash[hash] = ed->count;

   cnt = ed->count++;
   eina_rwlock_release(&ed->rwlock);
   return cnt;

on_error:
   eina_rwlock_release(&ed->rwlock);
   return -1;
}

int
eet_dictionary_string_get_size(const Eet_Dictionary *ed,
                               int                   idx)
{
   int length = 0;

   if (!ed) goto done;
   if (idx < 0) goto done;

   eina_rwlock_take_read((Eina_RWLock *)&ed->rwlock);
   if (idx < ed->count) length = ed->all[idx].len;
   eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
done:
   return length;
}

EAPI int
eet_dictionary_count(const Eet_Dictionary *ed)
{
   int cnt;

   if (!ed) return 0;

   eina_rwlock_take_read((Eina_RWLock *)&ed->rwlock);
   cnt = ed->count;
   eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
   return cnt;
}

int
eet_dictionary_string_get_hash(const Eet_Dictionary *ed,
                               int                   idx)
{
   int hash = -1;

   if (!ed) goto done;
   if (idx < 0) goto done;

   eina_rwlock_take_read((Eina_RWLock *)&ed->rwlock);
   if (idx < ed->count) hash = ed->all_hash[idx];
   eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
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

   eina_rwlock_take_read((Eina_RWLock *)&ed->rwlock);
   if (idx < ed->count)
     {
#ifdef _WIN32
        /* Windows file system could change the mmaped file when replacing a file. So we need to copy all string in memory to avoid bugs. */
        if (!(ed->all_allocated[idx >> 3] & (1 << (idx & 0x7))))
          {
             ed->all[idx].str = eina_stringshare_add(ed->all[idx].str);
             ed->all_allocated[idx >> 3] |= (1 << (idx & 0x7));
          }
#endif /* ifdef _WIN32 */
        s = ed->all[idx].str;
     }
   eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
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
   int mantisse, exponent;

   if (_eet_dictionary_string_get_me_cache(s, len, &mantisse, &exponent))
     {
        if (s[4] == '+') *result = (float)(mantisse << exponent);
        else *result = (float)mantisse / (float)(1 << exponent);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static inline Eina_Bool
_eet_dictionary_string_get_double_cache(const char *s,
                                        int         len,
                                        double     *result)
{
   int mantisse, exponent;

   if (_eet_dictionary_string_get_me_cache(s, len, &mantisse, &exponent))
     {
        if (s[4] == '+') *result = (double)(mantisse << exponent);
        else *result = (double)mantisse / (float)(1 << exponent);
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

   eina_rwlock_take_read((Eina_RWLock *)&ed->rwlock);
   if (!(idx < ed->count)) goto unlock_done;
   limit = EINA_TRUE;
unlock_done:
   eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
done:
   return limit;
}

static Eet_Convert *
eet_dictionary_convert_get(const Eet_Dictionary *ed,
                           int                   idx,
                           const char          **str)
{
   Eet_Convert *result;

   eina_rwlock_take_read((Eina_RWLock *)&ed->rwlock);
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
   eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
   return result;
}

Eina_Bool
eet_dictionary_string_get_float(const Eet_Dictionary *ed,
                                int                   idx,
                                float                *result)
{
   Eet_Convert *convert;
   const char *str;

   if (!_eet_dictionary_test(ed, idx, result)) return EINA_FALSE;

   convert = eet_dictionary_convert_get(ed, idx, &str);
   if (!convert) return EINA_FALSE;

   if (!(convert->type & EET_D_FLOAT))
     {
        eina_rwlock_take_read((Eina_RWLock *)&ed->rwlock);
        if (!_eet_dictionary_string_get_float_cache(str, ed->all[idx].len,
                                                    &convert->f))
          {
             long long mantisse = 0;
             long exponent = 0;

             if (eina_convert_atod(str, ed->all[idx].len, &mantisse,
                                   &exponent) == EINA_FALSE)
               {
                  eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
                  return EINA_FALSE;
               }
             convert->f = ldexpf((float)mantisse, exponent);
          }
        eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
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

   if (!_eet_dictionary_test(ed, idx, result)) return EINA_FALSE;

   convert = eet_dictionary_convert_get(ed, idx, &str);
   if (!convert) return EINA_FALSE;

   if (!(convert->type & EET_D_DOUBLE))
     {
        eina_rwlock_take_read((Eina_RWLock *)&ed->rwlock);

        if (!_eet_dictionary_string_get_double_cache(str, ed->all[idx].len,
                                                     &convert->d))
          {
             long long mantisse = 0;
             long exponent = 0;

             if (eina_convert_atod(str, ed->all[idx].len, &mantisse,
                                   &exponent) == EINA_FALSE)
               {
                  eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
                  return EINA_FALSE;
               }
             convert->d = ldexp((double)mantisse, exponent);
          }
        eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
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

   if (!_eet_dictionary_test(ed, idx, result)) return EINA_FALSE;

   convert = eet_dictionary_convert_get(ed, idx, &str);
   if (!convert) return EINA_FALSE;

   if (!(convert->type & EET_D_FIXED_POINT))
     {
        Eina_F32p32 fp;

        eina_rwlock_take_read((Eina_RWLock *)&ed->rwlock);
        if (!eina_convert_atofp(str, ed->all[idx].len, &fp))
          {
             eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
             return EINA_FALSE;
          }
        eina_rwlock_release((Eina_RWLock *)&ed->rwlock);

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
   int res = 0, i;

   if ((!ed) || (!string)) return 0;

   eina_rwlock_take_read((Eina_RWLock *)&ed->rwlock);
   if ((ed->start <= string) && (string < ed->end)) res = 1;

   if (!res)
     {
        for (i = 0; i < ed->count; i++)
          {
             if ((ed->all_allocated[i >> 3] & (1 << (i & 0x7))) && ed->all[i].str == string)
               {
                  res = 1;
                  break;
               }
          }
     }
   eina_rwlock_release((Eina_RWLock *)&ed->rwlock);
   return res;
}

