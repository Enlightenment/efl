#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#include "evas_fileless_image.h"

//#define MYDEBUG 1


Evas_List fileless_images = NULL;

char* _evas_generate_filename(void){
    
    char buf[100];
    char *p;
    int i,j;

    struct timeval tv;
   
    gettimeofday(&tv,NULL);
    srand(tv.tv_usec);
    i = rand();    
    srand(tv.tv_sec);
    j = rand();
    sprintf(buf, "s%d%d.mem", i, j);
    p = (char*)malloc(strlen(buf)+1);
    strcpy(p, buf);
#ifdef MYDEBUG
fprintf(stderr,"fli: generated: %s\n", p);
#endif
    return p;    
}

void _evas_add_fileless_image(Imlib_Image im, char* file){
    
    Evas_Fileless_Image *fli;
    Imlib_Image prev_im;

    if(!file) return;

    fli = (Evas_Fileless_Image*)malloc(sizeof(Evas_Fileless_Image));
    fli->file = (char*)malloc(strlen(file)+1);
    strcpy(fli->file, file);
    prev_im = imlib_context_get_image();
    imlib_context_set_image(im);
    fli->im = imlib_clone_image();
    imlib_context_set_image(prev_im);
    
    fileless_images = evas_list_append(fileless_images,(void*)fli);
#ifdef MYDEBUG
fprintf(stderr,"fli: added: %s\n", file);
#endif
    
}

void _evas_remove_fileless_image(char*file){
    Evas_List l;
    Imlib_Image prev_im;

    if(!file) return;
	
    for( l = fileless_images; l ; l = l -> next ){
	Evas_Fileless_Image *fli;
	fli = l->data;
	if(!strcmp(fli->file, file)){
	    free(fli->file);
	    prev_im = imlib_context_get_image();
	    imlib_context_set_image(fli->im);
	    imlib_free_image();
	    imlib_context_set_image(prev_im);
	    fileless_images = evas_list_remove(fileless_images, fli);
#ifdef MYDEBUG 
fprintf(stderr,"fli: removed: %s\n", file);
#endif
	    free(fli);
	    return;
	}
    }
}


Imlib_Image _evas_find_fileless_image(char*file){
    Evas_List l;
    Imlib_Image prev_im, res_im = NULL;

    if(!file) return;
    
    for( l = fileless_images ; l ; l = l -> next ){
	Evas_Fileless_Image *fli;
	fli = l -> data;
	if(!strcmp(fli->file, file)){
	    prev_im = imlib_context_get_image();
	    imlib_context_set_image(fli->im);
	    res_im = imlib_clone_image();
#ifdef MYDEBUG
fprintf(stderr, "fli: found: %s\n", file);    
#endif 
	    imlib_context_set_image(prev_im);
	}
    }

    return res_im;
}














