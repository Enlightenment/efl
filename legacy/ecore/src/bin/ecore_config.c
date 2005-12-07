#include "config.h"
#include "Ecore.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef BUILD_ECORE_CONFIG
#include <unistd.h>
#include <Eet.h>
#include "Ecore_Config.h"
#include "Ecore_Data.h"

// strcmp for paths - for sorting folders before files
int
pathcmp(const char *s1, const char *s2)
{
	char *s1d, *s2d;

	// strip common part of paths
	while(*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}

	// handle /foo/bar/baz <> /foo/bar_baz properly
	if (*s1 == '/' && *s2 != '/') return -1;
	if (*s1 != '/' && *s2 == '/') return  1;

	// skip leading /
	if (*s1 == '/') s1++;
	if (*s2 == '/') s2++;

	// order folders before files
	s1d = strchr(s1, '/');
	s2d = strchr(s2, '/');
	if (s1d != NULL && s2d == NULL) return -1;
	if (s1d == NULL && s2d != NULL) return  1;

	return strcmp(s1, s2);
}

int
del(const char *key)
{
	Ecore_Config_Prop *e;
	e = ecore_config_get(key);
	if(e == NULL) return -1;

	ecore_config_dst(e);
	return 0;
}

int
get(const char *key)
{
	Ecore_Config_Prop *e;

	if (!(e = ecore_config_get(key))) {
		fprintf(stderr, "No such property\n");
		return -1;
	}
		
	switch (e->type) {
	case ECORE_CONFIG_NIL:
		printf("\n");
		break;
	case ECORE_CONFIG_INT:
		printf("integer %ld\n", ecore_config_int_get(key));
		break;
	case ECORE_CONFIG_BLN:
		printf("bool    %d\n", ecore_config_boolean_get(key));
		break;
	case ECORE_CONFIG_FLT:
		printf("float   %lf\n", ecore_config_float_get(key));
		break;
	case ECORE_CONFIG_STR:
		printf("string  \"%s\"\n", ecore_config_string_get(key));
		break;
	case ECORE_CONFIG_RGB:
		printf("rgb     \"%s\"\n", ecore_config_argbstr_get(key));
		break;
	case ECORE_CONFIG_THM:
		printf("theme   \"%s\"\n", ecore_config_theme_get(key));
		break;
	default:
		fprintf(stderr, "Property has unrecognised type");
		return -1;
	}
	return 0;
}

int
list(const char *file)
{
	char *key;

	Eet_File *ef;
	Ecore_Config_Prop *e;
	Ecore_Sheap *keys;

	ef = eet_open(file, EET_FILE_MODE_READ);
	if (!ef) return -1;

	keys = ecore_sheap_new(ECORE_COMPARE_CB(pathcmp), eet_num_entries(ef));

	eet_close(ef);

	e = __ecore_config_bundle_local->data;

	do {
		ecore_sheap_insert(keys, e->key);
	} while((e = e->next));

	while((key = ecore_sheap_extract(keys))) {
		printf("%-28s\t", key);
		get(key);
	}

	return 0;
}

void
usage_and_exit(const char *prog, int ret, const char *msg)
{
	if (msg) fprintf(stderr, msg);
	fprintf(stderr, "Usage: %s -c <file> <command> [-k key]\n", prog);
	fprintf(stderr, "Modify ecore_config files\n\n");
	fprintf(stderr, "Accepted commands:\n");
	fprintf(stderr, "  -a          get all keys\n");
	fprintf(stderr, "  -g          get key\n");
	fprintf(stderr, "  -d          delete key\n");
	fprintf(stderr, "  -b <value>  set boolean\n");
	fprintf(stderr, "  -f <value>  set float\n");
	fprintf(stderr, "  -i <value>  set integer\n");
	fprintf(stderr, "  -n          set nil\n");
	fprintf(stderr, "  -r <value>  set RGBA\n");
	fprintf(stderr, "  -s <value>  set string\n");
	fprintf(stderr, "  -t <value>  set theme\n\n");
	fprintf(stderr, "  -k <key> must be specified for all commands except -a\n\n");
	exit(ret);
}

int
main(int argc, char * const argv[])
{
	const char *prog, *file, *key;
	void *value = (void *)NULL;
	char cmd = 's';
	int type = -1;
	int ret = 0;
	int i;
	float f;
	
	file = key = prog = NULL;

	prog = strdup(argv[0]);

	if(argc < 4)
		usage_and_exit(prog, 2, NULL);

	while((ret = getopt(argc, argv, "angdb:f:i:r:s:t:c:k:")) != -1) {
		switch(ret) {
			case 'k':
				key = strdup(optarg);
				break;
			case 'n':
				type = ECORE_CONFIG_NIL;
				value = NULL;
				break;
			case 'b':
				type = ECORE_CONFIG_BLN;
				i = atoi(optarg);
				value = &i;
				break;
			case 'i':
				type = ECORE_CONFIG_INT;
				i = atoi(optarg);
				value = &i;
				break;
			case 'f':
				type = ECORE_CONFIG_FLT;
				f = atof(optarg);
				value = &f;
				break;
			case 'r':
				type = ECORE_CONFIG_RGB;
				value = strdup(optarg);
				break;
			case 's':
				type = ECORE_CONFIG_STR;
				value = strdup(optarg);
				break;
			case 't':
				type = ECORE_CONFIG_THM;
				value = strdup(optarg);
				break;
			case 'c':
				file = strdup(optarg);
				break;
			case '?':
			case ':':
				usage_and_exit(prog, 2, "Bad argument\n");
			default:
				cmd = ret;
				break;
		}
	}

	if(cmd == 's' && type == -1)
		usage_and_exit(prog, 2, "You need to specify a command!\n");

	if(cmd != 'a' && key == NULL)
		usage_and_exit(prog, 2, "You need to specify key!\n");
	
	if(ecore_config_init("econfig") != ECORE_CONFIG_ERR_SUCC) {
		fprintf(stderr, "Couldn't init ecore_config!\n");
		return 1;
	}

	// Remove non-file data
	while (ecore_config_dst(__ecore_config_bundle_local->data));

	// Load configuration from file
	ecore_config_file_load(file);

	// Execute command
	switch (cmd) {
		case 's':
			if (ecore_config_typed_set(key, value, type) != ECORE_CONFIG_ERR_SUCC) {
				fprintf(stderr, "Set failed for %s\n", key);
				ret = 1;
			} else {
				ecore_config_file_save(file);
			}
			break;
		case 'd':
			if(del(key)) {
				fprintf(stderr, "Delete failed for %s\n", key);
				ret = 1;
			} else {
				ecore_config_file_save(file);
			}
			break;
		case 'g':
			if (get(key)) ret = 1;
			break;
		case 'a':
			if (list(file)) ret = 1;
			break;
	}

	ecore_config_shutdown();

	return ret;
}
#else
int
main(int argc, const char **argv)
{
   printf("Ecore_config module not compiled. This program is empty.\n");
   return -1;
}
#endif
