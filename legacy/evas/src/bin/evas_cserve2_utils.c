#include "evas_cserve2_slave.h"

static unsigned int
evas_cserve2_convert_argb_premul(unsigned int *data, unsigned int len)
{
   unsigned int *de = data + len;
   unsigned int nas = 0;

   while (data < de)
     {
	unsigned int  a = 1 + (*data >> 24);

	*data = (*data & 0xff000000) +
	  (((((*data) >> 8) & 0xff) * a) & 0xff00) +
	  (((((*data) & 0x00ff00ff) * a) >> 8) & 0x00ff00ff);
	data++;

	if ((a == 1) || (a == 256))
	  nas++;
     }

   return nas;
}

EAPI void
evas_cserve2_image_premul(Evas_Img_Load_Params *ilp)
{
   unsigned int nas;

   if (!ilp->alpha) return;

   nas = evas_cserve2_convert_argb_premul(ilp->buffer, ilp->w * ilp->h);
   if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (ilp->w * ilp->h))
     ilp->alpha_sparse = EINA_TRUE;
}

EAPI void
evas_cserve2_imave_alpha_sparse_set(Evas_Img_Load_Params *ilp)
{
   unsigned int *s, *se;
   unsigned int nas = 0;
   unsigned int len = ilp->w * ilp->h;

   if (!ilp->alpha) return;

   s = ilp->buffer;
   se = s + len;
   while (s < se)
     {
        unsigned int p = *s & 0xff000000;

        if (!p || (p == 0xff000000))
          nas++;
        s++;
     }
   if ((ALPHA_SPARSE_INV_FRACTION * nas) >= len)
     ilp->alpha_sparse = EINA_TRUE;
}
