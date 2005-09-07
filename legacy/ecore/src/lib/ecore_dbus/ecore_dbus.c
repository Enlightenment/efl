#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

/********************************************************************************/
/* About									*/
/********************************************************************************/
/* Author: Jorge Luis Zapata							*/
/* Version: 0.2.1								*/
/********************************************************************************/
/* Todo										*/
/********************************************************************************/
/* free allocated data								*/
/* make other authentication mechanisms besides external			*/
/********************************************************************************/
/* Changelog									*/
/********************************************************************************/
/* 0.0 	usable interface							*/
/* 0.1 	change dbus spec version (0.11):					*/
/* 	different header format							*/
/* 	new type signature							*/
/*	header length = 8 byte multiple 					*/
/*	paddings value must be null						*/
/*	body need not to end in a 8 byte boundary				*/
/*	new data type: variant,signature,dict					*/
/*	ecore_oldlist cant join two lists so is difficult to handle compound	*/
/*	data types (variant,struct,dict,array) in a stack way			*/
/* 0.2 	change again the spec version (0.8)					*/
/*	i realized that the first version was correct, when i read the spec	*/
/*	for ecore_dbus 0.1 i was reading a previous version :(			*/
/*	put again the data type byte in each marshaled data			*/
/*										*/
/* 29-03-05									*/
/* 0.2.1 some segfault fixes, new tests						*/

static int          ecore_dbus_server_send(Ecore_DBus_Server * svr,
					   char *command, int length);
Ecore_List2        *_ecore_dbus_message_print_field(Ecore_List2 * l,
						    unsigned char *buf);
void                _ecore_dbus_message_print_raw(unsigned char *msg,
						  unsigned int msg_len);

/**********************/
/* ecore dbus message */
/**********************/

#define BODY_LEN 64
#define HEADER_LEN 16

