/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Edje.h"
#include <Ecore_Job.h>
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

typedef struct _Sinfo Sinfo;

struct _Sinfo
{
   struct {
      Embryo_Function
	obj_init, obj_shutdown, obj_show, obj_show_immediate,
	obj_hide, obj_hide_immediate, obj_move, obj_move_immediate,
	obj_resize, obj_resize_immediate, obj_message
	;
   } fn;
   struct {
      Ecore_Job 
	*show, *hide, *move, *resize
	;
   } job;
   struct {
      int id;
      Evas_Hash *hash; // FIXME: hash -> bad. too big. one-way lookup etc.
   } oid;
};

static void _call_fn(Edje *ed, const char *fname, Embryo_Function fn);

/* frankly - these make the code shorter to type and read - just sanity for
 * development */
#define IFFN(func) if (si->fn.func != EMBRYO_FUNCTION_NONE)
#define IFNO(func) if (si->fn.func == EMBRYO_FUNCTION_NONE)
#define CLFN(func) IFFN(func) {_call_fn(ed, #func, si->fn.func);}
#define SI Sinfo *si; si = ed->script_only_data; if (!si) return
#define PINT(val) embryo_parameter_cell_push(ed->collection->script, (Embryo_Cell)(val))
#define PSTR(val) embryo_parameter_string_push(ed->collection->script, val)
#define GTFN(func) si->fn.func = embryo_program_function_find(ed->collection->script, #func)
#define DELJ(type) if (si->job.type) ecore_job_del(si->job.type);
#define ADDJ(type, func) si->job.type = ecore_job_add(func, ed);
#define ZERJ(type) si->job.type = NULL;
#define IFNJ(type) if (!si->job.type)
#define EXPF(func) embryo_program_native_call_add(ed->collection->script, #func, _exp_##func)

/* FIXME: need an oid -> ptr translation here. this is a hack. id # is linear\
 * and runs out after 2billion uses or so. the hash is fat if all u have is
 * a few objects... doesn't know how to do anything but evas_object as it
 * will evas_object_del at the end... so for now this is a testing stub. */
static int
_oid_track(Edje *ed, void *o)
{
   char buf[64];
   SI;
   
   si->oid.id++;
   snprintf(buf, sizeof(buf), "%i", si->oid.id);
   si->oid.hash = evas_hash_add(si->oid.hash, buf, o);
   return si->oid.id;
}

static void *
_oid_find(Edje *ed, int oid)
{
   char buf[64];
   SI;
   
   snprintf(buf, sizeof(buf), "%i", si->oid.id);
   return evas_hash_find(si->oid.hash, buf);
}

static void
_oid_del(Edje *ed, int oid)
{
   char buf[64];
   SI;

   snprintf(buf, sizeof(buf), "%i", si->oid.id);
   si->oid.hash = evas_hash_del(si->oid.hash, buf, NULL);
}

static Embryo_Cell
_exp_e_obj_rect_add(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Evas_Object *o;
   int oid;
   SI;
   
   o = evas_object_rectangle_add(evas_object_evas_get(ed->obj));
   if (!o) return 0;
   evas_object_smart_member_add(o, ed->obj);
   evas_object_clip_set(o, ed->clipper);
   _oid_track(ed, o);
   return oid;
}
    
