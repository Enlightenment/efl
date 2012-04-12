#include "Eobj.h"
#include "simple.h"
#include "comp.h"

static int cb_called = EINA_FALSE;

static Eina_Bool
_a_changed_cb(void *data, Eobj *obj, const Eobj_Event_Description *desc, void *event_info)
{
   (void) desc;
   (void) obj;
   int new_a = *((int *) event_info);
   printf("%s event_info:'%d' data:'%s'\n", __func__, new_a, (const char *) data);

   cb_called = EINA_TRUE;

   return EINA_TRUE;
}

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(COMP_CLASS, NULL);
   eobj_event_callback_add(obj, SIG_A_CHANGED, _a_changed_cb, NULL);

   int a;
   eobj_do(obj, SIMPLE_A_SET(1));
   if (!cb_called)
     {
        printf("Error! %d\n", __LINE__);
        return 1;
     }

   eobj_do(obj, SIMPLE_A_GET(&a));
   if (a != 1)
     {
        printf("Error! %d\n", __LINE__);
        return 1;
     }
   eobj_unref(obj);

   eobj_shutdown();
   return 0;
}

