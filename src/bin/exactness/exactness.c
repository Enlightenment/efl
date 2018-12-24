#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>

#include "exactness_private.h"

#define SCHEDULER_CMD_SIZE 1024

#define ORIG_SUBDIR "orig"
#define CURRENT_SUBDIR "current"

#define EXACTNESS_PATH_MAX 1024

#define BUF_SIZE 1024

typedef struct
{
   EINA_INLIST;
   char *name;
   const char *command;
} List_Entry;

typedef enum
{
   RUN_SIMULATION,
   RUN_PLAY,
   RUN_INIT
} Run_Mode;

static unsigned short _running_jobs = 0, _max_jobs = 1;
static Eina_List *_base_dirs = NULL;
static char *_dest_dir;
static char *_wrap_command = NULL, *_fonts_dir = NULL;
static int _verbose = 0;
static Eina_Bool _scan_objs = EINA_FALSE, _disable_screenshots = EINA_FALSE, _stabilize_shots = EINA_FALSE;

static Run_Mode _mode;
static List_Entry *_next_test_to_run = NULL;
static unsigned int _tests_executed = 0;

static Eina_List *_errors;
static Eina_List *_compare_errors;

static Eina_Bool _job_consume();

static void
_printf(int verbose, const char *fmt, ...)
{
   va_list ap;
   if (!_verbose || verbose > _verbose) return;

   va_start(ap, fmt);
   vprintf(fmt, ap);
   va_end(ap);
}

static Exactness_Image *
_image_load(const char *filename)
{
   int w, h;
   Evas_Load_Error err;
   Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, 100, 100, NULL);
   Eo *e = ecore_evas_get(ee);

   Eo *img = evas_object_image_add(e);
   evas_object_image_file_set(img, filename, NULL);
   err = evas_object_image_load_error_get(img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        fprintf(stderr, "could not load image '%s'. error string is \"%s\"\n",
              filename, evas_load_error_str(err));
        return NULL;
     }

   Exactness_Image *ex_img = malloc(sizeof(*ex_img));
   int len;
   evas_object_image_size_get(img, &w, &h);
   ex_img->w = w;
   ex_img->h = h;
   len = w * h * 4;
   ex_img->pixels = malloc(len);
   memcpy(ex_img->pixels, evas_object_image_data_get(img, EINA_FALSE), len);

   ecore_evas_free(ee);
   return ex_img;
}

static void
_image_save(Exactness_Image *ex_img, const char *output)
{
   Ecore_Evas *ee;
   Eo *e, *img;
   ee = ecore_evas_new(NULL, 0, 0, 100, 100, NULL);
   e = ecore_evas_get(ee);
   img = evas_object_image_add(e);
   evas_object_image_size_set(img, ex_img->w, ex_img->h);
   evas_object_image_data_set(img, ex_img->pixels);
   evas_object_image_save(img, output, NULL, NULL);
   ecore_evas_free(ee);
}

static Eina_Bool
_file_compare(const char *orig_dir, const char *ent_name)
{
   Eina_Bool result = EINA_FALSE;
   Exactness_Image *img1, *img2, *imgO = NULL;
   char *filename1 = alloca(strlen(orig_dir) + strlen(ent_name) + 20);
   char *filename2 = alloca(strlen(_dest_dir) + strlen(ent_name) + 20);
   sprintf(filename1, "%s/%s", orig_dir, ent_name);
   sprintf(filename2, "%s/%s/%s", _dest_dir, CURRENT_SUBDIR, ent_name);

   img1 = _image_load(filename1);
   img2 = _image_load(filename2);

   if (exactness_image_compare(img1, img2, &imgO))
     {
        char *buf = alloca(strlen(_dest_dir) + strlen(ent_name));
        sprintf(buf, "%s/%s/comp_%s", _dest_dir, CURRENT_SUBDIR, ent_name);
        _image_save(imgO, buf);
        _compare_errors = eina_list_append(_compare_errors, strdup(ent_name));
        result = EINA_TRUE;
     }
   exactness_image_free(img1);
   exactness_image_free(img2);
   exactness_image_free(imgO);
   return result;
}

