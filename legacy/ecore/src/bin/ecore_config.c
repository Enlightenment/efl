#include "config.h"
#include "Ecore.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef BUILD_ECORE_CONFIG
#include <unistd.h>
#include <getopt.h>
#include <Eet.h>
#include "Ecore_Config.h"
#include "ecore_config_private.h"

// strcmp for paths - for sorting folders before files
static int
pathcmp(const char *s1, const char *s2)
{
   char *s1d, *s2d;

   // strip common part of paths
   while(*s1 && *s2 && *s1 == *s2)
     {
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
   if (s1d && !s2d) return -1;
   if (!s1d && s2d) return  1;

   return strcmp(s1, s2);
}

static int
del(const char *key)
{
   Ecore_Config_Prop *e;
   e = ecore_config_get(key);
   if(!e) return -1;

   ecore_config_dst(e);
   return 0;
}

static int
get(const char *key)
{
   Ecore_Config_Prop *e;
   char *temp = NULL;

   if (!(e = ecore_config_get(key)))
     {
	EINA_LOG_ERR("No such property");
	return -1;
     }
     
   printf("%-10s", ecore_config_type_get(e));

   switch (e->type)
     {
	case ECORE_CONFIG_NIL:
	   printf("\n");
	   break;
	case ECORE_CONFIG_INT:
	   printf("%ld\n", ecore_config_int_get(key));
	   break;
	case ECORE_CONFIG_BLN:
	   printf("%d\n",  ecore_config_boolean_get(key));
	   break;
	case ECORE_CONFIG_FLT:
	   printf("%lf\n", ecore_config_float_get(key));
	   break;
	case ECORE_CONFIG_STR:
	   temp = ecore_config_string_get(key);
	   break;
	case ECORE_CONFIG_RGB:
	   temp = ecore_config_argbstr_get(key);
	   break;
	case ECORE_CONFIG_THM:
	   temp = ecore_config_theme_get(key);
	   break;
	default:
	   EINA_LOG_ERR("Property has unrecognized type");
	   return -1;
     }
   if(temp)
     {
	printf("\"%s\"\n", temp);
	free(temp);
     }
   return 0;
}

static int
list(const char *file)
{
   Ecore_Config_Prop *e;
   Eina_List *keys = NULL;
   char *key;

   e = __ecore_config_bundle_local->data;

   do
     {
	// don't show system settings
	if( !(e->flags & ECORE_CONFIG_FLAG_SYSTEM) )
	   keys = eina_list_append(keys, e->key);
     }
   while((e = e->next));
   keys = eina_list_sort(keys, -1, EINA_COMPARE_CB(pathcmp));

   EINA_LIST_FREE(keys, key)
     {
	printf("%-28s\t", key);
	get(key);
     }

   return 0;
}

static void
usage_and_exit(const char *prog, int ret, const char *msg)
{
   if (msg) fprintf(stderr, "%s\n\n", msg);
   fprintf(stderr, "Usage: %s <options> <command>\n", prog);
   fprintf(stderr, "Modify ecore_config files\n\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "  -c, --file=FILE config file\n");
   fprintf(stderr, "  -k, --key=KEY   must be given for all commands except -a\n\n");
   fprintf(stderr, "Commands:\n");
   fprintf(stderr, "  -a, --list         get all keys\n");
   fprintf(stderr, "  -g, --get          get key\n");
   fprintf(stderr, "  -d, --del          delete key\n");
   fprintf(stderr, "  -b, --bool=VALUE   set boolean\n");
   fprintf(stderr, "  -f, --float=VALUE  set float\n");
   fprintf(stderr, "  -i, --int=VALUE    set integer\n");
   fprintf(stderr, "  -r, --rgb=VALUE    set RGBA\n");
   fprintf(stderr, "  -s, --string=VALUE set string\n");
   fprintf(stderr, "  -t, --theme=VALUE  set theme\n\n");
   exit(ret);
}

int
main(int argc, char * const argv[])
{
   char *prog, *file, *key;
   void *value = (void *)NULL;
   char cmd = 's';
   int type = -1;
   int ret = 0;
   long i;
   float f;
   
   file = key = prog = NULL;
   eina_init();
   prog = strdup(argv[0]);

   if(argc < 4)
     usage_and_exit(prog, 2, NULL);

   while(1)
     {
	static struct option long_options[] = {
	   {"file",   1, 0, 'c'},
	   {"list",   0, 0, 'a'},
	   {"get",    0, 0, 'g'},
	   {"del",    0, 0, 'd'},
	   {"bool",   1, 0, 'b'},
	   {"float",  1, 0, 'f'},
	   {"int",    1, 0, 'i'},
	   {"rgb",    1, 0, 'r'},
	   {"string", 1, 0, 's'},
	   {"theme",  1, 0, 't'},
	   {"key",    1, 0, 'k'},
	   {0, 0, 0, 0}
	};

	ret = getopt_long(argc, argv, "c:agdb:f:i:r:s:t:k:", long_options, NULL);
	if(ret == -1)
	   break;

	switch(ret)
	   {
	     case 'k':
		key = strdup(optarg);
		break;
	     case 'n':
		if(value)
		   usage_and_exit(prog, 2, "too many commands");
		type = ECORE_CONFIG_NIL;
		value = NULL;
		break;
	     case 'b':
		if(value)
		   usage_and_exit(prog, 2, "too many commands");
		type = ECORE_CONFIG_BLN;
		i = atoi(optarg);
		value = &i;
		break;
	     case 'i':
		if(value)
		   usage_and_exit(prog, 2, "too many commands");
		type = ECORE_CONFIG_INT;
		i = atoi(optarg);
		value = &i;
		break;
	     case 'f':
		if(value)
		   usage_and_exit(prog, 2, "too many commands");
		type = ECORE_CONFIG_FLT;
		f = atof(optarg);
		value = &f;
		break;
	     case 'r':
		if(value)
		   usage_and_exit(prog, 2, "too many commands");
		type = ECORE_CONFIG_RGB;
		i = (long) strtoul( (*optarg == '#') ? (optarg + 1) : optarg, NULL, 16 );
		value = &i;
		break;
	     case 's':
		if(value)
		   usage_and_exit(prog, 2, "too many commands");
		type = ECORE_CONFIG_STR;
		value = strdup(optarg);
		break;
	     case 't':
		if(value)
		   usage_and_exit(prog, 2, "too many commands");
		type = ECORE_CONFIG_THM;
		value = strdup(optarg);
		break;
	     case 'c':
		if(file)
		   free(file);
		file = strdup(optarg);
		break;
	     case '?':
	     case ':':
		return 1;
	     default:
		cmd = ret;
		break;
	   }
     }

   if(cmd == 's' && type == -1)
     usage_and_exit(prog, 2, "You need to specify a command!");

   if(cmd != 'a' && !key)
     usage_and_exit(prog, 2, "You need to specify key!");
   
   if(ecore_config_init("econfig") != ECORE_CONFIG_ERR_SUCC)
     {
       EINA_LOG_ERR("Couldn't init ecore_config!");
	return 1;
     }

   // Load configuration from file
   ecore_config_file_load(file);

   ret = 0;

   // Execute command
   switch (cmd)
     {
	case 's':
	   if (ecore_config_typed_set(key, value, type) != ECORE_CONFIG_ERR_SUCC)
	     {
		fprintf(stderr, "Set failed for %s", key);
		ret = 1;
	     } else {
		ecore_config_file_save(file);
	     }
	     get(key); // display value after setting it
	   break;
	case 'd':
	   if(del(key))
	     {
		fprintf(stderr, "Delete failed for %s", key);
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
	default:
	   EINA_LOG_ERR("Unhandled command '%c'", cmd);
     }

   ecore_config_shutdown();

   if(type == ECORE_CONFIG_STR || type == ECORE_CONFIG_THM)
     free(value);

   if(file)
     free(file);
   eina_shutdown();
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
