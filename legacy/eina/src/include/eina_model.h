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
 * @page eina_model_01_c Eina_Model inheritance and function overriding
 * @include eina_model_01.c
 */

/**
 * @page eina_model_04_c Eina_Model inheritance, interfaces, and function overriding
 * @include eina_model_04_main.c
 * @include eina_model_04_animal.c
 * @include eina_model_04_human.c
 * @include eina_model_04_parrot.c
 * @include eina_model_04_child.c
 * @include eina_model_04_main.c
 * @include eina_model_04_whistler.c
 * @include eina_model_04_animal.h
 * @include eina_model_04_human.h
 * @include eina_model_04_whistler.h
 * @include eina_model_04_child.h
 * @include eina_model_04_parrot.h
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
 * @defgroup Eina_Model_Group Data Model API
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
 * Not as generic as #EINA_MODEL_TYPE_GENERIC, but way more efficient
 * is #EINA_MODEL_TYPE_STRUCT that instead of a hash of properties of
 * any type, it uses a struct to map properties. Its properties are
 * fixed set of names and they have fixed type, as defined by the
 * #Eina_Value_Struct_Desc description used internally.
 *
 * Examples:
 * @li @ref eina_model_01_c inheritance example, uses #EINA_MODEL_TYPE_GENERIC
 * @li @ref eina_model_02_example_page contains an easy to follow
 *     example that demonstrates several of the important features of
 *     eina_model, uses #EINA_MODEL_TYPE_GENERIC.
 * @li @ref eina_model_03_example_page walk-through example on how to
 *     inherit types, a suggestion of eina_model_load() usage and uses
 *     #EINA_MODEL_TYPE_STRUCT.
 * @li @ref eina_model_04_c Advanced inheritance, interfaces and interface
 *     function overloading example.
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
 * This is an opaque handle that is created with eina_model_new() and
 * released with eina_model_unref().
 *
 * It contains properties, children and may emit events. See
 * respectively:
 * @li eina_model_property_get() and eina_model_property_set()
 * @li eina_model_child_get() and eina_model_child_set()
 * @li eina_model_event_names_list_get(), eina_model_event_callback_add() and eina_model_event_callback_del()
 *
 * @see eina_model_new()
 * @see eina_model_ref() and eina_model_xref()
 * @see eina_model_unref(), eina_model_xunref() and eina_model_del()
 * @see eina_model_type_get() and eina_model_interface_get()
 * @since 1.2
 */
typedef struct _Eina_Model Eina_Model;

/**
 * @typedef Eina_Model_Type
 * Data Model Type.
 *
 * @see #_Eina_Model_Type explains fields.
 * @since 1.2
 */
typedef struct _Eina_Model_Type Eina_Model_Type;

/**
 * @typedef Eina_Model_Interface
 * Data Model Interface.
 *
 * @see #_Eina_Model_Interface explains fields.
 * @since 1.2
 */
typedef struct _Eina_Model_Interface Eina_Model_Interface;

/**
 * @typedef Eina_Model_Event_Description
 * Data Model Event Description.
 *
 * This is used to declare events supported by types and interfaces
 * and also to provide introspection to receivers of signals so they
 * can know which data they are receiving as @c event_info.
 *
 * @see EINA_MODEL_EVENT_DESCRIPTION()
 * @see #EINA_MODEL_EVENT_DESCRIPTION_SENTINEL
 * @see #_Eina_Model_Event_Description explains fields.
 * @since 1.2
 */
typedef struct _Eina_Model_Event_Description Eina_Model_Event_Description;

/**
 * @brief Creates a new model of type @a Type.
 * @param type The type of the model to create.
 * @return If successfull pointer to model, NULL otherwise.
 *
 * @see _Eina_Model_Type
 * @see eina_model_del()
 * @since 1.2
 */
EAPI Eina_Model *eina_model_new(const Eina_Model_Type *type);
/**
 * @brief Frees the memory associated with @a model
 * @param model The model instance.
 *
 * @see eina_model_new()
 * @since 1.2
 */
