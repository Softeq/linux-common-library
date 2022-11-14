make_softeq_component(rest OBJECT
  GROUP net
  )

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
#required for endpoints HTML generation using xslt processor
find_package(LibXml2 REQUIRED)

target_sources(${PROJECT_NAME}
  PRIVATE
  src/resthandler.cc
  )

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${LIBXML2_INCLUDE_DIRS}
  )

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  common-stdutils
  common-serialization
  common-net-http
  PRIVATE
  common-logging
  common-system
  ${LIBXML2_LIBRARIES}
  )

################################### SUBCOMPONENTS
if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/resthandler.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
