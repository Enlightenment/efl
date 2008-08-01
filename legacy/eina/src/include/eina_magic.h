#ifndef EINA_MAGIC_H_
#define EINA_MAGIC_H_

#ifdef DEBUG

#include "eina_types.h"

#define EINA_MAGIC_NONE            0x1234fedc

#define EINA_MAGIC	Eina_Magic __magic;

#define EINA_MAGIC_SET(d, m)       (d)->__magic = (m)
#define EINA_MAGIC_CHECK(d, m)     ((d) && ((d)->__magic == (m)))
#define EINA_MAGIC_FAIL(d, m)      eina_magic_fail((d), (d) ? (d)->__magic : 0, (m), __FILE__, __FUNCTION__, __LINE__);

typedef unsigned int               Eina_Magic;

EAPI int eina_magic_string_init();
EAPI int eina_magic_string_shutdown();

EAPI const char* eina_magic_string_get(Eina_Magic magic);
EAPI void eina_magic_string_set(Eina_Magic magic, const char *magic_name);
EAPI void eina_magic_fail(void *d, Eina_Magic m, Eina_Magic req_m,
			  const char *file, const char *fnc, int line);

#else

#define EINA_MAGIC_NONE            (void)
#define EINA_MAGIC
#define EINA_MAGIC_SET(d, m)       (void)
#define EINA_MAGIC_CHECK(d, m)	   (1)
#define EINA_MAGIC_FAIL(d, m)      (void)

#define eina_magic_string_get(Magic)                  (NULL)
#define eina_magic_string_set(Magic, Magic_Name)      (void)
#define eina_magic_fail(d, m, req_m, file, fnx, line) (void)
#define eina_magic_string_init()   (0)
#define eina_magic_string_shutdown() (0)

#endif

#endif /* EINA_MAGIC_H_ */
