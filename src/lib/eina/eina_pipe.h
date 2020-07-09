#ifndef EINA_PIPE_H
#define EINA_PIPE_H

EINA_API int
eina_pipe_new(int *fds);

EINA_API ssize_t
eina_pipe_read(int pipe, void* buffer, size_t count);

EINA_API ssize_t
eina_pipe_write(int pipe, void const* buffer, size_t count);

EINA_API void
eina_pipe_free(int pipe);

#endif
