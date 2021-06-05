file(TO_CMAKE_PATH "${CLANGTIDY_ROOT_DIR}" CLANGTIDY_ROOT_DIR)
set(CLANGTIDY_ROOT_DIR
    "${CLANGTIDY_ROOT_DIR}"
    CACHE PATH "Path to search for clangtidy")

# cppcheck app bundles on Mac OS X are GUI, we want command line only
set(_oldappbundlesettings ${CMAKE_FIND_APPBUNDLE})
set(CMAKE_FIND_APPBUNDLE NEVER)

# If we have a custom path, look there first.
if(CLANGTIDY_EXECUTABLE AND NOT EXISTS "${CLANGTIDY_EXECUTABLE}")
  set(CLANGTIDY_EXECUTABLE
      "notfound"
      CACHE PATH "" FORCE)
endif()

find_program(CLANGTIDY_EXECUTABLE NAMES clang-tidy)

# Restore original setting for appbundle finding
set(CMAKE_FIND_APPBUNDLE ${_oldappbundlesettings})

execute_process(
  COMMAND "${CLANGTIDY_EXECUTABLE}" --version
  OUTPUT_VARIABLE CLANGTIDY_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE ".* ([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" CLANGTIDY_VERSION
                     "${CLANGTIDY_VERSION}")

if(CLANGTIDY_EXECUTABLE)
  set(CLANGTIDY_DATABASE "-p")

  if(CLANGTIDY_VERSION VERSION_LESS "9.0.0")
    set(CLANGTIDY_WERROR "-warnings-as-errors=*")
    set(CLANGTIDY_FIX "-fix-errors")
  else()
    set(CLANGTIDY_WERROR "--warnings-as-errors=*")
    set(CLANGTIDY_FIX "--fix-errors")
  endif()

  if(CLANGTIDY_VERSION VERSION_LESS "5.0.0")
    set(CLANGTIDY_QUIET)
  elseif(CLANGTIDY_VERSION VERSION_LESS "9.0.0")
    set(CLANGTIDY_QUIET "-quiet")
  else()
    set(CLANGTIDY_QUIET "--quiet")
  endif()

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  clangtidy
  REQUIRED_VARS CLANGTIDY_EXECUTABLE CLANGTIDY_WERROR
  VERSION_VAR CLANGTIDY_VERSION)

if(CLANGTIDY_FOUND OR CLANGTIDY_MARK_AS_ADVANCED)
  mark_as_advanced(CLANGTIDY_ROOT_DIR)
endif()

mark_as_advanced(CLANGTIDY_EXECUTABLE)
