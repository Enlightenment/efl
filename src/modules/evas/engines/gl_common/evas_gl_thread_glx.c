#include "evas_gl_thread.h"


#ifndef GL_GLES

#define EVAS_TH_GLX_FN(ret, name, ...) \
 ret (*GL_TH_FN(name))(GL_TH_DP, ##__VA_ARGS__);
#define EVAS_TH_GLX_FN_ASYNC(ret, name, ...) \
 void *(*GL_TH_FN(name##_begin))(GL_TH_DP, ##__VA_ARGS__); \
 ret (*GL_TH_FN(name##_end))(void *ref);

typedef struct
{
   EVAS_TH_GLX_FN_LIST
} Evas_GL_Thread_GLX_Func;

#undef EVAS_TH_GLX_FN_ASYNC
#undef EVAS_TH_GLX_FN

#define EVAS_TH_GLX_FN(ret, name, ...) \
 typedef ret (*GL_TH_FNTYPE(name))(__VA_ARGS__);
#define EVAS_TH_GLX_FN_ASYNC(ret, name, ...) \
 typedef void *(*GL_TH_FNTYPE(name##_begin))(__VA_ARGS__); \
 typedef ret (*GL_TH_FNTYPE(name##_end))(void *ref);

EVAS_TH_GLX_FN_LIST

#undef EVAS_TH_GLX_FN_ASYNC
#undef EVAS_TH_GLX_FN


#ifdef EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC


/*
   void
   glXBindTexImage(Display *dpy, GLXDrawable drawable, int buffer, const int *attrib_list);
 */
typedef struct
{
   GL_TH_FNTYPE(glXBindTexImage) orig_func;
   Display *dpy;
   GLXDrawable drawable;
   int buffer;
   const int *attrib_list;

} GL_TH_ST(glXBindTexImage);

static void
GL_TH_CB(glXBindTexImage)(void *data)
{
   GL_TH_ST(glXBindTexImage) *thread_data = *(void**)data;

   thread_data->orig_func(thread_data->dpy,
                          thread_data->drawable,
                          thread_data->buffer,
                          thread_data->attrib_list);

}


void
GL_TH_FN(glXBindTexImage)(GL_TH_DP, Display *dpy, GLXDrawable drawable, int buffer, const int *attrib_list)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXBindTexImage) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        ((GL_TH_FNTYPE(glXBindTexImage))orig_func)(dpy, drawable, buffer, attrib_list);
        return;
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXBindTexImage) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->drawable = drawable;
   thread_data->buffer = buffer;
   thread_data->attrib_list = attrib_list;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXBindTexImage),
                              thread_mode);
}

/*
   GLXFBConfig *
   glXChooseFBConfig(Display *dpy, int screen, const int *attribList, int *nitems);
 */

typedef struct
{
   GL_TH_FNTYPE(glXChooseFBConfig) orig_func;
   GLXFBConfig * return_value;
   Display *dpy;
   int screen;
   const int *attribList;
   int *nitems;

} GL_TH_ST(glXChooseFBConfig);

static void
GL_TH_CB(glXChooseFBConfig)(void *data)
{
   GL_TH_ST(glXChooseFBConfig) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->screen,
                             thread_data->attribList,
                             thread_data->nitems);

}

GLXFBConfig *
GL_TH_FN(glXChooseFBConfig)(GL_TH_DP, Display *dpy, int screen, const int *attribList, int *nitems)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXChooseFBConfig) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXChooseFBConfig))orig_func)(dpy, screen, attribList, nitems);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXChooseFBConfig) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->screen = screen;
   thread_data->attribList = attribList;
   thread_data->nitems = nitems;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXChooseFBConfig),
                              thread_mode);

   return thread_data->return_value;
}

/*
   GLXContext
   glXCreateContext(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
 */

typedef struct
{
   GL_TH_FNTYPE(glXCreateContext) orig_func;
   GLXContext return_value;
   Display *dpy;
   XVisualInfo *vis;
   GLXContext shareList;
   Bool direct;

} GL_TH_ST(glXCreateContext);

static void
GL_TH_CB(glXCreateContext)(void *data)
{
   GL_TH_ST(glXCreateContext) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->vis,
                             thread_data->shareList,
                             thread_data->direct);

}

