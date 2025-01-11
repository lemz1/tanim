cmake_minimum_required(VERSION 3.10)

project(AnimationLibrary VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(OUTPUT_DIR_CONFIG "unknown")
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(OUTPUT_DIR_CONFIG "debug")
elseif(CMAKE_BUILD_TYPE MATCHES "Release")
    set(OUTPUT_DIR_CONFIG "release")
endif()

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/export/${OUTPUT_DIR_CONFIG}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIR}")

set(SOURCES 
  src/main.cpp
)

set(HEADERS

)

if (WIN32)
  list(APPEND CMAKE_PREFIX_PATH "vnd/webgpu/webgpu/windows-latest")
elseif (APPLE)
  list(APPEND CMAKE_PREFIX_PATH "vnd/webgpu/webgpu/macos-latest")
elseif (UNIX)
  list(APPEND CMAKE_PREFIX_PATH "vnd/webgpu/webgpu/ubuntu-latest")
endif()

find_package(Dawn REQUIRED)

add_library(webgpu ALIAS dawn::webgpu_dawn) # required for glfw3webgpu

add_subdirectory(vnd/glfw)
add_subdirectory(vnd/glfw3webgpu)

add_executable(AnimationLibrary ${SOURCES} ${HEADERS})

target_link_libraries(AnimationLibrary webgpu)

add_custom_command(TARGET AnimationLibrary POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  $<TARGET_FILE:dawn::webgpu_dawn>
  $<TARGET_FILE_DIR:AnimationLibrary>
)
