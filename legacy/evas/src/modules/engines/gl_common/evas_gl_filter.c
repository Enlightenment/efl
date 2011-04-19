#include "evas_gl_private.h"


void
evas_gl_common_filter_draw(void *data, Evas_Engine_GL_Context *gc,
		void *image, Evas_Filter_Info *filter)
{
   Evas_GL_Image *im = image;
   RGBA_Draw_Context *dc;
   GLuint prog;
   int r,g,b,a;
   int nomul, bgra;

   dc = gc->dc;

   if (dc->mul.use)
     {
	a = (dc->mul.col >> 24) & 0xff;
	r = (dc->mul.col >> 16) & 0xff;
	g = (dc->mul.col >> 8 ) & 0xff;
	b = (dc->mul.col      ) & 0xff;
     }
   else
     {
	r = g = b = a = 255;
     }

   nomul = (a == 255 && r == 255 && g == 255 && b == 255) ? 1 : 0;
   bgra = (gc->shared->info.bgra) ? 1 : 0;

   /* FIXME: This should so be a table */
#if 0
   if (filter->filter == EVAS_FILTER_BLUR)
     {
         if (bgra)
          {
             if (nomul)
                prog = gc->shared->shader.filter_blur_bgra_nomul.prog;
             else
                prog = gc->shared->shader.filter_blur_bgra.prog;
          }
        else
          {
             if (a == 255 && r == 255 && g == 255 && b == 255)
                prog = gc->shared->shader.filter_blur_nomul.prog;
             else
                prog = gc->shared->shader.filter_blur.prog;
          }
     }
   else
#endif
     if (filter->filter == EVAS_FILTER_INVERT)
     {
        if (bgra)
          {
             if (nomul)
                prog = gc->shared->shader.filter_invert_bgra_nomul.prog;
             else
                prog = gc->shared->shader.filter_invert_bgra.prog;
          }
        else
          {
             if (a == 255 && r == 255 && g == 255 && b == 255)
                prog = gc->shared->shader.filter_invert_nomul.prog;
             else
                prog = gc->shared->shader.filter_invert.prog;
          }
     }
   else if (filter->filter == EVAS_FILTER_SEPIA)
     {
        if (bgra)
          {
             if (nomul)
                prog = gc->shared->shader.filter_sepia_bgra_nomul.prog;
             else
                prog = gc->shared->shader.filter_sepia_bgra.prog;
          }
        else
          {
             if (nomul)
                prog = gc->shared->shader.filter_sepia_nomul.prog;
             else
                prog = gc->shared->shader.filter_sepia.prog;
          }

     }
   else /*if (filter->filter == EVAS_FILTER_GREYSCALE)*/
     {
        printf("BGRA: %s  Nomul: %s\n",bgra?"true":"false",nomul?"nomul":"mul");
        if (bgra)
          {
             if (nomul)
                prog = gc->shared->shader.filter_greyscale_bgra_nomul.prog;
             else
                prog = gc->shared->shader.filter_greyscale_bgra.prog;
          }
        else
          {
             if (nomul)
                prog = gc->shared->shader.filter_greyscale_nomul.prog;
             else
                prog = gc->shared->shader.filter_greyscale.prog;
          }

     }

   printf("Prog: %d %d %d\n",prog,im->w,im->h);
   gc->filter_prog = prog;
   evas_gl_common_image_update(gc, im);
   evas_gl_common_context_image_push(gc, im->tex, 0, 0, im->w, im->h,
                                      0, 0, im->w, im->h,
                                      r,g,b,a,
                                      1, im->tex_only);
   gc->filter_prog = 0;
}


Filtered_Image *
evas_gl_common_image_filtered_get(Evas_GL_Image *im, uint8_t *key, size_t len)
{
   Filtered_Image *fi;
   Eina_List *l;

   for (l = im->filtered ; l ; l = l->next)
     {
         fi = l->data;
         if (fi->keylen != len) continue;
         if (memcmp(key, fi->key, len) != 0) continue;
         fi->ref ++;
         return fi;
     }

   return NULL;
}

Filtered_Image *
evas_gl_common_image_filtered_save(Evas_GL_Image *im, Evas_GL_Image *fimage,
                                   uint8_t *key, size_t keylen)
{
   Filtered_Image *fi;
   Eina_List *l;

   for (l = im->filtered ; l ; l = l->next)
     {
        fi = l->data;
        if (fi->keylen != keylen) continue;
        if (memcmp(key, fi->key, keylen) != 0) continue;

        fi->image = (void *)fimage;
        fi->ref ++;
        return fi;
     }

   fi = calloc(1,sizeof(Filtered_Image));
   if (!fi) return NULL;

   fi->keylen = keylen;
   fi->key = malloc(keylen);
   memcpy(fi->key, key, keylen);
   fi->image = (void *)fimage;
   fi->ref = 1;

   im->filtered = eina_list_prepend(im->filtered, fi);

   return fi;
}

void
evas_gl_common_image_filtered_free(Evas_GL_Image *image, Filtered_Image *fi)
{
   fi->ref --;
   if (fi->ref) return;

   free(fi->key);
   evas_gl_common_image_free((void *)fi->image);
   fi->image = NULL;

   image->filtered = eina_list_remove(image->filtered, fi);
}



/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