static void
_exu_imgs_unpack(const char *exu_path, const char *dir, const char *ent_name)
{
   Exactness_Unit *unit = exactness_unit_file_read(exu_path);
   Exactness_Image *img;
   Eina_List *itr;
   Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, 100, 100, NULL);
   Eo *e = ecore_evas_get(ee);
   int n = 1;
   if (!unit) return;
   EINA_LIST_FOREACH(unit->imgs, itr, img)
     {
        Eo *o = evas_object_image_add(e);
        char *filename = alloca(strlen(dir) + strlen(ent_name) + 20);
        snprintf(filename, EXACTNESS_PATH_MAX, "%s/%s%c%.3d.png",
              dir, ent_name, SHOT_DELIMITER, n++);
        evas_object_image_size_set(o, img->w, img->h);
        evas_object_image_data_set(o, img->pixels);
        if (!evas_object_image_save(o, filename, NULL, NULL))
          {
             printf("Cannot save widget to <%s>\n", filename);
          }
        efl_del(o);
     }
   efl_del(e);
   ecore_evas_free(ee);
}

static void
_run_test_compare(const List_Entry *ent)
{
   char *path = alloca(EXACTNESS_PATH_MAX);
   char *origdir = alloca(strlen(_dest_dir) + 20);
   const char *base_dir;
   Eina_List *itr;
   int n = 1, nb_fails = 0;
   printf("STATUS %s: COMPARE\n", ent->name);
   EINA_LIST_FOREACH(_base_dirs, itr, base_dir)
     {
        sprintf(path, "%s/%s.exu", base_dir, ent->name);
        if (ecore_file_exists(path))
          {
             char *currentdir;
             sprintf(origdir, "%s/%s/%s", _dest_dir, CURRENT_SUBDIR, ORIG_SUBDIR);
             mkdir(origdir, 0744);
             _exu_imgs_unpack(path, origdir, ent->name);
             sprintf(path, "%s/%s/%s.exu", _dest_dir, CURRENT_SUBDIR, ent->name);
             currentdir = alloca(strlen(_dest_dir) + 20);
             sprintf(currentdir, "%s/%s", _dest_dir, CURRENT_SUBDIR);
             _exu_imgs_unpack(path, currentdir, ent->name);
             goto found;
          }
        else
          {
             sprintf(path, "%s/%s.rec", base_dir, ent->name);
             if (ecore_file_exists(path))
               {
                  sprintf(origdir, "%s/%s", _dest_dir, ORIG_SUBDIR);
                  goto found;
               }
          }
     }
found:
   do
     {
        sprintf(path, "%s/%s%c%.3d.png", origdir, ent->name, SHOT_DELIMITER, n);
        if (ecore_file_exists(path))
          {
             sprintf(path, "%s%c%.3d.png", ent->name, SHOT_DELIMITER, n);
             if (_file_compare(origdir, path)) nb_fails++;
          }
        else break;
        n++;
     } while (EINA_TRUE);
   if (!nb_fails)
      printf("STATUS %s: END - SUCCESS\n", ent->name);
   else
      printf("STATUS %s: END - FAIL (%d/%d)\n", ent->name, nb_fails, n - 1);
}

#define CONFIG "ELM_SCALE=1 ELM_FINGER_SIZE=10 "
static Eina_Bool
_run_command_prepare(const List_Entry *ent, char *buf)
{
   char scn_path[EXACTNESS_PATH_MAX];
   Eina_Strbuf *sbuf;
   const char *base_dir;
   Eina_List *itr;
   Eina_Bool is_exu;
   EINA_LIST_FOREACH(_base_dirs, itr, base_dir)
     {
        is_exu = EINA_TRUE;
        sprintf(scn_path, "%s/%s.exu", base_dir, ent->name);
        if (ecore_file_exists(scn_path)) goto ok;
        else
          {
             is_exu = EINA_FALSE;
             sprintf(scn_path, "%s/%s.rec", base_dir, ent->name);
             if (ecore_file_exists(scn_path)) goto ok;
          }
     }
   fprintf(stderr, "Test %s not found in the provided base directories\n", ent->name);
   return EINA_FALSE;
ok:
   sbuf = eina_strbuf_new();
   printf("STATUS %s: START\n", ent->name);
   eina_strbuf_append_printf(sbuf,
         "%s exactness_play %s %s%s %s%.*s %s%s%s-t '%s' ",
         _wrap_command ? _wrap_command : "",
         _mode == RUN_SIMULATION ? "-s" : "",
         _fonts_dir ? "-f " : "", _fonts_dir ? _fonts_dir : "",
         _verbose ? "-" : "", _verbose, "vvvvvvvvvv",
         _scan_objs ? "--scan-objects " : "",
         _disable_screenshots ? "--disable-screenshots " : "",
         _stabilize_shots ? "--stabilize-shots " : "",
         scn_path
         );
   if (is_exu)
     {
        if (_mode == RUN_PLAY)
           eina_strbuf_append_printf(sbuf, "-o '%s/%s/%s.exu' ", _dest_dir, CURRENT_SUBDIR, ent->name);
        if (_mode == RUN_INIT)
           eina_strbuf_append_printf(sbuf, "-o '%s' ", scn_path);
     }
   else
     {
        if (_mode == RUN_PLAY)
           eina_strbuf_append_printf(sbuf, "-o '%s/%s' ", _dest_dir, CURRENT_SUBDIR);
        if (_mode == RUN_INIT)
           eina_strbuf_append_printf(sbuf, "-o '%s/%s' ", _dest_dir, ORIG_SUBDIR);
     }
   if (ent->command)
     {
        eina_strbuf_append(sbuf, "-- ");
        eina_strbuf_append(sbuf, CONFIG);
        eina_strbuf_append(sbuf, ent->command);
     }
   strncpy(buf, eina_strbuf_string_get(sbuf), SCHEDULER_CMD_SIZE-1);
   eina_strbuf_free(sbuf);
   _printf(1, "Command: %s\n", buf);
   return EINA_TRUE;
}

