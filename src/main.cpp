#include <GLFW/glfw3.h>
#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <iostream>

#include "platform/glfw_wgpu_surface.h"

int main()
{
  if (!glfwInit())
  {
    std::cerr << "[GLFW] Could not initialize GLFW" << std::endl;
    return 1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window =
    glfwCreateWindow(1280, 720, "Animation", nullptr, nullptr);
  if (!window)
  {
    std::cerr << "[GLFW] Could not create Window" << std::endl;
    glfwTerminate();
    return 1;
  }

#ifdef __EMSCRIPTEN__
  auto instance = wgpu::CreateInstance(nullptr);
#else
  wgpu::InstanceDescriptor instanceDescriptor{};
  instanceDescriptor.features.timedWaitAnyEnable = true;
  auto instance = wgpu::CreateInstance(&instanceDescriptor);
#endif

  if (!instance)
  {
    std::cerr << "[WebGPU] Could not create Instance" << std::endl;
    return 1;
  }

  wgpu::RequestAdapterOptions adapterOptions{};
  wgpu::Adapter adapter;

  wgpu::RequestAdapterCallbackInfo adapaterCallbackInfo{};
  adapaterCallbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
  adapaterCallbackInfo.userdata = &adapter;
  adapaterCallbackInfo.callback = [](
                                    WGPURequestAdapterStatus status,
                                    WGPUAdapter adapter,
                                    WGPUStringView message,
                                    void* userdata
                                  )
  {
    auto& data = *static_cast<wgpu::Adapter*>(userdata);
    data = wgpu::Adapter::Acquire(adapter);
    if (!adapter)
    {
      std::cerr << "[WebGPU] Failed to get Adapter: "
                << wgpu::StringView(message) << std::endl;
    }
  };

  instance.WaitAny(
    instance.RequestAdapter(&adapterOptions, adapaterCallbackInfo),
    UINT64_MAX
  );
  if (!adapter)
  {
    std::cerr << "[WebGPU] Could not request Adapter" << std::endl;
    return 1;
  }

  wgpu::DeviceDescriptor deviceDescriptor{};
  deviceDescriptor.label = "Device";
  deviceDescriptor.defaultQueue.label = "Default Queue";
  deviceDescriptor.SetDeviceLostCallback(
    wgpu::CallbackMode::WaitAnyOnly,
    [](
      const wgpu::Device& device,
      wgpu::DeviceLostReason reason,
      wgpu::StringView message
    )
    {
      std::cout << "[WebGPU] Device Lost (" << reason << "): " << message
                << std::endl;
    }
  );
  deviceDescriptor.SetUncapturedErrorCallback(
    [](
      const wgpu::Device& device,
      wgpu::ErrorType type,
      wgpu::StringView message
    )
    {
      std::cerr << "[WebGPU] Device Uncaptured (" << type << "): " << message
                << std::endl;
    }
  );

  wgpu::Device device;
  wgpu::RequestDeviceCallbackInfo deviceCallbackInfo{};
  deviceCallbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
  deviceCallbackInfo.userdata = &device;
  deviceCallbackInfo.callback = [](
                                  WGPURequestDeviceStatus status,
                                  WGPUDevice device,
                                  struct WGPUStringView message,
                                  void* userdata
                                )
  {
    auto& data = *static_cast<wgpu::Device*>(userdata);
    data = wgpu::Device::Acquire(device);
    if (!device)
    {
      std::cerr << "[WebGPU] Failed to get Device: "
                << wgpu::StringView(message) << std::endl;
    }
  };
  instance.WaitAny(
    adapter.RequestDevice(&deviceDescriptor, deviceCallbackInfo),
    UINT64_MAX
  );

  auto queue = device.GetQueue();

  auto surface = platform::glfwCreateWGPUSurface(instance, window);
  if (!surface)
  {
    std::cerr << "[WebGPU] Could not create Surface" << std::endl;
    return 1;
  }

  wgpu::SurfaceConfiguration surfaceConfig{};
  surfaceConfig.device = device;
  surfaceConfig.width = 1280;
  surfaceConfig.height = 720;
  surfaceConfig.format = wgpu::TextureFormat::BGRA8Unorm;
  surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
  surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
  surfaceConfig.alphaMode = wgpu::CompositeAlphaMode::Auto;
  surface.Configure(&surfaceConfig);

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);

    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success)
    {
      break;
    }

    wgpu::TextureViewDescriptor textureViewDescriptor{};
    textureViewDescriptor.format = surfaceTexture.texture.GetFormat();
    textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    textureViewDescriptor.baseMipLevel = 0;
    textureViewDescriptor.mipLevelCount = 1;
    textureViewDescriptor.baseArrayLayer = 0;
    textureViewDescriptor.arrayLayerCount = 1;
    textureViewDescriptor.aspect = wgpu::TextureAspect::All;
    auto surfaceView =
      surfaceTexture.texture.CreateView(&textureViewDescriptor);

    wgpu::CommandEncoderDescriptor encoderDescriptor{};
    encoderDescriptor.label = "Command Encoder";
    auto encoder = device.CreateCommandEncoder(&encoderDescriptor);

    wgpu::RenderPassColorAttachment colorAttachment{};
    colorAttachment.view = surfaceView;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.1, 0.1, 0.1, 1.0};
    colorAttachment.depthSlice = wgpu::kDepthSliceUndefined;

    wgpu::RenderPassDescriptor renderPassDescriptor{};
    renderPassDescriptor.label = "Render Pass";
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.colorAttachments = &colorAttachment;

    auto renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
    renderPass.End();

    wgpu::CommandBufferDescriptor commandDescriptor{};
    commandDescriptor.label = "Command Buffer";
    auto command = encoder.Finish(&commandDescriptor);

    queue.Submit(1, &command);

    surface.Present();
  }
}
