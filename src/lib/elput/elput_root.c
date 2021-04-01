#include "elput_private.h"
#include <grp.h>
#include <sys/types.h>
#include <pwd.h>

# ifdef major
#  define MAJOR(x) major(x)
# else
#  define MAJOR(x) ((((x) >> 8) & 0xfff) | (((x) >> 32) & ~0xfff))
# endif

static Eina_Bool
_user_part_of_input(void)
{
   uid_t user;
   struct passwd *user_pw;
   gid_t *gids = NULL;
   int number_of_groups = 0;
   struct group *input_group;

   user = getuid();
   user_pw = getpwuid(user);
   input_group = getgrnam("input");

   EINA_SAFETY_ON_NULL_RETURN_VAL(user_pw, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input_group, EINA_FALSE);

   if (getgrouplist(user_pw->pw_name, getgid(), NULL, &number_of_groups) != -1)
     {
        ERR("Failed to enumerate groups of user");
        return EINA_FALSE;
     }
   number_of_groups ++;
   gids = alloca((number_of_groups) * sizeof(gid_t));
   if (getgrouplist(user_pw->pw_name, getgid(), gids, &number_of_groups) == -1)
     {
        ERR("Failed to get groups of user");
        return EINA_FALSE;
     }

   for (int i = 0; i < number_of_groups; ++i)
     {
        if (gids[i] == input_group->gr_gid)
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_root_connect(Elput_Manager **manager EINA_UNUSED, const char *seat EINA_UNUSED, unsigned int tty EINA_UNUSED)
{
   Elput_Manager *em;

   em = calloc(1, sizeof(Elput_Manager));
   if (!em) return EINA_FALSE;

   em->interface = &_root_interface;
   em->seat = eina_stringshare_add(seat);

   if (!_user_part_of_input())
     {
        free(em);
        return EINA_FALSE;
     }
   *manager = em;
   return EINA_TRUE;
}

static void
_root_disconnect(Elput_Manager *em EINA_UNUSED)
{
   //Nothing to do here, there is no data to free
}

static int
_root_open(Elput_Manager *em EINA_UNUSED, const char *path, int flags)
{
   struct stat st;
   int ret, fd = -1;
   int fl;

   ret = stat(path, &st);
   if (ret < 0) return -1;

   if (!S_ISCHR(st.st_mode)) return -1;

   fd = open(path, flags);
   if (fd < 0) return fd;

   if (MAJOR(st.st_rdev) == 226) //DRM_MAJOR
     em->drm_opens++;

   fl = fcntl(fd, F_GETFL);
   if (fl < 0) goto err;

   if (flags & O_NONBLOCK)
     fl |= O_NONBLOCK;

   ret = fcntl(fd, F_SETFL, fl);
   if (ret < 0) goto err;

   return fd;
err:
   close(fd);
   return -1;
}

static void
_root_open_async(Elput_Manager *em, const char *path, int flags)
{
   int fd, ret;

   fd = _root_open(em, path, flags);
   while (1)
     {
        ret = write(em->input.pipe, &fd, sizeof(int));
        if (ret < 0)
          {
             if ((errno == EAGAIN) || (errno == EINTR))
               continue;
             WRN("Failed to write to input pipe");
          }
        break;
     }
   close(em->input.pipe);
   em->input.pipe = -1;
}

static void
_root_close(Elput_Manager *em EINA_UNUSED, int fd)
{
   close(fd);
}

static Eina_Bool
_root_vt_set(Elput_Manager *em EINA_UNUSED, int vt EINA_UNUSED)
{
   //Nothing to do here
   return EINA_TRUE;
}

Elput_Interface _root_interface =
{
   _root_connect,
   _root_disconnect,
   _root_open,
   _root_open_async,
   _root_close,
   _root_vt_set,
};
