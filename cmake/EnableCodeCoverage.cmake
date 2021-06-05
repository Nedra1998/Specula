if(__enable_code_coverage)
  return()
endif()
set(__enable_code_coverage YES)

if(NOT GCOV_FOUND)
  find_package(gcov)
endif()
if(NOT LLVMCOV_FOUND)
  find_package(llvmcov)
endif()
if(NOT GCOVR_FOUND)
  find_package(gcovr)
endif()

# Constructs the base gcovr command to collect the coverage report information.
function(_get_gcovr_command)
  set(GCOVR_EXCLUDE_ARGS "")
  foreach(EXCLUDE ${ARGN})
    list(APPEND GCOVR_EXCLUDE_ARGS "-e")
    list(APPEND GCOVR_EXCLUDE_ARGS "${EXCLUDE}")
  endforeach()

  if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(EXECUTABLE "${GCOV_EXECUTABLE}")
  elseif(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang"
         OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
    set(EXECUTABLE "${LLVMCOV_EXECUTABLE} gcov")
  else()
    message(WARNING "Code coverage is only supported for GNU/Clang compilers")
  endif()

  set(GCOVR_CMD
      ${GCOVR_EXECUTABLE}
      --gcov-executable
      ${EXECUTABLE}
      --root
      ${PROJECT_SOURCE_DIR}
      ${GCOVR_EXCLUDE_ARGS}
      --object-directory=${PROJECT_BINARY_DIR}
      PARENT_SCOPE)
endfunction()

# Creates a target to print the coverage report into stdout as a human readable
# text string.
function(add_cov_report_text TARGET NAME GCOVR_CMD OUTPUT)
  add_custom_target(
    ${NAME}
    COMMAND ${TARGET}
    COMMAND ${GCOVR_CMD} --print-summary
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${TARGET}
    VERBATIM
    COMMENT "Collecting code coverage report")
endfunction()

# Creates a target to write the coverage report into HTML files, along with
# specific coverage details of each file.
function(add_cov_report_html TARGET NAME GCOVR_CMD OUTPUT)
  add_custom_target(
    ${NAME}
    COMMAND ${TARGET}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT}"
    COMMAND ${GCOVR_CMD} --html "${OUTPUT}/index.html" --html-details
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${TARGET}
    VERBATIM
    COMMENT "Collecting code coverage report")
endfunction()

# Creates a target to write the coverage report into XML files, along with
# detailed coverage information of each file.
function(add_cov_report_xml TARGET NAME GCOVR_CMD OUTPUT)
  add_custom_target(
    ${NAME}
    COMMAND ${TARGET}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT}"
    COMMAND ${GCOVR_CMD} --xml --xml-pretty --output "${OUTPUT}/index.xml"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${TARGET}
    VERBATIM
    COMMENT "Collecting code coverage report")
endfunction()

# Creates a CTest test for the coverage percentage. This currently will be
# configured as a dependency of the detected Catch2 unit tests, and will be
# executed after all of those have completed. The test collects the coverage
# information, and validates that either the line and/or branch coverage
# percentage is above the required level.
function(add_cov_test TARGET GCOVR_CMD TESTS LINE BRANCH)
  string(SHA1 HASH "${TARGET}")
  string(SUBSTRING "${HASH}" 0 7 HASH)
  set(CTEST_INCLUDE_FILE
      "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_include-${HASH}.cmake")
  set(CMD "")
  foreach(ARG ${GCOVR_CMD})
    set(CMD "${CMD} \"${ARG}\"")
  endforeach()
  set(CMD "${CMD} \"--fail-under-line\" \"${LINE}\" \"--fail-under-branch\" \
      \"${BRANCH}\"")
  file(
    WRITE "${CTEST_INCLUDE_FILE}"
    "if(${COV_TESTS})\n" "  add_test(${TARGET}-coverage ${CMD})\n"
    "  set_tests_properties(${TARGET}-coverage PROPERTIES DEPENDS \${${TESTS}} \
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")\n" "endif()\n")
  set_property(
    DIRECTORY
    APPEND
    PROPERTY TEST_INCLUDE_FILES "${CTEST_INCLUDE_FILE}")
