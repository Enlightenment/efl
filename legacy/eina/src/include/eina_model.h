/* EINA - EFL data type library
 * Copyright (C) 2012 ProFUSION embedded systems
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

#ifndef EINA_MODEL_H_
#define EINA_MODEL_H_

#include "eina_types.h"
#include "eina_value.h"
#include <stdarg.h>

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @since 1.2
 *
 * @{
 */

/**
 * @addtogroup Eina_Containers_Group Containers
 *
 * @{
 */

/**
 * @defgroup Eina_Model_Group Data Model API.
 *
 * Abstracts data access to hierarchical data in an efficient way,
 * extensible to different backing stores such as database or remote
 * access.
 *
 * It is heavily based on #Eina_Value, as properties are exchanged
 * using this data type as interface, although internally models may
 * store them as they want. See @ref Eina_Value_Group.
 *
 * Although extensible and easy to optimize, a simple generic type is
 * provided as #EINA_MODEL_TYPE_GENERIC. It is recommended that people
 * use it during development, get the logic right and just then
 * optimize what is needed (properties or children management).
 *
 * @code
 *
 *    static void _cb_on_deleted(void *data, Eina_Model *model, const Eina_Model_Event_Description *desc, void *event_info)
 *    {
 *       printf("deleted %p\n", model);
 *    }
 *
 *    int main(void)
 *    {
 *       Eina_Model *m;
 *       char *s;
 *       int i;
 *
 *       eina_init();
 *
 *       m = eina_model_new(EINA_MODEL_TYPE_GENERIC);
 *
 *       eina_model_event_callback_add(m, "deleted", _cb_on_deleted, NULL);
 *
 *       for (i = 0; i < 5; i++)
 *         {
 *            Eina_Value val;
 *            char name[2] = {'a'+ i, 0};
 *            eina_value_setup(&val, EINA_VALUE_TYPE_INT);
 *            eina_value_set(&val, i);
 *            eina_model_property_set(m, name, &val);
 *            eina_value_flush(&val);
 *         }
 *
 *       for (i = 0; i < 5; i++)
 *         {
 *            Eina_Value val;
 *            Eina_Model *c = eina_model_new(EINA_MODEL_TYPE_GENERIC);
 *            eina_value_setup(&val, EINA_VALUE_TYPE_INT);
 *            eina_value_set(&val, i);
 *            eina_model_property_set(c, "x", &val);
 *
 *            eina_model_event_callback_add(c, "deleted", _cb_on_deleted, NULL);
 *
 *            eina_model_child_append(m, c);
 *            eina_model_unref(c);
 *            eina_value_flush(&val);
 *         }
 *
 *       s = eina_model_to_string(m);
 *       printf("model as string:\n%s\n", s);
 *
 *       free(s);
 *       eina_model_unref(m);
 *
 *       return 0;
 *    }
 * @endcode
 *
 * @{
 */

/**
 * @var EINA_ERROR_MODEL_FAILED
 * Defined when model-specific errors happens.
 */
EAPI extern Eina_Error EINA_ERROR_MODEL_FAILED;

/**
 * @var EINA_ERROR_MODEL_METHOD_MISSING
 * Defined when model-specific errors happens.
 */
EAPI extern Eina_Error EINA_ERROR_MODEL_METHOD_MISSING;

/**
 * @typedef Eina_Model
 * Data Model Object.
 *
 * @since 1.2
 */
typedef struct _Eina_Model Eina_Model;


/**
 * @typedef Eina_Model_Type
 * Data Model Type.
 *
 * @since 1.2
 */
typedef struct _Eina_Model_Type Eina_Model_Type;

/**
 * @typedef Eina_Model_Interface
 * Data Model Interface.
 *
 * @since 1.2
 */
typedef struct _Eina_Model_Interface Eina_Model_Interface;

/**
 * @typedef Eina_Model_Event_Description
 * Data Model Event Description.
 *
 * @since 1.2
 */
typedef struct _Eina_Model_Event_Description Eina_Model_Event_Description;

/**
 * @typedef Eina_Model_Event_Cb
 * Notifies of events in this model.
 *
 * @since 1.2
 */
typedef void (*Eina_Model_Event_Cb)(void *data, Eina_Model *model, const Eina_Model_Event_Description *desc, void *event_info);

EAPI Eina_Model *eina_model_new(const Eina_Model_Type *type);
EAPI void eina_model_del(Eina_Model *model) EINA_ARG_NONNULL(1);

