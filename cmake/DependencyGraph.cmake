if(__add_dependency_graph)
  return()
endif()
set(__add_dependency_graph YES)

if(NOT DOT_FOUND)
  find_package(Dot QUIET)
endif()

function(add_dependency_graph target_name)
  set(OPTIONS)
  set(SINGLE_VALUE_KEYWORDS OUTPUT_FORMAT OUTPUT_DIR)
  set(MULTI_VALUE_KEYWORDS TARGET_DEPENDS TARGET_DEPENDERS)
  cmake_parse_arguments("" "${OPTIONS}" "${SINGLE_VALUE_KEYWORDS}" "${MULTI_VALUE_KEYWORDS}"
                        ${ARGN})

  if(DOT_FOUND)
    if(NOT _OUTPUT_FORMAT)
      set(_OUTPUT_FORMAT "png")
    endif()

    if(NOT _OUTPUT_DIR)
      set(_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${target_name}")
    endif()

    add_custom_command(
      OUTPUT "${_OUTPUT_DIR}/${target_name}.dot" "${_OUTPUT_DIR}/_${target_name}"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${_OUTPUT_DIR}"
      COMMAND ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR} --graphviz="${_OUTPUT_DIR}/${target_name}.dot"
      COMMENT "Generating graphviz dependency graphs")

    add_custom_command(
      OUTPUT "${_OUTPUT_DIR}/${target_name}.${_OUTPUT_FORMAT}"
      COMMAND ${DOT_EXECUTABLE} -T${_OUTPUT_FORMAT} "${_OUTPUT_DIR}/${target_name}.dot" -o
              "${_OUTPUT_DIR}/${target_name}.${_OUTPUT_FORMAT}"
      DEPENDS "${_OUTPUT_DIR}/${target_name}.dot"
      COMMENT "Generating project dependency graph")

    set(output_images "${_OUTPUT_DIR}/${target_name}.${_OUTPUT_FORMAT}")

    foreach(target ${_TARGET_DEPENDS})
      add_custom_command(
        OUTPUT "${_OUTPUT_DIR}/${target_name}.${target}.${_OUTPUT_FORMAT}"
        COMMAND ${DOT_EXECUTABLE} -T${_OUTPUT_FORMAT} "${_OUTPUT_DIR}/${target_name}.dot.${target}"
                -o "${_OUTPUT_DIR}/${target_name}.${target}.${_OUTPUT_FORMAT}"
        DEPENDS "${_OUTPUT_DIR}/${target_name}.dot"
        COMMENT "Generating depends graph for ${target}")
      list(APPEND output_images "${_OUTPUT_DIR}/${target_name}.${target}.${_OUTPUT_FORMAT}")
    endforeach()

    foreach(target ${_TARGET_DEPENDERS})
      add_custom_command(
        OUTPUT "${_OUTPUT_DIR}/${target_name}.${target}.dependers.${_OUTPUT_FORMAT}"
        COMMAND
          ${DOT_EXECUTABLE} -T${_OUTPUT_FORMAT}
          "${_OUTPUT_DIR}/${target_name}.dot.${target}.dependers" -o
          "${_OUTPUT_DIR}/${target_name}.${target}.dependers.${_OUTPUT_FORMAT}"
        DEPENDS "${_OUTPUT_DIR}/${target_name}.dot"
        COMMENT "Generating dependers graph for ${target}")
      list(APPEND output_images
           "${_OUTPUT_DIR}/${target_name}.${target}.dependers.${_OUTPUT_FORMAT}")
    endforeach()

    add_custom_target(
      ${target_name}
      DEPENDS "${_OUTPUT_DIR}/_${target_name}" ${output_images}
      COMMENT "Generating ${target_name} dependency graph")

  endif()
endfunction()
