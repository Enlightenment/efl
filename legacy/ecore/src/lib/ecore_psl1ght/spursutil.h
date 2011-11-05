/*
 * Copyright (C) Antonio José Ramos Márquez (bigboss)
 * Copyright (C) Youness Alaoui (KaKaRoTo)
 */

#ifndef __SPURSUTIL_H__
#define __SPURSUTIL_H__

#include <ppu-types.h>
#include <spurs/spurs.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize spurs with a given name prefix (*/
Spurs *initSpurs(const char *prefix_name);
void   endSpurs(Spurs *spurs);

#ifdef __cplusplus
}
#endif

#endif /* __SPURSUTIL_H__ */
