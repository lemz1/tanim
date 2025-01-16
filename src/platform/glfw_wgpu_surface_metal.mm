#ifndef _GLFW_COCOA
#error "We require Metal Backend"
#endif

#include <GLFW/glfw3.h>
#import <QuartzCore/CAMetalLayer.h>
#include <webgpu/webgpu_cpp.h>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

namespace platform
{
wgpu::Surface glfwCreateWGPUSurfaceCocoa(const wgpu::Instance& instance, GLFWwindow* window)
{
  id metalLayer = [CAMetalLayer layer];
  NSWindow* nsWindow = glfwGetCocoaWindow(window);
  [nsWindow.contentView setWantsLayer:YES];
  [nsWindow.contentView setLayer:metalLayer];

  wgpu::SurfaceSourceMetalLayer source{};
  source.layer = metalLayer;

  wgpu::SurfaceDescriptor descriptor{};
  descriptor.nextInChain = &source;
  descriptor.label = "Surface";

  return instance.CreateSurface(&descriptor);
}
}  // namespace platform
