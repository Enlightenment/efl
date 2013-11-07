#ifndef SIMPLE_H
#define SIMPLE_H

EAPI void simple_a_set(int a);

typedef struct
{
   int public_x2;
} Simple_Public_Data;

extern const Eo_Event_Description _EV_A_CHANGED;
#define EV_A_CHANGED (&(_EV_A_CHANGED))

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
