make_softeq_component(serialization INTERFACE)

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
target_link_libraries(${PROJECT_NAME}
  INTERFACE
  common-stdutils
  )

################################### SUBCOMPONENTS
option(ENABLE_SERIALIZATION_XML "XML serialization extension" ${BUILD_ALL})
if (ENABLE_SERIALIZATION_XML)
  add_subdirectory(extensions/xml)
  target_link_libraries(${PROJECT_NAME}
    INTERFACE
    common-serialization-xml
    )
endif ()

option(ENABLE_SERIALIZATION_JSON "JSON serialization extension" ${BUILD_ALL})
if (ENABLE_SERIALIZATION_JSON)
  add_subdirectory(extensions/json)
  target_link_libraries(${PROJECT_NAME}
    INTERFACE
    common-serialization-json
    )
endif ()

if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/base_member.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/helpers.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/object_assembler.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/serializers.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/deserializers.hh
  INSTALL_PARAMS
  # static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  #TODO: check
  NESTED_DIR
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/details
  )
