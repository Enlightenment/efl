#ifndef _ECORE_DESKTOP_H
# define _ECORE_DESKTOP_H

#include <Ecore_Data.h>
#include <sys/types.h>

/**
 * @file Ecore_Desktop.h
 * @brief The file that provides the freedesktop.org desktop, icon, and menu
 *        functions.
 *
 * This header provides the Ecore_Desktop freedesktop.org desktop, icon, 
 * and menu handling functions, as well as ancillary functions for searching
 * freedesktop.org specific paths.  Other freedesktop.org specifications
 * make use of similar files, paths, and icons, implementors can use / extend
 * this code to suit.
 *
 * Ecore_Desktop is not for every freedesktop.org specification, just those that
 * are associated with .desktop files.
 *
 * For path searching details, see @ref Ecore_Desktop_Paths_Group.
 *
 * For desktop file details, see @ref Ecore_Desktop_Main_Group.
 *
 * For icon theme details, see @ref Ecore_Desktop_Icon_Group.
 *
 * For menu file details, see @ref Ecore_Desktop_Menu_Group.
 */

struct _Ecore_Desktop
{
   /* FIXME: Do the ECORE_MAGIC thing here.
    * While this might help with segfaults and such, I think it's a waste of 
    * space and cycles that just covers up bugs.  On the other hand, it makes 
    * for a more robust library, and it's used everywhere else in ecore.
    */
   Eina_Hash         *data, *group, *Categories, *OnlyShowIn, *NotShowIn, *MimeTypes, *Actions;
   char               *original_path;
   char               *original_lang;
   char               *eap_name;
   char               *name;
   char               *generic;
   char               *comment;
   char               *type;
   char               *categories;
   char               *exec;
   char               *exec_params;
   char               *icon_class;
   char               *icon_theme;
   char               *icon;
   char               *icon_path;
   time_t              icon_time;      /* For checking if the icon cache is valid. */
   char               *path;
   char               *URL;
   char               *file;
   char               *deletiondate;
   char               *window_class;	/* window class */
   char               *window_name;	/* window name */
   char               *window_title;	/* window title */
   char               *window_role;	/* window role */
   unsigned char       wait_exit:1;	/* wait for app to exit before execing next */
   unsigned char       startup:1;
   unsigned char       hidden:1;
   unsigned char       no_display:1;
   unsigned char       allocated:1;
   unsigned char       ondisk:1;
   unsigned char       hard_icon:1;
   /* Actually calling this st_mtime causes compile issues, must be some strange macros at work. */
   time_t              mtime;	/* For checking if the cache is valid. */
};
typedef struct _Ecore_Desktop Ecore_Desktop;

struct _Ecore_Desktop_Icon_Theme
{
   Eina_Hash         *data, *group;
   Ecore_List         *Inherits;
   Ecore_List         *Directories;
   char               *path;
   char               *name;
   char               *comment;
   char               *example;
   char               *example_path;
   char               *inherits;
   char               *directories;
   int                 hidden;
   unsigned char       hicolor:1;
   /* Actually calling this st_mtime causes compile issues, must be some strange macros at work. */
   time_t              mtime;	/* For checking if the cache is valid. */
   double              last_checked;
};
typedef struct _Ecore_Desktop_Icon_Theme Ecore_Desktop_Icon_Theme;

struct _Ecore_Desktop_Icon_Theme_Directory
{
   Eina_Hash         *icons;
   char               *path;
   char               *full_path;
   char               *type;
   int                 size, minimum, maximum, threshold;
};
typedef struct _Ecore_Desktop_Icon_Theme_Directory
   Ecore_Desktop_Icon_Theme_Directory;

enum _Ecore_Desktop_Paths_Type
{
   ECORE_DESKTOP_PATHS_CONFIG = 0,
   ECORE_DESKTOP_PATHS_MENUS = 1,
   ECORE_DESKTOP_PATHS_DIRECTORIES = 2,
   ECORE_DESKTOP_PATHS_DESKTOPS = 3,
   ECORE_DESKTOP_PATHS_ICONS = 4,
   ECORE_DESKTOP_PATHS_KDE_LEGACY = 5,
   ECORE_DESKTOP_PATHS_XSESSIONS = 6,
   ECORE_DESKTOP_PATHS_MAX = 7
};
typedef enum _Ecore_Desktop_Paths_Type Ecore_Desktop_Paths_Type;

enum _Ecore_Desktop_Tree_Element_Type
{
   ECORE_DESKTOP_TREE_ELEMENT_TYPE_NULL = 0,
   ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING = 1,
   ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE = 2,
   ECORE_DESKTOP_TREE_ELEMENT_TYPE_HASH = 3
};
typedef enum _Ecore_Desktop_Tree_Element_Type Ecore_Desktop_Tree_Element_Type;

struct _Ecore_Desktop_Tree_Element
{
   void               *element;	/* A pointer to the element. */
   Ecore_Desktop_Tree_Element_Type type;	/* The type of the element. */
};
typedef struct _Ecore_Desktop_Tree_Element Ecore_Desktop_Tree_Element;

typedef struct _Ecore_Desktop_Tree Ecore_Desktop_Tree;
struct _Ecore_Desktop_Tree
{
   Ecore_Desktop_Tree_Element *elements;	/* An array of elements. */
   int                 size;	/* The size of the array. */
   char              **buffers;	/* An array of pointers to the bits of data. */
   int                 buffers_size;	/* The size of the array. */
   Ecore_Desktop_Tree *parent;	/* Parent if this is a child. */
};

