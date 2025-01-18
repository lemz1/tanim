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

  wgpu::InstanceDescriptor instanceDescriptor{};
  instanceDescriptor.features.timedWaitAnyEnable = true;
  auto instance = wgpu::CreateInstance(&instanceDescriptor);
  if (!instance)
  {
    std::cerr << "[WebGPU] Could not create Instance" << std::endl;
    return 1;
  }

  wgpu::RequestAdapterOptions adapterOptions{};

  wgpu::Adapter adapter;
  instance.WaitAny(
    instance.RequestAdapter(
      &adapterOptions,
      wgpu::CallbackMode::WaitAnyOnly,
      [](
        wgpu::RequestAdapterStatus status,
        wgpu::Adapter adapter,
        wgpu::StringView message,
        wgpu::Adapter* outAdapter
      )
      {
        *outAdapter = adapter;
        if (!adapter)
        {
          std::cerr << "[WebGPU] Failed to get Adapter: " << message
                    << std::endl;
        }
      },
      &adapter
    ),
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
  instance.WaitAny(
    adapter.RequestDevice(
      &deviceDescriptor,
      wgpu::CallbackMode::WaitAnyOnly,
      [](
        wgpu::RequestDeviceStatus status,
        wgpu::Device device,
        wgpu::StringView message,
        wgpu::Device* outDevice
      )
      {
        *outDevice = device;
        if (!device)
        {
          std::cerr << "[WebGPU] Failed to get Device: " << message
                    << std::endl;
        }
      },
      &device
    ),
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
    struct VertexOutput {
      @location(0) texCoord: vec2f,
    };

    @fragment fn fsMain(in: VertexOutput) -> @location(0) vec4f {
      return vec4f(in.texCoord, 0.0, 1.0);
    }
)";

  wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
  wgslDescriptor.code = shaderCode;
  wgslDescriptor.sType = wgpu::SType::ShaderSourceWGSL;

  wgpu::ShaderModuleDescriptor shaderModuleDescriptor{};
  shaderModuleDescriptor.label = "Fragment Module";
  shaderModuleDescriptor.nextInChain = &wgslDescriptor;

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
  pipelineDescriptor.vertex = renderer.VertexState();
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
