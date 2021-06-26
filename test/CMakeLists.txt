file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable(unit-tests ${SOURCES})
target_compile_features(unit-tests PUBLIC cxx_std_20)
target_link_libraries(unit-tests PRIVATE fmt spdlog Catch2::Catch2 specula
                                         magic_enum)
target_include_directories(unit-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

add_sycl_to_target(TARGET unit-tests)

if(BUILD_TESTS)
  include(Catch)
  catch_discover_tests(unit-tests)
endif()

if(CODE_COVERAGE)
  enable_code_coverage(
    unit-tests
    NAME coverage
    HTML TEST
    LINE_COVERAGE 75
    EXCLUDE "${PROJECT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
    TESTS "unit-tests_TESTS")
endif()

if(ENABLE_FORMATTING)
  format_target(unit-tests TEST)
endif()
if(ENABLE_ANALYSIS)
  analyze_target(unit-tests TEST)
endif()
