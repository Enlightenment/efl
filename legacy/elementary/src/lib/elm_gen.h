typedef struct Elm_Gen_Item             Elm_Gen_Item;

/**
 * Text fetching class function for Elm_Gen_Item_Class.
 * @param data The data passed in the item creation function
 * @param obj The base widget object
 * @param part The part name of the swallow
 * @return The allocated (NOT stringshared) string to set as the text
 */
typedef char                         *(*Elm_Gen_Item_Text_Get_Cb)(void *data, Evas_Object *obj, const char *part); /**< Label fetching class function for gen item classes. */

/**
 * Content (swallowed object) fetching class function for Elm_Gen_Item_Class.
 * @param data The data passed in the item creation function
 * @param obj The base widget object
 * @param part The part name of the swallow
 * @return The content object to swallow
 */
typedef Evas_Object                  *(*Elm_Gen_Item_Content_Get_Cb)(void *data, Evas_Object *obj, const char *part); /**< Content(swallowed object) fetching class function for gen item classes. */

/**
 * State fetching class function for Elm_Gen_Item_Class.
 * @param data The data passed in the item creation function
 * @param obj The base widget object
 * @param part The part name of the swallow
 * @return The boolean state of this element (resulting in the edje object being emitted a signal of "elm,state,partname,active" or "elm.state,partname,passve"
 */
typedef Eina_Bool                     (*Elm_Gen_Item_State_Get_Cb)(void *data, Evas_Object *obj, const char *part); /**< State fetching class function for gen item classes. */

/**
 * Deletion class function for Elm_Gen_Item_Class.
 * @param data The data passed in the item creation function
 * @param obj The base widget object
 */
typedef void                          (*Elm_Gen_Item_Del_Cb)(void *data, Evas_Object *obj); /**< Deletion class function for gen item classes. */

/**
 * Filter class function for Elm_Gen_Item_Class.
 * @param data The data passed in the item creation function
 * @param obj The base widget object
 * @param key The key needed for item filter to be decided on
 * @return The boolean state of filter for this element
 */
typedef Eina_Bool                     (*Elm_Gen_Item_Filter_Get_Cb)(void *data, Evas_Object *obj, void *key); /**< Filter seeking class function for gen item classes. */

#define ELM_GEN_ITEM_CLASS_VERSION 2
#define ELM_GEN_ITEM_CLASS_HEADER ELM_GEN_ITEM_CLASS_VERSION, 0, 0

typedef enum
{
   ELM_GLOB_MATCH_NO_ESCAPE = (1 << 0), /**< Treat backslash as an ordinary character instead of escape */
   ELM_GLOB_MATCH_PATH = (1 << 1), /**< Match a slash in string only with a slash in pattern and not by an asterisk (*) or a question mark (?) metacharacter, nor by a bracket expression ([]) containing a slash. */
   ELM_GLOB_MATCH_PERIOD = (1 << 2), /**< Leading  period in string has to be matched exactly by a period in pattern. A period is considered to be leading if it is the first character in string, or if both ELM_GLOB_MATCH_PATH is set and the period immediately follows a slash. */
   ELM_GLOB_MATCH_NOCASE = (1 << 3) /**< The pattern is matched case-insensitively. */
} Elm_Glob_Match_Flags; /**< Glob matching bitfiled flags. @since 1.11 */
