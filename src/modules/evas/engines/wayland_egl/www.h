/*
 * Copyright C 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

/*
 * Spring model implemented by Kristian Hogsberg.
 */

#ifndef EVAS_WWW_H
# define EVAS_WWW_H

# include "../gl_generic/Evas_Engine_GL_Generic.h"

typedef struct _Model Model;

Model *
wobbly_create(int x, int y, int width, int height);

void
wobbly_resize(Model *m, int dw, int dh);

void
wobbly_draw(Evas_Engine_GL_Context *gl_context, Model *m);

Eina_Bool
wobbly_process(Model *m, Evas_Engine_Info_Wayland_Egl *info, int w, int h, Eina_Bool redirected);

void
wobbly_bounds(Model *m, float *tlx, float *tly, float *brx, float *bry);

#endif
