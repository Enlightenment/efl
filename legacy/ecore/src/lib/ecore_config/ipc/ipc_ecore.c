/* by Azundris, with thanks to Corey Donohoe <atmos@atmos.org> */

//#include "config.h"
#include "errors.h"
#include "debug.h"
#include "ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include <Ecore.h>
#include <Ecore_Ipc.h>

#include "Ecore_Config.h"


/*****************************************************************************/



static int get_string(char **m,char **r) {
  char *q;
  int   l=0;

  if(!m||!*m)
    return ECORE_CONFIG_ERR_NODATA;
  if(!r)
    return ECORE_CONFIG_ERR_FAIL;
  q=*m;
  if(*q!='s')
    return ECORE_CONFIG_ERR_TYPEMISMATCH;
  q++;
  l=(*(q++))<<8;
  l+=*(q++);
  *r=q;
  q+=l;
  *m=q;
  E(1,"IPC/eCore: got string-%d \"%s\"\n",l,*r);
  return ECORE_CONFIG_ERR_SUCC; }






/*****************************************************************************/



static int send(Ecore_Ipc_Event_Client_Data *e,int code,char *reply) {
  static int our_ref=0;
  int        len=reply?strlen(reply)+1:0;
  our_ref++;
  E(1,"IPC/eCore: replying [0,0] %d IRT %d => %d {\"%s\":%d}\n",our_ref,e->ref,code,reply?reply:"",len);
  return ecore_ipc_client_send(e->client,0,0,our_ref,e->ref,code,reply,len); }






/*****************************************************************************/

static int handle_request(Ecore_Ipc_Server *server,Ecore_Ipc_Event_Client_Data *e) {
  Ecore_Config_Server *srv;
  long  serial=e->minor;
  int   ret=ECORE_CONFIG_ERR_FAIL;
  char *r=NULL,*k,*v,*m=(char *)e->data;
  srv=srv2ecore_config_srv(server);

  E(1,"IPC/eCore: client sent: [%d,%d] #%d (%d) @ %p\n",e->major,e->minor,e->ref,e->size,server);

  switch(e->major) {
    case IPC_PROP_LIST:
      r=ipc_prop_list(srv, serial);
      break;
    case IPC_PROP_DESC:
      if(get_string(&m,&k)==ECORE_CONFIG_ERR_SUCC)
        r=ipc_prop_desc(srv, serial,k);
      break;
    case IPC_PROP_GET:
      if(get_string(&m,&k)==ECORE_CONFIG_ERR_SUCC)
        r=ipc_prop_get(srv, serial,k);
      break;
    case IPC_PROP_SET:
      if(get_string(&m,&k)==ECORE_CONFIG_ERR_SUCC) {
        if(get_string(&m,&v)==ECORE_CONFIG_ERR_SUCC)
          return send(e,ipc_prop_set(srv, serial,k,v),NULL); }
      break;

    case IPC_BUNDLE_LIST:
      r=ipc_bundle_list(srv);
      break;
    case IPC_BUNDLE_NEW:
      if(get_string(&m,&k)==ECORE_CONFIG_ERR_SUCC)
        return send(e,k?ipc_bundle_new(srv, k):ECORE_CONFIG_ERR_FAIL,NULL);
      break;
    case IPC_BUNDLE_LABEL_SET:
      if(get_string(&m,&k)==ECORE_CONFIG_ERR_SUCC)
        return send(e,k?ipc_bundle_label_set(srv, serial,k):ECORE_CONFIG_ERR_FAIL,NULL);
      break;
    case IPC_BUNDLE_LABEL_FIND:
      if(get_string(&m,&k)==ECORE_CONFIG_ERR_SUCC)
        return send(e,ipc_bundle_label_find(srv, k),NULL);
      break;
    case IPC_BUNDLE_LABEL_GET:
      r=ipc_bundle_label_get(srv, serial);
      break; }

  ret=send(e,r?ECORE_CONFIG_ERR_SUCC:ECORE_CONFIG_ERR_FAIL,r);
  if(r) {
    free(r);
    return ret; }
  return ECORE_CONFIG_ERR_NOTFOUND; }






