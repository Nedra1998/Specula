file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

include(GenerateVersionHeader)
generate_version_header(OUTPUT_DIR
                        "${CMAKE_CURRENT_SOURCE_DIR}/include/specula/")
list(APPEND INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include/common/version.hpp")

add_library(specula ${SOURCES})
target_link_libraries(specula)
target_include_directories(specula PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

include(Format)
include(Analyze)
format_target(specula TEST)
analyze_target(specula TEST)
