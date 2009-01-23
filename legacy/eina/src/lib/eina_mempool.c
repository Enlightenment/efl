/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eina_mempool.h"
#include "eina_hash.h"
#include "eina_module.h"
#include "eina_private.h"
#include "eina_safety_checks.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

static Eina_Hash *_backends;
static Eina_Array *_modules;
static int _init_count = 0;

static Eina_Mempool *
_new_from_buffer(const char *name, const char *context, const char *options, va_list args)
{
	Eina_Mempool_Backend *be;
	Eina_Mempool *mp;

	Eina_Error err = EINA_ERROR_NOT_MEMPOOL_MODULE;

	eina_error_set(0);
	be = eina_hash_find(_backends, name);
	if (!be)
		goto on_error;

	err = EINA_ERROR_OUT_OF_MEMORY;
	mp = malloc(sizeof(Eina_Mempool));
	if (!mp) goto on_error;

	/* FIXME why backend is not a pointer? */
	mp->backend = *be;
	mp->backend_data = mp->backend.init(context, options, args);

	return mp;

  on_error:
	eina_error_set(err);
	return NULL;
}

/* Built-in backend's prototypes */
#ifdef EINA_STATIC_BUILD_CHAINED_POOL
Eina_Bool chained_init(void);
void chained_shutdown(void);
#endif

#ifdef EINA_STATIC_BUILD_PASS_THROUGH
Eina_Bool pass_through_init(void);
void pass_through_shutdown(void);
#endif

#ifdef EINA_STATIC_BUILD_EMEMOA_UNKNOWN
Eina_Bool ememoa_unknown_init(void);
void ememoa_unknown_shutdown(void);
#endif

#ifdef EINA_STATIC_BUILD_EMEMOA_FIXED
Eina_Bool ememoa_fixed_init(void);
void ememoa_fixed_shutdown(void);
#endif

#ifdef EINA_STATIC_BUILD_FIXED_BITMAP
Eina_Bool fixed_bitmap_init(void);
void fixed_bitmap_shutdown(void);
#endif

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

EAPI Eina_Bool eina_mempool_register(Eina_Mempool_Backend *be)
{
	return eina_hash_add(_backends, be->name, be);
}

EAPI void eina_mempool_unregister(Eina_Mempool_Backend *be)
{
	eina_hash_del(_backends, be->name, be);
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI Eina_Error EINA_ERROR_NOT_MEMPOOL_MODULE = 0;

/**
 *
 */
EAPI int
eina_mempool_init(void)
{
	if (!_init_count)
	{
		char *path;

		eina_hash_init();
		eina_module_init();

		EINA_ERROR_NOT_MEMPOOL_MODULE = eina_error_msg_register("Not a memory pool module.");
		_backends = eina_hash_string_superfast_new(NULL);

		/* dynamic backends */
		_modules = eina_module_list_get(NULL, PACKAGE_LIB_DIR "/eina/mp/", 0, NULL, NULL);

		path = eina_module_environment_path_get("HOME", "/.eina/mp/");
		_modules = eina_module_list_get(_modules, path, 0, NULL, NULL);
		if (path) free(path);

		path = eina_module_environment_path_get("EINA_MODULES_MEMPOOL_DIR", "/eina/mp/");
		_modules = eina_module_list_get(_modules, path, 0, NULL, NULL);
		if (path) free(path);

		path = eina_module_symbol_path_get(eina_mempool_init, "/eina/mp/");
		_modules = eina_module_list_get(_modules, path, 0, NULL, NULL);
		if (path) free(path);

		if (!_modules)
		{
			EINA_ERROR_PERR("ERROR: no mempool modules able to be loaded.\n");
			abort();
		}
		eina_module_list_load(_modules);
		/* builtin backends */
#ifdef EINA_STATIC_BUILD_CHAINED_POOL
		chained_init();
#endif
#ifdef EINA_STATIC_BUILD_PASS_THROUGH
		pass_through_init();
#endif
#ifdef EINA_STATIC_BUILD_EMEMOA_UNKNOWN
		ememoa_unknown_init();
#endif
#ifdef EINA_STATIC_BUILD_EMEMOA_FIXED
		ememoa_fixed_init();
#endif
	}
	return ++_init_count;
}

/**
 *
 */
EAPI int
eina_mempool_shutdown(void)
{
	_init_count--;
	if (_init_count != 0) return _init_count;

	/* builtin backends */
#ifdef EINA_STATIC_BUILD_CHAINED_POOL
	chained_shutdown();
#endif
#ifdef EINA_STATIC_BUILD_PASS_THROUGH
	pass_through_shutdown();
#endif
#ifdef EINA_STATIC_BUILD_EMEMOA_UNKNOWN
	ememoa_unknown_shutdown();
#endif
#ifdef EINA_STATIC_BUILD_EMEMOA_FIXED
	ememoa_fixed_shutdown();
#endif
	/* dynamic backends */
	eina_module_list_unload(_modules);
	eina_module_shutdown();
	/* TODO delete the _modules list */
	eina_hash_shutdown();
	return 0;
}

/**
 *
 */
EAPI Eina_Mempool *
eina_mempool_new(const char *name, const char *context, const char *options, ...)
{
	Eina_Mempool *mp;
	va_list args;

	EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

	va_start(args, options);
	mp = _new_from_buffer(name, context, options, args);
	va_end(args);

	return mp;
}

/**
 *
 */
EAPI void eina_mempool_delete(Eina_Mempool *mp)
{
        EINA_SAFETY_ON_NULL_RETURN(mp);
	EINA_SAFETY_ON_NULL_RETURN(mp->backend.shutdown);
	mp->backend.shutdown(mp->backend_data);
	free(mp);
}

EAPI void eina_mempool_gc(Eina_Mempool *mp)
{
        EINA_SAFETY_ON_NULL_RETURN(mp);
        EINA_SAFETY_ON_NULL_RETURN(mp->backend.garbage_collect);
	mp->backend.garbage_collect(mp->backend_data);
}

EAPI void eina_mempool_statistics(Eina_Mempool *mp)
{
        EINA_SAFETY_ON_NULL_RETURN(mp);
        EINA_SAFETY_ON_NULL_RETURN(mp->backend.statistics);
	mp->backend.statistics(mp->backend_data);
}
