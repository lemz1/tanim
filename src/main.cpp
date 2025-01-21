#include <GLFW/glfw3.h>
#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

#include "graphics/renderer.h"
#include "platform/glfw_wgpu_surface.h"

constexpr uint32_t windowWidth = 1280;
constexpr uint32_t windowHeight = 720;

std::optional<nlohmann::json> ReadJson(std::string_view path)
{
  std::ifstream file(path.data());
  if (!file.is_open())
  {
    std::cerr << "Could not open file" << std::endl;
    return std::nullopt;
  }

  nlohmann::json json;
  file >> json;

  return json;
}

int main()
{
  if (auto jsonOpt =
        ReadJson("assets/fonts/ARIALNB.TTF-msdf/ARIALNB.TTF-msdf.json"))
  {
    auto& json = *jsonOpt;
    std::cout << json["pages"] << std::endl;
  }

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

  wgpu::TextureDescriptor textureDescriptor{};
  textureDescriptor.label = "Texture";
  textureDescriptor.dimension = wgpu::TextureDimension::e2D;
  textureDescriptor.size = {32, 32, 1};
  textureDescriptor.mipLevelCount = 1;
  textureDescriptor.sampleCount = 1;
  textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
  textureDescriptor.usage =
    wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
  auto texture = device.CreateTexture(&textureDescriptor);

  std::vector<uint8_t> pixels(
    4 * textureDescriptor.size.width * textureDescriptor.size.height
  );
  for (uint32_t i = 0; i < textureDescriptor.size.width; ++i)
  {
    for (uint32_t j = 0; j < textureDescriptor.size.height; ++j)
    {
      uint8_t* p = &pixels[4 * (j * textureDescriptor.size.width + i)];
      p[0] = (uint8_t)i * 8;  // r
      p[1] = (uint8_t)j * 8;  // g
      p[2] = 128;             // b
      p[3] = 255;             // a
    }
  }

  wgpu::ImageCopyTexture destination{};
  destination.texture = texture;
  destination.mipLevel = 0;
  destination.origin = {0, 0, 0};
  destination.aspect = wgpu::TextureAspect::All;

  wgpu::TextureDataLayout source{};
  source.offset = 0;
  source.bytesPerRow = 4 * textureDescriptor.size.width;
  source.rowsPerImage = textureDescriptor.size.height;

  queue.WriteTexture(
    &destination,
    pixels.data(),
    pixels.size(),
    &source,
    &textureDescriptor.size
  );

  wgpu::SamplerDescriptor samplerDescriptor{};
  samplerDescriptor.label = "Sampler";
  samplerDescriptor.minFilter = wgpu::FilterMode::Linear;
  samplerDescriptor.magFilter = wgpu::FilterMode::Linear;
  samplerDescriptor.addressModeU = wgpu::AddressMode::ClampToEdge;
  samplerDescriptor.addressModeV = wgpu::AddressMode::ClampToEdge;
  samplerDescriptor.addressModeW = wgpu::AddressMode::ClampToEdge;
  auto sampler = device.CreateSampler(&samplerDescriptor);

  std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries(2);

  bindingLayoutEntries[0] = {};
  bindingLayoutEntries[0].binding = 0;
  bindingLayoutEntries[0].visibility = wgpu::ShaderStage::Fragment;
  bindingLayoutEntries[0].texture.sampleType = wgpu::TextureSampleType::Float;
  bindingLayoutEntries[0].texture.viewDimension =
    wgpu::TextureViewDimension::e2D;

  bindingLayoutEntries[1] = {};
  bindingLayoutEntries[1].binding = 1;
  bindingLayoutEntries[1].visibility = wgpu::ShaderStage::Fragment;
  bindingLayoutEntries[1].sampler.type = wgpu::SamplerBindingType::Filtering;

  wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor{};
  bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
  bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();
  wgpu::BindGroupLayout bindGroupLayout =
    device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);

  std::vector<wgpu::BindGroupEntry> bindGroupEntries(2);

  wgpu::TextureViewDescriptor textureViewDescriptor{};
  textureViewDescriptor.label = "Texture View";
  textureViewDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
  textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
  textureViewDescriptor.usage = wgpu::TextureUsage::TextureBinding;
  textureViewDescriptor.aspect = wgpu::TextureAspect::All;
  textureViewDescriptor.baseArrayLayer = 0;
  textureViewDescriptor.arrayLayerCount = 1;
  textureViewDescriptor.baseMipLevel = 0;
  textureViewDescriptor.mipLevelCount = 1;
  auto textureView = texture.CreateView(&textureViewDescriptor);

  bindGroupEntries[0] = {};
  bindGroupEntries[0].textureView = textureView;
  bindGroupEntries[0].binding = 0;

  bindGroupEntries[1] = {};
  bindGroupEntries[1].sampler = sampler;
  bindGroupEntries[1].binding = 1;

  wgpu::BindGroupDescriptor bindGroupDescriptor{};
  bindGroupDescriptor.label = "Bind Group";
  bindGroupDescriptor.entryCount = bindGroupEntries.size();
  bindGroupDescriptor.entries = bindGroupEntries.data();
  bindGroupDescriptor.layout = bindGroupLayout;

  auto bindGroup = device.CreateBindGroup(&bindGroupDescriptor);

  const char* shaderCode = R"(
    @group(0) @binding(0) var tex: texture_2d<f32>;
    @group(0) @binding(1) var texSampler: sampler;

    struct VertexOutput {
      @location(0) texCoord: vec2f,
    };

    @fragment fn fsMain(in: VertexOutput) -> @location(0) vec4f {
      //return vec4f(in.texCoord, 0.0, 1.0);
      return textureSample(tex, texSampler, in.texCoord);
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

  wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor{};
  pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
  pipelineLayoutDescriptor.bindGroupLayouts = &bindGroupLayout;
  pipelineLayoutDescriptor.label = "Pipeline Layout";
  auto pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

  wgpu::RenderPipelineDescriptor pipelineDescriptor{};
  pipelineDescriptor.fragment = &fragmentState;
  pipelineDescriptor.vertex = renderer.VertexState();
  pipelineDescriptor.layout = pipelineLayout;
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
    renderer.Flush(surfaceView, pipeline, bindGroup);

    surface.Present();
  }
}
