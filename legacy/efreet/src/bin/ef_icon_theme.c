#include "Efreet.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Ecore_File.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define SIZE 128
#define THEME "Tango"
#define FREE(x) do { free(x); x = NULL; } while (0);

static Eina_Bool _hash_keys(Eina_Hash *hash, const char *key, void *list);
static void ef_icon_theme_themes_find(const char *search_dir,
                                        Eina_Hash *themes);
static void ef_icons_find(Efreet_Icon_Theme *theme, Eina_List *themes,
                                                    Eina_Hash *icons);
static void ef_read_dir(const char *dir, Eina_Hash *icons);

int
ef_cb_efreet_icon_theme(void)
{
    int ret = 1;
    const char *tmp;

    unsetenv("XDG_DATA_HOME");
    efreet_shutdown();
    putenv("HOME=/var/tmp");
    efreet_init();

    tmp = efreet_icon_user_dir_get();
    if (strcmp(tmp, "/var/tmp/.local/share/icons"))
    {
        printf("efreet_icon_user_dir_get() returned incorrect "
                "value (%s) on HOME=/var/tmp\n", tmp);
        ret = 0;
    }

    efreet_shutdown();
    unsetenv("HOME");
    efreet_init();

    tmp = efreet_icon_user_dir_get();
    if (strcmp(tmp, "/tmp/.local/share/icons"))
    {
        printf("efreet_icon_user_dir_get() returned incorrect "
                "value (%s) on HOME=\n", tmp);
        ret = 0;
    }

    return ret;
}

static Eina_Bool
_hash_keys(Eina_Hash *hash __UNUSED__, const char *key, void *list)
{
  Eina_List **l = list;

  *l = eina_list_append(*l, key);
  return EINA_TRUE;
}

int
ef_cb_efreet_icon_theme_list(void)
{
    int ret = 1;
    Eina_List *themes;
    Eina_List *icon_dirs;
    Eina_List *l;
    Eina_Hash *dirs;
    Eina_Iterator *it;
    Efreet_Icon_Theme *theme;
    const char *dir;
    char buf[PATH_MAX];

    dirs = eina_hash_string_superfast_new(free);

    icon_dirs = efreet_data_dirs_get();

    ef_icon_theme_themes_find(efreet_icon_user_dir_get(), dirs);
    EINA_LIST_FOREACH(icon_dirs, l, dir)
    {
        snprintf(buf, sizeof(buf), "%s/icons", dir);
        ef_icon_theme_themes_find(buf, dirs);
    }
    ef_icon_theme_themes_find("/usr/share/pixmaps", dirs);

    themes = efreet_icon_theme_list_get();
    EINA_LIST_FOREACH(themes, l, theme)
    {
        if ((eina_hash_find(dirs, theme->name.internal)))
	    eina_hash_del(dirs, theme->name.internal, NULL);
        else
        {
            printf("efreet_icon_theme_list_get() returned %s which we didn't "
                    "see when scanning the directories.\n", theme->name.internal);
            ret = 0;
        }
    }
    while (themes)
      {
	 themes = eina_list_remove_list(themes, themes);
      }

    themes = NULL;
    it = eina_hash_iterator_key_new(dirs);
    eina_iterator_foreach(it, EINA_EACH(_hash_keys), &themes);
    eina_iterator_free(it);

    if (eina_list_count(themes) > 0)
    {
        char *dir;

        printf("efreet_icon_theme_list_get() missed: ");
	EINA_LIST_FOREACH(themes, l, dir)
            printf("%s ", dir);
        printf("\n");

        ret = 0;
    }
    while (themes)
      {
	 themes = eina_list_remove_list(themes, themes);
      }
    eina_hash_free(dirs);

    return ret;
}

