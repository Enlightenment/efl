#include "evas_cserve2_slave.h"

Eina_Bool
evas_cserve2_image_premul_data(unsigned int *data, unsigned int len)
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

   return ((ALPHA_SPARSE_INV_FRACTION * nas) >= len);
}
