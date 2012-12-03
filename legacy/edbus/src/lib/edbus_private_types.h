#ifndef EDBUS_PRIVATE_TYPES_H
#define EDBUS_PRIVATE_TYPES_H 1

#include "EDBus.h"
#include <Eina.h>
#include <dbus/dbus.h>
#include <Ecore.h>

typedef struct _EDBus_Connection_Name
{
   const char *name;
   const char *unique_id;
   Eina_Hash  *objects;
   int refcount;
   struct
   {
      Eina_Inlist *list; //EDBus_Connection_Context_NOC_Cb
      int          walking;
      Eina_List   *to_delete;
   } event_handlers;
   EDBus_Signal_Handler *name_owner_changed;
} EDBus_Connection_Name;

typedef struct _EDBus_Object_Context_Event_Cb
{
   EINA_INLIST;
   EDBus_Object_Event_Cb cb;
   const void           *cb_data;
   Eina_Bool             deleted : 1;
} EDBus_Object_Context_Event_Cb;

typedef struct _EDBus_Object_Context_Event
{
   Eina_Inlist *list;
   int          walking;
   Eina_List   *to_delete;
} EDBus_Object_Context_Event;


typedef struct _EDBus_Connection_Context_Event
{
   Eina_Inlist *list;
   int          walking;
   Eina_List   *to_delete;
} EDBus_Connection_Context_Event;

struct _EDBus_Connection
{
   EINA_MAGIC;
   EINA_INLIST;
   int                            refcount;
   EDBus_Connection_Type          type;
   DBusConnection                *dbus_conn;
   Eina_Hash                     *names; //EDBus_Connection_Name
   Eina_Inlist                   *data;
   Eina_Inlist                   *cbs_free;
   Eina_Inlist                   *signal_handlers;
   Eina_Inlist                   *pendings;
   Eina_Inlist                   *fd_handlers;
   Eina_Inlist                   *timeouts;
   Ecore_Idler                   *idler;
   Eina_Bool                      running_signal;
   EDBus_Connection_Context_Event event_handlers[EDBUS_CONNECTION_EVENT_LAST];
   Eina_Inlist                   *root_objs;//service_object
};

struct _EDBus_Object
{
   EINA_MAGIC;
   EINA_INLIST;
   int                        refcount;
   EDBus_Connection          *conn;
   const char                *name;
   const char                *path;
   Eina_Hash                 *proxies;
   Eina_Inlist               *pendings;
   Eina_List                 *signal_handlers;
   Eina_Inlist               *cbs_free;
   EDBus_Object_Context_Event event_handlers[EDBUS_OBJECT_EVENT_LAST];
   EDBus_Signal_Handler *interfaces_added;
   EDBus_Signal_Handler *interfaces_removed;
   EDBus_Signal_Handler *properties_changed;
};

struct _EDBus_Signal_Handler
{
   EINA_MAGIC;
   EINA_INLIST;
   int                       refcount;
   const char               *sender;
   const char               *path;
   const char               *interface;
   const char               *member;
   Eina_Strbuf              *match;
   Eina_Inlist              *args;
   Eina_Inlist_Sorted_State *state_args;
   EDBus_Connection         *conn;
   EDBus_Signal_Cb           cb;
   EDBus_Connection_Name    *bus;
   const void               *cb_data;
   Eina_Inlist              *cbs_free;
   Eina_Bool                 dangling;
};

struct _EDBus_Pending
{
   EINA_MAGIC;
   EINA_INLIST;
   EDBus_Message_Cb  cb;
   const void       *cb_data;
   DBusPendingCall  *dbus_pending;
   EDBus_Connection *conn;
   const char       *dest;
   const char       *path;
   const char       *interface;
   const char       *method;
   Eina_Inlist      *data;
   Eina_Inlist      *cbs_free;
   EDBus_Message    *msg_sent;
};

struct _EDBus_Message_Iter
{
   EINA_MAGIC;
   EINA_INLIST;
   DBusMessageIter dbus_iterator;
   Eina_Inlist *iterators;
   Eina_Bool writable;
};

struct _EDBus_Message
{
   EINA_MAGIC;
   int          refcount;
   DBusMessage *dbus_msg;
   EDBus_Message_Iter *iterator;
};

typedef struct _EDBus_Service_Object EDBus_Service_Object;
struct _EDBus_Service_Object
{
   EINA_INLIST;
   EDBus_Connection *conn;
   const char *path;
   Eina_Hash *interfaces;
   Eina_Strbuf *introspection_data;
   Eina_Bool introspection_dirty;
   Eina_Inlist *data;
   EDBus_Service_Object *parent;
   Eina_Inlist *children;
   //ObjectManager data
   Eina_Bool has_objectmanager:1;
   Eina_List *iface_added;
   Eina_List *iface_removed;
   Ecore_Idler *idler_iface_changed;
};

struct _EDBus_Service_Interface
{
   EINA_MAGIC;
   const char *name;
   Eina_Hash *methods;
   const EDBus_Signal *signals;
   Eina_Array *sign_of_signals;
   EDBus_Service_Object *obj;
   Eina_Hash *properties;
   EDBus_Property_Set_Cb set_func;
   EDBus_Property_Get_Cb get_func;
   Ecore_Idler *idler_propschanged;
   Eina_Array *props_changed;
   Eina_Array *prop_invalidated;
};

typedef struct _Signal_Argument
{
   EINA_INLIST;
   unsigned short index;
   const char *value;
} Signal_Argument;

#endif
