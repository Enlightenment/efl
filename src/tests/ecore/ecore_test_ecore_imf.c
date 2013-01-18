#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore_IMF.h>

#include "ecore_suite.h"

START_TEST(ecore_test_ecore_imf_init)
{
   ecore_imf_init();
   ecore_imf_shutdown();
}
END_TEST

static const char *built_modules[] = {
#ifdef ENABLE_XIM
  "xim",
#endif
#ifdef BUILD_ECORE_IMF_IBUS
  "ibus",
#endif
#ifdef BUILD_ECORE_IMF_SCIM
  "scim",
#endif
  NULL
};

static Eina_Bool
_find_list(const Eina_List *lst, const char *item)
{
   const Eina_List *n;
   const char *s;
   EINA_LIST_FOREACH(lst, n, s)
     {
        if (strcmp(s, item) == 0)
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

START_TEST(ecore_test_ecore_imf_modules)
{
   Eina_List *modules;
   const char **itr;

   ecore_imf_init();
   modules = ecore_imf_context_available_ids_get();

   for (itr = built_modules; *itr != NULL; itr++)
     {
        Eina_Bool found = _find_list(modules, *itr);
        fail_if(!found, "imf module should be built, but was not found: %s",
                *itr);
     }

   eina_list_free(modules);
   ecore_imf_shutdown();
}
END_TEST

START_TEST(ecore_test_ecore_imf_modules_load)
{
   const char **itr;

   ecore_imf_init();

   for (itr = built_modules; *itr != NULL; itr++)
     {
        Ecore_IMF_Context *ctx = ecore_imf_context_add(*itr);
        fail_if(ctx == NULL, "could not add imf context: %s", *itr);
        ecore_imf_context_del(ctx);
     }

   ecore_imf_shutdown();
}
END_TEST

void ecore_test_ecore_imf(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_imf_init);
   tcase_add_test(tc, ecore_test_ecore_imf_modules);
   tcase_add_test(tc, ecore_test_ecore_imf_modules_load);
}