static void
ef_icon_theme_themes_find(const char *search_dir, Eina_Hash *themes)
{
    Eina_List *dirs;
    char *dir;

    if (!search_dir || !themes) return;

    dirs = ecore_file_ls(search_dir);
    if (!dirs) return;

    while ((dir = eina_list_data_get(dirs)))
    {
        char p[PATH_MAX];

	dirs = eina_list_remove_list(dirs, dirs);
        /* if we've already added the theme we're done */
        if (eina_hash_find(themes, dir))
        {
            free(dir);
            continue;
        }

        /* if the index.theme file exists we open it and look for the hidden
         * flag. */
        snprintf(p, sizeof(p), "%s/%s/index.theme", search_dir, dir);
        if (ecore_file_exists(p))
        {
            Efreet_Ini *ini;
            char *d;
            int skip = 0;

            ini = efreet_ini_new(p);
            efreet_ini_section_set(ini, "Icon Theme");

            if (efreet_ini_boolean_get(ini, "Hidden")) skip = 1;
            if (!efreet_ini_localestring_get(ini, "Name")) skip = 1;
            efreet_ini_free(ini);

            if (!skip)
            {
                d = strdup(dir);
                eina_hash_add(themes, dir, d);
            }
        }
        free(dir);
    }
}

const char *icons[] =
{
    "address-book-new",
    "application-exit",
    "appointment-new",
    "contact-new",
    "dialog-apply",
    "dialog-cancel",
    "dialog-close",
    "dialog-ok",
    "document-new",
    "document-open",
    "document-open-recent",
    "document-page-setup",
    "document-print",
    "document-print-preview",
    "document-properties",
    "document-revert",
    "document-save",
    "document-save-as",
    "edit-copy",
    "edit-cut",
    "edit-delete",
    "edit-find",
    "edit-find-replace",
    "edit-paste",
    "edit-redo",
    "edit-select-all",
    "edit-undo",
    "format-indent-less",
    "format-indent-more",
    "format-justify-center",
    "format-justify-fill",
    "format-justify-left",
    "format-justify-right",
    "format-text-direction-ltr",
    "format-text-direction-rtl",
    "format-text-bold",
    "format-text-italic",
    "format-text-underline",
    "format-text-strikethrough",
    "go-bottom",
    "go-down",
    "go-first",
    "go-home",
    "go-jump",
    "go-last",
    "go-next",
    "go-previous",
    "go-top",
    "go-up",
    "help-about",
    "help-contents",
    "help-faq",
    "insert-image",
    "insert-link",
    "insert-object",
    "insert-text",
    "list-add",
    "list-remove",
    "mail-forward",
    "mail-mark-important",
    "mail-mark-junk",
    "mail-mark-notjunk",
    "mail-mark-read",
    "mail-mark-unread",
    "mail-message-new",
    "mail-reply-all",
    "mail-reply-sender",
    "mail-send-receive",
    "media-eject",
    "media-playback-pause",
    "media-playback-start",
    "media-playback-stop",
    "media-record",
    "media-seek-backward",
    "media-seek-forward",
    "media-skip-backward",
    "media-skip-forward",
    "system-lock-screen",
    "system-log-out",
    "system-run",
    "system-search",
    "system-search",
    "tools-check-spelling",
    "view-fullscreen",
    "view-refresh",
    "view-sort-ascending",
    "view-sort-descending",
    "window-close",
    "window-new",
    "zoom-best-fit",
    "zoom-in",
    "zoom-original",
    "zoom-out",
    "process-working",
    "accessories-calculator",
    "accessories-character-map",
    "accessories-dictionary",
    "accessories-text-editor",
    "help-browser",
    "multimedia-volume-control",
#if 0
    "preferences-desktop-accessibility",
    "preferences-desktop-font",
    "preferences-desktop-keyboard",
    "preferences-desktop-locale",
    "preferences-desktop-multimedia",
    "preferences-desktop-screensaver",
    "preferences-desktop-theme",
    "preferences-desktop-wallpaper",
    "system-file-manager",
    "system-software-update",
    "utilities-terminal",
    "applications-accessories",
    "applications-development",
    "applications-games",
    "applications-graphics",
    "applications-internet",
    "applications-multimedia",
    "applications-office",
    "applications-other",
    "applications-system",
    "applications-utilities",
    "preferences-desktop",
    "preferences-desktop-accessibility",
    "preferences-desktop-peripherals",
    "preferences-desktop-personal",
    "preferences-other",
    "preferences-system",
    "preferences-system-network",
    "system-help",
    "audio-card",
    "audio-input-microphone",
    "battery",
    "camera-photo",
    "camera-video",
    "computer",
    "drive-cdrom",
    "drive-harddisk",
    "drive-removable-media",
    "input-gaming",
    "input-keyboard",
    "input-mouse",
    "media-cdrom",
    "media-floppy",
    "multimedia-player",
    "multimedia-player",
    "network-wired",
    "network-wireless",
    "printer",
    "emblem-default",
    "emblem-documents",
    "emblem-downloads",
    "emblem-favorite",
    "emblem-important",
    "emblem-mail",
    "emblem-photos",
    "emblem-readonly",
    "emblem-shared",
    "emblem-symbolic-link",
    "emblem-synchronized",
    "emblem-system",
    "emblem-unreadable",
    "face-angel",
    "face-crying",
    "face-devil-grin",
    "face-devil-sad",
    "face-glasses",
    "face-kiss",
    "face-monkey",
    "face-plain",
    "face-sad",
    "face-smile",
    "face-smile-big",
    "face-smirk",
    "face-surprise",
    "face-wink",
    "application-x-executable",
    "audio-x-generic",
    "font-x-generic",
    "image-x-generic",
    "package-x-generic",
    "text-html",
    "text-x-generic",
    "text-x-generic-template",
    "text-x-script",
    "video-x-generic",
    "x-office-address-book",
    "x-office-calendar",
    "x-office-document",
    "x-office-presentation",
    "x-office-spreadsheet",
    "folder",
    "folder-remote",
    "network-server",
    "network-workgroup",
    "start-here",
    "user-desktop",
    "user-home",
    "user-trash",
    "appointment-missed",
    "appointment-soon",
    "audio-volume-high",
    "audio-volume-low",
    "audio-volume-medium",
    "audio-volume-muted",
    "battery-caution",
    "battery-low",
    "dialog-error",
    "dialog-information",
    "dialog-password",
    "dialog-question",
    "dialog-warning",
    "folder-drag-accept",
    "folder-open",
    "folder-visiting",
    "image-loading",
    "image-missing",
    "mail-attachment",
    "mail-unread",
    "mail-read",
    "mail-replied",
    "mail-signed",
    "mail-signed-verified",
    "media-playlist-repeat",
    "media-playlist-shuffle",
    "network-error",
    "network-idle",
    "network-offline",
    "network-receive",
    "network-transmit",
    "network-transmit-receive",
    "printer-error",
    "printer-printing",
    "software-update-available",
    "software-update-urgent",
    "sync-error",
    "sync-synchronizing",
    "task-due",
    "task-passed-due",
    "user-away",
    "user-idle",
    "user-offline",
    "user-online",
    "user-trash-full",
    "weather-clear",
    "weather-clear-night",
    "weather-few-clouds",
    "weather-few-clouds-night",
    "weather-fog",
    "weather-overcast",
    "weather-severe-alert",
    "weather-showers",
    "weather-showers-scattered",
    "weather-snow",
    "weather-storm",
#endif
    NULL
};

