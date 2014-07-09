#include "ecore_evas_extn_engine.h"

#define NBUF 2

static int blank = 0x00000000;
static const char *interface_extn_name = "extn";
static const int   interface_extn_version = 1;

static Ecore_Evas_Interface_Extn *_ecore_evas_extn_interface_new(void);
static void *_ecore_evas_socket_switch(void *data, void *dest_buf);

typedef struct _Extn Extn;

struct _Extn
{
   struct {
      Ecore_Ipc_Server *server;
      Eina_List *clients;
      Eina_List *handlers;
   } ipc;
   struct {
      const char *name;
      int         num;
      Eina_Bool   sys : 1;
   } svc;
   struct {
      Eina_List  *updates;
   } file;
   struct {
      Extnbuf *buf, *obuf; // current buffer and if needed an "old" buffer
      const char *base, *lock;
      int id, num, w, h;
      Eina_Bool sys : 1;
      Eina_Bool alpha : 1;
   } b[NBUF];
   int cur_b; // current buffer (b) being displayed or rendered to
   struct {
      Eina_Bool   done : 1; /* need to send change done event to the client(plug) */
   } profile;
};

static Eina_List *extn_ee_list = NULL;

static Eina_Bool
_ecore_evas_extn_module_init(void)
{
   return EINA_TRUE;
}

static void
_ecore_evas_extn_module_shutdown(void)
{
}

static void
_ecore_evas_extn_event_free(void *data, void *ev EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   _ecore_evas_unref(ee);
}

static void
_ecore_evas_extn_event(Ecore_Evas *ee, int event)
{
   _ecore_evas_ref(ee);
   ecore_event_add(event, ee, _ecore_evas_extn_event_free, ee);
}

static void
_ecore_evas_extn_plug_render_pre(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata;
   Extn *extn;

   if (!ee) return;
   bdata = ee->engine.data;
   if (!bdata) return;
   extn = bdata->data;
   if (!extn) return;
   bdata->pixels = _extnbuf_lock(extn->b[extn->cur_b].buf, NULL, NULL, NULL);
}

static void
_ecore_evas_extn_plug_render_post(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata;
   Extn *extn;

   if (!ee) return;
   bdata = ee->engine.data;
   if (!bdata) return;
   extn = bdata->data;
   if (!extn) return;
   _extnbuf_unlock(extn->b[extn->cur_b].buf);
}

static void
_ecore_evas_extn_plug_image_obj_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   ecore_evas_free(ee);
}

static void
_ecore_evas_extn_coord_translate(Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord xx, yy, ww, hh, fx, fy, fw, fh;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   evas_object_geometry_get(bdata->image, &xx, &yy, &ww, &hh);
   evas_object_image_fill_get(bdata->image, &fx, &fy, &fw, &fh);

   if (fw < 1) fw = 1;
   if (fh < 1) fh = 1;

   if (evas_object_map_get(bdata->image) &&
       evas_object_map_enable_get(bdata->image))
     {
        fx = 0; fy = 0;
        fw = ee->w; fh = ee->h;
        ww = ee->w; hh = ee->h;
     }

   if ((fx == 0) && (fy == 0) && (fw == ww) && (fh == hh))
     {
        *x = (ee->w * (*x - xx)) / fw;
        *y = (ee->h * (*y - yy)) / fh;
     }
   else
     {
        xx = (*x - xx) - fx;
        while (xx < 0) xx += fw;
        while (xx > fw) xx -= fw;
        *x = (ee->w * xx) / fw;

        yy = (*y - yy) - fy;
        while (yy < 0) yy += fh;
        while (yy > fh) yy -= fh;
        *y = (ee->h * yy) / fh;
     }
}

static void
_ecore_evas_extn_free(Ecore_Evas *ee)
{
   Extn *extn;
   Ecore_Ipc_Client *client;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   if (!bdata) return;

   extn = bdata->data;
   if (extn)
     {
        Ecore_Event_Handler *hdl;
        Ipc_Data_Update *ipc;
        int i;

        for (i = 0; i < NBUF; i++)
          {
             if (extn->b[i].buf) _extnbuf_free(extn->b[i].buf);
             if (extn->b[i].obuf) _extnbuf_free(extn->b[i].obuf);
             if (extn->b[i].base) eina_stringshare_del(extn->b[i].base);
             if (extn->b[i].lock) eina_stringshare_del(extn->b[i].lock);
             extn->b[i].buf = NULL;
             extn->b[i].obuf = NULL;
             extn->b[i].base = NULL;
             extn->b[i].lock = NULL;
          }
        if (extn->svc.name) eina_stringshare_del(extn->svc.name);
        if (extn->ipc.clients)
          {
             EINA_LIST_FREE(extn->ipc.clients, client)
               ecore_ipc_client_del(client);
          }
        if (extn->ipc.server) ecore_ipc_server_del(extn->ipc.server);

        EINA_LIST_FREE(extn->file.updates, ipc)
          free(ipc);

        EINA_LIST_FREE(extn->ipc.handlers, hdl)
          ecore_event_handler_del(hdl);
        free(extn);
        ecore_ipc_shutdown();
        bdata->data = NULL;
     }
   if (bdata->image)
     {
        Ecore_Evas *ee2;

        evas_object_event_callback_del_full(bdata->image,
                                            EVAS_CALLBACK_DEL,
                                            _ecore_evas_extn_plug_image_obj_del,
                                            ee);
        evas_event_callback_del_full(evas_object_evas_get(bdata->image),
                                     EVAS_CALLBACK_RENDER_PRE,
                                     _ecore_evas_extn_plug_render_pre,
                                     ee);
        evas_event_callback_del_full(evas_object_evas_get(bdata->image),
                                     EVAS_CALLBACK_RENDER_POST,
                                     _ecore_evas_extn_plug_render_post,
                                     ee);
        ee2 = evas_object_data_get(bdata->image, "Ecore_Evas_Parent");
        if (ee2)
          {
             ee2->sub_ecore_evas = eina_list_remove(ee2->sub_ecore_evas, ee);
          }
        evas_object_del(bdata->image);
		bdata->image = NULL;
     }
   free(bdata);
   ee->engine.data = NULL;
   extn_ee_list = eina_list_remove(extn_ee_list, ee);
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->req.w = w;
   ee->req.h = h;
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;

   /*
    * No need for it if not used later.
   Extn *extn;

   extn = bdata->data;
   */
   if (bdata->image)
     evas_object_image_size_set(bdata->image, ee->w, ee->h);
   /* Server can have many plugs, so I block resize comand from client to server *
      if ((extn) && (extn->ipc.server))
      {
      Ipc_Data_Resize ipc;

      ipc.w = ee->w;
      ipc.h = ee->h;
      ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_RESIZE, 0, 0, 0, &ipc, sizeof(ipc));
      }*/
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x EINA_UNUSED, int y EINA_UNUSED, int w, int h)
{
   _ecore_evas_resize(ee, w, h);
}

