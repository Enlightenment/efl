#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

void
evas_debug_error(void)
{
   fprintf(stderr, 
	   "*** EVAS ERROR: Evas Magic Check Failed!!!\n");
}

void
evas_debug_input_null(void)
{
   fprintf(stderr, 
	   "  Input object pointer is NULL!\n");
}

void
evas_debug_magic_null(void)
{
   fprintf(stderr, 
	   "  Input object is zero'ed out (maybe a freed object or zero-filled RAM)!\n");
}

void
evas_debug_magic_wrong(DATA32 expected, DATA32 supplied)
{
   fprintf(stderr, 
	   "  Input object is wrong type\n"
	   "    Expected: %08x - %s\n"
	   "    Supplied: %08s - %s\n",
	   expected, evas_debug_magic_string_get(expected),
	   supplied, evas_debug_magic_string_get(supplied));
}

char *
evas_debug_magic_string_get(DATA32 magic)
{
   switch (magic)
     {
      case MAGIC_EVAS:
	return "Evas";
	break;
      case MAGIC_OBJ:
	return "Evas_Object";
	break;
      case MAGIC_OBJ_RECTANGLE:
	return "Evas_Object (Rectangle)";
	break;
      case MAGIC_OBJ_LINE:
	return "Evas_Object (Line)";
	break;
      case MAGIC_OBJ_GRADIENT:
	return "Evas_Object (Gradient)";
	break;
      case MAGIC_OBJ_POLYGON:
	return "Evas_Object (Polygon)";
	break;
      case MAGIC_OBJ_IMAGE:
	return "Evas_Object (Image)";
	break;
      case MAGIC_OBJ_TEXT:
	return "Evas_Object (Text)";
	break;
      default:
	return "<UNKNOWN>";
     };
   return "<UNKNOWN>";
}
