/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

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
 * anim(Float:len, fname[], val)
 * cancel_anim(id)
 * emit(sig[], src[])
 * set_state(part_id, state[], Float:state_val)
 * set_tween_state(part_id, Float:tween, state1[], Float:state1_val, state2[], Float:state2_val)
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
 * get_geometry(part_id, &Float:x, &Float:y, &Float:w, &Float:h)
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
 * visible
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
 * set_repeat_events(part_id, rep)
 * get_repeat_events(part_id)
 * set_clip(part_id, clip_part_id)
 * get_clip(part_id)
 *
 * part_swallow(part_id, group_name)
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
	if (strlen(s) < params[3])
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

   return (Embryo_Cell)_edje_var_list_count_get(ed, (int) params[1]);
}

/* remove(id, n) */
static Embryo_Cell
_edje_embryo_fn_remove(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(2);

   _edje_var_list_remove_nth(ed, (int) params[1], (int) params[2]);

   return 0;
}

/* append_int(id, var) */
static Embryo_Cell
_edje_embryo_fn_append_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(2);

   _edje_var_list_int_append(ed, (int) params[1], (int) params[2]);

   return 0;
}

/* prepend_int(id, var) */
static Embryo_Cell
_edje_embryo_fn_prepend_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(2);

   _edje_var_list_int_prepend(ed, (int) params[1], (int) params[2]);

   return 0;
}

/* insert_int(id, pos, var) */
static Embryo_Cell
_edje_embryo_fn_insert_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(3);

   _edje_var_list_int_insert(ed, (int) params[1], (int) params[2],
                             (int) params[3]);

   return 0;
}

/* replace_int(id, pos, var) */
static Embryo_Cell
_edje_embryo_fn_replace_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(3);

   _edje_var_list_nth_int_set(ed, (int) params[1], (int) params[2],
                              (int) params[3]);

   return 0;
}

/* fetch_int(id, pos) */
static Embryo_Cell
_edje_embryo_fn_fetch_int(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(2);

   return _edje_var_list_nth_int_get(ed, (int) params[1],
                                     (int) params[2]);
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
     _edje_var_list_str_append(ed, (int) params[1], s);

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
     _edje_var_list_str_prepend(ed, (int) params[1], s);

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
     _edje_var_list_str_insert(ed, (int) params[1], (int) params[2], s);

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
	_edje_var_list_nth_str_set(ed, (int) params[1], (int) params[2], s);

   return 0;
}


