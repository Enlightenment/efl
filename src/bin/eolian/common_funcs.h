#ifndef __EOLIAN_COMMON_FUNCS_H
#define __EOLIAN_COMMON_FUNCS_H

#include <Eina.h>
#include <Eolian.h>

#define EO

extern int _eolian_gen_log_dom;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eolian_gen_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eolian_gen_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eolian_gen_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eolian_gen_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_eolian_gen_log_dom, __VA_ARGS__)

typedef struct
{
   char full_classname[PATH_MAX];

   char upper_eo_prefix[PATH_MAX];
   char lower_eo_prefix[PATH_MAX];

   char upper_classname[PATH_MAX];
   char lower_classname[PATH_MAX];
}_eolian_class_vars;

typedef struct
{
   char upper_func[PATH_MAX];

   char upper_eo_func[PATH_MAX];
   char lower_eo_func[PATH_MAX];

   char legacy_func[PATH_MAX];
}_eolian_class_func_vars;

void _template_fill(Eina_Strbuf *buf, const char *templ, const Eolian_Class class, const char *classname, const char *funcname, Eina_Bool reset);

char *_nextline(char *str, unsigned int lines);

char *_startline(char *str, char *pos);

char *_source_desc_get(const char *str);

void _class_env_create(const Eolian_Class class, const char *over_classname, _eolian_class_vars *env);

void _class_func_env_create(const Eolian_Class class, const char *funcname, Eolian_Function_Type ftype EINA_UNUSED, _eolian_class_func_vars *env);

char *_func_name_dedup(const char *classn, const char *funcn);

#endif
