#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eo.h>
#include "ecore_audio_private.h"

#include <CoreAudio/CoreAudio.h>

/* Notes:
 *
 * A lot of source code on the internet dealing with CoreAudio is deprecated.
 * sndfile-play (bundled with libsndfile) is no exception and uses an almost
 * 10 years old API. Nethertheless, sndfile-play has been heavily used to
 * create the CoreAudio module.
 *
 * Documentation is almost non-existant, but here is the technical note from
 * Apple explaining how CoreAudio objects should be manipulated:
 * https://developer.apple.com/library/mac/technotes/tn2223/_index.html
 */

#include "ecore_audio_obj_out_core_audio.h"

typedef struct
{
   Eo                                  *input;
   Eo                                  *output;
   AudioDeviceIOProcID                  proc_id;
   AudioStreamBasicDescription          format;
   AudioObjectID                        obj_id;
   UInt32                               buf_size;

   Eina_Bool                            is_playing;
   Eina_Bool                            fake_stereo;
} Core_Audio_Helper;


/* Apple's error codes are tricky: they are stored as 32 bits integers.
 * However, they are supposed to be represented as 4-bytes strings.
 * There is no equivalent of strerror() (of what I know).
 *
 * Ref: http://vgable.com/blog/2008/04/23/printing-a-fourcharcode/
 *
 * In case of error, take a look at CoreAudio/AudioHardwareBase.h where
 * the error codes are explained.
 */
#define APPLE_ERROR(err_) \
   (char[5]){((err_) >> 24) & 0xff, ((err_) >> 16) & 0xff, ((err_) >> 8) & 0xff, (err_) & 0xff, 0}

#define MY_CLASS ECORE_AUDIO_OUT_CORE_AUDIO_CLASS
#define MY_CLASS_NAME "Ecore_Audio_Out_Core_Audio"

/*============================================================================*
 *                                 Helper API                                 *
 *============================================================================*/

static Core_Audio_Helper *
_core_audio_helper_new(void)
{
   return calloc(1, sizeof(Core_Audio_Helper));
}

static void
_core_audio_helper_stop(Core_Audio_Helper *helper)
{
   EINA_SAFETY_ON_NULL_RETURN(helper);

   OSStatus err;

   if (!helper->is_playing) return;

   /* Stop audio device */
   err = AudioDeviceStop(helper->obj_id, helper->proc_id);
   if (EINA_UNLIKELY(err != noErr))
     ERR("Failed to stop audio device %i for proc id %p: '%s'",
         helper->obj_id, helper->proc_id, APPLE_ERROR(err));

   /* Remove proc ID */
   err = AudioDeviceDestroyIOProcID(helper->obj_id, helper->proc_id);
   if (EINA_UNLIKELY(err != noErr))
     ERR("Failed to stop audio device %i for proc id %p: '%s'",
         helper->obj_id, helper->proc_id, APPLE_ERROR(err));

   helper->is_playing = EINA_FALSE;
}

static void
_core_audio_helper_free(Core_Audio_Helper *helper)
{
   EINA_SAFETY_ON_NULL_RETURN(helper);

   if (helper->is_playing)
     _core_audio_helper_stop(helper);
   free(helper);
}


/*============================================================================*
 *                           Audio Object Properties                          *
 *============================================================================*/

static OSStatus
_audio_object_id_get(AudioObjectID *obj_id)
{
   OSStatus err;
   UInt32 size;
   AudioObjectPropertyAddress prop = {
      kAudioHardwarePropertyDefaultOutputDevice,
      kAudioObjectPropertyScopePlayThrough,
      kAudioObjectPropertyElementMaster
   };

   /* Default output device */
   size = sizeof(AudioObjectID);
   err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &prop, 0, NULL,
                                    &size, obj_id);
   return err;
}

static OSStatus
_audio_device_stream_format_get(AudioObjectID                obj_id,
                                AudioStreamBasicDescription *format)
{
   OSStatus err;
   UInt32 size;
   AudioObjectPropertyAddress prop = {
      kAudioDevicePropertyStreamFormat,
      kAudioObjectPropertyScopePlayThrough,
      kAudioObjectPropertyElementMaster /* Channel number */
   };

   size = sizeof(AudioStreamBasicDescription);
   err = AudioObjectGetPropertyData(obj_id, &prop, 0, NULL, &size, format);
   return err;
}

static OSStatus
_audio_device_stream_format_set(AudioObjectID                obj_id,
                                AudioStreamBasicDescription *format)
{
   OSStatus err;
   UInt32 size;
   AudioObjectPropertyAddress prop = {
      kAudioDevicePropertyStreamFormat,
      kAudioObjectPropertyScopePlayThrough,
      kAudioObjectPropertyElementMaster /* Channel number */
   };

   size = sizeof(AudioStreamBasicDescription);
   err = AudioObjectSetPropertyData(obj_id, &prop, 0, NULL, size, format);
   return err;
}


/*============================================================================*
 *                               Audio Callback                               *
 *============================================================================*/

