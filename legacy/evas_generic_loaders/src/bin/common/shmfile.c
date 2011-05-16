#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <zlib.h>

#ifdef __cplusplus
extern "C" {
#endif
   
int shm_fd = -1;
int shm_size = 0;
void *shm_addr = NULL;
char *shmfile = NULL;

void
shm_alloc(int dsize)
{
#ifdef HAVE_SHM_OPEN
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
#ifdef HAVE_SHM_OPEN
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
   shm_fd = -1;
}

#ifdef __cplusplus
}
#endif
