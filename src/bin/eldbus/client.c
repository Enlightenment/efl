#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "codegen.h"

char *output_dir = NULL;

static const Ecore_Getopt optdesc = {
  "eldbus_codegen",
  "%prog [options] <file.xml>",
  "1.0",
  "(C) 2012 - The Enlightenment Project",
  "BSD",
  "Generate DBUS code, making write DBus client applications easy\n",
  0,
  {
    ECORE_GETOPT_STORE_STR('p', "prefix", "The prefix for the generated code."),
    ECORE_GETOPT_STORE_STR('i', "interface", "To generate code of only one interface of xml."),
    ECORE_GETOPT_STORE_STR('o', "output file name", "The name of output files, only used if a interface is selected."),
    ECORE_GETOPT_STORE_STR('O', "output dir", "The directory to output files to."),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char **argv)
{
   char *xml_buf = NULL;
   DBus_Object *obj = NULL;
   char *prefix = NULL, *interface = NULL, *output = NULL;
   Eina_Bool quit_option = EINA_FALSE;
   int arg_index;
   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_STR(prefix),
      ECORE_GETOPT_VALUE_STR(interface),
      ECORE_GETOPT_VALUE_STR(output),
      ECORE_GETOPT_VALUE_STR(output_dir),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_NONE
   };

   eina_init();

   arg_index = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (arg_index == -1)
     {
        printf("Error: parsing arguments.\n");
        return -1;
     }
   if (quit_option)
     return 0;
   if (argc <= arg_index)
     {
        printf("Error: Missing file name.\n");
        return -1;
     }

   if (!file_read(argv[arg_index], &xml_buf))
     {
        printf("Error reading file.\n");
        goto end;
     }

   if (!eina_simple_xml_parse(xml_buf, strlen(xml_buf), EINA_TRUE,
                              parser, &obj))
     {
        printf("Error: could not parse XML\n");
        goto end;
     }

   source_client_generate(obj, prefix, interface, output);
   util_h_write();

end:
   if (obj)
     object_free(obj);

   free(xml_buf);
   eina_shutdown();
   return 0;
}