/* length functions */
/********************/
/* all of these increase the length */
void
_ecore_dbus_message_increase_lengthz(unsigned char **buf,
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
/*********************/
/* all of these increase the length */
void
_ecore_dbus_message_4byte_padding(unsigned char **buf, unsigned int *old_length)
{
   unsigned int        padding;

   padding = *old_length % 4;
   if (padding != 0)
     {
	padding = 4 - padding;
	_ecore_dbus_message_increase_lengthz(buf, *old_length, padding);
     }
   *old_length += padding;
}
void
_ecore_dbus_message_8byte_padding(unsigned char **buf, unsigned int *old_length)
{
   unsigned int        padding;

   padding = *old_length % 8;
   if (padding != 0)
     {
	padding = 8 - padding;
	_ecore_dbus_message_increase_lengthz(buf, *old_length, padding);
     }
   *old_length += padding;
}

void
_ecore_dbus_message_skip_8byte_padding(unsigned int *old_length)
{
   unsigned int        padding;

   padding = *old_length % 8;
   if (padding != 0)
      *old_length += 8 - padding;
   return;
}

void
_ecore_dbus_message_skip_4byte_padding(unsigned int *old_length)	/* DONE */
{
   unsigned int        padding;

   padding = *old_length % 4;
   if (padding != 0)
      *old_length += 4 - padding;
   return;
}

/* appending/reading functions */
/*******************************/
/* non of these increase the length */
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
_ecore_dbus_message_append_uint32(unsigned char *buf, unsigned long int i)
{
   unsigned char      *c;

   c = (unsigned char *)&i;
   *(buf + 0) = *(c + 0);
   *(buf + 1) = *(c + 1);
   *(buf + 2) = *(c + 2);
   *(buf + 3) = *(c + 3);
}

unsigned long
_ecore_dbus_message_read_uint32(unsigned char *buf)
{
   return *(unsigned long *)buf;
}

unsigned char
_ecore_dbus_message_read_byte(unsigned char *buf)
{
   return *(unsigned char *)buf;
}

/* field functions */
/*******************/

/* memory allocation */
/**********************/
Ecore_DBus_Message *
_ecore_dbus_message_new(Ecore_DBus_Server * svr)
{
   Ecore_DBus_Message *msg = calloc(1, sizeof(Ecore_DBus_Message));

   msg->ref_server = svr;
   msg->body_fields = NULL;
   msg->header_fields = NULL;
   msg->hpos = 0;
   msg->bpos = 0;
   msg->header = NULL;
   msg->body = NULL;
   msg->hlength = 0;
   msg->blength = 0;
   return msg;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_new_field(Ecore_DBus_Data_Type type, unsigned int offset)
{
   Ecore_DBus_Message_Field *f;

   f = malloc(sizeof(Ecore_DBus_Message_Field));
   f->offset = offset;
   f->type = type;
   f->count = 0;
   return f;
}

void
_ecore_dbus_message_free(void *data, void *ev)
{
   Ecore_DBus_Message *msg = ev;
   Ecore_List2        *l = (Ecore_List2 *) msg->body_fields;
   Ecore_List2        *next;

   while (l)
     {
	next = l->next;
	free(l);
	l = next;
     }
   l = (Ecore_List2 *) msg->header_fields;
   while (l)
     {
	next = l->next;
	free(l);
	l = next;
     }
   free(msg->header);
   free(msg->body);
   free(msg);
}

/* signature functions */
/***********************/

/* marshal functions */
/*********************/

/* cambiar todos los *msg por char** asi se podra cambiar el puntero sin problemas */
/* agregarle el parametro de signature por si se quiere q agregue una variable al signature */
/* y pasarle la lista de campos por si pone el argumento en la lista del header o del body */

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_byte(unsigned char **buf, unsigned int *old_length,
				 unsigned char c)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_increase_length(buf, *old_length + 1);	// increase the length + 1, old_length changed
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_BYTE);	// append the data type
   *old_length += 1;
   _ecore_dbus_message_increase_length(buf, *old_length + 1);	// increase the length + 1, old_length changed
   _ecore_dbus_message_append_byte(*buf + *old_length, c);	// append c at cur_length
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_BYTE, *old_length);	// create field
   *old_length += 1;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_uint32(unsigned char **buf,
				   unsigned int *old_length,
				   unsigned long int i)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_increase_length(buf, *old_length + 1);	// increase the length + 1, old_length changed
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_UINT32);	// append the data type
   *old_length += 1;
   _ecore_dbus_message_4byte_padding(buf, old_length);
   _ecore_dbus_message_increase_length(buf, *old_length + 4);
   _ecore_dbus_message_append_uint32(*buf + *old_length, i);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_UINT32, *old_length);
   *old_length += 4;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_path(unsigned char **buf, unsigned int *old_length,
				 unsigned char *str)
{
   int                 str_len;
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_increase_length(buf, *old_length + 1);	// increase the length + 1, old_length changed
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_OBJECT_PATH);	// append the data type
   *old_length += 1;

   str_len = strlen((char *)str);
   _ecore_dbus_message_4byte_padding(buf, old_length);
   _ecore_dbus_message_increase_length(buf, *old_length + 4);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_OBJECT_PATH,
				     *old_length);
   _ecore_dbus_message_append_uint32(*buf + *old_length, str_len);
   *old_length += 4;
   _ecore_dbus_message_increase_length(buf, *old_length + str_len + 1);	// for the \n 
   _ecore_dbus_message_append_nbytes(*buf + *old_length, str, str_len + 1);	// append all the string including \n        
   *old_length += str_len + 1;

   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_string(unsigned char **buf,
				   unsigned int *old_length, unsigned char *str)
{
   int                 str_len;
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_increase_length(buf, *old_length + 1);	// increase the length + 1, new_length changed
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_STRING);	// append the data type
   *old_length += 1;		// old_length = new_length

   str_len = strlen((char *)str);
   _ecore_dbus_message_4byte_padding(buf, old_length);
   _ecore_dbus_message_increase_length(buf, *old_length + 4);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_STRING, *old_length);
   _ecore_dbus_message_append_uint32(*buf + *old_length, str_len);
   *old_length += 4;
   _ecore_dbus_message_increase_length(buf, *old_length + str_len + 1);	// for the \n 
   _ecore_dbus_message_append_nbytes(*buf + *old_length, str, str_len + 1);	// append all the string including \n        
   *old_length += str_len + 1;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_array(unsigned char **msg, unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_8byte_padding(msg, old_length);
   _ecore_dbus_message_increase_length(msg, *old_length + 4);	// for the array length value
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_ARRAY, *old_length);
   *old_length += 4;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_data(unsigned char **buf, unsigned int *old_length,
				 Ecore_DBus_Data_Type type, void *data)
{
   Ecore_DBus_Message_Field *f = NULL;

   switch (type)
     {
     case ECORE_DBUS_DATA_TYPE_UINT32:
	f = _ecore_dbus_message_marshal_uint32(buf, old_length,
					       *(unsigned long *)data);
	break;
     case ECORE_DBUS_DATA_TYPE_STRING:
	f = _ecore_dbus_message_marshal_string(buf, old_length,
					       (unsigned char *)data);
	break;
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	f = _ecore_dbus_message_marshal_path(buf, old_length,
					     (unsigned char *)data);
	break;
     case ECORE_DBUS_DATA_TYPE_INVALID:
     case ECORE_DBUS_DATA_TYPE_BYTE:
     case ECORE_DBUS_DATA_TYPE_BOOLEAN:
     case ECORE_DBUS_DATA_TYPE_INT32:
     case ECORE_DBUS_DATA_TYPE_INT64:
     case ECORE_DBUS_DATA_TYPE_UINT64:
     case ECORE_DBUS_DATA_TYPE_DOUBLE:
     case ECORE_DBUS_DATA_TYPE_CUSTOM:
     case ECORE_DBUS_DATA_TYPE_ARRAY:
     case ECORE_DBUS_DATA_TYPE_DICT:
     default:
	printf("[ecore_dbus] unknown/unhandled data type %c\n", type);
	break;
     }
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_custom_header(unsigned char **buf,
					  unsigned int *old_length,
					  unsigned int code,
					  Ecore_DBus_Data_Type type, void *data)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_increase_length(buf, *old_length + 1);	// increase the length + 1, old_length changed
   _ecore_dbus_message_append_byte(*buf + *old_length, code);	// append header field name at cur_length
   *old_length += 1;
   f = _ecore_dbus_message_marshal_data(buf, old_length, type, data);	// marshal header field data
   f->hfield = code;
   return f;
}

