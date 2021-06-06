file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable(specula-cli ${SOURCES})
target_link_libraries(specula-cli PRIVATE CLI11::CLI11 fmt specula spdlog)
target_include_directories(specula-cli PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

enable_extra_compiler_warnings(specula-cli)

if(ENABLE_FORMATTING)
  format_target(specula-cli TEST)
endif()
if(ENABLE_ANALYSIS)
  analyze_target(specula-cli TEST)
endif()
