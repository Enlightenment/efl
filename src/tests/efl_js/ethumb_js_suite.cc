
#include "suite_runner.hh"

int main(int, char*[])
{
  return run_script(TESTS_SRC_DIR "/ethumb_js_suite.js", nullptr);
}
