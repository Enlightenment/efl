#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif
#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Ecore_Evas.h>
#include <Elementary.h>
#include <Exactness.h>

#include "exactness_private.h"

#define LDIFF(x) "<b><color=#F0F>"#x"</color></b>"
#define RDIFF(x) "<b><color=#0FF>"#x"</color></b>"

typedef enum
{
   EX_FONTS_DIR,
   EX_SCENARIO,
   EX_IMAGE,
   EX_OBJ_INFO
} _Data_Type;

typedef struct
{
   void *p1;
   void *p2;
   _Data_Type dt;
} _Compare_Item_Data;

typedef struct
{
   void *ex_parent;
   Eo *gl_item;
} _Item_Info;

static Eo *_main_box = NULL;
static Eina_List *_gls = NULL;
static Eina_List *_units = NULL;
static Eo *_comp_selected_item = NULL;

static Elm_Genlist_Item_Class *_grp_itc = NULL, *_scn_itc = NULL, *_img_itc = NULL;
static Elm_Genlist_Item_Class *_objs_itc = NULL, *_obj_itc = NULL;

static Eina_Hash *_item_infos_hash = NULL;

static Eina_Bool _show_only_diffs = EINA_FALSE;
static Eina_List *_comp_vvs = NULL;

static Eina_List *_modified_units = NULL;

static const char *
_action_name_get(Exactness_Action *act)
{
   if (!act) return NULL;
   switch(act->type)
     {
      case EXACTNESS_ACTION_MOUSE_IN: return "Mouse In";
      case EXACTNESS_ACTION_MOUSE_OUT: return "Mouse Out";
      case EXACTNESS_ACTION_MOUSE_WHEEL: return "Mouse Wheel";
      case EXACTNESS_ACTION_MULTI_DOWN: return "Multi Down";
      case EXACTNESS_ACTION_MULTI_UP: return "Multi Up";
      case EXACTNESS_ACTION_MULTI_MOVE: return "Multi Move";
      case EXACTNESS_ACTION_KEY_DOWN: return "Key Down";
      case EXACTNESS_ACTION_KEY_UP: return "Key Up";
      case EXACTNESS_ACTION_TAKE_SHOT: return "Take shot";
      case EXACTNESS_ACTION_EFL_EVENT: return "EFL Event";
      case EXACTNESS_ACTION_CLICK_ON: return "Click On";
      case EXACTNESS_ACTION_STABILIZE: return "Stabilize";
      default: return NULL;
     }
}

static int
_event_struct_len_get(Exactness_Action_Type type)
{
   switch(type)
     {
      case EXACTNESS_ACTION_MOUSE_WHEEL:
         return sizeof(Exactness_Action_Mouse_Wheel);
      case EXACTNESS_ACTION_MULTI_DOWN:
      case EXACTNESS_ACTION_MULTI_UP:
         return sizeof(Exactness_Action_Multi_Event);
      case EXACTNESS_ACTION_MULTI_MOVE:
         return sizeof(Exactness_Action_Multi_Move);
      case EXACTNESS_ACTION_KEY_DOWN:
      case EXACTNESS_ACTION_KEY_UP:
         return sizeof(Exactness_Action_Key_Down_Up);
      case EXACTNESS_ACTION_EFL_EVENT:
         return sizeof(Exactness_Action_Efl_Event);
      case EXACTNESS_ACTION_CLICK_ON:
         return sizeof(Exactness_Action_Click_On);
      default: return 0;
     }
}

static void
_action_specific_info_get(const Exactness_Action *act, char output[1024])
{
   switch(act->type)
     {
      case EXACTNESS_ACTION_MOUSE_WHEEL:
           {
              Exactness_Action_Mouse_Wheel *t = act->data;
              sprintf(output, "Direction %d Z %d", t->direction, t->z);
              break;
           }
      case EXACTNESS_ACTION_MULTI_UP: case EXACTNESS_ACTION_MULTI_DOWN:
           {
              Exactness_Action_Multi_Event *t = act->data;
              if (!t->d)
                 sprintf(output, "Button %d Flags %d", t->b, t->flags);
              else
                 sprintf(output, "D %d X %d Y %d Rad %f RadX %f RadY %f Pres %f Ang %f FX %f FY %f Flags %d",
                       t->d, t->x, t->y, t->rad, t->radx, t->rady, t->pres, t->ang, t->fx, t->fy, t->flags);
              break;
          }
      case EXACTNESS_ACTION_MULTI_MOVE:
           {
              Exactness_Action_Multi_Move *t = act->data;
              if (!t->d)
                 sprintf(output, "X %d Y %d", t->x, t->y);
              else
                 sprintf(output, "D %d X %d Y %d Rad %f RadX %f RadY %f Pres %f Ang %f FX %f FY %f",
                       t->d, t->x, t->y, t->rad, t->radx, t->rady, t->pres, t->ang, t->fx, t->fy);
              break;
           }
      case EXACTNESS_ACTION_KEY_UP: case EXACTNESS_ACTION_KEY_DOWN:
           {
              Exactness_Action_Key_Down_Up *t = act->data;
              sprintf(output, "Keyname %s Key %s String %s Compose %s Keycode %d",
                    t->keyname, t->key, t->string, t->compose, t->keycode);
              break;
           }
      case EXACTNESS_ACTION_EFL_EVENT:
           {
              Exactness_Action_Efl_Event *t = act->data;
              sprintf(output, "Widget %s Event %s", t->wdg_name, t->event_name);
              break;
           }
      case EXACTNESS_ACTION_CLICK_ON:
           {
              Exactness_Action_Click_On *t = act->data;
              sprintf(output, "Widget %s", t->wdg_name);
              break;
           }
      default:
           {
              output[0] = '\0';
              break;
           }
     }
}

static Eina_Bool
_is_hook_duplicate(const Exactness_Action *cur_act, const Exactness_Action *prev_act)
{
   if (!prev_act) return EINA_FALSE;
   if (cur_act->type == prev_act->type)
     {
        int len = _event_struct_len_get(cur_act->type);
        return (!len || !memcmp(cur_act->data, prev_act->data, len));
     }
   return EINA_FALSE;
}

