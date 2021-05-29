include(CMakeFindDependencyMacro)
find_dependency(
  Ogg
  Opus
)

include("${CMAKE_CURRENT_LIST_DIR}/opusfileTargets.cmake")

# Load information for each installed configuration.
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/opusfileTargets-*.cmake")
foreach(f ${CONFIG_FILES})
  include(${f})
endforeach()
