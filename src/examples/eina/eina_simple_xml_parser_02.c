//Compile with:
//gcc -Wall -o eina_simple_xml_parser_02 eina_simple_xml_parser_02.c `pkg-config --cflags --libs eina`

#include <Eina.h>
#include <stdio.h>
#include <string.h>


static Eina_Bool
_xml_attribute_parse_cb(void *data EINA_UNUSED, const char *key, const char *value)
{
   printf("attributes, key = %s, value = %s\n", key, value);
   return EINA_TRUE;
}

static Eina_Bool
_xml_tag_parse_cb(void *data EINA_UNUSED, Eina_Simple_XML_Type type, const char *content,
                  unsigned offset EINA_UNUSED, unsigned int length)
{
   if (length == 0) return EINA_FALSE;

   if (type == EINA_SIMPLE_XML_OPEN)
     {
        //Print tag
        if (!strncmp("Group", content, strlen("Group")))
          printf("tag = Group\n");
        else if (!strncmp("Label", content, strlen("Label")))
          printf("tag = Label\n");

        //Print attributes
        const char *tags = eina_simple_xml_tag_attributes_find(content, length);
        eina_simple_xml_attributes_parse(tags, length - (tags - content),
                                         _xml_attribute_parse_cb, NULL);
     }

   return EINA_TRUE;
}

int
main(void)
{
   FILE *file;
   long size;

   eina_init();

   //1. Open XML File
   file = fopen("exia.xml", "rb");

   if (!file) return 0;

   fseek(file, 0, SEEK_END);
   size = ftell(file);
   fseek(file, 0, SEEK_SET);

   //2. Read XML File
   char *buffer = malloc(size);
   if (!buffer)
     {
        fclose(file);
        return 0;
     }

   fread(buffer, 1, size, file);

   //3. Start Parsing XML
   eina_simple_xml_parse(buffer, size, EINA_FALSE, _xml_tag_parse_cb, NULL);

   fclose(file);
   free(buffer);

   return 0;
}