GLXContext
GL_TH_FN(glXCreateContext)(GL_TH_DP, Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXCreateContext) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXCreateContext))orig_func)(dpy, vis, shareList, direct);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXCreateContext) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->vis = vis;
   thread_data->shareList = shareList;
   thread_data->direct = direct;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXCreateContext),
                              thread_mode);

   return thread_data->return_value;
}

/*
   GLXContext
   glXCreateNewContext(Display *dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct);
 */

typedef struct
{
   GL_TH_FNTYPE(glXCreateNewContext) orig_func;
   GLXContext return_value;
   Display *dpy;
   GLXFBConfig config;
   int renderType;
   GLXContext shareList;
   Bool direct;

} GL_TH_ST(glXCreateNewContext);

static void
GL_TH_CB(glXCreateNewContext)(void *data)
{
   GL_TH_ST(glXCreateNewContext) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->config,
                             thread_data->renderType,
                             thread_data->shareList,
                             thread_data->direct);

}

GLXContext
GL_TH_FN(glXCreateNewContext)(GL_TH_DP, Display *dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXCreateNewContext) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXCreateNewContext))orig_func)(dpy, config, renderType, shareList, direct);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXCreateNewContext) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->config = config;
   thread_data->renderType = renderType;
   thread_data->shareList = shareList;
   thread_data->direct = direct;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXCreateNewContext),
                              thread_mode);

   return thread_data->return_value;
}

/*
   GLXPbuffer
   glXCreatePbuffer(Display *dpy, GLXFBConfig config, const int *attribList);
 */

typedef struct
{
   GL_TH_FNTYPE(glXCreatePbuffer) orig_func;
   GLXPbuffer return_value;
   Display *dpy;
   GLXFBConfig config;
   const int *attribList;

} GL_TH_ST(glXCreatePbuffer);

static void
GL_TH_CB(glXCreatePbuffer)(void *data)
{
   GL_TH_ST(glXCreatePbuffer) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->config,
                             thread_data->attribList);

}

GLXPbuffer
GL_TH_FN(glXCreatePbuffer)(GL_TH_DP, Display *dpy, GLXFBConfig config, const int *attribList)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXCreatePbuffer) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXCreatePbuffer))orig_func)(dpy, config, attribList);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXCreatePbuffer) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->config = config;
   thread_data->attribList = attribList;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXCreatePbuffer),
                              thread_mode);

   return thread_data->return_value;
}

/*
   GLXPixmap
   glXCreatePixmap(Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attribList);
 */

typedef struct
{
   GL_TH_FNTYPE(glXCreatePixmap) orig_func;
   GLXPixmap return_value;
   Display *dpy;
   GLXFBConfig config;
   Pixmap pixmap;
   const int *attribList;

} GL_TH_ST(glXCreatePixmap);

static void
GL_TH_CB(glXCreatePixmap)(void *data)
{
   GL_TH_ST(glXCreatePixmap) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->config,
                             thread_data->pixmap,
                             thread_data->attribList);

}

GLXPixmap
GL_TH_FN(glXCreatePixmap)(GL_TH_DP, Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attribList)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXCreatePixmap) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXCreatePixmap))orig_func)(dpy, config, pixmap, attribList);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXCreatePixmap) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->config = config;
   thread_data->pixmap = pixmap;
   thread_data->attribList = attribList;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXCreatePixmap),
                              thread_mode);

   return thread_data->return_value;
}

/*
   GLXWindow
   glXCreateWindow(Display *dpy, GLXFBConfig config, Window win, const int *attribList);
 */

typedef struct
{
   GL_TH_FNTYPE(glXCreateWindow) orig_func;
   GLXWindow return_value;
   Display *dpy;
   GLXFBConfig config;
   Window win;
   const int *attribList;

} GL_TH_ST(glXCreateWindow);

static void
GL_TH_CB(glXCreateWindow)(void *data)
{
   GL_TH_ST(glXCreateWindow) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->config,
                             thread_data->win,
                             thread_data->attribList);

}

