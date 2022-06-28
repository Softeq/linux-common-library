cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(common VERSION 0.3.0 LANGUAGES CXX)

# Build a release version by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug"
        AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(FATAL_ERROR "Unsupported CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

#TODO mode inside
option(USE_SYSTEMBUS "Use system dbus bus" ON)
if (USE_SYSTEMBUS)
  add_definitions(-DUSE_SYSTEMBUS)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC
  src/any.cc
  src/console_logger.cc
  src/cron.cc
  src/net/curl_helper.cc
  src/exceptions.cc
  src/fsutils.cc
  src/getopt_wrapper.cc
  src/net/http_session.cc
  src/net/http_connection_impl.cc
  src/net/http_server.cc
  src/net/http_server_impl.cc
  src/net/utils.cc
  src/log.cc
  src/logger_interface.cc
  src/packet_buffer.cc
  src/net/resthandler.cc
  src/sd_bus.cc
  src/serialization/json_serializer.cc
  src/serialization/xml_serializer.cc
  src/service.cc
  src/settings.cc
  src/signal_manager.cc
  src/stdutils.cc
  src/system_logger.cc
  src/timeutils.cc
  )

set(PUB_HDR
  include/softeq/common/any.hh
  include/softeq/common/cron.hh
  include/softeq/common/net/curl_helper.hh
  include/softeq/common/exceptions.hh  
  include/softeq/common/fsutils.hh
  include/softeq/common/getopt_wrapper.hh
  include/softeq/common/log.hh
  include/softeq/common/logger_interface.hh
  include/softeq/common/net/http_connection.hh
  include/softeq/common/net/http_server.hh
  include/softeq/common/net/http_session.hh
  include/softeq/common/optional.hh
  include/softeq/common/packet_buffer.hh
  include/softeq/common/net/resthandler.hh
  include/softeq/common/scope_guard.hh
  include/softeq/common/sd_bus.hh
  include/softeq/common/serialization/base_member.hh
  include/softeq/common/serialization/helpers.hh
  include/softeq/common/serialization/json_helpers.hh
  include/softeq/common/serialization/json_serializer.hh
  include/softeq/common/serialization/object_assembler.hh
  include/softeq/common/serialization/serializer.hh
  include/softeq/common/serialization/xml_helpers.hh
  include/softeq/common/serialization/xml_serializer.hh
  include/softeq/common/service.hh
  include/softeq/common/settings.hh
  include/softeq/common/stdutils.hh
  include/softeq/common/system_logger.hh
  include/softeq/common/timeutils.hh
  )

set(HDR
  src/console_logger.hh
  src/signal_manager.hh
  src/net/http_connection_impl.hh
  src/net/http_server_impl.hh
  src/net/utils.hh
  )

include (FindPkgConfig)

