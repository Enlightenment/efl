#include "edje_private.h"

#include <assert.h>

static Eina_Hash *signal_match = NULL;

static unsigned int
_edje_signal_match_key_length(const void *key EINA_UNUSED)
{
   return sizeof (Edje_Signal_Callback_Matches);
}

static int
_edje_signal_match_key_cmp(const void *key1, int key1_length EINA_UNUSED, const void *key2, int key2_length EINA_UNUSED)
{
   const Edje_Signal_Callback_Matches *a = key1;
   const Edje_Signal_Callback_Matches *b = key2;
   unsigned int i;

   if (a->matches_count != b->matches_count) return a->matches_count - b->matches_count;
   for (i = 0; i < a->matches_count; ++i)
     {
        if (a->matches[i].signal != b->matches[i].signal) return a->matches[i].signal - b->matches[i].signal;
        if (a->matches[i].source != b->matches[i].source) return a->matches[i].source - b->matches[i].source;
        if (a->matches[i].func != b->matches[i].func) return (unsigned char *)a->matches[i].func - (unsigned char *)b->matches[i].func;
     }
   return 0;
}

static int
_edje_signal_match_key_hash(const void *key, int key_length EINA_UNUSED)
{
   const Edje_Signal_Callback_Matches *a = key;
   unsigned int hash, i;

   hash = eina_hash_int32(&a->matches_count, sizeof (int));
   for (i = 0; i < a->matches_count; ++i)
     {
#ifdef EFL64
        hash ^= eina_hash_int64((const unsigned long long int *)&a->matches[i].signal, sizeof (char *));
        hash ^= eina_hash_int64((const unsigned long long int *)&a->matches[i].source, sizeof (char *));
        hash ^= eina_hash_int64((const unsigned long long int *)&a->matches[i].func, sizeof (Edje_Signal_Cb));
#else
        hash ^= eina_hash_int32((const unsigned int *)&a->matches[i].signal, sizeof (char *));
        hash ^= eina_hash_int32((const unsigned int *)&a->matches[i].source, sizeof (char *));
        hash ^= eina_hash_int32((const unsigned int *)&a->matches[i].func, sizeof (Edje_Signal_Cb));
#endif
     }
   return hash;
}

static const Edje_Signal_Callback_Matches *
_edje_signal_callback_matches_dup(const Edje_Signal_Callback_Matches *src)
{
   Edje_Signal_Callback_Matches *result;
   unsigned int i;

   result = calloc(1, sizeof (Edje_Signal_Callback_Matches));
   if (!result) return NULL;

   result->hashed = EINA_FALSE;
   result->matches = malloc(sizeof (Edje_Signal_Callback_Match) * src->matches_count);
   result->matches_count = src->matches_count;
   result->patterns = NULL;
   EINA_REFCOUNT_REF(result);

   for (i = 0; i < src->matches_count; i++)
     {
        result->matches[i].signal = eina_stringshare_ref(src->matches[i].signal);
        result->matches[i].source = eina_stringshare_ref(src->matches[i].source);
        result->matches[i].func = src->matches[i].func;
     }

   return result;
}

void
_edje_callbacks_patterns_clean(Edje_Signal_Callback_Group *gp)
{
   Edje_Signal_Callback_Matches *tmp;

   assert(EINA_REFCOUNT_GET(gp->matches) == 1);
   tmp = (Edje_Signal_Callback_Matches *)gp->matches;

   _edje_signal_callback_patterns_unref(tmp->patterns);
   tmp->patterns = NULL;
}

static void
_edje_callbacks_patterns_init(Edje_Signal_Callback_Group *gp)
{
   Edje_Signals_Sources_Patterns *ssp;
   Edje_Signal_Callback_Matches *tmp;

   if (gp->matches->patterns) return;

   tmp = (Edje_Signal_Callback_Matches *)gp->matches;
   tmp->patterns = calloc(1, sizeof (Edje_Signals_Sources_Patterns));
   if (!gp->matches->patterns) return;

   ssp = gp->matches->patterns;
   edje_match_callback_hash_build(gp->matches->matches,
                                  gp->matches->matches_count,
                                  &ssp->exact_match,
                                  &ssp->u.callbacks.globing);

   ssp->signals_patterns = edje_match_callback_signal_init(&ssp->u.callbacks.globing, tmp->matches);
   ssp->sources_patterns = edje_match_callback_source_init(&ssp->u.callbacks.globing, tmp->matches);
   EINA_REFCOUNT_REF(ssp);
}

void
edje_signal_init(void)
{
   signal_match = eina_hash_new(_edje_signal_match_key_length,
                                _edje_signal_match_key_cmp,
                                _edje_signal_match_key_hash,
                                NULL,
                                3);
}

void
edje_signal_shutdown(void)
{
   // FIXME: iterate and destroy leftover signal matcher
   eina_hash_free(signal_match);
}

