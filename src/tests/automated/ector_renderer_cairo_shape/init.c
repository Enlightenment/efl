Ector_Surface *surface = efl_add(ECTOR_CAIRO_SOFTWARE_SURFACE_CLASS, NULL);
obj = efl_add_ref(ECTOR_RENDERER_CAIRO_SHAPE_CLASS, parent, ector_renderer_surface_set(efl_added, surface));
