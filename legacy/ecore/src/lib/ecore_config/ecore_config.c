/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#include "Ecore_Config.h"
#include "ecore_config_private.h"
#include "ecore_config_ipc.h"

#include "ecore_config_util.h"

int                  DEBUG = 0;
EAPI Ecore_Config_Server *__ecore_config_server_global = NULL;
EAPI Ecore_Config_Server *__ecore_config_server_local = NULL;
EAPI Ecore_Config_Bundle *__ecore_config_bundle_local = NULL;
EAPI char                *__ecore_config_app_name = NULL;
int                  __ecore_config_system_init = 0;

static int           _ecore_config_system_init_no_load(void);
static int           _ecore_config_system_load(void);

static inline void  *__ecore_argb_to_long(int a, int r, int g, int b, long *v);
static inline void  *__ecore_argbstr_to_long(const char *argb, long *v);

static const char  *_ecore_config_type[] =
   { "undefined", "integer", "float", "string", "colour", "theme", "boolean", "structure" };

/**
 * @defgroup Ecore_Config_Property_Group Ecore Config Property Functions
 *
 * Functions that retrieve or set the attributes relating to a property.
 */

/**
 * Removes the given property from the local configuration and destroys it.
 * @param   e Property to destroy.
 * @return  @c NULL
 * @ingroup Ecore_Config_Property_Group
 */
EAPI Ecore_Config_Prop  *
ecore_config_dst(Ecore_Config_Prop * e)
{
   Ecore_Config_Bundle *t;
   Ecore_Config_Prop  *p, *c;
   Ecore_Config_Listener_List *l;

   p = NULL;
   c = e;
   t = __ecore_config_bundle_local;

   if (!e || !e->key)
      return NULL;
   if (t)
     {
	if (t->data == e)
	   t->data = e->next;
	else
	  {
	     do
	       {
		  p = c;
		  c = c->next;
	       }
	     while (c && (c != e));
	     if (c)		/* remove from list if even in there */
		p->next = c->next;
	  }
     }

   while (e->listeners)
     {
	l = e->listeners;
	e->listeners = e->listeners->next;
	free(l);
     }

   if (e->key)
      free(e->key);
   if (e->ptr && ((e->type == ECORE_CONFIG_STR) || (e->type == ECORE_CONFIG_THM)))
      free(e->ptr);

   memset(e, 0, sizeof(Ecore_Config_Prop));
   free(e);

   return NULL;
}

/**
 * @defgroup Ecore_Config_Get_Group Configuration Retrieve Functions
 *
 * Functions that retrieve configuration values, based on type.
 */

/**
 * Returns the property with the given key.
 * @param   key The unique name of the wanted property.
 * @return  The property that corresponds to the given key.  @c NULL if the
 *          key could not be found.
 * @ingroup Ecore_Config_Get_Group
 */
EAPI Ecore_Config_Prop  *
ecore_config_get(const char *key)
{
   Ecore_Config_Bundle *t;
   Ecore_Config_Prop  *e;

   t = __ecore_config_bundle_local;
   if (!t || !key)
      return NULL;
   e = t->data;
   while (e)
     {
	if (!strcmp(key, e->key))
	   return e;
	e = e->next;
     }
   return NULL;
}

/**
 * Returns the type of the property.
 * @param   e Property to get the type of.
 * @returns The type of the property.  If the property is invalid, then the
 *          string "not found" is returned.
 * @ingroup Ecore_Config_Property_Group
 */
EAPI const char         *
ecore_config_type_get(const Ecore_Config_Prop * e)
{
   if (e)
     {
	return _ecore_config_type[e->type];
     }
   return "not found";
}

/**
 * Returns the specified property as a string.
 * @param   key The property key.
 * @return  The string value of the property.  The function returns @c NULL if
 *          the property is not a string or is not set.
 * @ingroup Ecore_Config_Get_Group
 */
EAPI char               *
ecore_config_string_get(const char *key)
{
   return _ecore_config_string_get( ecore_config_get(key) );
}

char               *
_ecore_config_string_get(Ecore_Config_Prop *e)
{
   return (e && (e->type == ECORE_CONFIG_STR) && e->ptr) ? strdup(e->ptr) : NULL;
}

/**
 * Returns the specified property as an integer.
 * @param   key The property key.
 * @return  The value of the property.  The function returns -1 if the
 *          property is not an integer or is not set.
 * @ingroup Ecore_Config_Get_Group
 */
EAPI int
ecore_config_boolean_get(const char *key)
{
   return _ecore_config_boolean_get( ecore_config_get(key) );
}

int
_ecore_config_boolean_get(Ecore_Config_Prop *e)
{
   return (e && ((e->type == ECORE_CONFIG_INT) || (e->type == ECORE_CONFIG_BLN))) ? (e->val != 0) : -1;
}

/**
 * Returns the specified property as a long integer.
 * @param   key The property key.
 * @return  The integer value of the property.  The function returns 0 if the
 *          property is not an integer or is not set.
 * @ingroup Ecore_Config_Get_Group
 */
EAPI long
ecore_config_int_get(const char *key)
{
   return _ecore_config_int_get( ecore_config_get(key) );
}

long
_ecore_config_int_get(Ecore_Config_Prop *e)
{
   return (e && ((e->type == ECORE_CONFIG_INT) || (e->type == ECORE_CONFIG_RGB))) ? e->val : 0L;
}

/**
 * Returns the specified property as a float.
 * @param   key The property key.
 * @return  The float value of the property.  The function returns 0.0 if the 
 *          property is not a float or is not set.
 * @ingroup Ecore_Config_Get_Group
 */
EAPI float
ecore_config_float_get(const char *key)
{
   return _ecore_config_float_get( ecore_config_get(key) );
}

float
_ecore_config_float_get(Ecore_Config_Prop *e)
{
   return (e && (e->type == ECORE_CONFIG_FLT)) ? ((float)e->val / ECORE_CONFIG_FLOAT_PRECISION) : 0.0;
}

/**
 * Finds the alpha, red, green and blue values of a color property.
 * @param   key The property key.
 * @param   a   A pointer to an integer to store the alpha value into.
 * @param   r   A pointer to an integer to store the red value into.
 * @param   g   A pointer to an integer to store the green value into.
 * @param   b   A pointer to an integer to store the blue value into.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_FAIL
 *          otherwise.
 * @ingroup Ecore_Config_Get_Group
 */
EAPI int
ecore_config_argb_get(const char *key, int *a, int *r, int *g, int *b)
{
   return _ecore_config_argb_get( ecore_config_get(key), a, r, g, b);
}

