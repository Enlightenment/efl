#ifndef SIMPLE_H
#define SIMPLE_H

typedef struct
{
   int a;
} Simple_Public_Data;

void simple_a_set(Eo *self, int a);
/* Calls simple_other_call(other, obj) and then simple_other_call(obj, other)
 * for 'times' times in order to grow the call stack on other objects. */
void simple_other_call(Eo*self, Eo *other, int times);

#define SIMPLE_CLASS simple_class_get()
const Efl_Class *simple_class_get(void);

extern const Efl_Event_Description _SIMPLE_FOO;
extern const Efl_Event_Description _SIMPLE_BAR;

#define SIMPLE_FOO (&(_SIMPLE_FOO))
#define SIMPLE_BAR (&(_SIMPLE_BAR))

#endif
