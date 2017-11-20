#ifndef SG_RLE_H
#define SG_RLE_H

#include <Eina.h>

typedef struct _SG_Rle_Func             SG_Rle_Func;
typedef struct _SG_Rle                  SG_Rle;
typedef struct _SG_Rle_Data             SG_Rle_Data;

typedef struct _SG_Span                 SG_Span;

struct  _SG_Span
{
  short              x;
  short              y;
  unsigned short     len;
  unsigned short     coverage;
};

/* callback used by some of SG_Rle class function to notify user
 * span       : pointer to the start of the span list
 * span_count : the number of the spans in the list
 * user_data  : user_data passed by the user in the api
 * NOTE: the span pointer will be only valid duringing the duration
 *       of the call so user has to take a copy if the data need to be
 *       stored.
 */
typedef void   (*SG_Span_Cb)(SG_Span *span, int span_count, void *user_data);

struct _SG_Rle_Func
{
    SG_Rle         (*copy)(const SG_Rle *obj); //deep copy
    SG_Rle         (*ref)(SG_Rle *obj); // shallow copy
    Eina_Bool      (*empty)(const SG_Rle *obj);
    void           (*clear)(SG_Rle *obj);
    void           (*translate)(SG_Rle *obj, int x, int y);
    void           (*intersected_rect)(const SG_Rle *obj, Eina_Rect r, SG_Span_Cb cb, void *user_data);
    void           (*intersected_rle)(const SG_Rle *obj, const SG_Rle rle, SG_Span_Cb cb, void *user_data);
    void           (*intersect_rect)(SG_Rle *obj, Eina_Rect r);
    Eina_Rect      (*bounding_rect)(const SG_Rle *obj);
    void           (*append_spans)(SG_Rle *obj, const SG_Span *spans, int count);
};

SG_Rle      sg_rle_new(void);
void        sg_rle_free(SG_Rle *obj);

struct _SG_Rle
{
    SG_Rle_Func   *fn;
    short          x; // translate x
    short          y; // translate y
    SG_Rle_Data   *data; //ref counted data
};

#endif // SG_RLE_H
