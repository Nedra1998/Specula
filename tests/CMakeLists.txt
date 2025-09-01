# ===== Dependencies =====

CPMAddPackage(
  NAME Catch2
  GITHUB_REPOSITORY catchorg/Catch2
  GIT_TAG v3.10.0 EXCLUDE_FROM_ALL TRUE SYSTEM TRUE)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)

# ===== Sources =====

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# ===== Executable =====

add_executable(unit-tests ${header} ${sources})
add_executable(specula::tests ALIAS unit-tests)

target_compile_features(unit-tests PUBLIC cxx_std_20)

target_link_libraries(unit-tests PUBLIC specula::specula Catch2::Catch2)

target_include_directories(unit-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_COVERAGE)
  enable_coverage(
    unit-tests
    EXCLUDE
    "${PROJECT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ARGS
    --html-theme
    github.green)
endif()

# ===== Testing =====

catch_discover_tests(unit-tests)
