#ifndef EINA_VPATH_H
#define EINA_VPATH_H

#include "eina_prefix.h"

#ifdef EFL_BETA_API_SUPPORT

/**
 * Eina vpath is a path that can be prefixed with a virtual path.
 *
 * A virtual path can either start with (:XXXXXXXX:) that indicates a virtual
 * path with XXXXXXXX as the location, OR a normal path with / or a relative
 * path like ./ or ../ or even shell common locators like ~/ or ~username/
 *
 * The string between (: and :) is used as key to lookup the real value.
 * The key has to be set by an interface before, otherwise you will get an
 * error.
 *
 * Sample paths:
 *
 *   ~/myfiles/file.png
 *   ~bob/dir/file.txt
 *   /path/to/file.cfg
 *   ./file/relative/path.txt
 *   file/relative/path.txt
 *   (:tmp:)/some-temp-file/file.txt
 *   (:home:)/myfiles/file.png
 *
 *   (:app.dir:)/bin/app-executable
 *   (:app.bin:)/app-executable-file
 *   (:app.lib:)/app-library.so
 *   (:app.data:)/application/datafile.png
 *   (:app.locale:)/en_US/LC_MESSAGES/app.mo
 *   (:app.config:)/application-config.xml
 *   (:app.local:)/application-data-file.jpg
 *   (:app.tmp:)/some-temp-file/path/file.txt
 *
 *   (:usr.desktop:)/file-in-users-desktop-directory.txt
 *   (:usr.documents:)/letter-to-gradma.doc
 *   (:usr.downloads:)/file-downloaded-here.zip
 *   (:usr.music:)/fave-song.mp3
 *   (:usr.pictures:)/a-photo.,jpg
 *   (:usr.public:)/some-publicly-shared-file
 *   (:usr.templates:)/some-template-document.txt
 *   (:usr.videos:)/some-video-file.mp4
 *   (:usr.data:)/file-in-user-data-dir
 *   (:usr.config:)/file-in-user-conifg-dir
 *   (:usr.cache:)/file-in-user-cache-dir
 *   (:usr.run:)/file-in-xdg-runtime-dir
 *   (:usr.tmp:)/some-temp-file/path/file.txt
 *
 * Commonly mapped to real path (but may vary):
 *
 *   (:tmp:) - /tmp
 *   (:home:) - ~/
 *
 *   (:app.dir:) - /usr - (assuming app install PREFIX of /usr. may be /usr/local or others too)
 *   (:app.bin:) - /usr/bin - (almost always PREFIX/bin)
 *   (:app.lib:) - /usr/lib - (almost always PREFIX/lib)
 *   (:app.data:) - /usr/share/appname - (almost always PREFIX/share/appname)
 *   (:app.locale:) - /usr/share/locale - (almost always PREFIX/locale)
 *   (:app.config:) - ~/.config/appname
 *   (:app.local:) - ~/.local/share/appname
 *   (:app.tmp:) - ~/.local/tmp/appname
 *
 *   (:usr.desktop:) - ~/Desktop
 *   (:usr.documents:) - ~/Documents
 *   (:usr.downloads:) - ~/Downloads
 *   (:usr.music:) - ~/Music
 *   (:usr.pictures:) - ~/Pictures
 *   (:usr.public:) - ~/Public
 *   (:usr.templates:) - ~/Templates
 *   (:usr.videos:) - ~/Videos
 *   (:usr.data:) - ~/.local/share
 *   (:usr.config:) - ~/.config
 *   (:usr.cache:) - ~/.cache
 *   (:usr.run:) - /var/run/user/1000
 *   (:usr.tmp:) - ~/.local/tmp
 *
 * Additional info: https://phab.enlightenment.org/w/eina_vpath/
 *
 * @since 1.21
 *
 */
typedef const char * Eina_Vpath;

/**
 * Translate a virtual path into a normal path.
 * 
 * The return string is a string allocated by malloc and should be freed with
 * free() when no longer needed.
 *
 * @param[in] path The path.
 * @return NULL if failed, or a full normal string file path that is resolved
 *
 * @since 1.21
 *
 */
EAPI char *eina_vpath_resolve(Eina_Vpath path);

/**
 * Translate a virtual path into a normal path, and print it into str.
 *
 * @param[out] str the buffer to stuff the characters into
 * @param[in] size the size of the buffer
 * @param[in] format A snprintf style format string, which will get evaluated after the vpath strings are getting replaced
 * @param[in] ... The arguments for the format string
 *
 * @return the number of characters that are written into str, on a error a value < 0 is returned.
 *
 * @since 1.21
 *
 */
EAPI int eina_vpath_resolve_snprintf(char *str, size_t size, const char *format, ...);
#endif
#endif
