/* length functions */
#include <stdlib.h>
#include <string.h>

void
_ecore_dbus_message_append_length(unsigned char **buf,
				  unsigned int old_length, unsigned int size)
{
   *buf = realloc(*buf, old_length + size);
   memset(*buf + old_length, 0, size);
}

void
_ecore_dbus_message_increase_length(unsigned char **buf,
				    unsigned int new_length)
{
   *buf = realloc(*buf, new_length);
}

/* padding functions */

void
_ecore_dbus_message_padding(unsigned char **buf, unsigned int *old_length, int size)
{
   unsigned int        padding;

   padding = *old_length % size;
   if (padding != 0)
     {
	padding = size - padding;
	_ecore_dbus_message_append_length(buf, *old_length, padding);
     }
   *old_length += padding;
}

void
_ecore_dbus_message_padding_skip(unsigned int *old_length, int size)
{
   unsigned int        padding;

   padding = *old_length % size;
   if (padding != 0)
      *old_length += size - padding;
   return;
}

/* appending/reading functions */

void
_ecore_dbus_message_append_nbytes(unsigned char *buf, unsigned char *data,
				  unsigned int data_length)
{
   memcpy(buf, data, data_length);
}

void
_ecore_dbus_message_append_byte(unsigned char *buf, unsigned char c)
{
   *buf = c;
}

void
_ecore_dbus_message_append_uint32(unsigned char *buf, unsigned int i)
{
   unsigned char      *c;

   c = (unsigned char *)&i;
   *(buf + 0) = *(c + 0);
   *(buf + 1) = *(c + 1);
   *(buf + 2) = *(c + 2);
   *(buf + 3) = *(c + 3);
}

unsigned char
_ecore_dbus_message_read_byte(unsigned char *buf)
{
   return *(unsigned char *)buf;
}

unsigned int
_ecore_dbus_message_read_uint32(unsigned char *buf)
{
   return *(unsigned int *)buf;
}

