file(TO_CMAKE_PATH "${DOT_ROOT_DIR}" DOT_ROOT_DIR)
set(DOT_ROOT_DIR
    "${DOT_ROOT_DIR}"
    CACHE PATH "Path to search for dot")

if(DOT_ROOT_DIR)
  find_program(
    DOT_EXECUTABLE
    NAMES dot
    PATHS "${DOT_ROOT_DIR}"
    NO_DEFAULT_PATH)
endif()

find_program(DOT_EXECUTABLE NAMES dot)

if(DOT_EXECUTABLE)
  execute_process(
    COMMAND "${DOT_EXECUTABLE}" --version
    ERROR_VARIABLE DOT_VERSION
    ERROR_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE ".* ([0-9]+\\.([0-9]+\\.[0-9]+)?).*" "\\1" DOT_VERSION "${DOT_VERSION}")
endif()

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(
  Dot
  REQUIRED_VARS DOT_EXECUTABLE
  VERSION_VAR DOT_VERSION)

if(DOT_FOUND OR DOT_MARK_AS_ADVANCED)
  mark_as_advanced(DOT_ROOT_DIR)
endif()

mark_as_advanced(DOT_EXECUTABLE)
