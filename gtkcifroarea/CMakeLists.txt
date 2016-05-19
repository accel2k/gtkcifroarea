
#
# GtkCifroArea library.
add_library (gtkcifroarea-${GTK_VERSION} SHARED
             cairo-sdline.c
             gtk-cifro-area.c
             gtk-cifro-scope.c
             gtk-cifro-curve.c)

target_link_libraries (gtkcifroarea-${GTK_VERSION} ${GTK_LIBRARIES})

#
# Install targets.
install (TARGETS gtkcifroarea-${GTK_VERSION} EXPORT gtkcifroarea-${GTK_VERSION}
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

install (FILES gtk-cifro-area.h gtk-cifro-scope.h gtk-cifro-curve.h gtk-cifro-area-exports.h
         DESTINATION include/gtkcifroarea
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
