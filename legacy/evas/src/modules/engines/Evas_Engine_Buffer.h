#ifndef _EVAS_ENGINE_BUFFER_H
#define _EVAS_ENGINE_BUFFER_H

#define EVAS_ENGINE_BUFFER_DEPTH_ARGB32 0
#define EVAS_ENGINE_BUFFER_DEPTH_BGRA32 1
#define EVAS_ENGINE_BUFFER_DEPTH_RGB24  2
#define EVAS_ENGINE_BUFFER_DEPTH_BGR24  3
#define EVAS_ENGINE_BUFFER_DEPTH_RGB32  4

typedef struct _Evas_Engine_Info_Buffer Evas_Engine_Info_Buffer;

struct _Evas_Engine_Info_Buffer
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   struct {
      int   depth_type;

      void *dest_buffer;
      int   dest_buffer_row_bytes;

      char  use_color_key : 1;
      int   alpha_threshold;
      int   color_key_r;
      int   color_key_g;
      int   color_key_b;
      struct {
	 void * (*new_update_region) (int x, int y, int w, int h, int *row_bytes);
	 void   (*free_update_region) (int x, int y, int w, int h, void *data);
      } func;
   } info;
};
#endif


