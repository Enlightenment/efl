/* ethumbd's slave, driven over its own pipes.
 *
 * No daemon and no bus: the slave is a program that reads commands from its
 * standard input and writes answers to its standard output, so a test can be
 * the thing on the other end of those pipes. What that buys is a test of the
 * reading itself, which is where this went wrong.
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>

#include <Eina.h>
#include <Ecore.h>
#include <signal.h>

#include "ethumb_suite.h"
#include "../../bin/ethumb_client/ethumbd_private.h"

/* A real PNG, seven across and five down, so the slave has something it can
   actually open and scale. */
static const unsigned char real_png[] = {
   0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
   0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x05,
   0x08, 0x02, 0x00, 0x00, 0x00, 0x06, 0xf8, 0x61, 0x8f, 0x00, 0x00, 0x00,
   0x4e, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0x0d, 0xc8, 0x31, 0x01, 0x00,
   0x41, 0x08, 0x04, 0xb1, 0x55, 0x82, 0x08, 0x6a, 0x94, 0x50, 0x23, 0xe2,
   0xca, 0x51, 0xb1, 0x4a, 0x10, 0x81, 0xac, 0xff, 0x94, 0x91, 0x44, 0x88,
   0x12, 0x2d, 0x9e, 0xb0, 0x58, 0x21, 0x25, 0x91, 0x54, 0xd2, 0xc9, 0x4b,
   0x9c, 0x6c, 0xfe, 0x3b, 0xc4, 0x50, 0x43, 0x0f, 0x6f, 0xf0, 0xb0, 0xf3,
   0xaf, 0x09, 0x53, 0xa6, 0xcd, 0x33, 0x36, 0xeb, 0x7f, 0x8f, 0x38, 0xea,
   0xe8, 0xe3, 0x1d, 0x3e, 0xf6, 0xf8, 0x00, 0xf4, 0xdb, 0x2b, 0x7b, 0x89,
   0xa9, 0x1c, 0xd9, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae,
   0x42, 0x60, 0x82
};

/* The wire is native ints and length-prefixed strings, the way the daemon
   writes them. Built up in one buffer so the whole conversation can be
   handed over in a single write. */
typedef struct
{
   unsigned char buf[8192];
   size_t len;
} Wire;

static void
_put(Wire *w, const void *data, size_t len)
{
   ck_assert(w->len + len <= sizeof(w->buf));
   memcpy(w->buf + w->len, data, len);
   w->len += len;
}

static void
_put_int(Wire *w, int v)
{
   _put(w, &v, sizeof(v));
}

static void
_put_str(Wire *w, const char *s)
{
   int len = s ? (int) strlen(s) + 1 : 0;

   _put_int(w, len);
   if (len) _put(w, s, len);
}

/* A setup carrying only what a thumbnail needs: where to put it and how big.
   Every option is a type and a value, and the run ends with FINISHED. */
static void
_put_setup(Wire *w, int idx, const char *dir)
{
   int size = 64;

   _put_int(w, ETHUMBD_OP_SETUP);
   _put_int(w, idx);

   _put_int(w, ETHUMBD_SIZE_W);
   _put_int(w, size);
   _put_int(w, ETHUMBD_SIZE_H);
   _put_int(w, size);

   _put_int(w, ETHUMBD_DIRECTORY);
   _put_str(w, dir);

   _put_int(w, ETHUMBD_SETUP_FINISHED);
}

static void
_put_generate(Wire *w, int idx, const char *file, const char *thumb)
{
   _put_int(w, ETHUMBD_OP_GENERATE);
   _put_int(w, idx);
   _put_str(w, file);
   _put_str(w, NULL);   /* key */
   _put_str(w, thumb);
   _put_str(w, NULL);   /* thumb key */
}

/* Read exactly len bytes, or give up when the clock runs out. Whether the
   slave answers at all is the thing being measured, so this must not wait
   for ever. */
static Eina_Bool
_read_exact(int fd, void *buf, size_t len, double deadline)
{
   unsigned char *p = buf;
   size_t todo = len;

   while (todo)
     {
        struct pollfd pfd = { .fd = fd, .events = POLLIN };
        double left = deadline - ecore_time_unix_get();
        ssize_t r;

        if (left <= 0) return EINA_FALSE;
        if (poll(&pfd, 1, (int) (left * 1000)) <= 0) return EINA_FALSE;

        r = read(fd, p, todo);
        if (r <= 0) return EINA_FALSE;
        p += r;
        todo -= r;
     }

   return EINA_TRUE;
}

static char tmpdir[PATH_MAX];
static char srcfile[PATH_MAX];

