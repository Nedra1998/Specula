file(TO_CMAKE_PATH "${CMAKELINT_ROOT_DIR}" CMAKE_LINT_ROOT_DIR)
set(CMAKELINT_ROOT_DIR
    "${CMAKELINT_ROOT_DIR}"
    CACHE PATH "Path to search for cmake-lint")

if(CMAKELINT_ROOT_DIR)
  find_program(
    CMAKELINT_EXECUTABLE
    NAMES cmake-lint
    PATHS "${CMAKELINT_ROOT_DIR}"
    NO_DEFAULT_PATH)
endif()

find_program(CMAKELINT_EXECUTABLE NAMES cmake-lint)

if(CMAKELINT_EXECUTABLE)
  execute_process(
    COMMAND "${CMAKELINT_EXECUTABLE}" --version
    OUTPUT_VARIABLE CMAKELINT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE "([0-9]+\\.([0-9]+\\.[0-9]+)?)" "\\1" CMAKELINT_VERSION
                       "${CMAKELINT_VERSION}")
endif()

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(
  CmakeLint
  REQUIRED_VARS CMAKELINT_EXECUTABLE
  VERSION_VAR CMAKELINT_VERSION)

if(CMAKELINT_FOUND OR CMAKE_LINT_MARK_AS_ADVANCED)
  mark_as_advanced(CMAKELINT_ROOT_DIR)
endif()

mark_as_advanced(CMAKELINT_EXECUTABLE)
