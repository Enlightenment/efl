#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
aspect_fixed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ic = (Evas_Object *)data;
   elm_image_aspect_fixed_set(ic, elm_check_state_get(obj));
}

static void
fill_outside_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ic = (Evas_Object *)data;
   elm_image_fill_outside_set(ic, elm_check_state_get(obj));
}

static void
smooth_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *ic = (Evas_Object *)data;
   elm_image_smooth_set(ic, elm_check_state_get(obj));
}

static void
bt_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("preload-prescale", "Preload & Prescale");
   elm_win_autodel_set(win, EINA_TRUE);

   ic = elm_icon_add(win);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ic);
   snprintf(buf, sizeof(buf), "%s/images/insanely_huge_test_image.jpg",
            elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);

   elm_image_aspect_fixed_set(ic, EINA_FALSE);
   elm_image_preload_disabled_set(ic, EINA_TRUE);
   elm_image_prescale_set(ic, EINA_TRUE);
   evas_object_show(ic);

   evas_object_resize(win, 350, 350);
   evas_object_show(win);
}

void
test_icon(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *ic, *hbox, *tg, *bt;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("icon-test", "Icon Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   ic = elm_icon_add(box);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ic);
   evas_object_show(ic);

   hbox = elm_box_add(box);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   /* Test Aspect Fixed */
   tg = elm_check_add(hbox);
   elm_object_text_set(tg, "Aspect Fixed");
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", aspect_fixed_cb, ic);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   /* Test Fill Outside */
   tg = elm_check_add(hbox);
   elm_object_text_set(tg, "Fill Outside");
   evas_object_smart_callback_add(tg, "changed", fill_outside_cb, ic);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   /* Test Smooth */
   tg = elm_check_add(hbox);
   elm_object_text_set(tg, "Smooth");
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", smooth_cb, ic);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   /* Test Preload, Prescale */
   bt = elm_button_add(hbox);
   elm_object_text_set(bt, "Preload & Prescale");
   evas_object_smart_callback_add(bt, "clicked", bt_clicked, NULL);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   /* Set window icon too */
   ic = evas_object_image_filled_add(evas_object_evas_get(win));
   evas_object_image_load_scale_down_set(ic, 8);
   evas_object_image_file_set(ic, buf, NULL);
   elm_win_icon_object_set(win, ic);
   evas_object_show(ic);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

/* Test: Icon Transparent */
static void
icon_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("clicked!\n");
}

void
test_icon_transparent(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ic;
   char buf[PATH_MAX];
   int w, h;

   win = elm_win_add(NULL, "icon-transparent", ELM_WIN_BASIC);
   elm_win_title_set(win, "Icon Transparent");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_alpha_set(win, EINA_TRUE);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_object_size_get(ic, &w, &h);
   evas_object_size_hint_aspect_set(win, EVAS_ASPECT_CONTROL_BOTH, w, h);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_image_no_scale_set(ic, EINA_TRUE);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(ic, 0.5, 0.5);
   elm_win_resize_object_add(win, ic);
   evas_object_show(ic);

   evas_object_smart_callback_add(ic, "clicked", icon_clicked, NULL);

   evas_object_show(win);
}

/* Test: Icon Standard */
static char *
_gl_group_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return strdup(data);
}

static char *
_gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return strdup(data);
}

static Evas_Object *
_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *ic;
   const char *name = data;
   int size = elm_radio_value_get(evas_object_data_get(obj, "size_rdg"));

   if (!strcmp(part, "elm.swallow.icon"))
     {
        ic = elm_icon_add(obj);
        // evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
        elm_icon_standard_set(ic, name);
        if (size)
          evas_object_size_hint_min_set(ic, size, size);
        return ic;
     }

   return NULL;
}

