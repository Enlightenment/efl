//Compile with:
//gcc -Wall -o eina_log_02 eina_log_02.c `pkg-config --cflags --libs eina`

#include <stdlib.h>
#include <stdio.h>

#include <Eina.h>

void test(int i)
{
   EINA_LOG_DBG("Entering test");

   if (i < 0)
     {
         EINA_LOG_ERR("Argument is negative");
         return;
     }

   EINA_LOG_INFO("argument non negative");

   EINA_LOG_DBG("Exiting test");
}

int main(void)
{
   if (!eina_init())
     {
         printf("log during the initialization of Eina_Log module\n");
         return EXIT_FAILURE;
     }

   test(-1);
   test(0);

   eina_shutdown();

   return EXIT_SUCCESS;
}
