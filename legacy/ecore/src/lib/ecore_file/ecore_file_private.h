#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include "config.h"

#include "Ecore.h"

int ecore_file_monitor_init(void);
int ecore_file_monitor_shutdown(void);

/*
#define HAVE_POLL
#define HAVE_FAM
#define HAVE_DNOTIFY
#define HAVE_INOTIFY
*/
