#ifndef _EDJE_PRIVATE_H
#define _EDJE_PRIVATE_H

#include "config.h"

#include <Evas.h>
#include <Ecore.h>
#ifndef EDJE_FB_ONLY
#include <Ecore_X.h>
#else
#include <Ecore_Fb.h>
#endif
#include <Eet.h>
#include <Embryo.h>

#include <math.h>
#include <fnmatch.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include "Edje_Edit.h"

#ifndef ABS
#define ABS(x) (x < 0 ? -x : x)
#endif

/* increment this when the EET data descriptors have changed and old
 * EETs cannot be loaded/used correctly anymore.
 */
#define EDJE_FILE_VERSION 2

/* FIXME:
 * 
 * more example edje files
 * 
 * ? programs can do multiple actions from one signal
 * ? add containering (hbox, vbox, table, wrapping multi-line hbox & vbox)
 * ? text entry widget (single line only)
 * ? reduce linked list walking and list_nth calls
 * 
 * ? recursions, unsafe callbacks outside edje etc. with freeze, ref/unref and blobk/unblock and break_programs needs to be redesigned & fixed
 * ? all unsafe calls that may result in callbacks must be marked and dealt with
 */

typedef enum _Edje_Queue
{
   EDJE_QUEUE_APP,
     EDJE_QUEUE_SCRIPT
} Edje_Queue;

typedef struct _Edje_Message_Signal Edje_Message_Signal;
typedef struct _Edje_Message        Edje_Message;

struct _Edje_Message_Signal
{
   char *sig;
   char *src;
};

struct _Edje_Message
{
   Edje              *edje;
   Edje_Queue         queue;
   Edje_Message_Type  type;
   int                id;
   unsigned char     *msg;
};

extern Eet_Data_Descriptor *_edje_edd_edje_file;
extern Eet_Data_Descriptor *_edje_edd_edje_data;
extern Eet_Data_Descriptor *_edje_edd_edje_font_directory;
extern Eet_Data_Descriptor *_edje_edd_edje_font_directory_entry;
extern Eet_Data_Descriptor *_edje_edd_edje_image_directory;
extern Eet_Data_Descriptor *_edje_edd_edje_image_directory_entry;
extern Eet_Data_Descriptor *_edje_edd_edje_program;
extern Eet_Data_Descriptor *_edje_edd_edje_program_target;
extern Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory;
extern Eet_Data_Descriptor *_edje_edd_edje_part_collection_directory_entry;
extern Eet_Data_Descriptor *_edje_edd_edje_part_collection;
extern Eet_Data_Descriptor *_edje_edd_edje_part;
extern Eet_Data_Descriptor *_edje_edd_edje_part_description;
extern Eet_Data_Descriptor *_edje_edd_edje_part_image_id;

extern int              _edje_anim_count;
extern Ecore_Animator  *_edje_timer;
extern Evas_List       *_edje_animators;
extern Edje_Text_Style  _edje_text_styles[EDJE_TEXT_EFFECT_LAST];
extern Evas_List       *_edje_edjes;

void          _edje_embryo_script_init      (Edje *ed);
void          _edje_embryo_script_shutdown  (Edje *ed);
void          _edje_embryo_script_reset     (Edje *ed);
void          _edje_embryo_test_run         (Edje *ed, char *fname, char *sig, char *src);
Edje_Var     *_edje_var_new                 (void);
void          _edje_var_free                (Edje_Var *var);
void          _edje_var_init                (Edje *ed);
void          _edje_var_shutdown            (Edje *ed);
int           _edje_var_string_id_get       (Edje *ed, char *string);
int           _edje_var_var_int_get         (Edje *ed, Edje_Var *var);
void          _edje_var_var_int_set         (Edje *ed, Edje_Var *var, int v);
double        _edje_var_var_float_get       (Edje *ed, Edje_Var *var);
void          _edje_var_var_float_set       (Edje *ed, Edje_Var *var, double v);
const char   *_edje_var_var_str_get         (Edje *ed, Edje_Var *var);
void          _edje_var_var_str_set         (Edje *ed, Edje_Var *var, char *str);
int           _edje_var_int_get             (Edje *ed, int id);
void          _edje_var_int_set             (Edje *ed, int id, int v);
double        _edje_var_float_get           (Edje *ed, int id);
void          _edje_var_float_set           (Edje *ed, int id, double v);
const char   *_edje_var_str_get             (Edje *ed, int id);
void          _edje_var_str_set             (Edje *ed, int id, char *str);

void          _edje_var_list_var_append(Edje *ed, int id, Edje_Var *var);
void          _edje_var_list_var_prepend(Edje *ed, int id, Edje_Var *var);
void          _edje_var_list_var_append_relative(Edje *ed, int id, Edje_Var *var, Edje_Var *relative);
void          _edje_var_list_var_prepend_relative(Edje *ed, int id, Edje_Var *var, Edje_Var *relative);
Edje_Var     *_edje_var_list_nth(Edje *ed, int id, int n);

int           _edje_var_list_count_get(Edje *ed, int id);
void          _edje_var_list_remove_nth(Edje *ed, int id, int n);

int           _edje_var_list_nth_int_get(Edje *ed, int id, int n);
void          _edje_var_list_nth_int_set(Edje *ed, int id, int n, int v);
void          _edje_var_list_int_append(Edje *ed, int id, int v);
void          _edje_var_list_int_prepend(Edje *ed, int id, int v);
void          _edje_var_list_int_insert(Edje *ed, int id, int n, int v);

double        _edje_var_list_nth_float_get(Edje *ed, int id, int n);
void          _edje_var_list_nth_float_set(Edje *ed, int id, int n, double v);
void          _edje_var_list_float_append(Edje *ed, int id, double v);
void          _edje_var_list_float_prepend(Edje *ed, int id, double v);
void          _edje_var_list_float_insert(Edje *ed, int id, int n, double v);

const char   *_edje_var_list_nth_str_get(Edje *ed, int id, int n);
void          _edje_var_list_nth_str_set(Edje *ed, int id, int n, char *v);
void          _edje_var_list_str_append(Edje *ed, int id, char *v);
void          _edje_var_list_str_prepend(Edje *ed, int id, char *v);
void          _edje_var_list_str_insert(Edje *ed, int id, int n, char *v);
    
int           _edje_var_timer_add           (Edje *ed, double in, char *fname, int val);
void          _edje_var_timer_del           (Edje *ed, int id);

int           _edje_var_anim_add            (Edje *ed, double len, char *fname, int val);
void          _edje_var_anim_del            (Edje *ed, int id);
    
void          _edje_message_init            (void);
void          _edje_message_shutdown        (void);
void          _edje_message_cb_set          (Edje *ed, void (*func) (void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg), void *data);
Edje_Message *_edje_message_new             (Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id);
void          _edje_message_free            (Edje_Message *em);
void          _edje_message_send            (Edje *ed, Edje_Queue queue, Edje_Message_Type type, int id, void *emsg);
void          _edje_message_process         (Edje_Message *em);
void          _edje_message_queue_process   (void);
void          _edje_message_queue_clear     (void);
void          _edje_message_del             (Edje *ed);
    
#endif
