<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: evas_software_win32 - Win32 (WCE emulator) Debug--------------------
</h3>
<h3>Command Lines</h3>
Creating command line "rc.exe /l 0x409 /fo"emulatorDbg/evas_software_win32.res" /i "\Projects\mbed\evas\src\bin\evas_software_win32" /d "WCE_PLATFORM_STANDARDSDK" /d UNDER_CE=400 /d _WIN32_WCE=400 /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "_X86_" /d "x86" /d "_i386_" /r "C:\Projects\mbed\evas\src\bin\evas_software_win32\evas_software_win32.rc"" 
Creating temporary file "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP1E8.tmp" with contents
[
/nologo /W3 /Zi /Od /I "..\..\\" /I "..\..\src\bin" /I "..\..\src\lib" /D "DEBUG" /D "_i386_" /D "i_386_" /D "_X86_" /D "x86" /D UNDER_CE=400 /D _WIN32_WCE=400 /D "WCE_PLATFORM_STANDARDSDK" /D "UNICODE" /D "_UNICODE" /FR"emulatorDbg/" /Fo"emulatorDbg/" /Fd"emulatorDbg/" /Gs8192 /GF /c 
"C:\Projects\mbed\evas\src\bin\evas_software_win32\evas_software_win32.cpp"
]
Creating command line "cl.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP1E8.tmp" 
Creating temporary file "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP1E9.tmp" with contents
[
/nologo /W3 /Zi /Od /I "..\..\\" /I "..\..\src\bin" /I "..\..\src\lib" /D "DEBUG" /D "_i386_" /D "i_386_" /D "_X86_" /D "x86" /D UNDER_CE=400 /D _WIN32_WCE=400 /D "WCE_PLATFORM_STANDARDSDK" /D "MBCS" /D "_MBCS" /FR"emulatorDbg/" /Fo"emulatorDbg/" /Fd"emulatorDbg/" /Gs8192 /GF /c 
"C:\Projects\mbed\evas\src\bin\evas_test_main.c"
"C:\Projects\mbed\evas\src\bin\evas_software_win32\StdAfx.cpp"
]
Creating command line "cl.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP1E9.tmp" 
Creating temporary file "C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP1EA.tmp" with contents
[
commctrl.lib coredll.lib corelibc.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:yes /pdb:"emulatorDbg/evas_software_win32.pdb" /debug /nodefaultlib:"OLDNAMES.lib" /nodefaultlib:libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /out:"emulatorDbg/evas_software_win32.exe" /subsystem:windowsce,4.00 /MACHINE:IX86 
.\emulatorDbg\evas_software_win32.obj
.\emulatorDbg\evas_test_main.obj
.\emulatorDbg\StdAfx.obj
.\emulatorDbg\evas_software_win32.res
.\emulatorDbg\evaslib.lib
]
Creating command line "link.exe @C:\DOCUME~1\ADMINI~1\LOCALS~1\Temp\RSP1EA.tmp"
<h3>Output Window</h3>
Compiling resources...
Compiling...
evas_software_win32.cpp
C:\Projects\mbed\evas\src\bin\evas_software_win32\evas_software_win32.cpp(195) : warning C4101: 'szHello' : unreferenced local variable
Compiling...
evas_test_main.c
C:\Projects\mbed\evas\src\bin\evas_test_main.c(91) : warning C4244: 'function' : conversion from 'double ' to 'int ', possible loss of data
C:\Projects\mbed\evas\src\bin\evas_test_main.c(324) : warning C4101: 'iw' : unreferenced local variable
C:\Projects\mbed\evas\src\bin\evas_test_main.c(324) : warning C4101: 'ih' : unreferenced local variable
C:\Projects\mbed\evas\src\bin\evas_test_main.c(358) : warning C4101: 'iw' : unreferenced local variable
C:\Projects\mbed\evas\src\bin\evas_test_main.c(358) : warning C4101: 'ih' : unreferenced local variable
C:\Projects\mbed\evas\src\bin\evas_test_main.c(394) : warning C4101: 'iw' : unreferenced local variable
C:\Projects\mbed\evas\src\bin\evas_test_main.c(394) : warning C4101: 'ih' : unreferenced local variable
Generating Code...
Compiling...
StdAfx.cpp
C:\Projects\mbed\evas\src\bin\evas_test_main.c(91) : warning C4761: integral size mismatch in argument; conversion supplied
Generating Code...
Linking...
evaslib.lib(evas_callbacks.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_clip.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_events.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_layer.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_name.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_object_gradient.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_object_image.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_object_line.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_object_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_object_polygon.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_object_rectangle.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_object_text.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_rectangle.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_render.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_stack.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_hash.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_list.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_object_list.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_blend_alpha_color_pixel.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_blend_color_pixel.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_blend_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_blend_pixel_cmod_pixel.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_blend_pixel_mul_pixel.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_blend_pixel_pixel.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_blit_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_convert_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_convert_rgb_16.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_convert_rgb_32.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_cpu.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_draw_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_font_draw.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_font_load.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_font_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_font_query.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_gradient_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_image_load.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_image_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_line_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_polygon_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_rectangle_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_scale_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_scale_sample.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_scale_smooth.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_tiler.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_engine.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_outbuf.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_win32_buffer.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_win32_main.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(evas_path.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(winfnt.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(type42.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(type1.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(truetype.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(smooth.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(sfnt.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(raster.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(psmodule.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(pshinter.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(psaux.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(pfr.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(pcf.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(type1cid.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(cff.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(bdf.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(ftsystem.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(ftinit.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(ftglyph.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(ftbase.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info
evaslib.lib(autohint.obj) : warning LNK4204: 'C:\Projects\mbed\evas\proj\win32_gdi\emulatorDbg\vc60.pdb' is missing debugging information for referencing module; linking object as if no debug info



<h3>Results</h3>
evas_software_win32.exe - 0 error(s), 79 warning(s)
</pre>
</body>
</html>
