#ifndef _ECORE_DRM2_PRIVATE_H
# define _ECORE_DRM2_PRIVATE_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "Ecore.h"
# include "ecore_private.h"
# include "Eeze.h"
# include "Elput.h"
# include <Ecore_Drm2.h>

# include <unistd.h>
# include <strings.h>
# include <sys/mman.h>
# include <fcntl.h>
# include <ctype.h>

# include <sys/ioctl.h>
# include <dlfcn.h>

extern int _ecore_drm2_log_dom;
extern Eina_Bool _ecore_drm2_use_atomic;

# ifdef ECORE_DRM2_DEFAULT_LOG_COLOR
#  undef ECORE_DRM2_DEFAULT_LOG_COLOR
# endif
# define ECORE_DRM2_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_drm2_log_dom, __VA_ARGS__)

/* The following defines and structures were borrowed from drm.h */

/**
 * \file drm.h
 * Header for the Direct Rendering Manager
 *
 * \author Rickard E. (Rik) Faith <faith@valinux.com>
 *
 * \par Acknowledgments:
 * Dec 1999, Richard Henderson <rth@twiddle.net>, move to generic \c cmpxchg.
 */

/*
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

# define DRM_MODE_FEATURE_DIRTYFB 1

# define DRM_IOCTL_BASE 'd'
# define DRM_IOWR(nr,type) _IOWR(DRM_IOCTL_BASE,nr,type)
# define DRM_IOCTL_MODE_ADDFB2 DRM_IOWR(0xB8, drm_mode_fb_cmd2)

# define DRM_CLIENT_CAP_STEREO_3D 1
# define DRM_CLIENT_CAP_UNIVERSAL_PLANES 2
# define DRM_CLIENT_CAP_ATOMIC 3
# define DRM_CAP_TIMESTAMP_MONOTONIC 0x6

# define DRM_IOCTL_MODE_CREATE_DUMB DRM_IOWR(0xB2, drm_mode_create_dumb)
# define DRM_IOCTL_MODE_MAP_DUMB DRM_IOWR(0xB3, drm_mode_map_dumb)
# define DRM_IOCTL_MODE_DESTROY_DUMB DRM_IOWR(0xB4, drm_mode_destroy_dumb)

/* end drm.h */

/* The following defines and structures were borrowed from drm_mode.h */

/*
 * Copyright (c) 2007 Dave Airlie <airlied@linux.ie>
 * Copyright (c) 2007 Jakob Bornecrantz <wallbraker@gmail.com>
 * Copyright (c) 2008 Red Hat Inc.
 * Copyright (c) 2007-2008 Tungsten Graphics, Inc., Cedar Park, TX., USA
 * Copyright (c) 2007-2008 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

# define DRM_DISPLAY_MODE_LEN 32

# define DRM_MODE_TYPE_BUILTIN (1<<0)
# define DRM_MODE_TYPE_CLOCK_C ((1<<1) | DRM_MODE_TYPE_BUILTIN)
# define DRM_MODE_TYPE_CRTC_C ((1<<2) | DRM_MODE_TYPE_BUILTIN)
# define DRM_MODE_TYPE_PREFERRED (1<<3)
# define DRM_MODE_TYPE_DEFAULT (1<<4)
# define DRM_MODE_TYPE_USERDEF (1<<5)
# define DRM_MODE_TYPE_DRIVER (1<<6)

# define DRM_MODE_PROP_BLOB (1<<4)

# define DRM_MODE_DPMS_ON 0
# define DRM_MODE_DPMS_STANDBY 1
# define DRM_MODE_DPMS_SUSPEND 2
# define DRM_MODE_DPMS_OFF 3

# define DRM_MODE_FLAG_PHSYNC (1<<0)
# define DRM_MODE_FLAG_NHSYNC (1<<1)
# define DRM_MODE_FLAG_PVSYNC (1<<2)
# define DRM_MODE_FLAG_NVSYNC (1<<3)
# define DRM_MODE_FLAG_INTERLACE (1<<4)
# define DRM_MODE_FLAG_DBLSCAN (1<<5)
# define DRM_MODE_FLAG_CSYNC (1<<6)
# define DRM_MODE_FLAG_PCSYNC (1<<7)
# define DRM_MODE_FLAG_NCSYNC (1<<8)
# define DRM_MODE_FLAG_HSKEW (1<<9) /* hskew provided */
# define DRM_MODE_FLAG_BCAST (1<<10)
# define DRM_MODE_FLAG_PIXMUX (1<<11)
# define DRM_MODE_FLAG_DBLCLK (1<<12)
# define DRM_MODE_FLAG_CLKDIV2 (1<<13)
# define DRM_MODE_FLAG_3D_MASK (0x1f<<14)
# define DRM_MODE_FLAG_3D_NONE (0<<14)
# define DRM_MODE_FLAG_3D_FRAME_PACKING (1<<14)
# define DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE (2<<14)
# define DRM_MODE_FLAG_3D_LINE_ALTERNATIVE (3<<14)
# define DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL (4<<14)
# define DRM_MODE_FLAG_3D_L_DEPTH (5<<14)
# define DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH (6<<14)
# define DRM_MODE_FLAG_3D_TOP_AND_BOTTOM (7<<14)
# define DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF (8<<14)

