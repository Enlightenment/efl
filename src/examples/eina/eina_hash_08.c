//Compile with:
//gcc -g eina_hash_08.c -o eina_hash_08 `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <string.h>
#include <Eina.h>

/*
 * Eina Hash - phonebook
 *
 * This example demonstrate the use of Eina Hash by implementing a phonebook
 * that stores its contact data into the hash.
 *
 * It indexes the phone numbers by Contact Full Name, so it's a hash with
 * string keys.
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
}; // _start_entries

static void
_phone_entry_free_cb(void *data)
{
   free(data);
}

static Eina_Bool
_phone_book_foreach_cb(const Eina_Hash *phone_book EINA_UNUSED, const void *key,
		       void *data, void *fdata EINA_UNUSED)
{
   const char *name = key;
   const char *number = data;
   printf("%s: %s\n", name, number);

   // Return EINA_FALSE to stop this callback from being called
   return EINA_TRUE;
}

int
main(int argc EINA_UNUSED, const char *argv[] EINA_UNUSED)
{
   Eina_Hash *phone_book = NULL;
   int i;
   const char *entry_name = "Heitor Villa-Lobos";
   int entry_size;
   const char *saved_entry_name = "Alceu Valenca";
   int saved_entry_size = sizeof("Alceu Valenca");
   const char *phone = NULL;
   Eina_Bool r;

   eina_init();

   phone_book = eina_hash_string_superfast_new(_phone_entry_free_cb);

   // Add initial entries to our hash
   for (i = 0; _start_entries[i].name != NULL; i++)
     {
	eina_hash_add(phone_book, _start_entries[i].name,
		      strdup(_start_entries[i].number));
     }

   // Delete entries
   r = eina_hash_del(phone_book, entry_name, NULL);
   printf("Hash entry successfully deleted? %d\n\n", r);

   int hash = eina_hash_superfast("Ludwig van Beethoven",
				  sizeof("Ludwig van Beethoven"));

   r = eina_hash_del_by_key_hash(phone_book, "Ludwig van Beethoven",
				 sizeof("Ludwig van Beethoven"), hash);
   printf("Hash entry successfully deleted? %d\n\n", r);

   r = eina_hash_del_by_key(phone_book, "Richard Georg Strauss");
   printf("Hash entry successfully deleted? %d\n\n", r);

   // add entry by hash
   entry_name = "Raul_Seixas";
   entry_size = sizeof("Raul Seixas");
   phone = strdup("+33 33 333-33333");
   hash = eina_hash_superfast(entry_name, entry_size);
   eina_hash_add_by_hash(phone_book, entry_name, entry_size, hash, phone);

   // don't need to free 'phone' after the next del:
   r = eina_hash_del_by_data(phone_book, phone);
   printf("Hash entry successfully deleted? %d\n\n", r);

   // add entry by hash directly - no copy of the key will be done
   hash = eina_hash_superfast(saved_entry_name, saved_entry_size);
   phone = strdup("+44 44 444-44444");
   eina_hash_direct_add_by_hash(phone_book, saved_entry_name,
				saved_entry_size, hash, phone);

   // find the added entry by its hash:
   phone = eina_hash_find_by_hash(phone_book, saved_entry_name,
				  saved_entry_size, hash);
   if (phone)
     {
	char *newphone = strdup("+55 55 555-55555");
	phone = eina_hash_modify_by_hash(phone_book, saved_entry_name,
					 saved_entry_size, hash, newphone);
	if (phone)
	  printf("changing phone to %s, old one was %s\n", newphone, phone);
	else
	  printf("couldn't modify entry identified by %d\n", hash);
     }
   else
     {
	printf("couldn't find entry identified by %d\n", hash);
     }

   eina_hash_free(phone_book);

   eina_shutdown();
}