struct _Ecore_Desktop_Instrumentation
{
   double desktops_time;
   double desktops_in_cache_time;
   double desktops_not_found_time;
   double icons_time;
   double icons_in_cache_time;
   double icons_not_found_time;
   int desktops;
   int desktops_in_cache;
   int desktops_not_found;
   int icons;
   int icons_in_cache;
   int icons_not_found;
};


# ifdef __cplusplus
extern              "C"
{
# endif

   /* Function Prototypes */
   EAPI int            ecore_desktop_paths_init(void);
   EAPI void           ecore_desktop_paths_extras_clear(void);
   EAPI void           ecore_desktop_paths_prepend_user(Ecore_Desktop_Paths_Type
							type, const char *paths);
   EAPI void          
      ecore_desktop_paths_prepend_system(Ecore_Desktop_Paths_Type type,
					 const char *paths);
   EAPI void           ecore_desktop_paths_append_user(Ecore_Desktop_Paths_Type
						       type, const char *paths);
   EAPI void          
      ecore_desktop_paths_append_system(Ecore_Desktop_Paths_Type type,
					const char *paths);
   EAPI void           ecore_desktop_paths_regen(void);
   char               *ecore_desktop_paths_file_find(Ecore_List * paths,
						     const char *file, int sub,
						     int (*func) (void
								  *data,
								  const char
								  *path),
						     void *data);
   EAPI int            ecore_desktop_paths_for_each(Ecore_Desktop_Paths_Type
						    type,
						    Ecore_For_Each function,
						    void *user_data);
   char               *ecore_desktop_paths_recursive_search(const char *path,
							    const char *file,
							    int sub,
							    int (*dir_func)
							    (void *data,
							     const char *path),
							    int (*func) (void
									 *data,
									 const
									 char
									 *path),
							    void *data);
   EAPI int            ecore_desktop_paths_shutdown(void);

   Eina_Hash         *ecore_desktop_paths_to_hash(const char *paths);
   Ecore_List         *ecore_desktop_paths_to_list(const char *paths);

   EAPI int            ecore_desktop_init(void);
   EAPI int            ecore_desktop_shutdown(void);
   Eina_Hash         *ecore_desktop_ini_get(const char *file);
   Ecore_Desktop      *ecore_desktop_get(const char *file, const char *lang);
   void                ecore_desktop_save(Ecore_Desktop * desktop);
   EAPI Ecore_List    *ecore_desktop_get_command(Ecore_Desktop * desktop,
						 Ecore_List * files, int fill);
   EAPI char          *ecore_desktop_merge_command(char *exec, char *params);
   void                ecore_desktop_destroy(Ecore_Desktop * desktop);

   EAPI int            ecore_desktop_icon_init(void);
   EAPI int            ecore_desktop_icon_shutdown(void);
   EAPI char          *ecore_desktop_icon_find(const char *icon,
					       const char *icon_size,
					       const char *icon_theme);

   Eina_Hash         *ecore_desktop_icon_theme_list(void);
   Ecore_Desktop_Icon_Theme *ecore_desktop_icon_theme_get(const char *file,
							  const char *lang);
   void                ecore_desktop_icon_theme_destroy(Ecore_Desktop_Icon_Theme
							* icon_theme);

   EAPI void          
      ecore_desktop_menu_for_each(void (*func)
				  (const char *name, const char *path, const char *directory, Eina_Hash * apps));
   Ecore_Desktop_Tree *ecore_desktop_menu_get(char *file);

   Ecore_Desktop_Tree *ecore_desktop_tree_new(char *buffer);
   Ecore_Desktop_Tree *ecore_desktop_tree_add(Ecore_Desktop_Tree * tree,
					      const char *element);
   void                ecore_desktop_tree_track(Ecore_Desktop_Tree * tree,
						void *element);
   Ecore_Desktop_Tree *ecore_desktop_tree_extend(Ecore_Desktop_Tree * tree,
						 const char *element);
   Ecore_Desktop_Tree *ecore_desktop_tree_insert(Ecore_Desktop_Tree * tree,
						 int before, void *element,
						 Ecore_Desktop_Tree_Element_Type
						 type);
   Ecore_Desktop_Tree *ecore_desktop_tree_merge(Ecore_Desktop_Tree * tree,
						int before,
						Ecore_Desktop_Tree * element);
   Ecore_Desktop_Tree *ecore_desktop_tree_add_child(Ecore_Desktop_Tree * tree,
						    Ecore_Desktop_Tree *
						    element);
   Ecore_Desktop_Tree *ecore_desktop_tree_add_hash(Ecore_Desktop_Tree * tree,
						   Eina_Hash * element);
   void                ecore_desktop_tree_remove(Ecore_Desktop_Tree * tree,
						 int element);
   int                 ecore_desktop_tree_exist(Ecore_Desktop_Tree * tree,
						char *element);
   int                 ecore_desktop_tree_foreach(Ecore_Desktop_Tree * tree,
						  int level,
						  int (*func) (const void *data,
							       Ecore_Desktop_Tree
							       * tree,
							       int element,
							       int level),
						  const void *data);
   void                ecore_desktop_tree_dump(Ecore_Desktop_Tree * tree,
					       int level);
   void                ecore_desktop_tree_del(Ecore_Desktop_Tree * tree);

   Ecore_Desktop_Tree *ecore_desktop_xmlame_new(char *buffer);
   Ecore_Desktop_Tree *ecore_desktop_xmlame_get(char *file);

   char               *ecore_desktop_home_get(void);

   EAPI void           ecore_desktop_instrumentation_reset(void);
   EAPI void           ecore_desktop_instrumentation_print(void);

# ifdef __cplusplus
}
# endif

#endif