static int
_ecore_evas_modifiers_locks_mask_get(Evas *e)
{
   int mask = 0;

   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Shift"))
     mask |= MOD_SHIFT;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Control"))
     mask |= MOD_CTRL;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Alt"))
     mask |= MOD_ALT;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Meta"))
     mask |= MOD_META;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Hyper"))
     mask |= MOD_HYPER;
   if (evas_key_modifier_is_set(evas_key_modifier_get(e), "Super"))
     mask |= MOD_SUPER;
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Scroll_Lock"))
     mask |= MOD_SCROLL;
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Num_Lock"))
     mask |= MOD_NUM;
   if (evas_key_lock_is_set(evas_key_lock_get(e), "Caps_Lock"))
     mask |= MOD_CAPS;
   return mask;
}

static void
_ecore_evas_modifiers_locks_mask_set(Evas *e, int mask)
{
   if (mask & MOD_SHIFT) evas_key_modifier_on (e, "Shift");
   else                  evas_key_modifier_off(e, "Shift");
   if (mask & MOD_CTRL)  evas_key_modifier_on (e, "Control");
   else                  evas_key_modifier_off(e, "Control");
   if (mask & MOD_ALT)   evas_key_modifier_on (e, "Alt");
   else                  evas_key_modifier_off(e, "Alt");
   if (mask & MOD_META)  evas_key_modifier_on (e, "Meta");
   else                  evas_key_modifier_off(e, "Meta");
   if (mask & MOD_HYPER) evas_key_modifier_on (e, "Hyper");
   else                  evas_key_modifier_off(e, "Hyper");
   if (mask & MOD_SUPER) evas_key_modifier_on (e, "Super");
   else                  evas_key_modifier_off(e, "Super");
   if (mask & MOD_SCROLL) evas_key_lock_on (e, "Scroll_Lock");
   else                   evas_key_lock_off(e, "Scroll_Lock");
   if (mask & MOD_NUM)    evas_key_lock_on (e, "Num_Lock");
   else                   evas_key_lock_off(e, "Num_Lock");
   if (mask & MOD_CAPS)   evas_key_lock_on (e, "Caps_Lock");
   else                   evas_key_lock_off(e, "Caps_Lock");
}

static void
_ecore_evas_extn_cb_mouse_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Mouse_In *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Mouse_In ipc;
        memset(&ipc, 0, sizeof(ipc));

        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_IN, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_mouse_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Mouse_Out *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Mouse_Out ipc;
        memset(&ipc, 0, sizeof(ipc));

        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_OUT, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
       /* We have send mouse move event before mouse down event */
       {
          Ipc_Data_Ev_Mouse_Move ipc_move;
          memset(&ipc_move, 0, sizeof(ipc_move));
          Evas_Coord x, y;

          x = ev->canvas.x;
          y = ev->canvas.y;
          _ecore_evas_extn_coord_translate(ee, &x, &y);
          ipc_move.x = x;
          ipc_move.y = y;
          ipc_move.timestamp = ev->timestamp;
          ipc_move.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
          ipc_move.event_flags = ev->event_flags;
          ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_MOVE, 0, 0, 0, &ipc_move, sizeof(ipc_move));
       }
       {
          Ipc_Data_Ev_Mouse_Down ipc;
          memset(&ipc, 0, sizeof(ipc));
          ipc.b = ev->button;
          ipc.flags = ev->flags;
          ipc.timestamp = ev->timestamp;
          ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
          ipc.event_flags = ev->event_flags;
          ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_DOWN, 0, 0, 0, &ipc, sizeof(ipc));
       }
     }
}

static void
_ecore_evas_extn_cb_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Mouse_Up *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Mouse_Up ipc;
        memset(&ipc, 0, sizeof(ipc));

        ipc.b = ev->button;
        ipc.flags = ev->flags;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_UP, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Mouse_Move *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Mouse_Move ipc;
        memset(&ipc, 0, sizeof(ipc));
        Evas_Coord x, y;

        x = ev->cur.canvas.x;
        y = ev->cur.canvas.y;
        _ecore_evas_extn_coord_translate(ee, &x, &y);
        ipc.x = x;
        ipc.y = y;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_MOVE, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_mouse_wheel(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Mouse_Wheel *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Mouse_Wheel ipc;
        memset(&ipc, 0, sizeof(ipc));

        ipc.direction = ev->direction;
        ipc.z = ev->z;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MOUSE_WHEEL, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_multi_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Multi_Down *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Multi_Down ipc;
        memset(&ipc, 0, sizeof(ipc));
        Evas_Coord x, y;

        ipc.d = ev->device;
        x = ev->canvas.x;
        y = ev->canvas.y;
        _ecore_evas_extn_coord_translate(ee, &x, &y);
        ipc.x = x;
        ipc.y = y;
        ipc.rad = ev->radius;
        ipc.radx = ev->radius_x;
        ipc.rady = ev->radius_y;
        ipc.pres = ev->pressure;
        ipc.ang = ev->angle;
        ipc.fx = ev->canvas.xsub;
        ipc.fy = ev->canvas.ysub;
        ipc.flags = ev->flags;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MULTI_DOWN, 0, 0, 0, &ipc, sizeof(ipc));
     }
}


static void
_ecore_evas_extn_cb_multi_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Multi_Up *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Multi_Up ipc;
        memset(&ipc, 0, sizeof(ipc));
        Evas_Coord x, y;

        ipc.d = ev->device;
        x = ev->canvas.x;
        y = ev->canvas.y;
        _ecore_evas_extn_coord_translate(ee, &x, &y);
        ipc.x = x;
        ipc.y = y;
        ipc.rad = ev->radius;
        ipc.radx = ev->radius_x;
        ipc.rady = ev->radius_y;
        ipc.pres = ev->pressure;
        ipc.ang = ev->angle;
        ipc.fx = ev->canvas.xsub;
        ipc.fy = ev->canvas.ysub;
        ipc.flags = ev->flags;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MULTI_UP, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_multi_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Multi_Move *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Multi_Move ipc;
        memset(&ipc, 0, sizeof(ipc));
        Evas_Coord x, y;

        ipc.d = ev->device;
        x = ev->cur.canvas.x;
        y = ev->cur.canvas.y;
        _ecore_evas_extn_coord_translate(ee, &x, &y);
        ipc.x = x;
        ipc.y = y;
        ipc.rad = ev->radius;
        ipc.radx = ev->radius_x;
        ipc.rady = ev->radius_y;
        ipc.pres = ev->pressure;
        ipc.ang = ev->angle;
        ipc.fx = ev->cur.canvas.xsub;
        ipc.fy = ev->cur.canvas.ysub;
        ipc.timestamp = ev->timestamp;
        ipc.mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_MULTI_MOVE, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Key_Down *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Key_Down *ipc;
        char *st, *p;
        int len = 0;

        len = sizeof(Ipc_Data_Ev_Key_Down);
        if (ev->key) len += strlen(ev->key) + 1;
        if (ev->keyname) len += strlen(ev->keyname) + 1;
        if (ev->string) len += strlen(ev->string) + 1;
        if (ev->compose) len += strlen(ev->compose) + 1;
        len += 1;
        st = alloca(len);
        ipc = (Ipc_Data_Ev_Key_Down *)st;
        memset(st, 0, len);
        p = st + sizeof(Ipc_Data_Ev_Key_Down);
        if (ev->key)
          {
             strcpy(p, ev->key);
             ipc->key = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->keyname)
          {
             strcpy(p, ev->keyname);
             ipc->keyname = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->string)
          {
             strcpy(p, ev->string);
             ipc->string = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->compose)
          {
             strcpy(p, ev->compose);
             ipc->compose = p - (long)st;
             p += strlen(p) + 1;
          }
        ipc->timestamp = ev->timestamp;
        ipc->mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc->event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_KEY_DOWN, 0, 0, 0, ipc, len);
     }
}

