file(TO_CMAKE_PATH "${CPPCHECK_ROOT_DIR}" CPPCHECK_ROOT_DIR)
set(CPPCHECK_ROOT_DIR
    "${CPPCHECK_ROOT_DIR}"
    CACHE PATH "Path to search for cppcheck")

set(_oldappbundlesetting ${CMAKE_FIND_APPBUNDLE})
set(CMAKE_FIND_APPBUNDLE NEVER)

if(CPPCHECK_ROOT_DIR)
  find_program(
    CPPCHECK_EXECUTABLE
    NAMES cppcheck cli
    PATHS "${CPPCHECK_ROOT_DIR}"
    PATH_SUFFIXES cli
    NO_DEFAULT_PATH)
endif()

find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)

set(CMAKE_FIND_APPBUNDLE ${_oldappbundlesetting})

if(CPPCHECK_EXECUTABLE)
  execute_process(
    COMMAND "${CPPCHECK_EXECUTABLE}" --version
    OUTPUT_VARIABLE CPPCHECK_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE ".* ([0-9]\\.([0-9]\\.[0-9])?)" "\\1" CPPCHECK_VERSION "${CPPCHECK_VERSION}")
endif()

include(FindPackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(
  Cppcheck
  REQUIRED_VARS CPPCHECK_EXECUTABLE
  VERSION_VAR CPPCHECK_VERSION)

if(CPPCHECK_FOUND OR CPPCHECK_MARK_AS_ADVANCED)
  mark_as_advanced(CPPCHECK_ROOT_DIR)
endif()

mark_as_advanced(CPPCHECK_EXECUTABLE)
