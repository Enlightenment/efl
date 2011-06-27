//Compile with:
//gcc -Wall -o eina_log_01 eina_log_01.c `pkg-config --cflags --libs eina`

#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>

void test_warn(void)
{
   EINA_LOG_WARN("Here is a warning message");
}

int main(void)
{
   if (!eina_init())
     {
        printf("log during the initialization of Eina_Log module\n");
        return EXIT_FAILURE;
     }

   test_warn();

   eina_shutdown();

   return EXIT_SUCCESS;
}
