#ifndef _ECORDOVA_CONSOLE_PRIVATE_H
#define _ECORDOVA_CONSOLE_PRIVATE_H

//#include "ecordova_private.h"
#include "ecordova_console.eo.h"

typedef struct _Ecordova_Console_Data Ecordova_Console_Data;

/**
 * Ecordova.Console private data
 */
struct _Ecordova_Console_Data
{
   Eo                            *obj;
   Ecordova_Console_LoggerLevel   level;
   Eina_Bool                      console_use;
   Eina_Bool                      logger_use;
};

#endif
