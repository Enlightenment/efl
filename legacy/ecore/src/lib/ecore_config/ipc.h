#include <Ecore_Ipc.h>
#include "Ecore_Config.h"

typedef enum {
  IPC_NONE,
  IPC_PROP_LIST,
  IPC_PROP_DESC,
  IPC_PROP_GET,
  IPC_PROP_SET,

  IPC_BUNDLE_LIST,
  IPC_BUNDLE_NEW,
  IPC_BUNDLE_LABEL_GET,
  IPC_BUNDLE_LABEL_SET,
  IPC_BUNDLE_LABEL_FIND,

  IPC_LAST
} ex_ipc_call;

Ecore_Config_Server *srv2ecore_config_srv(void *srv);

char *ipc_prop_list(Ecore_Config_Server *srv, const long serial);
char *ipc_prop_desc(Ecore_Config_Server *srv, const long serial,const char *key);
char *ipc_prop_get(Ecore_Config_Server *srv, const long serial,const char *key);
int   ipc_prop_set(Ecore_Config_Server *srv, const long serial,const char *key,const char *val);

char *ipc_bundle_list(Ecore_Config_Server *srv);
int   ipc_bundle_new(Ecore_Config_Server *srv, const char *);
char *ipc_bundle_label_get(Ecore_Config_Server *srv, const long);
int   ipc_bundle_label_set(Ecore_Config_Server *srv, const long,const char *);
long  ipc_bundle_label_find(Ecore_Config_Server *srv, const char *);

