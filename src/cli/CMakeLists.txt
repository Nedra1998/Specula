file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable(specula-cli ${SOURCES})
target_link_libraries(specula-cli PRIVATE CLI11::CLI11 fmt)

include(Format)
include(Analyze)
format_target(specula-cli TEST)
analyze_target(specula-cli TEST)