int
_ecore_config_argb_get(Ecore_Config_Prop *e, int *a, int *r, int *g, int *b)
{
   if (e && ((e->type == ECORE_CONFIG_RGB)))
     {
	if(a) *a = (e->val >> 24) & 0xff;
	if(r) *r = (e->val >> 16) & 0xff;
	if(g) *g = (e->val >>  8) & 0xff;
	if(b) *b =  e->val        & 0xff;
	return ECORE_CONFIG_ERR_SUCC;
     }
   return ECORE_CONFIG_ERR_FAIL;
}

/**
 * Returns a color property as a long
 * @param   key The property key.
 * @return  ARGB data as long
 * @ingroup Ecore_Config_Get_Group
 */
EAPI long
ecore_config_argbint_get(const char *key)
{
   return _ecore_config_argbint_get( ecore_config_get(key) );
}

long
_ecore_config_argbint_get(Ecore_Config_Prop *e)
{
   if (e && ((e->type == ECORE_CONFIG_RGB)))
     {
	return e->val;
     }
   return 0L;
}

/**
 * Returns a color property as a string of hexadecimal characters.
 * @param   key The property key.
 * @return  A string of hexadecimal characters in the format #aarrggbb.
 * @ingroup Ecore_Config_Get_Group
 */
EAPI char               *
ecore_config_argbstr_get(const char *key)
{
   return _ecore_config_argbstr_get( ecore_config_get(key) );
}

char               *
_ecore_config_argbstr_get(Ecore_Config_Prop *e)
{
   char               *r;

   r = NULL;
   esprintf(&r, "#%08x", _ecore_config_int_get(e));
   return r;
}

/**
 * Returns a theme property.
 * @param   key The property key.
 * @return  The name of the theme the property refers to.  The function returns
 *          @c NULL if the property is not a theme or is not set.
 * @ingroup Ecore_Config_Get_Group
 */
EAPI char               *
ecore_config_theme_get(const char *key)
{
   return _ecore_config_theme_get( ecore_config_get(key) );
}

char               *
_ecore_config_theme_get(Ecore_Config_Prop *e)
{
   return (e && (e->type == ECORE_CONFIG_THM)) ? strdup(e->ptr) : NULL;
}

/**
 * Retrieves the key as a string.
 * @param   key The property key.
 * @return  Returns a character array in the form of 'key:type=value'.  @c NULL
 *          is returned if the property does not exist.
 * @ingroup Ecore_Config_Get_Group
 */
EAPI char               *
ecore_config_as_string_get(const char *key)
{
   Ecore_Config_Prop  *e;
   char               *val;
   char               *r;

   val = NULL;
   r = NULL;
   if (!(e = ecore_config_get(key)))
      E(0, "no such property, \"%s\"...\n", key);
   else
     {
	switch (e->type)
	   {
	     case ECORE_CONFIG_NIL:
		val = strdup("<nil>");
		break;
	     case ECORE_CONFIG_INT:
		esprintf(&val, "%ld",    _ecore_config_int_get(e));
		break;
	     case ECORE_CONFIG_BLN:
		esprintf(&val, "%ld",    _ecore_config_boolean_get(e));
		break;
	     case ECORE_CONFIG_FLT:
		esprintf(&val, "%lf",    _ecore_config_float_get(e));
		break;
	     case ECORE_CONFIG_STR:
		esprintf(&val, "\"%s\"", _ecore_config_string_get(e));
		break;
	     case ECORE_CONFIG_RGB:
		esprintf(&val, "#%08x",  _ecore_config_int_get(e));
		break;
	     case ECORE_CONFIG_THM:
		esprintf(&val, "\"%s\"", _ecore_config_theme_get(e));
		break;
	     case ECORE_CONFIG_SCT:
		break;
	     default:
		esprintf(&r, "%s:unknown_type", key);
		break;
	   }
	if (val)
	   {
	     esprintf(&r, "%s:%s=%s", key, _ecore_config_type[e->type], val);
	     free(val);
	   }
     }
   return r;
}

EAPI int
ecore_config_bound(Ecore_Config_Prop * e)
{
   int                 ret;
   long                v;

   ret = ECORE_CONFIG_ERR_SUCC;

   if (!e)
      return ECORE_CONFIG_ERR_FAIL;
   if (e->flags & ECORE_CONFIG_FLAG_BOUNDS)
     {
	if ((e->val < e->lo))
	  {
	     E(0,
	       "ecore_config_bounds(\"%s\",%ld): value out of range; adjusted to %ld...\n",
	       e->key, e->val, e->lo);
	     e->val = e->lo;
	  }
	else if ((e->val > e->hi))
	  {
	     E(0,
	       "ecore_config_bounds(\"%s\",%ld): value out of range; adjusted to %ld...\n",
	       e->key, e->val, e->hi);
	     e->val = e->hi;
	  }
	else
	   ret = ECORE_CONFIG_ERR_IGNORED;
     }
   else
      ret = ECORE_CONFIG_ERR_IGNORED;

   if (e->step)
     {
	v = ((int)(e->val / e->step)) * e->step;
	if (v != e->val)
	  {
	     if (e->type == ECORE_CONFIG_FLT)
		E(0,
		  "ecore_config_bound(\"%s\"): float value %f not a multiple of %f, adjusted to %f...\n",
		  e->key, ((double)e->val) / ECORE_CONFIG_FLOAT_PRECISION,
		  ((double)e->step) / ECORE_CONFIG_FLOAT_PRECISION,
		  ((double)v) / ECORE_CONFIG_FLOAT_PRECISION);
	     else
		E(0,
		  "ecore_config_bound(\"%s\"): integer value %ld not a multiple of %ld, adjusted to %ld...\n",
		  e->key, e->val, e->step, v);
	     ret = ECORE_CONFIG_ERR_SUCC;
	     e->val = v;
	  }
     }

   return ret;
}

/**
 * Tries to guess the type of a property.
 *
 * This function first checks to see if the property exists.  If it does, then
 * the type of the stored property is returned.  Otherwise, the function tries
 * to guess the type of the property based on @p val.
 *
 * @param  key The property key.
 * @param  val The value in string form.
 * @return The type of the property determined by the function.  Note that if
 *         val is @c NULL, @c ECORE_CONFIG_NIL will be returned.
 */
EAPI int
ecore_config_type_guess(const char *key, const char *val)
{
   Ecore_Config_Prop  *p;
   char               *l;
   long                v;

   l = NULL;

   if (key && (p = ecore_config_get(key)) && p->type != ECORE_CONFIG_NIL)
      return p->type;

   if (!val)
      return ECORE_CONFIG_NIL;
   if (val[0] == '#')
      return ECORE_CONFIG_RGB;
   v = strtol(val, &l, 10);
   if (*l)
     {
	float               f;

	if (sscanf(val, "%f%*s", &f) != 1)
	   return ECORE_CONFIG_STR;
	return ECORE_CONFIG_FLT;
     }
   return ECORE_CONFIG_INT;
}