/* unmarshal functions */
/***********************/

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_int32(unsigned char *buf,
				    unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_skip_4byte_padding(old_length);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_INT32, *old_length);
   *old_length += 4;
   return f;

}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_uint32(unsigned char *buf,
				     unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_skip_4byte_padding(old_length);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_UINT32, *old_length);
   *old_length += 4;
   return f;

}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_string(unsigned char *buf,
				     unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;
   unsigned long       str_len;

   _ecore_dbus_message_skip_4byte_padding(old_length);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_STRING, *old_length);
   str_len = _ecore_dbus_message_read_uint32(buf + *old_length);
   *old_length += 4;
   *old_length += str_len + 1;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_path(unsigned char *buf, unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;
   unsigned long       str_len;

   _ecore_dbus_message_skip_4byte_padding(old_length);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_OBJECT_PATH,
				     *old_length);
   str_len = _ecore_dbus_message_read_uint32(buf + *old_length);
   *old_length += 4;
   *old_length += str_len + 1;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_data(unsigned char *buf, unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f = NULL;
   unsigned char       type = _ecore_dbus_message_read_byte(buf + *old_length);

   *old_length += 1;
   switch (type)
     {
     case ECORE_DBUS_DATA_TYPE_INT32:
	f = _ecore_dbus_message_unmarshal_int32(buf, old_length);
	break;
     case ECORE_DBUS_DATA_TYPE_UINT32:
	f = _ecore_dbus_message_unmarshal_uint32(buf, old_length);
	break;
     case ECORE_DBUS_DATA_TYPE_STRING:
	f = _ecore_dbus_message_unmarshal_string(buf, old_length);
	break;
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	f = _ecore_dbus_message_unmarshal_path(buf, old_length);
	break;
     case ECORE_DBUS_DATA_TYPE_BYTE:
     case ECORE_DBUS_DATA_TYPE_BOOLEAN:
     case ECORE_DBUS_DATA_TYPE_INT64:
     case ECORE_DBUS_DATA_TYPE_UINT64:
     case ECORE_DBUS_DATA_TYPE_DOUBLE:
     case ECORE_DBUS_DATA_TYPE_CUSTOM:
     case ECORE_DBUS_DATA_TYPE_ARRAY:
     case ECORE_DBUS_DATA_TYPE_DICT:
     case ECORE_DBUS_DATA_TYPE_INVALID:
     default:
	printf("[ecore_dbus] unknown/unhandled data type %c\n", type);
	break;
     }
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_custom_header(unsigned char *buf,
					    unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;
   unsigned int        code;

   code = _ecore_dbus_message_read_byte(buf + *old_length);	// get header field name at cur_length
   *old_length += 1;
   f = _ecore_dbus_message_unmarshal_data(buf, old_length);	// unmarshal header field data
   f->hfield = code;
   return f;
}

Ecore_DBus_Message *
_ecore_dbus_message_unmarshal(Ecore_DBus_Server * svr, unsigned char *message)
{
   Ecore_DBus_Message_Field *f;

   /* init */
   /********/
   Ecore_DBus_Message *msg = _ecore_dbus_message_new(svr);

   printf("[ecore_dbus] unmarshaling\n");
   /* message header */
   /******************/
   /* common fields  */
   msg->byte_order = *(message + 0);
   msg->type = *(message + 1);
   msg->flags = *(message + 2);
   msg->protocol = *(message + 3);
   msg->hlength = *(unsigned long *)(message + 4);
   msg->blength = *(unsigned long *)(message + 8);
   msg->serial = *(unsigned long *)(message + 12);
   if (msg->type == ECORE_DBUS_MESSAGE_TYPE_INVALID)
     {
	printf("[ecore_dbus] message type invalid\n");
	return NULL;
     }
   /* memcpy the header part */
   _ecore_dbus_message_increase_length(&msg->header, msg->hlength);
   _ecore_dbus_message_append_nbytes(msg->header, message, msg->hlength);
   msg->hpos += 16;
   /* custom fields */
   while ((msg->hpos + 8 - (msg->hpos % 8)) < msg->hlength)
     {
	f = _ecore_dbus_message_unmarshal_custom_header(message, &msg->hpos);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }
   msg->hpos = msg->hlength;
   /* message body */
   /****************/
   message += msg->hlength;
   _ecore_dbus_message_increase_length(&msg->body, msg->blength);
   _ecore_dbus_message_append_nbytes(msg->body, message, msg->blength);
   while (msg->bpos < msg->blength)
     {
	f = _ecore_dbus_message_unmarshal_data(message, &msg->bpos);
	msg->body_fields = _ecore_list2_append(msg->body_fields, f);

     }
   return msg;
}

/* header functions */
/********************/
void
_ecore_dbus_message_common_header(Ecore_DBus_Message * msg, int type, int flags)
{
   _ecore_dbus_message_increase_lengthz(&msg->header, msg->hpos, 16);	// the body,header length arent filled only alloc
   msg->header[0] = msg->byte_order = 'l';	// endiannes (1)
   msg->header[1] = msg->type = (char)type;	// type (1)
   msg->header[2] = msg->flags = 0x0;	// flags (1) 0x1 = no reply expected, 0x2 auto activiation
   msg->header[3] = msg->protocol = 0x0;	// protocol (1)
   msg->ref_server->cnt_msg++;	// autoincrement the client_serial (0 is invalid)
   *(msg->header + 12) = msg->serial = msg->ref_server->cnt_msg;
   msg->hpos = 16;
}

void               *
_ecore_dbus_get_field(unsigned char *buf, Ecore_DBus_Message_Field * f)
{
   switch (f->type)
     {
     case ECORE_DBUS_DATA_TYPE_INT32:
     case ECORE_DBUS_DATA_TYPE_UINT32:
     case ECORE_DBUS_DATA_TYPE_BYTE:
     case ECORE_DBUS_DATA_TYPE_BOOLEAN:
     case ECORE_DBUS_DATA_TYPE_INT64:
     case ECORE_DBUS_DATA_TYPE_UINT64:
     case ECORE_DBUS_DATA_TYPE_DOUBLE:
	return buf + f->offset;
	break;
     case ECORE_DBUS_DATA_TYPE_STRING:
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	return buf + f->offset + 4;
	break;
     case ECORE_DBUS_DATA_TYPE_CUSTOM:
     case ECORE_DBUS_DATA_TYPE_ARRAY:
     case ECORE_DBUS_DATA_TYPE_DICT:
     case ECORE_DBUS_DATA_TYPE_INVALID:
     default:
	printf("[ecore_dbus] unknown/unhandled data type %c\n", f->type);
	break;

     }
   return NULL;
}

void               *
ecore_dbus_get_body_field(Ecore_DBus_Message * m, Ecore_DBus_Message_Field * mf,
			  unsigned int pos)
{
   Ecore_List2      *l, *list;
   unsigned int      i = 0;

   list = (Ecore_List2 *) mf;

   for (l = list; l; l = l->next)
     {
	if (i == pos)
	   return _ecore_dbus_get_field(m->body,
					(Ecore_DBus_Message_Field *) l);
	i++;
     }
   return NULL;
}

void               *
ecore_dbus_get_header_field(Ecore_DBus_Message * m,
			    Ecore_DBus_Message_Field * mf,
			    Ecore_DBus_Message_Header_Field hft)
{
   Ecore_List2      *l, *list;

   list = (Ecore_List2 *) mf;
   for (l = list; l; l = l->next)
      if (((Ecore_DBus_Message_Field *) l)->hfield == hft)
	 return _ecore_dbus_get_field(m->header,
				      (Ecore_DBus_Message_Field *) l);
   return NULL;
}

/* printing functions */
/**********************/
Ecore_List2      *
_ecore_dbus_message_print_field(Ecore_List2 * l, unsigned char *buf)
{
   int                 i;
   Ecore_DBus_Message_Field *f;

   f = (Ecore_DBus_Message_Field *) l;
   switch (f->type)
     {
     case ECORE_DBUS_DATA_TYPE_BYTE:
	printf
	   ("[ecore_dbus] field BYTE		: value offset = %d value = %d\n",
	    f->offset, (char)*(buf + f->offset));
	break;
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	printf
	   ("[ecore_dbus] field PATH		: value offset = %d length = %d value = %s\n",
	    f->offset, *(buf + f->offset), (buf + f->offset + 4));
	break;
     case ECORE_DBUS_DATA_TYPE_STRING:
	printf
	   ("[ecore_dbus] field STRING	: value offset = %d length = %d value = %s\n",
	    f->offset, *(buf + f->offset), (buf + f->offset + 4));
	break;
     case ECORE_DBUS_DATA_TYPE_INT32:
	printf
	   ("[ecore_dbus] field INT32	: value offset = %d value = %ld\n",
	    f->offset, (long int)*(buf + f->offset));
	break;
     case ECORE_DBUS_DATA_TYPE_UINT32:
	printf
	   ("[ecore_dbus] field UINT32	: value offset = %d value = %lu\n",
	    f->offset, (unsigned long int)*(buf + f->offset));
	break;
     case ECORE_DBUS_DATA_TYPE_ARRAY:
	printf
	   ("[ecore_dbus] field ARRAY	: value offset = %d length = %lu elements = %u\n",
	    f->offset, *(unsigned long int *)(buf + f->offset), f->count);
	printf("[ecore_dbus] * ARRAY elements begin *\n");
	l = l->next;
	for (i = 0; i < f->count; i++)
	   l = _ecore_dbus_message_print_field(l, buf);
	printf("[ecore_dbus] * ARRAY elements end *\n");
	return l;
	break;
     default:
	printf("[ecore_dbus] field !UNKNOWN!	: %c\n", f->type);
	break;
     }
   return l->next;
}

Ecore_List2      *
_ecore_dbus_message_print_header_field(Ecore_List2 * l, unsigned char *buf)
{
   static const char  *header_fields[] =
      { "INVALID", "PATH", "INTERFACE", "MEMBER", "ERROR_NAME", "REPLY_SERIAL",
"DESTINATION", "SERIAL", "SIGNATURE" };
   Ecore_DBus_Message_Field *f;

   f = (Ecore_DBus_Message_Field *) l;
   printf("[ecore_dbus] header field %s ", header_fields[f->hfield]);
   l = _ecore_dbus_message_print_field(l, buf);
   return l;
}

void
_ecore_dbus_message_print_fields(Ecore_DBus_Message_Field * f)
{
   int               i = 0;
   Ecore_List2      *l;

   l = (Ecore_List2 *) f;
   while (l)
     {
	printf("%d\n", i);
	l = l->next;
	i++;
     }

}

void
_ecore_dbus_message_print_raw(unsigned char *msg, unsigned int msg_len)
{
   int                 i;

   printf("[ecore_dbus] raw message:\n");
   for (i = 0; i < msg_len; i++)
     {
	if (msg[i] == 0)
	   printf(",");
	else if (msg[i] < 21)
	   printf("*");
	else
	   printf("%c", msg[i]);
     }
   printf("\n");
   printf("[ecore_dbus] end raw message\n");
}

void
ecore_dbus_message_print(Ecore_DBus_Message * msg)
{
   Ecore_List2        *list;
   static const char  *msg_type[] =
      { "INVALID", "METHOD_CALL", "METHOD_RETURN", "ERROR", "SIGNAL" };
   printf("[ecore_dbus] per field message:\n");
   printf("[ecore_dbus] header fields:\n");
   /* header common fields */
   printf("[ecore_dbus] header endianess	: %c\n", msg->byte_order);
   printf("[ecore_dbus] header type     	: %s\n", msg_type[msg->type]);
   printf("[ecore_dbus] header flags    	: %c\n", msg->flags + 48);
   printf("[ecore_dbus] header protocol 	: %c\n", msg->protocol + 48);
   printf("[ecore_dbus] header hlength  	: %u\n",
	  *(unsigned int *)(msg->header + 4));
   printf("[ecore_dbus] header blength  	: %lu\n", msg->blength);
   printf("[ecore_dbus] header serial   	: %lu\n", msg->serial);

   /* header custom fields */
   list = (Ecore_List2 *) msg->header_fields;
   while (list)
     {
	list = _ecore_dbus_message_print_header_field(list, msg->header);
     }
   /* body fields */
   printf("[ecore_dbus] body fields:\n");
   list = (Ecore_List2 *) msg->body_fields;
   while (list)
     {
	list = _ecore_dbus_message_print_field(list, msg->body);
     }
   printf("[ecore_dbus] end per field message\n");

}

/* message type functions */
/**************************/
/* podria retornar el id del mensaje, para asi saber las respuestas y poderlas referenciar en el cliente */
/* la idea aca seria hacer una funcion q llame metodos remotos, or ej: ("org.freedesktop.DBus", "/org/freedesktop/DBus","org.freedesktop.DBus","AddMatch","%s",rule);*/
/* el parsing de los argumentos a la funcion podrian ser similares al printf s = string, d = int, f = float etc */

unsigned int
ecore_dbus_message_new_method_call(Ecore_DBus_Server * svr, char *service,
				   char *path, char *interface, char *method,
				   char *fmt, ...)
{
   va_list             ap;

   Ecore_DBus_Message_Field *f;

   /* init message */
   Ecore_DBus_Message *msg = _ecore_dbus_message_new(svr);

   /* common header */
   _ecore_dbus_message_common_header(msg, ECORE_DBUS_MESSAGE_TYPE_METHOD_CALL,
				     0);
   /* custom header */
   if (path)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      1,
						      ECORE_DBUS_DATA_TYPE_OBJECT_PATH,
						      path);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }

   if (interface)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      2,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      interface);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }
   if (method)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      3,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      method);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }
   if (service)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      6,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      service);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }
   if (fmt)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      8,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      fmt);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }
   _ecore_dbus_message_8byte_padding(&msg->header, &msg->hpos);
   /* header length */
   *(msg->header + 4) = msg->hlength = msg->hpos;
   /* message body */
   va_start(ap, fmt);
   while (*fmt)
     {
	switch (*fmt)
	  {
	  case ECORE_DBUS_DATA_TYPE_UINT32:
	     f = _ecore_dbus_message_marshal_uint32(&msg->body, &msg->bpos,
						    va_arg(ap, unsigned long));
	     msg->body_fields = _ecore_list2_append(msg->body_fields, f);
	     break;
	  case ECORE_DBUS_DATA_TYPE_STRING:
	     f = _ecore_dbus_message_marshal_string(&msg->body, &msg->bpos,
						    (unsigned char *)va_arg(ap, char *));
	     msg->body_fields = _ecore_list2_append(msg->body_fields, f);
	     break;
	  case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	     f = _ecore_dbus_message_marshal_path(&msg->body, &msg->bpos,
						  (unsigned char *)va_arg(ap, char *));
	     msg->body_fields = _ecore_list2_append(msg->body_fields, f);
	     break;
	  case ECORE_DBUS_DATA_TYPE_INVALID:
	  case ECORE_DBUS_DATA_TYPE_BYTE:
	  case ECORE_DBUS_DATA_TYPE_BOOLEAN:
	  case ECORE_DBUS_DATA_TYPE_INT32:
	  case ECORE_DBUS_DATA_TYPE_INT64:
	  case ECORE_DBUS_DATA_TYPE_UINT64:
	  case ECORE_DBUS_DATA_TYPE_DOUBLE:
	  case ECORE_DBUS_DATA_TYPE_CUSTOM:
	  case ECORE_DBUS_DATA_TYPE_ARRAY:
	  case ECORE_DBUS_DATA_TYPE_DICT:
	  default:
	     printf("[ecore_dbus] unknown/unhandled data type %c\n", *fmt);
	     break;
	  }
	fmt++;
     }
   va_end(ap);
   *(unsigned int *)(msg->header + 8) = msg->blength = msg->bpos;
   /* show message */
   /*ecore_dbus_message_print(msg);
    * _ecore_dbus_message_print_raw(msg->header,msg->hlength);
    * _ecore_dbus_message_print_raw(msg->body,msg->blength); */
   /* send message */
   ecore_dbus_server_send(svr, (char *)msg->header, msg->hlength);
   if (msg->body)
      ecore_dbus_server_send(svr, (char *)msg->body, msg->blength);
   /* free data TODO free the list of fields */
   /*for(i=0; i<8; i++)
    * free(msg->header_fields[i]);
    * free(msg->buf);
    * free(msg); */
   return msg->serial;
}

