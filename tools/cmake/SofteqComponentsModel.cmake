if (NOT DEFINED PRODUCT_NAMESPACE)
  message(FATAL_ERROR "* Please define PRODUCT_NAMESPACE")
endif ()

# Build a release version by default
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
  message(FATAL_ERROR "* Unsupported CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif ()
message(STATUS "* Build type: ${CMAKE_BUILD_TYPE}")

set(BUILD_EXAMPLES ${BUILD_ALL} CACHE BOOL "Build example")
set(BUILD_TESTING ${BUILD_ALL} CACHE BOOL "Build tests")
option(BUILD_COVERAGE "Prepare coverage" ${BUILD_ALL})

include(FindPkgConfig)
include(CTest)
if (BUILD_TESTING)
  find_package(GTest REQUIRED)
endif ()
if (BUILD_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  include(${CMAKE_SOURCE_DIR}/tools/cmake/CodeCoverage.cmake)
endif ()

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_FIND_USE_PACKAGE_REGISTRY YES)

# enable -fPIC for all targets because of licence and we cannot install static libs
# TODO: may be it is better to add the flag only for targets of type OBJECT
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(GLOBAL_INSTALL_EXPORT_NAME "${PRODUCT_NAMESPACE}-${CMAKE_PROJECT_NAME}")

include(CMakeParseArguments)
##################################################################################
##################################################################################
##################################################################################
function(MAKE_SOFTEQ_COMPONENT NAME TYPE)
  set(options STANDALONE)
  set(oneValueArgs GROUP)
  set(multiValueArgs)
  cmake_parse_arguments(PARSE_ARGV 2 ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}")

  #each component may have own tools dir with additional cmake scripts
  set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake" ${CMAKE_MODULE_PATH} PARENT_SCOPE)

  #collect pathes for test coverage
  set(COMPONENTS_ABSPATHES ${CMAKE_CURRENT_SOURCE_DIR} ${COMPONENTS_ABSPATHES} CACHE INTERNAL "COMPONENTS_ABSPATHES")

  string(REPLACE ";" "-" PARENT_COMPONENT_NAME "${COMPONENTS}") 
  set(PARENT_COMPONENT_NAME ${PARENT_COMPONENT_NAME} PARENT_SCOPE)

  #COMPONENTS var will store full ierarchy of the component
  if (DEFINED ARGS_GROUP)
    string(REPLACE "/" ";" GROUP_LIST "${ARGS_GROUP}")
    list(APPEND COMPONENTS ${GROUP_LIST})
  endif ()
  list(APPEND COMPONENTS ${NAME})
  set(COMPONENTS "${COMPONENTS}" PARENT_SCOPE)

  set(COMPONENT_GROUP ${GROUP_LIST} PARENT_SCOPE)

  if (ARGS_STANALONE)
    set(COMPONENT_NAME "${NAME}")
  else ()
    string(REPLACE ";" "-" COMPONENT_NAME "${COMPONENTS}") 
  endif ()
  message(STATUS "* Making component ${COMPONENT_NAME}")

  project(${COMPONENT_NAME})
  set(PROJECT_NAME ${PROJECT_NAME} PARENT_SCOPE)

  #build FS path of the component
  string(REPLACE ";" "/" COMPONENT_PATH "${COMPONENTS}")
  string(PREPEND COMPONENT_PATH "/")
  set(COMPONENT_PATH "${COMPONENT_PATH}" PARENT_SCOPE)

  if (TYPE STREQUAL "EXECUTABLE")
    add_executable(${PROJECT_NAME})
  else ()
    add_library(${PROJECT_NAME} ${TYPE})
  endif ()

  if (BUILD_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT (TYPE STREQUAL "INTERFACE"))
    target_append_coverage_compiler_flags(${PROJECT_NAME})
  endif ()

  if (TYPE STREQUAL "INTERFACE")
    target_include_directories(${PROJECT_NAME}
      INTERFACE
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/../>
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../>
      $<INSTALL_INTERFACE:include/${PRODUCT_NAMESPACE}/>
      )
  else ()
    target_include_directories("${PROJECT_NAME}"
      PRIVATE
      #project installable includes
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
      #within component you can easyly access component's includes by "includename.hh" from installable dir
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/>
      #private includes in source directory
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/>
      #within component you can easyly access component's includes by "includename.hh" from private dir
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${NAME}>
      PUBLIC
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/../>
      $<INSTALL_INTERFACE:include/${PRODUCT_NAMESPACE}/>
      )
    if (EXISTS "${PROJECT_SOURCE_DIR}/include/")
      target_include_directories(${PROJECT_NAME}
        PUBLIC
        #private dir is accessible within whole project by "component/includename.hh"
        #NOTE: there is an assumption. only first layer of includes can be used and is useful
        #"includename.hh" from the component and "component/includename.hh" from the project
        #developer must be sure to do not link several projects with the same name on different
        #layers to the component. Such linkage can be covered by some pproject in the middle
        #  /project
        #    /layer_1_1
        #      /layer_2_1
        #        /samename
        #    /layer_1_2
        #      /samename
        #
        # ERROR:  project uses(links) components layer_1_1-layer_2_1-samename + components layer_1_1-samename
        # OK: layer_2_1 uses(links) components layer_1_1-layer_2_1-samename
        #     project uses(links) components layer_1_1-layer_2_1 + components layer_1_1-samename
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
        )
    endif ()
  endif ()

endfunction ()

##################################################################################
##################################################################################
##################################################################################
function (ADD_SOFTEQ_TESTING)
  if (BUILD_TESTING)
    # Each library has unit tests, of course
    # dependency on GTest is required on this layer. otherwise it will not find tests, build, work
    message(STATUS "* Tests are added to build")
    enable_testing()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
      option(BUILD_COVERAGE "Enable tests coverage" OFF)
    endif ()
    if (BUILD_COVERAGE)
      message(STATUS "* Tests coverage is added to build")

      list(REMOVE_DUPLICATES COMPONENTS_ABSPATHES)
      list(FILTER COMPONENTS_ABSPATHES EXCLUDE REGEX "tests")
      list(TRANSFORM COMPONENTS_ABSPATHES PREPEND "-f=")
      list(TRANSFORM COMPONENTS_ABSPATHES APPEND "/src/")
      list(APPEND COMPONENTS_ABSPATHES "-f=${CMAKE_SOURCE_DIR}/include/")

      add_custom_target(coverage
        # Create folder
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/Coverage
        # Running gcovr
        COMMAND ${GCOVR_PATH}
        --exclude-throw-branches
        --exclude-unreachable-branches
        ${COMPONENTS_ABSPATHES}
        --html --html-details --output=${PROJECT_BINARY_DIR}/Coverage/index.html
        --sonarqube=${PROJECT_BINARY_DIR}/Coverage/sonarcube.xml
        --print-summary --delete
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMENT "Running 'gcovr' to produce code coverage report."
        )

      add_custom_command(TARGET coverage POST_BUILD
        COMMAND ;
        COMMENT "Open file://${PROJECT_BINARY_DIR}/Coverage/index.html in your browser to view the coverage report."
        )
    endif ()

    add_custom_target(test_memcheck
      COMMAND ${CMAKE_CTEST_COMMAND} ${CMAKE_CTEST_ARGUMENTS} --force-new-ctest-process --test-action memcheck
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
      COMMENT "Running unit tests with 'Valgrind'."
      )
  endif ()
endfunction ()

##################################################################################
##################################################################################
##################################################################################
function (DEPLOY_SOFTEQ_COMPONENT NAME)
  set(options EXPORTS)
  set(oneValueArgs)
  set(multiValueArgs PUBLIC_HEADERS INSTALL_PARAMS NESTED_DIR)
  cmake_parse_arguments(PARSE_ARGV 1 ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}")

  set_target_properties(${NAME}
    PROPERTIES
    PUBLIC_HEADER "${ARGS_PUBLIC_HEADERS}"
    )

  if (ARGS_EXPORTS)
    install(TARGETS ${NAME}
      EXPORT "${GLOBAL_INSTALL_EXPORT_NAME}"
      PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PRODUCT_NAMESPACE}/${COMPONENT_PATH}"
      ${ARGS_INSTALL_PARAMS}
      )
  else ()
    install(TARGETS ${NAME}
      PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PRODUCT_NAMESPACE}/${COMPONENT_PATH}"
      ${ARGS_INSTALL_PARAMS}
      )
  endif ()

  # TODO: If multiple nested directories have to be installed
  #       the following command should be executed in the loop
  install(DIRECTORY ${ARGS_NESTED_DIR} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PRODUCT_NAMESPACE}/${COMPONENT_PATH})