static int
ecore_config_typed_val(Ecore_Config_Prop * e, const void *val, int type)
{

   if (!e)
     return ECORE_CONFIG_ERR_NODATA;

   if (!(val) && (type != ECORE_CONFIG_NIL && type != ECORE_CONFIG_SCT))
      e->ptr = NULL;
   else
     {
	if (type == ECORE_CONFIG_INT || type == ECORE_CONFIG_BLN)
	  {
	     e->val = (long) *((int *)val);
	     e->type = type;
	  }
	else if (type == ECORE_CONFIG_STR || type == ECORE_CONFIG_THM)
	  {
	     if (!(e->ptr = strdup(val)))
		return ECORE_CONFIG_ERR_OOM;
	     if (e->type == ECORE_CONFIG_NIL)
		e->type = type;
	  }
	else if (type == ECORE_CONFIG_RGB)
	  {
	     __ecore_argbstr_to_long((char *)val, &e->val);
	     e->type = ECORE_CONFIG_RGB;
	  }
	else if (type == ECORE_CONFIG_FLT)
	  {
	     e->val = (long) ((*((float *)val)) * ECORE_CONFIG_FLOAT_PRECISION);
	     e->type = ECORE_CONFIG_FLT;
	  }
	else if (type == ECORE_CONFIG_SCT)
	  {
	     e->type = ECORE_CONFIG_SCT;
	  }
	else
	  {
	   e->type = ECORE_CONFIG_NIL;
	  }

	ecore_config_bound(e);
	e->flags |= ECORE_CONFIG_FLAG_MODIFIED;
	e->flags &= ~ECORE_CONFIG_FLAG_CMDLN;
	return ECORE_CONFIG_ERR_SUCC;
     }
   return ECORE_CONFIG_ERR_IGNORED;
}

static int
ecore_config_typed_add(const char *key, const void *val, int type)
{
   int error = ECORE_CONFIG_ERR_SUCC;
   Ecore_Config_Prop  *e = NULL;
   Ecore_Config_Bundle *t;

   t = __ecore_config_bundle_local;
   if (!key)
      return ECORE_CONFIG_ERR_NODATA;

   if (!(e = calloc(1, sizeof(Ecore_Config_Prop))))
     {
	return ECORE_CONFIG_ERR_OOM;
     }
   else if (!(e->key = strdup(key)))
     {
	error = ECORE_CONFIG_ERR_OOM;
     }
   else if ((error = ecore_config_typed_val(e, val, type)) == ECORE_CONFIG_ERR_SUCC)
     {
	if (t)
	   {
	     e->next = t->data;
	     t->data = e;
	   }
	return ECORE_CONFIG_ERR_SUCC;
     }

   if(e->key)
     free(e->key);
   if(e)
     free(e);

   if (error == ECORE_CONFIG_ERR_SUCC)
      error = ECORE_CONFIG_ERR_FAIL;

   return error;
}

static int
ecore_config_add(const char *key, const char *val)
{
   int                 type;

   type = ecore_config_type_guess(key, val);
   return ecore_config_typed_add(key, val, type);
}

/**
 * Sets the description field of the indicated property.
 * @param   key  The property key.
 * @param   desc Description string.
 * @note    The description string is copied for the property's use.  You can
 *          free @p desc once this function is called.
 * @ingroup Ecore_Config_Property_Group
 */
EAPI int
ecore_config_describe(const char *key, const char *desc)
{
   Ecore_Config_Prop  *e;

   if (!(e = ecore_config_get(key)))
      return ECORE_CONFIG_ERR_NODATA;
   e->description = strdup(desc);
   return ECORE_CONFIG_ERR_SUCC;
}

/**
 * Set the short option character of a property.
 * @param   key       The property key.
 * @param   short_opt Character used to indicate the value of a property
 *                    given on the command line.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_NODATA
 *          is returned if the property does not exist.
 * @ingroup Ecore_Config_Property_Group
 */
EAPI int
ecore_config_short_opt_set(const char *key, char short_opt)
{
   Ecore_Config_Prop  *e;

   if (!(e = ecore_config_get(key)))
      return ECORE_CONFIG_ERR_NODATA;
   e->short_opt = short_opt;
   return ECORE_CONFIG_ERR_SUCC;
}

/**
 * Set the long option string of the property.
 * @param   key      The property key.
 * @param   long_opt String used to indicate the value of a property given
 *                   on the command line.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.  @c ECORE_CONFIG_ERR_NODATA
 *          is returned if the property does not exist.
 * @ingroup Ecore_Config_Property_Group
 */
EAPI int
ecore_config_long_opt_set(const char *key, const char *long_opt)
{
   Ecore_Config_Prop  *e;

   if (!(e = ecore_config_get(key)))
      return ECORE_CONFIG_ERR_NODATA;
   if (e->long_opt)
      free(e->long_opt);
   if (long_opt)
      e->long_opt = strdup(long_opt);
   return ECORE_CONFIG_ERR_SUCC;
}

static void
_ecore_config_listener_fire(Ecore_Config_Prop *prop)
{
   Ecore_Config_Listener_List *l;
   for (l = prop->listeners; l; l = l->next)
     l->listener(prop->key, prop->type, l->tag, l->data);

   /* fire change listeners for the generic struct container etc */
   if (prop->parent)
     _ecore_config_listener_fire(prop->parent);
}

/**
 * Sets the indicated property to the given value and type.
 * @param   key  The property key.
 * @param   val  A pointer to the value to set the property to.
 * @param   type The type of the property.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Property_Group
 */
EAPI int
ecore_config_typed_set(const char *key, const void *val, int type)
{
   Ecore_Config_Prop  *e;
   int                 ret;

   if (!key)
      return ECORE_CONFIG_ERR_NODATA;
/*  if (!t) { * global prop *
    e=ecore_config_get(key);
    if (e)
      for(l=e->listeners;l;l=l->next)
        l->listener(e->key,e->type,l->tag,l->data,t);
    return ECORE_CONFIG_ERR_SUCC;
  }
*/
   if (!(e = ecore_config_get(key)))
      return ecore_config_typed_add(key, val, type);

   if ((ret = ecore_config_typed_val(e, val, type)) == ECORE_CONFIG_ERR_SUCC)
     {
       _ecore_config_listener_fire(e);
     }
   else
     {
	E(0,
	  "ecore_config_typed_set(\"%s\"): ecore_config_typed_val() failed: %d\n",
	  key, ret);
     }

   return ret;
}

/**
 * @defgroup Ecore_Config_Set_Group Ecore Config Setters
 *
 * Functions that set the value of a property.
 */

