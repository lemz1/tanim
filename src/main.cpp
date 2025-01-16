#include <GLFW/glfw3.h>
#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <iostream>
#include <vector>

#include "graphics/renderer.h"
#include "platform/glfw_wgpu_surface.h"

constexpr uint32_t windowWidth = 1280;
constexpr uint32_t windowHeight = 720;

int main()
{
  if (!glfwInit())
  {
    std::cerr << "[GLFW] Could not initialize GLFW" << std::endl;
    return 1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window =
    glfwCreateWindow(windowWidth, windowHeight, "Animation", nullptr, nullptr);
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

  wgpu::SurfaceCapabilities surfaceCapabilities;
  surface.GetCapabilities(adapter, &surfaceCapabilities);

  wgpu::TextureFormat surfaceFormat = surfaceCapabilities.formats[0];

  wgpu::SurfaceConfiguration surfaceConfig{};
  surfaceConfig.device = device;
  surfaceConfig.width = windowWidth;
  surfaceConfig.height = windowHeight;
  surfaceConfig.format = surfaceFormat;
  surfaceConfig.usage = wgpu::TextureUsage::RenderAttachment;
  surfaceConfig.presentMode = wgpu::PresentMode::Fifo;
  surfaceConfig.alphaMode = wgpu::CompositeAlphaMode::Auto;
  surface.Configure(&surfaceConfig);

  auto renderer = graphics::Renderer(device);

  const char* shaderCode = R"(
    struct VertexInput {
      @location(0) position: vec3f,
      @location(1) texCoord: vec2f,
    };

    struct VertexOutput {
      @builtin(position) position: vec4f,
      @location(0) texCoord: vec2f,
    };

    @vertex fn vsMain(in: VertexInput) -> VertexOutput {
      var out: VertexOutput;
      out.position = vec4f(in.position, 1.0);
      out.texCoord = in.texCoord;
      return out;
    }
    @fragment fn fsMain(in: VertexOutput) -> @location(0) vec4f {
      return vec4f(in.texCoord, 0.0, 1.0);
    }
)";

  wgpu::ShaderModuleWGSLDescriptor fromWGSL{};
  fromWGSL.code = shaderCode;
  fromWGSL.sType = wgpu::SType::ShaderSourceWGSL;

  wgpu::ShaderModuleDescriptor shaderModuleDescriptor{};
  shaderModuleDescriptor.nextInChain = &fromWGSL;

  wgpu::ShaderModule shaderModule =
    device.CreateShaderModule(&shaderModuleDescriptor);

  wgpu::ColorTargetState colorTargetState{};
  colorTargetState.format = surfaceFormat;

  wgpu::FragmentState fragmentState{};
  fragmentState.module = shaderModule;
  fragmentState.targetCount = 1;
  fragmentState.targets = &colorTargetState;

  wgpu::RenderPipelineDescriptor pipelineDescriptor{};
  pipelineDescriptor.fragment = &fragmentState;
  pipelineDescriptor.vertex.module = shaderModule;
  pipelineDescriptor.vertex.bufferCount = 1;
  pipelineDescriptor.vertex.buffers = &renderer.VertexBufferLayout();
  wgpu::RenderPipeline pipeline =
    device.CreateRenderPipeline(&pipelineDescriptor);

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    wgpu::SurfaceTexture surfaceTexture;
    surface.GetCurrentTexture(&surfaceTexture);

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

    renderer.DrawQuad(-0.5f, +0.5f);
    renderer.DrawQuad(+0.5f, -0.5f);
    renderer.Flush(surfaceView, pipeline);

    surface.Present();
  }
}
