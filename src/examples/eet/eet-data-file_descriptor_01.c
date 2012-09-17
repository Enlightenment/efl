/*
 * build: gcc -o eet_data_file_descriptor eet-data-file_descriptor.c `pkg-config --cflags --libs eet eina`
 */
#include <Eina.h>
#include <Eet.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// complex real-world structures based on elmdentica database
typedef struct
{
   const char  *screen_name;
   const char  *name;
   const char  *message;
   unsigned int id;
   unsigned int status_id;
   unsigned int date;
   unsigned int timeline;
} My_Message;

typedef struct
{
   const char *dm_to;
   const char *message;
} My_Post;

typedef struct
{
   unsigned int id;
   const char  *name;
   Eina_List   *messages;
   My_Post     *posts;
   int          posts_count;
} My_Account;

typedef struct
{
   unsigned int version; // it is recommended to use versioned configuration!
   Eina_Hash   *accounts;
} My_Cache;

// string that represents the entry in eet file, you might like to have
// different profiles or so in the same file, this is possible with
// different strings
static const char MY_CACHE_FILE_ENTRY[] = "cache";

// keep the descriptor static global, so it can be
// shared by different functions (load/save) of this and only this
// file.
static Eet_Data_Descriptor *_my_cache_descriptor;
static Eet_Data_Descriptor *_my_account_descriptor;
static Eet_Data_Descriptor *_my_message_descriptor;
static Eet_Data_Descriptor *_my_post_descriptor;

// keep file handle alive, so mmap()ed strings are all alive as well
static Eet_File *_my_cache_file = NULL;
static Eet_Dictionary *_my_cache_dict = NULL;

static void
_my_cache_descriptor_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   // The FILE variant is good for caches and things that are just
   // appended, but needs to take care when changing strings and files must
   // be kept open so mmap()ed strings will be kept alive.
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, My_Cache);
   _my_cache_descriptor = eet_data_descriptor_file_new(&eddc);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, My_Account);
   _my_account_descriptor = eet_data_descriptor_file_new(&eddc);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, My_Message);
   _my_message_descriptor = eet_data_descriptor_file_new(&eddc);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, My_Post);
   _my_post_descriptor = eet_data_descriptor_file_new(&eddc);

   // Describe the members to be saved:
   // Use a temporary macro so we don't type a lot, also avoid errors:

