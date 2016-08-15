#ifndef _EO_ADD_FALLBACK_H
#define _EO_ADD_FALLBACK_H

#include <Eina.h>
#include <Eo.h>

typedef struct _Eo_Stack_Frame
{
   Eo *obj;
} Eo_Stack_Frame;

Eina_Bool _efl_add_fallback_init(void);
Eina_Bool _efl_add_fallback_shutdown(void);

Eo_Stack_Frame *_efl_add_fallback_stack_push(Eo *obj);
Eo_Stack_Frame *_efl_add_fallback_stack_pop(void);

#endif
