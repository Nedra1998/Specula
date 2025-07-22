# Findllvmcov.cmake - Locate the gcov executable and get its version. This module defines the
# following variables:
#
# - LLVMCOV_FOUND     - Boolean that indicates if llvm-cov was found.
# - LLVMCOV_EXECUTABLE - The path to the llvm-cov executable.
# - LLVMCOV_VERSION   - The version of the found llvm-cov.

include(FindPackageHandleStandardArgs)

# Try to find the llvm-cov executable in the system
find_program(LLVMCOV_EXECUTABLE NAMES llvm-cov)

if(LLVMCOV_EXECUTABLE)
  # Try to get the version of llvm-cov
  execute_process(
    COMMAND ${LLVMCOV_EXECUTABLE} --version
    OUTPUT_VARIABLE LLVMCOV_VERSION_OUTPUT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Extract the version from the output (expected format: "LLVM version 20.1.8")
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" LLVMCOV_VERSION "${LLVMCOV_VERSION_OUTPUT}")
endif()

# Use find_package_handle_standard_args to handle the result
find_package_handle_standard_args(
  llvmcov
  REQUIRED_VARS LLVMCOV_EXECUTABLE
  VERSION_VAR LLVMCOV_VERSION)

# Mark the LLVMCOV variables as advanced so they don't clutter the cache
mark_as_advanced(LLVMCOV_EXECUTABLE LLVMCOV_VERSION)
