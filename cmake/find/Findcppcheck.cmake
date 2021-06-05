file(TO_CMAKE_PATH "${CPPCHECK_ROOT_DIR}" CPPCHECK_ROOT_DIR)
set(CPPCHECK_ROOT_DIR
    "${CPPCHECK_ROOT_DIR}"
    CACHE PATH "Path to search for cppcheck")

# cppcheck app bundles on Mac OS X are GUI, we want command line only
set(_oldappbundlesettings ${CMAKE_FIND_APPBUNDLE})
set(CMAKE_FIND_APPBUNDLE NEVER)

# If we have a custom path, look there first.
if(CPPCHECK_EXECUTABLE AND NOT EXISTS "${CPPCHECK_EXECUTABLE}")
  set(CPPCHECK_EXECUTABLE
      "notfound"
      CACHE PATH "" FORCE)
endif()

find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)

# Restore original setting for appbundle finding
set(CMAKE_FIND_APPBUNDLE ${_oldappbundlesettings})

execute_process(
  COMMAND "${CPPCHECK_EXECUTABLE}" --version
  OUTPUT_VARIABLE CPPCHECK_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE ".*([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" CPPCHECK_VERSION
                     "${CPPCHECK_VERSION}")

if(CPPCHECK_EXECUTABLE)

  if(MSVC)
    set(CPPCHECK_TEMPLATE --template vs)
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CPPCHECK_TEMPLATE --template gcc)
  else()
    set(CPPCHECK_TEMPLATE --template gcc)
  endif()

  set(CPPCHECK_QUIET "--quiet")
  set(CPPCHECK_ALL "--enable=all")
  set(CPPCHECK_FORCE "--force")
  set(CPPCHECK_DATABASE "--project=")
  set(CPPCHECK_WERROR "--error-exitcode=255")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  cppcheck
  REQUIRED_VARS CPPCHECK_EXECUTABLE
  VERSION_VAR CPPCHECK_VERSION)

if(CPPCHECK_FOUND OR CPPCHECK_MARK_AS_ADVANCED)
  mark_as_advanced(CPPCHECK_ROOT_DIR)
endif()

mark_as_advanced(CPPCHECK_EXECUTABLE)
