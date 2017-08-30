#ifndef EOLIAN_GEN_HEADERS_H
#define EOLIAN_GEN_HEADERS_H

#include "main.h"

void eo_gen_params(Eina_Iterator *itr, Eina_Strbuf *buf, Eina_Strbuf **flagbuf, int *nidx, Eolian_Function_Type ftype);
void eo_gen_header_gen(const Eolian_Unit *src, const Eolian_Class *cl, Eina_Strbuf *buf, Eina_Bool legacy);

#endif