static void
_job_compare(void *data)
{
   _run_test_compare(data);

   _running_jobs--;
   _job_consume();
   /* If all jobs are done. */
   if (!_running_jobs) ecore_main_loop_quit();
}

static Eina_Bool
_job_deleted_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *msg = (Ecore_Exe_Event_Del *) event;
   List_Entry *ent = ecore_exe_data_get(msg->exe);

   if ((msg->exit_code != 0) || (msg->exit_signal != 0))
    {
        _errors = eina_list_append(_errors, ent);
     }

   if (_mode == RUN_PLAY)
     {
        ecore_job_add(_job_compare, ent);
     }
   else
     {
        _running_jobs--;
        _job_consume();
        if (!_running_jobs) ecore_main_loop_quit();
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_job_consume()
{
   static Ecore_Event_Handler *job_del_callback_handler = NULL;
   char buf[SCHEDULER_CMD_SIZE];
   List_Entry *ent = _next_test_to_run;

   if (_running_jobs == _max_jobs) return EINA_FALSE;
   if (!ent) return EINA_FALSE;

   if (_run_command_prepare(ent, buf))
     {
        _running_jobs++;
        _tests_executed++;

        if (!job_del_callback_handler)
          {
             job_del_callback_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                   _job_deleted_cb, NULL);
          }

        if (!ecore_exe_pipe_run(buf, ECORE_EXE_TERM_WITH_PARENT, ent))
          {
             fprintf(stderr, "Failed executing test '%s'\n", ent->name);
          }
     }
   _next_test_to_run = EINA_INLIST_CONTAINER_GET(
         EINA_INLIST_GET(ent)->next, List_Entry);


   return EINA_TRUE;
}

static void
_scheduler_run()
{
   while (_job_consume());
}

static List_Entry *
_list_file_load(const char *filename)
{
   List_Entry *ret = NULL;
   char buf[BUF_SIZE] = "";
   FILE *file;
   file = fopen(filename, "r");
   if (!file)
     {
        perror("Failed opening list file");
        return NULL;
     }

   while (fgets(buf, BUF_SIZE, file))
     {
        /* Skip comment/empty lines. */
        if ((*buf == '#') || (*buf == '\n') || (!*buf))
           continue;

        char *tmp;
        List_Entry *cur = calloc(1, sizeof(*cur));
        cur->name = strdup(buf);

        /* Set the command to the second half and put a \0 in between. */
        tmp = strchr(cur->name, ' ');
        if (tmp)
          {
             *tmp = '\0';
             cur->command = tmp + 1;
          }
        else
          {
             /* FIXME: error. */
             cur->command = "";
          }

        /* Replace the newline char with a \0. */
        tmp = strchr(cur->command, '\n');
        if (tmp)
          {
             *tmp = '\0';
          }

        ret = EINA_INLIST_CONTAINER_GET(
              eina_inlist_append(EINA_INLIST_GET(ret), EINA_INLIST_GET(cur)),
              List_Entry);
     }

   return ret;
}

static void
_list_file_free(List_Entry *list)
{
   while (list)
     {
        List_Entry *ent = list;
        list = EINA_INLIST_CONTAINER_GET(EINA_INLIST_GET(list)->next,
              List_Entry);

        free(ent->name);
        free(ent);
        /* we don't free ent->command because it's allocated together. */
     }
}

static int
_errors_sort_cb(List_Entry *a, List_Entry *b)
{
   return strcmp(a->name, b->name);
}

