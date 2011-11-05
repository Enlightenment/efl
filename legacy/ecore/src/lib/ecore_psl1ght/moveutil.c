#include <stdio.h>
#include <io/move.h>
#include <sys/memory.h>
#include <ppu-types.h>
#include <io/camera.h>
#include <ppu-types.h>
#include <spurs/spurs.h>
#include <sys/thread.h>
#include <sys/systime.h>

#include "spursutil.h"
#include "moveutil.h"

u16 oldGemPad = 0;
u16 newGemPad = 0;
u16 newGemAnalogT = 0;

static void
endCamera(moveContext *context)
{
   cameraStop (0);

   cameraClose (0);
   cameraEnd ();

   sysMemContainerDestroy (context->camInfo.container);
}

static int
initCamera(moveContext *context)
{
   int ret;

   context->camInfo.container = NULL;

   ret = cameraInit ();
   printf ("cameraInit() returned %d\n", ret);
   if (ret == 0)
     {
        cameraType type = CAM_TYPE_UNKNOWN;

        ret = cameraGetType (0, &type);
        if (ret == 0 && type == CAM_TYPE_PLAYSTATION_EYE)
          {
             context->camInfo.format = CAM_FORM_RAW8;
             context->camInfo.framerate = 60;
             context->camInfo.resolution = CAM_RESO_VGA;
             context->camInfo.info_ver = 0x0101;
             ret = sysMemContainerCreate (&context->camInfo.container, 0x200000);
             printf ("sysMemContainerCreate() for camera container returned %d\n", ret);

             ret = cameraOpenEx (0, &context->camInfo);
             switch (ret) {
                case 0:
                  printf ("Found me an eye, arrr!\n");
                  printf ("cameraOpenEx returned %08X\n", ret);
                  printf ("Video dimensions: %dx%d\n", context->camInfo.width, context->camInfo.height);
                  printf ("Buffer at %08X\n", context->camInfo.buffer);
                  printf ("pbuf0 Buffer at %08X\n", context->camInfo.pbuf[0]);
                  printf ("pbuf0 Buffer at %08X\n", context->camInfo.pbuf[1]);
                  printf ("context->camInfo.info_ver %X\n", context->camInfo.info_ver);

                  context->camRead.buffer = context->camInfo.buffer;
                  context->camRead.version = 0x0100;
                  printf ("Setting CameraReadEx %08X buffer to cameraInfoex buffer \n",
                          context->camRead.buffer);
                  break;

                default:
                  printf ("Error %X detected opening PlayStation Eye\n", ret);
                  goto error;
               }
          }
        else {
             printf ("Device detected is not a PlayStation Eye and this sample need it\n");
             goto error;
          }
     }
   else {
        goto error;
     }
   return ret;

error:
   if (context->camInfo.container)
     sysMemContainerDestroy (context->camInfo.container);
   return ret;
}

static int
readCamera(moveContext *context)
{
   int ret;

   ret = cameraReadEx (0, &context->camRead);
   switch (ret) {
      case CAMERA_ERRO_NEED_START:
        cameraReset (0);
        ret = gemPrepareCamera (128, 0.5);
        printf ("GemPrepareCamera return %d exposure set to 128 and quality to 0.5 before cameraStart\n",
                ret);
        printf ("lets go!! It's time to look your face in Sony Bravia :P\n");
        ret = cameraStart (0);
        printf ("cameraStart return %d \n", ret);
        printf ("*******************************************\n");
        printf ("* Now make sure you have a Move connected\n");
        printf ("* and point it towards the camera and press\n");
        printf ("* the action button to calibrate\n");
        printf ("*******************************************\n");
        break;

      case 0:
        break;

      default:
        printf ("error %08X ", ret);
        ret = 1;
        break;
     }
   // printf("despues de start return %d \n",ret);
   if (ret == 0 && context->camRead.readcount != 0)
     {
        return context->camRead.readcount;
     }
   else {
        return 0;
     }
}

moveContext *
initMove()
{
   moveContext *context = NULL;
   Spurs *spurs;
   gemAttribute gem_attr;
   int ret;
   int i;

   spurs = initSpurs ("gem_spurs");

   if (spurs == NULL)
     goto error;

   printf ("preparing GemAttribute structure with spurs\n");

   gem_attr.version = 2;
   gem_attr.max = 1;
   gem_attr.spurs = spurs;
   gem_attr.memory = NULL;
   gem_attr.spu_priorities[0] = 1;
   for (i = 1; i < 8; ++i)
     gem_attr.spu_priorities[i] = 0;

   printf ("calling GemInit with GemAttribute structure version=%d max_connect=%d spurs=%X\n",
           gem_attr.version, gem_attr.max, gem_attr.spurs);
   ret = gemInit (&gem_attr);
   printf ("return from GemInit %X \n", ret);
   if (ret)
     goto error;

   ret = gemPrepareCamera (128, 0.5);
   printf ("GemPrepareCamera return %d exposure set to 128 and quality to 0.5\n",
           ret);
   if (ret)
     goto error;
   ret = gemReset (0);
   printf ("GemReset return %X \n", ret);
   if (ret)
     goto error;

   context = (moveContext *)malloc (sizeof (moveContext));
   context->spurs = spurs;
   ret = initCamera (context);

   if (ret == 0)
     return context;

error:
   if (spurs)
     endSpurs (spurs);
   if (context)
     free (context);
   return NULL;
}

void
endMove(moveContext *context)
{
   /* Stop Move */
    gemEnd ();
    /* Stop Camera */
    endCamera (context);
    /* Stop Spurs */
    endSpurs (context->spurs);

    free (context);
}

int
processMove(moveContext *context)
{
   const unsigned int hues[] = { 4 << 24, 4 << 24, 4 << 24, 4 << 24 };
   int ret = -1;

   if (readCamera (context) > 0)
     {
        ret = gemUpdateStart (context->camRead.buffer, context->camRead.timestamp);
        //printf ("Return from gemUpdateStart %X\n", ret);
        if (ret == 0)
          {
             ret = gemUpdateFinish ();
             //printf ("Return from gemUpdateFinish %X\n", ret);
             if (ret == 0)
               {
                  ret = gemGetState (0, STATE_LATEST_IMAGE_TIME, 0, &context->state);
                  switch (ret) {
                     case 2:
                       gemForceRGB (0, 0.5, 0.5, 0.5);
                       break;

                     case 5:
                       gemTrackHues (hues, NULL);
                       break;

                     default:
                       break;
                    }
               }
          }
     }

   return ret;
}

void
moveGet3DPosition(moveContext *context, float *x, float *y, float *z)
{
   if (x)
     *x = vec_array (context->state.pos, 0);
   if (y)
     *y = vec_array (context->state.pos, 1);
   if (z)
     *z = vec_array (context->state.pos, 2);
}

