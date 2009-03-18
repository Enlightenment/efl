#include <string.h>

#include "evas_common.h"
#include "evas_engine.h"


DD_Output_Buffer *
evas_software_ddraw_output_buffer_new(int   depth,
                                      int   width,
                                      int   height,
                                      void *data)
{
   DD_Output_Buffer *ddob;

   ddob = calloc(1, sizeof(DD_Output_Buffer));
   if (!ddob) return NULL;

   ddob->data = data;
   ddob->depth = depth;
   ddob->width = width;
   ddob->height = height;
   ddob->pitch = width * depth / 8;
   ddob->psize = ddob->pitch * height;

   if (!ddob->data)
     {
        ddob->data = malloc(ddob->pitch * height);
        if (!ddob->data)
          {
            free(ddob);
            return NULL;
          }
     }

   return ddob;
}

void
evas_software_ddraw_output_buffer_free(DD_Output_Buffer *ddob)
{
   if (ddob->data) free(ddob->data);
   free(ddob);
}

void
evas_software_ddraw_output_buffer_paste(DD_Output_Buffer *ddob,
                                        void             *ddraw_data,
                                        int               ddraw_width,
                                        int               ddraw_height,
                                        int               ddraw_pitch,
                                        int               ddraw_depth,
                                        int               x,
                                        int               y)
{
   DATA8 *dd_data;
   DATA8 *evas_data;
   int    width;
   int    height;
   int    pitch;
   int    j;

   if ((x >= ddraw_width) || (y >= ddraw_height))
     return;

   /* compute the size of the data to copy on the back surface */
   width = ((x + ddob->width) > ddraw_width)
     ? ddraw_width - x
     : ddob->width;
   height = ((y + ddob->height) > ddraw_height)
     ? ddraw_height - y
     : ddob->height;
   pitch = width * ddob->depth / 8;

   dd_data = (DATA8 *)ddraw_data + y * ddraw_pitch + x * ddraw_depth;
   evas_data = (unsigned char *)ddob->data;
   for (j = 0; j < height; j++, evas_data += ddob->pitch, dd_data += ddraw_pitch)
     memcpy(dd_data, evas_data, pitch);
}

DATA8 *
evas_software_ddraw_output_buffer_data(DD_Output_Buffer *ddob,
                                       int              *bytes_per_line_ret)
{
   if (bytes_per_line_ret) *bytes_per_line_ret = ddob->pitch;
   return ddob->data;
}

int
evas_software_ddraw_output_buffer_depth(DD_Output_Buffer *ddob)
{
   return ddob->depth;
}
