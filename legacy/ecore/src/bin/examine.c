/* by Azundris */
/* Modified for ecore_config by HandyAndE */

#include "Ecore_Config.h"

#include <Ewl.h>
#include <Ecore.h>
#include <Ecore_Ipc.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ipc.h"
#include "ecore_config_client.h"



#define MI 4096


typedef enum {
  P_NONE=0,
  P_SERIAL=1,
  P_DIR=2,
  P_FILE=4,
  P_KEY=8,
  P_VALUE=16,
  P_SILENT=32,
  P_HELPONLY=64,
  P_LAST=P_HELPONLY } para;


typedef struct _call {
  ex_ipc_call  id;
  char        *name;
  para         signature; } call;



static call calls[]={
  { IPC_NONE,         "bundle",           P_HELPONLY},
  { IPC_BUNDLE_LIST,     "bundle-list",      P_NONE},
  { IPC_BUNDLE_NEW,      "bundle-new",       P_DIR},
  { IPC_BUNDLE_LABEL_GET,  "bundle-label-get",   P_SILENT|P_SERIAL},
  { IPC_BUNDLE_LABEL_GET,  "bundle-label",       P_SERIAL},
  { IPC_BUNDLE_LABEL_SET,  "bundle-label-set",   P_SILENT|P_SERIAL|P_DIR},
  { IPC_BUNDLE_LABEL_SET,  "bundle-label",       P_SILENT|P_HELPONLY},
  { IPC_BUNDLE_LABEL_FIND, "bundle-label-find",  P_SILENT|P_DIR},
  { IPC_BUNDLE_LABEL_FIND, "bundle-find",      P_DIR},

  { IPC_NONE,         "prop",          P_HELPONLY},
  { IPC_PROP_LIST,    "prop-list",     P_SERIAL},
  { IPC_PROP_DESC,    "prop-describe", P_SERIAL|P_KEY},
  { IPC_PROP_GET,     "prop-get",      P_SERIAL|P_KEY},
  { IPC_PROP_SET,     "prop-set",      P_SERIAL|P_KEY|P_VALUE},
  { IPC_PROP_LIST,    "list",          P_SILENT|P_SERIAL},
  { IPC_PROP_DESC,    "describe",      P_SILENT|P_SERIAL|P_KEY},
  { IPC_PROP_DESC,    "explain",       P_SILENT|P_SERIAL|P_KEY},
  { IPC_PROP_GET,     "print",         P_SILENT|P_SERIAL|P_KEY},
  { IPC_PROP_GET,     "get",           P_SILENT|P_SERIAL|P_KEY},
  { IPC_PROP_SET,     "set",           P_SILENT|P_SERIAL|P_KEY|P_VALUE},
  { IPC_PROP_SET,     "let",           P_SILENT|P_SERIAL|P_KEY|P_VALUE},

};



int debug=1;

void print_usage(void);

Ewl_Widget *main_win;
Ewl_Widget *main_box;

/*****************************************************************************/



int ex_ipc_server_con(void *data,int type,void *event) {
  connstate                  *cs=(connstate *)data;
  *cs=ONLINE;
  E(1,"ecore_config: Connected .\n"); /*to %s.\n", pipe_name)); */
  return 1; }



int ex_ipc_server_dis(void *data,int type,void *event) {
  connstate                  *cs=(connstate *)data;
  *cs=OFFLINE;
  ecore_main_loop_quit();
  E(1,"ecore_config: Disconnected.\n");
  return 1; }



