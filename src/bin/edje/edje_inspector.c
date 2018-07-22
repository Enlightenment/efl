#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <locale.h>
#include <fnmatch.h>
#include <unistd.h>
#include <errno.h>

#include "Edje.h"
#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT 1
#include "Edje_Edit.h"

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Getopt.h>

static int _log_dom;
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_log_dom, __VA_ARGS__)

#define INDENT          "   "
#define INDENT2         INDENT INDENT
#define INDENT3         INDENT2 INDENT
#define INDENT4         INDENT3 INDENT
#define INDENT5         INDENT4 INDENT
#define INDENT6         INDENT5 INDENT
#define INDENT7         INDENT6 INDENT

#define FLOAT_PRECISION 0.0001
#define FDIFF(a, b) (fabs((a) - (b)) > FLOAT_PRECISION)

/* context */
static Eina_List *groups;
static Ecore_Evas *ee;

/* options */
static const char *file;
static char *group = NULL;
static char *part = NULL;
static char *program = NULL;
static int detail = 1;
static Eina_Bool api_only = EINA_FALSE;
static Eina_Bool api_fix = EINA_FALSE;
static Eina_Bool machine = EINA_FALSE;

static const char *mode_choices[] = {
   "groups",
   "parts",
   "programs",
   "groups-names",
   "part-names",
   "global-data",
   "images",
   "fonts",
   "externals",
   NULL,
};

static const char *detail_choices[] = {
   "none",
   "terse",
   "all",
   NULL
};

