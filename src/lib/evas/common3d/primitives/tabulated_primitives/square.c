#include "../primitive_common.h"

const float vertices_of_square[] =
{
   /* positions       normals           vertex_color          tex_coords  tangents    */
   -0.5,  0.5, 0.0,   0.0, 0.0, -1.0,   1.0, 0.0, 0.0, 1.0,   0.0, 1.0,   1.0, 0.0, 0.0,
    0.5,  0.5, 0.0,   0.0, 0.0, -1.0,   1.0, 0.0, 0.0, 1.0,   1.0, 1.0,   1.0, 0.0, 0.0,
   -0.5, -0.5, 0.0,   0.0, 0.0, -1.0,   1.0, 0.0, 0.0, 1.0,   0.0, 0.0,   1.0, 0.0, 0.0,
    0.5, -0.5, 0.0,   0.0, 0.0, -1.0,   1.0, 0.0, 0.0, 1.0,   1.0, 0.0,   1.0, 0.0, 0.0,
};

const unsigned short indices_of_square[] = {0, 1, 2, 2, 1, 3};

void
evas_model_set_from_square_primitive(Evas_Canvas3D_Mesh *mesh, int frame)
{
   SET_VERTEX_DATA_FROM_ARRAY(mesh, frame, vertices_of_square, 4, indices_of_square, 6)
}