static void
_ecore_evas_extn_cb_key_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Key_Up *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Key_Up *ipc;
        char *st, *p;
        int len = 0;

        len = sizeof(Ipc_Data_Ev_Key_Up);
        if (ev->key) len += strlen(ev->key) + 1;
        if (ev->keyname) len += strlen(ev->keyname) + 1;
        if (ev->string) len += strlen(ev->string) + 1;
        if (ev->compose) len += strlen(ev->compose) + 1;
        len += 1;
        st = alloca(len);
        ipc = (Ipc_Data_Ev_Key_Up *)st;
        memset(st, 0, len);
        p = st + sizeof(Ipc_Data_Ev_Key_Down);
        if (ev->key)
          {
             strcpy(p, ev->key);
             ipc->key = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->keyname)
          {
             strcpy(p, ev->keyname);
             ipc->keyname = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->string)
          {
             strcpy(p, ev->string);
             ipc->string = p - (long)st;
             p += strlen(p) + 1;
          }
        if (ev->compose)
          {
             strcpy(p, ev->compose);
             ipc->compose = p - (long)st;
             p += strlen(p) + 1;
          }
        ipc->timestamp = ev->timestamp;
        ipc->mask = _ecore_evas_modifiers_locks_mask_get(ee->evas);
        ipc->event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_KEY_UP, 0, 0, 0, ipc, len);
     }
}

static void
_ecore_evas_extn_cb_hold(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Event_Hold *ev = event_info;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (extn->ipc.server)
     {
        Ipc_Data_Ev_Hold ipc;
        memset(&ipc, 0, sizeof(ipc));

        ipc.hold = ev->hold;
        ipc.timestamp = ev->timestamp;
        ipc.event_flags = ev->event_flags;
        ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_EV_HOLD, 0, 0, 0, &ipc, sizeof(ipc));
     }
}

static void
_ecore_evas_extn_cb_focus_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   ee->prop.focused = EINA_TRUE;
   extn = bdata->data;
   if (!extn) return;
   if (!extn->ipc.server) return;
   ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_FOCUS, 0, 0, 0, NULL, 0);
}

static void
_ecore_evas_extn_cb_focus_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   ee->prop.focused = EINA_FALSE;
   extn = bdata->data;
   if (!extn) return;
   if (!extn->ipc.server) return;
   ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_UNFOCUS, 0, 0, 0, NULL, 0);
}

static void
_ecore_evas_extn_cb_show(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   ee->visible = 1;
   extn = bdata->data;
   if (!extn) return;
   if (!extn->ipc.server) return;
   ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_SHOW, 0, 0, 0, NULL, 0);
}

static void
_ecore_evas_extn_cb_hide(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   ee->visible = 0;
   extn = bdata->data;
   if (!extn) return;
   if (!extn->ipc.server) return;
   ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_HIDE, 0, 0, 0, NULL, 0);
}

static void
_ecore_evas_extn_plug_profile_set(Ecore_Evas *ee, const char *profile)
{
   Extn *extn;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   _ecore_evas_window_profile_free(ee);
   ee->prop.profile.name = NULL;

   extn = bdata->data;
   if (!extn) return;

   if (profile)
     {
        ee->prop.profile.name = (char *)eina_stringshare_add(profile);
        if (extn->ipc.server)
          ecore_ipc_server_send(extn->ipc.server, MAJOR,
                                OP_PROFILE_CHANGE_REQUEST,
                                0, 0, 0, profile, strlen(profile) + 1);
     }
}

static void
_ecore_evas_extn_plug_msg_parent_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   extn = bdata->data;
   if (!extn) return;
   if (!extn->ipc.server) return;

   //ref = msg_domain
   //ref_to = msg_id
   ecore_ipc_server_send(extn->ipc.server, MAJOR, OP_MSG, msg_domain, msg_id, 0, data, size);
}

static const Ecore_Evas_Engine_Func _ecore_extn_plug_engine_func =
{
   _ecore_evas_extn_free,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_resize,
   _ecore_evas_move_resize,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL, //transparent
   NULL, // profiles_set
   _ecore_evas_extn_plug_profile_set,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,

   NULL, // render
   NULL, // screen_geometry_get
   NULL, // screen_dpi_get
   _ecore_evas_extn_plug_msg_parent_send,
   NULL,   // msg_send

   /* 1.8 abstractions */
   NULL, // pointer_xy_get
   NULL, // pointer_warp

   NULL, // wm_rot_preferred_rotation_set
   NULL, // wm_rot_available_rotations_set
   NULL, // wm_rot_manual_rotation_done_set
   NULL, // wm_rot_manual_rotation_done

   NULL  // aux_hints_set
};

