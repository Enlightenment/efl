#ifndef __EVAS_ENGINE_GL_COCOA_H__
#define __EVAS_ENGINE_GL_COCOA_H__

typedef struct _Evas_Engine_Info_GL_Cocoa Evas_Engine_Info_GL_Cocoa;

struct _Evas_Engine_Info_GL_Cocoa
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
   Evas_Engine_Info magic;

   /* engine specific data & parameters it needs to set up */
   void   *window;
   void   *view;
   int   depth;

};


#endif /* __EVAS_ENGINE_GL_COCOA_H__ */
