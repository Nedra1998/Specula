if(NOT CLANGFORMAT_FOUND)
  find_package(clangformat)
endif()
if(NOT CMAKEFORMAT_FOUND)
  find_package(cmakeformat)
endif()

function(create_format_target NAME)
  set(OPTIONS TEST)
  set(SINGLE_VALUE_ARGS COMMENT)
  set(MULTI_VALUE_ARGS CLANGFORMAT_ARGS CLANGFORMAT_SOURCES CMAKEFORMAT_ARGS
                       CMAKEFORMAT_SOURCES)
  cmake_parse_arguments(FMT "${OPTIONS}" "${SINGLE_VALUE_ARGS}"
                        "${MULTI_VALUE_ARGS}" ${ARGN})

  if(NOT FMT_COMMENT)
    set(FMT_COMMENT "Running formatter checks")
  endif()

  if(CLANGFORMAT_FOUND AND FMT_CLANGFORMAT_SOURCES)
    set(CLANGFORMAT_CMD ${CLANGFORMAT_EXECUTABLE} ${FMT_CLANGFORMAT_ARGS}
                        ${FMT_CLANGFORMAT_SOURCES})
  endif()
  if(CMAKEFORMAT_FOUND AND FMT_CMAKEFORMAT_SOURCES)
    set(CMAKEFORMAT_CMD ${CMAKEFORMAT_EXECUTABLE} ${FMT_CMAKEFORMAT_ARGS}
                        ${FMT_CMAKEFORMAT_SOURCES})
  endif()

  if(NOT FMT_TEST)
    add_custom_target(
      ${NAME}
      COMMAND ${CLANGFORMAT_CMD};
      COMMAND ${CMAKEFORMAT_CMD};
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "${NAME}: ${FMT_COMMENT}"
      DEPENDS ${FMT_CLANGFORMAT_SOURCES} ${FMT_CMAKEFORMAT_SOURCES})
  else()
    if(CLANGFORMAT_CMD)
      add_test(NAME ${NAME}-clangformat COMMAND ${CLANGFORMAT_CMD})
    endif()
    if(CMAKEFORMAT_CMD)
      add_test(NAME ${NAME}-cmakeformat COMMAND ${CMAKEFORMAT_CMD})
    endif()
  endif()
endfunction()

function(format_sources NAME)
  set(OPTIONS FIX TEST)
  set(SINGLE_VALUE_ARGS)
  set(MULTI_VALUE_ARGS SOURCES)
  cmake_parse_arguments(FMT "${OPTIONS}" "${SINGLE_VALUE_ARGS}"
                        "${MULTI_VALUE_ARGS}" ${ARGN})

  set(CLANGFORMAT_SOURCES)
  set(CMAKEFORMAT_SOURCES)
  foreach(src ${FMT_SOURCES})
    get_source_file_property(lang ${src} LANGUAGE)
    get_source_file_property(loc ${src} LOCATION)
    if("${loc}" MATCHES ".*\\.(c|cc|cxx|c\\+\\+|cpp|h|cxx|h\\+\\+|hpp)")
      list(APPEND CLANGFORMAT_SOURCES "${loc}")
    elseif("${loc}" MATCHES ".*(\\.cmake|CMakeLists.txt)")
      list(APPEND CMAKEFORMAT_SOURCES "${loc}")
    endif()
  endforeach()

  create_format_target(
    ${NAME}
    CLANGFORMAT_ARGS ${CLANGFORMAT_DRYRUN}
    CLANGFORMAT_SOURCES ${CLANGFORMAT_SOURCES}
    CMAKEFORMAT_ARGS ${CMAKEFORMAT_DRYRUN}
    CMAKEFORMAT_SOURCES ${CMAKEFORMAT_SOURCES})
  if(FMT_FIX)
    create_format_target(
      ${NAME}-fix
      CLANGFORMAT_ARGS ${CLANGFORMAT_INPLACE}
      CLANGFORMAT_SOURCES ${CLANGFORMAT_SOURCES}
      CMAKEFORMAT_ARGS ${CMAKEFORMAT_INPLACE}
      CMAKEFORMAT_SOURCES ${CMAKEFORMAT_SOURCES})
  endif()
  if(FMT_TEST)
    create_format_target(
      ${NAME} TEST
      CLANGFORMAT_ARGS ${CLANGFORMAT_DRYRUN}
      CLANGFORMAT_SOURCES ${CLANGFORMAT_SOURCES}
      CMAKEFORMAT_ARGS ${CMAKEFORMAT_DRYRUN}
      CMAKEFORMAT_SOURCES ${CMAKEFORMAT_SOURCES})
  endif()
endfunction()

function(format_target TARGET)
  if(NOT TARGET ${TARGET})
    message(FATAL_ERROR "format_target requires a valid target to format")
  endif()

  set(OPTIONS FIX TEST)
  set(SINGLE_VALUE_ARGS NAME)
  set(MULTI_VALUE_ARGS)
  cmake_parse_arguments(FMT "${OPTIONS}" "${SINGLE_VALUE_ARGS}"
                        "${MULTI_VALUE_ARGS}" ${ARGN})

  if(NOT FMT_NAME)
    set(FMT_NAME "${TARGET}-fmt")
  endif()

  get_target_property(sources ${TARGET} SOURCES)
  set(opts)
  if(FMT_FIX)
    list(APPEND opts FIX)
  endif()
  if(FMT_TEST)
    list(APPEND opts TEST)
  endif()
  format_sources(${FMT_NAME} ${opts} SOURCES ${sources})
endfunction()
