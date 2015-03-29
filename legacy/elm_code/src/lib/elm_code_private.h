#ifndef ELM_CODE_PRIVATE_H
# define ELM_CODE_PRIVATE_H

extern int _elm_code_lib_log_dom;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_elm_code_lib_log_dom, __VA_ARGS__)
#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_elm_code_lib_log_dom, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_elm_code_lib_log_dom, __VA_ARGS__)
#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_elm_code_lib_log_dom, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_elm_code_lib_log_dom, __VA_ARGS__)

#endif

/**
 * Structure holding the info about a selected region.
 */
typedef struct
{
   unsigned int start_line, end_line;
   unsigned int start_col, end_col;
} Elm_Code_Widget_Selection_Data;

typedef struct
{
   Elm_Code *code;
   Evas_Object *grid, *scroller;

   Evas_Font_Size font_size;
   double gravity_x, gravity_y;

   unsigned int cursor_line, cursor_col;
   Eina_Bool editable, focussed;
   Eina_Bool show_line_numbers;
   unsigned int line_width_marker, tabstop;
   Eina_Bool show_whitespace;

   Elm_Code_Widget_Selection_Data *selection;
} Elm_Code_Widget_Data;

/* Private parser callbacks */

void _elm_code_parse_setup();

void _elm_code_parse_line(Elm_Code *code, Elm_Code_Line *line);

void _elm_code_parse_file(Elm_Code *code, Elm_Code_File *file);
