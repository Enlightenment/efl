#include "edje_private.h"
#include <assert.h>

static Eina_Hash *signal_match = NULL;

static unsigned int
_edje_signal_match_key_length(const void *key EINA_UNUSED)
{
   return sizeof(Edje_Signal_Callback_Matches);
}

static int
_edje_signal_match_key_cmp(const void *key1, int key1_length EINA_UNUSED, const void *key2, int key2_length EINA_UNUSED)
{
   const Edje_Signal_Callback_Matches *a = key1;
   const Edje_Signal_Callback_Matches *b = key2;
   unsigned int i;

#define NOTEQUAL(x) (a->x != b->x)
#define LESSMORE(x) ((a->x > b->x) ? 1 : -1)
   if (NOTEQUAL(matches_count)) return LESSMORE(matches_count);
   for (i = 0; i < a->matches_count; ++i)
     {
        if (NOTEQUAL(matches[i].signal)) return LESSMORE(matches[i].signal);
        if (NOTEQUAL(matches[i].source)) return LESSMORE(matches[i].source);
        // Callback be it legacy or eo, have the same pointer size and so can be just compared like that
        if (NOTEQUAL(matches[i].legacy)) return LESSMORE(matches[i].legacy);
        if (a->free_cb && b->free_cb)
          {
             if (NOTEQUAL(free_cb[i])) return LESSMORE(free_cb[i]);
          }
        else if (a->free_cb || b->free_cb) return LESSMORE(free_cb);
     }
   return 0;
}

static int
_edje_signal_match_key_hash(const void *key, int key_length EINA_UNUSED)
{
   const Edje_Signal_Callback_Matches *a = key;
   unsigned int hash, i;

   hash = eina_hash_int32(&a->matches_count, sizeof(int));
   for (i = 0; i < a->matches_count; ++i)
     {
#ifdef EFL64
# define HASH(x) eina_hash_int64((const unsigned long long int *)&(a->x), sizeof(a->x))
#else
# define HASH(x) eina_hash_int32((const unsigned int *)&(a->x), sizeof(a->x))
#endif
        hash ^= HASH(matches[i].signal);
        hash ^= HASH(matches[i].source);
        // Callback be it legacy or eo, have the same pointer size and so using legacy for hash is enough
        hash ^= HASH(matches[i].legacy);
        if (a->free_cb) hash ^= HASH(free_cb[i]);
     }
   return hash;
}

static Edje_Signal_Callback_Matches *
_edje_signal_callback_matches_dup(const Edje_Signal_Callback_Matches *src)
{
   Edje_Signal_Callback_Matches *result;
   unsigned int i;

   result = calloc(1, sizeof (Edje_Signal_Callback_Matches));
   if (!result) return NULL;

   result->matches = malloc
     (sizeof(Edje_Signal_Callback_Match) * src->matches_count);
   if (!result->matches) goto err;
   result->matches_count = src->matches_count;
   EINA_REFCOUNT_REF(result);

   if (src->free_cb)
     {
        result->free_cb = malloc
          (sizeof(Eina_Free_Cb) * src->matches_count);
        if (!result->free_cb) goto err;
        memcpy(result->free_cb, src->free_cb,
               sizeof(Eina_Free_Cb) * src->matches_count);
     }

   for (i = 0; i < src->matches_count; i++)
     {
        result->matches[i].signal = eina_stringshare_ref(src->matches[i].signal);
        result->matches[i].source = eina_stringshare_ref(src->matches[i].source);
        result->matches[i].legacy = src->matches[i].legacy;
     }

   return result;
err:
   ERR("Allocation error in callback matches dup");
   free(result->free_cb);
   free(result->matches);
   free(result);
   return NULL;
}

void
_edje_callbacks_patterns_clean(Edje_Signal_Callback_Group *gp)
{
   Edje_Signal_Callback_Matches *tmp = (Edje_Signal_Callback_Matches *)gp->matches;

   if (!tmp) return;
   _edje_signal_callback_patterns_unref(tmp->patterns);
   tmp->patterns = NULL;
}

