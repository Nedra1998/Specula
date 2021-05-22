file(TO_CMAKE_PATH "${CLANGFORMAT_ROOT_DIR}" CLANGFORMAT_ROOT_DIR)
set(CLANGFORMAT_ROOT_DIR
    "${CLANGFORMAT_ROOT_DIR}"
    CACHE PATH "Path to search for clangformat")

# cppcheck app bundles on Mac OS X are GUI, we want command line only
set(_oldappbundlesettings ${CMAKE_FIND_APPBUNDLE})
set(CMAKE_FIND_APPBUNDLE NEVER)

# If we have a custom path, look there first.
if(CLANGFORMAT_EXECUTABLE AND NOT EXISTS "${CLANGFORMAT_EXECUTABLE}")
  set(CLANGFORMAT_EXECUTABLE
      "notfound"
      CACHE PATH FORCE "")
endif()

find_program(CLANGFORMAT_EXECUTABLE NAMES clang-format)

# Restore original setting for appbundle finding
set(CMAKE_FIND_APPBUNDLE ${_oldappbundlesettings})

execute_process(
  COMMAND "${CLANGFORMAT_EXECUTABLE}" --version
  OUTPUT_VARIABLE CLANGFORMAT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE ".* ([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1"
                     CLANGFORMAT_VERSION "${CLANGFORMAT_VERSION}")

if(CLANGFORMAT_EXECUTABLE)
  set(CLANGFORMAT_INPLACE "-i")

  if(CLANGFORMAT_VERSION VERSION_LESS "10.0.0")
    set(CLANGFORMAT_DRYRUN)
    set(CLANGFORMAT_WERROR)
  else()
    set(CLANGFORMAT_DRYRUN "--dry-run")
    set(CLANGFORMAT_WERROR "-Werror")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  clangformat
  REQUIRED_VARS CLANGFORMAT_EXECUTABLE
  VERSION_VAR CLANGFORMAT_VERSION)

if(CLANGFORMAT_FOUND OR CLANGFORMAT_MARK_AS_ADVANCED)
  mark_as_advanced(CLANGFORMAT_ROOT_DIR)
endif()

mark_as_advanced(CLANGFORMAT_EXECUTABLE)
