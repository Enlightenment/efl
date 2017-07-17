#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <ctype.h>

#define ACCESS_READ 1 << 0
#define ACCESS_WRITE 1 << 1

typedef struct _DBus_Object
{
   char *name;
   char *c_name;
   Eina_Inlist *ifaces;
} DBus_Object;

typedef struct _DBus_Interface
{
   EINA_INLIST;
   char *name;
   char *c_name;
   Eina_Inlist *methods;
   Eina_Inlist *signals;
   Eina_Inlist *properties;
   DBus_Object *obj;
} DBus_Interface;

typedef struct _DBus_Signal
{
   EINA_INLIST;
   char *name;
   char *c_name;
   char *cb_name;
   char *struct_name;
   char *free_function;
   Eina_Inlist *args;
   unsigned int arg_without_name;
   DBus_Interface *iface;
   Eina_Bool complex;
   char *signal_event;
} DBus_Signal;

typedef struct _DBus_Arg
{
   EINA_INLIST;
   char *type;
   char direction;
   char *name;
   char *c_name;
} DBus_Arg;

typedef enum
{
   NO_REPLY = 0,
   INVALID
} DBus_Annotation_Type;

typedef struct _DBus_Annotation
{
   DBus_Annotation_Type type;
   char *value;
} DBus_Annotation;

typedef struct _DBus_Method
{
   EINA_INLIST;
   char *name;
   char *c_name;
   char *cb_name;
   char *function_cb;
   Eina_Inlist *args;
   unsigned int arg_without_name;
   DBus_Interface *iface;
   Eina_Bool in_complex:1;
   Eina_Bool out_complex:1;
   Eina_Bool no_reply:1;
} DBus_Method;

typedef struct _DBus_Property
{
   EINA_INLIST;
   char *name;
   char *c_name;
   char *cb_name;
   char *type;
   char access;
   DBus_Interface *iface;
   Eina_Bool complex;
} DBus_Property;

Eina_Bool parser(void *data, Eina_Simple_XML_Type type, const char *content, unsigned offset, unsigned length);

void object_free(DBus_Object *obj);
DBus_Interface *interface_new(DBus_Object *obj);
void interface_free(DBus_Interface *iface);
DBus_Signal *signal_new(DBus_Interface *iface);
void signal_free(DBus_Signal *signal);
void arg_free(DBus_Arg *arg);
void method_free(DBus_Method *method);
DBus_Method *method_new(DBus_Interface *iface);
DBus_Property *property_new(DBus_Interface *iface);
void property_free(DBus_Property *property);

void source_client_generate(DBus_Object *path, const char *prefix, const char *interface_name, const char *output);

//utils
Eina_Bool file_read(const char *file_name, char **buffer);
Eina_Bool file_write(const char *file_name, const char *buffer);
Eina_Bool util_h_write(void);
char *replace_string(const char *string, const char *substr, const char *replacement);
char *dbus_name_to_c(const char *dbus);
char *string_build(const char *fmt, ...);
char *get_pieces(const char *string, char break_in, int amount);

extern char *output_dir;
