#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

static void
_clicked(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   printf("Button is clicked!!!\n");
}

static void
_pressed(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   printf("Button is pressed!!!\n");
}

static void
_unpressed(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   printf("Button is unpressed!!!\n");
}

void
test_ui_button(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx, *btn;
   char buf[PATH_MAX];

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Button"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE)
                );

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));

   // Text Only Button
   efl_add(EFL_UI_BUTTON_CLASS, bx,
           efl_text_set(efl_added, "Text"),
           efl_pack(bx, efl_added),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _clicked, NULL),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_PRESSED, _pressed, NULL),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_UNPRESSED, _unpressed, NULL)
          );

   // Icon Only Button
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   btn = efl_add(EFL_UI_BUTTON_CLASS, bx,
                 efl_pack(bx, efl_added),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _clicked, NULL),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_PRESSED, _pressed, NULL),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_UNPRESSED, _unpressed, NULL)
                );
   efl_add(EFL_UI_IMAGE_CLASS, btn,
           efl_file_set(efl_added, buf),
           efl_content_set(btn, efl_added)
          );

   // Text + Icon Button
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   btn = efl_add(EFL_UI_BUTTON_CLASS, bx,
                 efl_text_set(efl_added, "Text + Icon"),
                 efl_pack(bx, efl_added),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _clicked, NULL),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_PRESSED, _pressed, NULL),
                 efl_event_callback_add(efl_added, EFL_UI_EVENT_UNPRESSED, _unpressed, NULL)
                );
   efl_add(EFL_UI_IMAGE_CLASS, btn,
           efl_file_set(efl_added, buf),
           efl_content_set(btn, efl_added)
          );

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320,  400));
}

