#ifndef EMOTION_PRIVATE_H
#define EMOTION_PRIVATE_H

#include <Evas.h>
#include <Eet.h>
#include "Emotion.h"

Eina_Bool emotion_webcam_init(void);
void emotion_webcam_shutdown(void);
Eina_Bool emotion_webcam_config_load(Eet_File *ef);

Eina_Bool emotion_modules_init(void);
void emotion_modules_shutdown(void);

extern Eina_Hash *_emotion_backends;
extern Eina_Array *_emotion_modules;
extern int _emotion_log_domain;
extern Eina_Prefix *_emotion_pfx;

#define DBG(...) EINA_LOG_DOM_DBG(_emotion_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_emotion_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_emotion_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_emotion_log_domain, __VA_ARGS__)
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_emotion_log_domain, __VA_ARGS__)

#include "Emotion_Module.h"

#endif
