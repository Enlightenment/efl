/* EINA - EFL data type library
 * Copyright (C) 2011 Cedric Bail
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

#ifndef EINA_OBJECT_H__
#define EINA_OBJECT_H__

#include "eina_config.h"

#include "eina_types.h"

/**
 * @addtogroup Eina_Object Object
 *
 * @brief These functions manage object providing pointer checking
 * and memory repacking.
 *
 * Each Eina_Object is in fact only an ID and a generation count. This
 * make it possible to check if the ID is allocated by checking that it
 * is inside the boudary limit of the allocated range. The generation
 * count, give the possibility to check that we use a valid alive pointer
 * as generation is increased each time an object is allocated/destroyed.
 * And finally it provide type checking against Eina_Class.
 *
 * It is also to define link between object, then when father get deleted
 * all child will get deleted and their respective destructor will be called.
 *
 * @{
 */

typedef struct _Eina_Class Eina_Class;
typedef struct _Eina_Object Eina_Object;

typedef void (*Eina_Class_Callback)(Eina_Class *class,
				    void *object,
				    void *data);

Eina_Class *eina_class_new(const char *name,
			   unsigned int class_size,
			   unsigned int pool_size,
			   Eina_Class_Callback constructor,
			   Eina_Class_Callback destructor,
			   Eina_Class *parent,
			   void *data);
const char *eina_class_name_get(Eina_Class *class);
unsigned int eina_class_size_get(Eina_Class *class);
unsigned int eina_class_object_size_get(Eina_Class *class);
void eina_class_del(Eina_Class *class);
void eina_class_repack(Eina_Class *class);

Eina_Object *eina_object_add(Eina_Class *class);
void *eina_object_pointer_get(Eina_Class *class,
			      Eina_Object *object);
void eina_object_del(Eina_Class *class, 
		     Eina_Object *object);

Eina_Bool eina_object_parent_set(Eina_Class *parent_class, Eina_Object *parent,
				 Eina_Class *object_class, Eina_Object *object);
Eina_Object *eina_object_parent_get(Eina_Class *class, Eina_Object *object);

/**
 * @}
 */

#endif	   
