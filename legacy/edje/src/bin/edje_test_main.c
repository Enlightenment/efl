#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include "Edje.h"

Ecore_Evas *ee;
Evas *evas;

Evas_Object *o_bg;
Evas_Object *o_edje;

void signal_cb(void *data, Evas_Object *o, const char *sig, const char *src);
void resize_cb(Ecore_Evas *ee);
void key_cb(void *data, Evas *e, Evas_Object *o, void *ei);
void message_cb(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg);

void
signal_cb(void *data, Evas_Object *o, const char *sig, const char *src)
{
   printf("EMIT: \"%s\" \"%s\"\n", sig, src);
}

void
resize_cb(Ecore_Evas *ee)
{
   Evas_Coord w, h;

   evas_output_viewport_get(evas, NULL, NULL, &w, &h);
   evas_object_resize(o_bg, w, h);
   evas_object_resize(o_edje, w, h);
}

void
key_cb(void *data, Evas *e, Evas_Object *o, void *ei)
{
   Evas_Event_Key_Down *ev;
   
   ev = ei;
   printf("KEY: %s\n", ev->keyname);
   if (!strcmp(ev->keyname, "Escape")) ecore_main_loop_quit();
   if (!strcmp(ev->keyname, "space"))
     {
	edje_object_message_send(o_edje, EDJE_MESSAGE_NONE,
				 77, NULL);
     }
   if (!strcmp(ev->keyname, "1"))
     {
	Edje_Message_String msg;
	
	msg.str = "A Test String";
	edje_object_message_send(o_edje, EDJE_MESSAGE_STRING,
				 101, &msg);
     }
   if (!strcmp(ev->keyname, "2"))
     {
	Edje_Message_Int msg;
	
	msg.val = 42;
	edje_object_message_send(o_edje, EDJE_MESSAGE_INT,
				 102, &msg);
     }
   if (!strcmp(ev->keyname, "3"))
     {
	Edje_Message_Float msg;
	
	msg.val = 3.14159;
	edje_object_message_send(o_edje, EDJE_MESSAGE_FLOAT,
				 103, &msg);
     }
   if (!strcmp(ev->keyname, "4"))
     {
	Edje_Message_String_Set *msg;
	
	msg = calloc(1,
		     sizeof(Edje_Message_String_Set) - 
		     sizeof(char *) + 
		     (7 * sizeof(char *)));
	msg->count = 7;
	msg->str[0] = "String 1";
	msg->str[1] = "String 2";
	msg->str[2] = "String 3";
	msg->str[3] = "String 4";
	msg->str[4] = "String 5";
	msg->str[5] = "String 6";
	msg->str[6] = "String 7";
	edje_object_message_send(o_edje, EDJE_MESSAGE_STRING_SET,
				 104, msg);
	free(msg);
     }
   if (!strcmp(ev->keyname, "5"))
     {
	Edje_Message_Int_Set *msg;
	
	msg = calloc(1,
		     sizeof(Edje_Message_Int_Set) - 
		     sizeof(int) + 
		     (7 * sizeof(int)));
	msg->count = 7;
	msg->val[0] = 42;
	msg->val[1] = 43;
	msg->val[2] = 44;
	msg->val[3] = 45;
	msg->val[4] = 46;
	msg->val[5] = 47;
	msg->val[6] = 48;
	edje_object_message_send(o_edje, EDJE_MESSAGE_INT_SET,
				 105, msg);
	free(msg);
     }
   if (!strcmp(ev->keyname, "6"))
     {
	Edje_Message_Float_Set *msg;
	
	msg = calloc(1,
		     sizeof(Edje_Message_Float_Set) - 
		     sizeof(double) + 
		     (7 * sizeof(double)));
	msg->count = 7;
	msg->val[0] = 1.2345;
	msg->val[1] = 7.7777;
	msg->val[2] = 3.1415;
	msg->val[3] = 9.8765;
	msg->val[4] = 7.8901;
	msg->val[5] = 3.4567;
	msg->val[6] = 6.7890;
	edje_object_message_send(o_edje, EDJE_MESSAGE_FLOAT_SET,
				 106, msg);
	free(msg);
     }
   if (!strcmp(ev->keyname, "7"))
     {
	Edje_Message_String_Int msg;
	
	msg.str = "A Test String and Int";
	msg.val = 42;
	edje_object_message_send(o_edje, EDJE_MESSAGE_STRING_INT,
				 107, &msg);
     }
   if (!strcmp(ev->keyname, "8"))
     {
	Edje_Message_String_Float msg;
	
	msg.str = "A Test String and Float";
	msg.val = 777.777;
	edje_object_message_send(o_edje, EDJE_MESSAGE_STRING_FLOAT,
				 108, &msg);
     }
   if (!strcmp(ev->keyname, "9"))
     {
	Edje_Message_String_Int_Set *msg;
	
	msg = calloc(1,
		     sizeof(Edje_Message_String_Int_Set) - 
		     sizeof(int) + 
		     (7 * sizeof(int)));
	msg->str = "A Test String and Int Set";
	msg->count = 7;
	msg->val[0] = 42;
	msg->val[1] = 43;
	msg->val[2] = 44;
	msg->val[3] = 45;
	msg->val[4] = 46;
	msg->val[5] = 47;
	msg->val[6] = 48;
	edje_object_message_send(o_edje, EDJE_MESSAGE_STRING_INT_SET,
				 109, msg);
	free(msg);
     }
   if (!strcmp(ev->keyname, "0"))
     {
	Edje_Message_String_Float_Set *msg;
	
	msg = calloc(1,
		     sizeof(Edje_Message_String_Float_Set) - 
		     sizeof(double) + 
		     (7 * sizeof(double)));
	msg->str = "A Test String and Float Set";
	msg->count = 7;
	msg->val[0] = 1.2345;
	msg->val[1] = 7.7777;
	msg->val[2] = 3.1415;
	msg->val[3] = 9.8765;
	msg->val[4] = 7.8901;
	msg->val[5] = 3.4567;
	msg->val[6] = 6.7890;
	edje_object_message_send(o_edje, EDJE_MESSAGE_STRING_FLOAT_SET,
				 110, msg);
	free(msg);
     }
}

