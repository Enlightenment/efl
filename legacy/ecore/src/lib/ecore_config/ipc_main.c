/* ############## bad */
#define HAVE_EVAS2


#include "Ecore_Config.h"
#include "util.h"
#include "ipc.h"

#include "config.h"

#include <signal.h>
#include <dlfcn.h>
#include <stdio.h>
#include <glob.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>     /* malloc(), free() */

#ifndef TRUE
#  define FALSE 0
#  define TRUE (!FALSE)
#endif


typedef struct _ipc {
  void *lib;
  void *data;
  int (*ipc_init)(char *pipe_name,void **data);
  int (*ipc_exit)(void **data);
  int (*ipc_poll)(void **data);
  struct _ipc *next;
} ipc;



static ipc   *ipc_modules=NULL;
static unsigned long  ipc_timer=0L;


Ecore_Config_Server *srv2ecore_config_srv(void *srv) {
  ipc *ipc_tmp;
  Ecore_Config_Server *srv_tmp;
 
  ipc_tmp = ipc_modules;
  while (ipc_tmp) {
    srv_tmp = ipc_tmp->data;
    while (srv_tmp) {
      if (srv_tmp->server == srv)
        return srv_tmp;
      srv_tmp=srv_tmp->next;
    }
    ipc_tmp = ipc_tmp->next;
  }

  return NULL;
}

/*****************************************************************************/
/* INTERFACE FOR IPC MODULES */
/*****************************/



char *ipc_prop_list(Ecore_Config_Server *srv, const long serial) {
  Ecore_Config_Bundle *theme=ecore_config_bundle_get_by_serial(srv, serial);
  Ecore_Config_Prop   *e=theme?theme->data:NULL;
  estring       *s=estring_new(8192);
  int            f=0;

  while(e) {
    estring_appendf(s,"%s%s: %s",f?"\n":"",e->key,ecore_config_get_type(e));
    if(e->flags&PF_BOUNDS)
      if (e->type==PT_FLT)
        estring_appendf(s,", range %le..%le",(float)e->lo/ECORE_CONFIG_FLOAT_PRECISION,(float)e->hi/ECORE_CONFIG_FLOAT_PRECISION);
      else
        estring_appendf(s,", range %d..%d",e->lo,e->hi);
    f=1;
    e=e->next; }

  return estring_disown(s); }



char *ipc_prop_desc(Ecore_Config_Server *srv, const long serial,const char *key) {
#ifdef HAVE_EVAS2
  Ecore_Config_Bundle *theme=ecore_config_bundle_get_by_serial(srv, serial);
  Ecore_Config_Prop   *e=ecore_config_get(theme,key);

  if(e) {
    estring *s=estring_new(512);
    estring_appendf(s,"%s: %s",e->key,ecore_config_get_type(e));
    if(e->flags&PF_BOUNDS)
      estring_appendf(s,", range %d..%d",e->lo,e->hi);
    return estring_disown(s); }
#endif
  return strdup("<undefined>"); }



char *ipc_prop_get(Ecore_Config_Server *srv, const long serial,const char *key) {
#ifdef HAVE_EVAS2
  char          *ret=NULL;
  Ecore_Config_Bundle *theme=ecore_config_bundle_get_by_serial(srv, serial);
  if((ret=ecore_config_get_as_string(theme,key)))
    return ret;
#endif
  return strdup("<undefined>"); }



int ipc_prop_set(Ecore_Config_Server *srv, const long serial,const char *key,const char *val) {
#ifdef HAVE_EVAS2
  int ret;
  Ecore_Config_Bundle *theme=ecore_config_bundle_get_by_serial(srv, serial);
  ret=ecore_config_set(theme,key,(char *)val);
  E(1,"ipc.prop.set(%s->%s,\"%s\") => %d\n",theme->identifier,key,val,ret);
  return ret;
#else
  return ECORE_CONFIG_ERR_NOTSUPP;
#endif
}


/*****************************************************************************/


char *ipc_bundle_list(Ecore_Config_Server *srv) {
  Ecore_Config_Bundle *ns=ecore_config_bundle_get_1st(srv);
  estring       *s=estring_new(8192);
  int            f=0;

  if(!ns)
    return strdup("<no_bundles_created>");

  while(ns) {
    estring_appendf(s,"%s%d: %s",f?"\n":"",ecore_config_bundle_get_serial(ns),ecore_config_bundle_get_label(ns));
    f=1;
    ns=ecore_config_bundle_get_next(ns); }

  return estring_disown(s); }



int ipc_bundle_new(Ecore_Config_Server *srv, const char *label) {
  if (ecore_config_bundle_new(srv, label))
    return ECORE_CONFIG_ERR_SUCC;
  return ECORE_CONFIG_ERR_FAIL; }



char *ipc_bundle_label_get(Ecore_Config_Server *srv, const long serial) {
  Ecore_Config_Bundle *ns=ecore_config_bundle_get_by_serial(srv, serial);
  char          *label=ecore_config_bundle_get_label(ns);
  return strdup(label?label:"<no such bundle>"); }



