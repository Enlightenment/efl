#include "edje_private.h"

/*
 * ALREADY EXPORTED BY EMBRYO:
 *
 * enum Float_Round_Method {
 *    ROUND, FLOOR, CEIL, TOZERO
 * };
 * enum Float_Angle_Mode {
 *    RADIAN, DEGREES, GRADES
 * };
 *
 * numargs();
 * getarg(arg, index=0);
 * setarg(arg, index=0, value);
 *
 * Float:atof(string[]);
 * Float:fract(Float:value);
 *       round(Float:value, Float_Round_Method:method=ROUND);
 * Float:sqrt(Float:value);
 * Float:pow(Float:value, Float:exponent);
 * Float:log(Float:value, Float:base=10.0);
 * Float:sin(Float:value, Float_Angle_Mode:mode=RADIAN);
 * Float:cos(Float:value, Float_Angle_Mode:mode=RADIAN);
 * Float:tan(Float:value, Float_Angle_Mode:mode=RADIAN);
 * Float:abs(Float:value);
 *       atoi(str[]);
 *       fnmatch(glob[], str[]);
 *       strcmp(str1[], str2[]);
 *       strncmp(str1[], str2[]);
 *       strcpy(dst[], src[]);
 *       strncpy(dst[], src[], n);
 *       strlen(str[]);
 *       strcat(dst[], src[]);
 *       strncat(dst[], src[], n);
 *       strprep(dst[], src[]);
 *       strnprep(dst[], src[], n);
 *       strcut(dst[], str[], n, n2);
 *       snprintf(dst[], dstn, fmt[], ...);
 *       strstr(str[], ndl[]);
 *       strchr(str[], ch[]);
 *       strrchr(str[], ch[]);
 *       rand();
 * Float:randf();
 * Float:seconds();
 *       date(&year, &month, &day, &yearday, &weekday, &hr, &min, &Float:sec);
 *
 */

/* EDJE...
 *
 * implemented so far as examples:
 *
 * enum Msg_Type {
 *    MSG_NONE, MSG_STRING, MSG_INT, MSG_FLOAT, MSG_STRING_SET, MSG_INT_SET,
 *    MSG_FLOAT_SET, MSG_STRING_INT, MSG_INT_FLOAT, MSG_STRING_INT_SET,
 *    MSG_INT_FLOAT_SET
 * };
 *
 * get_int(id)
 * set_int(id, v)
 * Float:get_float (id)
 * set_float(id, Float:v)
 * get_strlen(id)
 * get_str(id, dst[], maxlen)
 * set_str(id, str[])
 * timer(Float:in, fname[], val)
 * cancel_timer(id)
 * reset_timer(id)
 * anim(Float:len, fname[], val)
 * cancel_anim(id)
 * emit(sig[], src[])
 * set_state(part_id, state[], Float:state_val)
 * get_state(part_id, dst[], maxlen, &Float:val)
 * set_tween_state(part_id, Float:tween, state1[], Float:state1_val, state2[], Float:state2_val)
 * play_sample(sample_name, speed, ...)
 * play_tone(tone_name, duration, ...)
 * play_vibration(sample_name, repeat)
 * run_program(program_id)
 * Direction:get_drag_dir(part_id)
 * get_drag(part_id, &Float:dx, &Float:&dy)
 * set_drag(part_id, Float:dx, Float:dy)
 * get_drag_size(part_id, &Float:dx, &Float:&dy)
 * set_drag_size(part_id, Float:dx, Float:dy)
 * set_text(part_id, str[])
 * get_text(part_id, dst[], maxlen)
 * get_min_size(w, h)
 * get_max_size(w, h)
 * set_color_class(class[], r, g, b, a)
 * get_color_class(class[], &r, &g, &b, &a)
 * set_text_class(class[], font[], Float:size)
 * get_text_class(class[], font[], &Float:size)
 * get_drag_step(part_id, &Float:dx, &Float:&dy)
 * set_drag_step(part_id, Float:dx, Float:dy)
 * get_drag_page(part_id, &Float:dx, &Float:&dy)
 * set_drag_page(part_id, Float:dx, Float:dy)
 * get_geometry(part_id, &x, &y, &w, &h)
 * get_mouse(&x, &y)
 * stop_program(program_id)
 * stop_programs_on(part_id)
 * set_min_size(w, h)
 * set_max_size(w, h)
 * send_message(Msg_Type:type, id, ...)
 *
 * count(id)
 * remove(id, n)
 *
 * append_int(id, v)
 * prepend_int(id, v)
 * insert_int(id, n, v)
 * replace_int(id, n, v)
 * fetch_int(id, n)
 *
 * append_str(id, str[])
 * prepend_str(id, str[])
 * insert_str(id, n, str[])
 * replace_str(id, n, str[])
 * fetch_str(id, n, dst[], maxlen)
 *
 * append_float(id, Float:v)
 * prepend_float(id, Float:v)
 * insert_float(id, n, Float:v)
 * replace_float(id, n, Float:v)
 * Float:fetch_float(id, n)
 *
 * custom_state(part_id, state[], Float:state_val = 0.0)
 * set_state_val(part_id, State_Param:param, ...)
 * get_state_val(part_id, State_Param:param, ...)
 *
 * Supported parameters:
 * align[Float:x, Float:y]
 * min[w, h]
 * max[w, h]
 * step[x,y]
 * aspect[Float:min, Float:max]
 * color[r,g,b,a]
 * color2[r,g,b,a]
 * color3[r,g,b,a]
 * aspect_preference
 * rel1[relx,rely]
 * rel1[part_id,part_id]
 * rel1[offx,offy]
 * rel2[relx,relyr]
 * rel2[part_id,part_id]
 * rel2[offx,offy]
 * image[image_id] <- all images have an Id not name in the edje
 * border[l,r,t,b]
 * fill[smooth]
 * fill[pos_relx,pos_rely,pos_offx,pos_offy]
 * fill[sz_relx,sz_rely,sz_offx,sz_offy]
 * color_class
 * text[text]
 * text[text_class]
 * text[font]
 * text[size]
 * text[style]
 * text[fit_x,fit_y]
 * text[min_x,min_y]
 * text[align_x,align_y]
 * visible[on]
 * map_on[on]
 * map_persp[part_id]
 * map_light[part_id]
 * map_rot_center[part_id]
 * map_rot_x[deg]
 * map_rot_y[deg]
 * map_rot_z[deg]
 * map_back_cull[on]
 * map_persp_on[on]
 * persp_zplane[z]
 * persp_focal[z]
 * box[layout]
 * box[fallback_layout]
 * box[Float:align_x, Float:align_y]
 * box[padding_x, padding_y]
 * box[min_x, min_y]
 *
 * ** part_id and program_id need to be able to be "found" from strings
 *
 * get_drag_count(part_id, &Float:dx, &Float:&dy)
 * set_drag_count(part_id, Float:dx, Float:dy)
 * set_drag_confine(part_id, confine_part_id)
 * get_size(&w, &h);
 * resize_request(w, h)
 * get_mouse_buttons()
 * //set_type(part_id, Type:type)
 * //set_effect(part_id, Effect:fx)
 * set_mouse_events(part_id, ev)
 * get_mouse_events(part_id)
 *
 * Pointer_Mode {
 *   POINTER_MODE_AUTOGRAB = 0,
 *   POINTER_MODE_NOGRAB = 1,
 *   POINTER_MODE_NOGREP = 2,
 * }
 * 
 * set_pointer_mode(part_id, mode)
 * set_repeat_events(part_id, rep)
 * get_repeat_events(part_id)
 * set_ignore_flags(part_id, flags)
 * get_ignore_flags(part_id)
 * set_mask_flags(part_id, flags)
 * get_mask_flags(part_id)
 *
 * set_focus(part_id, seat_name[])
 * unset_focus(seat_name[])
 *
 * part_swallow(part_id, group_name)
 *
 * external_param_get_int(id, param_name[])
 * external_param_set_int(id, param_name[], value)
 * Float:external_param_get_float(id, param_name[])
 * external_param_set_float(id, param_name[], Float:value)
 * external_param_get_strlen(id, param_name[])
 * external_param_get_str(id, param_name[], value[], value_maxlen)
 * external_param_set_str(id, param_name[], value[])
 * external_param_get_choice_len(id, param_name[])
 * external_param_get_choice(id, param_name[], value[], value_maxlen)
 * external_param_set_choice(id, param_name[], value[])
 * external_param_get_bool(id, param_name[])
 * external_param_set_bool(id, param_name[], value)
 *
 * physics_impulse(part_id, Float:x, Float:y, Float:z)
 * physics_torque_impulse(part_id, Float:x, Float:y, Float:z)
 * physics_force(part_id, Float:x, Float:y, Float:z)
 * physics_torque(part_id, Float:x, Float:y, Float:z)
 * physics_clear_forces(part_id)
 * physics_get_forces(part_id, &Float:x, &Float:y, &Float:z)
 * physics_get_torques(part_id, &Float:x, &Float:y, &Float:z)
 * physics_set_velocity(part_id, Float:x, Float:y, Float:z)
 * physics_get_velocity(part_id, &Float:x, &Float:y, &Float:z)
 * physics_set_ang_velocity(part_id, Float:x, Float:y, Float:z)
 * physics_get_ang_velocity(part_id, &Float:x, &Float:y, &Float:z)
 * physics_stop(part_id)
 * physics_set_rotation(part_id, Float:w, Float:x, Float:y, Float:z)
 * physics_get_rotation(part_id, &Float:w, &Float:x, &Float:y, &Float:z)
 *
 * ADD/DEL CUSTOM OBJECTS UNDER SOLE EMBRYO SCRIPT CONTROL
 *
 */

/* get_int(id) */
static Embryo_Cell
_edje_embryo_fn_get_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   return (Embryo_Cell)_edje_var_int_get(ed, (int)params[1]);
}

/* set_int(id, v) */
static Embryo_Cell
_edje_embryo_fn_set_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   _edje_var_int_set(ed, (int)params[1], (int)params[2]);
   return 0;
}

/* get_float(id) */
static Embryo_Cell
_edje_embryo_fn_get_float(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   float v;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   v = (float)_edje_var_float_get(ed, params[1]);
   return EMBRYO_FLOAT_TO_CELL(v);
}

/* set_float(id, v) */
static Embryo_Cell
_edje_embryo_fn_set_float(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   float v;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   v = EMBRYO_CELL_TO_FLOAT(params[2]);
   _edje_var_float_set(ed, (int)params[1], (double)v);
   return 0;
}

/* get_str(id, dst[], maxlen) */
static Embryo_Cell
_edje_embryo_fn_get_str(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *s;

   CHKPARAM(3);
   if (params[3] < 1) return 0;
   ed = embryo_program_data_get(ep);
   s = (char *)_edje_var_str_get(ed, (int)params[1]);
   if (s)
     {
        if ((int)strlen(s) < params[3])
          {
             SETSTR(s, params[2]);
          }
        else
          {
             char *ss;

             ss = alloca(strlen(s) + 1);
             strcpy(ss, s);
             ss[params[3] - 1] = 0;
             SETSTR(ss, params[2]);
          }
     }
   else
     {
        SETSTR("", params[2]);
     }
   return 0;
}

/* get_strlen(id) */
static Embryo_Cell
_edje_embryo_fn_get_strlen(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *s;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   s = (char *)_edje_var_str_get(ed, (int)params[1]);
   if (s)
     {
        return strlen(s);
     }
   return 0;
}

/* set_str(id, str[]) */
static Embryo_Cell
_edje_embryo_fn_set_str(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *s;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   GETSTR(s, params[2]);
   if (s)
     {
        _edje_var_str_set(ed, (int)params[1], s);
     }
   return 0;
}

/* count(id) */
static Embryo_Cell
_edje_embryo_fn_count(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(1);

   return (Embryo_Cell)_edje_var_list_count_get(ed, (int)params[1]);
}

/* remove(id, n) */
static Embryo_Cell
_edje_embryo_fn_remove(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(2);

   _edje_var_list_remove_nth(ed, (int)params[1], (int)params[2]);

   return 0;
}

/* append_int(id, var) */
static Embryo_Cell
_edje_embryo_fn_append_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(2);

   _edje_var_list_int_append(ed, (int)params[1], (int)params[2]);

   return 0;
}

/* prepend_int(id, var) */
static Embryo_Cell
_edje_embryo_fn_prepend_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(2);

   _edje_var_list_int_prepend(ed, (int)params[1], (int)params[2]);

   return 0;
}

/* insert_int(id, pos, var) */
static Embryo_Cell
_edje_embryo_fn_insert_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(3);

   _edje_var_list_int_insert(ed, (int)params[1], (int)params[2],
                             (int)params[3]);

   return 0;
}

/* replace_int(id, pos, var) */
static Embryo_Cell
_edje_embryo_fn_replace_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(3);

   _edje_var_list_nth_int_set(ed, (int)params[1], (int)params[2],
                              (int)params[3]);

   return 0;
}

/* fetch_int(id, pos) */
static Embryo_Cell
_edje_embryo_fn_fetch_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(2);

   return _edje_var_list_nth_int_get(ed, (int)params[1],
                                     (int)params[2]);
}

/* append_str(id, str[]) */
static Embryo_Cell
_edje_embryo_fn_append_str(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   char *s;

   CHKPARAM(2);

   GETSTR(s, params[2]);
   if (s)
     _edje_var_list_str_append(ed, (int)params[1], s);

   return 0;
}

/* prepend_str(id, str[]) */
static Embryo_Cell
_edje_embryo_fn_prepend_str(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   char *s;

   CHKPARAM(2);

   GETSTR(s, params[2]);
   if (s)
     _edje_var_list_str_prepend(ed, (int)params[1], s);

   return 0;
}

/* insert_str(id, pos, str[]) */
static Embryo_Cell
_edje_embryo_fn_insert_str(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   char *s;

   CHKPARAM(3);

   GETSTR(s, params[3]);
   if (s)
     _edje_var_list_str_insert(ed, (int)params[1], (int)params[2], s);

   return 0;
}

/* replace_str(id, pos, str[]) */
static Embryo_Cell
_edje_embryo_fn_replace_str(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   char *s;

   CHKPARAM(3);

   GETSTR(s, params[3]);
   if (s)
     _edje_var_list_nth_str_set(ed, (int)params[1], (int)params[2], s);

   return 0;
}

/* fetch_str(id, pos, dst[], maxlen) */
static Embryo_Cell
_edje_embryo_fn_fetch_str(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   char *s;

   CHKPARAM(4);

   s = (char *)_edje_var_list_nth_str_get(ed, (int)params[1],
                                          (int)params[2]);
   if (s)
     {
        if ((int)strlen(s) < params[4])
          {
             SETSTR(s, params[3]);
          }
        else
          {
             char *ss;

             ss = alloca(strlen(s) + 1);
             strcpy(ss, s);
             ss[params[4] - 1] = 0;
             SETSTR(ss, params[3]);
          }
     }
   else
     {
        SETSTR("", params[3]);
     }

   return 0;
}

/* append_float(id, Float:f) */
static Embryo_Cell
_edje_embryo_fn_append_float(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   float f;

   CHKPARAM(2);

   f = EMBRYO_CELL_TO_FLOAT(params[2]);
   _edje_var_list_float_append(ed, (int)params[1], f);

   return 0;
}

/* prepend_float(id, Float:f) */
static Embryo_Cell
_edje_embryo_fn_prepend_float(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   float f;

   CHKPARAM(2);

   f = EMBRYO_CELL_TO_FLOAT(params[2]);
   _edje_var_list_float_prepend(ed, (int)params[1], f);

   return 0;
}

/* insert_float(id, pos, Float:f) */
static Embryo_Cell
_edje_embryo_fn_insert_float(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   float f;

   CHKPARAM(3);

   f = EMBRYO_CELL_TO_FLOAT(params[3]);
   _edje_var_list_float_insert(ed, (int)params[1], (int)params[2], f);

   return 0;
}

