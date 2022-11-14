make_softeq_component(tests EXECUTABLE)

################################### PROJECT SPECIFIC GLOBALS
gtest_discover_tests(${PROJECT_NAME})

################################### COMPONENT SOURCES

target_sources(${PROJECT_NAME}
  PRIVATE
  main.cc
  log.cc
  logger_interface.cc
  )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  GTest::GTest
  GTest::Main
  ${PARENT_COMPONENT_NAME}
  )

################################### SUBCOMPONENTS

################################### INSTALLATION
