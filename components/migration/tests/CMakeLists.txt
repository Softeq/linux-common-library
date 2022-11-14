make_softeq_component(tests EXECUTABLE)
################################### PROJECT SPECIFIC GLOBALS
gtest_discover_tests(${PROJECT_NAME})

################################### COMPONENT SOURCES

target_sources(${PROJECT_NAME}
  PRIVATE
  main.cc
  manager.cc
  )

if (ENABLE_SERIALIZATION_JSON)
target_sources(${PROJECT_NAME}
  PRIVATE
  settings_migration.cc
  )
endif ()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  GTest::GTest
  GTest::Main
  ${PARENT_COMPONENT_NAME}
  $<TARGET_PROPERTY:${PARENT_COMPONENT_NAME},LINK_LIBRARIES>
  )

if (ENABLE_SERIALIZATION_JSON)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  common-serialization-json
  )

endif ()

################################### SUBCOMPONENTS

################################### INSTALLATION
