#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <Eina.h>

#include "eo_lexer.h"
#include "eolian_database.h"

static int _eo_tokenizer_log_dom = -1;
#ifdef CRITICAL
#undef CRITICAL
#endif
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_eo_tokenizer_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eo_tokenizer_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eo_tokenizer_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eo_tokenizer_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eo_tokenizer_log_dom, __VA_ARGS__)

static int _init_counter = 0;

int
eo_tokenizer_init()
{
   if (!_init_counter)
     {
        eina_init();
        eina_log_color_disable_set(EINA_FALSE);
        _eo_tokenizer_log_dom = eina_log_domain_register("eo_toknz", EINA_COLOR_CYAN);
     }
   return _init_counter++;
}

int
eo_tokenizer_shutdown()
{
   if (_init_counter <= 0) return 0;
   _init_counter--;
   if (!_init_counter)
     {
        eina_log_domain_unregister(_eo_tokenizer_log_dom);
        _eo_tokenizer_log_dom = -1;
        eina_shutdown();
     }
   return _init_counter;
}

static void
_eo_tokenizer_abort(Eo_Tokenizer *toknz,
                    const char *file, const char* fct, int line,
                    const char *fmt, ...)
{
   va_list ap;
   va_start (ap, fmt);
   eina_log_vprint(_eo_tokenizer_log_dom, EINA_LOG_LEVEL_ERR,
                   file, fct, line, fmt, ap);
   va_end(ap);
   fprintf(stderr, "File:%s\n toknz[%d] n:%d l:%d p:%d pe:%d ts:%s te:%s act:%d\n",
          toknz->source,
          toknz->cs, toknz->current_nesting, toknz->current_line,
          (int)(toknz->p - toknz->buf), (int)(toknz->pe - toknz->buf),
          toknz->ts, toknz->te, toknz->act);
   exit(EXIT_FAILURE);
}
#define ABORT(toknz, ...) \
   _eo_tokenizer_abort(toknz, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);

static void _eo_tokenizer_normalize_buf(char *buf)
{
   int c;
   char *s, *d;
   Eina_Bool in_space = EINA_TRUE;
   Eina_Bool in_newline = EINA_FALSE;

   /* ' '+ -> ' '
    * '\n' ' '* '*' ' '* -> '\n'
    */
   for (s = buf, d = buf; *s != '\0'; s++)
     {
        c = *s;
        *d = c;

        if (!in_space || (c != ' '))
          d++;

        if (c == ' ')
          in_space = EINA_TRUE;
        else
          in_space = EINA_FALSE;

        if (c == '\n')
          {
             in_newline = EINA_TRUE;
             in_space = EINA_TRUE;
          }
        else if (in_newline && c == '*' )
          {
             in_space = EINA_TRUE;
             in_newline = EINA_FALSE;
             d--;
          }
     }
   /* ' '+$ -> $ */
   d--;
   while (*d == ' ') d--;
   d++;
   if (d < buf) return;
   *d = '\0';
}

static const char*
_eo_tokenizer_token_get(Eo_Tokenizer *toknz, char *p)
{
   if (toknz->saved.tok == NULL) ABORT(toknz, "toknz->saved.tok is NULL");
   char d[BUFSIZE];
   int l = (p - toknz->saved.tok);
   memcpy(d, toknz->saved.tok, l);
   d[l] = '\0';
   _eo_tokenizer_normalize_buf(d);
   toknz->saved.tok = NULL;
   DBG("token : >%s<", d);
   return eina_stringshare_add(d);
}

static Eo_Class_Def*
_eo_tokenizer_class_get(Eo_Tokenizer *toknz, char *p)
{
   Eo_Class_Def *kls = calloc(1, sizeof(Eo_Class_Def));
   if (kls == NULL) ABORT(toknz, "calloc Eo_Class_Def failure");

   kls->name = _eo_tokenizer_token_get(toknz, p);

   return kls;
}

static Eo_Property_Def*
_eo_tokenizer_property_get(Eo_Tokenizer *toknz, char *p)
{
   Eo_Property_Def *prop = calloc(1, sizeof(Eo_Property_Def));
   if (prop == NULL) ABORT(toknz, "calloc Eo_Property_Def failure");

   prop->name = _eo_tokenizer_token_get(toknz, p);

   return prop;
}

static Eo_Method_Def*
_eo_tokenizer_method_get(Eo_Tokenizer *toknz, char *p)
{
   Eo_Method_Def *meth = calloc(1, sizeof(Eo_Method_Def));
   if (meth == NULL) ABORT(toknz, "calloc Eo_Method_Def failure");

   meth->name = _eo_tokenizer_token_get(toknz, p);

   return meth;
}

static Eo_Param_Def*
_eo_tokenizer_param_get(Eo_Tokenizer *toknz, char *p)
{
   char *s;

   Eo_Param_Def *param = calloc(1, sizeof(Eo_Param_Def));
   if (param == NULL) ABORT(toknz, "calloc Eo_Param_Def failure");

   s = p - 1; /* Don't look at ';' */
   /* Remove any space between the param name and ';'
    * This loop fixes the case where "char *name ;" becomes the type of the param.
    */
   while (*s == ' ') s--;
   for (; s >= toknz->saved.tok; s--)
     {
        if ((*s == ' ') || (*s == '*'))
          break;
     }

   if (s == toknz->saved.tok)
     ABORT(toknz, "wrong parameter: %s", _eo_tokenizer_token_get(toknz, p));
   s++;

   param->way = PARAM_IN;
   if (strncmp(toknz->saved.tok, "in ", 3) == 0)
     {
        toknz->saved.tok += 3;
        param->way = PARAM_IN;
     }
   else if (strncmp(toknz->saved.tok, "out ", 4) == 0)
     {
        toknz->saved.tok += 4;
        param->way = PARAM_OUT;
     }
   else if (strncmp(toknz->saved.tok, "inout ", 6) == 0)
     {
        toknz->saved.tok += 6;
        param->way = PARAM_INOUT;
     }

   param->type = _eo_tokenizer_token_get(toknz, s);

   toknz->saved.tok = s;
   param->name = _eo_tokenizer_token_get(toknz, p);

   return param;
}

