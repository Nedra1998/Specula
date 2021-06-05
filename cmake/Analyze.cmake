if(NOT CPPCHECK_FOUND)
  find_package(cppcheck)
endif()
if(NOT CLANGTIDY_FOUND)
  find_package(clangtidy)
endif()
if(NOT SHELLCHECK_FOUND)
  find_package(shellcheck)
endif()
if(NOT CMAKELINT_FOUND)
  find_package(cmakelint)
endif()

# Constructs a static analyzer target or test given arguments and sources for
# the possible static analyzer tools. Each target/test is only included if the
# static analyzer has been found successfuly.
function(create_analyze_target NAME)
  set(OPTIONS TEST)
  set(SINGLE_VALUE_ARGS COMMENT)
  set(MULTI_VALUE_ARGS
      CPPCHECK_ARGS
      CPPCHECK_SOURCES
      CLANGTIDY_ARGS
      CLANGTIDY_SOURCES
      SHELLCHECK_ARGS
      SHELLCHECK_SOURCES
      CMAKELINT_ARGS
      CMAKELINT_SOURCES)
  cmake_parse_arguments(ANLZ "${OPTIONS}" "${SINGLE_VALUE_ARGS}"
                        "${MULTI_VALUE_ARGS}" ${ARGN})
  if(NOT ANLZ_COMMENT)
    set(ANLZ_COMMENT "Running analyzer checks")
  endif()

  if(CPPCHECK_FOUND AND ANLZ_CPPCHECK_SOURCES)
    set(CPPCHECK_CMD ${CPPCHECK_EXECUTABLE} ${ANLZ_CPPCHECK_ARGS}
                     ${ANLZ_CPPCHECK_SOURCES})
  endif()
  if(CLANGTIDY_FOUND AND ANLZ_CLANGTIDY_SOURCES)
    set(CLANGTIDY_CMD ${CLANGTIDY_EXECUTABLE} ${ANLZ_CLANGTIDY_ARGS}
                      ${ANLZ_CLANGTIDY_SOURCES})
  endif()
  if(SHELLCHECK_FOUND AND ANLZ_SHELLCHECK_SOURCES)
    set(SHELLCHECK_CMD ${SHELLCHECK_EXECUTABLE} ${ANLZ_SHELLCHECK_ARGS}
                       ${ANLZ_SHELLCHECK_SOURCES})
  endif()
  if(CMAKELINT_FOUND AND ANLZ_CMAKELINT_SOURCES)
    set(CMAKELINT_CMD ${CMAKELINT_EXECUTABLE} ${ANLZ_CMAKELINT_ARGS}
                      ${ANLZ_CMAKELINT_SOURCES})
  endif()

  if(NOT ANLZ_TEST)
    add_custom_target(
      ${NAME}
      COMMAND ${CPPCHECK_CMD};
      COMMAND ${CLANGTIDY_CMD};
      COMMAND ${SHELLCHECK_CMD};
      COMMAND ${CMAKELINT_CMD};
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "${NAME}: ${ANLZ_COMMENT}"
      DEPENDS ${ANLZ_CPPCHECK_SOURCES} ${ANLZ_CLANGTIDY_SOURCES}
              ${ANLZ_SHELLCHECK_SOURCES} ${ANLZ_CMAKELINT_SOURCES})
  else()
    if(CLANGTIDY_CMD)
      add_test(NAME ${NAME}-clangtidy COMMAND ${CLANGTIDY_CMD})
    endif()
    if(CPPCHECK_CMD)
      add_test(NAME ${NAME}-cppcheck COMMAND ${CPPCHECK_CMD})
    endif()
    if(SHELLCHECK_CMD)
      add_test(NAME ${NAME}-shellcheck COMMAND ${SHELLCHECK_CMD})
    endif()
    if(CMAKELINT_CMD)
      add_test(NAME ${NAME}-cmakelint COMMAND ${CMAKELINT_CMD})
    endif()
  endif()
endfunction()

