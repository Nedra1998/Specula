if(__set_default_build_type)
  return()
endif()
set(__set_default_build_type YES)

# Set the default build type if it has not already been set.
function(set_default_build_type _type)
  if("${CMAKE_GENERATOR}" MATCHES "Makefiles"
     AND NOT CMAKE_BUILD_TYPE
     AND NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT __DEFAULT_BUILD_TYPE_SET)
      set(CMAKE_BUILD_TYPE
          "${_type}"
          CACHE STRING "" FORCE)
      set(__DEFAULT_BUILD_TYPE_SET
          YES
          CACHE INTERNAL "")
      set_property(
        CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel"
                                        "RelWithDebInfo")
    endif()
  endif()
endfunction()
