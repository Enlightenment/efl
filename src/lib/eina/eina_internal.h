#ifndef EINA_INTERNAL_H
#define EINA_INTERNAL_H

/*
 * eina_internal.h
 *
 * Lists public functions that are meant for internal use by EFL only and are
 * not stable API.
 */

/**
 * @brief Cancels all pending promise/futures.
 */
EAPI void __eina_promise_cancel_all(void);

#endif
