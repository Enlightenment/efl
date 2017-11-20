#ifndef SG_REGION_H
#define SG_REGION_H

#include <Eina.h>

typedef struct _SG_Region_Func          SG_Region_Func;
typedef struct _SG_Region               SG_Region;
typedef struct _SG_Region_Data          SG_Region_Data;

struct _SG_Region_Func
{
    SG_Region      (*copy)(SG_Region *obj); //deep copy
    SG_Region      (*ref)(SG_Region *obj); // shallow copy
    int            (*rect_count)(const SG_Region *obj);
    Eina_Rect      (*rect_at)(const SG_Region *obj, int index);
    void           (*add_rect)(SG_Region *obj, Eina_Rect r);
    void           (*substract_rect)(SG_Region *obj, Eina_Rect r);
    void           (*intersect_rect)(SG_Region *obj, Eina_Rect r);
    void           (*add_region)(SG_Region *obj, SG_Region r);
    void           (*substract_region)(SG_Region *obj, SG_Region r);
    void           (*intersect_region)(SG_Region *obj, SG_Region r);
    Eina_Bool      (*equal)(const SG_Region *obj, SG_Region r);
    void           (*clear)(SG_Region *obj);
    void           (*reset)(SG_Region *obj, Eina_Rect r);
    Eina_Bool      (*empty)(const SG_Region *obj);
    Eina_Bool      (*contains_rect)(const SG_Region *obj, Eina_Rect r);
    void           (*translate)(SG_Region *obj, int x, int y);
    Eina_Rect      (*bounding_rect)(const SG_Region *obj);

};

SG_Region   sg_region_new(void);
void        sg_region_free(SG_Region *obj);

struct _SG_Region
{
    SG_Region_Func   *fn;
    SG_Region_Data   *data;
};

#endif // SG_REGION_H
