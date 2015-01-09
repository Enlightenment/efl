#ifndef _ELDBUS_MODEL_SIGNAL_PRIVATE_H
#define _ELDBUS_MODEL_SIGNAL_PRIVATE_H

#include "Eldbus_Model.h"

typedef struct _Eldbus_Model_Signal_Data Eldbus_Model_Signal_Data;

/**
 * eldbus_model_signal
 */
struct _Eldbus_Model_Signal_Data
{
   Eo *obj;
   Eldbus_Signal_Handler *handler;
   const Eldbus_Introspection_Signal *signal;
};

#endif

