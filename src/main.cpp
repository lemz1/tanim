#include <GLFW/glfw3.h>
#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <filesystem>
#include <fstream>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

#include "graphics/renderer.h"
#include "graphics/text.h"
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

  auto renderer = graphics::Renderer(device, queue, surfaceFormat);

  auto& font = renderer.font("assets/fonts/ARIALBD.TTF-msdf");

  auto text = graphics::Text(device, queue, "Hello, World!", font);

  auto projectionMat = glm::perspectiveFov(
    glm::radians(45.0f),
    (float)windowWidth,
    (float)windowHeight,
    0.1f,
    1000.0f
  );

  auto viewMat = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -5.0f)) *
                 glm::mat4_cast(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));

  auto viewProjection = projectionMat * glm::inverse(viewMat);

  wgpu::BufferDescriptor uniformDescriptor{};
  uniformDescriptor.label = "Text Uniform Buffer";
  uniformDescriptor.size = sizeof(glm::mat4);
  uniformDescriptor.usage =
    wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
  auto uniformBuffer = device.CreateBuffer(&uniformDescriptor);

  queue.WriteBuffer(uniformBuffer, 0, &viewProjection, sizeof(glm::mat4));

  std::vector<wgpu::BindGroupEntry> vertexGroupEntries(2);
  vertexGroupEntries[0].buffer = text.buffer();
  vertexGroupEntries[0].binding = 0;

  vertexGroupEntries[1].buffer = uniformBuffer;
  vertexGroupEntries[1].binding = 1;

  wgpu::BindGroupDescriptor vertexGroupDescriptor{};
  vertexGroupDescriptor.label = "Vertex Bind Group";
  vertexGroupDescriptor.entryCount = vertexGroupEntries.size();
  vertexGroupDescriptor.entries = vertexGroupEntries.data();
  vertexGroupDescriptor.layout = renderer.textVertexBindGroupLayout();
  auto vertexGroup = device.CreateBindGroup(&vertexGroupDescriptor);

  std::vector<wgpu::BindGroupEntry> fragmentGroupEntries(2);
  fragmentGroupEntries[0].textureView = font.atlasView();
  fragmentGroupEntries[0].binding = 0;
  fragmentGroupEntries[1].sampler = renderer.linearSampler();
  fragmentGroupEntries[1].binding = 1;

  wgpu::BindGroupDescriptor fragmentGroupDescriptor{};
  fragmentGroupDescriptor.label = "Fragment Bind Group";
  fragmentGroupDescriptor.entryCount = fragmentGroupEntries.size();
  fragmentGroupDescriptor.entries = fragmentGroupEntries.data();
  fragmentGroupDescriptor.layout = renderer.textFragmentBindGroupLayout();
  auto fragmentGroup = device.CreateBindGroup(&fragmentGroupDescriptor);

  float time = 0.0;

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    float newTime = (float)glfwGetTime();
    float deltaTime = newTime - time;
    time = newTime;

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

    renderer.drawText(text, surfaceView, vertexGroup, fragmentGroup);

    surface.Present();
  }
}
