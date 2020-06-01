#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "vg_common.h"
#include <Evas.h>

#ifdef BUILD_VG_LOADER_JSON

#include <rlottie_capi.h>

static char*
_get_key_val(void *key)
{
   static char buf[30];
   snprintf(buf, sizeof(buf), "%zu", (size_t) key);
   return buf;
}

static void
_construct_drawable_nodes(Efl_Canvas_Vg_Container *parent, const LOTLayerNode *layer, int depth EINA_UNUSED)
{
   if (!parent) return;

   //This list is used for layer order verification
   Eina_List *list = (Eina_List*) efl_canvas_vg_container_children_direct_get(parent);

   for (unsigned int i = 0; i < layer->mNodeList.size; i++)
     {
        if (i > 0) list = eina_list_next(list);

        LOTNode *node = layer->mNodeList.ptr[i];
        if (!node) continue;

        //Image object
        if (node->mImageInfo.data)
          {
             char *key = _get_key_val(node);
             Efl_Canvas_Vg_Image *image = efl_key_data_get(parent, key);
             if (!image)
               {
                  image = efl_add(EFL_CANVAS_VG_IMAGE_CLASS, parent);
                  efl_key_data_set(parent, key, image);
               }
#if DEBUG
        for (int i = 0; i < depth; i++) printf("    ");
        printf("%s (%p)\n", efl_class_name_get(efl_class_get(image)), image);
#endif
             Eina_Matrix3 m;
             eina_matrix3_identity(&m);
             eina_matrix3_values_set( &m,
                                      node->mImageInfo.mMatrix.m11,  node->mImageInfo.mMatrix.m12, node->mImageInfo.mMatrix.m13,
                                      node->mImageInfo.mMatrix.m21,  node->mImageInfo.mMatrix.m22, node->mImageInfo.mMatrix.m23,
                                      node->mImageInfo.mMatrix.m31,  node->mImageInfo.mMatrix.m32, node->mImageInfo.mMatrix.m33);
             efl_canvas_vg_node_transformation_set(image, &m);
             efl_canvas_vg_image_data_set(image, node->mImageInfo.data, EINA_SIZE2D(node->mImageInfo.width, node->mImageInfo.height));
             efl_gfx_color_set(image, node->mImageInfo.mAlpha, node->mImageInfo.mAlpha, node->mImageInfo.mAlpha, node->mImageInfo.mAlpha);
             efl_gfx_entity_visible_set(image, EINA_TRUE);

             continue;
          }

        char *key = _get_key_val(node);
        Efl_Canvas_Vg_Shape *shape = efl_key_data_get(parent, key);
        if (!shape)
          {
             shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, parent);
             efl_key_data_set(parent, key, shape);
          }
        else
          {
             efl_gfx_path_reset(shape);

             //Layer order is mismatched!
             if (eina_list_data_get(list) != shape)
               efl_gfx_stack_raise_to_top(shape);
          }

        //Skip Invisible Stroke?
        if (node->mStroke.enable && node->mStroke.width == 0)
          {
             efl_gfx_entity_visible_set(shape, EINA_FALSE);
             continue;
          }

        const float *data = node->mPath.ptPtr;
        if (!data) continue;

        if (node->keypath) efl_key_data_set(shape, "_lot_node_name", node->keypath);
        efl_gfx_entity_visible_set(shape, EINA_TRUE);
#if DEBUG
        for (int i = 0; i < depth; i++) printf("    ");
        printf("%s (%p) keypath : %s\n", efl_class_name_get(efl_class_get(shape)), shape, node->keypath);
#endif
        //0: Path
        efl_gfx_path_reserve(shape, node->mPath.elmCount, node->mPath.ptCount);

        for (size_t i = 0; i < node->mPath.elmCount; i++)
          {
             switch (node->mPath.elmPtr[i])
               {
                case 0:
                   efl_gfx_path_append_move_to(shape, data[0], data[1]);
                   data += 2;
                   break;
                case 1:
                   efl_gfx_path_append_line_to(shape, data[0], data[1]);
                   data += 2;
                   break;
                case 2:
                   efl_gfx_path_append_cubic_to(shape, data[0], data[1], data[2], data[3], data[4], data[5]);
                   data += 6;
                   break;
                case 3:
                   efl_gfx_path_append_close(shape);
                   break;
                default:
                   ERR("No reserved path type = %d", node->mPath.elmPtr[i]);
               }
          }

        //1: Stroke
        if (node->mStroke.enable)
          {
             //Stroke Width
             efl_gfx_shape_stroke_width_set(shape, node->mStroke.width);

             //Stroke Cap
             Efl_Gfx_Cap cap;
             switch (node->mStroke.cap)
               {
                case CapFlat: cap = EFL_GFX_CAP_BUTT; break;
                case CapSquare: cap = EFL_GFX_CAP_SQUARE; break;
                case CapRound: cap = EFL_GFX_CAP_ROUND; break;
                default: cap = EFL_GFX_CAP_BUTT; break;
               }
             efl_gfx_shape_stroke_cap_set(shape, cap);

             //Stroke Join
             Efl_Gfx_Join join;
             switch (node->mStroke.join)
               {
                case JoinMiter: join = EFL_GFX_JOIN_MITER; break;
                case JoinBevel: join = EFL_GFX_JOIN_BEVEL; break;
                case JoinRound: join = EFL_GFX_JOIN_ROUND; break;
                default: join = EFL_GFX_JOIN_MITER; break;
               }
             efl_gfx_shape_stroke_join_set(shape, join);

             efl_gfx_shape_stroke_miterlimit_set(shape, node->mStroke.miterLimit);

             //Stroke Dash
             if (node->mStroke.dashArraySize > 0)
               {
                  int size = (node->mStroke.dashArraySize / 2);
                  Efl_Gfx_Dash *dash = malloc(sizeof(Efl_Gfx_Dash) * size);
                  if (dash)
                    {
                       for (int i = 0; i <= size; i+=2)
                         {
                            dash[i].length = node->mStroke.dashArray[i];
                            dash[i].gap = node->mStroke.dashArray[i + 1];
                         }
                       efl_gfx_shape_stroke_dash_set(shape, dash, size);
                       free(dash);
                    }
               }
          }

        //2: Fill Method
        switch (node->mBrushType)
          {
           case BrushSolid:
             {
                float pa = ((float)node->mColor.a) / 255;
                int r = (int)(((float) node->mColor.r) * pa);
                int g = (int)(((float) node->mColor.g) * pa);
                int b = (int)(((float) node->mColor.b) * pa);
                int a = node->mColor.a;

                if (node->mStroke.enable)
                  efl_gfx_shape_stroke_color_set(shape, r, g, b, a);
                else
                  efl_gfx_color_set(shape, r, g, b, a);
             }
             break;
           case BrushGradient:
             {
                Efl_Canvas_Vg_Gradient* grad = NULL;

                if (node->mGradient.type == GradientLinear)
                  {
                     char *key = _get_key_val(shape);
                     grad = efl_key_data_get(shape, key);
                     if (!grad)
                       {
                          grad = efl_add(EFL_CANVAS_VG_GRADIENT_LINEAR_CLASS, parent);
                          efl_key_data_set(shape, key, grad);
                       }
                     efl_gfx_entity_visible_set(grad, EINA_TRUE);
                     efl_gfx_gradient_linear_start_set(grad, node->mGradient.start.x, node->mGradient.start.y);
                     efl_gfx_gradient_linear_end_set(grad, node->mGradient.end.x, node->mGradient.end.y);
                  }
                else if (node->mGradient.type == GradientRadial)
                  {
                     char *key = _get_key_val(shape);
                     grad = efl_key_data_get(shape, key);
                     if (!grad)
                       {
                          grad = efl_add(EFL_CANVAS_VG_GRADIENT_RADIAL_CLASS, parent);
                          efl_key_data_set(shape, key, grad);
                       }
                     efl_gfx_entity_visible_set(grad, EINA_TRUE);
                     efl_gfx_gradient_radial_center_set(grad, node->mGradient.center.x, node->mGradient.center.y);
                     efl_gfx_gradient_radial_focal_set(grad, node->mGradient.focal.x, node->mGradient.focal.y);
                     efl_gfx_gradient_radial_radius_set(grad, node->mGradient.cradius);
                  }
                else
                  ERR("No reserved gradient type = %d", node->mGradient.type);

                if (grad)
                  {
                     //Gradient Stop
                     Efl_Gfx_Gradient_Stop* stops = malloc(sizeof(Efl_Gfx_Gradient_Stop) * node->mGradient.stopCount);
                     if (stops)
                       {
                          for (unsigned int i = 0; i < node->mGradient.stopCount; i++)
                            {
                               stops[i].offset = node->mGradient.stopPtr[i].pos;
                               float pa = ((float)node->mGradient.stopPtr[i].a) / 255;
                               stops[i].r = (int)(((float)node->mGradient.stopPtr[i].r) * pa);
                               stops[i].g = (int)(((float)node->mGradient.stopPtr[i].g) * pa);
                               stops[i].b = (int)(((float)node->mGradient.stopPtr[i].b) * pa);
                               stops[i].a = node->mGradient.stopPtr[i].a;
                            }
                          efl_gfx_gradient_stop_set(grad, stops, node->mGradient.stopCount);
                          free(stops);
                       }
                     if (node->mStroke.enable)
                       efl_canvas_vg_shape_stroke_fill_set(shape, grad);
                     else
                       efl_canvas_vg_shape_fill_set(shape, grad);
                  }
             }
             break;
           default:
              ERR("No reserved brush type = %d", node->mBrushType);
          }

        //3: Fill Rule
        if (node->mFillRule == FillEvenOdd)
          efl_gfx_shape_fill_rule_set(shape, EFL_GFX_FILL_RULE_ODD_EVEN);
        else if (node->mFillRule == FillWinding)
          efl_gfx_shape_fill_rule_set(shape, EFL_GFX_FILL_RULE_WINDING);
     }
}

