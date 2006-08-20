#include "config.h"
#include "Ecore.h"
#ifdef BUILD_ECORE_JOB
#include "Ecore_Job.h"
#endif
#ifdef BUILD_ECORE_X
#include "Ecore_X.h"
#endif
#ifdef BUILD_ECORE_FB
#include "Ecore_Fb.h"
#endif
#ifdef BUILD_ECORE_EVAS
#include "Ecore_Evas.h"
#endif
#ifdef BUILD_ECORE_CON
#include "Ecore_Con.h"
#endif
#ifdef BUILD_ECORE_IPC
#include "Ecore_Ipc.h"
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* APP GLOBALS */
double start_time = 0;

int
handler_signal_exit(void *data, int ev_type, void *ev)
{
   Ecore_Event_Signal_Exit *e;
   
   e = ev;
   if (e->interrupt) printf("exit: interrupt\n");
   if (e->quit)      printf("exit: quit\n");
   if (e->terminate) printf("exit: terminate\n");
   ecore_main_loop_quit();
   return 1;
}






int
handler_ipc_client_add(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_IPC
   Ecore_Ipc_Event_Client_Add *e;
   
   e = event;
   printf("!!! client %p connected to server!\n", e->client);
   return 1;
#endif   
}

int
handler_ipc_client_del(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_IPC
   Ecore_Ipc_Event_Client_Del *e;
   
   e = event;
   printf("!!! client %p disconnected from server!\n", e->client);
   return 1;
#endif   
}


int
handler_ipc_client_data(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_IPC
   Ecore_Ipc_Event_Client_Data *e;
   
   e = event;
   printf("!!! client sent: [%i] [%i] (%i) \"%s\"\n", e->major, e->minor, e->size, (char*)e->data);
   ecore_ipc_client_send(e->client, 1, 2, 0, 0, 0, "ABC", 4);
   /* we can disconnect a client like this: */
   /* ecore_ipc_client_del(e->client); */
   /* or we can end a server by: */
   /* ecore_ipc_server_del(ecore_ipc_client_server_get(e->client)); */
   return 1;
#endif   
}


int
handler_ipc_server_add(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_IPC
   Ecore_Ipc_Event_Server_Add *e;
   
   e = event;
   printf("!!! client made successful connect to server %p!\n", e->server);
   return 1;
#endif
}

int
handler_ipc_server_del(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_IPC
   Ecore_Ipc_Event_Server_Del *e;
   
   e = event;
   printf("!!! server went away!\n");
   /* clean up our server connection since it went away */
   ecore_ipc_server_del(e->server);
   return 1;
#endif   
}

int
handler_ipc_server_data(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_IPC
   Ecore_Ipc_Event_Server_Data *e;
   static int count = 0;
   
   e = event;
   printf("!!! server sent: [%i] [%i] (%i) \"%s\"\n", e->major, e->minor, e->size, (char*)e->data);
   ecore_ipc_server_send(e->server, 3, 4, 0, 0, 0, "EFG", 4);
   count++;
   if (count > 4)
     {
	printf("!!! go & disconnect from server!\n");
	ecore_ipc_server_del(e->server);
     }
   return 1;
#endif   
}

/**** ECORE_CON TEST CODE */
int
handler_client_add(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_CON
   Ecore_Con_Event_Client_Add *e;
   
   e = event;
   printf("!!! client %p connected to server!\n", e->client);
   return 1;
#endif   
}

int
handler_client_del(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_CON
   Ecore_Con_Event_Client_Del *e;
   
   e = event;
   printf("!!! client %p disconnected from server!\n", e->client);
   return 1;
#endif   
}


int
handler_client_data(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_CON
   Ecore_Con_Event_Client_Data *e;
   
   e = event;
   printf("!!! client sent: \"%s\"\n", (char*)e->data);
   ecore_con_client_send(e->client, "ABC", 4);
   /* we can disconnect a client like this: */
   /* ecore_con_client_del(e->client); */
   /* or we can end a server by: */
   /* ecore_con_server_del(ecore_con_client_server_get(e->client)); */
   return 1;
#endif   
}


