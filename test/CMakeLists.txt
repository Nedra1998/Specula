file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable(unit-tests ${SOURCES})
target_compile_features(unit-tests PUBLIC cxx_std_20)
target_link_libraries(unit-tests PRIVATE fmt spdlog Catch2::Catch2 specula
                                         magic_enum)
target_include_directories(unit-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

include(Catch)
catch_discover_tests(unit-tests)
enable_code_coverage(
  unit-tests
  NAME coverage
  HTML TEST
  LINE_COVERAGE 75
  EXCLUDE "${PROJECT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
  TESTS "unit-tests_TESTS")

format_target(unit-tests TEST)
analyze_target(unit-tests TEST)