static void
_construct_mask_nodes(Efl_Canvas_Vg_Container *parent, LOTMask *mask, int depth EINA_UNUSED)
{
   const float *data = mask->mPath.ptPtr;
   if (!data) return;

   char *key = _get_key_val(mask);
   Efl_Canvas_Vg_Shape *shape = efl_key_data_get(parent, key);
   if (!shape)
     {
        shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, parent);
        efl_key_data_set(parent, key, shape);
     }
   else
     efl_gfx_path_reset(shape);

#if DEBUG
        for (int i = 0; i < depth; i++) printf("    ");
        printf("%s (%p)\n", efl_class_name_get(efl_class_get(shape)), shape);
#endif

   efl_gfx_entity_visible_set(shape, EINA_TRUE);
   efl_gfx_path_reserve(shape, mask->mPath.elmCount, mask->mPath.ptCount);

   for (size_t i = 0; i < mask->mPath.elmCount; i++)
     {
        switch (mask->mPath.elmPtr[i])
          {
           case 0:
              efl_gfx_path_append_move_to(shape, data[0], data[1]);
              data += 2;
              break;
           case 1:
              efl_gfx_path_append_line_to(shape, data[0], data[1]);
              data += 2;
              break;
           case 2:
              efl_gfx_path_append_cubic_to(shape, data[0], data[1], data[2], data[3], data[4], data[5]);
              data += 6;
              break;
           case 3:
              efl_gfx_path_append_close(shape);
              break;
           default:
              ERR("No reserved path type = %d", mask->mPath.elmPtr[i]);
              break;
          }
     }
   //White color and alpha setting
   float pa = ((float)mask->mAlpha) / 255;
   int r = (int) (255.0f * pa);
   int g = (int) (255.0f * pa);
   int b = (int) (255.0f * pa);
   int a = mask->mAlpha;
   efl_gfx_color_set(shape, r, g, b, a);
}

