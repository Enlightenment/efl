
#include "suite_runner.hh"

int main(int, char*[])
{
  return run_script(TESTS_SRC_DIR "/eina_js_suite.js", nullptr);
}