# define DRM_PROP_NAME_LEN 32
# define DRM_CONNECTOR_NAME_LEN 32

# define DRM_MODE_CONNECTOR_Unknown 0
# define DRM_MODE_CONNECTOR_VGA 1
# define DRM_MODE_CONNECTOR_DVII 2
# define DRM_MODE_CONNECTOR_DVID 3
# define DRM_MODE_CONNECTOR_DVIA 4
# define DRM_MODE_CONNECTOR_Composite 5
# define DRM_MODE_CONNECTOR_SVIDEO 6
# define DRM_MODE_CONNECTOR_LVDS 7
# define DRM_MODE_CONNECTOR_Component 8
# define DRM_MODE_CONNECTOR_9PinDIN 9
# define DRM_MODE_CONNECTOR_DisplayPort 10
# define DRM_MODE_CONNECTOR_HDMIA 11
# define DRM_MODE_CONNECTOR_HDMIB 12
# define DRM_MODE_CONNECTOR_TV 13
# define DRM_MODE_CONNECTOR_eDP 14
# define DRM_MODE_CONNECTOR_VIRTUAL 15
# define DRM_MODE_CONNECTOR_DSI 16

# define DRM_MODE_OBJECT_CRTC 0xcccccccc
# define DRM_MODE_OBJECT_CONNECTOR 0xc0c0c0c0
# define DRM_MODE_OBJECT_ENCODER 0xe0e0e0e0
# define DRM_MODE_OBJECT_MODE 0xdededede
# define DRM_MODE_OBJECT_PROPERTY 0xb0b0b0b0
# define DRM_MODE_OBJECT_FB 0xfbfbfbfb
# define DRM_MODE_OBJECT_BLOB 0xbbbbbbbb
# define DRM_MODE_OBJECT_PLANE 0xeeeeeeee
# define DRM_MODE_OBJECT_ANY 0

# define DRM_MODE_PAGE_FLIP_EVENT 0x01
# define DRM_MODE_PAGE_FLIP_ASYNC 0x02
# define DRM_MODE_PAGE_FLIP_FLAGS (DRM_MODE_PAGE_FLIP_EVENT|DRM_MODE_PAGE_FLIP_ASYNC)

#  define DRM_MODE_ATOMIC_TEST_ONLY 0x0100
#  define DRM_MODE_ATOMIC_NONBLOCK  0x0200
#  define DRM_MODE_ATOMIC_ALLOW_MODESET 0x0400

#  define DRM_MODE_ATOMIC_FLAGS \
   (DRM_MODE_PAGE_FLIP_EVENT |\
       DRM_MODE_PAGE_FLIP_ASYNC |\
       DRM_MODE_ATOMIC_TEST_ONLY |\
       DRM_MODE_ATOMIC_NONBLOCK |\
       DRM_MODE_ATOMIC_ALLOW_MODESET)

typedef struct _drm_mode_property_enum
{
   uint64_t value;
   char name[DRM_PROP_NAME_LEN];
} drm_mode_property_enum;

