#include "Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* adding objects */
Evas_Object
evas_add_image_from_file(Evas e, char *file)
{
}

Evas_Object
evas_add_image_from_data(Evas e, void *data, Evas_Image_Format format, int w, int h)
{
}

/* set object settings */
void
evas_set_image_file(Evas e, Evas_Object o, char *file)
{
}

void
evas_set_image_data(Evas e, Evas_Object o, void *data, Evas_Image_Format format, int w, int h)
{
}

void
evas_set_image_scale_smoothness(Evas e, Evas_Object o, int smooth)
{
}

void
evas_set_image_fill(Evas e, Evas_Object o, double x, double y, double w, double h)
{
}

/* image query ops */
void
evas_get_image_size(Evas e, Evas_Object o, int *w, int *h)
{
}
