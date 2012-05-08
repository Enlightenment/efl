#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_SYS_TIMES_H
# include <sys/times.h>
#endif

#ifdef _WIN32
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
#endif

#include <Eio.h>

#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static Elm_Genlist_Item_Class it_eio;

#ifdef _WIN32
ULONGLONG st_time_kernel;
ULONGLONG st_time_user;
ULONGLONG en_time_kernel;
ULONGLONG en_time_user;
#else
static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;
#endif

static void _sel_file(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool _ls_filter_cb(void *data, Eio_File *handler, const char *file);
static void _ls_main_cb(void *data, Eio_File *handler, const char *file);
static void _ls_done_cb(void *data, Eio_File *handler);
static void _ls_error_cb(void *data, Eio_File *handler, int error);
static void _file_chosen(void *data, Evas_Object *obj, void *event_info);
static char *_gl_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part);
static Eina_Bool _gl_state_get(void *data, Evas_Object *obj, const char *part);
static void _gl_del(void *data, Evas_Object *obj);
static void _test_eio_clear(void *data, Evas_Object *obj, void *event);

static void
_sel_file(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
}

static Eina_Bool
_ls_filter_cb(void *data __UNUSED__, Eio_File *handler __UNUSED__, const char *file __UNUSED__)
{
  return EINA_TRUE;
}

static int
_compare_cb(const void *data1, const void *data2)
{
   Elm_Object_Item *it = (Elm_Object_Item *)data1;
   Elm_Object_Item *it2 = (Elm_Object_Item *)data2;
   return strcoll(elm_object_item_data_get(it),
                  elm_object_item_data_get(it2));
}

static void
_ls_main_cb(void *data, Eio_File *handler __UNUSED__, const char *file)
{
   elm_genlist_item_sorted_insert(data,
                                  &it_eio,
                                  eina_stringshare_add(file),
                                  NULL,
                                  ELM_GENLIST_ITEM_NONE,
                                  _compare_cb,
                                  _sel_file,
                                  NULL);
}

static void
_ls_done_cb(void *data __UNUSED__, Eio_File *handler __UNUSED__)
{
#ifdef _WIN32
   FILETIME tc;
   FILETIME te;
   FILETIME tk;
   FILETIME tu;
   ULARGE_INTEGER time_kernel;
   ULARGE_INTEGER time_user;

   if (!GetProcessTimes(GetCurrentProcess(),
                        &tc, &te, &tk, &tu))
     return;

   time_kernel.u.LowPart = tk.dwLowDateTime;
   time_kernel.u.HighPart = tk.dwHighDateTime;
   en_time_kernel = time_kernel.QuadPart;

   time_user.u.LowPart = tu.dwLowDateTime;
   time_user.u.HighPart = tu.dwHighDateTime;
   en_time_user = time_user.QuadPart;

   fprintf(stderr, "ls done\n");
   fprintf(stderr, "Kernel Time: %lld, User Time: %lld",
           (en_time_kernel - st_time_kernel),
           (en_time_user - st_time_user));
#else
   en_time = times(&en_cpu);
   fprintf(stderr, "ls done\n");
   fprintf(stderr, "Real Time: %.jd, User Time: %.jd, System Time: %.jd\n",
           (intmax_t)(en_time - st_time),
           (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
           (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
#endif
}

static void
_ls_error_cb(void *data __UNUSED__, Eio_File *handler __UNUSED__, int error)
{
   fprintf(stderr, "error: [%s]\n", strerror(error));
}

static void
_file_chosen(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   const char *file = event_info;
   if (file)
     {
#ifdef _WIN32
        FILETIME tc;
        FILETIME te;
        FILETIME tk;
        FILETIME tu;
        ULARGE_INTEGER time_kernel;
        ULARGE_INTEGER time_user;

        if (!GetProcessTimes(GetCurrentProcess(),
                             &tc, &te, &tk, &tu))
          return;

        time_kernel.u.LowPart = tk.dwLowDateTime;
        time_kernel.u.HighPart = tk.dwHighDateTime;
        st_time_kernel = time_kernel.QuadPart;

        time_user.u.LowPart = tu.dwLowDateTime;
        time_user.u.HighPart = tu.dwHighDateTime;
        st_time_user = time_user.QuadPart;
#else
        st_time = times(&st_cpu);
#endif
        eio_file_ls(file,
                    _ls_filter_cb,
                    _ls_main_cb,
                    _ls_done_cb,
                    _ls_error_cb,
                    data);
     }
}

static char *
_gl_text_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "Item # %s", (char*)data);
   return strdup(buf);
}

static Evas_Object *
_gl_content_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return NULL;
}

static Eina_Bool
_gl_state_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return EINA_FALSE;
}

static void
_gl_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__)
{
}

static void
_test_eio_clear(void *data, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   elm_genlist_clear(data);
}

void
test_eio(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *vbox, *hbox, *ic, *bt, *fs_bt, *gl;

   it_eio.item_style     = "default";
   it_eio.func.text_get = _gl_text_get;
   it_eio.func.content_get  = _gl_content_get;
   it_eio.func.state_get = _gl_state_get;
   it_eio.func.del       = _gl_del;

   win = elm_win_util_standard_add("fileselector-button", "File Selector Button");
   elm_win_autodel_set(win, EINA_TRUE);

   vbox = elm_box_add(win);
   elm_win_resize_object_add(win, vbox);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(vbox);

   gl = elm_genlist_add(win);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbox, gl);
   evas_object_show(gl);

   /* file selector button */
   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "file");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   fs_bt = elm_fileselector_button_add(win);
   elm_object_text_set(fs_bt, "Select a dir");
   elm_object_part_content_set(fs_bt, "icon", ic);
   elm_fileselector_button_inwin_mode_set(fs_bt, EINA_TRUE);
   elm_fileselector_button_folder_only_set(fs_bt, EINA_TRUE);

   elm_box_pack_end(hbox, fs_bt);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(fs_bt);
   evas_object_show(ic);

   /* attribute setting buttons */
   bt = elm_button_add(win);
   elm_object_text_set(bt, "clear");
   evas_object_smart_callback_add(bt, "clicked", _test_eio_clear, gl);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);
   evas_object_show(hbox);

   evas_object_smart_callback_add(fs_bt, "file,chosen", _file_chosen, gl);

   evas_object_resize(win, 300, 500);
   evas_object_show(win);
}

#endif