static Efl_Canvas_Vg_Container*
_construct_masks(Efl_Canvas_Vg_Container *mtarget, LOTMask *masks, unsigned int mask_cnt, int depth)
{
   char *key = NULL;

   Efl_Canvas_Vg_Container *msource = NULL;

   key = _get_key_val(mtarget);
   msource = efl_key_data_get(mtarget, key);
   if (!msource)
     {
        msource = efl_add(EFL_CANVAS_VG_CONTAINER_CLASS, mtarget);
        efl_key_data_set(mtarget, key, msource);
     }
   efl_gfx_entity_visible_set(msource, EINA_TRUE);

#if DEBUG
        for (int i = 0; i < depth; i++) printf("    ");
        printf("%s (%p), base mask => %p\n", efl_class_name_get(efl_class_get(msource)), msource, mtarget);
        depth++;
#endif

   //FIXME : EFL_GFX_VG_COMPOSITE_METHOD_ALPHA option is temporary
   //Currently matte alpha implements is same the mask intersect implement.
   //It has been implemented as a multiplication calculation.
   efl_canvas_vg_node_comp_method_set(mtarget, msource, EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA);

   mtarget = msource;

   //Make mask layers
   for (unsigned int i = 0; i < mask_cnt; i++)
     {
        LOTMask *mask = &masks[i];;
        key = _get_key_val(mask);
        msource = efl_key_data_get(mtarget, key);

        if (!msource)
          {
             msource = efl_add(EFL_CANVAS_VG_CONTAINER_CLASS, mtarget);
             efl_key_data_set(mtarget, key, msource);
          }
        efl_gfx_entity_visible_set(msource, EINA_TRUE);

#if DEBUG
        for (int i = 0; i < depth; i++) printf("    ");
        printf("%s (%p), real mask:%d => %p\n", efl_class_name_get(efl_class_get(msource)), msource, mask->mMode, mtarget);
#endif

        _construct_mask_nodes(msource, mask, depth + 1);

        Efl_Gfx_Vg_Composite_Method mask_mode;
        switch (mask->mMode)
          {
           case MaskSubstract:
              mask_mode = EFL_GFX_VG_COMPOSITE_METHOD_MASK_SUBSTRACT;
              break;
           case MaskIntersect:
              mask_mode = EFL_GFX_VG_COMPOSITE_METHOD_MASK_INTERSECT;
              break;
           case MaskDifference:
              mask_mode = EFL_GFX_VG_COMPOSITE_METHOD_MASK_DIFFERENCE;
              break;
           case MaskAdd:
           default:
              mask_mode = EFL_GFX_VG_COMPOSITE_METHOD_MASK_ADD;
              break;
          }
        efl_canvas_vg_node_comp_method_set(mtarget, msource, mask_mode);
        mtarget = msource;
     }
   return mtarget;
}

