
add_executable( cifrotest cifrotest.c )

target_link_libraries( cifrotest ${GTK_LIBRARIES} gtkcifroarea-${GTK_VERSION} m )
