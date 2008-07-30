#include "eina_mempool.h"
#include "eina_list.h"
#include "eina_module.h"
#include "eina_private.h"
/*============================================================================*
 *                                  Local                                     * 
 *============================================================================*/
static Eina_List *_modules;
static int _init_count = 0;

struct _Eina_Mempool
{
#ifdef DEBUG
	unsigned int magic;
#endif
	Eina_Module *module;
	Eina_Mempool_Backend *backend;
	void *backend_data;
};
static Eina_Mempool * _new_from_buffer(const char *name, void *buffer,
		unsigned int size, const char *options, va_list args)
{
	Eina_List *l;
	
	/* load the module with filename == name */
	for (l = _modules; l; l = eina_list_next(l))
	{
		Eina_Module *m;

		m = eina_list_data(l);
		/* check if the requested module name exists */
		if (!strncmp(eina_module_name_get(m), name, strlen(name)))
		{
			Eina_Mempool *mp;

			mp = malloc(sizeof(Eina_Mempool));
			eina_module_load(m);
			mp->module = m;
			mp->backend = eina_module_symbol_get(m, "mp_backend");
			mp->backend_data = mp->backend->init(buffer, size, options, args);

			return mp;
		}
	}
	return NULL;
}
/*============================================================================*
 *                                   API                                      * 
 *============================================================================*/
/**
 * 
 */
EAPI int eina_mempool_init(void)
{
	if (!_init_count) 
	{
		_modules = eina_module_list_get("/usr/local/lib/eina/mm_policies", 0, NULL, NULL);
	}
	/* get all the modules */
	return ++_init_count;
}
/**
 * 
 */
EAPI int eina_mempool_shutdown(void)
{
	if (!_init_count)
		return _init_count;
	_init_count--;
	if (!_init_count)
	{
		/* remove the list of modules */
		eina_module_list_free(_modules);
	}
	return _init_count;
}
/**
 * 
 */
EAPI Eina_Mempool * eina_mempool_new_from_buffer(const char *name, void *buffer,
		unsigned int size, const char *options, ...)
{
	Eina_Mempool *mp;
	va_list args;
	
	assert(name);
	assert(buffer);
	
	va_start(args, options);
	mp = _new_from_buffer(name, buffer, size, options, args);
	va_end(args);
	
	return mp;	
}
/**
 * 
 */
EAPI Eina_Mempool * eina_mempool_new(const char *name, unsigned int size, const char 
		*options, ...)
{
	Eina_Mempool *mp;
	void *buffer;
	va_list args;
	
	assert(name);
	
	buffer = malloc(sizeof(char) * size);
	va_start(args, options);
	mp = _new_from_buffer(name, buffer, size, options, args);
	va_end(args);
	
	return mp;
}
/**
 * 
 */
EAPI void eina_mempool_delete(Eina_Mempool *mp)
{
	Eina_List *l;
	
	assert(mp);
	
	mp->backend->shutdown(mp->backend_data);
	eina_module_unload(mp->module);
	free(mp);
}
/**
 * 
 */
EAPI void * eina_mempool_realloc(Eina_Mempool *mp, void *element, unsigned int size)
{
	
}
/**
 * 
 */
EAPI void * eina_mempool_alloc(Eina_Mempool *mp, unsigned int size)
{
	assert(mp);
	assert(mp->backend->alloc);
	
	return mp->backend->alloc(mp->backend_data, size);
}
/**
 * 
 */
EAPI void eina_mempool_free(Eina_Mempool *mp, void *element)
{
	assert(mp);
	assert(mp->backend->free);
	
	mp->backend->free(mp->backend_data, element);
}
