#ifndef EVAS_VG_PRIVATE_H_
# define EVAS_VG_PRIVATE_H_

typedef struct _Evas_VG_Container_Data Evas_VG_Container_Data;
struct _Evas_VG_Container_Data
{
   Eina_List *children;
};

Eina_Bool
evas_vg_path_dup(Evas_VG_Path_Command **out_cmd, double **out_pts,
                 const Evas_VG_Path_Command *in_cmd, const double *in_pts);

#endif