/**
 * Sets the indicated property to the value indicated by @a val.
 * @param   key The property key.
 * @param   val String representation of value to set.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_set(const char *key, const char *val)
{
   int                 type;
   int                 tmpi;
   float               tmpf;
   long                tmpl;

   type = ecore_config_type_guess(key, val);
   if (type == ECORE_CONFIG_INT || type == ECORE_CONFIG_BLN)
     {
	tmpi = atoi(val);
	return ecore_config_typed_set(key, &tmpi, type);
     }
   else if (type == ECORE_CONFIG_FLT)
     {
	tmpf = atof(val);
	return ecore_config_typed_set(key, &tmpf, type);
     }
   else if (type == ECORE_CONFIG_RGB)
     {
	__ecore_argbstr_to_long(val, &tmpl);
	return ecore_config_typed_set(key, &tmpl, type);
     }
   else
      return ecore_config_typed_set(key, val, type);
}

/**
 * Sets the indicated property to the value given in the string.
 * @param   key The property key.
 * @param   val String representation of the value.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_as_string_set(const char *key, const char *val)
{
   return ecore_config_set(key, val);
}

/**
 * Sets the indicated property to the given boolean.
 * @param   key The property key.
 * @param   val Boolean integer to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_boolean_set(const char *key, int val)
{
   val = val ? 1 : 0;
   return ecore_config_typed_set(key, &val, ECORE_CONFIG_BLN);
}

/**
 * Sets the indicated property to the given integer.
 * @param   key The property key.
 * @param   val Integer to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_int_set(const char *key, int val)
{
   return ecore_config_typed_set(key, &val, ECORE_CONFIG_INT);
}

/**
 * Sets the indicated property to the given string.
 * @param   key The property key.
 * @param   val String to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_string_set(const char *key, const char *val)
{
   return ecore_config_typed_set(key, val, ECORE_CONFIG_STR);
}

/**
 * Sets the indicated property to the given float value.
 * @param   key The property key.
 * @param   val Float to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_float_set(const char *key, float val)
{
   return ecore_config_typed_set(key, &val, ECORE_CONFIG_FLT);
}

/**
 * Sets the indicated property to a color value.
 * @param   key The property key
 * @param   a integer 0..255
 * @param   r integer 0..255
 * @param   g integer 0..255
 * @param   b integer 0..255
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_argb_set(const char *key, int a, int r, int g, int b)
{
   long v = 0;
   return ecore_config_typed_set(key, __ecore_argb_to_long(a,r,g,b, &v), ECORE_CONFIG_RGB);
}

/**
 * Sets the indicated property to a color value.
 * @param   key The property key
 * @param   argb ARGB data as long
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_argbint_set(const char *key, long argb)
{
   return ecore_config_typed_set(key, &argb, ECORE_CONFIG_RGB);
}

/**
 * Sets the indicated property to a color value.
 * @param   key The property key
 * @param   val Color value in ARGB format.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_argbstr_set(const char *key, const char *val)
{
   long v = 0;
   return ecore_config_typed_set(key, __ecore_argbstr_to_long(val, &v), ECORE_CONFIG_RGB);
}

/**
 * Sets the indicated property to a theme name.
 * @param   key The property key.
 * @param   val String giving the name of the theme.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_theme_set(const char *key, const char *val)
{
   return ecore_config_typed_set(key, val, ECORE_CONFIG_THM);
}

/**
 * Sets the theme preview group of an indicated property.
 * @param   key   The property key.
 * @param   group The group name.
 * @return  @c ECORE_CONFIG_ERR_SUCC on success.
 * @ingroup Ecore_Config_Set_Group
 */
EAPI int
ecore_config_theme_preview_group_set(const char *key, const char *group)
{
   int                 ret;
   Ecore_Config_Prop  *e;

   ret = ECORE_CONFIG_ERR_SUCC;
   if (!(e = ecore_config_get(key)))
     {				/* prop doesn't exist yet */
	if ((ret = ecore_config_typed_add(key, "", ECORE_CONFIG_THM)) != ECORE_CONFIG_ERR_SUCC)	/* try to add it */
	   return ret;		/* ...failed */
	if (!(e = ecore_config_get(key)))	/* get handle */
	   return ECORE_CONFIG_ERR_FAIL;
     }
   if (e->data)
      free(e->data);
   e->data = strdup(group);

   return ret;
}

EAPI int
ecore_config_typed_default(const char *key, const void *val, int type)
{
   int                 ret;
   Ecore_Config_Prop  *e;

   ret = ECORE_CONFIG_ERR_SUCC;

   if (!(e = ecore_config_get(key)))
     {				/* prop doesn't exist yet */
	if ((ret = ecore_config_typed_add(key, val, type)) != ECORE_CONFIG_ERR_SUCC)	/* try to add it */
	   return ret;		/* ...failed */
	if (!(e = ecore_config_get(key)))	/* get handle */
	   return ECORE_CONFIG_ERR_FAIL;
	e->flags = e->flags & ~ECORE_CONFIG_FLAG_MODIFIED;
     }
   else if (!(e->flags & ECORE_CONFIG_FLAG_MODIFIED) && !(e->flags & ECORE_CONFIG_FLAG_SYSTEM))
     {
	ecore_config_typed_set(key, val, type);
	if (!(e = ecore_config_get(key)))       /* get handle */
	   return ECORE_CONFIG_ERR_FAIL;
	e->flags = e->flags & ~ECORE_CONFIG_FLAG_MODIFIED;
     }
   return ret;
}

/**
 * @defgroup Ecore_Config_Default_Group Ecore Config Defaults
 *
 * Functions that are used to set the default values of properties.
 */