int
ef_cb_efreet_icon_match(void)
{
    int i, ret = 1;
    Eina_Hash *icon_hash;
    Efreet_Icon_Theme *theme;
    Eina_List *themes;
    Eina_List *l;

    themes = efreet_icon_theme_list_get();
    EINA_LIST_FOREACH(themes, l, theme)
    {
        if (!strcmp(theme->name.internal, THEME))
            break;
    }

    if (!theme)
    {
        printf("Theme not installed, SKIPPED.\n");
	while (themes)
	   themes = eina_list_remove_list(themes, themes);
        return 1;
    }

    icon_hash = eina_hash_string_superfast_new(free);

    ef_icons_find(theme, themes, icon_hash);
    while (themes)
       themes = eina_list_remove_list(themes, themes);

    double start = ecore_time_get();
    for (i = 0; icons[i] != NULL; i++)
    {
        char *path, *s;

        path = efreet_icon_path_find(THEME, icons[i], SIZE);

        if (!path)
        {
#if 1
            if (eina_hash_find(icon_hash, icons[i]))
            {
                printf("NOT FOUND %s\n", icons[i]);
                ret = 0;
            }
#endif
            continue;
        }
        else if (!eina_hash_find(icon_hash, icons[i]))
        {
            printf("Found icon not in hash: %s\n", icons[i]);
        }

        s = strrchr(path, '.');
        if (s) *s = '\0';
        s = strrchr(path, '/');
        if (s) s++;

        if (s && strcmp(s, icons[i]))
        {
            printf("Name mismatch name (%s) vs ef (%s)\n", icons[i], s);
            ret = 0;
        }
        free(path);
    }
    printf("Time: %f\n", (ecore_time_get() - start));
    eina_hash_free(icon_hash);

    start = ecore_time_get();
    for (i = 0; icons[i] != NULL; i++)
    {
        char *path, *s;

        path = efreet_icon_path_find(THEME, icons[i], SIZE);

        if (!path) continue;

        s = strrchr(path, '.');
        if (s) *s = '\0';
        s = strrchr(path, '/');
        if (s) s++;

        if (s && strcmp(s, icons[i]))
        {
            printf("Name mismatch name (%s) vs ef (%s)\n", icons[i], s);
            ret = 0;
        }
        free(path);
    }
    printf("Time: %f\n", (ecore_time_get() - start));

    return ret;
}

