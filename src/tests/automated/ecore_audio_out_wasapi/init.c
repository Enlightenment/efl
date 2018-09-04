#ifdef _WIN32
   obj = efl_add_ref(ECORE_AUDIO_OUT_WASAPI_CLASS, parent);
   fail_if(!obj, "ERROR: Cannot init Ecore.Audio.Out.Wasapi!\n")
