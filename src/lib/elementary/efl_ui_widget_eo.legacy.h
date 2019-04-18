#ifndef _EFL_UI_WIDGET_EO_LEGACY_H_
#define _EFL_UI_WIDGET_EO_LEGACY_H_

#ifndef _EFL_UI_WIDGET_EO_CLASS_TYPE
#define _EFL_UI_WIDGET_EO_CLASS_TYPE

typedef Eo Efl_Ui_Widget;

#endif

#ifndef _EFL_UI_WIDGET_EO_TYPES
#define _EFL_UI_WIDGET_EO_TYPES

/** Function pointer for on show region hook
 *
 * @ingroup Efl_Ui
 */
typedef void (*Efl_Ui_Scrollable_On_Show_Region)(void *data, Efl_Canvas_Object *obj, Eina_Rect region);

/** All relevant fields needed for the current state of focus registeration
 *
 * @ingroup Efl_Ui
 */
typedef struct _Efl_Ui_Widget_Focus_State
{
  Efl_Ui_Focus_Manager *manager; /**< The manager where the widget is registered
                                  * in */
  Efl_Ui_Focus_Object *parent; /**< The parent the widget is using as logical
                                * parent */
  Eina_Bool logical; /**< @c true if this is registered as logical currently */
} Efl_Ui_Widget_Focus_State;


#endif

/**
 * @brief This is the internal canvas object managed by a widget.
 *
 * This property is protected as it is meant for widget implementations only,
 * to set and access the internal canvas object. Do use this function unless
 * you're implementing a widget.
 *
 * Sets the new resize object for this widget.
 *
 * @param[in] obj The object.
 * @param[in] sobj A canvas object (often a @ref Efl_Canvas_Layout object).
 *
 * @ingroup Elm_Widget_Group
 */
EAPI void elm_widget_resize_object_set(Efl_Ui_Widget *obj, Efl_Canvas_Object *sobj);

/**
 * @brief Whether the widget is enabled (accepts and reacts to user inputs).
 *
 * The property works counted, this means, whenever n-caller set the value to
 * @c true, n-caller have to set it to @c false in order to get it out of the
 * disabled state again.
 *
 * Each widget may handle the disabled state differently, but overall disabled
 * widgets shall not respond to any input events. This is @c false by default,
 * meaning the widget is enabled.
 *
 * Enables or disables this widget.
 *
 * Disabling a widget will disable all its children recursively, but only this
 * widget will be marked as disabled internally.
 *
 * @param[in] obj The object.
 * @param[in] disabled @c true if the widget is disabled.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI void elm_widget_disabled_set(Efl_Ui_Widget *obj, Eina_Bool disabled);

/**
 * @brief Whether the widget is enabled (accepts and reacts to user inputs).
 *
 * The property works counted, this means, whenever n-caller set the value to
 * @c true, n-caller have to set it to @c false in order to get it out of the
 * disabled state again.
 *
 * Each widget may handle the disabled state differently, but overall disabled
 * widgets shall not respond to any input events. This is @c false by default,
 * meaning the widget is enabled.
 *
 * Returns whether the widget is disabled.
 *
 * This will return @c true if any widget in the parent hierarchy is disabled.
 * Re-enabling that parent may in turn change the disabled state of this
 * widget.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the widget is disabled.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI Eina_Bool elm_widget_disabled_get(const Efl_Ui_Widget *obj);

/**
 * @brief The widget style to use.
 *
 * Styles define different look and feel for widgets, and may provide different
 * parts for layout-based widgets. Styles vary from widget to widget and may be
 * defined by other themes by means of extensions and overlays.
 *
 * The style can only be set before @ref Efl.Object.finalize, which means at
 * construction time of the object (inside @c efl_add in C).
 *
 * Can only be called during construction, before finalize.
 *
 * @param[in] obj The object.
 * @param[in] style Name of the style to use. Refer to each widget's
 * documentation for the available style names, or to the themes in use.
 *
 * @return Whether the style was successfully applied or not, see the values of
 * Efl.Ui.Theme.Apply_Error for more information.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI Eina_Error elm_widget_style_set(Efl_Ui_Widget *obj, const char *style);

/**
 * @brief The widget style to use.
 *
 * Styles define different look and feel for widgets, and may provide different
 * parts for layout-based widgets. Styles vary from widget to widget and may be
 * defined by other themes by means of extensions and overlays.
 *
 * The style can only be set before @ref Efl.Object.finalize, which means at
 * construction time of the object (inside @c efl_add in C).
 *
 * Returns the current style of a widget.
 *
 * @param[in] obj The object.
 *
 * @return Name of the style to use. Refer to each widget's documentation for
 * the available style names, or to the themes in use.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI const char *elm_widget_style_get(const Efl_Ui_Widget *obj);


/**
 * @brief The ability for a widget to be focused.
 *
 * Unfocusable objects do nothing when programmatically focused. The nearest
 * focusable parent object the one really getting focus. Also, when they
 * receive mouse input, they will get the event, but not take away the focus
 * from where it was previously.
 *
 * @note Objects which are meant to be interacted with by input events are
 * created able to be focused, by default. All the others are not.
 *
 * This property's default value depends on the widget (eg. a box is not
 * focusable, but a button is).
 *
 * @param[in] obj The object.
 * @param[in] can_focus Whether the object is focusable.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI void elm_widget_can_focus_set(Efl_Ui_Widget *obj, Eina_Bool can_focus);

/**
 * @brief The ability for a widget to be focused.
 *
 * Unfocusable objects do nothing when programmatically focused. The nearest
 * focusable parent object the one really getting focus. Also, when they
 * receive mouse input, they will get the event, but not take away the focus
 * from where it was previously.
 *
 * @note Objects which are meant to be interacted with by input events are
 * created able to be focused, by default. All the others are not.
 *
 * This property's default value depends on the widget (eg. a box is not
 * focusable, but a button is).
 *
 * @param[in] obj The object.
 *
 * @return Whether the object is focusable.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI Eina_Bool elm_widget_can_focus_get(const Efl_Ui_Widget *obj);

/**
 * @brief The internal parent of this widget.
 *
 * @ref Efl_Ui_Widget objects have a parent hierarchy that may differ slightly
 * from their @ref Efl_Object or @ref Efl_Canvas_Object hierarchy. This is
 * meant for internal handling.
 *
 * @param[in] obj The object.
 * @param[in] parent Widget parent object
 *
 * @ingroup Elm_Widget_Group
 */