typedef struct _drmModeProperty
{
   uint32_t prop_id;
   uint32_t flags;
   char name[DRM_PROP_NAME_LEN];
   int count_values;
   uint64_t *values; /* store the blob lengths */
   int count_enums;
   drm_mode_property_enum *enums;
   int count_blobs;
   uint32_t *blob_ids; /* store the blob IDs */
} drmModePropertyRes, *drmModePropertyPtr;

typedef struct _drm_mode_fb_cmd2
{
   uint32_t fb_id;
   uint32_t width;
   uint32_t height;
   uint32_t pixel_format; /* fourcc code from drm_fourcc.h */
   uint32_t flags; /* see above flags */

   /*
    * In case of planar formats, this ioctl allows up to 4
    * buffer objects with offsets and pitches per plane.
    * The pitch and offset order is dictated by the fourcc,
    * e.g. NV12 (http://fourcc.org/yuv.php#NV12) is described as:
    *
    * YUV 4:2:0 image with a plane of 8 bit Y samples
    * followed by an interleaved U/V plane containing
    * 8 bit 2x2 subsampled colour difference samples.
    *
    * So it would consist of Y as offsets[0] and UV as
    * offsets[1].  Note that offsets[0] will generally
    * be 0 (but this is not required).
    *
    * To accommodate tiled, compressed, etc formats, a per-plane
    * modifier can be specified.  The default value of zero
    * indicates "native" format as specified by the fourcc.
    * Vendor specific modifier token.  This allows, for example,
    * different tiling/swizzling pattern on different planes.
    * See discussion above of DRM_FORMAT_MOD_xxx.
    */
   uint32_t handles[4];
   uint32_t pitches[4]; /* pitch for each plane */
   uint32_t offsets[4]; /* offset of each plane */
   uint64_t modifier[4]; /* ie, tiling, compressed (per plane) */
} drm_mode_fb_cmd2;

typedef struct _drm_mode_create_dumb
{
   uint32_t height;
   uint32_t width;
   uint32_t bpp;
   uint32_t flags;
   /* handle, pitch, size will be returned */
   uint32_t handle;
   uint32_t pitch;
   uint64_t size;
} drm_mode_create_dumb;

typedef struct _drm_mode_map_dumb
{
   /** Handle for the object being mapped. */
   uint32_t handle;
   uint32_t pad;
   /**
    * Fake offset to use for subsequent mmap call
    *
    * This is a fixed-size type for 32/64 compatibility.
    */
   uint64_t offset;
} drm_mode_map_dumb;

typedef struct _drm_mode_destroy_dumb
{
   uint32_t handle;
} drm_mode_destroy_dumb;

/* end drm_mode.h */

/* The following defines and structures were borrowed from xf86drm.h */

/**
 * \file xf86drm.h
 * OS-independent header for DRM user-level library interface.
 *
 * \author Rickard E. (Rik) Faith <faith@valinux.com>
 */

/*
 * Copyright 1999, 2000 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

# define DRM_EVENT_CONTEXT_VERSION 2

typedef enum
{
   DRM_VBLANK_ABSOLUTE = 0x00000000,
   DRM_VBLANK_RELATIVE = 0x00000001,
   DRM_VBLANK_EVENT = 0x04000000,
   DRM_VBLANK_FLIP = 0x08000000,
   DRM_VBLANK_NEXTONMISS = 0x10000000,
   DRM_VBLANK_SECONDARY = 0x20000000,
   DRM_VBLANK_SIGNAL = 0x40000000
} drmVBlankSeqType;

typedef struct _drmVBlankReq
{
   drmVBlankSeqType type;
   unsigned int sequence;
   unsigned long signal;
} drmVBlankReq;

typedef struct _drmVBlankReply
{
   drmVBlankSeqType type;
   unsigned int sequence;
   long tval_sec;
   long tval_usec;
} drmVBlankReply;

typedef union _drmVBlank
{
   drmVBlankReq request;
   drmVBlankReply reply;
} drmVBlank;

typedef struct _drmEventContext
{
   int version;
   void (*vblank_handler)(int fd,
                          unsigned int sequence,
                          unsigned int tv_sec,
                          unsigned int tv_usec,
                          void *user_data);
   void (*page_flip_handler)(int fd,
                             unsigned int sequence,
                             unsigned int tv_sec,
                             unsigned int tv_usec,
                             void *user_data);
} drmEventContext;

typedef struct _drmVersionBroken
{
   int version_major;
   int version_minor;
   int version_patchlevel;
   size_t name_len;
   // WARNING! this does NOT match the system drm.h headers because
   // literally drm.h is wrong. the below is correct. drm hapily
   // broke its ABI at some point.
   char *name;
   size_t date_len;
   char *date;
   size_t desc_len;
   char *desc;
} drmVersionBroken;

typedef struct _drmVersion
{
   int version_major;
   int version_minor;
   int version_patchlevel;
   int name_len;
   // WARNING! this does NOT match the system drm.h headers because
   // literally drm.h is wrong. the below is correct. drm hapily
   // broke its ABI at some point.
   char *name;
   int date_len;
   char *date;
   int desc_len;
   char *desc;
} drmVersion, *drmVersionPtr;

/* end xf86drm.h */

