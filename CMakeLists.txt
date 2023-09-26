cmake_minimum_required(VERSION 3.22)

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

option(BUILD_DOCUMENTATION "Build the generated documentation targets" OFF)
option(BUILD_TESTING "Build the test targets" OFF)
option(ENABLE_PROFILING "Enable profiling with Tracy" OFF)

# ---- Global Dependencies ----

include(CPM)

CPMAddPackage("gh:TheLartians/Ccache.cmake@1.2.5")

# ---- Global Configuration ----

set(CMAKE_CXX_STANDARD 20)

# ---- Subdirectories ----

add_subdirectory(src)
add_subdirectory(app)

if(BUILD_DOCUMENTATION)
  add_subdirectory(docs)
endif()

if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

# ---- Global Targets ----

# ---- Packaging ----
