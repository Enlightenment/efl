/* by Azundris */

//#include "config.h"

#include <Ecore.h>
#include <Ecore_Ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>


#include "ipc.h"
#include "ecore_config_client.h"



/*****************************************************************************/



const char *ecore_config_error(int no) {
  static const char *ecore_config_errstr[]={
    "Success",
    "General failure",
    "No data",
    "Action was not necessary",
    "Out of memory",
    "Not found",
    "Resource unavailable",
    "Parameters were of the wrong type",
    "Maximum length for path exceeded",
    "Partial success",
    "Parameter is not an executable file",
    "The type of monitor you are trying to set does not match the type of the object you are trying to set the monitor on",
    "File handed to meta-data provider has invalid contents",
    "File-I/O error in meta-data provider",
    "Could not activate meta-data provider",
    "Action requires a file",
    "Action not supported" };

  if(no<0)
    no=-no;
  if(no<=-ECORE_CONFIG_ERR_NOTSUPP)
    return ecore_config_errstr[no];
  return "No such error"; }



static int print_data(char *d,size_t l) {
  char *e=d+l,*f;
  while(d<e) {
    l=(d[1]<<8)|d[2];
    if(*d=='s') {
      f=d+3;
      printf("  STR-%d: \"%s\"\n",l,f); }
    else
      printf("  skipping chunk type '%c' {%d bytes}...\n",*d,l);
    d+=3+l; }
  return ECORE_CONFIG_ERR_SUCC; }



/*****************************************************************************/

int is_dir(const char *dir) {
  struct stat st;

  if (stat(dir, &st))
    return 0;

  return (S_ISDIR(st.st_mode));
}
          

int ex_ipc_init(ex_ipc_server_list **srv_list,char *pipe_name,connstate *cs) {
  int global, port, connected;
  struct stat st;
  char *p;
  char str[PATH_MAX], buf[PATH_MAX];
  DIR *dir;
  struct dirent *socket;
  Ecore_Ipc_Server *tmp_sock;
  ex_ipc_server_list *tmp;
    
  global=FALSE;
  port=0;
  
  if(ecore_ipc_init()<1)
    return ECORE_CONFIG_ERR_FAIL;
  if(!srv_list)
    return ECORE_CONFIG_ERR_FAIL;
  if(*srv_list)
    return ECORE_CONFIG_ERR_IGNORED;

  if((p=getenv("HOME"))) {  /* debug-only ### FIXME */
    sprintf(str,"%s/.ecore/%s/.global",p,pipe_name);
    if (stat(str, &st))
      global=FALSE;
    else
      global=TRUE;
    
    sprintf(str,"%s/.ecore/%s/",p,pipe_name);
    snprintf(buf,PATH_MAX,str);

    if(dir=opendir(buf)) {
      connected=0;
      while ((socket=readdir(dir))) {
        if (!strcmp(socket->d_name, ".") || !strcmp(socket->d_name, "..") ||
            !strcmp(socket->d_name, ".global"))
          continue;
        if (!is_dir(socket->d_name)) {
          port=atoi(socket->d_name);

          E(2,"Trying %s on port %d ", pipe_name, port);
          //if (!connected) {
          if((tmp_sock=ecore_ipc_server_connect(ECORE_IPC_LOCAL_USER,pipe_name,port,NULL))) {
            E(2, "- connected!\n");
            if (!(tmp=malloc(sizeof(ex_ipc_server_list))))
              return ECORE_CONFIG_ERR_OOM;
            memset(tmp,0,sizeof(ex_ipc_server_list));
            tmp->srv=tmp_sock;
    
            if (!*srv_list)
              *srv_list=tmp;
            else {
              tmp->next=*srv_list;
            *srv_list=tmp;}

    
          connected=1;
          if (!global) 
            break;
          } else {
            /* not connected */
            E(2, "- failed :(\n"); }
    
          //} else {
          //  E(3, "Should also try connecting to %d!!!\n", port);
          //}

        } /* is not dir */
      } /* while dir */

      if (connected) {

        ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_ADD, ex_ipc_server_con,cs);
        ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DEL, ex_ipc_server_dis,cs);
        ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DATA,ex_ipc_server_sent,NULL);

        return ECORE_CONFIG_ERR_SUCC;
      }
    } /* openfir */
  } else { /* if we cannot access the home variable */
    if((tmp_sock=ecore_ipc_server_connect(ECORE_IPC_LOCAL_USER,pipe_name,0,NULL))) {
E(2, "Connected to %s on default port (0)!\n", pipe_name);
      ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_ADD, ex_ipc_server_con,cs);
      ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DEL, ex_ipc_server_dis,cs);
      ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DATA,ex_ipc_server_sent,NULL);
    if (!(tmp=malloc(sizeof(ex_ipc_server_list))))
      return ECORE_CONFIG_ERR_OOM;
    memset(tmp,0,sizeof(ex_ipc_server_list));
    tmp->srv=tmp_sock;

    if (!*srv_list)
     *srv_list=tmp;
                                       
      return ECORE_CONFIG_ERR_SUCC; }
  }

  *srv_list=NULL;
  return ECORE_CONFIG_ERR_NOTFOUND; }



int ex_ipc_exit(ex_ipc_server_list **srv_list) {
  ex_ipc_server_list *tmp = *srv_list;
  if(!srv_list)
    return ECORE_CONFIG_ERR_FAIL;
  if(*srv_list)
    while (tmp) {
      ecore_ipc_server_del(tmp->srv);
      tmp = tmp->next;
    }
  *srv_list=NULL;
  ecore_ipc_shutdown();
  return ECORE_CONFIG_ERR_SUCC; }



int ex_ipc_sigexit(void *data, int type, void *event) {
  return 0; }



/*****************************************************************************/



int ex_ipc_send(ex_ipc_server_list **srv_list,int major,int minor,void *data,int size) {
  ex_ipc_server_list *tmp = *srv_list;
  static int ref=0;
  int        ret;

  if(!srv_list)
    return ECORE_CONFIG_ERR_FAIL;
  if(!*srv_list)
    return ECORE_CONFIG_ERR_NODATA;
  ret = ECORE_CONFIG_ERR_NODATA;

  if(size<0)
    size=data?strlen(data)+1:0;

  while (tmp) {
    ret=ecore_ipc_server_send(tmp->srv,major,minor,++ref,0,0,data,size);
    tmp = tmp->next;
  }
  E(2,"exsh: we sent: %2d.%02d  #%03d  \"%s\".%d  =>  %d\n",
    major,minor,ref,(char *)data,size,ret);

  if((debug>=2)&&(size>0))
    print_data(data,size);

  ecore_main_loop_begin();

  return ECORE_CONFIG_ERR_SUCC; }



/*****************************************************************************/



int send_append(char **mp,int *lp,char *dp) {
  size_t l=strlen(dp)+1+1+2+*lp,l2;
  char *m;

  if(!(m=realloc(*mp,l)))
    return ECORE_CONFIG_ERR_OOM;

  l2=*lp;
  *mp=m;
  *lp=l;
  m+=l2;
  l2=strlen(dp)+1;

  *(m++)='s';
  *(m++)=l2<<8;
  *(m++)=l2%256;
  strcpy(m,dp);

  return ECORE_CONFIG_ERR_SUCC; }



/*****************************************************************************/
