file(TO_CMAKE_PATH "${SHELLCHECK_ROOT_DIR}" SHELLCHECK_ROOT_DIR)
set(SHELLCHECK_ROOT_DIR
    "${SHELLCHECK_ROOT_DIR}"
    CACHE PATH "Path to search for shellcheck")

# shellcheck app bundles on Mac OS X are GUI, we want command line only
set(_oldappbundlesettings ${CMAKE_FIND_APPBUNDLE})
set(CMAKE_FIND_APPBUNDLE NEVER)

# If we have a custom path, look there first.
if(SHELLCHECK_EXECUTABLE AND NOT EXISTS "${SHELLCHECK_EXECUTABLE}")
  set(SHELLCHECK_EXECUTABLE
      "notfound"
      CACHE PATH FORCE "")
endif()

find_program(SHELLCHECK_EXECUTABLE NAMES shellcheck)

# Restore original setting for appbundle finding
set(CMAKE_FIND_APPBUNDLE ${_oldappbundlesettings})

execute_process(
  COMMAND "${SHELLCHECK_EXECUTABLE}" --version
  OUTPUT_VARIABLE SHELLCHECK_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE ".*([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" SHELLCHECK_VERSION
                     "${SHELLCHECK_VERSION}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  shellcheck
  REQUIRED_VARS SHELLCHECK_EXECUTABLE
  VERSION_VAR SHELLCHECK_VERSION)

if(SHELLCHECK_FOUND OR SHELLCHECK_MARK_AS_ADVANCED)
  mark_as_advanced(SHELLCHECK_ROOT_DIR)
endif()

mark_as_advanced(SHELLCHECK_EXECUTABLE)