/**
 * Sets the indicated property if it has not already been set or loaded.
 * @param   key  The property key.
 * @param   val  Default value of the key.
 * @param   lo   Lowest valid value for the key.
 * @param   hi   Highest valid value for the key.
 * @param   step Used by integer and float values.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there are no errors.
 * @note    The @p lo, @p hi and @p step parameters are only used when storing
 *          integer and float properties.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_default(const char *key, const char *val, float lo, float hi, float step)
{
   int                 ret, type;
   Ecore_Config_Prop  *e;

   type = ecore_config_type_guess(key, val);
   ret = ecore_config_typed_default(key, val, type);
   e = ecore_config_get(key);
   if (e)
     {
	if (type == ECORE_CONFIG_INT)
	  {
	     e->step = step;
	     e->flags |= ECORE_CONFIG_FLAG_BOUNDS;
	     e->lo = lo;
	     e->hi = hi;
	     ecore_config_bound(e);
	  }
	else if (type == ECORE_CONFIG_FLT)
	  {
	     e->step = (int)(step * ECORE_CONFIG_FLOAT_PRECISION);
	     e->flags |= ECORE_CONFIG_FLAG_BOUNDS;
	     e->lo = (int)(lo * ECORE_CONFIG_FLOAT_PRECISION);
	     e->hi = (int)(hi * ECORE_CONFIG_FLOAT_PRECISION);
	     ecore_config_bound(e);
	  }
     }

   return ret;
}

/**
 * Sets the indicated property to the given boolean if the property has not yet
 * been set.
 * @param   key The property key.
 * @param   val Boolean Integer to set the value to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there are no problems.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_boolean_default(const char *key, int val)
{
   val = val ? 1 : 0;
   return ecore_config_typed_default(key, &val, ECORE_CONFIG_BLN);
}

/**
 * Sets the indicated property to the given integer if the property has not yet
 * been set.
 * @param   key The property key.
 * @param   val Integer to set the value to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there are no problems.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_int_default(const char *key, int val)
{
   return ecore_config_typed_default(key, &val, ECORE_CONFIG_INT);
}

/**
 * Sets the indicated property to the given integer if the property has not yet
 * been set.
 *
 * The bounds and step values are set regardless.
 *
 * @param   key  The property key.
 * @param   val  Integer to set the property to.
 * @param   low  Lowest valid integer value for the property.
 * @param   high Highest valid integer value for the property.
 * @param   step Increment value for the property.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there were no problems.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_int_default_bound(const char *key, int val, int low, int high,
			       int step)
{
   Ecore_Config_Prop  *e;
   int                 ret;

   ret = ecore_config_typed_default(key, &val, ECORE_CONFIG_INT);
   e = ecore_config_get(key);
   if (e)
     {
	e->step = step;
	e->flags |= ECORE_CONFIG_FLAG_BOUNDS;
	e->lo = low;
	e->hi = high;
	ecore_config_bound(e);
     }

   return ret;
}

/**
 * Sets the indicated property to the given string if the property has not yet
 * been set.
 * @param   key The property key.
 * @param   val String to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there were no problems.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_string_default(const char *key, const char *val)
{
   return ecore_config_typed_default(key, val, ECORE_CONFIG_STR);
}

/**
 * Sets the indicated property to the given float if the property has not yet
 * been set.
 * @param   key The property key.
 * @param   val Float to set the property to.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there were no problems.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_float_default(const char *key, float val)
{
   return ecore_config_typed_default(key, &val, ECORE_CONFIG_FLT);
}

/**
 * Sets the indicated property to the given float if the property has not yet
 * been set.
 *
 * The bounds and step values are set regardless.
 *
 * @param   key  The property key.
 * @param   val  Float to set the property to.
 * @param   low  Lowest valid integer value for the property.
 * @param   high Highest valid float value for the property.
 * @param   step Increment value for the property.
 * @return  @c ECORE_CONFIG_ERR_SUCC if there were no problems.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_float_default_bound(const char *key, float val, float low,
				 float high, float step)
{
   Ecore_Config_Prop  *e;
   int                 ret;

   ret = ecore_config_typed_default(key, &val, ECORE_CONFIG_FLT);
   e = ecore_config_get(key);
   if (e)
     {
	e->step = (int)(step * ECORE_CONFIG_FLOAT_PRECISION);
	e->flags |= ECORE_CONFIG_FLAG_BOUNDS;
	e->lo = (int)(low * ECORE_CONFIG_FLOAT_PRECISION);
	e->hi = (int)(high * ECORE_CONFIG_FLOAT_PRECISION);
	ecore_config_bound(e);
     }

   return ret;
}

/**
 * Sets the indicated property to a color value if the property has not yet
 * been set.
 * @param  key The property key.
 * @param  a integer 0..255
 * @param  r integer 0..255
 * @param  g integer 0..255
 * @param  b integer 0..255
 * @return @c ECORE_CONFIG_ERR_SUCC if there are no problems.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_argb_default(const char *key, int a, int r, int g, int b)
{
   long v = 0;
   return ecore_config_typed_default(key, __ecore_argb_to_long(a,r,g,b, &v), ECORE_CONFIG_RGB);
}

/**
 * Sets the indicated property to a color value if the property has not yet
 * been set.
 * @param  key The property key.
 * @param  argb ARGB data as long
 * @return @c ECORE_CONFIG_ERR_SUCC if there are no problems.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_argbint_default(const char *key, long argb)
{
   return ecore_config_typed_default(key, &argb, ECORE_CONFIG_RGB);
}

/**
 * Sets the indicated property to a color value if the property has not yet
 * been set.
 * @param  key The property key.
 * @param  val Color value in ARGB format.
 * @return @c ECORE_CONFIG_ERR_SUCC if there are no problems.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_argbstr_default(const char *key, const char *val)
{
   long v = 0;
   return ecore_config_typed_default(key, __ecore_argbstr_to_long(val, &v), ECORE_CONFIG_RGB);
}

/**
 * Sets the indicated property to a theme name if the property has not yet
 * been set.
 * @param   key The property key.
 * @param   val String giving the name of the theme.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Default_Group
 */
EAPI int
ecore_config_theme_default(const char *key, const char *val)
{
   return ecore_config_typed_default(key, val, ECORE_CONFIG_THM);
}

/**
 * @defgroup Ecore_Config_Struct_Group Ecore Config Structures
 *
 * Functions that are used to create structures of properties.
 */

/**
 * Sets the indicated property to a structure if the property has not yet
 * been set.
 * @param   key The property key.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Struct_Group
 */
EAPI int
ecore_config_struct_create(const char *key)
{
   printf("WARNING: you are using ecore_config structures. These are very young");
   printf(" and not complete - you have been warned");

   return ecore_config_typed_default(key, NULL, ECORE_CONFIG_SCT);
}

static int
_ecore_config_struct_append(Ecore_Config_Prop *sct, Ecore_Config_Prop *add)
{
   Eina_List *l;

   if (!sct || !add || sct->type != ECORE_CONFIG_SCT)
     return ECORE_CONFIG_ERR_IGNORED;

   l = sct->data;
   sct->data = eina_list_append(l, add);
   add->parent = sct;

   return ECORE_CONFIG_ERR_SUCC;
}

static int
_ecore_config_struct_typed_add(const char *key, const char *name, const void *val,
    int type)
{
   char *subkey;
   int ret;

   subkey = malloc((strlen(key) + strlen(name) + 2) * sizeof(char));
   strcpy(subkey, key);
   strcat(subkey, ".");
   strcat(subkey, name);

   ecore_config_typed_default(subkey, val, type);
   ret = _ecore_config_struct_append(ecore_config_get(key),
                                     ecore_config_get(subkey));
   free(subkey);
   return ret;   
}