static Eina_Bool
_ipc_server_add(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Server_Add *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   if (ee != ecore_ipc_server_data_get(e->server))
     return ECORE_CALLBACK_PASS_ON;
   if (!eina_list_data_find(extn_ee_list, ee))
     return ECORE_CALLBACK_PASS_ON;
   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;
   //FIXME: find a way to let app know server there
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ipc_server_del(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Server_Del *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;
   int i;

   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;
   if (extn->ipc.server != e->server) return ECORE_CALLBACK_PASS_ON;
   evas_object_image_data_set(bdata->image, NULL);
   evas_object_image_pixels_dirty_set(bdata->image, EINA_TRUE);
   bdata->pixels = NULL;
   extn->ipc.server = NULL;

   for (i = 0; i < NBUF; i++)
     {
        if (extn->b[i].buf) _extnbuf_free(extn->b[i].buf);
        if (extn->b[i].obuf) _extnbuf_free(extn->b[i].obuf);
        if (extn->b[i].base) eina_stringshare_del(extn->b[i].base);
        if (extn->b[i].lock) eina_stringshare_del(extn->b[i].lock);
        extn->b[i].buf = NULL;
        extn->b[i].obuf = NULL;
        extn->b[i].base = NULL;
        extn->b[i].lock = NULL;
     }
   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ipc_server_data(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Server_Data *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   if (ee != ecore_ipc_server_data_get(e->server))
     return ECORE_CALLBACK_PASS_ON;
   if (!eina_list_data_find(extn_ee_list, ee))
     return ECORE_CALLBACK_PASS_ON;
   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;
   if (e->major != MAJOR)
     return ECORE_CALLBACK_PASS_ON;
   switch (e->minor)
     {
      case OP_UPDATE:
         // add rect to update list
         if (e->size >= (int)sizeof(Ipc_Data_Update))
           {
              Ipc_Data_Update *ipc = malloc(sizeof(Ipc_Data_Update));
              if (ipc)
                {
                   memcpy(ipc, e->data, sizeof(Ipc_Data_Update));
                   extn->file.updates = eina_list_append(extn->file.updates,
                                                         ipc);
                }
           }
         break;
      case OP_UPDATE_DONE:
        // e->response == display buffer #
        // updates finished being sent - done now. frame ready
           {
              Ipc_Data_Update *ipc;
              int n = e->response;

              EINA_LIST_FREE(extn->file.updates, ipc)
                {
                   if (bdata->image)
                     evas_object_image_data_update_add(bdata->image,
                                                       ipc->x, ipc->y,
                                                       ipc->w, ipc->h);
                   free(ipc);
                }
              if ((n >= 0) && (n < NBUF))
                {
                   void *data2;
                   int w = 0, h = 0, pn;

                   pn = extn->cur_b;
                   extn->cur_b = n;

                   if (extn->b[pn].buf) _extnbuf_unlock(extn->b[pn].buf);
                   if (extn->b[pn].obuf)
                     {
                        _extnbuf_unlock(extn->b[pn].obuf);
                        _extnbuf_free(extn->b[pn].obuf);
                        extn->b[pn].obuf = NULL;
                     }

                   evas_object_image_colorspace_set(bdata->image, EVAS_COLORSPACE_ARGB8888);
                   if (extn->b[n].buf)
                     {
                        data2 = _extnbuf_data_get(extn->b[n].buf, &w, &h, NULL);
                        bdata->pixels = data2;
                        evas_object_image_alpha_set(bdata->image,
                                                    extn->b[n].alpha);
                        evas_object_image_size_set(bdata->image, w, h);
                        evas_object_image_data_set(bdata->image, data2);
                     }
                   else
                     {
                        bdata->pixels = NULL;
                        evas_object_image_alpha_set(bdata->image, EINA_TRUE);
                        evas_object_image_size_set(bdata->image, 1, 1);
                        evas_object_image_data_set(bdata->image, &blank);
                     }
                }
           }
         break;
      case OP_SHM_REF0:
         // e->ref == shm id
         // e->ref_to == shm num
         // e->response == buffer num
         // e->data = shm ref string + nul byte
         if ((e->data) && (e->size > 0) &&
             (((unsigned char *)e->data)[e->size - 1] == 0))
           {
              int n = e->response;

              if ((n >= 0) && (n < NBUF))
                {
                   extn->b[n].id = e->ref;
                   extn->b[n].num = e->ref_to;
                   if (extn->b[n].base) eina_stringshare_del(extn->b[n].base);
                   extn->b[n].base = eina_stringshare_add(e->data);
                }
           }
         break;
      case OP_SHM_REF1:
         // e->ref == w
         // e->ref_to == h
         // e->response == buffer num
         // e->data = lockfile + nul byte
         if ((e->data) && (e->size > 0) &&
             (((unsigned char *)e->data)[e->size - 1] == 0))
           {
              int n = e->response;

              if ((n >= 0) && (n < NBUF))
                {
                   extn->b[n].w = e->ref;
                   extn->b[n].h = e->ref_to;
                   if (extn->b[n].lock) eina_stringshare_del(extn->b[n].lock);
                   extn->b[n].lock = eina_stringshare_add(e->data);
                }
           }
         break;
      case OP_SHM_REF2:
         // e->ref == alpha
         // e->ref_to == sys
         // e->response == buffer num
           {
              int n = e->response;

              if ((n >= 0) && (n < NBUF))
                {
                   extn->b[n].alpha = e->ref;
                   extn->b[n].sys = e->ref_to;
                   if (extn->b[n].buf)
                     {
                        if (_extnbuf_lock_get(extn->b[n].buf))
                          {
                             if (extn->b[n].obuf) ERR("obuf is non-null");
                             extn->b[n].obuf = extn->b[n].buf;
                          }
                        else
                          _extnbuf_free(extn->b[n].buf);
                     }
                   extn->b[n].buf = _extnbuf_new(extn->b[n].base,
                                                 extn->b[n].id,
                                                 extn->b[n].sys,
                                                 extn->b[n].num,
                                                 extn->b[n].w,
                                                 extn->b[n].h,
                                                 EINA_FALSE);
                   if ((extn->b[n].buf) && (extn->b[n].lock))
                     _extnbuf_lock_file_set(extn->b[n].buf,
                                            extn->b[n].lock);
                }
           }
         break;
      case OP_RESIZE:
         if ((e->data) && (e->size >= (int)sizeof(Ipc_Data_Resize)))
           {
              Ipc_Data_Resize *ipc = e->data;
              _ecore_evas_resize(ee, ipc->w, ipc->h);
           }
         break;
      case OP_PROFILE_CHANGE_DONE:
         /* profile change finished being sent - done now. */
         /* do something here */
         break;
      case OP_MSG_PARENT:
         if ((e->data) && (e->size > 0))
           {
              //ref = msg_domain
              //ref_to = msg_id
              if (ee->func.fn_msg_handle)
                {
                   INF("Message handle: ref=%d to=%d size=%d", e->ref, e->ref_to, e->size);
                   ee->func.fn_msg_handle(ee, e->ref, e->ref_to, e->data, e->size);
                }
           }
         break;
      default:
         break;
     }
   return ECORE_CALLBACK_PASS_ON;
}

EAPI Evas_Object *
ecore_evas_extn_plug_new_internal(Ecore_Evas *ee_target)
{
   Evas_Object *o;
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Buffer_Data *bdata;
   Ecore_Evas_Interface_Extn *iface;
   int w = 1, h = 1;

   if (!ee_target) return NULL;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   bdata = calloc(1, sizeof(Ecore_Evas_Engine_Buffer_Data));
   if (!bdata)
     {
	free(ee);
	return NULL;
     }
   ee->engine.data = bdata;
   o = evas_object_image_filled_add(ee_target->evas);
   /* this make problem in gl engine, so I'll block this until solve problem
   evas_object_image_content_hint_set(o, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);*/
   evas_object_image_colorspace_set(o, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_alpha_set(o, 1);
   evas_object_image_size_set(o, 1, 1);
   evas_object_image_data_set(o, &blank);

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_extn_plug_engine_func;

   ee->driver = "extn_plug";

   iface = _ecore_evas_extn_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

   ee->rotation = 0;
   ee->visible = 0;
   ee->w = w;
   ee->h = h;
   ee->req.w = ee->w;
   ee->req.h = ee->h;
   ee->profile_supported = 1;

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.focused = EINA_FALSE;
   ee->prop.borderless = EINA_TRUE;
   ee->prop.override = EINA_TRUE;
   ee->prop.maximized = EINA_FALSE;
   ee->prop.fullscreen = EINA_FALSE;
   ee->prop.withdrawn = EINA_FALSE;
   ee->prop.sticky = EINA_FALSE;

   bdata->image = o;
   evas_object_data_set(bdata->image, "Ecore_Evas", ee);
   evas_object_data_set(bdata->image, "Ecore_Evas_Parent", ee_target);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_IN,
                                  _ecore_evas_extn_cb_mouse_in, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_OUT,
                                  _ecore_evas_extn_cb_mouse_out, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_DOWN,
                                  _ecore_evas_extn_cb_mouse_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_UP,
                                  _ecore_evas_extn_cb_mouse_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_MOVE,
                                  _ecore_evas_extn_cb_mouse_move, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_WHEEL,
                                  _ecore_evas_extn_cb_mouse_wheel, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_DOWN,
                                  _ecore_evas_extn_cb_multi_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_UP,
                                  _ecore_evas_extn_cb_multi_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_MOVE,
                                  _ecore_evas_extn_cb_multi_move, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_KEY_DOWN,
                                  _ecore_evas_extn_cb_key_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_KEY_UP,
                                  _ecore_evas_extn_cb_key_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_HOLD,
                                  _ecore_evas_extn_cb_hold, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_FOCUS_IN,
                                  _ecore_evas_extn_cb_focus_in, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_FOCUS_OUT,
                                  _ecore_evas_extn_cb_focus_out, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_SHOW,
                                  _ecore_evas_extn_cb_show, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_HIDE,
                                  _ecore_evas_extn_cb_hide, ee);

   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_DEL,
                                  _ecore_evas_extn_plug_image_obj_del, ee);


   extn_ee_list = eina_list_append(extn_ee_list, ee);
   ee_target->sub_ecore_evas = eina_list_append(ee_target->sub_ecore_evas, ee);
   return o;
}

static Eina_Bool
_ecore_evas_extn_plug_connect(Ecore_Evas *ee, const char *svcname, int svcnum, Eina_Bool svcsys)
{
   Extn *extn;
   Ecore_Evas_Engine_Buffer_Data *bdata;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS)) return EINA_FALSE;

   bdata = ee->engine.data;
   if (!svcname)
     {
        bdata->data = NULL;
        return EINA_FALSE;
     }

   extn = calloc(1, sizeof(Extn));
   if (!extn) return EINA_FALSE;



   Ecore_Ipc_Type ipctype = ECORE_IPC_LOCAL_USER;

   ecore_ipc_init();
   extn->svc.name = eina_stringshare_add(svcname);
   extn->svc.num = svcnum;
   extn->svc.sys = svcsys;

   if (extn->svc.sys) ipctype = ECORE_IPC_LOCAL_SYSTEM;
   extn->ipc.server = ecore_ipc_server_connect(ipctype, (char *)extn->svc.name,
                                               extn->svc.num, ee);
   if (!extn->ipc.server)
     {
        bdata->data = NULL;
        eina_stringshare_del(extn->svc.name);
        free(extn);
        ecore_ipc_shutdown();
        return EINA_FALSE;
     }
   bdata->data = extn;
   extn->ipc.handlers = eina_list_append
      (extn->ipc.handlers,
       ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_ADD,
                               _ipc_server_add, ee));
   extn->ipc.handlers = eina_list_append
      (extn->ipc.handlers,
       ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DEL,
                               _ipc_server_del, ee));
   extn->ipc.handlers = eina_list_append
      (extn->ipc.handlers,
       ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DATA,
                               _ipc_server_data, ee));
   return EINA_TRUE;
}

