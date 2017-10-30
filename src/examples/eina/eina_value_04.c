//Compile with:
//gcc eina_value_04.c -o eina_value_04 `pkg-config --cflags --libs eina`

#include <Eina.h>
#include <time.h>

int main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Eina_Value tm_val;
   time_t rawtime;
   struct tm *timeinfo, timeinfo2;
   char buf[1024];

   eina_init();

   eina_value_setup(&tm_val, EINA_VALUE_TYPE_TM);

   time(&rawtime);
   timeinfo = localtime(&rawtime);

   eina_value_set(&tm_val, *timeinfo);
   strftime(buf, 1024, "Now its %d/%m/%y", timeinfo);
   printf("%s\n", buf);

   eina_value_get(&tm_val, &timeinfo2);
   strftime(buf, 1024, "Copy: %d/%m/%y", &timeinfo2);
   printf("%s\n", buf);

   eina_value_free(&tm_val);

   eina_shutdown();
}