GLXWindow
GL_TH_FN(glXCreateWindow)(GL_TH_DP, Display *dpy, GLXFBConfig config, Window win, const int *attribList)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXCreateWindow) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXCreateWindow))orig_func)(dpy, config, win, attribList);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXCreateWindow) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->config = config;
   thread_data->win = win;
   thread_data->attribList = attribList;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXCreateWindow),
                              thread_mode);

   return thread_data->return_value;
}

/*
   void
   glXDestroyPbuffer(Display *dpy, GLXPbuffer pbuf);
 */

typedef struct
{
   GL_TH_FNTYPE(glXDestroyPbuffer) orig_func;
   Display *dpy;
   GLXPbuffer pbuf;

} GL_TH_ST(glXDestroyPbuffer);

static void
GL_TH_CB(glXDestroyPbuffer)(void *data)
{
   GL_TH_ST(glXDestroyPbuffer) *thread_data = *(void **)data;

   thread_data->orig_func(thread_data->dpy,
                          thread_data->pbuf);

}

void
GL_TH_FN(glXDestroyPbuffer)(GL_TH_DP, Display *dpy, GLXPbuffer pbuf)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXDestroyPbuffer) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        ((GL_TH_FNTYPE(glXDestroyPbuffer))orig_func)(dpy, pbuf);
        return;
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXDestroyPbuffer) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->pbuf = pbuf;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXDestroyPbuffer),
                              thread_mode);
}

/*
   void
   glXDestroyPixmap(Display *dpy, GLXPixmap pixmap);
 */

typedef struct
{
   GL_TH_FNTYPE(glXDestroyPixmap) orig_func;
   Display *dpy;
   GLXPixmap pixmap;

} GL_TH_ST(glXDestroyPixmap);

static void
GL_TH_CB(glXDestroyPixmap)(void *data)
{
   GL_TH_ST(glXDestroyPixmap) *thread_data = *(void **)data;

   thread_data->orig_func(thread_data->dpy,
                          thread_data->pixmap);

}

void
GL_TH_FN(glXDestroyPixmap)(GL_TH_DP, Display *dpy, GLXPixmap pixmap)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXDestroyPixmap) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        ((GL_TH_FNTYPE(glXDestroyPixmap))orig_func)(dpy, pixmap);
        return;
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXDestroyPixmap) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->pixmap = pixmap;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXDestroyPixmap),
                              thread_mode);
}

/*
   void
   glXDestroyWindow(Display *dpy, GLXWindow window);
 */

typedef struct
{
   GL_TH_FNTYPE(glXDestroyWindow) orig_func;
   Display *dpy;
   GLXWindow window;

} GL_TH_ST(glXDestroyWindow);

static void
GL_TH_CB(glXDestroyWindow)(void *data)
{
   GL_TH_ST(glXDestroyWindow) *thread_data = *(void **)data;

   thread_data->orig_func(thread_data->dpy,
                          thread_data->window);

}

void
GL_TH_FN(glXDestroyWindow)(GL_TH_DP, Display *dpy, GLXWindow window)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXDestroyWindow) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        ((GL_TH_FNTYPE(glXDestroyWindow))orig_func)(dpy, window);
        return;
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXDestroyWindow) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->window = window;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXDestroyWindow),
                              thread_mode);
}

/*
   int
   glXGetConfig(Display *dpy, XVisualInfo *visual, int attrib, int *value);
 */

typedef struct
{
   GL_TH_FNTYPE(glXGetConfig) orig_func;
   int return_value;
   Display *dpy;
   XVisualInfo *visual;
   int attrib;
   int *value;

} GL_TH_ST(glXGetConfig);

static void
GL_TH_CB(glXGetConfig)(void *data)
{
   GL_TH_ST(glXGetConfig) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->visual,
                             thread_data->attrib,
                             thread_data->value);

}

int
GL_TH_FN(glXGetConfig)(GL_TH_DP, Display *dpy, XVisualInfo *visual, int attrib, int *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXGetConfig) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXGetConfig))orig_func)(dpy, visual, attrib, value);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXGetConfig) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->visual = visual;
   thread_data->attrib = attrib;
   thread_data->value = value;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXGetConfig),
                              thread_mode);

   return thread_data->return_value;
}

