# Findgenhtml.cmake - Locate the lcov executable and get its version. This module defines the
# following variables:
#
# - GENHTML_FOUND     - Boolean that indicates if genhtml was found.
# - GENHTML_EXECUTABLE - The path to the genhtml executable.
# - GENHTML_VERSION   - The version of the found genhtml.

include(FindPackageHandleStandardArgs)

# Try to find the genhtml executable in the system
find_program(GENHTML_EXECUTABLE NAMES genhtml)

if(GENHTML_EXECUTABLE)
  # Try to get the version of genhtml
  execute_process(
    COMMAND ${GENHTML_EXECUTABLE} --version
    OUTPUT_VARIABLE GENHTML_VERSION_OUTPUT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Extract the version from the output (expected format: "genhtml: LCOV version 2.3.1-1")
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" GENHTML_VERSION "${GENHTML_VERSION_OUTPUT}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(
  genhtml
  REQUIRED_VARS GENHTML_EXECUTABLE
  VERSION_VAR GENHTML_VERSION)

# Mark the GENHTML variables as advanced so they don't clutter the cache
mark_as_advanced(GENHTML_EXECUTABLE GENHTML_VERSION)