/*******************/
/* ecore_dbus_auth */
/*******************/

static char        *_ecore_dbus_getuid(void);
static char        *_ecore_dbus_hex_encode(char *src_str);

/* helper functions */
static char        *
_ecore_dbus_getuid(void)
{
   /* this calculation is from comp.lang.c faq */
#define MAX_LONG_LEN ((sizeof (long) * 8 + 2) / 3 + 1)	/* +1 for '-' */
   int                 len;
   char               *uid;
   char               *tmp;

   tmp = (char *)malloc(MAX_LONG_LEN);
   len = snprintf(tmp, MAX_LONG_LEN, "%ld", (long) getuid());
   uid = (char *)malloc(len + 1);
   uid = memcpy(uid, tmp, len);
   uid[len] = '\0';

   free(tmp);
   return uid;
}

/* encodes a string into a string of hex values	*/
/* each byte is two hex digits			*/
static char        *
_ecore_dbus_hex_encode(char *src_str)
{
   const char          hexdigits[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'a', 'b', 'c', 'd', 'e', 'f'
   };
   char               *enc_str;
   char               *p;
   char               *end;
   int                 len;
   int                 i = 0;

   len = strlen(src_str);
   p = src_str;
   end = p + len;

   enc_str = malloc(2 * len + 1);
   while (p != end)
     {
	enc_str[i] = hexdigits[(*p >> 4)];
	i++;
	enc_str[i] = hexdigits[(*p & 0x0f)];
	i++;
	p++;
     }
   enc_str[i] = '\0';
   return enc_str;
}