static Eo_Accessor_Param*
_eo_tokenizer_accessor_param_get(Eo_Tokenizer *toknz, char *p)
{
   Eo_Accessor_Param *param = calloc(1, sizeof(Eo_Accessor_Param));
   if (param == NULL) ABORT(toknz, "calloc Eo_Accessor_Param failure");

   /* Remove the colon and spaces - we just need the param name */
   while (*p == ':') p--;
   while (*p == ' ') p--;
   param->name = _eo_tokenizer_token_get(toknz, p);

   return param;
}

static Eo_Accessor_Def *
_eo_tokenizer_accessor_get(Eo_Tokenizer *toknz, Eo_Accessor_Type type)
{
   Eo_Accessor_Def *accessor = calloc(1, sizeof(Eo_Accessor_Def));
   if (accessor == NULL) ABORT(toknz, "calloc Eo_Accessor_Def failure");

   accessor->type = type;

   return accessor;
}

static Eo_Signal_Def*
_eo_tokenizer_signal_get(Eo_Tokenizer *toknz, char *p)
{
   Eo_Signal_Def *sgn = calloc(1, sizeof(Eo_Signal_Def));
   if (sgn == NULL) ABORT(toknz, "calloc Eo_Signal_Def failure");

   sgn->name = _eo_tokenizer_token_get(toknz, p);

   return sgn;
}

static Eo_DfltCtor_Def*
_eo_tokenizer_dflt_ctor_get(Eo_Tokenizer *toknz, char *p)
{
   Eo_DfltCtor_Def *ctor = calloc(1, sizeof(Eo_DfltCtor_Def));
   if (ctor == NULL) ABORT(toknz, "calloc Eo_DfltCtor_Def failure");

   ctor->name = _eo_tokenizer_token_get(toknz, p);

   return ctor;
}

static Eo_DfltDtor_Def*
_eo_tokenizer_dflt_dtor_get(Eo_Tokenizer *toknz, char *p)
{
   Eo_DfltDtor_Def *dtor = calloc(1, sizeof(Eo_DfltDtor_Def));
   if (dtor == NULL) ABORT(toknz, "calloc Eo_DfltDtor_Def failure");

   dtor->name = _eo_tokenizer_token_get(toknz, p);

   return dtor;
}

static Eo_Implement_Def*
_eo_tokenizer_implement_get(Eo_Tokenizer *toknz, char *p)
{
   Eo_Implement_Def *impl = calloc(1, sizeof(Eo_Implement_Def));
   if (impl == NULL) ABORT(toknz, "calloc Eo_DfltDtor_Def failure");

   impl->meth_name = _eo_tokenizer_token_get(toknz, p);

   return impl;
}

%%{
   machine common;

   access toknz->;
   variable p toknz->p;
   variable pe toknz->pe;
   variable eof toknz->eof;

   action inc_line {
      toknz->current_line += 1;
      DBG("inc[%d] %d", toknz->cs, toknz->current_line);
   }

   action save_line {
      toknz->saved.line = toknz->current_line;
      DBG("save line[%d] %d", toknz->cs, toknz->current_line);
   }

   action save_fpc {
      toknz->saved.tok = fpc;
      DBG("save token[%d] %p %c", toknz->cs, fpc, *fpc);
   }

   action show_comment {
      DBG("comment[%d] line%03d:%03d", toknz->cs,
          toknz->saved.line, toknz->current_line);
   }

   action show_ignore {
      DBG("ignore[%d] line:%d", toknz->cs, toknz->current_line);
   }

   action show_error {
      DBG("error[%d]", toknz->cs);
      char *s, *d;
      char buf[BUFSIZE];
      for (s = fpc, d = buf; (s <= toknz->pe); s++)
        {
           if ((*s == '\r') || (*s == '\n'))
             break;
           *d++ = *s;
        }
      *d = '\0';
      ERR("error n:%d l:%d c:'%c': %s",
          toknz->current_nesting, toknz->current_line, *fpc, buf);
      toknz->cs = eo_tokenizer_error;
      fbreak;  /* necessary to stop scanners */
   }

   cr                = '\n';
   cr_neg            = [^\n];
   ws                = [ \t\r];
   newline           = cr @inc_line;
   ignore            = (0x00..0x20 - cr) | newline;

   alnum_u           = alnum | '_';
   alpha_u           = alpha | '_';
   ident             = alpha+ >save_fpc (alnum | '_' )+;
   signal            = alpha+ >save_fpc (alnum | '_' | ',' )+;
   class_meth        = alpha+ >save_fpc (alnum | '_' | '::' )+;

   eo_comment        = "/*@" ignore* alnum_u >save_fpc ( any | cr @inc_line )* :>> "*/";
   c_comment         = "/*" ( any | cr @inc_line )* :>> "*/";
   cpp_comment       = "//" (any - cr )* newline;
   comment           = ( c_comment | cpp_comment ) > save_line;

   end_statement     = ';';
   begin_def         = '{';
   end_def           = '}' end_statement;
   begin_list        = '(';
   end_list          = ')';
   list_separator    = ',';
   colon             = ':';

}%%

