/*
 * Copyright 2006-2009 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* Original Author: Adam Jackson <ajax@nwnk.net> */
/* Heavily modified by: Leif Middelschulte <leif.middelschulte@gmail.com> */

#include "Ecore_X.h"

/* TODO:
 * - see other TODO's within this file.
 */

#define ECORE_X_RANDR_EDID_VERSION_10                                   ((1 << 8) | 0)
#define ECORE_X_RANDR_EDID_VERSION_11                                   ((1 << 8) | 1)
#define ECORE_X_RANDR_EDID_VERSION_12                                   ((1 << 8) | 2)
#define ECORE_X_RANDR_EDID_VERSION_13                                   ((1 << 8) | 3)
#define ECORE_X_RANDR_EDID_VERSION_14                                   ((1 << 8) | 4)

#define _ECORE_X_RANDR_EDID_OFFSET_MANUFACTURER                         0x08
#define _ECORE_X_RANDR_EDID_OFFSET_TYPE                                 0x14
#define _ECORE_X_RANDR_EDID_OFFSET_VERSION_MAJOR                        0x12
#define _ECORE_X_RANDR_EDID_OFFSET_VERSION_MINOR                        0x13
#define _ECORE_X_RANDR_EDID_OFFSET_DPMS                                 0x18
#define _ECORE_X_RANDR_EDID_OFFSET_COLORSPACE                           0x18
#define _ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK                     0x36
#define _ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_TYPE                3
#define _ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_CONTENT             5
#define _ECORE_X_RANDR_EDID_OFFSET_ASPECT_RATIO_PREFERRED               15
#define _ECORE_X_RANDR_EDID_OFFSET_ASPECT_RATIO                         14

#define _ECORE_X_RANDR_EDID_MASK_DIGITAL                                0x80
#define _ECORE_X_RANDR_EDID_MASK_DIGITAL_INTERFACE                      0x0f
#define _ECORE_X_RANDR_EDID_MASK_DIGITAL_TMDS_DFP_10                    0x01
#define _ECORE_X_RANDR_EDID_MASK_COLORSCHEME_ANALOGOUS                  0x18
#define _ECORE_X_RANDR_EDID_MASK_COLORSCHEME_DIGITAL_YCRCB_444          0x10
#define _ECORE_X_RANDR_EDID_MASK_COLORSCHEME_DIGITAL_YCRCB_422          0x08
#define _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_PREFERRED                 0xe0
#define _ECORE_X_RANDR_EDID_MASK_DPMS                                   0xE0
#define _ECORE_X_RANDR_EDID_MASK_DPMS_STANDBY                           0x80
#define _ECORE_X_RANDR_EDID_MASK_DPMS_SUSPEND                           0x40
#define _ECORE_X_RANDR_EDID_MASK_DPMS_OFF                               0x20
#define _ECORE_X_RANDR_EDID_MASK_INTERFACE_TYPE                         0x0f
#define _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_4_3                       0x80
#define _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_16_9                      0x40
#define _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_16_10                     0x20
#define _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_5_4                       0x10
#define _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_15_9                      0x08

#define _ECORE_X_RANDR_EDID_DISPLAY_DESCRIPTOR_BLOCK_CONTENT_LENGTH_MAX 13

typedef enum _Ecore_X_Randr_Edid_Aspect_Ratio_Preferred {
   ECORE_X_RANDR_EDID_ASPECT_RATIO_PREFERRED_4_3 = 0x00,
   ECORE_X_RANDR_EDID_ASPECT_RATIO_PREFERRED_16_9 = 0x01,
   ECORE_X_RANDR_EDID_ASPECT_RATIO_PREFERRED_16_10 = 0x02,
   ECORE_X_RANDR_EDID_ASPECT_RATIO_PREFERRED_5_4 = 0x03,
   ECORE_X_RANDR_EDID_ASPECT_RATIO_PREFERRED_15_9 = 0x04
} Ecore_X_Randr_Edid_Aspect_Ratio_Preferred;

/* Some convenience loops */
#define _ECORE_X_RANDR_EDID_FOR_EACH_EXTENSION_BLOCK(edid, edid_length, extension_block_iter) \
  for (extension_block_iter = edid; extension_block_iter < (edid + edid_length); extension_block_iter += 128)

