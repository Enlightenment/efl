#include "Eina.h"
#include "eina_private.h"

static void push(void *data, void *element)
{
	
}
static void * pop(void *data, unsigned int size)
{
	return NULL;
}

static void *init(unsigned int size)
{
	return NULL;
}

static void shutdown(void *data)
{
	
}

Eina_Mempool_Backend mp_backend = {
	.init = &init,
	.shutdown = &shutdown,
	.alloc = &pop,
	.free = &push,
};

