#include "Edje.h"
#include "edje_private.h"

/************************** API Routines **************************/

/* FIXDOC: Verify/Expand */
/** Returns the variable ID
 * @param obj A valid Evas_Object handle
 * @param name The variable name
 * 
 * @return Variable ID\n
 * 0 on Error.
 */
int
edje_object_variable_id_get(Evas_Object *obj, char *name)
{
   Edje *ed;
   int id;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (!name)) return 0;
   return _edje_var_string_id_get(ed, name);
}

/* FIXDOC: Verify/Expand */
/** Returns the variable int
 * @param obj A valid Evas_Object handle
 * @param id The variable id
 *
 * @return Variable int\n
 * 0 on Error
 */
int
edje_object_variable_int_get(Evas_Object *obj, int id)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (id <= 0)) return 0;
   return _edje_var_int_get(ed, id);
}

/* FIXDOC: Verify/Expand */
/** Sets the variable int
 * @param obj A valid Evas_Object handle
 * @param id The variable id
 * @param val The int value to set
 */
void
edje_object_variable_int_set(Evas_Object *obj, int id, int val)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (id <= 0)) return;
   _edje_var_int_set(ed, id, val);
}

/* FIXDOC: Verify/Expand */
/** Returns the variable float
 * @param obj A valid Evas_Object handle
 * @param id The variable id
 *
 * @return Variable float\n
 * 0 on Error
 */
double
edje_object_variable_float_get(Evas_Object *obj, int id)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (id <= 0)) return 0;
   return _edje_var_float_get(ed, id);
}

/* FIXDOC: Verify/Expand */
/** Sets the variable float
 * @param obj A valid Evas_Object handle
 * @param id The variable id
 * @param val The float value to set
 */
void
edje_object_variable_float_set(Evas_Object *obj, int id, double val)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (id <= 0)) return;
   _edje_var_float_set(ed, id, val);
}

/* FIXDOC: Verify/Expand */
/** Returns the variable string
 * @param obj A valid Evas_Object handle
 * @param id The variable id
 *
 * @return Variable String\n
 * 0 on Error
 */
char *
edje_object_variable_str_get(Evas_Object *obj, int id)
{
   Edje *ed;
   char *s;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (id <= 0)) return NULL;
   s = _edje_var_str_get(ed, id);
   if (!s) return NULL;
   return strdup(s);
}

/* FIXDOC: Verify/Expand */
/** Sets the variable string
 * @param obj A valid Evas_Object handle
 * @param id The variable id
 * @param str The string value to set
 */   
void
edje_object_variable_str_set(Evas_Object *obj, int id, char *str)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (id <= 0)) return;
   if (!str) str = "";
   _edje_var_str_set(ed, id, str);
}
