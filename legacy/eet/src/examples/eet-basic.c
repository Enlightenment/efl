#include <Eet.h>

int
main(void)
{
  Eet_File *ef;
  char     *ret;
  int       size;
  char     *entries[] =
    {
      "Entry 1",
      "Big text string here compared to others",
      "Eet is cool"
    };

  eet_init();

  // blindly open an file for output and write strings with their NUL char
  ef = eet_open("test.eet", EET_FILE_MODE_WRITE);
  eet_write(ef, "Entry 1", entries[0], strlen(entries[0]) + 1, 0);
  eet_write(ef, "Entry 2", entries[1], strlen(entries[1]) + 1, 1);
  eet_write(ef, "Entry 3", entries[2], strlen(entries[2]) + 1, 0);
  eet_close(ef);

  // open the file again and blindly get the entries we wrote
  ef = eet_open("test.eet", EET_FILE_MODE_READ);
  ret = eet_read(ef, "Entry 1", &size);
  printf("%s\n", ret);
  free(ret);
  ret = eet_read(ef, "Entry 2", &size);
  printf("%s\n", ret);
  free(ret);
  ret = eet_read(ef, "Entry 3", &size);
  printf("%s\n", ret);
  free(ret);
  eet_close(ef);

  eet_shutdown();
}
