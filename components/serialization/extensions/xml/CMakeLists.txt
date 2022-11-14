make_softeq_component(xml OBJECT)

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
find_package(LibXml2 REQUIRED)

target_sources(${PROJECT_NAME}
  PRIVATE
  src/xml.cc
  src/xml_utils.cc
  src/xml_struct_deserializer.cc
  src/xml_array_deserializer.cc
  src/xml_struct_serializer.cc
  src/xml_array_serializer.cc
  )

target_include_directories(${PROJECT_NAME}
#TODO: need to make it private
  PUBLIC
  ${LIBXML2_INCLUDE_DIRS}
  )

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  ${LIBXML2_LIBRARIES}
  )

################################### SUBCOMPONENTS

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/xml.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
