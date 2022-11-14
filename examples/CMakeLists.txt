cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(examples LANGUAGES CXX)

if (NOT BUILD_EXAMPLES)
  set(PRODUCT_NAMESPACE "softeq")
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  find_package(softeq-common REQUIRED)
endif()

link_libraries(
  softeq::common
  )

if (ENABLE_SYSTEM OR BUILD_ALL)
# Cron example
add_executable(cron_example
  cron.cc
  )

  # Getopt example
  add_executable(getopt_example
    getopt.cc
    )

  # basic UNIX service example
  add_executable(service_example
    service.cc
    )
endif()

if (ENABLE_NET_HTTP AND ENABLE_SETTINGS OR BUILD_ALL)
# HTTP/HTTPS example
add_executable(http_server_example
  http_server.cc)
find_path(LibMagic_ROOT_DIR
  NAMES include/magic.h
    )
  find_library(LibMagic_LIBRARY
    NAMES magic
    HINTS ${LibMagic_ROOT_DIR}/lib
    )
  if (LibMagic_LIBRARY)
    message(STATUS "LibMagic FOUND at ${LibMagic_LIBRARY}")
    target_compile_definitions(http_server_example
      PRIVATE
      HAVE_MAGIC
      )
    target_link_libraries(http_server_example
      PRIVATE
      ${LibMagic_LIBRARY}
      )
  endif ()
endif ()

if (ENABLE_SETTINGS OR BUILD_ALL)
# Settings example
add_executable(settings_example
  settings.cc
  )
endif ()

if (ENABLE_NET_CURL OR BUILD_ALL)
# Curl example
add_executable(curl_example
  curl_example.cc
  )
endif ()

if (ENABLE_NET_REST OR BUILD_ALL)
# REST server with autodoc example
add_executable(rest_server_example
  rest_server.cc
  )
target_compile_definitions(rest_server_example
  PRIVATE
  WEB_TRANSFORM_PATH="${CMAKE_CURRENT_SOURCE_DIR}/web_transform.xslt"
  )
endif ()

if (ENABLE_LOGGING OR BUILD_ALL)
# Logger example
add_executable(logger_example
  logger.cc
  )
endif ()