static const Ecore_Getopt optdesc = {
  "exactness",
  "%prog [options] <-r|-p|-i|-s> <list file>",
  PACKAGE_VERSION,
  "(C) 2013 Enlightenment",
  "BSD",
  "A pixel perfect test suite for EFL based applications.",
  0,
  {
    ECORE_GETOPT_APPEND('b', "base-dir", "The location of the exu/rec files.", ECORE_GETOPT_TYPE_STR),
    ECORE_GETOPT_STORE_STR('o', "output", "The location of the images."),
    ECORE_GETOPT_STORE_STR('w', "wrap", "Use a custom command to launch the tests (e.g valgrind)."),
    ECORE_GETOPT_STORE_USHORT('j', "jobs", "The number of jobs to run in parallel."),
    ECORE_GETOPT_STORE_TRUE('p', "play", "Run in play mode."),
    ECORE_GETOPT_STORE_TRUE('i', "init", "Run in init mode."),
    ECORE_GETOPT_STORE_TRUE('s', "simulation", "Run in simulation mode."),
    ECORE_GETOPT_STORE_TRUE(0, "scan-objects", "Extract information of all the objects at every shot."),
    ECORE_GETOPT_STORE_TRUE(0, "disable-screenshots", "Disable screenshots."),
    ECORE_GETOPT_STORE_STR('f', "fonts-dir", "Specify a directory of the fonts that should be used."),
    ECORE_GETOPT_STORE_TRUE(0, "stabilize-shots", "Wait for the frames to be stable before taking the shots."),
    ECORE_GETOPT_COUNT('v', "verbose", "Turn verbose messages on."),

    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

int
main(int argc, char *argv[])
{
   int ret = 0;
   List_Entry *test_list;
   int args = 0;
   const char *list_file;
   Eina_List *itr;
   const char *base_dir;
   char tmp[EXACTNESS_PATH_MAX];
   Eina_Bool mode_play = EINA_FALSE, mode_init = EINA_FALSE, mode_simulation = EINA_FALSE;
   Eina_Bool want_quit = EINA_FALSE, scan_objs = EINA_FALSE;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_LIST(_base_dirs),
     ECORE_GETOPT_VALUE_STR(_dest_dir),
     ECORE_GETOPT_VALUE_STR(_wrap_command),
     ECORE_GETOPT_VALUE_USHORT(_max_jobs),
     ECORE_GETOPT_VALUE_BOOL(mode_play),
     ECORE_GETOPT_VALUE_BOOL(mode_init),
     ECORE_GETOPT_VALUE_BOOL(mode_simulation),
     ECORE_GETOPT_VALUE_BOOL(scan_objs),
     ECORE_GETOPT_VALUE_BOOL(_disable_screenshots),
     ECORE_GETOPT_VALUE_STR(_fonts_dir),
     ECORE_GETOPT_VALUE_BOOL(_stabilize_shots),
     ECORE_GETOPT_VALUE_INT(_verbose),

     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_BOOL(want_quit),
     ECORE_GETOPT_VALUE_NONE
   };

   ecore_init();
   ecore_evas_init();
   evas_init();
   mode_play = mode_init = mode_simulation = EINA_FALSE;
   want_quit = EINA_FALSE;
   _dest_dir = "./";
   _scan_objs = scan_objs;

   args = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (args < 0)
     {
        fprintf(stderr, "Failed parsing arguments.\n");
        ret = 1;
        goto end;
     }
   else if (want_quit)
     {
        ret = 1;
        goto end;
     }
   else if (args == argc)
     {
        fprintf(stderr, "Expected test list as the last argument..\n");
        ecore_getopt_help(stderr, &optdesc);
        ret = 1;
        goto end;
     }
   else if (mode_play + mode_init + mode_simulation != 1)
     {
        fprintf(stderr, "At least and only one of the running modes can be set.\n");
        ecore_getopt_help(stderr, &optdesc);
        ret = 1;
        goto end;
     }

   if (!_base_dirs) _base_dirs = eina_list_append(NULL, "./recordings");

   list_file = argv[args];

   /* Load the list file and start iterating over the records. */
   test_list = _list_file_load(list_file);
   _next_test_to_run = test_list;

   if (!test_list)
     {
        fprintf(stderr, "No matching tests found in '%s'\n", list_file);
        ret = 1;
        goto end;
     }

   /* Pre-run summary */
   fprintf(stderr, "Running with settings:\n");
   fprintf(stderr, "\tConcurrent jobs: %d\n", _max_jobs);
   fprintf(stderr, "\tTest list: %s\n", list_file);
   fprintf(stderr, "\tBase dirs:\n");
   EINA_LIST_FOREACH(_base_dirs, itr, base_dir)
      fprintf(stderr, "\t\t%s\n", base_dir);
   fprintf(stderr, "\tDest dir: %s\n", _dest_dir);

   if (mode_play)
     {
        _mode = RUN_PLAY;
        if (snprintf(tmp, EXACTNESS_PATH_MAX, "%s/%s", _dest_dir, CURRENT_SUBDIR)
            >= EXACTNESS_PATH_MAX)
          {
             fprintf(stderr, "Path too long: %s", tmp);
             ret = 1;
             goto end;
          }
        mkdir(tmp, 0744);
     }
   else if (mode_init)
     {
        _mode = RUN_INIT;
        if (snprintf(tmp, EXACTNESS_PATH_MAX, "%s/%s", _dest_dir, ORIG_SUBDIR)
            >= EXACTNESS_PATH_MAX)
          {
             fprintf(stderr, "Path too long: %s", tmp);
             ret = 1;
             goto end;
          }
        mkdir(tmp, 0744);
     }
   else if (mode_simulation)
     {
        _mode = RUN_SIMULATION;
     }
   _scheduler_run();


   ecore_main_loop_begin();

   /* Results */
   printf("*******************************************************\n");
   if (mode_play && EINA_FALSE)
     {
        List_Entry *list_itr;

        EINA_INLIST_FOREACH(test_list, list_itr)
          {
             _run_test_compare(list_itr);
          }
     }

   printf("Finished executing %u out of %u tests.\n",
         _tests_executed,
         eina_inlist_count(EINA_INLIST_GET(test_list)));

   /* Sort the errors and the compare_errors. */
   _errors = eina_list_sort(_errors, 0, (Eina_Compare_Cb) _errors_sort_cb);
   _compare_errors = eina_list_sort(_compare_errors, 0, (Eina_Compare_Cb) strcmp);

   if (_errors || _compare_errors)
     {
        FILE *report_file;
        char report_filename[EXACTNESS_PATH_MAX] = "";
        /* Generate the filename. */
        snprintf(report_filename, EXACTNESS_PATH_MAX,
              "%s/%s/errors.html",
              _dest_dir, mode_init ? ORIG_SUBDIR : CURRENT_SUBDIR);
        report_file = fopen(report_filename, "w+");
        if (report_file)
          {
             printf("%s %p\n", report_filename, report_file);
             fprintf(report_file,
                   "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
                   "<html xmlns=\"http://www.w3.org/1999/xhtml\"><head><title>Exactness report</title></head><body>");

             if (_errors)
               {
                  fprintf(report_file,
                        "<h1>Tests that failed execution:</h1><ul>");
                  List_Entry *ent;
                  printf("List of tests that failed execution:\n");
                  EINA_LIST_FOREACH(_errors, itr, ent)
                    {
                       printf("\t* %s\n", ent->name);

                       fprintf(report_file, "<li>%s</li>", ent->name);
                    }
                  fprintf(report_file, "</ul>");
               }

             if (_compare_errors)
               {
                  fprintf(report_file,
                        "<h1>Images that failed comparison: (Original, Current, Diff)</h1><ul>");
                  char *test_name;
                  printf("List of images that failed comparison:\n");
                  EINA_LIST_FREE(_compare_errors, test_name)
                    {
                       Eina_Bool is_from_exu;
                       char origpath[EXACTNESS_PATH_MAX];
                       snprintf(origpath, EXACTNESS_PATH_MAX, "%s/%s/orig/%s",
                             _dest_dir, CURRENT_SUBDIR, test_name);
                       is_from_exu = ecore_file_exists(origpath);
                       printf("\t* %s\n", test_name);

                       fprintf(report_file, "<li><h2>%s</h2> <img src='%sorig/%s' alt='Original' /> <img src='%s' alt='Current' /> <img src='comp_%s' alt='Diff' /></li>",
                             test_name, is_from_exu ? "" : "../",
                             test_name, test_name, test_name);
                       free(test_name);
                    }
                  fprintf(report_file, "</ul>");
               }
             fprintf(report_file,
                   "</body></html>");

             printf("Report html: %s\n", report_filename);
             ret = 1;
          }
        else
          {
             perror("Failed opening report file");
          }
     }

   _list_file_free(test_list);
end:
   evas_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();

   return ret;
}
