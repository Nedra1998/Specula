cmake_minimum_required(VERSION 3.25)

# ---- Project ----

project(
  Specula
  VERSION 0.1.0
  DESCRIPTION "A C++20 Physically Based Renderer"
  LANGUAGES CXX)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

# ---- Include Guard ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ---- Global Options ----

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_PROFILING "Enable profiling with Tracy" OFF)

# ---- Global Dependencies ----

include(CPM)
include(DependencyGraph)
include(FormatTarget)
include(CheckTarget)

# ---- Global Configuration ----

set(CMAKE_CXX_STANDARD 20)

# ---- Subdirectories ----

add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(docs)
add_subdirectory(tests)

# ---- Global Targets ----

file(GLOB_RECURSE CXX_SOURCES ${PROJECT_SOURCE_DIR}/*.[ch]pp ${PROJECT_SOURCE_DIR}/*.[ch]
     ${PROJECT_SOURCE_DIR}/*.[h]pp ${PROJECT_SOURCE_DIR}/*.[h])
file(GLOB_RECURSE CMAKE_SOURCES ${PROJECT_SOURCE_DIR}/CMakeLists.txt
     ${PROJECT_SOURCE_DIR}/cmake/*.cmake)

list(FILTER CXX_SOURCES EXCLUDE REGEX ${PROJECT_BINARY_DIR}.*)
list(FILTER CMAKE_SOURCES EXCLUDE REGEX ${PROJECT_BINARY_DIR}.*)

add_dependency_graph(dependency-graph TARGET_DEPENDS app specula)
add_clang_format(format-cpp ${CXX_SOURCES})
add_cmake_format(format-cmake ${CMAKE_SOURCES})
add_cppcheck(check-cppcheck)
add_clang_tidy(check-clang-tidy ${CXX_SOURCES})
add_cmake_lint(check-cmake-lint ${CMAKE_SOURCES})

add_custom_target(
  format
  DEPENDS fix-format-cpp fix-format-cmake
  COMMENT "Running formatter fixes")

add_custom_target(
  check-format
  DEPENDS check-format-cpp check-format-cmake
  COMMENT "Running formatter checks")

add_custom_target(
  fix
  DEPENDS fix-format-cpp fix-format-cmake
  COMMENT "Running static analyzers and formatter fixes")
add_custom_target(
  check
  DEPENDS check-format-cpp check-format-cmake check-cppcheck check-clang-tidy check-cmake-lint
  COMMENT "Running static analyzers and formatter checks")

# ---- Packaging ----
