make_softeq_component(logging OBJECT)

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
target_sources(${PROJECT_NAME}
  PRIVATE
  src/log.cc
  src/logger_interface.cc
  src/system_logger.cc
  src/console_logger.cc
  )

################################### SUBCOMPONENTS
if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/logger_interface.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/log.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/system_logger.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
