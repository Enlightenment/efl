
#define DEBUG 999

#ifdef ECORE_CONFIG_DEBUG
#  define D(fmt,args...) do { if(DEBUG>=0) fprintf(stderr,fmt,## args); } while(0);
#else
#  define D(msg,args...)
#endif
#define E(lvl,fmt,args...) do { if(DEBUG>=(lvl)) fprintf(stderr,fmt,## args); } while(0)

