#pragma once

#ifdef _GLFW_X11
#define GLFW_EXPOSE_NATIVE_X11
#endif

#ifdef _GLFW_WAYLAND
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#ifdef _GLFW_COCOA
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#ifdef _GLFW_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <webgpu/webgpu_cpp.h>

#include <iostream>

namespace platform
{
wgpu::Surface
glfwCreateWGPUSurface(const wgpu::Instance& instance, GLFWwindow* window);
}