static void
_ecore_evas_socket_resize(Ecore_Evas *ee, int w, int h)
{
   Extn *extn;
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   int stride = 0;

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->req.w = w;
   ee->req.h = h;
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;
   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   extn = bdata->data;
   if (extn)
     {
        int i, last_try = 0;

        for (i = 0; i < NBUF; i++)
          {
             if (extn->b[i].buf) _extnbuf_free(extn->b[i].buf);
             if (extn->b[i].obuf) _extnbuf_free(extn->b[i].obuf);
             if (extn->b[i].base) eina_stringshare_del(extn->b[i].base);
             if (extn->b[i].lock) eina_stringshare_del(extn->b[i].lock);
             extn->b[i].buf = NULL;
             extn->b[i].obuf = NULL;
             extn->b[i].base = NULL;
             extn->b[i].lock = NULL;
          }
        bdata->pixels = NULL;
        for (i = 0; i < NBUF; i++)
          {
             do
               {
                  extn->b[i].buf = _extnbuf_new(extn->svc.name, extn->svc.num,
                                                extn->svc.sys, last_try,
                                                ee->w, ee->h, EINA_TRUE);
                  if (extn->b[i].buf) extn->b[i].num = last_try;
                  last_try++;
                  if (last_try > 1024) break;
               }
             while (!extn->b[i].buf);

          }

        if (extn->b[extn->cur_b].buf)
          bdata->pixels = _extnbuf_data_get(extn->b[extn->cur_b].buf,
                                            NULL, NULL, &stride);
        einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             if (ee->alpha)
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
             else
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
             einfo->info.dest_buffer = bdata->pixels;
             einfo->info.dest_buffer_row_bytes = stride;
             einfo->info.use_color_key = 0;
             einfo->info.alpha_threshold = 0;
             einfo->info.func.new_update_region = NULL;
             einfo->info.func.free_update_region = NULL;
             einfo->info.func.switch_buffer = _ecore_evas_socket_switch;
             einfo->info.switch_data = ee;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
          }

        if (extn->ipc.clients && extn->b[extn->cur_b].buf)
          {
             Ipc_Data_Resize ipc;
             Eina_List *l;
             Ecore_Ipc_Client *client;

             EINA_LIST_FOREACH(extn->ipc.clients, l, client)
               {
                  for (i = 0; i < NBUF; i++)
                    {
                       const char *lock;

                       ecore_ipc_client_send(client, MAJOR, OP_SHM_REF0,
                                             extn->svc.num, extn->b[i].num, i,
                                             extn->svc.name,
                                             strlen(extn->svc.name) + 1);
                       lock = _extnbuf_lock_file_get(extn->b[i].buf);
                       ecore_ipc_client_send(client, MAJOR, OP_SHM_REF1,
                                             ee->w, ee->h, i,
                                             lock, strlen(lock) + 1);
                       ecore_ipc_client_send(client, MAJOR, OP_SHM_REF2,
                                             ee->alpha, extn->svc.sys, i,
                                             NULL, 0);
                       ipc.w = ee->w;
                       ipc.h = ee->h;
                       ecore_ipc_client_send(client, MAJOR, OP_RESIZE,
                                             0, 0, 0, &ipc, sizeof(ipc));
                    }
               }
          }
     }
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_socket_move_resize(Ecore_Evas *ee, int x EINA_UNUSED, int y EINA_UNUSED, int w, int h)
{
   _ecore_evas_socket_resize(ee, w, h);
}

static void
_ecore_evas_extn_socket_window_profile_change_done_send(Ecore_Evas *ee)
{
   Extn *extn;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Ecore_Ipc_Client *client;
   Eina_List *l = NULL;
   char *s;
   int len = 0;

   extn = bdata->data;
   if (!extn) return;
   s = ee->prop.profile.name;
   if (s) len = strlen(s);
   EINA_LIST_FOREACH(extn->ipc.clients, l, client)
     {
        ecore_ipc_client_send(client, MAJOR,
                              OP_PROFILE_CHANGE_DONE,
                              0, 0, 0, s, len);
     }
}

