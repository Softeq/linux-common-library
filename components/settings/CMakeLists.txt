make_softeq_component(settings OBJECT)

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
target_sources(${PROJECT_NAME}
  PRIVATE
  src/settings.cc
  )

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  common-stdutils
  common-serialization
  PRIVATE
  common-logging
  )

################################### SUBCOMPONENTS
if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/settings.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
