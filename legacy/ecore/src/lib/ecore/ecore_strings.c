#include <Ecore.h>

static Ecore_Hash *ecore_strings = NULL;

/**
 * Retrieve an instance of a string for use in an ecore
 * @param  string The string to retrieve an instance
 * @return A pointer to a the string on success, NULL on failure.
 */
char *ecore_string_instance(char *string)
{
	Ecore_String *str;

	CHECK_PARAM_POINTER_RETURN("string", string, NULL);

	/*
	 * No strings have been loaded at this point, so create the hash
	 * table for storing string info for later.
	 */
	if (!ecore_strings)
		ecore_strings = ecore_hash_new(ecore_str_hash, ecore_str_compare);

	/*
	 * Check for a previous instance of the string, if not found, create
	 * it.
	 */
	str = ecore_hash_get(ecore_strings, string);
	if (!str) {

		/*
		 * Allocate and initialize a new string reference.
		 */
		str = (Ecore_String *)malloc(sizeof(Ecore_String));

		str->string = strdup(string);
		str->references = 0;

		ecore_hash_set(ecore_strings, string, str);
	}

	str->references++;

	return str->string;
}

/**
 * Release an instance of a string
 * @param string The string to release an instance
 * @return No value.
 *
 * Marks the string as losing an instance, will free the string if no other
 * instances are present.
 */
void ecore_string_release(char *string)
{
	Ecore_String *str;

	CHECK_PARAM_POINTER("string", string);

	str = ecore_hash_get(ecore_strings, string);
	if (!str)
		return;

	str->references--;
	if (str->references < 1) {
		ecore_hash_remove(ecore_strings, string);
		FREE(str->string);
	}
}