int
handler_server_add(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_CON
   Ecore_Con_Event_Server_Add *e;
   
   e = event;
   printf("!!! client made successful connect to server %p!\n", e->server);
   return 1;
#endif   
}

int
handler_server_del(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_CON
   Ecore_Con_Event_Server_Del *e;
   
   e = event;
   printf("!!! server went away!\n");
   /* clean up our server connection since it went away */
   ecore_con_server_del(e->server);
   return 1;
#endif   
}

int
handler_server_data(void *data, int type, void *event)
{
#ifdef BUILD_ECORE_CON
   Ecore_Con_Event_Server_Data *e;
   static int count = 0;
   
   e = event;
   printf("!!! server sent: \"%s\"\n", (char*)e->data);
   ecore_con_server_send(e->server, "EFG", 4);
   count++;
   if (count > 4)
     {
	printf("!!! go & disconnect from server!\n");
	ecore_con_server_del(e->server);
     }
   return 1;
#endif   
}


/* NB: also tests ECORE_JOB */
#ifdef BUILD_ECORE_JOB
void
job_call(void *data)
{
   printf("!! Job done \"%s\"!\n", (char *)data);
}
#endif

int
idle_enterer(void *data)
{
   printf("-------> Entering idle %3.3f\n", ecore_time_get() - start_time);
   return 1;
}

int
idler(void *data)
{
   printf("oo Idler %3.3f\n", ecore_time_get() - start_time);
   return 1;
}

int
timer(void *data)
{
   printf("Q- Timer tick %3.8f\n", ecore_time_get() - start_time);
   /* test ecore_job */
#ifdef BUILD_ECORE_JOB   
   ecore_job_add(job_call, "1");
   ecore_job_add(job_call, "2");
   ecore_job_add(job_call, "3");
#endif   
   return 1;
}

void
setup_ecore_test(void)
{
   ecore_idle_enterer_add(idle_enterer, NULL);
/*   ecore_idler_add(idler, NULL); */
   ecore_timer_add(2.0, timer, NULL);
}

#ifdef BUILD_ECORE_X
/**** ECORE_X TEST CODE */

Ecore_X_Window win = 0;

int
handler_x_key_down(void *data, int type, void *event)
{
   Ecore_X_Event_Key_Down *e;
   
   e = event;
   printf("Key down %s\n", e->keyname);
   return 1;
}

int
handler_x_key_up(void *data, int type, void *event)
{
   Ecore_X_Event_Key_Up *e;
   
   e = event;
   printf("Key up %s\n", e->keyname);
   return 1;
}

int
handler_x_mouse_button_down(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Button_Down *e;
   
   e = event;
   printf("Mouse down %i [%i][%i]\n", e->button, e->double_click, e->triple_click);
   return 1;
}

int
handler_x_mouse_button_up(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Button_Up *e;
   
   e = event;
   printf("Mouse up %i\n", e->button);
   return 1;
}

int
handler_x_mouse_move(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Move *e;
   
   e = event;
   printf("Mouse move to %i %i\n", e->x, e->y);
   return 1;
}

int
handler_x_mouse_in(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_In *e;
   
   e = event;
   printf("Mouse in\n");
   return 1;
}

int
handler_x_mouse_out(void *data, int type, void *event)
{
   Ecore_X_Event_Mouse_Out *e;
   
   e = event;
   printf("Mouse out\n");
   return 1;
}

int
handler_x_window_focus_in(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Focus_In *e;
   
   e = event;
   printf("Focus in\n");
   return 1;
}

int
handler_x_window_focus_out(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Focus_Out *e;
   
   e = event;
   printf("Focus out\n");
   return 1;
}

int
handler_x_window_damage(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Damage *e;
   
   e = event;
   printf("Damage %i %i, %ix%i\n", e->x, e->y, e->w, e->h);
   return 1;
}

int
handler_x_window_destroy(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Destroy *e;
   
   e = event;
   printf("Destroy\n");
   ecore_main_loop_quit();   
   return 1;
}

