#include "Ecore_Config.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <util.h>

Ecore_Config_Server *__ecore_config_server_global = NULL;
Ecore_Config_Server *__ecore_config_server_local = NULL;
Ecore_Config_Bundle *__ecore_config_bundle_local = NULL;
char                *__ecore_config_app_name = NULL;

Ecore_Config_Server *_ecore_config_ipc_init(char *name);
int _ecore_config_ipc_exit(void);

static char *_ecore_config_type[]={ "undefined", "integer", "float", "string", "colour" };

Ecore_Config_Prop *ecore_config_dst(Ecore_Config_Prop *e) {
  Ecore_Config_Bundle        *t;
  Ecore_Config_Prop          *p,*c;
  Ecore_Config_Listener_List *l;
  p=NULL;
  c=e;
  t = __ecore_config_bundle_local;

  if(!e||!e->key)
    return NULL;
  if(t) {
    if(t->data==e)
      t->data=e->next;
    else {
      do {
        p=c;
        c=c->next;
      } while(c&&(c!=e));
      if(c)    /* remove from list if even in there */
        p->next=c->next; }}

  while(e->listeners) {
    l=e->listeners;
    e->listeners=e->listeners->next;
    free(l); }

  if(e->key)
    free(e->key);
  if(e->ptr&&(e->type==PT_STR))
    free(e->ptr);

  memset(e,0,sizeof(Ecore_Config_Prop));
  free(e);

  return NULL; }

Ecore_Config_Prop *ecore_config_get(Ecore_Config_Bundle *t, const char *key) {
  Ecore_Config_Prop *e;
  if(!t||!key)
    return NULL;
  e=t->data;
  while(e) {
    if(!strcmp(key,e->key))
      return e;
    e=e->next; }
  return NULL; }



const char *ecore_config_get_type(const Ecore_Config_Prop *e) {
  if(e) {
    return _ecore_config_type[e->type]; }
  return "not found"; }


void *ecore_config_get_data(const char *key) {
  Ecore_Config_Prop *e;
  e=ecore_config_get(__ecore_config_bundle_local,key);
  return (e?((e->type==PT_STR)?((void *)&e->ptr):((void *)&e->val))
           :NULL); }


char *ecore_config_get_string(const char *key) {
  Ecore_Config_Prop *e;
  e=ecore_config_get(__ecore_config_bundle_local,key);
  return (e&&(e->type==PT_STR))?e->ptr:NULL; }


long ecore_config_get_int(const char *key) {
  Ecore_Config_Prop *e;
  e=ecore_config_get(__ecore_config_bundle_local,key);
  return (e&&((e->type==PT_INT)||(e->type==PT_RGB)))?e->val:0L; }


float ecore_config_get_float(const char *key) {
  Ecore_Config_Prop *e;
  e=ecore_config_get(__ecore_config_bundle_local,key);
  return (e&&(e->type==PT_FLT))?((float)e->val/ECORE_CONFIG_FLOAT_PRECISION):0.0; }

int ecore_config_get_rgb(const char *key,int *r, int *g, int *b) {
  Ecore_Config_Prop *e;
  e=ecore_config_get(__ecore_config_bundle_local,key);

  if(e&&((e->type==PT_RGB))) {
    *r=(e->val>>16)&0xff;
    *g=(e->val>>8)&0xff;
    *b=e->val&0xff;
    return ECORE_CONFIG_ERR_SUCC; }
  return ECORE_CONFIG_ERR_FAIL; }

char *ecore_config_get_rgbstr(const char *key) {
  char          *r;
  r=NULL;
  esprintf(&r,"#%06x",ecore_config_get_int(key));
  return r; }

char *ecore_config_get_as_string(const char *key) {
  Ecore_Config_Bundle *t;
  Ecore_Config_Prop   *e;
  char          *r;
  r=NULL;
  t = __ecore_config_bundle_local;
  if(!(e=ecore_config_get(t,key)))
    E(0,"no such property, \"%s\"...\n",key);
  else {
    const char *type=ecore_config_get_type(e);
    switch(e->type) {
      case PT_NIL:
        esprintf(&r,"%s:%s=<nil>",key,type); break;
      case PT_INT:
        esprintf(&r,"%s:%s=%ld",key,type,ecore_config_get_int(key)); break;
      case PT_FLT:
        esprintf(&r,"%s:%s=%lf",key,type,ecore_config_get_float(key)); break;
      case PT_STR:
        esprintf(&r,"%s:%s=\"%s\"",key,type,ecore_config_get_string(key)); break;
      case PT_RGB:
        esprintf(&r,"%s:%s=#%06x",key,type,ecore_config_get_int(key)); break;
      default:
        esprintf(&r,"%s:unknown_type",key); break; }}
  return r; }


