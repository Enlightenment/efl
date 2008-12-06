/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

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
      Eina_Hash *hash; // FIXME: hash -> bad. too big. one-way lookup etc.
   } oid;
};

static void _call_fn(Edje *ed, const char *fname, Embryo_Function fn);

/* frankly - these make the code shorter to type and read - just sanity for
 * development */
#define IFFN(func) if (si->fn.func != EMBRYO_FUNCTION_NONE)
#define IFNO(func) if (si->fn.func == EMBRYO_FUNCTION_NONE)
#define CLFN(func) IFFN(func) {_call_fn(ed, #func, si->fn.func);}
#define SI Sinfo *si; si = ed->script_only_data; if (!si) return
#define SI_RETURN(ret) Sinfo *si; si = ed->script_only_data; if (!si) return (ret)
#define PINT(val) embryo_parameter_cell_push(ed->collection->script, (Embryo_Cell)(val))
#define PSTR(val) embryo_parameter_string_push(ed->collection->script, val)
#define GTFN(func) si->fn.func = embryo_program_function_find(ed->collection->script, #func)
#define DELJ(type) if (si->job.type) ecore_job_del(si->job.type);
#define ADDJ(type, func) si->job.type = ecore_job_add(func, ed);
#define ZERJ(type) si->job.type = NULL;
#define IFNJ(type) if (!si->job.type)
#define EXPF(func) embryo_program_native_call_add(ed->collection->script, #func, _exp_##func)

typedef struct _Oid Oid;

struct _Oid
{
   Edje *ed;
   Evas_Object *obj;
   Evas_Coord x, y, w, h;
   int oid;
};

/* FIXME: using evas_hash and strings is just nasty! make a custom int hash */
static int
_oid_alloc(Edje *ed)
{
   SI_RETURN(0);
   
   si->oid.id++;
   return si->oid.id;
}

static Oid *
_oid_track(Edje *ed, Evas_Object *o)
{
   Oid *oi;
   char buf[64];
   SI_RETURN(NULL);
   
   oi = calloc(1, sizeof(Oid));
   if (!oi) return NULL;
   oi->oid = _oid_alloc(ed);
   if (!oi->oid)
     {
	free(oi);
	return NULL;
     }
   oi->ed = ed;
   oi->obj = o;
   evas_object_smart_member_add(oi->obj, oi->ed->obj);
   evas_object_clip_set(oi->obj, oi->ed->clipper);
   evas_object_geometry_get(oi->obj, &(oi->x), &(oi->y), &(oi->w), &(oi->h));
   snprintf(buf, sizeof(buf), "%i", oi->oid);
   si->oid.hash = evas_hash_add(si->oid.hash, buf, oi);
   return oi;
}

static Oid *
_oid_find(Edje *ed, int oid)
{
   char buf[64];
   SI_RETURN(NULL);

   snprintf(buf, sizeof(buf), "%i", oid);
   return evas_hash_find(si->oid.hash, buf);
}

static void
_oid_del(Edje *ed, int oid)
{
   char buf[64];
   SI;

   snprintf(buf, sizeof(buf), "%i", oid);
   si->oid.hash = evas_hash_del(si->oid.hash, buf, NULL);
}

static void
_oid_free(Oid *oid)
{
   free(oid);
}

static Eina_Bool
_oid_freeall_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata)
{
   Oid *oid = data;
   evas_object_del(oid->obj);
   free(oid);
   return 1;
}

static void
_oid_freeall(Edje *ed)
{
   SI;
   if (!si->oid.hash) return;
   evas_hash_foreach(si->oid.hash, _oid_freeall_cb, ed);
   evas_hash_free(si->oid.hash);
   si->oid.hash = NULL;
}

static Eina_Bool
_oid_moveall_cb(const Eina_Hash *hash, const void *key, void *data, void *fdata)
{
   Oid *oid = data;
   evas_object_move(oid->obj, oid->ed->x + oid->x, oid->ed->y + oid->y);
   return 1;
}

static void
_oid_moveall(Edje *ed)
{
   SI;
   if (!si->oid.hash) return;
   evas_hash_foreach(si->oid.hash, _oid_moveall_cb, ed);
}

/**********/