int
handler_x_window_configure(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Configure *e;
   const int                       desktop = 0;
   
   e = event;
   printf("Configure %i %i, %ix%i\n", e->x, e->y, e->w, e->h);
   printf("Switching desktops to %d\n", desktop);
   ecore_x_netwm_desktop_request_send(e->win, 0, desktop);
   return 1;
}

int
handler_x_window_delete_request(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Delete_Request *e;
   
   e = event;
   printf("Delete Request\n");
   ecore_main_loop_quit();   
   return 1;
}

int
handler_x_window_prop_title_change(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Prop_Title_Change *e;
   
   e = event;
   if (e->title)
     printf("Title change to \"%s\"\n", e->title);
   else
     printf("Title deleted\n");
   return 1;
}

int
handler_x_window_prop_visible_title_change(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Prop_Visible_Title_Change *e;
   
   e = event;
   if (e->title)
     printf("Visible title change to \"%s\"\n", e->title);
   else
     printf("Visible title deleted\n");
   return 1;
}

int
handler_x_window_prop_icon_name_change(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Prop_Icon_Name_Change *e;
   
   e = event;
   if (e->name)
     printf("Icon name change to \"%s\"\n", e->name);
   else
     printf("Icon name deleted\n");
   return 1;
}

int
handler_x_window_prop_visible_icon_name_change(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change *e;
   
   e = event;
   if (e->name)
     printf("Visible icon name change to \"%s\"\n", e->name);
   else
     printf("Visible icon name deleted\n");
   return 1;
}

int
handler_x_window_prop_client_machine_change(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Prop_Client_Machine_Change *e;
   
   e = event;
   if (e->name)
     printf("Client machine change to \"%s\"\n", e->name);
   else
     printf("Client machine deleted\n");
   return 1;
}

int
handler_x_window_prop_pid_change(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Prop_Pid_Change *e;
   
   e = event;
   if (e->pid)
     {
        printf("Pid change to \"%d\" ", e->pid);
	if (e->pid == getpid())
		printf("correct.\n");
	else
		printf("INCORRECT!\n");
     }
   else
     printf("Pid deleted\n");
   return 1;
}

int
handler_x_window_prop_name_class_change(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Prop_Name_Class_Change *e;
   
   e = event;
   if ((e->name) && (e->clas))
     printf("Name & Class change to \"%s\".\"%s\"\n", e->name, e->clas);
   else
     printf("Name & Class deleted\n");
   return 1;
}

void
setup_ecore_x_test(void)
{
   char *tmp;
   int pid;
   unsigned int desktop;

   win = ecore_x_window_new(0, 0, 0, 120, 60);
   ecore_x_netwm_name_set(win, "Ecore Test Program");
   ecore_x_icccm_title_set(win, "Ecore Test Program");
   //printf("Title currently: %s\n", tmp);
#if 0
   /* Visibile title should be set by the wm */
   tmp = ecore_x_netwm_visible_name_get(win);
   if (!tmp)
     {
        printf("No visible title, setting it to Ecore ... Program\n");
	ecore_x_window_prop_visible_title_set(win, "Ecore ... Program");
        tmp = ecore_x_window_prop_visible_title_get(win);
     }
   printf("Visible title: %s\n", tmp);
   free(tmp);
#endif
   ecore_x_netwm_icon_name_get(win, &tmp);
   if (!tmp)
     {
        printf("No icon name, setting it to Ecore_Test\n");
	ecore_x_netwm_icon_name_set(win, "Ecore_Test");
        ecore_x_netwm_icon_name_get(win, &tmp);
     }
   printf("Icon Name: %s\n", tmp);
   free(tmp);
#if 0
   /* Visibile icon should be set by the wm */
   tmp = ecore_x_window_prop_visible_icon_name_get(win);
   if (!tmp)
     {
        printf("No visible icon name, setting it to Ecore\n");
	ecore_x_window_prop_visible_icon_name_set(win, "Ecore");
        tmp = ecore_x_window_prop_visible_icon_name_get(win);
     }
   printf("Visible icon Name: %s\n", tmp);
   free(tmp);
#endif
   tmp = ecore_x_icccm_client_machine_get(win);
   if (tmp)
     {
        printf("Client machine: %s\n", tmp);
        free(tmp);
     }
   ecore_x_netwm_pid_get(win, &pid);
   printf("Pid: %d\n", pid);
   ecore_x_icccm_name_class_set(win, "ecore_test", "main");
   ecore_x_netwm_desktop_set(win, 1);
   ecore_x_netwm_desktop_get(win, &desktop);
   printf("Window on desktop %u\n", desktop);
   ecore_x_netwm_window_type_set(win, ECORE_X_WINDOW_TYPE_DIALOG);
   ecore_x_icccm_protocol_set(win, ECORE_X_WM_PROTOCOL_DELETE_REQUEST, 1);
   ecore_x_window_show(win);
   ecore_x_flush();
   
   ecore_event_handler_add(ECORE_X_EVENT_KEY_DOWN, handler_x_key_down, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_KEY_UP, handler_x_key_up, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_DOWN, handler_x_mouse_button_down, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_UP, handler_x_mouse_button_up, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_MOUSE_MOVE, handler_x_mouse_move, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_MOUSE_IN, handler_x_mouse_in, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_MOUSE_OUT, handler_x_mouse_out, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, handler_x_window_focus_in, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, handler_x_window_focus_out, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DAMAGE, handler_x_window_damage, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DESTROY, handler_x_window_destroy, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CONFIGURE, handler_x_window_configure, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DELETE_REQUEST, handler_x_window_delete_request, NULL);
   /*
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE, handler_x_window_prop_title_change, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE, handler_x_window_prop_visible_title_change, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE, handler_x_window_prop_icon_name_change, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE, handler_x_window_prop_visible_icon_name_change, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE, handler_x_window_prop_name_class_change, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE, handler_x_window_prop_client_machine_change, NULL);
   ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE, handler_x_window_prop_pid_change, NULL);
   */

}
#endif