/* replace_float(id, pos, Float:f) */
static Embryo_Cell
_edje_embryo_fn_replace_float(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(3);

   _edje_var_list_nth_float_set(ed, (int)params[1], (int)params[2],
                                EMBRYO_CELL_TO_FLOAT(params[3]));

   return 0;
}

/* Float:fetch_float(id, pos) */
static Embryo_Cell
_edje_embryo_fn_fetch_float(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   float f;

   CHKPARAM(2);

   f = _edje_var_list_nth_float_get(ed, (int)params[1], (int)params[2]);

   return EMBRYO_FLOAT_TO_CELL(f);
}

/* timer(Float:in, fname[], val) */
static Embryo_Cell
_edje_embryo_fn_timer(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *fname = NULL;
   float f;
   double in;
   int val;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   GETSTR(fname, params[2]);
   if ((!fname)) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   in = (double)f;
   val = params[3];
   return _edje_var_timer_add(ed, in, fname, val);
}

/* cancel_timer(id) */
static Embryo_Cell
_edje_embryo_fn_cancel_timer(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int id;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   id = params[1];
   if (id <= 0) return 0;
   _edje_var_timer_del(ed, id);
   return 0;
}

/* reset_timer(id) */
static Embryo_Cell
_edje_embryo_fn_reset_timer(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int id;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   id = params[1];
   if (id <= 0) return 0;
   _edje_var_timer_reset(ed, id);
   return 0;
}

/* anim(Float:len, fname[], val) */
static Embryo_Cell
_edje_embryo_fn_anim(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *fname = NULL;
   float f;
   double len;
   int val;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   GETSTR(fname, params[2]);
   if ((!fname)) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   len = (double)f;
   val = params[3];
   return _edje_var_anim_add(ed, len, fname, val);
}

/* cancel_anim(id) */
static Embryo_Cell
_edje_embryo_fn_cancel_anim(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int id;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   id = params[1];
   if (id <= 0) return 0;
   _edje_var_anim_del(ed, id);
   return 0;
}

/* get_anim_pos_map(Float:pos, Tween_Mode_Type:tween, Float:v1, Float:v2, &Float:ret) */
static Embryo_Cell
_edje_embryo_fn_get_anim_pos_map(Embryo_Program *ep, Embryo_Cell *params)
{
   double pos;
   Ecore_Pos_Map tween;
   double v1, v2;

   CHKPARAM(5);
   pos = EMBRYO_CELL_TO_FLOAT(params[1]);
   tween = params[2];
   v1 = EMBRYO_CELL_TO_FLOAT(params[3]);
   v2 = EMBRYO_CELL_TO_FLOAT(params[4]);

   switch (tween)
     {
      case ECORE_POS_MAP_LINEAR:
      case ECORE_POS_MAP_ACCELERATE:
      case ECORE_POS_MAP_DECELERATE:
      case ECORE_POS_MAP_SINUSOIDAL:
        pos = ecore_animator_pos_map(pos, tween, 0, 0);
        break;

      case ECORE_POS_MAP_ACCELERATE_FACTOR:
        pos = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE_FACTOR,
                                     v1, 0);
        break;

      case ECORE_POS_MAP_DECELERATE_FACTOR:
        pos = ecore_animator_pos_map(pos, ECORE_POS_MAP_DECELERATE_FACTOR,
                                     v1, 0);
        break;

      case ECORE_POS_MAP_SINUSOIDAL_FACTOR:
        pos = ecore_animator_pos_map(pos, ECORE_POS_MAP_SINUSOIDAL_FACTOR,
                                     v1, 0);
        break;

      case ECORE_POS_MAP_DIVISOR_INTERP:
      case ECORE_POS_MAP_BOUNCE:
      case ECORE_POS_MAP_SPRING:
        pos = ecore_animator_pos_map(pos, tween, v1, v2);
        break;

      default:
        break;
     }

   SETFLOAT(pos, params[5]);

   return 0;
}

/* set_min_size(Float:w, Float:h) */
static Embryo_Cell
_edje_embryo_fn_set_min_size(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   float f = 0.0;
   double w = 0.0, h = 0.0;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   w = (double)f;
   f = EMBRYO_CELL_TO_FLOAT(params[2]);
   h = (double)f;

   if (w < 0.0) w = 0.0;
   if (h < 0.0) h = 0.0;
   ed->collection->prop.min.w = w;
   ed->collection->prop.min.h = h;
   ed->recalc_call = EINA_TRUE;
   ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = EINA_TRUE;
#endif
   _edje_recalc(ed);
   return 0;
}

/* set_max_size(Float:w, Float:h) */
static Embryo_Cell
_edje_embryo_fn_set_max_size(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   float f = 0.0;
   double w = 0.0, h = 0.0;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   f = EMBRYO_CELL_TO_FLOAT(params[1]);
   w = (double)f;
   f = EMBRYO_CELL_TO_FLOAT(params[2]);
   h = (double)f;

   if (w < 0.0) w = 0.0;
   if (h < 0.0) h = 0.0;
   ed->collection->prop.max.w = w;
   ed->collection->prop.max.h = h;
   ed->recalc_call = EINA_TRUE;
   ed->dirty = EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = EINA_TRUE;
#endif
   _edje_recalc(ed);

   return 0;
}

/* stop_program(program_id) */
static Embryo_Cell
_edje_embryo_fn_stop_program(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int program_id = 0;
   Edje_Running_Program *runp;
   Edje_Pending_Program *pp;
   Eina_List *l, *ll;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   program_id = params[1];
   if (program_id < 0) return 0;

   ed->walking_actions = EINA_TRUE;

   EINA_LIST_FOREACH(ed->actions, l, runp)
     if (program_id == runp->program->id)
       _edje_program_end(ed, runp);
   EINA_LIST_FOREACH_SAFE(ed->pending_actions, l, ll, pp)
     if (program_id == pp->program->id)
       {
          ed->pending_actions = eina_list_remove_list(ed->pending_actions, l);
          ecore_timer_del(pp->timer);
          free(pp);
       }

   ed->walking_actions = EINA_FALSE;

   return 0;
}

/* stop_programs_on(part_id) */
static Embryo_Cell
_edje_embryo_fn_stop_programs_on(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;

   int part_id = 0;
   Edje_Real_Part *rp;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   if (rp)
     {
        Eina_List *l, *ll, *lll;
        Edje_Pending_Program *pp;
        Edje_Program_Target *pt;
        /* there is only ever 1 program acting on a part at any time */
        if (rp->program) _edje_program_end(ed, rp->program);
        EINA_LIST_FOREACH_SAFE(ed->pending_actions, l, ll, pp)
          {
             EINA_LIST_FOREACH(pp->program->targets, lll, pt)
               if (pt->id == part_id)
                 {
                    ed->pending_actions = eina_list_remove_list(ed->pending_actions, l);
                    ecore_timer_del(pp->timer);
                    free(pp);
                    break;
                 }
          }
     }
   return 0;
}

/* get_mouse(&x, &y) */
static Embryo_Cell
_edje_embryo_fn_get_mouse(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Evas_Coord x = 0, y = 0;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   evas_pointer_canvas_xy_get(ed->base.evas, &x, &y);
   x -= ed->x;
   y -= ed->y;
   SETINT((int)x, params[1]);
   SETINT((int)y, params[2]);
   return 0;
}

/* get_mouse_buttons() */
static Embryo_Cell
_edje_embryo_fn_get_mouse_buttons(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;

   CHKPARAM(0);
   ed = embryo_program_data_get(ep);
   return evas_pointer_button_down_mask_get(ed->base.evas);
}

/* emit(sig[], src[]) */
static Embryo_Cell
_edje_embryo_fn_emit(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *sig = NULL, *src = NULL;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   GETSTR(sig, params[1]);
   GETSTR(src, params[2]);
   if ((!sig) || (!src)) return 0;
   _edje_emit(ed, sig, src);
   return 0;
}

/* get_part_id(part[]) */
static Embryo_Cell
_edje_embryo_fn_get_part_id(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Edje_Part_Collection *col;
   Edje_Part **part;
   char *p;
   unsigned int i;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   GETSTR(p, params[1]);
   if (!p) return -1;
   col = ed->collection;
   if (!col) return -1;
   part = col->parts;
   for (i = 0; i < col->parts_count; i++, part++)
     {
        if (!(*part)->name) continue;
        if (!strcmp((*part)->name, p)) return (*part)->id;
     }
   return -1;
}

/* get_image_id(img[]) */
static Embryo_Cell
_edje_embryo_fn_get_image_id(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Edje_File *file;
   Edje_Image_Directory *dir;
   Edje_Image_Directory_Entry *dirent;
   char *p;
   unsigned int i;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   GETSTR(p, params[1]);
   if (!p) return -1;
   file = ed->file;
   if (!file) return -1;
   dir = file->image_dir;
   dirent = dir->entries;
   for (i = 0; i < dir->entries_count; i++, dirent++)
     {
        if (!dirent->entry) continue;
        if (!strcmp(dirent->entry, p)) return dirent->id;
     }
   return -1;
}

/* get_program_id(program[]) */
static Embryo_Cell
_edje_embryo_fn_get_program_id(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Edje_Program **prog;
   char *p;
   int i;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   GETSTR(p, params[1]);
   if (!p) return -1;
   prog = ed->collection->patterns.table_programs;
   if (!prog) return -1;
   for (i = 0; i < ed->collection->patterns.table_programs_size; i++, prog++)
     {
        if (!(*prog)->name) continue;
        if (!strcmp((*prog)->name, p)) return (*prog)->id;
     }
   return -1;
}

static Embryo_Cell
_edje_embryo_fn_play_sample(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *sample_name = NULL;
   float speed = 1.0;
   int channel = 0;

   if (params[0] < (int)(sizeof(Embryo_Cell) * 2))
     return 0;
   ed = embryo_program_data_get(ep);
   GETSTR(sample_name, params[1]);
   if ((!sample_name)) return 0;
   speed = EMBRYO_CELL_TO_FLOAT(params[2]);
   if (params[0] == (int)(sizeof(Embryo_Cell) * 3))
     GETINT(channel, params[3]);
   _edje_multisense_internal_sound_sample_play(ed, sample_name,
                                               (double)speed, channel);
   return 0;
}

static Embryo_Cell
_edje_embryo_fn_play_tone(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *tone_name = NULL;
   float duration = 0.1;
   int channel = 0;

   if (params[0] < (int)(sizeof(Embryo_Cell) * 2))
     return 0;
   ed = embryo_program_data_get(ep);
   GETSTR(tone_name, params[1]);
   if ((!tone_name)) return 0;
   duration = EMBRYO_CELL_TO_FLOAT(params[2]);
   if (params[0] == (int)(sizeof(Embryo_Cell) * 3))
     GETINT(channel, params[3]);
   _edje_multisense_internal_sound_tone_play(ed, tone_name,
                                             (double)duration, channel);
   return 0;
}

static Embryo_Cell
_edje_embryo_fn_play_vibration(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *sample_name = NULL;
   int repeat = 10;

   if (params[0] < (int)(sizeof(Embryo_Cell) * 2)) return 0;
   ed = embryo_program_data_get(ep);
   GETSTR(sample_name, params[1]);
   if ((!sample_name)) return 0;

   if (params[0] == (int)(sizeof(Embryo_Cell) * 2))
     GETINT(repeat, params[2]);

   _edje_multisense_internal_vibration_sample_play(ed, sample_name,
                                                   repeat);
   return 0;
}

/* set_state(part_id, state[], Float:state_val) */
static Embryo_Cell
_edje_embryo_fn_set_state(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *state = NULL;
   int part_id = 0;
   float f = 0.0;
   double value = 0.0;
   Edje_Real_Part *rp;

   if (!(HASNPARAMS(2) || HASNPARAMS(3))) return -1;

   ed = embryo_program_data_get(ep);
   GETSTR(state, params[2]);
   if ((!state)) return 0;
   part_id = params[1];
   if (part_id < 0) return 0;
   if (HASNPARAMS(3))
     {
        f = EMBRYO_CELL_TO_FLOAT(params[3]);
        value = (double)f;
     }
   else
     value = 0.0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   if (rp)
     {
        if (rp->program) _edje_program_end(ed, rp->program);
        _edje_part_description_apply(ed, rp, state, value, NULL, 0.0);
        _edje_part_pos_set(ed, rp, EDJE_TWEEN_MODE_LINEAR, ZERO, ZERO, ZERO,
                           ZERO, ZERO);
        _edje_recalc(ed);
     }
   return 0;
}

