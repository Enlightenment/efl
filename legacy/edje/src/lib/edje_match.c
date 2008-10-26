/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <string.h>

#include "edje_private.h"

/* States manipulations. */

typedef struct _Edje_State      Edje_State;
struct _Edje_State
{
  size_t        idx;
  size_t        pos;
};

struct _Edje_States
{
  size_t         size;
  Edje_State    *states;
  _Bool         *has;
};

static void
_edje_match_states_free(Edje_States      *states,
                        size_t            states_size)
{
   (void) states_size;
   free(states);
}

#define ALIGN(Size)                             \
  {                                             \
     Size--;                                    \
     Size |= sizeof (void*) - 1;                \
     Size++;                                    \
  };

static int
_edje_match_states_alloc(Edje_Patterns *ppat, int n)
{
   Edje_States *l;

   const size_t patterns_size = ppat->patterns_size;
   const size_t patterns_max_length = ppat->max_length;

   const size_t array_len = (patterns_max_length + 1) * patterns_size;

   size_t       states_size;
   size_t       has_size;
   size_t       states_has_size;
   size_t       struct_size;

   unsigned char        *states;
   unsigned char        *has;

   size_t        i;

   states_size = sizeof (*l->states) * array_len;
   ALIGN(states_size);

   has_size = sizeof (*l->has) * array_len;
   ALIGN(has_size);

   states_has_size = states_size + has_size;

   struct_size = sizeof (*l);
   ALIGN(struct_size);
   struct_size += states_has_size;

   l = malloc(n * struct_size);
   if (!l) return 0;

   ppat->states = l;
   ppat->states->size = 0;

   states = (unsigned char *) (l + n);
   has = states + states_size;

   for (i = 0; i < n; ++i)
     {
        l[i].states = (Edje_State *) states;
        l[i].has = (_Bool *) has;

	memset(l[i].has, 0, has_size);

        states += states_has_size;
        has += states_has_size;
     }

   return 1;
}

static void
_edje_match_states_insert(Edje_States    *list,
                          size_t          patterns_max_length,
                          size_t          idx,
                          size_t          pos)
{
   size_t i;

   i = (idx * (patterns_max_length + 1)) + pos;
   
   if (i < list->size)
     {
	if (list->has[i]) return;
     }
   list->has[i] = 1;
   
   i = list->size;
   list->states[i].idx = idx;
   list->states[i].pos = pos;
   list->has[i] = 0;
   list->size++;
}

static void
_edje_match_states_clear(Edje_States     *list,
                         size_t           patterns_size,
                         size_t           patterns_max_length)
{
   list->size = 0;
}

/* Token manipulation. */

enum status
{
   patterns_not_found		= 0,
   patterns_found		= 1,
   patterns_syntax_error	= 2
};

static size_t
_edje_match_patterns_exec_class_token(enum status	*status,
                                      const char	*cl_tok,
                                      char		 c)
{
  if (! *cl_tok)
    {
      *status = patterns_syntax_error;
      return 0;
    }
  else if (cl_tok[1] == '-' && cl_tok[2] != ']')
    {
      if (*cl_tok <= c && c <= cl_tok[2])
	*status = patterns_found;
      return 3;
    }
  else
    {
      if (c == *cl_tok)
	*status = patterns_found;
      return 1;
    }
}

static Edje_Match_Error
_edje_match_patterns_exec_class_complement(const char *cl_tok, size_t *ret)
{
  switch (*cl_tok)
    {
     case 0:
        return EDJE_MATCH_SYNTAX_ERROR;

     case '!':
        *ret = 1;
        return EDJE_MATCH_OK;

    default:
       *ret = 0;
       return EDJE_MATCH_OK;
    }
}

static Edje_Match_Error
_edje_match_patterns_exec_class(const char	*cl,
                                char		 c,
                                size_t          *ret)
{
   enum status	status = patterns_not_found;
   int		pos = 1;
   size_t       neg;

   if (_edje_match_patterns_exec_class_complement(cl + 1, &neg) != EDJE_MATCH_OK)
     return EDJE_MATCH_SYNTAX_ERROR;

   pos += neg;

   do
     pos += _edje_match_patterns_exec_class_token(&status, cl + pos, c);
   while (cl[pos] && cl[pos] != ']');

   if (status == patterns_syntax_error || ! cl[pos])
     return EDJE_MATCH_SYNTAX_ERROR;

   if (status == patterns_found)
     *ret = neg ? 0 : pos + 1;
   else
     *ret = neg ? pos + 1 : 0;

   return EDJE_MATCH_OK;
}

static Edje_Match_Error
_edje_match_patterns_exec_token(const char	*tok,
                                char		 c,
                                size_t          *ret)
{
  switch (*tok)
    {
    case '\\':
      if (tok[1])
        {
           *ret = tok[1] == c ? 2 : 0;
           return EDJE_MATCH_OK;
        }
      return EDJE_MATCH_SYNTAX_ERROR;

    case '?':
      *ret = 1;
      return EDJE_MATCH_OK;

    case '[':
       return _edje_match_patterns_exec_class(tok, c, ret);

    default:
       *ret = *tok == c ? 1 : 0;
       return EDJE_MATCH_OK;
    }
}

