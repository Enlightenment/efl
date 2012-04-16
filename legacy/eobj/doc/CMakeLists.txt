include(UseDoxygen OPTIONAL)

add_custom_target(__copy_img COMMAND ${CMAKE_COMMAND} -E copy_directory
   ${CMAKE_CURRENT_SOURCE_DIR}/img/
   ${CMAKE_CURRENT_BINARY_DIR}/html/)

add_dependencies(doc __copy_img)