%%{
   machine eo_tokenizer;
   include common;

   write data;

###### TOKENIZE ACCESSOR

   action end_accessor_comment {
      if (toknz->tmp.accessor->comment != NULL)
        ABORT(toknz, "accessor has already a comment");
      toknz->tmp.accessor->comment = _eo_tokenizer_token_get(toknz, fpc-1);
      INF("        %s", toknz->tmp.accessor->comment);
   }

   action end_accessor_rettype {
      if (toknz->tmp.accessor->ret.type != NULL)
        ABORT(toknz, "accessor has already a return type");
      toknz->tmp.accessor->ret.type = _eo_tokenizer_token_get(toknz, fpc);
      INF("        %s", toknz->tmp.accessor->ret.type);
   }

   action end_accessor_rettype_comment {
      if (toknz->tmp.accessor->ret.comment != NULL)
        ABORT(toknz, "accessor return type has already a comment");
      toknz->tmp.accessor->ret.comment = _eo_tokenizer_token_get(toknz, fpc-2);
      INF("        %s", toknz->tmp.accessor->ret.comment);
   }

   action end_accessor_legacy {
      toknz->tmp.accessor->legacy = _eo_tokenizer_token_get(toknz, fpc);
   }

   action end_accessor {
      INF("      }");
      toknz->tmp.prop->accessors = eina_list_append(toknz->tmp.prop->accessors, toknz->tmp.accessor);
      toknz->tmp.accessor = NULL;
      toknz->current_nesting--;
      fgoto tokenize_property;
   }

   action begin_param_desc {
      toknz->tmp.accessor_param = _eo_tokenizer_accessor_param_get(toknz, fpc);
   }

   action end_param_desc {
      toknz->tmp.accessor_param->attrs = _eo_tokenizer_token_get(toknz, fpc);
      toknz->tmp.accessor->params =
         eina_list_append(toknz->tmp.accessor->params, toknz->tmp.accessor_param);
      toknz->tmp.accessor_param = NULL;
   }

   rettype_comment = ws* eo_comment %end_accessor_rettype_comment;
   rettype = 'return' ws+ alpha+ >save_fpc (alnum_u | '*' | ws )+  %end_accessor_rettype end_statement rettype_comment?;

   legacy = 'legacy' ws+ ident %end_accessor_legacy end_statement;

   param_desc = ident ws* colon %begin_param_desc ws* alpha+ >save_fpc (alnum_u | list_separator | ws)* %end_param_desc end_statement;

   tokenize_accessor := |*
      ignore+;    #=> show_ignore;
      eo_comment  => end_accessor_comment;
      comment     => show_comment;
      rettype;
      legacy;
      param_desc;
      end_def     => end_accessor;
      any         => show_error;
      *|;

###### TOKENIZE PARAMS

   action end_param_comment {
      const char *c = _eo_tokenizer_token_get(toknz, fpc-2);
      if (toknz->tmp.param == NULL)
        ABORT(toknz, "no parameter set to associate this comment to: %s", c);
      toknz->tmp.param->comment = c;
      toknz->tmp.param = NULL;
   }

   action end_param {
      toknz->tmp.param = _eo_tokenizer_param_get(toknz, fpc);
      if (toknz->tmp.prop)
        toknz->tmp.prop->params = eina_list_append(toknz->tmp.prop->params, toknz->tmp.param);
      else if (toknz->tmp.meth)
        toknz->tmp.meth->params = eina_list_append(toknz->tmp.meth->params, toknz->tmp.param);
      else
        ABORT(toknz, "got a param but there is no property nor method waiting for it");
      INF("        %s : %s", toknz->tmp.param->name, toknz->tmp.param->type);
   }

   action end_params {
      INF("      }");
      toknz->tmp.param = NULL;
      toknz->current_nesting--;
      if (toknz->tmp.prop)
        fgoto tokenize_property;
      else if (toknz->tmp.meth)
        fgoto tokenize_method;
      else
        ABORT(toknz, "leaving tokenize_params but there is no property nor method pending");
   }

   param_comment = ws* eo_comment %end_param_comment;
   param = alpha+ >save_fpc (alnum_u | '*' | ws )+  %end_param end_statement param_comment?;

   tokenize_params := |*
      ignore+;    #=> show_ignore;
      comment     => show_comment;
      param;
      end_def     => end_params;
      any         => show_error;
      *|;

###### TOKENIZE PROPERTY

   action begin_property_get {
      INF("      get {");
      toknz->tmp.accessor = _eo_tokenizer_accessor_get(toknz, GETTER);
      toknz->current_nesting++;
      fgoto tokenize_accessor;
   }

   action begin_property_set {
      INF("      set {");
      toknz->tmp.accessor = _eo_tokenizer_accessor_get(toknz, SETTER);
      toknz->current_nesting++;
      fgoto tokenize_accessor;
   }

   action begin_property_params {
      INF("      params {");
      toknz->current_nesting++;
      fgoto tokenize_params;
   }

   action end_property {
      if (eina_list_count(toknz->tmp.prop->params) == 0)
        WRN("property '%s' has no parameters.", toknz->tmp.prop->name);
      if (eina_list_count(toknz->tmp.prop->accessors) == 0)
        WRN("property '%s' has no accessors.", toknz->tmp.prop->name);
      INF("    }");
      toknz->tmp.kls->properties = eina_list_append(toknz->tmp.kls->properties, toknz->tmp.prop);
      toknz->tmp.prop = NULL;
      toknz->current_nesting--;
      fgoto tokenize_properties;
   }

   prop_get = 'get' ignore* begin_def;
   prop_set = 'set' ignore* begin_def;
   prop_params = 'params' ignore* begin_def;

   tokenize_property := |*
      ignore+;    #=> show_ignore;
      comment     => show_comment;
      prop_get    => begin_property_get;
      prop_set    => begin_property_set;
      prop_params => begin_property_params;
      end_def     => end_property;
      any         => show_error;
      *|;

###### TOKENIZE PROPERTIES

   action begin_property {
      INF("    %s {", toknz->tmp.prop->name);
      toknz->current_nesting++;
      fgoto tokenize_property;
   }

   action end_property_name {
      if (toknz->tmp.prop != NULL)
        ABORT(toknz, "there is a pending property definition %s", toknz->tmp.prop->name);
      toknz->tmp.prop = _eo_tokenizer_property_get(toknz, fpc);
   }

   action end_properties {
      INF("  }");
      toknz->current_nesting--;
      fgoto tokenize_class;
   }

   begin_property = ident %end_property_name ignore* begin_def;

   tokenize_properties := |*
      ignore+;       #=> show_ignore;
      comment        => show_comment;
      begin_property => begin_property;
      end_def        => end_properties;
      any            => show_error;
      *|;

###### TOKENIZE METHOD

   action end_method_comment {
      if (toknz->tmp.meth->comment != NULL)
        ABORT(toknz, "method has already a comment");
      toknz->tmp.meth->comment = _eo_tokenizer_token_get(toknz, fpc-1);
      INF("        %s", toknz->tmp.meth->comment);
   }

   action begin_method_params {
      INF("      params {");
      toknz->current_nesting++;
      fgoto tokenize_params;
   }

   action end_method_rettype {
      if (toknz->tmp.meth->ret.type != NULL)
        ABORT(toknz, "method '%s' has already a return type", toknz->tmp.meth->name);
      toknz->tmp.meth->ret.type = _eo_tokenizer_token_get(toknz, fpc);
      INF("        %s", toknz->tmp.meth->ret.type);
   }

   action end_method_rettype_comment {
      if (toknz->tmp.meth->ret.comment != NULL)
        ABORT(toknz, "method '%s' return type has already a comment", toknz->tmp.meth->name);
      toknz->tmp.meth->ret.comment = _eo_tokenizer_token_get(toknz, fpc-2);
      INF("        %s", toknz->tmp.meth->ret.comment);
   }

   action end_method_legacy {
      toknz->tmp.meth->legacy = _eo_tokenizer_token_get(toknz, fpc);
   }

   action end_method_obj_const{
      toknz->tmp.meth->obj_const = EINA_TRUE;
      INF("        obj const");
   }

   action end_method {
      Eina_List **l;
      if (eina_list_count(toknz->tmp.meth->params) == 0)
        WRN("method '%s' has no parameters.", toknz->tmp.meth->name);
      INF("    }");
      switch (toknz->current_methods_type) {
        case METH_CONSTRUCTOR:
          l = &toknz->tmp.kls->constructors;
          break;
        case METH_DESTRUCTOR:
          l = &toknz->tmp.kls->destructors;
          break;
        case METH_REGULAR:
          l = &toknz->tmp.kls->methods;
          break;
        default:
          ABORT(toknz, "unknown method type %d", toknz->current_methods_type);
      }
      toknz->tmp.meth->type = toknz->current_methods_type;
      *l = eina_list_append(*l, toknz->tmp.meth);
      toknz->tmp.meth = NULL;
      toknz->current_nesting--;
      fgoto tokenize_methods;
   }

   meth_params = 'params' ignore* begin_def;
   meth_legacy = 'legacy' ws+ ident %end_method_legacy end_statement;
   meth_rettype_comment = ws* eo_comment %end_method_rettype_comment;
   meth_rettype = 'return' ws+ alpha+ >save_fpc (alnum_u | '*' | ws )+  %end_method_rettype end_statement meth_rettype_comment?;
   meth_obj_const = 'object' ws* colon ws* 'const' %end_method_obj_const end_statement;

   tokenize_method := |*
      ignore+;    #=> show_ignore;
      eo_comment  => end_method_comment;
      comment     => show_comment;
      meth_params => begin_method_params;
      meth_rettype;
      meth_legacy;
      meth_obj_const;
      end_def     => end_method;
      any         => show_error;
      *|;

###### TOKENIZE METHODS

   action begin_method {
      INF("    %s {", toknz->tmp.meth->name);
      toknz->current_nesting++;
      fgoto tokenize_method;
   }

   action end_method_name {
      if (toknz->tmp.meth != NULL)
        ABORT(toknz, "there is a pending method definition %s", toknz->tmp.meth->name);
      toknz->tmp.meth = _eo_tokenizer_method_get(toknz, fpc);
   }

   action end_methods {
      INF("  }");
      toknz->current_methods_type = METH_TYPE_LAST;
      toknz->current_nesting--;
      fgoto tokenize_class;
   }

   begin_method = ident %end_method_name ignore* begin_def;

   tokenize_methods := |*
      ignore+;       #=> show_ignore;
      comment        => show_comment;
      begin_method   => begin_method;
      end_def        => end_methods;
      any            => show_error;
      *|;

###### TOKENIZE CLASS

   action end_class_comment {
      if (toknz->tmp.kls->comment != NULL)
        ABORT(toknz, "class %s has already a comment", toknz->tmp.kls->name);
      toknz->tmp.kls->comment = _eo_tokenizer_token_get(toknz, fpc-1);
   }

   action end_str_item{
      const char *base = _eo_tokenizer_token_get(toknz, fpc);
      toknz->tmp.str_items = eina_list_append(toknz->tmp.str_items, base);
   }

   action end_inherits {
      toknz->tmp.kls->inherits = toknz->tmp.str_items;
      toknz->tmp.str_items = NULL;
   }

   action end_implements {
   }

   action end_signals {
   }

   action begin_constructors {
      INF("  constructors {");
      toknz->current_methods_type = METH_CONSTRUCTOR;
      toknz->current_nesting++;
      fgoto tokenize_methods;
   }

   action begin_destructors {
      INF("  destructors {");
      toknz->current_methods_type = METH_DESTRUCTOR;
      toknz->current_nesting++;
      fgoto tokenize_methods;
   }

   action begin_properties {
      INF("  properties {");
      toknz->current_nesting++;
      fgoto tokenize_properties;
   }

   action begin_methods {
      INF("  begin methods");
      toknz->current_methods_type = METH_REGULAR;
      toknz->current_nesting++;
      fgoto tokenize_methods;
   }

   action end_class {
      INF("end class: %s", toknz->tmp.kls->name);
      toknz->classes = eina_list_append(toknz->classes, toknz->tmp.kls);
      toknz->tmp.kls = NULL;
      toknz->current_nesting--;
      fgoto main;
   }

   action end_signal_name {
      toknz->tmp.signal = _eo_tokenizer_signal_get(toknz, fpc);
      toknz->tmp.kls->signals = eina_list_append(toknz->tmp.kls->signals, toknz->tmp.signal);
   }

   action end_signal_comment {
      if (toknz->tmp.signal->comment != NULL)
        ABORT(toknz, "signal %s has already a comment", toknz->tmp.signal->name);
      toknz->tmp.signal->comment = _eo_tokenizer_token_get(toknz, fpc-2);
      toknz->tmp.signal = NULL;
   }

   action end_dflt_ctor_def {
      if (toknz->tmp.kls->dflt_ctor != NULL)
        ABORT(toknz, "A default constructor has already been defined");
      toknz->tmp.kls->dflt_ctor = _eo_tokenizer_dflt_ctor_get(toknz, fpc);
   }

   action end_legacy_prefix {
      if (toknz->tmp.kls->legacy_prefix != NULL)
        ABORT(toknz, "A legacy prefix has already been given");
      toknz->tmp.kls->legacy_prefix = _eo_tokenizer_token_get(toknz, fpc);
   }

   action end_dflt_dtor_def {
      if (toknz->tmp.kls->dflt_dtor != NULL)
        ABORT(toknz, "A default destructor has already been defined");
      toknz->tmp.kls->dflt_dtor = _eo_tokenizer_dflt_dtor_get(toknz, fpc);
   }

   action end_dflt_ctor_comment {
      if (toknz->tmp.kls->dflt_ctor == NULL)
        ABORT(toknz, "No default constructor is defined for the comment");
      if (toknz->tmp.kls->dflt_ctor->comment != NULL)
        ABORT(toknz, "default constructor %s has already a comment", toknz->tmp.kls->dflt_ctor->name);
      toknz->tmp.kls->dflt_ctor->comment = _eo_tokenizer_token_get(toknz, fpc-2);
   }

   action end_dflt_dtor_comment {
      if (toknz->tmp.kls->dflt_dtor == NULL)
        ABORT(toknz, "No default destructor is defined for the comment");
      if (toknz->tmp.kls->dflt_dtor->comment != NULL)
        ABORT(toknz, "default destructor %s has already a comment", toknz->tmp.kls->dflt_dtor->name);
      toknz->tmp.kls->dflt_dtor->comment = _eo_tokenizer_token_get(toknz, fpc-2);
   }

   legacy_prefix = 'legacy_prefix' ignore* colon ignore* ident %end_legacy_prefix end_statement ignore*;

   class_it = ident %end_str_item ignore*;
   class_it_next = list_separator ignore* class_it;
   inherits = 'inherits' ignore* begin_def ignore* (class_it class_it_next*)? end_def;

   action impl_meth_store {
        toknz->tmp.impl = _eo_tokenizer_implement_get(toknz, fpc);
        toknz->tmp.kls->implements = eina_list_append(toknz->tmp.kls->implements, toknz->tmp.impl);
   }

   action impl_legacy_create {
        if (toknz->tmp.impl->legacy)
           ABORT(toknz, "Legacy section already allocated for implement item");
        toknz->tmp.impl->legacy = calloc(1, sizeof(Eo_Implement_Legacy_Def));
   }

   action impl_legacy_function_name_store {
        if (!toknz->tmp.impl->legacy)
           ABORT(toknz, "No legacy section");
        toknz->tmp.impl->legacy->function_name = _eo_tokenizer_token_get(toknz, fpc);
   }

   action impl_legacy_eo_param_store {
        toknz->tmp.impl_leg_param = calloc(1, sizeof(Eo_Implement_Legacy_Param_Def));
        toknz->tmp.impl->legacy->params = eina_list_append(
              toknz->tmp.impl->legacy->params, toknz->tmp.impl_leg_param);

        toknz->tmp.impl_leg_param->eo_name = _eo_tokenizer_token_get(toknz, fpc);
   }

   action impl_legacy_leg_param_store {
        toknz->tmp.impl_leg_param->legacy_name = _eo_tokenizer_token_get(toknz, fpc);
   }

   action impl_legacy_param_comment_store {
        toknz->tmp.impl_leg_param->comment = _eo_tokenizer_token_get(toknz, fpc-2);
   }

   action impl_legacy_return_type_store {
        if (!toknz->tmp.impl->legacy)
           ABORT(toknz, "No legacy section");
        toknz->tmp.impl->legacy->ret_type= _eo_tokenizer_token_get(toknz, fpc);
   }

   action impl_legacy_return_val_store {
        if (!toknz->tmp.impl->legacy)
           ABORT(toknz, "No legacy section");
        toknz->tmp.impl->legacy->ret_value = _eo_tokenizer_token_get(toknz, fpc);
   }

#  legacy legacy_function_name
#    {
#       params {
#         grp: NULL; /*@ in case 'grp' is in Eo but not in legacy, have to give default */
#         file; /*@ in case the 'file' parameter is the same */
#         :index; /*@ in case the param is in legacy but not in Eo */
#       };
#       return Eina_Bool::EINA_TRUE;
#    };

   impl_legacy_param_comment = ws* eo_comment %impl_legacy_param_comment_store;
   impl_legacy_eo_param = ident %impl_legacy_eo_param_store;
   impl_legacy_leg_param = ws* colon ws* ident %impl_legacy_leg_param_store ignore*;
   impl_legacy_param = impl_legacy_eo_param? ws* impl_legacy_leg_param? end_statement impl_legacy_param_comment? ignore*;
   impl_legacy_params = impl_legacy_param+;

   impl_legacy_return = 'return' ws+ ident %impl_legacy_return_type_store '::' ident %impl_legacy_return_val_store end_statement ignore*;

   impl_legacy_body = 'params' ignore* begin_def ignore* impl_legacy_params ignore* end_def ignore* impl_legacy_return? ignore*;

   impl_legacy_function_name = ident %impl_legacy_function_name_store;
   impl_legacy_token = 'legacy' %impl_legacy_create;
   impl_body = impl_legacy_token ws* impl_legacy_function_name? ignore* (end_statement | (begin_def ignore* impl_legacy_body ignore* end_def)) ignore*;
# class::func ; or { ... }
   impl_it = class_meth %impl_meth_store ignore* (end_statement | (begin_def ignore* impl_body ignore* end_def)) ignore*;
# implements { ... }
   implements = 'implements' ignore* begin_def ignore* impl_it* end_def;

   signal_comment = ws* eo_comment %end_signal_comment;
   signal_it = signal %end_signal_name ignore* end_statement signal_comment? ignore*;
   signals = 'signals' ignore* begin_def ignore* signal_it* end_def;

   dflt_ctor_comment = ws* eo_comment %end_dflt_ctor_comment;
   dflt_ctor = 'constructor' >save_fpc %end_dflt_ctor_def ignore* end_statement dflt_ctor_comment? ignore*;

   dflt_dtor_comment = ws* eo_comment %end_dflt_dtor_comment;
   dflt_dtor = 'destructor' >save_fpc %end_dflt_dtor_def ignore* end_statement dflt_dtor_comment? ignore*;

   constructors = 'constructors' ignore* begin_def;
   destructors = 'destructors' ignore* begin_def;
   properties = 'properties' ignore* begin_def;
   methods = 'methods' ignore* begin_def;

   tokenize_class := |*
      ignore+;       #=> show_ignore;
      eo_comment     => end_class_comment;
      comment        => show_comment;
      legacy_prefix;
      inherits       => end_inherits;
      implements     => end_implements;
      signals        => end_signals;
      dflt_ctor;
      dflt_dtor;
      constructors   => begin_constructors;
      destructors    => begin_destructors;
      properties     => begin_properties;
      methods        => begin_methods;
      end_def        => end_class;
      any            => show_error;
      *|;

###### TOP LEVEL

   action begin_class {
      INF("begin class: %s", toknz->tmp.kls->name);
      toknz->current_nesting++;
      fgoto tokenize_class;
   }

   action end_class_name {
      if (toknz->tmp.kls != NULL)
        ABORT(toknz, "there is a pending class definition %s", toknz->tmp.kls->name);
      toknz->tmp.kls = _eo_tokenizer_class_get(toknz, fpc);
   }

   action end_class_type {
      if (!toknz->tmp.kls)
         ABORT(toknz, "No pending class definition");
      toknz->tmp.kls->type = _eo_tokenizer_token_get(toknz, fpc);
   }

   class_name = ident %end_class_name;
   class_type = ws+ "as" ws+ >save_fpc ("Regular" | "Mixin" | "RegularNonInstantiable" | "Interface") %end_class_type; # class as type(Regular, Mixin...)
   begin_class = class_name class_type? ignore* begin_def;

   main := |*
      ignore+;    #=> show_ignore;
      comment     => show_comment;
      begin_class => begin_class;
      any         => show_error;
   *|;

}%%

