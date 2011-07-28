#include "widget_preview_tmpl_head.c"

Evas_Object *inwin, *lbl;

inwin = elm_win_inwin_add(win);
elm_object_style_set(inwin, "minimal_vertical");
evas_object_show(inwin);

lbl = elm_label_add(win);
elm_object_text_set(lbl, "The content of an inwin<ps>"
                    "can be anything that<ps>"
                    "may be shown in a popup.<ps><ps>"
                    "This one we are in is<ps>"
                    "using the <b>minimal_vertical</b> style.");
elm_win_inwin_content_set(inwin, lbl);
evas_object_show(lbl);

#include "widget_preview_tmpl_foot.c"
