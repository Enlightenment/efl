#ifndef MIXIN2_H
#define MIXIN2_H

typedef struct
{
   int count;
} Mixin2_Public_Data;

#define MIXIN2_CLASS mixin2_class_get()
const Efl_Class *mixin2_class_get(void);

#endif
