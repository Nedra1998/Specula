# Lint.cmake - Defines a set of global linting targets that run static analysis
# tools on the code files in the project. This module defines the following
# targets:
#
# - check

# Define the global linting target
add_custom_target(check COMMENT "Running all static analysis checks...")

# Build a list of all C++ source files in the project that formatting should
# be applied to
file(
  GLOB_RECURSE CXX_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
  "${PROJECT_SOURCE_DIR}/src/*.hpp"
  "${PROJECT_SOURCE_DIR}/include/*.cpp"
  "${PROJECT_SOURCE_DIR}/app/*.cpp"
  "${PROJECT_SOURCE_DIR}/app/*.hpp"
  "${PROJECT_SOURCE_DIR}/tests/*.cpp"
  "${PROJECT_SOURCE_DIR}/tests/*.hpp"
)

# If clang-tidy is available, add a target to run clang-tidy static analysis
# and add it to the global check target
find_package(clang-tidy)
if(clang-tidy_FOUND)
  # If run-clang-tidy is available, use it to run clang-tidy in parallel.
  # Otherwise, run clang-tidy on each file sequentially.
  if(RUN_CLANG_TIDY_EXECUTABLE)
    add_custom_target(
      check-clang-tidy
      COMMAND
        ${RUN_CLANG_TIDY_EXECUTABLE} -hide-progress -j 4 -use-color -quiet -p=${CMAKE_BINARY_DIR}
        -config-file=${PROJECT_SOURCE_DIR}/.clang-tidy ${CXX_SOURCE_FILES}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMENT "Running clang-tidy static analysis..."
      VERBATIM
    )
  else()
    add_custom_target(
      check-clang-tidy
      COMMAND
        ${CLANG_TIDY_EXECUTABLE} -p=${CMAKE_BINARY_DIR} --config-file=${PROJECT_SOURCE_DIR}/.clang-tidy
        ${CXX_SOURCE_FILES}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMENT "Running clang-tidy static analysis..."
      VERBATIM
    )
  endif()

  # Add the clang-tidy target as a dependency to the global check target
  add_dependencies(check check-clang-tidy)
endif()

# If cppcheck is available, add a target to run cppcheck static analysis and
# add it to the global check target
find_package(cppcheck)
if(cppcheck_FOUND)
  # Ensure the cppcheck build dir exists
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/cppcheck")

  # Create a custom target to run cppcheck with the appropriate options
  add_custom_target(
    check-cppcheck
    COMMAND
      ${CMAKE_COMMAND} -E env CLICOLOR_FORCE=1 ${CPPCHECK_EXECUTABLE} --quiet -j 4 --enable=all --std=c++20
      --suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck --cppcheck-build-dir=${CMAKE_BINARY_DIR}/cppcheck
      --project=${CMAKE_BINARY_DIR}/compile_commands.json -i ${CMAKE_BINARY_DIR}/**
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Running cppcheck static analysis..."
    VERBATIM
  )

  # Add the cppcheck target as a dependency to the global check target
  add_dependencies(check check-cppcheck)
endif()

# If include-what-you-use is available, add a target to run iwyu static analysis and
# add it to the global check target.
find_package(iwyu)
if(iwyu_FOUND)
  # If iwyu-tool is available, use it to run include-what-you-use on the entier
  # compilation database. Otherwise, run include-what-you-use on each file
  # sequentially.
  if(IWYU_TOOL_EXECUTABLE)
    add_custom_target(
      check-iwyu
      COMMAND
        ${IWYU_TOOL_EXECUTABLE} -j 0 -p=${CMAKE_BINARY_DIR} --exclude "${CMAKE_BINARY_DIR}" --output-format=clang | sed
        "/^$/d" | awk
        [[{ gsub(/^[^:]+:[0-9]+:[0-9]+:/, "\033[1m&\033[0m"); gsub(/error:/, "\033[1;31merror:\033[0m"); gsub(/warning:/, "\033[1;35mwarning:\033[0m"); gsub(/\047[^\047]+\047/, "\033[1;32m&\033[0m"); print }]]
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMENT "Running include-what-you-use static analysis..."
      VERBATIM
    )
  else()
    # Build a list of commands to run include-what-you-use on each file
    # individually
    set(IWYU_COMMANDS)
    foreach(SOURCE_FILE ${CXX_SOURCE_FILES})
      list(APPEND IWYU_COMMANDS COMMAND "${IWYU_EXECUTABLE} ${SOURCE_FILE}")
    endforeach()

    add_custom_target(
      check-iwyu
      ${IWYU_COMMANDS}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
      COMMENT "Running include-what-you-use static analysis..."
      VERBATIM
    )
  endif()

  # Add the iwyu target as a dependency to the global check target
  add_dependencies(check check-iwyu)
endif()