static void
_edje_signal_callback_unset(Edje_Signal_Callback_Group *gp, int idx)
{
   Edje_Signal_Callback_Match *m;

   m = gp->matches->matches + idx;
   eina_stringshare_del(m->signal);
   m->signal = NULL;
   eina_stringshare_del(m->source);
   m->source = NULL;
}

static void
_edje_signal_callback_set(Edje_Signal_Callback_Group *gp, int idx,
                          const char *sig, const char *src,
                          Edje_Signal_Cb func, void *data, Edje_Signal_Callback_Flags flags)
{
   Edje_Signal_Callback_Match *m;

   m = gp->matches->matches + idx;
   m->signal = eina_stringshare_ref(sig);
   m->source = eina_stringshare_ref(src);
   m->func = func;

   gp->custom_data[idx] = data;

   gp->flags[idx] = flags;
}

static Edje_Signal_Callback_Group *
_edje_signal_callback_grow(Edje_Signal_Callback_Group *gp)
{
   Edje_Signal_Callback_Matches *tmp;

   tmp = (Edje_Signal_Callback_Matches *)gp->matches;
   tmp->matches_count++;
   tmp->matches = realloc(tmp->matches, sizeof (Edje_Signal_Callback_Match) * tmp->matches_count);
   gp->custom_data = realloc(gp->custom_data, sizeof (void *) * tmp->matches_count);
   gp->flags = realloc(gp->flags, sizeof (Edje_Signal_Callback_Flags) * tmp->matches_count);

   return gp;
}

Eina_Bool
_edje_signal_callback_push(Edje_Signal_Callback_Group *gp,
                           const char *sig, const char *src,
                           Edje_Signal_Cb func, void *data, Eina_Bool propagate)
{
   unsigned int i;
   Edje_Signal_Callback_Flags flags;
   Edje_Signal_Callback_Matches *tmp;

   flags.delete_me = EINA_FALSE;
   flags.just_added = EINA_TRUE;
   flags.propagate = !!propagate;

   tmp = (Edje_Signal_Callback_Matches *)gp->matches;

   // let's first try to see if we do find an empty matching stop
   for (i = 0; i < tmp->matches_count; i++)
     {
        if ((sig == tmp->matches[i].signal) &&
            (src == tmp->matches[i].source) &&
            (func == tmp->matches[i].func) &&
            (gp->flags[i].delete_me))
          {
             _edje_signal_callback_unset(gp, i);
             _edje_signal_callback_set(gp, i, sig, src, func, data, flags);
             return EINA_TRUE;
          }
     }

   if (tmp->hashed)
     {
        if (EINA_REFCOUNT_GET(tmp) == 1)
          {
             eina_hash_del(signal_match, tmp, tmp);
             tmp->hashed = EINA_FALSE;
          }
        else
          {
             Edje_Signal_Callback_Matches *tmp_dup =
               (Edje_Signal_Callback_Matches *)
               _edje_signal_callback_matches_dup(tmp);
             if (!tmp_dup) return EINA_FALSE;
             EINA_REFCOUNT_UNREF(tmp)
               (void) 0; // do nothing because if refcount == 1 handle above.
             gp->matches = tmp = tmp_dup;
          }
        assert(tmp->hashed == EINA_FALSE);
     }

   // search an empty spot now
   for (i = 0; i < tmp->matches_count; i++)
     {
        if (gp->flags[i].delete_me)
          {
             _edje_signal_callback_unset(gp, i);
             _edje_signal_callback_set(gp, i, sig, src, func, data, flags);
             return EINA_TRUE;
          }
     }

   _edje_signal_callback_grow(gp);
   // Set propagate and just_added flags
   _edje_signal_callback_set(gp, tmp->matches_count - 1,
                             sig, src, func, data, flags);

   return EINA_TRUE;
}

const Edje_Signal_Callback_Group *
_edje_signal_callback_alloc(void)
{
   Edje_Signal_Callback_Group *escg;
   Edje_Signal_Callback_Matches *m;

   escg = calloc(1, sizeof (Edje_Signal_Callback_Group));
   if (!escg) return NULL;

   m = calloc(1, sizeof (Edje_Signal_Callback_Matches));
   if (!m)
     {
        free(escg);
        return NULL;
     }

   EINA_REFCOUNT_REF(m);
   escg->matches = m;

   return escg;
}

void
_edje_signal_callback_matches_unref(Edje_Signal_Callback_Matches *m)
{
   EINA_REFCOUNT_UNREF(m)
   {
      unsigned int i;

      _edje_signal_callback_patterns_unref(m->patterns);

      if (m->hashed)
        {
           eina_hash_del(signal_match, m, m);
           m->hashed = EINA_FALSE;
        }

      for (i = 0; i < m->matches_count; ++i)
        {
           eina_stringshare_del(m->matches[i].signal);
           eina_stringshare_del(m->matches[i].source);
        }
      free(m->matches);
      m->matches = NULL;
      free(m);
   }
}

