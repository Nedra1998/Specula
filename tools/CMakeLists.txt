# ========== Options ==========
# ========== Dependencies ==========

CPMAddPackage("gh:jarro2783/cxxopts@3.3.1")
CPMAddPackage("gh:lvandeve/lodepng#master")

# ========== Executable ==========
add_library(tools INTERFACE)
target_link_libraries(tools INTERFACE spdlog::spdlog fmt::fmt cxxopts::cxxopts)
target_include_directories(tools INTERFACE "${lodepng_SOURCE_DIR}")
target_compile_features(tools INTERFACE cxx_std_20)

add_executable(tool-embed-image embed-image.cpp)
target_link_libraries(tool-embed-image PUBLIC tools)