unsigned char *
_ecore_dbus_auth_external(void *data)
{
   char          *uid, *enc_uid, *msg;

   uid = _ecore_dbus_getuid();
   enc_uid = _ecore_dbus_hex_encode(uid);
   free(uid);
   msg = malloc(strlen(enc_uid) + 17);
   sprintf(msg, "AUTH EXTERNAL %s\r\n", enc_uid);
   free(enc_uid);
   return (unsigned char *)msg;
}

/*****************************/
/* main ecore_dbus functions */
/*****************************/

int                 ECORE_DBUS_EVENT_CLIENT_ADD = 0;
int                 ECORE_DBUS_EVENT_CLIENT_DEL = 0;
int                 ECORE_DBUS_EVENT_SERVER_ADD = 0;
int                 ECORE_DBUS_EVENT_SERVER_DEL = 0;
int                 ECORE_DBUS_EVENT_CLIENT_DATA = 0;
int                 ECORE_DBUS_EVENT_SERVER_DATA = 0;

static const Ecore_DBus_Auth auths[] = {
   {"EXTERNAL", 1, {_ecore_dbus_auth_external, NULL, NULL, NULL, NULL}},
   {"MAGIC_COOKIE", 0, {NULL, NULL, NULL, NULL, NULL}},
   {"DBUS_COOKIE_SHA1", 0, {NULL, NULL, NULL, NULL, NULL}},
   {"KERBEROS_V4", 0, {NULL, NULL, NULL, NULL, NULL}},
   {"SKEY", 0, {NULL, NULL, NULL, NULL, NULL}},
};

