 This folder contains Microsoft Visual Studion projects and some sources for
 building Evil.

Important:

 Do not launch manually any solutions or projects in the folder, to start
 a solution run proper start.bat script, it will setup environment for
 projects and will check system requirements.

 Before start make sure that set_env.bat:EXT_DIR variable points to proper
 extern directory. Extern directory layout and content (may be different, but
 this is guaranteed to be enough):

  extern
    bin
      charset.dll
      cjpeg.exe
      djpeg.exe
      exifautotran
      freetype6.dll
      glew32.dll
      glewinfo.exe
      glewinfo.txt
      iconv.dll
      jpeg62.dll
      jpegexiforient.exe
      jpegtran.exe
      libpng12.dll
      libpng13d.dll
      libpng3.dll
      libtiff3.dll
      libungif4.dll
      rdjpgcom.exe
      visualinfo.exe
      wrjpgcom.exe
      zlib1.dll

    include
      freetype
        ... (freetype includes)
      GL
        glew.h
        glxew.h
        wglew.h
      sys
        param.h

      ansidecl.h
      ft2build.h
      getopt.h
      gif_lib.h
      iconv.h
      jconfig.h
      jerror.h
      jmorecfg.h
      jpeglib.h
      libcharset.h
      localcharset.h
      png.h
      pngconf.h
      stdint.h
      tiff.h
      tiffconf.h
      tiffio.h
      tiffvers.h
      unistd.h
      zconf.h
      zlib.h

    lib
      charset.lib
      freetype.lib
      glew32.lib
      glew32s.lib
      iconv.lib
      jpeg.def
      jpeg.lib
      libpng.lib
      libpng13d.lib
      libtiff.def
      libtiff.lib
      libungif.lib
      zlib.def
      zlib.lib
