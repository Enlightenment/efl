#undef EFL_BETA_API_SUPPORT
#define EFL_NOLEGACY_API_SUPPORT

#include <Efl_Ui.h>

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev EINA_UNUSED)
{
}

EFL_MAIN()
