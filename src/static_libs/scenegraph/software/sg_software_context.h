#ifndef SG_SOFTWARE_CONTEXT_H
#define SG_SOFTWARE_CONTEXT_H

#include "sg_context.h"

typedef struct _SG_Software_Context                                  SG_Software_Context;
typedef struct _SG_Software_Render_Context                           SG_Software_Render_Context;

SG_Software_Context*           sg_software_context_create();


struct _SG_Software_Context
{
   SG_CONTEXT_CLASS
};

struct _SG_Software_Render_Context
{
   SG_RENDER_CONTEXT_CLASS
};
#endif //SG_SOFTWARE_CONTEXT_H