#define ADD_BASIC(member, eet_type) \
  EET_DATA_DESCRIPTOR_ADD_BASIC     \
    (_my_message_descriptor, My_Message, # member, member, eet_type)
   ADD_BASIC(screen_name, EET_T_STRING);
   ADD_BASIC(name, EET_T_STRING);
   ADD_BASIC(message, EET_T_STRING);
   ADD_BASIC(id, EET_T_UINT);
   ADD_BASIC(status_id, EET_T_UINT);
   ADD_BASIC(date, EET_T_UINT);
   ADD_BASIC(timeline, EET_T_UINT);
#undef ADD_BASIC

#define ADD_BASIC(member, eet_type) \
  EET_DATA_DESCRIPTOR_ADD_BASIC     \
    (_my_post_descriptor, My_Post, # member, member, eet_type)
   ADD_BASIC(dm_to, EET_T_STRING);
   ADD_BASIC(message, EET_T_STRING);
#undef ADD_BASIC

#define ADD_BASIC(member, eet_type) \
  EET_DATA_DESCRIPTOR_ADD_BASIC     \
    (_my_account_descriptor, My_Account, # member, member, eet_type)
   ADD_BASIC(name, EET_T_STRING);
   ADD_BASIC(id, EET_T_UINT);
#undef ADD_BASIC

   EET_DATA_DESCRIPTOR_ADD_LIST
     (_my_account_descriptor, My_Account, "messages", messages,
     _my_message_descriptor);
   EET_DATA_DESCRIPTOR_ADD_VAR_ARRAY
     (_my_account_descriptor, My_Account, "posts", posts,
     _my_post_descriptor);

#define ADD_BASIC(member, eet_type) \
  EET_DATA_DESCRIPTOR_ADD_BASIC     \
    (_my_cache_descriptor, My_Cache, # member, member, eet_type)
   ADD_BASIC(version, EET_T_UINT);
#undef ADD_BASIC

   EET_DATA_DESCRIPTOR_ADD_HASH
     (_my_cache_descriptor, My_Cache, "accounts", accounts,
     _my_account_descriptor);
} /* _my_cache_descriptor_init */

static void
_my_cache_descriptor_shutdown(void)
{
   eet_data_descriptor_free(_my_cache_descriptor);
   eet_data_descriptor_free(_my_account_descriptor);
   eet_data_descriptor_free(_my_message_descriptor);
   eet_data_descriptor_free(_my_post_descriptor);
} /* _my_cache_descriptor_shutdown */

// need to check if the pointer came from mmaped area in eet_dictionary
// or it was allocated with eina_stringshare_add()
static void
_eet_string_free(const char *str)
{
   if (!str)
     return;

   if ((_my_cache_dict) && (eet_dictionary_string_check(_my_cache_dict, str)))
     return;

   eina_stringshare_del(str);
} /* _eet_string_free */

static My_Message *
_my_message_new(const char *message)
{
   My_Message *msg = calloc(1, sizeof(My_Message));
   if (!msg)
     {
        fprintf(stderr, "ERROR: could not calloc My_Message\n");
        return NULL;
     }

   msg->message = eina_stringshare_add(message);
   return msg;
} /* _my_message_new */

static void
_my_message_free(My_Message *msg)
{
   _eet_string_free(msg->screen_name);
   _eet_string_free(msg->name);
   _eet_string_free(msg->message);
   free(msg);
} /* _my_message_free */

static Eina_Bool
_my_post_add(My_Account *acc,
             const char *message)
{
   int new_count = acc->posts_count + 1;
   My_Post *post = realloc(acc->posts, new_count * sizeof(My_Post));
   if (!post)
     {
        fprintf(stderr, "ERROR: could add My_Post\n");
        return EINA_FALSE;
     }

   post[acc->posts_count].message = eina_stringshare_add(message);
   post[acc->posts_count].dm_to = NULL;
   acc->posts_count = new_count;
   acc->posts = post;
   return EINA_TRUE;
} /* _my_post_new */

static void
_my_post_free(My_Post *post)
{
   _eet_string_free(post->dm_to);
   _eet_string_free(post->message);
} /* _my_post_free */

static My_Account *
_my_account_new(const char *name)
{
   My_Account *acc = calloc(1, sizeof(My_Account));
   if (!acc)
     {
        fprintf(stderr, "ERROR: could not calloc My_Account\n");
        return NULL;
     }

   acc->name = eina_stringshare_add(name);
   return acc;
} /* _my_account_new */

static void
_my_account_free(My_Account *acc)
{
   My_Message *m;
   int i;

   _eet_string_free(acc->name);

   EINA_LIST_FREE(acc->messages, m)
     _my_message_free(m);

   for (i = 0; i < acc->posts_count; i++)
     _my_post_free(&acc->posts[i]);
   free(acc->posts);

   free(acc);
} /* _my_account_free */

static My_Cache *
_my_cache_new(void)
{
   My_Cache *my_cache = calloc(1, sizeof(My_Cache));
   if (!my_cache)
     {
        fprintf(stderr, "ERROR: could not calloc My_Cache\n");
        return NULL;
     }

   my_cache->accounts = eina_hash_string_small_new(NULL);

   my_cache->version = 1;
   return my_cache;
} /* _my_cache_new */

static Eina_Bool
_my_cache_account_free_cb(const Eina_Hash *hash,
                          const void      *key,
                          void            *data,
                          void            *fdata)
{
   _my_account_free(data);
   return EINA_TRUE;
}

static void
_my_cache_free(My_Cache *my_cache)
{
   My_Account *acc;
   eina_hash_foreach(my_cache->accounts, _my_cache_account_free_cb, NULL);
   eina_hash_free(my_cache->accounts);
   free(my_cache);
} /* _my_cache_free */

static My_Account *
_my_cache_account_find(My_Cache   *my_cache,
                       const char *name)
{
   return eina_hash_find(my_cache->accounts, name);
} /* _my_cache_account_find */

static My_Cache *
_my_cache_load(const char *filename)
{
   My_Cache *my_cache;
   Eet_File *ef = eet_open(filename, EET_FILE_MODE_READ);
   if (!ef)
     {
        fprintf(stderr, "ERROR: could not open '%s' for read\n", filename);
        return NULL;
     }

   my_cache = eet_data_read(ef, _my_cache_descriptor, MY_CACHE_FILE_ENTRY);
   if (!my_cache)
     {
        eet_close(ef);
        return NULL;
     }

   if (my_cache->version < 1)
     {
        fprintf(stderr,
                "WARNING: version %#x was too old, upgrading it to %#x\n",
                my_cache->version, 1);

        my_cache->version = 1;
     }

   if (_my_cache_file)
     eet_close(_my_cache_file);

   _my_cache_file = ef;
   _my_cache_dict = eet_dictionary_get(ef);

   return my_cache;
} /* _my_cache_load */

static Eina_Bool
_my_cache_save(const My_Cache *my_cache,
               const char     *filename)
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
       (ef, _my_cache_descriptor, MY_CACHE_FILE_ENTRY, my_cache, EINA_TRUE);

   // VERY IMPORTANT NOTE:
   // after eet_close(), all strings mmaped from file will be GONE, invalid!
   // you'll need to free the old cache and open the new one.
   // For cache this is okay, as you should be saving not so often or just
   // at end.
   //
   // This is a trade off, you save memory by using mmap()ed strings, but
   // you have to care about this.
   eet_close(ef);

   if (ret)
     {
        unlink(filename);
        rename(tmp, filename);
     }

   return ret;
} /* _my_cache_save */

int
main(int   argc,
     char *argv[])
{
   My_Cache *my_cache;
   const Eina_List *l_acc;
   Eina_Iterator *it;
   My_Account *acc;
   int ret = 0;

   if (argc < 3)
     {
        fprintf(stderr,
                "Usage:\n\t%s <input> <output> [action] [action-params]\n\n"
                "Where actions and their parameters:\n"
                "\tacc <name>\n"
                "\tpost <account-name> <message>\n"
                "\tmessage <account-name> <message>\n"
                "\n",
                argv[0]);
        return -1;
     }

   eina_init();
   eet_init();
   _my_cache_descriptor_init();

   my_cache = _my_cache_load(argv[1]);
   if (!my_cache)
     {
        printf("creating new cache.\n");
        my_cache = _my_cache_new();
        if (!my_cache)
          {
             ret = -2;
             goto end;
          }
     }

   if (argc > 3)
     {
        if (strcmp(argv[3], "acc") == 0)
          {
             if (argc == 5)
               {
                  My_Account *acc = _my_cache_account_find(my_cache, argv[4]);
                  if (!acc)
                    {
                       acc = _my_account_new(argv[4]);
                       eina_hash_direct_add(my_cache->accounts, acc->name, acc);
                    }
                  else
                    fprintf(stderr, "ERROR: account '%s' already exists.\n",
                            argv[4]);
               }
             else
               fprintf(stderr,
                       "ERROR: wrong number of parameters (%d).\n",
                       argc);
          }
        else if (strcmp(argv[3], "post") == 0)
          {
             if (argc == 6)
               {
                  My_Account *acc = _my_cache_account_find(my_cache, argv[4]);
                  if (acc)
                    {
                       _my_post_add(acc, argv[5]);
                    }
                  else
                    fprintf(stderr, "ERROR: unknown account: '%s'\n", argv[4]);
               }
             else
               fprintf(stderr,
                       "ERROR: wrong number of parameters (%d).\n",
                       argc);
          }
        else if (strcmp(argv[3], "message") == 0)
          {
             if (argc == 6)
               {
                  My_Account *acc = _my_cache_account_find(my_cache, argv[4]);
                  if (acc)
                    {
                       My_Message *msg = _my_message_new(argv[5]);
                       acc->messages = eina_list_append(acc->messages, msg);
                    }
                  else
                    fprintf(stderr, "ERROR: unknown account: '%s'\n", argv[4]);
               }
             else
               fprintf(stderr,
                       "ERROR: wrong number of parameters (%d).\n",
                       argc);
          }
        else
          fprintf(stderr, "ERROR: unknown action '%s'\n", argv[2]);
     }

   printf("My_Cache:\n"
          "\tversion.: %#x\n"
          "\taccounts: %u\n",
          my_cache->version,
          eina_hash_population(my_cache->accounts));
   it = eina_hash_iterator_data_new(my_cache->accounts);
   EINA_ITERATOR_FOREACH(it, acc)
     {
        const My_Post *post;

        printf("\t  > %-#8x '%.20s' stats: m=%u, p=%u\n",
               acc->id, acc->name ? acc->name : "",
               eina_list_count(acc->messages),
               acc->posts_count);

        if (eina_list_count(acc->messages))
          {
             const Eina_List *l;
             const My_Message *msg;
             printf("\t  |messages:\n");

             EINA_LIST_FOREACH(acc->messages, l, msg)
               {
                  printf("\t  |   %-8x '%s' [%s]: '%.20s'\n",
                         msg->id,
                         msg->name ? msg->name : "",
                         msg->screen_name ? msg->screen_name : "",
                         msg->message ? msg->message : "");
               }
          }

        if (acc->posts_count)
          {
             const My_Post *post;
             int i;
             printf("\t  |posts:\n");

             for (i = 0; i < acc->posts_count; i++)
               {
                  post = &acc->posts[i];
                  if (post->dm_to)
                    printf("\t  |  @%s: '%.20s'\n", post->dm_to, post->message);
                  else
                    printf("\t  |  '%.20s'\n", post->message);
               }
          }

        printf("\n");
     }
   eina_iterator_free(it);

   if (!_my_cache_save(my_cache, argv[2]))
     ret = -3;

   _my_cache_free(my_cache);

end:
   if (_my_cache_file)
     eet_close(_my_cache_file);
   _my_cache_descriptor_shutdown();
   eet_shutdown();
   eina_shutdown();

   return ret;
} /* main */

