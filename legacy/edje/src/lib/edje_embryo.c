#include "Edje.h"
#include "edje_private.h"

/* 
 * ALREADY EXPORTED BY EMBRYO:
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
 */

#define CHKPARAM(n) if (params[0] != (sizeof(Embryo_Cell) * (n))) return 0;
#define GETSTR(str, par) { \
   Embryo_Cell *___cptr; \
   int ___l; \
   if ((___cptr = embryo_data_address_get(ep, (par)))) { \
   ___l = embryo_data_string_length_get(ep, ___cptr); \
   if (((str) = alloca(___l + 1))) \
   embryo_data_string_get(ep, ___cptr, (str));}}
#define SETSTR(str, par) { \
   Embryo_Cell *___cptr; \
   if ((___cptr = embryo_data_address_get(ep, (par)))) { \
   embryo_data_string_set(ep, str, ___cptr);}}

static void _edje_embryo_globals_init(Edje *ed);

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
	     
	     ss = strdup(s);
	     if (ss)
	       {
		  ss[params[3] - 1] = 0;
		  SETSTR(ss, params[2]);
		  free(ss);
	       }
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


/* EDJE...
 * 
 * implemented so far as examples:
 * 
 * emit(sig[], src[])
 * set_state(part_id, state[], Float:state_val)
 * set_tween_state(part_id, Float:tween, state1[], Float:state1_val, state2[], Float:state2_val)
 * run_program(program_id)
 * 
 * still need to implement this:
 * 
 * stop_program(program_id)
 * stop_programs_on(part_id)
 * Direction:get_drag_dir(part_id)
 * get_drag(part_id, &Float:dx, &Float:&dy)
 * set_drag(part_id, Float:dx, Float:dy)
 * get_drag_step(part_id, &Float:dx, &Float:&dy)
 * set_drag_step(part_id, Float:dx, Float:dy)
 * get_drag_page(part_id, &Float:dx, &Float:&dy)
 * set_drag_page(part_id, Float:dx, Float:dy)
 * get_drag_count(part_id, &Float:dx, &Float:&dy)
 * set_drag_count(part_id, Float:dx, Float:dy)
 * set_drag_confine(part_id, confine_part_id)
 * text_set(part_id, str[])
 * text_get(part_id, dst[], maxlen)
 * get_min_size(w, h)
 * set_min_size(&w, &h)
 * get_max_size(w, h)
 * set_max_size(&w, &h)
 * get_size(&w, &h)
 * resize_request(w, h)
 * get_mouse(&x, &y)
 * get_mouse_buttons()
 * set_color_class(class[], r, g, b, a)
 * get_color_class(class[], &r, &g, &b, &a)
 * set_text_class(class[], font[], Float:size)
 * get_text_class(class[], font[], &Float:size)
 * //set_type(part_id, Type:type)
 * //set_effect(part_id, Effect:fx)
 * set_mouse_events(part_id, ev)
 * get_mouse_events(part_id)
 * set_repeat_events(part_id, rep)
 * get_repeat_events(part_id)
 * set_clip(part_id, clip_part_id)
 * get_clip(part_id)
 */

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
     }
   return 0;
}

/* run_program(program_id) */
static Embryo_Cell
_edje_embryo_fn_run_program(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   char *state1 = NULL, *state2 = NULL;
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

/* MODIFY STATE VALUES
 * 
 * set_state_val(part_id, state[], Float:state_val, Param:param, ...)
 * get_state_val(part_id, state[], Float:state_val, Param:param, ...)
 * 
 * FOR THESE PROPERTIES:
 * 
 * visible
 * align[x,y]
 * min[w,h]
 * max[w,h]
 * step[x,y]
 * aspect[min,max]
 * rel1[rx,ry,part_id,part_id]
 * rel1[x,y]
 * rel2[rx,ry,part_id,part_id]
 * rel2[x,y]
 * image[id]
 * image[tween_list...] (get???)
 * border[l,r,t,b]
 * fill[smooth]
 * fill[pos_rx,pos_ry,pos_x,pos_y]
 * fill[sz_rx,sz_ry,sz_x,sz_y]
 * color_class
 * color[r,g,b,a]
 * color2[r,g,b,a]
 * color3[r,g,b,a]
 * text[text_class]
 * text[font]
 * text[size]
 * text[fit_x,fit_y]
 * text[min_x,min_y]
 * text[align_x,align_y]
 */

/* FUTURE: KEYS???
 * 
 */

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

   embryo_program_native_call_add(ep, "timer", _edje_embryo_fn_timer);
   embryo_program_native_call_add(ep, "cancel_timer", _edje_embryo_fn_cancel_timer);
   
   embryo_program_native_call_add(ep, "anim", _edje_embryo_fn_anim);
   embryo_program_native_call_add(ep, "cancel_anim", _edje_embryo_fn_cancel_anim);
   
   embryo_program_native_call_add(ep, "emit", _edje_embryo_fn_emit);
   embryo_program_native_call_add(ep, "set_state", _edje_embryo_fn_set_state);
   embryo_program_native_call_add(ep, "set_tween_state", _edje_embryo_fn_set_tween_state);
   embryo_program_native_call_add(ep, "run_program", _edje_embryo_fn_run_program);
   
   embryo_program_vm_push(ep); /* neew a new vm to run in */
   _edje_embryo_globals_init(ed);
}

void
_edje_embryo_script_shutdown(Edje *ed)
{
   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   if (embryo_program_recursion_get(ed->collection->script) > 0) return;
   embryo_program_vm_pop(ed->collection->script);
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
_edje_embryo_test_run(Edje *ed, char *fname, char *sig, char *src)
{
   Embryo_Function fn;
   
   if (!ed) return;
   if (!ed->collection) return;
   if (!ed->collection->script) return;
   _edje_embryo_script_reset(ed);
   fn = embryo_program_function_find(ed->collection->script, fname);
   if (fn != EMBRYO_FUNCTION_NONE)
     {
	printf("EDJE DEBUG: About to run script from progrqm.\n");
	embryo_parameter_string_push(ed->collection->script, sig);
	embryo_parameter_string_push(ed->collection->script, src);
	embryo_program_run(ed->collection->script, fn);
	printf("EDJE DEBUG: Done.\n");
     }
}

static void
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