/* fetch_str(id, pos, dst[], maxlen) */
static Embryo_Cell
_edje_embryo_fn_fetch_str(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   char *s;

   CHKPARAM(4);

   s = (char *) _edje_var_list_nth_str_get(ed, (int) params[1],
                                           (int) params[2]);
   if (s)
     {
	if (strlen(s) < params[4])
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
   _edje_var_list_float_append(ed, (int) params[1], f);

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
   _edje_var_list_float_prepend(ed, (int) params[1], f);

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
   _edje_var_list_float_insert(ed, (int) params[1], (int) params[2], f);

   return 0;
}

/* replace_float(id, pos, Float:f) */
static Embryo_Cell
_edje_embryo_fn_replace_float(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);

   CHKPARAM(3);

   _edje_var_list_nth_float_set(ed, (int) params[1], (int) params[2],
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

   f = _edje_var_list_nth_float_get(ed, (int) params[1], (int) params[2]);

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
   ed->dirty = 1;
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
   ed->dirty = 1;
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
   Eina_List *l;

   CHKPARAM(1);
   ed = embryo_program_data_get(ep);
   program_id = params[1];
   if (program_id < 0) return 0;

   ed->walking_actions = 1;

   EINA_LIST_FOREACH(ed->actions, l, runp)
     if (program_id == runp->program->id)
       _edje_program_end(ed, runp);

   ed->walking_actions = 0;

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
	/* there is only ever 1 program acting on a part at any time */
	if (rp->program) _edje_program_end(ed, rp->program);
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
   evas_pointer_canvas_xy_get(ed->evas, &x, &y);
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
   return evas_pointer_button_down_mask_get(ed->evas);
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

   CHKPARAM(3);
   ed = embryo_program_data_get(ep);
   GETSTR(state, params[2]);
   if ((!state)) return 0;
   part_id = params[1];
   if (part_id < 0) return 0;
   f = EMBRYO_CELL_TO_FLOAT(params[3]);
   value = (double)f;
   rp = ed->table_parts[part_id % ed->table_parts_size];
   if (rp)
     {
	if (rp->program) _edje_program_end(ed, rp->program);
	_edje_part_description_apply(ed, rp, state, value, NULL, 0.0);
	_edje_part_pos_set(ed, rp, EDJE_TWEEN_MODE_LINEAR, 0.0);
	_edje_recalc(ed);
     }
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
	     if (strlen(s) < params[3])
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
	_edje_part_pos_set(ed, rp, EDJE_TWEEN_MODE_LINEAR, tween);
	_edje_recalc(ed);
     }
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
   pr = ed->table_programs[program_id % ed->table_programs_size];
   if (pr)
     {
	_edje_program_run(ed, pr, 0, "", "");
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
   return(0);
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
   return(0);
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
   if (s){
     edje_object_part_text_set(ed->obj, rp->part->name, s);
   }
   return(0);
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
	if (strlen(s) < params[3])
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
   if (c_class == NULL) return 0;
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
   edje_object_color_class_set(ed->obj, class, params[2], params[3], params[4], params[5],
			       params[2], params[3], params[4], params[5],
			       params[2], params[3], params[4], params[5]);
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
   if( !class || !font ) return 0;
   fsize = (Evas_Font_Size) EMBRYO_CELL_TO_FLOAT(params[3]);
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
   if (t_class == NULL) return 0;
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
   return(0);
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
   Evas_Coord x = 0.0, y = 0.0, w = 0.0, h = 0.0;

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
   return(0);
}

/* send_message(Msg_Type:type, id,...); */
static Embryo_Cell
_edje_embryo_fn_send_message(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Edje_Message_Type type;
   int id, i, n;
   Embryo_Cell *ptr;

   if (params[0] < (sizeof(Embryo_Cell) * (2))) return 0;
   ed = embryo_program_data_get(ep);
   type = params[1];
   id = params[2];
   switch (type)
     {
      case EDJE_MESSAGE_NONE:
	_edje_message_send(ed, EDJE_QUEUE_APP, type, id, NULL);
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
		  embryo_data_string_get(ep, cptr, s);
		  emsg = alloca(sizeof(Edje_Message_String));
		  emsg->str = s;
		  _edje_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
	       }
	  }
	break;
      case EDJE_MESSAGE_INT:
	  {
	     Edje_Message_Int *emsg;

	     emsg = alloca(sizeof(Edje_Message_Int));
	     ptr = embryo_data_address_get(ep, params[3]);
	     emsg->val = (int)*ptr;
	     _edje_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
	  }
	break;
      case EDJE_MESSAGE_FLOAT:
	  {
	     Edje_Message_Float *emsg;
	     float f;

	     emsg = alloca(sizeof(Edje_Message_Float));
	     ptr = embryo_data_address_get(ep, params[3]);
	     f = EMBRYO_CELL_TO_FLOAT(*ptr);
	     emsg->val = (double)f;
	     _edje_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
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
		       embryo_data_string_get(ep, cptr, s);
		       emsg->str[i - 3] = s;
		    }
	       }
	     _edje_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
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
		  emsg->val[i - 3] = (int)*ptr;
	       }
	     _edje_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
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
		  f = EMBRYO_CELL_TO_FLOAT(*ptr);
		  emsg->val[i - 3] = (double)f;
	       }
	     _edje_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
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
		  embryo_data_string_get(ep, cptr, s);
		  emsg = alloca(sizeof(Edje_Message_String_Int));
		  emsg->str = s;
		  ptr = embryo_data_address_get(ep, params[4]);
		  emsg->val = (int)*ptr;
		  _edje_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
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
		  embryo_data_string_get(ep, cptr, s);
		  emsg = alloca(sizeof(Edje_Message_String_Float));
		  emsg->str = s;
		  ptr = embryo_data_address_get(ep, params[4]);
		  f = EMBRYO_CELL_TO_FLOAT(*ptr);
		  emsg->val = (double)f;
		  _edje_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
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
		  embryo_data_string_get(ep, cptr, s);
		  n = (params[0] / sizeof(Embryo_Cell)) + 1;
		  emsg = alloca(sizeof(Edje_Message_String_Int_Set) + ((n - 4 - 1) * sizeof(int)));
		  emsg->str = s;
		  emsg->count = n - 4;
		  for (i = 4; i < n; i++)
		    {
		       ptr = embryo_data_address_get(ep, params[i]);
		       emsg->val[i - 4] = (int)*ptr;
		    }
		  _edje_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
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
		  embryo_data_string_get(ep, cptr, s);
		  n = (params[0] / sizeof(Embryo_Cell)) + 1;
		  emsg = alloca(sizeof(Edje_Message_String_Float_Set) + ((n - 4 - 1) * sizeof(double)));
		  emsg->str = s;
		  emsg->count = n - 4;
		  for (i = 4; i < n; i++)
		    {
		       float f;

		       ptr = embryo_data_address_get(ep, params[i]);
		       f = EMBRYO_CELL_TO_FLOAT(*ptr);
		       emsg->val[i - 4] = (double)f;
		    }
		  _edje_message_send(ed, EDJE_QUEUE_APP, type, id, emsg);
	       }
	  }
	break;
      default:
	break;
     }
   return(0);
}

