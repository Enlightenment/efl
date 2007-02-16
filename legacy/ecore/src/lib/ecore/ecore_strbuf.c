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
  int len;
  int size;
  int step;
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
  CHECK_PARAM_POINTER("buf", buf); 
  CHECK_PARAM_POINTER("str", str); 

    int l;
    int off = 0;

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
