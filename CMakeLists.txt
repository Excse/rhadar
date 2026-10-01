### --- Project Initialization and Configuration --- ###
cmake_minimum_required(VERSION 3.24)             # Compatible with the ESP-IDF CMake toolchain
project(rhadar VERSION 0.1.0 LANGUAGES CXX) # Define project name, version, and language

# Use C++23 and enforce it
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to enable/disable testing:
option(BUILD_TESTING "Enable testing" ${PROJECT_IS_TOP_LEVEL})
# Option to enable/disable executable:
option(BUILD_EXECUTABLE "Create executable" ${PROJECT_IS_TOP_LEVEL})
# Option to enable/disable installation:
option(MAKE_INSTALLABLE "Make the library installable" ${PROJECT_IS_TOP_LEVEL})
# Option to control static vs shared library
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

### --- CMake Modules for Installation and Fetching --- ###
include(CMakePackageConfigHelpers) # Helper for creating config files for package managers
include(GNUInstallDirs)            # Define standard install directories (lib, include, etc.)
include(FetchContent)              # For downloading and using external dependencies


### --- Define Library Target --- ##

# Create the main library from source files
add_library(${PROJECT_NAME}
        src/components/base.cpp
        src/components/sensor.cpp
        src/connection.cpp
        src/device.cpp
        src/message.cpp
        src/origin.cpp
        src/utils/json.cpp
        src/utils/serializer.cpp)
# Create an alias for the library: rhadar::rhadar
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
# Propagate the required C++ standard to consumers, including installed targets.
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

# Setup include directories for consumers and installers
target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> # For build tree
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)      # For install tree

# Define public headers
set(PUBLIC_HEADERS
        include/components/base.h
        include/components/sensor.h
        include/connection.h
        include/device.h
        include/message.h
        include/origin.h
        include/result.h
        include/rhadar.h
        include/utils/enum.h
        include/utils/json.h
        include/utils/serializer.h)

# Include public headers in the library target.
target_sources(${PROJECT_NAME} PRIVATE ${PUBLIC_HEADERS})

# Set the properties of the resulting library
set_target_properties(${PROJECT_NAME}
        PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})


### --- Define Executable Target --- ###

if(BUILD_EXECUTABLE)
    # Create the main executable from the main source file
    add_executable(${PROJECT_NAME}_app src/main.cpp)
    # Link the library with the application
    target_link_libraries(${PROJECT_NAME}_app PRIVATE ${PROJECT_NAME})
    # Includes the headers to the application
    target_include_directories(${PROJECT_NAME}_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()


### --- Install Rules --- ###

if(MAKE_INSTALLABLE)
    # Install the library and its headers
    install(TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}Targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    # Preserve header subdirectories so installed include paths match the build tree.
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

    # Install the target export file (for find_package)
    install(EXPORT ${PROJECT_NAME}Targets
            FILE ${PROJECT_NAME}Targets.cmake
            NAMESPACE ${PROJECT_NAME}::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    # Generate version file for package configuration
    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion)

    # Creates a config file for package configuration
    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    # Install the generated config and config version file
    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
endif ()

### --- Unit Testing Setup --- ###

if(BUILD_TESTING)
    # Enable testing framework
    enable_testing()
    # Adds the test directory to the build
    add_subdirectory(tests)
endif()
