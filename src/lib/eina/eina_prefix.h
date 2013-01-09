#ifndef EINA_PREFIX_H_
#define EINA_PREFIX_H_

/**
 * @addtogroup Eina_Prefix_Group Prefix Group
 * @ingroup Eina
 *
 * @brief These functions provide the ability to determine the runtime
 * location of a software package
 *
 * @{
 *
 * @since 1.1.0
 */

/**
 * @typedef Eina_Prefix
 * This is a prefix object that is returned by eina_prefix_new() when trying
 * to determine the runtime location of the software in question so other
 * data files such as images, sound files, other executable utilities,
 * libraries, modules and locale files can be found.
 *
 * @since 1.1.0
 */
typedef struct _Eina_Prefix Eina_Prefix;

/**
 * @brief Create a new prefix handle given some input information
 *
 * @param argv0 If this is an executable this is argv[0] of the binary, or @c NULL if it is used from a shared library
 * @param symbol This is a symbol (function for example) inside the binary or library to find the source location of. Provide @c NULL if not used
 * @param envprefix This is the prefix to any environment variables that may override prefix detection and give the exact location of the software
 * @param sharedir This is the directory inside the standard share or data dir where the software will store data files
 * @param magicsharefile This is a magic file to check existence of to determine the prefix find was correct, and it must be located in the data
 * dir under the share dir provided above, or @c NULL if the check is not to be done.
 * @param pkg_bin This is the compile-time binary install dir
 * @param pkg_lib This is the compile-time library install dir
 * @param pkg_data This is the compile-time share/data install dir
 * @param pkg_locale This is the compile-time locale install dir
 * @return The prefix handle, or @c NULL on failure.
 *
 * Applications and libraries are most often not just single executables nor
 * single shared library binaries, but also come with extra modules they
 * have to load, extra binary utilities they need to run, or have data files
 * they need to load. A very primitive application ASSUMES a fixed install
 * location at compile-time, but this disallows the ability to re-locate
 * the application (or library) somewhere else after compilation (if you run
 * out of space on a given disk, partition etc. for example), or necessitate
 * the need for having to maintain environment variables for every piece of
 * software to let it know its location, or have to use large sets of
 * symlinks pointing from the compiled location to the new one.
 *
 * Being re-locatable at runtime allows much easier distribution and
 * installation into places like the users own home directory, instead of
 * on a system partition, if the developer wishes for easier distribution
 * of pre-compiled binaries.
 *
 * The prefix system is designed to locate where the given software is
 * installed (under a common prefix) at runtime and then report specific
 * locations of this prefix and common directories inside this prefix like
 * the binary, library, data and locale directories.
 *
 * To do this some information needs to be provided to eina_prefix_new(). If
 * you have developed a binary executable, then provide argv[0] as the @p argv0
 * argument. This plus the PATH environment variable help the prefix system
 * to determine its location. Call eina_prefix_new() early on before you
 * change working directory or anything about argv[0] so it gets accurate
 * information. It will use the first argument, being the executable itself,
 * to look in absolute directories, relative paths and PATH to see if it
 * finds the right executable to determine just where the actual binary is
 * installed and being run from. If you develop a share library, just pass
 * @c NULL as argv0
 *
 * It would prefer to use the @p symbol function to determine location as
 * that function will be unique inside the application and try and trace
 * back which file this function comes from (be it a binary or shared library)
 * as this avoids more expensive searches via @p argv0. It will use this
 * symbol if given in preference to argv0.
 *
 * The @p envprefix parameter, provides a string prefix to prepend before
 * environment variables to allow a fallback to specific environment variables
 * to locate the software. For example if "MYAPP" is provided a the prefix,
 * then it uses "MYAPP_PREFIX" as a master environment variable to specify
 * the exact install prefix for the software, or more specific environment
 * variables like "MYAPP_BIN_DIR", "MYAPP_LIB_DIR", "MYAPP_DATA_DIR" and
 * "MYAPP_LOCALE_DIR" which can be set by the user or scripts before
 * launching. If not provided (NULL) environment variables will not be
 * used to override compiled-in defaults or auto detections.
 *
 * The @p sharedir string provides a subdirectory inside the system shared
 * data dir for data files. For example, if the system dir is
 * /usr/local/share then this dir name is appended, creating
 * /usr/local/share/appname if this dir was the "appname" string. It is
 * expected the application or library installs data files in this directory.
 *
 * The @p magicsharefile is a filename or path of something inside the share
 * or data dir to be used to test that the prefix detection worked. For
 * example, your app will install a wallpaper image as
 * /usr/local/share/appname/images/wallpaper.jpg and so to check that this
 * worked, provide "images/wallpaper.jpg" as the @p magicsharefile string
 * so detection can know if it worked or not.
 *
 * The @p pkg_bin, @p pkg_lib, @p pkg_data and @p pkg_locale are compile-time
 * strings (the kind standard autoconf/automake define) to be passed in
 * so there can be a fallback to compiled-in defaults as well as use them
 * to determine actual names of directories like libdirs maybe changing to
 * be lib32 or lib64 instead of lib etc.
 *
 * Compile the following defining at compile time your prefixes like (example):
 *
 * gcc appname.c -o appname
 * -DPACKAGE_BIN_DIR=\\"/usr/local/bin\"
 * -DPACKAGE_LIB_DIR=\\"/usr/local/lib\"
 * -DPACKAGE_DATA_DIR=\\"/usr/local/share/appname\"
 * -DLOCALE_DIR=\\"/usr/local/share/locale\"
 * `pkg-config --cflags --libs eina`
 *
 * (of course add appropriate compile flags to linking etc. etc. and note that
 * locale dir is optional. if you don't need it provide data dir as the
 * locale dir. also note that the magicsharefile is optional for testing and
 * ensuring that the prefix check is correct. this file must be installed
 * in the application data dir (eg /usr/local/share/appname) and be referred
 * to using a unix-style relative path from that dir, eg directory/filename.png)
 *
 * @code
 * #include <Eina.h>
 *
 * static Eina_Prefix *pfx = NULL;
 *
 * int main(int argc, char **argv)
 * {
 *   eina_init();
 *
 *   pfx = eina_prefix_new(argv[0], main, "APPNAME", "appname", NULL,
 *                         PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
 *                         PACKAGE_DATA_DIR, LOCALE_DIR);
 *   if (!pfx) printf("ERROR: Critical error in finding prefix\n");
 *   printf("install prefix is: %s\n", eina_prefix_get(pfx));
 *   printf("binaries are in: %s\n", eina_prefix_bin_get(pfx));
 *   printf("libraries are in: %s\n", eina_prefix_lib_get(pfx));
 *   printf("data files are in: %s\n", eina_prefix_data_get(pfx));
 *   eina_prefix_free(pfx);
 *
 *   eina_shutdown();
 * }
 * @endcode
 *
 * @since 1.1.0
 */
