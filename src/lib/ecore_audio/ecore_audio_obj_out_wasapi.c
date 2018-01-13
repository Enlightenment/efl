#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _WIN32

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Eo.h>
#include "ecore_audio_private.h"

#define INITGUID
#include <initguid.h>
#include <functiondiscoverykeys.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <mmreg.h>
#include <wtypes.h>
#include <rpc.h>
#include <rpcdce.h>
#include <propkey.h>

#define MY_CLASS ECORE_AUDIO_OUT_WASAPI_CLASS
#define MY_CLASS_NAME "Ecore_Audio_Out_Wasapi"

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

static int client_connect_count = 0;

DEFINE_PROPERTYKEY(PKEY_Device_FriendlyName,0xb725f130, 0x47ef, 0x101a, 0xa5, 0xf1, 0x02, 0x60, 0x8c, 0x9e, 0xeb, 0xac, 10);

struct _Ecore_Audio_Wasapi_Class {

  Ecore_Job *state_job;
  Eina_List *outputs;
};

typedef struct _Ecore_Audio_Out_Wasapi_Device Ecore_Audio_Out_Wasapi_Device;
typedef struct _Ecore_Audio_Out_Wasapi_Data Ecore_Audio_Out_Wasapi_Data;

struct _Ecore_Audio_Out_Wasapi_Device
{
   IMMDevice *pDevice;
   IMMDeviceEnumerator *pDeviceEnumerator;
};

struct _Ecore_Audio_Out_Wasapi_Data
{
   Eo                   *in;
   Eo                   *out;
   IAudioClient         *client;
   IAudioRenderClient   *render;
   IAudioStreamVolume   *volume;
   WAVEFORMATEXTENSIBLE *wave_format;
   Ecore_Win32_Handler  *handler;
   HANDLE                event;
   UINT32                NumBufferFrames;
   Eina_Bool             spec_format;
   Eina_Bool             play;
};

Ecore_Audio_Out_Wasapi_Device *device = NULL;

EOLIAN static void
_ecore_audio_out_wasapi_ecore_audio_volume_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_Wasapi_Data *_pd, double volume)
{
   HRESULT hr;
   UINT32 count;
   const float  volumes = volume;

   hr = _pd->client->lpVtbl->GetService(_pd->client,
                  &IID_IAudioStreamVolume,
                  (void**)&(_pd->volume));

   if (hr != S_OK)
     {
        ERR("GetService does not have access to the IID_IAudioStreamVolume interface.");
        return;
     }

   hr = _pd->volume->lpVtbl->GetChannelCount(_pd->volume, &count);
   if (hr != S_OK)
     {
        ERR("The GetChannelCount method  can not to gets a count of the channels.");
        return;
     }

   hr = _pd->volume->lpVtbl->SetAllVolumes(_pd->volume, count, &volumes);
   if (hr != S_OK)
     {
        ERR("The SetAllVolumes method can not to sets the individual volume levels for all the channels in the audio stream.");
        return;
     }
}

static void
_clear(Ecore_Audio_Out_Wasapi_Data *_pd)
{
   if (_pd->event)   CloseHandle(_pd->event);
   if (_pd->handler) ecore_main_win32_handler_del(_pd->handler);
   if (_pd->render)  _pd->render->lpVtbl->Release(_pd->render);
   if (_pd->client)  _pd->client->lpVtbl->Release(_pd->client);

   _pd->event   = NULL;
   _pd->handler = NULL;
   _pd->render  = NULL;
   _pd->client  = NULL;
   _pd->play    = EINA_FALSE;
}

static void
_close_cb(void  *data, const Efl_Event *event EINA_UNUSED)
{
   _clear(data);
}

static void
_samplerate_changed_cb(void  *data, const Efl_Event *event EINA_UNUSED)
{
   HRESULT hr;

   IAudioClient                *client;
   IAudioClockAdjustment       *adjustment;
   Ecore_Audio_Out_Wasapi_Data *_pd;

   double       sr;
   double       speed;
   unsigned int samplerate;

   _pd = data;
   client = _pd->client;

   speed      = ecore_audio_obj_in_speed_get(_pd->in);
   samplerate = ecore_audio_obj_in_samplerate_get(_pd->in);
   samplerate = samplerate * speed;

   hr = client->lpVtbl->GetService(client,
                     &IID_IAudioClockAdjustment,
                     (void**)&(adjustment));
   if (hr != S_OK)
     {
         ERR("GetService does not have access to the IID_IAudioClockAdjustments interface.");
         return;
     }

   sr = (REFERENCE_TIME)(_pd->NumBufferFrames / samplerate);
   adjustment->lpVtbl->SetSampleRate(adjustment, sr);
}