static void
_reset_vg_tree(Efl_VG *node)
{
   //Hide all nodes visibility
   if (efl_isa(node, EFL_CANVAS_VG_CONTAINER_CLASS))
     {
        Eina_List *list = (Eina_List*) efl_canvas_vg_container_children_direct_get(node);
        Eina_List *list2;
        Efl_VG* child;
        EINA_LIST_FOREACH(list, list2, child)
          _reset_vg_tree(child);
     }
   efl_gfx_entity_visible_set(node, EINA_FALSE);
}

static void
_update_vg_tree(Efl_Canvas_Vg_Container *root, const LOTLayerNode *layer, int depth EINA_UNUSED)
{
   if (!layer->mVisible) return;

   efl_gfx_entity_visible_set(root, EINA_TRUE);
   efl_gfx_color_set(root, layer->mAlpha, layer->mAlpha, layer->mAlpha, layer->mAlpha);

   //Don't need to update it anymore since its layer is invisible.
   if (layer->mAlpha == 0) return;

   Efl_Canvas_Vg_Container *ptree = NULL;

   //Note: We assume that if matte is valid, next layer must be a matte source.
   int matte_mode = 0;
   Efl_Canvas_Vg_Container *mtarget = NULL;
   LOTLayerNode *mlayer = NULL;

   //Is this layer a container layer?
   for (unsigned int i = 0; i < layer->mLayerList.size; i++)
     {
        LOTLayerNode *clayer = layer->mLayerList.ptr[i];

        //Source Layer
        char *key = _get_key_val(clayer);
        Efl_Canvas_Vg_Container *ctree = efl_key_data_get(root, key);
        if (!ctree)
          {
             ctree = efl_add(EFL_CANVAS_VG_CONTAINER_CLASS, root);
             efl_key_data_set(root, key, ctree);
             if (clayer->keypath) efl_key_data_set(ctree, "_lot_node_name", clayer->keypath);
          }
#if DEBUG
        for (int i = 0; i < depth; i++) printf("    ");
        printf("%s (%p) matte:%d => %p %s\n", efl_class_name_get(efl_class_get(ctree)), ctree, matte_mode, ptree, clayer->keypath);
#endif
        _update_vg_tree(ctree, clayer, depth+1);

        if (matte_mode != 0)
          {
             efl_canvas_vg_node_comp_method_set(ptree, ctree, matte_mode);
             mtarget = ctree;
          }
        matte_mode = (int) clayer->mMatte;

        if (clayer->mMaskList.size > 0)
          {
             mlayer = clayer;
             if (!mtarget) mtarget = ctree;
          }
        else
           mtarget = NULL;

        ptree = ctree;

        //Remap Matte Mode
        switch (matte_mode)
          {
           case MatteNone:
              matte_mode = 0;
              break;
           case MatteAlpha:
              matte_mode = EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA;
              break;
           case MatteAlphaInv:
              matte_mode = EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA_INVERSE;
              break;
           case MatteLuma:
              matte_mode = 0;
              ERR("TODO: MatteLuma");
              break;
           case MatteLumaInv:
              matte_mode = 0;
              ERR("TODO: MatteLumaInv");
              break;
           default:
              matte_mode = 0;
              break;
          }

        //Construct node that have mask.
        if (mlayer && mtarget)
          ptree = _construct_masks(mtarget, mlayer->mMaskList.ptr, mlayer->mMaskList.size, depth + 1);
     }

   //Construct drawable nodes.
   if (layer->mNodeList.size > 0)
     _construct_drawable_nodes(root, layer, depth);

}
#endif

