//Compile with:
//gcc -g eina_hash_02.c -o eina_hash_02 `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <string.h>
#include <Eina.h>

/*
 * Eina Hash - Two more types of hash
 *
 * This example demonstrate two other types of hash in action - using
 * eina_hash_stringshared_new and eina_hash_new.
 *
 * It indexes the phone numbers by Contact Full Name, so it's a hash with string
 * keys, exactly the same as the other example.
 */

struct _Phone_Entry {
     const char *name; // Full name.
     const char *number; // Phone number.
};

typedef struct _Phone_Entry Phone_Entry;

static Phone_Entry _start_entries[] = {
       { "Wolfgang Amadeus Mozart", "+01 23 456-78910" },
       { "Ludwig van Beethoven", "+12 34 567-89101" },
       { "Richard Georg Strauss", "+23 45 678-91012" },
       { "Heitor Villa-Lobos", "+34 56 789-10123" },
       { NULL, NULL }
};

static void
_phone_entry_free_cb(void *data)
{
   free(data);
}

static void
_phone_book_stringshared_free_cb(void *data)
{
   Phone_Entry *e = data;
   eina_stringshare_del(e->name);
   eina_stringshare_del(e->number);
   free(e);
}

static Eina_Bool
_phone_book_stringshared_foreach_cb(const Eina_Hash *phone_book,
                                    const void *key EINA_UNUSED,
                                    void *data,
                                    void *fdata EINA_UNUSED)
{
   Phone_Entry *e = data;
   const char *name = e->name; // e->name == key
   const char *number = e->number;
   printf("%s: %s\n", name, number);

   return EINA_TRUE;
}

static void
example_hash_stringshared(void)
{
   Eina_Hash *phone_book = NULL;
   int i;

   // Create the hash as before
   phone_book = eina_hash_stringshared_new(_phone_book_stringshared_free_cb);

   // Add initial entries to our hash, using direct_add
   for (i = 0; _start_entries[i].name != NULL; i++)
     {
	Phone_Entry *e = malloc(sizeof(Phone_Entry));
	e->name = eina_stringshare_add(_start_entries[i].name);
	e->number = eina_stringshare_add(_start_entries[i].number);
	// Since we are storing the key (name) in our struct, we can use
	// eina_hash_direct_add. It could be used in the previous example
	// too, since each key is already stored in the _start_entries
	// static array, but we started it with the default add function.
	eina_hash_direct_add(phone_book, e->name, e);
     }

   // Iterate over the elements
   printf("List of phones:\n");
   eina_hash_foreach(phone_book, _phone_book_stringshared_foreach_cb, NULL);
   printf("\n");

   eina_hash_free(phone_book);
}

static unsigned int
_phone_book_string_key_length(const char *key)
{
   if (!key)
      return 0;

   return (int)strlen(key) + 1;
}

static int
_phone_book_string_key_cmp(const char *key1, int key1_length EINA_UNUSED,
                     const char *key2, int key2_length EINA_UNUSED)
{
   return strcmp(key1, key2);
}

static void
example_hash_big(void)
{
   Eina_Hash *phone_book = NULL;
   int i;
   const char *phone;

   // Create the same hash as used in eina_hash_01.c, but
   // use 1024 (2 ^ 10) buckets.
   phone_book = eina_hash_new(EINA_KEY_LENGTH(_phone_book_string_key_length),
			      EINA_KEY_CMP(_phone_book_string_key_cmp),
			      EINA_KEY_HASH(eina_hash_superfast),
			      _phone_entry_free_cb,
			      10);
   for (i = 0; _start_entries[i].name != NULL; i++)
     {
        eina_hash_add(phone_book, _start_entries[i].name,
                      strdup(_start_entries[i].number));
     }

   // Look for a specific entry and get its phone number
   phone = eina_hash_find(phone_book, "Heitor Villa-Lobos");
   if (phone)
     {
        printf("Printing entry.\n");
        printf("Name: Heitor Villa-Lobos\n");
        printf("Number: %s\n\n", phone);
     }

   eina_hash_free(phone_book);
}

int
main(int argc, const char *argv[])
{
   (void)argc;
   (void)argv;
   eina_init();

   example_hash_stringshared();
   example_hash_big();

   eina_shutdown();
}
