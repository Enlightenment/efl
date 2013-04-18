#ifndef ECORE_AUDIO_PROTECTED_H_
#define ECORE_AUDIO_PROTECTED_H_

#include "Eo.h"
#include "Ecore.h"
#include "Ecore_Audio.h"

#define ecore_audio_obj_in_read_internal(buf, len, ret) ECORE_AUDIO_OBJ_IN_ID(ECORE_AUDIO_OBJ_IN_SUB_ID_READ_INTERNAL), EO_TYPECHECK(void *, buf), EO_TYPECHECK(size_t, len), EO_TYPECHECK(ssize_t *, ret)

#endif