const Ecore_Getopt optdesc = {
   "edje_inspector",
   "%prog [options] <file.edj>",
   PACKAGE_VERSION,
   "(C) 2010 - The Enlightenment Project",
   "BSD",
   "Edje file inspector, let one see groups, parts, programs and other details "
   "of a compiled (binary) edje file.\n",
   0,
   {
      ECORE_GETOPT_CHOICE('m', "mode", "Choose which mode to operate on file.",
                          mode_choices),
      ECORE_GETOPT_CHOICE('d', "detail", "Choose detail level (default=terse)",
                          detail_choices),
      ECORE_GETOPT_STORE_STR('g', "group", "Limit output to group (or glob)."),
      ECORE_GETOPT_STORE_STR('p', "part", "Limit output to part (or glob)."),
      ECORE_GETOPT_STORE_STR('r', "program",
                             "Limit output to program (or glob)."),
      ECORE_GETOPT_STORE_TRUE('a', "api-only", "Limit to just api parts or "
                                               "programs."),
      ECORE_GETOPT_STORE_TRUE('A', "api-fix", "Fix API names to be C compliant."),
      ECORE_GETOPT_STORE_TRUE('M', "machine", "Produce machine readable output."),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_COPYRIGHT('C', "copyright"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
   }
};

static inline Eina_Bool
matches(const char *name, const char *pattern)
{
   if (!pattern) return EINA_TRUE;
   return fnmatch(pattern, name, 0) == 0;
}

static void
group_begin(const char *name)
{
   if (machine) printf("GROUP-BEGIN\nNAME: %s\n", name);
   else printf("group { name: '%s';\n", name);
}

static void
group_end(void)
{
   if (machine) puts("GROUP-END");
   else puts("}");
}

static void
group_details(Evas_Object *ed)
{
   int w, h;

   if (detail < 1) return;

   if (machine) puts("GROUP-DETAILS-BEGIN");

   w = edje_edit_group_min_w_get(ed);
   h = edje_edit_group_min_h_get(ed);
   if (machine) printf("MIN-W: %d\nMIN-H: %d\n", w, h);
   else if ((w > 0) || (h > 0))
     printf(INDENT "min: %d %d;\n", w, h);

   w = edje_edit_group_max_w_get(ed);
   h = edje_edit_group_max_h_get(ed);
   if (machine) printf("MAX-W: %d\nMAX-H: %d\n", w, h);
   else if ((w > 0) || (h > 0))
     printf(INDENT "max: %d %d;\n", w, h);

   if (detail > 1)
     {
        Eina_List *dl;
        dl = edje_edit_group_data_list_get(ed);
        if (dl)
          {
             Eina_List *l;
             const char *k;
             if (machine) puts(INDENT "GROUP-DETAILS-DATA-BEGIN");
             else puts(INDENT "data {");

             EINA_LIST_FOREACH(dl, l, k)
               {
                  const char *v = edje_edit_group_data_value_get(ed, k);
                  if (machine) printf("ITEM: \"%s\" \"%s\"\n", k, v);
                  else printf(INDENT2 "item: \"%s\" \"%s\";\n", k, v);
               }

             edje_edit_string_list_free(dl);

             if (machine) puts(INDENT "GROUP-DETAILS-DATA-END");
             else puts(INDENT "}");
          }
     }

   if (machine) puts("GROUP-DETAILS-END");
}

static void
parts_begin(void)
{
   if (machine) puts("PARTS-BEGIN");
   else puts(INDENT "parts {");
}

static void
parts_end(void)
{
   if (machine) puts("PARTS-END");
   else puts(INDENT "}");
}

static const char *
part_type_name_get(Edje_Part_Type t)
{
   switch (t)
     {
      case EDJE_PART_TYPE_RECTANGLE:
        return "RECT";

      case EDJE_PART_TYPE_TEXT:
        return "TEXT";

      case EDJE_PART_TYPE_IMAGE:
        return "IMAGE";

      case EDJE_PART_TYPE_PROXY:
        return "PROXY";

      case EDJE_PART_TYPE_SWALLOW:
        return "SWALLOW";

      case EDJE_PART_TYPE_TEXTBLOCK:
        return "TEXTBLOCK";

      case EDJE_PART_TYPE_GRADIENT:
        return "GRADIENT";

      case EDJE_PART_TYPE_GROUP:
        return "GROUP";

      case EDJE_PART_TYPE_BOX:
        return "BOX";

      case EDJE_PART_TYPE_TABLE:
        return "TABLE";

      case EDJE_PART_TYPE_EXTERNAL:
        return "EXTERNAL";

      case EDJE_PART_TYPE_SPACER:
        return "SPACER";

      case EDJE_PART_TYPE_VECTOR:
        return "VECTOR";

      case EDJE_PART_TYPE_NONE:
      case EDJE_PART_TYPE_LAST:
        ERR("Invalid part type %d", t);
        return "???";

      default:
        ERR("Unknown part type %d", t);
        return "???";
     }
}

static void
state_begin(const char *state, double value)
{
   if (machine)
     printf("PART-STATE-BEGIN\nNAME: %s\nVALUE: %2.1f\n", state, value);
   else
     {
        printf(INDENT3 "description { state: \"%s\" %2.1f;", state, value);
        if (detail > 0) putchar('\n');
     }
}

static const char *
aspect_pref_name_get(int id)
{
   switch (id)
     {
      case 0: return "NONE";

      case 1: return "VERTICAL";

      case 2: return "HORIZONTAL";

      case 3: return "BOTH";

      default:
        ERR("Unknown aspect preference %d", id);
        return "???";
     }
}

static const char *
border_fill_name_get(int id)
{
   switch (id)
     {
      case 0: return "NONE";

      case 1: return "DEFAULT";

      case 2: return "SOLID";

      default:
        ERR("Unknown border fill %d", id);
        return "???";
     }
}

static void
state_details(Evas_Object *ed, const char *ppart, const char *state, double value)
{
   Edje_Part_Type t = edje_edit_part_type_get(ed, ppart);
   double dx, dy;
   const char *str, *str2;
   int x, y, r, g, b, a;

   if (detail < 1) return;

   b = edje_edit_state_visible_get(ed, ppart, state, value);
   if (machine) printf("VISIBLE: %d\n", b);
   else if (!b)
     puts(INDENT4 "visible: 0;");

   edje_edit_state_color_get(ed, ppart, state, value, &r, &g, &b, &a);
   if (machine)
     printf("COLOR-R: %d\nCOLOR-G: %d\nCOLOR-B: %d\nCOLOR-A: %d\n", r, g, b, a);
   else if ((r != 255) || (g != 255) || (b != 255) || (a != 255))
     printf(INDENT4 "color: %d %d %d %d;\n", r, g, b, a);

   if (detail > 1)
     {
        edje_edit_state_color2_get(ed, ppart, state, value, &r, &g, &b, &a);
        if (machine)
          printf("COLOR2-R: %d\nCOLOR2-G: %d\nCOLOR2-B: %d\nCOLOR2-A: %d\n",
                 r, g, b, a);
        else if ((r != 255) || (g != 255) || (b != 255) || (a != 255))
          printf(INDENT4 "color2: %d %d %d %d;\n", r, g, b, a);

        edje_edit_state_color3_get(ed, ppart, state, value, &r, &g, &b, &a);
        if (machine)
          printf("COLOR3-R: %d\nCOLOR3-G: %d\nCOLOR3-B: %d\nCOLOR3-A: %d\n",
                 r, g, b, a);
        else if ((r != 255) || (g != 255) || (b != 255) || (a != 255))
          printf(INDENT4 "color3: %d %d %d %d;\n", r, g, b, a);
     }

   dx = edje_edit_state_align_x_get(ed, ppart, state, value);
   dy = edje_edit_state_align_y_get(ed, ppart, state, value);
   if (machine) printf("ALIGN-X: %g\nALIGN-Y: %g\n", dx, dy);
   else if (FDIFF(dx, 0.5) || FDIFF(dy, 0.5))
     printf(INDENT4 "align: %g %g;\n", dx, dy);

   x = edje_edit_state_min_w_get(ed, ppart, state, value);
   y = edje_edit_state_min_h_get(ed, ppart, state, value);
   if (machine) printf("MIN-W: %d\nMIN-H: %d\n", x, y);
   else if ((x) || (y))
     printf(INDENT4 "min: %d %d;\n", x, y);

   x = edje_edit_state_max_w_get(ed, ppart, state, value);
   y = edje_edit_state_max_h_get(ed, ppart, state, value);
   if (machine) printf("MAX-W: %d\nMAX-H: %d\n", x, y);
   else if ((x != -1) || (y != -1))
     printf(INDENT4 "max: %d %d;\n", x, y);

   //TODO Support fixed
   //TODO Support step

   if (detail > 1)
     {
        dx = edje_edit_state_aspect_min_get(ed, ppart, state, value);
        dy = edje_edit_state_aspect_max_get(ed, ppart, state, value);
        if (machine) printf("ASPECT-MIN: %g\nASPECT-MAX: %g\n", dx, dy);
        else if (FDIFF(dx, 0.0) || FDIFF(dy, 0.0))
          printf(INDENT4 "aspect: %g %g;\n", dx, dy);

        x = edje_edit_state_aspect_pref_get(ed, ppart, state, value);
        str = aspect_pref_name_get(x);
        if (machine) printf("ASPECT-PREFERENCE: %s\n", str);
        else if (x)
          printf(INDENT4 "aspect_preference: %s;\n", str);
        /* do not free this str! */

        str = edje_edit_state_color_class_get(ed, ppart, state, value);
        if (machine) printf("COLOR_CLASS: %s\n", str ? str : "");
        else if (str)
          printf(INDENT4 "color_class: \"%s\";\n", str);
        edje_edit_string_free(str);
     }

   dx = edje_edit_state_rel1_relative_x_get(ed, ppart, state, value);
   dy = edje_edit_state_rel1_relative_y_get(ed, ppart, state, value);
   x = edje_edit_state_rel1_offset_x_get(ed, ppart, state, value);
   y = edje_edit_state_rel1_offset_y_get(ed, ppart, state, value);
   str = edje_edit_state_rel1_to_x_get(ed, ppart, state, value);
   str2 = edje_edit_state_rel1_to_y_get(ed, ppart, state, value);
   if (FDIFF(dx, 0.0) || FDIFF(dy, 0.0) || (x) || (y) || (str) || (str2))
     {
        if (machine) puts("REL1-BEGIN");
        else puts(INDENT4 "rel1 {");

        if (machine) printf("RELATIVE-X: %g\nRELATIVE-Y: %g\n", dx, dy);
        else if (FDIFF(dx, 0.0) || FDIFF(dy, 0.0))
          printf(INDENT5 "relative: %g %g;\n", dx, dy);

        if (machine) printf("OFFSET-X: %d\nOFFSET-Y: %d\n", x, y);
        else if ((x) || (y))
          printf(INDENT5 "offset: %d %d;\n", x, y);

        if (machine)
          printf("TO-X: %s\nTO-Y: %s\n", str ? str : "", str2 ? str2 : "");
        else if (((str) && (str2)) && (!strcmp(str, str2)))
          printf(INDENT5 "to: \"%s\";\n", str);
        else
          {
             if (str) printf(INDENT5 "to_x: \"%s\";\n", str);
             if (str2) printf(INDENT5 "to_y: \"%s\";\n", str2);
          }

        if (machine) puts("REL1-END");
        else puts(INDENT4 "}");
     }
   edje_edit_string_free(str);
   edje_edit_string_free(str2);

   dx = edje_edit_state_rel2_relative_x_get(ed, ppart, state, value);
   dy = edje_edit_state_rel2_relative_y_get(ed, ppart, state, value);
   x = edje_edit_state_rel2_offset_x_get(ed, ppart, state, value);
   y = edje_edit_state_rel2_offset_y_get(ed, ppart, state, value);
   str = edje_edit_state_rel2_to_x_get(ed, ppart, state, value);
   str2 = edje_edit_state_rel2_to_y_get(ed, ppart, state, value);
   if (FDIFF(dx, 1.0) || FDIFF(dy, 1.0) || (x != -1) || (y != -1) ||
       (str) || (str2))
     {
        if (machine) puts("REL2-BEGIN");
        else puts(INDENT4 "rel2 {");

        if (machine) printf("RELATIVE-X: %g\nRELATIVE-Y: %g\n", dx, dy);
        else if (FDIFF(dx, 1.0) || FDIFF(dy, 1.0))
          printf(INDENT5 "relative: %g %g;\n", dx, dy);

        if (machine) printf("OFFSET-X: %d\nOFFSET-Y: %d\n", x, y);
        else if ((x != -1) || (y != -1))
          printf(INDENT5 "offset: %d %d;\n", x, y);

        if (machine)
          printf("TO-X: %s\nTO-Y: %s\n", str ? str : "", str2 ? str2 : "");
        else if (((str) && (str2)) && (!strcmp(str, str2)))
          printf(INDENT5 "to: \"%s\";\n", str);
        else
          {
             if (str) printf(INDENT5 "to_x: \"%s\";\n", str);
             if (str2) printf(INDENT5 "to_y: \"%s\";\n", str2);
          }

        if (machine) puts("REL2-END");
        else puts(INDENT4 "}");
     }
   edje_edit_string_free(str);
   edje_edit_string_free(str2);

   if (t == EDJE_PART_TYPE_IMAGE)
     {
        str = edje_edit_state_image_get(ed, ppart, state, value);

        if (machine) printf("IMAGE-BEGIN\nNORMAL: %s\n", str ? str : "");
        else if (detail > 1)
          {
             puts(INDENT4 "image {");
             if (str) printf(INDENT5 "normal: \"%s\";\n", str);
          }
        else if (str)
          printf(INDENT4 "image.normal: \"%s\";\n", str);

        edje_edit_string_free(str);

        if (detail > 1)
          {
             Eina_List *tweens, *l;
             int bl, br, bt, bb, x2, y2;
             double dx2, dy2;
             Eina_Bool has_orgin, has_size;

             tweens = edje_edit_state_tweens_list_get(ed, ppart, state, value);
             EINA_LIST_FOREACH(tweens, l, str)
               {
                  if (machine) printf("TWEEN: %s\n", str);
                  else printf(INDENT5 "tween: \"%s\";\n", str);
               }
             edje_edit_string_list_free(tweens);

             edje_edit_state_image_border_get
               (ed, ppart, state, value, &bl, &br, &bt, &bb);
             if (machine)
               printf("BORDER-LEFT: %d\nBORDER-RIGHT: %d\n"
                      "BORDER-TOP: %d\nBORDER-BOTTOM: %d\n", bl, br, bt, bb);
             else if ((bl) || (br) || (bt) || (bb))
               printf(INDENT5 "border: %d %d %d %d;\n", bl, br, bt, bb);

             x = edje_edit_state_image_border_fill_get(ed, ppart, state, value);
             str = border_fill_name_get(x);
             if (machine) printf("BORDER-FILL: %s\n", str);
             else if (x != 1)
               printf(INDENT5 "middle: %s;\n", str);
             /* do not free str! */

             // TODO support image.fill.smooth

             dx = edje_edit_state_fill_origin_relative_x_get
                 (ed, ppart, state, value);
             dy = edje_edit_state_fill_origin_relative_y_get
                 (ed, ppart, state, value);
             x = edje_edit_state_fill_origin_offset_x_get
                 (ed, ppart, state, value);
             y = edje_edit_state_fill_origin_offset_y_get
                 (ed, ppart, state, value);

             dx2 = edje_edit_state_fill_size_relative_x_get
                 (ed, ppart, state, value);
             dy2 = edje_edit_state_fill_size_relative_y_get
                 (ed, ppart, state, value);
             x2 = edje_edit_state_fill_size_offset_x_get
                 (ed, ppart, state, value);
             y2 = edje_edit_state_fill_size_offset_y_get
                 (ed, ppart, state, value);

             has_orgin = (FDIFF(dx, 0.0) || FDIFF(dy, 0.0) || (x) || (y));
             has_size = (FDIFF(dx2, 1.0) || FDIFF(dy2, 1.0) || (x2) || (y2));

             if ((has_orgin) || (has_size))
               {
                  if (machine) puts("IMAGE-FILL-BEGIN");
                  else puts(INDENT5 "fill {");

                  if (has_orgin)
                    {
                       if (machine)
                         printf("ORIGIN-RELATIVE-X: %g\n"
                                "ORIGIN-RELATIVE-Y: %g\n"
                                "ORIGIN-OFFSET-X: %d\n"
                                "ORIGIN-OFFSET-Y: %d\n",
                                dx, dy, x, y);
                       else
                         printf(INDENT6 "origin {\n"
                                INDENT7 "relative: %g %g;\n"
                                INDENT7 "offset: %d %d;\n"
                                INDENT6 "}\n",
                                dx, dy, x, y);
                    }

                  if (has_size)
                    {
                       if (machine)
                         printf("SIZE-RELATIVE-X: %g\n"
                                "SIZE-RELATIVE-Y: %g\n"
                                "SIZE-OFFSET-X: %d\n"
                                "SIZE-OFFSET-Y: %d\n",
                                dx2, dy2, x2, y2);
                       else
                         printf(INDENT6 "size {\n"
                                INDENT7 "relative: %g %g;\n"
                                INDENT7 "offset: %d %d;\n"
                                INDENT6 "}\n",
                                dx2, dy2, x2, y2);
                    }

                  if (machine) puts("IMAGE-FILL-END");
                  else puts(INDENT5 "}");
               }
          }

        if (machine) puts("IMAGE-END");
        else if (detail > 1)
          puts(INDENT4 "}");
     }
   else if (t == EDJE_PART_TYPE_PROXY)
     {
        int x2, y2;
        double dx2, dy2;
        Eina_Bool has_orgin, has_size;

        if (machine) puts("PROXY-BEGIN");
        else puts(INDENT4 "proxy {");
        // TODO Support source
        // TODO support proxy.fill.smooth

        dx = edje_edit_state_fill_origin_relative_x_get
            (ed, ppart, state, value);
        dy = edje_edit_state_fill_origin_relative_y_get
            (ed, ppart, state, value);
        x = edje_edit_state_fill_origin_offset_x_get
            (ed, ppart, state, value);
        y = edje_edit_state_fill_origin_offset_y_get
            (ed, ppart, state, value);

        dx2 = edje_edit_state_fill_size_relative_x_get
            (ed, ppart, state, value);
        dy2 = edje_edit_state_fill_size_relative_y_get
            (ed, ppart, state, value);
        x2 = edje_edit_state_fill_size_offset_x_get
            (ed, ppart, state, value);
        y2 = edje_edit_state_fill_size_offset_y_get
            (ed, ppart, state, value);

        has_orgin = (FDIFF(dx, 0.0) || FDIFF(dy, 0.0) || (x) || (y));
        has_size = (FDIFF(dx2, 1.0) || FDIFF(dy2, 1.0) || (x2) || (y2));

        if ((has_orgin) || (has_size))
          {
             if (machine) puts("PROXY-FILL-BEGIN");
             else puts(INDENT5 "fill {");

             if (has_orgin)
               {
                  if (machine)
                    printf("ORIGIN-RELATIVE-X: %g\n"
                           "ORIGIN-RELATIVE-Y: %g\n"
                           "ORIGIN-OFFSET-X: %d\n"
                           "ORIGIN-OFFSET-Y: %d\n",
                           dx, dy, x, y);
                  else
                    printf(INDENT6 "origin {\n"
                           INDENT7 "relative: %g %g;\n"
                           INDENT7 "offset: %d %d;\n"
                           INDENT6 "}\n",
                           dx, dy, x, y);
               }

             if (has_size)
               {
                  if (machine)
                    printf("SIZE-RELATIVE-X: %g\n"
                           "SIZE-RELATIVE-Y: %g\n"
                           "SIZE-OFFSET-X: %d\n"
                           "SIZE-OFFSET-Y: %d\n",
                           dx2, dy2, x2, y2);
                  else
                    printf(INDENT6 "size {\n"
                           INDENT7 "relative: %g %g;\n"
                           INDENT7 "offset: %d %d;\n"
                           INDENT6 "}\n",
                           dx2, dy2, x2, y2);
               }

             if (machine) puts("PROXY-FILL-END");
             else puts(INDENT5 "}");
          }

        if (machine) puts("PROXY-END");
        else puts(INDENT4 "}");
     }
   else if ((t == EDJE_PART_TYPE_TEXTBLOCK) || (t == EDJE_PART_TYPE_TEXT))
     {
        if (machine) puts("TEXT-BEGIN");
        else puts(INDENT4 "text {");

        str = edje_edit_state_text_get(ed, ppart, state, value);
        if (machine) printf("TEXT: %s\n", str ? str : "");
        else if (str)
          printf(INDENT5 "text: \"%s\";\n", str);
        edje_edit_string_free(str);

        str = edje_edit_state_font_get(ed, ppart, state, value);
        if (machine) printf("FONT: %s\n", str ? str : "");
        else if (str)
          printf(INDENT5 "font: \"%s\";\n", str);
        edje_edit_string_free(str);

        x = edje_edit_state_text_size_get(ed, ppart, state, value);
        if (machine) printf("SIZE: %d\n", x);
        else if (x > 0)
          printf(INDENT5 "size: %d;\n", x);

        // TODO text_class

        dx = edje_edit_state_text_align_x_get(ed, ppart, state, value);
        dy = edje_edit_state_text_align_y_get(ed, ppart, state, value);
        if (machine) printf("TEXT-ALIGN-X: %g\nTEXT-ALIGN-Y: %g\n", dx, dy);
        else if (FDIFF(dx, 0.5) || FDIFF(dy, 0.5))
          printf(INDENT5 "align: %g %g;\n", dx, dy);

        x = edje_edit_state_text_fit_x_get(ed, ppart, state, value);
        y = edje_edit_state_text_fit_y_get(ed, ppart, state, value);
        if (machine) printf("TEXT-FIT-X: %d\nTEXT-FIT-Y: %d\n", x, y);
        else if ((x) || (y))
          printf(INDENT5 "fit: %d %d;\n", x, y);

        dx = edje_edit_state_text_elipsis_get(ed, ppart, state, value);
        if (machine) printf("TEXT-ELLIPSIS: %g\n", dx);
        else if (FDIFF(dx, 0.5))
          printf(INDENT5 "ellipsis: %g;\n", dx);

        if (machine) puts("TEXT-END");
        else puts(INDENT4 "}");
     }
   else if (t == EDJE_PART_TYPE_EXTERNAL)
     {
        const Eina_List *params, *l;
        const Edje_External_Param *p;

        params = edje_edit_state_external_params_list_get
            (ed, ppart, state, value);

        if (params)
          {
             if (machine) puts("PARAMS-BEGIN");
             else puts(INDENT4 "params {");

             EINA_LIST_FOREACH(params, l, p)
               switch (p->type)
                 {
                  case EDJE_EXTERNAL_PARAM_TYPE_INT:
                    printf(INDENT5 "int: \"%s\" \"%d\";\n", p->name, p->i);
                    break;

                  case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
                    printf(INDENT5 "double: \"%s\" \"%g\";\n", p->name, p->d);
                    break;

                  case EDJE_EXTERNAL_PARAM_TYPE_STRING:
                    if (p->s)
                      printf(INDENT5 "string: \"%s\" \"%s\";\n",
                             p->name, p->s);
                    break;

                  case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
                    printf(INDENT5 "bool: \"%s\" \"%d\";\n", p->name, p->i);
                    break;

                  case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
                    if (p->s)
                      printf(INDENT5 "choice: \"%s\" \"%s\";\n",
                             p->name, p->s);
                    break;

                  default:
                    break;
                 }

             if (machine) puts("PARAMS-END");
             else puts(INDENT4 "}");
          }
     }
}

static void
state_end(void)
{
   if (machine) puts("PART-STATE-END");
   else if (detail > 0)
     puts(INDENT3 "}");
   else puts(" }");
}

static void
part_begin(Evas_Object *ed, const char *name)
{
   const char *type = part_type_name_get(edje_edit_part_type_get(ed, name));
   if (machine) printf("PART-BEGIN\nNAME: %s\nTYPE: %s\n", name, type);
   else
     {
        printf(INDENT2 "part { name: '%s'; type: %s;", name, type);
        if (detail > 0) putchar('\n');
     }
}

static const char *
text_effect_name_get(Edje_Text_Effect effect)
{
   switch (effect)
     {
      case EDJE_TEXT_EFFECT_NONE:
        return "NONE";

      case EDJE_TEXT_EFFECT_PLAIN:
        return "PLAIN";

      case EDJE_TEXT_EFFECT_OUTLINE:
        return "OUTLINE";

      case EDJE_TEXT_EFFECT_SOFT_OUTLINE:
        return "SOFT_OUTLINE";

      case EDJE_TEXT_EFFECT_SHADOW:
        return "SHADOW";

      case EDJE_TEXT_EFFECT_SOFT_SHADOW:
        return "SOFT_SHADOW";

      case EDJE_TEXT_EFFECT_OUTLINE_SHADOW:
        return "OUTLINE_SHADOW";

      case EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW:
        return "OUTLINE_SOFT_SHADOW";

      case EDJE_TEXT_EFFECT_FAR_SHADOW:
        return "FAR_SHADOW";

      case EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW:
        return "FAR_SOFT_SHADOW";

      case EDJE_TEXT_EFFECT_GLOW:
        return "GLOW";

      case EDJE_TEXT_EFFECT_LAST:
        ERR("Invalid part type %d", effect);
        return "???";

      default:
        ERR("Unknown effect type %d", effect);
        return "???";
     }
}

static inline Eina_Bool
_c_id_allowed(char c)
{
   if ((c >= '0') && (c <= '9')) return EINA_TRUE;
   if ((c >= 'a') && (c <= 'z')) return EINA_TRUE;
   if ((c >= 'A') && (c <= 'Z')) return EINA_TRUE;
   return EINA_FALSE;
}

static char *
_api_name_fix(const char *orig)
{
   char *d, *d_end, buf[256];
   const char *s;

   if (!orig) return NULL;
   if (!api_fix) return strdup(orig);

   s = orig;
   d = buf;
   d_end = d + sizeof(buf) - 1;
   for (; (*s != '\0') && (d < d_end); s++, d++)
     if (_c_id_allowed(*s)) *d = *s;
     else *d = '_';
   *d = '\0';

   return strdup(buf);
}

static char *
_part_api_name_get(Evas_Object *ed, const char *ppart)
{
   const char *orig = edje_edit_part_api_name_get(ed, ppart);
   char *fix = _api_name_fix(orig);
   edje_edit_string_free(orig);
   return fix;
}

static void
part_details(Evas_Object *ed, const char *ppart)
{
   Eina_List *states, *l;
   Eina_Bool b;
   const char *str, *str2;
   char *api;

   if (detail < 1) return;

   if (machine) puts("PART-DETAILS-BEGIN");

   str = api = _part_api_name_get(ed, ppart);
   str2 = edje_edit_part_api_description_get(ed, ppart);
   if (machine)
     {
        printf("API-NAME: %s\n", str ? str : "");
        printf("API-DESCRIPTION: %s\n", str2 ? str2 : "");
     }
   else if ((str) || (str2))
     printf(INDENT3 "api: \"%s\" \"%s\";\n", str ? str : "", str2 ? str2 : "");
   free(api);
   edje_edit_string_free(str2);

   b = edje_edit_part_mouse_events_get(ed, ppart);
   if (machine) printf("MOUSE_EVENTS: %d\n", b);
   else if (!b)
     puts(INDENT3 "mouse_events: 0;");

   if (detail > 1)
     {
        b = edje_edit_part_repeat_events_get(ed, ppart);
        if (machine) printf("REPEAT_EVENTS: %d\n", b);
        else if (b)
          puts(INDENT3 "repeat_events: 1;");

        b = edje_edit_part_scale_get(ed, ppart);
        if (machine) printf("SCALE: %d\n", b);
        else if (b)
          puts(INDENT3 "scale: 1;");
     }

   str = edje_edit_part_clip_to_get(ed, ppart);
   if (machine) printf("CLIP_TO: %s\n", str ? str : "");
   else if (str)
     printf(INDENT3 "clip_to: \"%s\";\n", str);
   edje_edit_string_free(str);

   str = edje_edit_part_source_get(ed, ppart);
   if (machine) printf("SOURCE: %s\n", str ? str : "");
   else if (str)
     printf(INDENT3 "source: \"%s\";\n", str);
   edje_edit_string_free(str);

   if (detail > 1)
     {
        if (edje_edit_part_type_get(ed, ppart) == EDJE_PART_TYPE_TEXT)
          {
             str = text_effect_name_get(edje_edit_part_effect_get(ed, ppart));
             if (machine) printf("EFFECT: %s\n", str ? str : "");
             else if (str)
               printf(INDENT3 "effect: %s;\n", str);
             /* do not free this str! */
          }

        if (edje_edit_part_drag_x_get(ed, ppart) ||
            edje_edit_part_drag_y_get(ed, ppart))
          {
             int dir, step, count;

             if (machine) puts("DRAGABLE-BEGIN");
             else puts(INDENT3 "dragable {");

             dir = edje_edit_part_drag_x_get(ed, ppart);
             step = edje_edit_part_drag_step_x_get(ed, ppart);
             count = edje_edit_part_drag_count_x_get(ed, ppart);
             if (machine) printf("DRAG-X: %d %d %d\n", dir, step, count);
             else printf(INDENT4 "x: %d %d %d;\n", dir, step, count);

             dir = edje_edit_part_drag_y_get(ed, ppart);
             step = edje_edit_part_drag_step_y_get(ed, ppart);
             count = edje_edit_part_drag_count_y_get(ed, ppart);
             if (machine) printf("DRAG-Y: %d %d %d\n", dir, step, count);
             else printf(INDENT4 "y: %d %d %d;\n", dir, step, count);

             str = edje_edit_part_drag_confine_get(ed, ppart);
             if (machine) printf("DRAG-CONFINE: %s\n", str ? str : "");
             else if (str)
               printf(INDENT4 "confine: \"%s\";\n", str);
             edje_edit_string_free(str);

             str = edje_edit_part_drag_event_get(ed, ppart);
             if (machine) printf("DRAG-EVENTS: %s\n", str ? str : "");
             else if (str)
               printf(INDENT4 "events: \"%s\";\n", str);
             edje_edit_string_free(str);

             if (machine) puts("DRAGABLE-END");
             else puts(INDENT3 "}");
          }
     }

   states = edje_edit_part_states_list_get(ed, ppart);
   EINA_LIST_FOREACH(states, l, str)
     {
        char state[512], *delim;
        double value;
        eina_strlcpy(state, str, sizeof(state)); /* bad states_list! :-( */
        delim = strchr(state, ' ');
        if (!delim) continue;
        *delim = '\0';
        delim++;
        value = strtod(delim, NULL);
        state_begin(state, value);
        state_details(ed, ppart, state, value);
        state_end();
     }
   edje_edit_string_list_free(states);

   if (machine) puts("PART-DETAILS-END");
}

static void
part_end(void)
{
   if (machine) puts("PART-END");
   else if (detail > 0)
     puts(INDENT2 "}");
   else puts(" }");
}

static int
_groups_names_list(void)
{
   Eina_List *l;
   const char *name;
   Eina_Bool found = EINA_FALSE;

   EINA_LIST_FOREACH(groups, l, name)
     {
        if (!matches(name, group))
          {
             DBG("filter out group '%s': does not match '%s'", name, group);
             continue;
          }
        found = EINA_TRUE;
        puts(name);
     }

   if (!found) WRN("no groups match '%s'", group);
   return !found;
}

static int
_parts_names_list(void)
{
   Eina_List *gl, *pl, *parts;
   const char *gname, *pname;
   Eina_Bool found_group = EINA_FALSE, found_part = EINA_FALSE;

   EINA_LIST_FOREACH(groups, gl, gname)
     {
        Evas_Object *ed;

        if (!matches(gname, group))
          {
             DBG("filter out group '%s': does not match '%s'", gname, group);
             continue;
          }

        ed = edje_edit_object_add(ecore_evas_get(ee));
        if (!edje_object_file_set(ed, file, gname))
          {
             Edje_Load_Error err = edje_object_load_error_get(ed);
             const char *errmsg = edje_load_error_str(err);
             ERR("could not load group '%s' from file '%s': %s",
                 gname, file, errmsg);
             evas_object_del(ed);
             continue;
          }

        found_group = EINA_TRUE;
        group_begin(gname);

        parts = edje_edit_parts_list_get(ed);
        EINA_LIST_FOREACH(parts, pl, pname)
          {
             if (!matches(pname, part))
               {
                  DBG("filter out part '%s': does not match '%s'", pname, part);
                  continue;
               }
             if (api_only)
               {
                  if (!edje_edit_part_api_name_get(ed, pname))
                    {
                       DBG("filter out part '%s': not API.", pname);
                       continue;
                    }
               }
             if (machine) printf("PART: %s\n", pname);
             else printf(INDENT "part: %s\n", pname);
          }
        edje_edit_string_list_free(parts);

        group_end();
        evas_object_del(ed);
     }

   if (!found_group) WRN("no groups match '%s'", group);
   if (!found_part) WRN("no parts match '%s'", part);
   return (!found_group) || (!found_part);
}

static Eina_Bool
_group_parts_list(Evas_Object *ed)
{
   Eina_Bool found = EINA_FALSE;
   Eina_List *parts, *l;
   const char *name;

   parts_begin();

   parts = edje_edit_parts_list_get(ed);
   EINA_LIST_FOREACH(parts, l, name)
     {
        if (!matches(name, part))
          {
             DBG("filter out part '%s': does not match '%s'", name, part);
             continue;
          }
        if (api_only)
          {
             if (!edje_edit_part_api_name_get(ed, name))
               {
                  DBG("filter out part '%s': not API.", name);
                  continue;
               }
          }

        found = EINA_TRUE;
        part_begin(ed, name);
        part_details(ed, name);
        part_end();
     }

   parts_end();
   return found;
}

static void
programs_begin(void)
{
   if (machine) puts("PROGRAMS-BEGIN");
   else puts(INDENT "programs {");
}

static void
programs_end(void)
{
   if (machine) puts("PROGRAMS-END");
   else puts(INDENT "}");
}

static void
program_begin(const char *name)
{
   if (machine) printf("PROGRAM-BEGIN\nNAME: %s\n", name ? name : "");
   else
     {
        printf(INDENT2 "program { name: '%s';\n", name ? name : "");
     }
}

static void
program_end(void)
{
   if (machine) puts("PROGRAM-END");
   else puts(INDENT2 "}");
}

static char *
_program_api_name_get(Evas_Object *ed, const char *pprogram)
{
   const char *orig = edje_edit_program_api_name_get(ed, pprogram);
   char *fix = _api_name_fix(orig);
   edje_edit_string_free(orig);
   return fix;
}

static const char *
_transition_name_get(Edje_Tween_Mode mode)
{
   switch (mode)
     {
      case EDJE_TWEEN_MODE_LINEAR: return "LINEAR";

      case EDJE_TWEEN_MODE_ACCELERATE: return "ACCELERATE";

      case EDJE_TWEEN_MODE_DECELERATE: return "DECELERATE";

      case EDJE_TWEEN_MODE_SINUSOIDAL: return "SINUSOIDAL";

      case EDJE_TWEEN_MODE_CUBIC_BEZIER: return "CUBIC_BEZIER";

      default:
        ERR("Unknown transition mode %d", mode);
        return "???";
     }
}

static void
program_details(Evas_Object *ed, const char *pprogram)
{
   const char *str, *str2;
   char *api;

   if (detail < 1) return;

   if (machine) puts("PROGRAM-DETAILS-BEGIN");

   str = api = _program_api_name_get(ed, pprogram);
   str2 = edje_edit_program_api_description_get(ed, pprogram);
   if (machine)
     {
        printf("API-NAME: %s\n", str ? str : "");
        printf("API-DESCRIPTION: %s\n", str2 ? str2 : "");
     }
   else if ((str) || (str2))
     printf(INDENT3 "api: \"%s\" \"%s\";\n", str ? str : "", str2 ? str2 : "");
   free(api);
   edje_edit_string_free(str2);

   str = edje_edit_program_signal_get(ed, pprogram);
   if (machine) printf("SIGNAL: %s\n", str ? str : "");
   else if (str)
     printf(INDENT3 "signal: \"%s\";\n", str);
   edje_edit_string_free(str);

   str = edje_edit_program_source_get(ed, pprogram);
   if (machine) printf("SOURCE: %s\n", str ? str : "");
   else if (str)
     printf(INDENT3 "source: \"%s\";\n", str);
   edje_edit_string_free(str);

   if (detail >= 1)
     {
        Eina_List *lst, *l;
        Edje_Action_Type type = edje_edit_program_action_get(ed, pprogram);
        switch (type)
          {
           case EDJE_ACTION_TYPE_ACTION_STOP:
             if (machine) puts("ACTION: ACTION_STOP");
             else puts(INDENT3 "action: ACTION_STOP;");
             break;

           case EDJE_ACTION_TYPE_STATE_SET:
             str = edje_edit_program_state_get(ed, pprogram);
             if (machine)
               printf("ACTION: STATE_SET\nACTION-STATE: %s %g\n",
                      str, edje_edit_program_value_get(ed, pprogram));
             else
               printf(INDENT3 "action: STATE_SET \"%s\" %2.1f;\n",
                      str, edje_edit_program_value_get(ed, pprogram));
             edje_edit_string_free(str);
             break;

           case EDJE_ACTION_TYPE_SIGNAL_EMIT:
             str = edje_edit_program_state_get(ed, pprogram);
             str2 = edje_edit_program_state2_get(ed, pprogram);
             if (machine)
               printf("ACTION: SIGNAL_EMIT\nACTION-SIGNAL: %s\n"
                      "ACTION-SOURCE: %s\n",
                      str ? str : "", str2 ? str2 : "");
             else if ((str) || (str2))
               printf(INDENT3 "action: SIGNAL_EMIT \"%s\" \"%s\";\n",
                      str ? str : "", str2 ? str2 : "");
             edje_edit_string_free(str);
             edje_edit_string_free(str2);
             break;

           //TODO Support Drag
           //~ case EDJE_ACTION_TYPE_DRAG_VAL_SET:
           //~ eina_strbuf_append(buf, I4"action: DRAG_VAL_SET TODO;\n");
           //~ break;
           //~ case EDJE_ACTION_TYPE_DRAG_VAL_STEP:
           //~ eina_strbuf_append(buf, I4"action: DRAG_VAL_STEP TODO;\n");
           //~ break;
           //~ case EDJE_ACTION_TYPE_DRAG_VAL_PAGE:
           //~ eina_strbuf_append(buf, I4"action: DRAG_VAL_PAGE TODO;\n");
           //~ break;
           default:
             ERR("Unhandled pprogram action type %d", type);
             break;
          }

        if (detail > 1)
          {
             double from, range;

             from = edje_edit_program_transition_time_get(ed, pprogram);
             if (from > 0.0)
               {
                  str = _transition_name_get
                      (edje_edit_program_transition_get(ed, pprogram));
                  if (machine)
                    printf("TRANSITION-NAME: %s\nTRANSITION-DURATION: %g\n",
                           str, from);
                  else printf(INDENT3 "transition: %s %g;\n", str, from);
                  /* do not free str! */
               }

             from = edje_edit_program_in_from_get(ed, pprogram);
             range = edje_edit_program_in_range_get(ed, pprogram);
             if (FDIFF(from, 0.0) || FDIFF(range, 0.0))
               {
                  if (machine)
                    printf("IN-FROM: %g\nIN-RANGE: %g\n", from, range);
                  else printf(INDENT3 "in: %g %g;\n", from, range);
               }
          }

        lst = edje_edit_program_targets_get(ed, pprogram);
        EINA_LIST_FOREACH(lst, l, str)
          if (machine) printf("TARGET: %s\n", str);
          else printf(INDENT3 "target: \"%s\";\n", str);
        edje_edit_string_list_free(lst);

        lst = edje_edit_program_afters_get(ed, pprogram);
        EINA_LIST_FOREACH(lst, l, str)
          if (machine) printf("AFTER: %s\n", str);
          else printf(INDENT3 "after: \"%s\";\n", str);
        edje_edit_string_list_free(lst);

        // TODO Support script {}
     }

   if (machine) puts("PROGRAM-DETAILS-END");
}

static Eina_Bool
_group_programs_list(Evas_Object *ed)
{
   Eina_Bool found = EINA_FALSE;
   Eina_List *programs, *l;
   const char *name;

   programs_begin();

   /* TODO: change programs to operate on their ID instead of names!
    * needs huge change in Edje_Edit.h
    */
   WRN("listing only programs with names!");
   programs = edje_edit_programs_list_get(ed);
   EINA_LIST_FOREACH(programs, l, name)
     {
        if (!matches(name, program))
          {
             DBG("filter out program '%s': does not match '%s'", name, program);
             continue;
          }
        if (api_only)
          {
             if (!edje_edit_program_api_name_get(ed, name))
               {
                  DBG("filter out program '%s': not API.", name);
                  continue;
               }
          }

        found = EINA_TRUE;
        program_begin(name);
        program_details(ed, name);
        program_end();
     }

   programs_end();
   return found;
}

static int
_list(const char *mode)
{
   Eina_List *l;
   const char *name;
   int ret = 0;
   Eina_Bool found_group = EINA_FALSE;
   Eina_Bool req_part, found_part, req_prog, found_prog;

   if ((!strcmp(mode, "parts")) || (!strcmp(mode, "groups")))
     {
        req_part = EINA_TRUE;
        found_part = EINA_FALSE;
     }
   else
     {
        req_part = EINA_FALSE;
        found_part = EINA_TRUE;
     }

   if ((!strcmp(mode, "programs")) || (!strcmp(mode, "groups")))
     {
        req_prog = EINA_TRUE;
        found_prog = EINA_FALSE;
     }
   else
     {
        req_prog = EINA_FALSE;
        found_prog = EINA_TRUE;
     }

   EINA_LIST_FOREACH(groups, l, name)
     {
        Evas_Object *ed;

        if (!matches(name, group))
          {
             DBG("filter out group '%s': does not match '%s'", name, group);
             continue;
          }

        ed = edje_edit_object_add(ecore_evas_get(ee));
        if (!edje_object_file_set(ed, file, name))
          {
             Edje_Load_Error err = edje_object_load_error_get(ed);
             const char *errmsg = edje_load_error_str(err);
             ERR("could not load group '%s' from file '%s': %s",
                 name, file, errmsg);
             evas_object_del(ed);
             continue;
          }

        found_group = EINA_TRUE;
        group_begin(name);
        group_details(ed);

        if (req_part) found_part |= _group_parts_list(ed);
        if (req_prog) found_prog |= _group_programs_list(ed);

        group_end();
        evas_object_del(ed);
     }

   /* no hard requirement for parts or programs for group listing */
   if (!strcmp(mode, "groups")) req_part = req_prog = EINA_FALSE;

   if (!found_group)
     {
        WRN("no groups match '%s'", group);
        ret = 1;
     }
   if ((req_part) && (!found_part))
     {
        WRN("no parts match '%s'", part);
        ret = 1;
     }
   if ((req_prog) && (!found_prog))
     {
        WRN("no programs match '%s'", program);
        ret = 1;
     }
   return ret;
}

static Evas_Object *
_edje_object_any_get(void)
{
   Evas_Object *ed = edje_edit_object_add(ecore_evas_get(ee));
   Eina_List *l;
   const char *name;
   if (!ed) return NULL;
   EINA_LIST_FOREACH(groups, l, name)
     if (edje_object_file_set(ed, file, name)) return ed;
   evas_object_del(ed);
   return NULL;
}

static Eina_Bool
_gdata_list(void)
{
   Evas_Object *ed = _edje_object_any_get();
   Eina_List *l, *data;
   const char *key;

   if (!ed) return EINA_FALSE;

   data = edje_edit_data_list_get(ed);

   if (machine) puts("DATA-BEGIN");
   else puts("data {");

   EINA_LIST_FOREACH(data, l, key)
     {
        const char *value = edje_edit_data_value_get(ed, key);
        if (machine) printf("ITEM: \"%s\" \"%s\"\n", key, value);
        else printf(INDENT "item: \"%s\" \"%s\";\n", key, value);
     }

   if (machine) puts("DATA-END");
   else puts("}");

   edje_edit_string_list_free(data);
   evas_object_del(ed);
   return EINA_TRUE;
}

static const char *
_comp_str_get(Evas_Object *ed, const char *img)
{
   static char buf[128];
   Edje_Edit_Image_Comp type = edje_edit_image_compression_type_get(ed, img);
   int rate;

   switch (type)
     {
      case EDJE_EDIT_IMAGE_COMP_RAW:
        return "RAW";

      case EDJE_EDIT_IMAGE_COMP_USER:
        return "USER";

      case EDJE_EDIT_IMAGE_COMP_COMP:
        return "COMP";

      case EDJE_EDIT_IMAGE_COMP_LOSSY:
        rate = edje_edit_image_compression_rate_get(ed, img);
        snprintf(buf, sizeof(buf), "LOSSY %d", rate);
        return buf;

      case EDJE_EDIT_IMAGE_COMP_LOSSY_ETC1:
        rate = edje_edit_image_compression_rate_get(ed, img);
        snprintf(buf, sizeof(buf), "LOSSY_ETC1 %d", rate);
        return buf;

      case EDJE_EDIT_IMAGE_COMP_LOSSY_ETC2:
        rate = edje_edit_image_compression_rate_get(ed, img);
        snprintf(buf, sizeof(buf), "LOSSY_ETC2 %d", rate);
        return buf;

      default:
        ERR("Unknown compression type %d", type);
        return "???";
     }
}

static Eina_Bool
_images_list(void)
{
   Evas_Object *ed = _edje_object_any_get();
   Eina_List *l, *images;
   const char *img;

   if (!ed) return EINA_FALSE;

   images = edje_edit_images_list_get(ed);

   if (machine) puts("IMAGES-BEGIN");
   else puts("images {");

   EINA_LIST_FOREACH(images, l, img)
     {
        int id = edje_edit_image_id_get(ed, img);
        const char *comp = _comp_str_get(ed, img);

        if (detail < 1)
          {
             if (machine) printf("IMAGE: %s\n", img);
             else printf(INDENT "image: \"%s\" %s;\n", img, comp);
          }
        else if (detail == 1)
          {
             if (machine) printf("IMAGE: \"%s\" \"%s\"\n", img, comp);
             else printf(INDENT "image: \"%s\" %s;\n", img, comp);
          }
        else
          {
             if (machine)
               printf("IMAGE: \"edje/images/%d\" \"%s\" \"%s\"\n",
                      id, img, comp);
             else
               printf(INDENT "image: \"%s\" %s; /* id: \"edje/images/%d\" */\n",
                      img, comp, id);
          }
     }

   if (machine) puts("IMAGES-END");
   else puts("}");

   edje_edit_string_list_free(images);
   evas_object_del(ed);
   return EINA_TRUE;
}

static Eina_Bool
_fonts_list(void)
{
   Evas_Object *ed = _edje_object_any_get();
   Eina_List *l, *fonts;
   const char *alias;

   if (!ed) return EINA_FALSE;

   fonts = edje_edit_fonts_list_get(ed);

   if (machine) puts("FONTS-BEGIN");
   else puts("fonts {");

   EINA_LIST_FOREACH(fonts, l, alias)
     {
        const char *path = edje_edit_font_path_get(ed, alias);

        if (detail < 1)
          {
             if (machine) printf("FONT: %s\n", alias);
             else printf(INDENT "font: \"%s\" \"%s\";\n", path, alias);
          }
        else if (detail == 1)
          {
             if (machine) printf("FONT: \"%s\" \"%s\"\n", path, alias);
             else printf(INDENT "font: \"%s\" \"%s\";\n", path, alias);
          }
        else
          {
             if (machine)
               printf("FONT: \"edje/fonts/%s\" \"%s\" \"%s\"\n",
                      alias, path, alias);
             else
               printf(INDENT
                      "font: \"%s\" \"%s\"; /* id: \"edje/fonts/%s\" */\n",
                      path, alias, alias);
          }

        edje_edit_string_free(path);
     }

   if (machine) puts("FONTS-END");
   else puts("}");

   edje_edit_string_list_free(fonts);
   evas_object_del(ed);
   return EINA_TRUE;
}

static Eina_Bool
_externals_list(void)
{
   Evas_Object *ed = _edje_object_any_get();
   Eina_List *l, *externals;
   const char *key;

   if (!ed) return EINA_FALSE;

   externals = edje_edit_externals_list_get(ed);

   if (machine) puts("EXTERNALS-BEGIN");
   else puts("externals {");

   EINA_LIST_FOREACH(externals, l, key)
     {
        if (machine) printf("EXTERNAL: %s\n", key);
        else printf(INDENT "external: \"%s\";\n", key);
     }

   if (machine) puts("EXTERNALS-END");
   else puts("}");

   edje_edit_string_list_free(externals);
   evas_object_del(ed);
   return EINA_TRUE;
}

int
main(int argc, char **argv)
{
   Eina_Bool quit_option = EINA_FALSE;
   char *mode = NULL;
   char *detail_name = NULL;
   int arg_index;
   int ret = 0;
   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_STR(mode),
      ECORE_GETOPT_VALUE_STR(detail_name),
      ECORE_GETOPT_VALUE_STR(group),
      ECORE_GETOPT_VALUE_STR(part),
      ECORE_GETOPT_VALUE_STR(program),
      ECORE_GETOPT_VALUE_BOOL(api_only),
      ECORE_GETOPT_VALUE_BOOL(api_fix),
      ECORE_GETOPT_VALUE_BOOL(machine),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_NONE
   };

   setlocale(LC_NUMERIC, "C");

   ecore_app_no_system_modules();

   ecore_init();
   ecore_evas_init();
   eina_init();
   edje_init();

   _log_dom = eina_log_domain_register("edje_inspector", EINA_COLOR_YELLOW);
   if (_log_dom < 0)
     {
        EINA_LOG_CRIT("could not register log domain 'edje_inspector'");
        ret = 1;
        goto error_log;
     }

   arg_index = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (arg_index < 0)
     {
        ERR("could not parse arguments.");
        ret = 1;
        goto error_getopt;
     }
   else if (quit_option)
     goto error_getopt;
   else if (arg_index != argc - 1)
     {
        ERR("incorrect number of parameters. Requires one single file.");
        ret = 1;
        goto error_getopt;
     }

   if (!mode) mode = (char *)mode_choices[0];

   if (detail_name)
     {
        if (!strcmp(detail_name, "none")) detail = 0;
        else if (!strcmp(detail_name, "terse"))
          detail = 1;
        else if (!strcmp(detail_name, "all"))
          detail = 2;
        else ERR("Unknown detail level: '%s'", detail_name);
     }

   file = argv[arg_index];

   // check if the file is accessible
   if (access(file, R_OK) == -1)
     {
        int e = errno;
        ERR("File '%s' not accessible, error %d (%s).\n",
            file, e, strerror(e));
        ret = 1;
        goto error_getopt;
     }

   DBG("mode=%s, detail=%d(%s), group=%s, part=%s, program=%s, api-only=%hhu"
       ", api-fix=%hhu, machine=%hhu, file=%s",
       mode, detail, detail_name,
       group ? group : "",
       part ? part : "",
       program ? program : "",
       api_only, api_fix, machine, file);

   groups = edje_file_collection_list(file);
   if (!groups)
     {
        ERR("no groups in edje file '%s'", file);
        ret = 1;
        goto error_getopt;
     }

   if (!strcmp(mode, "groups-names")) ret = _groups_names_list();
   else
     {
        ee = ecore_evas_buffer_new(1, 1);
        if (!ee)
          {
             ERR("could not create ecore_evas_buffer");
             ret = 1;
          }
        else
          {
             if (!strcmp(mode, "parts-names")) ret = _parts_names_list();
             else if (!strcmp(mode, "global-data"))
               ret = _gdata_list();
             else if (!strcmp(mode, "images"))
               ret = _images_list();
             else if (!strcmp(mode, "fonts"))
               ret = _fonts_list();
             else if (!strcmp(mode, "externals"))
               ret = _externals_list();
             else ret = _list(mode);
             ecore_evas_free(ee);
          }
     }

   edje_file_collection_list_free(groups);
error_getopt:
   eina_log_domain_unregister(_log_dom);
error_log:
   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return ret;
}

