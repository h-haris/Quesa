# CMakeLists.txt for Quesa XPC Service
# Alternative build configuration using CMake

cmake_minimum_required(VERSION 3.15)

# Option to enable XPC
option(QUESA_USE_XPC "Use XPC instead of PDO for IPC" ON)

if(QUESA_USE_XPC)
    # XPC Service Target
    project(Quesa3DDeviceServer)
    
    # Minimum macOS version for XPC
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version")
    
    # Source files for XPC service
    set(XPC_SERVICE_SOURCES
        Q3DdbXPC.mm
        Q3DcontrollerXPC.mm
    )
    
    # Header files
    set(XPC_SERVICE_HEADERS
        IPCprotocolXPC.h
        # Add other required Quesa headers
    )
    
    # Create XPC service bundle
    add_executable(Quesa3DDeviceServer MACOSX_BUNDLE ${XPC_SERVICE_SOURCES})
    
    # Set bundle properties
    set_target_properties(Quesa3DDeviceServer PROPERTIES
        BUNDLE TRUE
        BUNDLE_EXTENSION "xpc"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Quesa3DDeviceServer-Info.plist"
        MACOSX_BUNDLE_BUNDLE_NAME "Quesa3DDeviceServer"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.quesa.osx.3ddevice.server.xpc"
    )
    
    # Compile options
    target_compile_options(Quesa3DDeviceServer PRIVATE
        -Wall
        -Wextra
        -std=c++14
    )
    
    # Preprocessor definitions
    target_compile_definitions(Quesa3DDeviceServer PRIVATE
        QUESA_USE_XPC=1
    )
    
    # Include directories
    target_include_directories(Quesa3DDeviceServer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
        # Add other include paths as needed
    )
    
    # Link frameworks
    target_link_libraries(Quesa3DDeviceServer
        "-framework Foundation"
        "-framework CoreFoundation"
    )
    
    # Installation
    install(TARGETS Quesa3DDeviceServer
        BUNDLE DESTINATION XPCServices
    )
    
    message(STATUS "Configured Quesa XPC Service target")
    message(STATUS "  Bundle ID: com.quesa.osx.3ddevice.server.xpc")
    message(STATUS "  Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

# Quesa Framework with XPC
project(QuesaFramework)

set(QUESA_SOURCES
    # Add your Quesa framework sources here
    # ...
)

if(QUESA_USE_XPC)
    list(APPEND QUESA_SOURCES
        ControllerCoreOSX_XPC.mm
    )
    add_compile_definitions(QUESA_USE_XPC=1)
else()
    list(APPEND QUESA_SOURCES
        ControllerCoreOSX.mm
    )
endif()

# Create framework target
add_library(Quesa SHARED ${QUESA_SOURCES})

set_target_properties(Quesa PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_IDENTIFIER com.quesa.framework
    VERSION 1.0.0
    SOVERSION 1.0.0
    PUBLIC_HEADER "Quesa.h"
)

if(QUESA_USE_XPC)
    set_target_properties(Quesa PROPERTIES
        OUTPUT_NAME "Quesa-XPC"
    )
endif()

# Link frameworks
target_link_libraries(Quesa
    "-framework Foundation"
    "-framework CoreFoundation"
    "-framework Cocoa"
)

# Include directories
target_include_directories(Quesa PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

# Installation
install(TARGETS Quesa
    FRAMEWORK DESTINATION Library/Frameworks
)

message(STATUS "Configured Quesa Framework")
message(STATUS "  XPC Enabled: ${QUESA_USE_XPC}")
if(QUESA_USE_XPC)
    message(STATUS "  Framework Name: Quesa-XPC.framework")
else()
    message(STATUS "  Framework Name: Quesa.framework")
endif()
