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

static const char *_nicknames[] = {
       "mozzart",
       "betho",
       "george",
       "hector",
       NULL
};

static void
_phone_entry_free_cb(void *data)
{
   free(data);
}

static Eina_Bool
_phone_book_foreach_cb(const Eina_Hash *phone_book, const void *key,
		       void *data, void *fdata)
{
   Phone_Entry **pe = (Phone_Entry **)key;
   const char *nick = data;
   printf("%s: %s, nick=%s\n", (*pe)->name, (*pe)->number, nick);

   // Return EINA_FALSE to stop this callback from being called
   return EINA_TRUE;
}

int
main(int argc, const char *argv[])
{
   Eina_Hash *phone_book = NULL;
   int i;
   Phone_Entry *entry_vl = &_start_entries[3];
   Phone_Entry *p = NULL;
   char *nick = NULL;
   Eina_Bool r;
   Eina_Iterator *it;
   void *data;

   eina_init();

   phone_book = eina_hash_pointer_new(_phone_entry_free_cb);

   // Add initial entries to our hash
   for (i = 0; _start_entries[i].name != NULL; i++)
     {
	p = &_start_entries[i];
	eina_hash_add(phone_book, &p,
		      strdup(_nicknames[i]));
     }
   printf("Phonebook:\n");
   eina_hash_foreach(phone_book, _phone_book_foreach_cb, NULL);
   printf("\n");

   // Look for a specific entry and get its nickname
   nick = eina_hash_find(phone_book, &entry_vl);
   if (nick)
     {
	printf("Printing entry.\n");
	printf("Name: %s\n", entry_vl->name);
	printf("Number: %s\n", entry_vl->number);
	printf("Nick: %s\n\n", nick);
     }

   // Delete this entry
   r = eina_hash_del(phone_book, &entry_vl, NULL);
   printf("Hash entry successfully deleted? %d\n\n", r);

   // Modify the pointer data of an entry and free the old one
   p = &_start_entries[2];
   nick = eina_hash_modify(phone_book, &p,
			   strdup("el jorge"));
   free(nick);

   // Modify or add an entry to the hash with eina_hash_set
   // Let's first add a new entry
   eina_error_set(0);
   Phone_Entry *p1 = malloc(sizeof(*p1));
   p1->name = "Raul Seixas";
   p1->number = "+55 01 234-56789";
   nick = eina_hash_set(phone_book, &p1,
			strdup("raulzito"));
   if (!nick)
     {
	Eina_Error err = eina_error_get();
	if (!err)
	  {
	     printf("No previous nick found for Raul Seixas. ");
	     printf("Creating new entry.\n");
	  }
	else
	  printf("Error when setting nick for Raul Seixas\n");
     }
   else
     {
	printf("Old nick for Raul Seixas was %s\n", nick);
	free(nick);
     }

   printf("\n");

   // Now change the nick
   eina_error_set(0);
   nick = eina_hash_set(phone_book, &p1,
			strdup("raulzao"));
   if (nick)
     {
	printf("Changing nick for Raul Seixas to raulzao. ");
	printf("Old nick was %s\n", nick);
	free(nick);
     }
   else
     {
	Eina_Error err = eina_error_get();
	if (err)
	  printf("Error when changing nick for Raul Seixas\n");
	else
	  {
	     printf("No previous nick found for Raul Seixas. ");
	     printf("Creating new entry.\n");
	  }
     }

   // There are many ways to iterate over our Phone book.
   // First, iterate showing the names, phones and associated nicks.
   printf("Phonebook:\n");
   eina_hash_foreach(phone_book, _phone_book_foreach_cb, NULL);
   printf("\n");

   // Now iterate using an iterator
   printf("Phonebook:\n");
   it = eina_hash_iterator_tuple_new(phone_book);
   while (eina_iterator_next(it, &data))
     {
	Eina_Hash_Tuple *t = data;
	Phone_Entry **pe = (Phone_Entry **)t->key;
	nick = t->data;
	printf("%s: %s, nick=%s\n", (*pe)->name, (*pe)->number, nick);
     }
   eina_iterator_free(it); // Always free the iterator after its use
   printf("\n");

   // Just iterate over the keys (names)
   printf("List of names/numbers in the phone book:\n");
   it = eina_hash_iterator_key_new(phone_book);
   while (eina_iterator_next(it, &data))
     {
	Phone_Entry **pe = (Phone_Entry **)data;
	printf("%s: %s\n", (*pe)->name, (*pe)->number);
     }
   eina_iterator_free(it);
   printf("\n");

   // Just iterate over the data (nicks)
   printf("List of nicks in the phone book:\n");
   it = eina_hash_iterator_data_new(phone_book);
   while (eina_iterator_next(it, &data))
     {
	nick = data;
	printf("%s\n", nick);
     }
   eina_iterator_free(it);
   printf("\n");

   // Check how many items are in the phone book
   printf("There are %d items in the hash.\n\n",
	  eina_hash_population(phone_book));

   // Change the name (key) on an entry
   Phone_Entry *p2 = malloc(sizeof(*p2));
   p2->name = "Alceu Valenca";
   p2->number = "000000000000";
   eina_hash_move(phone_book, p1, p2);
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

   free(p1);
   free(p2);

   eina_shutdown();
}