void
_edje_signal_callback_free(const Edje_Signal_Callback_Group *cgp)
{
   Edje_Signal_Callback_Group *gp = (Edje_Signal_Callback_Group *)cgp;

   if (!gp) return;

   _edje_signal_callback_matches_unref((Edje_Signal_Callback_Matches *)gp->matches);
   gp->matches = NULL;
   free(gp->flags);
   gp->flags = NULL;
   free(gp->custom_data);
   gp->custom_data = NULL;
   free(gp);
}

Eina_Bool
_edje_signal_callback_disable(Edje_Signal_Callback_Group *gp,
                              const char *sig, const char *src,
                              Edje_Signal_Cb func, void *data)
{
   unsigned int i;

   if (!gp || !gp->matches) return EINA_FALSE;

   for (i = 0; i < gp->matches->matches_count; ++i)
     {
        if (sig == gp->matches->matches[i].signal &&
            src == gp->matches->matches[i].source &&
            func == gp->matches->matches[i].func &&
            gp->custom_data[i] == data &&
            !gp->flags[i].delete_me)
          {
             gp->flags[i].delete_me = EINA_TRUE;
             //return gp->custom_data[i];
             return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}

static void
_edje_signal_callback_move_last(Edje_Signal_Callback_Group *gp,
                                unsigned int i)
{
   Edje_Signal_Callback_Matches *m;
   unsigned int j;

   m = (Edje_Signal_Callback_Matches *)gp->matches;

   for (j = --m->matches_count; j > i; --j)
     {
        if (!gp->flags[j].delete_me)
          {
             _edje_signal_callback_unset(gp, i);
             m->matches[i].signal = m->matches[j].signal;
             m->matches[i].source = m->matches[j].source;
             m->matches[i].func = m->matches[j].func;
             gp->flags[i] = gp->flags[j];
             gp->custom_data[i] = gp->custom_data[j];
             return;
          }
        else
          {
             _edje_signal_callback_unset(gp, j);
             m->matches_count--;
          }
     }
}

const Edje_Signals_Sources_Patterns *
_edje_signal_callback_patterns_ref(const Edje_Signal_Callback_Group *gp)
{
   const Edje_Signal_Callback_Matches *m;
   Edje_Signal_Callback_Matches *tmp;

   if (gp->matches->hashed)
     goto got_it;

   m = eina_hash_find(signal_match, gp->matches);
   if (!m)
     {
        tmp = (Edje_Signal_Callback_Matches *)gp->matches;
        if (!(tmp->patterns && (EINA_REFCOUNT_GET(tmp->patterns) > 1)))
          {
             // Let compact it and remove uneeded pattern before building it
             // We can do that because the custom data are kept local into the matching code.
             unsigned int i;

             for (i = 0; i < tmp->matches_count; i++)
               {
                  if (gp->flags[i].delete_me)
                    _edje_signal_callback_move_last((Edje_Signal_Callback_Group *)gp, i);
               }
          }

        _edje_signal_callback_patterns_unref(tmp->patterns);
        tmp->patterns = NULL;

        _edje_callbacks_patterns_init((Edje_Signal_Callback_Group *)gp);
        eina_hash_add(signal_match, tmp, tmp);
        // We should be able to use direct_add, but if I do so valgrind stack explode and
        // it bagain to be a pain to debug efl apps. I can't understand what is going on.
        // eina_hash_direct_add(signal_match, tmp, tmp);
        tmp->hashed = EINA_TRUE;
     }
   else
     {
        _edje_signal_callback_matches_unref((Edje_Signal_Callback_Matches *)gp->matches);
        ((Edje_Signal_Callback_Group *)gp)->matches = m;
        tmp = (Edje_Signal_Callback_Matches *)gp->matches;
        EINA_REFCOUNT_REF(tmp);
     }

got_it:
   tmp = (Edje_Signal_Callback_Matches *)gp->matches;
   if (tmp->patterns) EINA_REFCOUNT_REF(tmp->patterns);
   return gp->matches->patterns;
}

void
_edje_signal_callback_patterns_unref(const Edje_Signals_Sources_Patterns *essp)
{
   Edje_Signals_Sources_Patterns *ssp;

   if (!essp) return;

   ssp = (Edje_Signals_Sources_Patterns *)essp;

   EINA_REFCOUNT_UNREF(ssp)
   {
      _edje_signals_sources_patterns_clean(ssp);

      eina_rbtree_delete(ssp->exact_match,
                         EINA_RBTREE_FREE_CB(edje_match_signal_source_free),
                         NULL);
      ssp->exact_match = NULL;

      eina_inarray_flush(&ssp->u.callbacks.globing);

      free(ssp);
   }
}

void
_edje_signal_callback_reset(Edje_Signal_Callback_Flags *flags, unsigned int length)
{
   unsigned int i;
   for (i = 0; i < length; ++i)
     flags[i].just_added = EINA_FALSE;
}

