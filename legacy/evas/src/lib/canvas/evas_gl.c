/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
#include "evas_common.h"
#include "evas_private.h"
#include "Evas_GL.h"

struct _Evas_GL
{
   DATA32      magic;
   Evas       *evas;

   Eina_List  *contexts;
   Eina_List  *surfaces;
};

struct _Evas_GL_Context
{
   void    *data;
};

struct _Evas_GL_Surface
{
   void    *data;
};

EAPI Evas_GL *
evas_gl_new(Evas *e)
{
   Evas_GL *evas_gl;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   evas_gl = calloc(1, sizeof(Evas_GL));
   if (!evas_gl) return NULL;

   evas_gl->magic = MAGIC_EVAS_GL;
   evas_gl->evas = e;

   return evas_gl;
}

EAPI void
evas_gl_free(Evas_GL *evas_gl)
{
   MAGIC_CHECK(evas_gl, Evas_GL, MAGIC_EVAS_GL);
   return;
   MAGIC_CHECK_END();


   // Delete undeleted surfaces
   while (evas_gl->surfaces)
     evas_gl_surface_destroy(evas_gl, evas_gl->surfaces->data);

   // Delete undeleted contexts
   while (evas_gl->contexts)
     evas_gl_context_destroy(evas_gl, evas_gl->contexts->data);

   evas_gl->magic = 0;
   free(evas_gl);
}

EAPI Evas_GL_Surface *
evas_gl_surface_create(Evas_GL *evas_gl, Evas_GL_Config *config, int width, int height)
{
   Evas_GL_Surface *surf;

   MAGIC_CHECK(evas_gl, Evas_GL, MAGIC_EVAS_GL);
   return NULL;
   MAGIC_CHECK_END();

   if (!config)
     {
        ERR("Invalid Config\n");
        return NULL;
     }

   surf = calloc(1, sizeof(Evas_GL_Surface));

   surf->data = evas_gl->evas->engine.func->gl_surface_create(evas_gl->evas->engine.data.output, config, width, height);

   if (!surf->data)
     {
        ERR("Failed creating a surface from the engine\n");
        free(surf);
        return NULL;
     }

   // Keep track of the surface creations
   evas_gl->surfaces = eina_list_prepend(evas_gl->surfaces, surf);

   return surf;
}

EAPI void
evas_gl_surface_destroy(Evas_GL *evas_gl, Evas_GL_Surface *surf)
{
   // Magic
   MAGIC_CHECK(evas_gl, Evas_GL, MAGIC_EVAS_GL);
   return;
   MAGIC_CHECK_END();

   if (!surf)
     {
        ERR("Trying to destroy a NULL surface pointer!\n");
        return;
     }

   // Call Engine's Surface Destroy
   evas_gl->evas->engine.func->gl_surface_destroy(evas_gl->evas->engine.data.output, surf->data);

   // Remove it from the list
   evas_gl->surfaces = eina_list_remove(evas_gl->surfaces, surf);

   // Delete the object
   free(surf);
   surf = NULL;
}

EAPI Evas_GL_Context *
evas_gl_context_create(Evas_GL *evas_gl, Evas_GL_Context *share_ctx)
{
   Evas_GL_Context *ctx;

   // Magic
   MAGIC_CHECK(evas_gl, Evas_GL, MAGIC_EVAS_GL);
   return NULL;
   MAGIC_CHECK_END();

   // Allocate a context object
   ctx = calloc(1, sizeof(Evas_GL_Context));
   if (!ctx)
     {
        ERR("Unable to create a Evas_GL_Context object\n");
        return NULL;
     }

   // Call engine->gl_create_context
   if (share_ctx)
     {
        ctx->data = evas_gl->evas->engine.func->gl_context_create(evas_gl->evas->engine.data.output, share_ctx->data);
     }
   else
     {
        ctx->data = evas_gl->evas->engine.func->gl_context_create(evas_gl->evas->engine.data.output, NULL);
     }

   // Set a few variables
   if (!ctx->data)
     {
        ERR("Failed creating a context from the engine\n");
        free(ctx);
        return NULL;
     }

   // Keep track of the context creations
   evas_gl->contexts = eina_list_prepend(evas_gl->contexts, ctx);

   return ctx;

}

EAPI void
evas_gl_context_destroy(Evas_GL *evas_gl, Evas_GL_Context *ctx)
{

   MAGIC_CHECK(evas_gl, Evas_GL, MAGIC_EVAS_GL);
   return;
   MAGIC_CHECK_END();

   if (!ctx)
     {
        ERR("Trying to destroy a NULL context pointer!\n");
        return;
     }

   // Call Engine's destroy
   evas_gl->evas->engine.func->gl_context_destroy(evas_gl->evas->engine.data.output, ctx->data);

   // Remove it from the list
   evas_gl->contexts = eina_list_remove(evas_gl->contexts, ctx);

   // Delete the object
   free(ctx);
   ctx = NULL;
}

EAPI Eina_Bool
evas_gl_make_current(Evas_GL *evas_gl, Evas_GL_Surface *surf, Evas_GL_Context *ctx)
{
   Eina_Bool ret;

   MAGIC_CHECK(evas_gl, Evas_GL, MAGIC_EVAS_GL);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   if ((!surf) || (!ctx))
     ret = (Eina_Bool)evas_gl->evas->engine.func->gl_make_current(evas_gl->evas->engine.data.output, NULL, NULL);
   else
     ret = (Eina_Bool)evas_gl->evas->engine.func->gl_make_current(evas_gl->evas->engine.data.output, surf->data, ctx->data);

   return ret;
}

EAPI const char *
evas_gl_string_query(Evas_GL *evas_gl, int name)
{
   MAGIC_CHECK(evas_gl, Evas_GL, MAGIC_EVAS_GL);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   return (const char *)evas_gl->evas->engine.func->gl_string_query(evas_gl->evas->engine.data.output, name);
}

EAPI Evas_GL_Func
evas_gl_proc_address_get(Evas_GL *evas_gl, const char *name)
{
   MAGIC_CHECK(evas_gl, Evas_GL, MAGIC_EVAS_GL);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   return (Evas_GL_Func)evas_gl->evas->engine.func->gl_proc_address_get(evas_gl->evas->engine.data.output, name);
}

EAPI Eina_Bool
evas_gl_native_surface_get(Evas_GL *evas_gl, Evas_GL_Surface *surf, Evas_Native_Surface *ns)
{
   MAGIC_CHECK(evas_gl, Evas_GL, MAGIC_EVAS_GL);
   return EINA_FALSE;
   MAGIC_CHECK_END();

   return (Eina_Bool)evas_gl->evas->engine.func->gl_native_surface_get(evas_gl->evas->engine.data.output, surf->data, ns);
}


EAPI Evas_GL_API *
evas_gl_api_get(Evas_GL *evas_gl)
{
   MAGIC_CHECK(evas_gl, Evas_GL, MAGIC_EVAS_GL);
   return NULL;
   MAGIC_CHECK_END();

   return (Evas_GL_API*)evas_gl->evas->engine.func->gl_api_get(evas_gl->evas->engine.data.output);

}
