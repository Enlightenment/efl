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
#include "eina_inlist.h"
#include <stdarg.h>

/**
 * @page eina_model_01_c eina_model_01.c Eina_Model inheritance and function overriding
 * @include eina_model_01.c
 */

/**
 * @page eina_model_02_example_page Creating a simple model
 * @dontinclude eina_model_02.c
 *
 * This example shows the creation of a model with five properties, named:
 * 'a', 'b', 'c', 'd' and 'e' with values 0, 1, 2, 3 and 4
 * respectively. In addition to the 5 properties our model also add 5 children,
 * and to each child we give a property named 'x' with a value of 1, 2, 3, 4 and
 * 5.
 *
 * In other words this piece of code shows how to use eina_model to store a list
 * of elements, given that the list itself has some properties.
 *
 * Now let's walk through the code and examine the interesting bits.
 *
 * This is some pretty standard initialization code.
 * @until eina_init
 *
 * We now create our eina_model, the important detail here is the type of the
 * model being created, for this example we use the generic type provided by
 * eina:
 * @until model_new
 *
 * Once our model has been created we can add callbacks to be notified of events
 * that happen to our model, for this example we are just going to add a
 * callback for the "delete" event. To get a list of events a given eina model
 * can emit see @ref eina_model_event_names_list_get().
 * @until callback_add
 *
 * Once we have a model, we need to populate it with information. There are two
 * types of information we can store on an eina model: properties and eina
 * models. We are going to start by looking at properties.
 *
 * Properties are, simply put, named values. They have a char* identifier and an
 * Eina_Value value. This means you can store in a property almost any type of
 * data. For this example we are going to add some very simple numeric
 * properties which will have single letter identifiers.
 * @until }
 * @until }
 *
 * Despite being able to store almost any value properties the least flexible
 * information unit we can put in an eina model. We can add eina models to our
 * eina model, this allows us to represt complex information hierarchies. This
 * example adds 5 models(with no children of their own) to our parent model @c
 * m.
 * @until }
 * The code here should be pretty easy to understand, we create a model, much
 * like we did before, and we then add a property to our model, again a task we
 * have already done.
 *
 * The important issue to note here is that we could have given each of our @c c
 * child models as complex an structure as we needed, they could each be a list
 * or a tree on their own right.
 *
 * Now that we have a populated model we print a string representation of
 * it(without forgetting to free the string):
 * @until free
 *
 * And since we are done using our model we release our reference to it(and
 * since no else holds references to it, it will be freed):
 * @until }
 *
 * Note that we don't need to iterate over the children of @c m unrefing it,
 * this is because we don't hold references to it, we freed our references right
 * after we added them to their parent model, so when the parent model dies(and
 * releases the references to it's children) they will be freed.
 *
 * The only thing we are going to look at is the callback we registered for
 * whenever a model is deleted, since our models don't do anything fancy we are
 * just going to print the memory address of the model being freed.
 * @until }
 *
 * Note that this means the memory address is still valid, our callback is
 * called just before the memory is freed so we could still access its
 * information here.
 *
 * The full code can be seen in @ref eina_model_02_c
 */

/**
 * @page eina_model_02_c eina_model_02.c
 * @include eina_model_02.c
 * @example eina_model_02.c
 */

