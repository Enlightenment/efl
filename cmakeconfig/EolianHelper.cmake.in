# macro to create a eolian generated c source file
#
# macro adds a generate rule, which depends on the original file the rule will output file.x
#
# The passed include snippet will just be added to the command

macro(_rule_eox file include deps)
    add_custom_command(
       OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${file}.x
       COMMAND eolian_gen ${include} -g c -o c:${CMAKE_CURRENT_SOURCE_DIR}/${file}.x ${CMAKE_CURRENT_SOURCE_DIR}/${file}
       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
       DEPENDS ${deps}
    )
endmacro()

# macro to create a eolian generated header file
#
# other details are like the eox rule
macro(_rule_eoh file include deps)
    add_custom_command(
       OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${file}.h
       COMMAND eolian_gen ${include} -g h -o h:${CMAKE_CURRENT_SOURCE_DIR}/${file}.h ${CMAKE_CURRENT_SOURCE_DIR}/${file}
       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
       DEPENDS ${deps}
    )
endmacro()

# Can be used to create .eo.x , .eo.h, .eot.h generator rules.
# <file>.eo files are generated into <file>.eo.x and <file>.eo.h files
# <file>.eot files are generated into <file>.eot.h files
# The standard include path of efl eolian files is automatically added to the includes
#
# build_files - A list of files
# relative_include_dirs - A list of dirs to include
#
# If one of the included files is changed the file will be rebuilded at the next
# make call
#
# The macro scans for .eo files, hey have to be in the build_files.
# The generator rules are just executed if the file is a dependecy of some lib/executable.

function(eo_rule_create build_files relative_include_dirs)
   string(REPLACE "\n" "" EOLIAN_EO_DIR_WITHOUT_NEWLINE "${EOLIAN_EO_DIR}")

   # add std includes
   list(APPEND include_dirs
      ${EOLIAN_EO_DIR_WITHOUT_NEWLINE}
    )

   # convert relative to absolute
   foreach(relative_include_dir ${relative_include_dirs})
      list(APPEND include_dirs
        ${CMAKE_CURRENT_SOURCE_DIR}/${relative_include_dir}
      )
   endforeach()

   # work with the absolute paths
   foreach(include_cmd ${include_dirs})
      # build include cmd
      string(CONCAT includes "${includes}" " -I${include_cmd}")
      # fetch dep files
      file(GLOB_RECURSE files "${include_cmd}/*.eo")
      foreach(file ${files})
        list(APPEND dep_files ${file})
      endforeach()
   endforeach()

   string(REPLACE " " ";" includes "${includes}")
   foreach(file ${build_files})
      get_filename_component(ext ${file} EXT)
      if (ext MATCHES "^\\.eo$")
         _rule_eoh("${file}" "${includes}" "${dep_files}")
         _rule_eox("${file}" "${includes}" "${dep_files}")
      endif()
      if (ext MATCHES "^\\.eot$")
         _rule_eoh("${file}" "${includes}" "${dep_files}")
      endif()
    endforeach()
endfunction()
