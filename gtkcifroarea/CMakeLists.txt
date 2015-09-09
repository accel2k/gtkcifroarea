
#
# Searching for Glib marshallers compiler.
find_program( MARSHAL_COMPILER glib-genmarshal )
if( ${MARSHAL_COMPILER} STREQUAL MARSHAL_COMPILER-NOTFOUND )
  message( FATAL_ERROR "GLib marshallers compiler not found." )
endif()

#
# 
file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/marshallers )
include_directories( ${CMAKE_BINARY_DIR}/marshallers )

#
# Header marshallers file.
execute_process( COMMAND ${MARSHAL_COMPILER}
                         --header
                         gtk-cifro-area-marshallers.list
                         OUTPUT_FILE ${CMAKE_BINARY_DIR}/marshallers/gtk-cifro-area-marshallers.h
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

#
# Source marshallers file.
execute_process( COMMAND ${MARSHAL_COMPILER}
                         --body
                         "gtk-cifro-area-marshallers.list"
                         OUTPUT_FILE ${CMAKE_BINARY_DIR}/marshallers/gtk-cifro-area-marshallers.c
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

#
# GtkCifroArea library.
add_library( gtkcifroarea-${GTK_VERSION} SHARED
             cairo-sdline.c
             gtk-cifro-area-state.c
             gtk-cifro-area.c
             gtk-cifro-scope.c
             gtk-cifro-scope-renderer.c
             gtk-cifro-curve.c
             gtk-cifro-curve-renderer.c
             ${CMAKE_BINARY_DIR}/marshallers/gtk-cifro-area-marshallers.c )

target_link_libraries( gtkcifroarea-${GTK_VERSION} ${GTK_LIBRARIES} )

#
# Install targets.
install( TARGETS gtkcifroarea-${GTK_VERSION} EXPORT gtkcifroarea-${GTK_VERSION}
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ )

install( FILES gtk-cifro-area.h
               gtk-cifro-area-state.h
               gtk-cifro-scope.h
               gtk-cifro-scope-types.h
               gtk-cifro-curve.h
               gtk-cifro-curve-types.h
               DESTINATION include/gtkcifroarea PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ )
