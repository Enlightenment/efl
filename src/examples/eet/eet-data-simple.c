/*
 * build: gcc -o eet_data_simple eet-data-simple.c `pkg-config --cflags --libs eet eina`
 */
#include <Eina.h>
#include <Eet.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// The struct that will be loaded and saved.
// note that only the members described in the eet_data_descriptor
// will be automatically handled. The other members will have their
// space reserved and zeroed (as it uses calloc()), but not
// saved or loaded from eet files.
typedef struct
{
   unsigned int version; // it is recommended to use versioned configuration!
   const char  *name;
   int          id;
   int          not_saved_value; // example of not saved data inside!
   Eina_Bool    enabled;
} My_Conf_Type;

// string that represents the entry in eet file, you might like to have
// different profiles or so in the same file, this is possible with
// different strings
static const char MY_CONF_FILE_ENTRY[] = "config";

// keep the descriptor static global, so it can be
// shared by different functions (load/save) of this and only this
// file.
static Eet_Data_Descriptor *_my_conf_descriptor;

static void
_my_conf_descriptor_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   // The class describe the functions to use to create the type and its
   // full allocated size.
   //
   // Eina types are very convenient, so use them to create the descriptor,
   // so we get eina_list,  eina_hash and eina_stringshare automatically!
   //
   // The STREAM variant is better for configuration files as the values
   // will likely change a lot.
   //
   // The other variant, FILE, is good for caches and things that are just
   // appended, but needs to take care when changing strings and files must
   // be kept open so mmap()ed strings will be kept alive.
   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, My_Conf_Type);
   _my_conf_descriptor = eet_data_descriptor_stream_new(&eddc);

   // Describe the members to be saved:
   // Use a temporary macro so we don't type a lot, also avoid errors:
#define MY_CONF_ADD_BASIC(member, eet_type) \
  EET_DATA_DESCRIPTOR_ADD_BASIC             \
    (_my_conf_descriptor, My_Conf_Type, # member, member, eet_type)

   MY_CONF_ADD_BASIC(version, EET_T_UINT);
   MY_CONF_ADD_BASIC(name, EET_T_STRING);
   MY_CONF_ADD_BASIC(id, EET_T_INT);
   MY_CONF_ADD_BASIC(enabled, EET_T_UCHAR);

#undef MY_CONF_ADD_BASIC
} /* _my_conf_descriptor_init */

static void
_my_conf_descriptor_shutdown(void)
{
   eet_data_descriptor_free(_my_conf_descriptor);
} /* _my_conf_descriptor_shutdown */

static My_Conf_Type *
_my_conf_new(void)
{
   My_Conf_Type *my_conf = calloc(1, sizeof(My_Conf_Type));
   if (!my_conf)
     {
        fprintf(stderr, "ERROR: could not calloc My_Conf_Type\n");
        return NULL;
     }

   my_conf->version = 0x112233;
   my_conf->enabled = EINA_TRUE;
   return my_conf;
} /* _my_conf_new */

static void
_my_conf_free(My_Conf_Type *my_conf)
{
   eina_stringshare_del(my_conf->name);
   free(my_conf);
} /* _my_conf_free */

static My_Conf_Type *
_my_conf_load(const char *filename)
{
   My_Conf_Type *my_conf;
   Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
   if (!ef)
     {
        fprintf(stderr, "ERROR: could not open '%s' for read\n", filename);
        return NULL;
     }

   my_conf = eet_data_read(ef, _my_conf_descriptor, MY_CONF_FILE_ENTRY);
   if (!my_conf)
     goto end;

   if (my_conf->version < 0x112233)
     {
        fprintf(stderr,
                "WARNING: version %#x was too old, upgrading it to %#x\n",
                my_conf->version, 0x112233);

        my_conf->version = 0x112233;
        my_conf->enabled = EINA_TRUE;
     }

end:
   eet_close(ef);
   return my_conf;
} /* _my_conf_load */

static Eina_Bool
_my_conf_save(const My_Conf_Type *my_conf,
              const char         *filename)
{
   char tmp[PATH_MAX];
   Eet_File *ef;
   Eina_Bool ret;
   unsigned int i, len;
   struct stat st;

   len = eina_strlcpy(tmp, filename, sizeof(tmp));
   if (len + 12 >= (int)sizeof(tmp))
     {
        fprintf(stderr, "ERROR: file name is too big: %s\n", filename);
        return EINA_FALSE;
     }

   i = 0;
   do
     {
        snprintf(tmp + len, 12, ".%u", i);
        i++;
     }
   while (stat(tmp, &st) == 0);

   ef = eet_open(tmp, EET_FILE_MODE_WRITE);
   if (!ef)
     {
        fprintf(stderr, "ERROR: could not open '%s' for write\n", tmp);
        return EINA_FALSE;
     }

   ret = eet_data_write
       (ef, _my_conf_descriptor, MY_CONF_FILE_ENTRY, my_conf, EINA_TRUE);
   eet_close(ef);

   if (ret)
     {
        unlink(filename);
        rename(tmp, filename);
     }

   return ret;
} /* _my_conf_save */

int
main(int   argc,
     char *argv[])
{
   My_Conf_Type *my_conf;
   int ret = 0;

   if (argc != 3)
     {
        fprintf(stderr, "Usage:\n\t%s <input> <output>\n\n", argv[0]);
        return -1;
     }

   eina_init();
   eet_init();
   _my_conf_descriptor_init();

   my_conf = _my_conf_load(argv[1]);
   if (!my_conf)
     {
        printf("creating new configuration.\n");
        my_conf = _my_conf_new();
        if (!my_conf)
          {
             ret = -2;
             goto end;
          }
     }

   printf("My_Conf_Type:\n"
          "\tversion: %#x\n"
          "\tname...: '%s'\n"
          "\tid.....: %d\n"
          "\tenabled: %hhu\n",
          my_conf->version,
          my_conf->name ? my_conf->name : "",
          my_conf->id,
          my_conf->enabled);

   if (!_my_conf_save(my_conf, argv[2]))
     ret = -3;

   _my_conf_free(my_conf);

end:
   _my_conf_descriptor_shutdown();
   eet_shutdown();
   eina_shutdown();

   return ret;
} /* main */

