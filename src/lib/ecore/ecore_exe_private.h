typedef enum
{
   ECORE_EXE_WIN32_SIGINT,
   ECORE_EXE_WIN32_SIGQUIT,
   ECORE_EXE_WIN32_SIGTERM,
   ECORE_EXE_WIN32_SIGKILL
} Ecore_Exe_Win32_Signal;

/* FIXME: Getting respawn to work
 *
 * There is no way that we can do anything about the internal state info of
 * an external exe.  The same can be said about the state of user code.  User
 * code in this context means the code that is using ecore_exe to manage exe's
 * for it.
 *
 * Document that the exe must be respawnable, in other words, there is no
 * state that it cannot regenerate by just killing it and starting it again.
 * This includes state that the user code knows about, as the respawn is
 * transparent to that code.  On the other hand, maybe a respawn event might
 * be useful, or maybe resend the currently non existent add event.  For
 * consistancy with ecore_con, an add event is good anyway.
 *
 * The Ecore_exe structure is reused for respawning, so that the (opaque)
 * pointer held by the user remains valid.  This means that the Ecore_Exe
 * init and del functions may need to be split into two parts each to avoid
 * duplicating code - common code part, and the rest.  This implies that
 * the unchanging members mentioned next should NEVER change.
 *
 * These structure members don't need to change -
 *   __list_data       - we stay on the list
 *   data              - passed in originally
 *   cmd               - passed in originally
 *   flags             - passed in originally
 *
 * These structure members need to change -
 *   tag               - state that must be regenerated, zap it
 *   pid               - it will be different
 *   child_fd_write    - it will be different
 *   child_fd_read     - it will be different
 *   child_fd_error    - it will be different
 *   write_fd_handler  - we cannot change the fd used by a handler, this changes coz the fd changes.
 *   read_fd_handler   - we cannot change the fd used by a handler, this changes coz the fd changes.
 *   error_fd_handler  - we cannot change the fd used by a handler, this changes coz the fd changes.
 *
 * Hmm, the read, write, and error buffers could be tricky.
 * They are not atomic, and could be in a semi complete state.
 * They fall into the "state must be regenerated" mentioned above.
 * A respawn/add event should take care of it.
 *
 * These structure members need to change -
 *   write_data_buf    - state that must be regenerated, zap it
 *   write_data_size   - state that must be regenerated, zap it
 *   write_data_offset - state that must be regenerated, zap it
 *   read_data_buf     - state that must be regenerated, zap it
 *   read_data_size    - state that must be regenerated, zap it
 *   error_data_buf    - state that must be regenerated, zap it
 *   error_data_size   - state that must be regenerated, zap it
 *   close_write       - state that must be regenerated, zap it
 *
 * There is the problem that an exe that fell over and needs respawning
 * might keep falling over, keep needing to be respawned, and tie up system
 * resources with the constant respawning.  An exponentially increasing
 * timeout (with maximum timeout) between respawns should take care of that.
 * Although this is not a "contention for a resource" problem, the exe falling
 * over may be, so a random element added to the timeout may help, and won't
 * hurt.  The user code may need to be informed that a timeout is in progress.
 */

struct _Ecore_Exe_Data
{
   pid_t             pid;
   void             *data;
   char             *tag, *cmd;
   Ecore_Exe_Flags   flags;

#ifdef _WIN32
   HANDLE process;
   HANDLE process_thread;
   DWORD process_id;
   DWORD thread_id;
   Ecore_Win32_Handler *h_close;
   Ecore_Exe_Win32_Signal sig;

   struct
   {
      HANDLE child_pipe;
      HANDLE child_pipe_x;
      HANDLE thread;
      void *data_buf;
      DWORD data_size;
   } pipe_read;

   struct
   {
      HANDLE child_pipe;
      HANDLE child_pipe_x;
      void *data_buf;
      int data_size;
   } pipe_write;

   struct
   {
      HANDLE child_pipe;
      HANDLE child_pipe_x;
      HANDLE thread;
      void *data_buf;
      DWORD data_size;
   } pipe_error;

