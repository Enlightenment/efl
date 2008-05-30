/* ************************** */
/* This is for script-only objects */
/* ************************** */

native e_obj_del(obj);

native e_obj_rect_add();

native e_obj_show(obj);
native e_obj_hide(obj);
native e_obj_move(obj, x, y);
native e_obj_resize(obj, w, h);
native e_obj_geometry_set(obj, x, y, w, h);
native e_obj_geometry_get(obj, &x, &y, &w, &h);
native e_obj_color_set(obj, r, g, b, a);
native e_obj_color_get(obj, &r, &g, &b, &a);

/* threw this in for debug... but need it */
native e_signal_emit(sig[], src[]);













/* ************************** */
/* The below is for non-script-only objects. these calls will not work for */
/* script-only objects. above are for script-only. perhaps this will be */
/* deprecated sometime? or merged to the above partly? */
/* ************************** */

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
/* Edje list calls */
/********************/
native       count      (id);
native       remove     (id, n);
native       append_int (id, v);
native       prepend_int(id, v);
native       insert_int (id, pos, v);
native       replace_int (id, pos, v);
native       fetch_int  (id, pos);
native       append_str (id, str[]);
native       prepend_str(id, str[]);
native       insert_str (id, pos, str[]);
native       replace_str(id, pos, str[]);
native       fetch_str  (id, pos, dst[], maxlen);
native       append_float (id, Float:v);
native       prepend_float(id, Float:v);
native       insert_float (id, pos, Float:v);
native       replace_float(id, pos, Float:v);
native Float:fetch_float  (id, pos);

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
native       emit             (sig[], src[]);
native       set_state        (part_id, state[], Float:state_val);
native       get_state        (part_id, dst[], maxlen, &Float:val);
native       set_tween_state  (part_id, Float:tween, state1[], Float:state1_val, state2[], Float:state2_val);
native       run_program      (program_id);
native       get_drag_dir     (part_id);
native       get_drag         (part_id, &Float:dx, &Float:dy);
native       set_drag         (part_id, Float:dx, Float:dy);
native       get_drag_size    (part_id, &Float:dx, &Float:dy);
native       set_drag_size    (part_id, Float:dx, Float:dy);
native       get_drag_step    (part_id, &Float:dx, &Float:dy);
native       set_drag_step    (part_id, Float:dx, Float:dy);
native       get_drag_page    (part_id, &Float:dx, &Float:dy);
native       set_drag_page    (part_id, Float:dx, Float:dy);
native       set_text         (part_id, str[]);
native       get_text         (part_id, dst[], maxlen);
native       get_min_size     (&w, &h);
native       get_max_size     (&w, &h);
native       get_color_class  (class[], &r, &g, &b, &a);
native       set_color_class  (class[], r, g, b, a);
native       set_text_class   (class[], font[], Float:size);
native       get_text_class   (class[], font[], &Float:size);
native       get_geometry     (part_id, &x, &y, &w, &h);
native       get_mouse        (&x, &y);
native       get_mouse_buttons();
native       stop_program     (program_id);
native       stop_programs_on (part_id);
native       set_min_size     (Float:w, Float:h);
native       set_max_size     (Float:w, Float:h);
native       part_swallow     (part_id, GROUP:str[]);

enum Msg_Type
{
   MSG_NONE = 0,
     MSG_STRING = 2,
     MSG_INT = 3,
     MSG_FLOAT = 4,
     MSG_STRING_SET = 5,
     MSG_INT_SET = 6,
     MSG_FLOAT_SET = 7,
     MSG_STRING_INT = 8,
     MSG_STRING_FLOAT = 9,
     MSG_STRING_INT_SET = 10,
     MSG_STRING_FLOAT_SET = 11
};

native send_message(Msg_Type:type, id, ...);

/**********************/
/* Custom state calls */
/**********************/

/* Example:
 *
 * Create the custom state for "my_part", based on the
 * default description of the same part:
 *
 * custom_state(PART:"my_part", "default", 0.0);
 *
 * Later, use set_state_val() to change the properties of the custom
 * state:
 *
 * set_state_val(PART:"my_part", STATE_ALIGNMENT, 0.5, 0.5);
 *
 * get_state_val() works the same way.
 */

native custom_state(part_id, state[], Float:state_val = 0.0);

enum State_Param
{
	STATE_ALIGNMENT = 1,
	STATE_MIN = 2,
	STATE_MAX = 3,
	STATE_STEP = 4,
	STATE_ASPECT = 5,
	STATE_ASPECT_PREF = 6,
	STATE_COLOR = 7,
	STATE_COLOR2 = 8,
	STATE_COLOR3 = 9,
	STATE_COLOR_CLASS = 10,
	STATE_REL1 = 11,
	STATE_REL1_TO = 12,
	STATE_REL1_OFFSET = 13,
	STATE_REL2 = 14,
	STATE_REL2_TO = 15,
	STATE_REL2_OFFSET = 16,
	STATE_IMAGE = 17,
	STATE_BORDER = 18,
	STATE_FILL_SMOOTH = 19,
	STATE_FILL_POS = 20,
	STATE_FILL_SIZE = 21,
	STATE_TEXT = 22,
	STATE_TEXT_CLASS = 23,
	STATE_TEXT_FONT = 24,
	STATE_TEXT_STYLE = 25,
	STATE_TEXT_SIZE = 26,
	STATE_TEXT_FIT = 27,
	STATE_TEXT_MIN = 28,
	STATE_TEXT_MAX = 29,
	STATE_TEXT_ALIGN = 30,
	STATE_VISIBLE = 31
};

native set_state_val(part_id, State_Param:p, ...);
native get_state_val(part_id, State_Param:p, ...);
