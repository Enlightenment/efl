// ld: libeina.so

#include "test_macros.h"
#include "eina_config.h"
#include "Eina.h"
#include "Efl_Config.h"

#include XSTR(TESTC)

int main(void)
{
  eina_init();
  CALL();
  eina_shutdown();
  return 0;
}

