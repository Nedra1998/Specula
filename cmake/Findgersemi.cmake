# Findgersemi.cmake - Locate the gersemi executable and get its version. This module defines
# the following variables:
#
# - GERSEMI_FOUND     - Boolean that indicates if gersemi was found.
# - GERSEMI_EXECUTABLE - The path to the gersemi executable.
# - GERSEMI_VERSION   - The version of the found gersemi.

include(FindPackageHandleStandardArgs)

# Try to find the gersemi executable in the system
find_program(GERSEMI_EXECUTABLE NAMES gersemi)

if(GERSEMI_EXECUTABLE)
  # Try to get the version of gersemi
  execute_process(
    COMMAND ${GERSEMI_EXECUTABLE} --version
    OUTPUT_VARIABLE GERSEMI_VERSION_OUTPUT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Extract the version from the output (expected format: "gersemi version x.y.z")
  string(REGEX MATCH "gersemi [0-9]+\\.[0-9]+\\.[0-9]+" _gersemi_version_match "${GERSEMI_VERSION_OUTPUT}")
  string(REGEX REPLACE "gersemi " "" GERSEMI_VERSION "${_gersemi_version_match}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(gersemi REQUIRED_VARS GERSEMI_EXECUTABLE VERSION_VAR GERSEMI_VERSION)

# Mark the GERSEMI variables as advanced so they don't clutter the cache
mark_as_advanced(GERSEMI_EXECUTABLE GERSEMI_VERSION)
