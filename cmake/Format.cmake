# Format.cmake - Defines a set of global formatting target that can check and
# apply formatting for all code files in the project. This module defines the
# following targets:
#
# - format
# - format-fix

# Define global formatting targets
add_custom_target(format COMMENT "Checking all code formatting...")
add_custom_target(format-fix COMMENT "Fixing all code formatting...")

# If clang-format is available, add targets to check and fix C++ formatting and
# add them as dependencies to the global format targets
find_package(clang-format)
if(clang-format_FOUND)
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

  # Define custom targets to check and fix C++ formatting using clang-format.
  add_custom_target(
    format-clang-format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror --color ${CXX_SOURCE_FILES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Checking C++ formatting with clang-format..."
    VERBATIM
  )

  add_custom_target(
    format-fix-clang-format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${CXX_SOURCE_FILES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Fixing C++ formatting with clang-format..."
    VERBATIM
  )

  # Add the clang-format targets as dependencies to the global format targets
  add_dependencies(format format-clang-format)
  add_dependencies(format-fix format-fix-clang-format)
endif()

# If gersemi is available, add targets to check and fix CMake formatting and add
# them as dependencies to the global format targets
find_package(gersemi)
if(gersemi_FOUND)
  # Build a list of all CMake source files in the project that formatting
  # should be applied to. Note that the CMakeLists.txt files are not included
  # as a recursive glob to avoid accidentally including build files or other
  # generated files that may be in the source tree.
  file(GLOB_RECURSE CMAKE_SOURCE_FILES "${PROJECT_SOURCE_DIR}/cmake/*.cmake")
  list(
    APPEND CMAKE_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/CMakeLists.txt"
    "${PROJECT_SOURCE_DIR}/app/CMakeLists.txt"
    "${PROJECT_SOURCE_DIR}/docs/CMakeLists.txt"
    "${PROJECT_SOURCE_DIR}/src/CMakeLists.txt"
    "${PROJECT_SOURCE_DIR}/tests/CMakeLists.txt"
  )

  # Define custom targets to check and fix CMake formatting using gersemi.
  add_custom_target(
    format-gersemi
    COMMAND ${GERSEMI_EXECUTABLE} --check --no-warn-about-unknown-commands ${CMAKE_SOURCE_FILES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Checking CMake formatting with gersemi..."
    VERBATIM
  )

  add_custom_target(
    format-fix-gersemi
    COMMAND ${GERSEMI_EXECUTABLE} --in-place --no-warn-about-unknown-commands ${CMAKE_SOURCE_FILES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Fixing CMake formatting with gersemi..."
    VERBATIM
  )

  # Add the gersemi targets as dependencies to the global format targets
  add_dependencies(format format-gersemi)
  add_dependencies(format-fix format-fix-gersemi)
endif()
