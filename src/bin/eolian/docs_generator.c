#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ctype.h>

#include "docs_generator.h"

static int
_indent_line(Eina_Strbuf *buf, int ind)
{
   int i;
   for (i = 0; i < ind; ++i)
     eina_strbuf_append_char(buf, ' ');
   return ind;
}

#define DOC_LINE_LIMIT 79
#define DOC_LINE_TEST 59
#define DOC_LINE_OVER 39

#define DOC_LIMIT(ind) ((ind > DOC_LINE_TEST) ? (ind + DOC_LINE_OVER) \
                                              : DOC_LINE_LIMIT)

int
_append_section(const char *desc, int ind, int curl, Eina_Strbuf *buf,
                Eina_Strbuf *wbuf)
{
   while (*desc)
     {
        eina_strbuf_reset(wbuf);
        while (*desc && isspace(*desc) && (*desc != '\n'))
          eina_strbuf_append_char(wbuf, *desc++);
        if (*desc == '\\')
          {
             desc++;
             if (*desc != '@')
               eina_strbuf_append_char(wbuf, '\\');
             eina_strbuf_append_char(wbuf, *desc++);
          }
        else if (*desc == '@')
          {
             desc++;
             if (isalpha(*desc))
               eina_strbuf_append(wbuf, "@ref ");
             else
               eina_strbuf_append_char(wbuf, '@');
          }
        while (*desc && !isspace(*desc))
          eina_strbuf_append_char(wbuf, *desc++);
        int limit = DOC_LIMIT(ind);
        int wlen = eina_strbuf_length_get(wbuf);
        if ((int)(curl + wlen) > limit)
          {
             curl = 3;
             eina_strbuf_append_char(buf, '\n');
             curl += _indent_line(buf, ind);
             eina_strbuf_append(buf, " * ");
             if (*eina_strbuf_string_get(wbuf) == ' ')
               eina_strbuf_remove(wbuf, 0, 1);
          }
        curl += eina_strbuf_length_get(wbuf);
        eina_strbuf_append(buf, eina_strbuf_string_get(wbuf));
        if (*desc == '\n')
          {
             desc++;
             eina_strbuf_append_char(buf, '\n');
             while (*desc == '\n')
               {
                  _indent_line(buf, ind);
                  eina_strbuf_append(buf, " *\n");
                  desc++;
               }
             curl = _indent_line(buf, ind) + 3;
             eina_strbuf_append(buf, " * ");
          }
     }
   return curl;
}

int
_append_since(const char *since, int indent, int curl, Eina_Strbuf *buf)
{
   if (since)
     {
        eina_strbuf_append_char(buf, '\n');
        _indent_line(buf, indent);
        eina_strbuf_append(buf, " *\n");
        curl = _indent_line(buf, indent);
        eina_strbuf_append(buf, " * @since ");
        eina_strbuf_append(buf, since);
        curl += strlen(since) + sizeof(" * @since ") - 1;
     }
   return curl;
}

void
_gen_doc_brief(const char *summary, const char *since, int indent,
               Eina_Strbuf *buf)
{
   int curl = 4 + indent;
   Eina_Strbuf *wbuf = eina_strbuf_new();
   eina_strbuf_append(buf, "/** ");
   curl = _append_section(summary, indent, curl, buf, wbuf);
   eina_strbuf_free(wbuf);
   curl = _append_since(since, indent, curl, buf);
   if ((curl + 3) > DOC_LIMIT(indent))
     {
        eina_strbuf_append_char(buf, '\n');
        _indent_line(buf, indent);
        eina_strbuf_append(buf, " */");
     }
   else
     eina_strbuf_append(buf, " */");
}

void
_gen_doc_full(const char *summary, const char *description, const char *since,
              int indent, Eina_Strbuf *buf)
{
   int curl = 0;
   Eina_Strbuf *wbuf = eina_strbuf_new();
   eina_strbuf_append(buf, "/**\n");
   curl += _indent_line(buf, indent);
   eina_strbuf_append(buf, " * @brief ");
   curl += sizeof(" * @brief ") - 1;
   _append_section(summary, indent, curl, buf, wbuf);
   eina_strbuf_append_char(buf, '\n');
   _indent_line(buf, indent);
   eina_strbuf_append(buf, " *\n");
   curl = _indent_line(buf, indent);
   eina_strbuf_append(buf, " * ");
   _append_section(description, indent, curl + 3, buf, wbuf);
   curl = _append_since(since, indent, curl, buf);
   eina_strbuf_append_char(buf, '\n');
   _indent_line(buf, indent);
   eina_strbuf_append(buf, " */");
   eina_strbuf_free(wbuf);
}

