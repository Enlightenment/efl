#ifndef _ECORE_CONFIG_PRIVATE_H
# define _ECORE_CONFIG_PRIVATE_H

/* debug */
int               DEBUG;
# define D(fmt,args...) do { if(DEBUG>=0) fprintf(stderr,fmt,## args); } while(0);
# define E(lvl,args...) do { if(DEBUG>=(lvl)) fprintf(stderr,## args); } while(0)






#endif
