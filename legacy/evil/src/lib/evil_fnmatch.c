#if defined (_MSC_VER) || defined (__MINGW32__) || defined (__MINGW32CE__)

#include <assert.h>
#include <string.h>

#include "fnmatch.h"
#include "evil_fnmatch_private.h"

enum fnmatch_status
{
  fnmatch_not_found    = 0,
  fnmatch_found        = 1,
  fnmatch_syntax_error = 2
};

static
size_t
fnmatch_match_class_token(enum fnmatch_status *status,
			  const char          *class_token,
			  char                 c)
{
  if (! *class_token)
    {
      *status = fnmatch_syntax_error;
      return 0;
    }
  else if (class_token[1] == '-' && class_token[2] != ']')
    {
      if (class_token[0] <= c && c <= class_token[2])
	*status = fnmatch_found;
      return 3;
    }
  else
    {
      if (c == *class_token)
	*status = fnmatch_found;
      return 1;
    }
}

static
size_t
fnmatch_complement_class(const char *class_token)
{
  switch (*class_token)
    {
    case 0:
      return FNM_SYNTAXERR;

    case '!':
      return 1;

    default:
      return 0;
    }
}

static
size_t
fnmatch_match_class(const char *class,
		    char        c)
{
  const size_t        complement = fnmatch_complement_class(class + 1);
  enum fnmatch_status status;
  size_t              pos;

  if (complement == FNM_SYNTAXERR)
    return FNM_SYNTAXERR;

  status = fnmatch_not_found;
  pos = 1 + complement;

  do
    pos += fnmatch_match_class_token(&status, class + pos, c);
  while (class[pos] && class[pos] != ']');

  if (status == fnmatch_syntax_error || ! class[pos])
    return FNM_SYNTAXERR;

  if (status == fnmatch_found)
    return complement ? 0 : pos + 1;
  else
    return complement ? pos + 1 : 0;
}

static
size_t
fnmatch_chrcasecmp(char a,
                   char b)
{
  if ('A' <= a && a <= 'Z')
    a += 'a' - 'A';
  if ('A' <= b && b <= 'Z')
    b += 'a' - 'A';
  return a == b;
}

static
size_t
fnmatch_match_token(const char *token,
		    char        c,
		    e_bool      leading,
		    int         flags)
{
  if (*token == '\\' && !(flags & FNM_NOESCAPE))
    return token[1] ? (token[1] == c ? 2 : 0) : FNM_SYNTAXERR;

  if (c == '/' && (flags & FNM_PATHNAME))
    return *token == '/';

  if (c == '.' && leading && (flags & FNM_PERIOD))
    return *token == '.';

  switch (*token)
    {
    case '?':
      return 1;

    case '[':
      return fnmatch_match_class(token, c);

    default:
      if (flags & FNM_CASEFOLD)
	return fnmatch_chrcasecmp(*token, c);
      return *token == c ? 1 : 0;
    }
}

static
void
fnmatch_init_states(struct list_of_states *states)
{
  states->size = 1;
  states->states[0] = 0;
  memset(states->has, 0, states->reserved * sizeof (*states->has));
  states->has[0] = 1;
}

static
size_t
fnmatch_check_finals(const char                  *pattern,
		     const struct list_of_states *states)
{
   size_t i, j;
   for (i = 0; i < states->size; ++i)
     {
       e_bool match = 1;

       for (j = states->states[i]; pattern[j]; ++j)
         if (pattern[j] != '*')
           {
             match = 0;
             break;
           }

       if (match)
         return 0;
     }
   return FNM_NOMATCH;
}

int
fnmatch(const char *pattern,
	const char *string,
	int         flags)
{
   struct list_of_states *states;
   struct list_of_states *new_states;
   e_bool                 leading = 1;
   char                  *c;
   size_t                 r;

  assert(pattern);
  assert(string);

  states = fnmatch_list_of_states_alloc(2, strlen(pattern));
  new_states = states + 1;

  if (! states)
    return FNM_NOMEM;
  fnmatch_init_states(states);


  for (c = (char *)string; *c && states->size; ++c)
    {
       size_t i;
      fnmatch_list_of_states_clear(new_states);

      for (i = 0; i < states->size; ++i)
	{
	  const size_t pos = states->states[i];

	  if (! pattern[pos])
	    {
	      if (*c == '/' && (flags & FNM_LEADING_DIR))
		return 0;
	      continue;
	    }
	  else if (pattern[pos] == '*')
	    {
	      fnmatch_list_of_states_insert(states, pos + 1);
	      if ((*c != '/' || !(flags & FNM_PATHNAME)) &&
		  (*c != '.' || !leading || !(flags & FNM_PERIOD)))
		fnmatch_list_of_states_insert(new_states, pos);
	    }
	  else
	    {
	      const int	m = fnmatch_match_token(pattern + pos, *c,
						leading, flags);

	      if (m == FNM_SYNTAXERR)
		return FNM_SYNTAXERR;
	      else if (m)
		fnmatch_list_of_states_insert(new_states, pos + m);
	    }
	}
      {
	struct list_of_states *tmp = states;

	states = new_states;
	new_states = tmp;
      }
      leading = *c == '/' && (flags & FNM_PATHNAME);
    }

  r = fnmatch_check_finals(pattern, states);
  fnmatch_list_of_states_free(states < new_states ? states : new_states, 2);
  return r;
}

#endif /* _MSC_VER || __MINGW32__ || __MINGW32CE__ */
