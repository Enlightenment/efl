#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "evil_fnmatch_private.h"

struct list_of_states*
fnmatch_list_of_states_alloc(size_t n,
                             size_t pattern_len)
{
  struct list_of_states	*l;

  const size_t	reserved        = pattern_len + 1;
  const size_t	states_size     = sizeof (*l->states) * reserved;
  const size_t	has_size        = sizeof (*l->has) * reserved;
  const size_t	states_has_size = states_size + has_size;
  const size_t	struct_size     = sizeof (*l) + states_has_size;

  unsigned char *states;
  unsigned char *has;
  size_t         i;

  if (! (l = malloc(n * struct_size)))
    return 0;

  states = (unsigned char *) (l + n);
  has = states + states_size;

  for (i = 0; i < n; ++i)
    {
      l[i].reserved = reserved;
      l[i].states = (size_t *) states;
      l[i].has = (e_bool *) has;
      states += states_has_size;
      has += states_has_size;
    }

  return l;
}

void
fnmatch_list_of_states_free(struct list_of_states *lists,
                            size_t                 n)
{
  assert(lists);

  (void) n;
  free(lists);
}

void
fnmatch_list_of_states_insert(struct list_of_states *list,
                              size_t                 state)
{
  assert(list);
  assert(state < list->reserved);

  if (list->has[state])
    return;

  list->states[list->size++] = state;
  list->has[state] = 1;
}

void
fnmatch_list_of_states_clear(struct list_of_states *list)
{
  assert(list);

  list->size = 0;
  memset(list->has, 0, list->reserved * sizeof (*list->has));
}
