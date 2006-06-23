#include "ecore_private.h"
#include "Ecore_Data.h"

static Ecore_List *group_list = NULL;

Ecore_Path_Group *__ecore_path_group_find(char *name);
Ecore_Path_Group *__ecore_path_group_find_id(int id);

/**
 * @defgroup Ecore_Path_Group Path Group Functions
 *
 * Functions that make it easier to find a file in a set of search paths.
 *
 * @todo Give this a better description.
 */

/**
 * Creates a new path group.
 * @param   group_name The name of the new group.
 * @return  @c 0 on error, the integer id of the new group on success.
 * @ingroup Ecore_Path_Group
 */
EAPI int
ecore_path_group_new(char *group_name)
{
   Ecore_Path_Group *last;
   Ecore_Path_Group *group;

   CHECK_PARAM_POINTER_RETURN("group_name", group_name, -1);

   if (!group_list)
     group_list = ecore_list_new();
   else
     {
	group = __ecore_path_group_find(group_name);
	if (group)
	  return -1;
     }

   group = (Ecore_Path_Group *)malloc(sizeof(Ecore_Path_Group));
   memset(group, 0, sizeof(Ecore_Path_Group));

   group->name = strdup(group_name);
   ecore_list_append(group_list, group);

   last = ecore_list_goto_last(group_list);
   group->id = last->id + 1;

   return group->id;
}

/**
 * Destroys a previously created path group
 * @param   group_id The unique identifier for the group.
 * @ingroup Ecore_Path_Group
 */
EAPI void
ecore_path_group_del(int group_id)
{
   Ecore_Path_Group *group;

   group = __ecore_path_group_find_id(group_id);

   if (!group)
     return;

   if (group->paths)
     {
	ecore_list_for_each(group->paths, ECORE_FOR_EACH(free), NULL);
	ecore_list_destroy(group->paths);
     }

   free(group->name);
   free(group);
}

/**
 * Adds a directory to be searched for files.
 * @param   group_id The unique identifier for the group to add the path.
 * @param   path     The new path to be added to the group.
 * @ingroup Ecore_Path_Group
 */
EAPI void
ecore_path_group_add(int group_id, char *path)
{
   Ecore_Path_Group *group;

   CHECK_PARAM_POINTER("path", path);

   group = __ecore_path_group_find_id(group_id);

   if (!group)
     return;

   if (!group->paths)
     group->paths = ecore_list_new();

   ecore_list_append(group->paths, strdup(path));
}

/**
 * Removes the given directory from the given group.
 * @param   group_id The identifier for the given group.
 * @param   path     The path of the directory to be removed.
 * @ingroup Ecore_Path_Group
 */
EAPI void
ecore_path_group_remove(int group_id, char *path)
{
   char *found;
   Ecore_Path_Group *group;

   CHECK_PARAM_POINTER("path", path);

   group = __ecore_path_group_find_id(group_id);

   if (!group || !group->paths)
     return;

   /*
    * Find the path in the list of available paths
    */
   ecore_list_goto_first(group->paths);

   while ((found = ecore_list_current(group->paths)) && strcmp(found, path))
     ecore_list_next(group->paths);

   /*
    * If the path is found, remove and free it
    */
   if (found)
     {
	ecore_list_remove(group->paths);
	free(found);
     }
}

/**
 * Finds a file in a group of paths.
 * @param   group_id The path group id to search.
 * @param   name     The name of the file to find.
 * @return  A pointer to a newly allocated path location of the found file
 *          on success.  @c NULL on failure.
 * @ingroup Ecore_Path_Group
 */
EAPI char *
ecore_path_group_find(int group_id, char *name)
{
   int r;
   char *p;
   struct stat st;
   char path[PATH_MAX];
   Ecore_Path_Group *group;

   CHECK_PARAM_POINTER_RETURN("name", name, NULL);

   group = __ecore_path_group_find_id(group_id);

   /*
    * Search the paths of the path group for the specified file name
    */
   ecore_list_goto_first(group->paths);
   p = ecore_list_next(group->paths);
   do
     {
	snprintf(path, PATH_MAX, "%s/%s", p, name);
	r = stat(path, &st);
     }
   while (((r < 0) || !S_ISREG(st.st_mode)) &&
	  (p = ecore_list_next(group->paths)));

   if (p)
     p = strdup(path);

   return p;
}

/**
 * Retrieves a list of all available files in the given path.
 * @param   group_id The identifier for the given path.
 * @return  A pointer to a newly allocated list of all files found in the paths
 *          identified by @p group_id.  @c NULL otherwise.
 * @ingroup Ecore_Path_Group
 */
EAPI Ecore_List *
ecore_path_group_available(int group_id)
{
   Ecore_List *avail = NULL;
   Ecore_Path_Group *group;
   char *path;

   group = __ecore_path_group_find_id(group_id);

   if (!group || !group->paths || ecore_list_is_empty(group->paths))
     return NULL;

   ecore_list_goto_first(group->paths);

   while ((path = ecore_list_next(group->paths)) != NULL)
     {
	DIR *dir;
	struct stat st;
	struct dirent *d;

	stat(path, &st);

	if (!S_ISDIR(st.st_mode))
	  continue;

	dir = opendir(path);

	if (!dir)
	  continue;

	while ((d = readdir(dir)) != NULL)
	  {
	     char ppath[PATH_MAX];
	     char *ext;
	     char n[PATH_MAX];
	     int l;

	     if (!strncmp(d->d_name, ".", 1))
	       continue;

	     ext = strrchr(d->d_name, '.');

	     if (!ext || strncmp(ext, ".so", 3))
	       continue;

	     snprintf(ppath, PATH_MAX, "%s/%s", path, d->d_name);

	     stat(ppath, &st);

	     if (!S_ISREG(st.st_mode))
	       continue;

	     l = strlen(d->d_name);

	     strncpy(n, d->d_name, l - 2);

	     if (!avail)
	       avail = ecore_list_new();

	     ecore_list_append(avail, strdup(n));
	  }
     }

   return avail;
}

/*
 * Find the specified group name
 */
Ecore_Path_Group *
__ecore_path_group_find(char *name)
{
   Ecore_Path_Group *group;

   CHECK_PARAM_POINTER_RETURN("name", name, NULL);

   ecore_list_goto_first(group_list);

   while ((group = ecore_list_next(group_list)) != NULL)
     if (!strcmp(group->name, name))
       return group;

   return NULL;
}

/*
 * Find the specified group id
 */
Ecore_Path_Group *
__ecore_path_group_find_id(int id)
{
   Ecore_Path_Group *group;

   ecore_list_goto_first(group_list);

   while ((group = ecore_list_next(group_list)) != NULL)
     if (group->id == id)
       return group;

   return NULL;
}