static void
_edje_match_patterns_exec_init_states(Edje_States       *states,
                                      size_t		 patterns_size,
                                      size_t             patterns_max_length)
{
   size_t       i;

   states->size = patterns_size;

   for (i = 0; i < patterns_size; ++i)
     {
        states->states[i].idx = i;
        states->states[i].pos = 0;
        states->has[i * (patterns_max_length + 1)] = 1;
     }
}

/* Exported function. */

#define EDJE_MATCH_INIT(Func, Type, Source, Show)               \
  Edje_Patterns*                                                \
  Func(Eina_List *lst)                                          \
  {                                                             \
     Edje_Patterns      *r;                                     \
     size_t              i;                                     \
                                                                \
     if (!lst || eina_list_count(lst) <= 0)                     \
       return NULL;                                             \
                                                                \
     r = malloc(sizeof (Edje_Patterns) +                        \
                eina_list_count(lst)                            \
                * sizeof (*r->finals)                           \
                * sizeof(*r->patterns));                        \
     if (!r) return NULL;                                       \
                                                                \
     r->patterns_size = eina_list_count(lst);                   \
     r->max_length = 0;                                         \
     r->patterns = (const char **) r->finals + r->patterns_size + 1;    \
                                                                \
     for (i = 0; lst; ++i)                                      \
       {                                                        \
          const char    *str;                                   \
          Type          *data;                                  \
          size_t         j;                                     \
                                                                \
          data = eina_list_data_get(lst);                           \
          if (!data)                                            \
            {                                                   \
               free(r);                                         \
               return NULL;                                     \
            }                                                   \
                                                                \
          str = data->Source;                                   \
          if (!str) str = "";                                   \
          r->patterns[i] = str;                                 \
                                                                \
          if (Show)                                             \
            fprintf(stderr, "%i [%s]\n", i, str);               \
                                                                \
          r->finals[i] = 0;                                     \
          for (j = 0; str[j]; ++j)                              \
            if (str[j] != '*')                                  \
              r->finals[i] = j + 1;                             \
                                                                \
          if (j > r->max_length)                                \
            r->max_length = j;                                  \
                                                                \
          lst = eina_list_next(lst);                            \
       }                                                        \
                                                                \
     if (!_edje_match_states_alloc(r, 2))                       \
       {                                                        \
          free(r);                                              \
          return NULL;                                          \
       }                                                        \
                                                                \
     return r;                                                  \
  }

EDJE_MATCH_INIT(edje_match_collection_dir_init,
                Edje_Part_Collection_Directory_Entry,
                entry, 0);
EDJE_MATCH_INIT(edje_match_programs_signal_init,
                Edje_Program,
                signal, 0);
EDJE_MATCH_INIT(edje_match_programs_source_init,
                Edje_Program,
                source, 0);
EDJE_MATCH_INIT(edje_match_callback_signal_init,
                Edje_Signal_Callback,
                signal, 0);
EDJE_MATCH_INIT(edje_match_callback_source_init,
                Edje_Signal_Callback,
                source, 0);

static int
_edje_match_collection_dir_exec_finals(const size_t      *finals,
                                       const Edje_States *states)
{
   size_t       i;

   for (i = 0; i < states->size; ++i)
     if (states->states[i].pos >= finals[states->states[i].idx])
       return 1;
   return 0;
}

static int
edje_match_programs_exec_check_finals(const size_t      *signal_finals,
                                      const size_t      *source_finals,
                                      const Edje_States *signal_states,
                                      const Edje_States *source_states,
                                      Eina_List         *programs,
                                      int (*func)(Edje_Program *pr, void *data),
                                      void              *data)
{
   size_t       i;
   size_t       j;

   for (i = 0; i < signal_states->size; ++i)
     if (signal_states->states[i].pos >= signal_finals[signal_states->states[i].idx])
       for (j = 0; j < source_states->size; ++j)
         if (signal_states->states[i].idx == source_states->states[j].idx
             && source_states->states[j].pos >= source_finals[source_states->states[j].idx])
           {
              Edje_Program  *pr;

              pr = eina_list_nth(programs, signal_states->states[i].idx);
              if (pr)
                {
                   if (func(pr, data))
                     return 0;
                }
       }

   return 1;
}

