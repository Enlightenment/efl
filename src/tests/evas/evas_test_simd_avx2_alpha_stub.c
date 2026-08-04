/* evas_test_simd_ops.c pulls in the i386/MMX blend kernels (BUILD_MMX is on
 * for any x86 config) through its #include of evas_op_blend_main_.c. Those
 * kernels reference ALPHA_255/ALPHA_256, which are otherwise defined as
 * non-exported (hidden visibility) globals inside libevas.so's own copy of
 * evas_blend_main.c. Pulling that whole file in as an extra source drags in
 * unrelated mask/mul compositor externs, so just provide the two constants
 * the MMX path actually needs, matching their definition in evas_blend_main.c.
 */
#include "evas_common_private.h"

const DATA32 ALPHA_255 = 255;
const DATA32 ALPHA_256 = 256;
