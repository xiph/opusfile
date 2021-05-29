cmake_minimum_required(VERSION 3.16)
project(opusfile
  VERSION 0.12  # TODO: retrieve version from git
  LANGUAGES C
)

# TODO: build documentation?
option(OP_DISABLE_HTTP "Disable HTTP support" OFF)
option(OP_DISABLE_FLOAT_API "Disable floating-point API" OFF)
option(OP_FIXED_POINT "Enable fixed-point calculation" OFF)
option(OP_ENABLE_ASSERTIONS "Enable assertions in code" OFF)
option(OP_DISABLE_EXAMPLES "Do not build example applications" OFF)

include(GNUInstallDirs)

find_package(Ogg REQUIRED)
find_package(Opus REQUIRED)

include(CMakePushCheckState)
include(CheckSymbolExists)
cmake_push_check_state(RESET)
list(APPEND CMAKE_REQUIRED_LIBRARIES "m")
check_symbol_exists(lrintf "math.h" OP_HAVE_LRINTF)
cmake_pop_check_state()

# TODO: shared libraries via BUILD_SHARED_LIBS does not work
add_library(opusfile
  "${CMAKE_CURRENT_SOURCE_DIR}/include/opusfile.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/info.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/internal.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/internal.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/opusfile.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/stream.c"
)
add_library(opusfile::opusfile ALIAS opusfile)
target_include_directories(opusfile
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(opusfile
  PUBLIC
    Ogg::ogg
    Opus::opus
)
target_compile_options(opusfile
  PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/wd4267>
    $<$<C_COMPILER_ID:MSVC>:/wd4244>
    $<$<C_COMPILER_ID:MSVC>:/wd4090>
    $<$<C_COMPILER_ID:Clang,GNU>:-std=c89>
    $<$<C_COMPILER_ID:Clang,GNU>:-pedantic>
    $<$<C_COMPILER_ID:Clang,GNU>:-Wall>
    $<$<C_COMPILER_ID:Clang,GNU>:-Wextra>
    $<$<C_COMPILER_ID:Clang,GNU>:-Wno-parentheses>
    $<$<C_COMPILER_ID:Clang,GNU>:-Wno-long-long>
    $<$<C_COMPILER_ID:Clang,GNU>:-fvisibility=hidden>
)
target_compile_definitions(opusfile
  PRIVATE
    $<$<BOOL:${OP_DISABLE_FLOAT_API}>:OP_DISABLE_FLOAT_API>
    $<$<BOOL:${OP_FIXED_POINT}>:OP_FIXED_POINT>
    $<$<BOOL:${OP_ENABLE_ASSERTIONS}>:OP_ENABLE_ASSERTIONS>
    $<$<BOOL:${OP_HAVE_LRINTF}>:OP_HAVE_LRINTF>
)
install(TARGETS opusfile
  EXPORT opusfileTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

if(NOT OP_DISABLE_HTTP)
  find_package(OpenSSL REQUIRED)

  include(CheckIncludeFile)
  include(CheckCSourceCompiles)
  cmake_push_check_state(RESET)
  if(WIN32)
    check_include_file("winsock2.h" OP_HAVE_WINSOCK2_H)
    if(NOT OP_HAVE_WINSOCK2_H)
      message(FATAL_ERROR "HTTP support requires a Winsock socket library")
    endif()
  else()
    check_include_file("sys/socket.h" OP_HAVE_SYS_SOCKET_H)
    if(NOT OP_HAVE_SYS_SOCKET_H)
      message(FATAL_ERROR "HTTP support requires a POSIX socket library")
    endif()
  endif()
  check_c_source_compiles(
    "#include <time.h>
    int main(void)
    {
      struct timespec ts;
      return clock_gettime(CLOCK_REALTIME, &ts);
    }"
    OP_HAVE_CLOCK_GETTIME
  )
  if(NOT OP_HAVE_CLOCK_GETTIME)
    check_symbol_exists(ftime "sys/timeb.h" OP_HAVE_FTIME)
    if(NOT OP_HAVE_FTIME)
      message(FATAL_ERROR "HTTP support requires either clock_gettime() or ftime()")
    endif()
  endif()
  cmake_pop_check_state()

  add_library(opusurl
    "${CMAKE_CURRENT_SOURCE_DIR}/src/http.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/internal.c"  # TODO: duplicated symbols because already defined in opusfile?
    "${CMAKE_CURRENT_SOURCE_DIR}/src/internal.h"
  )
  add_library(opusfile::opusurl ALIAS opusurl)
  if(WIN32)
    target_sources(opusurl PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/src/wincerts.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/winerrno.h"
    )
  endif()
  target_include_directories(opusurl
    PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/include"
    INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
  target_compile_definitions(opusurl
    PRIVATE
      $<$<BOOL:${OP_DISABLE_FLOAT_API}>:OP_DISABLE_FLOAT_API>
      $<$<BOOL:${OP_FIXED_POINT}>:OP_FIXED_POINT>
      $<$<BOOL:${OP_ENABLE_ASSERTIONS}>:OP_ENABLE_ASSERTIONS>
      $<$<BOOL:${OP_HAVE_LRINTF}>:OP_HAVE_LRINTF>
      $<$<BOOL:${OP_HAVE_CLOCK_GETTIME}>:OP_HAVE_CLOCK_GETTIME>
      $<$<BOOL:${OP_HAVE_FTIME}>:OP_HAVE_FTIME>
      OP_ENABLE_HTTP
  )
  target_link_libraries(opusurl
    PRIVATE
      opusfile
      OpenSSL::SSL
      $<$<C_COMPILER_ID:MSVC>:ws2_32>
      $<$<C_COMPILER_ID:MSVC>:crypt32>
  )
  target_compile_options(opusurl
    PRIVATE
      $<$<C_COMPILER_ID:MSVC>:/wd4267>
      $<$<C_COMPILER_ID:MSVC>:/wd4244>
      $<$<C_COMPILER_ID:MSVC>:/wd4090>
      $<$<C_COMPILER_ID:Clang,GNU>:-std=c89>
      $<$<C_COMPILER_ID:Clang,GNU>:-pedantic>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wall>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wextra>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wno-parentheses>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wno-long-long>
      $<$<C_COMPILER_ID:Clang,GNU>:-fvisibility=hidden>
  )
  install(TARGETS opusurl
    EXPORT opusfileTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  )
endif()

if(NOT OP_DISABLE_EXAMPLES)
  add_executable(opusfile_example
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/opusfile_example.c"
  )
  add_executable(opusfile::opusfile_example ALIAS opusfile_example)
  if(WIN32)
    target_sources(opusfile_example PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/examples/win32utf8.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/examples/win32utf8.h"
    )
  endif()
  target_include_directories(opusfile_example
    PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/examples"
  )
  target_link_libraries(opusfile_example
    PRIVATE
      opusfile
      opusurl
  )
  target_compile_options(opusfile_example
    PRIVATE
      $<$<C_COMPILER_ID:MSVC>:/wd4267>
      $<$<C_COMPILER_ID:MSVC>:/wd4244>
      $<$<C_COMPILER_ID:MSVC>:/wd4090>
      $<$<C_COMPILER_ID:Clang,GNU>:-std=c89>
      $<$<C_COMPILER_ID:Clang,GNU>:-pedantic>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wall>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wextra>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wno-parentheses>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wno-long-long>
      $<$<C_COMPILER_ID:Clang,GNU>:-fvisibility=hidden>
  )

  add_executable(seeking_example
    "${CMAKE_CURRENT_SOURCE_DIR}/examples/seeking_example.c"
  )
  add_executable(opusfile::seeking_example ALIAS seeking_example)
  if(WIN32)
    target_sources(seeking_example PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/examples/win32utf8.c"
      "${CMAKE_CURRENT_SOURCE_DIR}/examples/win32utf8.h"
    )
  endif()
  target_include_directories(seeking_example
    PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/examples"
  )
  target_link_libraries(seeking_example
    PRIVATE
      opusfile
      opusurl
  )
  target_compile_options(seeking_example
    PRIVATE
      $<$<C_COMPILER_ID:MSVC>:/wd4267>
      $<$<C_COMPILER_ID:MSVC>:/wd4244>
      $<$<C_COMPILER_ID:MSVC>:/wd4090>
      $<$<C_COMPILER_ID:Clang,GNU>:-std=c89>
      $<$<C_COMPILER_ID:Clang,GNU>:-pedantic>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wall>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wextra>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wno-parentheses>
      $<$<C_COMPILER_ID:Clang,GNU>:-Wno-long-long>
      $<$<C_COMPILER_ID:Clang,GNU>:-fvisibility=hidden>
  )
endif()

install(EXPORT opusfileTargets
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/opusfile"
  NAMESPACE opusfile::
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "opusfileConfigVersion.cmake"
  VERSION "${PACKAGE_VERSION}"
  COMPATIBILITY AnyNewerVersion
)
install(
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/opusfileConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/opusfileConfigVersion.cmake"
  DESTINATION
    "${CMAKE_INSTALL_LIBDIR}/cmake/opusfile"
)