static int                  init_count = 0;
static Ecore_List          *servers = NULL;
static Ecore_Event_Handler *handler[6];

static int          _ecore_dbus_event_client_add(void *data, int ev_type,
						 void *ev);
static int          _ecore_dbus_event_client_del(void *data, int ev_type,
						 void *ev);
static int          _ecore_dbus_event_server_add(void *data, int ev_type,
						 void *ev);
static int          _ecore_dbus_event_server_del(void *data, int ev_type,
						 void *ev);
static int          _ecore_dbus_event_client_data(void *data, int ev_type,
						  void *ev);
static int          _ecore_dbus_event_server_data(void *data, int ev_type,
						  void *ev);
/* helpers */
void                _ecore_dbus_message_free(void *data, void *ev);

int
ecore_dbus_init(void)
{
   int i = 0;

   if (++init_count != 1) return init_count;

   ecore_con_init();

   ECORE_DBUS_EVENT_CLIENT_ADD = ecore_event_type_new();
   ECORE_DBUS_EVENT_CLIENT_DEL = ecore_event_type_new();
   ECORE_DBUS_EVENT_SERVER_ADD = ecore_event_type_new();
   ECORE_DBUS_EVENT_SERVER_DEL = ecore_event_type_new();
   ECORE_DBUS_EVENT_CLIENT_DATA = ecore_event_type_new();
   ECORE_DBUS_EVENT_SERVER_DATA = ecore_event_type_new();

   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,
					  _ecore_dbus_event_client_add, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,
					  _ecore_dbus_event_client_del, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
					  _ecore_dbus_event_server_add, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
					  _ecore_dbus_event_server_del, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
					  _ecore_dbus_event_client_data, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
					  _ecore_dbus_event_server_data, NULL);

   return init_count;
}

