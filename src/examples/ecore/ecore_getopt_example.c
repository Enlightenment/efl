//Compile with:
// gcc -o ecore_getopt_example ecore_getopt_example.c `pkg-config --libs --cflags ecore eina`

#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <assert.h>

/* if defined will end the positional arguments with the special
 * action ECORE_GETOPT_ACTION_APPEND that will require at least one
 * trailing argument and will also consume the remaining arguments
 * until the end.
 *
 * if not defined unhandled positional arguments start at the index
 * returned by ecore_getopt_parse_positional(), that will be less or
 * equal to argc.
 */
#define END_WITH_POS_APPEND 1

static const char * available_choices[] = {
  "banana",
  "apple",
  "orange",
  NULL /* must be null terminated! */
};

static const Ecore_Getopt options = {
  /* program name, usually a macro PACKAGE_NAME */
  "ecore_getopt_example",
  /* usage line, leave empty to generate one with positional arguments */
  NULL,
  /* program version, usually a macro PACKAGE_VERSION */
  "0.1",
  /* copyright string */
  "(C) 2013 Enlightenment Project",
  /* license string */
  "BSD 2-Clause",
  /* long description, may be multiline and contain \n */
  "Example of Ecore_Getopt usage.\n"
  "\n"
  "This usage may span over multiple lines of text, with automatic line "
  "break based on $COLUMNS environment variable that is usually defined by "
  "your shell.\n"
  "You can have %%prog (expands to \"%prog\") or %%version (expands to \""
  "%version\") in the description to get the program name "
  "or version. Use double %% to get the percentage symbol.\n"
  "OneCanHaveVeryLongWorksInDescriptionLinesSuchAsThisOneAndItWillBeBrokenWhenTheyGoPast${COLUMNS}Characters.\n"
  "\tTab (\\t) is supported, like in the beginning of this line. They "
  "will work as tabulation to columns multiple of 8 spaces, so you can do "
  "tables such as:\n"
  "1\tsomething\tsome description\n"
  "23\totherthing\tsome description\n"
  "456\tyetanother\tsome description\n"
  "12345678\tthis is off\tthis is off\n",
  /* we want strict parsing (any error aborts) */
  EINA_TRUE,
  /* an array of argument descriptions (must terminate with sentinel) */
  {

    /* block of options that store a single value in a variable of type */
    ECORE_GETOPT_STORE_STR(0, "string", "Store a single string."),
    ECORE_GETOPT_STORE_BOOL(0, "bool", "Store a single boolean."),
    ECORE_GETOPT_STORE_SHORT(0, "short", "Store a single short."),
    ECORE_GETOPT_STORE_INT(0, "int", "Store a single integer."),
    ECORE_GETOPT_STORE_LONG(0, "long", "Store a single long integer."),
    ECORE_GETOPT_STORE_USHORT(0, "unsigned-short",
                              "Store a single unsigned short integer."),
    ECORE_GETOPT_STORE_UINT(0, "unsigned-int",
                            "Store a single unsigned integer."),
    ECORE_GETOPT_STORE_ULONG(0, "unsigned-long",
                             "Store a single unsigned long integer."),
    ECORE_GETOPT_STORE_DOUBLE(0, "double", "Store a single double."),

    /* block of options that store a single value in a variable of type
     * and use a default value if option IS SPECIFIED but no value is given
     * using =VALUE.
     * If option -o has default value of X, then the command lines produce:
     *    <empty>: nothing is set.
     *    -o: value is set to X.
     *    -o=Y: value is set to Y.
     */
    ECORE_GETOPT_STORE_DEF_STR(0, "default-string", "Store a single string.",
                               "default-string-value"),
    ECORE_GETOPT_STORE_DEF_BOOL(0, "default-bool", "Store a single boolean.",
                                EINA_TRUE),
    ECORE_GETOPT_STORE_DEF_SHORT(0, "default-short", "Store a single short.",
                                 123),
    ECORE_GETOPT_STORE_DEF_INT(0, "default-int", "Store a single integer.",
                               1234),
    ECORE_GETOPT_STORE_DEF_LONG(0, "default-long",
                                "Store a single long integer.", 12345),
    ECORE_GETOPT_STORE_DEF_USHORT(0, "default-unsigned-short",
                                  "Store a single unsigned short integer.",
                                  123),
    ECORE_GETOPT_STORE_DEF_UINT(0, "default-unsigned-int",
                                "Store a single unsigned integer.",
                                1234),
    ECORE_GETOPT_STORE_DEF_ULONG(0, "default-unsigned-long",
                                 "Store a single unsigned long integer.",
                                 12345),
    ECORE_GETOPT_STORE_DEF_DOUBLE(0, "default-double",
                                  "Store a single double.",
                                  12.345),

    /* you can specify the metavar so the --help will be more meaningful */
    ECORE_GETOPT_STORE_METAVAR_STR(0, "output", "Specify output file.",
                                   "FILENAME"),

    /* Other than storing a given value (or default), it is common to
     * have boolean options (ie: --debug, --daemon), those that set a
     * constant value, counters (ie: --verbose) or option from a fixed
     * set of choices.
     */
    ECORE_GETOPT_STORE_TRUE(0, "enable-x", "Enables X."),
    ECORE_GETOPT_STORE_FALSE(0, "disable-y", "Disables Y."),
    ECORE_GETOPT_STORE_CONST(0, "set-z", "Set z to constant XPTO.", "XPTO"),
    ECORE_GETOPT_COUNT(0, "countme",
                       "Counts number of times this option is given."),
    ECORE_GETOPT_CHOICE(0, "choose", "Choose from one of the options",
                        available_choices),

    /* one can create a list of given values of a certain type */
    ECORE_GETOPT_APPEND(0, "append-string", "Store multiple strings.",
                        ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_APPEND(0, "append-int", "Store multiple integers.",
                        ECORE_GETOPT_TYPE_INT),

    /* break options will force everything that goes after it to be ignored
     * by the option parser and they will go as arguments. This is the case
     * for xterm's -e. Example:
     *   program --string=A: stores "A" into str_value.
     *   program --string=A --break: still stores "A" into str_value.
     *   program --break --string=A: str_value is untouched, --string=a
     *                               is avaiable in argv[args], with
     *                               args being the index returned by
     *                               ecore_getopt_parse()
     *
     * Note that ecore_getopt will follow GNU and stop parsing arguments
     * once -- is found, similar to "rm -- -fr /". In this case the
     * return of ecore_getopt_parse() is to the index of "--" element in
     * argv[].
     */
    ECORE_GETOPT_BREAK(0, "break"),

    /* standard block to provide version, copyright, license and help */
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_HELP('h', "help"),

    /* positional arguments can be handled as well, add their
     * description after the last option was specified. They should
     * have empty short and long options, but have the metavar
     * defined.
     */
    ECORE_GETOPT_STORE_METAVAR_STR(0, NULL, "Positional string.", "STRING"),
    ECORE_GETOPT_STORE_METAVAR_INT(0, NULL, "Positional integer.", "INT"),
    ECORE_GETOPT_CHOICE_METAVAR(0, NULL, "Positional choice.", "CHOICE",
                                available_choices),

#ifdef END_WITH_POS_APPEND
    /* this will consume until the end of the command line, forcing
     * ecore_getopt_parse() to return args == argc on succes.
     * It will require at least one argument in the end of the command line.
     */
    ECORE_GETOPT_APPEND_METAVAR(0, NULL, "Extra options.", "ARG",
                                ECORE_GETOPT_TYPE_STR),
#endif

    /* the sentinel is required to notify end of descriptions */
    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char **argv)
{
   char *str_value = NULL;
   Eina_Bool bool_value = EINA_FALSE;
   short short_value = 0;
   int int_value = 0;
   long long_value = 0;
   unsigned short ushort_value = 0;
   unsigned int uint_value = 0;
   unsigned long ulong_value = 0;
   double double_value = 0;
   char *def_str_value = NULL;
   Eina_Bool def_bool_value = EINA_FALSE;
   short def_short_value = 0;
   int def_int_value = 0;
   long def_long_value = 0;
   unsigned short def_ushort_value = 0;
   unsigned int def_uint_value = 0;
   unsigned long def_ulong_value = 0;
   double def_double_value = 0;
   char *output_value = NULL;
   Eina_Bool use_x = EINA_FALSE; /* if stores true, then start with false */
   Eina_Bool use_y = EINA_TRUE; /* if stores false, then start with true */
   char *use_z = NULL; /* stores a pointer here */
   int count = 0;
   char *choice = NULL;
   Eina_List *lst_strs = NULL;
   Eina_List *lst_ints = NULL;
   Eina_Bool break_given = EINA_FALSE;
   Eina_Bool quit_option = EINA_FALSE;
   char *pos_str = NULL;
   int pos_int = 0;
   char *pos_choice = NULL;
#ifdef END_WITH_POS_APPEND
   Eina_List *pos_args = NULL;
#endif
   Ecore_Getopt_Value values[] = {
     /* block of options that store a single value in a variable of type */
     ECORE_GETOPT_VALUE_STR(str_value),
     ECORE_GETOPT_VALUE_BOOL(bool_value),
     ECORE_GETOPT_VALUE_SHORT(short_value),
     ECORE_GETOPT_VALUE_INT(int_value),
     ECORE_GETOPT_VALUE_LONG(long_value),
     ECORE_GETOPT_VALUE_USHORT(ushort_value),
     ECORE_GETOPT_VALUE_UINT(uint_value),
     ECORE_GETOPT_VALUE_ULONG(ulong_value),
     ECORE_GETOPT_VALUE_DOUBLE(double_value),

     /* you can use options with default value (if =VALUE is omitted) */
     ECORE_GETOPT_VALUE_STR(def_str_value),
     ECORE_GETOPT_VALUE_BOOL(def_bool_value),
     ECORE_GETOPT_VALUE_SHORT(def_short_value),
     ECORE_GETOPT_VALUE_INT(def_int_value),
     ECORE_GETOPT_VALUE_LONG(def_long_value),
     ECORE_GETOPT_VALUE_USHORT(def_ushort_value),
     ECORE_GETOPT_VALUE_UINT(def_uint_value),
     ECORE_GETOPT_VALUE_ULONG(def_ulong_value),
     ECORE_GETOPT_VALUE_DOUBLE(def_double_value),

     /* example of metavar usage */
     ECORE_GETOPT_VALUE_STR(output_value),

     /* example of store true, false, const */
     ECORE_GETOPT_VALUE_BOOL(use_x),
     ECORE_GETOPT_VALUE_BOOL(use_y),
     ECORE_GETOPT_VALUE_STR(use_z),
     ECORE_GETOPT_VALUE_INT(count),
     ECORE_GETOPT_VALUE_STR(choice),

     /* example of append multiple options */
     ECORE_GETOPT_VALUE_LIST(lst_strs),
     ECORE_GETOPT_VALUE_LIST(lst_ints),

     /* example of break option */
     ECORE_GETOPT_VALUE_BOOL(break_given),

     /* standard block to provide version, copyright, license and help */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -V/--version quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -C/--copyright quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -L/--license quits */
     ECORE_GETOPT_VALUE_BOOL(quit_option), /* -h/--help quits */

     /* example of positiona argument */
     ECORE_GETOPT_VALUE_STR(pos_str),
     ECORE_GETOPT_VALUE_INT(pos_int),
     ECORE_GETOPT_VALUE_STR(pos_choice),

#ifdef END_WITH_POS_APPEND
     ECORE_GETOPT_VALUE_LIST(pos_args),
#endif

     ECORE_GETOPT_VALUE_NONE /* sentinel */
   };
   int args, retval = EXIT_SUCCESS;

   ecore_init();

   /* during development is recommended to use the following to check
    * for duplicated options.
    */
   assert(ecore_getopt_parser_has_duplicates(&options) == EINA_FALSE);

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fputs("ERROR: Could not parse command line options.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   /* options that set 'quit_option' to true requires us to exit. */
   if (quit_option) goto end;

   args = ecore_getopt_parse_positional(&options, values, argc, argv, args);
   if (args < 0)
     {
        fputs("ERROR: Could not parse positional arguments.\n", stderr);
        retval = EXIT_FAILURE;
        goto end;
     }

   printf("given values:\n"
          "string = %s\n"
          "bool = %s\n"
          "short = %hd\n"
          "int = %d\n"
          "long = %ld\n"
          "unsigned-short = %hu\n"
          "unsigned-int = %u\n"
          "unsigned-long = %lu\n"
          "double = %f\n"
          "\n"
          "default-string = %s\n"
          "default-bool = %s\n"
          "default-short = %hd\n"
          "default-int = %d\n"
          "default-long = %ld\n"
          "default-unsigned-short = %hu\n"
          "default-unsigned-int = %u\n"
          "default-unsigned-long = %lu\n"
          "default-double = %f\n"
          "\n"
          "output = %s\n"
          "use-x = %s (disabled by default)\n"
          "use-y = %s (enabled by default)\n"
          "use-it = %s\n"
          "counted = %d --countme\n"
          "choice = %s\n"
          "\n"
          "--break = %s\n"
          "\nDeclared Positional:\n"
          "STRING = %s\n"
          "INT = %d\n"
          "CHOICE = %s\n"
          "\n",
          str_value,
          bool_value ? "true" : "false",
          short_value,
          int_value,
          long_value,
          ushort_value,
          uint_value,
          ulong_value,
          double_value,
          def_str_value,
          def_bool_value ? "true" : "false",
          def_short_value,
          def_int_value,
          def_long_value,
          def_ushort_value,
          def_uint_value,
          def_ulong_value,
          def_double_value,
          output_value,
          use_x ? "enabled" : "disabled",
          use_y ? "enabled" : "disabled",
          use_z,
          count,
          choice,
          break_given ? "given" : "omitted",
          pos_str,
          pos_int,
          pos_choice);

   if (!lst_strs)
     puts("no --append-string=VALUE was given.");
   else
     {
        char *str;
        printf("%u strings given with --append-string=VALUE:\n",
               eina_list_count(lst_strs));
        EINA_LIST_FREE(lst_strs, str)
          {
             printf("\t%s\n", str);
             free(str);
          }
     }

   if (!lst_ints)
     puts("no --append-int=VALUE was given.");
   else
     {
        int *pi;
        printf("%u integers given with --append-int=VALUE:\n",
               eina_list_count(lst_ints));
        EINA_LIST_FREE(lst_ints, pi)
          {
             printf("\t%d\n", *pi);
             free(pi);
          }
     }

#ifdef END_WITH_POS_APPEND
   assert(pos_args != NULL);
   assert(args == argc);
   if (1)
     {
        char *str;
        printf("%u extra arguments:\n",
               eina_list_count(pos_args));
        EINA_LIST_FREE(pos_args, str)
          {
             printf("\t%s\n", str);
             free(str);
          }
     }
#else
   if (args == argc)
     puts("no extra positional arguments were given.");
   else
     {
        printf("%d positional arguments were given:\n", argc - args);
        for (; args < argc; args++)
          printf("\t%s\n", argv[args]);
     }
#endif

 end:
   ecore_shutdown();
   return retval;
}
