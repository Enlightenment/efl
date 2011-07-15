/*
 * build: gcc -o eet_file eet-file.c `pkg-config --cflags --libs eet`
 */
#include <Eet.h>
#include <stdio.h>
#include <string.h>

static int
create_eet_file(void)
{
   Eet_File *ef;
   char buf[1024], *ptr;
   int size, len, i;
   const char *some_strings[] = {
        "And some more strings",
        "spread across several",
        "elements of an array!"
   };
   const char some_data[] =
      "\x1e\xe7\x0f\x42\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x35"
      "\x00\x00\x00\xa0\x00\x00\x00\xa0\x00\x00\x00\x24\x00\x00\x00\x11"
      "\x00\x00\x00\x00\x2f\x6d\x69\x73\x74\x65\x72\x69\x6f\x75\x73\x2f"
      "\x64\x61\x74\x61\x00\x41\x6e\x20\x45\x45\x54\x20\x69\x6e\x73\x69"
      "\x64\x65\x20\x6f\x66\x20\x61\x6e\x20\x45\x45\x54\x21\x0a\x54\x68"
      "\x69\x73\x20\x77\x61\x73\x6e\x27\x74\x20\x72\x65\x61\x6c\x6c\x79"
      "\x20\x75\x73\x65\x66\x75\x6c\x20\x62\x75\x74\x20\x69\x74\x20\x68"
      "\x65\x6c\x70\x65\x64\x20\x74\x6f\x20\x73\x68\x6f\x77\x20\x68\x6f"
      "\x77\x0a\x74\x6f\x20\x75\x73\x65\x20\x65\x65\x74\x5f\x6d\x65\x6d"
      "\x6f\x70\x65\x6e\x5f\x72\x65\x61\x64\x28\x29\x20\x74\x6f\x20\x6f"
      "\x70\x65\x6e\x20\x61\x6e\x20\x65\x65\x74\x20\x66\x69\x6c\x65\x20"
      "\x66\x72\x6f\x6d\x0a\x64\x61\x74\x61\x20\x61\x6c\x72\x65\x61\x64"
      "\x79\x20\x6c\x6f\x61\x64\x65\x64\x20\x69\x6e\x20\x6d\x65\x6d\x6f"
      "\x72\x79\x2e\x0a\x00";

   ef = eet_open("/tmp/my_file.eet", EET_FILE_MODE_WRITE);
   if (!ef) return 0;

   strcpy(buf, "Here is a string of data to save!");
   size = eet_write(ef, "/key/to_store/at", buf, sizeof(buf), 1);
   if (!size)
     {
        fprintf(stderr, "Error writing data!\n");
        eet_close(ef);
        return 0;
     }
   len = strlen(buf);
   printf("strlen() = %d, eet_write() = %d\n", len, size);

   ptr = buf;
   for (i = 0; i < 3; i++)
     {
        len = strlen(some_strings[i]) + 1;
        memcpy(ptr, some_strings[i], len);
        ptr += len;
     }
   eet_write(ef, "/several/strings", buf, sizeof(buf), 1);
   eet_sync(ef);

   eet_write(ef, "/some/mysterious/data", some_data, sizeof(some_data) - 1, 1);

   eet_delete(ef, "/several/strings");

   return (eet_close(ef) == EET_ERROR_NONE);
}

int
main(void)
{
  Eet_File *ef;
  char *ret, **list;
  int size, num, i;

  eet_init();

  if (!create_eet_file())
    return -1;

  ef = eet_open("/tmp/my_file.eet", EET_FILE_MODE_READ);
  if (!ef) return -1;

  list = eet_list(ef, "*", &num);
  if (list)
    {
      for (i = 0; i < num; i++)
        printf("Key stored: %s\n", list[i]);
      free(list);
    }

  ret = eet_read(ef, "/key/to_store/at", &size);
  if (ret)
    {
      printf("Data read (%i bytes):\n%s\n", size, ret);
      free(ret);
    }

  ret = eet_read(ef, "/several/strings", &size);
  if (ret)
    {
       printf("More data read (%i bytes):\n%s\n", size, ret);
       free(ret);
    }

  ret = eet_read(ef, "/some/mysterious/data", &size);
  if (ret)
    {
       Eet_File *ef2;

       ef2 = eet_memopen_read(ret, size);

       num = eet_num_entries(ef2);
       printf("Mysterious data has %d entries\n", num);

       printf("Mysterious data:\n%s\n",
              (char *)eet_read_direct(ef2, "/mysterious/data", NULL));

       eet_close(ef2);

       free(ret);
    }

  eet_close(ef);

  eet_shutdown();

  return 0;
}
