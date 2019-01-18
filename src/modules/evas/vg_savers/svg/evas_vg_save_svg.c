#include "vg_common.h"

static int _evas_vg_saver_svg_log_dom = -1;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_vg_saver_svg_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_vg_saver_svg_log_dom, __VA_ARGS__)

static void
printf_style(Svg_Style_Property *style, Eina_Strbuf *buf)
{
   if ((style->fill.paint.r) || (style->fill.paint.g) || (style->fill.paint.b))
     eina_strbuf_append_printf(buf, " fill=\"#%02X%02X%02X\" ", style->fill.paint.r, style->fill.paint.g, style->fill.paint.b);
   if (style->fill.fill_rule == EFL_GFX_FILL_RULE_ODD_EVEN)
     eina_strbuf_append_printf(buf, " fill-rule=\"evenodd\" ");
   if (style->fill.opacity != 255)
     eina_strbuf_append_printf(buf, " fill-opacity=\"%f\"", style->fill.opacity / 255.0);
   if ((style->stroke.paint.r) || (style->stroke.paint.g) || (style->stroke.paint.b))
     eina_strbuf_append_printf(buf, " stroke=\"#%02X%02X%02X\" ", style->stroke.paint.r, style->stroke.paint.g, style->stroke.paint.b);
   if (style->stroke.width)
     eina_strbuf_append_printf(buf, " stroke-width=\"%f\" ", style->stroke.width);
   if (style->stroke.cap == EFL_GFX_CAP_ROUND)
     eina_strbuf_append_printf(buf, " stroke-linecap=\"round\" ");
   else if (style->stroke.cap == EFL_GFX_CAP_SQUARE)
     eina_strbuf_append_printf(buf, " stroke-linecap=\"square\" ");
}

static void
_svg_node_printf(Svg_Node *parent, Eina_Strbuf *buf)
{
   int i = 0;
   double *points;
   double last_x = 0, last_y = 0;
   Efl_Gfx_Path_Command *commands;
   Eina_List *l;
   Svg_Node *data;

   switch (parent->type)
     {
      case SVG_NODE_DOC:
         eina_strbuf_append_printf(buf, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
         eina_strbuf_append_printf(buf, "<svg viewBox=\"%f %f %f %f\" xmlns=\"http://www.w3.org/2000/svg\""
                " xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n",
                parent->node.doc.vx, parent->node.doc.vy,
                parent->node.doc.vw, parent->node.doc.vh);
         EINA_LIST_FOREACH(parent->child, l, data)
           {
              _svg_node_printf(data, buf);
           }
         eina_strbuf_append_printf(buf, "</svg>\n");
         break;
      case SVG_NODE_G:
         eina_strbuf_append_printf(buf, "<g");
         if (parent->transform)
           eina_strbuf_append_printf(buf,
                                     " transform=\"matrix(%f %f %f %f %f %f)\"",
                                     parent->transform->xx,
                                     parent->transform->yx,
                                     parent->transform->xy,
                                     parent->transform->yy,
                                     parent->transform->xz,
                                     parent->transform->yz);
         eina_strbuf_append_printf(buf, ">\n");
         EINA_LIST_FOREACH(parent->child, l, data)
           {
              _svg_node_printf(data, buf);
           }
         eina_strbuf_append_printf(buf, "</g>\n");
         break;
      case SVG_NODE_CUSTOME_COMMAND:
         points = parent->node.command.points;
         commands = parent->node.command.commands;
         eina_strbuf_append_printf(buf, "<path d=\"");
         for (i = 0; i < parent->node.command.commands_count; i++)
           {
              switch (commands[i])
                {
                 case EFL_GFX_PATH_COMMAND_TYPE_END:
                    eina_strbuf_append_printf(buf, "\"");
                    if (parent->transform)
                      eina_strbuf_append_printf(buf, " transform=\"matrix(%f %f %f %f %f %f)\"",
                             parent->transform->xx,
                             parent->transform->yx,
                             parent->transform->xy,
                             parent->transform->yy,
                             parent->transform->xz,
                             parent->transform->yz);
                    printf_style(parent->style, buf);
                    eina_strbuf_append_printf(buf, "/>\n");
                    break;
                 case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO:
                    eina_strbuf_append_printf(buf, "m%f,%f ", points[0] - last_x, points[1] - last_y);
                    last_x = points[0];
                    last_y = points[1];
                    points += 2;
                    break;
                 case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO:
                    eina_strbuf_append_printf(buf, "L%f,%f ", points[0], points[1]);
                    last_x = points[0];
                    last_y = points[1];
                    points += 2;
                    break;
                 case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO:
                    eina_strbuf_append_printf(buf, "c%f,%f %f,%f %f,%f ",
                           points[0] - last_x, points[1] - last_y,
                           points[2] - last_x, points[3] - last_y,
                           points[4] - last_x, points[5] - last_y);
                    last_x = points[4];
                    last_y = points[5];
                    points += 6;
                    break;
                 case EFL_GFX_PATH_COMMAND_TYPE_CLOSE:
                    eina_strbuf_append_printf(buf, " z ");
                    break;
                 case EFL_GFX_PATH_COMMAND_TYPE_LAST:
                 default:
                    break;
                }
           }
         break;
      default:
         break;
     }
}

int
evas_vg_save_file_svg(Vg_File_Data *evg_data, const char *file, const char *key EINA_UNUSED, int compress EINA_UNUSED)
{
   Eina_Strbuf *buf = NULL;
   Svg_Node *root;
   FILE *f = fopen(file, "w+");
   if (!f)
     {
        ERR("Cannot open file '%s' for SVG save", file);
        return EFL_GFX_IMAGE_LOAD_ERROR_GENERIC;
     }

   root = vg_common_svg_create_svg_node(evg_data);
   buf = eina_strbuf_new();
   _svg_node_printf(root, buf);
   fprintf(f, "%s\n", eina_strbuf_string_get(buf));
   fclose(f);
   eina_strbuf_free(buf);

   return EVAS_LOAD_ERROR_NONE;
}

static Evas_Vg_Save_Func evas_vg_save_svg_func =
{
   evas_vg_save_file_svg
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_vg_save_svg_func);
   _evas_vg_saver_svg_log_dom = eina_log_domain_register
     ("vg-save-svg", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_vg_saver_svg_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "svg",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_VG_SAVER, vg_saver, svg);

#ifndef EVAS_STATIC_BUILD_VG_SVG
EVAS_EINA_MODULE_DEFINE(vg_saver, svg);
#endif
