if(__add_format_target)
  return()
endif()
set(__add_format_target YES)

if(NOT CLANGFORMAT_FOUND)
  find_package(ClangFormat)
endif()

if(NOT CMAKEFORMAT_FOUND)
  find_package(CmakeFormat)
endif()

function(add_clang_format target_name)
  if(CLANGFORMAT_FOUND)
    add_custom_target(
      fix-${target_name}
      COMMAND ${CLANGFORMAT_EXECUTABLE} -i --style=file ${ARGN}
      COMMENT "Formatting with clang-format...")
    add_custom_target(
      check-${target_name}
      COMMAND ${CLANGFORMAT_EXECUTABLE} --dry-run -Werror --style=file ${ARGN}
      COMMENT "Running clang-format...")
  endif()
endfunction()

function(add_cmake_format target_name)
  if(CMAKEFORMAT_FOUND)
    add_custom_target(
      fix-${target_name}
      COMMAND ${CMAKEFORMAT_EXECUTABLE} -i ${ARGN}
      COMMENT "Formatting with cmake-format...")
    add_custom_target(
      check-${target_name}
      COMMAND ${CMAKEFORMAT_EXECUTABLE} --check ${ARGN}
      COMMENT "Running cmake-format...")
  endif()
endfunction()