#ifdef BUILD_ECORE_EVAS
/* choose: TEST_X, TEST_FB */
#define TEST_X 

/**** ECORE_EVAS TEST CODE */

Ecore_Timer *anim_timer = NULL;
Ecore_Evas  *ee = NULL;
Evas        *evas = NULL;
Evas_Object *objects[64];

int
obj_timer(void *data)
{
   Evas_Object *o;
   Evas_Coord x, y;
   Evas_Coord w, h;
   Evas_Coord ow, oh;
   double t;
   int m, n;
   int i;
   
   t = ecore_time_get() - start_time;
   for (i = 0; i < (sizeof(objects) / sizeof(Evas_Object *)); i++)
     {
	o = objects[i];
	evas_output_viewport_get(evas_object_evas_get(o), NULL, NULL, &w, &h);
	evas_object_geometry_get(o, NULL, NULL, &ow, &oh);
	m = ((int)o / 36) & 0x3f;
	n = ((int)o / 763) & 0x3f;
	x = sin(t * (double)m / 12) * ((w - ow) / 2);
	y = sin(t * (double)n / 12) * ((h - oh) / 2);
	evas_object_move(o, (w / 2) - (ow / 2) + x, (h / 2) - (oh / 2) + y);
     }
   return 1;
}

void
del_req(Ecore_Evas *ee)
{
   int i;

   printf("request to go away... nice exit\n");
   for (i = 0; i < (sizeof(objects) / sizeof(Evas_Object *)); i++)
     objects[i] = NULL;
   ecore_timer_del(anim_timer);
   anim_timer = NULL;
   ecore_main_loop_quit();
}

