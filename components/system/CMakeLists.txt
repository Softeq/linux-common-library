make_softeq_component(system OBJECT)

################################### PROJECT SPECIFIC GLOBALS
option(USE_SYSTEMD "Use systemd as service manager" ON)
if (USE_SYSTEMD)
  option(USE_SYSTEMBUS "Use system sdbus bus" ON)
  if (USE_SYSTEMBUS)
    add_definitions(-DUSE_SYSTEMBUS)
  endif ()
endif ()

################################### COMPONENT SOURCES
find_package(Threads REQUIRED)

target_sources(${PROJECT_NAME}
  PRIVATE
  src/fsutils.cc
  src/getopt_wrapper.cc
  src/cron.cc
  src/service.cc
  src/signal_manager.cc
  src/time_provider.cc
  )

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  PRIVATE
  common-stdutils
  common-logging
  ${CMAKE_THREAD_LIBS_INIT}
  )

if (USE_SYSTEMD)
  pkg_check_modules(SYSTEMD REQUIRED libsystemd)

  target_sources(${PROJECT_NAME}
    PRIVATE
    src/sd_bus.cc
    )

  target_compile_definitions(${PROJECT_NAME}
    PUBLIC
    WITH_SYSTEMD)

  target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${SYSTEMD_INCLUDE_DIRS}
    )

  target_link_libraries(${PROJECT_NAME}
    PUBLIC
    ${SYSTEMD_LIBRARIES}
    )
endif ()

################################### SUBCOMPONENTS
if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

################################### INSTALLATION
set(PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/cron.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/time_provider.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/fsutils.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/getopt_wrapper.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/service.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/signal_manager.hh
  )

if (USE_SYSTEMD)
  set(PUBLIC_HEADERS
    ${PUBLIC_HEADERS}
    ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/sd_bus.hh
  )
endif ()

deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${PUBLIC_HEADERS}      
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
