#ifndef EVAS_POLYGON_PRIVATE_H
#define EVAS_POLYGON_PRIVATE_H

typedef struct _Efl_Canvas_Polygon_Data      Efl_Canvas_Polygon_Data;

struct _Efl_Canvas_Polygon_Data
{
   Eina_List           *points;
   void                *engine_data;
   struct {
      int x, y;
   } offset;
   Eina_Rectangle       geometry;
   Eina_Bool            changed : 1;
};
#endif
