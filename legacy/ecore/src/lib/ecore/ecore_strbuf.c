#include "ecore_private.h"
#include "Ecore.h"
#include "Ecore_Data.h"
#include "Ecore_Str.h"

#define ECORE_STRBUF_INIT_SIZE 32
#define ECORE_STRBUF_INIT_STEP 32
#define ECORE_STRBUF_MAX_STEP 4096

struct _ecore_strbuf
{
  char *buf;
  size_t len;
  size_t size;
  size_t step;
};

static int _ecore_strbuf_resize(Ecore_Strbuf *buf, size_t size);

/**
 * Create a new string buffer
 */
EAPI Ecore_Strbuf *
ecore_strbuf_new(void)
{
  Ecore_Strbuf *buf;

  buf = malloc(sizeof(Ecore_Strbuf));
  if (!buf) return NULL;

  buf->len = 0;
  buf->size = ECORE_STRBUF_INIT_SIZE;
  buf->step = ECORE_STRBUF_INIT_STEP;

  buf->buf = malloc(buf->size);
  buf->buf[0] = '\0';

  return buf;
}

/**
 * Free a string buffer
 * @param buf the buffer to free
 */
EAPI void
ecore_strbuf_free(Ecore_Strbuf *buf)
{
  CHECK_PARAM_POINTER("buf", buf);
  free(buf->buf);
  free(buf);
}

/**
 * Append a string to a buffer, reallocating as necessary.
 * @param buf the Ecore_Strbuf to append to
 * @param str the string to append
 */
EAPI void
ecore_strbuf_append(Ecore_Strbuf *buf, const char *str)
{
  size_t l;
  size_t off = 0;

  CHECK_PARAM_POINTER("buf", buf);
  CHECK_PARAM_POINTER("str", str);

  l = ecore_strlcpy(buf->buf + buf->len, str, buf->size - buf->len);

  while (l > buf->size - buf->len)
    {
        /* we successfully appended this much */
        off += buf->size - buf->len - 1;
        buf->len = buf->size - 1;
        buf->size += buf->step;
        if (buf->step < ECORE_STRBUF_MAX_STEP)
          buf->step *= 2;
        buf->buf = realloc(buf->buf, buf->size);
        *(buf->buf + buf->len) = '\0';

        l = ecore_strlcpy(buf->buf + buf->len, str + off, buf->size - buf->len);
    }
  buf->len += l;
}

/**
 * Insert a string to a buffer, reallocating as necessary.
 * @param buf the Ecore_Strbuf to insert
 * @param str the string to insert
 * @param pos the position to insert the string
 */
EAPI void
ecore_strbuf_insert(Ecore_Strbuf *buf, const char *str, size_t pos)
{
  size_t len;

  CHECK_PARAM_POINTER("buf", buf);
  CHECK_PARAM_POINTER("str", str);

  if (pos >= buf->len)
    {
	ecore_strbuf_append(buf, str);
	return;
    }

  /*
   * resize the buffer if necessary
   */
  len = strlen(str);
  if (!_ecore_strbuf_resize(buf, buf->len + len))
    return;
  /* move the existing text */
  memmove(buf->buf + len + pos, buf->buf + pos, buf->len - pos);
  /* and now insert the given string */
  memcpy(buf->buf + pos, str, len);
  buf->len += len;
  buf->buf[buf->len] = 0;
}

/**
 * Append a character to a string buffer, reallocating as necessary.
 * @param buf the Ecore_Strbuf to append to
 * @param c the char to append
 */
EAPI void
ecore_strbuf_append_char(Ecore_Strbuf *buf, char c)
{
  CHECK_PARAM_POINTER("buf", buf);
  if (buf->len >= buf->size - 1)
  {
      buf->size += buf->step;
      if (buf->step < ECORE_STRBUF_MAX_STEP)
        buf->step *= 2;
      buf->buf = realloc(buf->buf, buf->size);
  }

  buf->buf[(buf->len)++] = c;
  buf->buf[buf->len] = '\0';
}

/**
 * Retrieve a pointer to the contents of a string buffer
 * @param buf the buffer
 *
 * This pointer must not be modified, and will no longer be valid if
 * the Ecore_Strbuf is modified.
 */
EAPI const char *
ecore_strbuf_string_get(Ecore_Strbuf *buf)
{
  CHECK_PARAM_POINTER_RETURN("buf", buf, NULL);
  return buf->buf;
}

/**
 * Retrieve the length of the string buffer content
 * @param buf the buffer
 */
EAPI size_t
ecore_strbuf_length_get(Ecore_Strbuf *buf)
{
  CHECK_PARAM_POINTER_RETURN("buf", buf, 0);
  return buf->len;
}

/**
 * Replace the n-th string with an other string.
 * @param buf the Ecore_Strbuf to work with
 * @param str the string to replace
 * @param with the replaceing string
 * @param n the number of the fitting string
 *
 * @return true on success
 */
