if(__enable_code_coverage)
  return()
endif()
set(__enable_code_coverage YES)

function(enable_code_coverage TARGET)
  if(NOT TARGET ${TARGET})
    message(FATAL_ERROR "enable_code_coverage requires a valid target")
  endif()

  set(OPTIONS TEST)
  set(SINGLE_VALUE_ARGS NAME)
  set(MULTI_VALUE_ARGS EXCLUDE ARGS TESTS)
  cmake_parse_arguments(COV "${OPTIONS}" "${SINGLE_VALUE_ARGS}"
                        "${MULTI_VALUE_ARGS}" ${ARGN})

  if(NOT COV_NAME)
    set(COV_NAME "${TARGET}-cov")
  endif()

  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(
      WARNING "Code coverage may be inacruate when not using \"Debug\" builds")
  endif()

  get_target_property(TYPE ${TARGET} TYPE)
  if("${TYPE}" MATCHES "EXECUTABLE")
    if(COV_TESTS)
      string(SHA1 args_hash "${TARGET}")
      string(SUBSTRING ${args_hash} 0 7 args_hash)
      set(ctest_include_file
          "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_include-${args_hash}.cmake")
      file(
        WRITE "${ctest_include_file}"
        "if(${COV_TESTS})\n"
        "  add_test(${TARGET}-coverage echo \"Hello World!\")\n"
        "  set_tests_properties(${TARGET}-coverage PROPERTIES DEPENDS \${${COV_TESTS}})\n"
        "endif()\n")
      if(NOT ${CMAKE_VERSION} VERSION_LESS "3.10.0")
        # Add discovered tests to directory TEST_INCLUDE_FILES
        set_property(
          DIRECTORY
          APPEND
          PROPERTY TEST_INCLUDE_FILES "${ctest_include_file}")
      else()
        # Add discovered tests as directory TEST_INCLUDE_FILE if possible
        get_property(
          test_include_file_set
          DIRECTORY
          PROPERTY TEST_INCLUDE_FILE
          SET)
        if(NOT ${test_include_file_set})
          set_property(DIRECTORY PROPERTY TEST_INCLUDE_FILE
                                          "${ctest_include_file}")
        else()
          message(FATAL_ERROR "Cannot set more than one TEST_INCLUDE_FILE")
        endif()
      endif()
    endif()
  elseif("${TYPE}" MATCHES "STATIC_LIBRARY|SHARED_LIBRARY")
    if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      target_compile_options(${TARGET} PRIVATE --coverage)
      target_link_libraries(${TARGET} PRIVATE gcov)
    elseif(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang"
           OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
      target_compile_options(${TARGET} PRIVATE -fprofile-instrr-generate
                                               -fcoverage-mapping)
      target_link_options(${TARGET} PRIVATE -fprofile-instr-generate)
    else()
      message(
        WARNING "Unrecognized compiler must use GNU/Clang to enable coverage")
    endif()
  endif()

endfunction()
