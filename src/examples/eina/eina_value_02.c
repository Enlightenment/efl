//Compile with:
//gcc eina_value_02.c -o eina_value_02 `pkg-config --cflags --libs eina`

#include <Eina.h>

static Eina_Value_Struct_Desc *V1_DESC = NULL;
static Eina_Value_Struct_Desc *V2_DESC = NULL;

void value_init(void)
{
   typedef struct _My_Struct_V1 {
     int param1;
     char param2;
   } My_Struct_V1;


   static Eina_Value_Struct_Member v1_members[] = {
     // no eina_value_type as they are not constant initializers, see below.
     EINA_VALUE_STRUCT_MEMBER(NULL, My_Struct_V1, param1),
     EINA_VALUE_STRUCT_MEMBER(NULL, My_Struct_V1, param2)
   };
   v1_members[0].type = EINA_VALUE_TYPE_INT;
   v1_members[1].type = EINA_VALUE_TYPE_CHAR;
   static Eina_Value_Struct_Desc v1_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, // no special operations
     v1_members,
     EINA_C_ARRAY_LENGTH(v1_members),
     sizeof(My_Struct_V1)
   };
   V1_DESC = &v1_desc;

   typedef struct _My_Struct_V2 {
     int param1;
     char param2;
     int param3;
   } My_Struct_V2;
   static Eina_Value_Struct_Member v2_members[] = {
     // no eina_value_type as they are not constant initializers, see below.
     EINA_VALUE_STRUCT_MEMBER(NULL, My_Struct_V2, param1),
     EINA_VALUE_STRUCT_MEMBER(NULL, My_Struct_V2, param2),
     EINA_VALUE_STRUCT_MEMBER(NULL, My_Struct_V2, param3)
   };
   v2_members[0].type = EINA_VALUE_TYPE_INT;
   v2_members[1].type = EINA_VALUE_TYPE_CHAR;
   v2_members[2].type = EINA_VALUE_TYPE_INT;
   static Eina_Value_Struct_Desc v2_desc = {
     EINA_VALUE_STRUCT_DESC_VERSION,
     NULL, // no special operations
     v2_members,
     EINA_C_ARRAY_LENGTH(v2_members),
     sizeof(My_Struct_V2)
   };
   V2_DESC = &v2_desc;
}

void rand_init(Eina_Value *v)
{
   if (v->type != EINA_VALUE_TYPE_STRUCT)
     return;

   eina_value_struct_set(v, "param1", rand());
   eina_value_struct_set(v, "param2", rand() % 256);
   eina_value_struct_set(v, "param3", rand());
}

void my_struct_use(Eina_Value *params)
{
   int p1, p3;
   char p2;

   eina_value_struct_get(params, "param1", &p1);
   eina_value_struct_get(params, "param2", &p2);
   printf("param1: %d\nparam2: %c\n", p1, p2);

   if (eina_value_struct_get(params, "param3", &p3))
     printf("param3: %d\n", p3);
}

int main(int argc, char **argv)
{
   (void)argc;
   (void)argv;
   Eina_Value *v1, *v2;

   eina_init();
   value_init();
   srand(time(NULL));

   v1 = eina_value_struct_new(V1_DESC);
   v2 = eina_value_struct_new(V2_DESC);

   rand_init(v1);
   my_struct_use(v1);

   rand_init(v2);
   my_struct_use(v2);

   eina_value_free(v1);
   eina_value_free(v2);
   eina_shutdown();
}
