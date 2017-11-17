/* ECTOR - EFL retained mode drawing library
 * Copyright (C) 2014 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Ector.h>

#include "scenegraph_suite.h"
#include "../../../static_libs/scenegraph/Scenegraph.h"

START_TEST(scenegraph_node_create)
{
   SG_Node *root, *clip, *opacity, *transform, *geometry;

   root = (SG_Node *)sg_node_root_create();

   fail_if(root == 0);
   fail_if(root->fn->type(root) != SG_NODE_TYPE_ROOT);
   fail_if(root->m_type != SG_NODE_TYPE_ROOT);
   fail_if(root->fn->parent(root) != 0);


   opacity = (SG_Node *)sg_node_opacity_create();

   fail_if(opacity == 0);
   fail_if(opacity->fn->type(opacity) != SG_NODE_TYPE_OPACITY);
   fail_if(opacity->m_type != SG_NODE_TYPE_OPACITY);
   fail_if(opacity->fn->parent(opacity) != 0);

   clip = (SG_Node *)sg_node_clip_create();

   fail_if(clip == 0);
   fail_if(clip->fn->type(clip) != SG_NODE_TYPE_CLIP);
   fail_if(clip->m_type != SG_NODE_TYPE_CLIP);
   fail_if(clip->fn->parent(clip) != 0);

   transform = (SG_Node *)sg_node_transform_create();

   fail_if(transform == 0);
   fail_if(transform->fn->type(transform) != SG_NODE_TYPE_TRANSFORM);
   fail_if(transform->m_type != SG_NODE_TYPE_TRANSFORM);
   fail_if(transform->fn->parent(transform) != 0);

   geometry = (SG_Node *)sg_node_geometry_create();

   fail_if(geometry == 0);
   fail_if(geometry->fn->type(geometry) != SG_NODE_TYPE_GEOMETRY);
   fail_if(geometry->m_type != SG_NODE_TYPE_GEOMETRY);
   fail_if(geometry->fn->parent(geometry) != 0);

   sg_node_destroy(root);
   sg_node_destroy(opacity);
   sg_node_destroy(clip);
   sg_node_destroy(transform);
   sg_node_destroy(geometry);
}

END_TEST

START_TEST(scenegraph_node_api)
{
   SG_Node *root, *clip, *opacity, *transform, *geometry, *clip1;

   root = (SG_Node *)sg_node_root_create();
   opacity = (SG_Node *)sg_node_opacity_create();
   clip = (SG_Node *)sg_node_clip_create();
   transform = (SG_Node *)sg_node_transform_create();
   geometry = (SG_Node *)sg_node_geometry_create();

// append_child & prepend_child
   root->fn->append_child(root, opacity);
   root->fn->append_child(root, transform);
   root->fn->append_child(root, geometry);
   root->fn->prepend_child(root, clip);


   fail_if(root->fn->child_count(root) != 4);

// parent check
   fail_if(clip->fn->parent(clip) != root);
   fail_if(opacity->fn->parent(opacity) != root);
   fail_if(clip->fn->parent(clip) != root);
   fail_if(transform->fn->parent(transform) != root);
   fail_if(geometry->fn->parent(geometry) != root);

// check child odering
   fail_if(root->fn->child_at(root, 0) != clip);
   fail_if(root->fn->child_at(root, 2) != transform);

// check reordering insert_before
   root->fn->remove_child(root, opacity);
   root->fn->insert_child_before(root, opacity, clip);
   fail_if(root->fn->child_at(root, 0) != opacity);

//check reordering insert_after
   root->fn->remove_child(root, opacity);
   root->fn->insert_child_after(root, opacity, clip);
   fail_if(root->fn->child_at(root, 1) != opacity);

// reparent all child to its child
   root->fn->reparent_childs_to(root, clip);
   fail_if(root->fn->child_count(root) != 4);

   clip1 = (SG_Node *)sg_node_clip_create();

   root->fn->reparent_childs_to(root, clip1);
   fail_if(root->fn->child_count(root) != 0);
   fail_if(clip1->fn->child_count(clip1) != 4);

// node flags
   geometry->fn->flags_set(geometry, SG_NODE_OWNS_GEOMETRY, EINA_TRUE);
   fail_if(!(geometry->fn->flags(geometry) & SG_NODE_OWNS_GEOMETRY));
   geometry->fn->flags_set(geometry, SG_NODE_OWNS_GEOMETRY, EINA_FALSE);
   fail_if(geometry->fn->flags(geometry) & SG_NODE_OWNS_GEOMETRY);

// node deletion
   sg_node_destroy(clip);
   fail_if(clip1->fn->child_count(clip1) != 3);
   sg_node_destroy(transform);
   fail_if(clip1->fn->child_count(clip1) != 2);

   sg_node_destroy(clip1);

   sg_node_destroy(root);
}
END_TEST

START_TEST(scenegraph_transform_node_api)
{
   SG_Node *root;
   SG_Transform_Node *transform;
   Eina_Matrix3 m;

   root = (SG_Node *)sg_node_root_create();
   transform = sg_node_transform_create();

   root->fn->append_child(root, (SG_Node *)transform);

   fail_if(transform->fn->matrix_get(transform) != 0);

   eina_matrix3_identity(&m);
   eina_matrix3_scale(&m, 2, 1.5);
   transform->fn->matrix_set(transform, &m);

   fail_if(!eina_matrix3_equal(transform->fn->matrix_get(transform), &m));

   eina_matrix3_scale(&m, 2.5, 1.5);
   transform->fn->combined_matrix_set(transform, &m);
   fail_if(!eina_matrix3_equal(transform->fn->combined_matrix_get(transform), &m));

   sg_node_destroy(root);
}
END_TEST

START_TEST(scenegraph_opacity_node_api)
{
   SG_Node *root;
   SG_Opacity_Node *opacity;

   root = (SG_Node *)sg_node_root_create();
   opacity = sg_node_opacity_create();

   root->fn->append_child(root, (SG_Node *)opacity);

   fail_if(opacity->fn->opacity_get(opacity) != 1);
   fail_if(opacity->fn->combined_opacity_get(opacity) != 1);

   opacity->fn->opacity_set(opacity, 0.5);
   opacity->fn->combined_opacity_set(opacity, 0.25);

   fail_if(opacity->fn->opacity_get(opacity) != 0.5);
   fail_if(opacity->fn->combined_opacity_get(opacity) != 0.25);

   sg_node_destroy(root);
}
END_TEST

START_TEST(scenegraph_clip_node_api)
{
   SG_Node *root;
   SG_Clip_Node *clip;
   SG_Node *geometry;
   Eina_Rect r= EINA_RECT(0, 0, 10, 10);
   Eina_Rect r1;

   root = (SG_Node *)sg_node_root_create();
   clip = sg_node_clip_create();
   geometry = (SG_Node *)sg_node_geometry_create();

   root->fn->append_child(root, (SG_Node *)clip);
   root->fn->append_child(root, geometry);

   fail_if(clip->fn->rect_hint_get(clip) == EINA_TRUE);

   clip->fn->clip_rect_set(clip, r);
   r1 = clip->fn->clip_rect_get(clip);

   fail_if((r1.x != r.x) ||
           (r1.y != r.y) ||
           (r1.w != r.w) ||
           (r1.h != r.h));

   clip->fn->rect_hint_set(clip, EINA_TRUE);
   fail_if(clip->fn->rect_hint_get(clip) != EINA_TRUE);

   clip->fn->clipper_set((SG_Node *)clip, geometry);
   fail_if(clip->fn->clipper_get((SG_Node *)clip) != geometry);

   sg_node_destroy(root);
}
END_TEST

START_TEST(scenegraph_geometry_node_api)
{
   SG_Node *root;
   SG_Clip_Node *clip;
   SG_Node *geometry;
   Eina_Matrix3 m;

   root = (SG_Node *)sg_node_root_create();
   clip = sg_node_clip_create();
   geometry = (SG_Node *)sg_node_geometry_create();

   root->fn->append_child(root, (SG_Node *)clip);
   root->fn->append_child(root, geometry);

   fail_if(geometry->fn->inherited_opacity_get(geometry) != 1);
   fail_if(geometry->fn->geometry_get(geometry) != 0);
   fail_if(geometry->fn->render_matrix_get(geometry) != 0);

   geometry->fn->inherited_opacity_set(geometry, 0.5);
   fail_if(geometry->fn->inherited_opacity_get(geometry) != 0.5);

   eina_matrix3_identity(&m);
   eina_matrix3_scale(&m, 2, 1.5);
   geometry->fn->render_matrix_set(geometry, &m);
   fail_if(!eina_matrix3_equal(geometry->fn->render_matrix_get(geometry), &m));

   sg_node_destroy(root);
}
END_TEST

void
scenegraph_test_nodes(TCase *tc)
{
   tcase_add_test(tc, scenegraph_node_create);
   tcase_add_test(tc, scenegraph_node_api);
   tcase_add_test(tc, scenegraph_transform_node_api);
   tcase_add_test(tc, scenegraph_opacity_node_api);
   tcase_add_test(tc, scenegraph_clip_node_api);
   tcase_add_test(tc, scenegraph_geometry_node_api);
}
