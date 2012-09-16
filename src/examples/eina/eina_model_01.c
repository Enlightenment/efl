/*
 * Compile with:
 * gcc -o eina_model_01 eina_model_01.c `pkg-config --cflags --libs eina`
 */

/*
 * This example demonstrates the usage of Eina Model by implementing
 * Bank Account Class, which is inherited from Base Class;
 * and Credit Card Class, which is inherited from Bank Account Class.
 *
 * Base Class(Eina_Model_Type) --> Bank Account Class --> Credit Card Class
 *
 * Bank Account Class implements "bank_account_data_set()" and "print()" methods;
 * Credit Card Class inherits these two and implements "credit_card_data_set()"
 *
 *
 *  Bank Account Class::print() calls for "_bank_account_data_print"
 *  Credit Card Class ::print() is reloaded with "_credit_card_data_print()"
 *  which calls for parent function "_bank_account_data_print"
 *
 */


#include <Eina.h>
#include <eina_safety_checks.h>

/*
 * Defining type for new model type
 * Model will have two methods
 */
typedef struct _Bank_Account_Type
{
   Eina_Model_Type parent_class;
   void (*bank_account_data_set)(Eina_Model *, const char *name, const char *number);
   void (*print)(Eina_Model *);
} Bank_Account_Type;

/*
 * Defining type for Bank Account private data
 */
typedef struct _Bank_Account_Data
{
   char name[30];
   char number[30];
} Bank_Account_Data;

/*
 * Defining type for Credit Card model type, which will be inherited from Bank Account model type
 * Model will have two parent's methods and additional one
 */
typedef struct _Credit_Card_Type
{
   Bank_Account_Type parent_class;
   void (*credit_card_data_set)(Eina_Model *, const char *, const char *, int) ;
} Credit_Card_Type;

/*
 * Defining type for Credit Card private data
 */
typedef struct _Credit_Card_Data
{
   char number[30];
   char expiry_date[30];
   int pin;
} Credit_Card_Data;

static Bank_Account_Type _BANK_ACCOUNT_TYPE;
static Credit_Card_Type _CREDIT_CARD_TYPE;
static Eina_Model_Type *BANK_ACCOUNT_TYPE = (Eina_Model_Type *) &_BANK_ACCOUNT_TYPE;
static Eina_Model_Type *CREDIT_CARD_TYPE = (Eina_Model_Type *) &_CREDIT_CARD_TYPE;


/*
 * Defining method for for Bank Account data
 */
static void
_bank_account_data_set(Eina_Model *mdl, const char *name, const char *number)
{
   Bank_Account_Data *bdata = eina_model_type_private_data_get(mdl, BANK_ACCOUNT_TYPE);

   if (!bdata)
      printf("ERROR\n");

   if (name != NULL)
     {
        strncpy(bdata->name, name, sizeof(bdata->name));
        bdata->name[sizeof(bdata->number) - 1] = '\0';
     }

   if (number != NULL)
     {
        strncpy(bdata->number, number, sizeof(bdata->number));
        bdata->number[sizeof(bdata->number) - 1] = '\0';
     }

   printf("%s :: %s %p\n", eina_model_type_name_get(eina_model_type_get(mdl)) ,__func__, mdl);
}


static void
_credit_card_data_set(Eina_Model *mdl, const char *number, const char *expiry_date, int pin)
{
   Credit_Card_Data *cdata = eina_model_type_private_data_get(mdl, CREDIT_CARD_TYPE);

   if (!cdata)
      printf("ERROR\n");

   if (number != NULL)
     {
        strncpy(cdata->number, number, sizeof(cdata->number));
        cdata->number[sizeof(cdata->number) - 1] = '\0';
     }

   if (expiry_date != NULL)
     {
        strncpy(cdata->expiry_date, expiry_date, sizeof(cdata->expiry_date));
        cdata->expiry_date[sizeof(cdata->expiry_date) - 1] = '\0';
     }

   cdata->pin = pin;
   printf("%s :: %s %p\n", eina_model_type_name_get(eina_model_type_get(mdl)) ,__func__, mdl);
}

static void
_bank_account_data_print(Eina_Model *mdl)
{
   const Bank_Account_Data *bdata = eina_model_type_private_data_get(mdl, BANK_ACCOUNT_TYPE);

   printf("\n%s :: %s %p \n\tName: %s(%p)\n\tAccount: %s(%p)\n", eina_model_type_name_get(eina_model_type_get(mdl)) ,__func__, mdl
         , bdata->name, bdata->name, bdata->number, bdata->number);
}

