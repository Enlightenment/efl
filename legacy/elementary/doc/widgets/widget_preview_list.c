#include "widget_preview_tmpl_head.c"

static const char *dict[] = \
{
   "awkward",
   "businessman",
   "cylinder",
   "dying",
   "extremophile",
   "futhark",
   "guttural",
   "hypocrites",
   "issuing",
   "just",
   "knows",
   "lying",
   "mystery",
   "nutrients",
   "oxymoron",
   "putting",
   "quizzes",
   "running",
   "systematically",
   "tyranny",
   "usually",
   "vacuum",
   "want",
   "xenophobia",
   "yacht",
   "zebra"
};

Evas_Object *o = elm_list_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

unsigned int i;
for (i = 0; i < (sizeof(dict) / sizeof(dict[0])); i++)
  {
     char buf[32];
     elm_list_item_append(o, dict[i], NULL, NULL, NULL, NULL);
     snprintf(buf, sizeof(buf), "%c", dict[i][0]);
  }

#include "widget_preview_tmpl_foot.c"
