/* Edje exported calls */

/************************************************/
/* Basic data storage/retrieval (it's explicit) */
/************************************************/
/* Example:
 * 
 * In the "global" script section of a group:
 * 
 * script {
 *   public global_value1;
 *   public global_value2;
 *   public global_value3;
 * }
 * 
 * In the program script sections, OR in any global functions/routines:
 * ( several examples of setting/getting values)
 * 
 * set_int(global_value1, 7);
 * set_float(global_value2, 42.0);
 * set_str(global_value3, "I am a smelly fish!");
 * new value1 = get_int(global_value1);
 * new Float:value2 = get_float(global_value2);
 * new value3[100]; get_str(global_value3, value3, 100);
 * set_int(global_value1, value1);
 * set_float(global_value2, value2);
 * set_str(global_value3, value3);
 */
native       get_int   (id);
native       set_int   (id, val);
native Float:get_float (id);
native       set_float (id, Float:val);
native       get_strlen(id);
native       get_str   (id, dst[], maxlen);
native       set_str   (id, str[]);

/********************/
/* Edje timer calls */
/********************/
native       timer       (Float:in, fname[], val);
native       cancel_timer(id);

/*******************/
/* Edje anim calls */
/*******************/
native       anim       (Float:len, fname[], val);
native       cancel_anim(id);

/***********************************************************/
/* Edje utility calls for dealing with edjes/programs etc. */
/***********************************************************/
/* Example:
 * 
 * emit("this_thing", "clicked");
 * emit("state", "playing");
 * 
 * set_state(PART:"logo", "glowing", 0.0);
 * set_state(PART:"button", "default", 0.0);
 * 
 * set_tween_state(PART:"button", 0.5, "default", 0.0, "clicked", 0.0);
 * 
 * run_program(PROGRAM:"program_name");
 */
native       emit           (sig[], src[]);
native       set_state      (part_id, state[], Float:state_val);
native       set_tween_state(part_id, Float:tween, state1[], Float:state1_val, state2[], Float:state2_val);
native       run_program    (program_id);

native       stop_program   (program_id);
native       stop_programs_on(part_id);

native       set_drag(part_id, Float:dx, Float:dy);
native       get_drag(part_id, &Float:dx, &Float:dy);
native       set_drag_step(part_id, Float:dx, Float:dy);
native       get_drag_step(part_id, &Float:dx, &Float:dy);
native       set_drag_page(part_id, Float:dx, Float:dy);
native       get_drag_page(part_id, &Float:dx, &Float:dy);

native       text_set(part_id, text[]);

native       set_min_size(part_id, float:w, float:h);
native       get_min_size(part_id, &float:w, &float:h);
native       set_max_size(part_id, float:w, float:h);
native       get_max_size(part_id, &float:w, &float:h);
native       get_size(part_id, &float:w, &float:h);
