/**
 * @defgroup Scroller Scroller
 * @ingroup Elementary
 *
 * @image html scroller_inheritance_tree.png
 * @image latex scroller_inheritance_tree.eps
 *
 * A scroller holds (and clips) a single object and "scrolls it
 * around". This means that it allows the user to use a scroll bar (or
 * a finger) to drag the viewable region around, moving through a much
 * larger object that is contained in the scroller. The scroller will
 * always have a small minimum size by default as it won't be limited
 * by the contents of the scroller.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for scroller objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @li @c "edge,left" - the left edge of the content has been reached
 * @li @c "edge,right" - the right edge of the content has been reached
 * @li @c "edge,top" - the top edge of the content has been reached
 * @li @c "edge,bottom" - the bottom edge of the content has been reached
 * @li @c "scroll" - the content has been scrolled (moved)
 * @li @c "scroll,anim,start" - scrolling animation has started
 * @li @c "scroll,anim,stop" - scrolling animation has stopped
 * @li @c "scroll,drag,start" - dragging the contents around has started
 * @li @c "scroll,drag,stop" - dragging the contents around has stopped
 *
 * This widget implements the @ref elm-scrollable-interface interface.
 * Its (non-deprecated) API functions, except for elm_scroller_add(),
 * which gives basic scroller objects, are meant to be a basis for all
 * other scrollable widgets (i.e. widgets implementing @ref
 * elm-scrollable-interface). So, they will work both on pristine
 * scroller widgets and on other "specialized" scrollable widgets.
 *
 * @note The @c "scroll,anim,*" and @c "scroll,drag,*" signals are
 * only emitted by user intervention.
 *
 * @note When Elementary is under its default profile and theme (meant
 * for touch interfaces), scroll bars will @b not be draggable --
 * their function is merely to indicate how much has been scrolled.
 *
 * @note When Elementary is under its desktop/standard profile and
 * theme, the thumb scroll (a.k.a. finger scroll) won't work.
 *
 * Default content parts of the scroller widget that you can use are:
 * @li @c "default" - Content of the scroller
 *
 * In @ref tutorial_scroller you'll find an example on how to use most
 * of this API.
 * @{
 */

/**
 * @brief Type that controls when scrollbars should appear.
 *
 * @see elm_scroller_policy_set()
 */
typedef enum
{
   ELM_SCROLLER_POLICY_AUTO = 0, /**< Show scrollbars as needed */
   ELM_SCROLLER_POLICY_ON, /**< Always show scrollbars */
   ELM_SCROLLER_POLICY_OFF, /**< Never show scrollbars */
   ELM_SCROLLER_POLICY_LAST
} Elm_Scroller_Policy;

/**
 * @brief Add a new scroller to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Scroller
 */
EAPI Evas_Object                 *elm_scroller_add(Evas_Object *parent);

/**
 * @brief Set custom theme elements for the scroller
 *
 * @param obj The scroller object
 * @param widget The widget name to use (default is "scroller")
 * @param base The base name to use (default is "base")
 *
 * @deprecated Use elm_layout_theme_set() instead.
 *
 * @ingroup Scroller
 */
EINA_DEPRECATED EAPI void         elm_scroller_custom_widget_base_theme_set(Evas_Object *obj, const char *widget, const char *base);

