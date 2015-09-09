cmake_minimum_required( VERSION 2.6 )

project( GtkCifroArea )
set( GTK_CIFRO_AREA_VERSION 0.1 )

include( FindPkgConfig )

#
# Default build type.
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release )
endif()

#
# Gtk2 support.
if( CIFRO_AREA_WITH_GTK2 )
  add_definitions( -DCIFRO_AREA_WITH_GTK2 )
endif()

#
# Architecture check.
if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
  set( CPU_ARCH x32 )
elseif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set( CPU_ARCH x64 )
else()
  message( FATAL_ERROR "Unsupported architecture." )
endif()

#
# Compilers options.
if( ${CMAKE_C_COMPILER_ID} STREQUAL GNU  )
  if( ${CPU_ARCH} STREQUAL x32 )
    set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=pentium4 -mfpmath=sse" )
    set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -march=pentium4 -mfpmath=sse" )
  endif()
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -Wall" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall" )
  add_definitions( -D_GCC_COMPILER )
elseif( ${CMAKE_C_COMPILER_ID} STREQUAL MSVC )
  add_definitions( -D_MSVC_COMPILER )
else()
  message( FATAL_ERROR "Unsupported compiler" )
endif()

# Additional tuning for GCC on UNIX platform
if( ${CMAKE_C_COMPILER_ID} STREQUAL GNU AND UNIX )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pthread" )
  set( PLATFORM "posix" )
endif()

# Additional tuning for GCC on Windows platform
if( ${CMAKE_C_COMPILER_ID} STREQUAL GNU AND WIN32 )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mthreads" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mthreads" )
  add_definitions( -D_WIN32_WINNT=0x0600 )
  set( PLATFORM "win32" )
endif()

# Additional tuning for MSVC on Windows platform
if( ${CMAKE_C_COMPILER_ID} STREQUAL MSVC AND WIN32 )
  add_definitions( -D_WIN32_WINNT=0x0600 )
  add_definitions( -D_CRT_SECURE_NO_WARNINGS )
  set( PLATFORM "win32" )
endif()

#
# Output directories.
if( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif()

if( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif()

#
# Dependencies
if( NOT CIFRO_AREA_WITH_GTK2 )
  pkg_check_modules( GTK REQUIRED gtk+-3.0 )
  set( GTK_VERSION 3.0 )
else()
  pkg_check_modules( GTK REQUIRED gtk+-2.0 )
  set( GTK_VERSION 2.0 )
endif()
add_definitions( ${GTK_CFLAGS} )
link_directories( ${GTK_LIBRARY_DIRS} )

#
# Include directories.
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/gtkcifroarea )

#
# Targets.
add_definitions( -DG_LOG_DOMAIN="GtkCifroArea" )
add_subdirectory( gtkcifroarea )
add_subdirectory( tests )

#
# Pkgconfig support.
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/gtkcifroarea/gtkcifroarea.pc.in ${CMAKE_CURRENT_SOURCE_DIR}/bin/gtkcifroarea-${GTK_VERSION}.pc @ONLY )
install( FILES bin/gtkcifroarea-${GTK_VERSION}.pc DESTINATION lib/pkgconfig PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ )
