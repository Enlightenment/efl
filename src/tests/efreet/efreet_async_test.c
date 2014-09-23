#include "config.h" 

#include <unistd.h>
#include <Eina.h>
#include <Ecore.h>
#include <Efreet.h>

#define TCOUNT 10
#define MCOUNT 10
#define MENU "/etc/xdg/menus/applications.menu"

static int count = 0;
static Eina_Lock lock;
static Eina_Thread threads[TCOUNT];

static void
menu_cb(void *data EINA_UNUSED, Efreet_Menu *menu)
{
    efreet_menu_dump(menu, "");
    eina_lock_take(&lock);
    count++;
    fprintf(stderr, "count: %d\n", count);
    if (count == (MCOUNT * TCOUNT)) ecore_main_loop_quit();
    eina_lock_release(&lock);
    efreet_menu_free(menu);
}

static void *
thread_cb(void *data EINA_UNUSED, Eina_Thread t EINA_UNUSED)
{
    int i;
    for (i = 0; i < MCOUNT; i++) {
        efreet_menu_async_parse(MENU, menu_cb, NULL);
        usleep(1e5);
    }
    return NULL;
}

static void
create_threads(void)
{
    int i;

    for (i = 0; i < TCOUNT; i++)
    {
        if (!eina_thread_create(&threads[i], EINA_THREAD_NORMAL, -1, thread_cb, NULL))
            fprintf(stderr, "error: eina_thread_create\n");
        usleep(1e5);
    }
}

int
main(void)
{
    int i;

    eina_init();
    ecore_init();
    efreet_init();

    eina_lock_new(&lock);

    create_threads();
    ecore_main_loop_begin();
    for (i = 0; i < TCOUNT; i++)
        eina_thread_join(threads[i]);

    eina_lock_free(&lock);

    efreet_shutdown();
    ecore_shutdown();
    eina_shutdown();
    return 0;
}
