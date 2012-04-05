LIST(APPEND EVAS_CC_SOURCES
   elw_box.c
   elw_boxedbutton.c
   elw_win.c
   elw_button.c
   evas_obj.c
   test.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${ELEMENTARY_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/lib
   )

add_executable(evas ${EVAS_CC_SOURCES})
get_target_property(eobj_LIB_FILE eobj LOCATION)
target_link_libraries(evas
   ${EINA_LIBRARIES}
   ${EVAS_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   ${eobj_LIB_FILE}
   )

add_dependencies(evas eobj)
