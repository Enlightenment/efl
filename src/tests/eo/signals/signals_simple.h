#ifndef SIMPLE_H
#define SIMPLE_H

typedef struct
{
   int cb_count;
} Simple_Public_Data;

void simple_a_set(Eo *obj, int a);

extern const Efl_Event_Description _EV_A_CHANGED;
#define EV_A_CHANGED (&(_EV_A_CHANGED))

extern const Efl_Event_Description _EV_RESTART;
#define EV_RESTART (&(_EV_RESTART))

#define SIMPLE_CLASS simple_class_get()
const Efl_Class *simple_class_get(void);

#endif