/*
   int
   glXGetFBConfigAttrib(Display *dpy, GLXFBConfig config, int attribute, int *value);
 */

typedef struct
{
   GL_TH_FNTYPE(glXGetFBConfigAttrib) orig_func;
   int return_value;
   Display *dpy;
   GLXFBConfig config;
   int attribute;
   int *value;

} GL_TH_ST(glXGetFBConfigAttrib);

static void
GL_TH_CB(glXGetFBConfigAttrib)(void *data)
{
   GL_TH_ST(glXGetFBConfigAttrib) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->config,
                             thread_data->attribute,
                             thread_data->value);

}

int
GL_TH_FN(glXGetFBConfigAttrib)(GL_TH_DP, Display *dpy, GLXFBConfig config, int attribute, int *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXGetFBConfigAttrib) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXGetFBConfigAttrib))orig_func)(dpy, config, attribute, value);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXGetFBConfigAttrib) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->config = config;
   thread_data->attribute = attribute;
   thread_data->value = value;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXGetFBConfigAttrib),
                              thread_mode);

   return thread_data->return_value;
}

/*
   int
   glXGetVideoSync(unsigned int *count);
 */

typedef struct
{
   GL_TH_FNTYPE(glXGetVideoSync) orig_func;
   int return_value;
   unsigned int *count;

} GL_TH_ST(glXGetVideoSync);

static void
GL_TH_CB(glXGetVideoSync)(void *data)
{
   GL_TH_ST(glXGetVideoSync) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->count);

}

int
GL_TH_FN(glXGetVideoSync)(GL_TH_DP, unsigned int *count)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXGetVideoSync) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXGetVideoSync))orig_func)(count);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXGetVideoSync) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->count = count;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXGetVideoSync),
                              thread_mode);

   return thread_data->return_value;
}

/*
   XVisualInfo *
   glXGetVisualFromFBConfig(Display *dpy, GLXFBConfig config);
 */

typedef struct
{
   GL_TH_FNTYPE(glXGetVisualFromFBConfig) orig_func;
   XVisualInfo * return_value;
   Display *dpy;
   GLXFBConfig config;

} GL_TH_ST(glXGetVisualFromFBConfig);

static void
GL_TH_CB(glXGetVisualFromFBConfig)(void *data)
{
   GL_TH_ST(glXGetVisualFromFBConfig) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->config);

}

XVisualInfo *
GL_TH_FN(glXGetVisualFromFBConfig)(GL_TH_DP, Display *dpy, GLXFBConfig config)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXGetVisualFromFBConfig) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXGetVisualFromFBConfig))orig_func)(dpy, config);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXGetVisualFromFBConfig) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->config = config;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXGetVisualFromFBConfig),
                              thread_mode);

   return thread_data->return_value;
}

/*
   Bool
   glXMakeContextCurrent(Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
 */

typedef struct
{
   GL_TH_FNTYPE(glXMakeContextCurrent) orig_func;
   Bool return_value;
   Display *dpy;
   GLXDrawable draw;
   GLXDrawable read;
   GLXContext ctx;

} GL_TH_ST(glXMakeContextCurrent);


Display*    current_thread_dpy  = (Display*)0;
GLXDrawable current_thread_draw = (GLXDrawable)0;
GLXDrawable current_thread_read = (GLXDrawable)0;
GLXContext  current_thread_ctx  = (GLXContext)0;

static void
GL_TH_CB(glXMakeContextCurrent)(void *data)
{
   GL_TH_ST(glXMakeContextCurrent) *thread_data = *(void **)data;

   DBG("THREAD >> OTHER THREAD MAKECONTEXTCURRENT : (%p, %p, %p, %p)\n",
           thread_data->dpy, (void *)thread_data->draw,
           (void* )thread_data->read, thread_data->ctx);

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->draw,
                             thread_data->read,
                             thread_data->ctx);

   if (thread_data->return_value)
     {
        current_thread_dpy = thread_data->dpy;
        current_thread_draw = thread_data->draw;
        current_thread_read = thread_data->read;
        current_thread_ctx = thread_data->ctx;
     }

}

