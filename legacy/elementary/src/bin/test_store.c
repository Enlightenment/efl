#include <Elementary.h>

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

typedef struct _My_Item My_Item;

struct _My_Item
{
  char *from, *subject, *date, *head_content;
};

// callbacks just to see user interacting with genlist
static void
_st_selected(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("selected: %p\n", event_info);
}

static void
_st_double_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("double clicked: %p\n", event_info);
}

static void
_st_longpress(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   printf("longpress %p\n", event_info);
}

// store callbacks to handle loading/parsing/freeing of store items from src
static Elm_Genlist_Item_Class itc1 =
{
  "message", { NULL, NULL, NULL, NULL, NULL }, NULL
};

static const Elm_Store_Item_Mapping it1_mapping[] =
{
  {
    ELM_STORE_ITEM_MAPPING_LABEL,
      "elm.title.1", ELM_STORE_ITEM_MAPPING_OFFSET(My_Item, from),
      { .empty = {
        EINA_TRUE
      } } },
  {
    ELM_STORE_ITEM_MAPPING_LABEL,
      "elm.title.2", ELM_STORE_ITEM_MAPPING_OFFSET(My_Item, subject),
      { .empty = {
        EINA_TRUE
      } } },
  {
    ELM_STORE_ITEM_MAPPING_LABEL,
      "elm.text", ELM_STORE_ITEM_MAPPING_OFFSET(My_Item, head_content),
      { .empty = {
        EINA_TRUE
      } } },
  {
    ELM_STORE_ITEM_MAPPING_ICON,
      "elm.swallow.icon", 0,
      { .icon = {
        48, 48,
        ELM_ICON_LOOKUP_THEME_FDO,
        EINA_TRUE, EINA_FALSE,
        EINA_TRUE,
        EINA_FALSE, EINA_FALSE,
      } } },
  {
    ELM_STORE_ITEM_MAPPING_CUSTOM,
      "elm.swallow.end", 0,
      { .custom = {
        NULL
      } } },
  ELM_STORE_ITEM_MAPPING_END
};


////// **** WARNING ***********************************************************
////   * This function runs inside a thread outside efl mainloop. Be careful! *
//     ************************************************************************
static Eina_Bool
_st_store_list(void *data __UNUSED__, Elm_Store_Item_Info *item_info)
{
  Elm_Store_Item_Info_Filesystem *info = (Elm_Store_Item_Info_Filesystem *)item_info;
  int id;
  char sort_id[7];

  // create a sort id based on the filename itself assuming it is a numeric
  // value like the id number in mh mail folders which is what this test
  // uses as a data source
  char *file = strrchr(info->path, '/');
  if (file) file++;
  else file = info->path;
  id = atoi(file);
  sort_id[0] = ((id >> 30) & 0x3f) + 32;
  sort_id[1] = ((id >> 24) & 0x3f) + 32;
  sort_id[2] = ((id >> 18) & 0x3f) + 32;
  sort_id[3] = ((id >> 12) & 0x3f) + 32;
  sort_id[4] = ((id >>  6) & 0x3f) + 32;
  sort_id[5] = ((id >>  0) & 0x3f) + 32;
  sort_id[6] = 0;
  info->base.sort_id = strdup(sort_id);
  // choose the item genlist item class to use (only item style should be
  // provided by the app, store will fill everything else in, so it also
  // has to be writable
  info->base.item_class = &itc1; // based on item info - return the item class wanted (only style field used - rest reset to internal funcs store sets up to get label/icon etc)
  info->base.mapping = it1_mapping;
  info->base.data = NULL; // if we can already parse and load all of item here and want to - set this
  return EINA_TRUE; // return true to include this, false not to
}
//     ************************************************************************
////   * End of separate thread function.                                     *
////// ************************************************************************

