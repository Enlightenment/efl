#ifndef SIMPLE_H
#define SIMPLE_H

void simple_a_set(Eo *obj, int a);
int simple_a_get(Eo *obj);
void simple_b_set(Eo *obj, int b);
int simple_b_get(Eo *obj);

#define SIMPLE_CLASS simple_class_get()
const Efl_Class *simple_class_get(void);

#endif
