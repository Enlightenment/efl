/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EET_PRIVATE_H
#define _EET_PRIVATE_H

#ifdef __GNUC__
# if __GNUC__ >= 4
// BROKEN in gcc 4 on amd64
//#  pragma GCC visibility push(hidden)
# endif
#endif

typedef struct _Eet_String              Eet_String;

struct _Eet_String
{
  const char            *mmap;
  char                  *str;

  int                    hash;
  int                    len;

  int                    next;
  int                    prev;

  union
  {
    float                f;
    double               d;
  } convert;

  struct
  {
    unsigned int         converted : 1;
    unsigned int         is_float : 1;
  } flags;
};
struct _Eet_Dictionary
{
  Eet_String   *all;

  int           size;
  int           offset;

  int           hash[256];

  int           count;
  int           total;

  const char   *start;
  const char   *end;
};

Eet_Dictionary  *eet_dictionary_add(void);
void             eet_dictionary_free(Eet_Dictionary *ed);
int              eet_dictionary_string_add(Eet_Dictionary *ed, const char *string);
int              eet_dictionary_string_get_size(const Eet_Dictionary *ed, int index);
const char      *eet_dictionary_string_get_char(const Eet_Dictionary *ed, int index);
int              eet_dictionary_string_get_float(const Eet_Dictionary *ed, int index, float *result);
int              eet_dictionary_string_get_double(const Eet_Dictionary *ed, int index, double *result);
int              eet_dictionary_string_get_hash(const Eet_Dictionary *ed, int index);

int   _eet_hash_gen(const char *key, int hash_size);
int   _eet_string_to_double_convert(const char *src, long long *m, long *e);
void  _eet_double_to_string_convert(char des[128], double d);

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifdef DNDEBUG
#define EET_ASSERT(Test, Do) if (Test == 0) Do;
#else
#define EET_ASSERT(Test, Do) if (Test == 0) abort();
#endif

#endif
