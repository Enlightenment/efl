#ifndef _ECORE_DESKTOP_H
# define _ECORE_DESKTOP_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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


#define MAX_PATH 4096

#define E_FN_DEL(_fn, _h) if (_h) { _fn(_h); _h = NULL; }
#define E_REALLOC(p, s, n) p = (s *)realloc(p, sizeof(s) * n)
#define E_NEW(s, n) (s *)calloc(n, sizeof(s))
#define E_NEW_BIG(s, n) (s *)malloc(n * sizeof(s))
#define E_FREE(p) { if (p) {free(p); p = NULL;} }

Ecore_List         *ecore_desktop_paths_config;
Ecore_List         *ecore_desktop_paths_menus;
Ecore_List         *ecore_desktop_paths_directories;
Ecore_List         *ecore_desktop_paths_desktops;
Ecore_List         *ecore_desktop_paths_icons;
Ecore_List         *ecore_desktop_paths_kde_legacy;

struct _Ecore_Desktop
{
   Ecore_Hash         *data, *group, *Categories, *OnlyShowIn, *NotShowIn;
   char               *name;
   char               *generic;
   char               *comment;
   char               *type;
   char               *categories;
   char               *exec;
   char               *icon;
   char               *icon_path;
   char               *path;
   char               *deletiondate;
   char               *startup;
   char               *window_class;
   int                 allocated;	/* FIXME: NoDisplay, Hidden */
};
typedef struct _Ecore_Desktop Ecore_Desktop;

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
   void                ecore_desktop_paths_init(void);
   char               *ecore_desktop_paths_file_find(Ecore_List * paths,
						   char *file, int sub,
						   int (*func) (const
								void
								*data,
								char
								*path),
						   const void *data);
   char               *ecore_desktop_paths_recursive_search(char *path, char *d,
							    int (*dir_func)
							    (const void *data,
							     char *path),
							    int (*func) (const
									 void
									 *data,
									 char
									 *path),
							    const void *data);
   void                ecore_desktop_paths_shutdown(void);

   Ecore_Hash         *ecore_desktop_paths_to_hash(char *paths);
   Ecore_List         *ecore_desktop_paths_to_list(char *paths);

   void                ecore_desktop_init(void);
   void                ecore_desktop_shutdown(void);
   Ecore_Hash         *ecore_desktop_ini_get(char *file);
   Ecore_Desktop      *ecore_desktop_get(char *file);
   void                ecore_desktop_destroy(Ecore_Desktop * desktop);

   char               *ecore_desktop_icon_find(char *icon, char *icon_size,
					       char *icon_theme);

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
   void                ecore_esktop_tree_dump(Ecore_Desktop_Tree * tree,
					      int level);
   void                ecore_desktop_tree_del(Ecore_Desktop_Tree * tree);

   Ecore_Desktop_Tree *ecore_desktop_xmlame_new(char *buffer);
   Ecore_Desktop_Tree *ecore_desktop_xmlame_get(char *file);

   char               *ecore_desktop_home_get(void);

# ifdef __cplusplus
}
# endif

#endif
