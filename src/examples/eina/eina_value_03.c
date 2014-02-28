//Compile with:
//gcc eina_value_03.c -o eina_value_03 `pkg-config --cflags --libs eina`

#include <Eina.h>
#include <sys/time.h>

static Eina_Bool
_tz_setup(const Eina_Value_Type *type, void *mem)
{
   memset(mem, 0, type->value_size);
   return EINA_TRUE;
}

static Eina_Bool
_tz_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_tz_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void * dst)
{
   struct timezone *tzsrc = (struct timezone *)src;
   struct timezone *tzdst = dst;
   *tzdst = *tzsrc;
   return EINA_TRUE;
}

static Eina_Bool
_tz_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   struct timezone tza = *(struct timezone*)a;
   struct timezone tzb = *(struct timezone*)b;

   if (tza.tz_minuteswest < tzb.tz_minuteswest)
     return -1;
   else if (tza.tz_minuteswest > tzb.tz_minuteswest)
     return 1;
   return 0;
}

static Eina_Bool
_tz_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   *(struct timezone*)mem = *(struct timezone*)ptr;
   return EINA_TRUE;
}

static Eina_Bool
_tz_vset(const Eina_Value_Type *type, void *mem, va_list args)
{
   const struct timezone tz = va_arg(args, struct timezone);
   return _tz_pset(type, mem, &tz);
}

static Eina_Bool
_tz_pget(const Eina_Value_Type *type, const void *mem, void *ptr)
{
   memcpy(ptr, mem, type->value_size);
   return EINA_TRUE;
}

static Eina_Bool
_tz_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   struct timezone v = *(struct timezone*)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v.tz_minuteswest;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v.tz_minuteswest;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v.tz_minuteswest;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v.tz_minuteswest;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v.tz_minuteswest;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        char other_mem = v.tz_minuteswest;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v.tz_minuteswest;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v.tz_minuteswest;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v.tz_minuteswest;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v.tz_minuteswest;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     return eina_value_type_pset(convert, convert_mem, &v.tz_minuteswest);
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     return eina_value_type_pset(convert, convert_mem, &v.tz_minuteswest);
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%d", v.tz_minuteswest);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   return EINA_FALSE;
}

static Eina_Value_Type TZ_TYPE = {
   EINA_VALUE_TYPE_VERSION,
   sizeof(struct timezone),
   "struct timezone",
   _tz_setup,
   _tz_flush,
   _tz_copy,
   _tz_compare,
   _tz_convert_to,
   NULL, //No convert from
   _tz_vset,
   _tz_pset,
   _tz_pget
};

int main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Eina_Value vtv, vtz;
   struct timeval tv;
   struct timezone tz;
   char *s;

   eina_init();

   eina_value_setup(&vtv, EINA_VALUE_TYPE_TIMEVAL);
   eina_value_setup(&vtz, &TZ_TYPE);

   gettimeofday(&tv, &tz);
   eina_value_set(&vtv, tv);
   eina_value_set(&vtz, tz);

   s = eina_value_to_string(&vtv);
   printf("time: %s\n", s);
   free(s);
   s = eina_value_to_string(&vtz);
   printf("timezone: %s\n", s);
   free(s);

   eina_value_flush(&vtz);
   eina_value_flush(&vtv);
}
