/* X Windows Example
 */

#include <unistd.h>
#include <Ecore.h>
#include <Ecore_X.h>

int main (int argc, char *argv[]) {
  Ecore_X_Window win;

  ecore_x_init(NULL);
  
  win = ecore_x_window_new(0, 20, 20, 200, 200);
  ecore_x_window_show(win);
  ecore_x_flush(); // Ensure requests are sent to the server.
  usleep(3000000); // Sleep for 3 seconds.

  ecore_x_shutdown();
  return 0;
}
