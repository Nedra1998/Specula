if(__gernerate_version_header)
  return()
endif()
set(__gernerate_version_header YES)

find_package(Git QUIET)
set(_version_header_template
    "${CMAKE_CURRENT_LIST_DIR}/GenerateVersionHeader.hpp.in")

# Generates a C++ header file containing the CMake project version information,
# along with the current git commit, to be included in the project.
function(generate_version_header)
  set(OPTIONS)
  set(SINGLE_VALUE_KEYWORDS OUTPUT_DIR)
  set(MULTI_VALUE_KEYWORDS)
  cmake_parse_arguments(ver "${OPTIONS}" "${SINGLE_VALUE_KEYWORDS}"
                        "${MULTI_VALUE_KEYWORDS}" ${ARGN})

  string(MAKE_C_IDENTIFIER ${PROJECT_NAME} _id)
  string(TOUPPER ${_id} PROJECT_UPPER)
  string(TOLOWER ${_id} PROJECT_LOWER)

  if(GIT_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE PROJECT_VERSION_BUILD
      RESULT_VARIABLE git_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()

  if(NOT ver_OUTPUT_DIR)
    set(ver_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  if(NOT EXISTS ${_version_header_template})
    message(
      FATAL_ERROR
        "Missing template file ${_version_header_template} - should be \
        alongside GenerateVersionHeader.cmake")
  endif()
  configure_file("${_version_header_template}" "${ver_OUTPUT_DIR}/version.hpp")
endfunction()
