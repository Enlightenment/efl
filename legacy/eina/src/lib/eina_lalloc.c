#include "eina_lalloc.h"
#include "eina_private.h"

struct _Eina_Lalloc
{
	void 	*data;
	int 	num_allocated;
	int 	num_elements;
	int 	acc;
	Eina_Lalloc_Alloc alloc_cb;
	Eina_Lalloc_Free free_cb;
};

/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Eina_Lalloc * eina_lalloc_new(void *data, Eina_Lalloc_Alloc alloc_cb, Eina_Lalloc_Free free_cb, int num_init)
{
	Eina_Lalloc *a;

	a = calloc(1, sizeof(Eina_Lalloc));
	a->data = data;
	a->alloc_cb = alloc_cb;
	a->free_cb = free_cb;
	if (num_init > 0)
	{
		a->num_allocated = num_init;
		a->alloc_cb(a->data, a->num_allocated);
	}
	return a;
}

/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Eina_Bool eina_lalloc_element_add(Eina_Lalloc *a)
{
	if (a->num_elements == a->num_allocated)
	{
		if (a->alloc_cb(a->data, (1 << a->acc)) == EINA_TRUE)
		{
			a->num_allocated = (1 << a->acc);
			a->acc++;
		} else {
			return EINA_FALSE;
		}
	}
	a->num_elements++;

	return EINA_TRUE;
}

/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Eina_Bool eina_lalloc_elements_add(Eina_Lalloc *a, int num)
{
	int tmp;

	tmp = a->num_elements + num;
	if (tmp > a->num_allocated)
	{
		int allocated;
		int acc;

		allocated = a->num_allocated;
		acc = a->acc;

		while (tmp > allocated)
		{
			allocated = (1 << acc);
			acc++;
		}

		if (a->alloc_cb(a->data, allocated) == EINA_TRUE)
		{
			a->num_allocated = allocated;
			a->acc = acc;
		} else {
			return EINA_FALSE;
		}
	}
	a->num_elements += num;

	return EINA_TRUE;
}

/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void eina_lalloc_delete(Eina_Lalloc *a)
{
	a->free_cb(a->data);
	free(a);
}
