# Findclang-format.cmake - Locate the clang-format executable and get its version. This module defines
# the following variables:
#
# - CLANG_FORMAT_FOUND     - Boolean that indicates if clang-format was found.
# - CLANG_FORMAT_EXECUTABLE - The path to the clang-format executable.
# - CLANG_FORMAT_VERSION   - The version of the found clang-format.

include(FindPackageHandleStandardArgs)

# Try to find the clang-format executable in the system
find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)

if(CLANG_FORMAT_EXECUTABLE)
  # Try to get the version of clang-format
  execute_process(
    COMMAND ${CLANG_FORMAT_EXECUTABLE} --version
    OUTPUT_VARIABLE CLANG_FORMAT_VERSION_OUTPUT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Extract the version from the output (expected format: "clang-format version x.y.z")
  string(REGEX MATCH "version [0-9]+\\.[0-9]+\\.[0-9]+" _clang_format_version_match "${CLANG_FORMAT_VERSION_OUTPUT}")
  string(REGEX REPLACE "version " "" CLANG_FORMAT_VERSION "${_clang_format_version_match}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(clang-format REQUIRED_VARS CLANG_FORMAT_EXECUTABLE VERSION_VAR CLANG_FORMAT_VERSION)

# Mark the CLANG_FORMAT variables as advanced so they don't clutter the cache
mark_as_advanced(CLANG_FORMAT_EXECUTABLE CLANG_FORMAT_VERSION)
