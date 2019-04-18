#ifndef _ECORE_EXE_EO_LEGACY_H_
#define _ECORE_EXE_EO_LEGACY_H_

#ifndef _ECORE_EXE_EO_CLASS_TYPE
#define _ECORE_EXE_EO_CLASS_TYPE

typedef Eo Ecore_Exe;

#endif

#ifndef _ECORE_EXE_EO_TYPES
#define _ECORE_EXE_EO_TYPES

/** A structure which stores information on lines data from a child process.
 *
 * @ingroup Ecore_Exe_Event_Data
 */
typedef struct _Ecore_Exe_Event_Data_Line
{
  char *line; /**< The bytes of a line of buffered data */
  int size; /**< The size of the line buffer in bytes */
} Ecore_Exe_Event_Data_Line;

/** Ecore exe event data structure
 *
 * @ingroup Ecore_Exe
 */
typedef struct _Ecore_Exe_Event_Data
{
  Efl_Object *exe; /**< The handle to the process. FIXME: should actually be
                    * Ecore.Exe, workaround cyclic */
  void *data; /**< The raw binary data from the child process received */
  int size; /**< The size of this data in bytes */
  Ecore_Exe_Event_Data_Line *lines; /**< An array of line data if line buffered.
                                     * The last one has its line member set to
                                     * @c NULL */
} Ecore_Exe_Event_Data;

/** Flags for executing a child with its stdin and/or stdout piped back.
 *
 * @ingroup Ecore
 */
typedef enum
{
  ECORE_EXE_NONE = 0, /**< No exe flags at all */
  ECORE_EXE_PIPE_READ = 1, /**< Exe Pipe Read mask */
  ECORE_EXE_PIPE_WRITE = 2, /**< Exe Pipe Write mask */
  ECORE_EXE_PIPE_ERROR = 4, /**< Exe Pipe error mask */
  ECORE_EXE_PIPE_READ_LINE_BUFFERED = 8, /**< Reads are buffered until a newline
                                          * and split 1 line per
                                          * Ecore_Exe_Event_Data_Line */
  ECORE_EXE_PIPE_ERROR_LINE_BUFFERED = 16, /**< Errors are buffered until a
                                            * newline and split 1 line per
                                            * Ecore_Exe_Event_Data_Line */
  ECORE_EXE_PIPE_AUTO = 32, /**< stdout and stderr are buffered automatically */
  ECORE_EXE_RESPAWN = 64, /**< FIXME: Exe is restarted if it dies */
  ECORE_EXE_USE_SH = 128, /**< Use /bin/sh to run the command. */
  ECORE_EXE_NOT_LEADER = 256, /**< Do not use setsid() to set the executed
                               * process as its own session leader */
  ECORE_EXE_TERM_WITH_PARENT = 512, /**< Makes child receive SIGTERM when parent
                                     * dies. */
  ECORE_EXE_ISOLATE_IO = 1024 /**< Try and isolate stdin/out and err of the
                               * process so it isn't shared with the parent. */
} Ecore_Exe_Flags;


#endif



#endif
