#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
_win_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *socket_win = data;
   evas_object_del(socket_win);
}

static void
fill(Evas_Object *win, Eina_Bool do_bg)
{
   Evas_Object *bg, *sc, *bx, *ic, *bb, *av, *en;
   char buf[PATH_MAX];

   if (do_bg)
     {
        bg = elm_bg_add(win);
        elm_win_resize_object_add(win, bg);
        evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_show(bg);
     }

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(en, "This is a single line");
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_single_line_set(en, EINA_TRUE);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(en, "Entry 2");
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_single_line_set(en, EINA_TRUE);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_resizable_set(ic, 0, 0);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 3");
   elm_object_part_text_set(bb, "info", "10:32 4/11/2008");
   elm_object_part_content_set(bb, "icon", ic);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   av = elm_entry_add(win);
   elm_entry_anchor_hover_style_set(av, "popout");
   elm_entry_anchor_hover_parent_set(av, win);
   elm_object_text_set(av,
			   "Hi there. This is the most recent message in the "
			   "list of messages. It has one <a href=tel:+614321234>+61 432 1234</a> "
			   "(phone number) to click on.");
   elm_object_content_set(bb, av);
   evas_object_show(av);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 2");
   elm_object_part_text_set(bb, "info", "7:16 27/10/2008");
   elm_object_part_content_set(bb, "icon", ic);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   av = elm_entry_add(win);
   elm_entry_anchor_hover_style_set(av, "popout");
   elm_entry_anchor_hover_parent_set(av, win);
   elm_object_text_set(av,
			   "Hey what are you doing? This is the second last message "
			   "Hi there. This is the most recent message in the "
			   "list. It's a longer one so it can wrap more and "
			   "contains a <a href=contact:john>John</a> contact "
			   "link in it to test popups on links. The idea is that "
			   "all SMS's are scanned for things that look like phone "
			   "numbers or names that are in your contacts list, and "
			   "if they are, they become clickable links that pop up "
			   "a menus of obvious actions to perform on this piece "
			   "of information. This of course can be later explicitly "
			   "done by links maybe running local apps or even being "
			   "web URL's too that launch the web browser and point it "
			   "to that URL. <item relsize=16x16 vsize=full href=emoticon/omg></item>");
   elm_object_content_set(bb, av);
   evas_object_show(av);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_resizable_set(ic, 0, 0);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 1");
   elm_object_part_text_set(bb, "info", "20:47 18/6/2008");
   elm_object_part_content_set(bb, "icon", ic);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   av = elm_entry_add(win);
   elm_entry_anchor_hover_style_set(av, "popout");
   elm_entry_anchor_hover_parent_set(av, win);
   elm_object_text_set(av, "This is a short message. <item relsize=16x16 vsize=full href=emoticon/haha></item>");
   elm_object_content_set(bb, av);
   evas_object_show(av);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   elm_object_content_set(sc, bx);
   evas_object_show(bx);

   evas_object_show(sc);
}

void
test_win_socket(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *lb;
   Evas_Object *win_socket;

   /* for socket info window */
   win = elm_win_util_standard_add("win-socket", "Win Socket");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "<b>This is a small label</b>"
                       );
   evas_object_size_hint_weight_set(lb, 0.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "If you runs more than Window Plug Program<br/>"
                       "you can see each plug programs shared same<br/>"
                       "canvas<br/>"
                       );
   evas_object_size_hint_weight_set(lb, 0.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   evas_object_resize(win, 320, 300);

   evas_object_show(bx);
   evas_object_show(win);
   elm_win_resize_object_add(win, bx);

   /* for socket window */
   win_socket = elm_win_add(NULL, "Window Socket", ELM_WIN_SOCKET_IMAGE);

   if (!elm_win_socket_listen(win_socket, "ello", 0, EINA_FALSE))
     {
        printf("Fail to elm win socket listen \n");
        evas_object_del(win_socket);
        evas_object_del(win);
        return;
     }
   elm_win_title_set(win_socket, "Window Socket");
   elm_win_autodel_set(win_socket, EINA_TRUE);

   fill(win_socket, EINA_TRUE);

   evas_object_resize(win_socket, 400, 600);
   evas_object_show(win_socket);

   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _win_del,
                                  win_socket);
}
#endif