/**
 * Add an int property to the named structure. The property is set if it has not
 * yet been set.
 * @param   key The key of the structure to add to.
 * @param   name The name of the item to add - this will be appended to the key
 * @param   val the int to default to
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Struct_Group
 */
EAPI int
ecore_config_struct_int_add(const char *key, const char *name, int val)
{
   return _ecore_config_struct_typed_add(key, name, &val, ECORE_CONFIG_INT);
}

/**
 * Add a float property to the named structure. The property is set if it has
 * not yet been set.
 * @param   key The key of the structure to add to.
 * @param   name The name of the item to add - this will be appended to the key
 * @param   val The float to default to
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Struct_Group
 */
EAPI int
ecore_config_struct_float_add(const char *key, const char *name, float val)
{
   return _ecore_config_struct_typed_add(key, name, &val, ECORE_CONFIG_FLT);
}

/**
 * Add a string property to the named structure. The property is set if it has
 * not yet been set.
 * @param   key The key of the structure to add to.
 * @param   name The name of the item to add - this will be appended to the key
 * @param   val The string to default to
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Struct_Group
 */
EAPI int
ecore_config_struct_string_add(const char *key, const char *name, const char* val)
{
   return _ecore_config_struct_typed_add(key, name, val, ECORE_CONFIG_STR);
}

/**
 * Add an argb property to the named structure. The property is set if it has
 * not yet been set.
 * @param   key The key of the structure to add to.
 * @param   name The name of the item to add - this will be appended to the key
 * @param   a The alpha to default to
 * @param   r The red to default to
 * @param   g The green to default to
 * @param   b The blue to default to
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Struct_Group
 */
EAPI int
ecore_config_struct_argb_add(const char *key, const char *name, int a, int r,
                             int g, int b)
{
   long argb;
  
   __ecore_argb_to_long(a, r, g, b, &argb);
   return _ecore_config_struct_typed_add(key, name, &argb, ECORE_CONFIG_RGB);
}

/**
 * Add a theme property to the named structure. The property is set if it has
 * not yet been set.
 * @param   key The key of the structure to add to.
 * @param   name The name of the item to add - this will be appended to the key
 * @param   val The theme name to default to
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Struct_Group
 */
EAPI int
ecore_config_struct_theme_add(const char *key, const char *name, const char* val)
{
   return _ecore_config_struct_typed_add(key, name, val, ECORE_CONFIG_THM);
}

/**
 * Add a boolean property to the named structure. The property is set if it has
 * not yet been set.
 * @param   key The key of the structure to add to.
 * @param   name The name of the item to add - this will be appended to the key
 * @param   val The boolean to default to
 * @return  @c ECORE_CONFIG_ERR_SUCC if the property is set successfully.
 * @ingroup Ecore_Config_Struct_Group
 */
EAPI int
ecore_config_struct_boolean_add(const char *key, const char *name, int val)
{
   val = val ? 1 : 0;
   return _ecore_config_struct_typed_add(key, name, &val, ECORE_CONFIG_BLN);
}

/**
 * Get the contents of a defined structure property and load it into the passed
 * C struct
 * @param   key The name of the structure property to look up.
 * @param   data The struct to write into.
 * @return  @c ECORE_CONFIG_ERR_SUCC if the structure is written successfully.
 * @ingroup Ecore_Config_Struct_Group
 */
EAPI int
ecore_config_struct_get(const char *key, void *data)
{
   Ecore_Config_Prop *e, *f;
   Eina_List *l;
   unsigned char *ptr;
   long argb;

   e = ecore_config_get(key);
   if (!e)
     return ECORE_CONFIG_ERR_NODATA;

   l = e->data;
   ptr = data;
   while (l)
     {
	f = (Ecore_Config_Prop *) l->data;
	switch (f->type)
	  {
	     case ECORE_CONFIG_INT:
	       *((int *) ptr) = _ecore_config_int_get(f);
	       ptr += sizeof(int);
	     break;
	     case ECORE_CONFIG_BLN:
	       *((int *) ptr) = _ecore_config_boolean_get(f);
	       ptr += sizeof(int);
	     break;
	     case ECORE_CONFIG_FLT:
	       *((float *) ptr) = _ecore_config_float_get(f);
	       ptr += sizeof(float);
	     break;
	     case ECORE_CONFIG_STR:
	     case ECORE_CONFIG_THM:
	       *((char **) ptr) = _ecore_config_string_get(f);
	       ptr += sizeof(char *);
	     break;
	     case ECORE_CONFIG_RGB:
	       argb = _ecore_config_argbint_get(f);
	       *((int *) ptr) = (argb >> 24) & 0xff;
	       ptr += sizeof(int);
	       *((int *) ptr) = (argb >> 16) & 0xff;
	       ptr += sizeof(int);
	       *((int *) ptr) = (argb >> 8) & 0xff;
	       ptr += sizeof(int);
	       *((int *) ptr) = argb & 0xff;
	       ptr += sizeof(int);
	     break;
	     default:
	       printf("ARGH - STRUCT coding not implemented yet\n");
	  }
	l = eina_list_next(l);
     }
   return ECORE_CONFIG_ERR_SUCC;
}

/**
 * @defgroup Ecore_Config_Listeners_Group Ecore Config Listeners
 *
 * Functions that set and unset property listener callbacks.
 */

/**
 * Adds a callback function to the list of functions called when a property
 * changes.
 * @param   name     Name of the callback.
 * @param   key      The key of the property to listen to.
 * @param   listener Listener callback function.
 * @param   tag      Tag to pass to @p listener when it is called.
 * @param   data     Data to pass to @p listener when it is called.
 * @return  @c ECORE_CONFIG_ERR_SUCC if successful in setting up the callback.
 * @ingroup Ecore_Config_Listeners_Group
 */
EAPI int
ecore_config_listen(const char *name, const char *key,
		    Ecore_Config_Listener listener, int tag, void *data)
{
   Ecore_Config_Prop  *e;
   Ecore_Config_Listener_List *l;

   if (!key)
      return ECORE_CONFIG_ERR_NODATA;

   if (!(e = ecore_config_get(key)))
     {
	int                 ret = ecore_config_add(key, "");

	if (ret != ECORE_CONFIG_ERR_SUCC)
	  {
	     E(0, "ecore_config_listen: ecore_config_add(\"%s\") failed: %d\n",
	       key, ret);
	     return ret;
	  }
	if (!(e = ecore_config_get(key)))
	  {
	     E(0, "ecore_config_listen: list of properties corrupted!?\n");
	     return ECORE_CONFIG_ERR_FAIL;
	  }
     }

   for (l = e->listeners; l; l = l->next)
      if (!strcmp(l->name, name) || (l->listener == listener))
	{
	   E(1,
	     "ecore_config_listen: %s is already listening for changes of %s...\n",
	     name, key);
	   return ECORE_CONFIG_ERR_IGNORED;
	}

   if (!(l = malloc(sizeof(Ecore_Config_Listener_List))))
      return ECORE_CONFIG_ERR_OOM;

   E(1, "registering listener \"%s\" for \"%s\" (%d)...\n", name, key, e->type);

   memset(l, 0, sizeof(Ecore_Config_Listener_List));

   l->listener = listener;
   l->name = name;
   l->data = data;
   l->tag = tag;
   l->next = e->listeners;
   e->listeners = l;

   if (e->type != ECORE_CONFIG_NIL)	/* call right on creation if prop exists and has val */
      listener(key, e->type, tag, data);

   return ECORE_CONFIG_ERR_SUCC;
}