/* The following defines and structures were borrowed from xf86drmMode.h */

/*
 * \file xf86drmMode.h
 * Header for DRM modesetting interface.
 *
 * \author Jakob Bornecrantz <wallbraker@gmail.com>
 *
 * \par Acknowledgements:
 * Feb 2007, Dave Airlie <airlied@linux.ie>
 */

/*
 * Copyright (c) 2007-2008 Tungsten Graphics, Inc., Cedar Park, Texas.
 * Copyright (c) 2007-2008 Dave Airlie <airlied@linux.ie>
 * Copyright (c) 2007-2008 Jakob Bornecrantz <wallbraker@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

# define DRM_PLANE_TYPE_OVERLAY 0
# define DRM_PLANE_TYPE_PRIMARY 1
# define DRM_PLANE_TYPE_CURSOR  2

# define DRM_MODE_PROP_NAME_LEN 32

typedef enum
{
   DRM_MODE_CONNECTED = 1,
   DRM_MODE_DISCONNECTED = 2,
   DRM_MODE_UNKNOWNCONNECTION = 3
} drmModeConnection;

typedef enum
{
   DRM_MODE_SUBPIXEL_UNKNOWN = 1,
   DRM_MODE_SUBPIXEL_HORIZONTAL_RGB = 2,
   DRM_MODE_SUBPIXEL_HORIZONTAL_BGR = 3,
   DRM_MODE_SUBPIXEL_VERTICAL_RGB = 4,
   DRM_MODE_SUBPIXEL_VERTICAL_BGR = 5,
   DRM_MODE_SUBPIXEL_NONE = 6
} drmModeSubPixel;

typedef struct _drmModePropertyBlob
{
   uint32_t id;
   uint32_t length;
   void *data;
} drmModePropertyBlobRes, *drmModePropertyBlobPtr;

typedef struct _drmModeModeInfo
{
   uint32_t clock;
   uint16_t hdisplay, hsync_start, hsync_end, htotal, hskew;
   uint16_t vdisplay, vsync_start, vsync_end, vtotal, vscan;

   uint32_t vrefresh;

   uint32_t flags;
   uint32_t type;
   char name[DRM_DISPLAY_MODE_LEN];
} drmModeModeInfo, *drmModeModeInfoPtr;

typedef struct _drmModeCrtc
{
   uint32_t crtc_id;
   uint32_t buffer_id; /**< FB id to connect to 0 = disconnect */

   uint32_t x, y; /**< Position on the framebuffer */
   uint32_t width, height;
   int mode_valid;
   drmModeModeInfo mode;

   int gamma_size; /**< Number of gamma stops */
} drmModeCrtc, *drmModeCrtcPtr;

typedef struct _drmModeEncoder
{
   uint32_t encoder_id;
   uint32_t encoder_type;
   uint32_t crtc_id;
   uint32_t possible_crtcs;
   uint32_t possible_clones;
} drmModeEncoder, *drmModeEncoderPtr;

