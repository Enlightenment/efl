//Compile with:
//gcc -o location_example_01 location_example_01.c -g `pkg-config --cflags --libs elementary elocation`

#include <Elementary.h>
#ifdef ELM_ELOCATION
#include <Elocation.h>
#endif

static Evas_Object *label, *win;

#ifdef ELM_ELOCATION
static void
_print_position(Elocation_Position *position)
{
   char buffer[1024];

   if (!position) return;
   snprintf(buffer, sizeof(buffer),
            "<b>GeoClue position reply with data from timestamp</b> %i<br/>"
            "<b>Latitude:</b> %f<br/>"
            "<b>Longitude:</b> %f<br/>"
            "<b>Altitude:</b> %f<br/>"
            "<b>Accuracy level:</b> %i<br/>"
            "<b>Accuracy horizontal:</b> %f<br/>"
            "<b>Accuracy vertical:</b> %f",
            position->timestamp, position->latitude, position->longitude,
            position->altitude, position->accur->level,
            position->accur->horizontal, position->accur->vertical);
   elm_object_text_set(label, buffer);
}

static Eina_Bool
_position_changed(void *data, int ev_type, void *event)
{
   Elocation_Position *position;

   position = event;
   _print_position(position);
   return ECORE_CALLBACK_DONE;
}
#endif

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
#ifdef ELM_ELOCATION
   Elocation_Address *address;
   Elocation_Position *position;
#endif

   /* The program will proceed only if Ewebkit library is available. */
   if (elm_need_elocation() == EINA_FALSE)
     return -1;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("elocation", "Elocation example");
   elm_win_autodel_set(win, EINA_TRUE);

   label = elm_label_add(win);
   elm_label_line_wrap_set(label, ELM_WRAP_CHAR);
   elm_object_text_set(label, "Getting location ...");
   evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_ALWAYS);
   evas_object_resize(label, 600, 480);
   evas_object_show(label);

#ifdef ELM_ELOCATION
   address = elocation_address_new();
   position = elocation_position_new();

   ecore_event_handler_add(ELOCATION_EVENT_POSITION, _position_changed, NULL);

   elocation_position_get(position);
   _print_position(position);
#endif

   evas_object_resize(win, 600, 480);
   evas_object_show(win);

   elm_run();

#ifdef ELM_ELOCATION
   elocation_position_free(position);
   elocation_address_free(address);
#endif

   return 0;
}
ELM_MAIN()
