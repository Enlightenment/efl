/* edje exported calls */
native       get_int   (id);
native       set_int   (id, val);
native Float:get_float (id);
native       set_float (id, Float:val);
native       get_strlen(id);
native       get_str   (id, dst[], maxlen);
native       set_str   (id, str[]);
      
native       tst();
native       emit(sig[], src[]);