/* custom_state(part_id, state[], Float:state_val = 0.0) */
static Embryo_Cell
_edje_embryo_fn_custom_state(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Edje_Real_Part *rp;
   Edje_Part_Description *parent, *d;
   Edje_Part_Image_Id *iid;
   Eina_List *l;
   char *name;
   float val;

   CHKPARAM(3);

   if (params[1] < 0)
     return 0;

   if (!(rp = ed->table_parts[params[1] % ed->table_parts_size]))
     return 0;

   /* check whether this part already has a "custom" state */
   if (rp->custom.description)
     return 0;

   GETSTR(name, params[2]);
   if (!name)
     return 0;

   val = EMBRYO_CELL_TO_FLOAT(params[3]);

   if (!(parent = _edje_part_description_find(ed, rp, name, val)))
     return 0;

   /* now create the custom state */
   if (!(d = calloc(1, sizeof(Edje_Part_Description))))
     return 0;

   *d = *parent;

   d->state.name = (char *)eina_stringshare_add("custom");
   d->state.value = 0.0;

   /* make sure all the allocated memory is getting copied,
    * not just referenced
    */
   d->image.tween_list = NULL;

   EINA_LIST_FOREACH(parent->image.tween_list, l, iid)
     {
        Edje_Part_Image_Id *iid_new;

	iid_new = calloc(1, sizeof(Edje_Part_Image_Id));
	iid_new->id = iid->id;

	d->image.tween_list = eina_list_append(d->image.tween_list, iid_new);
     }

#define DUP(x) x ? (char *)eina_stringshare_add(x) : NULL
   d->color_class = DUP(d->color_class);
   d->text.text = DUP(d->text.text);
   d->text.text_class = DUP(d->text.text_class);
   d->text.font = DUP(d->text.font);
   d->text.style = DUP(d->text.style);
#undef DUP

   rp->custom.description = d;

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
   if (params[0] < (sizeof(Embryo_Cell) * 3))
     return 0;

   if (params[1] < 0)
     return 0;

   if (!(rp = ed->table_parts[params[1] % ed->table_parts_size]))
     return 0;

   /* check whether this part has a "custom" state */
   if (!rp->custom.description)
     return 0;

   switch (params[2])
     {
      case EDJE_STATE_PARAM_ALIGNMENT:
	 CHKPARAM(4);

	 GETFLOAT(rp->custom.description->align.x, params[3]);
	 GETFLOAT(rp->custom.description->align.y, params[4]);

	 break;
      case EDJE_STATE_PARAM_MIN:
	 CHKPARAM(4);

	 GETINT(rp->custom.description->min.w, params[3]);
	 GETINT(rp->custom.description->min.h, params[4]);

	 break;
      case EDJE_STATE_PARAM_MAX:
	 CHKPARAM(4);

	 GETINT(rp->custom.description->max.w, params[3]);
	 GETINT(rp->custom.description->max.h, params[4]);

	 break;
      case EDJE_STATE_PARAM_STEP:
	 CHKPARAM(4);

	 GETINT(rp->custom.description->step.x, params[3]);
	 GETINT(rp->custom.description->step.y, params[4]);

	 break;
      case EDJE_STATE_PARAM_ASPECT:
	 CHKPARAM(4);

	 GETFLOAT(rp->custom.description->aspect.min, params[3]);
	 GETFLOAT(rp->custom.description->aspect.max, params[4]);

	 break;
      case EDJE_STATE_PARAM_ASPECT_PREF:
	 CHKPARAM(3);

	 GETINT(rp->custom.description->aspect.prefer, params[3]);

	 break;
      case EDJE_STATE_PARAM_COLOR:
	 CHKPARAM(6);

	 GETINT(rp->custom.description->color.r, params[3]);
	 GETINT(rp->custom.description->color.g, params[4]);
	 GETINT(rp->custom.description->color.b, params[5]);
	 GETINT(rp->custom.description->color.a, params[6]);

	 break;
      case EDJE_STATE_PARAM_COLOR2:
	 CHKPARAM(6);

	 GETINT(rp->custom.description->color2.r, params[3]);
	 GETINT(rp->custom.description->color2.g, params[4]);
	 GETINT(rp->custom.description->color2.b, params[5]);
	 GETINT(rp->custom.description->color2.a, params[6]);

	 break;
      case EDJE_STATE_PARAM_COLOR3:
	 CHKPARAM(6);

	 GETINT(rp->custom.description->color3.r, params[3]);
	 GETINT(rp->custom.description->color3.g, params[4]);
	 GETINT(rp->custom.description->color3.b, params[5]);
	 GETINT(rp->custom.description->color3.a, params[6]);

	 break;
      case EDJE_STATE_PARAM_COLOR_CLASS:
	 CHKPARAM(3);

	 GETSTR(s, params[3]);
	 GETSTREVAS(s, rp->custom.description->color_class);

	 break;
      case EDJE_STATE_PARAM_REL1:
	 CHKPARAM(4);

	 GETFLOAT(rp->custom.description->rel1.relative_x, params[3]);
	 GETFLOAT(rp->custom.description->rel1.relative_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_REL1_TO:
	 CHKPARAM(4);

	 GETINT(rp->custom.description->rel1.id_x, params[3]);
	 GETINT(rp->custom.description->rel1.id_y, params[4]);

	 if (rp->param1.description->rel1.id_x >= 0)
	   rp->param1.rel1_to_x = ed->table_parts[rp->param1.description->rel1.id_x % ed->table_parts_size];
	 if (rp->param1.description->rel1.id_y >= 0)
	   rp->param1.rel1_to_y = ed->table_parts[rp->param1.description->rel1.id_y % ed->table_parts_size];

	 break;
      case EDJE_STATE_PARAM_REL1_OFFSET:
	 CHKPARAM(4);

	 GETINT(rp->custom.description->rel1.offset_x, params[3]);
	 GETINT(rp->custom.description->rel1.offset_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_REL2:
	 CHKPARAM(4);

	 GETFLOAT(rp->custom.description->rel2.relative_x, params[3]);
	 GETFLOAT(rp->custom.description->rel2.relative_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_REL2_TO:
	 CHKPARAM(4);

	 GETINT(rp->custom.description->rel2.id_x, params[3]);
	 GETINT(rp->custom.description->rel2.id_y, params[4]);

	 if (rp->param1.description->rel2.id_x >= 0)
	   rp->param1.rel2_to_x = ed->table_parts[rp->param1.description->rel2.id_x % ed->table_parts_size];
	 if (rp->param1.description->rel2.id_y >= 0)
	   rp->param1.rel2_to_y = ed->table_parts[rp->param1.description->rel2.id_y % ed->table_parts_size];

	 break;
      case EDJE_STATE_PARAM_REL2_OFFSET:
	 CHKPARAM(4);

	 GETINT(rp->custom.description->rel2.offset_x, params[3]);
	 GETINT(rp->custom.description->rel2.offset_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_IMAGE:
	 if ( (rp->part->type != EDJE_PART_TYPE_IMAGE) ) return 0;
	 CHKPARAM(3);

	 GETINT(rp->custom.description->image.id, params[3]);

	 break;
      case EDJE_STATE_PARAM_BORDER:
	 if ( (rp->part->type != EDJE_PART_TYPE_IMAGE) ) return 0;
	 CHKPARAM(6);

	 GETINT(rp->custom.description->border.l, params[3]);
	 GETINT(rp->custom.description->border.r, params[4]);
	 GETINT(rp->custom.description->border.t, params[5]);
	 GETINT(rp->custom.description->border.b, params[6]);

	 break;
      case EDJE_STATE_PARAM_FILL_SMOOTH:
	 if ( (rp->part->type != EDJE_PART_TYPE_IMAGE) ) return 0;
	 CHKPARAM(3);

	 GETINT(rp->custom.description->fill.smooth, params[3]);

	 break;
      case EDJE_STATE_PARAM_FILL_POS:
	 if ( (rp->part->type != EDJE_PART_TYPE_IMAGE) ) return 0;
	 CHKPARAM(6);

	 GETFLOAT(rp->custom.description->fill.pos_rel_x, params[3]);
	 GETFLOAT(rp->custom.description->fill.pos_rel_y, params[4]);
	 GETINT(rp->custom.description->fill.pos_abs_x, params[5]);
	 GETINT(rp->custom.description->fill.pos_abs_y, params[6]);

	 break;
      case EDJE_STATE_PARAM_FILL_SIZE:
	 if ( (rp->part->type != EDJE_PART_TYPE_IMAGE) ) return 0;
	 CHKPARAM(6);

	 GETFLOAT(rp->custom.description->fill.rel_x, params[3]);
	 GETFLOAT(rp->custom.description->fill.rel_y, params[4]);
	 GETINT(rp->custom.description->fill.abs_x, params[5]);
	 GETINT(rp->custom.description->fill.abs_y, params[6]);

	 break;
      case EDJE_STATE_PARAM_TEXT:
	 if ( (rp->part->type != EDJE_PART_TYPE_TEXT) && \
	      (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
	   return 0;
	 CHKPARAM(3);

	 GETSTR(s, params[3]);
	 GETSTREVAS(s, rp->custom.description->text.text);

	 break;
      case EDJE_STATE_PARAM_TEXT_CLASS:
	 if ( (rp->part->type != EDJE_PART_TYPE_TEXT) && \
	       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
	   return 0;
	 CHKPARAM(3);

	 GETSTR(s, params[3]);
	 GETSTREVAS(s, rp->custom.description->text.text_class);

	 break;
      case EDJE_STATE_PARAM_TEXT_FONT:
	 if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
	 CHKPARAM(3);

	 GETSTR(s, params[3]);
	 GETSTREVAS(s, rp->custom.description->text.font);

	 break;
      case EDJE_STATE_PARAM_TEXT_STYLE:
	 if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return 0;
	 CHKPARAM(3);

	 GETSTR(s, params[3]);
	 GETSTREVAS(s, rp->custom.description->text.style);

	 break;
      case EDJE_STATE_PARAM_TEXT_SIZE:
	 if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
	 CHKPARAM(3);

	 GETINT(rp->custom.description->text.size, params[3]);

	 break;
      case EDJE_STATE_PARAM_TEXT_FIT:
	 if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
	 CHKPARAM(4);

	 GETINT(rp->custom.description->text.fit_x, params[3]);
	 GETINT(rp->custom.description->text.fit_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_TEXT_MIN:
	 if ( (rp->part->type != EDJE_PART_TYPE_TEXT) && \
	       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
	   return 0;
	 CHKPARAM(4);

	 GETINT(rp->custom.description->text.min_x, params[3]);
	 GETINT(rp->custom.description->text.min_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_TEXT_MAX:
	 if ( (rp->part->type != EDJE_PART_TYPE_TEXT) && \
	       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
	   return 0;
	 CHKPARAM(4);

	 GETINT(rp->custom.description->text.max_x, params[3]);
	 GETINT(rp->custom.description->text.max_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_TEXT_ALIGN:
	 if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
	 CHKPARAM(4);

	 GETFLOAT(rp->custom.description->text.align.x, params[3]);
	 GETFLOAT(rp->custom.description->text.align.y, params[4]);

	 break;
      case EDJE_STATE_PARAM_VISIBLE:
	 CHKPARAM(3);

	 GETINT(rp->custom.description->visible, params[3]);

	 break;
      default:
	 break;
     }

   ed->dirty=1;
   return 0;
}

/* get_state_val(part_id, State_Param:p, ...) */
static Embryo_Cell
_edje_embryo_fn_get_state_val(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Edje_Real_Part *rp;
   char *s;

   /* we need at least 3 arguments */
   if (params[0] < (sizeof(Embryo_Cell) * 3))
     return 0;

   if (params[1] < 0)
     return 0;

   if (!(rp = ed->table_parts[params[1] % ed->table_parts_size]))
     return 0;

   /* check whether this part has a "custom" state */
   if (!rp->custom.description)
     return 0;

   switch (params[2])
     {
      case EDJE_STATE_PARAM_ALIGNMENT:
	 CHKPARAM(4);

	 SETFLOAT(rp->custom.description->align.x, params[3]);
	 SETFLOAT(rp->custom.description->align.y, params[4]);

	 break;
      case EDJE_STATE_PARAM_MIN:
	 CHKPARAM(4);

	 SETINT(rp->custom.description->min.w, params[3]);
	 SETINT(rp->custom.description->min.h, params[4]);

	 break;
      case EDJE_STATE_PARAM_MAX:
	 CHKPARAM(4);

	 SETINT(rp->custom.description->max.w, params[3]);
	 SETINT(rp->custom.description->max.h, params[4]);

	 break;
      case EDJE_STATE_PARAM_STEP:
	 CHKPARAM(4);

	 SETINT(rp->custom.description->step.x, params[3]);
	 SETINT(rp->custom.description->step.y, params[4]);

	 break;
      case EDJE_STATE_PARAM_ASPECT:
	 CHKPARAM(4);

	 SETFLOAT(rp->custom.description->aspect.min, params[3]);
	 SETFLOAT(rp->custom.description->aspect.max, params[4]);

	 break;
      case EDJE_STATE_PARAM_ASPECT_PREF:
	 CHKPARAM(3);

	 SETINT(rp->custom.description->aspect.prefer, params[3]);

	 break;
      case EDJE_STATE_PARAM_COLOR:
	 CHKPARAM(6);

	 SETINT(rp->custom.description->color.r, params[3]);
	 SETINT(rp->custom.description->color.g, params[4]);
	 SETINT(rp->custom.description->color.b, params[5]);
	 SETINT(rp->custom.description->color.a, params[6]);

	 break;
      case EDJE_STATE_PARAM_COLOR2:
	 CHKPARAM(6);

	 SETINT(rp->custom.description->color2.r, params[3]);
	 SETINT(rp->custom.description->color2.g, params[4]);
	 SETINT(rp->custom.description->color2.b, params[5]);
	 SETINT(rp->custom.description->color2.a, params[6]);

	 break;
      case EDJE_STATE_PARAM_COLOR3:
	 CHKPARAM(6);

	 SETINT(rp->custom.description->color3.r, params[3]);
	 SETINT(rp->custom.description->color3.g, params[4]);
	 SETINT(rp->custom.description->color3.b, params[5]);
	 SETINT(rp->custom.description->color3.a, params[6]);

	 break;
      case EDJE_STATE_PARAM_COLOR_CLASS:
	 CHKPARAM(4);

	 s = rp->custom.description->color_class;
	 SETSTRALLOCATE(s);

	 break;
      case EDJE_STATE_PARAM_REL1:
	 CHKPARAM(4);

	 SETFLOAT(rp->custom.description->rel1.relative_x, params[3]);
	 SETFLOAT(rp->custom.description->rel1.relative_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_REL1_TO:
	 CHKPARAM(4);

	 SETINT(rp->custom.description->rel1.id_x, params[3]);
	 SETINT(rp->custom.description->rel1.id_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_REL1_OFFSET:
	 CHKPARAM(4);

	 SETINT(rp->custom.description->rel1.offset_x, params[3]);
	 SETINT(rp->custom.description->rel1.offset_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_REL2:
	 CHKPARAM(4);

	 SETFLOAT(rp->custom.description->rel2.relative_x, params[3]);
	 SETFLOAT(rp->custom.description->rel2.relative_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_REL2_TO:
	 CHKPARAM(4);

	 SETINT(rp->custom.description->rel2.id_x, params[3]);
	 SETINT(rp->custom.description->rel2.id_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_REL2_OFFSET:
	 CHKPARAM(4);

	 SETINT(rp->custom.description->rel2.offset_x, params[3]);
	 SETINT(rp->custom.description->rel2.offset_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_IMAGE:
	 if ( (rp->part->type != EDJE_PART_TYPE_IMAGE) ) return 0;
	 CHKPARAM(3);

	 SETINT(rp->custom.description->image.id, params[3]);

	 break;
      case EDJE_STATE_PARAM_BORDER:
	 if ( (rp->part->type != EDJE_PART_TYPE_IMAGE) ) return 0;
	 CHKPARAM(6);

	 SETINT(rp->custom.description->border.l, params[3]);
	 SETINT(rp->custom.description->border.r, params[4]);
	 SETINT(rp->custom.description->border.t, params[5]);
	 SETINT(rp->custom.description->border.b, params[6]);

	 break;
      case EDJE_STATE_PARAM_FILL_SMOOTH:
	 if ( (rp->part->type != EDJE_PART_TYPE_IMAGE) ) return 0;
	 CHKPARAM(3);

	 SETINT(rp->custom.description->fill.smooth, params[3]);

	 break;
      case EDJE_STATE_PARAM_FILL_POS:
	 if ( (rp->part->type != EDJE_PART_TYPE_IMAGE) ) return 0;
	 CHKPARAM(6);

	 SETFLOAT(rp->custom.description->fill.pos_rel_x, params[3]);
	 SETFLOAT(rp->custom.description->fill.pos_rel_y, params[4]);
	 SETINT(rp->custom.description->fill.pos_abs_x, params[5]);
	 SETINT(rp->custom.description->fill.pos_abs_y, params[6]);

	 break;
      case EDJE_STATE_PARAM_FILL_SIZE:
	 if ( (rp->part->type != EDJE_PART_TYPE_IMAGE) ) return 0;
	 CHKPARAM(6);

	 SETFLOAT(rp->custom.description->fill.rel_x, params[3]);
	 SETFLOAT(rp->custom.description->fill.rel_y, params[4]);
	 SETINT(rp->custom.description->fill.abs_x, params[5]);
	 SETINT(rp->custom.description->fill.abs_y, params[6]);

	 break;
      case EDJE_STATE_PARAM_TEXT:
	 if ( (rp->part->type != EDJE_PART_TYPE_TEXT) && \
	       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
	   return 0;
	 CHKPARAM(4);

	 s = rp->custom.description->text.text;
	 SETSTRALLOCATE(s);

	 break;
      case EDJE_STATE_PARAM_TEXT_CLASS:
	 if ( (rp->part->type != EDJE_PART_TYPE_TEXT) && \
	       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
	   return 0;
	 CHKPARAM(4);

	 s = rp->custom.description->text.text_class;
	 SETSTRALLOCATE(s);

	 break;
      case EDJE_STATE_PARAM_TEXT_FONT:
	 if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
	 CHKPARAM(4);

	 s = rp->custom.description->text.font;
	 SETSTRALLOCATE(s);

	 break;
      case EDJE_STATE_PARAM_TEXT_STYLE:
	 if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return 0;
	 CHKPARAM(4);

	 s = rp->custom.description->text.style;
	 SETSTRALLOCATE(s);

	 break;
      case EDJE_STATE_PARAM_TEXT_SIZE:
	 if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
	 CHKPARAM(3);

	 SETINT(rp->custom.description->text.size, params[3]);

	 break;
      case EDJE_STATE_PARAM_TEXT_FIT:
	 if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
	 CHKPARAM(4);

	 SETINT(rp->custom.description->text.fit_x, params[3]);
	 SETINT(rp->custom.description->text.fit_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_TEXT_MIN:
	 if ( (rp->part->type != EDJE_PART_TYPE_TEXT) && \
	       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
	   return 0;
	 CHKPARAM(4);

	 SETINT(rp->custom.description->text.min_x, params[3]);
	 SETINT(rp->custom.description->text.min_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_TEXT_MAX:
	 if ( (rp->part->type != EDJE_PART_TYPE_TEXT) && \
	       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK))
	   return 0;
	 CHKPARAM(4);

	 SETINT(rp->custom.description->text.max_x, params[3]);
	 SETINT(rp->custom.description->text.max_y, params[4]);

	 break;
      case EDJE_STATE_PARAM_TEXT_ALIGN:
	 if ((rp->part->type != EDJE_PART_TYPE_TEXT)) return 0;
	 CHKPARAM(4);

	 SETFLOAT(rp->custom.description->text.align.x, params[3]);
	 SETFLOAT(rp->custom.description->text.align.y, params[4]);

	 break;
      case EDJE_STATE_PARAM_VISIBLE:
	 CHKPARAM(3);

	 SETINT(rp->custom.description->visible, params[3]);

	 break;
      default:
	 break;
     }

   return 0;
}

/* part_swallow(part_id, group_name) */
static Embryo_Cell
_edje_embryo_fn_part_swallow(Embryo_Program *ep, Embryo_Cell *params)
{
   int part_id = 0;
   char* group_name = 0;
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

   new_obj =  edje_object_add(ed->evas);
   if (!new_obj) return 0;

   if (!edje_object_file_set(new_obj, ed->file->path, group_name)) 
     {
        evas_object_del(new_obj);
        return 0;
     }
   edje_object_part_swallow(ed->obj, rp->part->name, new_obj);

   return 0;
}

void
_edje_embryo_script_init(Edje *ed)
{
   Embryo_Program *ep;

   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   ep = ed->collection->script;
   embryo_program_data_set(ep, ed);
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

   embryo_program_native_call_add(ep, "anim", _edje_embryo_fn_anim);
   embryo_program_native_call_add(ep, "cancel_anim", _edje_embryo_fn_cancel_anim);

   embryo_program_native_call_add(ep, "emit", _edje_embryo_fn_emit);
   embryo_program_native_call_add(ep, "set_state", _edje_embryo_fn_set_state);
   embryo_program_native_call_add(ep, "get_state", _edje_embryo_fn_get_state);
   embryo_program_native_call_add(ep, "set_tween_state", _edje_embryo_fn_set_tween_state);
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

   embryo_program_native_call_add(ep, "send_message", _edje_embryo_fn_send_message);
   embryo_program_native_call_add(ep, "get_geometry", _edje_embryo_fn_get_geometry);
   embryo_program_native_call_add(ep, "custom_state", _edje_embryo_fn_custom_state);
   embryo_program_native_call_add(ep, "set_state_val", _edje_embryo_fn_set_state_val);
   embryo_program_native_call_add(ep, "get_state_val", _edje_embryo_fn_get_state_val);

   embryo_program_native_call_add(ep, "part_swallow", _edje_embryo_fn_part_swallow);

//   embryo_program_vm_push(ed->collection->script);
//   _edje_embryo_globals_init(ed);
}

void
_edje_embryo_script_shutdown(Edje *ed)
{
   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   if (embryo_program_recursion_get(ed->collection->script) > 0) return;
//   embryo_program_vm_pop(ed->collection->script);
   embryo_program_free(ed->collection->script);
   ed->collection->script = NULL;
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
_edje_embryo_test_run(Edje *ed, const char *fname, const char *sig, const char *src)
{
   Embryo_Function fn;

   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   embryo_program_vm_push(ed->collection->script);
   _edje_embryo_globals_init(ed);

   //   _edje_embryo_script_reset(ed);
   fn = embryo_program_function_find(ed->collection->script, (char *)fname);
   if (fn != EMBRYO_FUNCTION_NONE)
     {
	void *pdata;
	int ret;

	embryo_parameter_string_push(ed->collection->script, (char *)sig);
	embryo_parameter_string_push(ed->collection->script, (char *)src);
	pdata = embryo_program_data_get(ed->collection->script);
	embryo_program_data_set(ed->collection->script, ed);
	/* 5 million instructions is an arbitary number. on my p4-2.6 here */
	/* IF embryo is ONLY runing embryo stuff and NO native calls thats */
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
	ret = embryo_program_run(ed->collection->script, fn);
	if (ret == EMBRYO_PROGRAM_FAIL)
	  {
	     printf("EDJE:        ERROR with embryo script.\n"
		    "ENTRY POINT: %s\n"
		    "ERROR:       %s\n",
		    fname,
		    embryo_error_string_get(embryo_program_error_get(ed->collection->script)));
	  }
	else if (ret == EMBRYO_PROGRAM_TOOLONG)
	  {
	     printf("EDJE:        ERROR with embryo script.\n"
		    "ENTRY POINT: %s\n"
		    "ERROR:       Script exceeded maximum allowed cycle count of %i\n",
		    fname,
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
