#include "Edje.h"
#include "edje_private.h"

#define CHKPARAM(n) if (params[0] != (sizeof(Embryo_Cell) * (n))) return 0;

/**** All the api exported to edje scripts ****/
/* tst() */
static Embryo_Cell
_edje_embryo_fn_tst(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   
   /* params[0] = number of bytes of params passed */
   ed = embryo_program_data_get(ep);
   printf("EDJE DEBUG: Embryo code detected for \"%s\":\"%s\"\n",
	  ed->path, ed->part);
   return 7;
}

/* emit(sig[], src[]) */
static Embryo_Cell
_edje_embryo_fn_emit(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed;
   Embryo_Cell *cptr;
   char *sig, *src;
   int l;
   
   CHKPARAM(2);
   ed = embryo_program_data_get(ep);
   
   cptr = embryo_data_address_get(ep, params[1]);
   l = embryo_data_string_length_get(ep, cptr);
   sig = alloca(l + 1);
   embryo_data_string_get(ep, cptr, sig);
   
   cptr = embryo_data_address_get(ep, params[2]);
   l = embryo_data_string_length_get(ep, cptr);
   src = alloca(l + 1);
   embryo_data_string_get(ep, cptr, src);
   
   _edje_emit(ed, sig, src);
   return 0;
}

/* 
 * ALREADY EXPORTEd By EMBRYO:
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
 * 
 * ROUTINES TO EXPORT:
 * 
 * BASIC NUTS & BOLTS
 * 
 * get_int(key[])
 * set_int(key[], val)
 * get_float(key[])
 * set_float(key[], Float:val)
 * get_str(key[], dst[], maxlen)
 * set_str(key[], str[])
 * 
 * TIMERS... (tick off in N seconds from now)
 *
 * timer(Float:in, fname[], val)
 * cancel_timer(id)
 * 
 * ANIMATORS... (run for N seconds, passing in position)
 * 
 * anim(Float:length, fname[], ...) (varargs = series of int's - no strings))
 * candel_anim(id);
 * 
 * EDJE...
 * 
 * set_state(part[], state[], Float:state_val)
 * set_tween_state(part[], state1[], Float:state1_val, state2[], Float:state2_val)
 * emit(sig[], src[])
 * run_program(name[])
 * stop_program(name[])
 * stop_programs_on(name[])
 * Direction:get_drag_dir(name[])
 * get_drag(name[], &Float:dx, &Float:&dy)
 * set_drag(name[], Float:dx, Float:dy)
 * get_drag_step(name[], &Float:dx, &Float:&dy)
 * set_drag_step(name[], Float:dx, Float:dy)
 * get_drag_page(name[], &Float:dx, &Float:&dy)
 * set_drag_page(name[], Float:dx, Float:dy)
 * get_drag_count(name[], &Float:dx, &Float:&dy)
 * set_drag_count(name[], Float:dx, Float:dy)
 * set_drag_confine(name[], confine_name[])
 * text_set(name[], str[])
 * text_get(name[], dst[], maxlen)
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
 * //set_type(name[], Type:type)
 * //set_effect(name[], Effect:fx)
 * set_mouse_events(name[], ev)
 * get_mouse_events(name[])
 * set_repeat_events(name[], rep)
 * get_repeat_events(name[])
 * set_clip(name[], clip_name[])
 * get_clip(name[], clip_name_dst[], clip_name_dst_max)
 * 
 * MODIFY STATE VALUES
 * 
 * set_state_val(name[], state[], Float:state_val, Param:param, ...)
 * get_state_val(name[], state[], Float:state_val, Param:param, ...)
 * 
 * for these:
 * 
 * visible
 * align[x,y]
 * min[w,h]
 * max[w,h]
 * step[x,y]
 * aspect[min,max]
 * rel1[rx,ry,namex[],namey[]]
 * rel1[x,y]
 * rel2[rx,ry,namex[],namey[]]
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
 * 
 * FUTURE: KEYS???
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
   embryo_program_native_call_add(ep, "tst", _edje_embryo_fn_tst);
   embryo_program_native_call_add(ep, "emit", _edje_embryo_fn_emit);
   
   embryo_program_vm_push(ep); /* neew a new vm to run in */
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
}

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
