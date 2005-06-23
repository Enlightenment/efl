#include "config.h"
#include "Ecore.h"

#ifdef BUILD_ECORE_CONFIG
#include "Ecore_Config.h"

int
set(const char *key, int ec_type, const char *value)
{
	int i;
	float f;
	
	switch (ec_type) {
	case PT_INT:
	case PT_BLN:
		i = atoi(value);
		if (ecore_config_typed_set(key, &i, ec_type) != ECORE_CONFIG_ERR_SUCC) return -1;
		break;
	case PT_FLT:
		f = atof(value);
		if (ecore_config_typed_set(key, &f, ec_type) != ECORE_CONFIG_ERR_SUCC) return -1;
		break;
	case PT_STR:
	case PT_RGB:
	case PT_THM:
	case PT_NIL:
		if (ecore_config_typed_set(key, value, ec_type) != ECORE_CONFIG_ERR_SUCC) return -1;
		break;
	}
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
	case PT_NIL:
		printf("\n");
		break;
	case PT_INT:
		printf("%ld\n", ecore_config_int_get(key));
		break;
	case PT_BLN:
		printf("%d\n", ecore_config_boolean_get(key));
		break;
	case PT_FLT:
		printf("%lf\n", ecore_config_float_get(key));
		break;
	case PT_STR:
		printf("%s\n", ecore_config_string_get(key));
		break;
	case PT_RGB:
		printf("%s\n", ecore_config_argbstr_get(key));
		break;
	case PT_THM:
		printf("%s\n", ecore_config_theme_get(key));
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
	fprintf(stderr, "Command not yet supported\n");
	return -1;
}

int
get_type(const char *key)
{
	Ecore_Config_Prop *e;
	if (!(e = ecore_config_get(key))) {
		fprintf(stderr, "No such property\n");
		return -1;
	}
		
	switch (e->type) {
	case PT_NIL:
		printf("nil\n");
		break;
	case PT_INT:
		printf("int\n");
		break;
	case PT_BLN:
		printf("bool\n");
		break;
	case PT_FLT:
		printf("float\n");
		break;
	case PT_STR:
		printf("string\n");
		break;
	case PT_RGB:
		printf("rgb\n");
		break;
	case PT_THM:
		printf("theme\n");
		break;
	default:
		fprintf(stderr, "Property has unrecognised type");
		return -1;
	}
	return 0;
}

int
parse_type(const char *type)
{
	if (!strcmp("nil", type)) {
		return PT_NIL;
	} else if (!strcmp("int", type)) {
		return PT_INT;
	} else if (!strcmp("float", type)) {
		return PT_FLT;
	} else if (!strcmp("bool", type)) {
		return PT_BLN;
	} else if (!strcmp("str", type)) {
		return PT_STR;
	} else if (!strcmp("rgb", type)) {
		return PT_RGB;
	} else if (!strcmp("theme", type)) {
		return PT_THM;
	}
	return -1;
}

void
usage_and_exit(const char *prog, int ret, const char *msg)
{
	if (msg) fprintf(stderr, msg);
	fprintf(stderr, "Usage: %s <config-file> {get|set|type|list} [args...]\n", prog);
	fprintf(stderr, "LIST:       %s <config-file> list\n", prog);
	fprintf(stderr, "GET:        %s <config-file> get <key>\n", prog);
	fprintf(stderr, "GET TYPE:   %s <config-file> type <key>\n", prog);
	fprintf(stderr, "SET:        %s <config-file> set <key> {nil|int|float|bool|str|rgb|theme} <value>\n", prog);
	exit(ret);
}

int
main(int argc, const char **argv)
{
	const char *prog, *file, *cmd, *key, *type, *value;
	int ec_type = -1;
	int ret = 0;
	prog = file = cmd = key = type = value = NULL;
	
	prog = argv[0];
	if (argc < 3) usage_and_exit(prog, 2, "Not enough arguments\n");

	file = argv[1];
	cmd = argv[2];

	// Check for valid command
	if (strcmp("get", cmd) &&
		strcmp("type", cmd) &&
		strcmp("set", cmd) &&
		strcmp("list", cmd))
	{
		usage_and_exit(prog, 2, "Unrecognised command\n");
	}
	
	// Check for enough arguments
	if ((*cmd == 's') || (*cmd == 'g') || (*cmd == 't')) {
		if (argc < 3) usage_and_exit(prog, 2, "Not enough arguments\n");
		key = argv[3];
	}

	if (*cmd == 's') {
		if (argc < 5) usage_and_exit(prog, 2, "No type and value specified\n");
		type = argv[4];

		if ((ec_type = parse_type(type)) < 0)
			usage_and_exit(prog, 3, "Unsupported type\n");

		if (strcmp(type, "nil")) {
			if (argc < 6)
				usage_and_exit(prog, 2, "No value specified\n");
			value = argv[5];
		}
	}
	
	// Load configuration from file
	ecore_config_init("econfig");
	ecore_config_file_load(file);
	
	// Execute command
	switch (*cmd) {
	case 's':
		if (set(key, ec_type, value)) {
			fprintf(stderr, "Set failed\n");
			ret = 4;
		} else {
			ecore_config_file_save(file);
		}
		break;
	case 'g':
		if (get(key)) ret = 4;
		break;
	case 't':
		if (get_type(key)) ret = 4;
		break;
	case 'l':
		if (list(file)) ret = 4;
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
