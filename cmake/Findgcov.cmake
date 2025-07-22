# Findgcov.cmake - Locate the gcov executable and get its version. This module defines the
# following variables:
#
# - GCOV_FOUND     - Boolean that indicates if gcov was found.
# - GCOV_EXECUTABLE - The path to the gcov executable.
# - GCOV_VERSION   - The version of the found gcov.

include(FindPackageHandleStandardArgs)

# Try to find the gcov executable in the system
find_program(GCOV_EXECUTABLE NAMES gcov)

if(GCOV_EXECUTABLE)
  # Try to get the version of gcov
  execute_process(
    COMMAND ${GCOV_EXECUTABLE} --version
    OUTPUT_VARIABLE GCOV_VERSION_OUTPUT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Extract the version from the output (expected format: "gcov (GCC) 15.1.1 20250425")
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" GCOV_VERSION "${GCOV_VERSION_OUTPUT}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(
  gcov
  REQUIRED_VARS GCOV_EXECUTABLE
  VERSION_VAR GCOV_VERSION)

# Mark the GCOV variables as advanced so they don't clutter the cache
mark_as_advanced(GCOV_EXECUTABLE GCOV_VERSION)