/**
 * @page eina_model_03_example_page Using Eina_Model and inheritance
 * @dontinclude eina_model_03.c
 *
 * This example will use two custom defined eina model types: @c PERSON_TYPE to
 * represent a person and @c ADDRESS_BOOK_TYPE to represent the an address book.
 * Both our types inherit from EINA_MODEL_TYPE_STRUCT, and, therefore,
 * store it's data on a struct. Our address book will be very simple it will
 * only contain one property, the name of the file where we store our address
 * book. The person type will contain two fields a name and en email. Let's look
 * at the code.
 *
 * We'll start with declaring the variables and functions we'll need to define
 * our custom type. This will all be explained when the variables get used.
 * @until address_book_init
 *
 * We then jump into our @c main function, declare a couple of variables and
 * initialize eina:
 * @until eina_init
 *
 * After eina is initialized we'll @c address_book_init() which will initialize
 * both our @c PERSON_TYPE and our @c ADDRESS_BOOK_TYPE. Details of this will be
 * shown latter on:
 * @until address_book_init
 *
 * Now that everything is correctly initialized we can create the model that
 * will represent our address book's
 * @until eina_model_new
 *
 * Before we can load data into our model we need to tell it where to load from,
 * we do this by setting it's filename property:
 * @until value_flush
 *
 * We then load data into our model and display it as a string:
 * @until free
 *
 * While @c eina_model_to_string allows you to see the contents of the model,
 * it's display format is not user friendly, it's best used for debugging. So
 * let's now print our model in a user friendly way.
 *
 * First we see how many people are in our address book and print that:
 * @until printf
 *
 * And now we iterate over every child of our address book model, which
 * represents a person:
 * @until person
 *
 * But again simply calling @c eina_model_to_string would result in not very
 * user friendly output, so we'll need to get the properties of the person(name
 * and email) and print them with some formatting:
 * @until printf
 *
 * We then free the resources we allocated to print this person:
 * @until }
 *
 * And that's it for our main function, now just freeing our resources:
 * @until }
 *
 * This however obviously doesn't conclude our example we need to examine how
 * the the loading of data works to really understand what is happening in the
 * @c main function.
 *
 * Let's start with the constructors(and the variables they use). Both our
 * constructors do two very important tasks:
 * @li Calls our parent's constructor, and
 * @li Sets the description of the struct on our model
 *
 * For these constructors that's all we need to do since most of our
 * functionality is provided by @c EINA_MODEL_TYPE_STRUCT.
 * @until }
 * @until }
 *
 * And now we have our load function, it opens the file from which we'll
 * read the address book:
 * @until EINA_SAFETY_ON_NULL_RETURN_VAL
 *
 * Once the file has been opened we read from it line by line and for each
 * non-blank line we get a name and an email:
 * @until email
 * @until email
 *
 * Once we have the name and email we create our person model, set it's
 * properties and make our person a child of the address book:
 * @until }
 *
 * And now that we're done reading the file we close it:
 * @until }
 *
 * This next function is perphaps the most interesting one of our example, it's
 * the one that creates the definition of our derived types.
 *
 * First thing we'll do is the description of the members of our person type.
 * @until person_members[1].type
 * Now the description of the struct itself(which uses the members):
 * @until }
 * And finally we define the person type itself:
 * @until person_type.constructor
 *
 * With the person now described we'll do the same process for our address book
 * type:
 * @until address_book_type.load
 *
 * So far everything we created has been in the scope of our function to make
 * this available outside(such as in the @c main function where we use @c
 * ADDRESS_BOOK_TYPE and on @c _address_book_load function where we use @c
 * PERSON_TYPE) we need to assign our descriptions and type to global variables:
 * @until }
 *
 * This concludes this example. A good exercise for the reader is to extend this
 * example to have the model save the addres book, for example once it's
 * unloaded, this can be done by overriding the .unload property of @c
 * ADDRESS_BOOK_TYPE.
 *
 * For the full code see: @ref eina_model_03_c
 */

/**
 * @page eina_model_03_c eina_model_03.c
 * @include eina_model_03.c
 * @example eina_model_03.c
 */

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
 * @ref eina_model_02_example_page contains an easy to follow example that
 * demonstrates several of the important features of eina_model.
 *
 * An inheritance example: @ref eina_model_01_c
 *
 * And a explained inheritance example: @ref eina_model_03_example_page
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

/**
 * @brief Creates a new model of type @a Type.
 *
 * @see _Eina_Model_Type
 * @see eina_model_del()
 * @since 1.2
 */
