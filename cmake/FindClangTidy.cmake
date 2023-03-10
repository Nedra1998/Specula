file(TO_CMAKE_PATH "${CLANGTIDY_ROOT_DIR}" CLANGTIDY_ROOT_DIR)
set(CLANGTIDY_ROOT_DIR
    "${CLANGTIDY_ROOT_DIR}"
    CACHE PATH "Path to search for clang-tidy")

if(CLANGTIDY_ROOT_DIR)
  find_program(
    CLANGTIDY_EXECUTABLE
    NAMES clang-tidy
    PATHS "${CLANGTIDY_ROOT_DIR}"
    NO_DEFAULT_PATH)
endif()

find_program(CLANGTIDY_EXECUTABLE NAMES clang-tidy)

if(CLANGTIDY_EXECUTABLE)
  execute_process(
    COMMAND "${CLANGTIDY_EXECUTABLE}" --version
    OUTPUT_VARIABLE CLANGTIDY_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX REPLACE ".* ([0-9]+\\.([0-9]+\\.[0-9]+)?).*" "\\1" CLANGTIDY_VERSION
                       "${CLANGTIDY_VERSION}")
endif()

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(
  ClangTidy
  REQUIRED_VARS CLANGTIDY_EXECUTABLE
  VERSION_VAR CLANGTIDY_VERSION)

if(CLANGTIDY_FOUND OR CLANGTIDY_MARK_AS_ADVANCED)
  mark_as_advanced(CLANGTIDY_ROOT_DIR)
endif()

mark_as_advanced(CLANGTIDY_EXECUTABLE)
