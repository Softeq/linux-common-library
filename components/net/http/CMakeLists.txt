make_softeq_component(http OBJECT
  GROUP net
  )

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
pkg_check_modules(MHD REQUIRED libmicrohttpd)
pkg_check_modules(UUID REQUIRED uuid)

target_sources(${PROJECT_NAME}
  PRIVATE
  src/http_connection_impl.cc
  src/http_server.cc
  src/http_server_impl.cc
  src/http_session.cc
  src/utils.cc
  )

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${MHD_INCLUDE_DIRS}
  ${UUID_INCLUDE_DIRS}
  )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  common-logging
  common-stdutils
  common-system
  ${MHD_LIBRARIES}
  ${UUID_LIBRARIES}
  )

################################### SUBCOMPONENTS
if (BUILD_TESTING AND ENABLE_NET_CURL)
  add_subdirectory(tests)
endif ()

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/http_connection.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/http_server.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/http_session.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