EAPI void eina_model_del(Eina_Model *model) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the type of @a model.
 * @param model The model instance.
 * @return The type of @a model.
 *
 * @see eina_model_new()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI const Eina_Model_Type *eina_model_type_get(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Returns the interface named @a name of @a model.
 * @param model The model instance.
 * @param name Name of interface to get.
 * @return If successfull requested interface, NULL otherwise.
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
 * @brief Increases the refcount of @a model.
 * @param model The model to increase reference.
 * @return The @a model with reference increased.
 * @return If successfull pointer to model, NULL otherwise.
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
 * @return If successfull pointer to model, NULL otherwise.
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
 * @return If successfull pointer to model, NULL otherwise.
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
 * @defgroup Eina_Model_Event_Group Data Model Events
 * Events and their usage with models.
 *
 * Events are specified by each type and interface level using
 * #Eina_Model_Event_Description. One can know all events supported by
 * a model with eina_model_event_names_list_get() and then
 * eina_model_event_description_get() to retrieve details.
 *
 * By default the following events are supported in every object:
 * @li deleted: last reference was released or eina_model_del() was called.
 * @li freed: memory was destroyed, destructors were called.
 * @li property,set: eina_model_property_set() was done.
 * @li property,deleted: eina_model_property_del() was done.
 * @li children,changed: children was changed somehow (added, modified, deleted)
 * @li child,inserted: new child was added (eina_model_child_append() or eina_model_child_insert_at())
 * @li child,set: child was replaced (eina_model_child_set())
 * @li child,deleted: eina_model_child_del() was done.
 * @li loaded: eina_model_load() was done.
 * @li unloaded: eina_model_unload() was done.
 *
 * Mix-in interfaces may emit these:
 * @li properties,loaded
 * @li properties,unloaded
 * @li children,loaded
 * @li children,unloaded
 *
 * One can be notified of events with eina_model_event_callback_add().
 *
 * Types emit these events with eina_model_event_callback_call(),
 * these are handled asynchronously unless event is frozen with
 * eina_model_event_callback_freeze() is blocking it. In this case the
 * events are ignored. Usually this is used in some cases that want to
 * avoid storm of events in batch operations.
 *
 * @{
 */

/**
 * @typedef Eina_Model_Event_Cb
 * Notifies of events in this model.
 *
 * @since 1.2
 */
typedef void (*Eina_Model_Event_Cb)(void *data, Eina_Model *model, const Eina_Model_Event_Description *desc, void *event_info);

/**
 * @brief Add a callback to be called when @a event_name is emited.
 * @param model The model instance.
 * @param event_name The name of event for which @a cb will be called.
 * @param cb The function to be called.
 * @param data Data @a cb will be called with. May be NULL.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
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
 * @param model The model instance.
 * @param event_name The name of event for which to delete callback.
 * @param cb The function given to eina_model_event_callback_add().
 * @param data Data given to eina_model_event_callback_add(). A NULL value means
 * every @a data will not be compared.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @see eina_model_event_callback_add()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_event_callback_del(Eina_Model *model,
                                             const char *event_name,
                                             Eina_Model_Event_Cb cb,
                                             const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Returns a description of the event named @c event_name
 * @param model The model instance.
 * @param event_name Name of event whose description is wanted.
 * @return Description of event.
 *
 * @see Eina_Model_Event_Description
 * @since 1.2
 */
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
/**
 * @brief Frees the list of event's names gotten from
 * eina_model_event_names_list_get().
 * @param list The list to free.
 *
 * @see eina_model_event_names_list_get()
 * @since 1.2
 */
EAPI void eina_model_event_names_list_free(Eina_List *list);

/**
 * @brief Calls every callback associated to @a name on model @a model with @a
 * event_info.
 * @param model The model instance.
 * @param name The event whose callbacks will be called.
 * @param event_info The data given to the callback as event_info. May be NULL.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @see eina_model_event_callback_add()
 * @see eina_model_event_callback_del()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_event_callback_call(Eina_Model *model,
                                              const char *name,
                                              const void *event_info) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Makes @a model not call the callbacks associated with @a name.
 * @param model The model instance.
 * @param name The event whose callbacks are to be frozen.
 * @return Count of freezes called on this event.
 *
 * @see eina_model_event_callback_call()
 * @see eina_model_event_callback_thaw()
 * @since 1.2
 */
EAPI int eina_model_event_callback_freeze(Eina_Model *model,
                                          const char *name) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Makes @a model able to call the callbacks associated with @a name.
 * @param model The model instance.
 * @param name The event whose callbacks are to be frozen.
 * @return Count of freezes still valid in this event.
 *
 * @warning Behavior is undefined if called on a @a model, @a name not frozen.
 *
 * @see eina_model_event_callback_call()
 * @see eina_model_event_callback_freeze()
 * @since 1.2
 */
EAPI int eina_model_event_callback_thaw(Eina_Model *model,
                                        const char *name) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */


/**
 * @brief Makes a shallow copy of @a model.
 * @param model The model instance.
 * @return Copied model.
 *
 * The returned model will have a copy of the properties of @a model and a
 * reference to the children of @a model.
 *
 * @see eina_model_deep_copy()
 * @since 1.2
 */
EAPI Eina_Model *eina_model_copy(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_MALLOC;
/**
 * @brief Makes a deep(complete) copy of @a model.
 * @param model The model instance.
 * @return Copied model.
 *
 * The returned model will have a copy of the properties of @a model, its
 * children will be created by making a deep copy of the children of @a model.
 *
 * @see eina_model_copy()
 * @since 1.2
 */
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

/**
 * @brief Loads the @a model's data.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * By convention this means loading data from an external source and populating
 * the models properties and children with it. For example in the case of file
 * system backed model, this means opening the relevant files and reading the
 * data from them(creating the properties and children from it).
 * @warning This convention should be followed, but no guarantees of behaviour
 * by user defined types can be given.
 *
 * @note The types provided by Eina_Model don't implement this method.
 * @note Calling this function on a model that doesn't implement it returns @c
 * EINA_TRUE without any effect on @a model.
 *
 * @see eina_model_unload()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_load(Eina_Model *model) EINA_ARG_NONNULL(1);
/**
 * @brief Unloads the @a model's data.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * By convention this means releasing data gotten from an external source. For
 * example of a database backed model this might mean releasing the iterator for
 * the currently loaded data or deleting a temporary table.
 * @warning This convention should be followed, but no guarantees of behaviour
 * by user defined types can be given.
 *
 * @note The types provided by Eina_Model don't implement this method.
 * @note Calling this function on a model that doesn't implement it returns @c
 * EINA_TRUE without any effect on @a model.
 *
 * @see eina_model_load()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_unload(Eina_Model *model) EINA_ARG_NONNULL(1);


/**
 * @defgroup Eina_Model_Properties_Group Data Model Properties
 * Properties and their usage with models.
 *
 * Properties are attributes of model. They have a name and contain a
 * data value (@ref Eina_Value_Group).
 *
 * The actual values and their types, if it is possible to read and
 * write them and if new properties can be created or deleted it is up
 * to the type.
 *
 * @{
 */
/**
 * @brief Gets the value of @a model's property named @a name.
 * @param[in] model The model from which to get the property.
 * @param[in] name The name of the property whose value is wanted.
 * @param[out] value A pointer to an Eina_Value to receive the property's value.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @return EINA_TRUE if @a model has a property named @a name, EINA_FALSE
 * otherwise.
 *
 * @see eina_model_property_set()
 * @see eina_model_property_del()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_property_get(const Eina_Model *model,
                                       const char *name,
                                       Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Sets the value of @a model's property named @a name to @a value.
 * @param model The model in which to set the property.
 * @param name The name of the property whose value is to set.
 * @param value A pointer to a const Eina_Value to containing the property's
 * value.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @see eina_model_property_get()
 * @see eina_model_property_del()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_property_set(Eina_Model *model,
                                       const char *name,
                                       const Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Deletes @a model's property named @a name.
 * @param model The model from which to delete the property.
 * @param name The name of the property to delete.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @see eina_model_property_set()
 * @see eina_model_property_get()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_property_del(Eina_Model *model,
                                       const char *name) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Gets a list of the names of every property of @a model.
 * @param model The model instance.
 * @return #Eina_List of names.
 *
 * @note The returned list should be freed with @c
 * eina_model_properties_names_list_free().
 *
 * @see eina_model_properties_names_list_free()
 * @see eina_model_property_get()
 * @since 1.2
 */
EAPI Eina_List *eina_model_properties_names_list_get(const Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Frees a list of names of properties gotten with @c
 * eina_model_properties_names_list_get().
 * @param list The list to free.
 *
 * @warning Behavior is undefined if called on a list not gotten from @c
 * eina_model_properties_names_list_get().
 *
 * @see eina_model_properties_names_list_get()
 * @see eina_model_property_get()
 * @since 1.2
 */
EAPI void eina_model_properties_names_list_free(Eina_List *list);

/**
 * @}
 */

/**
 * @defgroup Eina_Model_Children_Group Data Model Children
 * Children and their usage with models.
 *
 * Children are other model instances that are kept sequentially in
 * the model. They are accessed by their integer index within the
 * model. Their index may change if child are inserted or deleted
 * before them, as in an array.
 *
 * @{
 */

/**
 * @brief Returns the number of child models in @a model.
 * @param model The model instance.
 * @return Number of children in @a model.
 *
 * @see eina_model_child_append()
 * @see eina_model_child_get()
 * @see eina_model_child_del()
 * @since 1.2
 */
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
 * @warning The returned model has its reference increased, you must release it
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
 *
 * @see eina_model_child_append()
 * @see eina_model_child_insert_at()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_child_set(Eina_Model *model,
                                    unsigned int position,
                                    Eina_Model *child) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Deletes the child model in @a position-th of @a model.
 * @param model The model instance.
 * @param position The position of the child to be deleted.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning This decrements the reference count of the child being deleted,
 * which may, or not, cause it to be deconstructed and freed.
 *
 * @see eina_model_child_append()
 * @see eina_model_child_get()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_child_del(Eina_Model *model,
                                    unsigned int position) EINA_ARG_NONNULL(1);

/**
 * @brief Insert @a child in the @a position-th of the list of children of @a
 * model.
 * @param model The model instance.
 * @param position Position in which to insert child.
 * @param child The child to be inserted.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning This increments the reference count of the child being inserted, if
 * it will no longer be used by the inserting code it should call
 * eina_model_unref() on it.
 *
 * @see eina_model_child_append()
 * @see eina_model_child_set()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_child_insert_at(Eina_Model *model,
                                          unsigned int position,
                                          Eina_Model *child) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Appends @a child in @a model.
 * @param model The model instance.
 * @param child The child to be appended.
 * @return The position of the added child, or -1 on failure.
 *
 * @warning This increments the reference count of the child being inserted, if
 * it will no longer be used by the inserting code it should call
 * eina_model_unref() on it.
 *
 * @see eina_model_child_insert_at()
 * @see eina_model_child_set()
 * @since 1.2
 */
EAPI int eina_model_child_append(Eina_Model *model,
                                 Eina_Model *child) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Returns the position of @a other amongst the children of @a model.
 * @param model The parent model whose children will be searched.
 * @param start_position The first children to be compared with @a other.
 * @param other The model whose position is desired.
 * @return The position of the searched for child, or -1 if not found.
 *
 * @since 1.2
 */
EAPI int eina_model_child_find(const Eina_Model *model,
                               unsigned int start_position,
                               const Eina_Model *other) EINA_ARG_NONNULL(1, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns the position of a child of @a model that mathes the criteria.
 * @param model The model whose children will be searched.
 * @param start_position The position of the first child to be checked.
 * @param match The function used to check if a child matches the criteria.
 * @param data Data given the to the @a match function.
 * @return The position of the first child to match the criteria or -1 if no
 * child matches it.
 *
 * Returns the position of the first(from @a start_position) child of @a model
 * to which @a match returns EINA_TRUE.
 *
 * @since 1.2
 */
EAPI int eina_model_child_criteria_match(const Eina_Model *model,
                                         unsigned int start_position,
                                         Eina_Each_Cb match,
                                         const void *data) EINA_ARG_NONNULL(1, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Sorts the children of @a model according to @a compare.
 * @param model The model instance.
 * @param compare The function to be used in the comparison.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * The @a compare function receives to const pointer to eina models(const
 * *Eina_Model).
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_model_child_sort(Eina_Model *model,
                                     Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @defgroup Eina_Model_Iterators_Group Data Model Iterators
 * Iterators and their usage with models.
 *
 * One of the most common tasks of models is to iterate over their
 * children, either forwards or backwards, filtering by some criteria
 * or a different ordering function.
 *
 * @{
 */

/**
 * @brief create an iterator that outputs a child model on each iteration.
 * @param model the model instance.
 * @return Newly created iterator instance on success or @c NULL on failure.
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
 * @warning Each iteration(call to eina_iterator_next()) gives a child model
 * with reference @b increased! You must call eina_model_unref() after you're
 * done with it.
 *
 * @see eina_model_child_slice_iterator_get()
 * @see eina_model_child_reversed_iterator_get()
 * @see eina_model_child_sorted_iterator_get()
 * @see eina_model_child_filtered_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_iterator_get(Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Gets an iterator to a slice of @a model's children.
 * @param model The model whose children to iterate over.
 * @param start The first child included in the iterator.
 * @param count The number of children included in the iterator.
 * @return Newly created iterator instance on success or @c NULL on failure.
 *
 * @warning Each iteration(call to eina_iterator_next()) gives a child model
 * with reference @b increased! You must call eina_model_unref() after you're
 * done with it.
 *
 * @see eina_model_child_iterator_get()
 * @see eina_model_child_slice_reversed_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_slice_iterator_get(Eina_Model *model,
                                                        unsigned int start,
                                                        unsigned int count) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief create an iterator that outputs a child model in reversed order.
 * @param model the model instance.
 * @return Newly created iterator instance on success or @c NULL on failure.
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
 * @warning Each iteration(call to eina_iterator_next()) gives a child model
 * with reference @b increased! You must call eina_model_unref() after you're
 * done with it.
 *
 * @see eina_model_child_slice_iterator_get()
 * @see eina_model_child_reversed_iterator_get()
 * @see eina_model_child_sorted_iterator_get()
 * @see eina_model_child_filtered_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_reversed_iterator_get(Eina_Model *model) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets a reversed iterator to a slice of @a model's children.
 * @param model The model whose children to iterate over.
 * @param start The first child included in the iterator.
 * @param count The number of children included in the iterator.
 * @return Newly created iterator instance on success or @c NULL on failure.
 *
 * @warning Each iteration(call to eina_iterator_next()) gives a child model
 * with reference @b increased! You must call eina_model_unref() after you're
 * done with it.
 *
 * @see eina_model_child_reversed_iterator_get()
 * @see eina_model_child_slice_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_slice_reversed_iterator_get(Eina_Model *model,
                                                                 unsigned int start,
                                                                 unsigned int count) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief create an iterator that outputs a child model using sort criteria.
 * @param model the model instance.
 * @param compare compare function to use as sort criteria.
 * @return Newly created iterator instance on success or @c NULL on failure.
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
 * @warning Each iteration(call to eina_iterator_next()) gives a child model
 * with reference @b increased! You must call eina_model_unref() after you're
 * done with it.
 *
 * @see eina_model_child_slice_iterator_get()
 * @see eina_model_child_reversed_iterator_get()
 * @see eina_model_child_sorted_iterator_get()
 * @see eina_model_child_filtered_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_sorted_iterator_get(Eina_Model *model,
                                                         Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Returns a sorted iterator to a slice of @a model's children.
 * @param model The model whose children to iterate over.
 * @param start The position(before sorting) of the first child included in
 * the iterator.
 * @param count The number of children included in the iterator.
 * @param compare The function used to sort the children.
 * @return Newly created iterator instance on success or @c NULL on failure.
 *
 * @warning Each iteration(call to eina_iterator_next()) gives a child model
 * with reference @b increased! You must call eina_model_unref() after you're
 * done with it.
 *
 * @see eina_model_child_slice_iterator_get()
 * @see eina_model_child_reversed_iterator_get()
 * @see eina_model_child_sorted_iterator_get()
 * @see eina_model_child_filtered_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_slice_sorted_iterator_get(Eina_Model *model,
                                                               unsigned int start,
                                                               unsigned int count,
                                                               Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief create an iterator that indexes of children that matches.
 * @param model the model instance.
 * @param match function to select children.
 * @param data extra context given to @a match function.
 * @return Newly created iterator instance on success or @c NULL on failure.
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

/**
 * @brief Returns a filtered slice of the @a model's children.
 * @param model The model whose children to iterate over.
 * @param start The position of the first child to be tested for inclusion in
 * the iterator.
 * @param count The number of children to be tested for inclusion in the
 * iterator.
 * @param match The function used to decide which children will be included in
 * the iterator.
 * @param data Data passed to the @a match function.
 * @return Newly created iterator instance on success or @c NULL on failure.
 *
 * @note Only children for whom @a match returns EINA_TRUE will be included in
 * the iterator.
 *
 * @note Each iteration(call to eina_iterator_next()) gives an integer index!
 *
 * @warning The iterator may have less than @a count children, but not more.
 *
 * @see eina_model_child_slice_iterator_get()
 * @see eina_model_child_reversed_iterator_get()
 * @see eina_model_child_sorted_iterator_get()
 * @see eina_model_child_filtered_iterator_get()
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_child_slice_filtered_iterator_get(Eina_Model *model,
                                                                 unsigned int start,
                                                                 unsigned int count,
                                                                 Eina_Each_Cb match,
                                                                 const void *data) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;


/**
 * @}
 */

/**
 * @brief Convert model to string.
 * @param model the model instance.
 * @return Newly allocated memory or @c NULL on failure.
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
 * Functions and structures related to implementing new types or
 * extending existing ones.
 *
 * All eina_model_type functions takes an Eina_Model_Type or
 * Eina_Model_Interface as parameter and may be used to validate or
 * query information about them.
 *
 * The functions with prefix eina_model_type that matches eina_model
 * counterparts, such as eina_model_type_compare() and
 * eina_model_compare() are used as "super", that is, calls the @c
 * compare() method of the given type (or its parent) instead of the
 * most specific type of provided Eina_Model.
 *
 * Examples:
 * @li @ref eina_model_02_example_page contains an easy to follow
 *     example that demonstrates several of the important features of
 *     eina_model, uses #EINA_MODEL_TYPE_GENERIC.
 * @li @ref eina_model_03_example_page walk-through example on how to
 *     inherit types, a suggestion of eina_model_load() usage and uses
 *     #EINA_MODEL_TYPE_STRUCT.
 *
 * @{
 */

/**
 * @def EINA_MODEL_TYPE_VERSION
 * Current API version, used to validate #_Eina_Model_Type.
 */
#define EINA_MODEL_TYPE_VERSION (1)

/**
 * @struct _Eina_Model_Type
 * API to access models.
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
 *
 * @note The methods @c setup and @c flush should exist if there is
 *       private data, otherwise memory may be uninitialized or leaks.
 * @note It is recommended that @c constructor and @c destructor exist
 *       to correctly do their roles and call parents in the correct
 *       order. Whenever they do not exist, their parent pointer is
 *       called.
 * @note a runtime check will enforce just types with ABI version
 *       #EINA_MODEL_TYPE_VERSION are used by comparing with the @c version
 *       member.
 *
 * @since 1.2
 */
struct _Eina_Model_Type
{
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

/**
 * @brief Calls the constructor of @a type for @a model.
 * @param type The type whose constructor will be called.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This should be used to call the parent's type constructor, something like:
 * @code
 * static Eina_Bool my_type_constructor(Eina_Model *m)
 * {
 *     // call parents constructor:
 *     if (!eina_model_type_constructor(MY_TYPE->parent, m))
 *        return EINA_FALSE;
 *     // do my stuff
 *     return EINA_TRUE;
 * }
 * @endcode
 * @note You should only do your type's initialization after the parent type has
 * done his own(this is as to ensure you can call on your parent's methods).
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_new()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_constructor(const Eina_Model_Type *type,
                                           Eina_Model *model) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Calls the destructor of @a type for @a model.
 * @param type The type whose destructor will be called.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This should be used to call the parent's type destructor, something like:
 * @code
 * static Eina_Bool my_type_destructor(Eina_Model *m)
 * {
 *     // do my stuff
 *     // call parents destructor:
 *     if (!eina_model_type_destructor(MY_TYPE->parent, m))
 *        return EINA_FALSE;
 *     return EINA_TRUE;
 * }
 * @endcode
 * @note It's considered good practice to free your type's resources before
 * calling the parent's destructor.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_del()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_destructor(const Eina_Model_Type *type,
                                          Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Calls the copy method of @a type for @a model.
 * @param type The type whose copy method will be called.
 * @param src Pointer to the model to be copied.
 * @param dst Pointer to where copy will be put.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_copy()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_copy(const Eina_Model_Type *type,
                                    const Eina_Model *src,
                                    Eina_Model *dst) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Calls the deep copy method of @a type for @a model.
 * @param type The type whose copy method will be called.
 * @param src Pointer to the model to be copied.
 * @param dst Pointer to where copy will be put.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_deep_copy()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_deep_copy(const Eina_Model_Type *type,
                                         const Eina_Model *src,
                                         Eina_Model *dst) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Calls the compare method of @a type for @a model.
 * @param[in] type The type whose compare method will be called.
 * @param[in] a Pointer to the first model to be compared.
 * @param[in] b Pointer to the second model to be compared.
 * @param[out] cmp The value of the comparison, 1 if @a b is greater than @a a,
 * -1 if @a b is smaller than @a a, 0 if @a a and @a b are equal.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_compare()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_compare(const Eina_Model_Type *type,
                                       const Eina_Model *a,
                                       const Eina_Model *b,
                                       int *cmp) EINA_ARG_NONNULL(1, 2, 3, 4);
/**
 * @brief Calls the load method of @a type for @a model.
 * @param type The type whose load method will be called.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_load()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_load(const Eina_Model_Type *type,
                                    Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Calls the unload method of @a type for @a model.
 * @param type The type whose unload method will be called.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_unload()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_unload(const Eina_Model_Type *type,
                                      Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Calls the property get method of @a type for @a model.
 * @param[in] type The type whose property get method will be called.
 * @param[in] model The model instance.
 * @param[in] name Name of property to get.
 * @param[out] value Pointer to where value of property will be placed.
 * @return EINA_TRUE if able to get property, EINA_FALSE otherwise.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_property_get()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_property_get(const Eina_Model_Type *type,
                                            const Eina_Model *model,
                                            const char *name,
                                            Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3, 4);
/**
 * @brief Calls the property set method of @a type for @a model.
 * @param type The type whose property set method will be called.
 * @param model The model instance.
 * @param name Name of property whose value will be set.
 * @param value The value to be set.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_property_set()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_property_set(const Eina_Model_Type *type,
                                            Eina_Model *model,
                                            const char *name,
                                            const Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3, 4);
/**
 * @brief Calls the property del method of @a type for @a model.
 * @param type The type whose property delete method will be called.
 * @param model The model instance.
 * @param name The name of the property to be deleted.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_property_del()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_property_del(const Eina_Model_Type *type,
                                            Eina_Model *model,
                                            const char *name) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Calls the properties name list method of @a type for @a model.
 * @param type The type whose property name list get method will be called.
 * @param model The model instance.
 * @return #Eina_List of properties' names.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_properties_names_list_get()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_List *eina_model_type_properties_names_list_get(const Eina_Model_Type *type,
                                                          const Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Calls the child count method of @a type for @a model.
 * @param type The type whose child count method will be called.
 * @param model The model instance.
 * @return Number of children in @a model.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_count()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI int eina_model_type_child_count(const Eina_Model_Type *type,
                                     const Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Calls the child get method of @a type for @a model.
 * @param type The type whose child get method will be called.
 * @param model The model instance.
 * @param position The position of the child to get.
 * @return The child model, or NULL on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_get()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Model *eina_model_type_child_get(const Eina_Model_Type *type,
                                           const Eina_Model *model,
                                           unsigned int position) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Calls the child set method of @a type for @a model.
 * @param type The type whose child set method will be called.
 * @param model The model instance.
 * @param position The position of the child to be set.
 * @param child Pointer to value(child) to be set.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_set()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_child_set(const Eina_Model_Type *type,
                                         Eina_Model *model,
                                         unsigned int position,
                                         Eina_Model *child) EINA_ARG_NONNULL(1, 2, 4);
/**
 * @brief Calls the child del method of @a type for @a model.
 * @param type The type whose child delete method will be called.
 * @param model The model instance.
 * @param position Position of child to be deleted.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_del()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_child_del(const Eina_Model_Type *type,
                                         Eina_Model *model,
                                         unsigned int position) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Calls the child insert at method of @a type for @a model.
 * @param type The type whose child insert method will be called.
 * @param model The model instance.
 * @param position Position in which @a child will be inserted.
 * @param child The child to be inserted.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_insert_at()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_child_insert_at(const Eina_Model_Type *type,
                                               Eina_Model *model,
                                               unsigned int position,
                                               Eina_Model *child) EINA_ARG_NONNULL(1, 2, 4);
/**
 * @brief Calls the child find method of @a type for @a model.
 * @param type The type whose find method will be called.
 * @param model The model instance.
 * @param start_position The first position to search for.
 * @param other The child being searched for.
 * @return The index of the searched child, or -1 if not found.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_find()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI int eina_model_type_child_find(const Eina_Model_Type *type,
                                    const Eina_Model *model,
                                    unsigned int start_position,
                                    const Eina_Model *other) EINA_ARG_NONNULL(1, 2, 4);
/**
 * @brief Calls the child criteria match method of @a type for @a model.
 * @param type The type whose child criteria match method will be called.
 * @param model The model instance.
 * @param start_position The first position to be checked.
 * @param match The function used to determine if a child matches the criteria.
 * @param data Data given to the @a match function. May be NULL.
 * @return The position of the first child to match the criteria or -1 if no
 * child matches it.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_criteria_match()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI int eina_model_type_child_criteria_match(const Eina_Model_Type *type,
                                              const Eina_Model *model,
                                              unsigned int start_position,
                                              Eina_Each_Cb match,
                                              const void *data) EINA_ARG_NONNULL(1, 2, 4);
/**
 * @brief Calls the child sort method of @a type for @a model.
 * @param type The type whose child sort method will be called.
 * @param model The model instance.
 * @param compare Function used to compare children.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_sort()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI void eina_model_type_child_sort(const Eina_Model_Type *type,
                                     Eina_Model *model,
                                     Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Calls the child iterator get method of @a type for @a model.
 * @param type The type whose child iterator get method will be called.
 * @param model The model instance.
 * @param start The first child to be a part of the iterator.
 * @param count The number of children included in the iterator.
 * @return Newly created iterator instance on success or @c NULL on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_iterator_get()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_type_child_iterator_get(const Eina_Model_Type *type,
                                                       Eina_Model *model,
                                                       unsigned int start,
                                                       unsigned int count) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Calls the child reversed iterator get method of @a type for @a model.
 * @param type The type whose child reversed iterator get method will be called.
 * @param model The model instance.
 * @param start The first child to be a part of the iterator.
 * @param count The number of children included in the iterator.
 * @return Newly created iterator instance on success or @c NULL on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_reversed_iterator_get()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_type_child_reversed_iterator_get(const Eina_Model_Type *type,
                                                                Eina_Model *model,
                                                                unsigned int start,
                                                                unsigned int count) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Calls the child sorted iterator get method of @a type for @a model.
 * @param type The type whose child sorted iterator get method will be called.
 * @param model The model instance.
 * @param start The first child to be a part of the iterator.
 * @param count The number of children included in the iterator.
 * @param compare Function used to compare children.
 * @return Newly created iterator instance on success or @c NULL on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_sorted_iterator_get()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_type_child_sorted_iterator_get(const Eina_Model_Type *type,
                                                              Eina_Model *model,
                                                              unsigned int start,
                                                              unsigned int count,
                                                              Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 5);
/**
 * @brief Calls the child filtered get method of @a type for @a model.
 * @param type The type whose child filtered iterator get method will be called.
 * @param model The model instance.
 * @param start The first child to be a part of the iterator.
 * @param count Number of children to be checked for inclusion in the iterator.
 * @param match Function used to check if child will be included in the iterator.
 * @param data Data given to the @a match function. May be NULL.
 * @return Newly created iterator instance on success or @c NULL on failure.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_child_filtered_iterator_get()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Iterator *eina_model_type_child_filtered_iterator_get(const Eina_Model_Type *type,
                                                                Eina_Model *model,
                                                                unsigned int start,
                                                                unsigned int count,
                                                                Eina_Each_Cb match,
                                                                const void *data) EINA_ARG_NONNULL(1, 2, 5);
/**
 * @brief Calls the to string method of @a type for @a model.
 * @param type The type whose to string method will be called.
 * @param model The model instance.
 * @return String representationof @a model.
 *
 * @warning If model doesn't inherit from(or is of) @a type does nothing and
 * returns EINA_FALSE.
 *
 * @see eina_model_to_string()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI char *eina_model_type_to_string(const Eina_Model_Type *type,
                                     const Eina_Model *model) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_MALLOC;


/**
 * @brief Get resolved method from types that extend Eina_Model_Type given @a offset.
 * @param model the model to query the method
 * @param offset the byte offset in the structure given as type, it
 *        must be bigger than Eina_Model_Type itself.
 * @return Address to resolved method, or @c NULL if method is not implemented.
 *
 * The use of this function is discouraged, you should use
 * eina_model_method_resolve() instead.
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

/**
 * @brief Gets resolved method from @a type of @a model given @a offset.
 * @param type The type whose method offset resolve method will be called.
 * @param model The model instance.
 * @param offset The offset of the wanted method.
 * @return Address to resolved method, or @c NULL if method is not implemented.
 *
 * @see eina_model_method_offset_resolve()
 * @since 1.2
 */
EAPI const void *eina_model_type_method_offset_resolve(const Eina_Model_Type *type, const Eina_Model *model, unsigned int offset) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

#define eina_model_method_resolve(model, struct_type, method) eina_model_method_offset_resolve((model), offsetof(struct_type, method))

#define eina_model_type_method_resolve(type, model, struct_type, method) eina_model_type_method_offset_resolve((type), (model), offsetof(struct_type, method))

/**
 * @def EINA_MODEL_INTERFACE_VERSION
 * Current API version, used to validate #_Eina_Model_Interface.
 */
#define EINA_MODEL_INTERFACE_VERSION (1)

/**
 * @struct _Eina_Model_Interface
 * API to access models.
 *
 * Interfaces are managed by name, then multiple Eina_Model_Interface
 * may have the same name meaning it implements that name.
 *
 * Each interface will get its own private data of size @c
 * private_size (defined at each sub interface), this can be retrieved
 * with eina_model_interface_private_data_get().
 *
 * Private are created @b automatically and should be setup with @c
 * setup and flushed with @c flush. All interfaces functions that
 * exist are called! Don't call your parent's @c setup or @c flush!
 * The setup is done from parent to child. Flush is done from child to
 * parent (topological sort is applied to interface graph).
 *
 * @note The methods @c setup and @c flush should exist if there is
 *       private data, otherwise memory may be uninitialized or leaks.
 * @note It is recommended that @c constructor and @c destructor exist
 *       to correctly do their roles and call parents in the correct
 *       order. Whenever they do not exist, their parent pointer is
 *       called.
 * @note Interface's constructor and destructor are only called by
 *       type counterparts. Unlike setup and flush, they are not
 *       guaranteed to be called.
 * @note use the same name pointer on queries to speed up the lookups!
 * @note a runtime check will enforce just types with ABI version
 *       #EINA_MODEL_INTERFACE_VERSION are used by comparing with the
 *       @c version member.
 *
 * @since 1.2
 */
struct _Eina_Model_Interface
{
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
   void *__extension_ptr0; /**< not to be used @internal */
   void *__extension_ptr1; /**< not to be used @internal */
   void *__extension_ptr2; /**< not to be used @internal */
   void *__extension_ptr3; /**< not to be used @internal */
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

/**
 * @brief Calls the constructor of @a iface on @a model.
 * @param iface The interface whose constructor will be called.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If @a model doesn't implement @a iface does nothing and returns
 * EINA_FALSE.
 *
 * @see eina_model_new()
 * @see _Eina_Model_Interface
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_constructor(const Eina_Model_Interface *iface,
                                                Eina_Model *model) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Calls the destructor of @a iface on @a model.
 * @param iface The interface whose destructor will be called.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If @a model doesn't implement @a iface does nothing and returns
 * EINA_FALSE.
 *
 * @see eina_model_del()
 * @see _Eina_Model_Interface
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_destructor(const Eina_Model_Interface *iface,
                                               Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Calls the copy method of @a iface on @a model.
 * @param iface The interface whose copy method will be called.
 * @param src Pointer to the model to be copied.
 * @param dst Pointer to where copy will be put.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If @a model doesn't implement @a iface does nothing and returns
 * EINA_FALSE.
 *
 * @see eina_model_copy()
 * @see _Eina_Model_Interface
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_copy(const Eina_Model_Interface *iface,
                                         const Eina_Model *src,
                                         Eina_Model *dst) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Calls the deep copy method of @a iface on @a model.
 * @param iface The interface whose deep copy method will be called.
 * @param src Pointer to the model to be copied.
 * @param dst Pointer to where copy will be put.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If @a model doesn't implement @a iface does nothing and returns
 * EINA_FALSE.
 *
 * @see eina_model_deep_copy()
 * @see _Eina_Model_Interface
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_deep_copy(const Eina_Model_Interface *iface,
                                              const Eina_Model *src,
                                              Eina_Model *dst) EINA_ARG_NONNULL(1, 2, 3);

#define eina_model_interface_method_resolve(iface, model, struct_type, method) eina_model_interface_method_offset_resolve((iface), (model), offsetof(struct_type, method))

/**
 * @brief Gets the @a iface's method for @a model at @a offset.
 * @param iface The interface whose method offset resolve method will be called.
 * @param model The model instance.
 * @param offset The offset of the wanted method.
 * @return Address to resolved method, or @c NULL if method is not implemented.
 *
 * @see eina_model_method_offset_resolve()
 * @since 1.2
 */
EAPI const void *eina_model_interface_method_offset_resolve(const Eina_Model_Interface *iface, const Eina_Model *model, unsigned int offset) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;


/**
 * @struct _Eina_Model_Event_Description
 * Data Model Event Description.
 *
 * @see EINA_MODEL_EVENT_DESCRIPTION()
 * @see #EINA_MODEL_EVENT_DESCRIPTION_SENTINEL
 * @since 1.2
 */
struct _Eina_Model_Event_Description
{
   const char *name; /**< name used for lookups */
   const char *type; /**< used for introspection purposes, documents what goes as callback event information (@c event_info) */
   const char *doc; /**< documentation for introspection purposes */
};

/**
 * @def EINA_MODEL_EVENT_DESCRIPTION
 *
 * Helper to define Eina_Model_Event_Description fields.
 *
 * @since 1.2
 */
#define EINA_MODEL_EVENT_DESCRIPTION(name, type, doc) {name, type, doc}

/**
 * @def EINA_MODEL_EVENT_DESCRIPTION_SENTINEL
 *
 * Helper to define Eina_Model_Event_Description fields for sentinel (last
 * item).
 *
 * @since 1.2
 */
#define EINA_MODEL_EVENT_DESCRIPTION_SENTINEL {NULL, NULL, NULL}

/**
 * @brief Check @a type is valid.
 * @param type The type to be checked.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_check(const Eina_Model_Type *type) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;
/**
 * @brief Gets the name @a type.
 * @param type The type whose name is wanted.
 * @return Name of @a type.
 *
 * @since 1.2
 */
EAPI const char *eina_model_type_name_get(const Eina_Model_Type *type) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;
/**
 * @brief Gets the parent type of @a type.
 * @param type The type whose parent is wanted.
 * @return Type of parent.
 *
 * @since 1.2
 */
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

/**
 * @brief Checks if @a type is a subclass of(or the same as) @a self_or_parent.
 * @param type The type to be checked.
 * @param self_or_parent The type being checked for.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_model_type_subclass_check(const Eina_Model_Type *type,
                                              const Eina_Model_Type *self_or_parent) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;


/**
 * @brief Gets a interface with name @a name from @a type.
 * @param type The type instance.
 * @param name The name of the desired interface.
 * @return The interface implemented by @a type with name @a name, or null if
 * this type doesn't implement any interface with name @a name.
 *
 * @since 1.2
 */
EAPI const Eina_Model_Interface *eina_model_type_interface_get(const Eina_Model_Type *type,
                                                               const char *name) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Gets the private date of @a model for type @a type.
 * @param model The model instance.
 * @param type The type whose private data will be gotten.
 * @return Pointer to type's private data.
 *
 * @since 1.2
 */
EAPI void *eina_model_type_private_data_get(const Eina_Model *model,
                                            const Eina_Model_Type *type) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

/**
 * @brief Checks if @a iface is a valid interface.
 * @param iface The interface instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_check(const Eina_Model_Interface *iface) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Gets the private date of @a model for interface @a iface.
 * @param model The model instance.
 * @param iface The interface whose private data will be gotten.
 * @return Pointer to interface's private data.
 *
 * @since 1.2
 */
EAPI void *eina_model_interface_private_data_get(const Eina_Model *model,
                                                 const Eina_Model_Interface *iface) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

/**
 * @def EINA_MODEL_INTERFACE_PROPERTIES_VERSION
 * Current API version, used to validate #_Eina_Model_Interface_Properties.
 */
#define EINA_MODEL_INTERFACE_PROPERTIES_VERSION (1)

/**
 * @typedef Eina_Model_Interface_Properties
 * Interface to manage model's properties.
 *
 * This extends #Eina_Model_Interface as expected by interface name
 * #EINA_MODEL_INTERFACE_NAME_PROPERTIES.
 *
 * This interface is meant to help managing properties of a model, it
 * is used by #EINA_MODEL_TYPE_MIXIN in order to configure methods for
 * children independently from properties.
 *
 * @see #_Eina_Model_Interface_Properties explains fields.
 * @since 1.2
 */
typedef struct _Eina_Model_Interface_Properties Eina_Model_Interface_Properties;

/**
 * @struct _Eina_Model_Interface_Properties
 * Interface to manage model's properties.
 *
 * This extends #Eina_Model_Interface as expected by interface name
 * #EINA_MODEL_INTERFACE_NAME_PROPERTIES.
 *
 * This interface is meant to help managing properties of a model, it
 * is used by #EINA_MODEL_TYPE_MIXIN in order to configure methods for
 * children independently from properties.
 *
 * @since 1.2
 */
struct _Eina_Model_Interface_Properties
{
   Eina_Model_Interface base; /**< common interface methods */
   unsigned int version; /**< must be #EINA_MODEL_INTERFACE_PROPERTIES_VERSION */
   Eina_Bool (*compare)(const Eina_Model *a, const Eina_Model *b, int *cmp); /**< How to compare properties of this model */
   Eina_Bool (*load)(Eina_Model *model); /**< How to load properties of this model */
   Eina_Bool (*unload)(Eina_Model *model); /**< How to unload properties of this model */
   Eina_Bool (*get)(const Eina_Model *model, const char *name, Eina_Value *value); /**< Retrieve a property of this model given its name. The value will be returned as a copy and must be flushed with eina_value_flush(). The previous contents of value is ignored. */
   Eina_Bool (*set)(Eina_Model *model, const char *name, const Eina_Value *value); /**< Set a property of this model given its name. The value is assumed to be valied and is copied internally, thus it can be safely cleared with eina_value_flush() after this function returns. */
   Eina_Bool (*del)(Eina_Model *model, const char *name); /**< Delete a property given its name */
   Eina_List *(*names_list_get)(const Eina_Model *model); /**< List of stringshare with known property names */
};

/**
 * @brief Compares properties using @a iface's comparing function.
 *
 * @param[in] iface The interface used to compare the properties.
 * @param[in] a The first model whose properties will be compared.
 * @param[in] b The second model whose properties will be compared.
 * @param[out] cmp A pointer to an integer which will contain the result of the
 * comparison.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return EINA_FALSE.
 *
 * @see eina_model_compare()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_properties_compare(const Eina_Model_Interface *iface,
                                                       const Eina_Model *a,
                                                       const Eina_Model *b,
                                                       int *cmp) EINA_ARG_NONNULL(1, 2, 3, 4) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Loads properties using @a iface's loading function.
 * @param iface The properties interface whose load method will be called.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return EINA_FALSE.
 *
 * @see eina_model_load()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_properties_load(const Eina_Model_Interface *iface,
                                                    Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Unloads properties using @a iface's unloading function.
 * @param iface The properties interface whose unload method will be called.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return EINA_FALSE.
 *
 * @see eina_model_unload()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_properties_unload(const Eina_Model_Interface *iface,
                                                      Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Gets property named @a name using @a iface's function to get properties.
 * @param iface The properties interface whose property get method will be called.
 * @param model The model instance.
 * @param name The name of the property to get.
 * @param value Pointer to where value will be stored.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return EINA_FALSE.
 *
 * @see eina_model_property_get()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_properties_get(const Eina_Model_Interface *iface,
                                                   const Eina_Model *model,
                                                   const char *name,
                                                   Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3, 4);
/**
 * @brief Sets property named @a name using @a iface's function to set properties.
 * @param iface The properties interface whose property set method will be called.
 * @param model The model instance.
 * @param name The name of the property to set.
 * @param value The value to be set.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return EINA_FALSE.
 *
 * @see eina_model_property_set()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_properties_set(const Eina_Model_Interface *iface,
                                                   Eina_Model *model,
                                                   const char *name,
                                                   const Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3, 4);
/**
 * @brief Deletes property named @a name using @a iface's function to delete properties.
 * @param iface The properties interface whose property delete method will be called.
 * @param model The model instance.
 * @param name The name of the property to delete.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return EINA_FALSE.
 *
 * @see eina_model_property_del()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_properties_del(const Eina_Model_Interface *iface,
                                                   Eina_Model *model,
                                                   const char *name) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Gets properties name list using @a iface's function to get properties
 * name list.
 * @param iface The properties interface whose property name list get method
 * will be called.
 * @param model The model instance.
 * @return #Eina_List of properties' names.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return EINA_FALSE.
 *
 * @see eina_model_properties_names_list_get()
 * @since 1.2
 */
EAPI Eina_List *eina_model_interface_properties_names_list_get(const Eina_Model_Interface *iface,
                                                               const Eina_Model *model) EINA_ARG_NONNULL(1, 2); /**< list of stringshare */

/**
 * @typedef Eina_Model_Interface_Children
 * Interface to manage model's children.
 *
 * This extends #Eina_Model_Interface as expected by interface name
 * #EINA_MODEL_INTERFACE_NAME_CHILDREN.
 *
 * This interface is meant to help managing properties of a model, it
 * is used by #EINA_MODEL_TYPE_MIXIN in order to configure methods for
 * children independently from properties.
 *
 * @see #_Eina_Model_Interface_Children explains fields.
 * @since 1.2
 */
typedef struct _Eina_Model_Interface_Children Eina_Model_Interface_Children;

/**
 * @def EINA_MODEL_INTERFACE_CHILDREN_VERSION
 * Current API version, used to validate #_Eina_Model_Interface_Children.
 */
#define EINA_MODEL_INTERFACE_CHILDREN_VERSION (1)

/**
 * @struct _Eina_Model_Interface_Children
 * Interface to manage model's children.
 *
 * This extends #Eina_Model_Interface as expected by interface name
 * #EINA_MODEL_INTERFACE_NAME_CHILDREN.
 *
 * This interface is meant to help managing properties of a model, it
 * is used by #EINA_MODEL_TYPE_MIXIN in order to configure methods for
 * children independently from properties.
 *
 * @since 1.2
 */
struct _Eina_Model_Interface_Children
{
   Eina_Model_Interface base; /**< common interface methods */
   unsigned int version; /**< must be #EINA_MODEL_INTERFACE_CHILDREN_VERSION */
   Eina_Bool (*compare)(const Eina_Model *a, const Eina_Model *b, int *cmp); /**< How to compare children of this model */
   Eina_Bool (*load)(Eina_Model *model); /**< How to load children of this model */
   Eina_Bool (*unload)(Eina_Model *model); /**< How to unload children of this model */
   int (*count)(const Eina_Model *model); /**< How many children of this model */
   Eina_Model *(*get)(const Eina_Model *model, unsigned int position); /**< Retrieve a child of this model, returned child must have reference increased! */
   Eina_Bool (*set)(Eina_Model *model, unsigned int position, Eina_Model *child); /**< Set (replace) a child of this model, given child will have reference increased! */
   Eina_Bool (*del)(Eina_Model *model, unsigned int position); /**< Delete a child of this model. Existing child will have reference decreased! */
   Eina_Bool (*insert_at)(Eina_Model *model, unsigned int position, Eina_Model *child); /**< Insert a child into this model, given child will have reference increased! All elements towards the end of the internal list will be shifted to the end to make room for the new child. */
   void (*sort)(Eina_Model *model, Eina_Compare_Cb compare); /**< Reorder children to be sorted respecting comparison function @c compare() */
};

/**
 * @brief Compares children using @a iface's comparing function.
 *
 * @param[in] iface The interface used to compare the properties.
 * @param[in] a The first model whose properties will be compared.
 * @param[in] b The second model whose properties will be compared.
 * @param[out] cmp A pointer to an integer which will contain the result of the
 * comparison.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return EINA_FALSE.
 *
 * @see eina_model_compare()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_children_compare(const Eina_Model_Interface *iface,
                                                     const Eina_Model *a,
                                                     const Eina_Model *b,
                                                     int *cmp) EINA_ARG_NONNULL(1, 2, 3, 4) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Loads children using @a iface's loading function.
 * @param iface The children interface whose load method will be called.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return EINA_FALSE.
 *
 * @see eina_model_load()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_children_load(const Eina_Model_Interface *iface,
                                                  Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Unloads children using @a iface's unloading function.
 * @param iface The children interface whose unload method will be called.
 * @param model The model instance.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return EINA_FALSE.
 *
 * @see eina_model_unload()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_children_unload(const Eina_Model_Interface *iface,
                                                    Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Count children using @a iface's counting function.
 * @param iface The children interface whose count method will be called.
 * @param model The model instance.
 * @return Number of children in @a model.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return -1.
 *
 * @see eina_model_child_count()
 * @since 1.2
 */
EAPI int eina_model_interface_children_count(const Eina_Model_Interface *iface,
                                             const Eina_Model *model) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Get child using @a iface's function to get children.
 * @param iface The children interface whose get method will be called.
 * @param model The model instance.
 * @param position Position of child to be retrieved.
 * @return The requested child.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return -1.
 *
 * @see eina_model_child_get()
 * @since 1.2
 */
EAPI Eina_Model *eina_model_interface_children_get(const Eina_Model_Interface *iface,
                                                   const Eina_Model *model,
                                                   unsigned int position) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Set child using @a iface's function to set children.
 * @param iface The children interface whose set method will be called.
 * @param model The model instance.
 * @param position Position of child to be set.
 * @param child Value(child) to be set.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return -1.
 *
 * @see eina_model_child_set()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_children_set(const Eina_Model_Interface *iface,
                                                 Eina_Model *model,
                                                 unsigned int position,
                                                 Eina_Model *child) EINA_ARG_NONNULL(1, 2, 4);
/**
 * @brief Delete child using @a iface's function to delete children.
 * @param iface The children interface whose delete method will be called.
 * @param model The model instance.
 * @param position Position of child to be deleted.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return -1.
 *
 * @see eina_model_child_del()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_children_del(const Eina_Model_Interface *iface,
                                                 Eina_Model *model,
                                                 unsigned int position) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Insert child using @a iface's function to insert children.
 * @param iface The children interface whose insert method will be called.
 * @param model The model instance.
 * @param position Position in which to insert @a child.
 * @param child Value(child) to be inserted.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return -1.
 *
 * @see eina_model_child_insert_at()
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_children_insert_at(const Eina_Model_Interface *iface,
                                                       Eina_Model *model,
                                                       unsigned int position,
                                                       Eina_Model *child) EINA_ARG_NONNULL(1, 2, 4);
/**
 * @brief Sort children using @a iface's function to sort children.
 * @param iface The children interface whose sort method will be called.
 * @param model The model instance.
 * @param compare Function used to compare children.
 *
 * @warning If either model doesn't implement @a iface will do nothing and
 * return -1.
 *
 * @see eina_model_child_sort().
 * @since 1.2
 */
EAPI void eina_model_interface_children_sort(const Eina_Model_Interface *iface,
                                             Eina_Model *model,
                                             Eina_Compare_Cb compare) EINA_ARG_NONNULL(1, 2, 3);


/**
 * @}
 */

/**
 * @defgroup Eina_Model_Utils_Group Data Model Utilities
 *
 * Miscellaneous utilities to help usage or debug of @ref Eina_Model_Group.
 *
 * @{
 */

/**
 * @brief Checks if @a model is an instance of @a type.
 * @param model The model instance.
 * @param type The type being checked for.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @see eina_model_new()
 * @see _Eina_Model_Type
 * @since 1.2
 */
EAPI Eina_Bool eina_model_instance_check(const Eina_Model *model,
                                         const Eina_Model_Type *type) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Checks if @a model implements @a iface.
 * @param model The model instance.
 * @param iface The interface being checked for.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * @see _Eina_Model_Interface
 * @since 1.2
 */
EAPI Eina_Bool eina_model_interface_implemented(const Eina_Model *model, const Eina_Model_Interface *iface) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Returns the number of references to @a model.
 * @param model The model to query number of references.
 * @return Number of references to model.
 *
 * @see eina_model_ref()
 * @see eina_model_unref()
 * @see eina_model_xref()
 * @see eina_model_xunref()
 * @see eina_model_xrefs_get()
 * @since 1.2
 */
EAPI int eina_model_refcount(const Eina_Model *model) EINA_ARG_NONNULL(1);

/**
 * @typedef Eina_Model_XRef
 * Extended reference to model.
 *
 * This is returned by eina_model_xrefs_get() and should never be
 * modified. It is managed by eina_model_xref() and
 * eina_model_xunref() when @c EINA_MODEL_DEBUG is set to "1" or
 * "backtrace".
 *
 * @see #_Eina_Model_XRef explains fields.
 * @since 1.2
 */
typedef struct _Eina_Model_XRef Eina_Model_XRef;

/**
 * @struct _Eina_Model_XRef
 * Extended reference to model.
 *
 * This is returned by eina_model_xrefs_get() and should never be
 * modified. It is managed by eina_model_xref() and
 * eina_model_xunref() when @c EINA_MODEL_DEBUG is set to "1" or
 * "backtrace".
 *
 * @see eina_model_xrefs_get()
 * @see eina_models_usage_dump()
 * @since 1.2
 */
struct _Eina_Model_XRef
{
   EINA_INLIST;
   const void *id; /**< as given to eina_model_xref() */
   struct {
      const void * const *symbols; /**< only if @c EINA_MODEL_DEBUG=backtrace is set, otherwise is @c NULL */
      unsigned int count; /**< only if @c EINA_MODEL_DEBUG=backtrace is set, otherwise is 0 */
   } backtrace;
   char label[]; /**< Any given label given to eina_model_xref(). */
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

/**
 * @brief Create and setup an instance of #EINA_MODEL_TYPE_STRUCT.
 * @param desc struct description to use for properties.
 * @return newly created and set model, or @c NULL on errors.
 *
 * @see eina_model_type_struct_new()
 * @since 1.2
 */
EAPI Eina_Model *eina_model_struct_new(const Eina_Value_Struct_Desc *desc) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * @brief Create and setup an instance of type subclass of #EINA_MODEL_TYPE_STRUCT.
 * @param type a type which is subclass of #EINA_MODEL_TYPE_STRUCT.
 * @param desc struct description to use for properties.
 * @return newly created and set model, or @c NULL on errors.
 *
 * @see eina_model_struct_new()
 * @since 1.2
 */
EAPI Eina_Model *eina_model_type_struct_new(const Eina_Model_Type *type,
                                            const Eina_Value_Struct_Desc *desc) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_MALLOC;


/**
 * @brief Configure the internal properties of model implementing #EINA_MODEL_INTERFACE_PROPERTIES_STRUCT.
 * @param model The model instance to configure.
 * @param desc The structure description to use.
 * @param memory If not @c NULL, will be copied by model.
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This will setup the internal pointers so that the given @a desc is
 * used to manage the properties of this struct.
 *
 * If a given memory is provided, it will be copied (including
 * members) and no references are taken after this function returns.
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
 * @var EINA_MODEL_INTERFACE_NAME_PROPERTIES
 *
 * Interface that uses #Eina_Model_Interface_Properties as
 * #Eina_Model_Interface and can manage the model properties.
 *
 * @since 1.2
 */
EAPI extern const char *EINA_MODEL_INTERFACE_NAME_PROPERTIES;

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
 * Eina_Value_Struct::desc information that can be set by types such
 * as #EINA_MODEL_TYPE_STRUCT
 *
 * @see EINA_MODEL_INTERFACE_PROPERTIES_HASH
 *
 * @since 1.2
 */
EAPI extern const Eina_Model_Interface *EINA_MODEL_INTERFACE_PROPERTIES_STRUCT;

/**
 * @var EINA_MODEL_INTERFACE_NAME_CHILDREN
 *
 * Interface that uses #Eina_Model_Interface_Children as
 * #Eina_Model_Interface and can manage the model children.
 *
 * @since 1.2
 */
EAPI extern const char *EINA_MODEL_INTERFACE_NAME_CHILDREN;

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
