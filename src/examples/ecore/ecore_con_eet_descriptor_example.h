#ifndef _ECORE_CON_EET_DESCTRIPOR_EXAMPLE_H
#define _ECORE_CON_EET_DESCTRIPOR_EXAMPLE_H

#include <Eet.h>
#include <Ecore_Con_Eet.h>

#define EXAMPLE_STREAM "example_stream"

typedef struct
{
   int id;
   char *message;
   char *not_sending;
}ECE_Example;

void ece_example_descriptor_init(void);
void ece_example_descriptor_shutdown(void);
void ece_example_descriptor_register_descs(Ecore_Con_Eet *eet_svr);

#endif
