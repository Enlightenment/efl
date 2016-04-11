#ifndef _CODEGEN_EXAMPLE_GENERATED_H
#define _CODEGEN_EXAMPLE_GENERATED_H

#include <Edje.h>
#include <Elementary.h>
#include <Evas.h>

#include <stdlib.h>

/**
 * @brief Creates the layout object and set the theme
 * @param o The parent
 * @param th The theme to add to, or if NULL, the default theme
 * @param edje_file The path to edj, if NULL it's used the path given
 *                  to elementary_codegen
 */
Evas_Object *codegen_example_layout_add(Evas_Object *o, Elm_Theme *th, const char *edje_file);

/**
 * @brief The example title
 */
void codegen_example_title_set(Evas_Object *o, const char *value);
const char *codegen_example_title_get(const Evas_Object *o);

/**
 * @brief The swallow part
 */
void codegen_example_custom_set(Evas_Object *o, Evas_Object *value);
Evas_Object *codegen_example_custom_unset(Evas_Object *o);
Evas_Object *codegen_example_custom_get(const Evas_Object *o);

/**
 * @brief The box part
 */
Eina_Bool codegen_example_box_append(Evas_Object *o, Evas_Object *child);
Eina_Bool codegen_example_box_prepend(Evas_Object *o, Evas_Object *child);
Eina_Bool codegen_example_box_insert_before(Evas_Object *o, Evas_Object *child, const Evas_Object *reference);
Eina_Bool codegen_example_box_insert_at(Evas_Object *o, Evas_Object *child, unsigned int pos);
Evas_Object *codegen_example_box_remove(Evas_Object *o, Evas_Object *child);
Eina_Bool codegen_example_box_remove_all(Evas_Object *o, Eina_Bool clear);

/**
 * @brief The table part
 */
Eina_Bool codegen_example_table_pack(Evas_Object *o, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);
Evas_Object *codegen_example_table_unpack(Evas_Object *o, Evas_Object *child);
Eina_Bool codegen_example_table_clear(Evas_Object *o, Eina_Bool clear);

/**
 * @brief Executed when the button enlarges
 */
void codegen_example_swallow_grow_emit(Evas_Object *o);

/**
 * @brief Executed when the button reduces
 */
void codegen_example_swallow_shrink_emit(Evas_Object *o);

/**
 * @brief Emit the signal size,changed
 */
void codegen_example_size_changed_callback_add(Evas_Object *o, Edje_Signal_Cb func, void *data);
void codegen_example_size_changed_callback_del(Evas_Object *o, Edje_Signal_Cb func);

#endif /* _CODEGEN_EXAMPLE_GENERATED_H */
