
add_executable (cifrotest cifrotest.c)

target_link_libraries (cifrotest ${GTK_LIBRARIES} gtkcifroarea-${GTK_CIFRO_AREA_VERSION} ${MLIB})

install (TARGETS cifrotest
         COMPONENT test
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