endfunction()

# Add required compiler and link options to a target that are required to enable
# code coverage. This currently only supports GNU and Clang compilers.
function(add_cov_flags TARGET)
  if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${TARGET} PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_libraries(${TARGET} PRIVATE gcov)
  elseif(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang"
         OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
    target_compile_options(${TARGET} PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_libraries(${TARGET} PRIVATE gcov)
    target_link_options(${TARGET} PRIVATE --coverage)
  else()
    message(WARNING "Code coverage is only supported for GNU/Clang compilers")
  endif()
endfunction()

# Enable code coverage for a given target, and create the reporter targets for
# reporting the coverage percentage, and optionally add the CTest test for
# validation.
function(enable_code_coverage TARGET)
  if(NOT TARGET ${TARGET})
    message(FATAL_ERROR "enable_code_coverage requires a valid target")
  endif()
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(
      WARNING "Code coverage may be inacurate when not using \"Debug\" builds")
  endif()

  cmake_parse_arguments(
    COV "TEST;TEXT;HTML;XML"
    "REPORTER;NAME;OUTPUT;HTML_OUTPUT;XML_OUTPUT;LINE_COVERAGE;BRANCH_COVERAGE"
    "EXCLUDE;TESTS" ${ARGN})

  if(NOT COV_NAME)
    set(COV_NAME "${TARGET}-cov")
  endif()

  if(NOT COV_OUTPUT)
    set(COV_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/coverage")
  endif()

  if(NOT COV_HTML_OUTPUT)
    set(COV_HTML_OUTPUT "${COV_OUTPUT}/html")
  endif()
  if(NOT COV_XML_OUTPUT)
    set(COV_XML_OUTPUT "${COV_OUTPUT}/xml")
  endif()
  if(NOT COV_LINE_COVERAGE)
    set(COV_LINE_COVERAGE 0)
  endif()
  if(NOT COV_BRANCH_COVERAGE)
    set(COV_BRANCH_COVERAGE 0)
  endif()

  _get_gcovr_command(${COV_EXCLUDE})

  get_target_property(TYPE ${TARGET} TYPE)
  if("${TYPE}" MATCHES "EXECUTABLE")
    add_cov_flags(${TARGET})

    if(NOT COV_REPORTER OR COV_REPORTER STREQUAL "TEXT")
      add_cov_report_text(${TARGET} ${COV_NAME} "${GCOVR_CMD}" "${COV_OUTPUT}")
    elseif(COV_REPORTER STREQUAL "HTML")
      add_cov_report_html(${TARGET} ${COV_NAME} "${GCOVR_CMD}" "${COV_OUTPUT}")
    elseif(COV_REPORTER STREQUAL "XML")
      add_cov_report_xml(${TARGET} ${COV_NAME} "${GCOVR_CMD}" "${COV_OUTPUT}")
    endif()

    if(COV_TEXT)
      add_cov_report_text(${TARGET} ${COV_NAME}-text "${GCOVR_CMD}"
                          "${COV_OUTPUT}")
    endif()
    if(COV_HTML)
      add_cov_report_html(${TARGET} ${COV_NAME}-html "${GCOVR_CMD}"
                          "${COV_HTML_OUTPUT}")
    endif()
    if(COV_XML)
      add_cov_report_xml(${TARGET} ${COV_NAME}-xml "${GCOVR_CMD}"
                         "${COV_XML_OUTPUT}")
    endif()
    if(COV_TEST)
      add_cov_test(${TARGET} "${GCOVR_CMD}" "${COV_TESTS}" ${COV_LINE_COVERAGE}
                   ${COV_BRANCH_COVERAGE})
    endif()
  elseif("${TYPE}" MATCHES "STATIC_LIBRARY|SHARED_LIBRARY")
    add_cov_flags(${TARGET})
  else()
    message(WARNING "Cannot enable coverage on target of type \"${TYPE}\"")
  endif()
endfunction()