static void
_tree_make(void)
{
   FILE *f;

   snprintf(tmpdir, sizeof(tmpdir), "/tmp/ethumb_test_XXXXXX");
   ck_assert_ptr_ne(mkdtemp(tmpdir), NULL);

   snprintf(srcfile, sizeof(srcfile), "%s/pic.png", tmpdir);
   f = fopen(srcfile, "wb");
   ck_assert_ptr_ne(f, NULL);
   ck_assert_int_eq(fwrite(real_png, 1, sizeof(real_png), f), sizeof(real_png));
   fclose(f);
}

static void
_tree_remove(void)
{
   char cmd[PATH_MAX + 16];

   if (!tmpdir[0]) return;
   snprintf(cmd, sizeof(cmd), "rm -rf %s", tmpdir);
   if (system(cmd) != 0) fprintf(stderr, "could not remove %s\n", tmpdir);
   tmpdir[0] = '\0';
}

/* Start the slave with its standard input and output on pipes of ours. */
static pid_t
_slave_start(int *to_slave, int *from_slave)
{
   int in[2], out[2];
   pid_t pid;

   ck_assert_int_eq(pipe(in), 0);
   ck_assert_int_eq(pipe(out), 0);

   pid = fork();
   ck_assert_int_ge(pid, 0);

   if (!pid)
     {
        dup2(in[0], STDIN_FILENO);
        dup2(out[1], STDOUT_FILENO);
        close(in[0]); close(in[1]);
        close(out[0]); close(out[1]);
        execl(ETHUMBD_SLAVE_PATH, ETHUMBD_SLAVE_PATH, (char *) NULL);
        _exit(127);
     }

   close(in[0]);
   close(out[1]);
   *to_slave = in[1];
   *from_slave = out[0];

   return pid;
}

static void
_slave_stop(pid_t pid, int to_slave, int from_slave)
{
   close(to_slave);
   close(from_slave);
   kill(pid, SIGKILL);
   waitpid(pid, NULL, 0);
}

/* Two commands in one write, which is what a second thumbnail asked for
 * while the first is still being asked for looks like by the time it reaches
 * the slave.
 *
 * The slave read its commands through stdio while a file descriptor handler
 * decided when to read, and those two do not agree: a buffered read takes
 * everything the pipe has, so the command behind the one being read ended up
 * somewhere the handler could not see it, and was never read at all. The
 * daemon waited ten seconds and killed the slave for hanging.
 *
 * So what is checked here is not that a thumbnail comes out -- though one
 * does -- but that the answer comes out at all when the setup and the
 * generate arrive together.
 */
EFL_START_TEST(ethumb_slave_reads_a_second_command_in_the_same_write)
{
   int to_slave = -1, from_slave = -1;
   pid_t pid;
   Wire w = { { 0 }, 0 };
   char thumb[PATH_MAX];
   int size_cmd = 0, size_path = 0;
   /* An Eina_Bool on the wire, which is one byte and not four. */
   unsigned char success = 0xff;
   char path[PATH_MAX] = { 0 };
   double deadline;
   struct stat st;

   snprintf(thumb, sizeof(thumb), "%s/thumb.png", tmpdir);

   pid = _slave_start(&to_slave, &from_slave);

   /* Everything the slave needs, handed over at once: make an object, tell
      it what to do, tell it to do it. */
   _put_int(&w, ETHUMBD_OP_NEW);
   _put_int(&w, 0);
   _put_setup(&w, 0, tmpdir);
   _put_generate(&w, 0, srcfile, thumb);

   ck_assert_int_eq(write(to_slave, w.buf, w.len), (ssize_t) w.len);

   /* Generous: the slave has a library to start and an image to scale. What
      it is not allowed to do is never answer. */
   deadline = ecore_time_unix_get() + 30.0;

   ck_assert(_read_exact(from_slave, &size_cmd, sizeof(size_cmd), deadline));
   ck_assert(_read_exact(from_slave, &success, sizeof(success), deadline));
   ck_assert(_read_exact(from_slave, &size_path, sizeof(size_path), deadline));
   ck_assert_int_lt(size_path, (int) sizeof(path));
   if (size_path > 0)
     ck_assert(_read_exact(from_slave, path, size_path, deadline));

   ck_assert_int_eq(success, 1);
   ck_assert_str_eq(path, thumb);
   ck_assert_int_eq(stat(thumb, &st), 0);
   ck_assert_int_gt(st.st_size, 0);

   _slave_stop(pid, to_slave, from_slave);
}
EFL_END_TEST

void
ethumb_test_slave(TCase *tc)
{
   tcase_add_checked_fixture(tc, _tree_make, _tree_remove);
   tcase_add_test(tc, ethumb_slave_reads_a_second_command_in_the_same_write);
}