Eina_Bool
eo_tokenizer_walk(Eo_Tokenizer *toknz, const char *source)
{
   INF("tokenize %s...", source);
   toknz->source = eina_stringshare_add(source);

   FILE *stream;

   int done = 0;
   int have = 0;
   int offset = 0;

   stream = fopen(toknz->source, "rb");
   if (!stream)
     {
        ERR("unable to read in %s", toknz->source);
        return EINA_FALSE;
     }

   %% write init;

   while (!done)
     {
        int len;
        int space;

        toknz->p = toknz->buf + have;
        space = BUFSIZE - have;

        if (space == 0)
          {
             fclose(stream);
             ABORT(toknz, "out of buffer space");
          }

        len = fread(toknz->p, 1, space, stream);
        if (len == 0) break;
        toknz->pe = toknz->p + len;

        if (len < space)
          {
             toknz->eof = toknz->pe;
             done = 1;
          }

        %% write exec;

        if ( toknz->cs == %%{ write error; }%% )
          {
             ERR("wrong termination");
             break;
          }

        if ( toknz->ts == 0 )
          have = 0;
        else
          {
             DBG("move data and pointers before buffer feed");
             have = toknz->pe - toknz->ts;
             offset = toknz->ts - toknz->buf;
             memmove(toknz->buf, toknz->ts, have);
             toknz->te -= offset;
             toknz->ts = toknz->buf;
          }

        if (toknz->saved.tok != NULL)
          {
             if ((have == 0) || ((toknz->saved.tok - offset) < toknz->buf))
               {
                  WRN("reset lost saved token %p", toknz->saved.tok);
                  toknz->saved.tok = NULL;
               }
             else
               toknz->saved.tok -= offset;
          }
     }

   fclose(stream);

   return EINA_TRUE;
}

