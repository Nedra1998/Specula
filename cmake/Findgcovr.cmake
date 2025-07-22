# Findgcovr.cmake - Locate the gcovr executable and get its version. This module defines the
# following variables:
#
# - GCOVR_FOUND     - Boolean that indicates if gcovr was found.
# - GCOVR_EXECUTABLE - The path to the gcovr executable.
# - GCOVR_VERSION   - The version of the found gcovr.

include(FindPackageHandleStandardArgs)

# Try to find the gcovr executable in the system
find_program(GCOVR_EXECUTABLE NAMES gcovr)

if(GCOVR_EXECUTABLE)
  # Try to get the version of gcovr
  execute_process(
    COMMAND ${GCOVR_EXECUTABLE} --version
    OUTPUT_VARIABLE GCOVR_VERSION_OUTPUT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Extract the version from the output (expected format: "gcovr 8.4.dev0+gfe536afac.d20250125")
  string(REGEX MATCH "[0-9]+\\.[0-9]+" GCOVR_VERSION "${GCOVR_VERSION_OUTPUT}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(
  gcovr
  REQUIRED_VARS GCOVR_EXECUTABLE
  VERSION_VAR GCOVR_VERSION)

# Mark the GCOVR variables as advanced so they don't clutter the cache
mark_as_advanced(GCOVR_EXECUTABLE GCOVR_VERSION)
