#ifndef EINA_ERROR_H_
#define EINA_ERROR_H_

#include "eina_types.h"

#define EINA_ERROR_PERR(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_ERR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EINA_ERROR_PINFO(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EINA_ERROR_PWARN(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EINA_ERROR_PDBG(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_DBG, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum _Eina_Error_Level
{
	EINA_ERROR_LEVEL_ERR,
	EINA_ERROR_LEVEL_WARN,
	EINA_ERROR_LEVEL_INFO,
	EINA_ERROR_LEVEL_DBG,
	EINA_ERROR_LEVELS
} Eina_Error_Level;

EAPI int eina_error_init(void);
EAPI int eina_error_shutdown(void);
EAPI int eina_error_register(const char *msg);
EAPI int eina_error_get(void);
EAPI void eina_error_set(int err);
EAPI const char * eina_error_msg_get(int error);
EAPI void eina_error_print(Eina_Error_Level level, const char *file,
		const char *function, int line, const char *fmt, ...);
EAPI void eina_error_log_level_set(Eina_Error_Level level);

#endif /*EINA_ERROR_H_*/
