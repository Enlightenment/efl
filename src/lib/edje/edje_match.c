#include "edje_private.h"

/* States manipulations. */

typedef struct _Edje_State Edje_State;
struct _Edje_State
{
   unsigned int idx;
   unsigned int pos;
};

struct _Edje_States
{
   unsigned int size;
   Edje_State  *states;
   Eina_Bool   *has;
};

static void
_edje_match_states_free(Edje_States *states,
                        unsigned int states_size)
{
   (void)states_size;
   free(states);
}

#define ALIGN(Size)               \
  {                               \
     Size--;                      \
     Size |= sizeof (void *) - 1; \
     Size++;                      \
  };

static int
_edje_match_states_alloc(Edje_Patterns *ppat, int n)
{
   Edje_States *l;

   const unsigned int patterns_size = ppat->patterns_size;
   const unsigned int patterns_max_length = ppat->max_length;

   const unsigned int array_len = (patterns_max_length + 1) * patterns_size;

   unsigned int states_size;
   unsigned int has_size;
   unsigned int states_has_size;
   unsigned int struct_size;

   unsigned char *states;
   unsigned char *has;

   int i;

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

   states = (unsigned char *)(l + n);
   has = states + states_size;

   for (i = 0; i < n; ++i)
     {
        l[i].states = (Edje_State *)states;
        l[i].has = (Eina_Bool *)has;
        l[i].size = 0;

        memset(l[i].has, 0, has_size);

        states += states_has_size;
        has += states_has_size;
     }

   return 1;
}