#define _ECORE_X_RANDR_EDID_FOR_EACH_CEA_BLOCK(edid, edid_length, cea_block_iter) \
  _ECORE_X_RANDR_EDID_FOR_EACH_EXTENSION_BLOCK(edid, edid_length, cea_block_iter) \
  if (cea_block_iter[0] == 0x02)

/* The following macro is to be used with caution as it inherits another loop.
 * Therefore using a 'break;' statement will lead to continuation in the
 * inherent 'Extension block'-loop.
 */
#define _ECORE_X_RANDR_EDID_FOR_EACH_CEA_DETAILED_BLOCK(edid, edid_length, cea_block_iter, detailed_block_iter)                              \
  _ECORE_X_RANDR_EDID_FOR_EACH_CEA_BLOCK(edid, edid_length, cea_block_iter)                                                                  \
  for (detailed_block_iter = cea_block_iter + cea_block_iter[2]; detailed_block_iter + 18 < cea_block_iter + 127; detailed_block_iter += 18) \
    if (detailed_block_iter[0])

#define _ECORE_X_RANDR_EDID_FOR_EACH_DESCRIPTOR_BLOCK(edid, block) \
  for (block = edid + _ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK; block <= (edid + _ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK + (3 * 18)); block += 18)

#define _ECORE_X_RANDR_EDID_FOR_EACH_NON_PIXEL_DESCRIPTOR_BLOCK(edid, block) \
  _ECORE_X_RANDR_EDID_FOR_EACH_DESCRIPTOR_BLOCK(edid, block)                 \
  if ((block[0] == 0) && (block[1] == 0))

EAPI Eina_Bool
ecore_x_randr_edid_has_valid_header(unsigned char *edid,
                                    unsigned long  edid_length)
{
   const unsigned char header[] =
   { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };
   if (!edid) return EINA_FALSE;
   if (edid_length < 8) return EINA_FALSE;
   if (!memcmp(edid, header, 8)) return EINA_TRUE;
   return EINA_FALSE;
}

EAPI int
ecore_x_randr_edid_version_get(unsigned char *edid,
                               unsigned long  edid_length)
{
   if ((edid_length > _ECORE_X_RANDR_EDID_OFFSET_VERSION_MINOR) &&
       (ecore_x_randr_edid_has_valid_header(edid, edid_length)))
     return (edid[_ECORE_X_RANDR_EDID_OFFSET_VERSION_MAJOR] << 8) |
            edid[_ECORE_X_RANDR_EDID_OFFSET_VERSION_MINOR];
   return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
}

EAPI int
ecore_x_randr_edid_manufacturer_model_get(unsigned char *edid,
                                          unsigned long  edid_length)
{
   if ((edid_length > 0x0b) &&
       (ecore_x_randr_edid_has_valid_header(edid, edid_length)))
     return (int)(edid[0x0a] + (edid[0x0b] << 8));
   return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
}

EAPI int
ecore_x_randr_edid_manufacturer_serial_number_get(unsigned char *edid,
                                                  unsigned long  edid_length)
{
   if ((edid_length > 0x0f) &&
       (ecore_x_randr_edid_has_valid_header(edid, edid_length)))
     return (int)(edid[0x0c] + (edid[0x0d] << 8) +
                  (edid[0x0e] << 16) + (edid[0x0f] << 24));
   return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
}

EAPI char *
ecore_x_randr_edid_manufacturer_name_get(unsigned char *edid,
                                         unsigned long  edid_length)
{
   if ((edid_length > (_ECORE_X_RANDR_EDID_OFFSET_MANUFACTURER + 1)) &&
       (ecore_x_randr_edid_has_valid_header(edid, edid_length)))
     {
        unsigned char *x;
        char *name;

        x = (edid + _ECORE_X_RANDR_EDID_OFFSET_MANUFACTURER);
        name = malloc(sizeof(char) * 4);
        if (!name) return NULL;
        name[0] = ((x[0] & 0x7c) >> 2) + '@';
        name[1] = ((x[0] & 0x03) << 3) + ((x[1] & 0xe0) >> 5) + '@';
        name[2] = (x[1] & 0x1f) + '@';
        name[_ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_TYPE] = 0;
        return name;
     }
   return NULL;
}