static Embryo_Cell
_edje_embryo_fn_set_state_anim(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *state = NULL;
   int part_id = 0;
   float f = 0.0;
   double value = 0.0;
   char *tmp = NULL;
   Edje_Real_Part *rp;
   int anim_type = 0;
   double tween = 0.0, v1 = 0.0, v2 = 0.0, v3 = 0.0, v4 = 0.0;

   if (HASNPARAMS(4)) return -1;

   ed = embryo_program_data_get(ep);
   GETSTR(state, params[2]);
   if ((!state)) return 0;
   part_id = params[1];
   if (part_id < 0) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[3]);
   value = (double)f;
   anim_type = params[4];
   f = EMBRYO_CELL_TO_FLOAT(params[5]);
   tween = (double)f;
   if ((anim_type >= EDJE_TWEEN_MODE_LINEAR) &&
       (anim_type <= EDJE_TWEEN_MODE_DECELERATE))
     {
        if (HASNPARAMS(6))
          {
             GETSTR(tmp, params[6]);
             if ((tmp) && (!strcmp(tmp, "CURRENT")))
               anim_type |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
     }
   else if ((anim_type >= EDJE_TWEEN_MODE_ACCELERATE_FACTOR) &&
            (anim_type <= EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR))
     {
        if (HASNPARAMS(7))
          {
             GETSTR(tmp, params[7]);
             if ((tmp) && (!strcmp(tmp, "CURRENT")))
               anim_type |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
        else if (HASNPARAMS(5))
          {
             ERR("parse error. Need 6th parameter to set factor");
             return -1;
          }
        GETFLOAT_T(v1, params[6]);
     }
   else if ((anim_type >= EDJE_TWEEN_MODE_DIVISOR_INTERP) &&
            (anim_type <= EDJE_TWEEN_MODE_SPRING))
     {
        if (HASNPARAMS(8))
          {
             GETSTR(tmp, params[8]);
             if ((tmp) && (!strcmp(tmp, "CURRENT")))
               anim_type |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
        else if (HASNPARAMS(5))
          {
             ERR("parse error.Need 6th and 7th parameters to set factor and counts");
             return -1;
          }
        GETFLOAT_T(v1, params[6]);
        GETFLOAT_T(v2, params[7]);
     }
   else if (anim_type == EDJE_TWEEN_MODE_CUBIC_BEZIER)
     {
        if (HASNPARAMS(12))
          {
             GETSTR(tmp, params[8]);
             if ((tmp) && (!strcmp(tmp, "CURRENT")))
               anim_type |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
        else if (HASNPARAMS(5))
          {
             ERR("parse error.Need 6th, 7th, 9th and 10th parameters to set x1, y1, x2 and y2");
             return -1;
          }
        if (HASNPARAMS(10))
          {
             GETFLOAT_T(v1, params[6]);
             GETFLOAT_T(v2, params[7]);
             GETFLOAT_T(v3, params[9]);
             GETFLOAT_T(v4, params[10]);
          }
        else
          {
             GETFLOAT_T(v1, params[6]);
             GETFLOAT_T(v2, params[7]);
             GETFLOAT_T(v3, params[8]);
             GETFLOAT_T(v4, params[9]);
          }
     }
   rp = ed->table_parts[part_id % ed->table_parts_size];
   if (!rp) return 0;
   _edje_part_description_apply(ed, rp, NULL, 0.0, state, value);
   _edje_part_pos_set(ed, rp, anim_type, FROM_DOUBLE(tween), v1, v2,
                      v3, v4);
   _edje_recalc(ed);
   return 0;
}

/* get_state(part_id, dst[], maxlen, &Float:val) */
static Embryo_Cell
_edje_embryo_fn_get_state(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;
   const char *s;

   CHKPARAM(4);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   if (rp->chosen_description)
     {
        SETFLOAT(rp->chosen_description->state.value, params[4]);
        s = rp->chosen_description->state.name;
        if (s)
          {
             if ((int)strlen(s) < params[3])
               {
                  SETSTR(s, params[2]);
               }
             else
               {
                  char *ss;

                  ss = alloca(strlen(s) + 1);
                  strcpy(ss, s);
                  ss[params[3] - 1] = 0;
                  SETSTR(ss, params[2]);
               }
          }
        else
          {
             SETSTR("", params[2]);
          }
     }
   else
     {
        SETFLOAT(0.0, params[4]);
        SETSTR("", params[2]);
     }
   return 0;
}

/* set_tween_state(part_id, Float:tween, state1[], Float:state1_val, state2[], Float:state2_val) */
static Embryo_Cell
_edje_embryo_fn_set_tween_state(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *state1 = NULL, *state2 = NULL;
   int part_id = 0;
   float f = 0.0;
   double tween = 0.0, value1 = 0.0, value2 = 0.0;
   Edje_Real_Part *rp;

   CHKPARAM(6);
   ed = embryo_program_data_get(ep);
   GETSTR(state1, params[3]);
   GETSTR(state2, params[5]);
   if ((!state1) || (!state2)) return 0;
   part_id = params[1];
   if (part_id < 0) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[2]);
   tween = (double)f;
   f = EMBRYO_CELL_TO_FLOAT(params[4]);
   value1 = (double)f;
   f = EMBRYO_CELL_TO_FLOAT(params[6]);
   value2 = (double)f;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   if (rp)
     {
        if (rp->program) _edje_program_end(ed, rp->program);
        _edje_part_description_apply(ed, rp, state1, value1, state2, value2);
        _edje_part_pos_set(ed, rp, EDJE_TWEEN_MODE_LINEAR, FROM_DOUBLE(tween),
                           ZERO, ZERO, ZERO, ZERO);
        _edje_recalc(ed);
     }
   return 0;
}

static Embryo_Cell
_edje_embryo_fn_set_tween_state_anim(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *tmp = NULL;
   char *state1 = NULL, *state2 = NULL;
   int part_id = 0;
   int anim_type = 0;
   float f = 0.0;
   double tween = 0.0, value1 = 0.0, value2 = 0.0, v1 = 0.0, v2 = 0.0, v3 = 0.0, v4 = 0.0;
   Edje_Real_Part *rp;

   if (HASNPARAMS(6)) return -1;
   ed = embryo_program_data_get(ep);
   GETSTR(state1, params[2]);
   GETSTR(state2, params[4]);
   if ((!state1) || (!state2)) return 0;
   part_id = params[1];
   anim_type = params[6];
   if (part_id < 0) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[3]);
   value1 = (double)f;
   f = EMBRYO_CELL_TO_FLOAT(params[5]);
   value2 = (double)f;
   f = EMBRYO_CELL_TO_FLOAT(params[7]);
   tween = (double)f;
   if ((anim_type >= EDJE_TWEEN_MODE_LINEAR) &&
       (anim_type <= EDJE_TWEEN_MODE_DECELERATE))
     {
        if (HASNPARAMS(8))
          {
             GETSTR(tmp, params[8]);
             if (!tmp) return 0;
             if (!strcmp(tmp, "CURRENT"))
               anim_type |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
     }
   else if ((anim_type >= EDJE_TWEEN_MODE_ACCELERATE_FACTOR) &&
            (anim_type <= EDJE_TWEEN_MODE_SINUSOIDAL_FACTOR))
     {
        if (HASNPARAMS(9))
          {
             GETSTR(tmp, params[9]);
             if (!tmp) return 0;
             if (!strcmp(tmp, "CURRENT"))
               anim_type |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
        else if (HASNPARAMS(7))
          {
             ERR("parse error. Need 8th parameter to set factor");
             return -1;
          }
        GETFLOAT_T(v1, params[8]);
     }
   else if ((anim_type >= EDJE_TWEEN_MODE_DIVISOR_INTERP) &&
            (anim_type <= EDJE_TWEEN_MODE_SPRING))
     {
        if (HASNPARAMS(10))
          {
             GETSTR(tmp, params[10]);
             if (!tmp) return 0;
             if (!strcmp(tmp, "CURRENT"))
               anim_type |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
        else if (HASNPARAMS(7))
          {
             ERR("parse error.Need 8th and 9th parameters to set factor and counts");
             return -1;
          }
        GETFLOAT_T(v1, params[8]);
        GETFLOAT_T(v2, params[9]);
     }
   else if (anim_type == EDJE_TWEEN_MODE_CUBIC_BEZIER)
     {
        if (HASNPARAMS(12))
          {
             GETSTR(tmp, params[10]);
             if (!tmp) return 0;
             if (!strcmp(tmp, "CURRENT"))
               anim_type |= EDJE_TWEEN_MODE_OPT_FROM_CURRENT;
          }
        else if (HASNPARAMS(7))
          {
             ERR("parse error.Need 8th, 9th, 10th and 11th parameters to set x1, y1, x2 and y2");
             return -1;
          }
        if (HASNPARAMS(12))
          {
             GETFLOAT_T(v1, params[8]);
             GETFLOAT_T(v2, params[9]);
             GETFLOAT_T(v3, params[11]);
             GETFLOAT_T(v4, params[12]);
          }
        else
          {
             GETFLOAT_T(v1, params[8]);
             GETFLOAT_T(v2, params[9]);
             GETFLOAT_T(v3, params[10]);
             GETFLOAT_T(v4, params[11]);
          }
     }
   rp = ed->table_parts[part_id % ed->table_parts_size];
   if (!rp) return 0;

   if (rp->program) _edje_program_end(ed, rp->program);
   _edje_part_description_apply(ed, rp, state1, value1, state2, value2);
   _edje_part_pos_set(ed, rp, anim_type, FROM_DOUBLE(tween),
                      v1, v2, v3, v4);
   _edje_recalc(ed);
   return 0;
}

/* run_program(program_id) */
static Embryo_Cell
_edje_embryo_fn_run_program(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int program_id = 0;
   Edje_Program *pr;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   program_id = params[1];
   if (program_id < 0) return 0;
   pr = ed->collection->patterns.table_programs[program_id % ed->collection->patterns.table_programs_size];
   if (pr)
     {
        _edje_program_run(ed, pr, 0, "", "", NULL);
     }
   return 0;
}

/* get_drag_dir(part_id) */
static Embryo_Cell
_edje_embryo_fn_get_drag_dir(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   return edje_object_part_drag_dir_get(ed->obj, rp->part->name);
}

/* get_drag(part_id, &Float:dx, &Float:dy) */
static Embryo_Cell
_edje_embryo_fn_get_drag(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;
   double dx = 0.0, dy = 0.0;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   edje_object_part_drag_value_get(ed->obj, rp->part->name, &dx, &dy);
   SETFLOAT(dx, params[2]);
   SETFLOAT(dy, params[3]);

   return 0;
}

/* set_drag(part_id, Float:dx, Float:dy) */
static Embryo_Cell
_edje_embryo_fn_set_drag(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   edje_object_part_drag_value_set(ed->obj, rp->part->name,
                                   (double)EMBRYO_CELL_TO_FLOAT(params[2]),
                                   (double)EMBRYO_CELL_TO_FLOAT(params[3]));
   return 0;
}

/* get_drag_size(part_id, &Float:dx, &Float:dy) */
static Embryo_Cell
_edje_embryo_fn_get_drag_size(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;
   double dx = 0.0, dy = 0.0;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   edje_object_part_drag_size_get(ed->obj, rp->part->name, &dx, &dy);
   SETFLOAT(dx, params[2]);
   SETFLOAT(dy, params[3]);

   return 0;
}

/* set_drag_size(part_id, Float:dx, Float:dy) */
static Embryo_Cell
_edje_embryo_fn_set_drag_size(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   edje_object_part_drag_size_set(ed->obj, rp->part->name,
                                  (double)EMBRYO_CELL_TO_FLOAT(params[2]),
                                  (double)EMBRYO_CELL_TO_FLOAT(params[3]));
   return 0;
}

/* set_text(part_id, str[]) */
static Embryo_Cell
_edje_embryo_fn_set_text(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;
   char *s;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   GETSTR(s, params[2]);
   if (s)
     {
        edje_object_part_text_set(ed->obj, rp->part->name, s);
     }
   return 0;
}

/* get_text(part_id, dst[], maxlen) */
static Embryo_Cell
_edje_embryo_fn_get_text(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;
   char *s;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   s = (char *)edje_object_part_text_get(ed->obj, rp->part->name);
   if (s)
     {
        if ((int)strlen(s) < params[3])
          {
             SETSTR(s, params[2]);
          }
        else
          {
             char *ss;

             ss = alloca(strlen(s) + 1);
             strcpy(ss, s);
             ss[params[3] - 1] = 0;
             SETSTR(ss, params[2]);
          }
     }
   else
     {
        SETSTR("", params[2]);
     }
   return 0;
}

/* get_min_size(&w, &h) */
static Embryo_Cell
_edje_embryo_fn_get_min_size(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Evas_Coord w = 0, h = 0;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   edje_object_size_min_get(ed->obj, &w, &h);
   SETINT(w, params[1]);
   SETINT(h, params[2]);
   return 0;
}

/* get_max_size(&w, &h) */
static Embryo_Cell
_edje_embryo_fn_get_max_size(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Evas_Coord w = 0, h = 0;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   edje_object_size_max_get(ed->obj, &w, &h);
   SETINT(w, params[1]);
   SETINT(h, params[2]);
   return 0;
}

/* get_color_class(class[], &r, &g, &b, &a) */
static Embryo_Cell
_edje_embryo_fn_get_color_class(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Edje_Color_Class *c_class;
   char *class;

   CHKPARAM(5);
   ed = embryo_program_data_get(ep);
   GETSTR(class, params[1]);
   if (!class) return 0;
   c_class = _edje_color_class_find(ed, class);
   if (!c_class) return 0;
   SETINT(c_class->r, params[2]);
   SETINT(c_class->g, params[3]);
   SETINT(c_class->b, params[4]);
   SETINT(c_class->a, params[5]);
   return 0;
}

/* set_color_class(class[], r, g, b, a) */
static Embryo_Cell
_edje_embryo_fn_set_color_class(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *class;

   CHKPARAM(5);
   ed = embryo_program_data_get(ep);
   GETSTR(class, params[1]);
   if (!class) return 0;
   edje_object_color_class_set(ed->obj, class, params[2], params[3], params[4],
                               params[5], params[2], params[3], params[4],
                               params[5], params[2], params[3], params[4],
                               params[5]);
   return 0;
}

/* set_text_class(class[], font[], Float:size) */
static Embryo_Cell
_edje_embryo_fn_set_text_class(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *class, *font;
   Evas_Font_Size fsize;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   GETSTR(class, params[1]);
   GETSTR(font, params[2]);
   if ( !class || !font ) return 0;
   fsize = (Evas_Font_Size)EMBRYO_CELL_TO_FLOAT(params[3]);
   edje_object_text_class_set(ed->obj, class, font, fsize);
   return 0;
}

/* get_text_class(class[], font[], &Float:size) */
static Embryo_Cell
_edje_embryo_fn_get_text_class(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *class;
   Edje_Text_Class *t_class;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   GETSTR(class, params[1]);
   if (!class) return 0;
   t_class = _edje_text_class_find(ed, class);
   if (!t_class) return 0;
   SETSTR((char *)t_class->font, params[2]);
   SETFLOAT(t_class->size, params[3]);
   return 0;
}

/* get_drag_step(part_id, &Float:dx, &Float:&dy) */
static Embryo_Cell
_edje_embryo_fn_get_drag_step(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;
   double dx = 0.0, dy = 0.0;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   edje_object_part_drag_step_get(ed->obj, rp->part->name, &dx, &dy);
   SETFLOAT(dx, params[2]);
   SETFLOAT(dy, params[3]);

   return 0;
}

/* set_drag_step(part_id, Float:dx, Float:dy) */
static Embryo_Cell
_edje_embryo_fn_set_drag_step(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   edje_object_part_drag_step_set(ed->obj, rp->part->name,
                                  (double)EMBRYO_CELL_TO_FLOAT(params[2]),
                                  (double)EMBRYO_CELL_TO_FLOAT(params[3]));
   return 0;
}

/* get_drag_page(part_id, &Float:dx, &Float:&dy) */
static Embryo_Cell
_edje_embryo_fn_get_drag_page(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;
   double dx = 0.0, dy = 0.0;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   edje_object_part_drag_page_get(ed->obj, rp->part->name, &dx, &dy);
   SETFLOAT(dx, params[2]);
   SETFLOAT(dy, params[3]);

   return 0;
}

/* get_geometry(pard_id, &x, &y, &w, &h) */
static Embryo_Cell
_edje_embryo_fn_get_geometry(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;
   Evas_Coord x = 0, y = 0, w = 0, h = 0;

   CHKPARAM(5);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   edje_object_part_geometry_get(ed->obj, rp->part->name, &x, &y, &w, &h);
   SETINT(x, params[2]);
   SETINT(y, params[3]);
   SETINT(w, params[4]);
   SETINT(h, params[5]);

   return 0;
}

/* set_drag_page(part_id, Float:dx, Float:dy) */
static Embryo_Cell
_edje_embryo_fn_set_drag_page(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   edje_object_part_drag_page_set(ed->obj, rp->part->name,
                                  (double)EMBRYO_CELL_TO_FLOAT(params[2]),
                                  (double)EMBRYO_CELL_TO_FLOAT(params[3]));
   return 0;
}

/* send_message(Msg_Type:type, id,...); */
static Embryo_Cell
_edje_embryo_fn_send_message(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Edje_Message_Type type;
   int id, i, n;
   Embryo_Cell *ptr;

   if (params[0] < (int)(sizeof(Embryo_Cell) * (2))) return 0;
   ed = embryo_program_data_get(ep);
   type = params[1];
   id = params[2];
   switch (type)
     {
      case EDJE_MESSAGE_NONE:
        _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, NULL);
        break;

      case EDJE_MESSAGE_SIGNAL:
        break;

      case EDJE_MESSAGE_STRING:
      {
         Embryo_Cell *cptr;

         cptr = embryo_data_address_get(ep, params[3]);
         if (cptr)
           {
              Edje_Message_String *emsg;
              int l;
              char *s;

              l = embryo_data_string_length_get(ep, cptr);
              s = alloca(l + 1);
              s[0] = 0;
              embryo_data_string_get(ep, cptr, s);
              emsg = alloca(sizeof(Edje_Message_String));
              emsg->str = s;
              _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
           }
      }
      break;

      case EDJE_MESSAGE_INT:
      {
         Edje_Message_Int *emsg;

         emsg = alloca(sizeof(Edje_Message_Int));
         ptr = embryo_data_address_get(ep, params[3]);
         if (ptr) emsg->val = (int)*ptr;
         else emsg->val = 0;
         _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
      }
      break;

      case EDJE_MESSAGE_FLOAT:
      {
         Edje_Message_Float *emsg;
         float f;

         emsg = alloca(sizeof(Edje_Message_Float));
         ptr = embryo_data_address_get(ep, params[3]);
         if (ptr)
           {
              f = EMBRYO_CELL_TO_FLOAT(*ptr);
              emsg->val = (double)f;
           }
         else
           emsg->val = 0.0;
         _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
      }
      break;

      case EDJE_MESSAGE_STRING_SET:
      {
         Edje_Message_String_Set *emsg;

         n = (params[0] / sizeof(Embryo_Cell)) + 1;
         emsg = alloca(sizeof(Edje_Message_String_Set) + ((n - 3 - 1) * sizeof(char *)));
         emsg->count = n - 3;
         for (i = 3; i < n; i++)
           {
              Embryo_Cell *cptr;

              cptr = embryo_data_address_get(ep, params[i]);
              if (cptr)
                {
                   int l;
                   char *s;

                   l = embryo_data_string_length_get(ep, cptr);
                   s = alloca(l + 1);
                   s[0] = 0;
                   embryo_data_string_get(ep, cptr, s);
                   emsg->str[i - 3] = s;
                }
           }
         _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
      }
      break;

      case EDJE_MESSAGE_INT_SET:
      {
         Edje_Message_Int_Set *emsg;

         n = (params[0] / sizeof(Embryo_Cell)) + 1;
         emsg = alloca(sizeof(Edje_Message_Int_Set) + ((n - 3 - 1) * sizeof(int)));
         emsg->count = n - 3;
         for (i = 3; i < n; i++)
           {
              ptr = embryo_data_address_get(ep, params[i]);
              if (ptr) emsg->val[i - 3] = (int)*ptr;
              else emsg->val[i - 3] = 0;
           }
         _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
      }
      break;

      case EDJE_MESSAGE_FLOAT_SET:
      {
         Edje_Message_Float_Set *emsg;

         n = (params[0] / sizeof(Embryo_Cell)) + 1;
         emsg = alloca(sizeof(Edje_Message_Float_Set) + ((n - 3 - 1) * sizeof(double)));
         emsg->count = n - 3;
         for (i = 3; i < n; i++)
           {
              float f;

              ptr = embryo_data_address_get(ep, params[i]);
              if (ptr)
                {
                   f = EMBRYO_CELL_TO_FLOAT(*ptr);
                   emsg->val[i - 3] = (double)f;
                }
              else
                emsg->val[i - 3] = 0.0;
           }
         _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
      }
      break;

      case EDJE_MESSAGE_STRING_INT:
      {
         Edje_Message_String_Int *emsg;
         Embryo_Cell *cptr;

         cptr = embryo_data_address_get(ep, params[3]);
         if (cptr)
           {
              int l;
              char *s;

              l = embryo_data_string_length_get(ep, cptr);
              s = alloca(l + 1);
              s[0] = 0;
              embryo_data_string_get(ep, cptr, s);
              emsg = alloca(sizeof(Edje_Message_String_Int));
              emsg->str = s;
              ptr = embryo_data_address_get(ep, params[4]);
              if (ptr) emsg->val = (int)*ptr;
              else emsg->val = 0;
              _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
           }
      }
      break;

      case EDJE_MESSAGE_STRING_FLOAT:
      {
         Edje_Message_String_Float *emsg;
         Embryo_Cell *cptr;

         cptr = embryo_data_address_get(ep, params[3]);
         if (cptr)
           {
              int l;
              char *s;
              float f;

              l = embryo_data_string_length_get(ep, cptr);
              s = alloca(l + 1);
              s[0] = 0;
              embryo_data_string_get(ep, cptr, s);
              emsg = alloca(sizeof(Edje_Message_String_Float));
              emsg->str = s;
              ptr = embryo_data_address_get(ep, params[4]);
              if (ptr)
                {
                   f = EMBRYO_CELL_TO_FLOAT(*ptr);
                   emsg->val = (double)f;
                }
              else
                emsg->val = 0.0;
              _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
           }
      }
      break;

      case EDJE_MESSAGE_STRING_INT_SET:
      {
         Edje_Message_String_Int_Set *emsg;
         Embryo_Cell *cptr;

         cptr = embryo_data_address_get(ep, params[3]);
         if (cptr)
           {
              int l;
              char *s;

              l = embryo_data_string_length_get(ep, cptr);
              s = alloca(l + 1);
              s[0] = 0;
              embryo_data_string_get(ep, cptr, s);
              n = (params[0] / sizeof(Embryo_Cell)) + 1;
              emsg = alloca(sizeof(Edje_Message_String_Int_Set) + ((n - 4 - 1) * sizeof(int)));
              emsg->str = s;
              emsg->count = n - 4;
              for (i = 4; i < n; i++)
                {
                   ptr = embryo_data_address_get(ep, params[i]);
                   if (ptr) emsg->val[i - 4] = (int)*ptr;
                   else emsg->val[i - 4] = 0;
                }
              _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
           }
      }
      break;

      case EDJE_MESSAGE_STRING_FLOAT_SET:
      {
         Edje_Message_String_Float_Set *emsg;
         Embryo_Cell *cptr;

         cptr = embryo_data_address_get(ep, params[3]);
         if (cptr)
           {
              int l;
              char *s;

              l = embryo_data_string_length_get(ep, cptr);
              s = alloca(l + 1);
              s[0] = 0;
              embryo_data_string_get(ep, cptr, s);
              n = (params[0] / sizeof(Embryo_Cell)) + 1;
              emsg = alloca(sizeof(Edje_Message_String_Float_Set) + ((n - 4 - 1) * sizeof(double)));
              emsg->str = s;
              emsg->count = n - 4;
              for (i = 4; i < n; i++)
                {
                   float f;

                   ptr = embryo_data_address_get(ep, params[i]);
                   if (ptr)
                     {
                        f = EMBRYO_CELL_TO_FLOAT(*ptr);
                        emsg->val[i - 4] = (double)f;
                     }
                   else
                     emsg->val[i - 4] = 0.0;
                }
              _edje_util_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
           }
      }
      break;

      default:
        break;
     }
   return 0;
}

/* custom_state(part_id, state[], Float:state_val = 0.0) */
static Embryo_Cell
_edje_embryo_fn_custom_state(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Edje_Real_Part *rp;
   Edje_Part_Description_Common *parent, *d = NULL;
   char *name;
   float val;

   CHKPARAM(3);

   if (params[1] < 0)
     return 0;

   if (!(rp = ed->table_parts[params[1] % ed->table_parts_size]))
     return 0;

   /* check whether this part already has a "custom" state */
   if (rp->custom)
     return 0;

   GETSTR(name, params[2]);
   if (!name)
     return 0;

   val = EMBRYO_CELL_TO_FLOAT(params[3]);

   if (!(parent = _edje_part_description_find(ed, rp, name, val, EINA_TRUE)))
     return 0;

   /* now create the custom state */
   switch (rp->part->type)
     {
#define ALLOC_DESC(Short, Type, To) \
case EDJE_PART_TYPE_##Short: To = calloc(1, sizeof (Edje_Part_Description_##Type)); break;

#define ALLOC_COPY_DESC(Short, Type, To, Spec)             \
case EDJE_PART_TYPE_##Short:                               \
{                                                          \
   Edje_Part_Description_##Type * tmp;                     \
   Edje_Part_Description_##Type * new;                     \
   tmp = (Edje_Part_Description_##Type *)parent;           \
   new = calloc(1, sizeof (Edje_Part_Description_##Type)); \
   new->Spec = tmp->Spec;                                  \
   d = &new->common;                                       \
   break;                                                  \
}

        ALLOC_DESC(RECTANGLE, Common, d);
        ALLOC_DESC(SPACER, Common, d);
        ALLOC_DESC(SWALLOW, Common, d);
        ALLOC_DESC(GROUP, Common, d);

        ALLOC_COPY_DESC(IMAGE, Image, d, image);
        ALLOC_COPY_DESC(PROXY, Proxy, d, proxy);
        ALLOC_COPY_DESC(TEXT, Text, d, text);
        ALLOC_COPY_DESC(TEXTBLOCK, Text, d, text);
        ALLOC_COPY_DESC(BOX, Box, d, box);
        ALLOC_COPY_DESC(TABLE, Table, d, table);
        ALLOC_COPY_DESC(EXTERNAL, External, d, external_params);
        ALLOC_COPY_DESC(VECTOR, Vector, d, vg);
     }

   if (!d) return 0;

   rp->custom = eina_mempool_malloc(_edje_real_part_state_mp, sizeof (Edje_Real_Part_State));
   if (!rp->custom)
     {
        free(d);
        return 0;
     }

   memset(rp->custom, 0, sizeof (Edje_Real_Part_State));

   *d = *parent;

   d->state.name = (char *)eina_stringshare_add("custom");
   d->state.value = 0.0;

   /* make sure all the allocated memory is getting copied,
    * not just referenced
    */
   if (rp->part->type == EDJE_PART_TYPE_IMAGE)
     {
        Edje_Part_Description_Image *img_desc;
        Edje_Part_Description_Image *parent_img_desc;

        img_desc = (Edje_Part_Description_Image *)d;
        parent_img_desc = (Edje_Part_Description_Image *)parent;

        img_desc->image.tweens_count = parent_img_desc->image.tweens_count;
        img_desc->image.tweens = calloc(img_desc->image.tweens_count,
                                        sizeof(Edje_Part_Image_Id *));
        if (img_desc->image.tweens)
          {
             unsigned int i;

             for (i = 0; i < parent_img_desc->image.tweens_count; ++i)
               {
                  Edje_Part_Image_Id *iid_new;

                  iid_new = calloc(1, sizeof(Edje_Part_Image_Id));
                  if (!iid_new) continue;

                  *iid_new = *parent_img_desc->image.tweens[i];

                  img_desc->image.tweens[i] = iid_new;
               }
          }
     }

#define DUP(x) x ? (char *)eina_stringshare_add(x) : NULL
   d->color_class = DUP(d->color_class);

   if (rp->part->type == EDJE_PART_TYPE_TEXT
       || rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
     {
        Edje_Part_Description_Text *text_desc;

        text_desc = (Edje_Part_Description_Text *)d;

        text_desc->text.text_class = DUP(text_desc->text.text_class);
        text_desc->text.text.str = DUP(edje_string_get(&text_desc->text.text));
        text_desc->text.text.id = 0;
        text_desc->text.text.translated = NULL;
        text_desc->text.domain = DUP(text_desc->text.domain);
        text_desc->text.font.str = DUP(edje_string_get(&text_desc->text.font));
        text_desc->text.font.id = 0;
        text_desc->text.style.str = DUP(edje_string_get(&text_desc->text.style));
        text_desc->text.style.id = 0;
     }
#undef DUP

   rp->custom->description = d;

   return 0;
}

/* set_state_val(part_id, State_Param:p, ...) */
static Embryo_Cell
_edje_embryo_fn_set_state_val(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Edje_Real_Part *rp;
   char *s;

   /* we need at least 3 arguments */
   if (params[0] < (int)(sizeof(Embryo_Cell) * 3))
     return 0;

   if (params[1] < 0)
     return 0;

   if (!(rp = ed->table_parts[params[1] % ed->table_parts_size]))
     return 0;

   /* check whether this part has a "custom" state */
   if (!rp->custom)
     return 0;

   switch (params[2])
     {
      case EDJE_STATE_PARAM_ALIGNMENT:
        CHKPARAM(4);

        GETFLOAT_T(rp->custom->description->align.x, params[3]);
        GETFLOAT_T(rp->custom->description->align.y, params[4]);

        break;

      case EDJE_STATE_PARAM_MIN:
        CHKPARAM(4);

        GETINT(rp->custom->description->min.w, params[3]);
        GETINT(rp->custom->description->min.h, params[4]);

        break;

      case EDJE_STATE_PARAM_MAX:
        CHKPARAM(4);

        GETINT(rp->custom->description->max.w, params[3]);
        GETINT(rp->custom->description->max.h, params[4]);

        break;

      case EDJE_STATE_PARAM_STEP:
        CHKPARAM(4);

        GETINT(rp->custom->description->step.x, params[3]);
        GETINT(rp->custom->description->step.y, params[4]);

        break;

      case EDJE_STATE_PARAM_ASPECT:
        CHKPARAM(4);

        GETFLOAT_T(rp->custom->description->aspect.min, params[3]);
        GETFLOAT_T(rp->custom->description->aspect.max, params[4]);

        break;

      case EDJE_STATE_PARAM_ASPECT_PREF:
        CHKPARAM(3);

        GETINT(rp->custom->description->aspect.prefer, params[3]);

        break;

      case EDJE_STATE_PARAM_COLOR:
        CHKPARAM(6);

        GETINT(rp->custom->description->color.r, params[3]);
        GETINT(rp->custom->description->color.g, params[4]);
        GETINT(rp->custom->description->color.b, params[5]);
        GETINT(rp->custom->description->color.a, params[6]);

        break;

      case EDJE_STATE_PARAM_COLOR2:
        CHKPARAM(6);

        GETINT(rp->custom->description->color2.r, params[3]);
        GETINT(rp->custom->description->color2.g, params[4]);
        GETINT(rp->custom->description->color2.b, params[5]);
        GETINT(rp->custom->description->color2.a, params[6]);

        break;

      case EDJE_STATE_PARAM_COLOR3:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
             (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
           return 0;

         CHKPARAM(6);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         GETINT(text->text.color3.r, params[3]);
         GETINT(text->text.color3.g, params[4]);
         GETINT(text->text.color3.b, params[5]);
         GETINT(text->text.color3.a, params[6]);
         break;
      }

      case EDJE_STATE_PARAM_COLOR_CLASS:
        CHKPARAM(3);

        GETSTR(s, params[3]);
        GETSTREVAS(s, rp->custom->description->color_class);

        break;

      case EDJE_STATE_PARAM_REL1:
        CHKPARAM(4);

        GETFLOAT_T(rp->custom->description->rel1.relative_x, params[3]);
        GETFLOAT_T(rp->custom->description->rel1.relative_y, params[4]);

        break;

      case EDJE_STATE_PARAM_REL1_TO:
        CHKPARAM(4);

        GETINT(rp->custom->description->rel1.id_x, params[3]);
        GETINT(rp->custom->description->rel1.id_y, params[4]);

        break;

      case EDJE_STATE_PARAM_REL1_OFFSET:
        CHKPARAM(4);

        GETINT(rp->custom->description->rel1.offset_x, params[3]);
        GETINT(rp->custom->description->rel1.offset_y, params[4]);

        break;

      case EDJE_STATE_PARAM_REL2:
        CHKPARAM(4);

        GETFLOAT_T(rp->custom->description->rel2.relative_x, params[3]);
        GETFLOAT_T(rp->custom->description->rel2.relative_y, params[4]);

        break;

      case EDJE_STATE_PARAM_REL2_TO:
        CHKPARAM(4);

        GETINT(rp->custom->description->rel2.id_x, params[3]);
        GETINT(rp->custom->description->rel2.id_y, params[4]);

        break;

      case EDJE_STATE_PARAM_REL2_OFFSET:
        CHKPARAM(4);

        GETINT(rp->custom->description->rel2.offset_x, params[3]);
        GETINT(rp->custom->description->rel2.offset_y, params[4]);

        break;

      case EDJE_STATE_PARAM_IMAGE:
      {
         Edje_Part_Description_Image *img;

         if ((rp->part->type != EDJE_PART_TYPE_IMAGE)) return 0;
         CHKPARAM(3);

         img = (Edje_Part_Description_Image *)rp->custom->description;
         GETINT(img->image.id, params[3]);

         break;
      }

      case EDJE_STATE_PARAM_BORDER:
      {
         Edje_Part_Description_Image *img;

         if ((rp->part->type != EDJE_PART_TYPE_IMAGE)) return 0;
         CHKPARAM(6);

         img = (Edje_Part_Description_Image *)rp->custom->description;

         GETINT(img->image.border.l, params[3]);
         GETINT(img->image.border.r, params[4]);
         GETINT(img->image.border.t, params[5]);
         GETINT(img->image.border.b, params[6]);

         break;
      }

      case EDJE_STATE_PARAM_FILL_SMOOTH:
      {
         Edje_Part_Description_Image *img;
         Edje_Part_Description_Proxy *proxy;

         if ((rp->part->type != EDJE_PART_TYPE_IMAGE) && (rp->part->type != EDJE_PART_TYPE_PROXY)) return 0;
         CHKPARAM(3);

         if (rp->part->type == EDJE_PART_TYPE_IMAGE)
           {
              img = (Edje_Part_Description_Image *)rp->custom->description;
              GETINT(img->image.fill.smooth, params[3]);
           }
         else
           {
              proxy = (Edje_Part_Description_Proxy *)rp->custom->description;
              GETINT(proxy->proxy.fill.smooth, params[3]);
           }

         break;
      }

      case EDJE_STATE_PARAM_FILL_POS:
      {
         Edje_Part_Description_Image *img;
         Edje_Part_Description_Proxy *proxy;

         if ((rp->part->type != EDJE_PART_TYPE_IMAGE) && (rp->part->type != EDJE_PART_TYPE_PROXY)) return 0;
         CHKPARAM(6);

         if (rp->part->type == EDJE_PART_TYPE_IMAGE)
           {
              img = (Edje_Part_Description_Image *)rp->custom->description;

              GETFLOAT_T(img->image.fill.pos_rel_x, params[3]);
              GETFLOAT_T(img->image.fill.pos_rel_y, params[4]);
              GETINT(img->image.fill.pos_abs_x, params[5]);
              GETINT(img->image.fill.pos_abs_y, params[6]);
           }
         else
           {
              proxy = (Edje_Part_Description_Proxy *)rp->custom->description;

              GETFLOAT_T(proxy->proxy.fill.pos_rel_x, params[3]);
              GETFLOAT_T(proxy->proxy.fill.pos_rel_y, params[4]);
              GETINT(proxy->proxy.fill.pos_abs_x, params[5]);
              GETINT(proxy->proxy.fill.pos_abs_y, params[6]);
           }

         break;
      }

      case EDJE_STATE_PARAM_FILL_SIZE:
      {
         Edje_Part_Description_Image *img;
         Edje_Part_Description_Proxy *proxy;

         if ((rp->part->type != EDJE_PART_TYPE_IMAGE) && (rp->part->type != EDJE_PART_TYPE_PROXY)) return 0;
         CHKPARAM(6);

         if (rp->part->type == EDJE_PART_TYPE_IMAGE)
           {
              img = (Edje_Part_Description_Image *)rp->custom->description;

              GETFLOAT_T(img->image.fill.rel_x, params[3]);
              GETFLOAT_T(img->image.fill.rel_y, params[4]);
              GETINT(img->image.fill.abs_x, params[5]);
              GETINT(img->image.fill.abs_y, params[6]);
           }
         else
           {
              proxy = (Edje_Part_Description_Proxy *)rp->custom->description;

              GETFLOAT_T(proxy->proxy.fill.rel_x, params[3]);
              GETFLOAT_T(proxy->proxy.fill.rel_y, params[4]);
              GETINT(proxy->proxy.fill.abs_x, params[5]);
              GETINT(proxy->proxy.fill.abs_y, params[6]);
           }

         break;
      }

      case EDJE_STATE_PARAM_TEXT:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
             (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
           return 0;
         CHKPARAM(3);

         GETSTR(s, params[3]);

         text = (Edje_Part_Description_Text *)rp->custom->description;
         GETSTREVAS(s, text->text.text.str);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_CLASS:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
             (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
           return 0;
         CHKPARAM(3);

         GETSTR(s, params[3]);

         text = (Edje_Part_Description_Text *)rp->custom->description;
         GETSTREVAS(s, text->text.text_class);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_FONT:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
         CHKPARAM(3);

         GETSTR(s, params[3]);

         text = (Edje_Part_Description_Text *)rp->custom->description;
         GETSTREVAS(s, text->text.font.str);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_STYLE:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return 0;
         CHKPARAM(3);

         GETSTR(s, params[3]);

         text = (Edje_Part_Description_Text *)rp->custom->description;
         GETSTREVAS(s, text->text.style.str);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_SIZE:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
         CHKPARAM(3);

         text = (Edje_Part_Description_Text *)rp->custom->description;
         GETINT(text->text.size, params[3]);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_FIT:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         GETINT(text->text.fit_x, params[3]);
         GETINT(text->text.fit_y, params[4]);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_MIN:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
             (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
           return 0;
         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         GETINT(text->text.min_x, params[3]);
         GETINT(text->text.min_y, params[4]);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_MAX:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
             (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
           return 0;
         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         GETINT(text->text.max_x, params[3]);
         GETINT(text->text.max_y, params[4]);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_ALIGN:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         GETFLOAT_T(text->text.align.x, params[3]);
         GETFLOAT_T(text->text.align.y, params[4]);

         break;
      }

      case EDJE_STATE_PARAM_VISIBLE:
        CHKPARAM(3);

        GETINT(rp->custom->description->visible, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_ON:
        CHKPARAM(3);

        GETINT(rp->custom->description->map.on, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_PERSP:
        CHKPARAM(3);

        GETINT(rp->custom->description->map.id_persp, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_LIGHT:
        CHKPARAM(3);

        GETINT(rp->custom->description->map.id_light, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_ROT_CENTER:
        CHKPARAM(3);

        GETINT(rp->custom->description->map.rot.id_center, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_ROT_X:
        CHKPARAM(3);

        GETFLOAT_T(rp->custom->description->map.rot.x, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_ROT_Y:
        CHKPARAM(3);

        GETFLOAT_T(rp->custom->description->map.rot.y, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_ROT_Z:
        CHKPARAM(3);

        GETFLOAT_T(rp->custom->description->map.rot.z, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_BACK_CULL:
        CHKPARAM(3);

        GETINT(rp->custom->description->map.backcull, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_PERSP_ON:
        CHKPARAM(3);

        GETINT(rp->custom->description->map.persp_on, params[3]);

        break;

      case EDJE_STATE_PARAM_PERSP_ZPLANE:
        CHKPARAM(3);

        GETINT(rp->custom->description->persp.zplane, params[3]);

        break;

      case EDJE_STATE_PARAM_PERSP_FOCAL:
        CHKPARAM(3);

        GETINT(rp->custom->description->persp.focal, params[3]);

        break;

      case EDJE_STATE_PARAM_PROXY_SRC_CLIP:
      {
         Edje_Part_Description_Proxy *proxy;

         if ((rp->part->type != EDJE_PART_TYPE_PROXY)) return 0;
         CHKPARAM(3);

         proxy = (Edje_Part_Description_Proxy *)rp->custom->description;
         GETINT(proxy->proxy.source_clip, params[3]);

         break;
      }

      case EDJE_STATE_PARAM_PROXY_SRC_VISIBLE:
      {
         Edje_Part_Description_Proxy *proxy;

         if ((rp->part->type != EDJE_PART_TYPE_PROXY)) return 0;
         CHKPARAM(3);

         proxy = (Edje_Part_Description_Proxy *)rp->custom->description;
         GETINT(proxy->proxy.source_visible, params[3]);

         break;
      }

      case EDJE_STATE_PARAM_BOX_LAYOUT:
      {
         Edje_Part_Description_Box *box;
         if ((rp->part->type != EDJE_PART_TYPE_BOX)) return 0;
         CHKPARAM(3);

         GETSTR(s, params[3]);
         s = strdup(s);

         box = (Edje_Part_Description_Box *)rp->custom->description;
         box->box.layout = s;

         break;
      }

      case EDJE_STATE_PARAM_BOX_FALLBACK_LAYOUT:
      {
         Edje_Part_Description_Box *box;
         if ((rp->part->type != EDJE_PART_TYPE_BOX)) return 0;
         CHKPARAM(3);

         GETSTR(s, params[3]);
         s = strdup(s);

         box = (Edje_Part_Description_Box *)rp->custom->description;
         box->box.alt_layout = s;

         break;
      }

      case EDJE_STATE_PARAM_BOX_ALIGN:
      {
         Edje_Part_Description_Box *box;
         if ((rp->part->type != EDJE_PART_TYPE_BOX)) return 0;
         CHKPARAM(4);

         box = (Edje_Part_Description_Box *)rp->custom->description;
         GETFLOAT_T(box->box.align.x, params[3]);
         GETFLOAT_T(box->box.align.y, params[4]);

         break;
      }

      case EDJE_STATE_PARAM_BOX_PADDING:
      {
         Edje_Part_Description_Box *box;
         if ((rp->part->type != EDJE_PART_TYPE_BOX)) return 0;
         CHKPARAM(4);

         box = (Edje_Part_Description_Box *)rp->custom->description;
         GETINT(box->box.padding.x, params[3]);
         GETINT(box->box.padding.y, params[4]);

         break;
      }

      case EDJE_STATE_PARAM_BOX_MIN:
      {
         Edje_Part_Description_Box *box;
         if ((rp->part->type != EDJE_PART_TYPE_BOX)) return 0;
         CHKPARAM(4);

         box = (Edje_Part_Description_Box *)rp->custom->description;
         GETINT(box->box.min.h, params[3]);
         GETINT(box->box.min.v, params[4]);

         break;
      }

#ifdef HAVE_EPHYSICS
      case EDJE_STATE_PARAM_PHYSICS_MASS:
        CHKPARAM(3);

        GETFLOAT_T(rp->custom->description->physics.mass, params[3]);

        break;

      case EDJE_STATE_PARAM_PHYSICS_RESTITUTION:
        CHKPARAM(3);

        GETFLOAT_T(rp->custom->description->physics.restitution, params[3]);

        break;

      case EDJE_STATE_PARAM_PHYSICS_FRICTION:
        CHKPARAM(3);

        GETFLOAT_T(rp->custom->description->physics.friction, params[3]);

        break;

      case EDJE_STATE_PARAM_PHYSICS_DAMPING:
        CHKPARAM(4);

        GETFLOAT_T(rp->custom->description->physics.damping.linear, params[3]);
        GETFLOAT_T(rp->custom->description->physics.damping.angular,
                   params[4]);

        break;

      case EDJE_STATE_PARAM_PHYSICS_SLEEP:
        CHKPARAM(4);

        GETFLOAT_T(rp->custom->description->physics.sleep.linear, params[3]);
        GETFLOAT_T(rp->custom->description->physics.sleep.angular, params[4]);

        break;

      case EDJE_STATE_PARAM_PHYSICS_MATERIAL:
        CHKPARAM(3);

        GETINT(rp->custom->description->physics.material, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_DENSITY:
        CHKPARAM(3);

        GETFLOAT_T(rp->custom->description->physics.density, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_HARDNESS:
        CHKPARAM(3);

        GETFLOAT_T(rp->custom->description->physics.hardness, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_IGNORE_PART_POS:
        CHKPARAM(3);

        GETINT(rp->custom->description->physics.ignore_part_pos, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_LIGHT_ON:
        CHKPARAM(3);

        GETINT(rp->custom->description->physics.light_on, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_MOV_FREEDOM_LIN:
        CHKPARAM(5);

        GETINT(rp->custom->description->physics.mov_freedom.lin.x, params[3]);
        GETINT(rp->custom->description->physics.mov_freedom.lin.y, params[4]);
        GETINT(rp->custom->description->physics.mov_freedom.lin.z, params[5]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_MOV_FREEDOM_ANG:
        CHKPARAM(5);

        GETINT(rp->custom->description->physics.mov_freedom.ang.x, params[3]);
        GETINT(rp->custom->description->physics.mov_freedom.ang.y, params[4]);
        GETINT(rp->custom->description->physics.mov_freedom.ang.z, params[5]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_BACK_CULL:
        CHKPARAM(3);

        GETINT(rp->custom->description->physics.backcull, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_Z:
        CHKPARAM(3);

        GETINT(rp->custom->description->physics.z, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_DEPTH:
        CHKPARAM(3);

        GETINT(rp->custom->description->physics.depth, params[3]);
        break;

#endif
      default:
        break;
     }

#ifdef EDJE_CALC_CACHE
   rp->invalidate = EINA_TRUE;
#endif
   ed->dirty = EINA_TRUE;
   return 0;
}

/* get_state_val(part_id, State_Param:p, ...) */
static Embryo_Cell
_edje_embryo_fn_get_state_val(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Edje_Real_Part *rp;
   const char *s;

   /* we need at least 3 arguments */
   if (params[0] < (int)(sizeof(Embryo_Cell) * 3))
     return 0;

   if (params[1] < 0)
     return 0;

   if (!(rp = ed->table_parts[params[1] % ed->table_parts_size]))
     return 0;

   /* check whether this part has a "custom" state */
   if (!rp->custom)
     return 0;

   switch (params[2])
     {
      case EDJE_STATE_PARAM_ALIGNMENT:
        CHKPARAM(4);

        SETFLOAT_T(rp->custom->description->align.x, params[3]);
        SETFLOAT_T(rp->custom->description->align.y, params[4]);

        break;

      case EDJE_STATE_PARAM_MIN:
        CHKPARAM(4);

        SETINT(rp->custom->description->min.w, params[3]);
        SETINT(rp->custom->description->min.h, params[4]);

        break;

      case EDJE_STATE_PARAM_MAX:
        CHKPARAM(4);

        SETINT(rp->custom->description->max.w, params[3]);
        SETINT(rp->custom->description->max.h, params[4]);

        break;

      case EDJE_STATE_PARAM_STEP:
        CHKPARAM(4);

        SETINT(rp->custom->description->step.x, params[3]);
        SETINT(rp->custom->description->step.y, params[4]);

        break;

      case EDJE_STATE_PARAM_ASPECT:
        CHKPARAM(4);

        SETFLOAT_T(rp->custom->description->aspect.min, params[3]);
        SETFLOAT_T(rp->custom->description->aspect.max, params[4]);

        break;

      case EDJE_STATE_PARAM_ASPECT_PREF:
        CHKPARAM(3);

        SETINT(rp->custom->description->aspect.prefer, params[3]);

        break;

      case EDJE_STATE_PARAM_COLOR:
        CHKPARAM(6);

        SETINT(rp->custom->description->color.r, params[3]);
        SETINT(rp->custom->description->color.g, params[4]);
        SETINT(rp->custom->description->color.b, params[5]);
        SETINT(rp->custom->description->color.a, params[6]);

        break;

      case EDJE_STATE_PARAM_COLOR2:
        CHKPARAM(6);

        SETINT(rp->custom->description->color2.r, params[3]);
        SETINT(rp->custom->description->color2.g, params[4]);
        SETINT(rp->custom->description->color2.b, params[5]);
        SETINT(rp->custom->description->color2.a, params[6]);

        break;

      case EDJE_STATE_PARAM_COLOR3:
      {
         Edje_Part_Description_Text *text;

         if (rp->part->type == EDJE_PART_TYPE_TEXT
             || rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
           return 0;

         CHKPARAM(6);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         SETINT(text->text.color3.r, params[3]);
         SETINT(text->text.color3.g, params[4]);
         SETINT(text->text.color3.b, params[5]);
         SETINT(text->text.color3.a, params[6]);

         break;
      }

      case EDJE_STATE_PARAM_COLOR_CLASS:
        CHKPARAM(4);

        s = rp->custom->description->color_class;
        SETSTRALLOCATE(s);

        break;

      case EDJE_STATE_PARAM_REL1:
        CHKPARAM(4);

        SETFLOAT_T(rp->custom->description->rel1.relative_x, params[3]);
        SETFLOAT_T(rp->custom->description->rel1.relative_y, params[4]);

        break;

      case EDJE_STATE_PARAM_REL1_TO:
        CHKPARAM(4);

        SETINT(rp->custom->description->rel1.id_x, params[3]);
        SETINT(rp->custom->description->rel1.id_y, params[4]);

        break;

      case EDJE_STATE_PARAM_REL1_OFFSET:
        CHKPARAM(4);

        SETINT(rp->custom->description->rel1.offset_x, params[3]);
        SETINT(rp->custom->description->rel1.offset_y, params[4]);

        break;

      case EDJE_STATE_PARAM_REL2:
        CHKPARAM(4);

        SETFLOAT_T(rp->custom->description->rel2.relative_x, params[3]);
        SETFLOAT_T(rp->custom->description->rel2.relative_y, params[4]);

        break;

      case EDJE_STATE_PARAM_REL2_TO:
        CHKPARAM(4);

        SETINT(rp->custom->description->rel2.id_x, params[3]);
        SETINT(rp->custom->description->rel2.id_y, params[4]);

        break;

      case EDJE_STATE_PARAM_REL2_OFFSET:
        CHKPARAM(4);

        SETINT(rp->custom->description->rel2.offset_x, params[3]);
        SETINT(rp->custom->description->rel2.offset_y, params[4]);

        break;

      case EDJE_STATE_PARAM_IMAGE:
      {
         Edje_Part_Description_Image *img;

         if ((rp->part->type != EDJE_PART_TYPE_IMAGE)) return 0;
         CHKPARAM(3);

         img = (Edje_Part_Description_Image *)rp->custom->description;

         SETINT(img->image.id, params[3]);

         break;
      }

      case EDJE_STATE_PARAM_BORDER:
      {
         Edje_Part_Description_Image *img;

         if ((rp->part->type != EDJE_PART_TYPE_IMAGE)) return 0;
         CHKPARAM(6);

         img = (Edje_Part_Description_Image *)rp->custom->description;

         SETINT(img->image.border.l, params[3]);
         SETINT(img->image.border.r, params[4]);
         SETINT(img->image.border.t, params[5]);
         SETINT(img->image.border.b, params[6]);

         break;
      }

      case EDJE_STATE_PARAM_FILL_SMOOTH:
      {
         Edje_Part_Description_Image *img;
         Edje_Part_Description_Proxy *proxy;

         if ((rp->part->type != EDJE_PART_TYPE_IMAGE) && (rp->part->type != EDJE_PART_TYPE_PROXY)) return 0;
         CHKPARAM(3);

         if (rp->part->type == EDJE_PART_TYPE_IMAGE)
           {
              img = (Edje_Part_Description_Image *)rp->custom->description;

              SETINT(img->image.fill.smooth, params[3]);
           }
         else
           {
              proxy = (Edje_Part_Description_Proxy *)rp->custom->description;

              SETINT(proxy->proxy.fill.smooth, params[3]);
           }

         break;
      }

      case EDJE_STATE_PARAM_FILL_POS:
      {
         Edje_Part_Description_Image *img;
         Edje_Part_Description_Proxy *proxy;

         if ((rp->part->type != EDJE_PART_TYPE_IMAGE) && (rp->part->type != EDJE_PART_TYPE_PROXY)) return 0;
         CHKPARAM(6);

         if (rp->part->type == EDJE_PART_TYPE_IMAGE)
           {
              img = (Edje_Part_Description_Image *)rp->custom->description;

              SETFLOAT_T(img->image.fill.pos_rel_x, params[3]);
              SETFLOAT_T(img->image.fill.pos_rel_y, params[4]);
              SETINT(img->image.fill.pos_abs_x, params[5]);
              SETINT(img->image.fill.pos_abs_y, params[6]);
           }
         else
           {
              proxy = (Edje_Part_Description_Proxy *)rp->custom->description;

              SETFLOAT_T(proxy->proxy.fill.pos_rel_x, params[3]);
              SETFLOAT_T(proxy->proxy.fill.pos_rel_y, params[4]);
              SETINT(proxy->proxy.fill.pos_abs_x, params[5]);
              SETINT(proxy->proxy.fill.pos_abs_y, params[6]);
           }

         break;
      }

      case EDJE_STATE_PARAM_FILL_SIZE:
      {
         Edje_Part_Description_Image *img;
         Edje_Part_Description_Proxy *proxy;

         if ((rp->part->type != EDJE_PART_TYPE_IMAGE) && (rp->part->type != EDJE_PART_TYPE_PROXY)) return 0;
         CHKPARAM(6);

         if (rp->part->type == EDJE_PART_TYPE_IMAGE)
           {
              img = (Edje_Part_Description_Image *)rp->custom->description;

              SETFLOAT_T(img->image.fill.rel_x, params[3]);
              SETFLOAT_T(img->image.fill.rel_y, params[4]);
              SETINT(img->image.fill.abs_x, params[5]);
              SETINT(img->image.fill.abs_y, params[6]);
           }
         else
           {
              proxy = (Edje_Part_Description_Proxy *)rp->custom->description;

              SETFLOAT_T(proxy->proxy.fill.rel_x, params[3]);
              SETFLOAT_T(proxy->proxy.fill.rel_y, params[4]);
              SETINT(proxy->proxy.fill.abs_x, params[5]);
              SETINT(proxy->proxy.fill.abs_y, params[6]);
           }

         break;
      }

      case EDJE_STATE_PARAM_TEXT:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
             (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
           return 0;

         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         s = (char *)text->text.text.str;
         SETSTRALLOCATE(s);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_CLASS:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
             (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
           return 0;

         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         s = text->text.text_class;
         SETSTRALLOCATE(s);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_FONT:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;

         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         s = (char *)text->text.font.str;
         SETSTRALLOCATE(s);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_STYLE:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return 0;

         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         s = (char *)text->text.style.str;
         SETSTRALLOCATE(s);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_SIZE:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;

         CHKPARAM(3);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         SETINT(text->text.size, params[3]);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_FIT:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         SETINT(text->text.fit_x, params[3]);
         SETINT(text->text.fit_y, params[4]);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_MIN:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
             (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
           return 0;

         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         SETINT(text->text.min_x, params[3]);
         SETINT(text->text.min_y, params[4]);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_MAX:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
             (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
           return 0;

         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         SETINT(text->text.max_x, params[3]);
         SETINT(text->text.max_y, params[4]);

         break;
      }

      case EDJE_STATE_PARAM_TEXT_ALIGN:
      {
         Edje_Part_Description_Text *text;

         if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;

         CHKPARAM(4);

         text = (Edje_Part_Description_Text *)rp->custom->description;

         SETFLOAT_T(text->text.align.x, params[3]);
         SETFLOAT_T(text->text.align.y, params[4]);

         break;
      }

      case EDJE_STATE_PARAM_VISIBLE:
        CHKPARAM(3);

        SETINT(rp->custom->description->visible, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_ON:
        CHKPARAM(3);

        SETINT(rp->custom->description->map.on, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_PERSP:
        CHKPARAM(3);

        SETINT(rp->custom->description->map.id_persp, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_LIGHT:
        CHKPARAM(3);

        SETINT(rp->custom->description->map.id_light, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_ROT_CENTER:
        CHKPARAM(3);

        SETINT(rp->custom->description->map.rot.id_center, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_ROT_X:
        CHKPARAM(3);

        SETFLOAT_T(rp->custom->description->map.rot.x, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_ROT_Y:
        CHKPARAM(3);

        SETFLOAT_T(rp->custom->description->map.rot.y, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_ROT_Z:
        CHKPARAM(3);

        SETFLOAT_T(rp->custom->description->map.rot.z, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_BACK_CULL:
        CHKPARAM(3);

        SETINT(rp->custom->description->map.backcull, params[3]);

        break;

      case EDJE_STATE_PARAM_MAP_PERSP_ON:
        CHKPARAM(3);

        SETINT(rp->custom->description->map.persp_on, params[3]);

        break;

      case EDJE_STATE_PARAM_PERSP_ZPLANE:
        CHKPARAM(3);

        SETINT(rp->custom->description->persp.zplane, params[3]);

        break;

      case EDJE_STATE_PARAM_PERSP_FOCAL:
        CHKPARAM(3);

        SETINT(rp->custom->description->persp.focal, params[3]);

        break;

      case EDJE_STATE_PARAM_PROXY_SRC_CLIP:
      {
         Edje_Part_Description_Proxy *proxy;

         if ((rp->part->type != EDJE_PART_TYPE_PROXY)) return 0;
         CHKPARAM(3);

         proxy = (Edje_Part_Description_Proxy *)rp->custom->description;
         SETINT(proxy->proxy.source_clip, params[3]);

         break;
      }

      case EDJE_STATE_PARAM_PROXY_SRC_VISIBLE:
      {
         Edje_Part_Description_Proxy *proxy;

         if ((rp->part->type != EDJE_PART_TYPE_PROXY)) return 0;
         CHKPARAM(3);

         proxy = (Edje_Part_Description_Proxy *)rp->custom->description;
         SETINT(proxy->proxy.source_visible, params[3]);

         break;
      }

#ifdef HAVE_EPHYSICS
      case EDJE_STATE_PARAM_PHYSICS_MASS:
        CHKPARAM(3);

        SETFLOAT_T(rp->custom->description->physics.mass, params[3]);

        break;

      case EDJE_STATE_PARAM_PHYSICS_RESTITUTION:
        CHKPARAM(3);

        SETFLOAT_T(rp->custom->description->physics.restitution, params[3]);

        break;

      case EDJE_STATE_PARAM_PHYSICS_FRICTION:
        CHKPARAM(3);

        SETFLOAT_T(rp->custom->description->physics.friction, params[3]);

        break;

      case EDJE_STATE_PARAM_PHYSICS_DAMPING:
        CHKPARAM(4);

        SETFLOAT_T(rp->custom->description->physics.damping.linear, params[3]);
        SETFLOAT_T(rp->custom->description->physics.damping.angular,
                   params[4]);

        break;

      case EDJE_STATE_PARAM_PHYSICS_SLEEP:
        CHKPARAM(4);

        SETFLOAT_T(rp->custom->description->physics.sleep.linear, params[3]);
        SETFLOAT_T(rp->custom->description->physics.sleep.angular, params[4]);

        break;

      case EDJE_STATE_PARAM_PHYSICS_MATERIAL:
        CHKPARAM(3);

        SETINT(rp->custom->description->physics.material, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_DENSITY:
        CHKPARAM(3);

        SETFLOAT_T(rp->custom->description->physics.density, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_HARDNESS:
        CHKPARAM(3);

        SETFLOAT_T(rp->custom->description->physics.hardness, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_IGNORE_PART_POS:
        CHKPARAM(3);

        SETINT(rp->custom->description->physics.ignore_part_pos, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_LIGHT_ON:
        CHKPARAM(3);

        SETINT(rp->custom->description->physics.light_on, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_MOV_FREEDOM_LIN:
        CHKPARAM(5);

        SETINT(rp->custom->description->physics.mov_freedom.lin.x, params[3]);
        SETINT(rp->custom->description->physics.mov_freedom.lin.y, params[4]);
        SETINT(rp->custom->description->physics.mov_freedom.lin.z, params[5]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_MOV_FREEDOM_ANG:
        CHKPARAM(5);

        SETINT(rp->custom->description->physics.mov_freedom.ang.x, params[3]);
        SETINT(rp->custom->description->physics.mov_freedom.ang.y, params[4]);
        SETINT(rp->custom->description->physics.mov_freedom.ang.z, params[5]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_BACK_CULL:
        CHKPARAM(3);

        SETINT(rp->custom->description->physics.backcull, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_Z:
        CHKPARAM(3);

        SETINT(rp->custom->description->physics.z, params[3]);
        break;

      case EDJE_STATE_PARAM_PHYSICS_DEPTH:
        CHKPARAM(3);

        SETINT(rp->custom->description->physics.depth, params[3]);
        break;

#endif
      default:
        break;
     }

   return 0;
}

/* set_mouse_events(part_id, ev) */
static Embryo_Cell
_edje_embryo_fn_set_mouse_events(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   Edje *ed;
   Edje_Real_Part *rp;

   CHKPARAM(2);

   part_id = params[1];
   if (part_id < 0) return 0;

   ed = embryo_program_data_get(ep);
   rp = ed->table_parts[part_id % ed->table_parts_size];

   if (rp)
     _edje_real_part_mouse_events_set(ed, rp, params[2]);

   return 0;
}

/* get_mouse_events(part_id) */
static Embryo_Cell
_edje_embryo_fn_get_mouse_events(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   Edje *ed;
   Edje_Real_Part *rp;

   CHKPARAM(1);

   part_id = params[1];
   if (part_id < 0) return 0;

   ed = embryo_program_data_get(ep);
   rp = ed->table_parts[part_id % ed->table_parts_size];

   if (rp)
     return (Embryo_Cell)_edje_var_int_get(ed, (int)_edje_real_part_mouse_events_get(ed, rp));

   return 0;

}

/* set_pointer_mode(part_id, mode) */
static Embryo_Cell
_edje_embryo_fn_set_pointer_mode(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   Edje *ed;
   Edje_Real_Part *rp;

   CHKPARAM(2);

   part_id = params[1];
   if (part_id < 0) return 0;

   ed = embryo_program_data_get(ep);
   rp = ed->table_parts[part_id % ed->table_parts_size];

   if (rp)
     _edje_real_part_pointer_mode_set(ed, rp, params[2]);

   return 0;
}

/* set_repeat_events(part_id, rep) */
static Embryo_Cell
_edje_embryo_fn_set_repeat_events(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   Edje *ed;
   Edje_Real_Part *rp;

   CHKPARAM(2);

   part_id = params[1];
   if (part_id < 0) return 0;

   ed = embryo_program_data_get(ep);
   rp = ed->table_parts[part_id % ed->table_parts_size];

   if (rp)
     _edje_real_part_repeat_events_set(ed, rp, params[2]);

   return 0;
}

/* get_repeat_events(part_id) */
static Embryo_Cell
_edje_embryo_fn_get_repeat_events(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   Edje *ed;
   Edje_Real_Part *rp;

   CHKPARAM(1);

   part_id = params[1];
   if (part_id < 0) return 0;

   ed = embryo_program_data_get(ep);
   rp = ed->table_parts[part_id % ed->table_parts_size];

   if (rp)
     return (Embryo_Cell)_edje_var_int_get(ed, (int)_edje_real_part_repeat_events_get(ed, rp));

   return 0;

}

/* set_ignore_flags(part_id, flags) */
static Embryo_Cell
_edje_embryo_fn_set_ignore_flags(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   Edje *ed;
   Edje_Real_Part *rp;

   CHKPARAM(2);

   part_id = params[1];
   if (part_id < 0) return 0;

   ed = embryo_program_data_get(ep);
   rp = ed->table_parts[part_id % ed->table_parts_size];

   if (rp)
     _edje_real_part_ignore_flags_set(ed, rp, params[2]);

   return 0;
}

/* get_ignore_flags(part_id) */
static Embryo_Cell
_edje_embryo_fn_get_ignore_flags(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   Edje *ed;
   Edje_Real_Part *rp;

   CHKPARAM(1);

   part_id = params[1];
   if (part_id < 0) return 0;

   ed = embryo_program_data_get(ep);
   rp = ed->table_parts[part_id % ed->table_parts_size];

   if (rp)
     return (Embryo_Cell)_edje_var_int_get(ed, (int)_edje_real_part_ignore_flags_get(ed, rp));

   return 0;

}

/* set_mask_flags(part_id, flags) */
static Embryo_Cell
_edje_embryo_fn_set_mask_flags(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   Edje *ed;
   Edje_Real_Part *rp;

   CHKPARAM(2);

   part_id = params[1];
   if (part_id < 0) return 0;

   ed = embryo_program_data_get(ep);
   rp = ed->table_parts[part_id % ed->table_parts_size];

   if (rp)
     _edje_real_part_mask_flags_set(ed, rp, params[2]);

   return 0;
}

/* get_mask_flags(part_id) */
static Embryo_Cell
_edje_embryo_fn_get_mask_flags(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   Edje *ed;
   Edje_Real_Part *rp;

   CHKPARAM(1);

   part_id = params[1];
   if (part_id < 0) return 0;

   ed = embryo_program_data_get(ep);
   rp = ed->table_parts[part_id % ed->table_parts_size];

   if (rp)
     return (Embryo_Cell)_edje_var_int_get(ed, (int)_edje_real_part_mask_flags_get(ed, rp));

   return 0;

}

/* part_swallow(part_id, group_name) */
static Embryo_Cell
_edje_embryo_fn_part_swallow(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   char *group_name = 0;
   Edje *ed;
   Edje_Real_Part *rp;
   Evas_Object *new_obj;

   CHKPARAM(2);

   part_id = params[1];
   if (part_id < 0) return 0;

   GETSTR(group_name, params[2]);
   if (!group_name) return 0;

   ed = embryo_program_data_get(ep);

   rp = ed->table_parts[part_id % ed->table_parts_size];
   if (!rp) return 0;

   new_obj = edje_object_add(ed->base.evas);
   if (!new_obj) return 0;

   if (!edje_object_file_set(new_obj, ed->file->path, group_name))
     {
        evas_object_del(new_obj);
        return 0;
     }
   edje_object_part_swallow(ed->obj, rp->part->name, new_obj);
   _edje_subobj_register(ed, new_obj);

   return 0;
}

/* set_focus(part_id, seat_name[]) */
static Embryo_Cell
_edje_embryo_fn_set_focus(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   char *seat_name = NULL;

   if (!(HASNPARAMS(1) || HASNPARAMS(2))) return -1;
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   if (!rp) return 0;

   /* if no seat name is passed, that's fine. it means
      it should be applied to default seat */
   if (HASNPARAMS(2))
     {
        GETSTR(seat_name, params[2]);
        if (!seat_name) return 0;
     }

   _edje_part_focus_set(ed, seat_name, rp);

   return 0;
}

/* unset_focus(seat_name[]) */
static Embryo_Cell
_edje_embryo_fn_unset_focus(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *seat_name = NULL;

   if (!(HASNPARAMS(0) || HASNPARAMS(1))) return -1;
   ed = embryo_program_data_get(ep);

   /* seat name is optional. no seat means
      it should be applied to default seat */
   if (HASNPARAMS(1))
     {
        GETSTR(seat_name, params[1]);
        if (!seat_name) return 0;
     }

   _edje_part_focus_set(ed, seat_name, NULL);

   return 0;
}

/* external_param_get_int(id, param_name[]) */
static Embryo_Cell
_edje_embryo_fn_external_param_get_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_INT;
   eep.i = 0;
   _edje_external_param_get(NULL, rp, &eep);
   return eep.i;
}

/* external_param_set_int(id, param_name[], val) */
static Embryo_Cell
_edje_embryo_fn_external_param_set_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_INT;
   eep.i = params[3];
   return _edje_external_param_set(NULL, rp, &eep);
}

/* Float:external_param_get_float(id, param_name[]) */
static Embryo_Cell
_edje_embryo_fn_external_param_get_float(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name;
   float v;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
   eep.d = 0.0;
   _edje_external_param_get(NULL, rp, &eep);
   v = eep.d;
   return EMBRYO_FLOAT_TO_CELL(v);
}

/* external_param_set_float(id, param_name[], Float:val) */
static Embryo_Cell
_edje_embryo_fn_external_param_set_float(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_DOUBLE;
   eep.d = EMBRYO_CELL_TO_FLOAT(params[3]);
   return _edje_external_param_set(NULL, rp, &eep);
}

/* external_param_get_strlen(id, param_name[]) */
static Embryo_Cell
_edje_embryo_fn_external_param_get_strlen(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_STRING;
   eep.s = NULL;
   _edje_external_param_get(NULL, rp, &eep);
   if (!eep.s) return 0;
   return strlen(eep.s);
}

/* external_param_get_str(id, param_name[], val[], val_maxlen) */
static Embryo_Cell
_edje_embryo_fn_external_param_get_str(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name;
   size_t src_len, dst_len;

   CHKPARAM(4);
   dst_len = params[4];
   if (dst_len < 1) goto error;

   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) goto error;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_STRING;
   eep.s = NULL;
   _edje_external_param_get(NULL, rp, &eep);
   if (!eep.s) goto error;
   src_len = strlen(eep.s);
   if (src_len < dst_len)
     {
        SETSTR(eep.s, params[3]);
     }
   else
     {
        char *tmp = alloca(dst_len);
        memcpy(tmp, eep.s, dst_len - 1);
        tmp[dst_len - 1] = '\0';
        SETSTR(tmp, params[3]);
     }
   return 1;

error:
   SETSTR("", params[3]);
   return 0;
}

/* external_param_set_str(id, param_name[], val[]) */
static Embryo_Cell
_edje_embryo_fn_external_param_set_str(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name, *val;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_STRING;
   GETSTR(val, params[3]);
   if (!val) return 0;
   eep.s = val;
   return _edje_external_param_set(NULL, rp, &eep);
}

/* external_param_get_choice_len(id, param_name[]) */
static Embryo_Cell
_edje_embryo_fn_external_param_get_choice_len(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_CHOICE;
   eep.s = NULL;
   _edje_external_param_get(NULL, rp, &eep);
   if (!eep.s) return 0;
   return strlen(eep.s);
}

/* external_param_get_choice(id, param_name[], val[], val_maxlen) */
static Embryo_Cell
_edje_embryo_fn_external_param_get_choice(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name;
   size_t src_len, dst_len;

   CHKPARAM(4);
   dst_len = params[4];
   if (dst_len < 1) goto error;

   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) goto error;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_CHOICE;
   eep.s = NULL;
   _edje_external_param_get(NULL, rp, &eep);
   if (!eep.s) goto error;
   src_len = strlen(eep.s);
   if (src_len < dst_len)
     {
        SETSTR(eep.s, params[3]);
     }
   else
     {
        char *tmp = alloca(dst_len);
        memcpy(tmp, eep.s, dst_len - 1);
        tmp[dst_len - 1] = '\0';
        SETSTR(tmp, params[3]);
     }
   return 1;

error:
   SETSTR("", params[3]);
   return 0;
}

/* external_param_set_choice(id, param_name[], val[]) */
static Embryo_Cell
_edje_embryo_fn_external_param_set_choice(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name, *val;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_CHOICE;
   GETSTR(val, params[3]);
   if (!val) return 0;
   eep.s = val;
   return _edje_external_param_set(NULL, rp, &eep);
}

/* external_param_get_bool(id, param_name[]) */
static Embryo_Cell
_edje_embryo_fn_external_param_get_bool(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name;

   CHKPARAM(2);
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_BOOL;
   eep.i = 0;
   _edje_external_param_get(NULL, rp, &eep);
   return eep.i;
}

/* external_param_set_bool(id, param_name[], val) */
static Embryo_Cell
_edje_embryo_fn_external_param_set_bool(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id;
   Edje_Real_Part *rp;
   Edje_External_Param eep;
   char *param_name;

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);

   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   GETSTR(param_name, params[2]);
   if (!param_name) return 0;
   eep.name = param_name;
   eep.type = EDJE_EXTERNAL_PARAM_TYPE_BOOL;
   eep.i = params[3];
   return _edje_external_param_set(NULL, rp, &eep);
}

#ifdef HAVE_EPHYSICS
/* Generic function to call ephysics functions that apply an action to
* a body using 3 double values.
* To be used by the other functions only avoiding code duplication. */
static Embryo_Cell
_edje_embryo_fn_physics_components_set(Embryo_Program *ep, Embryo_Cell *params, void (*func)(EPhysics_Body *body, double x, double y, double z))
{
   Edje_Real_Part *rp;
   int part_id = 0;
   Edje *ed;

   CHKPARAM(4);

   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;

   rp = ed->table_parts[part_id % ed->table_parts_size];
   if ((rp) && (rp->body))
     {
        double x, y, z;

        x = (double)EMBRYO_CELL_TO_FLOAT(params[2]);
        y = (double)EMBRYO_CELL_TO_FLOAT(params[3]);
        z = (double)EMBRYO_CELL_TO_FLOAT(params[4]);

        func(rp->body, x, y, z);
     }

   return 0;
}

/* Generic function to call ephysics functions that get components related
 * to actions from a body using 3 double values.
 * To be used by the other functions only avoiding code duplication. */
static Embryo_Cell
_edje_embryo_fn_physics_components_get(Embryo_Program *ep, Embryo_Cell *params, void (*func)(const EPhysics_Body *body, double *x, double *y, double *z))
{
   Edje_Real_Part *rp;
   int part_id = 0;
   Edje *ed;

   CHKPARAM(4);

   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;

   rp = ed->table_parts[part_id % ed->table_parts_size];
   if ((rp) && (rp->body))
     {
        double x, y, z;
        func(rp->body, &x, &y, &z);
        SETFLOAT(x, params[2]);
        SETFLOAT(y, params[3]);
        SETFLOAT(z, params[4]);
     }

   return 0;
}

/* physics_impulse(part_id, Float:x, Float:y, Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_impulse(Embryo_Program *ep, Embryo_Cell *params)
{
   if (!EPH_LOAD()) return 0;
   return _edje_embryo_fn_physics_components_set(
            ep, params, EPH_CALL(ephysics_body_central_impulse_apply));
}

/* physics_torque_impulse(part_id, Float:x, Float:y, Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_torque_impulse(Embryo_Program *ep, Embryo_Cell *params)
{
   if (!EPH_LOAD()) return 0;
   return _edje_embryo_fn_physics_components_set(
            ep, params, EPH_CALL(ephysics_body_torque_impulse_apply));
}

/* physics_force(part_id, Float:x, Float:y, Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_force(Embryo_Program *ep, Embryo_Cell *params)
{
   if (!EPH_LOAD()) return 0;
   return _edje_embryo_fn_physics_components_set(
            ep, params, EPH_CALL(ephysics_body_central_force_apply));
}

/* physics_torque(part_id, Float:x, Float:y, Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_torque(Embryo_Program *ep, Embryo_Cell *params)
{
   if (!EPH_LOAD()) return 0;
   return _edje_embryo_fn_physics_components_set(
            ep, params, EPH_CALL(ephysics_body_torque_apply));
}

/* physics_clear_forces(part_id) */
static Embryo_Cell
_edje_embryo_fn_physics_clear_forces(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje_Real_Part *rp;
   int part_id = 0;
   Edje *ed;

   CHKPARAM(1);

   if (!EPH_LOAD()) return 0;
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;

   rp = ed->table_parts[part_id % ed->table_parts_size];
   if ((rp) && (rp->body))
     EPH_CALL(ephysics_body_forces_clear)(rp->body);

   return 0;
}

/* physics_get_forces(part_id, &Float:x, &Float:y, &Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_get_forces(Embryo_Program *ep, Embryo_Cell *params)
{
   if (!EPH_LOAD()) return 0;
   return _edje_embryo_fn_physics_components_get(
            ep, params, EPH_CALL(ephysics_body_forces_get));
}

/* physics_get_torques(part_id, &Float:x, &Float:y, &Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_get_torques(Embryo_Program *ep, Embryo_Cell *params)
{
   if (!EPH_LOAD()) return 0;
   return _edje_embryo_fn_physics_components_get(
            ep, params, EPH_CALL(ephysics_body_torques_get));
}

/* physics_set_velocity(part_id, Float:x, Float:y, Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_set_velocity(Embryo_Program *ep, Embryo_Cell *params)
{
   if (!EPH_LOAD()) return 0;
   return _edje_embryo_fn_physics_components_set(
            ep, params, EPH_CALL(ephysics_body_linear_velocity_set));
}

/* physics_get_velocity(part_id, &Float:x, &Float:y, &Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_get_velocity(Embryo_Program *ep, Embryo_Cell *params)
{
   if (!EPH_LOAD()) return 0;
   return _edje_embryo_fn_physics_components_get(
            ep, params, EPH_CALL(ephysics_body_linear_velocity_get));
}

/* physics_set_ang_velocity(part_id, Float:x, Float:y, Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_set_ang_velocity(Embryo_Program *ep, Embryo_Cell *params)
{
   if (!EPH_LOAD()) return 0;
   return _edje_embryo_fn_physics_components_set(
            ep, params, EPH_CALL(ephysics_body_angular_velocity_set));
}

/* physics_get_ang_velocity(part_id, &Float:x, &Float:y, &Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_get_ang_velocity(Embryo_Program *ep, Embryo_Cell *params)
{
   if (!EPH_LOAD()) return 0;
   return _edje_embryo_fn_physics_components_get(
            ep, params, EPH_CALL(ephysics_body_angular_velocity_get));
}

/* physics_stop(part_id) */
static Embryo_Cell
_edje_embryo_fn_physics_stop(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje_Real_Part *rp;
   int part_id = 0;
   Edje *ed;

   CHKPARAM(1);

   if (!EPH_LOAD()) return 0;
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;

   rp = ed->table_parts[part_id % ed->table_parts_size];
   if ((rp) && (rp->body))
     EPH_CALL(ephysics_body_stop)(rp->body);

   return 0;
}

/* physics_set_rotation(part_id, Float:w, Float:x, Float:y, Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_set_rotation(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje_Real_Part *rp;
   int part_id = 0;
   Edje *ed;

   CHKPARAM(5);

   if (!EPH_LOAD()) return 0;
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;

   rp = ed->table_parts[part_id % ed->table_parts_size];
   if ((rp) && (rp->body))
     {
        EPhysics_Quaternion quat;
        double w, x, y, z;

        w = (double)EMBRYO_CELL_TO_FLOAT(params[2]);
        x = (double)EMBRYO_CELL_TO_FLOAT(params[3]);
        y = (double)EMBRYO_CELL_TO_FLOAT(params[4]);
        z = (double)EMBRYO_CELL_TO_FLOAT(params[5]);

        EPH_CALL(ephysics_quaternion_set)(&quat, x, y, z, w);
        EPH_CALL(ephysics_quaternion_normalize)(&quat);
        EPH_CALL(ephysics_body_rotation_set)(rp->body, &quat);
     }

   return 0;
}

/* physics_get_rotation(part_id, &Float:w, &Float:x, &Float:y, &Float:z) */
static Embryo_Cell
_edje_embryo_fn_physics_get_rotation(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje_Real_Part *rp;
   int part_id = 0;
   Edje *ed;

   CHKPARAM(5);

   if (!EPH_LOAD()) return 0;
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;

   rp = ed->table_parts[part_id % ed->table_parts_size];
   if ((rp) && (rp->body))
     {
        EPhysics_Quaternion quat;
        double w, x, y, z;

        EPH_CALL(ephysics_body_rotation_get)(rp->body, &quat);
        EPH_CALL(ephysics_quaternion_get)(&quat, &x, &y, &z, &w);

        SETFLOAT(w, params[2]);
        SETFLOAT(x, params[3]);
        SETFLOAT(y, params[4]);
        SETFLOAT(z, params[5]);
     }

   return 0;
}

#endif

/* swallow_has_content(part_id) */
static Embryo_Cell
_edje_embryo_fn_swallow_has_content(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   int part_id = 0;
   Edje_Real_Part *rp;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   part_id = params[1];
   if (part_id < 0) return 0;
   rp = ed->table_parts[part_id % ed->table_parts_size];

   if ((!rp) ||
       (!rp->part) ||
       (rp->part->type != EDJE_PART_TYPE_SWALLOW) ||
       (!rp->typedata.swallow) ||
       (!rp->typedata.swallow->swallowed_object))
      return 0;

   return 1;
}

/* get_part_id(part[]) */
static Embryo_Cell
_edje_embryo_fn_echo(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *p;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   GETSTR(p, params[1]);
   if (!p) return -1;
   fprintf(stderr,
           "<EDJE ECHO> [%llx | %s:%s]: %s\n",
           (unsigned long long)((uintptr_t)ed),
           ed->path,
           ed->group,
           p);
   return 0;
}

void
_edje_embryo_script_init(Edje_Part_Collection *edc)
{
   Embryo_Program *ep;

   if (!edc) return;
   if (!edc->script) return;

   ep = edc->script;
   /* first advertise all the edje "script" calls */
   embryo_program_native_call_add(ep, "get_int", _edje_embryo_fn_get_int);
   embryo_program_native_call_add(ep, "set_int", _edje_embryo_fn_set_int);
   embryo_program_native_call_add(ep, "get_float", _edje_embryo_fn_get_float);
   embryo_program_native_call_add(ep, "set_float", _edje_embryo_fn_set_float);
   embryo_program_native_call_add(ep, "get_str", _edje_embryo_fn_get_str);
   embryo_program_native_call_add(ep, "get_strlen", _edje_embryo_fn_get_strlen);
   embryo_program_native_call_add(ep, "set_str", _edje_embryo_fn_set_str);
   embryo_program_native_call_add(ep, "count", _edje_embryo_fn_count);
   embryo_program_native_call_add(ep, "remove", _edje_embryo_fn_remove);
   embryo_program_native_call_add(ep, "append_int", _edje_embryo_fn_append_int);
   embryo_program_native_call_add(ep, "prepend_int", _edje_embryo_fn_prepend_int);
   embryo_program_native_call_add(ep, "insert_int", _edje_embryo_fn_insert_int);
   embryo_program_native_call_add(ep, "replace_int", _edje_embryo_fn_replace_int);
   embryo_program_native_call_add(ep, "fetch_int", _edje_embryo_fn_fetch_int);
   embryo_program_native_call_add(ep, "append_str", _edje_embryo_fn_append_str);
   embryo_program_native_call_add(ep, "prepend_str", _edje_embryo_fn_prepend_str);
   embryo_program_native_call_add(ep, "insert_str", _edje_embryo_fn_insert_str);
   embryo_program_native_call_add(ep, "replace_str", _edje_embryo_fn_replace_str);
   embryo_program_native_call_add(ep, "fetch_str", _edje_embryo_fn_fetch_str);
   embryo_program_native_call_add(ep, "append_float", _edje_embryo_fn_append_float);
   embryo_program_native_call_add(ep, "prepend_float", _edje_embryo_fn_prepend_float);
   embryo_program_native_call_add(ep, "insert_float", _edje_embryo_fn_insert_float);
   embryo_program_native_call_add(ep, "replace_float", _edje_embryo_fn_replace_float);
   embryo_program_native_call_add(ep, "fetch_float", _edje_embryo_fn_fetch_float);

   embryo_program_native_call_add(ep, "timer", _edje_embryo_fn_timer);
   embryo_program_native_call_add(ep, "cancel_timer", _edje_embryo_fn_cancel_timer);
   embryo_program_native_call_add(ep, "reset_timer", _edje_embryo_fn_reset_timer);

   embryo_program_native_call_add(ep, "anim", _edje_embryo_fn_anim);
   embryo_program_native_call_add(ep, "cancel_anim", _edje_embryo_fn_cancel_anim);
   embryo_program_native_call_add(ep, "get_anim_pos_map", _edje_embryo_fn_get_anim_pos_map);

   embryo_program_native_call_add(ep, "emit", _edje_embryo_fn_emit);
   embryo_program_native_call_add(ep, "get_part_id", _edje_embryo_fn_get_part_id);
   embryo_program_native_call_add(ep, "get_image_id", _edje_embryo_fn_get_image_id);
   embryo_program_native_call_add(ep, "get_program_id", _edje_embryo_fn_get_program_id);
   embryo_program_native_call_add(ep, "set_state", _edje_embryo_fn_set_state);
   embryo_program_native_call_add(ep, "get_state", _edje_embryo_fn_get_state);
   embryo_program_native_call_add(ep, "set_tween_state", _edje_embryo_fn_set_tween_state);
   embryo_program_native_call_add(ep, "set_tween_state_anim", _edje_embryo_fn_set_tween_state_anim);
   embryo_program_native_call_add(ep, "run_program", _edje_embryo_fn_run_program);
   embryo_program_native_call_add(ep, "get_drag_dir", _edje_embryo_fn_get_drag_dir);
   embryo_program_native_call_add(ep, "get_drag", _edje_embryo_fn_get_drag);
   embryo_program_native_call_add(ep, "set_drag", _edje_embryo_fn_set_drag);
   embryo_program_native_call_add(ep, "get_drag_size", _edje_embryo_fn_get_drag_size);
   embryo_program_native_call_add(ep, "set_drag_size", _edje_embryo_fn_set_drag_size);
   embryo_program_native_call_add(ep, "set_text", _edje_embryo_fn_set_text);
   embryo_program_native_call_add(ep, "get_text", _edje_embryo_fn_get_text);
   embryo_program_native_call_add(ep, "get_min_size", _edje_embryo_fn_get_min_size);
   embryo_program_native_call_add(ep, "get_max_size", _edje_embryo_fn_get_max_size);
   embryo_program_native_call_add(ep, "get_color_class", _edje_embryo_fn_get_color_class);
   embryo_program_native_call_add(ep, "set_color_class", _edje_embryo_fn_set_color_class);
   embryo_program_native_call_add(ep, "set_text_class", _edje_embryo_fn_set_text_class);
   embryo_program_native_call_add(ep, "get_text_class", _edje_embryo_fn_get_text_class);
   embryo_program_native_call_add(ep, "get_drag_step", _edje_embryo_fn_get_drag_step);
   embryo_program_native_call_add(ep, "set_drag_step", _edje_embryo_fn_set_drag_step);
   embryo_program_native_call_add(ep, "get_drag_page", _edje_embryo_fn_get_drag_page);
   embryo_program_native_call_add(ep, "set_drag_page", _edje_embryo_fn_set_drag_page);
   embryo_program_native_call_add(ep, "get_mouse", _edje_embryo_fn_get_mouse);
   embryo_program_native_call_add(ep, "get_mouse_buttons", _edje_embryo_fn_get_mouse_buttons);
   embryo_program_native_call_add(ep, "stop_program", _edje_embryo_fn_stop_program);
   embryo_program_native_call_add(ep, "stop_programs_on", _edje_embryo_fn_stop_programs_on);
   embryo_program_native_call_add(ep, "set_min_size", _edje_embryo_fn_set_min_size);
   embryo_program_native_call_add(ep, "set_max_size", _edje_embryo_fn_set_max_size);
   embryo_program_native_call_add(ep, "play_sample", _edje_embryo_fn_play_sample);
   embryo_program_native_call_add(ep, "play_tone", _edje_embryo_fn_play_tone);
   embryo_program_native_call_add(ep, "play_vibration", _edje_embryo_fn_play_vibration);
   embryo_program_native_call_add(ep, "send_message", _edje_embryo_fn_send_message);
   embryo_program_native_call_add(ep, "get_geometry", _edje_embryo_fn_get_geometry);
   embryo_program_native_call_add(ep, "custom_state", _edje_embryo_fn_custom_state);
   embryo_program_native_call_add(ep, "set_state_val", _edje_embryo_fn_set_state_val);
   embryo_program_native_call_add(ep, "get_state_val", _edje_embryo_fn_get_state_val);
   embryo_program_native_call_add(ep, "set_state_anim", _edje_embryo_fn_set_state_anim);

   embryo_program_native_call_add(ep, "set_mouse_events", _edje_embryo_fn_set_mouse_events);
   embryo_program_native_call_add(ep, "get_mouse_events", _edje_embryo_fn_get_mouse_events);
   embryo_program_native_call_add(ep, "set_pointer_mode", _edje_embryo_fn_set_pointer_mode);
   embryo_program_native_call_add(ep, "set_repeat_events", _edje_embryo_fn_set_repeat_events);
   embryo_program_native_call_add(ep, "get_repeat_events", _edje_embryo_fn_get_repeat_events);
   embryo_program_native_call_add(ep, "set_ignore_flags", _edje_embryo_fn_set_ignore_flags);
   embryo_program_native_call_add(ep, "get_ignore_flags", _edje_embryo_fn_get_ignore_flags);
   embryo_program_native_call_add(ep, "set_mask_flags", _edje_embryo_fn_set_mask_flags);
   embryo_program_native_call_add(ep, "get_mask_flags", _edje_embryo_fn_get_mask_flags);

   embryo_program_native_call_add(ep, "set_focus", _edje_embryo_fn_set_focus);
   embryo_program_native_call_add(ep, "unset_focus", _edje_embryo_fn_unset_focus);

   embryo_program_native_call_add(ep, "part_swallow", _edje_embryo_fn_part_swallow);

   embryo_program_native_call_add(ep, "external_param_get_int", _edje_embryo_fn_external_param_get_int);
   embryo_program_native_call_add(ep, "external_param_set_int", _edje_embryo_fn_external_param_set_int);
   embryo_program_native_call_add(ep, "external_param_get_float", _edje_embryo_fn_external_param_get_float);
   embryo_program_native_call_add(ep, "external_param_set_float", _edje_embryo_fn_external_param_set_float);
   embryo_program_native_call_add(ep, "external_param_get_strlen", _edje_embryo_fn_external_param_get_strlen);
   embryo_program_native_call_add(ep, "external_param_get_str", _edje_embryo_fn_external_param_get_str);
   embryo_program_native_call_add(ep, "external_param_set_str", _edje_embryo_fn_external_param_set_str);
   embryo_program_native_call_add(ep, "external_param_get_choice_len", _edje_embryo_fn_external_param_get_choice_len);
   embryo_program_native_call_add(ep, "external_param_get_choice", _edje_embryo_fn_external_param_get_choice);
   embryo_program_native_call_add(ep, "external_param_set_choice", _edje_embryo_fn_external_param_set_choice);
   embryo_program_native_call_add(ep, "external_param_get_bool", _edje_embryo_fn_external_param_get_bool);
   embryo_program_native_call_add(ep, "external_param_set_bool", _edje_embryo_fn_external_param_set_bool);

#ifdef HAVE_EPHYSICS
   embryo_program_native_call_add(ep, "physics_impulse", _edje_embryo_fn_physics_impulse);
   embryo_program_native_call_add(ep, "physics_torque_impulse", _edje_embryo_fn_physics_torque_impulse);
   embryo_program_native_call_add(ep, "physics_force", _edje_embryo_fn_physics_force);
   embryo_program_native_call_add(ep, "physics_torque", _edje_embryo_fn_physics_torque);
   embryo_program_native_call_add(ep, "physics_clear_forces", _edje_embryo_fn_physics_clear_forces);
   embryo_program_native_call_add(ep, "physics_get_forces", _edje_embryo_fn_physics_get_forces);
   embryo_program_native_call_add(ep, "physics_get_torques", _edje_embryo_fn_physics_get_torques);
   embryo_program_native_call_add(ep, "physics_set_velocity", _edje_embryo_fn_physics_set_velocity);
   embryo_program_native_call_add(ep, "physics_get_velocity", _edje_embryo_fn_physics_get_velocity);
   embryo_program_native_call_add(ep, "physics_set_ang_velocity", _edje_embryo_fn_physics_set_ang_velocity);
   embryo_program_native_call_add(ep, "physics_get_ang_velocity", _edje_embryo_fn_physics_get_ang_velocity);
   embryo_program_native_call_add(ep, "physics_stop", _edje_embryo_fn_physics_stop);
   embryo_program_native_call_add(ep, "physics_set_rotation", _edje_embryo_fn_physics_set_rotation);
   embryo_program_native_call_add(ep, "physics_get_rotation", _edje_embryo_fn_physics_get_rotation);
#endif

   embryo_program_native_call_add(ep, "swallow_has_content", _edje_embryo_fn_swallow_has_content);
   embryo_program_native_call_add(ep, "echo", _edje_embryo_fn_echo);
}

void
_edje_embryo_script_shutdown(Edje_Part_Collection *edc)
{
   if (!edc) return;
   if (!edc->script) return;
   if (embryo_program_recursion_get(edc->script) > 0) return;
   embryo_program_free(edc->script);
   edc->script = NULL;
}

void
_edje_embryo_script_reset(Edje *ed)
{
   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   if (embryo_program_recursion_get(ed->collection->script) > 0) return;
   embryo_program_vm_reset(ed->collection->script);
   _edje_embryo_globals_init(ed);
}

/* this may change in future - thus "test_run" is its name */
void
_edje_embryo_test_run(Edje *ed, Edje_Program *pr, const char *sig, const char *src)
{
   char fname[128];
   Embryo_Function fn;

   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   embryo_program_vm_push(ed->collection->script);
   _edje_embryo_globals_init(ed);

   //   _edje_embryo_script_reset(ed);
   snprintf(fname, sizeof(fname), "_p%i", pr->id);
   fn = embryo_program_function_find(ed->collection->script, (char *)fname);
   if (fn != EMBRYO_FUNCTION_NONE)
     {
        void *pdata;
        int ret;

        embryo_parameter_string_push(ed->collection->script, (char *)sig);
        embryo_parameter_string_push(ed->collection->script, (char *)src);
        pdata = embryo_program_data_get(ed->collection->script);
        embryo_program_data_set(ed->collection->script, ed);
        /* 5 million instructions is an arbitrary number. on my p4-2.6 here */
        /* IF embryo is ONLY running embryo stuff and NO native calls thats */
        /* about 0.016 seconds, and longer on slower cpu's. if a simple */
        /* embryo script snippet hasn't managed to do its work in 5 MILLION */
        /* embryo virtual machine instructions - something is wrong, or */
        /* embryo is simply being mis-used. Embryo is meant to be minimal */
        /* logic enhancment - not entire applications. this cycle count */
        /* does NOT include time spent in native function calls, that the */
        /* script may call to do the REAL work, so in terms of time this */
        /* will likely end up being much longer than 0.016 seconds - more */
        /* like 0.03 - 0.05 seconds or even more */
        embryo_program_max_cycle_run_set(ed->collection->script, 5000000);
        if (embryo_program_recursion_get(ed->collection->script) && (!ed->collection->script_recursion))
          ERR("You are running Embryo->EDC->Embryo with script program '%s';\n"
              "A run_program runs the '%d'th program '%s' in the group '%s' of file %s;\n"
              "By the power of Grayskull, your previous Embryo stack is now broken!",
              fname, (fn + 1), pr->name, ed->group, ed->path);

        ret = embryo_program_run(ed->collection->script, fn);
        if (ret == EMBRYO_PROGRAM_FAIL)
          {
             ERR("ERROR with embryo script. "
                 "OBJECT NAME: '%s', "
                 "OBJECT FILE: '%s', "
                 "ENTRY POINT: '%s (%s)', "
                 "SIGNAL: '%s', "
                 "SOURCE: '%s', "
                 "ERROR: '%s'",
                 ed->collection->name,
                 ed->file->path,
                 fname, pr->name,
                 sig, src,
                 embryo_error_string_get(embryo_program_error_get(ed->collection->script)));
          }
        else if (ret == EMBRYO_PROGRAM_TOOLONG)
          {
             ERR("ERROR with embryo script. "
                 "OBJECT NAME: '%s', "
                 "OBJECT FILE: '%s', "
                 "ENTRY POINT: '%s (%s)', "
                 "SIGNAL: '%s', "
                 "SOURCE: '%s', "
                 "ERROR: 'Script exceeded maximum allowed cycle count of %i'",
                 ed->collection->name,
                 ed->file->path,
                 fname, pr->name,
                 sig, src,
                 embryo_program_max_cycle_run_get(ed->collection->script));
          }
        embryo_program_data_set(ed->collection->script, pdata);
     }
   embryo_program_vm_pop(ed->collection->script);
}

void
_edje_embryo_globals_init(Edje *ed)
{
   int n, i;
   Embryo_Program *ep;

   ep = ed->collection->script;
   n = embryo_program_variable_count_get(ep);
   for (i = 0; i < n; i++)
     {
        Embryo_Cell cell, *cptr;

        cell = embryo_program_variable_get(ep, i);
        if (cell != EMBRYO_CELL_NONE)
          {
             cptr = embryo_data_address_get(ep, cell);
             if (cptr) *cptr = EDJE_VAR_MAGIC_BASE + i;
          }
     }
}

