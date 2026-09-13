// Vector object rendering tests.
//
// These run on every engine that can be brought up, because the interesting
// failures have been engine-specific: the GL engine rasterises vector shapes
// through a span-buffer path that the software engine does not use, so a bug
// in it is invisible to a buffer-engine test.  Engines that cannot be created
// here (no display, not built) are skipped rather than failed.

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define EFL_BETA_API_SUPPORT 1
#define EFL_EO_API_SUPPORT 1

#include <stdio.h>

#include <Evas.h>
#include <Ecore_Evas.h>

#include "evas_suite.h"

#define WIN_W 256
#define WIN_H 256

// Engines to attempt.  "buffer" is always available and headless; the rest
// are tried and skipped when they cannot be created.
static const char *_engines[] = { "buffer", "opengl_x11", NULL };

static Eo *
_vg_object_add(Evas *e, int x, int y, int w, int h, int alpha)
{
   Eo *vg, *shape, *root;

   vg = efl_add(EFL_CANVAS_VG_OBJECT_CLASS, e);
   efl_gfx_entity_size_set(vg, EINA_SIZE2D(w, h));
   efl_gfx_entity_position_set(vg, EINA_POSITION2D(x, y));
   efl_gfx_entity_visible_set(vg, EINA_TRUE);

   if (alpha < 255)
     {
        // A container whose colour has alpha below 255 asks for group
        // opacity, which is a different rendering path from a shape that is
        // merely translucent.
        root = efl_add(EFL_CANVAS_VG_CONTAINER_CLASS, vg);
        efl_gfx_color_set(root, alpha, alpha, alpha, alpha);
        shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, root);
     }
   else
     {
        shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, vg);
        root = shape;
     }

   // Opaque white, so that whatever alpha survives to the framebuffer is
   // readable straight off one channel.
   efl_gfx_path_append_rect(shape, 0, 0, w, h, 0, 0);
   efl_gfx_color_set(shape, 255, 255, 255, 255);

   efl_canvas_vg_object_root_node_set(vg, root);
   return vg;
}

// Render the scene @p build makes and return the pixel at (@p px, @p py),
// or EINA_FALSE when this engine is unavailable.
static Eina_Bool
_scene_sample(const char *engine, void (*build)(Evas *e),
              int frames, int px, int py, unsigned int *out)
{
   Ecore_Evas *ee;
   Evas *e;
   Evas_Object *snap, *bg;
   unsigned int *pixels;
   int i, sw = 0;

   ee = ecore_evas_new(engine, 0, 0, WIN_W, WIN_H, NULL);
   if (!ee)
     {
        printf("Skipping: cannot create ecore_evas for '%s'\n", engine);
        return EINA_FALSE;
     }
   ecore_evas_show(ee);
   ecore_evas_manual_render_set(ee, EINA_TRUE);
   e = ecore_evas_get(ee);

   // Opaque black behind, so a sampled pixel reports coverage directly.
   bg = evas_object_rectangle_add(e);
   evas_object_color_set(bg, 0, 0, 0, 255);
   evas_object_geometry_set(bg, 0, 0, WIN_W, WIN_H);
   evas_object_show(bg);

   build(e);

   // Several frames: the first one populates caches, and failures in the
   // vector paths have tended to need a second pass to show up.
   for (i = 0; i < frames; i++)
     ecore_evas_manual_render(ee);

   // A snapshot reads back uniformly whether the engine renders to memory
   // or to a window.
   snap = evas_object_image_filled_add(e);
   evas_object_image_snapshot_set(snap, EINA_TRUE);
   evas_object_geometry_set(snap, 0, 0, WIN_W, WIN_H);
   evas_object_show(snap);
   ecore_evas_manual_render(ee);

   evas_object_image_size_get(snap, &sw, NULL);
   pixels = evas_object_image_data_get(snap, EINA_FALSE);
   if (pixels && sw > 0) *out = pixels[py * sw + px];
   else                  *out = 0;
   if (pixels) evas_object_image_data_set(snap, pixels);

   ecore_evas_free(ee);
   return (pixels != NULL) && (sw > 0);
}

// A narrow object followed by a much wider one, the wider one asking for
// group opacity.
//
// Both halves matter.  Every vector object on a canvas shares one ector
// surface, so the narrow object is what fixes that surface's stride, and the
// wide one then renders through it.  The group-opacity path additionally
// swaps the surface's pixel buffer out and back while it renders, which is
// what turned a mis-sized buffer into a use-after-free on the GL engine.
static void
_build_narrow_then_wide(Evas *e)
{
   _vg_object_add(e, 0, 0, 16, 16, 255);
   _vg_object_add(e, 32, 32, 192, 192, 128);
}

EFL_START_TEST(evas_vg_mixed_sizes_group_opacity)
{
   const char **eng;

   for (eng = _engines; *eng; eng++)
     {
        unsigned int px = 0;

        if (!_scene_sample(*eng, _build_narrow_then_wide, 4, 128, 128, &px))
          continue;

        // Surviving this far is most of the point: before the buffer was
        // sized and owned correctly, the GL engine wrote about a megabyte
        // past a 57 KB allocation here and then crashed on a later frame.
        printf("engine %s: centre pixel %08x\n", *eng, px);
     }
}
EFL_END_TEST

// Group opacity has to reach the framebuffer.
//
// An opaque white shape inside a container with alpha 128, over black, must
// land near 128 on every channel.  The GL engine used to composite the group
// into a buffer it never read back, so the group arrived fully opaque while
// the software engine had it right - a visible difference between engines
// for the same scene.
EFL_START_TEST(evas_vg_container_alpha_is_applied)
{
   const char **eng;

   for (eng = _engines; *eng; eng++)
     {
        unsigned int px = 0;
        int r;

        if (!_scene_sample(*eng, _build_narrow_then_wide, 4, 128, 128, &px))
          continue;

        r = (px >> 16) & 0xff;
        // Wide tolerance: this is checking that the alpha was applied at
        // all, not the exact rounding of the blend.
        ck_assert_msg(r > 100 && r < 160,
                      "engine %s: group alpha 128 over black should give a "
                      "channel near 128, got %d (pixel %08x)",
                      *eng, r, px);
     }
}
EFL_END_TEST

// A surface cache too small for even one vector surface must still hand back
// something usable.  The budget is advisory - the entry just stored is at the
// head of the LRU and, when nothing else is cached, is also its tail, so a
// trim that walks from the tail can otherwise free the surface the caller is
// about to draw with.  That is invisible at any sane budget, which is why it
// is provoked here rather than left to chance.
EFL_START_TEST(evas_vg_tiny_surface_cache)
{
   const char **eng;
   char prev[64] = "";
   const char *old_env = getenv("EVAS_SURFACE_CACHE_SIZE");

   if (old_env) snprintf(prev, sizeof(prev), "%s", old_env);
   setenv("EVAS_SURFACE_CACHE_SIZE", "16", 1);   // 16 KB: smaller than one surface

   for (eng = _engines; *eng; eng++)
     {
        unsigned int px = 0;
        _scene_sample(*eng, _build_narrow_then_wide, 4, 128, 128, &px);
     }

   if (prev[0]) setenv("EVAS_SURFACE_CACHE_SIZE", prev, 1);
   else         unsetenv("EVAS_SURFACE_CACHE_SIZE");
}
EFL_END_TEST

void evas_test_vg(TCase *tc)
{
   tcase_add_test(tc, evas_vg_mixed_sizes_group_opacity);
   tcase_add_test(tc, evas_vg_container_alpha_is_applied);
   tcase_add_test(tc, evas_vg_tiny_surface_cache);
}