find_package(Threads REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(nlohmann_json 3.3.0 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(MHD REQUIRED libmicrohttpd)
pkg_check_modules(UUID REQUIRED uuid)
pkg_check_modules (SYSTEMD "libsystemd")
pkg_search_module(OPENSSL openssl)

# The -utest version of the common library is the same as the normal one except
# the debug version depends on Gcov, i.e., the installable version shouldn't
# depend on Gcov.
option(ENABLE_TESTING "Enable building tests" ON)
set(projects_to_build ${PROJECT_NAME})
if(ENABLE_TESTING)
    list(APPEND projects_to_build ${PROJECT_NAME}-utest)
endif()
foreach(project_name ${projects_to_build})
    #it MUST be shared lib only due to LGPL
    add_library(${project_name} SHARED ${SRC} ${PUB_HDR} ${HDR})
    add_library(softeq::${project_name} ALIAS ${project_name})

    set_target_properties(${project_name}
        PROPERTIES
        OUTPUT_NAME "sq${project_name}"
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
        SOVERSION ${PROJECT_VERSION_MAJOR}
        )

    # Define headers for this library. PUBLIC headers are used for
    # compiling the library, and will be added to consumers' build
    # paths.
    target_include_directories(${project_name}
        PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:include>"
        PRIVATE src)

    if (SYSTEMD_FOUND)
        target_compile_definitions(${project_name} PUBLIC WITH_SYSTEMD)
    endif (SYSTEMD_FOUND)

    if(OPENSSL_FOUND)
        message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
        message(STATUS "Enabling HTTPS support")
        target_compile_definitions(${project_name} PUBLIC HTTPS_ENABLED)
    endif(OPENSSL_FOUND)

    target_include_directories(${project_name}
        PRIVATE
        ${LIBXML2_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
        ${MHD_INCLUDE_DIRS}
        ${UUID_INCLUDE_DIRS}
        )

    add_definitions(
        ${LIBXML2_CFLAGS}
        ${CURL_CFLAGS}
        ${MHD_CFLAGS}
        )

    # Dependencies on libraries that we defined in the top-level file
    target_link_libraries(${project_name}
        PUBLIC
        ${CMAKE_THREAD_LIBS_INIT}
        ${CURL_LIBRARIES}
        systemd
        PRIVATE
        ${LIBXML2_LIBRARIES}
        ${MHD_LIBRARIES}
        nlohmann_json::nlohmann_json
        ${UUID_LIBRARIES}
        )
endforeach()

install(TARGETS ${PROJECT_NAME} EXPORT SofteqCommonTargets
  LIBRARY
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    COMPONENT shlib
)

include(CMakePackageConfigHelpers)

configure_file(cmake/softeq-${PROJECT_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/softeq-${PROJECT_NAME}Config.cmake
  COPYONLY
)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/softeq-${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/softeq-${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/softeq-${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/softeq-${PROJECT_NAME}
  COMPONENT
    dev
)

export(EXPORT SofteqCommonTargets
  FILE
    ${CMAKE_CURRENT_BINARY_DIR}/softeq-${PROJECT_NAME}Targets.cmake
  NAMESPACE
    softeq::
)

install(EXPORT SofteqCommonTargets
  FILE
    softeq-${PROJECT_NAME}Targets.cmake
  NAMESPACE
    softeq::
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/softeq-${PROJECT_NAME}
)

export(PACKAGE softeq-${PROJECT_NAME})

foreach ( file ${PUB_HDR} )
  get_filename_component( dir ${file} DIRECTORY )
  install( FILES ${file} DESTINATION ${dir} COMPONENT dev)
endforeach()

if (ENABLE_TESTING)
  # Each library has unit tests, of course
  message(STATUS "Tests are added to build")
  enable_testing()
  add_subdirectory(tests)

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    include(cmake/CodeCoverage.cmake)
    target_append_coverage_compiler_flags(${PROJECT_NAME}-utest)

    add_custom_target(coverage
      # Create folder
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/Coverage
      # Running gcovr
      COMMAND ${GCOVR_PATH}
              --exclude-throw-branches
              --exclude-unreachable-branches
              --filter ${PROJECT_SOURCE_DIR}/src
              --filter ${PROJECT_SOURCE_DIR}/include
              --html --html-details
              --print-summary
              --output=${PROJECT_BINARY_DIR}/Coverage/index.html
              --sonarqube=${PROJECT_BINARY_DIR}/Coverage/sonarcube.xml
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      COMMENT "Running 'gcovr' to produce code coverage report."
    )
    add_custom_command(TARGET coverage POST_BUILD
      COMMAND ;
      COMMENT "Open ${PROJECT_BINARY_DIR}/Coverage/index.html in your browser to view the coverage report."
    )

    include(CTest) # for valgrind
    add_custom_target(test_memcheck
      COMMAND ${CMAKE_CTEST_COMMAND} ${CMAKE_CTEST_ARGUMENTS} --force-new-ctest-process --test-action memcheck
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      COMMENT "Running unit tests with 'Valgrind'."
    )
  endif ()
endif()

# Each library has examples, of course
option(ENABLE_EXAMPLES "Enable building examples" OFF)
if (ENABLE_EXAMPLES)
  # Each library has unit tests, of course
  message(STATUS "Examples are added to build")
  add_subdirectory(examples)
endif()
