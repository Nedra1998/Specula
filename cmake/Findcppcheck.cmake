# Findcppcheck.cmake - Locate the cppcheck executable and get its version. This module defines the
# following variables:
#
# - CPPCHECK_FOUND     - Boolean that indicates if cppcheck was found.
# - CPPCHECK_EXECUTABLE - The path to the cppcheck executable.
# - CPPCHECK_VERSION   - The version of the found cppcheck.

include(FindPackageHandleStandardArgs)

# Try to find the cppcheck executable in the system
find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)

if(CPPCHECK_EXECUTABLE)
  # Try to get the version of cppcheck
  execute_process(
    COMMAND ${CPPCHECK_EXECUTABLE} --version
    OUTPUT_VARIABLE CPPCHECK_VERSION_OUTPUT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Extract the version from the output (expected format: "Cppcheck x.y.z")
  string(REGEX MATCH "Cppcheck [0-9]+\\.[0-9]+\\.[0-9]+" _cppcheck_version_match
               "${CPPCHECK_VERSION_OUTPUT}")
  string(REGEX REPLACE "Cppcheck " "" CPPCHECK_VERSION "${_cppcheck_version_match}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(
  cppcheck
  REQUIRED_VARS CPPCHECK_EXECUTABLE
  VERSION_VAR CPPCHECK_VERSION)

# Mark the CPPCHECK variables as advanced so they don't clutter the cache
mark_as_advanced(CPPCHECK_EXECUTABLE CPPCHECK_VERSION)
