#ifndef _EVAS_MAP_H
#define _EVAS_MAP_H

EAPI void
evas_common_map_rgba(RGBA_Image *src, RGBA_Image *dst,
                      RGBA_Draw_Context *dc,
                      int npoints, RGBA_Map_Point *points,
                      int smooth, int level);

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