int
ecore_dbus_shutdown(void)
{
   int i = 0;

   if (--init_count != 0) return init_count;

   for (i = 0; i < 6; i++)
     ecore_event_handler_del(handler[i]);

   ecore_con_shutdown();

   return init_count;
}

Ecore_DBus_Server  *
ecore_dbus_server_connect(Ecore_DBus_Type compl_type, char *name, int port,
			  const void *data)
{
   /* TODO */
   /* se tiene q conectar por direccion (unix socket path) */
   /* o por BUS directo                                    */

   Ecore_DBus_Server  *svr;
   Ecore_DBus_Type     type;
   Ecore_Con_Type      extra = 0;

   svr = calloc(1, sizeof(Ecore_DBus_Server));
   if (!svr)
      return NULL;
   type = compl_type;
   switch (type)
     {
     case ECORE_DBUS_BUS_SESSION:
	svr->server =
	   ecore_con_server_connect(ECORE_CON_LOCAL_USER | extra, name, port,
				    svr);
	break;
     case ECORE_DBUS_BUS_SYSTEM:
	svr->server =
	   ecore_con_server_connect(ECORE_CON_LOCAL_USER | extra, name, port,
				    svr);
	break;

     default:
	free(svr);
	return NULL;
     }
   if (!svr->server)
     {
	printf("no hay servidor\n");
	free(svr);
	return NULL;
     }
   svr->authenticated = 0;
   svr->cnt_msg = 0;
   svr->auth_type = -1;
   svr->auth_type_transaction = 0;
   servers = _ecore_list2_append(servers, svr);

   return svr;
}