Bool
GL_TH_FN(glXMakeContextCurrent)(GL_TH_DP, Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXMakeContextCurrent) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXMakeContextCurrent))orig_func)(dpy, draw, read, ctx);
     }

   /* Skip for noop make-current */
   if (current_thread_dpy == dpy &&
       current_thread_draw == draw &&
       current_thread_read == read &&
       current_thread_ctx == ctx)
      return EGL_TRUE;

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXMakeContextCurrent) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->draw = draw;
   thread_data->read = read;
   thread_data->ctx = ctx;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXMakeContextCurrent),
                              thread_mode);

   return thread_data->return_value;
}


/*
   Bool
   glXMakeContextCurrentASYNC(Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx);
 */

typedef struct
{
   GL_TH_FNTYPE(glXMakeContextCurrent) orig_func;
   int thread_type;
   Bool return_value;
   Eina_Bool finished;

   Display *dpy;
   GLXDrawable draw;
   GLXDrawable read;
   GLXContext ctx;

} GL_TH_ST(glXMakeContextCurrentASYNC);


static void
GL_TH_CB(glXMakeContextCurrentASYNC)(void *data)
{
   GL_TH_ST(glXMakeContextCurrentASYNC) *thread_data = *(void **)data;

   DBG("THREAD >> OTHER THREAD MAKECONTEXTCURRENT(ASYNC) : (%p, %p, %p, %p)\n",
           thread_data->dpy, (void *)thread_data->draw,
           (void* )thread_data->read, thread_data->ctx);

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->draw,
                             thread_data->read,
                             thread_data->ctx);

   if (thread_data->return_value)
     {
        current_thread_dpy = thread_data->dpy;
        current_thread_draw = thread_data->draw;
        current_thread_read = thread_data->read;
        current_thread_ctx = thread_data->ctx;
     }

   thread_data->finished = EINA_TRUE;

}

void *
GL_TH_FN(glXMakeContextCurrentASYNC_begin)(GL_TH_DP, Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx)
{
   GL_TH_ST(glXMakeContextCurrentASYNC) *thread_data, **thread_data_ptr;
   void *thcmd_ref;

   thread_data = eina_mempool_malloc(_mp_command,
                                     sizeof(GL_TH_ST(glXMakeContextCurrentASYNC)));
   if (!thread_data)
      return NULL;

   if (!evas_gl_thread_enabled(thread_type))
     {
        thread_data->return_value = ((GL_TH_FNTYPE(glXMakeContextCurrent))orig_func)(dpy, draw, read, ctx);
        thread_data->finished = EINA_TRUE;
        return thread_data;
     }

   /* Skip for noop make-current */
   if (current_thread_dpy == dpy &&
       current_thread_draw == draw &&
       current_thread_read == read &&
       current_thread_ctx == ctx)
     {
        thread_data->return_value = True;
        thread_data->finished = EINA_TRUE;
        return thread_data;
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXMakeContextCurrentASYNC) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->draw = draw;
   thread_data->read = read;
   thread_data->ctx = ctx;
   thread_data->finished = EINA_FALSE;
   thread_data->thread_type = thread_type;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXMakeContextCurrentASYNC),
                              EVAS_GL_THREAD_MODE_ASYNC_FINISH);

   return thread_data;
}

Bool
GL_TH_FN(glXMakeContextCurrentASYNC_end)(void *ref)
{
   GL_TH_ST(glXMakeContextCurrentASYNC) *thread_data = (GL_TH_ST(glXMakeContextCurrentASYNC) *)ref;

   if (!thread_data->finished)
      evas_gl_thread_cmd_wait(thread_data->thread_type, thread_data, &thread_data->finished);

   Bool return_value = thread_data->return_value;
   eina_mempool_free(_mp_command, thread_data);

   return return_value;
}


/*
   GLXContext
   glXGetCurrentContext(void);
 */

GLXContext
GL_TH_FN(glXGetCurrentContext)(int thread_type EINA_UNUSED, void *orig_func EINA_UNUSED)
{
   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXGetCurrentContext))orig_func)();
     }

   /* Current context is completely tracked by this variable */
   return current_thread_ctx;
}

/*
   void
   glXDestroyContext(Display *dpy, GLXContext ctx);
 */

