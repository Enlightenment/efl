#ifndef _EVAS_BUFFER_HELPER_H_
#define _EVAS_BUFFER_HELPER_H_

#include <Evas.h>

/*
 * Create canvas with buffer and create its internal buffer
*/
Evas* canvas_create(int width, int height);

/*
 * return internal buffer
*/
void* canvas_buffer(Evas *evas);

/*
 * Destroy canvas and internal buffer
*/
void canvas_destroy(Evas *canvas);

/*
 * Save canvas into image with PPM P6 format (*.ppm)
*/
void canvas_save(Evas *canvas, const char *dest);

/*
 * Draw canvas (this should be called after making changes into evas)
*/
void canvas_draw(Evas *canvas);


#endif // !_EVAS_BUFFER_HELPER_H_