static Eina_Bool
_write_cb(void  *data, Ecore_Win32_Handler *wh EINA_UNUSED)
{
   Ecore_Audio_Out_Wasapi_Data *_pd;

   HRESULT hr;
   BYTE   *pData;
   UINT32  numAvailableFrames;
   UINT32  numPaddingFrames;
   int     nBlockAlign;
   ssize_t bread;

   _pd              = data;
   pData            = NULL;
   numPaddingFrames = 0;
   bread            = 0;

   nBlockAlign = _pd->wave_format->Format.nBlockAlign;

   hr = (_pd->client->lpVtbl->GetBufferSize(_pd->client,  &(_pd->NumBufferFrames)));
   if (hr != S_OK)
     {
        ERR("The GetBufferSize does not can retrieves the size (maximum capacity) of the endpoint buffer.");
        return ECORE_CALLBACK_CANCEL;
     }

   if (!_pd->render)
     {
        hr = _pd->client->lpVtbl->GetService(_pd->client,
                         &IID_IAudioRenderClient,
                         (void**)&(_pd->render));
        if (hr != S_OK)
          {
             ERR("GetService does not have access to the IID_IAudioRenderClient interface.");
             return ECORE_CALLBACK_CANCEL;
          }
     }

   numPaddingFrames = 0;
   hr = _pd->client->lpVtbl->GetCurrentPadding(_pd->client, &(numPaddingFrames));
   if (hr == S_OK)
     {
        numAvailableFrames = _pd->NumBufferFrames - numPaddingFrames;
        if (numAvailableFrames == 0) return ECORE_CALLBACK_RENEW;

        hr = _pd->render->lpVtbl->GetBuffer(_pd->render, numAvailableFrames, &pData);
        if (hr == S_OK)
          {
             bread = ecore_audio_obj_in_read(_pd->in, pData,  nBlockAlign*numAvailableFrames);
             if (bread > 0)
               {
                  if (bread < (nBlockAlign * numAvailableFrames))
                    memset((char *)pData + bread, 0, (nBlockAlign * numAvailableFrames) - bread);

                  hr = _pd->render->lpVtbl->ReleaseBuffer(_pd->render, (UINT32)(bread / nBlockAlign), 0);
                  if (hr == S_OK)
                    {
                       if ((bread % nBlockAlign) == 0) return ECORE_CALLBACK_RENEW;
                    }
                  else
                    {
                       ERR("ReleaseBuffer method cannot releases the buffer space acquired in the previous call to the IAudioRenderClient::GetBuffer method.");
                    }
               }
          }
        else
          {
             ERR("GetBuffer method  cannot retrieves a pointer to the next available space in the rendering endpoint buffer into which the caller can write a data packet.");
          }
     }
   else
     {
        ERR("GetCurrentPadding method cannot retrieves the number of frames of padding in the endpoint buffer.");
     }

   if (_pd->client) _pd->client->lpVtbl->Stop(_pd->client);

   if (_pd->handler) ecore_main_win32_handler_del(_pd->handler);
   _pd->handler = NULL;

   efl_event_callback_call(_pd->out, ECORE_AUDIO_OUT_WASAPI_EVENT_STOP, NULL);

   _pd->play = EINA_FALSE;

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
wave_format_selection(Ecore_Audio_Out_Wasapi_Data *_pd, Eo *in)
{
   HRESULT               hr;

   IAudioClient         *client;
   WAVEFORMATEXTENSIBLE *buff;
   WAVEFORMATEXTENSIBLE *correct_wave_format;

   double       speed;
   unsigned int channels;
   unsigned int samplerate;

   client              = _pd->client;
   buff                = NULL;
   correct_wave_format = NULL;

   speed      = ecore_audio_obj_in_speed_get(in);
   channels   = ecore_audio_obj_in_channels_get(in);
   samplerate = ecore_audio_obj_in_samplerate_get(in);
   samplerate = samplerate * speed;

   if (_pd->spec_format)
     {
        CoTaskMemFree(_pd->wave_format);
        _pd->wave_format = NULL;
        _pd->spec_format = EINA_FALSE;
     }

   hr = client->lpVtbl->GetMixFormat(client, (WAVEFORMATEX **)&(buff));
   if (hr != S_OK)
     {
        return EINA_FALSE;
     }

   buff->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
   buff->Format.nChannels = channels;
   buff->Format.nSamplesPerSec = samplerate;
   buff->Format.nBlockAlign = (buff->Format.nChannels * buff->Format.wBitsPerSample) / 8;
   buff->Format.nAvgBytesPerSec = buff->Format.nSamplesPerSec * buff->Format.nBlockAlign;
   buff->Format.cbSize  = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
   buff->dwChannelMask = 0;

   switch(channels)
     {
      case 1:
        buff->dwChannelMask = KSAUDIO_SPEAKER_MONO;
        break;
      case 2:
        buff->dwChannelMask = KSAUDIO_SPEAKER_STEREO;
        break;
      case 4:
        buff->dwChannelMask = KSAUDIO_SPEAKER_QUAD;
        break;
      case 6:
        buff->dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
        break;
      case 8:
        buff->dwChannelMask = KSAUDIO_SPEAKER_7POINT1;
        break;
      default:
        buff->dwChannelMask = 0;
        break;
      }

   buff->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

   hr = client->lpVtbl->IsFormatSupported(client,
                    AUDCLNT_SHAREMODE_SHARED, (WAVEFORMATEX *)buff,
                    (WAVEFORMATEX **)&correct_wave_format);
   switch (hr)
     {
      case S_OK:
        _pd->wave_format = buff;
        return EINA_TRUE;
      case S_FALSE:
        ERR("Succeeded with a closest match to the specified format.");
        _pd->spec_format = EINA_TRUE;
        _pd->wave_format = correct_wave_format;
        return EINA_TRUE;
      case E_POINTER:
        ERR("code: E_POINTER");
        return EINA_FALSE;
      case E_INVALIDARG:
        ERR("code: E_INVALIDARG");
        return EINA_FALSE;
      case AUDCLNT_E_DEVICE_INVALIDATED:
        ERR("code: AUDCLNT_E_DEVICE_INVALIDATED");
        return EINA_FALSE;
      case AUDCLNT_E_SERVICE_NOT_RUNNING:
        ERR("code: AUDCLNT_E_SERVICE_NOT_RUNNING");
        return EINA_FALSE;
      default:
        ERR("IsFormatSupported - return code is unknown");
        return EINA_FALSE;
      }
   return EINA_TRUE;
}


static Eina_Bool
_client_initialize( Ecore_Audio_Out_Wasapi_Data *_pd )
{
   HRESULT hr;
   IAudioClient *client;
   REFERENCE_TIME hnsRequestedDuration;
   WAVEFORMATEXTENSIBLE *correct_wave_format;
   DWORD flags;
   int nbf;
   int sps;

   client               = _pd->client;
   hnsRequestedDuration = REFTIMES_PER_SEC;
   correct_wave_format  = _pd->wave_format;
   flags                = AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_NOPERSIST;

   hr = client->lpVtbl->GetDevicePeriod(client, NULL, &hnsRequestedDuration);

   hr = client->lpVtbl->Initialize(client,
                    AUDCLNT_SHAREMODE_SHARED,
                    flags,
                    hnsRequestedDuration,
                    hnsRequestedDuration,
                    (WAVEFORMATEX *)correct_wave_format,
                    NULL);
   switch (hr)
     {
      case S_OK:
        break;
      case AUDCLNT_E_ALREADY_INITIALIZED:
        ERR("code: AUDCLNT_E_ALREADY_INITIALIZED");
        return EINA_FALSE;
      case AUDCLNT_E_WRONG_ENDPOINT_TYPE:
        ERR("code: AUDCLNT_E_WRONG_ENDPOINT_TYPE");
        return EINA_FALSE;
      case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED:
        ERR("code: AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED");

        hr = client->lpVtbl->GetBufferSize(client,  &(_pd->NumBufferFrames));
        if (hr != S_OK)
          {
             ERR("The GetBufferSize does not can retrieves the size (maximum capacity) of the endpoint buffer.");
             return EINA_FALSE;
          }

        nbf = _pd->NumBufferFrames;
        sps = correct_wave_format->Format.nSamplesPerSec;

        //*Calculate the aligned buffer size in 100-nansecond units (hns).(https://msdn.microsoft.com/en-us/library/windows/desktop/dd370875(v=vs.85).aspx)*//
        hnsRequestedDuration = (REFERENCE_TIME)(((REFTIMES_PER_SEC * nbf) / sps) + 0.5);
        if (client)
          client->lpVtbl->Release(client);

        hr = device->pDevice->lpVtbl->Activate(device->pDevice,
                         &IID_IAudioClient,
                         CLSCTX_ALL,
                         NULL,
                         (void**)&(client));
        if (hr != S_OK)
          {
             ERR("The Activate method cannot create a COM object with the specified interface.");
             return EINA_FALSE;
          }

        hr = client->lpVtbl->GetMixFormat(client, (WAVEFORMATEX **)&(correct_wave_format));
        if (hr != S_OK)
          {
             ERR("The GetMixFormat  cannot retrieves the stream format that the audio engine uses for its internal processing of shared-mode streams.");
             return EINA_FALSE;
          }

        hr = client->lpVtbl->Initialize(client,
                         AUDCLNT_SHAREMODE_SHARED,
                         flags,
                         hnsRequestedDuration,
                         hnsRequestedDuration,
                         (WAVEFORMATEX *)correct_wave_format,
                         NULL);

        if (hr != S_OK) return EINA_FALSE;
        return EINA_TRUE;
      case AUDCLNT_E_BUFFER_SIZE_ERROR:
        ERR("code: AUDCLNT_E_BUFFER_SIZE_ERROR");
        return EINA_FALSE;
      case AUDCLNT_E_CPUUSAGE_EXCEEDED:
        ERR("code: AUDCLNT_E_CPUUSAGE_EXCEEDED");
        return EINA_FALSE;
      case AUDCLNT_E_DEVICE_INVALIDATED:
        ERR("code: AUDCLNT_E_DEVICE_INVALIDATED");
        return EINA_FALSE;
      case AUDCLNT_E_DEVICE_IN_USE:
        ERR("code: AUDCLNT_E_DEVICE_IN_USE");
        return EINA_FALSE;
      case AUDCLNT_E_ENDPOINT_CREATE_FAILED:
        ERR("code: AUDCLNT_E_ENDPOINT_CREATE_FAILED");
        return EINA_FALSE;
      case AUDCLNT_E_INVALID_DEVICE_PERIOD:
        ERR("code: AUDCLNT_E_INVALID_DEVICE_PERIOD");
        return EINA_FALSE;
      case AUDCLNT_E_UNSUPPORTED_FORMAT:
        ERR("code: AUDCLNT_E_UNSUPPORTED_FORMAT");
        return EINA_FALSE;
      case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:
        ERR("code: AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED");
        return EINA_FALSE;
      case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL:
        ERR("code: AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL");
        return EINA_FALSE;
      case AUDCLNT_E_SERVICE_NOT_RUNNING:
        ERR("code: AUDCLNT_E_SERVICE_NOT_RUNNING");
        return EINA_FALSE;
      case E_POINTER:
        ERR("code: E_POINTER");
        return EINA_FALSE;
      case E_INVALIDARG:
        ERR("code: E_INVALIDARG");
        return EINA_FALSE;
      case E_OUTOFMEMORY:
        ERR("code: E_OUTOFMEMORY");
        return EINA_FALSE;
      default:
        ERR("code: ");
        return EINA_FALSE;
     }
   if (!_pd->event )
     {
        _pd->event = CreateEvent(NULL, 0, 0, NULL);
        hr = client->lpVtbl->SetEventHandle(client, _pd->event);
        if (hr != S_OK) return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_input_attach_internal(Eo *eo_obj EINA_UNUSED, Eo *in, Ecore_Audio_Out_Wasapi_Data *_pd)
{
   HRESULT hr;

   if (!device || !device->pDevice) return EINA_FALSE;

   if (!_pd->client)
     {
        hr = device->pDevice->lpVtbl->Activate(device->pDevice,
                         &IID_IAudioClient,
                         CLSCTX_ALL,
                         NULL,
                         (void**)&(_pd->client));
        if (hr != S_OK)
          {
             ERR("The Activate method cannot create a COM object with the specified interface.");
             return EINA_FALSE;
          }
     }

   if (!wave_format_selection( _pd, in))
     return EINA_FALSE;

   if (!_client_initialize(_pd))
     return EINA_FALSE;

   _pd->in = in;

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_ecore_audio_out_wasapi_ecore_audio_out_input_attach(Eo *eo_obj, Ecore_Audio_Out_Wasapi_Data *_pd, Eo *in)
{
   Eina_Bool ret = ecore_audio_obj_out_input_attach(efl_super(eo_obj, MY_CLASS), in);

   if (!ret) return EINA_FALSE;
   if (!device) return EINA_FALSE;
   if (_pd->play) return EINA_TRUE;
   if (!_input_attach_internal(eo_obj, in, _pd)) return EINA_FALSE;

   ecore_main_win32_handler_add(_pd->event, _write_cb, _pd);
   efl_event_callback_add(in, ECORE_AUDIO_IN_EVENT_IN_SAMPLERATE_CHANGED, _samplerate_changed_cb, eo_obj);
   efl_event_callback_add(eo_obj, ECORE_AUDIO_OUT_WASAPI_EVENT_STOP, _close_cb, _pd);

   _pd->play = EINA_TRUE;
   _pd->out  = eo_obj;
   _pd->client->lpVtbl->Start(_pd->client);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_ecore_audio_out_wasapi_ecore_audio_out_input_detach(Eo *eo_obj, Ecore_Audio_Out_Wasapi_Data *_pd EINA_UNUSED, Eo *in)
{
   Eina_Bool ret = ecore_audio_obj_out_input_detach(efl_super(eo_obj, MY_CLASS), in);
   if (!ret) return EINA_FALSE;

   efl_event_callback_call(_pd->out, ECORE_AUDIO_OUT_WASAPI_EVENT_STOP, NULL);
   return EINA_TRUE;
}

EOLIAN static Eo *
_ecore_audio_out_wasapi_efl_object_constructor(Eo *eo_obj, Ecore_Audio_Out_Wasapi_Data *_pd EINA_UNUSED)
{
   HRESULT hr;

   if (!device)
     {
        device = calloc(1, sizeof(Ecore_Audio_Out_Wasapi_Device));
        if (!device) return NULL;
     }

   if (device->pDeviceEnumerator && device->pDevice)
     {
        client_connect_count++;
        eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
        Ecore_Audio_Output *out_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_CLASS);
        out_obj->need_writer = EINA_FALSE;
        return eo_obj;
     }

   hr = CoInitialize(NULL);

   if (hr == S_OK || hr == S_FALSE)
     {
        if (device->pDeviceEnumerator)
          device->pDeviceEnumerator->lpVtbl->Release(device->pDeviceEnumerator);

        if (device->pDevice)
          device->pDevice->lpVtbl->Release(device->pDevice);

        hr = CoCreateInstance(&CLSID_MMDeviceEnumerator,
                         NULL,
                         CLSCTX_ALL,
                         &IID_IMMDeviceEnumerator,
                         (void**)&(device->pDeviceEnumerator));
        if (hr == S_OK)
          {
             hr = device->pDeviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(device->pDeviceEnumerator,
                              eRender,
                              eMultimedia,
                              &(device->pDevice));
             if (hr == S_OK)
               {
                  client_connect_count++;
                  eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
                  Ecore_Audio_Output *out_obj = efl_data_scope_get(eo_obj, ECORE_AUDIO_OUT_CLASS);
                   out_obj->need_writer = EINA_FALSE;
                  return eo_obj;
               }
             device->pDeviceEnumerator->lpVtbl->Release(device->pDeviceEnumerator);
          }
     }

   CoUninitialize();
   free(device);
   device = NULL;
   return NULL;
}

EOLIAN static void
_ecore_audio_out_wasapi_efl_object_destructor(Eo *eo_obj, Ecore_Audio_Out_Wasapi_Data *_pd)
{
   _clear(_pd);
   client_connect_count--;
   efl_destructor(efl_super(eo_obj, MY_CLASS));

   if (!client_connect_count)
     {
        if (device->pDevice)
          device->pDevice->lpVtbl->Release(device->pDevice);

        if (device->pDeviceEnumerator)
          device->pDeviceEnumerator->lpVtbl->Release(device->pDeviceEnumerator);

        free(device);
        device = NULL;
        CoUninitialize();
     }
}

#include "ecore_audio_out_wasapi.eo.c"

#endif /*_WIN32*/
