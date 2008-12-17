#ifndef EVAS_XLIB_COLOR_H
#define EVAS_XLIB_COLOR_H

void evas_software_xlib_x_color_init (void);

Convert_Pal *evas_software_xlib_x_color_allocate (Display         *disp,
                                                  Colormap         cmap,
                                                  Visual          *vis,
                                                  Convert_Pal_Mode colors);

void evas_software_xlib_x_color_deallocate (Display     *disp,
                                            Colormap     cmap,
                                            Visual      *vis,
                                            Convert_Pal *pal);

#endif
