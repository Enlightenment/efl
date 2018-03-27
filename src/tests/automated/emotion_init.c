   fail_if(!ecore_evas_init(), "ERROR: Cannot init ECore Evas!\n");
   fail_if(!emotion_init(), "ERROR: Cannot init Emotion!\n");

   Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, 200, 200, NULL);
   fail_if(!ee, "Error: Canot create ecore evas!\n");

   parent = ecore_evas_get(ee);
   fail_if(!parent, "Error: Canot get ecore evas obj!\n");