EAPI const Eina_Model_Type *eina_model_type_get(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI const Eina_Model_Interface *eina_model_interface_get(const Eina_Model *model,
                                                          const char *name) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI Eina_Bool eina_model_instance_check(const Eina_Model *model,
                                         const Eina_Model_Type *type) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI Eina_Bool eina_model_interface_implemented(const Eina_Model *model, const Eina_Model_Interface *iface) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI Eina_Model *eina_model_ref(Eina_Model *model) EINA_ARG_NONNULL(1);
EAPI void eina_model_unref(Eina_Model *model) EINA_ARG_NONNULL(1);
EAPI int eina_model_refcount(const Eina_Model *model) EINA_ARG_NONNULL(1);

EAPI Eina_Bool eina_model_event_callback_add(Eina_Model *model,
                                             const char *event_name,
                                             Eina_Model_Event_Cb cb,
                                             const void *data) EINA_ARG_NONNULL(1, 2, 3);
EAPI Eina_Bool eina_model_event_callback_del(Eina_Model *model,
                                             const char *event_name,
                                             Eina_Model_Event_Cb cb,
                                             const void *data) EINA_ARG_NONNULL(1, 2, 3);

EAPI const Eina_Model_Event_Description *eina_model_event_description_get(const Eina_Model *model,
                                                                           const char *event_name) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI Eina_List *eina_model_event_names_list_get(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI void eina_model_event_names_list_free(Eina_List *list);

EAPI Eina_Bool eina_model_event_callback_call(Eina_Model *model,
                                              const char *name,
                                              const void *event_info) EINA_ARG_NONNULL(1, 2);

EAPI int eina_model_event_callback_freeze(Eina_Model *model,
                                          const char *name) EINA_ARG_NONNULL(1, 2);
EAPI int eina_model_event_callback_thaw(Eina_Model *model,
                                        const char *name) EINA_ARG_NONNULL(1, 2);


EAPI Eina_Model *eina_model_copy(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_MALLOC;
EAPI Eina_Model *eina_model_deep_copy(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

EAPI int eina_model_compare(const Eina_Model *a, const Eina_Model *b) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2);

EAPI Eina_Bool eina_model_load(Eina_Model *model) EINA_ARG_NONNULL(1);
EAPI Eina_Bool eina_model_unload(Eina_Model *model) EINA_ARG_NONNULL(1);


EAPI Eina_Bool eina_model_property_get(const Eina_Model *model,
                                       const char *name,
                                       Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3);
EAPI Eina_Bool eina_model_property_set(Eina_Model *model,
                                       const char *name,
                                       const Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3);
EAPI Eina_Bool eina_model_property_del(Eina_Model *model,
                                       const char *name) EINA_ARG_NONNULL(1, 2);

EAPI Eina_List *eina_model_properties_names_list_get(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI void eina_model_properties_names_list_free(Eina_List *list);

EAPI int eina_model_child_count(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the child at a given position from a model.
 * @param model the model instance.
 * @param position index of child to get.
 * @return child instance with reference @b increased, or @c NULL on error.
 *
 * The given @a position must be valid, otherwise it may fail and
 * return @c NULL, one should check for a valid position with
 * eina_model_child_count().
 *
 * The returned model has its reference increased, you must release it
 * with eina_model_unref(). This convention is imposed to avoid the
 * object being removed before the caller function has time to use it.
 */
EAPI Eina_Model *eina_model_child_get(const Eina_Model *model,
                                      unsigned int position) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Set the child at a given position from a model.
 * @param model the model instance.
 * @param position index of child to set.
 * @param child the child to use at given position.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * The given @a position must be valid, otherwise it may fail and
 * return #EINA_FALSE, one should check for a valid position with
 * eina_model_child_count().
 *
 * The existing child is replaced. Its reference will be decreased
 * automatically. To insert a new item instead of replacing, use
 * eina_model_child_insert_at() or eina_model_child_append().
 *
 * The given model will be adopted by @a model, that is, the @a child
 * will have its reference increased if this call succeeds.
 */
EAPI Eina_Bool eina_model_child_set(Eina_Model *model,
                                    unsigned int position,
                                    Eina_Model *child) EINA_ARG_NONNULL(1, 3);

EAPI Eina_Bool eina_model_child_del(Eina_Model *model,
                                    unsigned int position) EINA_ARG_NONNULL(1);

EAPI Eina_Bool eina_model_child_insert_at(Eina_Model *model,
                                          unsigned int position,
                                          Eina_Model *child) EINA_ARG_NONNULL(1, 3);

EAPI int eina_model_child_append(Eina_Model *model,
                                 Eina_Model *child) EINA_ARG_NONNULL(1, 2);

EAPI int eina_model_child_find(const Eina_Model *model,
                               unsigned int start_position,
                               const Eina_Model *other) EINA_ARG_NONNULL(1, 3) EINA_WARN_UNUSED_RESULT;

EAPI int eina_model_child_search(const Eina_Model *model,
                                 unsigned int start_position,
                                 Eina_Each_Cb match,
                                 const void *data) EINA_ARG_NONNULL(1, 3) EINA_WARN_UNUSED_RESULT;

EAPI Eina_Bool eina_model_child_sort(Eina_Model *model,
                                     Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2);


/**
 * @brief create an iterator that outputs a child model on each iteration.
 * @param model the model instance.
 * @return newly created iterator instance on success or @c NULL on failure.
 *
 * Each iteration output a child model with reference @b increased!
 * You must call eina_model_unref() after you're done with it.
 *
 * @code
 *  Eina_Model *child;
 *  Eina_Iterator *it = eina_model_child_iterator_get(model);
 *  EINA_ITERATOR_FOREACH(it, child)
 *    {
 *       use_child(child);
 *       eina_model_unref(child);
 *    }
 *  eina_iterator_free(it);
 * @endcode
 *
 * @see eina_model_child_slice_iterator_get()
 * @see eina_model_child_reversed_iterator_get()
 * @see eina_model_child_sorted_iterator_get()
 * @see eina_model_child_filtered_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_iterator_get(Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Iterator *eina_model_child_slice_iterator_get(Eina_Model *model,
                                                        unsigned int start,
                                                        unsigned int count) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief create an iterator that outputs a child model in reversed order.
 * @param model the model instance.
 * @return newly created iterator instance on success or @c NULL on failure.
 *
 * Each iteration output a child model with reference @b increased!
 * You must call eina_model_unref() after you're done with it.
 *
 * The order is reversed, that is, the last element is outputted first.
 *
 * @code
 *  Eina_Model *child;
 *  Eina_Iterator *it = eina_model_child_reversed_iterator_get(model);
 *  EINA_ITERATOR_FOREACH(it, child)
 *    {
 *       use_child(child);
 *       eina_model_unref(child);
 *    }
 *  eina_iterator_free(it);
 * @endcode
 *
 * @see eina_model_child_slice_iterator_get()
 * @see eina_model_child_reversed_iterator_get()
 * @see eina_model_child_sorted_iterator_get()
 * @see eina_model_child_filtered_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_reversed_iterator_get(Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

EAPI Eina_Iterator *eina_model_child_slice_reversed_iterator_get(Eina_Model *model,
                                                                 unsigned int start,
                                                                 unsigned int count) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief create an iterator that outputs a child model using sort criteria.
 * @param model the model instance.
 * @param compare compare function to use as sort criteria.
 * @return newly created iterator instance on success or @c NULL on failure.
 *
 * Each iteration output a child model with reference @b increased!
 * You must call eina_model_unref() after you're done with it.
 *
 * The sort will not affect the main object @a model, it's just a view
 * of it.
 *
 * @code
 *  Eina_Model *child;
 *  Eina_Iterator *it = eina_model_child_sorted_iterator_get(model, EINA_COMPARE_CB(eina_model_compare));
 *  EINA_ITERATOR_FOREACH(it, child)
 *    {
 *       use_child(child);
 *       eina_model_unref(child);
 *    }
 *  eina_iterator_free(it);
 * @endcode
 *
 * @see eina_model_child_slice_iterator_get()
 * @see eina_model_child_reversed_iterator_get()
 * @see eina_model_child_sorted_iterator_get()
 * @see eina_model_child_filtered_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_sorted_iterator_get(Eina_Model *model,
                                                         Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

EAPI Eina_Iterator *eina_model_child_slice_sorted_iterator_get(Eina_Model *model,
                                                               unsigned int start,
                                                               unsigned int count,
                                                               Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief create an iterator that indexes of children that matches.
 * @param model the model instance.
 * @param match function to select children.
 * @param data extra context given to @a match function.
 * @return newly created iterator instance on success or @c NULL on failure.
 *
 * Unlike other iterators, each iteration output an integer index!
 * This is useful if you want to highlight the matching model
 * somewhere else.
 *
 * @code
 *  unsigned int idx;
 *  Eina_Iterator *it = eina_model_child_filtered_iterator_get(model, filter, ctx);
 *  EINA_ITERATOR_FOREACH(it, idx)
 *    {
 *       Eina_Model *child = eina_model_child_get(model, idx);
 *       printf("matches at %u %p\n", idx, child);
 *       eina_model_unref(child);
 *    }
 *  eina_iterator_free(it);
 * @endcode
 *
 * @see eina_model_child_slice_iterator_get()
 * @see eina_model_child_reversed_iterator_get()
 * @see eina_model_child_sorted_iterator_get()
 * @see eina_model_child_filtered_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_filtered_iterator_get(Eina_Model *model,
                                                           Eina_Each_Cb match,
                                                           const void *data) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

EAPI Eina_Iterator *eina_model_child_slice_filtered_iterator_get(Eina_Model *model,
                                                                 unsigned int start,
                                                                 unsigned int count,
                                                                 Eina_Each_Cb match,
                                                                 const void *data) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Convert model to string.
 * @param model the model instance.
 * @return newly allocated memory or @c NULL on failure.
 * @since 1.2
 */
EAPI char *eina_model_to_string(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * @defgroup Eina_Model_Type_Group Data Model Type management
 *
 * @{
 */

/**
 * @struct _Eina_Model_Type
 * API to access models.
 *
 * The methods @c setup, @c flush, @c constructor, @c destructor and
 * @c property_get are mandatory and must exist, otherwise type cannot
 * be used.
 *
 * Each type of the hierarchy and each interface will get its own
 * private data of size @c private_size (defined at each subtype or
 * interface), this can be retrieved with
 * eina_model_type_private_data_get() and
 * eina_model_interface_private_data_get().
 *
 * Private are created @b automatically and should be setup with @c
 * setup and flushed with @c flush. All types (or interfaces)
 * functions that exist are called! Don't call your parent's @c setup or
 * @c flush! The setup is done from parent to child. Flush is done from
 * child to parent.
 *
 * After memory setup was done, @c constructor of the toplevel type
 * defining it is called. If desired it may call parent's constructor
 * in whatever order is desired. This may be used to create
 * properties, children and may use parent's data if needed. Just the
 * topmost type constructor is called, if interface constructors
 * should be called, do them in the desired order from the type
 * constructor.
 *
 * When the model is deleted, explicitly with eina_model_del() or
 * implicitly with eina_model_unref() on the last reference, the @c
 * destructor is called. It must release references to other
 * models. When the last reference is dropped, every @c flush is
 * called from child to parent, then memory is freed. Just the topmost
 * type destructor is called, if interface destructors should be
 * called, do them in the desired order from the type destructor.
 *
 * @note a runtime check will enforce just types with ABI version
 *       #EINA_MODEL_TYPE_VERSION are used by comparing with the @c version
 *       member.
 *
 * @since 1.2
 */
struct _Eina_Model_Type
{
   /**
    * @def EINA_MODEL_TYPE_VERSION
    * Current API version, used to validate type.
    */
#define EINA_MODEL_TYPE_VERSION (1)
   unsigned int version; /**< must be #EINA_MODEL_TYPE_VERSION */
   unsigned int private_size; /**< used to allocate type private data */
   const char *name; /**< name for debug and introspection */
   const Eina_Model_Type *parent; /**< parent type, must be EINA_MODEL_TYPE_BASE or a child of */
   const Eina_Model_Interface **interfaces; /**< null terminated array of interfaces */
   const Eina_Model_Event_Description *events; /**< null terminated array of events */
   Eina_Bool (*setup)(Eina_Model *model); /**< setup type private data, do @b not call parent type setup! */
   Eina_Bool (*flush)(Eina_Model *model); /**< flush type private data, do @b not call parent type flush! */
   Eina_Bool (*constructor)(Eina_Model *model); /**< construct type instance, setup was already called. Should call parent's or interfaces' constructor if needed */
   Eina_Bool (*destructor)(Eina_Model *model); /**< destruct type instance, flush will be called after it. Should call parent's or interfaces' destructor if needed. Release reference to other models here. */
   Eina_Bool (*copy)(const Eina_Model *src, Eina_Model *dst); /**< copy type private data, do @b not call parent type copy! */
   Eina_Bool (*deep_copy)(const Eina_Model *src, Eina_Model *dst); /**< deep copy type private data, do @b not call parent type deep copy! */
   Eina_Bool (*compare)(const Eina_Model *a, const Eina_Model *b, int *cmp);
   Eina_Bool (*load)(Eina_Model *model);
   Eina_Bool (*unload)(Eina_Model *model);
   Eina_Bool (*property_get)(const Eina_Model *model, const char *name, Eina_Value *value);
   Eina_Bool (*property_set)(Eina_Model *model, const char *name, const Eina_Value *value);
   Eina_Bool (*property_del)(Eina_Model *model, const char *name);
   Eina_List *(*properties_names_list_get)(const Eina_Model *model); /**< list of stringshare */
   int (*child_count)(const Eina_Model *model);
   Eina_Model *(*child_get)(const Eina_Model *model, unsigned int position);
   Eina_Bool (*child_set)(Eina_Model *model, unsigned int position, Eina_Model *child);
   Eina_Bool (*child_del)(Eina_Model *model, unsigned int position);
   Eina_Bool (*child_insert_at)(Eina_Model *model, unsigned int position, Eina_Model *child);
   int (*child_find)(const Eina_Model *model, unsigned int start_position, const Eina_Model *other);
   int (*child_search)(const Eina_Model *model, unsigned int start_position, Eina_Each_Cb match, const void *data);
   void (*child_sort)(Eina_Model *model, Eina_Compare_Cb compare);
   Eina_Iterator *(*child_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count);
   Eina_Iterator *(*child_reversed_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count);
   Eina_Iterator *(*child_sorted_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count, Eina_Compare_Cb compare);
   Eina_Iterator *(*child_filtered_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count, Eina_Each_Cb match, const void *data);
   char *(*to_string)(const Eina_Model *model); /**< used to represent model as string, usually for debug purposes or user convenience */
   const void *value; /**< may hold extension methods */
};

EAPI Eina_Bool eina_model_type_constructor(const Eina_Model_Type *type,
                                           Eina_Model *model) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Bool eina_model_type_destructor(const Eina_Model_Type *type,
                                          Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_type_copy(const Eina_Model_Type *type,
                                    const Eina_Model *src,
                                    Eina_Model *dst) EINA_ARG_NONNULL(1, 2, 3);
EAPI Eina_Bool eina_model_type_deep_copy(const Eina_Model_Type *type,
                                         const Eina_Model *src,
                                         Eina_Model *dst) EINA_ARG_NONNULL(1, 2, 3);
EAPI Eina_Bool eina_model_type_compare(const Eina_Model_Type *type,
                                       const Eina_Model *a,
                                       const Eina_Model *b,
                                       int *cmp) EINA_ARG_NONNULL(1, 2, 3, 4);
EAPI Eina_Bool eina_model_type_load(const Eina_Model_Type *type,
                                    Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_type_unload(const Eina_Model_Type *type,
                                      Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_type_property_get(const Eina_Model_Type *type,
                                            const Eina_Model *model,
                                            const char *name,
                                            Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3, 4);
EAPI Eina_Bool eina_model_type_property_set(const Eina_Model_Type *type,
                                            Eina_Model *model,
                                            const char *name,
                                            const Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3, 4);
EAPI Eina_Bool eina_model_type_property_del(const Eina_Model_Type *type,
                                            Eina_Model *model,
                                            const char *name) EINA_ARG_NONNULL(1, 2, 3);
EAPI Eina_List *eina_model_type_properties_names_list_get(const Eina_Model_Type *type,
                                                          const Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI int eina_model_type_child_count(const Eina_Model_Type *type,
                                     const Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Model *eina_model_type_child_get(const Eina_Model_Type *type,
                                           const Eina_Model *model,
                                           unsigned int position) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_type_child_set(const Eina_Model_Type *type,
                                         Eina_Model *model,
                                         unsigned int position,
                                         Eina_Model *child) EINA_ARG_NONNULL(1, 2, 4);
EAPI Eina_Bool eina_model_type_child_del(const Eina_Model_Type *type,
                                         Eina_Model *model,
                                         unsigned int position) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_type_child_insert_at(const Eina_Model_Type *type,
                                               Eina_Model *model,
                                               unsigned int position,
                                               Eina_Model *child) EINA_ARG_NONNULL(1, 2, 4);
EAPI int eina_model_type_child_find(const Eina_Model_Type *type,
                                    const Eina_Model *model,
                                    unsigned int start_position,
                                    const Eina_Model *other) EINA_ARG_NONNULL(1, 2, 4);
EAPI int eina_model_type_child_search(const Eina_Model_Type *type,
                                      const Eina_Model *model,
                                      unsigned int start_position,
                                      Eina_Each_Cb match,
                                      const void *data) EINA_ARG_NONNULL(1, 2, 4);
EAPI void eina_model_type_child_sort(const Eina_Model_Type *type,
                                     Eina_Model *model,
                                     Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);
EAPI Eina_Iterator *eina_model_type_child_iterator_get(const Eina_Model_Type *type,
                                                       Eina_Model *model,
                                                       unsigned int start,
                                                       unsigned int count) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Iterator *eina_model_type_child_reversed_iterator_get(const Eina_Model_Type *type,
                                                                Eina_Model *model,
                                                                unsigned int start,
                                                                unsigned int count) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Iterator *eina_model_type_child_sorted_iterator_get(const Eina_Model_Type *type,
                                                              Eina_Model *model,
                                                              unsigned int start,
                                                              unsigned int count,
                                                              Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 5);
EAPI Eina_Iterator *eina_model_type_child_filtered_iterator_get(const Eina_Model_Type *type,
                                                                Eina_Model *model,
                                                                unsigned int start,
                                                                unsigned int count,
                                                                Eina_Each_Cb match,
                                                                const void *data) EINA_ARG_NONNULL(1, 2, 5);
EAPI char *eina_model_type_to_string(const Eina_Model_Type *type,
                                     const Eina_Model *model) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * @struct _Eina_Model_Interface
 *
 * Interfaces are managed by name, then multiple Eina_Model_Interface
 * may have the same name meaning it implements that name.
 *
 * @note use the same name pointer on queries to speed up the lookups!
 *
 * @since 1.2
 */
struct _Eina_Model_Interface
{
   /**
    * @def EINA_MODEL_INTERFACE_VERSION
    * Current API version, used to validate interface.
    */
#define EINA_MODEL_INTERFACE_VERSION (1)
   unsigned int version; /**< must be #EINA_MODEL_INTERFACE_VERSION */
   unsigned int private_size; /**< used to allocate interface private data */
   const char *name; /**< name for debug and introspection */
   const Eina_Model_Interface **interfaces; /**< null terminated array of parent interfaces */
   const Eina_Model_Event_Description *events; /**< null terminated array of events */
   Eina_Bool (*setup)(Eina_Model *model); /**< setup interface private data, do @b not call parent interface setup! */
   Eina_Bool (*flush)(Eina_Model *model); /**< flush interface private data, do @b not call parent interface flush! */
   Eina_Bool (*constructor)(Eina_Model *model); /**< construct interface instance, setup was already called. Should call parent's constructor if needed */
   Eina_Bool (*destructor)(Eina_Model *model); /**< destruct interface instance, flush will be called after it. Should call parent's destructor if needed. Release reference to other models here. */
   Eina_Bool (*copy)(const Eina_Model *src, Eina_Model *dst); /**< copy interface private data, do @b not call parent interface copy! */
   Eina_Bool (*deep_copy)(const Eina_Model *src, Eina_Model *dst); /**< deep copy interface private data, do @b not call parent interface deep copy! */
   const void *value; /**< holds the actual interface methods */
};

EAPI Eina_Bool eina_model_interface_constructor(const Eina_Model_Interface *iface,
                                                Eina_Model *model) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Bool eina_model_interface_destructor(const Eina_Model_Interface *iface,
                                               Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_interface_copy(const Eina_Model_Interface *iface,
                                         const Eina_Model *src,
                                         Eina_Model *dst) EINA_ARG_NONNULL(1, 2, 3);
EAPI Eina_Bool eina_model_interface_deep_copy(const Eina_Model_Interface *iface,
                                              const Eina_Model *src,
                                              Eina_Model *dst) EINA_ARG_NONNULL(1, 2, 3);


struct _Eina_Model_Event_Description
{
   const char *name; /**< name used for lookups */
   const char *type; /**< used for introspection purposes, documents what goes as callback event information (@c event_info) */
   const char *doc; /**< documentation for introspection purposes */
};
#define EINA_MODEL_EVENT_DESCRIPTION(name, type, doc) {name, type, doc}
#define EINA_MODEL_EVENT_DESCRIPTION_SENTINEL {NULL, NULL, NULL}

EAPI Eina_Bool eina_model_type_check(const Eina_Model_Type *type) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;
EAPI const char *eina_model_type_name_get(const Eina_Model_Type *type) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;
EAPI const Eina_Model_Type *eina_model_type_parent_get(const Eina_Model_Type *type) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI Eina_Bool eina_model_type_subclass_check(const Eina_Model_Type *type,
                                              const Eina_Model_Type *self_or_parent) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;


EAPI const Eina_Model_Interface *eina_model_type_interface_get(const Eina_Model_Type *type,
                                                               const char *name) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI void *eina_model_type_private_data_get(const Eina_Model *model,
                                            const Eina_Model_Type *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

EAPI Eina_Bool eina_model_interface_check(const Eina_Model_Interface *iface) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI void *eina_model_interface_private_data_get(const Eina_Model *model,
                                                 const Eina_Model_Interface *iface) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

/**
 * @var EINA_MODEL_TYPE_BASE
 * Base type for all eina model types.
 *
 * @since 1.2
 */
EAPI extern const Eina_Model_Type *EINA_MODEL_TYPE_BASE;

/**
 * @var EINA_MODEL_TYPE_MIXIN
 *
 * Type that uses #EINA_MODEL_INTERFACE_NAME_PROPERTIES and
 * #EINA_MODEL_INTERFACE_NAME_CHILDREN to manage the model.
 *
 * This is an abstract type, it does not work out of the box as one
 * needs to subclass it and define the interface implementations for
 * properties and children, as done by #EINA_MODEL_TYPE_GENERIC
 *
 * @see EINA_MODEL_TYPE_GENERIC
 *
 * @since 1.2
 */
EAPI extern const Eina_Model_Type *EINA_MODEL_TYPE_MIXIN;

/**
 * @var EINA_MODEL_TYPE_GENERIC
 *
 * Subclass of #EINA_MODEL_TYPE_MIXIN that uses
 * #EINA_MODEL_INTERFACE_PROPERTIES_HASH and
 * #EINA_MODEL_INTERFACE_CHILDREN_INARRAY.
 *
 * Should be generic enough to hold lots of items with runtime
 * configurable properties of any type.
 *
 * @see #EINA_MODEL_TYPE_STRUCT
 *
 * @since 1.2
 */
EAPI extern const Eina_Model_Type *EINA_MODEL_TYPE_GENERIC;

/**
 * @var EINA_MODEL_TYPE_STRUCT
 *
 * Subclass of #EINA_MODEL_TYPE_MIXIN that uses
 * #EINA_MODEL_INTERFACE_PROPERTIES_STRUCT and
 * #EINA_MODEL_INTERFACE_CHILDREN_INARRAY.
 *
 * Should be struct enough to hold lots of items with compile time
 * configurable properties of any type.
 *
 * @see #EINA_MODEL_TYPE_GENERIC
 *
 * @since 1.2
 */
EAPI extern const Eina_Model_Type *EINA_MODEL_TYPE_STRUCT;

EAPI Eina_Model *eina_model_struct_new(const Eina_Value_Struct_Desc *desc) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_MALLOC;


/**
 * @var EINA_MODEL_INTERFACE_NAME_PROPERTIES
 *
 * Interface that uses #Eina_Model_Interface_Properties as
 * Eina_Model_Interface::value and can manage the model properties.
 *
 * @since 1.2
 */
EAPI extern const char *EINA_MODEL_INTERFACE_NAME_PROPERTIES;

typedef struct _Eina_Model_Interface_Properties Eina_Model_Interface_Properties;
struct _Eina_Model_Interface_Properties
{
#define EINA_MODEL_INTERFACE_PROPERTIES_VERSION (1)
   unsigned int version;
   Eina_Bool (*compare)(const Eina_Model *a, const Eina_Model *b, int *cmp);
   Eina_Bool (*load)(Eina_Model *model);
   Eina_Bool (*unload)(Eina_Model *model);
   Eina_Bool (*get)(const Eina_Model *model, const char *name, Eina_Value *value);
   Eina_Bool (*set)(Eina_Model *model, const char *name, const Eina_Value *value);
   Eina_Bool (*del)(Eina_Model *model, const char *name);
   Eina_List *(*names_list_get)(const Eina_Model *model); /**< list of stringshare */
};

EAPI Eina_Bool eina_model_interface_properties_compare(const Eina_Model_Interface *iface,
                                                       const Eina_Model *a,
                                                       const Eina_Model *b,
                                                       int *cmp) EINA_ARG_NONNULL(1, 2, 3, 4) EINA_WARN_UNUSED_RESULT;

EAPI Eina_Bool eina_model_interface_properties_load(const Eina_Model_Interface *iface,
                                                    Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_interface_properties_unload(const Eina_Model_Interface *iface,
                                                      Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_interface_properties_get(const Eina_Model_Interface *iface,
                                                   const Eina_Model *model,
                                                   const char *name,
                                                   Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3, 4);
EAPI Eina_Bool eina_model_interface_properties_set(const Eina_Model_Interface *iface,
                                                   Eina_Model *model,
                                                   const char *name,
                                                   const Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3, 4);
EAPI Eina_Bool eina_model_interface_properties_del(const Eina_Model_Interface *iface,
                                                   Eina_Model *model,
                                                   const char *name) EINA_ARG_NONNULL(1, 2, 3);
EAPI Eina_List *eina_model_interface_properties_names_list_get(const Eina_Model_Interface *iface,
                                                               const Eina_Model *model) EINA_ARG_NONNULL(1, 2); /**< list of stringshare */

/**
 * @var EINA_MODEL_INTERFACE_PROPERTIES_HASH
 *
 * Implements #Eina_Model_Interface_Properties
 * (#EINA_MODEL_INTERFACE_NAME_PROPERTIES) using #Eina_Hash.
 *
 * @note This function is generic but uses too much space given the
 *       hash data type. For huge number of elements it's better to
 *       use custom implementation instead.
 *
 * @see EINA_MODEL_INTERFACE_PROPERTIES_STRUCT
 *
 * @since 1.2
 */
EAPI extern const Eina_Model_Interface *EINA_MODEL_INTERFACE_PROPERTIES_HASH;

/**
 * @var EINA_MODEL_INTERFACE_PROPERTIES_STRUCT
 *
 * Implements #Eina_Model_Interface_Properties
 * (#EINA_MODEL_INTERFACE_NAME_PROPERTIES) using #Eina_Value_Struct.
 *
 * The interface private data is #Eina_Value of type
 * #EINA_VALUE_TYPE_STRUCT. Properties will be accessed using
 * #Eina_Value_Struct::desc information that can be set by types such
 * as #EINA_MODEL_TYPE_STRUCT
 *
 * @see EINA_MODEL_INTERFACE_PROPERTIES_HASH
 *
 * @since 1.2
 */
EAPI extern const Eina_Model_Interface *EINA_MODEL_INTERFACE_PROPERTIES_STRUCT;

/**
 * @brief Configure the internal properties of model implementing #EINA_MODEL_INTERFACE_PROPERTIES_STRUCT.
 *
 * @param model The model instance to configure.
 * @param desc The structure description to use.
 * @param memory If not @c NULL, will be adopted by model.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This will setup the internal pointers so that the given @a desc is
 * used to manage the properties of this struct.
 *
 * If a given memory is provided, it will be adopted (not copied!),
 * being free'd when the model is gone.
 *
 * @see #EINA_VALUE_TYPE_STRUCT
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_model_struct_set(Eina_Model *model,
                                     const Eina_Value_Struct_Desc *desc,
                                     void *memory) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Get the internal properties of model implementing #EINA_MODEL_INTERFACE_PROPERTIES_STRUCT.
 *
 * @param model the model instance.
 * @param p_desc where to return the structure description in use.
 * @param p_memory where to return the structure memory in use.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * No copies are made! The memory and description may be invalidaded
 * by calls to eina_model_struct_set() or eina_model_del().
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_model_struct_get(const Eina_Model *model,
                                     const Eina_Value_Struct_Desc **p_desc,
                                     void **p_memory) EINA_ARG_NONNULL(1, 2);

/**
 * @var EINA_MODEL_INTERFACE_NAME_CHILDREN
 *
 * Interface that uses #Eina_Model_Interface_Children as
 * Eina_Model_Interface::value and can manage the model children.
 *
 * @since 1.2
 */
EAPI extern const char *EINA_MODEL_INTERFACE_NAME_CHILDREN;

/**
 * @typedef Eina_Model_Interface_Children
 *
 * The #Eina_Model_Interface::value when name is
 * #EINA_MODEL_INTERFACE_NAME_CHILDREN interface is implemented.
 *
 * @since 1.2
 */
typedef struct _Eina_Model_Interface_Children Eina_Model_Interface_Children;

/**
 * @struct _Eina_Model_Interface_Children
 *
 * The #Eina_Model_Interface::value when name is
 * #EINA_MODEL_INTERFACE_NAME_CHILDREN interface is implemented.
 *
 * The methods are called in the same way children methods from
 * #Eina_Model_Type.
 *
 * @since 1.2
 */
struct _Eina_Model_Interface_Children
{
#define EINA_MODEL_INTERFACE_CHILDREN_VERSION (1)
   unsigned int version;
   Eina_Bool (*compare)(const Eina_Model *a, const Eina_Model *b, int *cmp);
   Eina_Bool (*load)(Eina_Model *model);
   Eina_Bool (*unload)(Eina_Model *model);
   int (*count)(const Eina_Model *model);
   Eina_Model *(*get)(const Eina_Model *model, unsigned int position);
   Eina_Bool (*set)(Eina_Model *model, unsigned int position, Eina_Model *child);
   Eina_Bool (*del)(Eina_Model *model, unsigned int position);
   Eina_Bool (*insert_at)(Eina_Model *model, unsigned int position, Eina_Model *child);
   void (*sort)(Eina_Model *model, Eina_Compare_Cb compare);
};

EAPI Eina_Bool eina_model_interface_children_compare(const Eina_Model_Interface *iface,
                                                     const Eina_Model *a,
                                                     const Eina_Model *b,
                                                     int *cmp) EINA_ARG_NONNULL(1, 2, 3, 4) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Bool eina_model_interface_children_load(const Eina_Model_Interface *iface,
                                                  Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_interface_children_unload(const Eina_Model_Interface *iface,
                                                    Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI int eina_model_interface_children_count(const Eina_Model_Interface *iface,
                                             const Eina_Model *model) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Model *eina_model_interface_children_get(const Eina_Model_Interface *iface,
                                                   const Eina_Model *model,
                                                   unsigned int position) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_interface_children_set(const Eina_Model_Interface *iface,
                                                 Eina_Model *model,
                                                 unsigned int position,
                                                 Eina_Model *child) EINA_ARG_NONNULL(1, 2, 4);
EAPI Eina_Bool eina_model_interface_children_del(const Eina_Model_Interface *iface,
                                                 Eina_Model *model,
                                                 unsigned int position) EINA_ARG_NONNULL(1, 2);
EAPI Eina_Bool eina_model_interface_children_insert_at(const Eina_Model_Interface *iface,
                                                       Eina_Model *model,
                                                       unsigned int position,
                                                       Eina_Model *child) EINA_ARG_NONNULL(1, 2, 4);
EAPI void eina_model_interface_children_sort(const Eina_Model_Interface *iface,
                                             Eina_Model *model,
                                             Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @var EINA_MODEL_INTERFACE_CHILDREN_INARRAY
 *
 * Implements #Eina_Model_Interface_Children
 * (#EINA_MODEL_INTERFACE_NAME_CHILDREN) using #Eina_Inarray. It
 * should be efficient in space and time for most operations.
 *
 * @note it may become slow if eina_model_child_insert_at() is used at(or near)
 *       the beginning of the array as the members from that position
 *       to the end must be memmove()d.
 *
 * @since 1.2
 */
EAPI extern const Eina_Model_Interface *EINA_MODEL_INTERFACE_CHILDREN_INARRAY;

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
#endif
