#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eet.h>
#include <Eeze.h>
#include <Ecore_Con.h>
#include <Eeze_Disk.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "eeze_scanner.h"

#define DBG(...)            EINA_LOG_DOM_DBG(es_log_dom, __VA_ARGS__)
#define INF(...)            EINA_LOG_DOM_INFO(es_log_dom, __VA_ARGS__)
#define WRN(...)            EINA_LOG_DOM_WARN(es_log_dom, __VA_ARGS__)
#define ERR(...)            EINA_LOG_DOM_ERR(es_log_dom, __VA_ARGS__)
#define CRI(...)            EINA_LOG_DOM_CRIT(es_log_dom, __VA_ARGS__)

static int es_log_dom = -1;
static Ecore_Con_Server *svr = NULL;
static Eet_Data_Descriptor *es_edd = NULL;
static Eina_Hash *clients = NULL;

static Eina_List *storage_devices = NULL;
static Eina_List *storage_cdrom = NULL;

static Eina_List *volume_cdrom = NULL;
static Eina_List *volume_devices = NULL;

static void
event_send(const char *device, Eeze_Scanner_Event_Type type, Eina_Bool volume)
{
   Eeze_Scanner_Event ev;
   const Eina_List *l;
   Ecore_Con_Client *cl;

   ev.device = device;
   ev.type = type;
   ev.volume = volume;
   EINA_LIST_FOREACH(ecore_con_server_clients_get(svr), l, cl)
     {
        Eet_Connection *ec;

        ec = eina_hash_find(clients, &cl);
        if (!ec) continue;
        INF("Serializing event...");
        eet_connection_send(ec, es_edd, &ev, NULL);
     }
}

static Eina_Bool
event_write(const void *data, size_t size, Ecore_Con_Client *cl)
{
   INF("Event sent!");
   ecore_con_client_send(cl, data, size);
   return EINA_TRUE;
}