EAPI int
ecore_strbuf_replace(Ecore_Strbuf *buf, const char *str, const char *with,
                     unsigned int n)
{
  size_t len1, len2;
  char *spos;
  size_t pos;

  CHECK_PARAM_POINTER_RETURN("buf", buf, 0);
  CHECK_PARAM_POINTER_RETURN("str", str, 0);
  CHECK_PARAM_POINTER_RETURN("with", with, 0);

  if (n == 0)
     return 0;

  spos = buf->buf;
  while (n--)
    {
	spos = strstr(spos, str);
	if (!spos || *spos == '\0')
	   return 0;
	if (n) spos++;
    }

  pos = spos - buf->buf;
  len1 = strlen(str);
  len2 = strlen(with);
  if (len1 != len2)
    {
	/* resize the buffer if necessary */
	if (!_ecore_strbuf_resize(buf, buf->len - len1 + len2))
	   return 0;
	/* move the existing text */
	memmove(buf->buf + pos + len2, buf->buf + pos + len1,
			buf->len - pos - len1);
    }
  /* and now insert the given string */
  memcpy(buf->buf + pos, with, len2);
  buf->len += len2 - len1;
  buf->buf[buf->len] = 0;

  return 1;
}

/**
 * Replace all strings with an other string.
 * @param buf the Ecore_Strbuf to work with
 * @param str the string to replace
 * @param with the replaceing string
 *
 * @return how often the string was replaced
 */
EAPI int
ecore_strbuf_replace_all(Ecore_Strbuf *buf, const char *str, const char *with)
{
  size_t len1, len2, len;
  char *tmp_buf = NULL;
  char *spos;
  size_t pos, start;
  size_t pos_tmp, start_tmp;
  int n = 0;

  CHECK_PARAM_POINTER_RETURN("buf", buf, 0);
  CHECK_PARAM_POINTER_RETURN("str", str, 0);
  CHECK_PARAM_POINTER_RETURN("with", with, 0);

  spos = strstr(buf->buf, str);
  if (!spos || *spos == '\0')
     return 0;

  len1 = strlen(str);
  len2 = strlen(with);

  /* if the size of the two string is equal, it is fairly easy to replace them
   * we don't need to resize the buffer or doing other calculations */
  if (len1 == len2)
    {
	while (spos)
	   {
	       memcpy(spos, with, len2);
	       spos = strstr(spos + len2, str);
	       n++;
	   }
	return n;
    }

  pos = pos_tmp = spos - buf->buf;
  tmp_buf = buf->buf;
  buf->buf = malloc(buf->size);
  if (!buf->buf)
     {
	 buf->buf = tmp_buf;
	 return 0;
     }
  start = start_tmp = 0;
  len = buf->len;

  while (spos)
     {
	n++;
	len = (len + len2) - len1;
	/* resize the buffer if necessary */
	if (!_ecore_strbuf_resize(buf, len))
	   {
	       /* we have to stop replacing here, because we haven't enough
		* memory to go on */
	       len = (len + len1) - len2;
	       break;
	   }

	/* copy the untouched text */
	memcpy(buf->buf + start, tmp_buf + start_tmp, pos - start);
	/* copy the new string */
	memcpy(buf->buf + pos, with, len2);

	/* calculate the next positions */
	start_tmp = pos_tmp + len1;
	start = pos + len2;
	spos = strstr(tmp_buf + start_tmp, str);
	/* this calculations don't make sense if spos == NULL, but the
	 * calculated values won't be used, because the loop will stop
	 * then */
	pos_tmp = spos - tmp_buf;
	pos = start + pos_tmp - start_tmp;
    }
  /* and now copy the rest of the text */
  memcpy(buf->buf + start, tmp_buf + start_tmp, len - start);
  buf->len = len;
  buf->buf[buf->len] = 0;

  free(tmp_buf);

  return n;
}


/**
 * resize the buffer
 * @param buf the buffer to resize
 * @param size the minimum size of the buffer
 */
static int
_ecore_strbuf_resize(Ecore_Strbuf *buf, size_t size)
{
  char *buffer;
  size_t new_size;
  size_t new_step;

  new_size = buf->size;
  new_step = buf->step;

  /*
   * first we have to determine the new buffer size
   */
  if (size == buf->size)
    /* nothing to do */
    return 1;
  else if (size > buf->size)
    {
	/* enlarge the buffer */
        while (size > new_size)
           {
               new_size += new_step;
               if (new_step < ECORE_STRBUF_MAX_STEP)
                  new_step *= 2;
	   }
    }
  else
    {
        /* shrink the buffer */
	/*
	 * to be done
	 */
	return 1;
    }

  /* reallocate the buffer to the new size */
  buffer = realloc(buf->buf, new_size);
  if (!buffer)
    return 0;

  buf->buf = buffer;
  buf->size = new_size;
  buf->step = new_step;
  return 1;
}