#ifdef BUILD_VG_LOADER_JSON
void
_value_provider_override(Vg_File_Data *vfd)
{
   Lottie_Animation *lot_anim = (Lottie_Animation *) vfd->loader_data;

   Eina_List *l;
   Efl_Gfx_Vg_Value_Provider *vp;
   EINA_LIST_FOREACH(vfd->vp_list, l, vp)
     {
        const char *keypath;
        Efl_Gfx_Vg_Value_Provider_Flags flag;
        flag = efl_gfx_vg_value_provider_updated_get(vp);

        if (flag & EFL_GFX_VG_VALUE_PROVIDER_FLAGS_FILL_COLOR)
          {
             int r, g, b, a;
             r = g = b = a = 0;
             efl_gfx_vg_value_provider_fill_color_get(vp, &r, &g, &b, &a);
             keypath = efl_gfx_vg_value_provider_keypath_get(vp);

             lottie_animation_property_override(lot_anim, LOTTIE_ANIMATION_PROPERTY_FILLCOLOR, (char*)keypath, r / 255.0, g / 255.0, b / 255.0);
             lottie_animation_property_override(lot_anim, LOTTIE_ANIMATION_PROPERTY_FILLOPACITY, (char*)keypath, (a / 255.0) * 100.0);
          }
        if (flag & EFL_GFX_VG_VALUE_PROVIDER_FLAGS_STROKE_COLOR)
          {
             int r, g, b, a;
             r = g = b = a = 0;
             efl_gfx_vg_value_provider_stroke_color_get(vp, &r, &g, &b, &a);
             keypath = efl_gfx_vg_value_provider_keypath_get(vp);

             lottie_animation_property_override(lot_anim, LOTTIE_ANIMATION_PROPERTY_STROKECOLOR, (char*)keypath, r / 255.0, g / 255.0, b / 255.0);
             lottie_animation_property_override(lot_anim, LOTTIE_ANIMATION_PROPERTY_STROKEOPACITY, (char*)keypath, (a / 255.0) * 100.0);
          }
        if (flag & EFL_GFX_VG_VALUE_PROVIDER_FLAGS_STROKE_WIDTH)
          {
             double w;
             w = efl_gfx_vg_value_provider_stroke_width_get(vp);
             keypath = efl_gfx_vg_value_provider_keypath_get(vp);

             lottie_animation_property_override(lot_anim, LOTTIE_ANIMATION_PROPERTY_STROKEWIDTH, (char*)keypath, w);
          }
        if (flag & EFL_GFX_VG_VALUE_PROVIDER_FLAGS_TRANSFORM_MATRIX)
          {
             Eina_Matrix4 m, *orig_m;
             double tx, ty, sx, sy, radian_z, si, cs;

             orig_m = efl_gfx_vg_value_provider_transform_get(vp);
             if (!orig_m) continue;

             /*
              * NOTE: We need to impelements 3-axis transform.
              * now lottie animation provide z projection transform.
              * In this cace, we calcuate to T * R * S order.
              */
             eina_matrix4_copy(&m, orig_m);
             keypath = efl_gfx_vg_value_provider_keypath_get(vp);

             // Calc Translate
             eina_matrix4_values_get(&m, NULL, NULL, NULL, &tx,
                                         NULL, NULL, NULL, &ty,
                                         NULL, NULL, NULL, NULL,
                                         NULL, NULL, NULL, NULL);
             eina_matrix4_translate(&m, -1 * tx, -1 * ty, 0);

             // Calc Rotate
             eina_matrix4_values_get(&m,  &cs, NULL, NULL, NULL,
                                          &si, NULL, NULL, NULL,
                                         NULL, NULL, NULL, NULL,
                                         NULL, NULL, NULL, NULL);
             radian_z = atan2(si, cs);
             eina_matrix4_rotate(&m, -1 * radian_z, EINA_MATRIX_AXIS_Z);

             // Calc Scale
             eina_matrix4_values_get(&m,  &sx, NULL, NULL, NULL,
                                         NULL,  &sy, NULL, NULL,
                                         NULL, NULL, NULL, NULL,
                                         NULL, NULL, NULL, NULL);

             lottie_animation_property_override(lot_anim, LOTTIE_ANIMATION_PROPERTY_TR_SCALE, (char*)keypath, 100.0 * sx, 100.0 * sy);
             lottie_animation_property_override(lot_anim, LOTTIE_ANIMATION_PROPERTY_TR_ROTATION, (char*)keypath, radian_z * (180.0 / M_PI));
             lottie_animation_property_override(lot_anim, LOTTIE_ANIMATION_PROPERTY_TR_POSITION, (char*)keypath, tx, ty);
          }
     }
}