EAPI char *
ecore_x_randr_edid_display_name_get(unsigned char *edid,
                                    unsigned long  edid_length)
{
   unsigned char *block = NULL;
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return NULL;
   _ECORE_X_RANDR_EDID_FOR_EACH_NON_PIXEL_DESCRIPTOR_BLOCK(edid, block)
   {
      if (block[_ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_TYPE] == 0xfc)
        {
           char *name, *p;
           const char *edid_name;

           edid_name = (const char *)block + _ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_CONTENT;
           name = malloc(sizeof(char) * _ECORE_X_RANDR_EDID_DISPLAY_DESCRIPTOR_BLOCK_CONTENT_LENGTH_MAX);
           if (!name) return NULL;
           strncpy(name, edid_name, (_ECORE_X_RANDR_EDID_DISPLAY_DESCRIPTOR_BLOCK_CONTENT_LENGTH_MAX - 1));
           name[_ECORE_X_RANDR_EDID_DISPLAY_DESCRIPTOR_BLOCK_CONTENT_LENGTH_MAX] = 0;
           for (p = name; *p; p++)
             {
                if ((*p < ' ') || (*p > '~')) *p = 0;
             }
           return name;
        }
   }
   return NULL;
}

EAPI Ecore_X_Randr_Edid_Aspect_Ratio
ecore_x_randr_edid_display_aspect_ratio_preferred_get(unsigned char *edid,
                                                      unsigned long  edid_length)
{
   unsigned char *block = NULL;
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
   _ECORE_X_RANDR_EDID_FOR_EACH_NON_PIXEL_DESCRIPTOR_BLOCK(edid, block)
   {
      if ((block[_ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_TYPE] == 0xfd) &&
          (block[10] == 0x04))
        {
           Ecore_X_Randr_Edid_Aspect_Ratio_Preferred preferred_ratio =
             (Ecore_X_Randr_Edid_Aspect_Ratio_Preferred)
             ((block[_ECORE_X_RANDR_EDID_OFFSET_ASPECT_RATIO_PREFERRED] &
               _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_PREFERRED) >> 5);
           switch (preferred_ratio)
             {
              case ECORE_X_RANDR_EDID_ASPECT_RATIO_PREFERRED_4_3:
                return ECORE_X_RANDR_EDID_ASPECT_RATIO_4_3;

              case ECORE_X_RANDR_EDID_ASPECT_RATIO_PREFERRED_16_9:
                return ECORE_X_RANDR_EDID_ASPECT_RATIO_16_9;

              case ECORE_X_RANDR_EDID_ASPECT_RATIO_PREFERRED_16_10:
                return ECORE_X_RANDR_EDID_ASPECT_RATIO_16_10;

              case ECORE_X_RANDR_EDID_ASPECT_RATIO_PREFERRED_5_4:
                return ECORE_X_RANDR_EDID_ASPECT_RATIO_5_4;

              case ECORE_X_RANDR_EDID_ASPECT_RATIO_PREFERRED_15_9:
                return ECORE_X_RANDR_EDID_ASPECT_RATIO_15_9;

              default:
                return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
             }
        }
   }
   return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
}

EAPI Ecore_X_Randr_Edid_Aspect_Ratio
ecore_x_randr_edid_display_aspect_ratios_get(unsigned char *edid,
                                             unsigned long  edid_length)
{
   Ecore_X_Randr_Edid_Aspect_Ratio ret = ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
   unsigned char *block = NULL;
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
   _ECORE_X_RANDR_EDID_FOR_EACH_NON_PIXEL_DESCRIPTOR_BLOCK(edid, block)
   {
      if ((block[_ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_TYPE] == 0xfd) &&
          (block[10] == 0x04))
        {
           if (block[_ECORE_X_RANDR_EDID_OFFSET_ASPECT_RATIO] & _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_4_3)
             ret |= ECORE_X_RANDR_EDID_ASPECT_RATIO_4_3;
           if (block[_ECORE_X_RANDR_EDID_OFFSET_ASPECT_RATIO] & _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_16_9)
             ret |= ECORE_X_RANDR_EDID_ASPECT_RATIO_16_9;
           if (block[_ECORE_X_RANDR_EDID_OFFSET_ASPECT_RATIO] & _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_16_10)
             ret |= ECORE_X_RANDR_EDID_ASPECT_RATIO_16_10;
           if (block[_ECORE_X_RANDR_EDID_OFFSET_ASPECT_RATIO] & _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_5_4)
             ret |= ECORE_X_RANDR_EDID_ASPECT_RATIO_5_4;
           if (block[_ECORE_X_RANDR_EDID_OFFSET_ASPECT_RATIO] & _ECORE_X_RANDR_EDID_MASK_ASPECT_RATIO_15_9)
             ret |= ECORE_X_RANDR_EDID_ASPECT_RATIO_15_9;
        }
   }
   return ret;
}

