cmake_minimum_required(VERSION 3.10)

project(tanim VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(OUTPUT_DIR_CONFIG "unknown")
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(OUTPUT_DIR_CONFIG "debug")
elseif (CMAKE_BUILD_TYPE MATCHES "Release")
    set(OUTPUT_DIR_CONFIG "release")
endif ()

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/export/${OUTPUT_DIR_CONFIG}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}")

set(TANIM_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(TANIM_SOURCES 
  ${TANIM_DIR}/src/main.cpp
  ${TANIM_DIR}/src/vendor.cpp
  ${TANIM_DIR}/src/platform/glfw_wgpu_surface.cpp
  ${TANIM_DIR}/src/graphics/renderer.cpp
  ${TANIM_DIR}/src/graphics/font.cpp
)

if (APPLE)
  list(APPEND TANIM_SOURCES ${TANIM_DIR}/src/platform/glfw_wgpu_surface_metal.mm)
endif ()

set(TANIM_HEADERS
  ${TANIM_DIR}/src/platform/glfw_wgpu_surface.h
  ${TANIM_DIR}/src/graphics/renderer.h
  ${TANIM_DIR}/src/graphics/font.h
)

add_executable(tanim ${TANIM_SOURCES} ${TANIM_HEADERS})

target_include_directories(tanim PRIVATE ${TANIM_DIR}/src)

# Assets

if(APPLE)
add_custom_command(TARGET tanim POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${TANIM_DIR}/assets
  $<TARGET_FILE_DIR:tanim>/../Resources/assets
)
else()
add_custom_command(TARGET tanim POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${TANIM_DIR}/assets
  $<TARGET_FILE_DIR:tanim>/assets
)
endif()

# Shared Library RPATH

if (UNIX AND NOT APPLE)
  set_target_properties(tanim PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN"
  )
elseif (APPLE)
    set_target_properties(tanim PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.lemz1.tanim
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
endif ()

# Dawn

if (WIN32)
  list(APPEND CMAKE_PREFIX_PATH "vnd/webgpu/webgpu/windows-latest")
elseif (APPLE)
  list(APPEND CMAKE_PREFIX_PATH "vnd/webgpu/webgpu/macos-latest")
elseif (UNIX)
  list(APPEND CMAKE_PREFIX_PATH "vnd/webgpu/webgpu/ubuntu-latest")
endif ()

find_package(Dawn REQUIRED)

target_link_libraries(tanim PRIVATE dawn::webgpu_dawn)

if(APPLE)
    add_custom_command(TARGET tanim POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:tanim>/../Frameworks
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:dawn::webgpu_dawn>
        $<TARGET_FILE_DIR:tanim>/../Frameworks/
    )
else()
    add_custom_command(TARGET tanim POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:dawn::webgpu_dawn>
        $<TARGET_FILE_DIR:tanim>
    )
endif()


# GLFW

add_subdirectory(vnd/glfw)
target_link_libraries(tanim PRIVATE glfw)

if (GLFW_BUILD_WIN32)
  target_compile_definitions(tanim PRIVATE _GLFW_WIN32)
elseif (GLFW_BUILD_X11)
  target_compile_definitions(tanim PRIVATE _GLFW_X11)
elseif (GLFW_BUILD_WAYLAND)
  target_compile_definitions(tanim PRIVATE _GLFW_WAYLAND)
elseif (GLFW_BUILD_COCOA)
  target_compile_definitions(tanim PRIVATE _GLFW_COCOA)
  target_link_libraries(tanim PRIVATE "-framework Metal" "-framework QuartzCore")
endif ()

# GLM

target_include_directories(tanim PRIVATE ${TANIM_DIR}/vnd/glm)

# JSON

target_include_directories(tanim PRIVATE ${TANIM_DIR}/vnd/json/single_include)

# STB

target_include_directories(tanim PRIVATE ${TANIM_DIR}/vnd/stb)