static OSStatus
_audio_io_proc_cb(AudioObjectID          obj_id         EINA_UNUSED,
                  const AudioTimeStamp  *in_now         EINA_UNUSED,
                  const AudioBufferList *input_data     EINA_UNUSED,
                  const AudioTimeStamp  *input_time     EINA_UNUSED,
                  AudioBufferList       *output_data,
                  const AudioTimeStamp  *in_output_time EINA_UNUSED,
                  void                  *data)
{
   Core_Audio_Helper *helper = data;
   float *buf;
   int size, bread, sample_count, k;

   size = output_data->mBuffers[0].mDataByteSize;
   buf = output_data->mBuffers[0].mData;
   sample_count = size / sizeof(float);

   if (helper->fake_stereo)
     {
        bread = ecore_audio_obj_in_read(helper->input, buf, size * 2);

        for (k = bread - 1; k >= 0; --k)
          {
             buf[2 * k + 0] = buf[k];
             buf[2 * k + 1] = buf[k];
          }
        bread /= 2;
     }
   else
     {
        bread = ecore_audio_obj_in_read(helper->input, buf, size * 4);
        bread /= 4;
     }

   /* Done playing */
   if (bread < sample_count)
     {
        INF("Done playing: %i < %i", bread, sample_count);
        /* Auto-detached. Don't need to do more. */
     }

   return noErr;
}


/*============================================================================*
 *                                   Eo API                                   *
 *============================================================================*/

EOLIAN static void
_ecore_audio_out_core_audio_ecore_audio_volume_set(Eo *obj, void *sd EINA_UNUSED, double volume)
{
   // TODO Change volume of playing inputs
   ecore_audio_obj_volume_set(eo_super(obj, MY_CLASS), volume);
}

EOLIAN static Eina_Bool
_ecore_audio_out_core_audio_ecore_audio_out_input_attach(Eo *obj, void *sd EINA_UNUSED, Eo *input)
{
   Core_Audio_Helper *helper;
   UInt32 channels;
   OSStatus err;
   Eina_Bool chk;

   chk = ecore_audio_obj_out_input_attach(eo_super(obj, MY_CLASS), input);
   if (EINA_UNLIKELY(!chk))
     {
        ERR("Failed to attach input (eo_do_super)");
        goto return_failure;
     }

   helper = _core_audio_helper_new();
   if (EINA_UNLIKELY(helper == NULL))
     {
        CRI("Failed to allocate memory");
        goto detach;
     }

   /* Keep track of input source and output object */
   helper->input = input;
   helper->output = obj;

   /* Default output device */
   err = _audio_object_id_get(&(helper->obj_id));
   if (EINA_UNLIKELY(err != noErr))
     {
        ERR("Failed to get object property: default output device: '%s'",
            APPLE_ERROR(err));
        goto free_helper;
     }

   /* Get data format description */
   err = _audio_device_stream_format_get(helper->obj_id, &(helper->format));
   if (EINA_UNLIKELY(err != noErr))
     {
        ERR("Failed to get property: stream format: '%s'", APPLE_ERROR(err));
        goto free_helper;
     }

   /* Forward samplerate to CoreAudio */
   helper->format.mSampleRate = ecore_audio_obj_in_samplerate_get(input);

   /* Set channels. If only 1 channel, emulate stereo */
   channels = ecore_audio_obj_in_channels_get(input);
   if (channels == 1)
     {
        DBG("Fake stereo enabled for input %p", input);
        helper->fake_stereo = EINA_TRUE;
        channels = 2;
     }
   helper->format.mChannelsPerFrame = channels;

   /* Set new format description */
   err = _audio_device_stream_format_set(helper->obj_id, &(helper->format));
   if (EINA_UNLIKELY(err != noErr))
     {
        ERR("Failed to set property: stream format: '%s'", APPLE_ERROR(err));
        goto free_helper;
     }

   /* We want linear PCM */
   if (helper->format.mFormatID != kAudioFormatLinearPCM)
     {
        ERR("Invalid format ID. Expected linear PCM: '%s'", APPLE_ERROR(err));
        goto free_helper;
     }

   /* Create IO proc ID */
   err = AudioDeviceCreateIOProcID(helper->obj_id, _audio_io_proc_cb,
                                   helper, &(helper->proc_id));
   if (err != noErr)
     {
        ERR("Failed to create IO proc ID. Error: '%s'", APPLE_ERROR(err));
        goto free_helper;
     }

   /* Keep track of data for deallocation */
   efl_key_data_set(input, "coreaudio_data", helper);

   /* Start playing */
   helper->is_playing = EINA_TRUE;
   err = AudioDeviceStart(helper->obj_id, helper->proc_id);
   if (err != noErr)
     {
        ERR("Failed to start proc ID %p for device id %i: '%s'",
            helper->proc_id, helper->obj_id, APPLE_ERROR(err));
        goto free_proc_id;
     }

   return EINA_TRUE;

free_proc_id:
   AudioDeviceDestroyIOProcID(helper->obj_id, helper->proc_id);
free_helper:
   free(helper);
detach:
   ecore_audio_obj_out_input_detach(eo_super(obj, MY_CLASS), input);
return_failure:
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_ecore_audio_out_core_audio_ecore_audio_out_input_detach(Eo *obj, void *sd EINA_UNUSED, Eo *input)
{
   Core_Audio_Helper *data;
   Eina_Bool ret;

   DBG("Detach");
   /* Free helper */
   data = efl_key_data_get(input, "coreaudio_data");
   _core_audio_helper_free(data);

   ret = ecore_audio_obj_out_input_detach(eo_super(obj, MY_CLASS), input);

   return ret;
}

#include "ecore_audio_out_core_audio.eo.c"
