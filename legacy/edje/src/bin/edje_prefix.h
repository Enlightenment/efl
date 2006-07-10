/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

int         e_prefix_determine(char *argv0);
void        e_prefix_shutdown(void);
void        e_prefix_fallback(void);
const char *e_prefix_get(void);
const char *e_prefix_bin_get(void);
const char *e_prefix_data_get(void);
const char *e_prefix_lib_get(void);
