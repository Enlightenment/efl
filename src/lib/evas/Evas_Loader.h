/* EVAS - EFL Scene Graph
 * Copyright (C) 2013 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * What does it means for you ?
 *    Evas Loader are dynamically linked to Evas at run time. You can use anything
 * you want here as long as the license of the module you add to your system match
 * the application you are using it in. Be aware that if you make a module that use
 * a GPL library, it may turn the license of your binary to GPL. Evas doesn't ship
 * with any GPL module.
 */

#ifndef _EVAS_LOADER_H
#define _EVAS_LOADER_H

#include "Eina.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

/* the module api version */
#define EVAS_MODULE_API_VERSION 2

/* the module types */
typedef enum _Evas_Module_Type
{
   EVAS_MODULE_TYPE_ENGINE = 0,
   EVAS_MODULE_TYPE_IMAGE_LOADER = 1,
   EVAS_MODULE_TYPE_IMAGE_SAVER = 2,
   EVAS_MODULE_TYPE_OBJECT = 3
} Evas_Module_Type;

typedef struct _Evas_Module_Api    Evas_Module_Api;
typedef struct _Evas_Module        Evas_Module;
typedef struct _Evas_Module_Path   Evas_Module_Path;
typedef struct _Evas_Module_Engine Evas_Module_Engine;
typedef struct _Evas_Module_Public Evas_Module_Public;

/* the module api structure, all modules should define this struct */
struct _Evas_Module_Api
{
   int			version;
   const char		*name;
   const char		*author;

   struct
     {
	int (*open)(Evas_Module *);
	void (*close)(Evas_Module *);
     } func;
};

/* the module structure */
struct _Evas_Module
{
   const Evas_Module_Api *definition;

   void		*functions;	/* this are the functions exported by the module */
   int           id_engine;	/* some internal data for the module i.e the id for engines */

   int           ref; /* how many refs */
   int           last_used; /* the cycle count when it was last used */

   Eina_Lock     lock;

   unsigned char	loaded : 1;
};

typedef struct _Evas_Image_Load_Opts Evas_Image_Load_Opts;
typedef struct _Evas_Image_Animated  Evas_Image_Animated;
typedef struct _Evas_Image_Property  Evas_Image_Property;
typedef struct _Evas_Image_Load_Func Evas_Image_Load_Func;

typedef enum _Evas_Load_Error
{
   EVAS_LOAD_ERROR_NONE = 0, /**< No error on load */
   EVAS_LOAD_ERROR_GENERIC = 1, /**< A non-specific error occurred */
   EVAS_LOAD_ERROR_DOES_NOT_EXIST = 2, /**< File (or file path) does not exist */
   EVAS_LOAD_ERROR_PERMISSION_DENIED = 3, /**< Permission denied to an existing file (or path) */
   EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED = 4, /**< Allocation of resources failure prevented load */
   EVAS_LOAD_ERROR_CORRUPT_FILE = 5, /**< File corrupt (but was detected as a known format) */
   EVAS_LOAD_ERROR_UNKNOWN_FORMAT = 6 /**< File is not a known format */
} Evas_Load_Error; /**< Evas image load error codes one can get - see evas_load_error_str() too. */

typedef enum _Evas_Image_Animated_Loop_Hint
{
   EVAS_IMAGE_ANIMATED_HINT_NONE = 0,
   EVAS_IMAGE_ANIMATED_HINT_LOOP = 1, /**< Image's animation mode is loop like 1->2->3->1->2->3 */
   EVAS_IMAGE_ANIMATED_HINT_PINGPONG = 2 /**< Image's animation mode is pingpong like 1->2->3->2->1-> ... */
} Evas_Image_Animated_Loop_Hint;

typedef enum _Evas_Image_Scale_Hint
{
   EVAS_IMAGE_SCALE_HINT_NONE = 0, /**< No scale hint at all */
   EVAS_IMAGE_SCALE_HINT_DYNAMIC = 1, /**< Image is being re-scaled over time, thus turning scaling cache @b off for its data */
   EVAS_IMAGE_SCALE_HINT_STATIC = 2 /**< Image is not being re-scaled over time, thus turning scaling cache @b on for its data */
} Evas_Image_Scale_Hint; /**< How an image's data is to be treated by Evas, with regard to scaling cache */

struct _Evas_Image_Property
{
   unsigned int  w;
   unsigned int  h;
   
   unsigned char scale;

   Eina_Bool     rotated;
   Eina_Bool     alpha;
   Eina_Bool     premul;
   Eina_Bool     alpha_sparse;
};

struct _Evas_Image_Animated
{
   Eina_List *frames;

   Evas_Image_Animated_Loop_Hint loop_hint;

   int        frame_count;
   int        loop_count;
   int        cur_frame;

   Eina_Bool  animated;
};

struct _Evas_Image_Load_Opts
{
   struct {
      unsigned int      x, y, w, h;
   } region;
   struct {
      int src_x, src_y, src_w, src_h;
      int dst_w, dst_h;
      int smooth;
      Evas_Image_Scale_Hint scale_hint;
   } scale_load;
   double               dpi; // if > 0.0 use this
   unsigned int         w, h; // if > 0 use this
   unsigned int         degree;//if>0 there is some info related with rotation
   int                  scale_down_by; // if > 1 then use this

   Eina_Bool            orientation; // if EINA_TRUE => should honor orientation information provided by file (like jpeg exif info)
};

struct _Evas_Image_Load_Func
{
  void     *(*file_open) (Eina_File *f, Eina_Stringshare *key,
			  Evas_Image_Load_Opts *opts,
			  Evas_Image_Animated *animated,
			  int *error);
  void     (*file_close) (void *loader_data);

  Eina_Bool (*file_head) (void *loader_data,
			  Evas_Image_Property *prop,
			  int *error);
  Eina_Bool (*file_data) (void *loader_data,
			  Evas_Image_Property *prop,
			  void *pixels, int *error);
  double    (*frame_duration) (void *loader_data,
			       int start, int frame_num);

  Eina_Bool threadable;
  Eina_Bool do_region;
};

EAPI Eina_Bool    evas_module_register   (const Evas_Module_Api *module, Evas_Module_Type type);
EAPI Eina_Bool    evas_module_unregister (const Evas_Module_Api *module, Evas_Module_Type type);

#define EVAS_MODULE_DEFINE(Type, Tn, Name)		\
  Eina_Bool evas_##Tn##_##Name##_init(void)		\
  {							\
     return evas_module_register(&evas_modapi, Type);	\
  }							\
  void evas_##Tn##_##Name##_shutdown(void)		\
  {							\
     evas_module_unregister(&evas_modapi, Type);	\
  }

#define EVAS_EINA_MODULE_DEFINE(Tn, Name)	\
  EINA_MODULE_INIT(evas_##Tn##_##Name##_init);	\
  EINA_MODULE_SHUTDOWN(evas_##Tn##_##Name##_shutdown);

#ifdef __cplusplus
}
#endif

#endif