void
message_cb(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg)
{
   printf("MESSAGE FROM OBJ, TYPE: %i, ID %i\n", type, id);
   if (type == EDJE_MESSAGE_NONE)
     {
     }
   else if (type == EDJE_MESSAGE_STRING)
     {
	Edje_Message_String *emsg;
	
	emsg = (Edje_Message_String *)msg;
	printf("  STRING: \"%s\"\n", emsg->str);
     }
   else if (type == EDJE_MESSAGE_INT)
     {
	Edje_Message_Int *emsg;
	
	emsg = (Edje_Message_Int *)msg;
	printf("  INT: %i\n", emsg->val);
     }
   else if (type == EDJE_MESSAGE_FLOAT)
     {
	Edje_Message_Float *emsg;
	
	emsg = (Edje_Message_Float *)msg;
	printf("  FLOAT: %f\n", emsg->val);
     }
   else if (type == EDJE_MESSAGE_STRING_SET)
     {
	Edje_Message_String_Set *emsg;
	int i;
	
	emsg = (Edje_Message_String_Set *)msg;
	for (i = 0; i < emsg->count; i++)
	  printf("  STR: %s\n", emsg->str[i]);
     }
   else if (type == EDJE_MESSAGE_INT_SET)
     {
	Edje_Message_Int_Set *emsg;
	int i;
	
	emsg = (Edje_Message_Int_Set *)msg;
	for (i = 0; i < emsg->count; i++)
	  printf("  INT: %i\n", emsg->val[i]);
     }
   else if (type == EDJE_MESSAGE_FLOAT_SET)
     {
	Edje_Message_Float_Set *emsg;
	int i;
	
	emsg = (Edje_Message_Float_Set *)msg;
	for (i = 0; i < emsg->count; i++)
	  printf("  FLOAT: %f\n", emsg->val[i]);
     }
   else if (type == EDJE_MESSAGE_STRING_INT)
     {
	Edje_Message_String_Int *emsg;
	
	emsg = (Edje_Message_String_Int *)msg;
	printf("  STR: %s\n", emsg->str);
	printf("  INT: %i\n", emsg->val);
     }
   else if (type == EDJE_MESSAGE_STRING_FLOAT)
     {
	Edje_Message_String_Float *emsg;
	
	emsg = (Edje_Message_String_Float *)msg;
	printf("  STR: %s\n", emsg->str);
	printf("  FLOAT: %f\n", emsg->val);
     }
   else if (type == EDJE_MESSAGE_STRING_INT_SET)
     {
	Edje_Message_String_Int_Set *emsg;
	int i;
	
	emsg = (Edje_Message_String_Int_Set *)msg;
	printf("  STR: %s\n", emsg->str);
	for (i = 0; i < emsg->count; i++)
	  printf("  INT: %i\n", emsg->val[i]);
     }
   else if (type == EDJE_MESSAGE_STRING_FLOAT_SET)
     {
	Edje_Message_String_Float_Set *emsg;
	int i;
	
	emsg = (Edje_Message_String_Float_Set *)msg;
	printf("  STR: %s\n", emsg->str);
	for (i = 0; i < emsg->count; i++)
	  printf("  INT: %f\n", emsg->val[i]);
     }
}

