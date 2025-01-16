#pragma once

#include <GLFW/glfw3.h>
#include <webgpu/webgpu_cpp.h>

#include <iostream>

namespace platform
{
wgpu::Surface
glfwCreateWGPUSurface(const wgpu::Instance& instance, GLFWwindow* window);
}