static void
_standard_genlist_populate(Evas_Object *gl)
{
   Elm_Genlist_Item_Class *itc, *itc_g;
   Elm_Object_Item *git;

   itc_g = elm_genlist_item_class_new();
   itc_g->item_style = "group_index";
   itc_g->func.text_get = _gl_group_text_get;

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = _gl_text_get;
   itc->func.content_get = _gl_content_get;

#define CONTEXT(NAME) \
   git = elm_genlist_item_append(gl, itc_g, NAME, NULL, \
                                 ELM_GENLIST_ITEM_GROUP, NULL, NULL);

#define IC(NAME) \
   elm_genlist_item_append(gl, itc, NAME, git, \
                           ELM_GENLIST_ITEM_NONE, NULL, NULL);

   CONTEXT("Places")
   IC("user-home")
   IC("user-bookmarks")
   IC("user-desktop")
   IC("user-trash")
   IC("folder")
   IC("folder-drag-accept")
   IC("folder-remote")
   IC("folder-open")
   IC("folder-home")
   IC("folder-download")
   IC("folder-recent")
   IC("folder-documents")
   IC("folder-pictures")
   IC("folder-music")
   IC("folder-videos")
   IC("folder-visiting")
   IC("network-server")
   IC("network-workgroup")
   IC("start-here")

   CONTEXT("Emblems")
   IC("emblem-default")
   IC("emblem-documents")
   IC("emblem-downloads")
   IC("emblem-favorite")
   IC("emblem-important")
   IC("emblem-mail")
   IC("emblem-photos")
   IC("emblem-readonly")
   IC("emblem-shared")
   IC("emblem-symbolic-link")
   IC("emblem-unreadable")
   IC("emblem-system")
   IC("emblem-synchronized")
   IC("emblem-synchronizing")

   CONTEXT("Actions")
   IC("address-book-new")
   IC("application-exit")
   IC("appointment-new")
   IC("call-start")
   IC("call-stop")
   IC("contact-new")
   IC("document-open")
   IC("document-open-recent")
   IC("document-page-setup")
   IC("document-properties")
   IC("document-save")
   IC("document-save-as")
   IC("document-send")
   IC("document-close")
   IC("document-new")
   IC("document-print")
   IC("document-print-preview")
   IC("document-revert")
   IC("document-export")
   IC("document-import")
   IC("edit-clear")
   IC("edit-copy")
   IC("edit-cut")
   IC("edit-delete")
   IC("edit-find")
   IC("edit-find-replace")
   IC("edit-paste")
   IC("edit-redo")
   IC("edit-undo")
   IC("edit-select-all")
   IC("folder-copy")
   IC("folder-move")
   IC("folder-new")
   IC("format-indent-less")
   IC("format-indent-more")
   IC("format-justify-center")
   IC("format-justify-fill")
   IC("format-justify-left")
   IC("format-justify-right")
   IC("format-text-direction-ltr")
   IC("format-text-direction-rtl")
   IC("format-text-bold")
   IC("format-text-italic")
   IC("format-text-underline")
   IC("format-text-strikethrough")
   IC("go-home")
   IC("go-bottom")
   IC("go-down")
   IC("go-first")
   IC("go-jump")
   IC("go-last")
   IC("go-next")
   IC("go-previous")
   IC("go-top")
   IC("go-up")
   IC("insert-image")
   IC("insert-link")
   IC("insert-object")
   IC("insert-text")
   IC("list-add")
   IC("list-remove")
   IC("mail-forward")
   IC("mail-mark-important")
   IC("mail-mark-junk")
   IC("mail-mark-notjunk")
   IC("mail-mark-read")
   IC("mail-mark-unread")
   IC("mail-message-new")
   IC("mail-reply-all")
   IC("mail-reply-sender")
   IC("mail-send")
   IC("mail-send-receive")
   IC("media-eject")
   IC("media-playback-pause")
   IC("media-playback-start")
   IC("media-playback-stop")
   IC("media-record")
   IC("media-seek-backward")
   IC("media-seek-forward")
   IC("media-skip-backward")
   IC("media-skip-forward")
   IC("object-flip-horizontal")
   IC("object-flip-vertical")
   IC("object-rotate-left")
   IC("object-rotate-right")
   IC("process-stop")
   IC("tools-check-spelling")
   IC("view-list-compact")
   IC("view-list-details")
   IC("view-list-icons")
   IC("view-fullscreen")
   IC("view-refresh")
   IC("view-restore")
   IC("view-sort-ascending")
   IC("view-sort-descending")
   IC("view-close")
   IC("window-close")
   IC("window-new")
   IC("system-run")
   IC("system-shutdown")
   IC("system-reboot")
   IC("system-lock-screen")
   IC("system-log-out")
   IC("system-search")
   IC("help-about")
   IC("help-contents")
   IC("help-faq")
   IC("zoom-in")
   IC("zoom-out")
   IC("zoom-original")
   IC("zoom-fit")
   IC("zoom-fill")
   IC("zoom-fit-best")

   CONTEXT("Status")
   IC("appointment-missed")
   IC("appointment-soon")
   IC("audio-volume-high")
   IC("audio-volume-low")
   IC("audio-volume-medium")
   IC("audio-volume-muted")
   IC("audio-volume")
   IC("battery-caution")
   IC("battery-empty")
   IC("battery-low")
   IC("battery-good")
   IC("battery-full")
   IC("battery-caution-charging")
   IC("battery-empty-charging")
   IC("battery-low-charging")
   IC("battery-good-charging")
   IC("battery-full-charging")
   IC("bluetooth-active")
   IC("bluetooth-disabled")
   IC("changes-allow")
   IC("changes-prevent")
   IC("dialog-error")
   IC("dialog-information")
   IC("dialog-password")
   IC("dialog-question")
   IC("dialog-warning")
   IC("media-playlist-repeat")
   IC("media-playlist-shuffle")
   IC("network-cellular-3g")
   IC("network-cellular-4g")
   IC("network-cellular-edge")
   IC("network-cellular-gprs")
   IC("network-cellular-umts")
   IC("network-cellular-connected")
   IC("network-cellular-signal-excellent")
   IC("network-cellular-signal-good")
   IC("network-cellular-signal-none")
   IC("network-cellular-signal-ok")
   IC("network-cellular-signal-weak")
   IC("network-cellular-signal-acquiring")
   IC("network-error")
   IC("network-offline")
   IC("network-receive")
   IC("network-transmit-receive")
   IC("network-transmit")
   IC("network-idle")
   IC("network-vpn")
   IC("network-vpn-acquiring")
   IC("network-wireless-acquiring")
   IC("network-wireless-encrypted")
   IC("network-wireless-signal-excellent")
   IC("network-wireless-signal-good")
   IC("network-wireless-signal-none")
   IC("network-wireless-signal-ok")
   IC("network-wireless-signal-weak")
   IC("printer-error")
   IC("printer-printing")
   IC("printer-warning")
   IC("security-high")
   IC("security-medium")
   IC("security-low")
   IC("software-update-available")
   IC("software-update-urgent")
   IC("task-due")
   IC("task-past-due")
   IC("user-available")
   IC("user-away")
   IC("user-busy")
   IC("user-invisible")
   IC("user-idle")
   IC("user-offline")
   IC("user-trash-full")
   IC("weather-clear-night")
   IC("weather-clear")
   IC("weather-clouds-night")
   IC("weather-clouds")
   IC("weather-few-clouds-night")
   IC("weather-few-clouds")
   IC("weather-fog")
   IC("weather-overcast")
   IC("weather-severe-alert")
   IC("weather-showers-scattered")
   IC("weather-showers")
   IC("weather-snow")
   IC("weather-storm")
   IC("mail-attachment")
   IC("mail-unread")
   IC("mail-read")
   IC("mail-replied")
   IC("mail-signed")
   IC("mail-signed-verified")
   IC("starred")
   IC("non-starred")

   CONTEXT("Devices")
   IC("audio-card")
   IC("audio-input-microphone")
   IC("battery")
   IC("camera")
   IC("camera-photo")
   IC("camera-web")
   IC("camera-video")
   IC("drive-harddisk")
   IC("drive-harddisk-ieee1394")
   IC("drive-harddisk-usb")
   IC("drive-optical")
   IC("drive-removable-media")
   IC("drive-removable-media-ieee1394")
   IC("drive-removable-media-usb")
   IC("computer")
   IC("computer-laptop")
   IC("multimedia-player")
   IC("media-optical")
   IC("media-optical-cd")
   IC("media-optical-audio")
   IC("media-optical-dvd")
   IC("media-optical-bd")
   IC("media-flash")
   IC("media-flash-cf")
   IC("media-flash-ms")
   IC("media-flash-sd")
   IC("media-flash-sm")
   IC("media-floppy")
   IC("media-zip")
   IC("media-tape")
   IC("media-memory")
   IC("processor")
   IC("input-mouse")
   IC("input-tablet")
   IC("input-keyboard")
   IC("input-gaming")
   IC("pda")
   IC("phone")
   IC("printer")
   IC("scanner")
   IC("modem")
   IC("video-display")
   IC("network-wired")
   IC("network-wireless")

   CONTEXT("Categories")
   IC("applications-accessories")
   IC("applications-development")
   IC("applications-games")
   IC("applications-graphics")
   IC("applications-internet")
   IC("applications-multimedia")
   IC("applications-office")
   IC("applications-other")
   IC("applications-science")
   IC("applications-system")
   IC("applications-utilities")
   IC("preferences-desktop")
   IC("preferences-system")
   IC("preferences-other")

   CONTEXT("Applications")
   IC("preferences-color")
   IC("preferences-profile")
   IC("preferences-desktop-display")
   IC("preferences-desktop-font")
   IC("preferences-desktop-keyboard")
   IC("preferences-desktop-locale")
   IC("preferences-desktop-multimedia")
   IC("preferences-desktop-screensaver")
   IC("preferences-desktop-theme")
   IC("preferences-desktop-wallpaper")
   IC("system-file-manager")

   CONTEXT("International")
   IC("flag-ad")
   IC("flag-af")
   IC("flag-al")
   IC("flag-am")
   IC("flag-ar")
   IC("flag-at")
   IC("flag-au")
   IC("flag-az")
   IC("flag-ba")
   IC("flag-bd")
   IC("flag-be")
   IC("flag-bg")
   IC("flag-br")
   IC("flag-bt")
   IC("flag-bw")
   IC("flag-by")
   IC("flag-ca")
   IC("flag-cd")
   IC("flag-ch")
   IC("flag-cm")
   IC("flag-cn")
   IC("flag-cz")
   IC("flag-de")
   IC("flag-dk")
   IC("flag-ee")
   IC("flag-es")
   IC("flag-et")
   IC("flag-fi")
   IC("flag-fo")
   IC("flag-fr")
   IC("flag-gb")
   IC("flag-ge")
   IC("flag-gh")
   IC("flag-gn")
   IC("flag-gr")
   IC("flag-hr")
   IC("flag-hu")
   IC("flag-ie")
   IC("flag-il")
   IC("flag-in")
   IC("flag-iq")
   IC("flag-ir")
   IC("flag-is")
   IC("flag-it")
   IC("flag-jp")
   IC("flag-ke")
   IC("flag-kg")
   IC("flag-kh")
   IC("flag-kr")
   IC("flag-ku")
   IC("flag-kz")
   IC("flag-la")
   IC("flag-lk")
   IC("flag-lt")
   IC("flag-lv")
   IC("flag-ma")
   IC("flag-md")
   IC("flag-me")
   IC("flag-mk")
   IC("flag-ml")
   IC("flag-mm")
   IC("flag-mn")
   IC("flag-mt")
   IC("flag-mv")
   IC("flag-my")
   IC("flag-ng")
   IC("flag-nl")
   IC("flag-no")
   IC("flag-np")
   IC("flag-ph")
   IC("flag-pk")
   IC("flag-pl")
   IC("flag-pt")
   IC("flag-ro")
   IC("flag-rs")
   IC("flag-ru")
   IC("flag-se")
   IC("flag-si")
   IC("flag-sk")
   IC("flag-sn")
   IC("flag-sy")
   IC("flag-th")
   IC("flag-tj")
   IC("flag-tm")
   IC("flag-tr")
   IC("flag-tw")
   IC("flag-tz")
   IC("flag-ua")
   IC("flag-us")
   IC("flag-uz")
   IC("flag-vn")
   IC("flag-za")

   CONTEXT("Mimetypes")
   IC("inode-directory")

#undef CONTEXT
#undef IC

   elm_genlist_item_class_free(itc);
   elm_genlist_item_class_free(itc_g);
}

