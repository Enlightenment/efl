#ifndef EVAS_VG_PRIVATE_H_
# define EVAS_VG_PRIVATE_H_

typedef struct _Evas_VG_Node_Data Evas_VG_Node_Data;
struct _Evas_VG_Node_Data
{
   Eina_Matrix3 *m;
   Evas_VG_Node *mask;

   void (*render_pre)(void);
   void (*render)(void);

   double x, y;
   int r, g, b, a;
   Eina_Bool visibility;
};

typedef struct _Evas_VG_Container_Data Evas_VG_Container_Data;
struct _Evas_VG_Container_Data
{
   Eina_List *children;
};

Eina_Bool
efl_graphics_path_dup(Efl_Graphics_Path_Command **out_cmd, double **out_pts,
                      const Efl_Graphics_Path_Command *in_cmd, const double *in_pts);

#endif