Eo_Tokenizer*
eo_tokenizer_get(void)
{
   Eo_Tokenizer *toknz = calloc(1, sizeof(Eo_Tokenizer));
   if (!toknz) return NULL;

   toknz->ts = NULL;
   toknz->te = NULL;
   /* toknz->top = 0; */
   toknz->source = NULL;
   toknz->max_nesting = 10;
   toknz->current_line = 1;
   toknz->current_nesting = 0;
   toknz->current_methods_type = METH_TYPE_LAST;
   toknz->saved.tok = NULL;
   toknz->saved.line = 0;
   toknz->classes = NULL;

   return toknz;
}

static char *_accessor_type_str[ACCESSOR_TYPE_LAST] = { "setter", "getter" };
static char *_param_way_str[PARAM_WAY_LAST] = { "IN", "OUT", "INOUT" };

void
eo_tokenizer_dump(Eo_Tokenizer *toknz)
{
   const char *s;
   Eina_List *k, *l, *m;

   Eo_Class_Def *kls;
   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;
   Eo_Signal_Def *sgn;
   /* Eo_Ret_Def *ret; */

   EINA_LIST_FOREACH(toknz->classes, k, kls)
     {
        printf("Class: %s (%s)\n",
               kls->name, (kls->comment ? kls->comment : "-"));
        printf("  inherits from :");
        EINA_LIST_FOREACH(kls->inherits, l, s)
           printf(" %s", s);
        printf("\n");
        printf("  implements:");
        EINA_LIST_FOREACH(kls->implements, l, s)
           printf(" %s", s);
        printf("\n");
        printf("  signals:\n");
        EINA_LIST_FOREACH(kls->signals, l, sgn)
           printf("    %s (%s)\n", sgn->name, sgn->comment);

        EINA_LIST_FOREACH(kls->constructors, l, meth)
          {
             printf("  constructors: %s\n", meth->name);
             printf("    return: %s (%s)\n", meth->ret.type, meth->ret.comment);
             printf("    legacy : %s\n", meth->legacy);
             EINA_LIST_FOREACH(meth->params, m, param)
               {
                  printf("    param: %s %s : %s (%s)\n",
                         _param_way_str[param->way], param->name,
                         param->type, param->comment);
               }
          }

        EINA_LIST_FOREACH(kls->destructors, l, meth)
          {
             printf("  destructors: %s\n", meth->name);
             printf("    return: %s (%s)\n", meth->ret.type, meth->ret.comment);
             printf("    legacy : %s\n", meth->legacy);
             EINA_LIST_FOREACH(meth->params, m, param)
               {
                  printf("    param: %s %s : %s (%s)\n",
                         _param_way_str[param->way], param->name,
                         param->type, param->comment);
               }
          }

        EINA_LIST_FOREACH(kls->properties, l, prop)
          {
             printf("  property: %s\n", prop->name);
             EINA_LIST_FOREACH(prop->params, m, param)
               {
                  printf("    param: %s : %s (%s)\n",
                         param->name, param->type, param->comment);
               }
             EINA_LIST_FOREACH(prop->accessors, m, accessor)
               {
                  printf("    accessor: %s : %s (%s)\n",
                         accessor->ret.type, _accessor_type_str[accessor->type],
                         accessor->comment);
                  printf("      legacy : %s\n", accessor->legacy);
               }
          }

        EINA_LIST_FOREACH(kls->methods, l, meth)
          {
             printf("  method: %s\n", meth->name);
             printf("    return: %s (%s)\n", meth->ret.type, meth->ret.comment);
             printf("    legacy : %s\n", meth->legacy);
             printf("    obj_const : %s\n", meth->obj_const?"true":"false");
             EINA_LIST_FOREACH(meth->params, m, param)
               {
                  printf("    param: %s %s : %s (%s)\n",
                         _param_way_str[param->way], param->name,
                         param->type, param->comment);
               }
          }

     }

}