static void *
_ecore_evas_socket_switch(void *data, void *dest_buf EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn = bdata->data;

   extn->cur_b++;
   if (extn->cur_b >= NBUF) extn->cur_b = 0;
   bdata->pixels = _extnbuf_data_get(extn->b[extn->cur_b].buf,
                                     NULL, NULL, NULL);
   return bdata->pixels;
}

int
_ecore_evas_extn_socket_render(Ecore_Evas *ee)
{
   Eina_List *updates = NULL, *l, *ll;
   Ecore_Evas *ee2;
   int rend = 0;
   Eina_Rectangle *r;
   Extn *extn;
   Ecore_Ipc_Client *client;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   int cur_b;

   extn = bdata->data;
   if (!extn) return rend;
   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }
   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);

   cur_b = extn->cur_b;
   if (bdata->pixels)
     {
        bdata->pixels = _extnbuf_lock(extn->b[cur_b].buf, NULL, NULL, NULL);
        updates = evas_render_updates(ee->evas);
        _extnbuf_unlock(extn->b[cur_b].buf);
     }
   if (updates)
     {
        EINA_LIST_FOREACH(updates, l, r)
          {
             Ipc_Data_Update ipc;


             ipc.x = r->x;
             ipc.y = r->y;
             ipc.w = r->w;
             ipc.h = r->h;
             EINA_LIST_FOREACH(extn->ipc.clients, ll, client)
               ecore_ipc_client_send(client, MAJOR, OP_UPDATE, 0, 0, 0, &ipc,
                                     sizeof(ipc));
          }
        evas_render_updates_free(updates);
        _ecore_evas_idle_timeout_update(ee);
        EINA_LIST_FOREACH(extn->ipc.clients, ll, client)
           ecore_ipc_client_send(client, MAJOR, OP_UPDATE_DONE, 0, 0,
                                 cur_b, NULL, 0);
        if (extn->profile.done)
          {
             _ecore_evas_extn_socket_window_profile_change_done_send(ee);
             extn->profile.done = EINA_FALSE;
          }
     }

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
   return updates ? 1 : rend;
}

static Eina_Bool
_ipc_client_add(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Add *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;
   Ipc_Data_Resize ipc;
   Ipc_Data_Update ipc2;
   int i;

   if (ee != ecore_ipc_server_data_get(ecore_ipc_client_server_get(e->client)))
     return ECORE_CALLBACK_PASS_ON;
   if (!eina_list_data_find(extn_ee_list, ee))
     return ECORE_CALLBACK_PASS_ON;
   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;

   extn->ipc.clients = eina_list_append(extn->ipc.clients, e->client);

   for (i = 0; i < NBUF; i++)
     {
        const char *lock;

        ecore_ipc_client_send(e->client, MAJOR, OP_SHM_REF0,
                              extn->svc.num, extn->b[i].num, i,
                              extn->svc.name,
                              strlen(extn->svc.name) + 1);
        lock = _extnbuf_lock_file_get(extn->b[i].buf);
        ecore_ipc_client_send(e->client, MAJOR, OP_SHM_REF1,
                              ee->w, ee->h, i,
                              lock, strlen(lock) + 1);
        ecore_ipc_client_send(e->client, MAJOR, OP_SHM_REF2,
                              ee->alpha, extn->svc.sys, i,
                              NULL, 0);
     }
   ipc.w = ee->w; ipc.h = ee->h;
   ecore_ipc_client_send(e->client, MAJOR, OP_RESIZE,
                         0, 0, 0, &ipc, sizeof(ipc));
   ipc2.x = 0; ipc2.y = 0; ipc2.w = ee->w; ipc2.h = ee->h;
   ecore_ipc_client_send(e->client, MAJOR, OP_UPDATE, 0, 0, 0, &ipc2,
                         sizeof(ipc2));
   ecore_ipc_client_send(e->client, MAJOR, OP_UPDATE_DONE, 0, 0,
                         extn->cur_b, NULL, 0);
   _ecore_evas_extn_event(ee, ECORE_EVAS_EXTN_CLIENT_ADD);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ipc_client_del(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Del *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;
   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;
   if (!eina_list_data_find(extn->ipc.clients, e->client)) return ECORE_CALLBACK_PASS_ON;

   extn->ipc.clients = eina_list_remove(extn->ipc.clients, e->client);

   _ecore_evas_extn_event(ee, ECORE_EVAS_EXTN_CLIENT_DEL);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ipc_client_data(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Ipc_Event_Client_Data *e = event;
   Ecore_Evas *ee = data;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;

   if (ee != ecore_ipc_server_data_get(ecore_ipc_client_server_get(e->client)))
     return ECORE_CALLBACK_PASS_ON;
   if (!eina_list_data_find(extn_ee_list, ee))
     return ECORE_CALLBACK_PASS_ON;
   extn = bdata->data;
   if (!extn) return ECORE_CALLBACK_PASS_ON;
   if (e->major != MAJOR)
     return ECORE_CALLBACK_PASS_ON;
   switch (e->minor)
     {
      case OP_RESIZE:
         if ((e->data) && (e->size >= (int)sizeof(Ipc_Data_Resize)))
           {

              Ipc_Data_Resize *ipc = e->data;
              /* create callbacke data size changed */
              _ecore_evas_socket_resize(ee, ipc->w, ipc->h);
           }
         break;
      case OP_SHOW:
         if (!ee->visible)
           {
              ee->visible = 1;
              if (ee->func.fn_show) ee->func.fn_show(ee);
           }
         break;
      case OP_HIDE:
         if (ee->visible)
           {
              ee->visible = 0;
              if (ee->func.fn_hide) ee->func.fn_hide(ee);
           }
         break;
      case OP_FOCUS:
         if (!ee->prop.focused)
           {
              ee->prop.focused = EINA_TRUE;
              evas_focus_in(ee->evas);
              if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
           }
         break;
      case OP_UNFOCUS:
         if (ee->prop.focused)
           {
              ee->prop.focused = EINA_FALSE;
              evas_focus_out(ee->evas);
              if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
           }
         break;
      case OP_EV_MOUSE_IN:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_In))
           {
              Ipc_Data_Ev_Mouse_In *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_in(ee->evas, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MOUSE_OUT:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_Out))
           {
              Ipc_Data_Ev_Mouse_Out *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_out(ee->evas, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MOUSE_UP:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_Up))
           {
              Ipc_Data_Ev_Mouse_Up *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_up(ee->evas, ipc->b, ipc->flags, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MOUSE_DOWN:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_Down))
           {
              Ipc_Data_Ev_Mouse_Up *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_down(ee->evas, ipc->b, ipc->flags, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MOUSE_MOVE:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_Move))
           {
              Ipc_Data_Ev_Mouse_Move *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_move(ee->evas, ipc->x, ipc->y, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MOUSE_WHEEL:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Mouse_Wheel))
           {
              Ipc_Data_Ev_Mouse_Wheel *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_mouse_wheel(ee->evas, ipc->direction, ipc->z, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MULTI_UP:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Multi_Up))
           {
              Ipc_Data_Ev_Multi_Up *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_multi_up(ee->evas, ipc->d, ipc->x, ipc->y, ipc->rad, ipc->radx, ipc->rady, ipc->pres, ipc->ang, ipc->fx, ipc->fy, ipc->flags, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MULTI_DOWN:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Multi_Down))
           {
              Ipc_Data_Ev_Multi_Down *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_multi_down(ee->evas, ipc->d, ipc->x, ipc->y, ipc->rad, ipc->radx, ipc->rady, ipc->pres, ipc->ang, ipc->fx, ipc->fy, ipc->flags, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_EV_MULTI_MOVE:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Multi_Move))
           {
              Ipc_Data_Ev_Multi_Move *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
              evas_event_feed_multi_move(ee->evas, ipc->d, ipc->x, ipc->y, ipc->rad, ipc->radx, ipc->rady, ipc->pres, ipc->ang, ipc->fx, ipc->fy, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;

#define STRGET(val) \
         do { \
              if ((ipc->val) && (ipc->val < (char *)(long)(e->size - 1))) \
              ipc->val = ((char *)ipc) + (long)ipc->val; \
              else \
              ipc->val = NULL; \
         } while (0)

      case OP_EV_KEY_UP:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Key_Up))
           {
              if ((e->data) && (e->size > 0) &&
                  (((unsigned char *)e->data)[e->size - 1] == 0))
                {
                   Ipc_Data_Ev_Key_Up *ipc = e->data;
                   Evas_Event_Flags flags;

                   STRGET(keyname);
                   STRGET(key);
                   STRGET(string);
                   STRGET(compose);
                   flags = evas_event_default_flags_get(ee->evas);
                   evas_event_default_flags_set(ee->evas, ipc->event_flags);
                   _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
                   evas_event_feed_key_up(ee->evas, ipc->keyname, ipc->key, ipc->string, ipc->compose, ipc->timestamp, NULL);
                   evas_event_default_flags_set(ee->evas, flags);
                }
           }
         break;
      case OP_EV_KEY_DOWN:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Key_Down))
           {
              if ((e->data) && (e->size > 0) &&
                  (((unsigned char *)e->data)[e->size - 1] == 0))
                {
                   Ipc_Data_Ev_Key_Down *ipc = e->data;
                   Evas_Event_Flags flags;

                   STRGET(keyname);
                   STRGET(key);
                   STRGET(string);
                   STRGET(compose);
                   flags = evas_event_default_flags_get(ee->evas);
                   evas_event_default_flags_set(ee->evas, ipc->event_flags);
                   _ecore_evas_modifiers_locks_mask_set(ee->evas, ipc->mask);
                   evas_event_feed_key_down(ee->evas, ipc->keyname, ipc->key, ipc->string, ipc->compose, ipc->timestamp, NULL);
                   evas_event_default_flags_set(ee->evas, flags);
                }
           }
         break;
      case OP_EV_HOLD:
         if (e->size >= (int)sizeof(Ipc_Data_Ev_Hold))
           {
              Ipc_Data_Ev_Hold *ipc = e->data;
              Evas_Event_Flags flags;

              flags = evas_event_default_flags_get(ee->evas);
              evas_event_default_flags_set(ee->evas, ipc->event_flags);
              evas_event_feed_hold(ee->evas, ipc->hold, ipc->timestamp, NULL);
              evas_event_default_flags_set(ee->evas, flags);
           }
         break;
      case OP_PROFILE_CHANGE_REQUEST:
        if ((e->data) && (e->size > 0) &&
            (((unsigned char *)e->data)[e->size - 1] == 0))
          {
             _ecore_evas_window_profile_free(ee);
             ee->prop.profile.name = (char *)eina_stringshare_add(e->data);

             if (ee->func.fn_state_change)
               ee->func.fn_state_change(ee);

             extn->profile.done = EINA_TRUE;
           }
         break;
      case OP_MSG:
         if ((e->data) && (e->size > 0))
           {
              //ref = msg_domain
              //ref_to = msg_id
              if (ee->func.fn_msg_parent_handle)
                {
                   INF("Message parent handle: ref=%d to=%d size=%d", e->ref, e->ref_to, e->size);
                   ee->func.fn_msg_parent_handle(ee, e->ref, e->ref_to, e->data, e->size);
                }
           }
         break;
      default:
         break;
     }
   return ECORE_CALLBACK_PASS_ON;
}

