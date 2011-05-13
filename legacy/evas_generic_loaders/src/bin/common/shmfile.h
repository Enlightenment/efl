#ifndef SHMFILE_H
#define SHMFILE_H 1

#ifdef __cplusplus
extern "C" {
#endif
   
extern int   shm_fd;
extern int   shm_size;
extern void *shm_addr;
extern char *shmfile;

void shm_alloc  (int dsize);
void shm_free   (void);

#ifdef __cplusplus
}
#endif

#endif