static void
cb_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev;
   
   ev = event_info;
   if (!strcmp(ev->keyname, "Escape")) ecore_main_loop_quit();
   if (!strcmp(ev->keyname, "q")) ecore_main_loop_quit();
   if (!strcmp(ev->keyname, "Q")) ecore_main_loop_quit();
   if (!strcmp(ev->keyname, "f"))
     {
	if (!ecore_evas_fullscreen_get(ee))
	  ecore_evas_fullscreen_set(ee, 1);
	else
	  ecore_evas_fullscreen_set(ee, 0);
     }
   if (!strcmp(ev->keyname, "o"))
     {
	if (!ecore_evas_override_get(ee))
	  ecore_evas_override_set(ee, 1);
	else
	  ecore_evas_override_set(ee, 0);
     }
   if (!strcmp(ev->keyname, "r"))
     {
	int r;
	
	r = ecore_evas_rotation_get(ee);
	if      (r == 0)   r = 90;
	else if (r == 90)  r = 270;
	else if (r == 270) r = 0;
	ecore_evas_rotation_set(ee, r);
     }
   if (!strcmp(ev->keyname, "b"))
     {
	if (!ecore_evas_borderless_get(ee))
	  ecore_evas_borderless_set(ee, 1);
	else
	  ecore_evas_borderless_set(ee, 0);
     }
   if (!strcmp(ev->keyname, "d"))
     {
	if (!ecore_evas_avoid_damage_get(ee))
	  ecore_evas_avoid_damage_set(ee, 1);
	else
	  ecore_evas_avoid_damage_set(ee, 0);
     }
   if (!strcmp(ev->keyname, "s"))
     {
	if (!ecore_evas_shaped_get(ee))
	  ecore_evas_shaped_set(ee, 1);
	else
	  ecore_evas_shaped_set(ee, 0);
     }
#if 1 /* no data files shipped yet to test this */   
   if (!strcmp(ev->keyname, "p"))
     {
	char *fl;
	
	ecore_evas_cursor_get(ee, &fl, NULL, NULL, NULL);
	if (!fl)
	  ecore_evas_cursor_set(ee, "data/pointers/mouse_pointer.png", 1000000, 2, 2);
	else
	  ecore_evas_cursor_set(ee, NULL, 0, 0, 0);
     }
#endif   
}

int
setup_ecore_evas_test(void)
{
   Evas_Object *o;
   int i;
   
   /* create a new ecore wrapped evas canvas in X */
#ifdef TEST_X
   ee = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);
#endif
#ifdef TEST_FB
   ee = ecore_evas_fb_new(NULL, 270,  240, 320);
#endif
   if (!ee) return 0;
   ecore_evas_title_set(ee, "Ecore Evas Test");
   ecore_evas_name_class_set(ee, "ecore_test", "test_evas");   
   /* callback if clsoe button is pressed and win is asked to be deleted */
   ecore_evas_callback_delete_request_set(ee, del_req);
   
   /* get the actual evas ecore created for us */
   evas = ecore_evas_get(ee);

   /* image cache set (in bytes) */
   evas_image_cache_set(evas, 0 * 1024);
   /* font cache set (in bytes) */
   evas_font_cache_set(evas, 0 * 1024);
   /* add a directory to look in for fonts */
   evas_font_path_append(evas, "./");
   
   /* set up a big white rectangle on the bg */
   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 100000, 100000);
   evas_object_layer_set(o, -1000000);
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, cb_key_down, ee);
   evas_object_focus_set(o, 1);
   evas_object_show(o);
	
   /* make a bunch of rectangles */
   for (i = 0; i < (sizeof(objects) / sizeof(Evas_Object *)); i++)
     {
	o = evas_object_rectangle_add(evas);
	evas_object_move(o, 10, 10);
	evas_object_pass_events_set(o, 1);
	evas_object_color_set(o, 
			      ((int)o) & 0xff, 
			      ((int)o / 30) & 0xff, 
			      ((int)o / 65) & 0xff, 
			      ((int)o / 156) & 0xff);
	evas_object_resize(o, 4 + (((int)o) % 100), 4 + (((int)o / 50) % 100));
	evas_object_show(o);
	objects[i] = o;
     }
   /* add a timer to animate them */
   anim_timer = ecore_timer_add(0.01, obj_timer, NULL);
   
   ecore_evas_show(ee);
   return 1;
}
#endif