Eina_Strbuf *
docs_generate_full(const Eolian_Documentation *doc, int indent)
{
   if (!doc) return NULL;

   const char *sum = eolian_documentation_summary_get(doc);
   const char *desc = eolian_documentation_description_get(doc);
   const char *since = eolian_documentation_since_get(doc);

   Eina_Strbuf *buf = eina_strbuf_new();
   if (!desc)
     _gen_doc_brief(sum, since, indent, buf);
   else
     _gen_doc_full(sum, desc, since, indent, buf);
   return buf;
}

Eina_Strbuf *
docs_generate_function(const Eolian_Function *fid, Eolian_Function_Type ftype, int indent)
{
   const Eolian_Function_Parameter *par = NULL;
   const Eolian_Function_Parameter *vpar = NULL;

   const Eolian_Documentation *doc, *pdoc, *rdoc;

   Eina_Iterator *itr = NULL;
   Eina_Iterator *vitr = NULL;
   Eina_Bool force_out = EINA_FALSE;

   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Strbuf *wbuf = NULL;

   const char *sum = NULL, *desc = NULL, *since = NULL;

   int curl = 0;

   if (ftype == EOLIAN_UNRESOLVED)
     ftype = EOLIAN_METHOD;

   if (ftype == EOLIAN_METHOD)
     {
        doc = eolian_function_documentation_get(fid, EOLIAN_METHOD);
        pdoc = NULL;
     }
   else
     {
        doc = eolian_function_documentation_get(fid, EOLIAN_PROPERTY);
        pdoc = eolian_function_documentation_get(fid, ftype);
        if (!doc && pdoc) doc = pdoc;
        if (pdoc == doc) pdoc = NULL;
     }

   rdoc = eolian_function_return_documentation_get(fid, ftype);

   if (doc)
     {
         sum = eolian_documentation_summary_get(doc);
         desc = eolian_documentation_description_get(doc);
         since = eolian_documentation_since_get(doc);
         if (pdoc && eolian_documentation_since_get(pdoc))
           since = eolian_documentation_since_get(pdoc);
     }

   if (ftype == EOLIAN_METHOD)
     {
        itr = eolian_function_parameters_get(fid);
        if (!itr || !eina_iterator_next(itr, (void**)&par))
          {
             eina_iterator_free(itr);
             itr = NULL;
          }
     }
   else
     {
        itr = eolian_property_keys_get(fid, ftype);
        vitr = eolian_property_values_get(fid, ftype);
        if (!vitr || !eina_iterator_next(vitr, (void**)&vpar))
          {
             eina_iterator_free(vitr);
             vitr = NULL;
         }
     }

   if (!itr || !eina_iterator_next(itr, (void**)&par))
     {
        eina_iterator_free(itr);
        itr = NULL;
     }

   /* when return is not set on getter, value becomes return instead of param */
   if (ftype == EOLIAN_PROP_GET && !eolian_function_return_type_get(fid, ftype))
     {
        if (!eina_iterator_next(vitr, (void**)&vpar))
          {
             /* one value - not out param */
             eina_iterator_free(vitr);
             rdoc = eolian_parameter_documentation_get(vpar);
             vitr = NULL;
             vpar = NULL;
          }
        else
          {
             /* multiple values - always out params */
             eina_iterator_free(vitr);
             vitr = eolian_property_values_get(fid, ftype);
             if (!vitr)
               vpar = NULL;
             else if (!eina_iterator_next(vitr, (void**)&vpar))
               {
                  eina_iterator_free(vitr);
                  vitr = NULL;
                  vpar = NULL;
               }
          }
     }

   if (!par)
     {
        /* no keys, try values */
        itr = vitr;
        par = vpar;
        vitr = NULL;
        vpar = NULL;
        if (ftype == EOLIAN_PROP_GET)
          force_out = EINA_TRUE;
     }

   /* only summary, nothing else; generate standard brief doc */
   if (!desc && !par && !vpar && !rdoc && (ftype == EOLIAN_METHOD || !pdoc))
     {
        _gen_doc_brief(sum ? sum : "No description supplied.", since, indent, buf);
        return buf;
     }

   wbuf = eina_strbuf_new();

   eina_strbuf_append(buf, "/**\n");
   curl += _indent_line(buf, indent);
   eina_strbuf_append(buf, " * @brief ");
   curl += sizeof(" * @brief ") - 1;
   _append_section(sum ? sum : "No description supplied.",
                   indent, curl, buf, wbuf);

   eina_strbuf_append_char(buf, '\n');
   if (desc || since || par || rdoc || pdoc)
     {
        _indent_line(buf, indent);
        eina_strbuf_append(buf, " *\n");
     }

   if (desc)
     {
        curl = _indent_line(buf, indent);
        eina_strbuf_append(buf, " * ");
        _append_section(desc, indent, curl + 3, buf, wbuf);
        eina_strbuf_append_char(buf, '\n');
        if (par || rdoc || pdoc || since)
          {
             _indent_line(buf, indent);
             eina_strbuf_append(buf, " *\n");
          }
     }

   if (pdoc)
     {
        const char *pdesc = eolian_documentation_description_get(pdoc);
        curl = _indent_line(buf, indent);
        eina_strbuf_append(buf, " * ");
        _append_section(eolian_documentation_summary_get(pdoc), indent,
            curl + 3, buf, wbuf);
        eina_strbuf_append_char(buf, '\n');
        if (pdesc)
          {
             _indent_line(buf, indent);
             eina_strbuf_append(buf, " *\n");
             curl = _indent_line(buf, indent);
             eina_strbuf_append(buf, " * ");
             _append_section(pdesc, indent, curl + 3, buf, wbuf);
             eina_strbuf_append_char(buf, '\n');
          }
        if (par || rdoc || since)
          {
             _indent_line(buf, indent);
             eina_strbuf_append(buf, " *\n");
          }
     }

   while (par)
     {
        const Eolian_Documentation *adoc = eolian_parameter_documentation_get(par);
        curl = _indent_line(buf, indent);

        Eolian_Parameter_Dir dir = EOLIAN_OUT_PARAM;
        if (!force_out)
          dir = eolian_parameter_direction_get(par);

        switch (dir)
          {
           case EOLIAN_IN_PARAM:
             eina_strbuf_append(buf, " * @param[in] ");
             curl += sizeof(" * @param[in] ") - 1;
             break;
           case EOLIAN_OUT_PARAM:
             eina_strbuf_append(buf, " * @param[out] ");
             curl += sizeof(" * @param[out] ") - 1;
             break;
           case EOLIAN_INOUT_PARAM:
             eina_strbuf_append(buf, " * @param[in,out] ");
             curl += sizeof(" * @param[in,out] ") - 1;
             break;
          }

        const char *nm = eolian_parameter_name_get(par);
        eina_strbuf_append(buf, nm);
        curl += strlen(nm);

        if (adoc)
          {
             eina_strbuf_append_char(buf, ' ');
             curl += 1;
             _append_section(eolian_documentation_summary_get(adoc),
                             indent, curl, buf, wbuf);
          }

        eina_strbuf_append_char(buf, '\n');
        if (!eina_iterator_next(itr, (void**)&par))
          {
             par = NULL;
             if (vpar)
               {
                  eina_iterator_free(itr);
                  itr = vitr;
                  par = vpar;
                  vitr = NULL;
                  vpar = NULL;
                  if (ftype == EOLIAN_PROP_GET)
                    force_out = EINA_TRUE;
               }
          }

        if (!par && (rdoc || since))
          {
             _indent_line(buf, indent);
             eina_strbuf_append(buf, " *\n");
          }
     }
   eina_iterator_free(itr);

   if (rdoc)
     {
        curl = _indent_line(buf, indent);
        eina_strbuf_append(buf, " * @return ");
        curl += sizeof(" * @return ") - 1;
        _append_section(eolian_documentation_summary_get(rdoc), indent, curl,
            buf, wbuf);
        eina_strbuf_append_char(buf, '\n');
        if (since)
          {
             _indent_line(buf, indent);
             eina_strbuf_append(buf, " *\n");
          }
     }

   if (since)
     {
        curl = _indent_line(buf, indent);
        eina_strbuf_append(buf, " * @since ");
        eina_strbuf_append(buf, since);
        eina_strbuf_append_char(buf, '\n');
     }

   _indent_line(buf, indent);
   eina_strbuf_append(buf, " */");
   eina_strbuf_free(wbuf);
   return buf;
}
