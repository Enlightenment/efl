//Compile with:
//gcc -o location_example_01 location_example_01.c -g `pkg-config --cflags --libs elementary elocation`
#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif
#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

#include <Elementary.h>
#ifdef ELM_ELOCATION
#include <Elocation.h>
#endif

static Evas_Object *pos_label, *addr_label, *box, *win;

#ifdef ELM_ELOCATION
static void
_print_position(Elocation_Position *position)
{
   char buffer[1024];

   if (!position) return;
   snprintf(buffer, sizeof(buffer),
            "<b>### Position Detail ###</b><br/>"
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
   elm_object_text_set(pos_label, buffer);
}

static void
_print_address(Elocation_Address *address)
{
   char buffer[1024];

   if (!address) return;
   snprintf(buffer, sizeof(buffer),
            "<b>### Address Detail ###</b><br/>"
            "<b>Address update with data from timestamp:</b> %i<br/>"
            "<b>Country:</b> %s<br/>"
            "<b>Countrycode:</b> %s<br/>"
            "<b>Locality:</b> %s<br/>"
            "<b>Postalcode:</b> %s<br/>"
            "<b>Region:</b> %s<br/>"
            "<b>Timezone:</b> %s<br/>"
            "<b>Accuracy level:</b> %i<br/>"
            "<b>Accuracy horizontal:</b> %f<br/>"
            "<b>Accuracy vertical:</b> %f",
            address->timestamp, address->country, address->countrycode,
            address->locality, address->postalcode, address->region,
            address->timezone, address->accur->level, address->accur->horizontal,
            address->accur->vertical);
   elm_object_text_set(addr_label, buffer);
}

static Eina_Bool
_position_changed(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   Elocation_Position *position;

   position = event;
   _print_position(position);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_address_changed(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   Elocation_Address *address;

   address = event;
   _print_address(address);

   return ECORE_CALLBACK_DONE;
}
#endif

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
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

   box = elm_box_add(win);
   elm_win_resize_object_add(win, box);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

   pos_label = elm_label_add(box);
   elm_label_line_wrap_set(pos_label, ELM_WRAP_CHAR);
   elm_object_text_set(pos_label, "Getting location ...");
   evas_object_size_hint_weight_set(pos_label, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(pos_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_label_slide_mode_set(pos_label, ELM_LABEL_SLIDE_MODE_ALWAYS);
   elm_box_pack_end(box, pos_label);
   evas_object_show(pos_label);
   elm_box_padding_set(box, 0, 50);

   addr_label = elm_label_add(box);
   elm_label_line_wrap_set(addr_label, ELM_WRAP_CHAR);
   elm_object_text_set(addr_label, "Getting location ...");
   evas_object_size_hint_weight_set(addr_label, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(addr_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_label_slide_mode_set(addr_label, ELM_LABEL_SLIDE_MODE_ALWAYS);
   elm_box_pack_end(box, addr_label);
   evas_object_show(addr_label);

#ifdef ELM_ELOCATION
   address = elocation_address_new();
   position = elocation_position_new();

   ecore_event_handler_add(ELOCATION_EVENT_POSITION, _position_changed, NULL);
   ecore_event_handler_add(ELOCATION_EVENT_ADDRESS, _address_changed, NULL);

   elocation_position_get(position);
   _print_position(position);

   elocation_address_get(address);
   _print_address(address);
#endif

   evas_object_show(box);
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
