/**
 * Example of animation using Evas_VG.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o evas_vg_batman evas-vg-batman.c `pkg-config --libs --cflags evas ecore ecore-evas eina ector eo efl`
 * @endverbatim
 */

#define WIDTH 800
#define HEIGHT 600

#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif

#include <Eo.h>
#include <Efl.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>

#include <math.h>
#include <Eina.h>
#include "lottiemodel.h"

#include<iostream>
using namespace std;
#include"sgregion.h"
#include"sgmatrix.h"
#include"sgpath.h"
#include"ftraster_p.h"
#include"sgelapsedtimer.h"
#include"sgdebug.h"
#include "sgjson.h"
#include<iostream>
#include<fstream>
#include<sstream>


class VGDrawable {
public:
    VGDrawable(std::shared_ptr<LottieObject> model):mModelData(model), drawable(nullptr){}
    void update(Efl_VG *parent);
public:
    std::shared_ptr<LottieObject>    mModelData;
    std::vector<std::unique_ptr<VGDrawable>> mChildren;
    Efl_VG *drawable;
};

void VGDrawable::update(Efl_VG *parent)
{
    if (!drawable) {
        switch (mModelData->type()) {
        case LottieObject::Type::Composition:
        case LottieObject::Type::Layer:
        case LottieObject::Type::ShapeGroup:
        case LottieObject::Type::Transform:
        case LottieObject::Type::Repeater:
            drawable = evas_vg_container_add(parent);
            break;
        case LottieObject::Type::Ellipse:
        case LottieObject::Type::Rect:
        case LottieObject::Type::Shape:
            drawable = evas_vg_shape_add(parent);
            break;
        default:
            break;
        }
        for(auto &child : mChildren) {
            child->update(drawable);
        }
    }

    switch (mModelData->type()) {
    case LottieObject::Type::Composition:
    case LottieObject::Type::Layer:
    case LottieObject::Type::ShapeGroup:
    case LottieObject::Type::Transform:
    case LottieObject::Type::Repeater:
        drawable = evas_vg_container_add(parent);
        break;
    case LottieObject::Type::Ellipse:
    case LottieObject::Type::Rect:
    case LottieObject::Type::Shape:
        drawable = evas_vg_shape_add(parent);
        break;
    default:
        break;
    }
}


std::unique_ptr<VGDrawable>
createCustomTree(std::shared_ptr<LottieObject> model)
{
      std::unique_ptr<VGDrawable> obj(new VGDrawable(model));
      if (model->hasChildren()) {
          LottieGroupObject *group = static_cast<LottieGroupObject *>(model.get());
          for (auto child : group->mChildren) {
              obj->mChildren.push_back(createCustomTree(child));
          }
      }
      return obj;
}

static Evas_Object *background = NULL;
static Evas_Object *vg = NULL;

static void
_on_resize(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(background, w, h);
   evas_object_resize(vg, w, h);
}

int
main(void)
{
   Ecore_Evas *ee;
   Evas *e;
   Efl_VG *circle;
   Efl_VG *root;

   if (!ecore_evas_init())
     return -1;
   //setenv("ECORE_EVAS_ENGINE", "opengl_x11", 1);
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) return -1;

   ecore_evas_callback_resize_set(ee, _on_resize);
   ecore_evas_show(ee);

   e = ecore_evas_get(ee);
   background = evas_object_rectangle_add(e);
   evas_object_color_set(background, 70, 70, 70, 255);
   evas_object_show(background);

   vg = evas_object_vg_add(e);
   evas_object_show(vg);

   _on_resize(ee);

   root = evas_vg_container_add(NULL);

   evas_object_vg_root_node_set(vg, root);

   Eina_Matrix3 matrix;
   eina_matrix3_identity(&matrix);
   eina_matrix3_scale(&matrix, 1.1, 1.1);
   evas_vg_node_transformation_set(root, &matrix);


   circle = evas_vg_shape_add(root);
   evas_vg_shape_append_circle(circle, WIDTH / 2, HEIGHT / 2, 200);
   evas_vg_node_color_set(circle, 255, 255, 255, 255);
   evas_vg_shape_stroke_width_set(circle, 1);
   evas_vg_shape_stroke_color_set(circle, 255, 0, 0, 255);

   std::string filepath = DEMO_DIR;
   filepath += "LightBulb.json";
   initialize(GuaranteedLogger(), "/tmp/", "ssglog", 1);
   set_log_level(LogLevel::INFO);

   std::ifstream file;
   file.open(filepath);
   std::stringstream buffer;
   buffer << file.rdbuf();

   SGJson json(buffer.str().data());
   file.close();

   std::unique_ptr<VGDrawable> DrawableRoot = createCustomTree(json.mComposition);

   ecore_main_loop_begin();

   ecore_evas_shutdown();
   return 0;
}





