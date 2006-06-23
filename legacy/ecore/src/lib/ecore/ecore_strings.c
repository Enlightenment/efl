#include "ecore_private.h"
#include "Ecore_Data.h"

static void ecore_string_free_cb(void *data);

static Ecore_Hash *ecore_strings = NULL;
static int ecore_string_init_count = 0;

/**
 * @defgroup Ecore_String_Group String Instance Functions
 *
 * These functions allow you to store one copy of a string, and use it
 * throughout your program.
 */

/**
 * Initialize the ecore string internal structure.
 * @return  Zero on failure, non-zero on successful initialization.
 */
EAPI int 
ecore_string_init()
{
   /*
    * No strings have been loaded at this point, so create the hash
    * table for storing string info for later.
    */
   if (!ecore_string_init_count)
     {
	ecore_strings = ecore_hash_new(ecore_str_hash, ecore_str_compare);
	if (!ecore_strings)
	  return 0;
	ecore_hash_set_free_value(ecore_strings, ecore_string_free_cb);
     }
   ecore_string_init_count++;

   return 1;
}

/**
 * Retrieves an instance of a string for use in an ecore program.
 * @param   string The string to retrieve an instance of.
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 * @ingroup Ecore_String_Group
 */
EAPI const char *
ecore_string_instance(char *string)
{
   Ecore_String *str;

   CHECK_PARAM_POINTER_RETURN("string", string, NULL);

   /*
    * Check for a previous instance of the string, if not found, create
    * it.
    */
   str = ecore_hash_get(ecore_strings, string);
   if (!str)
     {

	/*
	 * Allocate and initialize a new string reference.
	 */
	str = (Ecore_String *)malloc(sizeof(Ecore_String));

	str->string = strdup(string);
	str->references = 0;

	ecore_hash_set(ecore_strings, str->string, str);
     }

   str->references++;

   return str->string;
}

/**
 * Notes that the given string has lost an instance.
 *
 * It will free the string if no other instances are left.
 *
 * @param   string The given string.
 * @ingroup Ecore_String_Group
 */
EAPI void 
ecore_string_release(const char *string)
{
   Ecore_String *str;

   CHECK_PARAM_POINTER("string", string);

   str = ecore_hash_get(ecore_strings, (char *)string);
   if (!str)
     return;

   str->references--;
   if (str->references < 1)
     {
	ecore_hash_remove(ecore_strings, (char *)string);
	FREE(str->string);
	FREE(str);
     }
}

/**
 * Shutdown the ecore string internal structures
 */
EAPI void 
ecore_string_shutdown()
{
   --ecore_string_init_count;
   if (!ecore_string_init_count)
     {
	ecore_hash_destroy(ecore_strings);
	ecore_strings = NULL;
     }
}

static void 
ecore_string_free_cb(void *data)
{
   Ecore_String *str;

   str = data;
   FREE(str->string);
   FREE(str);
}