static void
_edje_match_states_insert(Edje_States *list,
                          unsigned int patterns_max_length,
                          unsigned int idx,
                          unsigned int pos)
{
   unsigned int i;

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
_edje_match_states_clear(Edje_States *list,
                         EINA_UNUSED unsigned int patterns_size,
                         EINA_UNUSED unsigned int patterns_max_length)
{
   list->size = 0;
}

/* Token manipulation. */

enum status
{
   patterns_not_found           = 0,
   patterns_found               = 1,
   patterns_syntax_error        = 2
};

static unsigned int
_edje_match_patterns_exec_class_token(enum status *status,
                                      const char *cl_tok,
                                      char c)
{
   if (!*cl_tok)
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
_edje_match_patterns_exec_class_complement(const char *cl_tok, unsigned int *ret)
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
_edje_match_patterns_exec_class(const char *cl,
                                char c,
                                unsigned int *ret)
{
   enum status status = patterns_not_found;
   int pos = 1;
   unsigned int neg;

   if (_edje_match_patterns_exec_class_complement(cl + 1, &neg) != EDJE_MATCH_OK)
     return EDJE_MATCH_SYNTAX_ERROR;

   pos += neg;

   do
     {
        pos += _edje_match_patterns_exec_class_token(&status, cl + pos, c);
     }
   while (cl[pos] && cl[pos] != ']');

   if (status == patterns_syntax_error || !cl[pos])
     return EDJE_MATCH_SYNTAX_ERROR;

   if (status == patterns_found)
     *ret = neg ? 0 : pos + 1;
   else
     *ret = neg ? pos + 1 : 0;

   return EDJE_MATCH_OK;
}

static Edje_Match_Error
_edje_match_patterns_exec_token(const char *tok,
                                char c,
                                unsigned int *ret)
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
_edje_match_patterns_exec_init_states(Edje_States *states,
                                      unsigned int patterns_size,
                                      unsigned int patterns_max_length)
{
   unsigned int i;

   states->size = patterns_size;

   for (i = 0; i < patterns_size; ++i)
     {
        states->states[i].idx = i;
        states->states[i].pos = 0;
        states->has[i * (patterns_max_length + 1)] = 1;
     }
}

/* Exported function. */

#define EDJE_MATCH_INIT_LIST(Func, Type, Source, Show)              \
  Edje_Patterns *                                                   \
  Func(const Eina_List *lst)                                        \
  {                                                                 \
     Edje_Patterns *r;                                              \
     unsigned int i;                                                \
                                                                    \
     if (!lst || eina_list_count(lst) <= 0)                         \
       return NULL;                                                 \
                                                                    \
     r = malloc(sizeof (Edje_Patterns) +                            \
                eina_list_count(lst)                                \
                * sizeof(*r->finals)                                \
                * sizeof(*r->patterns));                            \
     if (!r) return NULL;                                           \
                                                                    \
     r->ref = 1;                                                    \
     r->delete_me = EINA_FALSE;                                     \
     r->patterns_size = eina_list_count(lst);                       \
     r->max_length = 0;                                             \
     r->patterns = (const char **)r->finals + r->patterns_size + 1; \
                                                                    \
     for (i = 0; lst; ++i)                                          \
       {                                                            \
          const char *str;                                          \
          Type *data;                                               \
          unsigned int j;                                           \
          int special = 0;                                          \
                                                                    \
          data = eina_list_data_get(lst);                           \
          if (!data)                                                \
            {                                                       \
               free(r);                                             \
               return NULL;                                         \
            }                                                       \
                                                                    \
          str = data->Source;                                       \
          if (!str) str = "";                                       \
          r->patterns[i] = str;                                     \
                                                                    \
          if (Show)                                                 \
            INF("%lu [%s]", (unsigned long)i, str);                 \
                                                                    \
          r->finals[i] = 0;                                         \
          for (j = 0; str[j]; ++j)                                  \
            if (str[j] != '*')                                      \
              {                                                     \
                 r->finals[i] = j + 1;                              \
                 special++;                                         \
              }                                                     \
          j += special ? special + 1 : 0;                           \
                                                                    \
          if (j > r->max_length)                                    \
            r->max_length = j;                                      \
                                                                    \
          lst = eina_list_next(lst);                                \
       }                                                            \
                                                                    \
     if (!_edje_match_states_alloc(r, 2))                           \
       {                                                            \
          free(r);                                                  \
          return NULL;                                              \
       }                                                            \
                                                                    \
     return r;                                                      \
  }

#define EDJE_MATCH_INIT_ARRAY(Func, Type, Source, Show)             \
  Edje_Patterns *                                                   \
  Func(Type * const *lst, unsigned int count)                       \
  {                                                                 \
     Edje_Patterns *r;                                              \
     unsigned int i;                                                \
                                                                    \
     if (!lst || count == 0)                                        \
       return NULL;                                                 \
                                                                    \
     r = malloc(sizeof (Edje_Patterns) +                            \
                count                                               \
                * sizeof(*r->finals)                                \
                * sizeof(*r->patterns));                            \
     if (!r) return NULL;                                           \
                                                                    \
     r->ref = 1;                                                    \
     r->delete_me = EINA_FALSE;                                     \
     r->patterns_size = count;                                      \
     r->max_length = 0;                                             \
     r->patterns = (const char **)r->finals + r->patterns_size + 1; \
                                                                    \
     for (i = 0; i < count; ++i)                                    \
       {                                                            \
          const char *str;                                          \
          unsigned int j;                                           \
          int special = 0;                                          \
                                                                    \
          if (!lst[i])                                              \
            {                                                       \
               free(r);                                             \
               return NULL;                                         \
            }                                                       \
                                                                    \
          str = lst[i]->Source;                                     \
          if (!str) str = "";                                       \
          r->patterns[i] = str;                                     \
                                                                    \
          if (Show)                                                 \
            INF("%lu [%s]", (unsigned long)i, str);                 \
                                                                    \
          r->finals[i] = 0;                                         \
          for (j = 0; str[j]; ++j)                                  \
            if (str[j] != '*')                                      \
              {                                                     \
                 r->finals[i] = j + 1;                              \
                 special++;                                         \
              }                                                     \
          j += special ? special + 1 : 0;                           \
                                                                    \
          if (j > r->max_length)                                    \
            r->max_length = j;                                      \
       }                                                            \
                                                                    \
     if (!_edje_match_states_alloc(r, 2))                           \
       {                                                            \
          free(r);                                                  \
          return NULL;                                              \
       }                                                            \
                                                                    \
     return r;                                                      \
  }

#define EDJE_MATCH_INIT_INARRAY(Func, Source, Show)                            \
  Edje_Patterns *                                                              \
  Func(const Eina_Inarray * array, const Edje_Signal_Callback_Match * matches) \
  {                                                                            \
     Edje_Patterns *r;                                                         \
     int *it;                                                                  \
     unsigned int i = 0;                                                       \
                                                                               \
     if (!matches)                                                             \
       return NULL;                                                            \
                                                                               \
     r = malloc(sizeof (Edje_Patterns) +                                       \
                eina_inarray_count(array)                                      \
                * sizeof(*r->finals)                                           \
                * sizeof(*r->patterns));                                       \
     if (!r) return NULL;                                                      \
                                                                               \
     r->ref = 1;                                                               \
     r->delete_me = EINA_FALSE;                                                \
     r->patterns_size = eina_inarray_count(array);                             \
     r->max_length = 0;                                                        \
     r->patterns = (const char **)r->finals + r->patterns_size + 1;            \
                                                                               \
     EINA_INARRAY_FOREACH(array, it)                                           \
     {                                                                         \
        const char *str;                                                       \
        unsigned int j;                                                        \
        int special = 0;                                                       \
                                                                               \
        str = (matches + *it)->Source;                                         \
        if (!str) str = "";                                                    \
        r->patterns[i] = str;                                                  \
                                                                               \
        if (Show)                                                              \
          INF("%lu [%s]", (unsigned long)i, str);                              \
                                                                               \
        r->finals[i] = 0;                                                      \
        for (j = 0; str[j]; ++j)                                               \
          if (str[j] != '*')                                                   \
            {                                                                  \
               r->finals[i] = j + 1;                                           \
               special++;                                                      \
            }                                                                  \
        j += special ? special + 1 : 0;                                        \
                                                                               \
        if (j > r->max_length)                                                 \
          r->max_length = j;                                                   \
                                                                               \
        i++;                                                                   \
     }                                                                         \
                                                                               \
     if (!_edje_match_states_alloc(r, 2))                                      \
       {                                                                       \
          free(r);                                                             \
          return NULL;                                                         \
       }                                                                       \
                                                                               \
     return r;                                                                 \
  }

EDJE_MATCH_INIT_LIST(edje_match_collection_dir_init,
                     Edje_Part_Collection_Directory_Entry,
                     entry, 0);
EDJE_MATCH_INIT_ARRAY(edje_match_programs_signal_init,
                      Edje_Program,
                      signal, 0);
EDJE_MATCH_INIT_ARRAY(edje_match_programs_source_init,
                      Edje_Program,
                      source, 0);
EDJE_MATCH_INIT_INARRAY(edje_match_callback_signal_init,
                        signal, 0);
EDJE_MATCH_INIT_INARRAY(edje_match_callback_source_init,
                        source, 0);

static Eina_Bool
_edje_match_collection_dir_exec_finals(const unsigned int *finals,
                                       const Edje_States *states)
{
   unsigned int i;

   for (i = 0; i < states->size; ++i)
     {
        if (states->states[i].pos >= finals[states->states[i].idx])
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
edje_match_programs_exec_check_finals(const unsigned int *signal_finals,
                                      const unsigned int *source_finals,
                                      const Edje_States *signal_states,
                                      const Edje_States *source_states,
                                      Edje_Program **programs,
                                      Eina_Bool (*func)(Edje_Program *pr, void *data),
                                      void *data,
                                      Eina_Bool prop EINA_UNUSED)
{
   unsigned int i;
   unsigned int j;

   /* when not enought memory, they could be NULL */
   if (!signal_finals || !source_finals) return EINA_TRUE;

   for (i = 0; i < signal_states->size; ++i)
     {
        if (signal_states->states[i].pos >= signal_finals[signal_states->states[i].idx])
          {
             for (j = 0; j < source_states->size; ++j)
               {
                  if (signal_states->states[i].idx == source_states->states[j].idx
                      && source_states->states[j].pos >= source_finals[source_states->states[j].idx])
                    {
                       Edje_Program *pr;

                       pr = programs[signal_states->states[i].idx];
                       if (pr)
                         {
                            if (func(pr, data))
                              return EINA_FALSE;
                         }
                    }
               }
          }
     }

   return EINA_TRUE;
}

static int
edje_match_callback_exec_check_finals(const Edje_Signals_Sources_Patterns *ssp,
                                      const Edje_Signal_Callback_Match *matches,
                                      const Edje_States *signal_states,
                                      const Edje_States *source_states,
                                      const char *sig,
                                      const char *source,
                                      Edje *ed,
                                      Eina_Bool prop)
{
   const Edje_Signal_Callback_Match *cb;
   Eina_Array run;
   unsigned int i;
   unsigned int j;
   int r = 1;

   eina_array_step_set(&run, sizeof (Eina_Array), 4);

   for (i = 0; i < signal_states->size; ++i)
     if (signal_states->states[i].pos >= ssp->signals_patterns->finals[signal_states->states[i].idx])
       {
          for (j = 0; j < source_states->size; ++j)
            {
               if (signal_states->states[i].idx == source_states->states[j].idx
                   && source_states->states[j].pos >= ssp->sources_patterns->finals[source_states->states[j].idx])
                 {
                    int *e;

                    e = eina_inarray_nth(&ssp->u.callbacks.globing, signal_states->states[i].idx);

                    cb = &matches[*e];
                    if (cb)
                      {
                         if ((prop) && ed->callbacks->flags[*e].propagate) continue;
                         eina_array_push(&run, cb);
                         r = 2;
                      }
                 }
            }
       }

   while ((cb = eina_array_pop(&run)))
     {
        int idx = cb - matches;

        if (ed->callbacks->flags[idx].delete_me) continue;

        if (ed->callbacks->flags[idx].legacy)
          cb->legacy((void *)ed->callbacks->custom_data[idx], ed->obj, sig, source);
        else
          cb->eo((void *)ed->callbacks->custom_data[idx], ed->obj, sig, source);
        if (_edje_block_break(ed))
          {
             r = 0;
             break;
          }
        if ((ssp->signals_patterns->delete_me) || (ssp->sources_patterns->delete_me))
          {
             r = 0;
             break;
          }
     }

   eina_array_flush(&run);

   return r;
}

static Edje_States *
_edje_match_fn(const Edje_Patterns *ppat,
               const char *string,
               Edje_States *states)
{
   Edje_States *new_states = states + 1;
   const char *c;

   for (c = string; *c && states->size; ++c)
     {
        unsigned int i;

        _edje_match_states_clear(new_states, ppat->patterns_size, ppat->max_length);

        for (i = 0; i < states->size; ++i)
          {
             const unsigned int idx = states->states[i].idx;
             const unsigned int pos = states->states[i].pos;

             if (!ppat->patterns[idx][pos])
               continue;
             else if (ppat->patterns[idx][pos] == '*')
               {
                  _edje_match_states_insert(states, ppat->max_length, idx, pos + 1);
                  _edje_match_states_insert(new_states, ppat->max_length, idx, pos);
               }
             else
               {
                  unsigned int m;

                  if (_edje_match_patterns_exec_token(ppat->patterns[idx] + pos,
                                                      *c,
                                                      &m) != EDJE_MATCH_OK)
                    return NULL;

                  if (m)
                    _edje_match_states_insert(new_states, ppat->max_length, idx, pos + m);
               }
          }
        {
           Edje_States *tmp = states;

           states = new_states;
           new_states = tmp;
        }
     }

   return states;
}

Eina_Bool
edje_match_collection_dir_exec(const Edje_Patterns *ppat,
                               const char *string)
{
   Edje_States *result;
   Eina_Bool r = EINA_FALSE;

   /* under high memory presure, it could be NULL */
   if (!ppat) return EINA_FALSE;

   _edje_match_patterns_exec_init_states(ppat->states, ppat->patterns_size, ppat->max_length);

   result = _edje_match_fn(ppat, string, ppat->states);

   if (result)
     r = _edje_match_collection_dir_exec_finals(ppat->finals, result);

   return r;
}

Eina_Bool
edje_match_programs_exec(const Edje_Patterns *ppat_signal,
                         const Edje_Patterns *ppat_source,
                         const char *sig,
                         const char *source,
                         Edje_Program **programs,
                         Eina_Bool (*func)(Edje_Program *pr, void *data),
                         void *data,
                         Eina_Bool prop)
{
   Edje_States *signal_result;
   Edje_States *source_result;
   Eina_Bool r = EINA_FALSE;

   /* under high memory presure, they could be NULL */
   if (!ppat_source || !ppat_signal) return EINA_FALSE;

   _edje_match_patterns_exec_init_states(ppat_signal->states,
                                         ppat_signal->patterns_size,
                                         ppat_signal->max_length);
   _edje_match_patterns_exec_init_states(ppat_source->states,
                                         ppat_source->patterns_size,
                                         ppat_source->max_length);

   signal_result = _edje_match_fn(ppat_signal, sig, ppat_signal->states);
   source_result = _edje_match_fn(ppat_source, source, ppat_source->states);

   if (signal_result && source_result)
     r = edje_match_programs_exec_check_finals(ppat_signal->finals,
                                               ppat_source->finals,
                                               signal_result,
                                               source_result,
                                               programs,
                                               func,
                                               data,
                                               prop);
   return r;
}

int
edje_match_callback_exec(const Edje_Signals_Sources_Patterns *ssp,
                         const Edje_Signal_Callback_Match *matches,
                         const char *sig,
                         const char *source,
                         Edje *ed,
                         Eina_Bool prop)
{
   Edje_States *signal_result;
   Edje_States *source_result;
   int r = 0;

   /* under high memory presure, they could be NULL */
   if (!ssp->sources_patterns || !ssp->signals_patterns) return 0;

   ssp->signals_patterns->ref++;
   ssp->sources_patterns->ref++;
   _edje_match_patterns_exec_init_states(ssp->signals_patterns->states,
                                         ssp->signals_patterns->patterns_size,
                                         ssp->signals_patterns->max_length);
   _edje_match_patterns_exec_init_states(ssp->sources_patterns->states,
                                         ssp->sources_patterns->patterns_size,
                                         ssp->sources_patterns->max_length);

   signal_result = _edje_match_fn(ssp->signals_patterns, sig, ssp->signals_patterns->states);
   source_result = _edje_match_fn(ssp->sources_patterns, source, ssp->sources_patterns->states);

   if (signal_result && source_result)
     r = edje_match_callback_exec_check_finals(ssp,
                                               matches,
                                               signal_result,
                                               source_result,
                                               sig,
                                               source,
                                               ed,
                                               prop);
   ssp->signals_patterns->ref--;
   ssp->sources_patterns->ref--;
   if (ssp->signals_patterns->ref <= 0) edje_match_patterns_free(ssp->signals_patterns);
   if (ssp->sources_patterns->ref <= 0) edje_match_patterns_free(ssp->sources_patterns);
   return r;
}

void
edje_match_patterns_free(Edje_Patterns *ppat)
{
   if (!ppat) return;

   ppat->delete_me = EINA_TRUE;
   ppat->ref--;
   if (ppat->ref > 0) return;
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

static Eina_Rbtree_Direction
_edje_signal_source_node_cmp(const Edje_Signal_Source_Char *n1,
                             const Edje_Signal_Source_Char *n2,
                             EINA_UNUSED void *data)
{
   int cmp;

   cmp = strcmp(n1->signal, n2->signal);
   if (cmp) return cmp < 0 ? EINA_RBTREE_LEFT : EINA_RBTREE_RIGHT;

   return strcmp(n1->source, n2->source) < 0 ? EINA_RBTREE_LEFT : EINA_RBTREE_RIGHT;
}

static int
_edje_signal_source_key_cmp(const Edje_Signal_Source_Char *node,
                            const char *sig,
                            EINA_UNUSED int length,
                            const char *source)
{
   int cmp;

   cmp = strcmp(node->signal, sig);
   if (cmp) return cmp;

   return strcmp(node->source, source);
}

Eina_List *
edje_match_program_hash_build(Edje_Program *const *programs,
                              unsigned int count,
                              Eina_Rbtree **tree)
{
   Eina_List *result = NULL;
   Eina_Rbtree *new = NULL;
   unsigned int i;

   for (i = 0; i < count; ++i)
     {
        if (programs[i]->signal && !strpbrk(programs[i]->signal, "*?[\\")
            && programs[i]->source && !strpbrk(programs[i]->source, "*?[\\"))
          {
             Edje_Signal_Source_Char *item;

             item = (Edje_Signal_Source_Char *)eina_rbtree_inline_lookup(new, programs[i]->signal, 0,
                                                                         EINA_RBTREE_CMP_KEY_CB(_edje_signal_source_key_cmp), programs[i]->source);
             if (!item)
               {
                  item = malloc(sizeof (Edje_Signal_Source_Char));
                  if (!item) continue;

                  item->signal = programs[i]->signal;
                  item->source = programs[i]->source;
                  eina_inarray_step_set(&item->list, sizeof (Eina_Inarray), sizeof (void *), 8);

                  new = eina_rbtree_inline_insert(new, EINA_RBTREE_GET(item),
                                                  EINA_RBTREE_CMP_NODE_CB(_edje_signal_source_node_cmp), NULL);
               }

             eina_inarray_push(&item->list, &programs[i]);
          }
        else
          result = eina_list_prepend(result, programs[i]);
     }

   *tree = new;
   return result;
}

void
edje_match_callback_hash_build(const Edje_Signal_Callback_Match *callbacks,
                               int callbacks_count,
                               Eina_Rbtree **tree,
                               Eina_Inarray *result)
{
   Eina_Rbtree *new = NULL;
   int i;

   eina_inarray_step_set(result, sizeof (Eina_Inarray), sizeof (int), 8);

   for (i = 0; i < callbacks_count; ++i, ++callbacks)
     {
        if (callbacks->signal && !strpbrk(callbacks->signal, "*?[\\")
            && callbacks->source && !strpbrk(callbacks->source, "*?[\\"))
          {
             Edje_Signal_Source_Char *item;

             item = (Edje_Signal_Source_Char *)eina_rbtree_inline_lookup(new, callbacks->signal, 0,
                                                                         EINA_RBTREE_CMP_KEY_CB(_edje_signal_source_key_cmp), callbacks->source);
             if (!item)
               {
                  item = malloc(sizeof (Edje_Signal_Source_Char));
                  if (!item) continue;

                  item->signal = callbacks->signal;
                  item->source = callbacks->source;
                  eina_inarray_step_set(&item->list, sizeof (Eina_Inarray), sizeof (int), 8);

                  new = eina_rbtree_inline_insert(new, EINA_RBTREE_GET(item),
                                                  EINA_RBTREE_CMP_NODE_CB(_edje_signal_source_node_cmp), NULL);
               }

             eina_inarray_push(&item->list, &i);
          }
        else
          {
             eina_inarray_push(result, &i);
          }
     }

   *tree = new;
}

const Eina_Inarray *
edje_match_signal_source_hash_get(const char *sig,
                                  const char *source,
                                  const Eina_Rbtree *tree)
{
   Edje_Signal_Source_Char *lookup;

   lookup = (Edje_Signal_Source_Char *)eina_rbtree_inline_lookup(tree, sig, 0,
                                                                 EINA_RBTREE_CMP_KEY_CB(_edje_signal_source_key_cmp), source);

   if (lookup) return &lookup->list;
   return NULL;
}

void
edje_match_signal_source_free(Edje_Signal_Source_Char *key, EINA_UNUSED void *data)
{
   eina_inarray_flush(&key->list);
   free(key);
}

