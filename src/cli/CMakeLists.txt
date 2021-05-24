file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable(specula-cli ${SOURCES})
target_link_libraries(specula-cli PRIVATE CLI11::CLI11 fmt specula spdlog)

format_target(specula-cli TEST)
analyze_target(specula-cli TEST)
enable_extra_compiler_warnings(specula-cli)
