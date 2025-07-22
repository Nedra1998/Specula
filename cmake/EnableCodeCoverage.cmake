# EnableCodeCoverage.cmake - Enable code coverage reporting for a target

if(__enable_code_coverage)
  return()
endif()
set(__enable_code_coverage YES)

function(_add_coverage_flags TARGET)
  if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${TARGET} PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(${TARGET} PRIVATE --coverage)
    target_link_libraries(${TARGET} PRIVATE gcov)
  elseif(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang" OR CMAKE_CXX_COMPILER_ID MATCHES
                                                           "(Apple)?[Cc]lang")
    target_compile_options(${TARGET} PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(${TARGET} PRIVATE --coverage)
    target_link_libraries(${TARGET} PRIVATE gcov)
  else()
    message(WARNING "Code coverage is only supported for GNU/Clang compilers")
    return()
  endif()
endfunction()

function(_add_coverage_target TARGET)
  set(options)
  set(oneValueArgs NAME COMMENT OUTPUT ROOT OBJ)
  set(mulitValueArgs EXCLUDE ARGS)
  cmake_parse_arguments("" "${options}" "${oneValueArgs}" "${mulitValueArgs}" ${ARGN})

  find_package(gcovr REQUIRED)

  if(NOT _NAME)
    set(_NAME "coverage")
  endif()

  if(NOT _COMMENT)
    set(_COMMENT "Collecting test coverage for target ${TARGET}")
  endif()

  if(NOT _ROOT)
    set(_ROOT ${PROJECT_SOURCE_DIR})
  endif()

  if(NOT _OBJ)
    set(_OBJ ${PROJECT_BINARY_DIR})
  endif()

  if(NOT _OUTPUT)
    set(_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/coverage)
  endif()

  set(command "${GCOVR_EXECUTABLE}")
  if((CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
    find_package(gcov REQUIRED)
    list(APPEND command --gcov-executable ${GCOV_EXECUTABLE})
  elseif((CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang" OR CMAKE_CXX_COMPILER_ID MATCHES
                                                            "(Apple)?[Cc]lang"))
    find_package(gcov REQUIRED)
    find_package(llvmcov REQUIRED)
    list(APPEND command --gcov-executable "${LLVMCOV_EXECUTABLE} gcov")
  else()
    message(WARNING "Code coverage is only supported for GNU/Clang compilers")
    return()
  endif()

  list(APPEND command --root ${_ROOT} --object-directory ${_OBJ})

  foreach(exclude ${_EXCLUDE})
    list(APPEND command --exclude "${exclude}")
  endforeach()

  list(APPEND command --print-summary --html-nested "${_OUTPUT}/index.html")
  list(APPEND command ${_ARGS})

  add_custom_target(
    ${_NAME}
    COMMAND ${TARGET}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_OUTPUT}
    COMMAND ${command}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
    BYPRODUCTS ${_OUTPUT}
    COMMENT "${_COMMENT}")
endfunction()

function(enable_coverage TARGET)
  _add_coverage_flags(${TARGET})

  get_target_property(_target_type ${TARGET} TYPE)
  if("${_target_type}" MATCHES "EXECUTABLE")
    _add_coverage_target(${TARGET} ${ARGN})
  endif()
endfunction()
