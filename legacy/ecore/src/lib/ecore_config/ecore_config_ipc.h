#include <Ecore_Ipc.h>
#include "Ecore_Config.h"

typedef enum
{
   IPC_NONE,
   IPC_PROP_LIST,
   IPC_PROP_DESC,
   IPC_PROP_GET,
   IPC_PROP_SET, /* end of the codes shared by evidence and econf */

   IPC_GLOBAL_PROP_LIST,

   IPC_BUNDLE_LIST,
   IPC_BUNDLE_NEW,
   IPC_BUNDLE_LABEL_GET,
   IPC_BUNDLE_LABEL_SET,
   IPC_BUNDLE_LABEL_FIND,

   IPC_LAST
} Ecore_Config_Ipc_Call;

Ecore_Config_Server *_ecore_config_ipc_init(const char *pipe_name);
int                  _ecore_config_ipc_exit(void);

Ecore_Config_Server *_ecore_config_server_convert(void *srv);

char               *_ecore_config_ipc_prop_list(Ecore_Config_Server * srv,
						const long serial);
char               *_ecore_config_ipc_prop_desc(Ecore_Config_Server * srv,
						const long serial,
						const char *key);
char               *_ecore_config_ipc_prop_get(Ecore_Config_Server * srv,
					       const long serial,
					       const char *key);
int                 _ecore_config_ipc_prop_set(Ecore_Config_Server * srv,
					       const long serial,
					       const char *key,
					       const char *val);

char               *_ecore_config_ipc_bundle_list(Ecore_Config_Server * srv);
int                 _ecore_config_ipc_bundle_new(Ecore_Config_Server * srv,
						 const char *);
char               *_ecore_config_ipc_bundle_label_get(Ecore_Config_Server *
						       srv, const long);
int                 _ecore_config_ipc_bundle_label_set(Ecore_Config_Server *
						       srv, const long,
						       const char *);
long                _ecore_config_ipc_bundle_label_find(Ecore_Config_Server *
							srv, const char *);
