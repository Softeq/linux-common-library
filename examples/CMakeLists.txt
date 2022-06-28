project(examples)

# Crom example
add_executable(cron_example
  cron.cc
  )
target_link_libraries(cron_example softeq::common)

# Getopt example
add_executable(getopt_example
  getopt.cc
  )
target_link_libraries(getopt_example softeq::common)

# basic UNIX service example
add_executable(service_example
  service.cc
  )
target_link_libraries(service_example softeq::common)

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
  target_link_libraries(http_server_example ${LibMagic_LIBRARY})
endif ()

target_link_libraries(http_server_example softeq::common)

# Settings example
add_executable(settings_example
  settings.cc
  )
target_link_libraries(settings_example softeq::common)

# Curl example
add_executable(curl_example
  curl_example.cc
  )
target_link_libraries(curl_example softeq::common)

# REST server with autodoc example
add_executable(rest_server_example
  rest_server.cc
  )
target_compile_definitions(rest_server_example
  PRIVATE
  WEB_TRANSFORM_PATH="${CMAKE_CURRENT_SOURCE_DIR}/web_transform.xslt"
  )
target_link_libraries(rest_server_example softeq::common)

# Logger example
add_executable(logger_example
  logger.cc
  )
target_link_libraries(logger_example softeq::common)
