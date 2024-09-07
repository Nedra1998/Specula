# Findsccache.cmake - Locate the sccache executable and get its version. This module defines the
# following variables:
#
# - SCCACHE_FOUND     - Boolean that indicates if sccache was found.
# - SCCACHE_EXECUTABLE - The path to the sccache executable.
# - SCCACHE_VERSION   - The version of the found sccache.

include(FindPackageHandleStandardArgs)

# Try to find sccache in the system
find_program(SCCACHE_EXECUTABLE NAMES sccache)

if(SCCACHE_EXECUTABLE)
  # Try to get the version of sccache
  execute_process(
    COMMAND ${SCCACHE_EXECUTABLE} --version
    OUTPUT_VARIABLE SCCACHE_VERSION_OUTPUT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Extract the version from the output (expected format: "sccache x.y.z")
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" SCCACHE_VERSION "${SCCACHE_VERSION_OUTPUT}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(
  sccache
  REQUIRED_VARS SCCACHE_EXECUTABLE
  VERSION_VAR SCCACHE_VERSION)

# Mark the SCCACHE variables as advanced so they don't clutter the cache
mark_as_advanced(SCCACHE_EXECUTABLE SCCACHE_VERSION)
