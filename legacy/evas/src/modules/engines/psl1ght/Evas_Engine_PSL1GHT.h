#ifndef _EVAS_ENGINE_PSL1GHT_H
#define _EVAS_ENGINE_PSL1GHT_H

typedef struct _Evas_Engine_Info_PSL1GHT Evas_Engine_Info_PSL1GHT;

struct _Evas_Engine_Info_PSL1GHT
{
   /* PRIVATE - don't mess with this baby or evas will poke its tongue out */
   /* at you and make nasty noises */
     Evas_Engine_Info        magic;

     /* non-blocking or blocking mode */
     Evas_Engine_Render_Mode render_mode;
};
#endif

