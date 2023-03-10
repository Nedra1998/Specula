if(__add_check_target)
  return()
endif()
set(__add_check_target YES)

if(NOT CPPCHECK_FOUND)
  find_package(Cppcheck QUIET)
endif()

if(NOT CLANGTIDY_FOUND)
  find_package(ClangTidy QUIET)
endif()

if(NOT CMAKELINT_FOUND)
  find_package(CmakeLint)
endif()

function(add_cppcheck TARGET_NAME)
  if(CPPCHECK_FOUND)
    add_custom_target(
      ${TARGET_NAME}
      COMMAND
        ${CPPCHECK_EXECUTABLE} --project=${PROJECT_BINARY_DIR}/compile_commands.json
        --cppcheck-build-dir=${PROJECT_BINARY_DIR} -i ${PROJECT_BINARY_DIR}
        --suppress=*:${PROJECT_BINARY_DIR}/* --quiet --force --enable=warning --enable=style
        --enable=performance --enable=portability --enable=information --enable=missingInclude
      WORKING_DIRECTORY "${CMAKE_CURRENT_WORKING_DIR}"
      COMMENT "Running cppcheck..."
      VERBATIM)
  endif()
endfunction()

function(add_clang_tidy TARGET_NAME)
  if(CLANGTIDY_FOUND)
    add_custom_target(
      ${TARGET_NAME}
      COMMAND ${CLANGTIDY_EXECUTABLE} --format-style=file --quiet -p ${PROJECT_BINARY_DIR} ${ARGN}
      WORKING_DIRECTORY "${CMAKE_CURRENT_WORKING_DIR}"
      COMMENT "Running clang-tidy..."
      VERBATIM)
  endif()
endfunction()

function(add_cmake_lint TARGET_NAME)
  if(CMAKELINT_FOUND)
    add_custom_target(
      ${TARGET_NAME}
      COMMAND ${CMAKELINT_EXECUTABLE} --suppress-decorations ${ARGN}
      COMMENT "Running cmake-lint..."
      VERBATIM)
  endif()
endfunction()
