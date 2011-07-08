#ifndef _EVAS_ENGINE_SOFTWARE_X11_H
# define _EVAS_ENGINE_SOFTWARE_X11_H

typedef enum
{
   EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XLIB,
   EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XCB
} Evas_Engine_Info_Software_X11_Backend;

typedef struct _Evas_Engine_Info_Software_X11 Evas_Engine_Info_Software_X11;

struct _Evas_Engine_Info_Software_X11
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   struct 
     {
        Evas_Engine_Info_Software_X11_Backend backend;

        void *connection, *screen;
        unsigned int drawable, mask;
        void *visual;
        unsigned int colormap;
        int depth, rotation;

        Eina_Bool alloc_grayscale : 1;
        Eina_Bool debug : 1;
        Eina_Bool shape_dither : 1;
        Eina_Bool destination_alpha : 1;
        Eina_Bool track_mask_changes : 1;

        int alloc_colors_max;
     } info;

   /* engine specific function calls to query stuff about the destination */
   struct 
     {
        void *(*best_visual_get) (int backend, void *connection, int screen);
        unsigned int (*best_colormap_get) (int backend, void *connection, int screen);
        int (*best_depth_get) (int backend, void *connection, int screen);
     } func;

   unsigned char mask_changed : 1;

   /* non-blocking or blocking mode */
   Evas_Engine_Render_Mode render_mode;
};

#endif