endfunction ()

##################################################################################
##################################################################################
##################################################################################
function (INSTALL_SOFTEQ_FRAMEWORK NAME)
  # Installation layout. This works for all platforms:
  #   * <prefix>/lib*/cmake/<PRODUCT_NAMESPACE>-<NAME>/
  #   * <prefix>/lib*/
  #   * <prefix>/include/<PRODUCT_NAMESPACE>

  # Include module with function 'write_basic_package_version_file'
  include(CMakePackageConfigHelpers)

  #prepare cmake-version file
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT_NAMESPACE}-${NAME}ConfigVersion.cmake"
    COMPATIBILITY SameMinorVersion
    )

  #prepare cmake-build-configurations file
  configure_package_config_file(
    "data/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT_NAMESPACE}-${NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PRODUCT_NAMESPACE}-${NAME}"
    )

  #install cmake-version cmake-build-configurations files
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT_NAMESPACE}-${NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PRODUCT_NAMESPACE}-${NAME}ConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PRODUCT_NAMESPACE}-${NAME}"
    )

  deploy_softeq_component(${NAME} EXPORTS
    PUBLIC_HEADERS ${PUBLIC_HEADERS}
    INSTALL_PARAMS
    INCLUDES DESTINATION include/${PRODUCT_NAMESPACE}
    )

  #creates cmake-targets file in build dir for cmake registry
  export(EXPORT
    "${GLOBAL_INSTALL_EXPORT_NAME}"
    FILE
    ${CMAKE_CURRENT_BINARY_DIR}/${GLOBAL_INSTALL_EXPORT_NAME}.cmake
    NAMESPACE
    ${PRODUCT_NAMESPACE}::
    )

  #install cmake-targets file to PREFIX_DIR
  install(EXPORT
    "${GLOBAL_INSTALL_EXPORT_NAME}"
    NAMESPACE "${PRODUCT_NAMESPACE}::"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PRODUCT_NAMESPACE}-${NAME}"
    )

  #exports the package to the registry
  export(PACKAGE ${PRODUCT_NAMESPACE}-${NAME})

  # build a CPack driven installer package
  include(InstallRequiredSystemLibraries)
  set(CPACK_SET_DESTDIR ON)
  set(CPACK_PACKAGE_CONTACT "support@softeq.com")
  set(CPACK_GENERATOR "STGZ;TGZ")
  set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_BUILD}")

  include(CPack)

endfunction ()