////// **** WARNING ***********************************************************
////   * This function runs inside a thread outside efl mainloop. Be careful! *
//     ************************************************************************
static void
_st_store_fetch(void *data __UNUSED__, Elm_Store_Item *sti)
{
  const char *path = elm_store_item_filesystem_path_get(sti);
  My_Item *myit;
  FILE *f;
  char buf[4096], *p;
  Eina_Bool have_content = EINA_FALSE;
  char *content = NULL, *content_pos = NULL, *content_end = NULL;

  // if we already have my item data - skip
  if (elm_store_item_data_get(sti)) return;
  // open the mail file and parse it
  f = fopen(path, "rb");
  if (!f) return;

  // alloc my item in memory that holds data to show in the list
  myit = calloc(1, sizeof(My_Item));
  if (!myit)
    {
      fclose(f);
      return;
    }
  while (fgets(buf, sizeof(buf), f))
    {
      if (!have_content)
        {
          if (!isblank(buf[0]))
            {
              // get key: From:, Subject: etc.
              if (!strncmp(buf, "From:", 5))
                {
                  p = buf + 5;
                  while ((*p) && (isblank(*p))) p++;
                  p = strdup(p);
                  if (p)
                    {
                      myit->from = p;
                      p = strchr(p, '\n');
                      if (p) *p = 0;
                    }
                }
              else if (!strncmp(buf, "Subject:", 8))
                {
                  p = buf + 8;
                  while ((*p) && (isblank(*p))) p++;
                  p = strdup(p);
                  if (p)
                    {
                      myit->subject = p;
                      p = strchr(p, '\n');
                      if (p) *p = 0;
                    }
                }
              else if (!strncmp(buf, "Date:", 5))
                {
                  p = buf + 5;
                  while ((*p) && (isblank(*p))) p++;
                  p = strdup(p);
                  if (p)
                    {
                      myit->date = p;
                      p = strchr(p, '\n');
                      if (p) *p = 0;
                    }
                }
              else if (buf[0] == '\n') // begin of content
                have_content = EINA_TRUE;
            }
        }
      else
        {
          // get first 320 bytes of content/body
          if (!content)
            {
              content = calloc(1, 320);
              content_pos = content;
              content_end = content + 319;
            }
          strncat(content_pos, buf, content_end - content_pos - 1);
          content_pos = content + strlen(content);
        }
    }
  fclose(f);
  myit->head_content = elm_entry_utf8_to_markup(content);
  free(content);
  elm_store_item_data_set(sti, myit);
}
//     ************************************************************************
////   * End of separate thread function.                                     *
////// ************************************************************************

static void
_st_store_unfetch(void *data __UNUSED__, Elm_Store_Item *sti)
{
  My_Item *myit = elm_store_item_data_get(sti);
  if (!myit) return;
  if (myit->from) free(myit->from);
  if (myit->subject) free(myit->subject);
  if (myit->date) free(myit->date);
  if (myit->head_content) free(myit->head_content);
  free(myit);
  elm_store_item_data_set(sti, NULL);
}

void
test_store(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
  Evas_Object *win, *bg, *gl, *bx;

  Elm_Store *st;

  win = elm_win_add(NULL, "store", ELM_WIN_BASIC);
  elm_win_title_set(win, "Store");
  elm_win_autodel_set(win, 1);

  bg = elm_bg_add(win);
  elm_win_resize_object_add(win, bg);
  evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_show(bg);

  bx = elm_box_add(win);
  evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  elm_win_resize_object_add(win, bx);
  evas_object_show(bx);

  gl = elm_genlist_add(win);
  elm_genlist_height_for_width_mode_set(gl, EINA_TRUE);
  evas_object_smart_callback_add(gl, "selected", _st_selected, NULL);
  evas_object_smart_callback_add(gl, "clicked,double", _st_double_clicked, NULL);
  evas_object_smart_callback_add(gl, "longpressed", _st_longpress, NULL);
  evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
  evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
  elm_box_pack_end(bx, gl);
  evas_object_show(gl);

  st = elm_store_filesystem_new();
  elm_store_list_func_set(st, _st_store_list, NULL);
  elm_store_fetch_func_set(st, _st_store_fetch, NULL);
  //elm_store_fetch_thread_set(st, EINA_FALSE);
  elm_store_unfetch_func_set(st, _st_store_unfetch, NULL);
  elm_store_sorted_set(st, EINA_TRUE);
  elm_store_target_genlist_set(st, gl);
  elm_store_filesystem_directory_set(st, "./store");

  evas_object_resize(win, 480, 800);
  evas_object_show(win);
}
#endif