int ipc_bundle_label_set(Ecore_Config_Server *srv, const long serial,const char *label) {
  Ecore_Config_Bundle *ns=ecore_config_bundle_get_by_serial(srv, serial);
  if (!(ns->identifier=malloc(sizeof(label))))
    return ECORE_CONFIG_ERR_OOM;
  memcpy(ns->identifier,label,sizeof(label));
  return ECORE_CONFIG_ERR_SUCC; }



long ipc_bundle_label_find(Ecore_Config_Server *srv, const char *label) {
  Ecore_Config_Bundle *ns=ecore_config_bundle_get_by_label(srv, label);
  return ns?ecore_config_bundle_get_serial(ns):-1; }




static int ipc_poll(void *data) {
  ipc *m=(ipc *)data;
  Ecore_Config_Server *s;

  while(m) {
    s = m->data;
    while (s) {
      m->ipc_poll(&s->server);
      s = s->next;
    }
    m=m->next; }

  return TRUE; }



int ipc_exit(void) {
  ipc *m;
  Ecore_Config_Server *l;
  
  if(ipc_timer)
    timeout_remove(ipc_timer);
  while(ipc_modules) {
    m=ipc_modules;
    ipc_modules=ipc_modules->next;
    l=m->data;
    while(l) {
      m->ipc_exit(&l->server);
      l=l->next;
    }
    free(m); }
  return ECORE_CONFIG_ERR_IGNORED; }



Ecore_Config_Server *ipc_init(char *pipe_name) {
  char             buf[PATH_MAX];
  glob_t           globbuf;
  int              ret;
  unsigned int     c;
  ipc              *nm=NULL;
  Ecore_Config_Server   *list=NULL;
  Ecore_Config_Server   *ret_srv=NULL;

  if (nm) {
    list=(Ecore_Config_Server *)nm->data;
    while (list) {
      if (!strcmp(list->name, pipe_name))
        return NULL;

      list = list->next;
    }
  }

  list=NULL;

  if (ipc_modules) {
    nm = ipc_modules;
    while (nm) {
      list=malloc(sizeof(Ecore_Config_Server));
      memset(list, 0, sizeof(Ecore_Config_Server));
      if((ret=nm->ipc_init(pipe_name,&list->server))!=ECORE_CONFIG_ERR_SUCC) {
        E(2,"ipc_init: failed to register %s, code %d\n", pipe_name, ret);
        break;
      }
      
      E(2,"ipc_init: registered \"%s\"...\n",pipe_name);
      
      list->name=strdup(pipe_name);
      list->next=nm->data;
        
      nm->data=list;
      if (ret_srv) ret_srv=list;
      nm = nm->next;
    }
  
    return ret_srv;
  }

  if(((ret=snprintf(buf,PATH_MAX,PACKAGE_LIB_DIR "/ecore_config_ipc_*.so"))<0)||
     (ret>=PATH_MAX))
    return NULL;

  glob(buf,0,NULL,&globbuf);
  if(!globbuf.gl_pathc)
    return NULL;

  for(c=0;c<globbuf.gl_pathc;c++) {
    if(!(nm=malloc(sizeof(ipc)))) {
      ret=ECORE_CONFIG_ERR_OOM;
      goto done; }
    memset(nm,0,sizeof(ipc));
    
    E(1,"ipc_init: checking \"%s\"...\n",globbuf.gl_pathv[c]);
    ret=dlmulti("IPC-plugin",globbuf.gl_pathv[c],RTLD_NOW,&nm->lib,
                "!ipc_init !ipc_exit !ipc_poll",
                &nm->ipc_init,&nm->ipc_exit,&nm->ipc_poll);
    if(ret==ECORE_CONFIG_ERR_NODATA)
      E(0,"ipc_init: could not load \"%s\": %s...\n",globbuf.gl_pathv[c],dlerror());
    else if(ret==ECORE_CONFIG_ERR_SUCC) {
      list=malloc(sizeof(Ecore_Config_Server));
/*      memcpy(list, 0, sizeof(Ecore_Config_Server));*/
      if((ret=nm->ipc_init(pipe_name,&list->server))!=ECORE_CONFIG_ERR_SUCC)
        E(0,"ipc_init: could not initialize \"%s\": %d\n",globbuf.gl_pathv[c],ret);
      else {
        char *p=globbuf.gl_pathv[c];
        if(DEBUG!=0) {
          char *q=strrchr(p,DIR_DELIMITER);
          if(q)
            p=++q; }
        E(0,"ipc_init: adding \"%s\"...\n",p);
        E(2,"ipc_init: registered \"%s\"...\n",pipe_name);
        
        list->name=strdup(pipe_name);
        list->next=nm->data;
        nm->data=list;
        if (!ret_srv) ret_srv=list;

        nm->next=ipc_modules;
        ipc_modules=nm; }}
    if(ret!=ECORE_CONFIG_ERR_SUCC)
      free(nm); }

 done:
  globfree(&globbuf);

  if(ipc_modules) {
    /* ### temporary evilness */
/*    if((debug>0)||(getenv("USER")&&!strcmp(getenv("USER"),"aje"))) {
      signal(SIGINT,SIG_DFL);
      signal(SIGSEGV,SIG_DFL); } */
    ipc_timer=timeout_add(100,ipc_poll,ipc_modules); }
  return ret_srv; }



/*****************************************************************************/
