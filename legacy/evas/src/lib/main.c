#include "evas_common.h"
#include "evas_private.h"

static Evas_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Evas_Version *evas_version = &_version;

int _evas_alloc_error = 0;
static int _evas_debug_init = 0;
static int _evas_debug_show = 0;
static int _evas_debug_abort = 0;

EAPI Evas_Alloc_Error
evas_alloc_error(void)
{
   return _evas_alloc_error;
}

/* free cached items only in ram for speed reasons. return 0 if can't free */
int
evas_mem_free(int mem_required __UNUSED__)
{
   return 0;
}

/* start reducing quality of images etc. return 0 if can't free anything */
int
evas_mem_degrade(int mem_required __UNUSED__)
{
   return 0;
}

void *
evas_mem_calloc(int size)
{
   void *ptr;

   ptr = calloc(1, size);
   if (ptr) return ptr;
   MERR_BAD();
   while ((!ptr) && (evas_mem_free(size))) ptr = calloc(1, size);
   if (ptr) return ptr;
   while ((!ptr) && (evas_mem_degrade(size))) ptr = calloc(1, size);
   if (ptr) return ptr;
   MERR_FATAL();
   return NULL;
}

void
evas_debug_error(void)
{
   if (!_evas_debug_init)
     {
	if (getenv("EVAS_DEBUG_SHOW")) _evas_debug_show = 1;
	if (getenv("EVAS_DEBUG_ABORT")) _evas_debug_abort = 1;
	_evas_debug_init = 1;
     }
   if (_evas_debug_show)
     CRIT("Evas Magic Check Failed!!!");
}

void
evas_debug_input_null(void)
{
   if (!_evas_debug_init)
     {
	if (getenv("EVAS_DEBUG_SHOW")) _evas_debug_show = 1;
	if (getenv("EVAS_DEBUG_ABORT")) _evas_debug_abort = 1;
	_evas_debug_init = 1;
     }
   if (_evas_debug_show)
     CRIT("Input object pointer is NULL!");
   if (_evas_debug_abort) abort();
}

void
evas_debug_magic_null(void)
{
   if (!_evas_debug_init)
     {
	if (getenv("EVAS_DEBUG_SHOW")) _evas_debug_show = 1;
	if (getenv("EVAS_DEBUG_ABORT")) _evas_debug_abort = 1;
	_evas_debug_init = 1;
     }
   if (_evas_debug_show)
     CRIT("Input object is zero'ed out (maybe a freed object or zero-filled RAM)!");
   if (_evas_debug_abort) abort();
}

void
evas_debug_magic_wrong(DATA32 expected, DATA32 supplied)
{
   if (!_evas_debug_init)
     {
	if (getenv("EVAS_DEBUG_SHOW")) _evas_debug_show = 1;
	if (getenv("EVAS_DEBUG_ABORT")) _evas_debug_abort = 1;
	_evas_debug_init = 1;
     }
   if (_evas_debug_show)
     CRIT("Input object is wrong type\n"
	  "    Expected: %08x - %s\n"
	  "    Supplied: %08x - %s",
	  expected, evas_debug_magic_string_get(expected),
	  supplied, evas_debug_magic_string_get(supplied));
   if (_evas_debug_abort) abort();
}

void
evas_debug_generic(const char *str)
{
   if (!_evas_debug_init)
     {
	if (getenv("EVAS_DEBUG_SHOW")) _evas_debug_show = 1;
	if (getenv("EVAS_DEBUG_ABORT")) _evas_debug_abort = 1;
	_evas_debug_init = 1;
     }
   if (_evas_debug_show)
     CRIT("%s", str);
   if (_evas_debug_abort) abort();
}

const char *
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
      case MAGIC_OBJ_POLYGON:
	return "Evas_Object (Polygon)";
	break;
      case MAGIC_OBJ_IMAGE:
	return "Evas_Object (Image)";
	break;
      case MAGIC_OBJ_TEXT:
	return "Evas_Object (Text)";
	break;
      case MAGIC_OBJ_SMART:
	return "Evas_Object (Smart)";
	break;
      case MAGIC_EVAS_GL:
	return "Evas_GL";
      case MAGIC_MAP:
	return "Evas_Map";
      default:
	return "<UNKNOWN>";
     };
   return "<UNKNOWN>";
}
