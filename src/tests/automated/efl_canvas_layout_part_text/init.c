   parent = evas_new();
   evas_output_method_set(parent, evas_render_method_lookup("buffer"));
   Evas_Engine_Info *einfo = evas_engine_info_get(parent);
   evas_engine_info_set(parent, einfo);

   evas_output_size_set(parent, 500, 500);
   evas_output_viewport_set(parent, 0, 0, 500, 500);
   (void)obj;
