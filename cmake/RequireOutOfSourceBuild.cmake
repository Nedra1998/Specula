get_filename_component(_src "${CMAKE_SOURCE_DIR}" ABSOLUTE)
get_filename_component(_cur_src "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
get_filename_component(_bin "${CMAKE_BINARY_DIR}" ABSOLUTE)

string(LENGTH "${_src}" _src_len)
string(LENGTH "${_cur_src}" _cur_src_len)
string(LENGTH "${_bin}" _bin_len)

set(_test)

if(NOT "${_bin_len}" GREATER "${_src_len}")
  list(APPEND _test _src)
endif()

if(NOT "${_bin_len}" GREATER "${_cur_src_len}")
  list(APPEND _test _cur_src)
endif()

foreach(_var ${_test})
  string(SUBSTRING "${${_var}}" 0 ${_bin_len} _chopped)
  if("${_bin}" STREQUAL "${_chopped}")
    get_filename_component(_parent "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
    message(
      FATAL_ERROR
        "You must set a binary directory that is different from your source \
        directory. You might consider cmake options: \
        -B${CMAKE_SOURCE_DIR}/build or \
        -B${_parent}/build-${CMAKE_PROJECT_NAME}")
  endif()
endforeach()
