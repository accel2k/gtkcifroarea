
project (GtkCifroArea)

cmake_minimum_required (VERSION 2.6)

set (GTK_CIFRO_AREA_VERSION 1.0)

include (FindPkgConfig)

#
# Default build type.
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif ()

#
# Compilers options.
if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -Wall")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og -Wall")
  set (MLIB m)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /wd4244 /wd4305")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /wd4244 /wd4305")
endif ()

#
# Gtk2 support.
if (CIFRO_AREA_WITH_GTK2)
  add_definitions (-DCIFRO_AREA_WITH_GTK2)
endif ()

#
# Output directories.
if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif ()

if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif ()

if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif ()

#
# Dependencies
if (NOT CIFRO_AREA_WITH_GTK2)
  pkg_check_modules (GTK REQUIRED "gtk+-3.0")
  set (GTK_VERSION 3.0)
else ()
  pkg_check_modules (GTK REQUIRED "gtk+-2.0")
  set (GTK_VERSION 2.0)
endif ()
add_definitions (${GTK_CFLAGS})
link_directories (${GTK_LIBRARY_DIRS})

#
# Include directories.
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/gtkcifroarea")

#
# Targets.
add_definitions (-DG_LOG_DOMAIN="GtkCifroArea")
add_subdirectory (gtkcifroarea)
add_subdirectory (tests)

#
# Pkgconfig support.
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/gtkcifroarea/gtkcifroarea.pc.in"
                "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/gtkcifroarea-${GTK_VERSION}.pc"
                @ONLY)

install (FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/gtkcifroarea-${GTK_VERSION}.pc"
         DESTINATION "lib/pkgconfig"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)
