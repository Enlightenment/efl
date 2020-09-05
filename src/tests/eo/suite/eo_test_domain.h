#ifndef DOMAIN_H
#define DOMAIN_H

typedef struct
{
   int a;
} Domain_Public_Data;

void domain_a_set(Eo *obj, int a);
int  domain_a_get(Eo *obj);
void domain_recursive(Eo *obj, int n);

extern const Efl_Event_Description _EV_DOMAIN_A_CHANGED;
#define EV_DOMAIN_A_CHANGED (&(_EV_DOMAIN_A_CHANGED))

#define DOMAIN_CLASS domain_class_get()
const Efl_Class *domain_class_get(void);

#endif
