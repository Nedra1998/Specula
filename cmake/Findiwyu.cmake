# Findinclude-what-you-use.cmake - Locate the include-what-you-use executable and get its version.
# This module defines the following variables:
#
# - IWYU_FOUND     - Boolean that indicates if include-what-you-use was found.
# - IWYU_EXECUTABLE - The path to the include-what-you-use executable.
# - IWYU_VERSION   - The version of the found include-what-you-use.

include(FindPackageHandleStandardArgs)

# Try to find the include-what-you-use executable in the system
find_program(IWYU_EXECUTABLE NAMES include-what-you-use iwyu)

if(IWYU_EXECUTABLE)
  # Try to get the version of include-what-you-use
  execute_process(
    COMMAND ${IWYU_EXECUTABLE} --version
    OUTPUT_VARIABLE IWYU_VERSION_OUTPUT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Extract the version from the output (expected format: "include-what-you-use x.y.z" or "iwyu
  # x.y.z")
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" IWYU_VERSION "${IWYU_VERSION_OUTPUT}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(
  iwyu
  REQUIRED_VARS IWYU_EXECUTABLE
  VERSION_VAR IWYU_VERSION)

# Mark the INCLUDE_WHAT_YOU_USE variables as advanced so they don't clutter the cache
mark_as_advanced(IWYU_EXECUTABLE IWYU_VERSION)
