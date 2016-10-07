#ifndef EOLIAN_GEN_SOURCES_H
#define EOLIAN_GEN_SOURCES_H

#include "main.h"

void eo_gen_source_gen(const Eolian_Class *cl, Eina_Strbuf *buf);
void eo_gen_impl_gen(const Eolian_Class *cl, Eina_Strbuf *buf);

#endif
