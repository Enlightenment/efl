#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Eina.h>

#include "eina_suite.h"

EFL_START_TEST(eina_crc_simple)
{
   unsigned int seed = 0xffffffff, i, ret;
   const char *data[] =
     {
        "Hello World",
        "This is to test crc",
        "4baAsru=A$r&e.-",
        "=-.^Y@~Lp5e1)b^",
        "J(uhX4)!&Q#2,jr"
     };

   unsigned int result[] =
     {
        0x4a17b156,
        0x738bec38,
        0xcd56f3c6,
        0x9732147f,
        0x12c660a3
     };
   const char *s1 = "Hello ";
   const char *s2 = "World";

   for (i = 0; i < sizeof(data) / sizeof(data[0]); ++i)
     {
        ret = eina_crc(data[i], strlen(data[i]), seed, EINA_TRUE);
        fail_if(ret != result[i]);
     }

   ret = eina_crc(s1, strlen(s1), 0xffffffff, EINA_TRUE);
   ret = eina_crc(s2, strlen(s2), ret, EINA_FALSE);
   fail_if(ret != result[0]);
}
EFL_END_TEST

void eina_test_crc(TCase *tc)
{
   tcase_add_test(tc, eina_crc_simple);
}
