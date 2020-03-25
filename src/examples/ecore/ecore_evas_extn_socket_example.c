/**
 * Ecore example illustrating the basics of ecore evas extn socket usage.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 * You can check functions of ecore extn socket if you use ecore extn plug together.
 *
 * @verbatim
 * gcc -o ecore_evas_extn_socket_example ecore_evas_extn_socket_example.c `pkg-config --libs --cflags evas ecore ecore-evas`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define EINA_UNUSED
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <unistd.h>

// protocol version - change this as needed
#define MSG_DOMAIN_CONTROL_OBJECT 0x1004
#define MSG_ID_BG_COLOR 0x1005
#define MSG_ID_TEXT 0x1006
static int num = 0;
typedef struct _Msg_Color Msg_Color;

struct _Msg_Color
{
   int r;
   int g;
   int b;
   int a;
};

static void
_on_delete(Ecore_Evas *ee)
{
   Ecore_Evas *ee_socket = NULL;
   char *text = NULL;
   ee_socket = ecore_evas_data_get(ee, "sock");
   if (ee_socket)
     {
        text = ecore_evas_data_get(ee_socket, "text");
        if (text) free(text);
     }
   ecore_main_loop_quit();
}

static void
_ecore_evas_msg_parent_handle(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
   printf("Receive msg from clien msg_domain=%x msg_id=%x size=%d\n", msg_domain, msg_id, size);
   if (!data) return;

   if (msg_domain == MSG_DOMAIN_CONTROL_OBJECT)
     {
        if (msg_id == MSG_ID_BG_COLOR)
          {
             Evas_Object *bg = NULL;
             Msg_Color *color = NULL;
             int r = 0, g = 0, b = 0, a = 0;

             bg = ecore_evas_data_get(ee, "bg");
             if (bg && (size == sizeof(Msg_Color)))
               {
                  color = data;
                  r = color->r;
                  g = color->g;
                  b = color->b;
                  a = color->a;
                  printf("Receive msg is color r=%x g=%x b=%x a=%x\n", r, g, b, a);
                  evas_object_color_set(bg, color->r, color->g, color->b, color->a);
               }
          }
     }
}

static void
_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;
   size_t len = 0;
   char *buf = NULL;

   num++;

   if (!ee) return;
   buf = ecore_evas_data_get(ee, "text");
   if (!buf) free(buf);

   buf = (char *)malloc(40);
   memset(buf, 0, 40);
   sprintf(buf, "Hello. I'm server!! number=%d!!", num);
   len = strlen(buf);
   ecore_evas_data_set(ee, "text", buf);

   printf("ecore extn socket txt = \"%s\" len=%zu.", buf, len);
   ecore_evas_msg_send(ee, MSG_DOMAIN_CONTROL_OBJECT, MSG_ID_TEXT, buf, len);
}

int
main(void)
{
   Ecore_Evas *ee;
   Ecore_Evas *ee_socket = NULL;
   Evas *canvas;
   Evas_Object *bg, *text;
   Evas_Object *sock_bg;
   int w, h;
   int socket_w, socket_h;

   if (ecore_evas_init() <= 0)
     return 1;

   w = 480;
   h = 200;

   ee = ecore_evas_new(NULL, 0, 0, w, h, NULL);
   ecore_evas_title_set(ee, "Ecore Evas Extn Socket Example");
   ecore_evas_show(ee);

   ecore_evas_callback_delete_request_set(ee, _on_delete);

   printf("Using %s engine! ee=%p\n", ecore_evas_engine_name_get(ee), ee);

   //create ecore evas to show info
   canvas = ecore_evas_get(ee);
   if (ecore_evas_ecore_evas_get(canvas) == ee)
     printf("Everything is sane!\n");

   bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(bg, 255, 255, 0, 255);
   evas_object_resize(bg, w, h);
   evas_object_show(bg);

   text = evas_object_text_add(canvas);
   evas_object_color_set(text, 0, 0, 0, 255);
   evas_object_text_style_set(text, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_font_set(text, "Sans", 15);
   evas_object_text_text_set(text, "1. Run ecore_evas_extn_plug_examples!!");
   evas_object_move(text, 40, 30);
   evas_object_show(text);

   text = evas_object_text_add(canvas);
   evas_object_color_set(text, 0, 0, 0, 255);
   evas_object_text_style_set(text, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_font_set(text, "Sans", 15);
   evas_object_text_text_set(text, "2. Press green rect to send msg to clients!!");
   evas_object_move(text, 40, 60);
   evas_object_show(text);

   bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(bg, 0, 255, 0, 255);
   evas_object_resize(bg, w/4, h/4);
   evas_object_move(bg, w/4, h/2);
   evas_object_show(bg);

   //create ecore evas extn socket
   socket_w = 460;
   socket_h = 250;

   ee_socket = ecore_evas_extn_socket_new(1, 1);
   ecore_evas_data_set(ee, "sock", ee_socket);

   if (!ee_socket)
     {
        printf("Fail to create ecore extn socket!\n");
        return 0;
     }

   if(!ecore_evas_extn_socket_listen(ee_socket, "socket_exam_service", 0, EINA_FALSE))
     {
        printf("Fail to listen socket_exam_service!\n");
		return 0;
     }
   ecore_evas_resize(ee_socket, socket_w, socket_h);
   ecore_evas_show(ee_socket);

   //callback to deal with cient extn's message
   ecore_evas_callback_msg_parent_handle_set(ee_socket, _ecore_evas_msg_parent_handle);
   //_mouse_up is function to send msg to client extn plug
   evas_object_event_callback_add(bg, EVAS_CALLBACK_MOUSE_UP, _mouse_up, ee_socket);

   canvas = ecore_evas_get(ee_socket);

   sock_bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(sock_bg, 0, 0, 255, 255);
   evas_object_resize(sock_bg, socket_w, socket_h);
   evas_object_move(sock_bg, 0, 0);
   evas_object_show(sock_bg);
   ecore_evas_data_set(ee_socket, "bg", sock_bg);

   ecore_evas_extn_socket_events_block_set(ee_socket, EINA_FALSE);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();

   return 0;
}