static int ecore_config_bound(Ecore_Config_Prop *e) {
  int  ret;
  long v;
  ret=ECORE_CONFIG_ERR_SUCC;

  if(!e)
    return ECORE_CONFIG_ERR_FAIL;
  if(e->flags&PF_BOUNDS) {
    if((e->val<e->lo)) {
      E(0,"ecore_config_bounds(\"%s\",%ld): value out of range; adjusted to %ld...\n",e->key,e->val,e->lo);
      e->val=e->lo; }
    else if((e->val>e->hi)) {
      E(0,"ecore_config_bounds(\"%s\",%ld): value out of range; adjusted to %ld...\n",e->key,e->val,e->hi);
      e->val=e->hi; }
    else
      ret=ECORE_CONFIG_ERR_IGNORED; }
  else
    ret=ECORE_CONFIG_ERR_IGNORED;

  if(e->step) {
    v=((int)(e->val/e->step))*e->step;
    if(v!=e->val) {
      if(e->type==PT_FLT)
        E(0,"ecore_config_bound(\"%s\"): float value %f not a multiple of %f, adjusted to %f...\n",
          e->key,((double)e->val)/ECORE_CONFIG_FLOAT_PRECISION,((double)e->step)/ECORE_CONFIG_FLOAT_PRECISION,((double)v)/ECORE_CONFIG_FLOAT_PRECISION);
      else
        E(0,"ecore_config_bound(\"%s\"): integer value %ld not a multiple of %ld, adjusted to %ld...\n",
          e->key,e->val,e->step,v);
      ret=ECORE_CONFIG_ERR_SUCC;
      e->val=v; }}

  return ret; }

int ecore_config_guess_type(char *val) {
  char *l;
  long v;
  l=NULL;

  if(!val)
    return PT_NIL;
  if (val[0]=='#')
    return PT_RGB;
  v=strtol(val,&l,10);
  if(*l) {
    float f;
    if (sscanf(val,"%f%*s",&f)!=1)
      return PT_STR;
    return PT_FLT;
    }
  return PT_INT;
  }

static int ecore_config_val_typed(Ecore_Config_Prop *e,void *val,int type) {
  char          *l;
  long           v;
  float         *f;

  l=NULL;
  v=0;
  e->type=PT_NIL;

  if(!val)
    e->ptr=NULL;
  else {
    if (type==PT_INT) {
      e->val=(int)val;
      e->type=PT_INT;
    } else if (type==PT_STR) {
      if(!(e->ptr=strdup(val)))
        return ECORE_CONFIG_ERR_OOM;
      e->type=PT_STR;
    } else if (type==PT_RGB) {
      if (((char *)val)[0]=='#') {
        if((v=strtol(&((char *)val)[1],&l,16))<0) {
          v=0;
          E(0,"ecore_config_val: key \"%s\" -- hexadecimal value less than zero, bound to zero...\n", (char *)val);
          l=(char *)val;
        }
      } else {
        E(0,"ecore_config_val: key \"%s\" -- value \"%s\" not a valid hexadecimal RGB value?\n",e->key,(char *)val);
        return ECORE_CONFIG_ERR_FAIL;
      }
      if(*l)
        E(0,"ecore_config_val: key \"%s\" -- value \"%s\" not a valid hexadecimal RGB value?\n",e->key,(char *)val);
      else {
        e->val=v;
        e->type=PT_RGB;
      }
    } else if (type==PT_FLT) {
      f = (float *)val;
      e->val=(long)((*f)*ECORE_CONFIG_FLOAT_PRECISION);
      e->type=PT_FLT;
    }

    ecore_config_bound(e);
    e->flags|=PF_MODIFIED;
    return ECORE_CONFIG_ERR_SUCC;
  } 
  return ECORE_CONFIG_ERR_IGNORED;
}

static int ecore_config_add_typed(Ecore_Config_Bundle *t, const char *key, void* val, int type) {
  Ecore_Config_Prop *e;

  if (!key)
    return ECORE_CONFIG_ERR_NODATA;

  if(!(e=malloc(sizeof(Ecore_Config_Prop))))
    goto ret;
  memset(e,0,sizeof(Ecore_Config_Prop));

  if(!(e->key=strdup(key)))
    goto ret_free_nte;

  if(!val)
    e->type=PT_NIL;
  else if(ecore_config_val_typed(e,val,type)==ECORE_CONFIG_ERR_OOM)
    goto ret_free_key;

  e->next=t?t->data:NULL;
  if(t)     t->data=e;

  return ECORE_CONFIG_ERR_SUCC;

  ret_free_key:
  free(e->key);
  ret_free_nte:
  free(e);
  ret:
  return ECORE_CONFIG_ERR_OOM; }
                                         

