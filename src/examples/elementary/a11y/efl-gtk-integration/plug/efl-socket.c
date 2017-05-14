#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT
#include <Elementary.h>

static struct app_state {
     Evas_Object *entry;
     Elm_Atspi_Socket *socket;
     Elm_Atspi_Socket *proxy;
     Evas_Object *container;
} state;

static void a11y_init(void)
{
   if (state.socket)
     efl_del(state.socket);

   if (!elm_entry_is_empty(state.entry))
     {
        printf("Embedded plug: %s\n", elm_object_text_get(state.entry));
        state.socket = efl_add(ELM_ATSPI_SOCKET_CLASS, NULL);
        // Weird constructor
        state.proxy = efl_add(ELM_ATSPI_PROXY_CLASS, NULL, elm_atspi_proxy_id_constructor(efl_added, elm_object_text_get(state.entry)));

        // fires accessible parent should be set to eo_parent (???)
        elm_interface_atspi_accessible_parent_set(state.socket, state.container);
        // following line shuold be not necessary:

        elm_interface_atspi_accessible_parent_set(state.proxy, state.socket);
        elm_atspi_socket_embed(state.socket, state.proxy);
     }
}

static void
_connect_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   if (elm_config_atspi_mode_get())
     a11y_init();
   else
     printf("Atspi mode is not enabled. Adjust elementary configuration using elementary_config app\n");
}

static void
_create_layout(Evas_Object *win)
{
   Evas_Object *box = elm_box_add(win);
   evas_object_size_hint_expand_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);

   Evas_Object *label = elm_label_add(box);
   evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(label, "Past Plug Id here:");
   elm_box_pack_end(box, label);
   evas_object_show(label);

   Evas_Object *entry = elm_entry_add(box);
   elm_entry_single_line_set(entry, EINA_TRUE);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, entry);
   evas_object_show(entry);
   state.entry = entry;

   Evas_Object *button = elm_button_add(box);
   elm_object_text_set(button, "Connect");
   elm_box_pack_end(box, button);
   evas_object_smart_callback_add(button, "clicked", _connect_clicked_cb, NULL);
   evas_object_show(button);

   state.container = elm_label_add(box);
   evas_object_size_hint_align_set(state.container, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(state.container, "Embedded content goes here:");
   elm_box_pack_end(box, state.container);
   evas_object_show(state.container);

   evas_object_show(box);
}

int elm_main(int argc, char **argv)
{
   Evas_Object *win = elm_win_util_standard_add("Efl socket", "Efl socket");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   evas_object_resize(win, 300, 200);

   _create_layout(win);

   evas_object_show(win);
   elm_run();
   return 0;
}

ELM_MAIN();
