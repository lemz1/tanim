#include "glfw_webgpu_surface.h"

namespace platform
{
wgpu::Surface
glfwCreateWGPUSurface(const wgpu::Instance& instance, GLFWwindow* window)
{
#ifdef _GLFW_WIN32
  HWND hwnd = glfwGetWin32Window(window);
  HINSTANCE hinstance = GetModuleHandle(NULL);

  wgpu::SurfaceSourceWindowsHWND sourceWindows{};
  sourceWindows.hwnd = hwnd;
  sourceWindows.hinstance = hinstance;
  sourceWindows.sType = wgpu::SType::SurfaceSourceWindowsHWND;

  wgpu::SurfaceDescriptor descriptor{};
  descriptor.nextInChain = &sourceWindows;

  return instance.CreateSurface(&descriptor);
#else
  return wgpu::Surface();
#endif
}
}  // namespace platform