EAPI char *
ecore_x_randr_edid_display_ascii_get(unsigned char *edid,
                                     unsigned long  edid_length)
{
   unsigned char *block = NULL;
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return NULL;
   _ECORE_X_RANDR_EDID_FOR_EACH_NON_PIXEL_DESCRIPTOR_BLOCK(edid, block)
   {
      if (block[_ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_TYPE] == 0xfe)
        {
           char *ascii, *p;
           const char *edid_ascii = (const char *)block +
             _ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_CONTENT;
           /*
            * TODO: Two of these in a row, in the third and fourth slots,
            * seems to be specified by SPWG: http://www.spwg.org/
            */
           ascii = malloc(sizeof(char) * _ECORE_X_RANDR_EDID_DISPLAY_DESCRIPTOR_BLOCK_CONTENT_LENGTH_MAX);
           if (!ascii) return NULL;
           strncpy(ascii, edid_ascii, (_ECORE_X_RANDR_EDID_DISPLAY_DESCRIPTOR_BLOCK_CONTENT_LENGTH_MAX - 1));
           ascii[_ECORE_X_RANDR_EDID_DISPLAY_DESCRIPTOR_BLOCK_CONTENT_LENGTH_MAX] = 0;
           for (p = ascii; *p; p++)
             {
                if ((*p < ' ') || (*p > '~')) *p = 0;
             }
           return ascii;
        }
   }
   return NULL;
}

EAPI char *
ecore_x_randr_edid_display_serial_get(unsigned char *edid,
                                      unsigned long  edid_length)
{
   unsigned char *block = NULL;
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return NULL;
   _ECORE_X_RANDR_EDID_FOR_EACH_NON_PIXEL_DESCRIPTOR_BLOCK(edid, block)
   {
      if (block[_ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_TYPE] == 0xff)
        {
           char *serial, *p;
           const char *edid_serial = (const char *)block +
             _ECORE_X_RANDR_EDID_OFFSET_DESCRIPTOR_BLOCK_CONTENT;
           /*
            * TODO: Two of these in a row, in the third and fourth slots,
            * seems to be specified by SPWG: http://www.spwg.org/
            */
           serial = malloc(sizeof(char) * _ECORE_X_RANDR_EDID_DISPLAY_DESCRIPTOR_BLOCK_CONTENT_LENGTH_MAX);
           if (!serial) return NULL;
           strncpy(serial, edid_serial, (_ECORE_X_RANDR_EDID_DISPLAY_DESCRIPTOR_BLOCK_CONTENT_LENGTH_MAX - 1));
           serial[_ECORE_X_RANDR_EDID_DISPLAY_DESCRIPTOR_BLOCK_CONTENT_LENGTH_MAX] = 0;
           for (p = serial; *p; p++)
             {
                if ((*p < ' ') || (*p > '~')) *p = 0;
             }
           return serial;
        }
   }
   return NULL;
}

