#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Con.h>

#include <unistd.h>

enum
{
   MODE_NONE,
   MODE_GLX
};

int _vsync_init_glx(void);
double _vsync_wait_glx(void);

static void _svr_broadcast_time(double t);

static int _vsync_mode = MODE_NONE;

static int
_vsync_init(void)
{
   if (_vsync_init_glx()) _vsync_mode = MODE_GLX;
   else return 0;
   return 1;
}

static double
_vsync_wait(void)
{
   if (_vsync_mode == MODE_GLX) return _vsync_wait_glx();
   return 0.0;
}

typedef struct
{
   Eina_Thread_Queue_Msg head;
   char val;
} Msg;

static Eina_Thread_Queue *thq = NULL;
static volatile int ticking = 0;

static void
_tick_core(void *data EINA_UNUSED, Ecore_Thread *thread)
{
   Msg *msg;
   void *ref;
   int tick = 0;

   for (;;)
     {
        if (!tick)
          {
             msg = eina_thread_queue_wait(thq, &ref);
             if (msg)
               {
                  tick = msg->val;
                  eina_thread_queue_wait_done(thq, ref);
               }
          }
        else
          {
             do
               {
                  msg = eina_thread_queue_poll(thq, &ref);
                  if (msg)
                    {
                       tick = msg->val;
                       eina_thread_queue_wait_done(thq, ref);
                    }
               }
             while (msg);
          }
        if (tick == -1) exit(0);
        if (tick)
          {
             double *t;

             t = malloc(sizeof(*t));
             if (t)
               {
                  *t = _vsync_wait();
                  do
                    {
                       msg = eina_thread_queue_poll(thq, &ref);
                       if (msg)
                         {
                            tick = msg->val;
                            eina_thread_queue_wait_done(thq, ref);
                         }
                    }
                  while (msg);
                  if (tick) ecore_thread_feedback(thread, t);
               }
          }
     }
}

static void
_tick_notify(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED, void *msg)
{
   double *t = msg;

   if (t)
     {
        _svr_broadcast_time(*t);
        free(t);
     }
}

static void
_tick_init(void)
{
   thq = eina_thread_queue_new();
   ecore_thread_feedback_run(_tick_core, _tick_notify,
                             NULL, NULL, NULL, EINA_TRUE);
}

static void
_tick_send(char val)
{
   Msg *msg;
   void *ref;
   msg = eina_thread_queue_send(thq, sizeof(Msg), &ref);
   msg->val = val;
   eina_thread_queue_send_done(thq, ref);
}

static void
_tick_start(void)
{
   ticking++;
   if (ticking == 1) _tick_send(1);
}

static void
_tick_end(void)
{
   if (ticking <= 0) return;
   ticking--;
   if (ticking == 0) _tick_send(0);
}

/*--------------------------------------------------------------------*/

typedef struct
{
   Ecore_Con_Client *client;
   int enabled;
} Clientdata;

static Ecore_Con_Server *svr = NULL;
static Eina_List *clients = NULL;

static void
_svr_broadcast_time(double t)
{
   Eina_List *l;
   Clientdata *cdat;

   EINA_LIST_FOREACH(clients, l, cdat)
     {
        if (cdat->enabled > 0)
          ecore_con_client_send(cdat->client, &t, sizeof(t));
     }
}

static Eina_Bool
_svr_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Add *ev = event;
   if (svr != ecore_con_client_server_get(ev->client)) return EINA_TRUE;
   Clientdata *cdat = calloc(1, sizeof(Clientdata));
   if (cdat)
     {
        cdat->client = ev->client;
        clients = eina_list_append(clients, cdat);
        ecore_con_client_data_set(ev->client, cdat);
     }
   return EINA_FALSE;
}

static Eina_Bool
_svr_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Del *ev = event;
   if (svr != ecore_con_client_server_get(ev->client)) return EINA_TRUE;
   Clientdata *cdat = ecore_con_client_data_get(ev->client);
   if (cdat)
     {
        while (cdat->enabled > 0)
          {
             cdat->enabled--;
             if (cdat->enabled == 0) _tick_end();
          }
        clients = eina_list_remove(clients, cdat);
        free(cdat);
     }
   return EINA_FALSE;
}

static Eina_Bool
_svr_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Data *ev = event;
   if (svr != ecore_con_client_server_get(ev->client)) return EINA_TRUE;
   Clientdata *cdat = ecore_con_client_data_get(ev->client);
   if (cdat)
     {
        char *dat = ev->data;
        int i;
        int penabled = cdat->enabled;

        for (i = 0; i < ev->size; i++)
          {
             if (dat[i]) cdat->enabled++;
             else if (cdat->enabled > 0) cdat->enabled--;
          }
        if (ev->size > 0)
          {
             if (penabled != cdat->enabled)
               {
                  if (cdat->enabled == 1) _tick_start();
                  else if (cdat->enabled == 0) _tick_end();
               }
          }
     }
   return EINA_FALSE;
}

static void
_svr_init(void)
{
   char buf[4096], *disp, *s;

   disp = getenv("DISPLAY");
   if (!disp) disp = ":0";
   snprintf(buf, sizeof(buf), "ecore-x-vsync-%s", disp);
   for (s = buf; *s; s++)
     {
        if (*s == ':') *s = '=';
     }
   svr = ecore_con_server_add(ECORE_CON_LOCAL_USER, buf, 1, NULL);
   if (!svr) exit(0);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, _svr_add, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, _svr_del, NULL);
   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, _svr_data, NULL);
}

/*--------------------------------------------------------------------*/

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   eina_init();
   ecore_app_no_system_modules();
   ecore_init();
   ecore_con_init();

   if (!_vsync_init()) return 7;
   _svr_init();
   _tick_init();

    ecore_main_loop_begin();
   _tick_send(-1);
   pause();
   return 0;
}
