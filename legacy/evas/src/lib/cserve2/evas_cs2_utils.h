/* THIS FILE TO BE SHARED WITH THE BIN PART. KEEP IT CLEAN. THERE BE DRAGONS */
#ifndef _EVAS_CSERVE2_UTILS_H_
#define _EVAS_CSERVE2_UTILS_H_

typedef struct _Fash_Glyph2      Fash_Glyph2;

Fash_Glyph2 *fash_gl_new(void (*free_cb)(void *glyph));
void fash_gl_free(Fash_Glyph2 *fash);
void *fash_gl_find(Fash_Glyph2 *fash, int item);
void fash_gl_add(Fash_Glyph2 *fash, int item, void *glyph);
void fash_gl_del(Fash_Glyph2 *fash, int item);

#endif /* EVAS_CSERVE2_UTILS_H_ */
