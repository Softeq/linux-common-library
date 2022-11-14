make_softeq_component(tests EXECUTABLE)

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES

target_sources(${PROJECT_NAME}
  PRIVATE
  main.cc
  fsutils.cc
  getopt.cc
  sd_bus.cc
  cron.cc
  signal_manager.cc
  service.cc
  time_provider.cc
  )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  GTest::GTest
  GTest::Main
  ${PARENT_COMPONENT_NAME}
  $<TARGET_PROPERTY:${PARENT_COMPONENT_NAME},LINK_LIBRARIES>
  )

#test of external app launch. the name is hardcoded in service.cc
add_executable(svservice_test
  svservice_test.cc
  )
add_compile_definitions(SVSERVICE_TEST_DIR="${PROJECT_BINARY_DIR}/components/system/tests")

target_link_libraries(svservice_test
  PRIVATE
  ${PARENT_COMPONENT_NAME}
  $<TARGET_PROPERTY:${PARENT_COMPONENT_NAME},LINK_LIBRARIES>
  )

target_include_directories(svservice_test
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
  )

# gtest_discover_tests() replaced by
# gtest_add_tests() becouse set_tests_properties can not
# find testы to add properties
gtest_add_tests(${PROJECT_NAME} "" AUTO)

# default timeout in our test system is 10 sec
set_tests_properties(Cron.DefTimeProvider PROPERTIES TIMEOUT 80)

# this tests should not be run in parallel with any other tests
set_tests_properties(
  Sdbus.Ctor
  Sdbus.ReadProperties
  Sdbus.UpdateProperties
  Sdbus.ServerCtorWrongPath
  Sdbus.DoubleServerSetup
  Sdbus.SetUnsupportedTypeProperty
  Sdbus.Callback
  PROPERTIES RUN_SERIAL TRUE)

################################### SUBCOMPONENTS

################################### INSTALLATION
if (USE_SYSTEMD AND NOT EXISTS "/usr/share/dbus-1/system.d/net.Sdbus.Moduletest.conf")
  message(STATUS "* If you run the test locally, please copy env/net.Sdbus.Moduletest.conf
  file to /usr/share/dbus-1/system.d, otherwise sdbus tests would fail.")
endif ()