typedef struct _drmModeConnector
{
   uint32_t connector_id;
   uint32_t encoder_id; /**< Encoder currently connected to */
   uint32_t connector_type;
   uint32_t connector_type_id;
   drmModeConnection connection;
   uint32_t mmWidth, mmHeight; /**< HxW in millimeters */
   drmModeSubPixel subpixel;

   int count_modes;
   drmModeModeInfoPtr modes;

   int count_props;
   uint32_t *props; /**< List of property ids */
   uint64_t *prop_values; /**< List of property values */

   int count_encoders;
   uint32_t *encoders; /**< List of encoder ids */
} drmModeConnector, *drmModeConnectorPtr;

typedef struct _drmModeRes
{
   int count_fbs;
   uint32_t *fbs;

   int count_crtcs;
   uint32_t *crtcs;

   int count_connectors;
   uint32_t *connectors;

   int count_encoders;
   uint32_t *encoders;

   uint32_t min_width, max_width;
   uint32_t min_height, max_height;
} drmModeRes, *drmModeResPtr;

typedef struct _drmModeObjectProperties
{
   uint32_t count_props;
   uint32_t *props;
   uint64_t *prop_values;
} drmModeObjectProperties, *drmModeObjectPropertiesPtr;

typedef struct _drmModePlane
{
   uint32_t count_formats;
   uint32_t *formats;
   uint32_t plane_id;

   uint32_t crtc_id;
   uint32_t fb_id;

   uint32_t crtc_x, crtc_y;
   uint32_t x, y;

   uint32_t possible_crtcs;
   uint32_t gamma_size;
} drmModePlane, *drmModePlanePtr;

typedef struct _drmModePlaneRes
{
   uint32_t count_planes;
   uint32_t *planes;
} drmModePlaneRes, *drmModePlaneResPtr;

typedef struct _drmModeClip
{
   unsigned short x1, y1;
   unsigned short x2, y2;
} drmModeClip, *drmModeClipPtr;

# ifdef HAVE_ATOMIC_DRM

typedef struct _drmModeAtomicReqItem
{
   uint32_t object_id;
   uint32_t property_id;
   uint64_t value;
} drmModeAtomicReqItem, *drmModeAtomicReqItemPtr;

typedef struct _drmModeAtomicReq
{
   uint32_t cursor;
   uint32_t size_items;
   drmModeAtomicReqItemPtr items;
} drmModeAtomicReq, *drmModeAtomicReqPtr;

/* end xf86drmMode.h */

typedef struct _Ecore_Drm2_Atomic_State Ecore_Drm2_Atomic_State;

typedef struct _Ecore_Drm2_Atomic_Blob
{
   uint32_t id, value;
   size_t len;
   void *data;
} Ecore_Drm2_Atomic_Blob;

typedef struct _Ecore_Drm2_Atomic_Property
{
   uint32_t id, value;
} Ecore_Drm2_Atomic_Property;

typedef struct _Ecore_Drm2_Connector_State
{
   uint32_t obj_id;
   Ecore_Drm2_Atomic_Property crtc;
   Ecore_Drm2_Atomic_Property dpms;
   Ecore_Drm2_Atomic_Property aspect;
   Ecore_Drm2_Atomic_Property scaling;
   Ecore_Drm2_Atomic_Blob edid;
} Ecore_Drm2_Connector_State;

typedef struct _Ecore_Drm2_Crtc_State
{
   uint32_t obj_id;
   int index;
   Ecore_Drm2_Atomic_Property active;
   Ecore_Drm2_Atomic_Blob mode;
} Ecore_Drm2_Crtc_State;

typedef struct _Ecore_Drm2_Plane_State
{
   uint32_t obj_id, mask;
   Ecore_Drm2_Atomic_Property type;
   Ecore_Drm2_Atomic_Property cid, fid;
   Ecore_Drm2_Atomic_Property sx, sy, sw, sh;
   Ecore_Drm2_Atomic_Property cx, cy, cw, ch;
   Ecore_Drm2_Atomic_Property rotation;

   /* these are not part of an atomic state, but we store these here
    * so that we do not have to refetch properties when iterating planes */
   uint32_t rotation_map[6];
   uint32_t supported_rotations;
} Ecore_Drm2_Plane_State;