/*****************************************************************************/



static int ipc_client_add(void *data,int type,void *event) {
  Ecore_Ipc_Server           **server=(Ecore_Ipc_Server **)data;
  Ecore_Ipc_Event_Client_Data *e=(Ecore_Ipc_Event_Client_Data *)event;

  if (*server != ecore_ipc_client_server_get(e->client))
    return 1;

  E(1,"IPC/eCore: Client connected. @ %p\n",server);
  return 1; }



static int ipc_client_del(void *data, int type, void *event) {
  Ecore_Ipc_Server           **server=(Ecore_Ipc_Server **)data;
  Ecore_Ipc_Event_Client_Data *e=(Ecore_Ipc_Event_Client_Data *)event;

  if (*server != ecore_ipc_client_server_get(e->client))
    return 1;
  
  E(1,"IPC/eCore: Client disconnected. @ %p\n",server);
  return 1; }



static int ipc_client_sent(void *data,int type,void *event) {
  Ecore_Ipc_Server            **server=(Ecore_Ipc_Server **)data;
  Ecore_Ipc_Event_Client_Data  *e=(Ecore_Ipc_Event_Client_Data *)event;

  if (*server != ecore_ipc_client_server_get(e->client))
    return 1;

  handle_request(*server,e);
  return 1; }






/*****************************************************************************/



int ipc_init(char *pipe_name, void **data) {
  Ecore_Ipc_Server **server=(Ecore_Ipc_Server **)data;
  struct stat        st;
  char              *p;
  int                port=0;
  char              *str;

  if(!server)
    return ECORE_CONFIG_ERR_FAIL;

//  if(*server)
//    return ECORE_CONFIG_ERR_IGNORED;

  ecore_init();
  if(ecore_ipc_init()<1)
    return ECORE_CONFIG_ERR_FAIL;

  if((p=getenv("HOME"))) {  /* debug-only ### FIXME */
    char buf[PATH_MAX];
    str=malloc(1000*sizeof(char));
    int stale=1;
    while (stale) {
      sprintf(str,"%s/.ecore/%s/%d",p,pipe_name,port);
      snprintf(buf,PATH_MAX,str);

      if(!stat(buf,&st)) {
        E(0,"IPC/eCore: pipe \"%s\" already exists!?\n",buf);
//      if(unlink(buf))
//	E(0,"IPC/eCore: could not remove pipe \"%s\": %d\n",buf,errno); }}
        port++;
      } else {
        stale = 0;
      }
    }
  }
  *server=ecore_ipc_server_add(ECORE_IPC_LOCAL_USER,pipe_name,port,NULL);
  ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_ADD, ipc_client_add, server);
  ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DEL, ipc_client_del, server);
  ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DATA,ipc_client_sent,server);

  sprintf(str,"IPC/eCore: Server is listening on %s.\n", pipe_name);
  E(1,str);

  return ECORE_CONFIG_ERR_SUCC; }



int ipc_exit(void **data) {
  int                ret=ECORE_CONFIG_ERR_SUCC;
  Ecore_Ipc_Server **server=(Ecore_Ipc_Server **)data;

  if(!server)
    return ECORE_CONFIG_ERR_FAIL;

  if(*server) {
    ecore_ipc_server_del(*server);
    *server=NULL; }

  ecore_ipc_shutdown();

  return ret; }






/*****************************************************************************/



int ipc_poll(void **data) {
  Ecore_Ipc_Server **server=(Ecore_Ipc_Server **)data;

  if(!server)
    return ECORE_CONFIG_ERR_FAIL;

  ecore_main_loop_iterate();

  return ECORE_CONFIG_ERR_SUCC; }



/*****************************************************************************/
