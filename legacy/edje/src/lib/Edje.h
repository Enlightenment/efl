#ifndef _EDJE_H
#define _EDJE_H

#include <Evas.h>

#define EDJE_DRAG_DIR_NONE 0
#define EDJE_DRAG_DIR_X    1
#define EDJE_DRAG_DIR_Y    2
#define EDJE_DRAG_DIR_XY   3

#define EDJE_LOAD_ERROR_NONE                       0
#define EDJE_LOAD_ERROR_GENERIC                    1
#define EDJE_LOAD_ERROR_DOES_NOT_EXIST             2
#define EDJE_LOAD_ERROR_PERMISSION_DENIED          3
#define EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED 4
#define EDJE_LOAD_ERROR_CORRUPT_FILE               5
#define EDJE_LOAD_ERROR_UNKNOWN_FORMAT             6
#define EDJE_LOAD_ERROR_INCOMPATIBLE_FILE          7

#ifdef __cplusplus
extern "C" {
#endif
   
/***************************************************************************/
  
  /** Initialize the EDJE library.
   *
   * @return The new init count.
   */
   int          edje_init                       (void);
  
  /** Shutdown the EET library.
   *
   * @return The new init count.
   */
   int          edje_shutdown                   (void);
  
  /** Set the frametime
   * @param t The frametime
   *
   * Sets the frametime in seconds, by default this is 1/60.
   */
   void         edje_frametime_set              (double t);

  /** Get the frametime
   * @return The frametime
   *
   * Returns the frametime in seconds, by default this is 1/60.
   */
   double       edje_frametime_get              (void);

  /** Freeze all objects in the Edje.
   */
   void         edje_freeze                     (void);

  /** Thaw all objects in Edje
   */
   void         edje_thaw                       (void);
   
  /** Get the collection list from the edje file ?!
   * @param file The file path?
   *
   * @return The file Evas List of files
   */   
   Evas_List   *edje_file_collection_list       (const char *file);

  /** Free file collection ?!
   * @param lst The Evas List of files
   *
   * Frees the file collection.
   */
   void         edje_file_collection_list_free  (Evas_List *lst);

  /** Get edje file data
   * @param file The file
   * @param key The data key
   * @return The file data string
   */
   char        *edje_file_data_get              (const char *file, const char *key);



  /** Set Edje color class
   * @param color_class
   * @param r Object Red value
   * @param g Object Green value
   * @param b Object Blue value
   * @param a Object Alpha value
   * @param r2 Outline Red value
   * @param g2 Outline Green value
   * @param b2 Outline Blue value
   * @param a2 Outline Alpha value
   * @param r3 Shadow Red value
   * @param g3 Shadow Green value
   * @param b3 Shadow Blue value
   * @param a3 Shadow Alpha value
   *
   * Sets the color class for the Edje.
   */   
   void         edje_color_class_set(const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

  /** Set the Edje text class
   * @param text_class The text class name ?!
   * @param font The font name
   * @param size The font size
   *
   * This sets the Edje text class ?!
   */
   void         edje_text_class_set(const char *text_class, const char *font, Evas_Font_Size size);
   
  /** Set the object minimum size
   * @param obj A valid Evas_Object handle
   * @param minw The minimum width
   * @param minh The minimum height
   *
   * This sets the minimum size restriction for the object.
   */
   void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);

  /** Set the object maximum size
   * @param obj A vaild Evas_Object handle
   * @param maxw The maximum width
   * @param maxh The maximum height
   *
   * This sets the maximum size restriction for the object.
   */
   void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);
   
  /** Contruct edje object ?! What's the correct terminology?
   * @param evas A valid Evas handle
   * @return The Evas_Object pointer.
   */
   Evas_Object *edje_object_add                 (Evas *evas);

  /** Get Edje object data
   * @param obj A valid Evas_Object handle
   * @param key The data key
   * @return The data string
   */
   const char  *edje_object_data_get            (Evas_Object *obj, const char *key);

  /** Sets the .eet location and loads the Edje. ?! Assuming eet file
   * @param obj A valid Evas_Object handle
   * @param file The path to the .eet file
   * @param part The group name in the eet
   * @return 0 on Error\n
   * 1 on Success
   *
   * This loads the .eet file and sets up the Edje.
   */
  int          edje_object_file_set            (Evas_Object *obj, const char *file, const char *part);

  /** Get the .eet location and group for the Evas Object. ?! Assuming eet file
   * @param obj A valid Evas_Object handle
   * @param file The .eet file location pointer
   * @param part The eet part pointer
   *
   * @ return 0 on Error\n
   * 1 on Success
   *
   * This gets the .eet file location and group for the given Evas_Object.
   */
  void         edje_object_file_get            (Evas_Object *obj, const char **file, const char **part);

  /** Get the Edje load error
   * @param obj A valid Evas_Object handle
   *
   * @return The load error from the object's Edje.  1/0, one being no error ?!
   */
  int          edje_object_load_error_get      (Evas_Object *obj);

  /** Adds a callback for the object.
   * @param obj A valid Evas_Object handle
   * @param emission Signal to activate callback ?!
   * @param source Source of signal
   * @param func The function to be executed when the callback is signaled
   *
   * Creates a callback for the object to execute the given function.
   */
  void         edje_object_signal_callback_add (Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *obj, const char *emission, const char *source), void *data);

  /** Delete an object callback
   * @param obj A valid Evas_Object handle
   * @param emission ?!
   * @param source ?!
   *
   * Deletes an existing callback
   */
  void        *edje_object_signal_callback_del (Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *obj, const char *emission, const char *source));

  /** Send a signal to the Edje
   * @param A vaild Evas_Object handle
   * @param emission The signal
   * @param source The signal source
   *
   * This sends a signal to the edje.  These are defined in the programs section of an edc. ?! Better description of signals
   */
  void         edje_object_signal_emit         (Evas_Object *obj, const char *emission, const char *source);

  /** Set the Edje to play or pause
   * @param obj A vaild Evas_Object handle
   * @param play Play instruction (1 to play, 0 to pause)
   *
   * This sets the Edje to play or pause depending on the parameter.  This has no effect if the Edje is already in that state. ?! Better description
   */
  void         edje_object_play_set            (Evas_Object *obj, int play);

  /** Get the Edje play/pause state
   * @param obj A valid Evas_Object handle
   * @return 0 if Edje not connected, Edje delete_me, or Edje paused\n
   * 1 if Edje set to play
   */
  int          edje_object_play_get            (Evas_Object *obj);

  /** Set 
   * @param obj A valid Evas_Object handle
   * @param on ?!
   */
  void         edje_object_animation_set       (Evas_Object *obj, int on);

  /** Get the Edje object animation state
   * @param obj A valid Evas_Object handle
   * @return 0 on Error or not animated\n ?!
   * 1 if animated
   */
  int          edje_object_animation_get       (Evas_Object *obj);

  /** Freeze object
   * @param obj A valid Evas_Object handle
   * @return The frozen state\n
   * 0 on Error
   *
   * This puts all changes on hold.  Successive freezes will nest, requiring an equal number of thaws.
   */
  int          edje_object_freeze              (Evas_Object *obj);

  /** Thaw object
   * @param obj A valid Evas_Object handle
   * @return The frozen state\n
   * 0 on Error
   *
   * This allows frozen changes to occur.
   */
  int          edje_object_thaw                (Evas_Object *obj);

  /** Sets the object color class
   * @param color_class
   * @param r Object Red value
   * @param g Object Green value
   * @param b Object Blue value
   * @param a Object Alpha value
   * @param r2 Outline Red value
   * @param g2 Outline Green value
   * @param b2 Outline Blue value
   * @param a2 Outline Alpha value
   * @param r3 Shadow Red value
   * @param g3 Shadow Green value
   * @param b3 Shadow Blue value
   * @param a3 Shadow Alpha value
   *
   * Applys the color class to the object, where the first color is the object, the second is the outline, and the third is the shadow.
   */   
  void         edje_object_color_class_set     (Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);

  /** Sets Edje text class
   * @param text_class The text class name
   * @param font Font name
   * @param size Font Size
   *
   * Sets the text class for the Edje.
   */
  void         edje_object_text_class_set      (Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size);

  /** Get the minimum size for an object
   * @param obj A valid Evas_Object handle
   * @param minw Minimum width pointer
   * @param minh Minimum height pointer
   *
   * Gets the object's minimum size values from the Edje. These are set to zero if no Edje is connected to the Evas Object.
   */
  void         edje_object_size_min_get        (Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);

  /** Get the maximum size for an object
   * @param obj A valid Evas_Object handle
   * @param maxw Maximum width pointer
   * @param maxh Maximum height pointer
   *
   * Gets the object's maximum size values from the Edje.  These are set to zero if no Edje is connected to the Evas Object.
   */
  void         edje_object_size_max_get        (Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh);

  /** Calculate minimum size
   * @param obj A valid Evas_Object handle
   * @param minw Minimum width pointer
   * @param minh Minimum height pointer
   *
   * Calculates the object's minimum size ?!
   */
  void         edje_object_size_min_calc       (Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh);

  /** Check if Edje part exists
   * @param obj A valid Evas_Object handle
   * @param part The part name to check
   * @return 0 on Error\n
   * 1 if Edje part exists
   */
  int          edje_object_part_exists         (Evas_Object *obj, const char *part);

  /** Get Edje part geometry
   * @param obj A valid Evas_Object handle
   * @param part The Edje part
   * @param x The x coordinate pointer
   * @param y The y coordinate pointer
   * @param w The width pointer
   * @param h The height pointer
   *
   * Gets the Edje part geometry
   */
  void         edje_object_part_geometry_get   (Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

  /** Sets the callback to be called when text for the specified part is changed
   * @param obj A valid Evas Object handle
   * @param part The part name
   * @param func The function callback to call when text has been changed
   * @param data The data to pass to the callback's data parameter
   */
  void         edje_object_text_change_cb_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, const char *part), void *data);
       
  /** Sets the text for an object part
   * @param obj A valid Evas Object handle
   * @param part The part name
   * @param text The text string
   */
   void         edje_object_part_text_set       (Evas_Object *obj, const char *part, const char *text);

  /** Returns the text of the object part
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @return The text string
   */
  const char  *edje_object_part_text_get       (Evas_Object *obj, const char *part);

  /** Swallows an object into the edje
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param obj_swallow The object to swallow
   *
   * Describe swallowing ?!
   */
  void         edje_object_part_swallow        (Evas_Object *obj, const char *part, Evas_Object *obj_swallow);

  /** Unswallow an object
   * @param obj A valid Evas_Object handle
   * @param obj_swallow The swallowed object
   *
   * Describe unswallowing ?!
   */
  void         edje_object_part_unswallow      (Evas_Object *obj, Evas_Object *obj_swallow);

  /** Get the swallowed part ?!
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @return The swallowed object
   */
  Evas_Object *edje_object_part_swallow_get    (Evas_Object *obj, const char *part);

  /** Returns the state of the edje part
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param val_ret 
   *
   * @return The part state:\n
   * "default" for the default state\n
   * "" for other states
   */
  /* FIXME: Correctly return other states */
  const char  *edje_object_part_state_get      (Evas_Object *obj, const char *part, double *val_ret);

  /** Determine if part is dragable ?!
   * @param obj A valid Evas_Object handle
   * @param part The part name
   *
   * @return 1 if dragable
   * 0 if Error/not ?!
   */
  int          edje_object_part_drag_dir_get   (Evas_Object *obj, const char *part);

  /** Set drag value
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param dx Change in x ?!
   * @param dy Change in y ?!
   */
  void         edje_object_part_drag_value_set (Evas_Object *obj, const char *part, double dx, double dy);

  /** Get the part drag value
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param dx The dx pointer
   * @param dy The dy pointer
   *
   * Gets the drag values for the object part ?!
   */
  void         edje_object_part_drag_value_get (Evas_Object *obj, const char *part, double *dx, double *dy);

  /** Set the drag size
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param dw The drag width
   * @param dh The drag height
   *
   * Does?!
   */
  void         edje_object_part_drag_size_set  (Evas_Object *obj, const char *part, double dw, double dh);

  /** Get the drag size
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param dw The drag width pointer
   * @param dh The drag height pointer
   *
   * Gets the drag size for the Edje object.
   */
  void         edje_object_part_drag_size_get  (Evas_Object *obj, const char *part, double *dw, double *dh);

  /** Sets the drag step
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param dx The x step
   * @param dy The y step
   *
   * Sets the x and y step.?!
   */
  void         edje_object_part_drag_step_set  (Evas_Object *obj, const char *part, double dx, double dy);

  /** Gets the drag step
   * @param obj A valid Evas_Object handle
   * @param part The part
   * @param dx The x step pointer
   * @param dy The y step pointer
   *
   * Gets the x and y step for the Edje object ?!
   */
  void         edje_object_part_drag_step_get  (Evas_Object *obj, const char *part, double *dx, double *dy);

  /** Sets the drag page
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param dx ?!
   * @param df ?!
   *
   * Sets the drag page dx dy
   */
  void         edje_object_part_drag_page_set  (Evas_Object *obj, const char *part, double dx, double dy);

  /** Gets the drag page
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param dx The x drag pointer
   * @param dy The y drag pointer
   *
   * Gets the x and y drag page settings.
   */
  void         edje_object_part_drag_page_get  (Evas_Object *obj, const char *part, double *dx, double *dy);

  /** Step now ?!
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param dx The x step
   * @param dy The y step
   *
   * Steps x,y ?!
   */
  void         edje_object_part_drag_step      (Evas_Object *obj, const char *part, double dx, double dy);

  /** Page now
   * @param obj A valid Evas_Object handle
   * @param part The part name
   * @param dx The x page
   * @param dy The y page
   *
   * Pages x,y ?!
   */
  void         edje_object_part_drag_page      (Evas_Object *obj, const char *part, double dx, double dy);

  /** Returns the variable ID
   * @param obj A valid Evas_Object handle
   * @param name The variable name
   * 
   * @return Variable ID\n
   * 0 on Error.
   */
  int          edje_object_variable_id_get     (Evas_Object *obj, char *name);

  /** Returns the variable int
   * @param obj A valid Evas_Object handle
   * @param id The variable id
   *
   * @return Variable int\n
   * 0 on Error
   */
  int          edje_object_variable_int_get    (Evas_Object *obj, int id);

  /** Sets the variable int
   * @param obj A valid Evas_Object handle
   * @param id The variable id
   * @param val The int value to set
   */
  void         edje_object_variable_int_set    (Evas_Object *obj, int id, int val);

  /** Returns the variable float
   * @param obj A valid Evas_Object handle
   * @param id The variable id
   *
   * @return Variable float\n
   * 0 on Error
   */
  double       edje_object_variable_float_get  (Evas_Object *obj, int id);

  /** Sets the variable float
   * @param obj A valid Evas_Object handle
   * @param id The variable id
   * @param val The float value to set
   */
  void         edje_object_variable_float_set  (Evas_Object *obj, int id, double val);

  /** Returns the variable string
   * @param obj A valid Evas_Object handle
   * @param id The variable id
   *
   * @return Variable String\n
   * 0 on Error
   */
  char        *edje_object_variable_str_get    (Evas_Object *obj, int id);

  /** Sets the variable string
   * @param obj A valid Evas_Object handle
   * @param id The variable id
   * @param str The string value to set
   */   
  void         edje_object_variable_str_set    (Evas_Object *obj, int id, char *str);
       
#ifdef __cplusplus
}
#endif

#endif