EAPI Eina_Model *eina_model_new(const Eina_Model_Type *type);
/**
 * @brief Frees the memory associated with @a model
 *
 * @see eina_model_new()
 * @since 1.2
 */
EAPI void eina_model_del(Eina_Model *model) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the type of @a model.
 *
 * @see eina_model_new()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI const Eina_Model_Type *eina_model_type_get(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Returns the interface named @a name of @a model.
 *
 * The name of every interface of @a model will be compared to @a name, the
 * first one to match will be returned.
 *
 * @see eina_model_new()
 * @see _Eina_Model_Interface
 * @since 1.2
 */
EAPI const Eina_Model_Interface *eina_model_interface_get(const Eina_Model *model,
                                                          const char *name) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Checks if @a model is an instance of @a type.
 *
 * @see eina_model_new()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_instance_check(const Eina_Model *model,
                                         const Eina_Model_Type *type) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Checks if @a model implements @a iface.
 *
 * @see _Eina_Model_Interface
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_implemented(const Eina_Model *model, const Eina_Model_Interface *iface) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Increases the refcount of @a model.
 * @param model The model to increase reference.
 * @return The @a model with reference increased.
 *
 * @see eina_model_new()
 * @see eina_model_unref()
 * @since 1.2
 */
EAPI Eina_Model *eina_model_ref(Eina_Model *model) EINA_ARG_NONNULL(1);

/**
 * @brief Increases the refcount of @a model, informs reference identifier.
 * @param model The model to increase reference.
 * @param id An identifier to mark this reference.
 * @param label An optional label to help debug, may be @c NULL.
 * @return The @a model with reference increased.
 *
 * This extended version of reference explicitly marks the origin of
 * the reference and eina_model_xunref() should be used to check and
 * remove it.
 *
 * Usually the @a id is another object, like a parent object, or some
 * class/structure/file/function that is holding the reference for
 * some reason.
 *
 * Its purpose is to help debuging if Eina was compiled with model
 * usage debug enabled and environment variable @c EINA_MODEL_DEBUG=1
 * is set.
 *
 * It is recommended to use eina_model_xref() and eina_model_xunref()
 * pair whenever you want to be sure you released your
 * references. Both at your own type, or using applications. As an
 * example #EINA_MODEL_INTERFACE_CHILDREN_INARRAY will use this to
 * make sure it deleted every managed children.
 *
 * In order to debug leaks, consider using eina_model_xrefs_get() or
 * eina_models_usage_dump() for a global picture. However, some
 * references are not tracked, namely:
 *
 * @li eina_model_new()
 * @li eina_model_child_get()
 * @li eina_model_child_iterator_get()
 * @li eina_model_child_reversed_iterator_get()
 * @li eina_model_child_sorted_iterator_get()
 * @li eina_model_child_filtered_iterator_get()
 * @li eina_model_child_slice_iterator_get()
 * @li eina_model_child_slice_reversed_iterator_get()
 * @li eina_model_child_slice_sorted_iterator_get()
 * @li eina_model_child_slice_filtered_iterator_get()
 *
 * @note this function is slower than eina_model_ref() if
 *       @c EINA_MODEL_DEBUG is set to "1" or "backtrace". Otherwise it
 *       should have the same performance cost.
 *
 * @see eina_model_ref()
 * @see eina_model_xunref()
 * @since 1.2
 */
EAPI Eina_Model *eina_model_xref(Eina_Model *model,
                                 const void *id,
                                 const char *label) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Decreases the refcount of @a model.
 * @param model The model to decrease reference.
 *
 * After this function returns, consider @a model pointer invalid.
 *
 * @see eina_model_ref()
 * @see eina_model_del()
 * @since 1.2
 */
EAPI void eina_model_unref(Eina_Model *model) EINA_ARG_NONNULL(1);

/**
 * @brief Decreases the refcount of @a model, informs reference identifier.
 * @param model The model to decrease reference.
 * @param id An identifier to mark this reference.
 *
 * This function will match eina_model_xref() and the @a id must match
 * a previously call, otherwise it will produce an error if @c
 * EINA_MODEL_DEBUG is set to "1" or "backtrace", and the reference is
 * not decreased!
 *
 * After this function returns, consider @a model pointer invalid.
 *
 * @note this function is slower than eina_model_unref() if
 *       @c EINA_MODEL_DEBUG is set to "1" or "backtrace". Otherwise it
 *       should have the same performance cost.
 *
 * @see eina_model_xref()
 * @since 1.2
 */
EAPI void eina_model_xunref(Eina_Model *model,
                            const void *id) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Returns the number of references to @a model.
 * @param model The model to query number of references.
 * @return number of references to model
 *
 * @see eina_model_ref()
 * @see eina_model_unref()
 * @see eina_model_xref()
 * @see eina_model_xunref()
 * @see eina_model_xrefs_get()
 * @since 1.2
 */
EAPI int eina_model_refcount(const Eina_Model *model) EINA_ARG_NONNULL(1);

typedef struct _Eina_Model_XRef Eina_Model_XRef;
struct _Eina_Model_XRef
{
   EINA_INLIST;
   const void *id; /**< as given to eina_model_xref() */
   struct {
      const void * const *symbols; /**< only if @c EINA_MODEL_DEBUG=backtrace is set, otherwise is @c NULL */
      unsigned int count; /**< only if @c EINA_MODEL_DEBUG=backtrace is set, otherwise is 0 */
   } backtrace;
   char label[];
};

/**
 * @brief Returns the current references of this model.
 * @param model The model to query references.
 * @return List of reference holders as Eina_Model_XRef. This is the internal
 *         list for speed purposes, do not modify or free it in anyway!
 *
 * @note This list only exist if environment variable
 *       @c EINA_MODEL_DEBUG is set to "1" or "backtrace".
 *
 * @note The backtrace information is only available if environment
 *       variable @c EINA_MODEL_DEBUG=backtrace is set.
 * @since 1.2
 */
EAPI const Eina_Inlist *eina_model_xrefs_get(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * @brief Add a callback to be called when @a event_name is emited.
 *
 * @see eina_model_event_callback_del()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_event_callback_add(Eina_Model *model,
                                             const char *event_name,
                                             Eina_Model_Event_Cb cb,
                                             const void *data) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Remove a callback that was to be called when @a event_name was emited.
 *
 * @see eina_model_event_callback_add()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_event_callback_del(Eina_Model *model,
                                             const char *event_name,
                                             Eina_Model_Event_Cb cb,
                                             const void *data) EINA_ARG_NONNULL(1, 2, 3);

EAPI const Eina_Model_Event_Description *eina_model_event_description_get(const Eina_Model *model,
                                                                           const char *event_name) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Returns list of events this model may emit.
 * @param model The model whose events are to be listed.
 * @return An Eina_List of stringshares with the name of every event. Free the
 * list with eina_model_event_names_list_free().
 *
 * @since 1.2
 */
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

/**
 * @brief Compares two models.
 * @param a The first model to compare.
 * @param b The second model to compare.
 * @return Greater than zero if @a a > @a b, zero if @a a == @a b and less than
 * zero if @a a < @a b
 *
 * The default comparison checks that the properties of @a a and @a b all have
 * the same name and value, and then recursively compares all children.
 *
 * A model with less properties or children is considered smaller than one with
 * more properties.
 *
 * @since 1.2
 */
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

EAPI int eina_model_child_criteria_match(const Eina_Model *model,
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
 * This code shows how to use iterators to do something (in this example call
 * use_child()) on every child element.
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
 * This code shows how to use iterators to do something (in this example call
 * use_child()) on every child element starting from last to first.
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
 * This bit of code shows how to use iterators to do something (in this example
 * call use_child()) on every child element in the order given by the @a compare
 * function.
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
 * If no child element matches a valid, and empty, iterator will be returned.
 * Indexes returned by this iterator are guaranteed to exists.
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
 * This bit of code shows how to use iterators to do something (in this example
 * print the address) on child elements that match the criteria given of @a match.
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
 *
 * The default format of the ouput is:
 * Type_Name({Property_Name: Property_Value, ...}, [Child0, Child1, ...])
 *
 * Where:
 *  @li Type_Name: eina_model_type_name_get(eina_model_type_get(model))
 *  @li Properties are sorted alphabetically.
 *  @li Property_Value is created using eina_value_to_string().
 *  @li Children are converted using eina_model_to_string()
 *
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
 * @warning The methods @c setup, @c flush, @c constructor, @c destructor and
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
 * properties, children and may use parent's data if needed. The only
 * constructor caled is that of the most specialized type, if interface
 * constructors should be called, do them in the desired order from the type
 * constructor.
 *
 * When the model is deleted, explicitly with eina_model_del() or
 * implicitly with eina_model_unref() on the last reference, the @c
 * destructor is called. It must release references to other
 * models. When the last reference is dropped, every @c flush is
 * called from child to parent, then memory is freed. The only
 * destructor caled is that of the most specialized type, if interface
 * destructors should be called, do them in the desired order from the type
 * destructor.
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
   unsigned int type_size; /**< used to know sizeof(Eina_Model_Type) or subtypes (which may be bigger, by including Eina_Model_Type as a header */
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
   int (*child_criteria_match)(const Eina_Model *model, unsigned int start_position, Eina_Each_Cb match, const void *data);
   void (*child_sort)(Eina_Model *model, Eina_Compare_Cb compare);
   Eina_Iterator *(*child_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count);
   Eina_Iterator *(*child_reversed_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count);
   Eina_Iterator *(*child_sorted_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count, Eina_Compare_Cb compare);
   Eina_Iterator *(*child_filtered_iterator_get)(Eina_Model *model, unsigned int start, unsigned int count, Eina_Each_Cb match, const void *data);
   char *(*to_string)(const Eina_Model *model); /**< used to represent model as string, usually for debug purposes or user convenience */
   void *__extension_ptr0; /**< not to be used */
   void *__extension_ptr1; /**< not to be used */
   void *__extension_ptr2; /**< not to be used */
   void *__extension_ptr3; /**< not to be used */
};

#define EINA_MODEL_TYPE_INIT(name, type, private_type, parent, interfaces, events) \
  {EINA_MODEL_TYPE_VERSION,                                         \
   sizeof(private_type),                                            \
   sizeof(type),                                                    \
   name,                                                            \
   parent,                                                          \
   interfaces,                                                      \
   events,                                                          \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL                                                             \
   }

#define EINA_MODEL_TYPE_INIT_NOPRIVATE(name, type, parent, interfaces, events) \
  {EINA_MODEL_TYPE_VERSION,                                         \
   0,                                                               \
   sizeof(type),                                                    \
   name,                                                            \
   parent,                                                          \
   interfaces,                                                      \
   events,                                                          \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL                                                             \
   }

#define EINA_MODEL_TYPE_INIT_NULL                                   \
  {0,                                                               \
   0,                                                               \
   0,                                                               \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL,                                                            \
   NULL                                                             \
   }

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
EAPI int eina_model_type_child_criteria_match(const Eina_Model_Type *type,
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
 * @brief Get resolved method from types that extend Eina_Model_Type given offset.
 *
 * @param model the model to query the method
 * @param offset the byte offset in the structure given as type, it
 *        must be bigger than Eina_Model_Type itself.
 * @return address to resolved method, or @c NULL if method is not
 *         implemented.
 *
 * The use of this function is discouraged, you should use
 * #eina_model_method_resolve instead.
 *
 * When implementing new types that augments the basic methods from
 * Eina_Model_Type, the recommended structure layout is as follow:
 * @code
 * typedef struct _My_Type My_Type;
 * struct _My_Type {
 *    Eina_Model_Type base;
 *    int (*my_method)(Eina_Model *model);
 * };
 *
 * int my_type_my_method(Eina_Model *model);
 * @endcode
 *
 * Then the implementation of @c my_type_my_method() needs to get the
 * most specific @c my_method that is not @c NULL from type hierarchy,
 * also called "resolve the method".
 *
 * To do this in an efficient way, Eina_Model infrastructure
 * pre-resolves all methods and provides this function for efficient
 * query. The recommended implementation of my_type_my_method() would
 * be:
 * @code
 * int my_type_my_method(Eina_Model *model)
 * {
 *   int (*meth)(Eina_Model *);
 *
 *   EINA_SAFETY_ON_FALSE_RETURN(eina_model_instance_check(model, MY_TYPE), -1);
 *
 *   meth = eina_model_method_offset_resolve(model, offsetof(My_Type, my_method));
 *   EINA_SAFETY_ON_NULL_RETURN(meth, -1);
 *   return meth(model);
 * }
 * @endcode
 *
 * @note offset must be bigger than Eina_Model_Type, otherwise use
 *       specific functions such as eina_model_property_get().
 *
 * @see eina_model_method_resolve
 * @see eina_model_type_method_resolve
 * @since 1.2
 */
EAPI const void *eina_model_method_offset_resolve(const Eina_Model *model, unsigned int offset) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

EAPI const void *eina_model_type_method_offset_resolve(const Eina_Model_Type *type, const Eina_Model *model, unsigned int offset) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

#define eina_model_method_resolve(model, struct_type, method) eina_model_method_offset_resolve((model), offsetof(struct_type, method))

#define eina_model_type_method_resolve(type, model, struct_type, method) eina_model_type_method_offset_resolve((type), (model), offsetof(struct_type, method))

/**
 * @struct _Eina_Model_Interface
 *
 * @warning The methods @c setup, @c flush, @c constructor and @c destructor are
 * mandatory and must exist, otherwise type cannot be used.
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
   unsigned int interface_size; /**< used to know sizeof(Eina_Model_Interface) or subtypes (which may be bigger, by including Eina_Model_Interface as header */
   const char *name; /**< name for debug and introspection */
   const Eina_Model_Interface **interfaces; /**< null terminated array of parent interfaces */
   const Eina_Model_Event_Description *events; /**< null terminated array of events */
   Eina_Bool (*setup)(Eina_Model *model); /**< setup interface private data, do @b not call parent interface setup! */
   Eina_Bool (*flush)(Eina_Model *model); /**< flush interface private data, do @b not call parent interface flush! */
   Eina_Bool (*constructor)(Eina_Model *model); /**< construct interface instance, setup was already called. Should call parent's constructor if needed */
   Eina_Bool (*destructor)(Eina_Model *model); /**< destruct interface instance, flush will be called after it. Should call parent's destructor if needed. Release reference to other models here. */
   Eina_Bool (*copy)(const Eina_Model *src, Eina_Model *dst); /**< copy interface private data, do @b not call parent interface copy! */
   Eina_Bool (*deep_copy)(const Eina_Model *src, Eina_Model *dst); /**< deep copy interface private data, do @b not call parent interface deep copy! */
   void *__extension_ptr0; /**< not to be used */
   void *__extension_ptr1; /**< not to be used */
   void *__extension_ptr2; /**< not to be used */
   void *__extension_ptr3; /**< not to be used */
};

#define EINA_MODEL_INTERFACE_INIT(name, iface, private_type, parent, events) \
  {EINA_MODEL_INTERFACE_VERSION,                                        \
   sizeof(private_type),                                                \
   sizeof(iface),                                                       \
   name,                                                                \
   parent,                                                              \
   events,                                                              \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL                                                                 \
   }

#define EINA_MODEL_INTERFACE_INIT_NOPRIVATE(name, iface, parent, events) \
  {EINA_MODEL_INTERFACE_VERSION,                                        \
   0,                                                                   \
   sizeof(iface),                                                       \
   name,                                                                \
   parent,                                                              \
   events,                                                              \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL,                                                                \
   NULL                                                                 \
   }

#define EINA_MODEL_INTERFACE_INIT_NULL      \
  {0,                                       \
   0,                                       \
   0,                                       \
   NULL,                                    \
   NULL,                                    \
   NULL,                                    \
   NULL,                                    \
   NULL,                                    \
   NULL,                                    \
   NULL,                                    \
   NULL,                                    \
   NULL,                                    \
   NULL,                                    \
   NULL,                                    \
   NULL,                                    \
   NULL                                     \
   }

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

#define eina_model_interface_method_resolve(iface, model, struct_type, method) eina_model_interface_method_offset_resolve((iface), (model), offsetof(struct_type, method))

EAPI const void *eina_model_interface_method_offset_resolve(const Eina_Model_Interface *iface, const Eina_Model *model, unsigned int offset) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;


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

/**
 * @brief Setup the type to be a subclass of another parent type.
 * @param type type to be modified
 * @param parent type to be used as parent
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * Although @a type is modified, the following properties are not
 * touched or they are actually used for validation:
 *
 * @li @c type->version must be #EINA_MODEL_TYPE_VERSION;
 * @li @c type->private_size unmodified, should be set to type's size;
 * @li @c type->name unmodified, should be set to type's name.
 *
 *
 * All other fields are modified as follow:
 *
 * @li @c type->type_size initiated to parent->type_size
 * @li @c type->interfaces = NULL;
 * @li @c type->events = NULL;
 * @li @c type->setup = NULL;
 * @li @c type->flush = NULL;
 * @li @c type->constructor = NULL;
 * @li @c type->destructor = NULL;
 * @li @c type->copy = NULL;
 * @li @c type->deep_copy = NULL;
 * @li @c type->compare = NULL;
 * @li @c type->load = NULL;
 * @li @c type->unload = NULL;
 * @li @c type->property_get = NULL;
 * @li @c type->property_set = NULL;
 * @li @c type->property_del = NULL;
 * @li @c type->properties_names_list_get = NULL;
 * @li @c type->child_count = NULL;
 * @li @c type->child_get = NULL;
 * @li @c type->child_set = NULL;
 * @li @c type->child_del = NULL;
 * @li @c type->child_insert_at = NULL;
 * @li @c type->child_find = NULL;
 * @li @c type->child_criteria_match = NULL;
 * @li @c type->child_sort = NULL;
 * @li @c type->child_iterator_get = NULL;
 * @li @c type->child_reversed_iterator_get = NULL;
 * @li @c type->child_sorted_iterator_get = NULL;
 * @li @c type->child_filtered_iterator_get = NULL;
 * @li @c type->to_string = NULL;
 *
 * If you have custom methods, overload them afterwards
 * eina_model_type_subclass_setup() returns with #EINA_TRUE.
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_subclass_setup(Eina_Model_Type *type,
                                              const Eina_Model_Type *parent) EINA_ARG_NONNULL(1, 2);

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
   Eina_Model_Interface base;
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
   Eina_Model_Interface base;
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
 * @brief Dump usage of all existing modules.
 * @since 1.2
 */
EAPI void eina_models_usage_dump(void);

/**
 * @brief Return a list of all live models.
 * @return a newly allocated list of Eina_Model. Free using
 *         eina_models_list_free()
 *
 * @note this is meant to debug purposes, do not modify the models in
 *       any way!
 *
 * @note due performance reasons, this is only @b enabled when
 *       @c EINA_MODEL_DEBUG is set to "1" or "backtrace".
 *
 * @since 1.2
 */
EAPI Eina_List *eina_models_list_get(void);

/**
 * @brief Release list returned by eina_models_list_get()
 * @param list the list to release.
 */
EAPI void eina_models_list_free(Eina_List *list);

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