Eina_Bool
eo_tokenizer_database_fill(const char *filename)
{
   const char *s;
   Eina_List *k, *l, *m;

   Eo_Class_Def *kls;
   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;
   Eo_Signal_Def *signal;
   Eo_Implement_Def *impl;
   /* Eo_Ret_Def *ret; */

   Eo_Tokenizer *toknz = eo_tokenizer_get();
   if (!toknz)
     {
        ERR("can't create eo_tokenizer");
        return EINA_FALSE;
     }

   if (access(filename, F_OK) != 0)
     {
        ERR("error accessing file %s : %s", filename, strerror(errno));
        return EINA_FALSE;
     }
   eo_tokenizer_walk(toknz, filename);

   EINA_LIST_FOREACH(toknz->classes, k, kls)
     {
        Eolian_Class_Type type = EOLIAN_CLASS_REGULAR;
        if (kls->type)
          {
             if (!strcmp(kls->type, "Regular")) type = EOLIAN_CLASS_REGULAR;
             else if (!strcmp(kls->type, "RegularNonInstantiable")) type = EOLIAN_CLASS_REGULAR_NON_INSTANT;
             else if (!strcmp(kls->type, "Mixin")) type = EOLIAN_CLASS_MIXIN;
             else if (!strcmp(kls->type, "Interface")) type = EOLIAN_CLASS_INTERFACE;
             else type = EOLIAN_CLASS_UNKNOWN_TYPE;
          }
        database_class_add(kls->name, type);
        if (kls->comment) database_class_description_set(kls->name, kls->comment);

        EINA_LIST_FOREACH(kls->inherits, l, s)
           database_class_inherit_add(kls->name, s);

        if (kls->legacy_prefix)
          {
             database_class_legacy_prefix_set(kls->name, kls->legacy_prefix);
          }
        if (kls->dflt_ctor)
          {
             Eolian_Function foo_id = database_function_new(kls->dflt_ctor->name, DFLT_CONSTRUCTOR);
             database_class_function_add(kls->name, foo_id);
             database_function_description_set(foo_id, EOLIAN_COMMENT, kls->dflt_ctor->comment);
          }
        if (kls->dflt_dtor)
          {
             Eolian_Function foo_id = database_function_new(kls->dflt_dtor->name, DFLT_DESTRUCTOR);
             database_class_function_add(kls->name, foo_id);
             database_function_description_set(foo_id, EOLIAN_COMMENT, kls->dflt_dtor->comment);
          }
        EINA_LIST_FOREACH(kls->constructors, l, meth)
          {
             Eolian_Function foo_id = database_function_new(meth->name, CONSTRUCTOR);
             database_class_function_add(kls->name, foo_id);
             database_function_description_set(foo_id, EOLIAN_RETURN_COMMENT, meth->ret.comment);
             database_function_data_set(foo_id, EOLIAN_LEGACY, meth->legacy);
             EINA_LIST_FOREACH(meth->params, m, param)
               {
                  database_function_parameter_add(foo_id, (Eolian_Parameter_Dir)param->way, param->type, param->name, param->comment);
               }
          }

        EINA_LIST_FOREACH(kls->destructors, l, meth)
          {
             Eolian_Function foo_id = database_function_new(meth->name, DESTRUCTOR);
             database_class_function_add(kls->name, foo_id);
             database_function_description_set(foo_id, EOLIAN_RETURN_COMMENT, meth->ret.comment);
             database_function_data_set(foo_id, EOLIAN_LEGACY, meth->legacy);
             EINA_LIST_FOREACH(meth->params, m, param)
               {
                  database_function_parameter_add(foo_id, (Eolian_Parameter_Dir)param->way, param->type, param->name, param->comment);
               }
          }

        EINA_LIST_FOREACH(kls->properties, l, prop)
          {
             Eolian_Function foo_id = database_function_new(prop->name, UNRESOLVED);
             EINA_LIST_FOREACH(prop->params, m, param)
               {
                  /* IN_PARAM doesn't care */
                  database_function_parameter_add(foo_id, EOLIAN_IN_PARAM, param->type, param->name, param->comment);
               }
             EINA_LIST_FOREACH(prop->accessors, m, accessor)
               {
                  database_function_type_set(foo_id, (accessor->type == SETTER?SET:GET));
                  if (accessor->ret.type)
                     database_function_data_set(foo_id,
                           (accessor->type == SETTER?EOLIAN_PROP_SET_RETURN_TYPE:EOLIAN_PROP_GET_RETURN_TYPE),
                           accessor->ret.type);
                  database_function_description_set(foo_id,
                        (accessor->type == SETTER?EOLIAN_COMMENT_SET:EOLIAN_COMMENT_GET),
                        accessor->comment);
                  Eo_Accessor_Param *acc_param;
                  Eina_List *m2;
                  /* Only in get access, we check const attribute */
                  if (accessor->type == GETTER)
                    {
                       EINA_LIST_FOREACH(accessor->params, m2, acc_param)
                         {
                            Eolian_Function_Parameter desc = eolian_function_parameter_get(foo_id, acc_param->name);
                            if (!desc)
                              {
                                 printf("Error - %s not known as parameter of property %s\n", acc_param->name, prop->name);
                                 return EINA_FALSE;
                              }
                            if (strstr(acc_param->attrs, "const"))
                              {
                                 database_parameter_get_const_attribute_set(desc, EINA_TRUE);
                              }
                         }
                    }
               }
             database_class_function_add(kls->name, foo_id);
          }

        EINA_LIST_FOREACH(kls->methods, l, meth)
          {
             Eolian_Function foo_id = database_function_new(meth->name, METHOD_FUNC);
             database_class_function_add(kls->name, foo_id);
             database_function_data_set(foo_id, EOLIAN_METHOD_RETURN_TYPE, meth->ret.type);
             database_function_description_set(foo_id, EOLIAN_RETURN_COMMENT, meth->ret.comment);
             database_function_description_set(foo_id, EOLIAN_COMMENT, meth->comment);
             database_function_data_set(foo_id, EOLIAN_LEGACY, meth->legacy);
             database_function_object_set_as_const(foo_id, meth->obj_const);
             EINA_LIST_FOREACH(meth->params, m, param)
               {
                  database_function_parameter_add(foo_id, (Eolian_Parameter_Dir)param->way, param->type, param->name, param->comment);
               }
          }

        EINA_LIST_FOREACH(kls->implements, l, impl)
          {
             const char *class = impl->meth_name;
             char *func = strstr(class, "::");
             if (func) *func = '\0';
             func += 2;
             Eolian_Function_Type ftype = UNRESOLVED;
             char *type_as_str = strstr(func, "::");
             if (type_as_str)
               {
                  *type_as_str = '\0';
                  if (!strcmp(type_as_str+2, "set")) ftype = SET;
                  else if (!strcmp(type_as_str+2, "get")) ftype = GET;
               }
             Eolian_Implement impl_desc = database_implement_new(class, func, ftype);
             if (impl->legacy)
               {
                  Eo_Implement_Legacy_Def *eo_leg = impl->legacy;
                  Eolian_Implement_Legacy leg = database_implement_legacy_add(
                        impl_desc, eo_leg->function_name);
                  database_implement_legacy_return_add(leg, eo_leg->ret_type, eo_leg->ret_value);
                  if (eo_leg->params)
                    {
                       Eina_List *itr;
                       Eo_Implement_Legacy_Param_Def *p;
                       EINA_LIST_FOREACH(eo_leg->params, itr, p)
                          database_implement_legacy_param_add(leg, p->eo_name,
                                p->legacy_name, p->comment);
                    }
               }
             database_class_implement_add(kls->name, impl_desc);
          }

        EINA_LIST_FOREACH(kls->signals, l, signal)
          {
             Eolian_Event ev = database_event_new(signal->name, signal->comment);
             database_class_event_add(kls->name, ev);
          }

     }

   eo_tokenizer_free(toknz);
   return EINA_TRUE;
}

void
eo_tokenizer_free(Eo_Tokenizer *toknz)
{
   Eo_Class_Def *kls;

   if (toknz->source)
     eina_stringshare_del(toknz->source);

   EINA_LIST_FREE(toknz->classes, kls)
      eo_definitions_class_def_free(kls);

   free(toknz);
}