static void
_credit_card_data_print(Eina_Model *mdl)
{
   void (*pf)(Eina_Model *);
   const Eina_Model_Type *ptype = eina_model_type_parent_get(eina_model_type_get(mdl));
   //const Eina_Model_Type *ptype = eina_model_type_get(mdl);

   pf = eina_model_type_method_resolve(ptype, mdl, Bank_Account_Type, print);
   if (pf)
      pf(mdl);
   else
      printf("ERROR: %d", __LINE__);
   const Credit_Card_Data *cdata = eina_model_type_private_data_get(mdl, CREDIT_CARD_TYPE);
   printf("%s :: %s %p \n\tNumber: %s(%p)\n\tCC Expiry Date: %s(%p)\n\tCC PIN: %d(%p)\n", eina_model_type_name_get(eina_model_type_get(mdl)) ,__func__, mdl
         , cdata->number, cdata->number, cdata->expiry_date, cdata->expiry_date, cdata->pin, &cdata->pin);
}

#define BANK_ACCOUNT(x) ((Bank_Account_Type *) x)
#define CREDIT_CARD(x) ((Credit_Card_Type *) x)

void
bank_account_data_set(Eina_Model *mdl, const char *name, char *number)
{

   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(mdl, BANK_ACCOUNT_TYPE));

   void (*pf)(Eina_Model *, const char *, const char *);
   pf = eina_model_method_resolve(mdl, Bank_Account_Type, bank_account_data_set);
   if (pf)
      pf(mdl, name, number);
   else
      printf("ERROR %d\n", __LINE__);
}

void
data_print(Eina_Model *mdl)
{
   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(mdl, BANK_ACCOUNT_TYPE));

   void (*pf)(Eina_Model *);
   pf = eina_model_method_resolve(mdl, Bank_Account_Type, print);
   if (pf)
      pf(mdl);
   else
      printf("ERROR %d\n", __LINE__);
}

void
credit_card_data_set(Eina_Model *mdl, const char *number, const char *expiry_date, int pin)
{
   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(mdl, CREDIT_CARD_TYPE));

   void (*pf)(Eina_Model *, const char *, const char *, int);
   pf = eina_model_method_resolve(mdl, Credit_Card_Type, credit_card_data_set);
   if (pf)
      pf(mdl, number, expiry_date, pin);
   else
      printf("ERROR %d\n", __LINE__);
}

int main(void)
{
   Eina_Model *b, *cc;

   eina_init();

   memset(&_BANK_ACCOUNT_TYPE, 0, sizeof(_BANK_ACCOUNT_TYPE));
   memset(&_CREDIT_CARD_TYPE, 0, sizeof(_CREDIT_CARD_TYPE));

   BANK_ACCOUNT_TYPE->version = EINA_MODEL_TYPE_VERSION;
   BANK_ACCOUNT_TYPE->type_size = sizeof(Bank_Account_Type);
   BANK_ACCOUNT_TYPE->private_size = sizeof(Bank_Account_Data);
   BANK_ACCOUNT_TYPE->name = "Bank_Account_Model";
   BANK_ACCOUNT_TYPE->parent = EINA_MODEL_TYPE_GENERIC;
   BANK_ACCOUNT(BANK_ACCOUNT_TYPE)->bank_account_data_set = _bank_account_data_set;
   BANK_ACCOUNT(BANK_ACCOUNT_TYPE)->print = _bank_account_data_print;

   CREDIT_CARD_TYPE->version = EINA_MODEL_TYPE_VERSION;
   CREDIT_CARD_TYPE->type_size = sizeof(Credit_Card_Type);
   CREDIT_CARD_TYPE->private_size = sizeof(Credit_Card_Data);
   CREDIT_CARD_TYPE->name = "Credit_Card_Model";
   CREDIT_CARD_TYPE->parent = BANK_ACCOUNT_TYPE;
   CREDIT_CARD(CREDIT_CARD_TYPE)->credit_card_data_set = _credit_card_data_set;
   BANK_ACCOUNT(CREDIT_CARD_TYPE)->print = _credit_card_data_print;

   b = eina_model_new(BANK_ACCOUNT_TYPE);   //creating object of bank class
   cc = eina_model_new(CREDIT_CARD_TYPE);   //creating object of credit card class

   bank_account_data_set(b, "Bill Clark", "8569214756");
   bank_account_data_set(cc, "John Smith", "3154789");

   credit_card_data_set(cc, "5803 6589 4786 3279 9173", "01/01/2015", 1234);

   data_print(b);
   data_print(cc);

   eina_model_unref(b);
   eina_model_unref(cc);

   eina_shutdown();

   return 0;
}