/**
 * @brief Make the scroller minimum size limited to the minimum size of the content
 *
 * @param obj The scroller object
 * @param w Enable limiting minimum size horizontally
 * @param h Enable limiting minimum size vertically
 *
 * By default the scroller will be as small as its design allows,
 * irrespective of its content. This will make the scroller minimum size the
 * right size horizontally and/or vertically to perfectly fit its content in
 * that direction.
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_content_min_limit(Evas_Object *obj, Eina_Bool w, Eina_Bool h);

/**
 * @brief Show a specific virtual region within the scroller content object
 *
 * @param obj The scroller object
 * @param x X coordinate of the region
 * @param y Y coordinate of the region
 * @param w Width of the region
 * @param h Height of the region
 *
 * This will ensure all (or part if it does not fit) of the designated
 * region in the virtual content object (0, 0 starting at the top-left of the
 * virtual content object) is shown within the scroller.
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_region_show(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * @brief Set the scrollbar visibility policy
 *
 * @param obj The scroller object
 * @param policy_h Horizontal scrollbar policy
 * @param policy_v Vertical scrollbar policy
 *
 * This sets the scrollbar visibility policy for the given scroller.
 * ELM_SCROLLER_POLICY_AUTO means the scrollbar is made visible if it is
 * needed, and otherwise kept hidden. ELM_SCROLLER_POLICY_ON turns it on all
 * the time, and ELM_SCROLLER_POLICY_OFF always keeps it off. This applies
 * respectively for the horizontal and vertical scrollbars.
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v);

/**
 * @brief Gets scrollbar visibility policy
 *
 * @param obj The scroller object
 * @param policy_h Horizontal scrollbar policy
 * @param policy_v Vertical scrollbar policy
 *
 * @see elm_scroller_policy_set()
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v);

/**
 * @brief Get the currently visible content region
 *
 * @param obj The scroller object
 * @param x X coordinate of the region
 * @param y Y coordinate of the region
 * @param w Width of the region
 * @param h Height of the region
 *
 * This gets the current region in the content object that is visible through
 * the scroller. The region co-ordinates are returned in the @p x, @p y, @p
 * w, @p h values pointed to.
 *
 * @note All coordinates are relative to the content.
 *
 * @see elm_scroller_region_show()
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_region_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Get the size of the content object
 *
 * @param obj The scroller object
 * @param w Width of the content object.
 * @param h Height of the content object.
 *
 * This gets the size of the content object of the scroller.
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_child_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

/**
 * @brief Set bouncing behavior
 *
 * @param obj The scroller object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * When scrolling, the scroller may "bounce" when reaching an edge of the
 * content object. This is a visual way to indicate the end has been reached.
 * This is enabled by default for both axis. This API will set if it is enabled
 * for the given axis with the boolean parameters for each axis.
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);

/**
 * @brief Get the bounce behaviour
 *
 * @param obj The Scroller object
 * @param h_bounce Will the scroller bounce horizontally or not
 * @param v_bounce Will the scroller bounce vertically or not
 *
 * @see elm_scroller_bounce_set()
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);

/**
 * @brief Set scroll page size relative to viewport size.
 *
 * @param obj The scroller object
 * @param h_pagerel The horizontal page relative size
 * @param v_pagerel The vertical page relative size
 *
 * The scroller is capable of limiting scrolling by the user to "pages". That
 * is to jump by and only show a "whole page" at a time as if the continuous
 * area of the scroller content is split into page sized pieces. This sets
 * the size of a page relative to the viewport of the scroller. 1.0 is "1
 * viewport" is size (horizontally or vertically). 0.0 turns it off in that
 * axis. This is mutually exclusive with page size
 * (see elm_scroller_page_size_set()  for more information). Likewise 0.5
 * is "half a viewport". Sane usable values are normally between 0.0 and 1.0
 * including 1.0. If you only want 1 axis to be page "limited", use 0.0 for
 * the other axis.
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_page_relative_set(Evas_Object *obj, double h_pagerel, double v_pagerel);

/**
 * Get a given scroller widget's scrolling page size, relative to
 * its viewport size.
 *
 * @param obj The scroller object
 * @param h_pagerel Pointer to a variable where to store the
 * horizontal page (relative) size
 * @param v_pagerel Pointer to a variable where to store the
 * vertical page (relative) size
 *
 * @see elm_scroller_page_relative_set() for more details
 *
 * @since 1.7
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_page_relative_get(const Evas_Object *obj, double *h_pagerel, double *v_pagerel);

/**
 * @brief Set scroll page size.
 *
 * @param obj The scroller object
 * @param h_pagesize The horizontal page size
 * @param v_pagesize The vertical page size
 *
 * This sets the page size to an absolute fixed value, with 0 turning it off
 * for that axis.
 *
 * @see elm_scroller_page_relative_set()
 * @see elm_scroller_page_size_get()
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_page_size_set(Evas_Object *obj, Evas_Coord h_pagesize, Evas_Coord v_pagesize);

/**
 * @brief Retrieve a scroller widget's current page size.
 *
 * @param obj The scroller object
 * @param h_pagesize Where to store its horizontal page size
 * @param v_pagesize Where to store its vertical page size
 *
 * @see elm_scroller_page_size_set() for more details
 * @see elm_scroller_page_relative_set()
 *
 * @since 1.7
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_page_size_get(const Evas_Object *obj, Evas_Coord *h_pagesize, Evas_Coord *v_pagesize);

/**
 * @brief Get scroll current page number.
 *
 * @param obj The scroller object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * The page number starts from 0. 0 is the first page.
 * Current page means the page which meets the top-left of the viewport.
 * If there are two or more pages in the viewport, it returns the number of the page
 * which meets the top-left of the viewport.
 *
 * @see elm_scroller_last_page_get()
 * @see elm_scroller_page_show()
 * @see elm_scroller_page_bring_in()
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_current_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);

/**
 * @brief Get scroll last page number.
 *
 * @param obj The scroller object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * The page number starts from 0. 0 is the first page.
 * This returns the last page number among the pages.
 *
 * @see elm_scroller_current_page_get()
 * @see elm_scroller_page_show()
 * @see elm_scroller_page_bring_in()
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_last_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);

/**
 * Show a specific virtual region within the scroller content object by page number.
 *
 * @param obj The scroller object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * 0, 0 of the indicated page is located at the top-left of the viewport.
 * This will jump to the page directly without animation.
 *
 * Example of usage:
 *
 * @code
 * sc = elm_scroller_add(win);
 * elm_object_content_set(sc, content);
 * elm_scroller_page_relative_set(sc, 1, 0);
 * elm_scroller_current_page_get(sc, &h_page, &v_page);
 * elm_scroller_page_show(sc, h_page + 1, v_page);
 * @endcode
 *
 * @see elm_scroller_page_bring_in()
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_page_show(Evas_Object *obj, int h_pagenumber, int v_pagenumber);

/**
 * Show a specific virtual region within the scroller content object by page number.
 *
 * @param obj The scroller object
 * @param h_pagenumber The horizontal page number
 * @param v_pagenumber The vertical page number
 *
 * 0, 0 of the indicated page is located at the top-left of the viewport.
 * This will slide to the page with animation.
 *
 * Example of usage:
 *
 * @code
 * sc = elm_scroller_add(win);
 * elm_object_content_set(sc, content);
 * elm_scroller_page_relative_set(sc, 1, 0);
 * elm_scroller_last_page_get(sc, &h_page, &v_page);
 * elm_scroller_page_bring_in(sc, h_page, v_page);
 * @endcode
 *
 * @see elm_scroller_page_show()
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_page_bring_in(Evas_Object *obj, int h_pagenumber, int v_pagenumber);

/**
 * @brief Show a specific virtual region within the scroller content object.
 *
 * @param obj The scroller object
 * @param x X coordinate of the region
 * @param y Y coordinate of the region
 * @param w Width of the region
 * @param h Height of the region
 *
 * This will ensure all (or part if it does not fit) of the designated
 * region in the virtual content object (0, 0 starting at the top-left of the
 * virtual content object) is shown within the scroller. Unlike
 * elm_scroller_region_show(), this allow the scroller to "smoothly slide"
 * to this location (if configuration in general calls for transitions). It
 * may not jump immediately to the new location and make take a while and
 * show other content along the way.
 *
 * @see elm_scroller_region_show()
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_region_bring_in(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);

/**
 * @brief Set event propagation on a scroller
 *
 * @param obj The scroller object
 * @param propagation If propagation is enabled or not
 *
 * This enables or disables event propagation from the scroller
 * content to the scroller and its parent. By default event
 * propagation is @b enabled.
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_propagate_events_set(Evas_Object *obj, Eina_Bool propagation);

/**
 * @brief Get event propagation for a scroller
 *
 * @param obj The scroller object
 * @return The propagation state
 *
 * This gets the event propagation for a scroller.
 *
 * @see elm_scroller_propagate_events_set()
 *
 * @ingroup Scroller
 */
EAPI Eina_Bool                    elm_scroller_propagate_events_get(const Evas_Object *obj);

/**
 * @brief Set scrolling gravity on a scroller
 *
 * @param obj The scroller object
 * @param x The scrolling horizontal gravity
 * @param y The scrolling vertical gravity
 *
 * The gravity, defines how the scroller will adjust its view
 * when the size of the scroller contents increase.
 *
 * The scroller will adjust the view to glue itself as follows.
 *
 *  x=0.0, for showing the left most region of the content.
 *  x=1.0, for showing the right most region of the content.
 *  y=0.0, for showing the bottom most region of the content.
 *  y=1.0, for showing the top most region of the content.
 *
 * Default values for x and y are 0.0
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_gravity_set(Evas_Object *obj, double x, double y);

/**
 * @brief Get scrolling gravity values for a scroller
 *
 * @param obj The scroller object
 * @param x The scrolling horizontal gravity
 * @param y The scrolling vertical gravity
 *
 * This gets gravity values for a scroller.
 *
 * @see elm_scroller_gravity_set()
 *
 * @ingroup Scroller
 */
EAPI void                         elm_scroller_gravity_get(const Evas_Object *obj, double *x, double *y);

/**
 * @}
 */
