//Compile with:
//gcc -Wall -o eina_simple_json_parser_01 eina_simple_json_parser_01.c `pkg-config --cflags --libs eina`

#include <Eina.h>
#include <stdio.h>
#include <string.h>

static Eina_Bool _json_cb(void *data, Eina_Simple_JSON_Type type, const char *content, unsigned length);
static Eina_Bool _json_obj_cb(void *data, Eina_Simple_JSON_Value_Type type, const char *key, const char *content, unsigned length);
static Eina_Bool _json_array_cb(void *data, Eina_Simple_JSON_Value_Type type, const char *content, unsigned length);
static Eina_Bool _print(const void *container, void *data, void *fdata);

int
main(void)
{
   FILE *file;
   long size;
   char *buffer;

   eina_init();

   if ((file = fopen("chat.json", "rb")))
     {
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if ((buffer = malloc(size)))
          {
             size = fread(buffer, 1, size, file);
             eina_simple_json_parse(buffer, size, _json_cb, NULL);
             free(buffer);
          }
        else
          EINA_LOG_ERR("Can't allocate memory!");

        fclose(file);
     }
   else
     EINA_LOG_ERR("Can't open chat.json!");

   eina_shutdown();

   return 0;
}

static Eina_Bool
_json_obj_cb(void *data, Eina_Simple_JSON_Value_Type type, const char *key, const char *content, unsigned length)
{
   printf("-- %s: ", key);
   _json_array_cb(NULL, type, content, length);
   return EINA_TRUE;
}


static Eina_Bool
_json_array_cb(void *data, Eina_Simple_JSON_Value_Type type, const char *content, unsigned length)
{
   switch(type)
     {
      case EINA_SIMPLE_JSON_VALUE_ARRAY:
        {
           printf("\nArray [\n");
           eina_simple_json_array_parse(content, length, _json_array_cb, data);
           printf("]\n");
           break;
        }
      case EINA_SIMPLE_JSON_VALUE_OBJECT:
        {
           printf("{\n");
           if (eina_simple_json_object_parse(content, length, _json_obj_cb, data) == EINA_FALSE)
              printf("Parse Error\n");

           printf("}\n");
           break;
        }
      case EINA_SIMPLE_JSON_VALUE_NUMBER:
        {
           printf("Number   %.*s\n", length, content);
           break;
        }
      case EINA_SIMPLE_JSON_VALUE_STRING:
        {
           printf("String   %.*s\n", length, content);
           break;
        }
      case EINA_SIMPLE_JSON_VALUE_TRUE:
      case EINA_SIMPLE_JSON_VALUE_FALSE:
        {
           printf("Bool     %.*s\n", length, content);
           break;
        }
      case EINA_SIMPLE_JSON_VALUE_NULL:
        {
           printf("Null     %.*s\n", length, content);
           break;
        }
      case EINA_SIMPLE_JSON_VALUE_ERROR:
      default:
        {
           printf("\nERROR\n");
           break;
        }
     }

   return EINA_TRUE;
}

static Eina_Bool
_json_cb(void *data, Eina_Simple_JSON_Type type, const char *content, unsigned length)
{
   if (type == EINA_SIMPLE_JSON_OBJECT)
     {
        eina_simple_json_object_parse(content, length, _json_obj_cb, data);
     }
   else if (type == EINA_SIMPLE_JSON_ARRAY)
     {
        eina_simple_json_array_parse(content, length, _json_array_cb, data);
     }
   else if (type == EINA_SIMPLE_JSON_ERROR)
     {
        printf("ERROR  %s:%d \n", __FILE__, __LINE__);
     }

   return EINA_TRUE;
}