static void
_rdg_changed_cb(void *data, Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   Evas_Object *li = data;

   elm_genlist_realized_items_update(li);
}

static void
_slider_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *icon = data;
   int size = (int)elm_slider_value_get(obj);

   evas_object_size_hint_min_set(icon, size, size);
}

static void
_list_selected_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *list_it = event_info;
   Evas_Object *icon = evas_object_data_get(obj, "resize_icon");

   elm_icon_standard_set(icon, elm_object_item_text_get(list_it));
}

static void
_std_btn_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Evas_Object *win, *panes, *icon;

   win = elm_win_util_standard_add("icon-test-std-auto", "Icon Standard");
   elm_win_autodel_set(win, EINA_TRUE);

   panes = elm_panes_add(win);
   evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_panes_content_left_min_size_set(panes, 16);
   elm_panes_content_right_min_size_set(panes, 16);
   elm_win_resize_object_add(win, panes);
   evas_object_show(panes);

   icon = elm_icon_add(panes);
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_icon_standard_set(icon, "folder");
   elm_object_part_content_set(panes, "left", icon);
   evas_object_show(icon);

   icon = elm_icon_add(panes);
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_icon_standard_set(icon, "user-home");
   elm_object_part_content_set(panes, "right", icon);
   evas_object_show(icon);

   evas_object_resize(win, 300, 200);
   evas_object_show(win);
}