int
ecore_dbus_server_send(Ecore_DBus_Server * svr, char *command, int length)
{
   int                 ret;

   ret = ecore_con_server_send(svr->server, command, length);
   printf
      ("[ecore_dbus] ecore_dbus_server: %p ecore_con_server: %p sent %d of %d bytes\n",
       svr, svr->server, ret, length);
   return ret;
}

/* private functions */
static int
_ecore_dbus_event_client_add(void *data, int ev_type, void *ev)
{
   printf("CLIENT ADDED \n");
   printf("this line should appear\n");
   return 1;
}

static int
_ecore_dbus_event_client_del(void *data, int ev_type, void *ev)
{
   return 0;
}

static int
_ecore_dbus_event_server_add(void *data, int ev_type, void *ev)
{

   Ecore_DBus_Event_Server_Add *e;

   e = ev;
   if (!_ecore_list2_find(servers, ecore_con_server_data_get(e->server)))
      return 1;
   {
      Ecore_DBus_Server  *svr;

      svr = ecore_con_server_data_get(e->server);
      ecore_dbus_server_send(svr, "\0", 1);
      ecore_dbus_server_send(svr, "AUTH\r\n", 6);
      printf("[ecore_dbus] begining auth process\n");
   }
   return 0;
}

static int
_ecore_dbus_event_server_del(void *udata, int ev_type, void *ev)
{
   Ecore_Con_Event_Server_Del *e;

   e = ev;
   if (!_ecore_list2_find(servers, ecore_con_server_data_get(e->server)))
      return 1;
   {
      Ecore_DBus_Server  *svr;

      svr = ecore_con_server_data_get(e->server);
      {
	 Ecore_DBus_Event_Server_Del *e2;

	 e2 = calloc(1, sizeof(Ecore_DBus_Event_Server_Del));
	 if (e2)
	   {
	      e2->server = svr;
	      ecore_event_add(ECORE_DBUS_EVENT_SERVER_DEL, e2, NULL, NULL);
	   }
      }
   }
   return 0;
}

static int
_ecore_dbus_event_client_data(void *udata, int ev_type, void *ev)
{
   printf("CLIENT DATA");
   return 1;
}

static int
_ecore_dbus_event_server_data(void *udata, int ev_type, void *ev)
{
   Ecore_Con_Event_Server_Data *e;

   e = ev;
   if (!_ecore_list2_find(servers, ecore_con_server_data_get(e->server)))
      return 1;

   {
      Ecore_DBus_Server  *svr;
      Ecore_DBus_Event_Server_Add *svr_add;

      svr = ecore_con_server_data_get(e->server);

      /* authentication protocol */
      /***************************/
      if (!svr->authenticated)
	{
	   const Ecore_DBus_Auth *auth;
	   Ecore_DBus_Auth_Transaction trans;

	   if (!strncmp(e->data, "OK", 2))
	     {
		printf("[ecore_dbus] auth type %s successful\n",
		       auths[svr->auth_type].name);
		ecore_dbus_server_send(svr, "BEGIN\r\n", 7);
		svr->authenticated = 1;
		svr_add = malloc(sizeof(Ecore_DBus_Event_Server_Add));
		svr_add->server = svr;
		ecore_event_add(ECORE_DBUS_EVENT_SERVER_ADD, svr_add, NULL,
				NULL);
	     }
	   if (!strncmp(e->data, "DATA", 4))
	     {
		printf("[ecore_dbus] requering data (unaivable)\n");
	     }
	   if (!strncmp(e->data, "ERROR", 5))
	     {
		printf("[ecore_dbus] auth process error\n");
	     }
	   if (!strncmp(e->data, "REJECTED", 8))
	     {
		unsigned char      *msg;

		if (svr->auth_type >= 0)
		   printf("[ecore_dbus] auth type %s rejected\n",
			  auths[svr->auth_type].name);
		svr->auth_type++;
		auth = &auths[svr->auth_type];
		trans = auth->transactions[0];
		printf("[ecore_dbus] auth type %s started\n", auth->name);
		msg = trans(NULL);
		ecore_dbus_server_send(svr, (char *)msg, strlen((char *)msg));
		free(msg);

	     }
	}
      /* message protocol */
      /********************/
      else
	{
	   Ecore_DBus_Message *msg;
	   unsigned int        offset = 0;

	   printf("[ecore_dbus] received server data, %d bytes\n", e->size);
	   while (e->size)
	     {
		msg = _ecore_dbus_message_unmarshal(svr, e->data + offset);
		if (msg == NULL)
		   break;
		offset += msg->hlength + msg->blength;
		e->size -= msg->hlength + msg->blength;
		printf("[ecore_dbus] dbus message length %lu bytes, still %d\n",
		       msg->hlength + msg->blength, e->size);
		/*ecore_dbus_message_print(msg); */
		ecore_event_add(ECORE_DBUS_EVENT_SERVER_DATA, msg,
				_ecore_dbus_message_free, NULL);

	     }
	}
   }
   return 1;
}