int ex_ipc_server_sent(void *data,int type,void *event) {
  Ewl_Widget *row, *cell[2], *text[2];
  char *label, *type, *start, *end;
  Ecore_Ipc_Event_Server_Data *e;
  
  e=(Ecore_Ipc_Event_Server_Data *)event;
  E(2,"application sent: %2d.%02d  #%03d=>#%03d  \"%s\".%d\n",
    e->major,e->minor,0,0,(char *)e->data,e->size);
  if(e->data&&(e->size>0)) {
    printf("%s\n",(char *)e->data);

    row = ewl_grid_new(2, 1);
    cell[0] = ewl_cell_new();
    cell[1] = ewl_cell_new();
    text[0] = ewl_text_new("Property");
    text[1] = ewl_text_new("type");
    
    ewl_container_append_child(EWL_CONTAINER(cell[0]), text[0]);
    ewl_container_append_child(EWL_CONTAINER(cell[1]), text[1]);
    ewl_grid_add(EWL_GRID(row), cell[0], 1, 1, 1, 1);
    ewl_grid_add(EWL_GRID(row), cell[1], 2, 2, 1, 1);

    ewl_widget_show(cell[0]);
    ewl_widget_show(cell[1]);
    ewl_widget_show(text[0]);
    ewl_widget_show(text[1]);

    ewl_container_append_child(EWL_CONTAINER(main_box), row);
    ewl_widget_show(row);
    
    start = (char *)e->data;
    end = ((char *)e->data) + strlen((char *)e->data);
    
    while (*start) {
      label = start;
      while (*start) {
        if (*start == ':') {
          *start = '\0';
          break;
        }
        start++;
      }
      start++; 
      type = ++start;
      while (*start) {
        if (*start == '\n') {
          *start = '\0';
          break;
        }
        start++;
      }

      if (*label && *type) {
        row = ewl_grid_new(2, 1);
        cell[0] = ewl_cell_new();
        cell[1] = ewl_cell_new();
        text[0] = ewl_text_new(label);

        if (!strcmp(type, "string"))
          text[1] = ewl_entry_new("<a string>");
        else if (!strcmp(type, "integer")) 
          text[1] = ewl_spinner_new();
        else 
          text[1] = ewl_text_new(type);
        ewl_object_set_minimum_h(EWL_OBJECT(text[1]), 12);

        ewl_container_append_child(EWL_CONTAINER(cell[0]), text[0]);
        ewl_container_append_child(EWL_CONTAINER(cell[1]), text[1]);
        ewl_grid_add(EWL_GRID(row), cell[0], 1, 1, 1, 1);
        ewl_grid_add(EWL_GRID(row), cell[1], 2, 2, 1, 1);
             
        ewl_widget_show(cell[0]);
        ewl_widget_show(cell[1]);
        ewl_widget_show(text[0]);
        ewl_widget_show(text[1]);
        
        ewl_container_append_child(EWL_CONTAINER(main_box), row);
        ewl_widget_show(row);
        ewl_object_set_minimum_h(EWL_OBJECT(row), 20);
      }

      start++;
    }
  }
  else if(e->response<0)
    printf("error #%d\n",e->response);
  else if(!e->response)
    printf("OK\n");
  else
    printf("result: %d\n",e->response);
  ecore_main_loop_quit();
  return 1; }



/*****************************************************************************/
/* parser */



static long get_serial(char **m) {
  long ret=-1;
  if(m&&*m) {
    char *b;
    ret=strtol(*m,&b,10);
    if(b==*m)
      ret=0;
    else {
      while(isspace(*b))
        b++;
      *m=b; }}
  return ret; }



static int get_token(char **beg,char **end,int tol) {
  int     l;
  char   *b=*beg,*e;

  if(!b||!*b)
    return ECORE_CONFIG_ERR_NODATA;

  while(isspace(*b))
    b++;
  e=b;

  if(*e=='\"') {
    do {
      e++;
      if(*e=='\\')
        e++;
    } while(*e&&*e!='\"'); }
  else {
    while(*e&&!isspace(*e)) {
      if(tol&&isupper(*e))
        *e=tolower(*e);
      e++; }}

  if(*e) {
    *(e++)='\0';
    while(*e&&isspace(*e))
      *(e++)='\0';
    *end=e; }
  else
    *end=NULL;
  *beg=b;
  l=e-b;
  return (l<=0)?ECORE_CONFIG_ERR_NODATA:l; }



/*****************************************************************************/



