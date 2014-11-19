#ifndef _EVAS_MAP_H
#define _EVAS_MAP_H

typedef void (*Evas_Common_Map_RGBA_Cb)            (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Map_Point *p, int smooth, int level);
typedef Eina_Bool (*Evas_Common_Map_Thread_RGBA_Cb)     (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Map *map, int smooth, int level, int offset);

EAPI void
evas_common_map_rgba_cb(RGBA_Image *src, RGBA_Image *dst,
                        RGBA_Draw_Context *dc,
                        int npoints, RGBA_Map_Point *points,
                        int smooth, int level,
                        Evas_Common_Map_RGBA_Cb cb);

EAPI Eina_Bool evas_common_map_thread_rgba_cb(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Map *map, int smooth, int level, int offset, Evas_Common_Map_Thread_RGBA_Cb cb);

EAPI void
evas_common_map_rgba(RGBA_Image *src, RGBA_Image *dst,
                      RGBA_Draw_Context *dc,
                      int npoints, RGBA_Map_Point *points,
                      int smooth, int level);

EAPI void evas_common_map_rgba_draw(RGBA_Image *src, RGBA_Image *dst, int clip_x, int clip_y, int clip_w, int clip_h, DATA32 mul_col, int render_op, int npoints, RGBA_Map_Point *p, int smooth, Eina_Bool anti_alias, int level, RGBA_Image *mask_ie, int mask_x, int mask_y);

EAPI Eina_Bool
evas_common_map_rgba_prepare(RGBA_Image *src, RGBA_Image *dst,
                             RGBA_Draw_Context *dc,
                             RGBA_Map *m);
EAPI void
evas_common_map_rgba_do(const Eina_Rectangle *clip,
                        RGBA_Image *src, RGBA_Image *dst,
			RGBA_Draw_Context *dc,
			const RGBA_Map *m,
			int smooth, int level);
EAPI void
evas_common_map_rgba_clean(RGBA_Map *m);

#endif /* _EVAS_MAP_H */