/**
 * Removes a listener callback.
 * @param  name     Name of the callback to remove.
 * @param  key      The property key the callback is listening to.
 * @param  listener The callback function to remove.
 * @return @c ECORE_CONFIG_ERR_SUCC if successful in removing the callback.
 *         If no callback matches the given parameters, then
 *         @c ECORE_CONFIG_ERR_NOTFOUND is returned.  If @c NULL is passed
 *         for the key pointer, @c ECORE_CONFIG_ERR_NODATA is returned.
 * @ingroup Ecore_Config_Listeners_Group
 */
EAPI int
ecore_config_deaf(const char *name, const char *key,
		  Ecore_Config_Listener listener)
{
   Ecore_Config_Prop  *e;
   Ecore_Config_Listener_List *l, *p;
   int                 ret;

   ret = ECORE_CONFIG_ERR_NOTFOUND;

   if (!key)
      return ECORE_CONFIG_ERR_NODATA;

   if (!(e = ecore_config_get(key)))
      return ECORE_CONFIG_ERR_NOTFOUND;

   for (p = NULL, l = e->listeners; l; p = l)
     {
	Ecore_Config_Listener_List *nl;
	
	nl = l->next;
	if ((name && !strcmp(l->name, name)) || (l->listener == listener))
	  {
	     ret = ECORE_CONFIG_ERR_SUCC;
	     if (!p)
		e->listeners = e->listeners->next;
	     else
		p->next = l->next;
	     memset(l, 0, sizeof(Ecore_Config_Listener));
	     free(l);
	  }
	l = nl;
     }

   return ret;
}

/**
 * Locates the first configuration bundle on the given server.
 * @param  srv The configuration server.
 * @return Pointer to the first configuration bundle.
 */
EAPI Ecore_Config_Bundle *
ecore_config_bundle_1st_get(Ecore_Config_Server * srv)
{				/* anchor: global, but read-only */
   return srv->bundles;
}

/**
 * Locates the configuration bundle after the given one.
 * @param  ns  The configuration bundle.
 * @return The next configuration bundle.
 */
EAPI Ecore_Config_Bundle *
ecore_config_bundle_next_get(Ecore_Config_Bundle * ns)
{
   return ns ? ns->next : NULL;
}

/**
 * Locates a configuration bundle on a configuration server based on its serial
 * number.
 * @param  srv    The configuration server.
 * @param  serial Serial number.
 * @return The configuration bundle with the given serial number.
 */
EAPI Ecore_Config_Bundle *
ecore_config_bundle_by_serial_get(Ecore_Config_Server * srv, long serial)
{
   Ecore_Config_Bundle *eb;

   eb = srv->bundles;

   if (serial < 0)
      return NULL;
   else if (serial == 0)
     {
	Ecore_Config_Bundle *r = eb;

	return r;
     }

   while (eb)
     {
	if (eb->serial == serial)
	   return eb;
	eb = eb->next;
     }
   return NULL;
}

/**
 * Gets the Ecore_Config_Bundle with the given identifier from the given
 * server.
 * @param  srv   The configuration server.
 * @param  label The bundle's identifier string.
 * @return The bundle with the given identifier string, or @c NULL if it
 *         could not be found.
 */
EAPI Ecore_Config_Bundle *
ecore_config_bundle_by_label_get(Ecore_Config_Server * srv, const char *label)
{
   Ecore_Config_Bundle *ns;

   ns = srv->bundles;

   while (ns)
     {
	if (ns->identifier && !strcmp(ns->identifier, label))
	   return ns;
	ns = ns->next;
     }
   return NULL;
}

/**
 * Retrieves the bundle's serial number.
 * @param  ns The configuration bundle.
 * @return The bundle's identifier string, or -1 if ns is @c NULL.
 */
EAPI long
ecore_config_bundle_serial_get(Ecore_Config_Bundle * ns)
{
   return ns ? ns->serial : -1;
}

/**
 * Retrieves the bundle's identifier.
 * @param  ns The configuration bundle.
 * @return The bundle's identifer string.
 */
EAPI char               *
ecore_config_bundle_label_get(Ecore_Config_Bundle * ns)
{
   return ns ? ns->identifier : NULL;
}

/**
 * Creates a new Ecore_Config_Bundle.
 * @param  srv        Config server.
 * @param  identifier Identifier string for the new bundle.
 * @return A pointer to a new Ecore_Config_Bundle.  @c NULL is returned if the
 *         structure couldn't be allocated.
 */
EAPI Ecore_Config_Bundle *
ecore_config_bundle_new(Ecore_Config_Server * srv, const char *identifier)
{
   Ecore_Config_Bundle *t;
   static long         ss;

   ss = 0;			/* bundle unique serial */

   if ((t = malloc(sizeof(Ecore_Config_Bundle))))
     {
	memset(t, 0, sizeof(Ecore_Config_Bundle));

	t->identifier = (char *)identifier;
	t->serial = ++ss;
	t->owner = srv->name;
	t->next = srv->bundles;
	srv->bundles = t;
     }
   return t;
}

static Ecore_Config_Server *
do_init(const char *name)
{
   return _ecore_config_ipc_init(name);
}

static Ecore_Config_Server *
ecore_config_init_local(const char *name)
{
   char               *p;
   char               *buf;

   if ((p = getenv("HOME")))
     {				/* debug-only ### FIXME */
	if (!(buf = malloc(PATH_MAX * sizeof(char))))
	   return NULL;
	snprintf(buf, PATH_MAX, "%s/.ecore/%s/.global", p, name);
	unlink(buf);

	free(buf);
     }

   return do_init(name);
}

static Ecore_Config_Server *
ecore_config_init_global(const char *name)
{
   char               *p;
   int global;
   char               *buf;
   global = 0;

   if ((p = getenv("HOME")))
     {				/* debug-only ### FIXME */
	if (!(buf = malloc(PATH_MAX * sizeof(char))))
	   return NULL;
	snprintf(buf, PATH_MAX, "%s/.ecore/%s/.global", p, name);
	global = creat(buf, S_IRWXU);

	if (global)
	   close(global);

	free(buf);
     }

   return do_init(name);
}

