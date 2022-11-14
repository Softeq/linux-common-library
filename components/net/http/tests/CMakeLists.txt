make_softeq_component(tests EXECUTABLE)

################################### PROJECT SPECIFIC GLOBALS
gtest_discover_tests(${PROJECT_NAME})

################################### COMPONENT SOURCES

target_sources(${PROJECT_NAME}
  PRIVATE
  main.cc
  http_server.cc
  )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  GTest::GTest
  GTest::Main
  ${PARENT_COMPONENT_NAME}
  $<TARGET_PROPERTY:${PARENT_COMPONENT_NAME},LINK_LIBRARIES>
  common-net-curl_helper
  )

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  UNIT_TEST
  )

################################### SUBCOMPONENTS

################################### INSTALLATION
