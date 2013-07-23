#ifndef _EVAS_CSERVE2_SLAVE_H
#define _EVAS_CSERVE2_SLAVE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Evas.h>

/* begin bunch of stuff from evas_common_private.h so that we don't need to drag
 * a lot of useless @SOMETHING_CFLAGS@ around */
typedef unsigned long long		DATA64;
typedef unsigned int			DATA32;
typedef unsigned short			DATA16;
typedef unsigned char                   DATA8;

#ifndef WORDS_BIGENDIAN
/* x86 */
#define A_VAL(p) (((DATA8 *)(p))[3])
#define R_VAL(p) (((DATA8 *)(p))[2])
#define G_VAL(p) (((DATA8 *)(p))[1])
#define B_VAL(p) (((DATA8 *)(p))[0])
#define AR_VAL(p) ((DATA16 *)(p)[1])
#define GB_VAL(p) ((DATA16 *)(p)[0])
#else
/* ppc */
#define A_VAL(p) (((DATA8 *)(p))[0])
#define R_VAL(p) (((DATA8 *)(p))[1])
#define G_VAL(p) (((DATA8 *)(p))[2])
#define B_VAL(p) (((DATA8 *)(p))[3])
#define AR_VAL(p) ((DATA16 *)(p)[0])
#define GB_VAL(p) ((DATA16 *)(p)[1])
#endif

/* if more than 1/ALPHA_SPARSE_INV_FRACTION is "alpha" (1-254) then sparse
 * alpha flag gets set */
#define ALPHA_SPARSE_INV_FRACTION 3

#define IMG_MAX_SIZE 65000

#define IMG_TOO_BIG(w, h) \
   ((((unsigned long long)w) * ((unsigned long long)h)) >= \
       ((1ULL << (29 * (sizeof(void *) / 4))) - 2048))

#define RGB_JOIN(r,g,b) \
        (((r) << 16) + ((g) << 8) + (b))

#define ARGB_JOIN(a,r,g,b) \
        (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))
/* end bunchf of stuff from evas_common_private.h */

typedef struct _Evas_Loader_Module_Api Evas_Loader_Module_Api;
typedef struct _Evas_Img_Load_Params Evas_Img_Load_Params;

#define EVAS_CSERVE2_MODULE_API_VERSION 1
struct _Evas_Loader_Module_Api {
   int version;
   const char *type;
   Eina_Bool (*head_load)(Evas_Img_Load_Params *p, const char *file, const char *key, int *error);
   Eina_Bool (*data_load)(Evas_Img_Load_Params *p, const char *file, const char *key, int *error);
};

struct _Evas_Img_Load_Params {
   unsigned int w, h;
   unsigned int degree;
   unsigned int scale;
   int frame_count;
   int loop_count;
   int loop_hint;
   struct {
      unsigned int w, h;
      unsigned int rx, ry, rw, rh;
      int scale_down_by;
      double dpi;
      Eina_Bool orientation;
   } opts;
   void *buffer;
   Eina_Bool has_opts : 1;
   Eina_Bool rotated : 1;
   Eina_Bool alpha : 1;
   Eina_Bool alpha_sparse : 1;
   Eina_Bool animated : 1;
};

Eina_Bool evas_cserve2_image_premul_data(unsigned int *data, unsigned int len);

#endif /* _EVAS_CSERVE2_SLAVE_H */
