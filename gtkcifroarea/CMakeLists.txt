
add_library (gtkcifroarea-${GTK_CIFRO_AREA_VERSION} SHARED
             cairo-sdline.c
             gtk-cifro-area.c
             gtk-cifro-area-control.c
             gtk-cifro-scope.c
             gtk-cifro-curve.c)

target_link_libraries (gtkcifroarea-${GTK_CIFRO_AREA_VERSION} ${GTK_LIBRARIES})

configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/gtkcifroarea.pc.in"
                "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/gtkcifroarea-${GTK_CIFRO_AREA_VERSION}.pc"
                @ONLY)

install (FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/gtkcifroarea-${GTK_CIFRO_AREA_VERSION}.pc"
         DESTINATION "lib/pkgconfig"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)

install (TARGETS gtkcifroarea-${GTK_CIFRO_AREA_VERSION} EXPORT gtkcifroarea-${GTK_CIFRO_AREA_VERSION}
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

install (FILES gtk-cifro-area.h gtk-cifro-area-control.h gtk-cifro-scope.h gtk-cifro-curve.h gtk-cifro-area-exports.h
         DESTINATION "include/gtkcifroarea-${GTK_CIFRO_AREA_VERSION}"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
