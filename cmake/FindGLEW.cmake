if(NOT PREFER_BUNDLED_LIBS)
  set(CMAKE_MODULE_PATH ${ORIGINAL_CMAKE_MODULE_PATH})
  find_package(GLEW)
  set(CMAKE_MODULE_PATH ${OWN_CMAKE_MODULE_PATH})
  if(GLEW_FOUND)
    set(GLEW_BUNDLED OFF)
    set(GLEW_DEP)
  endif()
endif()

if(NOT GLEW_FOUND)
  set(GLEW_BUNDLED ON)
  set(GLEW_SRC_DIR other/glew/include)
  set_src(GLEW_INCLUDES GLOB ${GLEW_SRC_DIR}/GL glew.h glxew.h wglew.h)
  add_library(glew EXCLUDE_FROM_ALL OBJECT ${GLEW_INCLUDES})
  set(GLEW_INCLUDEDIR ${GLEW_SRC_DIR})
  target_include_directories(glew PRIVATE ${GLEW_INCLUDEDIR})
  target_compile_definitions(glew PRIVATE GLEW_STATIC)

  set(GLEW_DEP $<TARGET_OBJECTS:glew>)
  set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDEDIR})
  set(GLEW_LIBRARIES)

  list(APPEND TARGETS_DEP glew)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(GLEW DEFAULT_MSG GLEW_INCLUDEDIR)
endif()
