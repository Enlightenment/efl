#ifndef __EVIL_FNMATCH_PRIVATE_H__
#define __EVIL_FNMATCH_PRIVATE_H__


typedef int e_bool;

struct list_of_states
{
  size_t	reserved;
  size_t	size;
  size_t	*states;
  e_bool		*has;
};

struct list_of_states *fnmatch_list_of_states_alloc(size_t n, size_t pattern_len);

void fnmatch_list_of_states_free(struct list_of_states *lists, size_t n);

void fnmatch_list_of_states_insert(struct list_of_states *list, size_t state);

void fnmatch_list_of_states_clear(struct list_of_states *list);


#endif /* __EVIL_FNMATCH_PRIVATE_H__ */
