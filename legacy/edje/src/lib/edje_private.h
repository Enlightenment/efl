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

#include "Edje_Edit.h"

/* increment this when the EET data descriptors have changed and old
 * EETs cannot be loaded/used correctly anymore.
 */
#define EDJE_FILE_VERSION 1

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


extern Eet_Data_Descriptor *_edje_edd_edje_file;
extern Eet_Data_Descriptor *_edje_edd_edje_data;
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
extern Ecore_Timer     *_edje_timer;
extern Evas_List       *_edje_animators;
extern Edje_Text_Style  _edje_text_styles[EDJE_TEXT_EFFECT_LAST];
extern Evas_List       *_edje_edjes;

void _edje_embryo_script_init     (Edje *ed);
void _edje_embryo_script_shutdown (Edje *ed);
void _edje_embryo_script_reset    (Edje *ed);
void _edje_embryo_test_run        (Edje *ed, char *fname, char *sig, char *src);
    
#endif
