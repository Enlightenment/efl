/*
 * A little helper library for abstracting the path finding which depends on autotools (or later on other buildsystems)
 *
 * Author: Marcel Hollerbach
 */
#include <stdio.h>
#include <Eina.h>

/*
 * Fills the provided buffer with the path to the binary of a given subsystem
 */
Eina_Bool bs_binary_get(char *path, size_t maxlen, const char *subsystem, const char *bin_name);

/*
 * Fills the provided buffer with the path to a helper binary built under
 * src/generic, which is where the loaders that run in a process of their own
 * live. They are installed somewhere else entirely, so asking for the
 * installed path from a build tree finds the wrong one or none at all.
 */
Eina_Bool bs_binary_generic_get(char *path, size_t maxlen, const char *subsystem, const char *bin_name);

/*
 * Fills the provided buffer with the path to the data file/dir of a given subsystem
 */
Eina_Bool bs_data_path_get(char *path, size_t maxlen, const char *subsystem, const char *file);

/*
 * Fills the provided buffer with the path to the .so file for modules of a given subsystem
 */
Eina_Bool bs_mod_get(char *path, size_t maxlen, const char *subsystem, const char *mod_name);

Eina_Bool bs_mod_dir_get(char *path, size_t maxlen, const char *subsystem, const char *mod_name);