void
test_icon_standard(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Evas_Object *win, *li, *box, *hbox, *fr, *rd, *rdg, *label, *icon, *sl, *bt;

   win = elm_win_util_standard_add("icon-test-std", "Icon Standard");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   li = elm_genlist_add(box);
   evas_object_size_hint_expand_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(li, "selected", _list_selected_cb, NULL);
   evas_object_show(li);

   // lookup order
   fr = elm_frame_add(box);
   elm_object_text_set(fr, "standard icon order lookup");
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, fr);
   evas_object_show(fr);

   hbox = elm_box_add(fr);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_object_content_set(fr, hbox);
   evas_object_show(hbox);

   label = elm_label_add(hbox);
   elm_object_text_set(label, "Lookup order has moved to elementary_config");
   elm_box_pack_end(hbox, label);
   evas_object_show(label);

   // size
   fr = elm_frame_add(box);
   elm_object_text_set(fr, "standard icon size");
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, fr);
   evas_object_show(fr);
   
   hbox = elm_box_add(fr);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   elm_object_content_set(fr, hbox);
   evas_object_show(hbox);

   rdg = elm_radio_add(hbox);
   elm_radio_state_value_set(rdg, 0);
   elm_object_text_set(rdg, "Free");
   elm_box_pack_end(hbox, rdg);
   evas_object_show(rdg);
   evas_object_smart_callback_add(rdg, "changed", _rdg_changed_cb, li);
   evas_object_data_set(li, "size_rdg", rdg);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 16);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "16");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 22);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "22");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 24);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "24");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 32);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "32");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 48);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "48");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 64);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "64");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   rd = elm_radio_add(hbox);
   elm_radio_state_value_set(rd, 128);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "128");
   elm_box_pack_end(hbox, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, li);

   // populate and pack the list
   _standard_genlist_populate(li);
   elm_box_pack_end(box, li);

   // live resize
   fr = elm_frame_add(box);
   elm_object_text_set(fr, "live resize");
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, 0.0);
   elm_box_pack_end(box, fr);
   evas_object_show(fr);
   
   hbox = elm_box_add(fr);
   elm_object_content_set(fr, hbox);
   evas_object_show(hbox);

   icon = elm_icon_add(hbox);
   elm_icon_standard_set(icon, "folder");
   evas_object_size_hint_min_set(icon, 16, 16);
   elm_box_pack_end(hbox, icon);
   evas_object_show(icon);
   evas_object_data_set(li, "resize_icon", icon);

   sl = elm_slider_add(hbox);
   elm_object_text_set(sl, "min_size");
   elm_slider_min_max_set(sl, 16, 256);
   elm_slider_value_set(sl, 16);
   elm_slider_unit_format_set(sl, "%.0f px");
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.0);
   evas_object_smart_callback_add(sl, "changed", _slider_changed_cb, icon);
   elm_box_pack_end(hbox, sl);
   evas_object_show(sl);

   bt = elm_button_add(box);
   elm_object_text_set(bt, "Another size test, without using min_size");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _std_btn_clicked_cb, NULL);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   // show the win
   evas_object_resize(win, 300, 500);
   evas_object_show(win);
}
