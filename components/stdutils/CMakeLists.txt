make_softeq_component(stdutils OBJECT)

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
target_sources(${PROJECT_NAME}
  PRIVATE
  src/any.cc
  src/stdutils.cc
  src/timeutils.cc
  src/packet_buffer.cc
  )

################################### SUBCOMPONENTS
if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/any.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/optional.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/packet_buffer.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/padded.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/scope_guard.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/stdutils.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/timeutils.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