/**** MAIN */
int
main(int argc, const char **argv)
{
   /* get the time the program started at */
   start_time = ecore_time_get();

   /* init ecore */
   ecore_init();
   /* tell ecore what our arguments are */
   ecore_app_args_set(argc, argv);

#if 1
   /* setup to test ecore module basics */
   setup_ecore_test();
#endif

#ifdef BUILD_ECORE_CON 
#if 0   
   /* init ecore_con */
   ecore_con_init();
     {
	Ecore_Con_Server *server;
	
/*	server = ecore_con_server_add(ECORE_CON_LOCAL_USER, "ecore_test", 0, NULL);*/
	server = ecore_con_server_add(ECORE_CON_LOCAL_SYSTEM, "ecore_test", 0, NULL);
/*	server = ecore_con_server_add(ECORE_CON_REMOTE_SYSTEM, "localhost", 7654, NULL); */
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, handler_client_add, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, handler_client_del, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, handler_client_data, NULL);
	printf("create server %p\n", server);
     }
     {
	Ecore_Con_Server *server;
	
/*	server = ecore_con_server_connect(ECORE_CON_LOCAL_USER, "ecore_test", 0, NULL);*/
	server = ecore_con_server_connect(ECORE_CON_LOCAL_SYSTEM, "ecore_test", 0, NULL);
/*	server = ecore_con_server_connect(ECORE_CON_REMOTE_SYSTEM, "localhost", 7654, NULL); */
/*	server = ecore_con_server_connect(ECORE_CON_REMOTE_SYSTEM, "www.rasterman.com", 80, NULL); */
	ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, handler_server_add, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, handler_server_del, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, handler_server_data, NULL);
	printf("connect to server: %p\n", server);
	if (server)
	  {
	     char data[160 * 1024];
	     
	     strcpy(data, "BLAHPANTS!");
	     ecore_con_server_send(server, data, 160 * 1024);
	  }
     }
#endif   
#endif

#ifdef BUILD_ECORE_IPC
#if 1   
   /* init ecore_ipc */
   ecore_ipc_init();
     {
	Ecore_Ipc_Server *server;
	
	server = ecore_ipc_server_add(ECORE_IPC_LOCAL_SYSTEM, "ecore_ipc_test", 0, NULL);
/*	server = ecore_ipc_server_add(ECORE_IPC_REMOTE_SYSTEM, "localhost", 4567, NULL); */
	ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_ADD, handler_ipc_client_add, NULL);
	ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DEL, handler_ipc_client_del, NULL);
	ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DATA, handler_ipc_client_data, NULL);
	printf("create ipc server %p\n", server);
     }
     {
	Ecore_Ipc_Server *server;
	
	server = ecore_ipc_server_connect(ECORE_IPC_LOCAL_SYSTEM, "ecore_ipc_test", 0, NULL);
/*	server = ecore_ipc_server_connect(ECORE_IPC_REMOTE_SYSTEM, "localhost", 4567, NULL); */
	ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_ADD, handler_ipc_server_add, NULL);
	ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DEL, handler_ipc_server_del, NULL);
	ecore_event_handler_add(ECORE_IPC_EVENT_SERVER_DATA, handler_ipc_server_data, NULL);
	printf("connect to ipc server: %p\n", server);
	if (server)
	  {
	     char data[160 * 1024];
	     
	     strcpy(data, "BLAHPANTS!");
	     ecore_ipc_server_send(server, 5, 6, 0, 0, 0, data, 160 * 1024);
	  }
     }
#endif   
#endif
   
#ifdef BUILD_ECORE_EVAS   
   /* init ecore_evas */
/*   if (!ecore_evas_init()) return -1; */
#endif
   
   /* setup a callback to handle a systsme signal to quit */
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, handler_signal_exit, NULL);
   
#ifdef BUILD_ECORE_X
   /* setup to test ecore_x module things */
   if (!ecore_x_init(NULL)) return -1;
   setup_ecore_x_test();
#endif

#ifdef BUILD_ECORE_EVAS
   /* setup to test ecore_evas module */
/*   if (!setup_ecore_evas_test()) return -1; */
#endif
   
   /* run the main loop */
   ecore_main_loop_begin();

#ifdef BUILD_ECORE_EVAS
   /* shut down ecore_evas */
   ecore_evas_shutdown();
#endif   
#ifdef BUILD_ECORE_IPC
   /* shut down ecore_ipc */
   ecore_ipc_shutdown();
#endif
#ifdef BUILD_ECORE_CON 
   /* shut down ecore_con */
   ecore_con_shutdown();
#endif
   /* shut down ecore */
   ecore_shutdown();
   return 0;
}
