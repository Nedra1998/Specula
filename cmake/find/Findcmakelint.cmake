file(TO_CMAKE_PATH "${CMAKELINT_ROOT_DIR}" CMAKELINT_ROOT_DIR)
set(CMAKELINT_ROOT_DIR
    "${CMAKELINT_ROOT_DIR}"
    CACHE PATH "Path to search for cmake-lint")

# cppcheck app bundles on Mac OS X are GUI, we want command line only
set(_oldappbundlesettings ${CMAKE_FIND_APPBUNDLE})
set(CMAKE_FIND_APPBUNDLE NEVER)

# If we have a custom path, look there first.
if(CMAKELINT_EXECUTABLE AND NOT EXISTS "${CMAKELINT_EXECUTABLE}")
  set(CMAKELINT_EXECUTABLE
      "notfound"
      CACHE PATH "" FORCE)
endif()

find_program(CMAKELINT_EXECUTABLE NAMES cmake-lint)

# Restore original setting for appbundle finding
set(CMAKE_FIND_APPBUNDLE ${_oldappbundlesettings})

execute_process(
  COMMAND "${CMAKELINT_EXECUTABLE}" --version
  OUTPUT_VARIABLE CMAKELINT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE ".*([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" CMAKELINT_VERSION
                     "${CMAKELINT_VERSION}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  cmakelint
  REQUIRED_VARS CMAKELINT_EXECUTABLE
  VERSION_VAR CMAKELINT_VERSION)

if(CMAKELINT_FOUND OR CMAKELINT_MARK_AS_ADVANCED)
  mark_as_advanced(CMAKELINT_ROOT_DIR)
endif()

mark_as_advanced(CMAKELINT_EXECUTABLE)