typedef struct
{
   GL_TH_FNTYPE(glXDestroyContext) orig_func;
   Display *dpy;
   GLXContext ctx;

} GL_TH_ST(glXDestroyContext);

static void
GL_TH_CB(glXDestroyContext)(void *data)
{
   GL_TH_ST(glXDestroyContext) *thread_data = *(void **)data;

   thread_data->orig_func(thread_data->dpy,
                          thread_data->ctx);

}

void
GL_TH_FN(glXDestroyContext)(GL_TH_DP, Display *dpy, GLXContext ctx)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXDestroyContext) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        ((GL_TH_FNTYPE(glXDestroyContext))orig_func)(dpy, ctx);
        return;
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXDestroyContext) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->ctx = ctx;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXDestroyContext),
                              thread_mode);
}


/*
   int
   glXQueryDrawable(Display *dpy, GLXDrawable draw, int attribute, unsigned int *value);
 */

typedef struct
{
   GL_TH_FNTYPE(glXQueryDrawable) orig_func;
   int return_value;
   Display *dpy;
   GLXDrawable draw;
   int attribute;
   unsigned int *value;

} GL_TH_ST(glXQueryDrawable);

static void
GL_TH_CB(glXQueryDrawable)(void *data)
{
   GL_TH_ST(glXQueryDrawable) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->draw,
                             thread_data->attribute,
                             thread_data->value);

}

int
GL_TH_FN(glXQueryDrawable)(GL_TH_DP, Display *dpy, GLXDrawable draw, int attribute, unsigned int *value)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXQueryDrawable) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXQueryDrawable))orig_func)(dpy, draw, attribute, value);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXQueryDrawable) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->draw = draw;
   thread_data->attribute = attribute;
   thread_data->value = value;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXQueryDrawable),
                              thread_mode);

   return thread_data->return_value;
}

/*
   Bool
   glXQueryExtension(Display *dpy, int *errorb, int *event);
 */

typedef struct
{
   GL_TH_FNTYPE(glXQueryExtension) orig_func;
   Bool return_value;
   Display *dpy;
   int *errorb;
   int *event;

} GL_TH_ST(glXQueryExtension);

static void
GL_TH_CB(glXQueryExtension)(void *data)
{
   GL_TH_ST(glXQueryExtension) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->errorb,
                             thread_data->event);

}

Bool
GL_TH_FN(glXQueryExtension)(GL_TH_DP, Display *dpy, int *errorb, int *event)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXQueryExtension) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXQueryExtension))orig_func)(dpy, errorb, event);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXQueryExtension) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->errorb = errorb;
   thread_data->event = event;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXQueryExtension),
                              thread_mode);

   return thread_data->return_value;
}

/*
   const char *
   glXQueryExtensionsString(Display *dpy, int screen);
 */

typedef struct
{
   GL_TH_FNTYPE(glXQueryExtensionsString) orig_func;
   const char * return_value;
   Display *dpy;
   int screen;

} GL_TH_ST(glXQueryExtensionsString);

static void
GL_TH_CB(glXQueryExtensionsString)(void *data)
{
   GL_TH_ST(glXQueryExtensionsString) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->screen);

}

const char *
GL_TH_FN(glXQueryExtensionsString)(GL_TH_DP, Display *dpy, int screen)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXQueryExtensionsString) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXQueryExtensionsString))orig_func)(dpy, screen);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXQueryExtensionsString) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->screen = screen;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXQueryExtensionsString),
                              thread_mode);

   return thread_data->return_value;
}

/*
   Bool
   glXReleaseBuffersMESA(Display *dpy, GLXDrawable drawable);
 */

typedef struct
{
   GL_TH_FNTYPE(glXReleaseBuffersMESA) orig_func;
   Bool return_value;
   Display *dpy;
   GLXDrawable drawable;

} GL_TH_ST(glXReleaseBuffersMESA);

static void
GL_TH_CB(glXReleaseBuffersMESA)(void *data)
{
   GL_TH_ST(glXReleaseBuffersMESA) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->dpy,
                             thread_data->drawable);

}

