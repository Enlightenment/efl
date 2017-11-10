#include "sg_software_context.h"
#include <Eina.h>


/* SG_Software_Render_Context class Start */

static SG_Render_Context_Func SG_SOFTWARE_RENDER_CONTEXT_VTABLE = {NULL};

inline static void
renderer_context_dtr(SG_Render_Context *obj EINA_UNUSED)
{

}

inline static void
render_context_ctr(SG_Software_Render_Context *obj, SG_Context *context)
{
   sg_render_context_ctr((SG_Render_Context *)obj, context);
   if (!SG_SOFTWARE_RENDER_CONTEXT_VTABLE.dtr)
     {
        // copy the base table
        SG_SOFTWARE_RENDER_CONTEXT_VTABLE = *obj->fn;
        SG_SOFTWARE_RENDER_CONTEXT_VTABLE.base = obj->fn;
        // update the override functions
        SG_SOFTWARE_RENDER_CONTEXT_VTABLE.dtr = renderer_context_dtr;
     }
}

static SG_Software_Render_Context *
sg_software_render_context_create(SG_Context *context)
{
   SG_Software_Render_Context* obj = (SG_Software_Render_Context *) calloc(1, sizeof(SG_Software_Render_Context));
   render_context_ctr(obj, context);
   return obj;
}
/* SG_Software_Render_Context class End */


/* SG_Software_Context class Start */

static SG_Context_Func SG_SOFTWARE_CONTEXT_VTABLE = {NULL};


inline static SG_Render_Context *
render_context_create(SG_Context *obj)
{
   return (SG_Render_Context *)sg_software_render_context_create(obj);
}

inline static void
sg_software_context_dtr(SG_Context *obj EINA_UNUSED)
{

}

void sg_software_context_ctr(SG_Software_Context *obj)
{
   sg_context_ctr((SG_Context *)obj);
   if (!SG_SOFTWARE_CONTEXT_VTABLE.dtr)
     {
        // copy the base table
        SG_SOFTWARE_CONTEXT_VTABLE = *obj->fn;
        SG_SOFTWARE_CONTEXT_VTABLE.base = obj->fn;
        // update the override functions
        SG_SOFTWARE_CONTEXT_VTABLE.dtr = sg_software_context_dtr;
        SG_SOFTWARE_CONTEXT_VTABLE.render_context_create = render_context_create;
     }
}

SG_Software_Context *
sg_software_context_create(void)
{
   SG_Software_Context* obj = (SG_Software_Context *) calloc(1, sizeof(SG_Software_Context));
   sg_software_context_ctr(obj);
   return obj;
}
/* SG_Context class End */