EAPI Eina_Bool
ecore_x_randr_edid_info_has_valid_checksum(unsigned char *edid,
                                           unsigned long  edid_length)
{
   unsigned char *cea_block_iter = NULL;
   char sum = 0;
   int i;
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return EINA_FALSE;
   if (edid_length < 128) return EINA_FALSE;

   /* Check the EDID block itself */
   for (i = 0; i < 128; i++) sum += edid[i];
   if (sum) return EINA_FALSE;

   /* Check the cea extension blocks */
   _ECORE_X_RANDR_EDID_FOR_EACH_CEA_BLOCK(edid, edid_length, cea_block_iter)
   {
      for (i = 0, sum = 0; i < 128; i++) sum += cea_block_iter[i];
   }
   if (sum) return EINA_FALSE;
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_x_randr_edid_dpms_available_get(unsigned char *edid,
                                      unsigned long  edid_length)
{
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return EINA_FALSE;
   return !!(edid[_ECORE_X_RANDR_EDID_OFFSET_DPMS] &
             _ECORE_X_RANDR_EDID_MASK_DPMS);
}

EAPI Eina_Bool
ecore_x_randr_edid_dpms_standby_available_get(unsigned char *edid,
                                              unsigned long  edid_length)
{
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return EINA_FALSE;
   if (edid[_ECORE_X_RANDR_EDID_OFFSET_DPMS] & _ECORE_X_RANDR_EDID_MASK_DPMS)
     return !!(edid[_ECORE_X_RANDR_EDID_OFFSET_DPMS] &
               _ECORE_X_RANDR_EDID_MASK_DPMS_STANDBY);
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_randr_edid_dpms_suspend_available_get(unsigned char *edid,
                                              unsigned long  edid_length)
{
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return EINA_FALSE;
   if (edid[_ECORE_X_RANDR_EDID_OFFSET_DPMS] & _ECORE_X_RANDR_EDID_MASK_DPMS)
     return !!(edid[_ECORE_X_RANDR_EDID_OFFSET_DPMS] &
               _ECORE_X_RANDR_EDID_MASK_DPMS_SUSPEND);
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_randr_edid_dpms_off_available_get(unsigned char *edid,
                                          unsigned long  edid_length)
{
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return EINA_FALSE;
   if (edid[_ECORE_X_RANDR_EDID_OFFSET_DPMS] & _ECORE_X_RANDR_EDID_MASK_DPMS)
     return !!(edid[_ECORE_X_RANDR_EDID_OFFSET_DPMS] &
               _ECORE_X_RANDR_EDID_MASK_DPMS_OFF);
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_randr_edid_display_type_digital_get(unsigned char *edid,
                                            unsigned long  edid_length)
{
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return EINA_FALSE;
   return !!(edid[_ECORE_X_RANDR_EDID_OFFSET_TYPE] &
             _ECORE_X_RANDR_EDID_MASK_DIGITAL);
}

EAPI Ecore_X_Randr_Edid_Display_Colorscheme
ecore_x_randr_edid_display_colorscheme_get(unsigned char *edid,
                                           unsigned long  edid_length)
{
   Ecore_X_Randr_Edid_Display_Colorscheme colorscheme = ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return colorscheme;
   if (ecore_x_randr_edid_display_type_digital_get(edid, edid_length))
     {
        colorscheme = ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_RGB_4_4_4;
        if (edid[_ECORE_X_RANDR_EDID_OFFSET_COLORSPACE] &
            _ECORE_X_RANDR_EDID_MASK_COLORSCHEME_DIGITAL_YCRCB_444)
          colorscheme |= ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_RGB_YCRCB_4_4_4;
        if (edid[_ECORE_X_RANDR_EDID_OFFSET_COLORSPACE] &
            _ECORE_X_RANDR_EDID_MASK_COLORSCHEME_DIGITAL_YCRCB_422)
          colorscheme |= ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_RGB_YCRCB_4_2_2;
     }
   else
     colorscheme = edid[_ECORE_X_RANDR_EDID_OFFSET_COLORSPACE] & _ECORE_X_RANDR_EDID_MASK_COLORSCHEME_ANALOGOUS;
   return colorscheme;
}

EAPI Ecore_X_Randr_Edid_Display_Interface_Type
ecore_x_randr_edid_display_interface_type_get(unsigned char *edid,
                                              unsigned long  edid_length)
{
   Ecore_X_Randr_Edid_Display_Interface_Type type = ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
   int version = ecore_x_randr_edid_version_get(edid, edid_length);

   if (version < ECORE_X_RANDR_EDID_VERSION_13) return type;
   type = edid[_ECORE_X_RANDR_EDID_OFFSET_TYPE] &
     _ECORE_X_RANDR_EDID_MASK_INTERFACE_TYPE;
   if (type > ECORE_X_RANDR_EDID_DISPLAY_INTERFACE_DISPLAY_PORT)
     type = ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
   return type;
}