static int handle_any(Ecore_Ipc_Server **server,call *cp,char *line) {
  long  serial=-1;
  int   ret,pars=0,l=0;
  char *b=line,*e,*file=NULL,*k=NULL,*v=NULL,*m=NULL;

  if(cp->signature&P_SERIAL)
    serial=get_serial(&b);

  if(cp->signature&(P_DIR|P_FILE)) {
    if((ret=get_token(&b,&e,FALSE))<0)
      return ret;
    file=b;
    pars++;
    b=e; }
    /*  b=e?e+1:NULL; */

  if(cp->signature&(P_KEY)) {
    if((ret=get_token(&b,&e,TRUE))<0)
      return ret;
    k=b;
    pars++;
    b=e; }
    /*  b=e?e+1:NULL; */
        
  if(cp->signature&(P_VALUE)) {
    if((ret=get_token(&b,&e,FALSE))<0)
      return ret;
    v=b;
    pars++;
    b=e; }
    /*  b=e?e+1:NULL; */
        
  if(file)
    send_append(&m,&l,file);
  if(k)
    send_append(&m,&l,k);
  if(v)
    send_append(&m,&l,v);

  E(3,"found \"%s\" |serial:%ld|file:%s|key:%s|value:%s|\n",cp->name,serial,file,k,v);

  ret=ex_ipc_send(server,cp->id,serial,m,l);
  if(m)
    free(m);
  return ret; }






/*****************************************************************************/



static int abbrevcmp(char *a,char *t) {
  size_t  l1;
  char   *p,*q;
  int     ret;

  while(1) {
    if(!(p=strchr(a,'-')))
      p=a+strlen(a);
    l1=p-a;
    if((ret=strncasecmp(a,t,l1)))
      return ret;

    a=p;
    if(*a)
      a++;

    if(!(q=strchr(t,'-')))
      t=t+strlen(t);
    else
      t=q+1;

    if(!*a)
      return *t?-1:0;
    else if(!*t)
      return 1; }}



static call *find_call(char *b) {
  int          nc=sizeof(calls)/sizeof(call);
  call        *cp=calls,*r=NULL;
  while(nc-->0) {
    if(!abbrevcmp(b,cp->name)) {
      if(r) {
        printf("\"%s\" is not unique.\n",b);
        return NULL; }
      r=cp; }
    cp++; }
  return r; }





/*****************************************************************************/

void  __destroy_main_window(Ewl_Widget *main_win, void *ex_data, void *user_data) {
  ewl_widget_destroy(main_win);
  ewl_main_quit();

  return;
}
        

int main(int argc,char **argv) {
  Ecore_Ipc_Server *server=NULL;
  int               ret=ECORE_CONFIG_ERR_SUCC,cc=0;
  connstate         cs=OFFLINE;
  char             *p,*f=NULL;
  char             *pipe_name=NULL;
  char              buf[MI];

  if (argc <= 1  || (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0))  {
    print_usage();
    return 0;
  }

  pipe_name=argv[1];
  printf("ecore_configr: connecting to %s.\n", pipe_name);
  
  ecore_init();
  ecore_app_args_set(argc,(const char **)argv);
  ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT,ex_ipc_sigexit,&cs);

  ewl_init(&argc, argv);

  main_win = ewl_window_new();
  ewl_window_set_title(EWL_WINDOW(main_win), "Examine Configuration Client");
  ewl_object_set_minimum_size(EWL_OBJECT(main_win), 200, 100);
  ewl_callback_append(main_win, EWL_CALLBACK_DELETE_WINDOW, __destroy_main_window, NULL);

  main_box = ewl_vbox_new();
  ewl_container_append_child(EWL_CONTAINER(main_win), main_box);
  ewl_object_set_padding(EWL_OBJECT(main_box), 2, 2, 2, 2);
  ewl_widget_show(main_box);
 

reconnect:
  cc++;
  if((ret=ex_ipc_init(&server,pipe_name,&cs))!=ECORE_CONFIG_ERR_SUCC)
    E(0,"exsh: %sconnect to %s failed: %d\n",(cc>1)?"re":"",pipe_name,ret);
  else {

    call *c=find_call("prop-list");
    handle_any(&server, c, "");
    ewl_widget_show(main_win);
    ewl_main();
    
  }

  ex_ipc_exit(&server);
  ecore_shutdown();

  return ret; }

void print_usage(void) {
  printf("Examine - ecore_config Graphical Configuration Client\n");
  printf("Version 0.0.1 (Nov 3 2003)\n");
  printf("(c)2002-2003 by Azundris and HandyAndE.\n");
  printf("Usage: examine [options] target\n\n");
  printf("Supported Options:\n");
  printf("-h, --help        Print this help text\n");

}

/*****************************************************************************/
