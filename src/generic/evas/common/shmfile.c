#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#include <time.h>
#ifdef HAVE_SHM_OPEN
# include <sys/mman.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <zlib.h>

#ifdef _WIN32
# include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
HANDLE shm_fd = NULL;
#else
int shm_fd = -1;
#endif
static int shm_size = 0;
void *shm_addr = NULL;
char *shmfile = NULL;

void
shm_alloc(unsigned long dsize)
{
#ifdef _WIN32
   if (!shmfile) shmfile = malloc(1024);
   if (!shmfile) goto failed;
   shmfile[0] = 0;
   srand(time(NULL));
   do
     {
        snprintf(shmfile, 1024, "/evas-loader.%i.%i",
                 (int)getpid(), (int)rand());
        shm_fd = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                   0, dsize, shmfile);
     }
   while ((shm_fd == NULL) || (GetLastError() == ERROR_ALREADY_EXISTS));

   shm_addr = MapViewOfFile(shm_fd, FILE_MAP_WRITE, 0, 0, dsize);
   if (!shm_addr)
     {
        free(shmfile);
        CloseHandle(shm_fd);
        goto failed;
     }
   shm_size = dsize;
   return;
failed:
#elif HAVE_SHM_OPEN
   if (!shmfile) shmfile = malloc(1024);
   if (!shmfile) goto failed;
   shmfile[0] = 0;
   srand(time(NULL));
   do
     {
        snprintf(shmfile, 1024, "/evas-loader.%i.%i",
                 (int)getpid(), (int)rand());
        shm_fd = shm_open(shmfile, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
     }
   while (shm_fd < 0);

   if (ftruncate(shm_fd, dsize) < 0)
     {
        close(shm_fd);
        shm_unlink(shmfile);
        shm_fd = -1;
	goto failed;
     }
   shm_addr = mmap(NULL, dsize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
   if (shm_addr == MAP_FAILED)
     {
        close(shm_fd);
        shm_unlink(shmfile);
        shm_fd = -1;
        goto failed;
     }
   shm_size = dsize;
   return;
failed:
#endif
   shm_addr = malloc(dsize);
}

void
shm_free(void)
{
#ifdef _WIN32
   if (shm_fd)
     {
        UnmapViewOfFile(shm_addr);
        CloseHandle(shm_fd);
        free(shmfile);
        shm_addr = NULL;
        shm_fd = NULL;
        shmfile = NULL;
        return;
     }
#elif HAVE_SHM_OPEN
   if (shm_fd >= 0)
     {
        munmap(shm_addr, shm_size);
        close(shm_fd);
        shm_fd = -1;
        shm_addr = NULL;
        if (shmfile) free(shmfile);
        shmfile = NULL;
        return;
     }
#endif
   free(shm_addr);
   shm_addr = NULL;
#ifdef _WIN32
   shm_fd = NULL;
#else
   shm_fd = -1;
#endif
}

#ifdef __cplusplus
}
#endif
