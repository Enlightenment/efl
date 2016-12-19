/*
 * Coverity Model - not a real source file.
 *
 * See https://scan.coverity.com/tune
 * See https://scan.coverity.com/models
 *
 * Coverity doesn't detect changes to this file, update manually at:
 *
 * https://scan.coverity.com/projects/enlightenment-foundation-libraries?tab=analysis_settings
 */

typedef struct {
   int fd;
} Eo;

/* force Coverity to understand these take ownership of fd */

void efl_loop_fd_set(Eo *obj, int fd)
{
   obj->fd = fd;
}

void efl_loop_fd_file_set(Eo *obj, int fd)
{
   obj->fd = fd;
}
