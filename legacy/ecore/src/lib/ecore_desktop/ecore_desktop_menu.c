/*
 * This conforms with the freedesktop.org Desktop Menu Specification version 0.92
 *
 * This is gonna be repetative and slow.  The idea is to first get it to correctly
 * follow the spec in a way that is easy to check.  Only then can we start to 
 * optomize into "ugly but fast".
 *
 * There are notes at the very end about all those nasty steps we need to do to 
 * follow the spec.
 */

#include <Ecore.h>

#include "Ecore_Desktop.h"
#include "ecore_desktop_private.h"
#include "ecore_private.h"

#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>

//#define DEBUG 1

struct _ecore_desktop_menu_expand_apps_data
{
   char               *path;
   Eina_Hash          *pool;
   int                 length;
};

struct _ecore_desktop_menu_unxml_data
{
   char               *file;
   char               *base;
   char               *path;
   Ecore_Desktop_Tree *stack, *merge_stack;
   int                 unallocated, level;
};

struct _ecore_desktop_menu_generate_data
{
   char               *name, *path;
   Ecore_Desktop_Tree *rules;
   Eina_Hash         *pool, *apps;
   int                 unallocated;

   Ecore_Desktop_Tree *rule;
   int                 include;
};

struct _ecore_desktop_menu_legacy_data
{
   Ecore_Desktop_Tree *merge;
   Ecore_Desktop_Tree *current;
   char               *menu;
   char               *prefix;
   char               *path;
   int                 length, menu_length, level;
};

static int          _ecore_desktop_menu_make_apps(const void *data,
						  Ecore_Desktop_Tree * tree,
						  int element, int level);
static Ecore_Desktop_Tree *_ecore_desktop_menu_get0(char *file,
						    Ecore_Desktop_Tree *
						    merge_stack, int level);
static Ecore_Desktop_Tree *_ecore_desktop_menu_create_menu(void);
static int          _ecore_desktop_menu_unxml(const void *data,
					      Ecore_Desktop_Tree * tree,
					      int element, int level);
static int          _ecore_desktop_menu_check_directory(void *data,
							const char *path);
static int          _ecore_desktop_menu_check_menu(void *data,
						   const char *path);
static int          _ecore_desktop_menu_legacy_menu_dir(void *data,
							const char *path);
static int          _ecore_desktop_menu_legacy_menu(void *data,
						    const char *path);
static void         _ecore_desktop_menu_unxml_rules(Ecore_Desktop_Tree * rules,
						    Ecore_Desktop_Tree * tree,
						    char type, char sub_type);
static void         _ecore_desktop_menu_unxml_moves(Ecore_Desktop_Tree * menu,
						    Ecore_Desktop_Tree * tree);
static void         _ecore_desktop_menu_add_dirs(Ecore_Desktop_Tree * tree,
						 Ecore_List * paths, const char *pre,
						 const char *post, char *extra,
						 int element);
static int          _ecore_desktop_menu_expand_apps(struct
						    _ecore_desktop_menu_unxml_data
						    *unxml_data, char *app_dir,
						    Eina_Hash * pool);
static int          _ecore_desktop_menu_check_app(void *data, const char *path);

static int          _ecore_desktop_menu_merge(const void *data,
					      Ecore_Desktop_Tree * tree,
					      int element, int level);
static int          _ecore_desktop_menu_expand_default_dirs(const void *data,
							    Ecore_Desktop_Tree
							    * tree,
							    int element,
							    int level);

static int          _ecore_desktop_menu_generate(const void *data,
						 Ecore_Desktop_Tree * tree,
						 int element, int level);
static int          _ecore_desktop_menu_is_include(const void *data, Ecore_Desktop_Tree * tree, int element, int level __UNUSED__);
static void         _ecore_desktop_menu_inherit_apps(void *value,
						     void *user_data);
static void         _ecore_desktop_menu_select_app(void *value,
						   void *user_data);
static int          _ecore_desktop_menu_apply_rules(struct
						    _ecore_desktop_menu_generate_data
						    *generate_data,
						    Ecore_Desktop_Tree * rule,
						    char *key,
						    Ecore_Desktop * desktop);

/**
 * @defgroup Ecore_Desktop_Menu_Group menu Functions
 *
 * Functions that deal with freedesktop.org menus.
 */

EAPI void
ecore_desktop_menu_for_each(void (*func)
			    (const char *name, const char *path, const char *directory, Eina_Hash * apps))
{
   char               *menu_file;

   /* Find the main menu file. */
   menu_file = ecore_desktop_paths_file_find(ecore_desktop_paths_menus,
					     "applications.menu", -1, NULL,
					     NULL);
   if (!menu_file)
     {
	/* Try various quirks of various systems and other wms. */
	menu_file = ecore_desktop_paths_file_find(ecore_desktop_paths_menus,
						  "kde-applications.menu", -1, NULL,
						  NULL);
        if (!menu_file)
          {
	     menu_file = ecore_desktop_paths_file_find(ecore_desktop_paths_menus,
						  "debian-menu.menu", -1, NULL,
						  NULL);
	     /* FIXME: If all else fails, run debians funky menu generator shit. */
	  }
     }

   if (menu_file)
     {
	Ecore_Desktop_Tree *menus;

	/* convert the xml into menus */
	menus = ecore_desktop_menu_get(menu_file);
	if (menus)
	  {
	     /* create the .desktop and order files from the menu */
	     ecore_desktop_tree_foreach(menus, 0, _ecore_desktop_menu_make_apps,
					func);
// FIXME: Can't free this just yet, causes major memory corruption.
// this leaks a LOT (428,200 bytes in one valgrind run...)
//               ecore_desktop_tree_del(menus);
	  }
	free(menu_file);
     }
}

