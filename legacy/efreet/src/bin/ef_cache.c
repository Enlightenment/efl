#include "Efreet.h"

int
main(int argc, char **argv)
{
    if (!efreet_init()) return 1;
    ecore_main_loop_begin();
    efreet_shutdown();
    return 0;
}
