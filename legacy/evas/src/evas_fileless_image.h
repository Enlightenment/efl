#include "Evas.h"

typedef struct _Evas_Fileless_Image Evas_Fileless_Image;

struct _Evas_Fileless_Image{
    char *file;
    Imlib_Image im;
}; 

Imlib_Image _evas_find_fileless_image(char*file);
char* _evas_generate_filename(void);
void _evas_add_fileless_image(Imlib_Image im, char*file);
void _evas_remove_fileless_image(char*file);






