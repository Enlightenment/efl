#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* callbacks */
void
evas_callback_add(Evas e, Evas_Object o, Evas_Callback_Type callback, void (*func) (void *_data, Evas _e, char *_class, Evas_Object _o, int _b, int _x, int _y), void *data)

{
}

void
evas_callback_del(Evas e, Evas_Object o, Evas_Callback_Type callback)
{
}
