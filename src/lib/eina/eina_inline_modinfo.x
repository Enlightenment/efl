/* EINA - EFL data type library
 * Copyright (C) 2016 Amitesh Singh
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

#ifndef EINA_INLINE_MODINFO_X_
#define EINA_INLINE_MODINFO_X_

#define __EINA_MODINFO_CONCAT(a, b) a##b
#define _EINA_MODINFO_CONCAT(a, b) __EINA_MODINFO_CONCAT(a, b)
#define __EINA_MODULE_UNIQUE_ID(id) _EINA_MODINFO_CONCAT(__EINA_MODULE_UNIQUE_ID_, id)

#define _EINA_MODINFO(name, info) \
  EAPI const char __EINA_MODULE_UNIQUE_ID(name)[] \
__attribute__((__used__)) __attribute__((unused, aligned(1))) = info;
#define EINA_MODINFO(tag, info) _EINA_MODINFO(tag, info)

/**
  * @defgroup Eina_Module_Group Module
  *  
  * These macros allow you to define module informations like author/description/version/license.
  * eina_modinfo - shows information about an eina module.
  * eina_modinfo pulls out information from the eina modules given on command line.
  *
  * $ eina_modinfo module.so
  * version: 0.1
  * description:   Entry test
  * license: GPLv2
  * author:  Enlightenment Community
  *
  */

/** 
  * @defgroup Eina_Module_Group Module
  *  
  * This macro is used for defining license.
  *
  */
#define EINA_MODULE_LICENSE(_license) EINA_MODINFO(license, _license)
/**
  * @defgroup Eina_Module_Group Module
  *  
  * This macro is used for defining author
  * Use "name <email>" or just "name"
  * for multiple authors, use multiple lines like below
  *        EINA_MODULE_AUTHOR("Author 1 <author1.email>\n
                              "Author 2 <author2.email>");
  */
#define EINA_MODULE_AUTHOR(_author) EINA_MODINFO(author, _author)
/**
  * @defgroup Eina_Module_Group Module
  *  
  * This macro is used for defining version.
  */
#define EINA_MODULE_VERSION(_ver) EINA_MODINFO(ver, _ver)
/**
  * @defgroup Eina_Module_Group Module
  *  
  * This macro is used for defining description.
  * Explain what your module does.
  */
#define EINA_MODULE_DESCRIPTION(_desc) EINA_MODINFO(desc, _desc)

#endif