int
main(int argc, char **argv)
{
   Evas_Object *o;
   
   if (argc != 3)
     {
	printf("Usage: edje_test edje_file.edj part_to_load\n");
	exit(-1);
     }
   
   ecore_init();
   ecore_app_args_set(argc, (const char **)argv);
   ecore_evas_init();
   edje_init();

   if (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_SOFTWARE_X11))
     ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 240, 320);
   else
     {
	if (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_SOFTWARE_FB))
	  ee = ecore_evas_fb_new(NULL, 270, 240, 320);
     }
   if (!ee)
     {
	if (ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_SOFTWARE_FB))
	  ee = ecore_evas_fb_new(NULL, 270, 240, 320);
	if (!ee)
	  ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 240, 320);
	if (!ee)
	  {
	     fprintf(stderr, "Cannot create Canvas!\n");
	     exit(-1);
	  }
     }

   evas = ecore_evas_get(ee);
   evas_image_cache_set(evas, 8192 * 1024);
   evas_font_cache_set(evas, 512 * 1024);
   
   o = evas_object_rectangle_add(evas);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 240, 320);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, key_cb, NULL);
   evas_object_focus_set(o, 1);
   evas_object_show(o);
   o_bg = o;
   
   o = edje_object_add(evas);
   edje_object_signal_callback_add(o, "*", "*", signal_cb, NULL);
   edje_object_message_handler_set(o, message_cb, NULL);
   edje_object_file_set(o, argv[1], argv[2]);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 240, 320);
   evas_object_show(o);
   o_edje = o;

/*   
     {
	Evas_Coord mw, mh;
	
	printf("---EMIT SIGNALS\n");
	edje_object_signal_emit(o_edje, "icon_visible", "");
	printf("---PROCESS SIGNALS\n");
	edje_object_message_signal_process(o_edje);
	printf("---MIN CALC\n");
	edje_object_size_min_calc(o_edje, &mw, &mh);
	printf("---MIN %ix%i\n", mw, mh);
	evas_object_resize(o_edje, mw, mh);
     }
 */
   
   ecore_evas_callback_resize_set(ee, resize_cb);   
   ecore_evas_show(ee);
   
   ecore_main_loop_begin();

   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   return 0;
}
