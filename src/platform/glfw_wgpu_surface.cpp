#include "glfw_wgpu_surface.h"

#include <cassert>

#ifdef _GLFW_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#ifdef _GLFW_X11
#define GLFW_EXPOSE_NATIVE_X11
#endif

#ifdef _GLFW_WAYLAND
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#include <GLFW/glfw3native.h>

namespace platform
{
wgpu::Surface
glfwCreateWGPUSurfaceCocoa(const wgpu::Instance& instance, GLFWwindow* window);

wgpu::Surface
glfwCreateWGPUSurface(const wgpu::Instance& instance, GLFWwindow* window)
{
#ifdef _GLFW_WIN32
  wgpu::SurfaceSourceWindowsHWND source{};
  source.hinstance = GetModuleHandle(NULL);
  source.hwnd = glfwGetWin32Window(window);
  source.sType = wgpu::SType::SurfaceSourceWindowsHWND;

  wgpu::SurfaceDescriptor descriptor{};
  descriptor.nextInChain = &source;
  descriptor.label = "Surface";

  return instance.CreateSurface(&descriptor);
#endif

#ifdef _GLFW_X11
  wgpu::SurfaceSourceXlibWindow source{};
  source.display = glfwGetX11Display();
  source.window = glfwGetX11Window(windnow);
  source.sType = wgpu::SType::SurfaceSourceXlibWindow;

  wgpu::SurfaceDescriptor descriptor{};
  descriptor.nextInChain = &source;
  descriptor.label = "Surface";

  return instance.CreateSurface(&descriptor);
#endif

#ifdef _GLFW_WAYLAND
  wgpu::SurfaceSourceWaylandSurface source{};
  source.display = glfwGetWaylandDisplay();
  source.surface = glfwGetWaylandWindow(window);

  wgpu::SurfaceDescriptor descriptor{};
  descriptor.nextInChain = &source;
  descriptor.label = "Surface";

  return instance.CreateSurface(&descriptor);
#endif

#ifdef _GLFW_COCOA
  return glfwCreateWGPUSurfaceCocoa(instance, window);
#endif

  std::cerr << "[WebGPU] Surface not supported" << std::endl;
  assert(false);
}
}  // namespace platform
