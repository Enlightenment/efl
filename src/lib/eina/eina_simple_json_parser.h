/* EINA - EFL data type library
 * Copyright (C) 2016 Expertise Solutions
 *                    Larry Lira Jr
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_SIMPLE_JSON_H_
#define EINA_SIMPLE_JSON_H_

#include "eina_config.h"

#include "eina_types.h"

typedef enum _Eina_Simple_JSON_Type
{
   EINA_SIMPLE_JSON_ERROR,
   EINA_SIMPLE_JSON_OBJECT,
   EINA_SIMPLE_JSON_ARRAY,
   EINA_SIMPLE_JSON_OPEN,
   EINA_SIMPLE_JSON_CLOSE
} Eina_Simple_JSON_Type;

typedef enum _Eina_Simple_JSON_Value_Type
{
   EINA_SIMPLE_JSON_VALUE_OBJECT,
   EINA_SIMPLE_JSON_VALUE_ARRAY,
   EINA_SIMPLE_JSON_VALUE_STRING,
   EINA_SIMPLE_JSON_VALUE_INT,
   EINA_SIMPLE_JSON_VALUE_DOUBLE,
   EINA_SIMPLE_JSON_VALUE_BOOLEAN,
   EINA_SIMPLE_JSON_VALUE_NULL,
   EINA_SIMPLE_JSON_VALUE_ERROR
} Eina_Simple_JSON_Value_Type;

typedef Eina_Bool (*Eina_Simple_JSON_Cb)(void *data, Eina_Simple_JSON_Type type, const char *content, unsigned length);
typedef Eina_Bool (*Eina_Simple_JSON_Object_Cb)(void *data, Eina_Simple_JSON_Value_Type type, const char *key, const char *content, unsigned length);
typedef Eina_Bool (*Eina_Simple_JSON_Array_Cb)(void *data, Eina_Simple_JSON_Value_Type type, const char *content, unsigned length);

/**
 * Parse a section of JSON string text
 *
 * @param buf the input string. May not contain \0 terminator.
 * @param buflen the input string size.
 * @param func what to call back while parse to do some action. Whenever this
 *        function return #EINA_FALSE the parser will abort.
 * @param data what to give as context to @a func.
 *
 * @return #EINA_TRUE on success or #EINA_FALSE if it was aborted by user or parsing error.
 */
EAPI Eina_Bool eina_simple_json_parse(const char *buf, unsigned buflen,
						Eina_Simple_JSON_Cb func, const void *data);

/**
 * Given a buffer with json objects members, parse them to key:value pairs.
 *
 * @param buf the input string. May not contain \0 terminator.
 * @param buflen the input string size.
 * @param func what to call back while parse to do some action.
 * @param data data to pass to the callback function.
 *
 * @return #EINA_TRUE on success or #EINA_FALSE if it was aborted by user or parsing error.
 */
EAPI Eina_Bool eina_simple_json_object_parse(const char *buf, unsigned buflen,
						Eina_Simple_JSON_Object_Cb func, const void *data);

/**
 * Given a buffer with json array elements, parse them to values.
 *
 * @param buf the input string. May not contain \0 terminator.
 * @param buflen the input string size.
 * @param func what to call back while parse to do some action.
 * @param data data to pass to the callback function.
 *
 * @return #EINA_TRUE on success or #EINA_FALSE if it was aborted by user or parsing error.
 */
EAPI Eina_Bool eina_simple_json_array_parse(const char *buf, unsigned buflen,
						Eina_Simple_JSON_Array_Cb func, const void *data);


#endif /* EINA_SIMPLE_JSON_H_ */
