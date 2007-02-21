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

/**
 * Create a new string buffer
 */
Ecore_Strbuf *
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
void
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
void
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
void
ecore_strbuf_insert(Ecore_Strbuf *buf, const char *str, size_t pos)
{
  size_t len;
  size_t new_size;

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
  new_size = buf->size;
  while (len + buf->len > new_size)
    {
        new_size += buf->step;
        if (buf->step < ECORE_STRBUF_MAX_STEP)
          buf->step *= 2;
    }

  if (new_size != buf->size)
    {
	buf->size = new_size;
        buf->buf = realloc(buf->buf, buf->size);
    }
  /* move the existing text */
  memmove(buf->buf + len + pos, buf->buf + pos, buf->len - pos);
  /* and now insert the given string */
  strncpy(buf->buf + pos, str, len);
  buf->len += len;
  buf->buf[buf->len] = 0;
}

/**
 * Append a character to a string buffer, reallocating as necessary.
 * @param buf the Ecore_Strbuf to append to
 * @param c the char to append
 */
void
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
const char *
ecore_strbuf_string_get(Ecore_Strbuf *buf)
{
  CHECK_PARAM_POINTER_RETURN("buf", buf, NULL); 
  return buf->buf;
}

/**
 * Retrieve the length of the string buffer content
 * @param buf the buffer
 */
size_t
ecore_strbuf_length_get(Ecore_Strbuf *buf)
{
  CHECK_PARAM_POINTER_RETURN("buf", buf, 0); 
  return buf->len;
}