static void
ef_icons_find(Efreet_Icon_Theme *theme, Eina_List *themes, Eina_Hash *icons)
{
    Eina_List *l, *ll;
    char path[PATH_MAX];
    const char *theme_path;

    if (!theme || !icons) return;

    EINA_LIST_FOREACH(theme->paths, l, theme_path)
    {
        Efreet_Icon_Theme_Directory *dir;

	 EINA_LIST_FOREACH(theme->directories, ll, dir)
            {
                snprintf(path, sizeof(path), "%s/%s/", theme_path, dir->name);
                ef_read_dir(path, icons);
            }
        }

    if (theme->inherits)
    {
        Efreet_Icon_Theme *parent_theme;
        char *parent;

	EINA_LIST_FOREACH(theme->inherits, l, parent)
        {
	    EINA_LIST_FOREACH(themes, ll, parent_theme)
            {
                if (!strcmp(parent_theme->name.internal, parent))
                    ef_icons_find(parent_theme, themes, icons);
            }
        }
    }
    else
    {
        Efreet_Icon_Theme *parent_theme;

	EINA_LIST_FOREACH(themes, l, parent_theme)
        {
            if (!strcmp(parent_theme->name.internal, "hicolor"))
                ef_icons_find(parent_theme, themes, icons);
        }
    }

    ef_read_dir("/usr/share/pixmaps", icons);
}

static void
ef_read_dir(const char *dir, Eina_Hash *icons)
{
    Eina_List *files;
    char *file;

    if (!dir || !icons) return;

    files = ecore_file_ls(dir);
    if (!files) return;

    while ((file = eina_list_data_get(files)))
    {
        char *p;

	files = eina_list_remove_list(files, files);
        p = strrchr(file, '.');
        if (!p)
        {
            FREE(file);
            continue;
        }

        if (!strcmp(p, ".png") || !strcmp(p, ".xpm"))
        {
            *p = '\0';

            eina_hash_add(icons, file, strdup(file));
        }

        FREE(file);
    }
}
