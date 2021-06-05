file(TO_CMAKE_PATH "${CMAKEFORMAT_ROOT_DIR}" CMAKEFORMAT_ROOT_DIR)
set(CMAKEFORMAT_ROOT_DIR
    "${CMAKEFORMAT_ROOT_DIR}"
    CACHE PATH "Path to search for cmake-format")

# cppcheck app bundles on Mac OS X are GUI, we want command line only
set(_oldappbundlesettings ${CMAKE_FIND_APPBUNDLE})
set(CMAKE_FIND_APPBUNDLE NEVER)

# If we have a custom path, look there first.
if(CMAKEFORMAT_EXECUTABLE AND NOT EXISTS "${CMAKEFORMAT_EXECUTABLE}")
  set(CMAKEFORMAT_EXECUTABLE
      "notfound"
      CACHE PATH "" FORCE)
endif()

find_program(CMAKEFORMAT_EXECUTABLE NAMES cmake-format)

# Restore original setting for appbundle finding
set(CMAKE_FIND_APPBUNDLE ${_oldappbundlesettings})

execute_process(
  COMMAND "${CMAKEFORMAT_EXECUTABLE}" --version
  OUTPUT_VARIABLE CMAKEFORMAT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE ".*([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" CMAKEFORMAT_VERSION
                     "${CMAKEFORMAT_VERSION}")

if(CMAKEFORMAT_EXECUTABLE)
  set(CMAKEFORMAT_INPLACE "--in-place")
  set(CMAKEFORMAT_DRYRUN "--check")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  cmakeformat
  REQUIRED_VARS CMAKEFORMAT_EXECUTABLE
  VERSION_VAR CMAKEFORMAT_VERSION)

if(CMAKEFORMAT_FOUND OR CMAKEFORMAT_MARK_AS_ADVANCED)
  mark_as_advanced(CMAKEFORMAT_ROOT_DIR)
endif()

mark_as_advanced(CMAKEFORMAT_EXECUTABLE)