static int ecore_config_add(Ecore_Config_Bundle *t,const char *key,char *val) {
  int type;
  type=ecore_config_guess_type(val);
  return ecore_config_add_typed(t,key,val,type); }
    
int ecore_config_set_typed(Ecore_Config_Bundle *t,const char *key,void *val,int type) {
  Ecore_Config_Prop *e;
  Ecore_Config_Listener_List *l;
  int ret;

  if (!t||!key)
    return ECORE_CONFIG_ERR_NODATA;

  if (!(e=ecore_config_get(t,key)))
    return ecore_config_add_typed(t,key,val,type);

  if ((ret=ecore_config_val_typed(e,val,type))==ECORE_CONFIG_ERR_SUCC) {
    for(l=e->listeners;l;l=l->next)
      l->listener(e->key,e->type,l->tag,l->data,t);
    }
  else {
    E(0,"ecore_config_set_typed(\"%s\"): ecore_config_val_typed() failed: %d\n",key,ret);
    }

  return ret;
  }

int ecore_config_set(Ecore_Config_Bundle *t,const char *key,char *val) {
  int type;
  type=ecore_config_guess_type(val);
  if (type == PT_INT)
    return ecore_config_set_typed(t,key,(void*) atoi(val),type);
  else
    return ecore_config_set_typed(t,key,(void*) val,type); }

int ecore_config_set_as_string(const char *key,char *val) {
  return ecore_config_set(__ecore_config_bundle_local,key,val); }

int ecore_config_set_int(const char *key, int val) {
  return ecore_config_set_typed(__ecore_config_bundle_local,key,(void *)val,PT_INT); }

int ecore_config_set_string(const char *key, char* val) {
  return ecore_config_set_typed(__ecore_config_bundle_local,key,(void *)val,PT_STR); }

int ecore_config_set_float(const char *key, float val) {
  return ecore_config_set_typed(__ecore_config_bundle_local,key,(void *)&val,PT_FLT); }

int ecore_config_set_rgb(const char *key, char* val) {
  return ecore_config_set_typed(__ecore_config_bundle_local,key,(void *)val,PT_RGB); }


static int ecore_config_default_typed(Ecore_Config_Bundle *t,const char *key,void *val,int type) {
  int            ret;
  Ecore_Config_Prop *e;
  ret=ECORE_CONFIG_ERR_SUCC;

  if(!(e=ecore_config_get(t,key))) {                  /* prop doesn't exist yet */
    if((ret=ecore_config_add_typed(t,key,val,type))!=ECORE_CONFIG_ERR_SUCC)  /* try to add it */
      return ret;                                  /* ...failed */
    if(!(e=ecore_config_get(t,key)))                  /* get handle */
      return ECORE_CONFIG_ERR_FAIL;
    e->flags=e->flags&~PF_MODIFIED; }

  return ret; }

int ecore_config_default(const char *key,char *val,float lo,float hi,float step) {
  int ret, type;
  Ecore_Config_Bundle *t;
  Ecore_Config_Prop *e;
  t = __ecore_config_bundle_local;
          
  type=ecore_config_guess_type(val);
  ret=ecore_config_default_typed(t, key, val, type);
  e=ecore_config_get(t,key);
  if (e) {
    if (type==PT_INT) {
      e->step=step;
      e->flags|=PF_BOUNDS;
      e->lo=lo;
      e->hi=hi;
      ecore_config_bound(e);
    } else if (type==PT_FLT) {
      e->step=(int)(step*ECORE_CONFIG_FLOAT_PRECISION);
      e->flags|=PF_BOUNDS;
      e->lo=(int)(lo*ECORE_CONFIG_FLOAT_PRECISION);
      e->hi=(int)(hi*ECORE_CONFIG_FLOAT_PRECISION);
      ecore_config_bound(e);
    }
  }

  return ret;
}
    
int ecore_config_default_int(const char *key,int val) {
  return  ecore_config_default_typed(__ecore_config_bundle_local, key, (void *) val, PT_INT);
}