Bool
GL_TH_FN(glXReleaseBuffersMESA)(GL_TH_DP, Display *dpy, GLXDrawable drawable)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXReleaseBuffersMESA) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXReleaseBuffersMESA))orig_func)(dpy, drawable);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXReleaseBuffersMESA) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->drawable = drawable;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXReleaseBuffersMESA),
                              thread_mode);

   return thread_data->return_value;
}

/*
   void
   glXReleaseTexImage(Display *dpy, GLXDrawable drawable, int buffer);
 */

typedef struct
{
   GL_TH_FNTYPE(glXReleaseTexImage) orig_func;
   Display *dpy;
   GLXDrawable drawable;
   int buffer;

} GL_TH_ST(glXReleaseTexImage);

static void
GL_TH_CB(glXReleaseTexImage)(void *data)
{
   GL_TH_ST(glXReleaseTexImage) *thread_data = *(void **)data;

   thread_data->orig_func(thread_data->dpy,
                          thread_data->drawable,
                          thread_data->buffer);

}

void
GL_TH_FN(glXReleaseTexImage)(GL_TH_DP, Display *dpy, GLXDrawable drawable, int buffer)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXReleaseTexImage) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        ((GL_TH_FNTYPE(glXReleaseTexImage))orig_func)(dpy, drawable, buffer);
        return;
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXReleaseTexImage) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->drawable = drawable;
   thread_data->buffer = buffer;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXReleaseTexImage),
                              thread_mode);
}

/*
   void
   glXSwapBuffers(Display *dpy, GLXDrawable drawable);
 */

typedef struct
{
   GL_TH_FNTYPE(glXSwapBuffers) orig_func;
   Display *dpy;
   GLXDrawable drawable;

} GL_TH_ST(glXSwapBuffers);

static void
GL_TH_CB(glXSwapBuffers)(void *data)
{
   GL_TH_ST(glXSwapBuffers) *thread_data = *(void **)data;

   thread_data->orig_func(thread_data->dpy,
                          thread_data->drawable);

}

void
GL_TH_FN(glXSwapBuffers)(GL_TH_DP, Display *dpy, GLXDrawable drawable)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXSwapBuffers) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        ((GL_TH_FNTYPE(glXSwapBuffers))orig_func)(dpy, drawable);
        return;
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXSwapBuffers) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->drawable = drawable;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXSwapBuffers),
                              thread_mode);
}

/*
   void
   glXSwapIntervalEXT(Display *dpy, GLXDrawable drawable, int interval);
 */

typedef struct
{
   GL_TH_FNTYPE(glXSwapIntervalEXT) orig_func;
   Display *dpy;
   GLXDrawable drawable;
   int interval;

} GL_TH_ST(glXSwapIntervalEXT);

static void
GL_TH_CB(glXSwapIntervalEXT)(void *data)
{
   GL_TH_ST(glXSwapIntervalEXT) *thread_data = *(void **)data;

   thread_data->orig_func(thread_data->dpy,
                          thread_data->drawable,
                          thread_data->interval);

}

void
GL_TH_FN(glXSwapIntervalEXT)(GL_TH_DP, Display *dpy, GLXDrawable drawable, int interval)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXSwapIntervalEXT) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        ((GL_TH_FNTYPE(glXSwapIntervalEXT))orig_func)(dpy, drawable, interval);
        return;
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXSwapIntervalEXT) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->dpy = dpy;
   thread_data->drawable = drawable;
   thread_data->interval = interval;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXSwapIntervalEXT),
                              thread_mode);
}

/*
   int
   glXSwapIntervalSGI(int interval);
 */

typedef struct
{
   GL_TH_FNTYPE(glXSwapIntervalSGI) orig_func;
   int return_value;
   int interval;

} GL_TH_ST(glXSwapIntervalSGI);

static void
GL_TH_CB(glXSwapIntervalSGI)(void *data)
{
   GL_TH_ST(glXSwapIntervalSGI) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->interval);

}

int
GL_TH_FN(glXSwapIntervalSGI)(GL_TH_DP, int interval)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXSwapIntervalSGI) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXSwapIntervalSGI))orig_func)(interval);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXSwapIntervalSGI) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->interval = interval;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXSwapIntervalSGI),
                              thread_mode);

   return thread_data->return_value;
}

