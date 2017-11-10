#ifndef SG_CONTEXT_H
#define SG_CONTEXT_H

typedef struct _SG_Context                                  SG_Context;
typedef struct _SG_Render_Context                           SG_Render_Context;

typedef struct _SG_Context_Func                             SG_Context_Func;
typedef struct _SG_Render_Context_Func                      SG_Render_Context_Func;

// forward declaration
typedef struct _SG_Rectangle_Node                           SG_Rectangle_Node;
typedef struct _SG_Shape_Node                               SG_Shape_Node;
typedef struct _SG_Image_Node                               SG_Image_Node;
typedef struct _SG_Glyph_Node                               SG_Glyph_Node;
typedef struct _SG_Renderer                                 SG_Renderer;
// forward declaration end

// for subclassing
void             sg_context_ctr(SG_Context *obj);
void             sg_context_destroy(SG_Context *obj);

void             sg_render_context_ctr(SG_Render_Context *obj, SG_Context *context);
void             sg_render_context_destroy(SG_Render_Context *obj);

struct _SG_Context_Func
{
   void                        (*dtr)(SG_Context *obj); /* virtual */
   SG_Render_Context          *(*render_context_create)(SG_Context *obj);/* pure virtual */
   void                       *(*render_context_initialize)(SG_Context *obj, SG_Render_Context *render_context);
   void                       *(*render_context_invalidate)(SG_Context *obj, SG_Render_Context *render_context);

   // internal drawable nodes
   SG_Rectangle_Node          *(*rectangle_node_create)(SG_Context *obj);/* pure virtual */
   SG_Shape_Node              *(*shape_node_create)(SG_Context *obj);/* pure virtual */
   SG_Image_Node              *(*image_node_create)(SG_Context *obj);/* pure virtual */
   SG_Glyph_Node              *(*glyph_node_create)(SG_Context *obj);/* pure virtual */

   void                        (*backend_set)(SG_Context *obj, const char *backend);
   const char                 *(*backend_get)(const SG_Context *obj);
   SG_Context_Func            *base;
};

struct _SG_Render_Context_Func
{
   void                        (*dtr)(SG_Render_Context *obj); /* virtual */
   SG_Context                 *(*scenegraph_context_get)(SG_Render_Context *obj);
   SG_Renderer                *(*renderer_create)(SG_Render_Context *obj); /* pure virtual */
   void                       *(*render_frame)(SG_Render_Context *obj, SG_Renderer *renderer); /* pure virtual */
   SG_Render_Context_Func     *base;
};

#define SG_CONTEXT_CLASS                 \
   SG_Context_Func         *fn;

#define SG_RENDER_CONTEXT_CLASS           \
   SG_Render_Context_Func         *fn;    \
   SG_Context                     *m_sg;

struct _SG_Context
{
   SG_CONTEXT_CLASS
};

struct _SG_Render_Context
{
   SG_RENDER_CONTEXT_CLASS
};

#endif // SG_CONTEXT_H