static void
_edje_callbacks_patterns_init(Edje_Signal_Callback_Group *gp)
{
   Edje_Signals_Sources_Patterns *ssp;
   Edje_Signal_Callback_Matches *tmp = (Edje_Signal_Callback_Matches *)gp->matches;

   if (!tmp) return;
   if (tmp->patterns) return;

   tmp->patterns = calloc(1, sizeof(Edje_Signals_Sources_Patterns));
   if (!tmp->patterns) goto err;

   ssp = tmp->patterns;
   edje_match_callback_hash_build(tmp->matches,
                                  tmp->matches_count,
                                  &ssp->exact_match,
                                  &ssp->u.callbacks.globing);
   ssp->signals_patterns = edje_match_callback_signal_init
     (&ssp->u.callbacks.globing, tmp->matches);
   ssp->sources_patterns = edje_match_callback_source_init
     (&ssp->u.callbacks.globing, tmp->matches);
   EINA_REFCOUNT_REF(ssp);
   return;
err:
   ERR("Alloc error on patterns init");
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
   Edje_Signal_Callback_Matches *tmp = (Edje_Signal_Callback_Matches *)gp->matches;
   Edje_Signal_Callback_Match *m;

   if (!tmp) return;
   if (!tmp->matches) return;
   m = tmp->matches + idx;
   eina_stringshare_del(m->signal);
   m->signal = NULL;
   eina_stringshare_del(m->source);
   m->source = NULL;
}

static void
_edje_signal_callback_set(Edje_Signal_Callback_Group *gp, int idx,
                          const char *sig, const char *src,
                          Edje_Signal_Cb func_legacy,
                          Efl_Signal_Cb func_eo, Eina_Free_Cb func_free_cb,
                          void *data, Edje_Signal_Callback_Flags flags)
{
   Edje_Signal_Callback_Matches *tmp = (Edje_Signal_Callback_Matches *)gp->matches;
   Edje_Signal_Callback_Match *m;

   if (!tmp) return;
   if (!tmp->matches) return;
   m = tmp->matches + idx;
   m->signal = eina_stringshare_ref(sig);
   m->source = eina_stringshare_ref(src);
   if (func_legacy) m->legacy = func_legacy;
   else m->eo = func_eo;
   if (func_free_cb)
     {
        if (!tmp->free_cb)
          tmp->free_cb = calloc(tmp->matches_count, sizeof(Eina_Free_Cb));
        if (!tmp->free_cb) goto err;
        tmp->free_cb[idx] = func_free_cb;
     }
   gp->custom_data[idx] = data;
   gp->flags[idx] = flags;
   return;
err:
   ERR("Alloc err in callback set");
}

static Edje_Signal_Callback_Group *
_edje_signal_callback_grow(Edje_Signal_Callback_Group *gp)
{
   Edje_Signal_Callback_Matches *tmp;
   Edje_Signal_Callback_Match *m;
   Eina_Free_Cb *f;
   void **cd;
   Edje_Signal_Callback_Flags *fl;

   tmp = (Edje_Signal_Callback_Matches *)gp->matches;
   if (!tmp) return NULL;
   tmp->matches_count++;
   // what about data in the data build by edje_match_callback_hash_build
   // that this may kill by changing the tmp->matches ptr. this is handled
   // in _edje_signal_callback_push() by re-initting patterns
   m = realloc(tmp->matches, sizeof(Edje_Signal_Callback_Match) * tmp->matches_count);
   if (!m) goto err;
   tmp->matches = m;
   memset(&(tmp->matches[tmp->matches_count - 1]), 0, sizeof(Edje_Signal_Callback_Match));
   if (tmp->free_cb)
     {
        f = realloc(tmp->free_cb, sizeof(Eina_Free_Cb) * tmp->matches_count);
        if (!f) goto err;
        tmp->free_cb = f;
        tmp->free_cb[tmp->matches_count - 1] = NULL;
     }
   cd = realloc(gp->custom_data, sizeof(void *) * tmp->matches_count);
   if (!cd) goto err;
   gp->custom_data = cd;
   gp->custom_data[tmp->matches_count - 1] = NULL;
   fl = realloc(gp->flags, sizeof(Edje_Signal_Callback_Flags) * tmp->matches_count);
   if (!fl) goto err;
   gp->flags = fl;
   memset(&(gp->flags[tmp->matches_count - 1]), 0, sizeof(Edje_Signal_Callback_Flags));
   return gp;
err:
   ERR("Allocation error in rowing signal callback group");
   tmp->matches_count--;
   return NULL;
}

