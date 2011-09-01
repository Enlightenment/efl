#ifndef EMOTION_GENERIC_PLUGIN_H
#define EMOTION_GENERIC_PLUGIN_H

#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEFAULTWIDTH		320
#define DEFAULTHEIGHT		240
#define DEFAULTPITCH		4

typedef enum _Emotion_Generic_Cmd Emotion_Generic_Cmd;
typedef enum _Emotion_Generic_Result Emotion_Generic_Result;
typedef struct _Emotion_Generic_Video_Frame Emotion_Generic_Video_Frame;
typedef struct _Emotion_Generic_Video_Shared Emotion_Generic_Video_Shared;

enum _Emotion_Generic_Cmd
{
   EM_CMD_INIT, // param: shared memory identifier (string)
   EM_CMD_PLAY, // param: position (float)
   EM_CMD_STOP, // param: none
   EM_CMD_FILE_SET, // param: filename (string)
   EM_CMD_FILE_SET_DONE, // param: none
   EM_CMD_FILE_CLOSE, // param: none
   EM_CMD_POSITION_SET, // param: position (float)
   EM_CMD_SPEED_SET, // param: speed (float)
   EM_CMD_AUDIO_MUTE_SET, // param: muted (int)
   EM_CMD_VOLUME_SET, // param: volume (float)
   EM_CMD_AUDIO_TRACK_SET, // param: track id (int)
   EM_CMD_LAST
};

enum _Emotion_Generic_Result
{
   EM_RESULT_INIT, // param: none
   EM_RESULT_FILE_SET, // param: none
   EM_RESULT_FILE_SET_DONE, // param: none
   EM_RESULT_PLAYBACK_STOPPED, // param: none
   EM_RESULT_FILE_CLOSE, // param: none
   EM_RESULT_FRAME_NEW, // param: none
   EM_RESULT_FRAME_SIZE, // param: int, int (width, height)
   EM_RESULT_LENGTH_CHANGED, // param: float
   EM_RESULT_POSITION_CHANGED, // param: float
   EM_RESULT_SEEKABLE_CHANGED, // param: int
   EM_RESULT_AUDIO_TRACK_INFO, // param: current track, track count, track_id, track_name, track_id2, track_name2, ...
			       // (int, int, int, string, int, string, ...)
   EM_RESULT_LAST
};

/* structure for frames 2 buffers to keep integrity */
struct _Emotion_Generic_Video_Frame
{
   unsigned char *frames[3];
};

/* structure for frames 2 buffers to keep integrity */
struct _Emotion_Generic_Video_Shared
{
   int size;
   int width;
   int height;
   int pitch;
   /**
    * - "emotion" is the frame from where the Emotion process is reading pixels.
    *   The player shouldn't touch this frame.
    * - "player" is the frame where the slayer process is writing pixels.
    *   The emotion process shouldn't touch this frame.
    * - "last" is the last frame that was rendered by the player. Emotion will
    *   use this frame the next time it will fetch pixels to Evas.
    * - "next" is the unused frame. The player currently using the "player"
    *   should, after finishing this frame, set "last" to "player", and "player"
    *   to "next", and finally "next" to "last" so this operation can be done
    *   many times in case that Emotion does not request pixels fast enough.
    */
   struct {
	int emotion;
	int player;
	int last;
	int next;
   } frame;
   sem_t lock;
};

inline void
emotion_generic_shm_get(const char *shmname, Emotion_Generic_Video_Shared **vs, Emotion_Generic_Video_Frame *vf)
{
   int shmfd = -1;
   int size;
   Emotion_Generic_Video_Shared *t_vs;

   shmfd = shm_open(shmname, O_RDWR, 0777);

   t_vs = mmap(NULL, sizeof(*t_vs), PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);
   size = t_vs->size;
   munmap(t_vs, sizeof(*t_vs));
   t_vs = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, shmfd, 0);

   vf->frames[0] = (char *)t_vs + sizeof(*t_vs);
   vf->frames[1] = (char *)t_vs + sizeof(*t_vs) + t_vs->height * t_vs->width * t_vs->pitch;
   vf->frames[2] = (char *)t_vs + sizeof(*t_vs) + 2 * t_vs->height * t_vs->width * t_vs->pitch;

   *vs = t_vs;
}

inline void
emotion_generic_shm_free(Emotion_Generic_Video_Shared *vs)
{
   munmap(vs, vs->size);
}

#endif // EMOTION_GENERIC_PLUGIN_H
