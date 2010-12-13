 This folder contains Microsoft Visual Studio projects and some sources for
 building Evil.

Important:

 Do not launch manually any solutions or projects in the folder, to start
 a solution run the start.bat script, it will setup environment for
 projects and will check system requirements. start.bat will detect
 automatically the version of Visual Studio which is installed. It
 currently supports Visual Studio 2008, Visual Studio 2010 should convert
 the files.

 Before start make sure that the EXT_DIR variable points to proper
 extern directory. Extern directory layout and content (may be different, but
 this is guaranteed to be enough):

  extern
    bin
      freetype6.dll
      iconv.dll
      jpeg62.dll
      libpng14.dll
      libtiff3.dll
      libungif4.dll
      zlib1.dll

    include
      freetype
        ... (freetype includes)
      sys
        param.h

      ft2build.h
      gif_lib.h
      iconv.h
      jconfig.h
      jerror.h
      jmorecfg.h
      jpeglib.h
      png.h
      pngconf.h
      tiff.h
      tiffconf.h
      tiffio.h
      tiffvers.h
      zconf.h
      zlib.h

    lib
      freetype.lib
      iconv.lib
      jpeg.lib
      libpng.lib
      libtiff.lib
      libungif.lib
      zlib.lib
