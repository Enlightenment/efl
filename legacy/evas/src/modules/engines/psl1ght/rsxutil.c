/*
 * This software is distributed under the terms of the MIT License
 */

#include <ppu-lv2.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sysutil/video.h>
#include <rsx/gcm_sys.h>
#include <rsx/rsx.h>
#include <io/pad.h>
#include <time.h>
#include <cairo/cairo.h>
#include <math.h>

#include "rsxutil.h"

#define GCM_LABEL_INDEX 255

static void
waitRSXIdle(gcmContextData *context);

static int flipped = FALSE;

void
waitFlip()
{
   if (flipped)
     {
        while (gcmGetFlipStatus () != 0)
          usleep (200);  /* Sleep, to not stress the cpu. */
        gcmResetFlipStatus ();
        flipped = FALSE;
     }
}

int
flipBuffer(gcmContextData *context, s32 buffer)
{
   if (gcmSetFlip (context, buffer) == 0)
     {
        rsxFlushBuffer (context);
        // Prevent the RSX from continuing until the flip has finished.
        gcmSetWaitFlip (context);

        flipped = TRUE;
        return TRUE;
     }
   return FALSE;
}

int
makeBuffer(rsxBuffer *buffer, u16 width, u16 height, int id)
{
   int depth = sizeof(u32);
   int pitch = depth * width;
   int size = depth * width * height;

   buffer->ptr = (uint32_t *)rsxMemalign (64, size);

   if (buffer->ptr == NULL)
     goto error;

   if (rsxAddressToOffset (buffer->ptr, &buffer->offset) != 0)
     goto error;

   /* Register the display buffer with the RSX */
   if (gcmSetDisplayBuffer (id, buffer->offset, pitch, width, height) != 0)
     goto error;

   buffer->width = width;
   buffer->height = height;
   buffer->id = id;

   return TRUE;

error:
   if (buffer->ptr != NULL)
     rsxFree (buffer->ptr);

   return FALSE;
}

int
copyBuffer(gcmContextData *context, rsxBuffer *source, rsxBuffer *destination)
{
   rsxSetTransferData(context, GCM_TRANSFER_LOCAL_TO_LOCAL,
                      destination->offset,
                      destination->width * sizeof(u32),
                      source->offset,
                      source->width * sizeof(u32),
                      source->width * sizeof(u32),
                      source->height);
}

int
getResolution(u16 *width, u16 *height)
{
   videoState state;
   videoResolution resolution;

   /* Get the state of the display */
   if (videoGetState (0, 0, &state) == 0 &&
       videoGetResolution (state.displayMode.resolution, &resolution) == 0)
     {
        if (width)
          *width = resolution.width;
        if (height)
          *height = resolution.height;

        return TRUE;
     }
   return FALSE;
}

static u8
getPreferredResolution(u16 width, u16 height)
{
   videoDeviceInfo info;
   videoResolution res;
   int area = width * height;
   int mode_area;
   int min_area_diff = abs (area - (720 * 480));
   int area_diff;
   u8 resolution = VIDEO_RESOLUTION_480;
   int i;

   videoGetDeviceInfo(0, 0, &info);

   for (i = 0; i < info.availableModeCount; i++) {
        videoGetResolution (info.availableModes[i].resolution, &res);
        mode_area = res.width * res.height;
        area_diff = abs (area - mode_area);
        if (area_diff < min_area_diff)
          {
             min_area_diff = area_diff;
             resolution = info.availableModes[i].resolution;
          }
     }

   return resolution;
}

int
setResolution(gcmContextData *context, u16 *width, u16 *height)
{
   videoState state;
   videoConfiguration vconfig;
   videoResolution res;
   u8 resolution;

   resolution = getPreferredResolution (*width, *height);

   /* Get the state of the display */
   if (videoGetState (0, 0, &state) != 0)
     return FALSE;

   /* Make sure display is enabled */
   if (state.state != 0)
     return FALSE;

   if (videoGetResolution (resolution, &res) != 0)
     return FALSE;

   /* Configure the buffer format to xRGB */
   memset (&vconfig, 0, sizeof(videoConfiguration));
   vconfig.resolution = resolution;
   vconfig.format = VIDEO_BUFFER_FORMAT_XRGB;
   vconfig.pitch = res.width * sizeof(u32);
   vconfig.aspect = VIDEO_ASPECT_AUTO;

   flushRSX(context);

   if (videoConfigure (0, &vconfig, NULL, 0) != 0)
     return FALSE;

   *width = res.width;
   *height = res.height;

   return TRUE;
}

gcmContextData *
initScreen(void *host_addr, u32 size)
{
   gcmContextData *context = NULL; /* Context to keep track of the RSX buffer. */
   videoState state;
   videoConfiguration vconfig;
   videoResolution res; /* Screen Resolution */

   /* Initilise Reality, which sets up the command buffer and shared IO memory */
   context = rsxInit (CB_SIZE, size, host_addr);
   if (context == NULL)
     goto error;

   /* Get the state of the display */
   if (videoGetState (0, 0, &state) != 0)
     goto error;

   /* Make sure display is enabled */
   if (state.state != 0)
     goto error;

   /* Get the current resolution */
   if (videoGetResolution (state.displayMode.resolution, &res) != 0)
     goto error;

   /* Configure the buffer format to xRGB */
   memset (&vconfig, 0, sizeof(videoConfiguration));
   vconfig.resolution = state.displayMode.resolution;
   vconfig.format = VIDEO_BUFFER_FORMAT_XRGB;
   vconfig.pitch = res.width * sizeof(u32);
   vconfig.aspect = state.displayMode.aspect;

   flushRSX(context);

   if (videoConfigure (0, &vconfig, NULL, 0) != 0)
     goto error;

   if (videoGetState (0, 0, &state) != 0)
     goto error;

   gcmSetFlipMode (GCM_FLIP_VSYNC); // Wait for VSYNC to flip

   gcmResetFlipStatus();

   return context;

error:
   if (context)
     rsxFinish (context, 0);

   return NULL;
}

void
freeScreen(gcmContextData *context)
{
   rsxFinish (context, 0);
}

static void
waitFinish(gcmContextData *context, u32 sLabelVal)
{
   rsxSetWriteBackendLabel (context, GCM_LABEL_INDEX, sLabelVal);

   rsxFlushBuffer (context);

   while (*(vu32 *)gcmGetLabelAddress (GCM_LABEL_INDEX) != sLabelVal)
     usleep(30);
}

static void
waitRSXIdle(gcmContextData *context)
{
   static u32 sLabelVal = 1;

   rsxSetWriteBackendLabel (context, GCM_LABEL_INDEX, sLabelVal);
   rsxSetWaitLabel (context, GCM_LABEL_INDEX, sLabelVal);

   sLabelVal++;

   waitFinish(context, sLabelVal++);
}

void
flushRSX(gcmContextData *context)
{
   if (flipped)
     waitFlip ();
   waitRSXIdle(context);
}

