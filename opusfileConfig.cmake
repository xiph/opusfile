include(CMakeFindDependencyMacro)
find_dependency(
  Ogg
  Opus
)

# TODO: include(...opusfileTargets-debug.cmake)?
include("${CMAKE_CURRENT_LIST_DIR}/opusfileTargets.cmake")
