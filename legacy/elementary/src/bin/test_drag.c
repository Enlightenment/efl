#include <stdbool.h>

#include <Ecore_X.h>
#include <Elementary.h>
#include "elm_priv.h"
/* nash: I have NFI what this does: Just copying the other tests */
#ifndef ELM_LIB_QUICKLAUNCH


#define IM      "/home/nash/work/samsung/autopaste/images/"
static const char *images[] = {
        IM "cow.jpg",
        IM "img_3104.jpg",
        IM"xmas.jpg",
        IM"river.jpg"
};
#define N_IMAGES 4


void
test_drag_source(void *data, Evas_Object *obj, void *eventinfo){
	Evas_Object *win, *bg, *bx, *ph, *ctrls;
	int i;

	win = elm_win_add(NULL, "drag", ELM_WIN_BASIC);
	elm_win_title_set(win, "Drag Source");
	elm_win_autodel_set(win, 1);

	bg = elm_bg_add(win);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg);

	bx = elm_box_add(win);
        evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
        elm_win_resize_object_add(win, bx);
        evas_object_show(bx);

        ctrls = elm_box_add(win);
        elm_box_horizontal_set(ctrls, true);
        evas_object_size_hint_weight_set(ctrls, EVAS_HINT_EXPAND, 0);
        evas_object_size_hint_align_set(ctrls, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(bx, ctrls);
        evas_object_show(ctrls);


	for (i = 0 ; i < N_IMAGES ; i ++){
                ph = elm_photo_add(win);
                elm_photo_file_set(ph, images[i]);
                evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND,
                                EVAS_HINT_EXPAND);
                evas_object_size_hint_align_set(ph, EVAS_HINT_FILL,
                                EVAS_HINT_FILL);
                elm_photo_size_set(ph, 80);
                elm_box_pack_end(ctrls, ph);
                evas_object_show(ph);
               // evas_object_data_set(ph,"URI",images[i]);
          //      evas_object_smart_callback_add(ph, "clicked",
            //                    on_select_image, ph);

        }




	evas_object_size_hint_min_set(bg, 468, 160);
	evas_object_resize(win, 320, 320);

	evas_object_show(win);
}

static Eina_Bool
_dnd_enter(void *data, int etype, void *ev){
	printf("enter\n");
	return true;
}

static Eina_Bool
_dnd_leave(void *data, int etype, void *ev){
	printf("leave\n");
	return true;
}
static Eina_Bool
_dnd_position(void *data, int etype, void *ev){
	struct _Ecore_X_Event_Xdnd_Position *pos;
	Ecore_X_Rectangle rect;

	pos = ev;

	printf("position: %3d,%3d, Action: %s\n",
			pos->position.x,pos->position.y,
			ecore_x_atom_name_get(pos->action));

	/* Need to send a status back */
	rect.x = pos->position.x - 5;
	rect.y = pos->position.y - 5;
	rect.width = 10;
	rect.height = 10;
	ecore_x_dnd_send_status(true, false, rect, pos->action);

	return true;
}
static Eina_Bool
_dnd_status(void *data, int etype, void *ev){
	printf("status\n");
	return true;
}


Evas_Object *en;
static Eina_Bool
_dnd_drop(void *data, int etype, void *ev){
	struct _Ecore_X_Event_Xdnd_Drop *drop;
	bool rv;

	drop = ev;
	printf("Drop: %3d,%3d, Action: %s\n",
			drop->position.x,drop->position.y,
			ecore_x_atom_name_get(drop->action));

	rv = elm_selection_get(ELM_SEL_XDND, ELM_SEL_MARKUP, en);
	if (rv != true){
		printf("Selection set fail\n");
	}

	return true;
}
static Eina_Bool
_dnd_finish(void *data, int etype, void *ev){
	printf("finish\n");
	return true;
}
void
test_drag_dest(void *data, Evas_Object *obj, void *event){
	Evas_Object *win, *bg, *bx;
	Ecore_Evas *ee;
	Ecore_X_Window xwin;

	win = elm_win_add(NULL, "drag", ELM_WIN_BASIC);
	elm_win_title_set(win, "Drag Destination");
	elm_win_autodel_set(win, 1);

	bg = elm_bg_add(win);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bg);
	evas_object_show(bg);

	evas_object_size_hint_min_set(bg, 160, 160);
	evas_object_resize(win, 480, 150);

	bx = elm_box_add(win);
	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bx);
	evas_object_show(bx);

	en = elm_entry_add(win);
	elm_entry_line_wrap_set(en, 0);
	elm_entry_entry_set(en,"Your mum!");
	evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(bx, en);
	evas_object_show(en);

	printf("Adding events\n");
	ecore_event_handler_add(ECORE_X_EVENT_XDND_ENTER, _dnd_enter, NULL);
	ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION, _dnd_position, NULL);
	ecore_event_handler_add(ECORE_X_EVENT_XDND_STATUS, _dnd_status, NULL);
	ecore_event_handler_add(ECORE_X_EVENT_XDND_LEAVE, _dnd_leave, NULL);
	ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP, _dnd_drop, NULL);
	ecore_event_handler_add(ECORE_X_EVENT_XDND_FINISHED, _dnd_finish, NULL);

	printf("Enabled DND\n");
	ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win));
	xwin = (Ecore_X_Window)ecore_evas_window_get(ee);

	ecore_x_dnd_aware_set(xwin, true);

	evas_object_show(win);
}

void
test_drag_genlist(void *data, Evas_Object *obj, void *event){
	printf("No genlist yet\n");
}


#endif
