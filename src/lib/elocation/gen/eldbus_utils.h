#ifndef ELDBUS_UTILS_H
#define ELDBUS_UTILS_H 1

typedef struct _Eldbus_Error_Info
{
   const char *error;
   const char *message;
} Eldbus_Error_Info;

typedef void (*Eldbus_Codegen_Property_Set_Cb)(void *data, const char *propname, Eldbus_Proxy *proxy, Eldbus_Pending *p, Eldbus_Error_Info *error_info);

typedef void (*Eldbus_Codegen_Property_String_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, const char *value);
typedef void (*Eldbus_Codegen_Property_Int32_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, int value);
typedef void (*Eldbus_Codegen_Property_Byte_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, unsigned char value);
typedef void (*Eldbus_Codegen_Property_Bool_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, Eina_Bool value);
typedef void (*Eldbus_Codegen_Property_Int16_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, short int value);
typedef void (*Eldbus_Codegen_Property_Uint16_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, unsigned short int value);
typedef void (*Eldbus_Codegen_Property_Uint32_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, unsigned int value);
typedef void (*Eldbus_Codegen_Property_Double_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, double value);
typedef void (*Eldbus_Codegen_Property_Int64_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, int64_t value);
typedef void (*Eldbus_Codegen_Property_Uint64_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, uint64_t value);
typedef void (*Eldbus_Codegen_Property_Complex_Get_Cb)(void *data, Eldbus_Pending *p, const char *propname, Eldbus_Proxy *proxy, Eldbus_Error_Info *error_info, Eina_Value *value);

#endif