int ecore_config_default_int_bound(const char *key,int val,int low,int high,int step) {
  Ecore_Config_Bundle *t;
  Ecore_Config_Prop *e;
  int                ret;
  t = __ecore_config_bundle_local;
  
  ret=ecore_config_default_typed(t, key, (void *) val, PT_INT);
  e=ecore_config_get(t,key);
  if (e) {
    e->step=step;
    e->flags|=PF_BOUNDS;
    e->lo=low;
    e->hi=high; 
    ecore_config_bound(e);
  }
  
  return ret;
}

int ecore_config_default_string(const char *key,char *val) {
  return ecore_config_default_typed(__ecore_config_bundle_local, key, (void *) val, PT_STR);
}

int ecore_config_default_float(const char *key,float val){
  return ecore_config_default_typed(__ecore_config_bundle_local, key, (void *) &val, PT_FLT);
}
   
int ecore_config_default_float_bound(const char *key,float val,float low,float high,float step) {
  Ecore_Config_Prop *e;
  int                ret;
      
  ret=ecore_config_default_typed(__ecore_config_bundle_local, key, (void *) &val, PT_FLT);
  e=ecore_config_get(__ecore_config_bundle_local,key);
  if (e) {                  
    e->step=(int)(step*ECORE_CONFIG_FLOAT_PRECISION);
    e->flags|=PF_BOUNDS;
    e->lo=(int)(low*ECORE_CONFIG_FLOAT_PRECISION);
    e->hi=(int)(high*ECORE_CONFIG_FLOAT_PRECISION);
    ecore_config_bound(e);
  }
                            
  return ret;
}

int ecore_config_default_rgb(const char *key,char *val) {
  return ecore_config_default_typed(__ecore_config_bundle_local, key, (void *) val, PT_RGB);
}

int ecore_config_listen(const char *name,const char *key,
                        Ecore_Config_Listener listener,int tag,void *data) {
  Ecore_Config_Bundle        *t;
  Ecore_Config_Prop          *e;
  Ecore_Config_Listener_List *l;
  t = __ecore_config_bundle_local;

  if(!t||!key)
    return ECORE_CONFIG_ERR_NODATA;

  if(!(e=ecore_config_get(t,key))) {
    int ret=ecore_config_add(t,key,"");
    if(ret!=ECORE_CONFIG_ERR_SUCC) {
      E(0,"ecore_config_listen: ecore_config_add(\"%s\") failed: %d\n",key,ret);
      return ret; }
    if(!(e=ecore_config_get(t,key))) {
      E(0,"ecore_config_listen: list of properties corrupted!?\n");
      return ECORE_CONFIG_ERR_FAIL; }}

  for(l=e->listeners;l;l=l->next)
    if(!strcmp(l->name,name)||(l->listener==listener)) {
      E(1,"ecore_config_listen: %s is already listening for changes of %s in %s...\n",
        name,key,t->identifier?t->identifier:"");
      return ECORE_CONFIG_ERR_IGNORED; }

  if(!(l=malloc(sizeof(Ecore_Config_Listener_List))))
    return ECORE_CONFIG_ERR_OOM;

  E(1,"registering listener \"%s\" for \"%s\" (%d)...\n",name,key,e->type);

  memset(l,0,sizeof(Ecore_Config_Listener_List));

  l->listener =listener;
  l->name     =name;
  l->data     =data;
  l->tag      =tag;
  l->next     =e->listeners;
  e->listeners=l;

  if(e->type!=PT_NIL)     /* call right on creation if prop exists and has val */
    listener(key,e->type,tag,data,t);

  return ECORE_CONFIG_ERR_SUCC; }

int ecore_config_deaf(const char *name,const char *key, 
                      Ecore_Config_Listener listener) {
  Ecore_Config_Prop          *e;
  Ecore_Config_Listener_List *l,*p;
  int                   ret;
  ret=ECORE_CONFIG_ERR_NOTFOUND;

  if(!__ecore_config_bundle_local||!key)
    return ECORE_CONFIG_ERR_NODATA;

  if(!(e=ecore_config_get(__ecore_config_bundle_local,key)))
    return ECORE_CONFIG_ERR_NOTFOUND;

  for(p=NULL,l=e->listeners;l;p=l,l=l->next) {
    if((name&&!strcmp(l->name,name))||(l->listener==listener)) {
      ret=ECORE_CONFIG_ERR_SUCC;
      if(!p)
        e->listeners=e->listeners->next;
      else
        p->next=l->next;
      memset(l,0,sizeof(Ecore_Config_Listener));
      free(l); }}

  return ret; }

Ecore_Config_Bundle *ecore_config_bundle_get_1st(Ecore_Config_Server *srv) {  /* anchor: global, but read-only */
  return srv->bundles; }

