#ifndef ELDBUS_PRIVATE_TYPES_H
#define ELDBUS_PRIVATE_TYPES_H 1

typedef struct _Eldbus_Connection_Name
{
   const char *name;
   const char *unique_id;
   Eina_Hash  *objects;
   int refcount;
   struct
   {
      Eina_Inlist *list; //Eldbus_Connection_Context_NOC_Cb
      int          walking;
      Eina_List   *to_delete;
   } event_handlers;
   Eldbus_Signal_Handler *name_owner_changed;
   Eldbus_Pending *name_owner_get;
} Eldbus_Connection_Name;

typedef struct _Eldbus_Object_Context_Event_Cb
{
   EINA_INLIST;
   Eldbus_Object_Event_Cb cb;
   const void           *cb_data;
   Eina_Bool             deleted : 1;
} Eldbus_Object_Context_Event_Cb;

typedef struct _Eldbus_Object_Context_Event
{
   Eina_Inlist *list;
   int          walking;
   Eina_List   *to_delete;
} Eldbus_Object_Context_Event;


typedef struct _Eldbus_Connection_Context_Event
{
   Eina_Inlist *list;
   int          walking;
   Eina_List   *to_delete;
} Eldbus_Connection_Context_Event;

struct _Eldbus_Connection
{
   EINA_MAGIC;
   EINA_INLIST;
   int                            refcount;
   Eldbus_Connection_Type          type;
   DBusConnection                *dbus_conn;
   Eina_Hash                     *names; //Eldbus_Connection_Name
   Eina_Inlist                   *data;
   Eina_Inlist                   *cbs_free;
   Eina_Inlist                   *signal_handlers;
   Eina_Inlist                   *pendings;
   Eina_Inlist                   *fd_handlers;
   Eina_Inlist                   *timeouts;
   Ecore_Idle_Enterer            *idle_enterer;
   Eldbus_Connection_Context_Event event_handlers[ELDBUS_CONNECTION_EVENT_LAST];
   Eina_Inlist                   *root_objs;//service_object
   Eldbus_Proxy                   *fdo_proxy;
   Eina_Bool                     shared : 1; /* Indicate if connection can be shared */
};

struct _Eldbus_Object
{
   EINA_MAGIC;
   EINA_INLIST;
   int                        refcount;
   Eldbus_Connection          *conn;
   const char                *name;
   const char                *path;
   Eina_Hash                 *proxies;
   Eina_Inlist               *pendings;
   Eina_List                 *signal_handlers;
   Eina_Inlist               *cbs_free;
   Eldbus_Object_Context_Event event_handlers[ELDBUS_OBJECT_EVENT_LAST];
   Eldbus_Signal_Handler *interfaces_added;
   Eldbus_Signal_Handler *interfaces_removed;
   Eldbus_Signal_Handler *properties_changed;
   Eldbus_Proxy *properties;
};

struct _Eldbus_Signal_Handler
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
   Eldbus_Connection         *conn;
   Eldbus_Signal_Cb           cb;
   Eldbus_Connection_Name    *bus;
   const void               *cb_data;
   Eina_Inlist              *cbs_free;
   Eina_Bool                 dangling;
};

struct _Eldbus_Pending
{
   EINA_MAGIC;
   EINA_INLIST;
   Eldbus_Message_Cb  cb;
   const void       *cb_data;
   DBusPendingCall  *dbus_pending;
   Eldbus_Connection *conn;
   const char       *dest;
   const char       *path;
   const char       *interface;
   const char       *method;
   Eina_Inlist      *data;
   Eina_Inlist      *cbs_free;
   Eldbus_Message    *msg_sent;
};

struct _Eldbus_Message_Iter
{
   EINA_MAGIC;
   EINA_INLIST;
   DBusMessageIter dbus_iterator;
   Eina_Inlist *iterators;
   Eina_Bool writable;
};

struct _Eldbus_Message
{
   EINA_MAGIC;
   int          refcount;
   DBusMessage *dbus_msg;
   Eldbus_Message_Iter *iterator;
};

typedef struct _Eldbus_Service_Object Eldbus_Service_Object;
struct _Eldbus_Service_Object
{
   EINA_INLIST;
   Eldbus_Connection *conn;
   const char *path;
   Eina_Hash *interfaces;
   Eina_Strbuf *introspection_data;
   Eina_Bool introspection_dirty;
   Eina_Inlist *data;
   Eldbus_Service_Object *parent;
   Eina_Inlist *children;

   Eina_Bool fallback :1; /* Fallback mechanism flag */

   //ObjectManager data
   Eldbus_Service_Interface *objmanager;
   Eina_List *iface_added;
   Eina_List *iface_removed;
   Ecore_Idle_Enterer *idle_enterer_iface_changed;
};

struct _Eldbus_Service_Interface
{
   EINA_MAGIC;
   const char *name;
   Eina_Hash *methods;
   const Eldbus_Signal *signals;
   Eina_Array *sign_of_signals;
   Eldbus_Service_Object *obj;
   Eina_Hash *properties;
   Eldbus_Property_Set_Cb set_func;
   Eldbus_Property_Get_Cb get_func;
   Ecore_Idle_Enterer *idle_enterer_propschanged;
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
