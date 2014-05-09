/**
 * Ecore example illustrating the basics of ecore evas extn plug usage.
 *
 * For checking with ecore evas extn socket, please run with ecore_evas_extn_socket_example.
 *
 * @verbatim
 * gcc -o ecore_evas_extn_plug_example ecore_evas_extn_plug_example.c `pkg-config --libs --cflags ecore-evas`
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
// procotol version - change this as needed
#define MSG_DOMAIN_CONTROL_OBJECT 0x1004
#define MSG_ID_BG_COLOR 0x1005
#define MSG_ID_TEXT 0x1006

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
   Msg_Color *color = NULL;

   color = ecore_evas_data_get(ee, "color");
   if (color) free(color);
   ecore_main_loop_quit();
}

static void
_button_1_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;
   Msg_Color *color = NULL;
   int r = 0, g = 0, b = 0, a = 0;

   ee = data;
   color = ecore_evas_data_get(ee, "color");

   printf("Plug's button pressed\n");
   if (!color)
     {
        color = malloc(sizeof(Msg_Color));
        r = 255;
        g = 0;
        b = 0;
        a = 255;
	    ecore_evas_data_set(ee, "color", color);
     }
   else
     {
        r = ((color->r) + 100) % 255;
        g = ((color->g) + 100) % 255;
        b = ((color->b) + 100) % 255;
        a = color->a;
     }
   printf("Send color info (%x,%x,%x,%x)\n", r, g, b, a);

   color->r = r;
   color->g = g;
   color->b = b;
   color->a = a;

   ecore_evas_msg_parent_send(ee, MSG_DOMAIN_CONTROL_OBJECT, MSG_ID_BG_COLOR, color, sizeof(Msg_Color)); 
}

static void
_ecore_evas_msg_handle(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
   if (!data) return;
   printf("Receive msg from server msg_domain=%x msg_id=%x size=%d\n", msg_domain, msg_id, size);

   if (msg_domain == MSG_DOMAIN_CONTROL_OBJECT)
     {
        if (msg_id == MSG_ID_TEXT)
          {
             Evas_Object *text = NULL;
             char *txt = data;
             int len = 0;
             len = strlen(txt);
             printf("data len= (%d).\n", len);

             text = ecore_evas_data_get(ee, "text");
             if (text && (size == (int)strlen(txt)))
               {
                  printf("Receive msg is text (%s).\n", txt);
                  evas_object_text_text_set(text, txt);
               }
          }
     }
}

int
main(void)
{
   Ecore_Evas *ee, *ee_plug;
   Evas *canvas;
   Evas_Object *bg, *button1, *text, *noti_text;
   Evas_Object *plug;
   int w, h;
   int x1, x2, y; //for button position
   int plug_x, plug_y, plug_w = 0, plug_h = 0; //for button position

   if (ecore_evas_init() <= 0)
     return 1;

   w = 480;
   h = 500;
   x1 = 20;
   x2 = 170;
   y = 100;
   plug_x = 10;
   plug_y = y + (h / 4) + 20;
   plug_w = 460;
   plug_h = 200;

   ee = ecore_evas_new(NULL, 0, 0, w, h, NULL);
   ecore_evas_title_set(ee, "Ecore Evas Extn Plug Example");
   ecore_evas_show(ee);

   ecore_evas_callback_delete_request_set(ee, _on_delete);

   printf("Using %s engine! ee=%p\n", ecore_evas_engine_name_get(ee), ee);

   //create ecore evas to show info
   canvas = ecore_evas_get(ee);
   if (ecore_evas_ecore_evas_get(canvas) == ee)
     printf("Everything is sane!\n");

   bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(bg, 255, 0, 255, 255);
   evas_object_resize(bg, w, h);
   evas_object_show(bg);

   //button to change socket's bg
   button1 = evas_object_rectangle_add(canvas);
   evas_object_color_set(button1, 0, 255, 255, 255);
   evas_object_resize(button1, w/4, 100);
   evas_object_move(button1, x1, y);
   evas_object_show(button1);

   text = evas_object_text_add(canvas);
   evas_object_color_set(text, 0, 0, 0, 255);
   evas_object_text_style_set(text, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_font_set(text, "Sans", 15);
   evas_object_text_text_set(text, "Chagne bg!!");
   evas_object_move(text, x1 + 5, y + 10);
   evas_object_show(text);


   //button to send msg1 to socket
   bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(bg, 0, 255, 255, 255);
   evas_object_resize(bg, w/2, 100);
   evas_object_move(bg, x2, y);
   evas_object_show(bg);
   text = evas_object_text_add(canvas);
   evas_object_color_set(text, 0, 0, 0, 255);
   evas_object_text_style_set(text, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_font_set(text, "Sans", 15);
   evas_object_text_text_set(text, "No Message from server!!");
   evas_object_move(text, x2 + 5, y + 10);
   evas_object_show(text);

   //text to noti plug area
   noti_text = evas_object_text_add(canvas);
   evas_object_color_set(noti_text, 0, 0, 0, 255);
   evas_object_text_style_set(noti_text, EVAS_TEXT_STYLE_PLAIN);
   evas_object_text_font_set(noti_text, "Sans", 15);
   evas_object_text_text_set(noti_text, "Below is the plug area!!");
   evas_object_move(noti_text, x1, plug_y - 25);
   evas_object_show(noti_text);

   //create ecore evas extn plug(image object) show socket area
   plug = ecore_evas_extn_plug_new(ee);
   ecore_evas_data_set(ee, "plug", plug);
   if (!plug)
     {
        printf("Fail to create ecore extn plug!\n");
		return 0;
     }

   ee_plug = ecore_evas_object_ecore_evas_get(plug);
   ecore_evas_data_set(ee_plug, "text", text);

   if(!ecore_evas_extn_plug_connect(plug, "socket_exam_service", 0, EINA_FALSE))
     {
        printf("Fail to connect socket_exam_service!\n");
		return 0;
     }

   evas_object_event_callback_add(button1, EVAS_CALLBACK_MOUSE_UP, _button_1_up, ee_plug);
   //callback to deal with extn socket message
   ecore_evas_callback_msg_handle_set(ee_plug, _ecore_evas_msg_handle);

   ecore_evas_data_set(ee_plug, "text", text);

   evas_object_resize(plug, plug_w, plug_h);
   evas_object_move(plug, plug_x, plug_y);
   evas_object_show(plug);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();

   return 0;
}

