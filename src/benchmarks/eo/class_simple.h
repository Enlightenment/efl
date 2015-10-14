#ifndef SIMPLE_H
#define SIMPLE_H

typedef struct
{
   int a;
} Simple_Public_Data;

EAPI void simple_a_set(int a);
/* Calls simple_other_call(other, obj) and then simple_other_call(obj, other)
 * for 'times' times in order to grow the call stack on other objects. */
EAPI void simple_other_call(Eo *other, int times);

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

EOAPI extern const Eo_Event_Description _SIMPLE_FOO;
EOAPI extern const Eo_Event_Description _SIMPLE_BAR;

#define SIMPLE_FOO (&(_SIMPLE_FOO))
#define SIMPLE_BAR (&(_SIMPLE_BAR))

#endif