# Constructs static analyzer target and/or test given a list of source files.
# The source files are sorted into sources to provide for each analyzer, and the
# arguments for the analyzers are constructed.
function(analyze_sources NAME)
  set(OPTIONS FIX TEST)
  set(SINGLE_VALUE_ARGS)
  set(MULTI_VALUE_ARGS SOURCES)
  cmake_parse_arguments(ANLZ "${OPTIONS}" "${SINGLE_VALUE_ARGS}"
                        "${MULTI_VALUE_ARGS}" ${ARGN})

  set(CPPCHECK_SOURCES)
  set(CLANGTIDY_SOURCES)
  set(SHELLCHECK_SOURCES)
  set(CMAKELINT_SOURCES)
  foreach(src ${ANLZ_SOURCES})
    get_source_file_property(loc ${src} LOCATION)
    if("${loc}" MATCHES ".*\\.(c|cc|cxx|c\\+\\+|cpp|h|cxx|h\\+\\+|hpp)$")
      list(APPEND CPPCHECK_SOURCES "${loc}")
      list(APPEND CLANGTIDY_SOURCES "${loc}")
    elseif("${loc}" MATCHES ".*\\.(sh|bash)")
      list(APPEND SHELLCHECK_SOURCES "${loc}")
    elseif("${loc}" MATCHES ".*(\\.cmake|CMakeLists.txt)")
      list(APPEND CMAKELINT_SOURCES "${loc}")
    endif()
  endforeach()

  set(CPPCHECK_ARGS)
  set(CLANGTIDY_ARGS)
  set(SHELLCHECK_ARGS "--check-sourced")
  set(CMAKELINT_ARGS)
  if(CMAKE_EXPORT_COMPILE_COMMANDS)
    list(APPEND CPPCHECK_ARGS
         "${CPPCHECK_DATABASE}${PROJECT_BINARY_DIR}/compile_commands.json")
    list(APPEND CLANGTIDY_ARGS "${CLANGTIDY_DATABASE}"
         "${PROJECT_BINARY_DIR}/compile_commands.json")
  endif()

  create_analyze_target(
    ${NAME}
    CPPCHECK_ARGS ${CPPCHECK_ARGS}
    CPPCHECK_SOURCES ${CPPCHECK_SOURCES}
    CLANGTIDY_ARGS ${CLANGTIDY_ARGS}
    CLANGTIDY_SOURCES ${CLANGTIDY_SOURCES}
    SHELLCHECK_ARGS ${SHELLCHECK_ARGS}
    SHELLCHECK_SOURCES ${SHELLCHECK_SOURCES}
    CMAKELINT_ARGS ${CMAKELINT_ARGS}
    CMAKELINT_SOURCES ${CMAKELINT_SOURCES})
  if(ANLZ_FIX)
    create_analyze_target(
      ${NAME}-fix
      CLANGTIDY_ARGS ${CLANGTIDY_ARGS} ${CLANGTIDY_FIX}
      CLANGTIDY_SOURCES ${CLANGTIDY_SOURCES})
  endif()
  if(ANLZ_TEST)
    create_analyze_target(
      ${NAME} TEST
      CPPCHECK_ARGS ${CPPCHECK_ARGS}
      CPPCHECK_SOURCES ${CPPCHECK_SOURCES}
      CLANGTIDY_ARGS ${CLANGTIDY_ARGS}
      CLANGTIDY_SOURCES ${CLANGTIDY_SOURCES}
      SHELLCHECK_ARGS ${SHELLCHECK_ARGS}
      SHELLCHECK_SOURCES ${SHELLCHECK_SOURCES}
      CMAKELINT_ARGS ${CMAKELINT_ARGS}
      CMAKELINT_SOURCES ${CMAKELINT_SOURCES})
  endif()
endfunction()

# Extracts the sources from a provided target to construct an anlyzer target for
# the target. The sources are extracted from the target properties and passed to
# the `analyze_sources` to construct the actual targets.
function(analyze_target TARGET)
  if(NOT TARGET ${TARGET})
    message(FATAL_ERROR "analyze_target requires a valid target to analyze")
  endif()

  set(OPTIONS FIX TEST)
  set(SINGLE_VALUE_ARGS NAME)
  set(MULTI_VALUE_ARGS)
  cmake_parse_arguments(ANLZ "${OPTIONS}" "${SINGLE_VALUE_ARGS}"
                        "${MULTI_VALUE_ARGS}" ${ARGN})

  if(NOT ANLZ_NAME)
    set(ANLZ_NAME "${TARGET}-analyze")
  endif()

  get_target_property(sources ${TARGET} SOURCES)
  set(opts)
  if(ANLZ_FIX)
    list(APPEND opts FIX)
  endif()
  if(ANLZ_TEST)
    list(APPEND opts TEST)
  endif()
  analyze_sources(${ANLZ_NAME} ${opts} SOURCES ${sources})
endfunction()