#endif


Eina_Bool
vg_common_json_create_vg_node(Vg_File_Data *vfd)
{
#ifdef BUILD_VG_LOADER_JSON
   Lottie_Animation *lot_anim = (Lottie_Animation *) vfd->loader_data;
   if (!lot_anim) return EINA_FALSE;

   if (vfd->vp_list) _value_provider_override(vfd);

   unsigned int frame_num = (vfd->anim_data) ? vfd->anim_data->frame_num : 0;
   const LOTLayerNode *tree =
      lottie_animation_render_tree(lot_anim, frame_num,
                                   vfd->view_box.w, vfd->view_box.h);
   //Root node
   Efl_Canvas_Vg_Container *root = vfd->root;
   if (!root)
     {
        root = efl_add_ref(EFL_CANVAS_VG_CONTAINER_CLASS, NULL);
        if (!root) return EINA_FALSE;
        efl_key_data_set(root, _get_key_val((void *) tree), tree);
        if (tree->keypath) efl_key_data_set(root, "_lot_node_name", tree->keypath);
        vfd->root = root;
     }
   else _reset_vg_tree(root);

#if DEBUG
   printf("%s (%p)\n", efl_class_name_get(efl_class_get(vfd->root)), vfd->root);
#endif

   _update_vg_tree(root, tree, 1);
#else
   return EINA_FALSE;
#endif
   return EINA_TRUE;
}
