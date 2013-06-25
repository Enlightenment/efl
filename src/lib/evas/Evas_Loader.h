#ifndef _EVAS_LOADER_H
#define _EVAS_LOADER_H

typedef struct _Evas_Image_Load_Opts Evas_Image_Load_Opts;
typedef struct _Evas_Image_Animated  Evas_Image_Animated;
typedef struct _Evas_Image_Property  Evas_Image_Property;
typedef struct _Evas_Image_Load_Func Evas_Image_Load_Func;

typedef enum _Evas_Image_Animated_Loop_Hint
{
   EVAS_IMAGE_ANIMATED_HINT_NONE = 0,
   EVAS_IMAGE_ANIMATED_HINT_LOOP = 1, /**< Image's animation mode is loop like 1->2->3->1->2->3 */
   EVAS_IMAGE_ANIMATED_HINT_PINGPONG = 2 /**< Image's animation mode is pingpong like 1->2->3->2->1-> ... */
} Evas_Image_Animated_Loop_Hint;

typedef enum _Evas_Image_Scale_Hint
{
   EVAS_IMAGE_SCALE_HINT_NONE = 0, /**< No scale hint at all */
   EVAS_IMAGE_SCALE_HINT_DYNAMIC = 1, /**< Image is being re-scaled over time, thus turning scaling cache @b off for its data */
   EVAS_IMAGE_SCALE_HINT_STATIC = 2 /**< Image is not being re-scaled over time, thus turning scaling cache @b on for its data */
} Evas_Image_Scale_Hint; /**< How an image's data is to be treated by Evas, with regard to scaling cache */

struct _Evas_Image_Property
{
   unsigned int  w;
   unsigned int  h;
   
   unsigned char scale;

   Eina_Bool     rotated;
   Eina_Bool     alpha;
   Eina_Bool     premul;
   Eina_Bool     alpha_sparse;
};

struct _Evas_Image_Animated
{
   Eina_List *frames;

   Evas_Image_Animated_Loop_Hint loop_hint;

   int        frame_count;
   int        loop_count;
   int        cur_frame;

   Eina_Bool  animated;
};

struct _Evas_Image_Load_Opts
{
   struct {
      unsigned int      x, y, w, h;
   } region;
   struct {
      int src_x, src_y, src_w, src_h;
      int dst_w, dst_h;
      int smooth;
      Evas_Image_Scale_Hint scale_hint;
   } scale_load;
   double               dpi; // if > 0.0 use this
   unsigned int         w, h; // if > 0 use this
   unsigned int         degree;//if>0 there is some info related with rotation
   int                  scale_down_by; // if > 1 then use this

   Eina_Bool            orientation; // if EINA_TRUE => should honor orientation information provided by file (like jpeg exif info)
};

struct _Evas_Image_Load_Func
{
  void     *(*file_open) (Eina_File *f, const char *key,
			  Evas_Image_Load_Opts *opts,
			  Evas_Image_Animated *animated,
			  int *error);
  void     (*file_close) (void *loader_data);

  Eina_Bool (*file_head) (void *loader_data,
			  Evas_Image_Property *prop,
			  int *error);
  Eina_Bool (*file_data) (void *loader_data,
			  Evas_Image_Property *prop,
			  void *pixels, int *error);
  double    (*frame_duration) (void *loader_data,
			       int start, int frame_num);

  Eina_Bool threadable;
  Eina_Bool do_region;
};

#endif