Ecore_Config_Bundle *ecore_config_bundle_get_next(Ecore_Config_Bundle *ns) {
  return ns?ns->next:NULL; }

Ecore_Config_Bundle *ecore_config_bundle_get_by_serial(Ecore_Config_Server *srv, long serial) {
  Ecore_Config_Bundle *eb;
  eb=srv->bundles;

  if(serial<0)
    return NULL;
  else if(serial==0) {
    Ecore_Config_Bundle *r=eb;
    return r; }

  while(eb) {
    if(eb->serial==serial)
      return eb;
    eb=eb->next; }
  return NULL; }

Ecore_Config_Bundle *ecore_config_bundle_get_by_label(Ecore_Config_Server *srv, const char *label) {
  Ecore_Config_Bundle *ns;
  ns=srv->bundles;

  while(ns) {
    if(ns->identifier&&!strcmp(ns->identifier,label))
      return ns;
    ns=ns->next; }
  return NULL; }
                    

long ecore_config_bundle_get_serial(Ecore_Config_Bundle *ns) {
  return ns?ns->serial:-1; }
  
char *ecore_config_bundle_get_label(Ecore_Config_Bundle *ns) {
  return ns?ns->identifier:NULL; }


Ecore_Config_Bundle *ecore_config_bundle_new(Ecore_Config_Server *srv,const char *identifier) {
  Ecore_Config_Bundle *t;
  static long  ss;
  ss=0; /* bundle unique serial */

  if((t=malloc(sizeof(Ecore_Config_Bundle)))) {
    memset(t,0,sizeof(Ecore_Config_Bundle));

    t->identifier=identifier;
    t->serial=++ss;
    t->owner=srv->name;
    t->next=srv->bundles;
    srv->bundles=t;
  }
  return t; }

Ecore_Config_Server *do_init(char *name) {
  return _ecore_config_ipc_init(name);
}

Ecore_Config_Server *ecore_config_init_local(char *name) {
  char *p;
  char *buf;

  if((p=getenv("HOME"))) {  /* debug-only ### FIXME */
    if (!(buf=malloc(PATH_MAX*sizeof(char))))
      return NULL;
    snprintf(buf,PATH_MAX,"%s/.ecore/%s/.global",p,name);
    unlink(buf);

    free(buf);
  }
                                
  return do_init(name);
}

Ecore_Config_Server *ecore_config_init_global(char *name) {
  char *p;
  int global;
  char *buf;
  global=0;
  
  if((p=getenv("HOME"))) {  /* debug-only ### FIXME */
    if (!(buf=malloc(PATH_MAX*sizeof(char))))
      return NULL;
    snprintf(buf,PATH_MAX,"%s/.ecore/%s/.global",p,name);
    global = creat(buf, S_IRWXU);
    if (global)
      close(global);
    free(buf);
  }
  
  return do_init(name);
}

int ecore_config_init(char *name) {
  char                *buf, *p;
  Ecore_Config_Prop   *sys;
  
  __ecore_config_app_name = strdup(name);
  __ecore_config_server_local = ecore_config_init_local(name);
  if (!__ecore_config_server_local) 
    return ECORE_CONFIG_ERR_FAIL;

  __ecore_config_server_global = ecore_config_init_global(ECORE_CONFIG_GLOBAL_ID);
  if (!__ecore_config_server_global)
    return ECORE_CONFIG_ERR_FAIL;

  __ecore_config_bundle_local = ecore_config_bundle_new(__ecore_config_server_local, "config");

  if((p=getenv("HOME"))) {  /* debug-only ### FIXME */
    if ((buf=malloc(PATH_MAX*sizeof(char)))) {
      snprintf(buf,PATH_MAX,"%s/.e/config.db",p);
      if (ecore_config_load_file(buf) != 0)
        if (ecore_config_load_file(PACKAGE_DATA_DIR "/system.db") != 0)
          return ECORE_CONFIG_ERR_NOFILE;
      sys = __ecore_config_bundle_local->data;
      while (sys) {
        /* unmark it modified - modification will mean it has been overridden */
        sys->flags&=~PF_MODIFIED;
        /* mark as system so that examine can hide them */
        sys->flags&=PF_SYSTEM;
        sys=sys->next;
      }
    }
    free(buf);
  }

  return ECORE_CONFIG_ERR_SUCC;
}

int ecore_config_exit(void) {
  int ret;
  ret = _ecore_config_ipc_exit();
  free(__ecore_config_app_name);
  free(__ecore_config_bundle_local);
  free(__ecore_config_server_local);
  free(__ecore_config_server_global);
  return ret;
}

