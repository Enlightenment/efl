//Compile with:
//gcc -g eina_model_03.c -o eina_model_03 `pkg-config --cflags --libs eina`

#include <Eina.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static Eina_Model_Type *ADDRESS_BOOK_TYPE;
static Eina_Model_Type *PERSON_TYPE;

static void address_book_init(void);

int main(void)
{
   Eina_Model *address_book;
   Eina_Value val;
   int i, count;
   char *s;

   eina_init();

   address_book_init();

   address_book = eina_model_new(ADDRESS_BOOK_TYPE);

   eina_value_setup(&val, EINA_VALUE_TYPE_STRING);
   eina_value_set(&val, "addr_book.txt");
   eina_model_property_set(address_book, "filename", &val);
   eina_value_flush(&val);

   eina_model_load(address_book);
   s = eina_model_to_string(address_book);
   printf("model as string:\n%s\n\n", s);
   free(s);

   count = eina_model_child_count(address_book);
   printf("Address Book with %d entries:\n", count);
   for (i = 0; i < count; i++)
     {
        Eina_Model *person = eina_model_child_get(address_book, i);
        Eina_Value nameval, emailval;
        const char *name, *email;

        eina_model_property_get(person, "name", &nameval);
        eina_model_property_get(person, "email", &emailval);

        eina_value_get(&nameval, &name);
        eina_value_get(&emailval, &email);

        printf("%02d \"%s\" <%s>\n", i, name, email);

        // We don't need property values anymore
        eina_value_flush(&nameval);
        eina_value_flush(&emailval);

        // We don't need our reference to person anymore
        eina_model_unref(person);
     }

   eina_model_unref(address_book);
   eina_shutdown();

   return 0;
}

// Structure Descriptions are just used internally in the type constructors:
static Eina_Value_Struct_Desc *ADDRESS_BOOK_DESC;
static Eina_Value_Struct_Desc *PERSON_DESC;

static Eina_Bool
_person_constructor(Eina_Model *model)
{
   // call parent type constructor, like "super" in other languages:
   if (!eina_model_type_constructor(EINA_MODEL_TYPE_STRUCT, model))
     return EINA_FALSE;

   // Do specific setup of our internal structure, letting it know about
   // our description
   return eina_model_struct_set(model, PERSON_DESC, NULL);
}

static Eina_Bool
_address_book_constructor(Eina_Model *model)
{
   // call parent type constructor, like "super" in other languages:
   if (!eina_model_type_constructor(EINA_MODEL_TYPE_STRUCT, model))
     return EINA_FALSE;

   // Do specific setup of our internal structure, letting it know about
   // our description
   return eina_model_struct_set(model, ADDRESS_BOOK_DESC, NULL);
}

static Eina_Bool
_address_book_load(Eina_Model *model)
{
   const char *filename;
   Eina_Value val;
   char buf[256];
   FILE *f;

   // We retrieve filename from property of same name:
   eina_model_property_get(model, "filename", &val);
   eina_value_get(&val, &filename);

   EINA_SAFETY_ON_NULL_RETURN_VAL(filename, EINA_FALSE);

   f = fopen(filename, "r");

   // Now that we have used filename, we must free its memory holder:
   eina_value_flush(&val);

   EINA_SAFETY_ON_NULL_RETURN_VAL(f, EINA_FALSE);

   while (fgets(buf, sizeof(buf), f))
     {
        Eina_Model *person;
        char *name, *email;

        if (strlen(buf) <= 1)
          continue;

        name = strtok(buf, "\t");
        email = strtok(NULL, "\n");

        if ((!name) || (!email)) continue;

        // Create person
        person = eina_model_new(PERSON_TYPE);

        // Setup value type as string, as our properties are strings:
        eina_value_setup(&val, EINA_VALUE_TYPE_STRING);

        // Set string properties:
        eina_value_set(&val, name);
        eina_model_property_set(person, "name", &val);

        eina_value_set(&val, email);
        eina_model_property_set(person, "email", &val);

        // Flush value, free string
        eina_value_flush(&val);

        // Add person to the end of model children
        eina_model_child_append(model, person);

        // Model already holds its reference to person, we release ours
        eina_model_unref(person);
     }

   fclose(f);
   return EINA_TRUE;
}

static void
address_book_init(void)
{
   // Declare type for internal struct, this is just used to easily
   // create Eina_Value_Struct_Member array for Eina_Value_Struct_Desc.
   //
   // We don't need this structure outside address_book_init()
   // as it is managed automatically by Eina_Value_Struct, used by
   // Eina_Model_Struct! Handy! :-)
   typedef struct _Person Person;
   struct _Person
   {
      const char *name;
      const char *email;
   };
   static Eina_Value_Struct_Member person_members[] = {
     // no eina_value_type as they are not constant initializers, see below.
     EINA_VALUE_STRUCT_MEMBER(NULL, Person, name),
     EINA_VALUE_STRUCT_MEMBER(NULL, Person, email)
   };
   // Values that cannot be set on static declarations since they are not
   // constant initializers. It is a nitpick from C that we need to deal with
   // here and on all our other declarations.
   person_members[0].type = EINA_VALUE_TYPE_STRING;
   person_members[1].type = EINA_VALUE_TYPE_STRING;

   static Eina_Value_Struct_Desc person_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, // no special operations
     person_members,
     EINA_C_ARRAY_LENGTH(person_members),
     sizeof(Person)
   };
   static Eina_Model_Type person_type = EINA_MODEL_TYPE_INIT_NOPRIVATE
     ("Person_Type",
      Eina_Model_Type,
      NULL, // no type as EINA_MODEL_TYPE_STRUCT is not constant initializer!
      NULL, // no extra interfaces
      NULL  // no extra events);
      );
   person_type.parent = EINA_MODEL_TYPE_STRUCT;
   // Set our overloaded methods:
   person_type.constructor = _person_constructor;

   typedef struct _Address_Book Address_Book;
   struct _Address_Book
   {
      const char *filename;
   };
   static Eina_Value_Struct_Member address_book_members[] = {
     // no eina_value_type as they are not constant initializers, see below.
     EINA_VALUE_STRUCT_MEMBER(NULL, Address_Book, filename)
   };
   address_book_members[0].type = EINA_VALUE_TYPE_STRING;
   static Eina_Value_Struct_Desc address_book_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, // no special operations
     address_book_members,
     EINA_C_ARRAY_LENGTH(address_book_members),
     sizeof(Address_Book)
   };
   static Eina_Model_Type address_book_type = EINA_MODEL_TYPE_INIT_NOPRIVATE
     ("Address_Book_Type",
      Eina_Model_Type,
      NULL, // no type as EINA_MODEL_TYPE_STRUCT is not constant initializer!
      NULL, // no extra interfaces
      NULL  // no extra events);
      );
   address_book_type.parent = EINA_MODEL_TYPE_STRUCT;
   // Set our overloaded methods:
   address_book_type.constructor = _address_book_constructor;
   address_book_type.load = _address_book_load;

   // Expose the configured pointers to public usage:
   // NOTE: they are static, so they live after this function returns!
   PERSON_TYPE = &person_type;
   PERSON_DESC = &person_desc;

   ADDRESS_BOOK_TYPE = &address_book_type;
   ADDRESS_BOOK_DESC = &address_book_desc;
}