struct _Ecore_Drm2_Atomic_State
{
   int crtcs, conns, planes;
   Ecore_Drm2_Crtc_State *crtc_states;
   Ecore_Drm2_Connector_State *conn_states;
   Ecore_Drm2_Plane_State *plane_states;
};
# endif

typedef enum _Ecore_Drm2_Backlight_Type
{
   ECORE_DRM2_BACKLIGHT_RAW,
   ECORE_DRM2_BACKLIGHT_PLATFORM,
   ECORE_DRM2_BACKLIGHT_FIRMWARE
} Ecore_Drm2_Backlight_Type;

typedef enum _Ecore_Drm2_Transform
{
   ECORE_DRM2_TRANSFORM_NORMAL,
   ECORE_DRM2_TRANSFORM_90,
   ECORE_DRM2_TRANSFORM_180,
   ECORE_DRM2_TRANSFORM_270,
   ECORE_DRM2_TRANSFORM_FLIPPED,
   ECORE_DRM2_TRANSFORM_FLIPPED_90,
   ECORE_DRM2_TRANSFORM_FLIPPED_180,
   ECORE_DRM2_TRANSFORM_FLIPPED_270
} Ecore_Drm2_Transform;

typedef enum _Ecore_Drm2_Rotation
{
   ECORE_DRM2_ROTATION_NORMAL = 1,
   ECORE_DRM2_ROTATION_90 = 2,
   ECORE_DRM2_ROTATION_180 = 4,
   ECORE_DRM2_ROTATION_270 = 8,
   ECORE_DRM2_ROTATION_REFLECT_X = 16,
   ECORE_DRM2_ROTATION_REFLECT_Y = 32
} Ecore_Drm2_Rotation;

struct _Ecore_Drm2_Fb
{
   int fd;
   int w, h;
   int depth, bpp;
   uint32_t id, hdl;
   uint32_t stride, size;
   uint32_t format;

   void *gbm_bo;

   Eina_Bool gbm : 1;
   Eina_Bool busy : 1;

   void *mmap;
};

struct _Ecore_Drm2_Output_Mode
{
   uint32_t id;
   uint32_t flags;
   int32_t width, height;
   uint32_t refresh;
   drmModeModeInfo info;
};

struct _Ecore_Drm2_Output
{
   Eina_Stringshare *name;
   Eina_Stringshare *make, *model, *serial;

   int fd;
   int pipe;
   int x, y, w, h, pw, ph;

   uint32_t subpixel;
   uint32_t crtc_id, conn_id, conn_type;
   uint32_t scale;

   struct
     {
        char eisa[13];
        char monitor[13];
        char pnp[5];
        char serial[13];
        unsigned char *blob; // unused when doing atomic
     } edid;

   struct
     {
        const char *path;
        int value, max;
        Ecore_Drm2_Backlight_Type type;
     } backlight;

   drmModeCrtcPtr ocrtc;

   Ecore_Drm2_Fb *current, *next, *pending;

   Eina_Matrix4 matrix, inverse;
   Ecore_Drm2_Transform transform;

   /* unused when doing atomic */
   drmModePropertyPtr dpms;

   Ecore_Drm2_Output_Mode *current_mode;
   Eina_List *modes;

   Eina_List *planes;

   void *user_data;
   Ecore_Drm2_Release_Handler release_cb;
   void *release_data;

# ifdef HAVE_ATOMIC_DRM
   Ecore_Drm2_Crtc_State *crtc_state;
   Ecore_Drm2_Connector_State *conn_state;
   Ecore_Drm2_Plane_State *plane_state;
# endif

   Eina_Bool connected : 1;
   Eina_Bool primary : 1;
   Eina_Bool cloned : 1;
   Eina_Bool enabled : 1;
};

struct _Ecore_Drm2_Device
{
   Elput_Manager *em;

   int fd;
   const char *path;

   int num_crtcs;
   uint32_t *crtcs;

   struct
     {
        uint32_t crtc, conn;
     } alloc;

   struct
     {
        uint32_t width, height;
     } min, max;

   Eeze_Udev_Watch *watch;
   Ecore_Event_Handler *active_hdlr;
   Ecore_Event_Handler *device_change_hdlr;

# ifdef HAVE_ATOMIC_DRM
   Ecore_Drm2_Atomic_State *state;
# endif

