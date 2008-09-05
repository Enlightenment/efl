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

#include "eina_mempool.h"
#include "eina_list.h"
#include "eina_module.h"
#include "eina_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

static Eina_Module_Group *_group;
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

static Eina_Mempool *
_new_from_buffer(const char *module, const char *context, const char *options, va_list args)
{
	Eina_Mempool *mp;
	Eina_Module *m;

	eina_error_set(0);
	m = eina_module_new(_group, module);
	if (!m) return NULL;
	if (eina_module_load(m) == EINA_FALSE) {
		eina_error_set(EINA_ERROR_NOT_MEMPOOL_MODULE);
		goto on_error;
	}

	mp = malloc(sizeof(Eina_Mempool));
	if (!mp) {
		eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
		goto on_error;
	}
	mp->module = m;
	mp->backend = eina_module_export_object_get(m);
	mp->backend_data = mp->backend->init(context, options, args);

	return mp;

  on_error:
	if (m) eina_module_delete(m);
	return NULL;
}

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

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
		eina_module_init();

		_group = eina_module_group_new();
		if (!_group) return 0;

		eina_module_app_register(_group, "eina", "mp", NULL);

		EINA_ERROR_NOT_MEMPOOL_MODULE = eina_error_msg_register("Not a memory pool module.");
	}
	/* get all the modules */
	return ++_init_count;
}

/**
 *
 */
EAPI int
eina_mempool_shutdown(void)
{
	if (!_init_count)
		return _init_count;
	_init_count--;
	if (!_init_count)
	{
		/* remove the list of modules */
		eina_module_group_delete(_group);
		eina_module_shutdown();
	}
	return _init_count;
}

/**
 * 
 */
EAPI Eina_Mempool *
eina_mempool_new(const char *name, const char *context, const char *options, ...)
{
	Eina_Mempool *mp;
	va_list args;

	assert(name);

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
	assert(mp);
	assert(mp->backend->realloc);

	return mp->backend->realloc(mp->backend_data, element, size);
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
