
gtk_doc_add_module (GtkCifroArea
                    SUFFIXES "h" "c"
                    SOURCE "${CMAKE_SOURCE_DIR}/gtkcifroarea/"
                    LIBRARIES gtkcifroarea-${GTK_CIFRO_AREA_VERSION}
                    XML "${CMAKE_CURRENT_SOURCE_DIR}/gtk-cifro-area-docs.xml"
                    IGNOREHEADERS "${CMAKE_SOURCE_DIR}/gtkcifroarea/cairo-sdline.h"
                                  "${CMAKE_SOURCE_DIR}/gtkcifroarea/gtk-cifro-area-exports.h")

add_custom_target (documentation ALL DEPENDS doc-GtkCifroArea)

install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/GtkCifroArea/html/"
         DESTINATION "share/doc/gtkcifroarea-${GTK_CIFRO_AREA_VERSION}/gtkcifroarea"
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
         DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