Eina_Bool
_edje_signal_callback_push(Edje_Signal_Callback_Group *gp,
                           const char *sig, const char *src,
                           Edje_Signal_Cb func_legacy,
                           Efl_Signal_Cb func_eo, Eina_Free_Cb func_free_cb,
                           void *data, Eina_Bool propagate)
{
   unsigned int i;
   Edje_Signal_Callback_Flags flags;
   Edje_Signal_Callback_Matches *tmp;
   Edje_Signal_Callback_Match *m;

   flags.delete_me = EINA_FALSE;
   flags.just_added = EINA_TRUE;
   flags.propagate = !!propagate;
   flags.legacy = !!func_legacy;

   // FIXME: properly handle legacy and non legacy case, including free function
   tmp = (Edje_Signal_Callback_Matches *)gp->matches;
   if (!tmp) return EINA_FALSE;
   if (tmp->hashed)
     {
        if (EINA_REFCOUNT_GET(tmp) == 1)
          {
             // special case - it's a single ref so make it private
             // and move it out of the shared hash to be private
             if (!eina_hash_del(signal_match, tmp, tmp))
               {
                  ERR("Can't del from hash!");
               }
             tmp->hashed = EINA_FALSE;
          }
        else
          {
             // already multiple refs to the match - so make a
             // private copy of it we can modify
             Edje_Signal_Callback_Matches *tmp_dup =
               _edje_signal_callback_matches_dup(tmp);
             if (!tmp_dup) return EINA_FALSE;
             // unreff tmp but it's > 1 ref so it'll be safe but we're not
             // using it anymore here so indicate that with the unref
             EINA_REFCOUNT_UNREF(tmp)
               {
                  (void)0; // do nothing because if refcount == 1 handle above.
               }
             gp->matches = tmp = tmp_dup;
          }
     }

   // tmp will not be hashed at this point so no need to del+add from hash
   // search an empty spot now
   for (i = 0; i < tmp->matches_count; i++)
     {
        if (gp->flags[i].delete_me)
          {
             _edje_signal_callback_unset(gp, i);
             _edje_signal_callback_set(gp, i, sig, src, func_legacy, func_eo, func_free_cb, data, flags);
             return EINA_TRUE;
          }
     }

   m = tmp->matches;
   if (_edje_signal_callback_grow(gp))
     {
        // Set propagate and just_added flags
        _edje_signal_callback_set(gp, tmp->matches_count - 1,
                                  sig, src, func_legacy, func_eo, func_free_cb, data, flags);
        if (m != tmp->matches)
          {
             _edje_callbacks_patterns_clean(gp);
             _edje_callbacks_patterns_init(gp);
          }
     }
   else
     {
        if (tmp->hashed)
          eina_hash_add(signal_match, tmp, tmp);
        goto err;
     }
   return EINA_TRUE;
err:
   ERR("Allocation error in pushing callback");
   return EINA_TRUE;
}

const Edje_Signal_Callback_Group *
_edje_signal_callback_alloc(void)
{
   Edje_Signal_Callback_Group *escg;
   Edje_Signal_Callback_Matches *m;

   escg = calloc(1, sizeof (Edje_Signal_Callback_Group));
   if (!escg) goto err;
   m = calloc(1, sizeof (Edje_Signal_Callback_Matches));
   if (!m) goto err;
   EINA_REFCOUNT_REF(m);
   escg->matches = m;
   return escg;
err:
   ERR("Alloc error in signal callback alloc");
   free(escg);
   return NULL;
}

void
_edje_signal_callback_matches_unref(Edje_Signal_Callback_Matches *m,
                                    Edje_Signal_Callback_Flags *flags,
                                    void **custom_data)
{
   unsigned int i;

   if (m->free_cb)
     {
        for (i = 0; i < m->matches_count; ++i)
          {
             if (!flags[i].delete_me && m->free_cb[i])
               m->free_cb[i](custom_data[i]);
          }
     }

   EINA_REFCOUNT_UNREF(m)
     {
        if (m->hashed)
          {
             if (!eina_hash_del(signal_match, m, m))
               {
                  ERR("Can't del from hash!");
               }
          }
        for (i = 0; i < m->matches_count; ++i)
          {
             eina_stringshare_del(m->matches[i].signal);
             eina_stringshare_del(m->matches[i].source);
             m->matches[i].signal = NULL;
             m->matches[i].source = NULL;
          }
        _edje_signal_callback_patterns_unref(m->patterns);
        free(m->matches);
        free(m->free_cb);
        m->patterns = NULL;
        m->matches = NULL;
        m->free_cb = NULL;
        m->hashed = EINA_FALSE;
        free(m);
     }
}

void
_edje_signal_callback_free(const Edje_Signal_Callback_Group *cgp)
{
   Edje_Signal_Callback_Group *gp = (Edje_Signal_Callback_Group *)cgp;

   if (!gp) return;
   _edje_signal_callback_matches_unref
     ((Edje_Signal_Callback_Matches *)gp->matches, gp->flags, gp->custom_data);
   free(gp->flags);
   free(gp->custom_data);
   gp->matches = NULL;
   gp->flags = NULL;
   gp->custom_data = NULL;
   free(gp);
}

