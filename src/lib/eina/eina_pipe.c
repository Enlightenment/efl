
#include <Eina.h>

#ifdef _WIN32
#include <fcntl.h>
#include <evil_private.h>

EINA_API int
eina_pipe_new(int *fds)
{
  HANDLE handles[2];
  if (CreatePipe(&handles[0], &handles[1], NULL, 0))
    {
      fds[0] = _open_osfhandle((intptr_t)handles[0], _O_RDONLY);
      fds[1] = _open_osfhandle((intptr_t)handles[1], _O_APPEND);
      return 0;
    }
  else
    {
      fds[0] = fds[1] = -1;
      return -1;
    }
}

EINA_API ssize_t
eina_pipe_read(int pipe, void* buffer, size_t count)
{
  DWORD read_bytes = 0;
  if (ReadFile((HANDLE)_get_osfhandle(pipe), buffer, count, &read_bytes, NULL))
    {
      return read_bytes;
    }
  else
    {
      return -1;
    }
}

EINA_API ssize_t
eina_pipe_write(int pipe, void const* buffer, size_t count)
{
  DWORD written_bytes = 0;
  if (WriteFile((HANDLE)_get_osfhandle(pipe), buffer, count, &written_bytes, NULL))
    {
      return written_bytes;
    }
  else
    {
      return -1;
    }
}

EINA_API void
eina_pipe_free(int pipe)
{
  _close(pipe);
}
#else
#include <unistd.h>

EINA_API int
eina_pipe_new(int *fds)
{
  return pipe(fds);
}

EINA_API ssize_t
eina_pipe_read(int pipe, void* buffer, size_t count)
{
  return read(pipe, buffer, count);
}

EINA_API ssize_t
eina_pipe_write(int pipe, void const* buffer, size_t count)
{
  return write(pipe, buffer, count);
}

EINA_API void
eina_pipe_free(int pipe)
{
  close(pipe);
}
#endif
