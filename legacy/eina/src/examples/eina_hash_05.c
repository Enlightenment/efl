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
     int32_t id; // Full name.
     const char *number; // Phone number.
};

typedef struct _Phone_Entry Phone_Entry;

static Phone_Entry _start_entries[] = {
       { 1, "+01 23 456-78910" },
       { 2, "+12 34 567-89101" },
       { 3, "+23 45 678-91012" },
       { 4, "+34 56 789-10123" },
       { -1, NULL }
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
   const int32_t *id = key;
   const char *number = data;
   printf("%d: %s\n", *id, number);

   // Return EINA_FALSE to stop this callback from being called
   return EINA_TRUE;
}

int
main(int argc, const char *argv[])
{
   Eina_Hash *phone_book = NULL;
   int i;
   int32_t entry_id = 4;
   char *phone = NULL;
   Eina_Bool r;
   Eina_Iterator *it;
   void *data;

   eina_init();

   phone_book = eina_hash_int32_new(_phone_entry_free_cb);

   // Add initial entries to our hash
   for (i = 0; _start_entries[i].id != -1; i++)
     {
	eina_hash_add(phone_book, &_start_entries[i].id,
		      strdup(_start_entries[i].number));
     }

   // Look for a specific entry and get its phone number
   phone = eina_hash_find(phone_book, &entry_id);
   if (phone)
     {
	printf("Printing entry.\n");
	printf("Id: %d\n", entry_id);
	printf("Number: %s\n\n", phone);
     }

   // Delete this entry
   r = eina_hash_del(phone_book, &entry_id, NULL);
   printf("Hash entry successfully deleted? %d\n\n", r);

   // Modify the pointer data of an entry and free the old one
   int32_t id3 = 3;
   phone = eina_hash_modify(phone_book, &id3,
			    strdup("+23 45 111-11111"));
   free(phone);

   // Modify or add an entry to the hash with eina_hash_set
   // Let's first add a new entry
   int32_t id5 = 5;
   eina_error_set(0);
   phone = eina_hash_set(phone_book, &id5,
			 strdup("+55 01 234-56789"));
   if (!phone)
     {
	Eina_Error err = eina_error_get();
	if (!err)
	  {
	     printf("No previous phone found for id5. ");
	     printf("Creating new entry.\n");
	  }
	else
	  printf("Error when setting phone for Raul Seixas\n");
     }
   else
     {
	printf("Old phone for id5 was %s\n", phone);
	free(phone);
     }

   printf("\n");

   // Now change the phone number
   eina_error_set(0);
   phone = eina_hash_set(phone_book, &id5,
			 strdup("+55 02 222-22222"));
   if (phone)
     {
	printf("Changing phone for id5 to +55 02 222-22222. ");
	printf("Old phone was %s\n", phone);
	free(phone);
     }
   else
     {
	Eina_Error err = eina_error_get();
	if (err)
	  printf("Error when changing phone for id5\n");
	else
	  {
	     printf("No previous phone found for id5. ");
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
	const int32_t *id = t->key;
	const char *number = t->data;
	printf("%d: %s\n", *id, number);
     }
   eina_iterator_free(it); // Always free the iterator after its use
   printf("\n");

   // Just iterate over the keys (names)
   printf("List of ids in the phone book:\n");
   it = eina_hash_iterator_key_new(phone_book);
   while (eina_iterator_next(it, &data))
     {
	const int32_t *id = data;
	printf("%d\n", *id);
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
   int32_t id6 = 6;
   eina_hash_move(phone_book, &id5, &id6);
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