static Eina_Bool
_are_scenario_entries_different(Exactness_Action *act1, Exactness_Action *act2)
{
   if (!act1 ^ !act2) return EINA_TRUE;
   if (act1->type != act2->type) return EINA_TRUE;
   switch(act1->type)
     {
      case EXACTNESS_ACTION_MOUSE_WHEEL:
         return !!memcmp(act1->data, act2->data, sizeof(Exactness_Action_Mouse_Wheel));
      case EXACTNESS_ACTION_MULTI_DOWN: case EXACTNESS_ACTION_MULTI_UP:
         return !!memcmp(act1->data, act2->data, sizeof(Exactness_Action_Multi_Event));
      case EXACTNESS_ACTION_MULTI_MOVE:
         return !!memcmp(act1->data, act2->data, sizeof(Exactness_Action_Multi_Move));
      case EXACTNESS_ACTION_KEY_UP: case EXACTNESS_ACTION_KEY_DOWN:
         return !!memcmp(act1->data, act2->data, sizeof(Exactness_Action_Key_Down_Up));
      case EXACTNESS_ACTION_EFL_EVENT:
           {
              Exactness_Action_Efl_Event *e1 = act1->data;
              Exactness_Action_Efl_Event *e2 = act2->data;
              return (!!strcmp(e1->wdg_name, e2->wdg_name) ||
                    !!strcmp(e1->event_name, e2->event_name));
           }
      case EXACTNESS_ACTION_CLICK_ON:
           {
              Exactness_Action_Click_On *e1 = act1->data;
              Exactness_Action_Click_On *e2 = act2->data;
              return (!!strcmp(e1->wdg_name, e2->wdg_name));
           }
      default:
         return EINA_FALSE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_are_images_different(Exactness_Image *e_img1, Exactness_Image *e_img2)
{
   unsigned int w, h;
   int *pxs1 = NULL;
   int *pxs2 = NULL;
   if (!e_img1 ^ !e_img2) return EINA_TRUE;
   if (e_img1->w != e_img2->w) return EINA_TRUE;
   if (e_img1->h != e_img2->h) return EINA_TRUE;
   pxs1 = e_img1->pixels;
   pxs2 = e_img2->pixels;
   for (w = 0; w < e_img1->w; w++)
     {
        for (h = 0; h < e_img1->h; h++)
          {
             if (pxs1[h * e_img1->w + w] != pxs2[h * e_img1->w + w])
                return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static Eina_Bool
_are_objs_different(Exactness_Object *e_obj1, Exactness_Object *e_obj2, Eina_Bool check_objs)
{
   if (!e_obj1 ^ !e_obj2) return EINA_TRUE;
   Eina_List *itr1 = e_obj1->children;
   Eina_List *itr2 = e_obj2->children;
   if (check_objs &&
         (strcmp(e_obj1->kl_name, e_obj2->kl_name) ||
          e_obj1->x != e_obj2->x || e_obj1->y != e_obj2->y ||
          e_obj1->w != e_obj2->w || e_obj1->h != e_obj2->h)) return EINA_TRUE;
   while (itr1 || itr2)
     {
        if ((!itr1) ^ (!itr2)) return EINA_TRUE;
        e_obj1 = eina_list_data_get(itr1);
        e_obj2 = eina_list_data_get(itr2);

        if (_are_objs_different(e_obj1, e_obj2, EINA_TRUE)) return EINA_TRUE;

        itr1 = eina_list_next(itr1);
        itr2 = eina_list_next(itr2);
     }
   return EINA_FALSE;
}

static Eina_Bool
_are_objs_trees_different(Exactness_Objects *e_objs1, Exactness_Objects *e_objs2)
{
   if (!e_objs1 ^ !e_objs2) return EINA_TRUE;
   Eina_List *itr1 = e_objs1->objs;
   Eina_List *itr2 = e_objs2->objs;
   Exactness_Object *e_obj1, *e_obj2;
   while (itr1 || itr2)
     {
        if ((!itr1) ^ (!itr2)) return EINA_TRUE;
        e_obj1 = eina_list_data_get(itr1);
        e_obj2 = eina_list_data_get(itr2);

        if (_are_objs_different(e_obj1, e_obj2, EINA_TRUE)) return EINA_TRUE;

        itr1 = eina_list_next(itr1);
        itr2 = eina_list_next(itr2);
     }
   return EINA_FALSE;
}

static void
_win_del(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   efl_exit(0); /* exit the program's main loop that runs in elm_run() */
}

static void
_gui_win_create()
{
   Eo *win, *bg;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   win = elm_win_add(NULL, "Window", ELM_WIN_BASIC);
   evas_object_smart_callback_add(win, "delete,request", _win_del, NULL);
   elm_win_maximized_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_title_set(win, "Exactness Inspector");
   efl_gfx_entity_size_set(win, EINA_SIZE2D(1000, 800));

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.000000, 1.000000);
   efl_gfx_entity_visible_set(bg, EINA_TRUE);
   elm_win_resize_object_add(win, bg);

   _main_box = elm_box_add(win);
   elm_box_horizontal_set(_main_box, EINA_TRUE);
   elm_box_homogeneous_set(_main_box, EINA_TRUE);
   evas_object_size_hint_weight_set(_main_box, 1.000000, 1.000000);
   efl_gfx_entity_visible_set(_main_box, EINA_TRUE);
   elm_win_resize_object_add(win, _main_box);

   efl_gfx_entity_visible_set(win, EINA_TRUE);
}

static char *
_grp_text_get(void *data, Evas_Object *gl, const char *part EINA_UNUSED)
{
   char buf[256];
   const char *str = NULL;
   Eina_Bool compare = !!efl_key_data_get(gl, "_exactness_gl_compare");
   _Data_Type dt = (_Data_Type) data;
   switch (dt)
     {
      case EX_FONTS_DIR:
           {
              char buf2[256];
              if (!compare)
                {
                   Exactness_Unit *unit = efl_key_data_get(gl, "unit");
                   sprintf(buf2, "Fonts directory: %s", unit->fonts_path?unit->fonts_path:"None");
                }
              else
                {
                   Eo *gl1 = eina_list_nth(_gls, 0);
                   Eo *gl2 = eina_list_nth(_gls, 1);
                   Exactness_Unit *unit1 = efl_key_data_get(gl1, "unit");
                   Exactness_Unit *unit2 = efl_key_data_get(gl2, "unit");
                   if (!!unit1->fonts_path ^ !!unit2->fonts_path)
                      sprintf(buf2, "Fonts directory comparison: XXXXX");
                   else if (!strcmp(unit1->fonts_path, unit2->fonts_path))
                      sprintf(buf2, "Fonts directory comparison: %s", unit1->fonts_path);
                   else
                      sprintf(buf2, "Fonts directory comparison: "LDIFF(%s)"/"RDIFF(%s),
                            unit1->fonts_path, unit2->fonts_path);
                }
              return strdup(buf2);
           }
      case EX_SCENARIO: { str = "Scenario"; break; }
      case EX_IMAGE: { str = "Images"; break; }
      case EX_OBJ_INFO: { str = "Objects"; break; }
      default: { str = "Unknown"; break; }
     }
   sprintf(buf, "%s%s", str, compare ? " comparison" : "");
   if (dt == EX_FONTS_DIR) eina_stringshare_del(str);
   return strdup(buf);
}

static char *
_scn_text_get(void *data, Evas_Object *gl, const char *part EINA_UNUSED)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   char *ret = NULL;
   Eina_Bool compare = !!efl_key_data_get(gl, "_exactness_gl_compare");
   if (compare)
     {
        _Compare_Item_Data *vv = data;
        Exactness_Action *a1 = vv->p1;
        Exactness_Action *a2 = vv->p2;

        if (!a1 ^ !a2) return strdup("XXXXX");

        if (a1->delay_ms != a2->delay_ms) eina_strbuf_append_printf(buf, "[+"LDIFF(%.3f)"/+"RDIFF(%.3f)"]: ", a1->delay_ms/1000.0, a2->delay_ms/1000.0);
        else eina_strbuf_append_printf(buf, "+%.3f: ", a1->delay_ms / 1000.0);

        if (a1->type != a2->type)
           eina_strbuf_append_printf(buf, "["LDIFF(%s)"/"RDIFF(%s)"] - XXXXXX", _action_name_get(a1), _action_name_get(a2));
        else
          {
             char params1[1024];
             char params2[2024];
             _action_specific_info_get(a1, params1);
             _action_specific_info_get(a2, params2);

             eina_strbuf_append_printf(buf, "%s", _action_name_get(a1));
             if (*params1 || *params2)
               {
                  if (strcmp(params1, params2))
                     eina_strbuf_append_printf(buf, " - ["LDIFF(%s)"/"RDIFF(%s)"]", params1, params2);
                  else
                     eina_strbuf_append_printf(buf, " - %s", params1);
               }
          }
     }
   else
     {
        Exactness_Action *act = data;
        char specific_output[1024];
        if (act)
          {
             eina_strbuf_append_printf(buf, "+%.3f: ", act->delay_ms / 1000.0);
             eina_strbuf_append_printf(buf, "%s", _action_name_get(act));
             _action_specific_info_get(act, specific_output);
             if (*specific_output) eina_strbuf_append_printf(buf, " - %s", specific_output);
          }
        else
           eina_strbuf_append(buf, "XXXXX");
     }

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static int
_unit_shot_no_get(Exactness_Unit *unit, Exactness_Action *act_ref)
{
   Eina_List *itr;
   Exactness_Action *act;
   int ret = 0;
   if (!unit) return -1;
   EINA_LIST_FOREACH(unit->actions, itr, act)
     {
        if (act->type == EXACTNESS_ACTION_TAKE_SHOT)
          {
             if (act == act_ref) return ret;
             ret++;
          }
     }
   return -1;
}

static void
_goto_shot(void *data EINA_UNUSED, Evas_Object *bt, void *event_info EINA_UNUSED)
{
   Eo *gl = efl_key_data_get(bt, "gl");
   Eina_Bool compare = !!efl_key_data_get(gl, "_exactness_gl_compare");
   if (compare)
     {
        _Compare_Item_Data *vv;
        Eina_List *itr;
        Eo *gl1 = eina_list_nth(_gls, 0);
        Eo *gl2 = eina_list_nth(_gls, 1);
        Exactness_Unit *unit1 = efl_key_data_get(gl1, "unit");
        Exactness_Unit *unit2 = efl_key_data_get(gl2, "unit");
        int shot1_no = (intptr_t)efl_key_data_get(bt, "shot1_no");
        int shot2_no = (intptr_t)efl_key_data_get(bt, "shot2_no");
        Exactness_Image *ex_img1 = shot1_no != -1 ? eina_list_nth(unit1->imgs, shot1_no) : NULL;
        Exactness_Image *ex_img2 = shot2_no != -1 ? eina_list_nth(unit2->imgs, shot2_no) : NULL;
        EINA_LIST_FOREACH(_comp_vvs, itr, vv)
          {
             if (vv->p1 == ex_img1 && vv->p2 == ex_img2)
               {
                  _Item_Info *ii = eina_hash_find(_item_infos_hash, &vv);
                  if (ii && ii->gl_item)
                     elm_genlist_item_show(ii->gl_item, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
               }
          }
     }
   else
     {
        Exactness_Unit *unit = efl_key_data_get(gl, "unit");
        int shot_no = (intptr_t)efl_key_data_get(bt, "shot_no");
        Exactness_Image *ex_img = shot_no != -1 ? eina_list_nth(unit->imgs, shot_no) : NULL;
        _Item_Info *ii = eina_hash_find(_item_infos_hash, &ex_img);
        if (ii && ii->gl_item)
           elm_genlist_item_show(ii->gl_item, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
     }
}

static Evas_Object *
_scn_content_get(void *data, Evas_Object *gl, const char *part)
{
   Eina_Bool compare = !!efl_key_data_get(gl, "_exactness_gl_compare");
   if (compare)
     {
        if (!strcmp(part, "elm.swallow.end"))
          {
             _Compare_Item_Data *vv = data;
             Exactness_Action *v1 = vv->p1;
             Exactness_Action *v2 = vv->p2;
             if (v1 && v2 && v1->type == EXACTNESS_ACTION_TAKE_SHOT &&
                v2->type == EXACTNESS_ACTION_TAKE_SHOT)
             {
                Eo *gl1 = eina_list_nth(_gls, 0);
                Eo *gl2 = eina_list_nth(_gls, 1);
                Exactness_Unit *unit1 = efl_key_data_get(gl1, "unit");
                Exactness_Unit *unit2 = efl_key_data_get(gl2, "unit");
                int shot1_no = _unit_shot_no_get(unit1, v1);
                int shot2_no = _unit_shot_no_get(unit2, v2);
                Exactness_Image *ex_img1 = shot1_no != -1 ? eina_list_nth(unit1->imgs, shot1_no) : NULL;
                Exactness_Image *ex_img2 = shot2_no != -1 ? eina_list_nth(unit2->imgs, shot2_no) : NULL;
                Exactness_Image *ex_imgO = NULL;
                exactness_image_compare(ex_img1, ex_img2, &ex_imgO);

                if (ex_imgO)
                  {
                     Eo *bt, *ic, *evas_img;

                     bt = elm_button_add(gl);
                     evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                     evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
                     evas_object_show(bt);
                     efl_key_data_set(bt, "gl", gl);
                     efl_key_data_set(bt, "shot1_no", (void *)(intptr_t)shot1_no);
                     efl_key_data_set(bt, "shot2_no", (void *)(intptr_t)shot2_no);
                     evas_object_smart_callback_add(bt, "clicked", _goto_shot, NULL);

                     ic = elm_icon_add(bt);
                     evas_img = elm_image_object_get(ic);
                     evas_object_image_size_set(evas_img, ex_imgO->w, ex_imgO->h);
                     evas_object_image_data_set(evas_img, ex_imgO->pixels);
                     evas_object_show(ic);
                     elm_object_part_content_set(bt, "icon", ic);
                     return bt;
                  }
             }
          }
     }
   else
     {
        if (!strcmp(part, "elm.swallow.end"))
          {
             Exactness_Action *v = data;
             Exactness_Unit *unit = efl_key_data_get(gl, "unit");
             int shot_no = _unit_shot_no_get(unit, v);
             Exactness_Image *ex_img = shot_no != -1 ? eina_list_nth(unit->imgs, shot_no) : NULL;

             if (ex_img)
               {
                  Eo *bt, *ic, *evas_img;

                  bt = elm_button_add(gl);
                  evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
                  evas_object_show(bt);
                  efl_key_data_set(bt, "gl", gl);
                  efl_key_data_set(bt, "shot_no", (void *)(intptr_t)shot_no);
                  evas_object_smart_callback_add(bt, "clicked", _goto_shot, NULL);

                  ic = elm_icon_add(bt);
                  evas_img = elm_image_object_get(ic);
                  evas_object_image_size_set(evas_img, ex_img->w, ex_img->h);
                  evas_object_image_data_set(evas_img, ex_img->pixels);
                  evas_object_show(ic);
                  elm_object_part_content_set(bt, "icon", ic);

                  return bt;
               }
          }
     }
   return NULL;
}

static Evas_Object *
_img_content_get(void *data, Evas_Object *gl, const char *part)
{
   if (strcmp(part, "elm.swallow.content")) return NULL;
   Eo *img = elm_image_add(gl);
   Eo *evas_img = elm_image_object_get(img);
   Eina_Bool compare = !!efl_key_data_get(gl, "_exactness_gl_compare");
   if (compare)
     {
        _Compare_Item_Data *vv = data;
        Exactness_Image *ex_img1 = vv->p1;
        Exactness_Image *ex_img2 = vv->p2;
        Exactness_Image *ex_imgO = NULL;
        exactness_image_compare(ex_img1, ex_img2, &ex_imgO);

        evas_object_image_size_set(evas_img, ex_imgO->w, ex_imgO->h);
        evas_object_image_data_set(evas_img, ex_imgO->pixels);
        evas_object_size_hint_min_set(img, ELM_SCALE_SIZE(300), ELM_SCALE_SIZE(300));
     }
   else
     {
        if (!data)
          {
             efl_del(img);
             return NULL;
          }
        Exactness_Image *ex_img = data;
        evas_object_image_size_set(evas_img, ex_img->w, ex_img->h);
        evas_object_image_data_set(evas_img, ex_img->pixels);
        evas_object_size_hint_min_set(img, ELM_SCALE_SIZE(300), ELM_SCALE_SIZE(300));
     }
   return img;
}

static char *
_objs_text_get(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return strdup("Shot");
}

static char *
_obj_text_get(void *data, Evas_Object *gl, const char *part EINA_UNUSED)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   char *ret = NULL;
   Eina_Bool compare = !!efl_key_data_get(gl, "_exactness_gl_compare");
   if (compare)
     {
        _Compare_Item_Data *vv = data;
        Exactness_Object *e_obj1 = vv->p1;
        Exactness_Object *e_obj2 = vv->p2;
        if ((!e_obj1 ^ !e_obj2) || strcmp(e_obj1->kl_name, e_obj2->kl_name))
           eina_strbuf_append_printf(buf, "("LDIFF(%s)"/"RDIFF(%s)")",
                 e_obj1 ? e_obj1->kl_name : "XXXXX",
                 e_obj2 ? e_obj2->kl_name : "XXXXX");
        else
           eina_strbuf_append_printf(buf, "%s", e_obj1->kl_name);

        eina_strbuf_append(buf, " x = ");
        if ((!e_obj1 ^ !e_obj2) || e_obj1->x != e_obj2->x)
           eina_strbuf_append_printf(buf, LDIFF(%d)"/"RDIFF(%d),
                 e_obj1 ? e_obj1->x : -1,
                 e_obj2 ? e_obj2->x : -1);
        else
           eina_strbuf_append_printf(buf, "%d", e_obj1->x);

        eina_strbuf_append(buf, " y = ");
        if ((!e_obj1 ^ !e_obj2) || e_obj1->y != e_obj2->y)
           eina_strbuf_append_printf(buf, LDIFF(%d)"/"RDIFF(%d),
                 e_obj1 ? e_obj1->y : -1,
                 e_obj2 ? e_obj2->y : -1);
        else
           eina_strbuf_append_printf(buf, "%d", e_obj1->y);

        eina_strbuf_append(buf, " w = ");
        if ((!e_obj1 ^ !e_obj2) || e_obj1->w != e_obj2->w)
           eina_strbuf_append_printf(buf, LDIFF(%d)"/"RDIFF(%d),
                 e_obj1 ? e_obj1->w : -1,
                 e_obj2 ? e_obj2->w : -1);
        else
           eina_strbuf_append_printf(buf, "%d", e_obj1->w);

        eina_strbuf_append(buf, " h = ");
        if ((!e_obj1 ^ !e_obj2) || e_obj1->h != e_obj2->h)
           eina_strbuf_append_printf(buf, LDIFF(%d)"/"RDIFF(%d),
                 e_obj1 ? e_obj1->h : -1,
                 e_obj2 ? e_obj2->h : -1);
        else
           eina_strbuf_append_printf(buf, "%d", e_obj1->h);

        if (e_obj1 && e_obj2 && _are_objs_different(e_obj1, e_obj2, EINA_FALSE))
           eina_strbuf_append(buf, " - DIFF INSIDE");
     }
   else
     {
        Exactness_Object *e_obj = data;
        eina_strbuf_append_printf(buf,
              "%s: x = %d y = %d w = %d h = %d",
              e_obj->kl_name,
              e_obj->x, e_obj->y, e_obj->w, e_obj->h);
     }

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_itc_init()
{
   if (!_grp_itc)
     {
        _grp_itc = elm_genlist_item_class_new();
        _grp_itc->item_style = "group_index";
        _grp_itc->func.text_get = _grp_text_get;
     }

   if (!_scn_itc)
     {
        _scn_itc = elm_genlist_item_class_new();
        _scn_itc->item_style = "default_style";
        _scn_itc->func.text_get = _scn_text_get;
        _scn_itc->func.content_get = _scn_content_get;
     }

   if (!_img_itc)
     {
        _img_itc = elm_genlist_item_class_new();
        _img_itc->item_style = "full";
        _img_itc->func.content_get = _img_content_get;
     }

   if (!_objs_itc)
     {
        _objs_itc = elm_genlist_item_class_new();
        _objs_itc->item_style = "default_style";
        _objs_itc->func.text_get = _objs_text_get;
     }

   if (!_obj_itc)
     {
        _obj_itc = elm_genlist_item_class_new();
        _obj_itc->item_style = "default_style";
        _obj_itc->func.text_get = _obj_text_get;
     }
}

static void
_comp_gl_dragged_cb(Evas_Object *obj, void *data EINA_UNUSED)
{
   int x = 0, y = 0;
   Eo *gl;
   Eina_List *itr;
   elm_interface_scrollable_content_pos_get(obj, &x, &y);
   EINA_LIST_FOREACH(_gls, itr, gl)
     {
        if (gl != obj)
           elm_interface_scrollable_content_pos_set(gl, x, y, EINA_FALSE);
     }
}

static void
_obj_item_realize(Exactness_Object *ex_obj)
{
   _Item_Info *ii = eina_hash_find(_item_infos_hash, &ex_obj);
   if (!ii) return;
   if (ii->gl_item) return;
   _obj_item_realize(ii->ex_parent);
   _Item_Info *iip = eina_hash_find(_item_infos_hash, &(ii->ex_parent));
   if (iip->gl_item) elm_genlist_item_expanded_set(iip->gl_item, EINA_TRUE);
}

static void
_gl_expand_request_cb(void *data EINA_UNUSED, Evas_Object *gl, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   Eina_Bool compare = !!efl_key_data_get(gl, "_exactness_gl_compare");
   if (compare)
     {
        const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(glit);
        if (itc == _objs_itc)
          {
             _Compare_Item_Data *vv = elm_object_item_data_get(glit);
             _Item_Info *ii = eina_hash_find(_item_infos_hash, &(vv->p1));
             if (ii) elm_genlist_item_expanded_set(ii->gl_item, EINA_TRUE);
             ii = eina_hash_find(_item_infos_hash, &(vv->p2));
             if (ii) elm_genlist_item_expanded_set(ii->gl_item, EINA_TRUE);
          }
        else if (itc == _obj_itc)
          {
             _Compare_Item_Data *vv = elm_object_item_data_get(glit);
             _Item_Info *ii = eina_hash_find(_item_infos_hash, &(vv->p1));
             if (!ii || !ii->gl_item) _obj_item_realize(vv->p1);
             if (!ii) ii = eina_hash_find(_item_infos_hash, &(vv->p1));
             if (ii && ii->gl_item) elm_genlist_item_expanded_set(ii->gl_item, EINA_TRUE);

             ii = eina_hash_find(_item_infos_hash, &(vv->p2));
             if (!ii || !ii->gl_item) _obj_item_realize(vv->p2);
             if (!ii) ii = eina_hash_find(_item_infos_hash, &(vv->p2));
             if (ii && ii->gl_item) elm_genlist_item_expanded_set(ii->gl_item, EINA_TRUE);
          }
     }
   elm_genlist_item_expanded_set(glit, EINA_TRUE);
}

static void
_gl_contract_request_cb(void *data EINA_UNUSED, Evas_Object *gl EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   Eina_Bool compare = !!efl_key_data_get(gl, "_exactness_gl_compare");
   if (compare)
     {
        const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(glit);
        if (itc == _objs_itc)
          {
             _Compare_Item_Data *vv = elm_object_item_data_get(glit);
             _Item_Info *ii = eina_hash_find(_item_infos_hash, &(vv->p1));
             if (ii) elm_genlist_item_expanded_set(ii->gl_item, EINA_FALSE);
             ii = eina_hash_find(_item_infos_hash, &(vv->p2));
             if (ii) elm_genlist_item_expanded_set(ii->gl_item, EINA_FALSE);
          }
        else if (itc == _obj_itc)
          {
             _Compare_Item_Data *vv = elm_object_item_data_get(glit);
             _Item_Info *ii = eina_hash_find(_item_infos_hash, &(vv->p1));
             if (ii && ii->gl_item) elm_genlist_item_expanded_set(ii->gl_item, EINA_FALSE);

             ii = eina_hash_find(_item_infos_hash, &(vv->p2));
             if (ii && ii->gl_item) elm_genlist_item_expanded_set(ii->gl_item, EINA_FALSE);
          }
     }
   elm_genlist_item_expanded_set(glit, EINA_FALSE);
}

static void
_gl_expanded_cb(void *_data EINA_UNUSED, Evas_Object *gl EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(glit);
   Eina_Bool compare = !!efl_key_data_get(gl, "_exactness_gl_compare");
   if (itc == _objs_itc)
     {
        if (compare)
          {
             _Compare_Item_Data *vv = elm_object_item_data_get(glit);
             Exactness_Objects *e_objs1 = vv->p1;
             Exactness_Objects *e_objs2 = vv->p2;
             Eina_List *itr1 = e_objs1->main_objs, *itr2 = e_objs2->main_objs;

             while (itr1 || itr2)
               {
                  Exactness_Object *e_obj1 = eina_list_data_get(itr1);
                  Exactness_Object *e_obj2 = eina_list_data_get(itr2);
                  vv = calloc(1, sizeof(*vv));
                  vv->p1 = e_obj1;
                  vv->p2 = e_obj2;
                  vv->dt = EX_OBJ_INFO;
                  elm_genlist_item_append(gl, _obj_itc, vv, glit,
                        e_obj1->children || e_obj2->children ? ELM_GENLIST_ITEM_TREE : ELM_GENLIST_ITEM_NONE,
                        NULL, NULL);
                  itr1 = eina_list_next(itr1);
                  itr2 = eina_list_next(itr2);
               }
          }
        else
          {
             Exactness_Objects *e_objs = elm_object_item_data_get(glit);
             Eina_List *itr;
             Exactness_Object *e_obj;
             EINA_LIST_FOREACH(e_objs->main_objs, itr, e_obj)
               {
                  _Item_Info *ii = eina_hash_find(_item_infos_hash, &e_obj);
                  if (!ii)
                    {
                       ii = calloc(1, sizeof(*ii));
                       eina_hash_set(_item_infos_hash, &e_obj, ii);
                    }
                  ii->ex_parent = e_objs;
                  ii->gl_item = elm_genlist_item_append(gl, _obj_itc, e_obj, glit,
                        e_obj->children ? ELM_GENLIST_ITEM_TREE : ELM_GENLIST_ITEM_NONE,
                        NULL, NULL);
                  efl_wref_add(ii->gl_item, &(ii->gl_item));
               }
          }
     }
   else if (itc == _obj_itc)
     {
        if (compare)
          {
             _Compare_Item_Data *vv = elm_object_item_data_get(glit);
             Exactness_Object *e_obj1 = vv->p1;
             Exactness_Object *e_obj2 = vv->p2;
             Eina_List *itr1 = e_obj1->children, *itr2 = e_obj2->children;

             while (itr1 || itr2)
               {
                  e_obj1 = eina_list_data_get(itr1);
                  e_obj2 = eina_list_data_get(itr2);
                  vv = calloc(1, sizeof(*vv));
                  vv->p1 = e_obj1;
                  vv->p2 = e_obj2;
                  vv->dt = EX_OBJ_INFO;
                  elm_genlist_item_append(gl, _obj_itc, vv, glit,
                        (e_obj1 && e_obj1->children) || (e_obj2 && e_obj2->children) ?
                        ELM_GENLIST_ITEM_TREE : ELM_GENLIST_ITEM_NONE,
                        NULL, NULL);
                  itr1 = eina_list_next(itr1);
                  itr2 = eina_list_next(itr2);
               }
          }
        else
          {
             Exactness_Object *e_obj = elm_object_item_data_get(glit), *e_obj2;
             Eina_List *itr;

             EINA_LIST_FOREACH(e_obj->children, itr, e_obj2)
               {
                  _Item_Info *ii = eina_hash_find(_item_infos_hash, &e_obj2);
                  if (!ii)
                    {
                       ii = calloc(1, sizeof(*ii));
                       eina_hash_set(_item_infos_hash, &e_obj2, ii);
                    }
                  ii->ex_parent = e_obj;
                  ii->gl_item = elm_genlist_item_append(gl, _obj_itc, e_obj2, glit,
                        e_obj2->children ? ELM_GENLIST_ITEM_TREE : ELM_GENLIST_ITEM_NONE,
                        NULL, NULL);
                  efl_wref_add(ii->gl_item, &(ii->gl_item));
               }
          }
     }
}

static void
_gl_contracted_cb(void *data EINA_UNUSED, Evas_Object *gl EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   elm_genlist_item_subitems_clear(glit);
}

static void
_comp_gl_selected_cb(void *data EINA_UNUSED, Evas_Object *gl EINA_UNUSED, void *event_info)
{
   _comp_selected_item = event_info;
   _Compare_Item_Data *vv = elm_object_item_data_get(_comp_selected_item);
   if (vv->p1)
     {
        _Item_Info *ii = eina_hash_find(_item_infos_hash, &(vv->p1));
        if (!ii || !ii->gl_item) _obj_item_realize(vv->p1);
        elm_genlist_item_selected_set(ii->gl_item, EINA_TRUE);
     }

   if (vv->p2)
     {
        _Item_Info *ii = eina_hash_find(_item_infos_hash, &(vv->p2));
        if (!ii || !ii->gl_item) _obj_item_realize(vv->p2);
        elm_genlist_item_selected_set(ii->gl_item, EINA_TRUE);
     }
}

static void
_scn_item_remove(void *data, Evas_Object *menu EINA_UNUSED, void *item EINA_UNUSED)
{
   Eo *glit = data;
   Exactness_Unit *unit = efl_key_data_get(efl_parent_get(glit), "unit");
   Exactness_Action *act = elm_object_item_data_get(glit);
   unit->actions = eina_list_remove(unit->actions, act);
   if (!eina_list_data_find(_modified_units, unit))
      _modified_units = eina_list_append(_modified_units, unit);
   efl_del(glit);
}

static void
_gl_clicked_right_cb(void *data, Evas_Object *gl, void *event_info)
{
   int x = 0, y = 0;
   Eo *win = data, *menu;
   Elm_Object_Item *glit = event_info;

   if (elm_genlist_item_item_class_get(glit) == _scn_itc)
     {
        elm_genlist_item_selected_set(glit, EINA_TRUE);
        evas_pointer_canvas_xy_get(evas_object_evas_get(gl), &x, &y);

        menu = elm_menu_add(win);
        elm_menu_move(menu, x, y);
        elm_menu_item_add(menu, NULL, NULL, "Remove", _scn_item_remove, glit);
        efl_gfx_entity_visible_set(menu, EINA_TRUE);
     }
}

static void
_gl_img_show(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   static Eo *_img_win = NULL;
   Exactness_Image *ex_img = data;
   if (_img_win) efl_del(_img_win);
   _img_win = efl_add(EFL_UI_WIN_CLASS, elm_win_get(obj),
         efl_ui_win_type_set(efl_added, EFL_UI_WIN_DIALOG_BASIC),
         efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   efl_wref_add(_img_win, &_img_win);

   Evas_Object *image = elm_image_add(_img_win);
   Eo *evas_img = elm_image_object_get(image);
   evas_object_image_size_set(evas_img, ex_img->w, ex_img->h);
   evas_object_image_data_set(evas_img, ex_img->pixels);
   efl_content_set(_img_win, image);

   efl_gfx_entity_size_set(_img_win, EINA_SIZE2D(550, 500));
}

static void
_gui_unit_display(Exactness_Unit *unit1, Exactness_Unit *unit2)
{
   Eina_List *itr1, *itr2;
   Eo *gl1, *gl2 = NULL, *glc = NULL;

   gl1 = elm_genlist_add(_main_box);
   elm_genlist_homogeneous_set(gl1, EINA_TRUE);
   evas_object_size_hint_weight_set(gl1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl1, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_gfx_entity_visible_set(gl1, EINA_TRUE);
   _gls = eina_list_append(_gls, gl1);
   elm_box_pack_end(_main_box, gl1);

   efl_key_data_set(gl1, "unit", unit1);
   evas_object_smart_callback_add(gl1, "expand,request", _gl_expand_request_cb, NULL);
   evas_object_smart_callback_add(gl1, "contract,request", _gl_contract_request_cb, NULL);
   evas_object_smart_callback_add(gl1, "expanded", _gl_expanded_cb, NULL);
   evas_object_smart_callback_add(gl1, "contracted", _gl_contracted_cb, NULL);
   if (!unit2)
      evas_object_smart_callback_add(gl1, "clicked,right", _gl_clicked_right_cb, elm_win_get(_main_box));

   if (unit2)
     {
        glc = elm_genlist_add(_main_box);
        elm_genlist_homogeneous_set(glc, EINA_TRUE);
        evas_object_size_hint_weight_set(glc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(glc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        efl_gfx_entity_visible_set(glc, EINA_TRUE);
        elm_box_pack_end(_main_box, glc);

        evas_object_smart_callback_add(glc, "expand,request", _gl_expand_request_cb, NULL);
        evas_object_smart_callback_add(glc, "contract,request", _gl_contract_request_cb, NULL);
        evas_object_smart_callback_add(glc, "expanded", _gl_expanded_cb, NULL);
        evas_object_smart_callback_add(glc, "contracted", _gl_contracted_cb, NULL);

        efl_key_data_set(glc, "_exactness_gl_compare", glc);
        elm_interface_scrollable_scroll_down_cb_set(glc, _comp_gl_dragged_cb);
        elm_interface_scrollable_scroll_up_cb_set(glc, _comp_gl_dragged_cb);
        evas_object_smart_callback_add(glc, "selected", _comp_gl_selected_cb, NULL);

        gl2 = elm_genlist_add(_main_box);
        elm_genlist_homogeneous_set(gl2, EINA_TRUE);
        evas_object_size_hint_weight_set(gl2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(gl2, EVAS_HINT_FILL, EVAS_HINT_FILL);
        efl_gfx_entity_visible_set(gl2, EINA_TRUE);
        _gls = eina_list_append(_gls, gl2);
        elm_box_pack_end(_main_box, gl2);

        efl_key_data_set(gl2, "unit", unit2);
        evas_object_smart_callback_add(gl2, "expand,request", _gl_expand_request_cb, NULL);
        evas_object_smart_callback_add(gl2, "contract,request", _gl_contract_request_cb, NULL);
        evas_object_smart_callback_add(gl2, "expanded", _gl_expanded_cb, NULL);
        evas_object_smart_callback_add(gl2, "contracted", _gl_contracted_cb, NULL);
     }
   _itc_init();

   if (unit1->fonts_path || (unit2 && unit2->fonts_path))
     {
        if (!_show_only_diffs || !unit1 || !unit2 ||
              !unit1->fonts_path || !unit2->fonts_path ||
              strcmp(unit1->fonts_path, unit2->fonts_path))
          {
             elm_genlist_item_append(gl1, _grp_itc, (void *)EX_FONTS_DIR, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
             elm_genlist_item_append(gl2, _grp_itc, (void *)EX_FONTS_DIR, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
             elm_genlist_item_append(glc, _grp_itc, (void *)EX_FONTS_DIR, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
          }
     }
   itr1 = unit1 ? unit1->actions : NULL;
   itr2 = unit2 ? unit2->actions : NULL;

   if (itr1 || itr2)
     {
        elm_genlist_item_append(gl1, _grp_itc, (void *)EX_SCENARIO, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
        elm_genlist_item_append(gl2, _grp_itc, (void *)EX_SCENARIO, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
        elm_genlist_item_append(glc, _grp_itc, (void *)EX_SCENARIO, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
     }
   while (itr1 || itr2)
     {
        Exactness_Action *v1 = itr1 ? eina_list_data_get(itr1) : NULL;
        Exactness_Action *v2 = itr2 ? eina_list_data_get(itr2) : NULL;
        if (!_show_only_diffs || _are_scenario_entries_different(v1, v2))
          {
             _Item_Info *ii = calloc(1, sizeof(*ii));
             eina_hash_set(_item_infos_hash, &v1, ii);
             ii->gl_item = elm_genlist_item_append(gl1, _scn_itc, v1, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
             if (unit2)
               {
                  _Compare_Item_Data *vv = calloc(1, sizeof(*vv));
                  vv->p1 = v1;
                  vv->p2 = v2;
                  vv->dt = EX_SCENARIO;
                  ii = calloc(1, sizeof(*ii));
                  eina_hash_set(_item_infos_hash, &v2, ii);
                  ii->gl_item = elm_genlist_item_append(gl2, _scn_itc, v2, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
                  elm_genlist_item_append(glc, _scn_itc, vv, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
               }
          }
        if (itr1) itr1 = eina_list_next(itr1);
        if (itr2) itr2 = eina_list_next(itr2);
     }

   itr1 = unit1 ? unit1->imgs : NULL;
   itr2 = unit2 ? unit2->imgs : NULL;

   if (itr1 || itr2)
     {
        elm_genlist_item_append(gl1, _grp_itc, (void *)EX_IMAGE, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
        elm_genlist_item_append(gl2, _grp_itc, (void *)EX_IMAGE, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
        elm_genlist_item_append(glc, _grp_itc, (void *)EX_IMAGE, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
     }
   while (itr1 || itr2)
     {
        Exactness_Image *img1 = itr1 ? eina_list_data_get(itr1) : NULL;
        Exactness_Image *img2 = itr2 ? eina_list_data_get(itr2) : NULL;
        if (!_show_only_diffs || _are_images_different(img1, img2))
          {
             _Item_Info *ii = calloc(1, sizeof(*ii));
             eina_hash_set(_item_infos_hash, &img1, ii);
             ii->gl_item = elm_genlist_item_append(gl1, _img_itc, img1, NULL, ELM_GENLIST_ITEM_NONE, _gl_img_show, img1);
             if (unit2)
               {
                  _Compare_Item_Data *vv = calloc(1, sizeof(*vv));
                  vv->p1 = img1;
                  vv->p2 = img2;
                  vv->dt = EX_IMAGE;
                  ii = calloc(1, sizeof(*ii));
                  eina_hash_set(_item_infos_hash, &img2, ii);
                  ii->gl_item = elm_genlist_item_append(gl2, _img_itc, img2, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
                  /* This item info is needed to go to images from scenario shot entry */
                  ii = calloc(1, sizeof(*ii));
                  eina_hash_set(_item_infos_hash, &vv, ii);
                  ii->gl_item = elm_genlist_item_append(glc, _img_itc, vv, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
                  _comp_vvs = eina_list_append(_comp_vvs, vv);
               }
          }
        if (itr1) itr1 = eina_list_next(itr1);
        if (itr2) itr2 = eina_list_next(itr2);
     }

   itr1 = unit1 ? unit1->objs : NULL;
   itr2 = unit2 ? unit2->objs : NULL;

   if (itr1 || itr2)
     {
        elm_genlist_item_append(gl1, _grp_itc, (void *)EX_OBJ_INFO, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
        elm_genlist_item_append(gl2, _grp_itc, (void *)EX_OBJ_INFO, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
        elm_genlist_item_append(glc, _grp_itc, (void *)EX_OBJ_INFO, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
     }
   while (itr1 || itr2)
     {
        Exactness_Objects *objs1 = itr1 ? eina_list_data_get(itr1) : NULL;
        Exactness_Objects *objs2 = itr2 ? eina_list_data_get(itr2) : NULL;
        if (!_show_only_diffs || _are_objs_trees_different(objs1, objs2))
          {
             _Item_Info *ii = calloc(1, sizeof(*ii));
             eina_hash_set(_item_infos_hash, &objs1, ii);
             ii->gl_item = elm_genlist_item_append(gl1, _objs_itc, objs1, NULL,
                   ELM_GENLIST_ITEM_TREE, NULL, NULL);
             efl_wref_add(ii->gl_item, &(ii->gl_item));
             if (unit2)
               {
                  _Compare_Item_Data *vv = calloc(1, sizeof(*vv));
                  vv->p1 = objs1;
                  vv->p2 = objs2;
                  vv->dt = EX_OBJ_INFO;
                  ii = calloc(1, sizeof(*ii));
                  eina_hash_set(_item_infos_hash, &objs2, ii);
                  ii->gl_item = elm_genlist_item_append(gl2, _objs_itc, objs2, NULL,
                        ELM_GENLIST_ITEM_TREE, NULL, NULL);
                  efl_wref_add(ii->gl_item, &(ii->gl_item));
                  elm_genlist_item_append(glc, _objs_itc, vv, NULL, ELM_GENLIST_ITEM_TREE, NULL, NULL);
               }
          }
        if (itr1) itr1 = eina_list_next(itr1);
        if (itr2) itr2 = eina_list_next(itr2);
     }
}

static void
_diff_result_print(Exactness_Unit *unit1, Exactness_Unit *unit2)
{
   Eina_List *itr1, *itr2;

   int nb_scenario = 0, nb_diff_scenario = 0;
   int nb_image = 0, nb_diff_image = 0;
   int nb_objtree= 0, nb_diff_objtree = 0;

   itr1 = unit1 ? unit1->actions : NULL;
   itr2 = unit2 ? unit2->actions : NULL;

   while (itr1 || itr2)
     {
        Exactness_Action *v1 = itr1 ? eina_list_data_get(itr1) : NULL;
        Exactness_Action *v2 = itr2 ? eina_list_data_get(itr2) : NULL;

        nb_scenario++;
        if (_are_scenario_entries_different(v1, v2))
          nb_diff_scenario++;

        if (itr1) itr1 = eina_list_next(itr1);
        if (itr2) itr2 = eina_list_next(itr2);
     }

   itr1 = unit1 ? unit1->imgs : NULL;
   itr2 = unit2 ? unit2->imgs : NULL;

   while (itr1 || itr2)
     {
        Exactness_Image *img1 = itr1 ? eina_list_data_get(itr1) : NULL;
        Exactness_Image *img2 = itr2 ? eina_list_data_get(itr2) : NULL;

        nb_image++;
        if (_are_images_different(img1, img2))
          nb_diff_image++;

        if (itr1) itr1 = eina_list_next(itr1);
        if (itr2) itr2 = eina_list_next(itr2);
     }

   itr1 = unit1 ? unit1->objs : NULL;
   itr2 = unit2 ? unit2->objs : NULL;

   while (itr1 || itr2)
     {
        Exactness_Objects *objs1 = itr1 ? eina_list_data_get(itr1) : NULL;
        Exactness_Objects *objs2 = itr2 ? eina_list_data_get(itr2) : NULL;

        nb_objtree++;
        if (_are_objs_trees_different(objs1, objs2))
          nb_diff_objtree++;

        if (itr1) itr1 = eina_list_next(itr1);
        if (itr2) itr2 = eina_list_next(itr2);
     }

   printf("%s\nscenario (%d/%d)\nimage (%d/%d)\nobjs_tree (%d/%d)\n",
          nb_diff_scenario || nb_diff_image || nb_diff_objtree ?
          "Failure" : "Success",
          nb_scenario - nb_diff_scenario, nb_scenario,
          nb_image - nb_diff_image, nb_image,
          nb_objtree - nb_diff_objtree, nb_objtree);
}

static Exactness_Image *
_image_read(const char *filename)
{
   int w, h;
   Evas_Load_Error err;
   Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, 100, 100, NULL);

   /* the canvas pointer, de facto */
   Eo *e = ecore_evas_get(ee);

   Eo *img = evas_object_image_add(e);
   evas_object_image_file_set(img, filename, NULL);
   err = evas_object_image_load_error_get(img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        fprintf(stderr, "could not load image '%s'. error string is \"%s\"\n",
              filename, evas_load_error_str(err));
        return NULL;
     }

   Exactness_Image *ex_img = malloc(sizeof(*ex_img));
   int len;
   evas_object_image_size_get(img, &w, &h);
   ex_img->w = w;
   ex_img->h = h;
   len = w * h * 4;
   ex_img->pixels = malloc(len);
   memcpy(ex_img->pixels, evas_object_image_data_get(img, EINA_FALSE), len);

   ecore_evas_free(ee);
   return ex_img;
}

static const Ecore_Getopt optdesc = {
  "exactness_inspect",
  "%prog [options] [<rec file> | <file1 file2>]",
  NULL,
  "(C) 2016 Enlightenment",
  "BSD",
  "Inspector for Exactness",
  0,
  {
    ECORE_GETOPT_STORE_USHORT('d', "delay", "Delay the given recording by a given time (in milliseconds)."),
    ECORE_GETOPT_STORE_TRUE('c', "clean", "Clean the given recording from wrong actions."),
    ECORE_GETOPT_STORE_TRUE('l', "list", "List the actions of the given recording."),
    ECORE_GETOPT_STORE_TRUE('C', "compare", "Compare given files (images files or objects eet files)."),
    ECORE_GETOPT_STORE_TRUE(0, "show-only-diffs", "Show only differences during comparison."),
    ECORE_GETOPT_STORE_TRUE(0, "stabilize", "Stabilize after the given shot number in --shot."),
    ECORE_GETOPT_STORE_TRUE(0, "pack", "Pack the given input files (scenario and images) into the given output."),
    ECORE_GETOPT_STORE_STR('o', "output", "Output."),
    ECORE_GETOPT_STORE_USHORT('s', "shot", "Select a specific shot (1 = 1st shot...)."),

    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char *argv[])
{
   Eina_List *units_filenames = NULL;
   const char *ext = NULL;
   char *output = NULL;
   Exactness_Unit *unit = NULL;
   int ret = 1, args = 0;
   unsigned short delay = 0, shot = 0;
   Eina_Bool write_file = EINA_FALSE;
   Eina_Bool want_quit, clean = EINA_FALSE, list_get = EINA_FALSE, compare_files = EINA_FALSE;
   Eina_Bool stabilize = EINA_FALSE, show_only_diffs = EINA_FALSE, gui_needed = EINA_TRUE;
   Eina_Bool pack = EINA_FALSE;

   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_USHORT(delay),
     ECORE_GETOPT_VALUE_BOOL(clean),
     ECORE_GETOPT_VALUE_BOOL(list_get),
     ECORE_GETOPT_VALUE_BOOL(compare_files),
     ECORE_GETOPT_VALUE_BOOL(show_only_diffs),
     ECORE_GETOPT_VALUE_BOOL(stabilize),
     ECORE_GETOPT_VALUE_BOOL(pack),
     ECORE_GETOPT_VALUE_STR(output),
     ECORE_GETOPT_VALUE_USHORT(shot),

     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_NONE
   };

   ecore_evas_init();
   ecore_init();
   eet_init();
   elm_init(0, NULL);
   want_quit = EINA_FALSE;

   args = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (args < 0)
     {
        fprintf(stderr, "Failed parsing arguments.\n");
        goto end;
     }
   if (want_quit)
     {
        goto end;
     }
   if ((clean || delay || shot || list_get || stabilize || pack) && args == argc)
     {
        fprintf(stderr, "Expected scenario (.rec/.exu) as the last argument.\n");
        ecore_getopt_help(stderr, &optdesc);
        goto end;
     }
   if (shot && (!delay && !stabilize))
     {
        fprintf(stderr, "shot option can only be used with delay or stabilize option.\n");
        goto end;
     }
   if (delay && !shot)
     {
        fprintf(stderr, "delay option can only be used with shot option.\n");
        goto end;
     }
   if (stabilize && !shot)
     {
        fprintf(stderr, "stabilize option can only be used with shot option.\n");
        goto end;
     }
   if (compare_files && argc - args < 2)
     {
        fprintf(stderr, "Expected at least two files to compare as last arguments.\n");
        ecore_getopt_help(stderr, &optdesc);
        goto end;
     }
   if (show_only_diffs && !compare_files)
     {
        fprintf(stderr, "--show-only-diffs is available with --compare only\n");
        goto end;
     }
   if (show_only_diffs && output)
     {
        fprintf(stderr, "--show-only-diffs works in GUI only\n");
        goto end;
     }
   _show_only_diffs = show_only_diffs;

   if (clean || delay || list_get || stabilize || pack)
     {
        int arg;
        Eina_List *images = NULL;
        gui_needed = EINA_FALSE;
        for (arg = args; arg < argc; arg++)
          {
             const char *src_file = argv[arg];
             ext = strrchr(src_file, '.');
             if (!ext)
               {
                  fprintf(stderr, "Extension required\n");
                  goto end;
               }
             if (!strcmp(ext, ".exu"))
               {
                  if (!unit) unit = exactness_unit_file_read(src_file);
                  else
                    {
                       fprintf(stderr, "%s - scenario already provided\n", src_file);
                       goto end;
                    }
               }
             else if (!strcmp(ext, ".rec"))
               {
                  if (!unit) unit = legacy_rec_file_read(src_file);
                  else
                    {
                       fprintf(stderr, "%s - scenario already provided\n", src_file);
                       goto end;
                    }
               }
             else if (!strcmp(ext, ".png"))
               {
                  Exactness_Image *ex_img = _image_read(src_file);
                  if (!ex_img)
                    {
                       fprintf(stderr, "Issue while reading %s\n", src_file);
                       goto end;
                    }
                  images = eina_list_append(images, ex_img);
               }
             else
               {
                  fprintf(stderr, "Correct extension (.exu/.rec/.png) required\n");
                  goto end;
               }
          }
        if (unit)
          {
             Exactness_Image *ex_img;
             EINA_LIST_FREE(images, ex_img)
               {
                  unit->imgs = eina_list_append(unit->imgs, ex_img);
                  unit->nb_shots++;
               }
          }
     }
   else
     {
        int arg;
        if (output) gui_needed = EINA_FALSE;
        for (arg = args; arg < argc; arg++)
          {
             ext = strrchr(argv[arg], '.');
             if (!ext)
               {
                  fprintf(stderr, "Extension required\n");
                  goto end;
               }
             if (!strcmp(ext, ".exu"))
               {
                  Exactness_Unit *ex_unit = exactness_unit_file_read(argv[arg]);
                  units_filenames = eina_list_append(units_filenames, argv[arg]);
                  _units = eina_list_append(_units, ex_unit);
               }
             else if (!strcmp(ext, ".rec"))
               {
                  Exactness_Unit *ex_unit = legacy_rec_file_read(argv[arg]);
                  if (!ex_unit)
                    {
                       fprintf(stderr, "Issue while reading %s\n", argv[arg]);
                       goto end;
                    }
                  _units = eina_list_append(_units, ex_unit);
               }
             else if (!strcmp(ext, ".png"))
               {
                  Exactness_Unit *ex_unit = calloc(1, sizeof(*ex_unit));
                  Exactness_Image *ex_img = _image_read(argv[arg]);
                  if (!ex_img)
                    {
                       fprintf(stderr, "Issue while reading %s\n", argv[arg]);
                       goto end;
                    }
                  ex_unit->imgs = eina_list_append(ex_unit->imgs, ex_img);
                  ex_unit->nb_shots++;
                  _units = eina_list_append(_units, ex_unit);
               }
          }
     }

   if (clean)
     {
        Exactness_Action *act;
        Eina_List *itr, *itr2;
        EINA_LIST_FOREACH_SAFE(unit->actions, itr, itr2, act)
          {
             Exactness_Action *prev_act = eina_list_data_get(eina_list_prev(itr));
             if (_is_hook_duplicate(act, prev_act))
               {
                  prev_act->delay_ms += act->delay_ms;
                  unit->actions = eina_list_remove_list(unit->actions, itr);
               }
          }
        EINA_LIST_REVERSE_FOREACH_SAFE(unit->actions, itr, itr2, act)
          {
             if (act->type == EXACTNESS_ACTION_TAKE_SHOT) break;
             unit->actions = eina_list_remove(unit->actions, act);
          }
        write_file = EINA_TRUE;
     }

   if (delay || stabilize)
     {
        Exactness_Action *act;
        Eina_List *itr;
        unsigned int cur_shot = 0;
        EINA_LIST_FOREACH(unit->actions, itr, act)
          {
             if (act->type == EXACTNESS_ACTION_TAKE_SHOT)
               {
                  cur_shot++;
                  if (cur_shot == shot)
                    {
                       if (delay) act->delay_ms = delay;
                       if (stabilize)
                         {
                            Exactness_Action *s_act = malloc(sizeof(*s_act));
                            s_act->type = EXACTNESS_ACTION_STABILIZE;
                            s_act->delay_ms = act->delay_ms;
                            s_act->n_evas = act->n_evas;
                            s_act->data = NULL;
                            act->delay_ms = 0; /* Shot right after stabilization */
                            unit->actions = eina_list_prepend_relative(unit->actions, s_act, act);
                         }
                       write_file = EINA_TRUE;
                       break;
                    }
               }
          }
     }

   if (pack) write_file = EINA_TRUE;

   if (list_get)
     {
        Exactness_Action *act;
        Eina_List *itr;
        if (unit->fonts_path) printf("Fonts dir: %s\n", unit->fonts_path);
        EINA_LIST_FOREACH(unit->actions, itr, act)
          {
             char specific_output[1024];
             printf("+%.3f: %s", act->delay_ms / 1000.0, _action_name_get(act));
             _action_specific_info_get(act, specific_output);
             if (*specific_output) printf(" - %s", specific_output);
             printf("\n");
          }
     }

   if (compare_files && output)
     {
        const char *out_ext = strrchr(output, '.');
        Exactness_Unit *unit1 = NULL, *unit2 = NULL, *unitO = NULL;
        int nb_diffs = 0;
        Eina_List *itr1, *itr2;
        EINA_LIST_FOREACH(_units, itr1, unit)
          {
             if (!unit1) unit1 = unit;
             else if (!unit2) unit2 = unit;
             else
               {
                  fprintf(stderr, "Too much files to compare (only 2).\n");
                  goto end;
               }
          }

        if (!strcmp(out_ext, ".png"))
          {
             if (unit1->nb_shots != 1 || unit2->nb_shots != 1)
               {
                  fprintf(stderr, "Comparison output can be png only if the number of shots to compare is 1.\n");
                  goto end;
               }
          }

        itr1 = unit1 ? unit1->imgs : NULL;
        itr2 = unit2 ? unit2->imgs : NULL;

        while (itr1 || itr2)
          {
             Exactness_Image *ex_img1 = itr1 ? eina_list_data_get(itr1) : NULL;
             Exactness_Image *ex_img2 = itr2 ? eina_list_data_get(itr2) : NULL;
             Exactness_Image *ex_imgO = NULL;
             Eina_Bool has_diff = exactness_image_compare(ex_img1, ex_img2, &ex_imgO);
             if (has_diff || !strcmp(out_ext, ".exu"))
               {
                  if (has_diff) nb_diffs++;
                  if (!unitO) unitO = calloc(1, sizeof(*unitO));
                  unitO->imgs = eina_list_append(unitO->imgs, ex_imgO);
                  unitO->nb_shots++;
               }
             itr1 = eina_list_next(itr1);
             itr2 = eina_list_next(itr2);
          }
        if (!strcmp(out_ext, ".png"))
          {
             Ecore_Evas *ee;
             Eo *e, *img;
             if (unitO->nb_shots == 1)
               {
                  Exactness_Image *ex_imgO = eina_list_data_get(unitO->imgs);
                  ee = ecore_evas_new(NULL, 0, 0, 100, 100, NULL);
                  e = ecore_evas_get(ee);
                  img = evas_object_image_add(e);
                  evas_object_image_size_set(img, ex_imgO->w, ex_imgO->h);
                  evas_object_image_data_set(img, ex_imgO->pixels);
                  evas_object_image_save(img, output, NULL, NULL);
                  ecore_evas_free(ee);
                  goto end;
               }
             ret = 0;
          }
        else if (!strcmp(out_ext, ".exu"))
          {
             _diff_result_print(unit1, unit2);
             if (nb_diffs) exactness_unit_file_write(unitO, output);
             else ret = 0;
          }
        else
          {
             fprintf(stderr, "Correct output extension (.exu/.png) required\n");
          }
        goto end;
     }

   ret = 0;
   if (write_file)
     {
        if (!output)
          {
             fprintf(stderr, "An output file is required to write the modifications.\n");
          }
        else
          {
             const char *out_ext = strrchr(output, '.');
             if (!out_ext || strcmp(out_ext, ".exu"))
               {
                  fprintf(stderr, "Only exu extension is supported as output.\n");
                  goto end;
               }
             exactness_unit_file_write(unit, output);
          }
        goto end;
     }

   if (gui_needed)
     {
        Eina_List *itr;
        Exactness_Unit *unit1 = NULL, *unit2 = NULL;
        Eina_Bool need_compare = compare_files && eina_list_count(_units) == 2;
        _item_infos_hash = eina_hash_pointer_new(NULL);
        _gui_win_create();
        EINA_LIST_FOREACH(_units, itr, unit)
          {
             if (need_compare)
               {
                  if (!unit1) unit1 = unit;
                  else unit2 = unit;
               }
             else _gui_unit_display(unit, NULL);
          }
        if (need_compare) _gui_unit_display(unit1, unit2);
        elm_run();
        EINA_LIST_FREE(_modified_units, unit)
          {
             int i = 0;
             EINA_LIST_FOREACH(_units, itr, unit2)
               {
                  if (unit2 == unit) break;
                  i++;
               }
             exactness_unit_file_write(unit, eina_list_nth(units_filenames, i));
          }
     }

end:
   elm_shutdown();
   eet_shutdown();
   ecore_shutdown();
   ecore_evas_shutdown();

   return ret;
}
