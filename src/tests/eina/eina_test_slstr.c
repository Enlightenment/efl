#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define EINA_SLSTR_INTERNAL
#include <Eina.h>

#include "eina_suite.h"

static Eina_Slstr *
_slstr_copy(void)
{
   const char local[] = "Hello world 1";

   return eina_slstr_copy_new(local);
}

START_TEST(slstr_copy)
{
   Eina_Slstr *str;

   eina_init();

   str = _slstr_copy();
   ck_assert_str_eq(str, "Hello world 1");

   eina_shutdown();
}
END_TEST

static Eina_Slstr *
_slstr_steal(void)
{
   char *local = strdup("Hello world 2");

   return eina_slstr_copy_new(local);
}

START_TEST(slstr_steal)
{
   Eina_Slstr *str;

   eina_init();

   str = _slstr_steal();
   ck_assert_str_eq(str, "Hello world 2");

   eina_shutdown();
}
END_TEST

static Eina_Slstr *
_slstr_stringshare(void)
{
   Eina_Stringshare *str = eina_stringshare_add("Hello world 3");

   return eina_slstr_stringshare_new(str);
}

START_TEST(slstr_stringshare)
{
   Eina_Stringshare *ss;
   Eina_Slstr *str;

   eina_init();

   str = _slstr_stringshare();
   ss = eina_stringshare_add("Hello world 3");
   fail_if(ss != str);

   eina_shutdown();
}
END_TEST

static Eina_Slstr *
_slstr_tmpstr(void)
{
   Eina_Tmpstr *str = eina_tmpstr_add("Hello world 4");

   return eina_slstr_tmpstr_new(str);
}

START_TEST(slstr_tmpstr)
{
   Eina_Slstr *str;

   eina_init();

   str = _slstr_tmpstr();
   ck_assert_str_eq(str, "Hello world 4");

   eina_shutdown();
}
END_TEST

static Eina_Slstr *
_slstr_strbuf(void)
{
   Eina_Strbuf *str = eina_strbuf_new();

   eina_strbuf_append(str, "Hello ");
   eina_strbuf_append(str, "world ");
   eina_strbuf_append_printf(str, "%d", 5);

   return eina_slstr_strbuf_new(str);
}

START_TEST(slstr_strbuf)
{
   Eina_Slstr *str;

   eina_init();

   str = _slstr_strbuf();
   ck_assert_str_eq(str, "Hello world 5");

   eina_shutdown();
}
END_TEST

static Eina_Slstr *
_slstr_printf(int val)
{
   return eina_slstr_printf("Hello %s %d", "world", val);
}

START_TEST(slstr_slstr_printf)
{
   Eina_Slstr *str;

   eina_init();

   str = _slstr_printf(6);
   ck_assert_str_eq(str, "Hello world 6");

   eina_shutdown();
}
END_TEST

static void
_many_do(void)
{
   const int many = 2048;
   Eina_Slstr *str;
   int k;

   for (k = 0; k < many; k++)
     {
        char local[64];

        str = _slstr_printf(k);
        sprintf(local, "Hello world %d", k);
        ck_assert_str_eq(str, local);
     }
}

START_TEST(slstr_many)
{
   eina_init();

   _many_do();

   eina_slstr_local_clear();

   eina_shutdown();
}
END_TEST

static void *
_thread_cb(void *data EINA_UNUSED, Eina_Thread th EINA_UNUSED)
{
   _many_do();

   return NULL;
}

START_TEST(slstr_thread)
{
   const int threads = 8;
   Eina_Thread th[threads];
   int k;

   eina_init();

   for (k = 0; k < threads; k++)
     fail_if(!eina_thread_create(&th[k], EINA_THREAD_NORMAL, -1, _thread_cb, NULL));

   for (k = 0; k < threads; k++)
     eina_thread_join(th[k]);

   eina_slstr_local_clear();

   eina_shutdown();
}
END_TEST

void
eina_test_slstr(TCase *tc)
{
   tcase_add_test(tc, slstr_copy);
   tcase_add_test(tc, slstr_steal);
   tcase_add_test(tc, slstr_stringshare);
   tcase_add_test(tc, slstr_tmpstr);
   tcase_add_test(tc, slstr_strbuf);
   tcase_add_test(tc, slstr_slstr_printf);
   tcase_add_test(tc, slstr_many);
   tcase_add_test(tc, slstr_thread);
}