static Eina_Bool
disk_mount(void *data EINA_UNUSED, int type EINA_UNUSED, Eeze_Disk *disk)
{
   Eina_List *l;
   Eeze_Scanner_Device *d;
   if (eeze_disk_type_get(disk) != EEZE_DISK_TYPE_CDROM) return ECORE_CALLBACK_RENEW;

   EINA_LIST_FOREACH(storage_cdrom, l, d)
     {
        if (d->device == eeze_disk_syspath_get(disk))
          {
             d->mounted = !d->mounted;
             break;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static void
cl_setup(Ecore_Con_Client *cl EINA_UNUSED, Eet_Connection *ec)
{
   Eina_List *l;
   Eeze_Scanner_Device *dev;
   Eeze_Scanner_Event ev;
   const char *sys;

   INF("Sending initial events to new client");
   EINA_LIST_FOREACH(storage_devices, l, sys)
     {
        ev.device = sys;
        ev.type = EEZE_SCANNER_EVENT_TYPE_ADD;
        ev.volume = EINA_FALSE;
        eet_connection_send(ec, es_edd, &ev, NULL);
     }
   EINA_LIST_FOREACH(storage_cdrom, l, dev)
     {
        ev.device = dev->device;
        ev.type = EEZE_SCANNER_EVENT_TYPE_ADD;
        ev.volume = EINA_FALSE;
        eet_connection_send(ec, es_edd, &ev, NULL);
     }
   EINA_LIST_FOREACH(volume_devices, l, sys)
     {
        ev.device = sys;
        ev.type = EEZE_SCANNER_EVENT_TYPE_ADD;
        ev.volume = EINA_TRUE;
        eet_connection_send(ec, es_edd, &ev, NULL);
     }
   EINA_LIST_FOREACH(volume_cdrom, l, dev)
     {
        ev.device = dev->device;
        ev.type = EEZE_SCANNER_EVENT_TYPE_ADD;
        ev.volume = EINA_TRUE;
        eet_connection_send(ec, es_edd, &ev, NULL);
     }
}

static Eina_Bool
es_read(const void *eet_data EINA_UNUSED, size_t size EINA_UNUSED, void *user_data EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
cl_add(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Add *ev)
{
   Eet_Connection *ec;
   INF("Added client");

   ec = eet_connection_new(es_read, (Eet_Write_Cb*)event_write, ev->client);
   if (!ec)
     {
        ERR("Could not create eet serializer! Lost client!");
        ecore_con_client_del(ev->client);
        return ECORE_CALLBACK_RENEW;
     }

   eina_hash_add(clients, &ev->client, ec);
   cl_setup(ev->client, ec);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
cl_del(void *data EINA_UNUSED, int type EINA_UNUSED, Ecore_Con_Event_Client_Del *ev)
{
   Eet_Connection *ec;
   Eina_Bool d;
   INF("Removed client");
   ec = eina_hash_find(clients, &ev->client);
   if (ec)
     {
        eina_hash_del_by_data(clients, &ec);
        eet_connection_close(ec, &d);
     }

   return ECORE_CALLBACK_RENEW;
}

static void
eet_setup(void)
{
   Eet_Data_Descriptor_Class eddc;

   if (!eet_eina_stream_data_descriptor_class_set(&eddc, sizeof(eddc), "eeze_scanner_event", sizeof(Eeze_Scanner_Event)))
     {
        CRI("Could not create eet data descriptor!");
        exit(1);
     }

   es_edd = eet_data_descriptor_stream_new(&eddc);
#define DAT(MEMBER, TYPE) EET_DATA_DESCRIPTOR_ADD_BASIC(es_edd, Eeze_Scanner_Event, #MEMBER, MEMBER, EET_T_##TYPE)
   DAT(device, INLINED_STRING);
   DAT(type, UINT);
   DAT(volume, UCHAR);
#undef DAT
}

static Eina_Bool
cdrom_timer(Eeze_Scanner_Device *dev)
{
   const char *devpath;
   int fd;

   /* cdrom already mounted, no need to poll */
   if (dev->mounted) return EINA_TRUE;
   devpath = eeze_udev_syspath_get_devpath(dev->device);
   fd = open(devpath, O_RDONLY);
   if (fd < 0)
     {
        Eina_List *l;

        l = eina_list_data_find_list(volume_cdrom, dev);
        if (l)
          {
             /* disc removed, delete volume */
             INF("Removed cdrom '%s'", dev->device);
             volume_cdrom = eina_list_remove_list(volume_cdrom, l);
             event_send(dev->device, EEZE_SCANNER_EVENT_TYPE_CHANGE, EINA_TRUE);
          }
        /* just in case */
        dev->mounted = EINA_FALSE;
     }
   else
     {
        if (!eina_list_data_find(volume_cdrom, dev))
          {
             INF("Added cdrom '%s'", dev->device);
             volume_cdrom = eina_list_append(volume_cdrom, dev);
             event_send(dev->device, EEZE_SCANNER_EVENT_TYPE_CHANGE, EINA_TRUE);
          }
        close(fd);
     }
   eina_stringshare_del(devpath);
   return EINA_TRUE;
}

static void
storage_setup(void)
{
   Eina_List *l, *ll;
   const char *sys;

   storage_devices = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_INTERNAL, NULL);
   if (!storage_devices)
     {
        ERR("No storage devices found! This is not supposed to happen!");
        exit(1);
     }
   EINA_LIST_FOREACH(storage_devices, l, sys)
     event_send(sys, EEZE_SCANNER_EVENT_TYPE_ADD, EINA_FALSE);

   ll = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_REMOVABLE, NULL);
   EINA_LIST_FREE(ll, sys)
     {
        event_send(sys, EEZE_SCANNER_EVENT_TYPE_ADD, EINA_FALSE);
        storage_devices = eina_list_append(storage_devices, sys);
     }

   l = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_CDROM, NULL);
   EINA_LIST_FREE(l, sys)
     {
        Eeze_Scanner_Device *dev;
        Eeze_Disk *disk;

        dev = calloc(1, sizeof(Eeze_Scanner_Device));
        if (!dev)
          {
             ERR("Lost cdrom device '%s'!", sys);
             eina_stringshare_del(sys);
             continue;
          }
        disk = eeze_disk_new(sys);
        if (!disk)
          {
             ERR("Lost cdrom device '%s'!", sys);
             eina_stringshare_del(sys);
             free(dev);
             continue;
          }
        dev->device = sys;
        dev->mounted = eeze_disk_mounted_get(disk);
        eeze_disk_free(disk);
        event_send(sys, EEZE_SCANNER_EVENT_TYPE_ADD, EINA_FALSE);
        dev->poller = ecore_poller_add(ECORE_POLLER_CORE, 32, (Ecore_Task_Cb)cdrom_timer, dev);
        storage_cdrom = eina_list_append(storage_cdrom, dev);
     }
   volume_devices = eeze_udev_find_by_type(EEZE_UDEV_TYPE_DRIVE_MOUNTABLE, NULL);
   EINA_LIST_FOREACH_SAFE(volume_devices, l, ll, sys)
     {
        Eina_List *c;
        Eeze_Scanner_Device *dev;

        EINA_LIST_FOREACH(storage_cdrom, c, dev)
          if (sys == dev->device)
            {
               eina_stringshare_del(sys);
               volume_devices = eina_list_remove_list(volume_devices, l);
               volume_cdrom = eina_list_append(volume_cdrom, dev);
               event_send(sys, EEZE_SCANNER_EVENT_TYPE_ADD, EINA_TRUE);
               l = NULL;
               break;
            }
        if (!l) continue;
        event_send(sys, EEZE_SCANNER_EVENT_TYPE_ADD, EINA_TRUE);
     }
}

static void
cb_vol_chg(const char *device, Eeze_Udev_Event ev, void *data EINA_UNUSED, Eeze_Udev_Watch *watch EINA_UNUSED)
{
   Eina_List *l;
   Eeze_Scanner_Device *dev;

   DBG("device='%s'", device);

   if (ev == EEZE_UDEV_EVENT_ONLINE) ev = EEZE_UDEV_EVENT_ADD;
   else if (ev == EEZE_UDEV_EVENT_OFFLINE) ev = EEZE_UDEV_EVENT_REMOVE;

   event_send(device, (Eeze_Scanner_Event_Type)ev, EINA_TRUE);
   switch (ev)
     {
        case EEZE_UDEV_EVENT_ADD:
        case EEZE_UDEV_EVENT_ONLINE:
          INF("Added volume '%s'", device);
          EINA_LIST_FOREACH(storage_cdrom, l, dev)
            if (device == dev->device)
              {
                 volume_cdrom = eina_list_append(volume_cdrom, dev);
                 return;
              }
          volume_devices = eina_list_append(volume_devices, eina_stringshare_add(device));
          break;
        case EEZE_UDEV_EVENT_REMOVE:
        case EEZE_UDEV_EVENT_OFFLINE:
          INF("Removed volume '%s'", device);
          EINA_LIST_FOREACH(volume_cdrom, l, dev)
            if (device == dev->device)
              {
                 volume_cdrom = eina_list_remove_list(volume_cdrom, l);
                 return;
              }
          volume_devices = eina_list_remove(volume_devices, device);
          eina_stringshare_del(device);
          break;
        default:
          INF("Changed volume '%s'", device);
          break;
     }
}

static void
cb_stor_chg(const char *device, Eeze_Udev_Event ev, void *data EINA_UNUSED, Eeze_Udev_Watch *watch EINA_UNUSED)
{
   Eina_List *l;
   Eeze_Scanner_Device *dev = NULL;
   const char *str;


   DBG("device='%s'", device);   
   switch (ev)
     {
        case EEZE_UDEV_EVENT_ADD:
        case EEZE_UDEV_EVENT_ONLINE:
          INF("Added device '%s'", device);
          event_send(device, (Eeze_Scanner_Event_Type)ev, EINA_FALSE);
          str = eeze_udev_syspath_get_property(device, "ID_CDROM");
          if (!str)
            {
               storage_devices = eina_list_append(storage_devices, eina_stringshare_add(device));
               return;
            }
          eina_stringshare_del(str);
          dev = calloc(1, sizeof(Eeze_Scanner_Device));
          if (!dev) return;
          dev->device = eina_stringshare_add(device);
          dev->poller = ecore_poller_add(ECORE_POLLER_CORE, 32,
                                         (Ecore_Task_Cb)cdrom_timer, dev);
          storage_cdrom = eina_list_append(storage_cdrom, dev);
          break;
        case EEZE_UDEV_EVENT_REMOVE:
        case EEZE_UDEV_EVENT_OFFLINE:
          if (!eina_list_data_find(storage_devices, device))
            {
               EINA_LIST_FOREACH(storage_cdrom, l, dev)
                 if (dev->device == device) break;
               if ((!dev) || (dev->device != device)) return;
            }
          INF("Removed device '%s'", device);
          event_send(device, (Eeze_Scanner_Event_Type)ev, EINA_FALSE);
          EINA_LIST_FOREACH(storage_cdrom, l, dev)
            if (device == dev->device)
              {
                 if (dev->poller) ecore_poller_del(dev->poller);
                 storage_cdrom = eina_list_remove_list(storage_cdrom, l);
                 eina_stringshare_del(dev->device);
                 free(dev);
                 return;
              }
          storage_devices = eina_list_remove(storage_devices, device);
          eina_stringshare_del(device);
          break;
        default:
          INF("Changed device '%s'", device);
          break;
     }
}

static void
es_exit(int sig)
{
   ecore_con_server_del(svr);
   exit(sig);   
}

static void
sigs_setup(void)
{
   sigset_t sigs = {{0}};
   struct sigaction s = {{0}};

   sigfillset(&sigs);
   sigdelset(&sigs, SIGSEGV);
   sigdelset(&sigs, SIGTERM);
   sigdelset(&sigs, SIGINT);
   sigdelset(&sigs, SIGQUIT);

   s.sa_handler = es_exit;
   s.sa_flags = 0;
   sigaction(SIGTERM, &s, NULL);
   sigaction(SIGSEGV, &s, NULL);
   sigaction(SIGINT, &s, NULL);
}

int
main(void)
{
   int ret;
   eina_init();
   ecore_init();
   ecore_con_init();
   eeze_init();
   eeze_disk_function();
   eeze_mount_tabs_watch();

   sigs_setup();
   es_log_dom = eina_log_domain_register("eeze_scanner", EINA_COLOR_CYAN);

   eet_setup();
   clients = eina_hash_pointer_new(NULL);
   ret = 1;
   EINA_SAFETY_ON_NULL_GOTO(clients, error);

   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, (Ecore_Event_Handler_Cb)cl_add, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, (Ecore_Event_Handler_Cb)cl_del, NULL);
   ecore_event_handler_add(EEZE_EVENT_DISK_UNMOUNT, (Ecore_Event_Handler_Cb)disk_mount, NULL);
   ecore_event_handler_add(EEZE_EVENT_DISK_MOUNT, (Ecore_Event_Handler_Cb)disk_mount, NULL);

   eeze_udev_watch_add(EEZE_UDEV_TYPE_DRIVE_INTERNAL, EEZE_UDEV_EVENT_NONE, cb_stor_chg, NULL);
   eeze_udev_watch_add(EEZE_UDEV_TYPE_DRIVE_REMOVABLE, EEZE_UDEV_EVENT_NONE, cb_stor_chg, NULL);
   eeze_udev_watch_add(EEZE_UDEV_TYPE_DRIVE_CDROM, EEZE_UDEV_EVENT_NONE, cb_stor_chg, NULL);
   eeze_udev_watch_add(EEZE_UDEV_TYPE_DRIVE_MOUNTABLE, EEZE_UDEV_EVENT_NONE, cb_vol_chg, NULL);

   svr = ecore_con_server_add(ECORE_CON_LOCAL_SYSTEM, "eeze_scanner", 0, NULL);
   ret = 2;
   EINA_SAFETY_ON_NULL_GOTO(svr, error);
   
   storage_setup();
   ecore_main_loop_begin();

   ecore_con_server_del(svr);
   return 0;
error:
   ERR("Could not start up!");
   exit(ret);
}
