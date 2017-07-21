#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "codegen.h"
#include <ctype.h>

Eina_Bool
file_read(const char *file_name, char **buffer)
{
   FILE *xml_handler;
   int data; /* fgetc needs int to detect EOF correctly */
   Eina_Strbuf *buf;

   xml_handler = fopen(file_name, "rt");
   if (!xml_handler)
     {
        printf("Error to read file: %s\n", file_name);
        return EINA_FALSE;
     }
   buf = eina_strbuf_new();

   while ((data = fgetc(xml_handler)) != EOF)
     eina_strbuf_append_char(buf, (char)data);

   fclose(xml_handler);
   *buffer = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);

   return EINA_TRUE;
}

Eina_Bool
file_write(const char *file_name, const char *buffer)
{
   FILE *file_handler;
   const char *filename = file_name;
   Eina_Strbuf *fname = NULL;

   fname = eina_strbuf_new();

   if (output_dir)
     {
        eina_strbuf_append_printf(fname, "%s/%s", output_dir, file_name);
        filename = eina_strbuf_string_get(fname);
     }
   file_handler = fopen(filename, "wt");
   if (!file_handler)
     {
        printf("Error to write file: %s\n", filename);
        eina_strbuf_free(fname);
        return EINA_FALSE;
     }

   if (fwrite(buffer, strlen(buffer), 1, file_handler) < 1)
     {
        printf("Error writing to file: %s\n", filename);
     }
   fclose(file_handler);
   eina_strbuf_free(fname);

   return EINA_TRUE;
}

char *
dbus_name_to_c(const char *dbus)
{
   char *str_cpy = strdup(dbus), *pch, *ret;
   Eina_Strbuf *buffer = eina_strbuf_new();
   unsigned i;

   pch = strtok(str_cpy, "/.");
   if (!pch)
     {
        ret = strdup("root");
        goto end;
     }
   eina_strbuf_append(buffer, pch);

   while ((pch = strtok(NULL, "/.")))
     eina_strbuf_append_printf(buffer, "_%s",pch);

   ret = eina_strbuf_string_steal(buffer);
   for (i = 0; ret[i]; i++)
     {
        if (i > 0 && ret[i-1] != '_' && ret[i] > '@' && ret[i] < '[')//upper case
          eina_strbuf_append_printf(buffer, "_%c", tolower(ret[i]));
        else
          eina_strbuf_append_char(buffer, tolower(ret[i]));
     }
   free(ret);
   ret = eina_strbuf_string_steal(buffer);
end:
   free(str_cpy);
   eina_strbuf_free(buffer);
   return ret;
}

char *
replace_string(const char *string, const char *substr, const char *replacement)
{
   char *str_cpy = strdup(string);
   char *pch;
   char *ret;
   Eina_Strbuf *buffer = eina_strbuf_new();

   pch = strtok(str_cpy, substr);
   if (!pch)
     {
        eina_strbuf_free(buffer);
        return str_cpy;
     }
   eina_strbuf_append(buffer, pch);

   while ((pch = strtok(NULL, substr)))
     eina_strbuf_append_printf(buffer, "%s%s", replacement, pch);

   ret = eina_strbuf_string_steal(buffer);
   free(str_cpy);
   eina_strbuf_free(buffer);
   return ret;
}

char *
get_pieces(const char *string, char break_in, int amount)
{
   int i;
   int found = 0;

   for (i = strlen(string) - 1; i && amount > found; i--)
     if (string[i] == break_in)
       found++;

   if (found)
     return strdup(string+i+2);
   else
     return strdup(string);
}

char *
string_build(const char *fmt, ...)
{
   va_list ap;
   Eina_Strbuf *buffer = eina_strbuf_new();
   char *ret;

   va_start(ap, fmt);
   eina_strbuf_prepend_vprintf(buffer, fmt, ap);
   va_end(ap);

   ret = eina_strbuf_string_steal(buffer);
   eina_strbuf_free(buffer);

   return ret;
}

#define UTIL_H "\
#ifndef ELDBUS_UTILS_H\n\
#define ELDBUS_UTILS_H 1\n\
\n\
typedef struct _Eldbus_Error_Info\n\
{\n\
   const char *error;\n\
   const char *message;\n\
} Eldbus_Error_Info;\n\
\n\
typedef void (*Eldbus_Codegen_Property_Set_Cb)(void *data, const char *propname, Eldbus_Proxy *proxy, Eldbus_Pending *p, Eldbus_Error_Info *error_info);\n\
\n\
typedef void (*Eldbus_Codegen_Property_String_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, const char *value);\n\
typedef void (*Eldbus_Codegen_Property_Int32_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, int value);\n\
typedef void (*Eldbus_Codegen_Property_Byte_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, unsigned char value);\n\
typedef void (*Eldbus_Codegen_Property_Bool_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, Eina_Bool value);\n\
typedef void (*Eldbus_Codegen_Property_Int16_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, short int value);\n\
typedef void (*Eldbus_Codegen_Property_Uint16_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, unsigned short int value);\n\
typedef void (*Eldbus_Codegen_Property_Uint32_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, unsigned int value);\n\
typedef void (*Eldbus_Codegen_Property_Double_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, double value);\n\
typedef void (*Eldbus_Codegen_Property_Int64_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, int64_t value);\n\
typedef void (*Eldbus_Codegen_Property_Uint64_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, uint64_t value);\n\
typedef void (*Eldbus_Codegen_Property_Complex_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, Eina_Value *value);\n\
\n\
#endif\
"

Eina_Bool
util_h_write(void)
{
   return file_write("eldbus_utils.h", UTIL_H);
}
