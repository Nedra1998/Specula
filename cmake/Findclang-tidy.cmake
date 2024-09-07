# Findclang-tidy.cmake - Locate the clang-tidy executable and get its version. This module defines
# the following variables:
#
# - CLANG_TIDY_FOUND     - Boolean that indicates if clang-tidy was found.
# - CLANG_TIDY_EXECUTABLE - The path to the clang-tidy executable.
# - CLANG_TIDY_VERSION   - The version of the found clang-tidy.

include(FindPackageHandleStandardArgs)

# Try to find the clang-tidy executable in the system
find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)

if(CLANG_TIDY_EXECUTABLE)
  # Try to get the version of clang-tidy
  execute_process(
    COMMAND ${CLANG_TIDY_EXECUTABLE} --version
    OUTPUT_VARIABLE CLANG_TIDY_VERSION_OUTPUT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Extract the version from the output (expected format: "clang-tidy version x.y.z")
  string(REGEX MATCH "version [0-9]+\\.[0-9]+\\.[0-9]+" _clang_tidy_version_match
               "${CLANG_TIDY_VERSION_OUTPUT}")
  string(REGEX REPLACE "version " "" CLANG_TIDY_VERSION "${_clang_tidy_version_match}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(
  clang-tidy
  REQUIRED_VARS CLANG_TIDY_EXECUTABLE
  VERSION_VAR CLANG_TIDY_VERSION)

# Mark the CLANG_TIDY variables as advanced so they don't clutter the cache
mark_as_advanced(CLANG_TIDY_EXECUTABLE CLANG_TIDY_VERSION)
