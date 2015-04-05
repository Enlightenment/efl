/**
    @brief Emodel Library Public API Calls
    These routines are used for MVC (Model View Controller) Library interaction.
 */
/**
 * @page emodel_main Emodel
 *
 * @date 2014 (created)
 * @section toc Table of Contents
 *
 * @li @ref emodel_main_intro
 * @li @ref emodel_main_intro_example
 *
 * @section emodel_main_intro Introduction
 *
 * The Emodel(model) generic object system for Emodel View Controller.
 *
 * @section emodel_main_work How does Emodel work?
 *
 * The Model notifies Views and Controllers when there is a change in its state.
 * The other way around is also true and the Model can be passive and be poolled
 * for update rather than generating output representations.
 *
 * Concrete objects can implement functionalities by overriding Emodel's API, abstracting its complexities from
 * applications that can keep focus on higher level implementation.
 *
 * Examples of concrete implementations that can make use Emodel:
 *
 * Filesystem and I/O operations;
 * Database management;
 * GUI forms, lists and trees.
 *
 * Application code tends to be small in number of lines,
 * more simple and code readability is improved.
 *
 * Emodel use EO Events. Views and Controllers must register
 * in events to be able to recieve notifications about state changes in Emodel.
 * Some of currently available events are:
 *
 *   EMODEL_EVENT_LOAD_STATUS
 *   EMODEL_EVENT_PROPERTIES_CHANGED
 *   EMODEL_EVENT_CHILD_ADDED
 *   EMODEL_EVENT_CHILD_REMOVED
 *   EMODEL_EVENT_CHILDREN_COUNT_CHANGED
 *
 *   Example code using Emodel_Eio that returns the number of files in '/tmp' directory:
 *
 *   @code
 * static Eina_Bool
 * _children_count_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
 * {
 *    size_t *len = event_info;
 *    fprintf(stdout, "Children count len=%lu\n", *len);
 *    return EINA_TRUE;
 * }
 *
 * int
 * main(int argc, const char **argv)
 * {
 *    size_t total;
 *    Eo *model;
 *    if(!ecore_init())
 *    {
 *       fprintf(stderr, "ERROR: Cannot init Ecore!\n");
 *       return 1;
 *    }
 *    if(!eio_init())
 *    {
 *       fprintf(stderr, "ERROR: Cannot init Eio!\n");
 *       return 1;
 *    }
 *
 *    model = eo_add_custom(EIO_MODEL_CLASS, NULL, eio_model_constructor("/tmp"));
 *    eo_do(model, eo_event_callback_add(EMODEL_EVENT_CHILDREN_COUNT_CHANGED, _children_count_cb, NULL));
 *    eo_do(model, emodel_children_count_get(&total));
 *    fprintf(stdout, "total=%lu\n", total);
 *
 *    ecore_main_loop_begin();
 *    eo_unref(model);
 *    eio_shutdown();
 *    ecore_shutdown();
 *    return 0;
 * }
 *   @endcode
 *
 *   In previous example the concrete Emodel_Eio counts, asynchronously, the number of files in given directory,
 *   emodel_children_count_get() returns into 'total' pointer the last known number of children. In the meantime
 *   when background count is finished _children_count_cb() is invoked receiving the number of files as event_info data.
 *   This is achieved by registering the Model as EMODEL_EVENT_CHILDREN_COUNT_CHANGED event listener and every time
 *   the count (number of children) changes, the event is disptached to listeners.
 *
 *   The principles may remain the same for different events and the logic remains.
 *
 * @li @ref emodel_main_intro_example
 *
 * @include emodel_test_file.c
 *
 * Recommended reading:
 *
 * @li @ref Eo, where you'll understand how Eo Events work.
 * @li @ref Eio, where you'll find EFL Eio implementation and interfaces.
 * @li @ref Emodel_Eio, the concrete EIO implementation using both Emodel and Eio.
 * @li @ref Ecore, You'll get more information about I/O filesystem events.
 *
 * @defgroup Emodel
 *
 * @addtogroup Emodel
 * @{
 */

#ifndef _EMODEL_H
#define _EMODEL_H

#include <Efl_Config.h>
#include <Eo.h>
#include <Emodel_Common.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <emodel.eo.h>

#ifdef __cplusplus
}
#endif
#endif //_EMODEL_H
