file(TO_CMAKE_PATH "${GCOVR_ROOT_DIR}" GCOVR_ROOT_DIR)
set(GCOVR_ROOT_DIR
    "${GCOVR_ROOT_DIR}"
    CACHE PATH "Path to search for gcovr")

# cppcheck app bundles on Mac OS X are GUI, we want command line only
set(_oldappbundlesettings ${CMAKE_FIND_APPBUNDLE})
set(CMAKE_FIND_APPBUNDLE NEVER)

# If we have a custom path, look there first.
if(GCOVR_EXECUTABLE AND NOT EXISTS "${GCOVR_EXECUTABLE}")
  set(GCOVR_EXECUTABLE
      "notfound"
      CACHE PATH "" FORCE)
endif()

find_program(GCOVR_EXECUTABLE NAMES gcovr)

# Restore original setting for appbundle finding
set(CMAKE_FIND_APPBUNDLE ${_oldappbundlesettings})

execute_process(
  COMMAND "${GCOVR_EXECUTABLE}" --version
  OUTPUT_VARIABLE GCOVR_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE ".*([0-9]+\\.[0-9]+).*" "\\1" GCOVR_VERSION
                     "${GCOVR_VERSION}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  gcovr
  REQUIRED_VARS GCOVR_EXECUTABLE
  VERSION_VAR GCOVR_VERSION)

if(GCOVR_FOUND OR GCOVR_MARK_AS_ADVANCED)
  mark_as_advanced(GCOVR_ROOT_DIR)
endif()

mark_as_advanced(GCOVR_EXECUTABLE)
