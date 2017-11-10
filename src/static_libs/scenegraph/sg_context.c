#include "sg_context.h"
#include <Eina.h>

/* SG_Context class Start */

static SG_Context_Func SG_CONTEXT_VTABLE = {NULL};

inline static void
sg_context_dtr(SG_Context *obj EINA_UNUSED)
{

}

void sg_context_ctr(SG_Context *obj)
{
   obj->fn = &SG_CONTEXT_VTABLE;
   obj->fn->dtr = sg_context_dtr;
}

void sg_context_destroy(SG_Context *obj)
{
   if (obj)
     {
        obj->fn->dtr(obj);
        free(obj);
     }

}

/* SG_Context class End */

/* SG_Render_Context class Start */

static SG_Render_Context_Func SG_RENDER_CONTEXT_VTABLE = {0};

inline static void
sg_render_context_dtr(SG_Render_Context *obj EINA_UNUSED)
{

}

void sg_render_context_ctr(SG_Render_Context *obj, SG_Context *context)
{
   obj->fn = &SG_RENDER_CONTEXT_VTABLE;
   obj->fn->dtr = sg_render_context_dtr;
   // update data
   obj->m_sg = context;
}

void sg_render_context_destroy(SG_Render_Context *obj)
{
   if (obj)
     {
        obj->fn->dtr(obj);
        free(obj);
     }

}

/* SG_Context class End */