static int
edje_match_callback_exec_check_finals(const size_t      *signal_finals,
                                      const size_t      *source_finals,
                                      const Edje_States *signal_states,
                                      const Edje_States *source_states,
                                      const char        *signal,
                                      const char        *source,
                                      Eina_List         *callbacks,
                                      Edje              *ed)
{
   size_t       i;
   size_t       j;
   int          r = 1;

   for (i = 0; i < signal_states->size; ++i)
     if (signal_states->states[i].pos >= signal_finals[signal_states->states[i].idx])
       for (j = 0; j < source_states->size; ++j)
         if (signal_states->states[i].idx == source_states->states[j].idx
             && source_states->states[j].pos >= source_finals[source_states->states[j].idx])
           {
              Edje_Signal_Callback      *escb;

              escb = eina_list_nth(callbacks, signal_states->states[i].idx);
              if (escb)
                {
                   if ((!escb->just_added)
                       && (!escb->delete_me))
                     {
                        escb->func(escb->data, ed->obj, signal, source);
                        r = 2;
                     }
                   if (_edje_block_break(ed))
                     return 0;
                }
       }

   return r;
}


static Edje_States*
_edje_match_fn(const Edje_Patterns      *ppat,
               const char               *string,
               Edje_States              *states)
{
   Edje_States  *new_states = states + 1;
   const char   *c;

   for (c = string; *c && states->size; ++c)
     {
        size_t  i;

        _edje_match_states_clear(new_states, ppat->patterns_size, ppat->max_length);

        for (i = 0; i < states->size; ++i)
          {
             const size_t       idx = states->states[i].idx;
             const size_t       pos = states->states[i].pos;

             if (!ppat->patterns[idx][pos])
               continue;
             else if (ppat->patterns[idx][pos] == '*')
               {
                  _edje_match_states_insert(states, ppat->max_length, idx, pos + 1);
                  _edje_match_states_insert(new_states, ppat->max_length, idx, pos);
               }
             else
               {
                  size_t        m;

                  if (_edje_match_patterns_exec_token(ppat->patterns[idx] + pos,
                                                      *c,
                                                      &m) != EDJE_MATCH_OK)
                    return NULL;

                  if (m)
                    _edje_match_states_insert(new_states, ppat->max_length, idx, pos + m);
               }
          }
        {
           Edje_States  *tmp = states;

           states = new_states;
           new_states = tmp;
        }
     }

   return states;
}

int
edje_match_collection_dir_exec(const Edje_Patterns      *ppat,
                               const char               *string)
{
   Edje_States  *result;
   int           r = 0;

   _edje_match_patterns_exec_init_states(ppat->states, ppat->patterns_size, ppat->max_length);

   result = _edje_match_fn(ppat, string, ppat->states);

   if (result)
     r = _edje_match_collection_dir_exec_finals(ppat->finals, result);

   return r;
}

int
edje_match_programs_exec(const Edje_Patterns    *ppat_signal,
                         const Edje_Patterns    *ppat_source,
                         const char             *signal,
                         const char             *source,
                         Eina_List              *programs,
                         int (*func)(Edje_Program *pr, void *data),
                         void                   *data)
{
   Edje_States  *signal_result;
   Edje_States  *source_result;
   int           r = 0;

   _edje_match_patterns_exec_init_states(ppat_signal->states,
                                         ppat_signal->patterns_size,
                                         ppat_signal->max_length);
   _edje_match_patterns_exec_init_states(ppat_source->states,
                                         ppat_source->patterns_size,
                                         ppat_source->max_length);

   signal_result = _edje_match_fn(ppat_signal, signal, ppat_signal->states);
   source_result = _edje_match_fn(ppat_source, source, ppat_source->states);

   if (signal_result && source_result)
     r = edje_match_programs_exec_check_finals(ppat_signal->finals,
                                               ppat_source->finals,
                                               signal_result,
                                               source_result,
                                               programs,
                                               func,
                                               data);
   return r;
}

int
edje_match_callback_exec(const Edje_Patterns    *ppat_signal,
                         const Edje_Patterns    *ppat_source,
                         const char             *signal,
                         const char             *source,
                         Eina_List              *callbacks,
                         Edje                   *ed)
{
   Edje_States  *signal_result;
   Edje_States  *source_result;
   int           r = 0;

   _edje_match_patterns_exec_init_states(ppat_signal->states,
                                         ppat_signal->patterns_size,
                                         ppat_signal->max_length);
   _edje_match_patterns_exec_init_states(ppat_source->states,
                                         ppat_source->patterns_size,
                                         ppat_source->max_length);

   signal_result = _edje_match_fn(ppat_signal, signal, ppat_signal->states);
   source_result = _edje_match_fn(ppat_source, source, ppat_source->states);

   if (signal_result && source_result)
     r = edje_match_callback_exec_check_finals(ppat_signal->finals,
                                               ppat_source->finals,
                                               signal_result,
                                               source_result,
                                               signal,
                                               source,
                                               callbacks,
                                               ed);
   return r;
}

void
edje_match_patterns_free(Edje_Patterns *ppat)
{
   _edje_match_states_free(ppat->states, 2);
   free(ppat);
}

void
_edje_signals_sources_patterns_clean(Edje_Signals_Sources_Patterns *ssp)
{
   if (!ssp->signals_patterns)
     return;

   edje_match_patterns_free(ssp->signals_patterns);
   edje_match_patterns_free(ssp->sources_patterns);
   ssp->signals_patterns = NULL;
   ssp->sources_patterns = NULL;
}
