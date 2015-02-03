#include "evas_engine.h"
#include <Ecore_X.h>

typedef struct _Native Native;

struct _Native
{
   Evas_Native_Surface ns;
   Pixmap              pixmap;
   Visual             *visual;

   Ecore_X_Image      *exim;
};

void *evas_xlib_image_native_set(void *data, void *image, void *native);
