file(TO_CMAKE_PATH "${LLVMCOV_ROOT_DIR}" LLVMCOV_ROOT_DIR)
set(LLVMCOV_ROOT_DIR
    "${LLVMCOV_ROOT_DIR}"
    CACHE PATH "Path to search for llvm-cov")

# cppcheck app bundles on Mac OS X are GUI, we want command line only
set(_oldappbundlesettings ${CMAKE_FIND_APPBUNDLE})
set(CMAKE_FIND_APPBUNDLE NEVER)

# If we have a custom path, look there first.
if(LLVMCOV_EXECUTABLE AND NOT EXISTS "${LLVMCOV_EXECUTABLE}")
  set(LLVMCOV_EXECUTABLE
      "notfound"
      CACHE PATH FORCE "")
endif()

find_program(LLVMCOV_EXECUTABLE NAMES llvm-cov)
find_program(LLVMPROFDATA_EXECUTABLE NAMES llvm-profdata)

# Restore original setting for appbundle finding
set(CMAKE_FIND_APPBUNDLE ${_oldappbundlesettings})

execute_process(
  COMMAND "${LLVMCOV_EXECUTABLE}" --version
  OUTPUT_VARIABLE LLVMCOV_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE ".* ([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" LLVMCOV_VERSION
                     "${LLVMCOV_VERSION}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  llvmcov
  REQUIRED_VARS LLVMCOV_EXECUTABLE LLVMPROFDATA_EXECUTABLE
  VERSION_VAR LLVMCOV_VERSION)

if(LLVMCOV_FOUND OR LLVMCOV_MARK_AS_ADVANCED)
  mark_as_advanced(LLVMCOV_ROOT_DIR)
endif()

mark_as_advanced(LLVMCOV_EXECUTABLE)
