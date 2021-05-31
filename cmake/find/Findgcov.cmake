file(TO_CMAKE_PATH "${GCOV_ROOT_DIR}" GCOV_ROOT_DIR)
set(GCOV_ROOT_DIR
    "${GCOV_ROOT_DIR}"
    CACHE PATH "Path to search for gcov")

# cppcheck app bundles on Mac OS X are GUI, we want command line only
set(_oldappbundlesettings ${CMAKE_FIND_APPBUNDLE})
set(CMAKE_FIND_APPBUNDLE NEVER)

# If we have a custom path, look there first.
if(GCOV_EXECUTABLE AND NOT EXISTS "${GCOV_EXECUTABLE}")
  set(GCOV_EXECUTABLE
      "notfound"
      CACHE PATH FORCE "")
endif()

find_program(GCOV_EXECUTABLE NAMES gcov)

# Restore original setting for appbundle finding
set(CMAKE_FIND_APPBUNDLE ${_oldappbundlesettings})

execute_process(
  COMMAND "${GCOV_EXECUTABLE}" --version
  OUTPUT_VARIABLE GCOV_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE ".*([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" GCOV_VERSION
                     "${GCOV_VERSION}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  gcov
  REQUIRED_VARS GCOV_EXECUTABLE
  VERSION_VAR GCOV_VERSION)

if(GCOV_FOUND OR GCOV_MARK_AS_ADVANCED)
  mark_as_advanced(GCOV_ROOT_DIR)
endif()

mark_as_advanced(GCOV_EXECUTABLE)
