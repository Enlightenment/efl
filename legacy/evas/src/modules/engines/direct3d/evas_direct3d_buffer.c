#include <string.h>

#include "evas_engine.h"


Direct3D_Output_Buffer *
evas_direct3d_output_buffer_new(int   depth,
                                int   width,
                                int   height,
                                void *data)
{
   Direct3D_Output_Buffer *d3dob;

   d3dob = calloc(1, sizeof(Direct3D_Output_Buffer));
   if (!d3dob) return NULL;

   d3dob->image = data;
   d3dob->depth = depth;
   d3dob->width = width;
   d3dob->height = height;
   d3dob->pitch = width * (depth >> 3);

   if (!d3dob->image)
     {
        d3dob->image = malloc(d3dob->pitch * height);
        if (!d3dob->image)
          {
            free(d3dob);
            return NULL;
          }
     }

   return d3dob;
}

void
evas_direct3d_output_buffer_free(Direct3D_Output_Buffer *d3dob)
{
  if (d3dob->image) free(d3dob->image);
  free(d3dob);
}

void
evas_direct3d_output_buffer_paste(Direct3D_Output_Buffer *d3dob,
                                  DATA8                  *d3d_data,
                                  int                     d3d_width,
                                  int                     d3d_height,
                                  int                     d3d_pitch,
                                  int                     x,
                                  int                     y)
{
   DATA8         *evas_data;
   int            width;
   int            height;
   int            pitch;
   int            j;

   if ((x >= d3d_width) || (y >= d3d_height))
     return;

   /* compute the size of the data to copy on the back surface */
   width = ((x + d3dob->width) > d3d_width)
     ? d3d_width - x
     : d3dob->width;
   height = ((y + d3dob->height) > d3d_height)
     ? d3d_height - y
     : d3dob->height;
   pitch = width * (d3dob->depth >> 3);

   d3d_data += y * d3d_pitch + x * (d3dob->depth >> 3);
   evas_data = (unsigned char *)d3dob->image;
   for (j = 0; j < height; j++, evas_data += d3dob->pitch, d3d_data += d3d_pitch)
     memcpy(d3d_data, evas_data, pitch);
}

DATA8 *
evas_direct3d_output_buffer_data(Direct3D_Output_Buffer *d3dob,
                                 int                    *bytes_per_line_ret)
{
   if (bytes_per_line_ret) *bytes_per_line_ret = d3dob->pitch;
   return d3dob->image;
}

int
evas_direct3d_output_buffer_depth(Direct3D_Output_Buffer *d3dob)
{
   return d3dob->depth;
}
