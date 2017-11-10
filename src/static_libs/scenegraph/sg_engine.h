#ifndef SG_ENGINE_H
#define SG_ENGINE_H

#include "sg_renderer.h"

typedef struct _SG_Engine_Func        SG_Engine_Func;
typedef struct _SG_Engine             SG_Engine;


struct _SG_Engine_Func
{
     SG_Renderer *create_renderer(const SG_Engine *obj);
     SG_Context         *sg_context;
     SG_Render_Contex   *sg_render_context;
};

#endif // SG_ENGINE_H
