project(${CMAKE_PROJECT_NAME}_tests LANGUAGES CXX)

if(${CMAKE_VERSION} VERSION_LESS "3.12.1")
    # workaround for bug https://gitlab.kitware.com/cmake/cmake/-/issues/18198
    cmake_policy(SET CMP0057 NEW)
endif()
find_package(GTest REQUIRED)
find_package(LibXml2 REQUIRED)
pkg_check_modules(UUID REQUIRED uuid)

if (USE_SYSTEMBUS)
  message("If you run the test locally, please copy tests/env/net.Sdbus.Moduletest.conf 
  file to /usr/share/dbus-1/system.d, otherwise sdbus tests would fail.")
endif()

add_executable(${PROJECT_NAME}
  exceptions.cc
  logger_interface.cc
  log.cc
  signal_manager.cc
  testmain.cc
  fsutils.cc
  packet_buffer.cc
  settings.cc
  curl_helper.cc
  stdutils.cc
  optional.cc
  getopt.cc
  serialization.cc
  any.cc
  scope_guard.cc
  service.cc
  cron.cc
  timeutils.cc
  http_server.cc
  http_session.cc
  rest_server.cc
  sd_bus.cc
)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  UNIT_TEST
  )

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
  WEB_TRANSFORM_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../examples/web_transform.xslt"
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${LIBXML2_INCLUDE_DIRS}
  )

target_link_libraries(${PROJECT_NAME}
  ${UUID_LIBRARIES}
  softeq::common-utest
  GTest::GTest
  GTest::Main
)

GTEST_ADD_TESTS(${PROJECT_NAME} "" AUTO)

# default timeout in our test system is 10 sec
set_tests_properties(Cron.EachMinJobs PROPERTIES TIMEOUT 190)

add_executable(svservice_test
  svservice_test.cc
)
target_link_libraries(svservice_test softeq::common)