static int
_ecore_desktop_menu_make_apps(const void *data, Ecore_Desktop_Tree * tree,
			      int element, int level __UNUSED__)
{
   if (tree->elements[element].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
     {
	if (strncmp((char *)tree->elements[element].element, "<MENU ", 6) == 0)
	  {
	     int                 len;
	     char               *path;
	     char               *name;
	     char               *directory;
	     Eina_Hash         *apps;
	     void                (*func) (char *name, char *path, char *directory,
					  Eina_Hash * apps);

	     func = data;
	     name = (char *)tree->elements[element].element;
	     directory = name;
	     directory += 8;
	     path = strchr(directory, '<');
	     path++;
	     directory = strchr(path, '<');
	     directory++;
	     len = strlen(directory);
	     if (len > 2)
	       {
	          path = strdup(directory);
		  if (path)
		    {
	               path[len - 1] = '\0';
                       /* FIXME: Figure out what to do if it's just ".directory". */
	               directory = ecore_desktop_paths_file_find(ecore_desktop_paths_directories, path, 0, NULL, NULL);
		       free(path);
		    }
		  else
	             directory = NULL;
	       }
	     else
	        directory = NULL;
	     path = (char *)tree->elements[element + 1].element;
//             pool = (Eina_Hash *) tree->elements[element + 2].element;
	     apps = (Eina_Hash *) tree->elements[element + 4].element;
	     path = &path[11];
#ifdef DEBUG
	     printf("OUTPUTTING MENU - %s \t\t%s \t\t%s\n", path, name, directory);
#endif
	     if (func)
		func(name, path, directory, apps);
	     free(directory);
	  }
     }
   return 0;
}

/**
 * Decode a freedesktop.org menu XML jungle.
 *
 * Using the algorithm specified by freedesktop.org, fully decode
 * a menu based on an initial menu file.
 *
 * @param   file The base file for the menu.
 * @return  The resulting menu tree.
 * @ingroup Ecore_Desktop_Menu_Group
 */

Ecore_Desktop_Tree *
ecore_desktop_menu_get(char *file)
{
   return _ecore_desktop_menu_get0(file, NULL, 0);
}

static Ecore_Desktop_Tree *
_ecore_desktop_menu_get0(char *file, Ecore_Desktop_Tree * merge_stack,
			 int level)
{
   Ecore_Desktop_Tree *menu_xml;
   struct _ecore_desktop_menu_unxml_data data;
   int                 oops = 0;

   /* Preperation. */
   data.stack = ecore_desktop_tree_new(NULL);
   /* FIXME data.base and data.path leak */
   data.base = ecore_file_strip_ext(ecore_file_file_get(file));
   data.path = ecore_file_dir_get(file);
   if ((level == 0) && (merge_stack == NULL))
      merge_stack = ecore_desktop_tree_new(NULL);
#ifdef DEBUG
printf("MENU FILE %d - %s\n", level, file);
#endif
   menu_xml = ecore_desktop_xmlame_get(file);
   if ((data.stack) && (data.base) && (data.path) && (merge_stack)
       && (menu_xml))
     {
	int                 i;

	data.file = file;
	data.level = level;
	data.merge_stack = merge_stack;

	/* Setup the merge stack. */
	if (merge_stack->size <= level)
	  {
      /* XXX storing a pointer to a static string probably isn't a good idea. this is most likely the cause of the valgrind error mentioned below. (once this function has exited that memory will no longer be valid) */
	     while (merge_stack->size < level)
		ecore_desktop_tree_add(merge_stack, "");
	     ecore_desktop_tree_add(merge_stack, file);
	  }
	else
	   merge_stack->elements[level].element = file;

	/* Find out if we are looping. */
	for (i = 0; i < level; i++)
	  {
	     char               *text;

	     /* I can safely assume that they are all strings. */
	     text = (char *)merge_stack->elements[i].element;
/* printf("---  %s  ---  %s\n", text, file);
	     if (text)
	     if (file) */
	     if (strcmp(text, file) == 0)
	       {
/* FIXME: valgrind says -
==5761== Conditional jump or move depends on uninitialised value(s)
==5761==    at 0x1B90565C: strcmp (in /usr/lib/valgrind/vgpreload_memcheck.so)
==5761==    by 0x1BA3E984: _ecore_desktop_menu_get0 (ecore_desktop_menu.c:176)
==5761==    by 0x1BA3FE3F: _ecore_desktop_menu_merge (ecore_desktop_menu.c:1146)
==5761==    by 0x1BA3D5A2: ecore_desktop_tree_foreach (ecore_desktop_tree.c:267)
 * which is the strcmp just above.  But it doesn't complain about the first two if's, 
 * or the printf, which I inserted to try and track this down.
 * No idea what it actually is complaining about, so I'll comment it for future study.
 *
 * This is probably caused by the fact that a static string was passed into the
 * tree. See the comment above where this occurs.
 */
		  fprintf(stderr,
			  "\n### Oops, infinite menu merging loop detected at %s\n",
			  file);
		  oops++;
	       }
	  }

	if (oops == 0)
	  {
	     /* Get on with it. */
	     ecore_desktop_tree_foreach(menu_xml, 0, _ecore_desktop_menu_unxml,
					&data);
	     ecore_desktop_tree_foreach(menu_xml, 0, _ecore_desktop_menu_merge,
					&data);

	     /* The rest of this is only done after ALL the menus have been merged. */
	     if (level == 0)
	       {
		  ecore_desktop_tree_foreach(menu_xml, 0,
					     _ecore_desktop_menu_expand_default_dirs,
					     &data);

#ifdef DEBUG
		  ecore_desktop_tree_dump(menu_xml, 0);
		  printf("\n\n");
#endif

		  data.unallocated = FALSE;
		  ecore_desktop_tree_foreach(menu_xml, 0,
					     _ecore_desktop_menu_generate,
					     &data);

		  data.unallocated = TRUE;
		  ecore_desktop_tree_foreach(menu_xml, 0,
					     _ecore_desktop_menu_generate,
					     &data);

#ifdef DEBUG
//		  ecore_desktop_tree_dump(menu_xml, 0);
//		  printf("\n\n");
#endif
	       }
	  }
     }
   else
      oops++;

   if (oops)
     {
	E_FN_DEL(ecore_desktop_tree_del, (menu_xml));
	if (level == 0)
	  {
	     E_FN_DEL(ecore_desktop_tree_del, (merge_stack));
	  }
   E_FREE(data.path);
   E_FREE(data.base);
   E_FN_DEL(ecore_desktop_tree_del, (data.stack));
     }


   return menu_xml;
}

static int
_ecore_desktop_menu_unxml(const void *data, Ecore_Desktop_Tree * tree,
			  int element, int level)
{
   struct _ecore_desktop_menu_unxml_data *unxml_data;

   unxml_data = (struct _ecore_desktop_menu_unxml_data *)data;
   if (tree->elements[element].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
     {
	if (strncmp((char *)tree->elements[element].element, "<!", 2) == 0)
	  {
	     tree->elements[element].type =
		ECORE_DESKTOP_TREE_ELEMENT_TYPE_NULL;
	     tree->elements[element].element = NULL;
	  }
	else if (strcmp((char *)tree->elements[element].element, "<Menu") == 0)
	  {
	     Ecore_Desktop_Tree *menu;

	     menu = _ecore_desktop_menu_create_menu();
	     if (menu)
	       {
		  int                 i;
		  char               *flags = "    ", *name = "", *directory =
		     "", *menu_path = "";
		  char                temp[PATH_MAX];

		  flags = (char *)menu->elements[0].element;
		  flags += 7;
		  tree->elements[element].element = menu;
		  tree->elements[element].type =
		     ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE;
		  for (i = element + 1; i < tree->size; i++)
		    {
		       int                 result = 0;

		       if (tree->elements[i].type ==
			   ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
			 {
			    if (strncmp
				((char *)tree->elements[i].element, "<!",
				 2) == 0)
			      {
				 tree->elements[i].type =
				    ECORE_DESKTOP_TREE_ELEMENT_TYPE_NULL;
				 tree->elements[i].element = NULL;
			      }
			    else
			       if (strcmp
				   ((char *)tree->elements[i].element,
				    "<Deleted/") == 0)
			      {
				 flags[1] = 'D';
				 result = 1;
			      }
			    else
			       if (strcmp
				   ((char *)tree->elements[i].element,
				    "<NotDeleted/") == 0)
			      {
				 flags[1] = ' ';
				 result = 1;
			      }
			    else
			       if (strcmp
				   ((char *)tree->elements[i].element,
				    "<OnlyUnallocated/") == 0)
			      {
				 flags[2] = 'O';
				 result = 1;
			      }
			    else
			       if (strcmp
				   ((char *)tree->elements[i].element,
				    "<NotOnlyUnallocated/") == 0)
			      {
				 flags[2] = ' ';
				 result = 1;
			      }
			    else
			       if (strcmp
				   ((char *)tree->elements[i].element,
				    "</Menu") == 0)
			      {
				 result = 1;
			      }
			    else
			      {
				 ecore_desktop_tree_extend(menu,
							   (char *)tree->
							   elements[i].element);
				 result = 1;
			      }
			 }
		       else if (tree->elements[i].type ==
				ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
			 {
			    Ecore_Desktop_Tree *sub;

			    sub =
			       (Ecore_Desktop_Tree *) tree->elements[i].element;
			    if ((sub) && (sub->size))
			      {
				 if (sub->elements[0].type ==
				     ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
				   {
				      if (strcmp
					  ((char *)sub->elements[0].element,
					   "<Name") == 0)
					{
					   int                 i, length = 0;

					   name =
					      strdup((char *)sub->elements[1].
						     element);
					   sprintf(temp,
						   "<MENU <%.4s> <%s> <%s>",
						   flags, name, directory);
					   menu->elements[0].element =
					      strdup(temp);
					   ecore_desktop_tree_track(menu, name);
					   ecore_desktop_tree_track(menu,
								    menu->
								    elements[0].
								    element);
					   flags =
					      (char *)menu->elements[0].element;
					   flags += 7;
					   /* The rest of this is probably not needed, except to ease debugging. */
					   if (unxml_data->stack->size <= level)
					     {
						while (unxml_data->stack->size <
						       level)
						   ecore_desktop_tree_add
						      (unxml_data->stack, "");
						ecore_desktop_tree_add
						   (unxml_data->stack, name);
					     }
					   else
					      unxml_data->stack->
						 elements[level].element = name;
					   temp[0] = '\0';
					   for (i = 0; i <= level; i++)
					     {
						char               *text;

						/* I can safely assume that they are all strings. */
						text =
						   (char *)unxml_data->stack->
						   elements[i].element;
						if (text[0] != '\0')
						  {
						     sprintf(&temp[length],
							     "%s%s",
							     ((length) ? "/" :
							      ""), text);
						     length +=
							strlen(text) +
							((length) ? 1 : 0);
						  }
					     }
					   menu_path = strdup(temp);
					   sprintf(temp, "<MENU_PATH %s",
						   menu_path);
					   menu->elements[1].element =
					      strdup(temp);
					   ecore_desktop_tree_track(menu,
								    menu_path);
					   ecore_desktop_tree_track(menu,
								    menu->
								    elements[1].
								    element);
					   result = 1;
					}
				      /* FIXME: Move this to later in the sequence. */
				      else
					 if (strcmp
					     ((char *)sub->elements[0].element,
					      "<Directory") == 0)
					{
					   directory =
					      strdup((char *)sub->elements[1].
						     element);
					   sprintf(temp,
						   "<MENU <%.4s> <%s> <%s>",
						   flags, name, directory);
					   menu->elements[0].element =
					      strdup(temp);
					   ecore_desktop_tree_track(menu,
								    directory);
					   ecore_desktop_tree_track(menu,
								    menu->
								    elements[0].
								    element);
					   flags =
					      (char *)menu->elements[0].element;
					   flags += 7;
					   result = 1;
					}
				      else
					 if (strcmp
					     ((char *)sub->elements[0].element,
					      "<Menu") == 0)
					{
					   _ecore_desktop_menu_unxml(data, sub,
								     0,
								     level + 1);
					   ecore_desktop_tree_add_child(menu,
									(Ecore_Desktop_Tree
									 *)
									sub->
									elements
									[0].
									element);
					   /* FIXME: Dunno if this causes a memory leak, but for now we play it safe. */
					   tree->elements[i].type =
					      ECORE_DESKTOP_TREE_ELEMENT_TYPE_NULL;
					   tree->elements[i].element = NULL;
//                                                 result = 1;
					}
				      else
					 if (strcmp
					     ((char *)sub->elements[0].element,
					      "<Move") == 0)
					{
					   _ecore_desktop_menu_unxml_moves
					      (menu, sub);
					   result = 1;
					}
				      else
					{
					   if ((sub->size == 3)
					       && (sub->elements[1].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
					       && (((char *)sub->elements[1].element)[0] != '<'))
					     {
						char temp[PATH_MAX];

						snprintf(temp, PATH_MAX, "%s %s",
						        (char *)sub->elements[0].element,
							(char *)sub->elements[1].element);
						ecore_desktop_tree_extend(menu, temp);
						result = 1;
					     }
					   else
					     {
						ecore_desktop_tree_add_child
						   (menu, sub);
						tree->elements[i].type =
						   ECORE_DESKTOP_TREE_ELEMENT_TYPE_NULL;
						tree->elements[i].element =
						   NULL;
					     }
					}
				   }
			      }
			 }
		       if (result)
			 {
			    if (tree->elements[i].type ==
				ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
			       ecore_desktop_tree_del((Ecore_Desktop_Tree *)
						      tree->elements[i].
						      element);
			    tree->elements[i].type =
			       ECORE_DESKTOP_TREE_ELEMENT_TYPE_NULL;
			    tree->elements[i].element = NULL;
			 }
		    }

		  /* Add it if it has not been deleted. */
		  if (flags[1] != 'D')
		    {
		       tree->elements[element].element = menu;
		       tree->elements[element].type =
			  ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE;
		    }
	       }
	  }
     }
   return 0;
}

static Ecore_Desktop_Tree *
_ecore_desktop_menu_create_menu()
{
   Ecore_Desktop_Tree *menu, *rules;
   Eina_Hash         *pool, *apps;

   menu = ecore_desktop_tree_new(NULL);
   rules = ecore_desktop_tree_new(NULL);
   pool = eina_hash_string_superfast_new(free);
   apps = eina_hash_string_superfast_new(free);
   if ((menu) && (rules) && (pool) && (apps))
     {
	ecore_desktop_tree_extend(menu, "<MENU <    > <> <>");
	ecore_desktop_tree_extend(menu, "<MENU_PATH ");
	ecore_desktop_tree_add_hash(menu, pool);
	ecore_desktop_tree_add_child(menu, rules);
	ecore_desktop_tree_add_hash(menu, apps);
     }
   else
     {
	if (apps)
	   eina_hash_free(apps);
	if (pool)
	   eina_hash_free(pool);
	if (rules)
	   ecore_desktop_tree_del(rules);
	if (menu)
	   ecore_desktop_tree_del(menu);
	menu = NULL;
     }

   return menu;
}

static int
_ecore_desktop_menu_check_directory(void *data, const char *path)
{
   char               *p;
   Ecore_Desktop_Tree *merge;

   merge = (Ecore_Desktop_Tree *) data;
   p = strrchr(path, '.');
   if (p)
     {
	if (strcmp(p, ".directory") == 0)
	  {
	     char                merge_file[PATH_MAX];

	     sprintf(merge_file, "<Directory %s", path);
	     ecore_desktop_tree_extend(merge, merge_file);
	  }
     }

   return 0;
}

static int
_ecore_desktop_menu_check_menu(void *data, const char *path)
{
   char               *p;
   Ecore_Desktop_Tree *merge;

   merge = (Ecore_Desktop_Tree *) data;
   p = strrchr(path, '.');
   if (p)
     {
	if (strcmp(p, ".menu") == 0)
	  {
	     char                merge_file[PATH_MAX];

	     sprintf(merge_file, "<MergeFile type=\"path\" %s", path);
	     ecore_desktop_tree_extend(merge, merge_file);
	  }
     }

   return 0;
}

static int
_ecore_desktop_menu_legacy_menu_dir(void *data, const char *path)
{
   struct _ecore_desktop_menu_legacy_data *legacy_data;
   Ecore_Desktop_Tree *menu;
   char               *temp_path;
   int                 start;
   int                 i, count = 0;

   legacy_data = (struct _ecore_desktop_menu_legacy_data *)data;
   for (i = legacy_data->length; path[i] != '\0'; i++)
     {
	if (path[i] == '/')
	   count++;
     }
   if (count == 1)
     {
	legacy_data->current = legacy_data->merge;
	legacy_data->level = 0;
     }
   else if (count <= legacy_data->level)
     {
	if ((legacy_data->current) && (legacy_data->current->parent))
	  {
	     legacy_data->current = legacy_data->current->parent;
	     legacy_data->level--;
	  }
     }

   start = legacy_data->length;
   count = 0;
   temp_path = strdup(path);
   if (temp_path)
     {
	for (i = legacy_data->length; temp_path[i] != '\0'; i++)
	  {
	     if (temp_path[i] == '/')
	       {
		  if (count >= legacy_data->level)
		    {
		       temp_path[i] = '\0';
		       menu = _ecore_desktop_menu_create_menu();
		       if (menu)
			 {
			    char                temp[PATH_MAX];

			    sprintf(temp, "<MENU <   L> <%s> <>",
				    &temp_path[start]);
			    menu->elements[0].element = strdup(temp);
			    ecore_desktop_tree_track(menu,
						     menu->elements[0].element);
			    sprintf(temp, "<MENU_PATH %s/%s", legacy_data->menu,
				    &temp_path[legacy_data->length]);
			    menu->elements[1].element = strdup(temp);
			    ecore_desktop_tree_track(menu,
						     menu->elements[1].element);

			    ecore_desktop_tree_add_child(legacy_data->current,
							 menu);
			    /*  This is not needed, but if it was, this is where it would go.
			     * sprintf(temp, "<AppDir %s/", temp_path);
			     * ecore_desktop_tree_extend(menu, temp);
			     */
			    sprintf(temp, "<DirectoryDir %s/", temp_path);
			    ecore_desktop_tree_extend(menu, temp);

			    legacy_data->current = menu;
			    legacy_data->level++;
			 }
		       temp_path[i] = '/';
		    }
		  start = i + 1;
		  count++;
	       }
	  }
	free(temp_path);
     }

   legacy_data->level = count;

   return 0;
}

static int
_ecore_desktop_menu_legacy_menu(void *data, const char *path)
{
   struct _ecore_desktop_menu_legacy_data *legacy_data;
   char               *menu_path;
   char               *file;
   char                temp[PATH_MAX];
   int                 i, count = 0, menu_count = 0;

   legacy_data = (struct _ecore_desktop_menu_legacy_data *)data;
   if ((legacy_data->current) && (legacy_data->current->size > 0) &&
       (legacy_data->current->elements[1].type ==
	ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING))
     {
	menu_path = (char *)legacy_data->current->elements[1].element;
	menu_path += 12 + legacy_data->menu_length;
     }
   else
     {
	menu_path = "";
	printf("  PROBLEM IN LEGACYDIR FILE - %s - %s  %s\n",
	       legacy_data->prefix, legacy_data->path,
	       &path[legacy_data->length]);
     }

   for (i = legacy_data->length; path[i] != '\0'; i++)
     {
	if (path[i] == '/')
	   count++;
     }
   for (i = 0; menu_path[i] != '\0'; i++)
     {
	if (menu_path[i] == '/')
	   menu_count++;
     }
   /* FIXME: The (legacy_data->current) test is just to patch a seggie, find out why. */
   while ((menu_count >= count) && (legacy_data->current))
     {
	legacy_data->current = legacy_data->current->parent;
	menu_count--;
     }
   if ((legacy_data->current) && (legacy_data->current->size > 0) &&
       (legacy_data->current->elements[1].type ==
	ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING))
     {
	menu_path = (char *)legacy_data->current->elements[1].element;
	menu_path += 12 + legacy_data->menu_length;
     }
   else
     {
	/* FIXME:  Bugger, a corner case.
	 * If this is .directory, then maybe ignore it?
	 * If this is a desktop, we need to put it into the pool of the menu that legacy_data->merge will get merged into.
	 */
	menu_path = "";
	printf("  LEGACYDIR FILE - %s - %s  %s\n", legacy_data->prefix,
	       legacy_data->path, &path[legacy_data->length]);
	return 0;
     }

   menu_count = strlen(menu_path);
   if (menu_count)
      menu_count++;
   file = (char *)&path[legacy_data->length + menu_count];
   count = strlen(file);

   if (strcmp(".directory", file) == 0)
     {
	menu_path = (char *)legacy_data->current->elements[0].element;
	menu_count = strlen(menu_path);
	menu_path[menu_count - 3] = '\0';
	sprintf(temp, "%s <.directory>", menu_path);
	legacy_data->current->elements[0].element = strdup(temp);
	ecore_desktop_tree_track(legacy_data->current,
				 legacy_data->current->elements[0].element);
     }
   else if (strcmp(".desktop", &file[count - 8]) == 0)
     {
	Eina_Hash         *pool;
	Ecore_Desktop_Tree *rules;

	pool = (Eina_Hash *) legacy_data->current->elements[2].element;
	rules =
	   (Ecore_Desktop_Tree *) legacy_data->current->elements[3].element;
	if (rules->size == 0)
	  {
	     Ecore_Desktop_Tree *new_rules;

	     new_rules = ecore_desktop_tree_new(NULL);
	     if (new_rules)
		ecore_desktop_tree_add_child(rules, new_rules);
	  }
	sprintf(temp, "%s%s", legacy_data->prefix, file);
	eina_hash_add(pool, temp, strdup(path));
#ifdef DEBUG
//	printf
//	   ("POOLING - _ecore_desktop_menu_legacy_menu(void *data, %s) - %s - %s\n",
//	    path, file, temp);
#endif
	if (rules->size > 0)
	  {
	     rules = (Ecore_Desktop_Tree *) rules->elements[0].element;
	     sprintf(temp, "IOF %s%s", legacy_data->prefix, file);
	     ecore_desktop_tree_extend(rules, temp);
	  }
     }
#ifdef DEBUG
   else
      printf("PROBLEM - _ecore_desktop_menu_legacy_menu(void *data, %s) - %s\n",
	     path, file);
#endif

   return 0;
}

static void
_ecore_desktop_menu_unxml_rules(Ecore_Desktop_Tree * rules,
				Ecore_Desktop_Tree * tree, char type,
				char sub_type)
{
   int                 i;
   char                temp[PATH_MAX];

   for (i = 0; i < tree->size; i++)
     {
	if (tree->elements[i].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
	  {
             /* FIXME <All /> or <All foo="bar"> would be valid input (which this won't catch) */
	     if (strcmp((char *)tree->elements[i].element, "<All/") == 0)
	       {
		  sprintf(temp, "%c%cA", type, sub_type);
		  ecore_desktop_tree_extend(rules, temp);
	       }
	     else if (strcmp((char *)tree->elements[i].element, "<Filename") ==
		      0)
	       {
		  sprintf(temp, "%c%cF %s", type, sub_type,
			  (char *)tree->elements[i + 1].element);
		  ecore_desktop_tree_extend(rules, temp);
	       }
	     else if (strcmp((char *)tree->elements[i].element, "<Category") ==
		      0)
	       {
		  sprintf(temp, "%c%cC %s", type, sub_type,
			  (char *)tree->elements[i + 1].element);
		  ecore_desktop_tree_extend(rules, temp);
	       }
	     else if (strcmp((char *)tree->elements[i].element, "<Or") == 0)
	       {
		  _ecore_desktop_menu_unxml_rules(rules,
						  (Ecore_Desktop_Tree *) tree->
						  elements[i + 1].element,
						  type, sub_type);
	       }
	     else if ((strcmp((char *)tree->elements[i].element, "<And") == 0)
		      || (strcmp((char *)tree->elements[i].element, "<Not") ==
			  0))
	       {
		  char                this_type;
		  Ecore_Desktop_Tree *sub;

		  this_type = ((char *)tree->elements[i].element)[1];
		  sub = ecore_desktop_tree_new(NULL);
		  if (sub)
		    {
		       ecore_desktop_tree_add_child(rules, sub);
		       for (i++; i < tree->size; i++)
			 {
			    if (tree->elements[i].type ==
				ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
			       _ecore_desktop_menu_unxml_rules(sub,
							       (Ecore_Desktop_Tree
								*) tree->
							       elements[i].
							       element, type,
							       this_type);
			 }
		    }
	       }
	  }
	else if (tree->elements[i].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
	  {
	     _ecore_desktop_menu_unxml_rules(rules,
					     (Ecore_Desktop_Tree *) tree->
					     elements[i].element, type,
					     sub_type);
	  }
     }
}

static void
_ecore_desktop_menu_unxml_moves(Ecore_Desktop_Tree * menu,
				Ecore_Desktop_Tree * tree)
{
   int                 i;
   char               *old = NULL;
   char               *new = NULL;

   for (i = 0; i < tree->size; i++)
     {
	if (tree->elements[i].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
	  {
	     Ecore_Desktop_Tree *sub;

	     sub = (Ecore_Desktop_Tree *) tree->elements[i].element;
	     if ((sub) && (sub->size))
	       {
		  if (sub->elements[0].type ==
		      ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
		    {
		       if (strcmp((char *)sub->elements[0].element, "<Old") ==
			   0)
			  old = strdup((char *)sub->elements[1].element);
		       if (strcmp((char *)sub->elements[0].element, "<New") ==
			   0)
			  new = strdup((char *)sub->elements[1].element);
		    }
	       }
	  }
	if ((old) && (new))
	  {
	     char                temp[PATH_MAX * 2];

	     sprintf(temp, "<MOVE <%s> <%s>", old, new);
	     ecore_desktop_tree_extend(menu, temp);
	  }

	if (old) free(old);
	old = NULL;
	if (new) free(new);
	new = NULL;
     }
}

static void
_ecore_desktop_menu_add_dirs(Ecore_Desktop_Tree * tree, Ecore_List * paths,
			     const char *pre, const char *post __UNUSED__, char *extra, int element __UNUSED__)
{
   char                t[PATH_MAX], *this_path;

   /* reverse the order of the dirs. */
   ecore_list_first_goto(paths);
   while ((this_path = ecore_list_next(paths)) != NULL)
     {
	if (extra)
	   sprintf(t, "%s %s%s-merged/", pre, this_path, extra);
	else
	   sprintf(t, "%s %s", pre, this_path);
	if (tree)
	   ecore_desktop_tree_extend(tree, t);
     }
}

static int
_ecore_desktop_menu_expand_apps(struct _ecore_desktop_menu_unxml_data
				*unxml_data, char *app_dir, Eina_Hash * pool)
{
   if (pool)
     {
	struct _ecore_desktop_menu_expand_apps_data our_data;
	char                dir[PATH_MAX];

	our_data.pool = pool;
	sprintf(dir, "%s", app_dir);
	if (dir[0] != '/')
	   sprintf(dir, "%s/%s", unxml_data->path, app_dir);
	our_data.path = dir;
	our_data.length = strlen(dir);
#ifdef DEBUG
	printf
	   ("EXPANDING - _ecore_desktop_menu_expand_apps(unxml_data, %s) - %s\n",
	    app_dir, dir);
#endif
	ecore_desktop_paths_recursive_search(dir, NULL, -1, NULL,
					     _ecore_desktop_menu_check_app,
					     &our_data);
     }
   return 0;
}

static int
_ecore_desktop_menu_check_app(void *data, const char *path)
{
   char               *p;
   struct _ecore_desktop_menu_expand_apps_data *our_data;

   our_data = (struct _ecore_desktop_menu_expand_apps_data *)data;
   p = strrchr(path, '.');
   if (p)
     {
	if (strcmp(p, ".desktop") == 0)
	  {
	     int                 i;
	     char               *file;

	     file = strdup(path + our_data->length);
	     if (file)
	       {
		  if (path)
		    {
		       for (i = 0; file[i] != '\0'; i++)
			 if (file[i] == '/')
			   file[i] = '-';
		       eina_hash_add(our_data->pool, file, strdup(path));
		    }
		  free(file);
	       }
#ifdef DEBUG
//		  printf
//		     ("POOLING - _ecore_desktop_menu_check_app(void *data, %s) - %s\n",
//		      path, file);
#endif
	       }
	  }
#ifdef DEBUG
	else
	   printf("PROBLEM - _ecore_desktop_menu_check_app(void *data, %s)\n",
		  path);
#endif
     }

   return 0;
}

static int
_ecore_desktop_menu_merge(const void *data, Ecore_Desktop_Tree * tree,
			  int element, int level)
{
   struct _ecore_desktop_menu_unxml_data *unxml_data;
   Ecore_Desktop_Tree *merge;
   int                 result = 0;

   unxml_data = (struct _ecore_desktop_menu_unxml_data *)data;
   merge = ecore_desktop_tree_new(NULL);
   if (tree->elements[element].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
     {
	char               *string;

	string = (char *)tree->elements[element].element;
	if (strcmp(string, "<DefaultMergeDirs/") == 0)
	  {
	     if (unxml_data->base)
		_ecore_desktop_menu_add_dirs(merge, ecore_desktop_paths_menus,
					     "<MergeDir", "</MergeDir",
					     unxml_data->base, element);
	     result = 1;
	  }
	else if (strcmp(string, "<KDELegacyDirs/") == 0)
	  {
	     _ecore_desktop_menu_add_dirs(merge,
					  ecore_desktop_paths_kde_legacy,
					  "<LegacyDir prefix=\"kde-\"",
					  "</LegacyDir", NULL, element);
	     result = 1;
	  }
	else if (strncmp(string, "<MergeDir ", 10) == 0)
	  {
	     char                merge_path[PATH_MAX];

	     if (string[10] == '/')
		sprintf(merge_path, "%s", &string[10]);
	     else
		sprintf(merge_path, "%s%s", unxml_data->path, &string[10]);
	     ecore_desktop_paths_recursive_search(merge_path, NULL, -1, NULL,
						  _ecore_desktop_menu_check_menu,
						  merge);
	     result = 1;
	  }
	else if (strncmp(string, "<LegacyDir ", 11) == 0)
	  {
	     char                merge_path[PATH_MAX];
	     struct _ecore_desktop_menu_legacy_data legacy_data;

	     string += 11;
	     legacy_data.prefix = NULL;
	     legacy_data.merge = merge;
	     legacy_data.current = merge;
	     legacy_data.level = 0;
	     legacy_data.menu = (char *)tree->elements[0].element;
	     legacy_data.menu += 14;
	     legacy_data.menu_length =
		strchr(legacy_data.menu, '>') - legacy_data.menu;
	     legacy_data.menu[legacy_data.menu_length] = '\0';
	     if (strncmp(string, "prefix=\"", 8) == 0)
	       {
		  string += 8;
		  legacy_data.prefix = string;
		  while ((*string != '"') && (*string != '\0'))
		     string++;
		  if (*string != '\0')
		     *string++ = '\0';
		  while ((*string == ' ') && (*string != '\0'))
		     string++;
	       }
	     if (string[0] == '/')
		sprintf(merge_path, "%s", string);
	     else
		sprintf(merge_path, "%s%s", unxml_data->path, string);
	     legacy_data.path = merge_path;
	     legacy_data.length = strlen(merge_path);
#ifdef DEBUG
	     printf("<LEGACYDIR> - %s - %s\n", legacy_data.prefix, merge_path);
#endif
	     ecore_desktop_paths_recursive_search(merge_path, NULL, -1,
						  _ecore_desktop_menu_legacy_menu_dir,
						  _ecore_desktop_menu_legacy_menu,
						  &legacy_data);
	     legacy_data.menu[legacy_data.menu_length] = '>';
	     result = 1;
	  }
	else if (strncmp(string, "<MergeFile ", 11) == 0)
	  {
	     char                merge_path[PATH_MAX];
	     int                 path_type = 1;

	     /* FIXME: need to weed out duplicate <MergeFile's, use the last one. */
	     string += 11;
	     if (strncmp(string, "type=\"", 6) == 0)
	       {
		  string += 6;
		  if (strncmp(string, "parent\"", 7) == 0)
		     path_type = 0;
		  while ((*string != '"') && (*string != '\0'))
		     string++;
		  if (*string != '\0')
		     string++;
		  while ((*string == ' ') && (*string != '\0'))
		     string++;
	       }
	     if (path_type)
	       {
		  if (string[0] == '/')
		     snprintf(merge_path, PATH_MAX, "%s", string);
		  else
		     snprintf(merge_path, PATH_MAX, "%s/%s", unxml_data->path, string);
	       }
	     else		/* This is a parent type MergeFile. */
	       {
	          char *xdg_path;
		  int found = -1;

		  /* The spec is a little unclear, and the examples may look like they
		   * contradict the description, but it all makes sense if you cross
		   * reference it with the XDG Base Directory Specification (version 0.6).
		   * To make things harder, parent type MergeFiles never appear on my box.
		   *
		   * What you do is this.
		   *
		   * Just plain ignore the specified path in the MergeFile element, it's for 
		   * legacy apps that don't understand parent types.
		   *
		   * Take the XDG_CONFIG_DIRS stuff as a whole ($XDG_CONFIG_HOME, then 
		   * $XDG_CONFIG_DIRS), in this code that will be ecore_desktop_paths_config.
		   *
		   * If this menu file is from one of the directories in ecore_desktop_paths_config,
		   * scan the rest of ecore_desktop_paths_config looking for the new menu.  In other 
		   * words start searching in the next ecore_desktop_paths_config entry after the one
		   * that this menu is in.
		   *
		   * The file to look for is the path to this menu with the portion from
		   * ecore_desktop_paths_config stripped off the beginning.  For instance, the top level
		   * menu file is typically /etc/xdg/menus/applications.menu, and /etc/xdg is
		   * typically in ecore_desktop_paths_config, so search for menus/applications.menu.
		   *
		   * If this menu file is NOT from one of the directories in ecore_desktop_paths_menus,
		   * insert nothing.
		   *
		   * The first one found wins, if none are found, don't merge anything.
		   */

		  merge_path[0] = '\0';
	          ecore_list_first_goto(ecore_desktop_paths_config);
	          while ((xdg_path = ecore_list_next(ecore_desktop_paths_config)) != NULL)
	            {
		       if (found < 0)
		         {
			    int length = strlen(xdg_path);

		            if (strncmp(xdg_path, unxml_data->file, length) == 0)
			       found = length;
			 }
		       else
		         {
			    snprintf(merge_path, PATH_MAX, "%s%s", xdg_path, &(unxml_data->file)[found]);
			    if (ecore_file_exists(merge_path))
			       break;
			    merge_path[0] = '\0';
			 }
		    }

	       }
	     if (merge_path[0] != '\0')
	       {
		  Ecore_Desktop_Tree *new_menu;

		  new_menu =
		     _ecore_desktop_menu_get0(merge_path,
					      unxml_data->merge_stack,
					      level + 1);
		  if (new_menu)
		    {
		       if (new_menu->size > 1)
			 {
			    if (new_menu->elements[1].type ==
				ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
			      {
				 new_menu =
				    (Ecore_Desktop_Tree *) new_menu->
				    elements[1].element;
				 if (new_menu->size > 0)
				   {
				      if (new_menu->elements[0].type ==
					  ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
					{
					   merge =
					      (Ecore_Desktop_Tree *) new_menu->
					      elements[0].element;
					   ecore_desktop_tree_remove(merge, 0);
					   ecore_desktop_tree_remove(merge, 1);
					   ecore_desktop_tree_remove(merge, 2);
					   ecore_desktop_tree_remove(merge, 3);
					   ecore_desktop_tree_remove(merge, 4);
					   /* FIXME: The MENU_PATHs need to be prefixed. */
					}
				      else
					 printf
					    ("FUCK an error in _ecore_desktop_menu_merge(%s)\n",
					     merge_path);
				   }
				 else
				    printf
				       ("FUCK another error in _ecore_desktop_menu_merge(%s)\n",
					merge_path);
			      }
			    else
			       printf
				  ("FUCK ME! An error in _ecore_desktop_menu_merge(%s)\n",
				   merge_path);
			 }
		    }
	       }
	     result = 1;
	  }
     }

   if (result)
     {
	if ((merge) && (merge->size))
	   ecore_desktop_tree_merge(tree, element + 1, merge);

	tree->elements[element].type = ECORE_DESKTOP_TREE_ELEMENT_TYPE_NULL;
	tree->elements[element].element = NULL;
     }

   return 0;
}

static int
_ecore_desktop_menu_expand_default_dirs(const void *data,
					Ecore_Desktop_Tree * tree, int element,
					int level __UNUSED__)
{
   struct _ecore_desktop_menu_unxml_data *unxml_data;
   Ecore_Desktop_Tree *merge;
   int                 result = 0;

   unxml_data = (struct _ecore_desktop_menu_unxml_data *)data;
   merge = ecore_desktop_tree_new(NULL);
   if (tree->elements[element].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
     {
	char               *string;

	string = (char *)tree->elements[element].element;
	if (strcmp(string, "<DefaultAppDirs/") == 0)
	  {
	     _ecore_desktop_menu_add_dirs(merge, ecore_desktop_paths_desktops,
					  "<AppDir", "</AppDir", NULL, element);
	     result = 1;
	  }
	else if (strcmp(string, "<DefaultDirectoryDirs/") == 0)
	  {
	     _ecore_desktop_menu_add_dirs(merge,
					  ecore_desktop_paths_directories,
					  "<DirectoryDir", "</DirectoryDir",
					  NULL, element);
	     result = 1;
	  }
     }
   if (result)
     {
	if ((merge) && (merge->size))
	   ecore_desktop_tree_merge(tree, element + 1, merge);

	tree->elements[element].type = ECORE_DESKTOP_TREE_ELEMENT_TYPE_NULL;
	tree->elements[element].element = NULL;
     }

   return 0;
}

static int
_ecore_desktop_menu_generate(const void *data, Ecore_Desktop_Tree * tree,
			     int element, int level)
{
   struct _ecore_desktop_menu_unxml_data *unxml_data;

   unxml_data = (struct _ecore_desktop_menu_unxml_data *)data;
   if (tree->elements[element].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
     {
	if (strncmp((char *)tree->elements[element].element, "<MENU ", 6) == 0)
	  {
	     int                 i;
	     struct _ecore_desktop_menu_generate_data generate_data;

	     generate_data.unallocated = unxml_data->unallocated;
	     generate_data.name = (char *)tree->elements[element].element;
	     generate_data.path = (char *)tree->elements[element + 1].element;
	     generate_data.pool = (Eina_Hash *) tree->elements[element + 2].element;
	     generate_data.rules = (Ecore_Desktop_Tree *) tree->elements[element + 3].element;
	     generate_data.apps = (Eina_Hash *) tree->elements[element + 4].element;

	     /* generate and inherit the pools on the first pass, and preparse the include/exclude logic. */
	     if (!generate_data.unallocated)
	       {
		  int                 i;

		  for (i = element + 5; i < tree->size; i++)
		    {
		       int                 result = 0;
		       char               *string;

		       if (tree->elements[i].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
			 {
			    string = (char *)tree->elements[i].element;
			    if (strncmp(string, "<AppDir ", 8) == 0)
			      {
				 _ecore_desktop_menu_expand_apps(unxml_data, &string[8], generate_data.pool);
				 result = 1;
			      }
			    else if (strncmp(string, "<DirectoryDir ", 14) == 0)
			      {
				 char                merge_path[PATH_MAX];
				 Ecore_Desktop_Tree *merge;

				 if (string[14] == '/')
				    sprintf(merge_path, "%s", &string[14]);
				 else
				    sprintf(merge_path, "%s%s", unxml_data->path, &string[14]);
				 merge = ecore_desktop_tree_new(NULL);
				 if (merge)
				   {
				      ecore_desktop_paths_recursive_search(merge_path, NULL, -1, NULL, _ecore_desktop_menu_check_directory, merge);
				      ecore_desktop_tree_merge(tree, i + 1, merge);
				   }
				 result = 1;
			      }
			 }
		       else if (tree->elements[i].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
			 {
			    Ecore_Desktop_Tree *sub;

			    sub = (Ecore_Desktop_Tree *) tree->elements[i].element;
			    if ((sub) && (sub->size))
			      {
				 if (sub->elements[0].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
				   {
				      string = (char *)sub->elements[0].element;
				      if ((strcmp(string, "<Include") == 0) || (strcmp(string, "<Exclude") == 0))
					{
					   Ecore_Desktop_Tree *new_sub;

					   new_sub = ecore_desktop_tree_new(NULL);
					   if (new_sub)
					     {
						ecore_desktop_tree_add_child(generate_data.rules, new_sub);
						_ecore_desktop_menu_unxml_rules(new_sub, sub, string[1], 'O');
					     }
					   result = 1;
					}
				   }
			      }
			 }

		       if (result)
			 {
			    tree->elements[i].type = ECORE_DESKTOP_TREE_ELEMENT_TYPE_NULL;
			    tree->elements[i].element = NULL;
			 }
		    }

		  if (unxml_data->stack->size <= level)
		    {
		       while (unxml_data->stack->size < level)
			  ecore_desktop_tree_add_hash(unxml_data->stack, generate_data.pool);
		       ecore_desktop_tree_add_hash(unxml_data->stack, generate_data.pool);
		    }
		  else
		    {
		       unxml_data->stack->elements[level].type = ECORE_DESKTOP_TREE_ELEMENT_TYPE_HASH;
		       unxml_data->stack->elements[level].element = generate_data.pool;
		    }
		  for (i = level - 1; i >= 0; i--)
		    {
		       if (unxml_data->stack->elements[i].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_HASH)
			 {
			    Eina_Hash         *ancestor;

			    ancestor = (Eina_Hash *) unxml_data->stack->elements[i].element;
			    eina_hash_foreach(ancestor, _ecore_desktop_menu_inherit_apps, generate_data.pool);
			 }
		    }
	       }

	     /* Process the rules. */
	     if (generate_data.name[9] == (generate_data.unallocated ? 'O' : ' '))
	       {
#ifdef DEBUG
		  printf("MAKING MENU - %s \t\t%s\n", generate_data.path, generate_data.name);
		  ecore_desktop_tree_dump(generate_data.rules, 0);
		  printf("\n\n");
#endif
		  for (i = 0; i < generate_data.rules->size; i++)
		    {
		       if (generate_data.rules->elements[i].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
			 {
			    generate_data.rule = (Ecore_Desktop_Tree *) generate_data.rules->elements[i].element;
			    if (generate_data.rule->size > 0)
			      {
			         char type = 'I';

                                 /* Find out if this is an include or an exclude.  This info may be way down in the tree. */
				 if (generate_data.rule->elements[0].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
		                    ecore_desktop_tree_foreach(generate_data.rule, 0, _ecore_desktop_menu_is_include, &type);
				 if (type == 'I')
				   {
				      generate_data.include = TRUE;
				      eina_hash_foreach(generate_data.pool, _ecore_desktop_menu_select_app, &generate_data);
				   }
				 else
				   {
				      generate_data.include = FALSE;
				      eina_hash_foreach(generate_data.apps, _ecore_desktop_menu_select_app, &generate_data);
				   }
			      }
			 }
		       else
			  printf("Fuck, a bug in _ecore_desktop_menus.\n");
		    }
	       }
	  }
     }
   return 0;
}

static int
_ecore_desktop_menu_is_include(const void *data, Ecore_Desktop_Tree * tree, int element, int level __UNUSED__)
{
   char *result;

   result = (char *)data;
   if (tree->elements[element].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_STRING)
     {
	*result = ((char *)tree->elements[element].element)[0];
	return 1;
     }

   return 0;
}

static Eina_Bool
_ecore_desktop_menu_inherit_apps(const Eina_Hash *hash, const void *key, void *value, void *user_data)
{
#ifdef DEBUG
//   printf("CHECKING %s - %s\n", app, key);
#endif
   if (!eina_hash_find(user_data, key))
      eina_hash_add(user_data, key, strdup(value));
}

static void
_ecore_desktop_menu_select_app(const Eina_Hash *hash, const void *key, void *value, void *user_data)
{
   Ecore_Desktop      *desktop;
   struct _ecore_desktop_menu_generate_data *generate_data;

   generate_data = (struct _ecore_desktop_menu_generate_data *)user_data;

   /* FIXME: pass an actuall language parameter. */
   desktop = ecore_desktop_get(value, NULL);

   if (desktop)
     {
	if ((generate_data->unallocated) && (desktop->allocated))
	   return;

	if (_ecore_desktop_menu_apply_rules(generate_data, generate_data->rule, key, desktop))
	  {
	     desktop->allocated = TRUE;
	     if (generate_data->include)
	       {
		  eina_hash_add(generate_data->apps, key, strdup(value));
#ifdef DEBUG
		  printf("INCLUDING %s%s - %s\n", ((generate_data->unallocated) ? "UNALLOCATED " : ""), value, key);
#endif
	       }
	     else
	       {
		  eina_hash_del(generate_data->apps, key);
#ifdef DEBUG
		  printf("EXCLUDING %s%s - %s\n", ((generate_data->unallocated) ? "UNALLOCATED " : ""), value, key);
#endif
	       }
	  }
     }
}

static int
_ecore_desktop_menu_apply_rules(struct _ecore_desktop_menu_generate_data *generate_data, Ecore_Desktop_Tree * rule, char *key, Ecore_Desktop * desktop)
{
   char                type = 'O';
   int                 result = FALSE;
   int                 i;

   for (i = 0; i < rule->size; i++)
     {
	if (rule->elements[i].type == ECORE_DESKTOP_TREE_ELEMENT_TYPE_TREE)
	  {
	     result = _ecore_desktop_menu_apply_rules(generate_data, (Ecore_Desktop_Tree *) rule->elements[i].element, key, desktop);
	  }
	else
	  {
	     char               *rul;
	     char                match;
	     int                 sub_result = FALSE;

	     rul = (char *)rule->elements[i].element;
	     type = rul[1];
	     match = rul[2];
	     switch (match)
	       {
	       case 'A':
		  {
		     sub_result = TRUE;
		     break;
		  }

	       case 'F':
		  {
		     if (strcmp(key, &rul[4]) == 0)
			sub_result = TRUE;
		     break;
		  }

	       case 'C':
		  {
		     /* Try to match a category. */
		     if ((desktop->Categories) && (eina_hash_find(desktop->Categories, &rul[4]) != NULL))
		        sub_result = TRUE;
		     break;
		  }
	       }
	     switch (type)
	       {
	          case 'A':  result = TRUE;  if (!sub_result)  return FALSE;  break;
	          case 'N':  result = TRUE;  if (sub_result)   return FALSE;  break;
	          default:		     if (sub_result)   return TRUE;   break;
	       }
	  }
     }

   return result;
}

/*

<Menu (tree)
  name
  flags = "   " or "MDO" the first letter of - Marked, Deleted, OnlyUnallocated 
  pool (hash)
    id = path
    id = path
  rules (tree)
    rule
    rule
  menu (hash)
    id = path
    id = path
  <Menu (tree)
  <Menu (tree)

rules (tree)
  include/exclude or all/file/category x
  and/not (tree)
    include/exclude and/not all/file/category x

ALL
  return true

FILENAME
  if the rule string matches the desktop id return true
  otherwise return false

CATEGORY
  loop through the apps categories
  as soon as one matches the rule string, return true
  otherwise return false.

OR (implied)
  loop through the rules
  as soon as one matches, return true
  otherwise return false.

SUB RULES
  process the sub rules, return the result

AND
  loop through the rules
  as soon as one doesn't match, return false
  otherwise return true.

NOT (implied OR)
  loop through the rules
  as soon as one matches, return false
  otherwise return true.
 */

/*  FDO PARSING STEPS.

An asterisk in the first column marks those bits that are done.
The spec is not numbered, so I have invented a numbering to help match the
steps to the code.

 10000000 merge menus
*11000000   expand <KDELegacyDir>'s to <LegacyDir>.
*12000000   expand <LegacyDir>'s
*12100000     for each dir (recursive)
*12110000       create recursively nested <MENU <   L> <dirname> <> element
*12120000    //   <AppDir>dirpath</AppDir>
*12130000       <DirectoryDir>dirpath</DirectoryDir>
*12140000       if exist .directory
*12141000         add <.directory> to name
*12150000       <Include>
*12160000       for each *.desktop
 12161000         if no categories in bar.desktop
*12161100	    <Filename>prefix-bar.desktop</Filename> 
 12162000         add "Legacy" to categories
*12163000         add any prefix to the desktop ID.
*12164000	  add it to the pool
*12170000       </Include>
*13000000   for each <MergeFile> and <MergeDir> element
*13100000     get the root <Menu> elements from that elements file/s.
*13200000     remove the <Name> element from those root <Menu> elements.
*13300000     replace that element with the child elements of those root <Menu> elements.
*13400000?    expand the <DefaultMergeDirs> to <MergeDir>'s $XDG_CONFIG_DIRS/menus/foo-merged/ for the menu file foo.menu
FIXME: The above should probably move to before 13000000
*14000000   loop until all <MergeFile> and <MergeDir> elements are done,
*14000000   careful to avoid infinite loops in files that reference each other.
*15000000   for each <Menu> recursively
 15100000     consolidate duplicate child <Menu>s.
*15200000     expand <DefaultAppDir>s and <DefaultDirectoryDir>s to <AppDir>s and <DirectoryDir>s.
 15300000     consolidate duplicate child <AppDir>s, <DirectoryDir>s, and <Directory>s.
 16000000   for each <Menu> recursively (but from the deepest level out)
 16100000     for each <Move>
 16110000       do the move.
 16120000       mark any <Menu> that has been affected.
 17000000   for each marked <Menu> recursively
 17100000     consolidate duplicate child <Menu>s.
*18000000   for each <Menu> recursively
*18100000     if there are <Deleted> elements that are not ovreridden by a <NotDelete> element
*18110000       remove this <Menu> element and all it's children.

*20000000 generate menus
*21000000   for each <Menu> that is <NotOnlyUnallocated> (which is the default)
*21100000     for each <AppDir>
*21110000       for each .desktop
*21111000         if it exists in the pool, replace it.
*21112000	  else add it to the pool.
*21200000     for each parent <Menu>
*21210000       for each .desktop in the pool
*21211000         if it doesn't exist in the child <Menu> pool
*21211100	    add it to the pool.
*21300000     for each <Include> and <Exclude>
*21310000       if rule is an <Include>
*21311000         for each .desktop in pool
*21311100           for each rule
*21311110             if rule matches .desktop in pool
*21311111	        add .desktop to menu.
*21311112	        mark it as allocated
*21320000       if rule is an <Exclude>
*21321000         for each .desktop in menu
*21321100           for each rule
*21321110             if rule matches .desktop in menu
*21321111	        remove .desktop from menu.
*21321112	        leave it as allocated.

*30000000 generate unallocated menus
*31000000   Same as for generate menus, but only the <OnlyUnallocated> ones.
*32000000   Only the unallocated .desktop entries can be used.

 40000000 generate menu layout
   This part of the spec is a whole other rabbit hole, and optional.  B-)
*/


/*  THINGS TAHT CAN BE DUPLICATED

AppDir        last dup is used
.desktop      last dup is used
DirectoryDir  last dup is used
.directory    last dup is used
Directory     last dup is used, unless it points to a non existant entry, then work your way backwards
LegacyDir     last dup is used
MergeDir      last dup is used
MergeFile     last dup is used

DefaultMergeDirs   dirs that are earlier in the path go later in the menu, so they have priority
KDELegacyDirs      dirs that are earlier in the path go later in the menu, so they have priority

OnlyUnallocated & NotOnlyUnallocated   last one wins, default is NotOnlyUnallocated
Deleted & NotDeleted                   last one wins

Include & Exclude  done in order
*/
