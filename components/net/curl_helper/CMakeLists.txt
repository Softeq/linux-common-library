make_softeq_component(curl_helper OBJECT
  GROUP net
  )

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
pkg_check_modules(CURL REQUIRED libcurl)

target_sources(${PROJECT_NAME}
  PRIVATE
  src/curl_helper.cc
  )

target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${CURL_INCLUDE_DIRS}
  )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  #TODO: remove logger . implement exceptions
  common-logging
  common-stdutils
  PUBLIC
  ${CURL_LIBRARIES}
  )

################################### SUBCOMPONENTS
if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/curl_helper.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
