/*
 * This file has the description of a eet decriptor which would be used
 * in ecore_con_eet_server_example.c and ecore_con_eet_client_example.c.
 * The structure for the descriptor is defined in ecore_con_eet_descriptor_example.h.
 */

#include "ecore_con_eet_descriptor_example.h"

static Eet_Data_Descriptor *_ece_example_descriptor;

void
ece_example_descriptor_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, ECE_Example);
   _ece_example_descriptor = eet_data_descriptor_stream_new(&eddc);

#define ECE_EXAMPLE_ADD_BASIC(member, eet_type) \
  EET_DATA_DESCRIPTOR_ADD_BASIC             \
    (_ece_example_descriptor, ECE_Example, # member, member, eet_type)

   ECE_EXAMPLE_ADD_BASIC(id, EET_T_INT);
   ECE_EXAMPLE_ADD_BASIC(message, EET_T_STRING);
   /* not_sending is not being added to the descriptor to show that,
    * only those parameters which have been added are sent in the
    * communication.
    */
#undef ECE_EXAMPLE_ADD_BASIC
}

void
ece_example_descriptor_shutdown(void)
{
   eet_data_descriptor_free(_ece_example_descriptor);
}

void
ece_example_descriptor_register_descs(Ecore_Con_Eet *eet_svr)
{
   ecore_con_eet_register(eet_svr, EXAMPLE_STREAM, _ece_example_descriptor);
}
