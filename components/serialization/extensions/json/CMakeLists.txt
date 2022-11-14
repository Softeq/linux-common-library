make_softeq_component(json OBJECT)

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
find_package(nlohmann_json 3.3.0 REQUIRED)

target_sources(${PROJECT_NAME}
  PRIVATE
  src/json.cc
  src/json_utils.cc
  src/json_struct_serializer.cc
  src/json_array_serializer.cc
  src/json_struct_deserializer.cc
  src/json_array_deserializer.cc
  )

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  common-stdutils
  nlohmann_json::nlohmann_json
  )

################################### SUBCOMPONENTS

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/json.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