static void
_ecore_evas_extn_socket_alpha_set(Ecore_Evas *ee, int alpha)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;
   Eina_List *l;
   Ecore_Ipc_Client *client;

   if (((ee->alpha) && (alpha)) || ((!ee->alpha) && (!alpha))) return;
   ee->alpha = alpha;

   extn = bdata->data;
   if (extn)
     {
        Evas_Engine_Info_Buffer *einfo;

        einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             if (ee->alpha)
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
             else
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
             evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
             evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
        EINA_LIST_FOREACH(extn->ipc.clients, l, client)
          {
             int i;

             for (i = 0; i < NBUF; i++)
               {
                  const char *lock;

                  ecore_ipc_client_send(client, MAJOR, OP_SHM_REF0,
                                        extn->svc.num, extn->b[i].num, i,
                                        extn->svc.name,
                                        strlen(extn->svc.name) + 1);
                  lock = _extnbuf_lock_file_get(extn->b[i].buf);
                  ecore_ipc_client_send(client, MAJOR, OP_SHM_REF1,
                                        ee->w, ee->h, i,
                                        lock, strlen(lock) + 1);
                  ecore_ipc_client_send(client, MAJOR, OP_SHM_REF2,
                                        ee->alpha, extn->svc.sys, i,
                                        NULL, 0);
               }
          }
     }
}

static void
_ecore_evas_extn_socket_profile_set(Ecore_Evas *ee, const char *profile)
{
   _ecore_evas_window_profile_free(ee);
   ee->prop.profile.name = NULL;

   if (profile)
     {
        ee->prop.profile.name = (char *)eina_stringshare_add(profile);

        if (ee->func.fn_state_change)
          ee->func.fn_state_change(ee);
     }
}

static void
_ecore_evas_extn_socket_available_profiles_set(Ecore_Evas *ee, const char **plist, int n)
{
   int i;
   _ecore_evas_window_available_profiles_free(ee);
   ee->prop.profile.available_list = NULL;

   if ((plist) && (n >= 1))
     {
        ee->prop.profile.available_list = calloc(n, sizeof(char *));
        if (ee->prop.profile.available_list)
          {
             for (i = 0; i < n; i++)
                ee->prop.profile.available_list[i] = (char *)eina_stringshare_add(plist[i]);
             ee->prop.profile.count = n;

             if (ee->func.fn_state_change)
               ee->func.fn_state_change(ee);
          }
     }
}