   Eina_Bool close_threads : 1;
   Eina_Bool is_suspended : 1;
#else
   Ecore_Fd_Handler *write_fd_handler; /* the fd_handler to handle write to child - if this was used, or NULL if not */
   Ecore_Fd_Handler *read_fd_handler; /* the fd_handler to handle read from child - if this was used, or NULL if not */
   Ecore_Fd_Handler *error_fd_handler; /* the fd_handler to handle errors from child - if this was used, or NULL if not */
   void             *write_data_buf; /* a data buffer for data to write to the child -
                                      * realloced as needed for more data and flushed when the fd handler says writes are possible
                                      */
   int               write_data_size; /* the size in bytes of the data buffer */
   int               write_data_offset; /* the offset in bytes in the data buffer */
   void             *read_data_buf; /* data read from the child awating delivery to an event */
   int               read_data_size; /* data read from child in bytes */
   void             *error_data_buf; /* errors read from the child awating delivery to an event */
   int               error_data_size; /* errors read from child in bytes */
   int               child_fd_write; /* fd to write TO to send data to the child */
   int               child_fd_read; /* fd to read FROM when child has sent us (the parent) data */
   int               child_fd_error; /* fd to read FROM when child has sent us (the parent) errors */
   int               child_fd_write_x; /* fd to write TO to send data to the child */
   int               child_fd_read_x; /* fd to read FROM when child has sent us (the parent) data */
   int               child_fd_error_x; /* fd to read FROM when child has sent us (the parent) errors */

   int               start_bytes, end_bytes, start_lines, end_lines; /* Number of bytes/lines to auto pipe at start/end of stdout/stderr. */

   Ecore_Timer      *doomsday_clock; /* The Timer of Death.  Muahahahaha. */
   void             *doomsday_clock_dead; /* data for the doomsday clock */
#endif

   Ecore_Exe_Cb      pre_free_cb;
   Eina_Bool         close_stdin : 1;
};

typedef struct _Ecore_Exe_Data Ecore_Exe_Data;

EAPI extern int ECORE_EXE_EVENT_ADD;
EAPI extern int ECORE_EXE_EVENT_DEL;
EAPI extern int ECORE_EXE_EVENT_DATA;
EAPI extern int ECORE_EXE_EVENT_ERROR;
extern Eina_List *_ecore_exe_exes;

Ecore_Exe *_ecore_exe_find(pid_t pid);
void *_ecore_exe_event_del_new(void);
void _ecore_exe_event_del_free(void *data EINA_UNUSED, void *ev);
void _ecore_exe_event_exe_data_free(void *data EINA_UNUSED, void *ev);
Ecore_Exe_Event_Add * _ecore_exe_event_add_new(void);
void _ecore_exe_event_add_free(void *data EINA_UNUSED, void *ev);

void _impl_ecore_exe_run_priority_set(int pri);
int _impl_ecore_exe_run_priority_get(void);
void _impl_ecore_exe_auto_limits_set(Ecore_Exe *obj, Ecore_Exe_Data *exe, int start_bytes, int end_bytes, int start_lines, int end_lines);
Eo *_impl_ecore_exe_eo_base_finalize(Eo *obj, Ecore_Exe_Data *exe);
void _impl_ecore_exe_efl_control_suspend_set(Eo *obj EINA_UNUSED, Ecore_Exe_Data *exe, Eina_Bool suspend);
Eina_Bool _impl_ecore_exe_send(Ecore_Exe *obj, Ecore_Exe_Data *exe, const void *data, int size);
Ecore_Exe_Event_Data *_impl_ecore_exe_event_data_get(Ecore_Exe *obj, Ecore_Exe_Data *exe, Ecore_Exe_Flags flags);
void _impl_ecore_exe_eo_base_destructor(Eo *obj, Ecore_Exe_Data *exe);
void _impl_ecore_exe_pause(Ecore_Exe *obj, Ecore_Exe_Data *exe);
void _impl_ecore_exe_continue(Ecore_Exe *obj, Ecore_Exe_Data *exe);
void _impl_ecore_exe_interrupt(Ecore_Exe *obj, Ecore_Exe_Data *exe);
void _impl_ecore_exe_quit(Ecore_Exe *obj, Ecore_Exe_Data *exe);
void _impl_ecore_exe_terminate(Ecore_Exe *obj, Ecore_Exe_Data *exe);
void _impl_ecore_exe_kill(Ecore_Exe *obj, Ecore_Exe_Data *exe);
void _impl_ecore_exe_signal(Ecore_Exe *obj, Ecore_Exe_Data *exe, int num);
void _impl_ecore_exe_hup(Ecore_Exe *obj EINA_UNUSED, Ecore_Exe_Data *exe);
