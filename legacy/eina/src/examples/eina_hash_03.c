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
_phone_book_foreach_cb(const Eina_Hash *phone_book, const void *key,
		       void *data, void *fdata)
{
   const char *name = key;
   const char *number = data;
   printf("%s: %s\n", name, number);

   // Return EINA_FALSE to stop this callback from being called
   return EINA_TRUE;
}

int
main(int argc, const char *argv[])
{
   Eina_Hash *phone_book = NULL;
   int i;
   const char *entry_name = "Heitor Villa-Lobos";
   char *phone = NULL;
   Eina_Bool r;
   Eina_Iterator *it;
   void *data;

   eina_init();

   phone_book = eina_hash_string_small_new(_phone_entry_free_cb);

   // Add initial entries to our hash
   for (i = 0; _start_entries[i].name != NULL; i++)
     {
	eina_hash_add(phone_book, _start_entries[i].name,
		      strdup(_start_entries[i].number));
     }

   // Look for a specific entry and get its phone number
   phone = eina_hash_find(phone_book, entry_name);
   if (phone)
     {
	printf("Printing entry.\n");
	printf("Name: %s\n", entry_name);
	printf("Number: %s\n\n", phone);
     }

   // Delete this entry
   r = eina_hash_del(phone_book, entry_name, NULL);
   printf("Hash entry successfully deleted? %d\n\n", r);

   // Modify the pointer data of an entry and free the old one
   phone = eina_hash_modify(phone_book, "Richard Georg Strauss",
			    strdup("+23 45 111-11111"));
   free(phone);

   // Modify or add an entry to the hash with eina_hash_set
   // Let's first add a new entry
   eina_error_set(0);
   phone = eina_hash_set(phone_book, "Raul Seixas",
			 strdup("+55 01 234-56789"));
   if (!phone)
     {
	Eina_Error err = eina_error_get();
	if (!err)
	  {
	     printf("No previous phone found for Raul Seixas. ");
	     printf("Creating new entry.\n");
	  }
	else
	  printf("Error when setting phone for Raul Seixas\n");
     }
   else
     {
	printf("Old phone for Raul Seixas was %s\n", phone);
	free(phone);
     }

   printf("\n");

   // Now change the phone number
   eina_error_set(0);
   phone = eina_hash_set(phone_book, "Raul Seixas",
			 strdup("+55 02 222-22222"));
   if (phone)
     {
	printf("Changing phone for Raul Seixas to +55 02 222-22222. ");
	printf("Old phone was %s\n", phone);
	free(phone);
     }
   else
     {
	Eina_Error err = eina_error_get();
	if (err)
	  printf("Error when changing phone for Raul Seixas\n");
	else
	  {
	     printf("No previous phone found for Raul Seixas. ");
	     printf("Creating new entry.\n");
	  }
     }

   // There are many ways to iterate over our Phone book.
   // First, iterate showing the names and associated numbers.
   printf("List of phones:\n");
   eina_hash_foreach(phone_book, _phone_book_foreach_cb, NULL);
   printf("\n");

   // Now iterate using an iterator
   printf("List of phones:\n");
   it = eina_hash_iterator_tuple_new(phone_book);
   while (eina_iterator_next(it, &data))
     {
	Eina_Hash_Tuple *t = data;
	const char *name = t->key;
	const char *number = t->data;
	printf("%s: %s\n", name, number);
     }
   eina_iterator_free(it); // Always free the iterator after its use
   printf("\n");

   // Just iterate over the keys (names)
   printf("List of names in the phone book:\n");
   it = eina_hash_iterator_key_new(phone_book);
   while (eina_iterator_next(it, &data))
     {
	const char *name = data;
	printf("%s\n", name);
     }
   eina_iterator_free(it);
   printf("\n");

   // Just iterate over the data (numbers)
   printf("List of numbers in the phone book:\n");
   it = eina_hash_iterator_data_new(phone_book);
   while (eina_iterator_next(it, &data))
     {
	const char *number = data;
	printf("%s\n", number);
     }
   eina_iterator_free(it);
   printf("\n");

   // Check how many items are in the phone book
   printf("There are %d items in the hash.\n\n",
	  eina_hash_population(phone_book));

   // Change the name (key) on an entry
   eina_hash_move(phone_book, "Raul Seixas", "Alceu Valenca");
   printf("List of phones after change:\n");
   eina_hash_foreach(phone_book, _phone_book_foreach_cb, NULL);
   printf("\n");

   // Empty the phone book, but don't destroy it
   eina_hash_free_buckets(phone_book);
   printf("There are %d items in the hash.\n\n",
	  eina_hash_population(phone_book));

   // Phone book could still be used, but we are freeing it since we are
   // done for now
   eina_hash_free(phone_book);

   eina_shutdown();
}
