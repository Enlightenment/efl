#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "eina_stdio.h"

int
eina_vasprintf_wrap(char **ret, const char *fmt, int nargs, ...)
{
   va_list ap;
   va_start(ap, nargs);

   int len = eina_vasprintf(ret, fmt, ap);

   va_end(ap);
   return len;
}

int
simple_test(void)
{
   const char *expected_str = 
     "\tString: Hello World!\n"
     "\tInteger: 46234\n"
     "\tDouble: 9.629400E-03\n";

   const char *fmt = 
     "\tString: %s\n"
     "\tInteger: %d\n"
     "\tDouble: %E\n";

   char* str;
   int len = eina_vasprintf_wrap(&str, fmt, 3, "Hello World!", 46234, 0.0096294);

   printf("## Expecting:\n```%s```\n", expected_str);
   printf("## Returning:\n```%s```\n", str);

   printf("\tResult:"); 
   int result = strcmp(str, expected_str);
   if (!result) printf("........ok!\n");
   else printf("........nok!\n");

   free(str);
   return result;
}

int
main(void)
{
#ifdef GNU
   printf("Test using vasprintf from GNU:\n");
#else
   printf("Test using implementd eina_vasprintf:\n");
#endif

   simple_test();

   return 0;
}
