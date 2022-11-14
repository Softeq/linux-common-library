make_softeq_component(tests EXECUTABLE)

################################### PROJECT SPECIFIC GLOBALS
gtest_discover_tests(${PROJECT_NAME})

################################### COMPONENT SOURCES

target_include_directories(${PROJECT_NAME}
  PRIVATE
  .
  )

target_sources(${PROJECT_NAME}
  PRIVATE
  main.cc
  object_assembler.cc

  structures/basic_structures.cc
  structures/complex_object.cc
  structures/enum_object.cc
  structures/primitives_object.cc
  structures/map_object.cc
  structures/arrays_of_primitives.cc
  structures/vector_of_maps.cc
  structures/inheritance.cc
  structures/tuples.cc
  structures/custom_type.cc
  structures/test_structure.cc
  structures/automatic_serialization.cc
  )

if (ENABLE_SERIALIZATION_XML)
  target_sources(${PROJECT_NAME}
    PRIVATE

    xml/basic.cc
    xml/helpers.cc
    xml/data_structures.cc
    xml/multithreading.cc
    )
endif ()

if (ENABLE_SERIALIZATION_JSON)
  target_sources(${PROJECT_NAME}
    PRIVATE

    json/basic.cc
    json/helpers.cc
    json/data_structures.cc
    json/deserialization_using_objects_creation.cc
    json/nested_levels_control.cc  
    )
endif ()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  GTest::GTest
  GTest::Main
  common
  ${PARENT_COMPONENT_NAME}
  $<TARGET_PROPERTY:${PARENT_COMPONENT_NAME},INTERFACE_LINK_LIBRARIES>
  )

################################### SUBCOMPONENTS

################################### INSTALLATION
