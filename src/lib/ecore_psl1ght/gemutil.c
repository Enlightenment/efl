#include <stdio.h>
#include <io/move.h>
#include <sys/memory.h>
#include <ppu-types.h>
#include <io/camera.h>
#include <ppu-types.h>
#include <spurs/spurs.h>
#include <sys/spu.h>
#include <sys/thread.h>
#include <sys/systime.h>

#define SPURS_DEFAULT_PREFIX_NAME "gem_spurs"

u16 oldGemPad = 0;
u16 newGemPad = 0;
u16 newGemAnalogT = 0;
extern cameraReadInfo camread;

static inline float
vec_array(vec_float4 vec, unsigned int idx)
{
   union {
      vec_float4 vec;
      float      array[4];
   } v;

   v.vec = vec;

   if (idx > 3)
     return -1;
   return v.array[idx];
}

int
initMove()
{
   Spurs *spurs;
   gemAttribute gem_attr;
   u8 gem_spu_priorities[8] = { 1, 0, 0, 0, 0, 0, 0, 0 };
   int ret;
   int i;

   spurs = initSpurs (NULL);

   if (spurs == NULL)
     goto error;

   printf ("preparing GemAttribute structure with spurs\n");

   initAttributeGem (&gem_attr, 1, NULL, spurs, gem_spu_priorities);
   gem_attr->version = 2;
   gem_attr->max = 1;
   gem_attr->spurs = spurs;
   gem_attr->memory = NULL;
   gem_attr->spu_priorities[0] = 1;
   for (i = 1; i < 8; ++i)
     gem_attr->spu_priorities[i] = 0;

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
   return ret;

error:
   if (spurs)
     endSpurs (spurs);
   return NULL;
}

int
endMove()
{
   endSpurs (spurs);
   gemEnd ();
   return 0;
}

int
proccessGem(int t)
{
   int ret;

   switch (t) {
      case 0:

        ret = gemUpdateStart (camread.buffer, camread.timestamp);

        if (ret != 0)
          {
             printf ("Return from gemUpdateStart %X\n", ret);
          }
        break;

      case 2:

        ret = gemUpdateFinish ();
        if (ret != 0)
          {
             printf ("Return from gemUpdateFinish %X\n", ret);
          }
        break;

      case 3:
        ret = gemConvertVideoFinish ();
        if (ret != 0)
          {
             printf ("Return from gemConvertVideoFinish %X\n", ret);
          }
        break;

      default:
        ret = -1;
        break;
     }
   return ret;
}

void
readGemPad(int num_gem)
{
   gemState gem_state;
   int ret;
   unsigned int hues[] = { 4 << 24, 4 << 24, 4 << 24, 4 << 24 };
   ret = gemGetState (0, 0, -22000, &gem_state);

   newGemPad = gem_state.paddata.buttons & (~oldGemPad);
   newGemAnalogT = gem_state.paddata.ANA_T;
   oldGemPad = gem_state.paddata.buttons;

   switch (ret) {
      case 2:
        gemForceRGB (num_gem, 0.5, 0.5, 0.5);
        break;

      case 5:

        gemTrackHues (hues, NULL);
        break;

      default:
        break;
     }
}

void
readGemAccPosition(int num_gem)
{
   vec_float4 position;

   gemGetAccelerometerPositionInDevice (num_gem, &position);

   printf (" accelerometer device coordinates [%f,%f,%f,%f]\n",
           vec_array (position, 0), vec_array (position, 1), vec_array (position, 2),
           vec_array (position, 3));
}

void
readGemInertial(int num_gem)
{
   gemInertialState gem_inertial_state;
   int ret;

   ret = gemGetInertialState (num_gem, 0, -22000, &gem_inertial_state);
   printf ("gemGetInertialState return %X\n", ret);
   printf ("counter %d temperature %f\n", gem_inertial_state.counter,
           gem_inertial_state.temperature);

   printf (" accelerometer sensor   [%f,%f,%f,%f]\n",
           vec_array (gem_inertial_state.accelerometer, 0),
           vec_array (gem_inertial_state.accelerometer, 1),
           vec_array (gem_inertial_state.accelerometer, 2),
           vec_array (gem_inertial_state.accelerometer, 3));

   printf (" accelerometer_bias sensor   [%f,%f,%f,%f]\n",
           vec_array (gem_inertial_state.accelerometer_bias, 0),
           vec_array (gem_inertial_state.accelerometer_bias, 1),
           vec_array (gem_inertial_state.accelerometer_bias, 2),
           vec_array (gem_inertial_state.accelerometer_bias, 3));

   printf (" gyro sensor  [%f,%f,%f,%f]\n", vec_array (gem_inertial_state.gyro,
                                                       0), vec_array (gem_inertial_state.gyro, 1),
           vec_array (gem_inertial_state.gyro, 2),
           vec_array (gem_inertial_state.gyro, 3));

   printf (" gyro_bias sensor  [%f,%f,%f,%f]\n",
           vec_array (gem_inertial_state.gyro_bias, 0),
           vec_array (gem_inertial_state.gyro_bias, 1),
           vec_array (gem_inertial_state.gyro_bias, 2),
           vec_array (gem_inertial_state.gyro_bias, 3));
}

void
readGem()
{
   gemState gem_state;
   proccessGem (0);

   proccessGem (2);

   readGemPad (0); // This will read buttons from Move
   switch (newGemPad) {
      case 1:
        printf ("Select pressed \n");
        break;

      case 2:
        printf ("T pressed value %d\n", newGemAnalogT);
        printf
          ("Frame %d center of the sphere in world coordinates %f %f %f %f \n",
          camread.frame, vec_array (gem_state.pos, 0), vec_array (gem_state.pos,
                                                                  1), vec_array (gem_state.pos, 2), vec_array (gem_state.pos, 3));
        break;

      case 4:
        printf ("Move pressed \n");
        gemCalibrate (0);
        break;

      case 8:
        printf ("Start pressed \n");
        pos_x = 0;
        pos_y = 0;
        break;

      case 16:
        printf ("Triangle pressed \n");
        getImageState ();
        break;

      case 32:
        printf ("Circle pressed \n");
        break;

      case 64:
        printf ("Cross pressed \n");
        printf ("X,Y,Z position (mm) %f %f %f\n", vec_array (gem_state.pos, 0),
                vec_array (gem_state.pos, 1), vec_array (gem_state.pos, 2));
        readGemAccPosition (0);
        break;

      case 128:
        printf ("Square pressed \n");
        readGemInertial (0);
        break;

      default:
        break;
     }
}

void
getImageState()
{
   int ret;

   gemImageState imgState;

   gemGetImageState (0, &imgState);
   printf (" u  [%f]\n", imgState.u);
   printf (" v  [%f]\n", imgState.v);
   printf (" r  [%f]\n", imgState.r);
   printf (" projectionx  [%f]\n", imgState.projectionx);
   printf (" projectiony  [%f]\n", imgState.projectiony);
   printf (" distance  [%f]\n", imgState.distance);
   printf ("visible=%d r_valid=%d\n", imgState.visible, imgState.r_valid);
   printf ("tiemestamp=%Ld\n", imgState.frame_time);
}

