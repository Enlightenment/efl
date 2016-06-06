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

/* Private parser callbacks */

void _elm_code_parse_setup();

void _elm_code_parse_line(Elm_Code *code, Elm_Code_Line *line);

void _elm_code_parse_file(Elm_Code *code, Elm_Code_File *file);

void _elm_code_parse_reset_file(Elm_Code *code, Elm_Code_File *file);

void _elm_code_parser_free(Elm_Code_Parser *parser);

#endif
