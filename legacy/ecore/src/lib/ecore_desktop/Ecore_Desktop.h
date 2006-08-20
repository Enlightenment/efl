#ifndef _ECORE_DESKTOP_H
# define _ECORE_DESKTOP_H

#include <Ecore_Data.h>

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

extern Ecore_List         *ecore_desktop_paths_config;
extern Ecore_List         *ecore_desktop_paths_menus;
extern Ecore_List         *ecore_desktop_paths_directories;
extern Ecore_List         *ecore_desktop_paths_desktops;
extern Ecore_List         *ecore_desktop_paths_icons;
extern Ecore_List         *ecore_desktop_paths_kde_legacy;

struct _Ecore_Desktop
{
   Ecore_Hash         *data, *group, *Categories, *OnlyShowIn, *NotShowIn;
   char               *eap_name;
   char               *name;
   char               *generic;
   char               *comment;
   char               *type;
   char               *categories;
   char               *exec;
   char               *icon;
   char               *icon_class;
   char               *icon_path;
   char               *path;
   char               *deletiondate;
   char               *startup;
   char               *window_class;
   int                 allocated;	/* FIXME: NoDisplay, Hidden */
};
typedef struct _Ecore_Desktop Ecore_Desktop;

struct _Ecore_Desktop_Icon_Theme
{
   Ecore_Hash         *data, *group;
   Ecore_List         *Directories;
   char               *path;
   char               *name;
   char               *comment;
   char               *example;
   char               *example_path;
   char               *inherits;
   char               *directories;
   int                 hidden;
};
typedef struct _Ecore_Desktop_Icon_Theme Ecore_Desktop_Icon_Theme;


enum _Ecore_Desktop_Tree_Element_Type
{
   ECORE_DESKTOP_TREE_ELEMENT_TYPE_NULL = 0,
   ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING = 1,
   ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE = 2,
   ECORE_DESKTOP_TREE_ELEMENT_TYPE_HASH = 3,
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

# ifdef __cplusplus
extern              "C"
{
# endif

   /* Function Prototypes */
   EAPI int            ecore_desktop_paths_init(void);
   char               *ecore_desktop_paths_file_find(Ecore_List * paths,
						   const char *file, int sub,
						   int (*func) (void
								*data,
								const char
								*path),
						   void *data);
   char               *ecore_desktop_paths_recursive_search(const char *path, const char *file, int sub,
							    int (*dir_func)
							    (void *data,
							     const char *path),
							    int (*func) (void
									 *data,
									 const char
									 *path),
							    void *data);
   EAPI int            ecore_desktop_paths_shutdown(void);

   Ecore_Hash         *ecore_desktop_paths_to_hash(const char *paths);
   Ecore_List         *ecore_desktop_paths_to_list(const char *paths);

   EAPI int            ecore_desktop_init(void);
   EAPI int            ecore_desktop_shutdown(void);
   Ecore_Hash         *ecore_desktop_ini_get(const char *file);
   Ecore_Desktop      *ecore_desktop_get(const char *file, const char *lang);
   void                ecore_desktop_destroy(Ecore_Desktop * desktop);


   EAPI int            ecore_desktop_icon_init(void);
   EAPI int            ecore_desktop_icon_shutdown(void);
   const char         *ecore_desktop_icon_find(const char *icon, 
   					       const char *icon_size,
					       const char *icon_theme);


   Ecore_Hash         *ecore_desktop_icon_theme_list(void);
   Ecore_Desktop_Icon_Theme      *ecore_desktop_icon_theme_get(const char *file, const char *lang);
   void                ecore_desktop_icon_theme_destroy(Ecore_Desktop_Icon_Theme *icon_theme);


   Ecore_Desktop_Tree *ecore_desktop_menu_get(char *file);


   Ecore_Desktop_Tree *ecore_desktop_tree_new(char *buffer);
   Ecore_Desktop_Tree *ecore_desktop_tree_add(Ecore_Desktop_Tree * tree,
					      char *element);
   void                ecore_desktop_tree_track(Ecore_Desktop_Tree * tree,
						void *element);
   Ecore_Desktop_Tree *ecore_desktop_tree_extend(Ecore_Desktop_Tree * tree,
						 char *element);
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
						   Ecore_Hash * element);
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

# ifdef __cplusplus
}
# endif

#endif