EAPI void elm_widget_parent_set(Efl_Ui_Widget *obj, Efl_Ui_Widget *parent);

/**
 * @brief The internal parent of this widget.
 *
 * @ref Efl_Ui_Widget objects have a parent hierarchy that may differ slightly
 * from their @ref Efl_Object or @ref Efl_Canvas_Object hierarchy. This is
 * meant for internal handling.
 *
 * @param[in] obj The object.
 *
 * @return Widget parent object
 *
 * @ingroup Elm_Widget_Group
 */
EAPI Efl_Ui_Widget *elm_widget_parent_get(const Efl_Ui_Widget *obj);

/**
 * @brief Virtual function handling sub objects being added.
 *
 * Sub objects can be any canvas object, not necessarily widgets.
 *
 * See also @ref elm_widget_parent_get.
 *
 * @param[in] obj The object.
 * @param[in] sub_obj Sub object to be added. Not necessarily a widget itself.
 *
 * @return Indicates if the operation succeeded.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI Eina_Bool elm_widget_sub_object_add(Efl_Ui_Widget *obj, Efl_Canvas_Object *sub_obj);

/**
 * @brief Virtual function handling sub objects being removed.
 *
 * Sub objects can be any canvas object, not necessarily widgets.
 *
 * See also @ref elm_widget_parent_get.
 *
 * @param[in] obj The object.
 * @param[in] sub_obj Sub object to be removed. Should be a child of this
 * widget.
 *
 * @return Indicates if the operation succeeded.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI Eina_Bool elm_widget_sub_object_del(Efl_Ui_Widget *obj, Efl_Canvas_Object *sub_obj);

/**
 * @brief Virtual function called when the widget needs to re-apply its theme.
 *
 * This may be called when the object is first created, or whenever the widget
 * is modified in any way that may require a reload of the theme. This may
 * include but is not limited to scale, theme, or mirrored mode changes.
 *
 * @note even widgets not based on layouts may override this method to handle
 * widget updates (scale, mirrored mode, etc...).
 *
 * @param[in] obj The object.
 *
 * @return Indicates success, and if the current theme or default theme was
 * used.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI Eina_Error elm_widget_theme_apply(Efl_Ui_Widget *obj);

/**
 * @brief Region of interest inside this widget, that should be given priority
 * to be visible inside a scroller.
 *
 * When this widget or one of its subwidgets is given focus, this region should
 * be shown, which means any parent scroller should attempt to display the
 * given area of this widget. For instance, an entry given focus should scroll
 * to show the text cursor if that cursor moves. In this example, this region
 * defines the relative geometry of the cursor within the widget.
 *
 * @note The region is relative to the top-left corner of the widget, i.e. X,Y
 * start from 0,0 to indicate the top-left corner of the widget. W,H must be
 * greater or equal to 1 for this region to be taken into account, otherwise it
 * is ignored.
 *
 * @param[in] obj The object.
 *
 * @return The relative region to show. If width or height is <= 0 it will be
 * ignored, and no action will be taken.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI Eina_Rect elm_widget_focus_region_get(const Efl_Ui_Widget *obj);

/**
 * @brief The rectangle region to be highlighted on focus.
 *
 * This is a rectangle region where the focus highlight should be displayed.
 *
 * This is a read-only property.
 *
 * @param[in] obj The object.
 *
 * @return The rectangle area.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI Eina_Rect elm_widget_focus_highlight_geometry_get(const Efl_Ui_Widget *obj);

/**
 * @brief Register focus with the given configuration.
 *
 * The implementation can feel free to change the logical flag as it wants, but
 * other than that it should strictly keep the configuration.
 *
 * The implementation in elm.widget updates the current state into what is
 * passed as configured state, respecting manager changes, registeration and
 * unregistration based on if it should be registered or unregistered.
 *
 * A manager field that is @c null means that the widget should not or was not
 * registered.
 *
 * @param[in] obj The object.
 * @param[in] current_state The focus manager to register with.
 * @param[in,out] configured_state The evaluated Focus state that should be
 * used.
 * @param[in] redirect A redirect that will be set by the elm.widget
 * implementation.
 *
 * @return Returns whether the widget is registered or not.
 *
 * @ingroup Elm_Widget_Group
 */
EAPI Eina_Bool elm_widget_focus_state_apply(Efl_Ui_Widget *obj, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect);

#endif
