#ifndef _ELPUT_PRIVATE_H
# define _ELPUT_PRIVATE_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "Ecore.h"
# include "ecore_private.h"
# include "Ecore_Input.h"
# include "Eeze.h"
# include "Eldbus.h"
# include <Elput.h>

# include <unistd.h>
# include <linux/vt.h>
# include <linux/kd.h>
# include <linux/major.h>
# include <linux/input.h>
# include <libinput.h>

# ifdef HAVE_SYSTEMD
#  include <systemd/sd-login.h>
# endif

# ifdef ELPUT_DEFAULT_LOG_COLOR
#  undef ELPUT_DEFAULT_LOG_COLOR
# endif
# define ELPUT_DEFAULT_LOG_COLOR EINA_COLOR_GREEN

extern int _elput_log_dom;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_elput_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_elput_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_elput_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_elput_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_elput_log_dom, __VA_ARGS__)

typedef struct _Elput_Interface
{
   Eina_Bool (*connect)(Elput_Manager **manager, const char *seat, unsigned int tty, Eina_Bool sync);
   void (*disconnect)(Elput_Manager *manager);
   int (*open)(Elput_Manager *manager, const char *path, int flags);
   void (*close)(Elput_Manager *manager, int fd);
   int (*activate)(Elput_Manager *manager, int vt);
   void (*restore)(Elput_Manager *manager);
} Elput_Interface;

struct _Elput_Manager
{
   Elput_Interface *interface;

   int fd;
   char *sid;
   const char *seat;
   unsigned int vt_num;

   struct
     {
        char *path;
        Eldbus_Object *obj;
        Eldbus_Connection *conn;
     } dbus;

   Eina_Bool sync : 1;
};

extern Elput_Interface _logind_interface;

#endif
