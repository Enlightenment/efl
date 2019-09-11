// Include file for interfaces .eo.c files
#define DUMMY_TEST_IFACE_PROTECTED

#include "libefl_mono_native_test.h"

void _dummy_test_iface_static_prop_set(int data EINA_UNUSED)
{
}

int _dummy_test_iface_static_prop_get()
{
    return -1;
}

#include "dummy_test_iface.eo.c"
#include "dummy_inherit_iface.eo.c"
