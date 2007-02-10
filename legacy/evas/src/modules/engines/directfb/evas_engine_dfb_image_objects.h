#ifndef EVAS_ENGINE_DFB_IMAGE_OBJECTS_H
#define EVAS_ENGINE_DFB_IMAGE_OBJECTS_H
void               *evas_engine_directfb_image_load(void *data, const char *file,
						    const char *key, int *error, Evas_Image_Load_Opts *lo);
void               *evas_engine_directfb_image_new_from_data(void *data, int w,
							     int h,
							     DATA32 *image_data, 
							     int alpha, int cspace);
void               *evas_engine_directfb_image_new_from_copied_data(void *data,
								    int w,
								    int h,
								    DATA32 *image_data, 
								    int alpha, int cspace);
void                evas_engine_directfb_image_free(void *data, void *image);
void                evas_engine_directfb_image_size_get(void *data, void *image,
							int *w, int *h);
void               *evas_engine_directfb_image_size_set(void *data, void *image,
							int w, int h);
void               *evas_engine_directfb_image_dirty_region(void *data,
							    void *image, int x,
							    int y, int w,
							    int h);
void               *evas_engine_directfb_image_data_get(void *data, void *image,
							int to_write,
							DATA32 ** image_data);
void               *evas_engine_directfb_image_data_put(void *data, void *image,
							DATA32 * image_data);
void               *evas_engine_directfb_image_alpha_set(void *data,
							 void *image,
							 int has_alpha);
int                 evas_engine_directfb_image_alpha_get(void *data,
							 void *image);
void               *evas_engine_directfb_image_border_set(void *data,
							  void *image,
							  int l, int r, int t, int b);
void                evas_engine_directfb_image_border_get(void *data,
							  void *image,
							  int *l, int *r, int *t, int *b);
void                evas_engine_directfb_image_draw(void *data, void *context,
						    void *surface, void *image,
						    int src_x, int src_y,
						    int src_w, int src_h,
						    int dst_x, int dst_y,
						    int dst_w, int dst_h,
						    int smooth);
char               *evas_engine_directfb_image_comment_get(void *data,
							   void *image,
							   char *key);
char               *evas_engine_directfb_image_format_get(void *data,
							  void *image);
void                evas_engine_directfb_image_colorspace_set(void *data, 
							      void *image, 
							      int cspace);
int                 evas_engine_directfb_image_colorspace_get(void *data, 
							      void *image);
void                evas_engine_directfb_image_native_set(void *data, 
							  void *image, 
							  void *native);
void               *evas_engine_directfb_image_native_get(void *data, 
							  void *image);
void                evas_engine_directfb_image_cache_flush(void *data);
void                evas_engine_directfb_image_cache_set(void *data, int bytes);
int                 evas_engine_directfb_image_cache_get(void *data);

#endif
