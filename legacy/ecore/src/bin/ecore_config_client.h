typedef enum { OFFLINE,ONLINE } connstate;

typedef struct ex_ipc_server_list {
  Ecore_Ipc_Server          *srv;
  connstate                  state;
  struct ex_ipc_server_list *next; } ex_ipc_server_list;

/* in client */
int ex_ipc_server_con(void *data,int type,void *event);
int ex_ipc_server_dis(void *data,int type,void *event);
int ex_ipc_server_sent(void *data,int type,void *event);

/* in client lib */
const char *ecore_config_error(int no);
int ex_ipc_init(ex_ipc_server_list **srv_list,char *pipe_name,connstate *cs);
int ex_ipc_exit(ex_ipc_server_list **srv_list);
int ex_ipc_sigexit(void *data, int type, void *event);
int ex_ipc_send(ex_ipc_server_list **srv_list,int major,int minor,void *data,int size);
int send_append(char **mp,int *lp,char *dp);

extern int debug;

#ifndef E
#  define E(lvl,fmt,args...) do { if(debug>=(lvl)) fprintf(stderr,fmt,## args); } while(0)
#endif
