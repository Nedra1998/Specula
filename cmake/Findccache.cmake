# Findccache.cmake - Locate the ccache executable and get its version. This module defines the
# following variables:
#
# - CCACHE_FOUND      - Boolean that indicates if ccache was found.
# - CCACHE_EXECUTABLE - The path to the ccache executable.
# - CCACHE_VERSION    - The version of the found ccache.

include(FindPackageHandleStandardArgs)

# Try to find ccache in the system
find_program(CCACHE_EXECUTABLE NAMES ccache)

if(CCACHE_EXECUTABLE)
  # Try to get the version of ccache
  execute_process(
    COMMAND ${CCACHE_EXECUTABLE} --version
    OUTPUT_VARIABLE CCACHE_VERSION_OUTPUT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Extract the version from the output (expected format: "ccache version x.y.z")
  string(REGEX MATCH "version [0-9]+\\.[0-9]+\\.[0-9]+" _ccache_version_match
               "${CCACHE_VERSION_OUTPUT}")
  string(REGEX REPLACE "version " "" CCACHE_VERSION "${_ccache_version_match}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(
  ccache
  REQUIRED_VARS CCACHE_EXECUTABLE
  VERSION_VAR CCACHE_VERSION)

# Mark the CCACHE variables as advanced so they don't clutter the cache
mark_as_advanced(CCACHE_EXECUTABLE CCACHE_VERSION)
