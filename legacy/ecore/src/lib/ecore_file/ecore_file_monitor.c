/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"

Ecore_File_Type
ecore_file_monitor_type_get(Ecore_File_Monitor *em)
{
   return em->type;
}
