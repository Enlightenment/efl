#include "eina_error.h"
#include "eina_list.h"
#include "eina_private.h"
/* TODO
 * + printing errors to stdout or stderr can be implemented
 * using a queue, usful for multiple threads printing
 * + add a wapper for assert?
 * + add common error numbers, messages
 * + add a calltrace of erros, not only store the last error but a list of them
 * and also store the function that set it
 */
/*============================================================================*
 *                                  Local                                     * 
 *============================================================================*/
static int _curr = 1;
static int _init_count = 0;
static Eina_List *_error_list;
static int _err;

#define RED     "\033[31;1m"
#define GREEN   "\033[32;1m"
#define YELLOW  "\033[33;1m"
#define WHITE   "\033[37;1m"
#define NOTHING "\033[0m"


#ifdef DEBUG
static Eina_Error_Level _error_level = EINA_ERROR_LEVEL_DBG;
#else
static Eina_Error_Level _error_level = EINA_ERROR_LEVEL_ERR;
#endif

static char *_colors[EINA_ERROR_LEVELS] = {
	[EINA_ERROR_LEVEL_ERR] = RED,
	[EINA_ERROR_LEVEL_WARN] = YELLOW,
	[EINA_ERROR_LEVEL_INFO] = NOTHING,
	[EINA_ERROR_LEVEL_DBG] = GREEN,
};

static void _error_print(Eina_Error_Level level, const char *file,
		const char *fnc, int line, const char *fmt, va_list args)
{
	if (level <= _error_level)
	{
		printf("%s", _colors[level]);
		printf("[%s:%d] %s() ", file, line, fnc);
		printf("%s", _colors[EINA_ERROR_LEVEL_INFO]);
		vprintf(fmt, args);
	}
}
/*============================================================================*
 *                                 Global                                     * 
 *============================================================================*/
void eina_error_magic_check(unsigned int magic, unsigned int cmp)
{
	assert(magic == cmp);
}
/*============================================================================*
 *                                   API                                      * 
 *============================================================================*/
/**
 * 
 */
EAPI int eina_error_init(void)
{
	if (!_init_count) 
	{
		char *level;
		/* TODO register the eina's basic errors */
		/* TODO load the environment variable for getting the log level */
		if ((level = getenv("EINA_ERROR_LEVEL")))
		{
			_error_level = atoi(level);
		}
	}
	/* get all the modules */
	return ++_init_count;
}
/**
 * 
 */
EAPI int eina_error_shutdown(void)
{
	if (!_init_count)
		return _init_count;
	_init_count--;
	if (!_init_count)
	{
		/* remove the error strings */
		while (_error_list)
		{
			free(eina_list_data(_error_list));
			_error_list = eina_list_free(_error_list);
		}
	}
	return _init_count;
}

/**
 * Register a new error type
 * @param str The description of the error
 * @return The unique number identifier for this error
 */
EAPI int eina_error_register(const char *msg)
{
	char *str;
	
	str = strdup(msg);
	_error_list = eina_list_append(_error_list, str);
	
	return ++_curr;
}
/**
 * 
 */
EAPI int eina_error_get(void)
{
	return _err;
}
/**
 *  
 */
EAPI void eina_error_set(int err)
{
	_err = err;
}
/**
 * Given an error number return the description of it
 * @param error The error number
 * @return The description of the error
 */
EAPI const char * eina_error_msg_get(int error)
{
	return eina_list_nth(_error_list, error);
}
/**
 * 
 */
EAPI void eina_error_print(Eina_Error_Level level, const char *file, 
		const char *fnc, int line, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	_error_print(level, file, fnc, line, fmt, args);
	va_end(args);
}
/**
 * 
 */
EAPI void eina_error_log_level_set(Eina_Error_Level level)
{
	_error_level = level;
}
