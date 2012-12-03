/*
 * Copyright (C) Antonio José Ramos Márquez (bigboss)
 * Copyright (C) Youness Alaoui (KaKaRoTo)
 */

#ifndef __MOVEUTIL_H__
#define __MOVEUTIL_H__

#include <ppu-types.h>
#include <spurs/spurs.h>
#include <io/camera.h>
#include <io/move.h>

typedef struct
{
   Spurs         *spurs;
   cameraInfoEx   camInfo;
   cameraReadInfo camRead;
   gemState       state;
} moveContext;

static inline float
vec_array(vec_float4 vec, unsigned int idx)
{
   union {
      vec_float4 vec;
      float      array[4];
   } v;

   v.vec = vec;

   if (idx > 3)
     return -1;
   return v.array[idx];
}

moveContext *initMove();
void         endMove(moveContext *context);
void         readGemState(moveContext *context);
int          processMove(moveContext *context);
void         moveGet3DPosition(moveContext *context, float *x, float *y, float *z);

#endif /* __MOVEUTIL_H__ */