Eina_Bool
_edje_signal_callback_disable(Edje_Signal_Callback_Group *gp,
                              const char *sig, const char *src,
                              Edje_Signal_Cb func_legacy,
                              EflLayoutSignalCb func, Eina_Free_Cb func_free_cb, void *data)
{
   unsigned int i;

   if (!gp || !gp->matches) return EINA_FALSE;

   for (i = 0; i < gp->matches->matches_count; ++i)
     {
        if ((sig == gp->matches->matches[i].signal) &&
            (src == gp->matches->matches[i].source) &&
            (!gp->flags[i].delete_me) &&
            (((func == gp->matches->matches[i].eo) &&
              ((!gp->matches->free_cb) || (func_free_cb == gp->matches->free_cb[i])) &&
              (gp->custom_data[i] == data) &&
              (!gp->flags[i].legacy)) ||
             ((func_legacy == gp->matches->matches[i].legacy) &&
              (gp->custom_data[i] == data) &&
              (gp->flags[i].legacy)))
            )
          {
             if (func && func_free_cb) func_free_cb(data);
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
   if (!m) return;

   if (m->hashed)
     {
        if (!eina_hash_del(signal_match, m, m))
          {
             ERR("Can't del from hash!");
          }
     }
   for (j = --m->matches_count; j > i; --j)
     {
        if (!gp->flags[j].delete_me)
          {
             _edje_signal_callback_unset(gp, i);
             memcpy(&m->matches[i], &m->matches[j], sizeof(Edje_Signal_Callback_Match));
             gp->flags[i] = gp->flags[j];
             gp->custom_data[i] = gp->custom_data[j];
             if (m->hashed)
               eina_hash_add(signal_match, m, m);
             return;
          }
        else
          {
             _edje_signal_callback_unset(gp, j);
             m->matches_count--;
          }
     }
   if (m->hashed)
     eina_hash_add(signal_match, m, m);
}

const Edje_Signals_Sources_Patterns *
_edje_signal_callback_patterns_ref(Edje_Signal_Callback_Group *gp)
{
   const Edje_Signal_Callback_Matches *m;
   Edje_Signal_Callback_Matches *tmp;

   tmp = (Edje_Signal_Callback_Matches *)gp->matches;
   if (!tmp) return NULL;
   if (tmp->hashed) goto got_it;
   m = eina_hash_find(signal_match, tmp);
   if (!m)
     {
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
        m = eina_hash_find(signal_match, tmp);
        if (m)
          {
             WRN("Found exact match in signal matches this would conflict with");
             goto got_it;
          }
        // We should be able to use direct_add, but if I do so valgrind stack explode and
        // it bagain to be a pain to debug efl apps. I can't understand what is going on.
        // eina_hash_direct_add(signal_match, tmp, tmp);
        eina_hash_add(signal_match, tmp, tmp);
        tmp->hashed = EINA_TRUE;
     }
   else
     {
        if (m == tmp)
          {
             WRN("Should not happen - gp->match == hash found match");
             goto got_it;
          }
        if (tmp->matches_count != m->matches_count)
          {
             unsigned int i, smaller, larger;
             void **cd;
             Edje_Signal_Callback_Flags *fl;

             ERR("Match replacement match count don't match");
             smaller = tmp->matches_count;
             larger = m->matches_count;
             if (larger > smaller)
               {
                  cd = realloc(gp->custom_data, sizeof(void *) * larger);
                  for (i = smaller; i < larger; i++) cd[i] = NULL;
                  gp->custom_data = cd;
                  fl = realloc(gp->flags, sizeof(Edje_Signal_Callback_Flags) * larger);
                  for (i = smaller; i < larger; i++) memset(&(fl[i]), 0, sizeof(Edje_Signal_Callback_Flags));
                  gp->flags = fl;
               }
          }
        _edje_signal_callback_matches_unref
          ((Edje_Signal_Callback_Matches *)tmp, gp->flags, gp->custom_data);
        ((Edje_Signal_Callback_Group *)gp)->matches = m;
        tmp = (Edje_Signal_Callback_Matches *)gp->matches;
        EINA_REFCOUNT_REF(tmp);
     }

got_it:
   if (tmp->patterns) EINA_REFCOUNT_REF(tmp->patterns);
   return tmp->patterns;
}

void
_edje_signal_callback_patterns_unref(const Edje_Signals_Sources_Patterns *essp)
{
   Edje_Signals_Sources_Patterns *ssp = (Edje_Signals_Sources_Patterns *)essp;

   if (!ssp) return;
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