static Embryo_Cell
_exp_e_obj_show(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Evas_Object *o;
   SI;

   CHKPARAM(1);
   if (!(o = _oid_find(ed, params[1]))) return -1;
   evas_object_show(o);
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_move(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Evas_Object *o;
   SI;
   
   CHKPARAM(3);
   if (!(o = _oid_find(ed, params[1]))) return -1;
   evas_object_move(o, ed->x + params[2], ed->y + params[3]);
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_resize(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Evas_Object *o;
   SI;
   
   CHKPARAM(3);
   if (!(o = _oid_find(ed, params[1]))) return -1;
   evas_object_resize(o, params[2], params[3]);
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_color_set(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Evas_Object *o;
   SI;

   CHKPARAM(5);
   if (!(o = _oid_find(ed, params[1]))) return -1;
   evas_object_color_set(o, params[2], params[3], params[4], params[5]);
   return 0;
}

static Embryo_Cell
_exp_e_signal_emit(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   char *sig = NULL, *src = NULL;
   SI;
   
   CHKPARAM(2);
   GETSTR(sig, params[1]);
   GETSTR(src, params[2]);
   if ((!sig) || (!src)) return -1;
   _edje_emit(ed, sig, src);
   return 0;
}

int
_edje_script_only(Edje *ed)
{
   if ((ed->collection) && (ed->collection->script) &&
       (ed->collection->script_only))
     return 1;
   return 0;
}

void
_edje_script_only_init(Edje *ed)
{
   Embryo_Function fn;
   Sinfo *si;
   
   si = calloc(1, sizeof(Sinfo));
   if (!si) return;
   ed->script_only_data = si;

   embryo_program_data_set(ed->collection->script, ed);
   
   EXPF(e_obj_rect_add);
   EXPF(e_obj_show);
   EXPF(e_obj_move);
   EXPF(e_obj_resize);
   EXPF(e_obj_color_set);
   EXPF(e_signal_emit);
   
   embryo_program_vm_push(ed->collection->script);
   embryo_program_max_cycle_run_set(ed->collection->script, 5000000);

   GTFN(obj_init);
   GTFN(obj_shutdown);
   GTFN(obj_show);
   GTFN(obj_show_immediate);
   GTFN(obj_hide);
   GTFN(obj_hide_immediate);
   GTFN(obj_move);
   GTFN(obj_move_immediate);
   GTFN(obj_resize);
   GTFN(obj_resize_immediate);
   GTFN(obj_message);

   CLFN(obj_init);
   _edje_script_only_move(ed);
}

static Evas_Bool
_shutdown_oid_hash(const Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   evas_object_del(data);
   return 1;
}
void
_edje_script_only_shutdown(Edje *ed)
{
   Embryo_Function fn;
   SI;

   CLFN(obj_shutdown);
   DELJ(show);
   DELJ(hide);
   DELJ(move);
   DELJ(resize);
   if (si->oid.hash)
     {
	evas_hash_foreach(si->oid.hash, _shutdown_oid_hash, ed);
	evas_hash_free(si->oid.hash);
     }
}

static void
_show_job(void *data)
{
   Edje *ed = data;
   SI;
   
   ZERJ(show);
   CLFN(obj_show);
}
void
_edje_script_only_show(Edje *ed)
{
   SI;
   
   IFFN(obj_show)
     {
	IFNJ(hide)
	  {
	     DELJ(show);
	     ADDJ(show, _show_job);
	  }
	else
	  {
	     DELJ(hide);
	  }
     }
   IFNO(obj_show_immediate) return;
   CLFN(obj_show_immediate);
}

static void
_hide_job(void *data)
{
   Edje *ed = data;
   SI;
   
   ZERJ(hide);
   CLFN(obj_hide);
}
void
_edje_script_only_hide(Edje *ed)
{
   SI;
   
   IFFN(obj_hide)
     {
	IFNJ(show)
	  {
	     DELJ(hide);
	     ADDJ(hide, _hide_job);
	  }
	else
	  {
	     DELJ(show);
	  }
     }
   IFNO(obj_hide_immediate) return;
   CLFN(obj_hide_immediate);
}

static void
_move_job(void *data)
{
   Edje *ed = data;
   SI;
   
   ZERJ(move);
   PINT(ed->x);
   PINT(ed->y);
   CLFN(obj_move);
}
void
_edje_script_only_move(Edje *ed)
{
   SI;
   
   if ((si->fn.obj_move == EMBRYO_FUNCTION_NONE) &&
       (si->fn.obj_move_immediate == EMBRYO_FUNCTION_NONE))
     {
	/* FIXME: auto-move sub-objects. this means we need to store in-edje
	 * x,y and other stuff... */
	return;
     }
   IFFN(obj_move)
     {
	DELJ(move);
	ADDJ(move, _move_job);
     }
   IFNO(obj_move_immediate) return;
   PINT(ed->x);
   PINT(ed->y);
   CLFN(obj_move_immediate);
}

static void
_resize_job(void *data)
{
   Edje *ed = data;
   SI;
   
   ZERJ(resize);
   PINT(ed->w);
   PINT(ed->h);
   CLFN(obj_resize);
}
void
_edje_script_only_resize(Edje *ed)
{
   SI;

   IFFN(obj_resize)
     {
	DELJ(resize);
	ADDJ(resize, _resize_job);
     }
   PINT(ed->w);
   PINT(ed->h);
   CLFN(obj_resize_immediate);
}

void
_edje_script_only_message(Edje *ed, Edje_Message *em)
{
   SI;
   
   IFNO(obj_message) return;
   _edje_message_parameters_push(em);
   CLFN(obj_message);
}

/**************************************************/

static void
_call_fn(Edje *ed, const char *fname, Embryo_Function fn)
{
   int ret;
   
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
}