static void
_ecore_evas_extn_socket_msg_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Extn *extn;
   Eina_List *l;
   Ecore_Ipc_Client *client;

   extn = bdata->data;
   if (extn)
     {
        EINA_LIST_FOREACH(extn->ipc.clients, l, client)
           ecore_ipc_client_send(client, MAJOR, OP_MSG_PARENT,
                                 msg_domain, msg_id, 0,
                                 data,
                                 size);
     }
}

static const Ecore_Evas_Engine_Func _ecore_extn_socket_engine_func =
{
   _ecore_evas_extn_free,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_socket_resize,
   _ecore_evas_socket_move_resize,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_extn_socket_alpha_set,
   NULL, //transparent
   _ecore_evas_extn_socket_available_profiles_set,
   _ecore_evas_extn_socket_profile_set,

   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,

   _ecore_evas_extn_socket_render, // render
   NULL,  // screen_geometry_get
   NULL,  // screen_dpi_get
   NULL,
   _ecore_evas_extn_socket_msg_send,

   /* 1.8 abstractions */
   NULL, // pointer_xy_get
   NULL, // pointer_warp

   NULL, // wm_rot_preferred_rotation_set
   NULL, // wm_rot_available_rotations_set
   NULL, // wm_rot_manual_rotation_done_set
   NULL, // wm_rot_manual_rotation_done

   NULL  // aux_hints_set
};

EAPI Ecore_Evas *
ecore_evas_extn_socket_new_internal(int w, int h)
{
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas_Interface_Extn *iface;
   Ecore_Evas_Engine_Buffer_Data *bdata;
   Ecore_Evas *ee;
   int rmethod;

   rmethod = evas_render_method_lookup("buffer");
   if (!rmethod) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   bdata = calloc(1, sizeof(Ecore_Evas_Engine_Buffer_Data));
   if (!bdata)
     {
	free(ee);
	return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_extn_socket_engine_func;
   ee->engine.data = bdata;

   ee->driver = "extn_socket";

   iface = _ecore_evas_extn_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

   ee->rotation = 0;
   ee->visible = 0;
   ee->w = w;
   ee->h = h;
   ee->req.w = ee->w;
   ee->req.h = ee->h;
   ee->profile_supported = 1; /* to accept the profile change request from the client(plug) */

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.focused = EINA_FALSE;
   ee->prop.borderless = EINA_TRUE;
   ee->prop.override = EINA_TRUE;
   ee->prop.maximized = EINA_FALSE;
   ee->prop.fullscreen = EINA_FALSE;
   ee->prop.withdrawn = EINA_FALSE;
   ee->prop.sticky = EINA_FALSE;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        if (ee->alpha)
          einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
        else
          einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
        einfo->info.dest_buffer = NULL;
        einfo->info.dest_buffer_row_bytes = 0;
        einfo->info.use_color_key = 0;
        einfo->info.alpha_threshold = 0;
        einfo->info.func.new_update_region = NULL;
        einfo->info.func.free_update_region = NULL;
        einfo->info.func.switch_buffer = _ecore_evas_socket_switch;
        einfo->info.switch_data = ee;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }
   evas_key_modifier_add(ee->evas, "Shift");
   evas_key_modifier_add(ee->evas, "Control");
   evas_key_modifier_add(ee->evas, "Alt");
   evas_key_modifier_add(ee->evas, "Meta");
   evas_key_modifier_add(ee->evas, "Hyper");
   evas_key_modifier_add(ee->evas, "Super");
   evas_key_lock_add(ee->evas, "Caps_Lock");
   evas_key_lock_add(ee->evas, "Num_Lock");
   evas_key_lock_add(ee->evas, "Scroll_Lock");

   extn_ee_list = eina_list_append(extn_ee_list, ee);

   _ecore_evas_register(ee);

   return ee;
}

Eina_Bool
_ecore_evas_extn_socket_listen(Ecore_Evas *ee, const char *svcname, int svcnum, Eina_Bool svcsys)
{
   Extn *extn;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   extn = calloc(1, sizeof(Extn));
   if (!extn)
     {
        return EINA_FALSE;
     }
   else
     {
        Ecore_Ipc_Type ipctype = ECORE_IPC_LOCAL_USER;
        int i;
        int last_try = 0;

        ecore_ipc_init();
        extn->svc.name = eina_stringshare_add(svcname);
        extn->svc.num = svcnum;
        extn->svc.sys = svcsys;

        for (i = 0; i < NBUF; i++)
          {
             do
               {
                  extn->b[i].buf = _extnbuf_new(extn->svc.name, extn->svc.num,
                                                extn->svc.sys, last_try,
                                                ee->w, ee->h, EINA_TRUE);
                  if (extn->b[i].buf) extn->b[i].num = last_try;
                  last_try++;
                  if (last_try > 1024) break;
               }
             while (!extn->b[i].buf);

          }

        if (extn->b[extn->cur_b].buf)
          bdata->pixels = _extnbuf_data_get(extn->b[extn->cur_b].buf,
                                            NULL, NULL, NULL);
        else
          {
             eina_stringshare_del(extn->svc.name);
             free(extn);
             ecore_ipc_shutdown();
             return EINA_FALSE;
          }

        if (extn->svc.sys) ipctype = ECORE_IPC_LOCAL_SYSTEM;
        extn->ipc.server = ecore_ipc_server_add(ipctype,
                                                (char *)extn->svc.name,
                                                extn->svc.num, ee);
        if (!extn->ipc.server)
          {
             for (i = 0; i < NBUF; i++)
               {
                  if (extn->b[i].buf) _extnbuf_free(extn->b[i].buf);
                  if (extn->b[i].obuf) _extnbuf_free(extn->b[i].obuf);
                  if (extn->b[i].base) eina_stringshare_del(extn->b[i].base);
                  if (extn->b[i].lock) eina_stringshare_del(extn->b[i].lock);
                  extn->b[i].buf = NULL;
                  extn->b[i].obuf = NULL;
                  extn->b[i].base = NULL;
                  extn->b[i].lock = NULL;
               }
             eina_stringshare_del(extn->svc.name);
             free(extn);
             ecore_ipc_shutdown();
             return EINA_FALSE;
          }
        bdata->data = extn;
        extn->ipc.handlers = eina_list_append
           (extn->ipc.handlers,
            ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_ADD,
                                    _ipc_client_add, ee));
        extn->ipc.handlers = eina_list_append
           (extn->ipc.handlers,
            ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DEL,
                                    _ipc_client_del, ee));
        extn->ipc.handlers = eina_list_append
           (extn->ipc.handlers,
            ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DATA,
                                    _ipc_client_data, ee));
     }
   return EINA_TRUE;
}

static Ecore_Evas_Interface_Extn *
_ecore_evas_extn_interface_new(void)
{
   Ecore_Evas_Interface_Extn *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Extn));
   if (!iface) return NULL;

   iface->base.name = interface_extn_name;
   iface->base.version = interface_extn_version;

   iface->connect = _ecore_evas_extn_plug_connect;
   iface->listen = _ecore_evas_extn_socket_listen;

   return iface;
}

EINA_MODULE_INIT(_ecore_evas_extn_module_init);
EINA_MODULE_SHUTDOWN(_ecore_evas_extn_module_shutdown);