EAPI Eina_Prefix *
eina_prefix_new(const char *argv0, void *symbol, const char *envprefix,
                const char *sharedir, const char *magicsharefile,
                const char *pkg_bin, const char *pkg_lib,
                const char *pkg_data, const char *pkg_locale) EINA_ARG_NONNULL(6, 7, 8, 9) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Free the prefix object and all its contents
 *
 * @param pfx The prefix object
 *
 * Free the prefix object and all its allocated content. It will be invalid
 * to access the object after being freed.
 *
 * @since 1.1.0
 */
EAPI void
eina_prefix_free(Eina_Prefix *pfx) EINA_ARG_NONNULL(1);

/**
 * @brief Get the prefix base directory
 *
 * @param pfx The prefix object
 * @return The base prefix (eg "/usr/local", "/usr", "/opt/appname" or
 * "/home/user/myapps/appname" etc.) that the software resides in at runtime.
 *
 * @since 1.1.0
 */
EAPI const char *
eina_prefix_get(Eina_Prefix *pfx) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Get the binary installation directory
 *
 * @param pfx The prefix object
 * @return The location of installed binaries (eg "/usr/local/bin",
 * "/usr/bin", "/opt/appname/bin", "/home/user/myapps/appname/bin" etc.).
 *
 * @since 1.1.0
 */
EAPI const char *
eina_prefix_bin_get(Eina_Prefix *pfx) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Get the library installation directory
 *
 * @param pfx The prefix object
 * @return The location of installed binaries (eg "/usr/local/lib",
 * "/usr/lib32", "/opt/appname/lib64", "/home/user/myapps/appname/lib" etc.).
 *
 * @since 1.1.0
 */
EAPI const char *
eina_prefix_lib_get(Eina_Prefix *pfx) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Get the data installation directory
 *
 * @param pfx The prefix object
 * @return The location of installed binaries (eg "/usr/local/share/appname",
 * "/usr/share/appname", "/opt/appname/share/appname", "/home/user/myapps/appname/share/appname" etc.).
 *
 * @since 1.1.0
 */
EAPI const char *
eina_prefix_data_get(Eina_Prefix *pfx) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Get the locale installation directory
 *
 * @param pfx The prefix object
 * @return The location of installed binaries (eg "/usr/local/share/locale",
 * "/usr/share/locale", "/opt/appname/share/locale", "/home/user/myapps/appname/share/locale" etc.).
 *
 * @since 1.1.0
 */
EAPI const char *
eina_prefix_locale_get(Eina_Prefix *pfx) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @}
 */
#endif
