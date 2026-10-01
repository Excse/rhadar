cmake_minimum_required(VERSION 3.21)
project(rhadar VERSION 0.1.0 LANGUAGES CXX)

option(RHADAR_BUILD_TESTING "Build rhadar host tests" ${PROJECT_IS_TOP_LEVEL})
option(RHADAR_INSTALL "Enable rhadar installation" ${PROJECT_IS_TOP_LEVEL})
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
include(GNUInstallDirs)

add_library(rhadar
    src/connection.cpp
    src/device.cpp
    src/message.cpp
    src/origin.cpp
    src/components/base.cpp
    src/components/sensor.cpp
    src/utils/json.cpp
    src/utils/serializer.cpp)
add_library(rhadar::rhadar ALIAS rhadar)
target_compile_features(rhadar PUBLIC cxx_std_23)
target_include_directories(rhadar PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
set_target_properties(rhadar PROPERTIES
    CXX_EXTENSIONS OFF
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

if(RHADAR_BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

if(RHADAR_INSTALL)
    include(CMakePackageConfigHelpers)
    install(TARGETS rhadar EXPORT rhadarTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT rhadarTargets FILE rhadarTargets.cmake
        NAMESPACE rhadar:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rhadar)
    configure_package_config_file(cmake/Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/rhadarConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rhadar)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/rhadarConfigVersion.cmake
        VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/rhadarConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/rhadarConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rhadar)
endif()
