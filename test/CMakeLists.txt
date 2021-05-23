file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable(unit-tests ${SOURCES})
target_compile_features(unit-tests PUBLIC cxx_std_20)
target_link_libraries(unit-tests PRIVATE fmt spdlog Catch2::Catch2 specula)
target_include_directories(unit-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

include(Catch)
catch_discover_tests(unit-tests)

include(Format)
include(Analyze)
format_target(unit-tests TEST)
analyze_target(unit-tests TEST)
