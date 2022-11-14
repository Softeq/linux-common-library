make_softeq_component(migration OBJECT)
################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
target_sources(${PROJECT_NAME}
  PRIVATE
  src/migration_manager.cc
  src/migration_task.cc
  )

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  common-settings
  common-stdutils
  common-logging
  )

################################### SUBCOMPONENTS
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/migration_manager.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/migration_settings.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/migration_task.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  LIBRARY DESTINATION EXCLUDE_FROM_ALL
  )