   Eina_List *outputs;
};

/* extern int (*sym_drmClose)(int fd); */
/* extern int (*sym_drmWaitVBlank)(int fd, drmVBlank *vbl); */
extern int (*sym_drmHandleEvent)(int fd, drmEventContext *evctx);
extern void *(*sym_drmGetVersion)(int fd);
extern void (*sym_drmFreeVersion)(void *drmver);
extern void *(*sym_drmModeGetProperty)(int fd, uint32_t propertyId);
extern void (*sym_drmModeFreeProperty)(drmModePropertyPtr ptr);
extern void *(*sym_drmModeGetPropertyBlob)(int fd, uint32_t blob_id);
extern void (*sym_drmModeFreePropertyBlob)(drmModePropertyBlobPtr ptr);
extern int (*sym_drmModeDestroyPropertyBlob)(int fd, uint32_t id);
extern int (*sym_drmIoctl)(int fd, unsigned long request, void *arg);
extern void *(*sym_drmModeObjectGetProperties)(int fd, uint32_t object_id, uint32_t object_type);
extern void (*sym_drmModeFreeObjectProperties)(drmModeObjectPropertiesPtr ptr);
extern int (*sym_drmModeCreatePropertyBlob)(int fd, const void *data, size_t size, uint32_t *id);
# ifdef HAVE_ATOMIC_DRM
extern void *(*sym_drmModeAtomicAlloc)(void);
extern void (*sym_drmModeAtomicFree)(drmModeAtomicReqPtr req);
extern int (*sym_drmModeAtomicAddProperty)(drmModeAtomicReqPtr req, uint32_t object_id, uint32_t property_id, uint64_t value);
extern int (*sym_drmModeAtomicCommit)(int fd, drmModeAtomicReqPtr req, uint32_t flags, void *user_data);
extern void (*sym_drmModeAtomicSetCursor)(drmModeAtomicReqPtr req, int cursor);
# endif
extern void *(*sym_drmModeGetEncoder)(int fd, uint32_t encoder_id);
extern void (*sym_drmModeFreeEncoder)(drmModeEncoderPtr ptr);
extern void *(*sym_drmModeGetCrtc)(int fd, uint32_t crtcId);
extern void (*sym_drmModeFreeCrtc)(drmModeCrtcPtr ptr);
extern int (*sym_drmModeSetCrtc)(int fd, uint32_t crtcId, uint32_t bufferId, uint32_t x, uint32_t y, uint32_t *connectors, int count, drmModeModeInfoPtr mode);
extern void *(*sym_drmModeGetResources)(int fd);
extern void (*sym_drmModeFreeResources)(drmModeResPtr ptr);
extern void *(*sym_drmModeGetConnector)(int fd, uint32_t connectorId);
extern void (*sym_drmModeFreeConnector)(drmModeConnectorPtr ptr);
extern int (*sym_drmModeConnectorSetProperty)(int fd, uint32_t connector_id, uint32_t property_id, uint64_t value);
extern int (*sym_drmGetCap)(int fd, uint64_t capability, uint64_t *value);
extern int (*sym_drmSetClientCap)(int fd, uint64_t capability, uint64_t value);
extern void *(*sym_drmModeGetPlaneResources)(int fd);
extern void (*sym_drmModeFreePlaneResources)(drmModePlaneResPtr ptr);
extern void *(*sym_drmModeGetPlane)(int fd, uint32_t plane_id);
extern void (*sym_drmModeFreePlane)(drmModePlanePtr ptr);
extern int (*sym_drmModeAddFB)(int fd, uint32_t width, uint32_t height, uint8_t depth, uint8_t bpp, uint32_t pitch, uint32_t bo_handle, uint32_t *buf_id);
extern int (*sym_drmModeRmFB)(int fd, uint32_t bufferId);
extern int (*sym_drmModePageFlip)(int fd, uint32_t crtc_id, uint32_t fb_id, uint32_t flags, void *user_data);
extern int (*sym_drmModeDirtyFB)(int fd, uint32_t bufferId, drmModeClipPtr clips, uint32_t num_clips);

#endif