/**
 * @defgroup Ecore_Config_App_Lib_Group Ecore Config App Library Functions
 *
 * Functions that are used to start up and shutdown the Enlightened
 * Property Library when used directly by an application.
 */

/**
 * Initializes the Enlightened Property Library.
 *
 * Either this function or @ref ecore_config_system_init must be run
 * before any other function in the Enlightened Property Library, even
 * if you have run @ref ecore_init .  The name given is used to
 * determine the default configuration to load.
 *
 * @param  name Application name
 * @return @c ECORE_CONFIG_ERR_SUCC if the library is successfully set up.
 *         @c ECORE_CONFIG_ERR_FAIL otherwise.
 * @ingroup Ecore_Config_App_Lib_Group
 */
EAPI int
ecore_config_init(const char *name)
{
   char                *path;
   Ecore_Config_Prop   *list;
   _ecore_config_system_init_no_load();

   __ecore_config_app_name = strdup(name);
   __ecore_config_server_local = ecore_config_init_local(name);
   if (!__ecore_config_server_local)
      return ECORE_CONFIG_ERR_FAIL;

   list = __ecore_config_bundle_local->data;
   free( __ecore_config_bundle_local );
   __ecore_config_bundle_local =
      ecore_config_bundle_new(__ecore_config_server_local, "config");
   __ecore_config_bundle_local->data = list;

   path = ecore_config_theme_default_path_get();
   ecore_config_string_default("/e/themes/search_path", path);
   if (path)
      free(path);

   list = ecore_config_get("/e/themes/search_path");
   if (list)
     {
	list->flags |= ECORE_CONFIG_FLAG_SYSTEM;
	list->flags &= ~ECORE_CONFIG_FLAG_MODIFIED;
     }

   return _ecore_config_system_load();
}

/**
 * Frees memory and shuts down the library for an application.
 * @return @c ECORE_CONFIG_ERR_IGNORED .
 * @ingroup Ecore_Config_App_Lib_Group
 */
EAPI int
ecore_config_shutdown(void)
{
   return ecore_config_system_shutdown();
}

/**
 * @defgroup Ecore_Config_Lib_Lib_Group Ecore Config Library Functions
 *
 * Functions that are used to start up and shutdown the Enlightened
 * Property Library when used directly by an application.
 */

/**
 * Initializes the Enlightened Property Library.
 *
 * This function is meant to be run from other programming libraries.
 * It should not be called from applications.
 *
 * This function (or @ref ecore_config_init )
 * must be run before any other function in the
 * Enlightened Property Library, even if you have run @ref ecore_init .
 *
 * @return @c ECORE_CONFIG_ERR_SUCC if the library is successfully set up.
 *         @c ECORE_CONFIG_ERR_FAIL otherwise.
 * @ingroup Ecore_Config_Lib_Lib_Group
 */
EAPI int
ecore_config_system_init(void)
{
   _ecore_config_system_init_no_load();
   return _ecore_config_system_load();
}

static            int
_ecore_config_system_init_no_load(void)
{
   char               *p;

   __ecore_config_system_init++;
   if (__ecore_config_system_init > 1)
      return ECORE_CONFIG_ERR_IGNORED;

   DEBUG = -1;
   if ((p = getenv("ECORE_CONFIG_DEBUG")) && p[0] != 0)
     {
	DEBUG = atoi(p);
     }

   __ecore_config_server_global =
      ecore_config_init_global(ECORE_CONFIG_GLOBAL_ID);
   if (!__ecore_config_server_global)
	return ECORE_CONFIG_ERR_FAIL;

   __ecore_config_bundle_local =
      ecore_config_bundle_new(__ecore_config_server_global, "system");

   /* set up a simple default path */
   ecore_config_string_default("/e/themes/search_path", PACKAGE_DATA_DIR "../ewl/themes");
   
   return ECORE_CONFIG_ERR_SUCC;
}


static             int
_ecore_config_system_load(void)
{
   char               *buf, *p;
   Ecore_Config_Prop  *sys;

   if (__ecore_config_system_init != 1)
	return ECORE_CONFIG_ERR_FAIL;

   if ((p = getenv("HOME")))
     {                          /* debug-only ### FIXME */
	if ((buf = malloc(PATH_MAX * sizeof(char))))
	  {
	     snprintf(buf, PATH_MAX, "%s/.e/config.eet", p);
	     if (ecore_config_file_load(buf) != 0) {
		/* even if this file (system.eet) dosen't exist we can 
		 * continue without it as it isn't striclty necessary.
		*/
		ecore_config_file_load(PACKAGE_DATA_DIR "/system.eet");
	     }
	     sys = __ecore_config_bundle_local->data;
	     while (sys)
	       {
		  /* unmark it modified - modification will mean it has been overridden */
		  sys->flags &= ~ECORE_CONFIG_FLAG_MODIFIED;
		  /* mark as system so that examine can hide them */
		  sys->flags |= ECORE_CONFIG_FLAG_SYSTEM;
		  sys = sys->next;
	       }
	  }
	free(buf);
     }

   return ECORE_CONFIG_ERR_SUCC;
}


/**
 * Frees memory and shuts down the library for other programming libraries.
 * @return @c ECORE_CONFIG_ERR_IGNORED
 * @ingroup Ecore_Config_Lib_Lib_Group
 */
EAPI int
ecore_config_system_shutdown(void)
{
   int                 ret;

   __ecore_config_system_init--;
   if (__ecore_config_system_init > 0)
      return ECORE_CONFIG_ERR_IGNORED;

   ret = _ecore_config_ipc_exit();
   if (__ecore_config_app_name)
      free(__ecore_config_app_name);
   while(__ecore_config_bundle_local->data)
     ecore_config_dst(__ecore_config_bundle_local->data);
   free(__ecore_config_bundle_local);
   free(__ecore_config_server_local);
   free(__ecore_config_server_global);
   return ret;
}

static inline void *
__ecore_argb_to_long(int a, int r, int g, int b, long *v)
{
   *v = ((a << 24) & 0xff000000 )
      | ((r << 16) &   0xff0000 )
      | ((g <<  8) &     0xff00 )
      | ( b        &       0xff );

   return v;
}

static inline void *
__ecore_argbstr_to_long(const char *argb, long *v)
{
   char *l = NULL;

   // convert hexadecimal string #..., #0x..., 0x..., ... to long
   if(*argb == '#')
     argb++;
   *v = (long)strtoul( argb, &l, 16);

   if(*l)
     {
	E(0, "ecore_config_val: value \"%s\" not a valid hexadecimal RGB value?\n", argb);
	return NULL;
     }

   return v;
}

