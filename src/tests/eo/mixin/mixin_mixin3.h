#ifndef MIXIN3_H
#define MIXIN3_H

typedef struct
{
   int count;
} Mixin3_Public_Data;

#define MIXIN3_CLASS mixin3_class_get()
const Eo_Class *mixin3_class_get(void);

#endif