static Embryo_Cell
_exp_e_obj_del(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Oid *oid;
   SI_RETURN(-1);

   CHKPARAM(1);
   if (!(oid = _oid_find(ed, params[1]))) return -1;
   evas_object_del(oid->obj);
   _oid_del(ed, oid->oid);
   _oid_free(oid);
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_rect_add(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Evas_Object *o;
   Oid *oid;
   SI_RETURN(-1);
   
   o = evas_object_rectangle_add(evas_object_evas_get(ed->obj));
   if (!o) return 0;
   oid = _oid_track(ed, o);
   if (oid) return oid->oid;
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_show(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Oid *oid;
   SI_RETURN(-1);

   CHKPARAM(1);
   if (!(oid = _oid_find(ed, params[1]))) return -1;
   evas_object_show(oid->obj);
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_hide(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Oid *oid;
   SI_RETURN(-1);

   CHKPARAM(1);
   if (!(oid = _oid_find(ed, params[1]))) return -1;
   evas_object_hide(oid->obj);
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_move(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Oid *oid;
   SI_RETURN(-1);
   
   CHKPARAM(3);
   if (!(oid = _oid_find(ed, params[1]))) return -1;
   if ((oid->x == params[2]) && (oid->y == params[3])) return -1;
   oid->x = params[2];
   oid->y = params[3];
   evas_object_move(oid->obj, ed->x + oid->x, ed->y + oid->y);
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_resize(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Oid *oid;
   SI_RETURN(-1);
   
   CHKPARAM(3);
   if (!(oid = _oid_find(ed, params[1]))) return -1;
   if ((oid->w == params[2]) && (oid->h == params[3])) return -1;
   oid->w = params[2];
   oid->h = params[3];
   evas_object_resize(oid->obj, oid->w, oid->h);
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_geometry_set(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Oid *oid;
   SI_RETURN(-1);
   
   CHKPARAM(5);
   if (!(oid = _oid_find(ed, params[1]))) return -1;
   if ((oid->x == params[2]) && (oid->y == params[3]) &&
       (oid->w == params[4]) && (oid->h == params[5])) return -1;
   oid->x = params[2];
   oid->y = params[3];
   oid->w = params[4];
   oid->h = params[5];
   evas_object_move(oid->obj, ed->x + oid->x, ed->y + oid->y);
   evas_object_resize(oid->obj, oid->w, oid->h);
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_geometry_get(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Oid *oid;
   SI_RETURN(-1);
   
   CHKPARAM(5);
   if (!(oid = _oid_find(ed, params[1]))) return -1;
   SETINT(oid->x, params[2]);
   SETINT(oid->y, params[3]);
   SETINT(oid->w, params[4]);
   SETINT(oid->h, params[5]);
   return 0;
}
    
static Embryo_Cell
_exp_e_obj_color_set(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Oid *oid;
   SI_RETURN(-1);

   CHKPARAM(5);
   if (!(oid = _oid_find(ed, params[1]))) return -1;
   evas_object_color_set(oid->obj, params[2], params[3], params[4], params[5]);
   return 0;
}

static Embryo_Cell
_exp_e_obj_color_get(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   Oid *oid;
   int r, g, b, a;
   SI_RETURN(-1);

   CHKPARAM(5);
   if (!(oid = _oid_find(ed, params[1]))) return -1;
   evas_object_color_get(oid->obj, &r, &g , &b, &a);
   SETINT(r, params[2]);
   SETINT(g, params[3]);
   SETINT(b, params[4]);
   SETINT(a, params[5]);
   return 0;
}

static Embryo_Cell
_exp_e_signal_emit(Embryo_Program *ep, Embryo_Cell *params)
{
   Edje *ed = embryo_program_data_get(ep);
   char *sig = NULL, *src = NULL;
   SI_RETURN(-1);
   
   CHKPARAM(2);
   GETSTR(sig, params[1]);
   GETSTR(src, params[2]);
   if ((!sig) || (!src)) return -1;
   _edje_emit(ed, sig, src);
   return 0;
}

/**********/

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
   Sinfo *si;
   
   si = calloc(1, sizeof(Sinfo));
   if (!si) return;
   ed->script_only_data = si;

   embryo_program_data_set(ed->collection->script, ed);
   
   EXPF(e_obj_del);
   EXPF(e_obj_rect_add);
   EXPF(e_obj_show);
   EXPF(e_obj_hide);
   EXPF(e_obj_move);
   EXPF(e_obj_resize);
   EXPF(e_obj_geometry_set);
   EXPF(e_obj_geometry_get);
   EXPF(e_obj_color_set);
   EXPF(e_obj_color_get);
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

void
_edje_script_only_shutdown(Edje *ed)
{
   SI;

   CLFN(obj_shutdown);
   DELJ(show);
   DELJ(hide);
   DELJ(move);
   DELJ(resize);
   _oid_freeall(ed);
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
   
   _oid_moveall(ed);
   ZERJ(move);
   IFNO(obj_move) return;
   PINT(ed->x);
   PINT(ed->y);
   CLFN(obj_move);
}
void
_edje_script_only_move(Edje *ed)
{
   SI;

   DELJ(move);
   ADDJ(move, _move_job);
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

