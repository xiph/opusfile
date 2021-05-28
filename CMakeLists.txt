cmake_minimum_required(VERSION 3.16)
project(opusfile
  VERSION 0.12  # TODO: retrieve version from git
  LANGUAGES C
)

# TODO: port autotools options exactly
option(BUILD_OPUSURL "Build opusurl" OFF)
option(OP_DISABLE_FLOAT_API "Disable floating-point API" OFF)
option(OP_FIXED_POINT "Enable fixed-point calculation" OFF)

include(GNUInstallDirs)

find_package(Ogg REQUIRED)
find_package(Opus REQUIRED)

include(CMakePushCheckState)
include(CheckSymbolExists)
cmake_push_check_state(RESET)
list(APPEND CMAKE_REQUIRED_LIBRARIES "m")
check_symbol_exists(lrintf "math.h" HAVE_LRINTF)
# TODO: OP_HAVE_CLOCK_GETTIME
cmake_pop_check_state()

# TODO: shared libraries via BUILD_SHARED_LIBS does not work
add_library(opusfile
  "${CMAKE_CURRENT_SOURCE_DIR}/src/info.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/internal.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/opusfile.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/stream.c"
)
add_library(opusfile::opusfile ALIAS opusfile)
target_include_directories(opusfile
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
  INTERFACE
    $<BUILD_INTERFACE:"${CMAKE_CURRENT_BINARY_DIR}/include">
    $<INSTALL_INTERFACE:"${CMAKE_INSTALL_INCLUDEDIR}">
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
    -std=c89
    -pedantic
    -Wall
    -Wextra
    -Wno-parentheses
    -Wno-long-long
)
target_compile_definitions(opusfile
  PRIVATE
    $<${HAVE_LRINTF}:OP_HAVE_LRINTF>
)
install(TARGETS opusfile
  EXPORT opusfileTargets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

if(BUILD_OPUSURL)
  find_package(OpenSSL REQUIRED)

  add_library(opusurl
    "${CMAKE_CURRENT_SOURCE_DIR}/src/http.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/wincerts.c"
  )
  add_library(opusfile::opusurl ALIAS opusurl)
  target_include_directories(opusurl
    PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}/include"
    INTERFACE
      $<BUILD_INTERFACE:"${CMAKE_CURRENT_BINARY_DIR}/include">
      $<INSTALL_INTERFACE:"${CMAKE_INSTALL_INCLUDEDIR}">
  )
  target_compile_definitions(opusurl
    PRIVATE
      OP_ENABLE_HTTP
  )
  target_link_libraries(opusurl
    PRIVATE
      opusfile
      OpenSSL::SSL
      ws2_32.lib
      crypt32.lib
  )
  target_compile_options(opusurl
    PRIVATE
      $<$<C_COMPILER_ID:MSVC>:/wd4267>
      $<$<C_COMPILER_ID:MSVC>:/wd4244>
      $<$<C_COMPILER_ID:MSVC>:/wd4090>
      -std=c89
      -pedantic
      -Wall
      -Wextra
      -Wno-parentheses
      -Wno-long-long
  )
  install(TARGETS opusurl
    EXPORT opusfileTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
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
