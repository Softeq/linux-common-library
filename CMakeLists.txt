cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
set(PRODUCT_NAMESPACE "softeq")
project(common VERSION 0.4.0 LANGUAGES CXX)

set(BUILD_ALL ON CACHE BOOL "Build everything")

include(tools/cmake/SofteqComponentsModel.cmake)
#it MUST be shared lib due to LGPL
make_softeq_component(${PROJECT_NAME} SHARED)

add_library(${PRODUCT_NAMESPACE}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

################################### PROJECT SPECIFIC GLOBALS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
# -O0            (optimization level: -O0, -O1, -O2, -O3, -Os, -Og, -Ofast)
# -Wall          (enables all compiler's warning messages)
# -Wextra        (nables some extra warning flags that are not enabled by -Wall)
# -Wpedantic     (issue all the warnings demanded by strict ISO C and ISO C++)
# -Wfatal-errors (compiler to abort compilation on the first error)
# -Werror        (warning == error)
# -pthread       (multithrading enable)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  LINKER_LANGUAGE CXX
  OUTPUT_NAME "${PRODUCT_NAMESPACE}${PROJECT_NAME}"
  VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
  SOVERSION ${PROJECT_VERSION_MAJOR}
  )

configure_file(include/common/version.hh.in
  include/${COMPONENT_PATH}/version.hh
  )

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_BINARY_DIR}/include/${COMPONENT_PATH}
  )

################################### COMPONENT SOURCES

target_sources(${PROJECT_NAME}
  PRIVATE
  src/version.cc
  )

set(PUBLIC_HEADERS
  ${CMAKE_BINARY_DIR}/include/${COMPONENT_PATH}/version.hh
  )

########################################### SUBCOMPONENTS

option(ENABLE_LOGGING "Logging" ${BUILD_ALL})
if (ENABLE_LOGGING)
  add_subdirectory(components/logging)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
    common-logging
    )
endif ()

option(ENABLE_STDUTILS "Standatd library utils" ${BUILD_ALL})
if (ENABLE_STDUTILS)
  add_subdirectory(components/stdutils)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
    common-stdutils
    )
endif ()

if (ENABLE_LOGGING AND ENABLE_STDUTILS)
  option(ENABLE_SYSTEM "System extensions" ${BUILD_ALL})
  if (ENABLE_SYSTEM)
    add_subdirectory(components/system)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      common-system
      )
  endif ()
endif ()

if (ENABLE_STDUTILS)
  option(ENABLE_SERIALIZATION "Serialization" ${BUILD_ALL})
  if (ENABLE_SERIALIZATION)
    add_subdirectory(components/serialization)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      common-serialization
      $<TARGET_PROPERTY:common-serialization,INTERFACE_LINK_LIBRARIES>
      )
endif ()
endif ()

if (ENABLE_SERIALIZATION)
  option(ENABLE_SETTINGS "Settings" ${BUILD_ALL})
  if (ENABLE_SETTINGS)
    add_subdirectory(components/settings)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      common-settings
      )
  endif ()
endif ()

if (ENABLE_LOGGING AND ENABLE_STDUTILS AND ENABLE_SETTINGS)
  option(ENABLE_MIGRATION "Migration" ${BUILD_ALL})
  if (ENABLE_MIGRATION)
    add_subdirectory(components/migration)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      common-migration
      )
  endif ()
endif ()

if (ENABLE_LOGGING AND ENABLE_STDUTILS)
  option(ENABLE_NET_CURL "Curl wrapper" ${BUILD_ALL})
  if (ENABLE_NET_CURL)
    add_subdirectory(components/net/curl_helper)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      common-net-curl_helper
      )
  endif ()
endif ()

if (ENABLE_SYSTEM)
  option(ENABLE_NET_HTTP "Htpp(s) server implementation" ${BUILD_ALL})
  if (ENABLE_NET_HTTP)
    add_subdirectory(components/net/http)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      common-net-http
      )
  endif ()
endif ()

if (ENABLE_NET_HTTP AND ENABLE_SERIALIZATION AND ENABLE_SERIALIZATION_JSON)
  option(ENABLE_NET_REST "REST handler implementation" ${BUILD_ALL})
  if (ENABLE_NET_REST)
    add_subdirectory(components/net/rest)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      common-net-rest
      )
  endif ()
endif ()

option(BUILD_EXAMPLES "Enable building examples")
if (BUILD_EXAMPLES)
  message(STATUS "* Examples are added to build")
  add_subdirectory(examples)
endif ()

add_softeq_testing()

########################################### INSTALLATION

install_softeq_framework(${PROJECT_NAME})