/*
   int
   glXWaitVideoSync(int divisor, int remainder, unsigned int *count);
 */

typedef struct
{
   GL_TH_FNTYPE(glXWaitVideoSync) orig_func;
   int return_value;
   int divisor;
   int remainder;
   unsigned int *count;

} GL_TH_ST(glXWaitVideoSync);

static void
GL_TH_CB(glXWaitVideoSync)(void *data)
{
   GL_TH_ST(glXWaitVideoSync) *thread_data = *(void **)data;

   thread_data->return_value =
      thread_data->orig_func(thread_data->divisor,
                             thread_data->remainder,
                             thread_data->count);

}

int
GL_TH_FN(glXWaitVideoSync)(GL_TH_DP, int divisor, int remainder, unsigned int *count)
{
   int thread_mode = EVAS_GL_THREAD_MODE_FINISH;
   GL_TH_ST(glXWaitVideoSync) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;
   void *thcmd_ref;

   if (!evas_gl_thread_enabled(thread_type))
     {
        return ((GL_TH_FNTYPE(glXWaitVideoSync))orig_func)(divisor, remainder, count);
     }

   thread_data_ptr =
      evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(glXWaitVideoSync) *), &thcmd_ref);
   *thread_data_ptr = thread_data;

   thread_data->divisor = divisor;
   thread_data->remainder = remainder;
   thread_data->count = count;
   thread_data->orig_func = orig_func;

   evas_gl_thread_cmd_enqueue(thcmd_ref,
                              GL_TH_CB(glXWaitVideoSync),
                              thread_mode);

   return thread_data->return_value;
}



/*****************************************************************************/


static Evas_GL_Thread_GLX_Func th_glx_func;
Eina_Bool th_glx_func_initialized = EINA_FALSE;

void *
evas_gl_thread_glx_func_get()
{
   if (!th_glx_func_initialized)
     {
#define THREAD_FUNCTION_ASSIGN(func) th_glx_func.func = func;

#define EVAS_TH_GLX_FN(ret, name, ...) \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name));
#define EVAS_TH_GLX_FN_ASYNC(ret, name, ...) \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name##_begin)); \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name##_end));

   EVAS_TH_GLX_FN_LIST

#undef EVAS_TH_GLX_FN_ASYNC
#undef EVAS_TH_GLX_FN

#undef THREAD_FUNCTION_ASSIGN

        th_glx_func_initialized = EINA_TRUE;
     }

   return &th_glx_func;
}


#else /* ! EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */
      /* compiled for GL backend */

#include <dlfcn.h>

#define EVAS_TH_GLX_FN(ret, name, ...) \
 ret (*GL_TH_FN(name))(GL_TH_DP, ##__VA_ARGS__);
#define EVAS_TH_GLX_FN_ASYNC(ret, name, ...) \
 void *(*GL_TH_FN(name##_begin))(GL_TH_DP, ##__VA_ARGS__); \
 ret (*GL_TH_FN(name##_end))(void *ref);

   EVAS_TH_GLX_FN_LIST

#undef EVAS_TH_GLX_FN_ASYNC
#undef EVAS_TH_GLX_FN

void
_glx_thread_link_init(void *func_ptr)
{
   const Evas_GL_Thread_GLX_Func *th_glx_func = func_ptr;

   if (!th_glx_func)
     {
        ERR("Thread functions (GLX BASE) are not exist");
        return;
     }

#define THREAD_FUNCTION_ASSIGN(func) func = th_glx_func->func;

#define EVAS_TH_GLX_FN(ret, name, ...) \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name));
#define EVAS_TH_GLX_FN_ASYNC(ret, name, ...) \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name##_begin)); \
 THREAD_FUNCTION_ASSIGN(GL_TH_FN(name##_end));

   EVAS_TH_GLX_FN_LIST

#undef EVAS_TH_GLX_FN_ASYNC
#undef EVAS_TH_GLX_FN

#undef THREAD_FUNCTION_ASSIGN
}


#endif /* EVAS_GL_RENDER_THREAD_COMPILE_FOR_GL_GENERIC */

#endif /* ! GL_GLES */
