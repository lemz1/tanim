#include "renderer.h"

#include <chrono>
#include <fstream>
#include <iostream>

namespace graphics
{
constexpr size_t textCharacterCount = 2048;

Renderer::Renderer(
  const wgpu::Device& device,
  const wgpu::Queue& queue,
  wgpu::TextureFormat format
)
  : _device(device), _queue(queue)
{
  createSamplers();
  createTextBuffers();
  createTextPipeline(format);
}

void Renderer::drawText(Text& text, const Camera& camera)
{
  _queue.WriteBuffer(
    _textUniformBuffer,
    0,
    &camera.viewProjection(),
    sizeof(glm::mat4)
  );

  for (auto& character : text._characters)
  {
    _textCharacterData.emplace_back(character.data());
  }
}

void Renderer::flush(const wgpu::TextureView& view)
{
  _queue.WriteBuffer(
    _textCharacterBuffer,
    0,
    _textCharacterData.data(),
    _textCharacterData.size() * sizeof(TextCharacterGPU)
  );

  wgpu::CommandEncoderDescriptor encoderDescriptor{};
  encoderDescriptor.label = "Renderer Command Encoder";
  auto encoder = _device.CreateCommandEncoder(&encoderDescriptor);

  wgpu::RenderPassColorAttachment colorAttachment{};
  colorAttachment.view = view;
  colorAttachment.loadOp = wgpu::LoadOp::Clear;
  colorAttachment.storeOp = wgpu::StoreOp::Store;
  colorAttachment.clearValue = {0.1, 0.1, 0.1, 1.0};
  colorAttachment.depthSlice = wgpu::kDepthSliceUndefined;

  wgpu::RenderPassDescriptor renderPassDescriptor{};
  renderPassDescriptor.label = "Renderer Render Pass";
  renderPassDescriptor.colorAttachmentCount = 1;
  renderPassDescriptor.colorAttachments = &colorAttachment;

  auto renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
  flushText(renderPass);
  renderPass.End();

  wgpu::CommandBufferDescriptor commandDescriptor{};
  commandDescriptor.label = "Renderer Command Buffer";
  auto command = encoder.Finish(&commandDescriptor);

  _queue.Submit(1, &command);
}

const graphics::Font& Renderer::font(const std::filesystem::path& path)
{
  if (_fonts.find(path) != _fonts.end())
  {
    return _fonts.at(path);
  }

  _fonts.insert({path, graphics::Font(_device, _queue, path)});
  return _fonts.at(path);
}

void Renderer::createSamplers()
{
  wgpu::SamplerDescriptor linearDescriptor{};
  linearDescriptor.label = "Renderer Linear Sampler";
  linearDescriptor.minFilter = wgpu::FilterMode::Linear;
  linearDescriptor.magFilter = wgpu::FilterMode::Linear;
  linearDescriptor.addressModeU = wgpu::AddressMode::ClampToEdge;
  linearDescriptor.addressModeV = wgpu::AddressMode::ClampToEdge;
  linearDescriptor.addressModeW = wgpu::AddressMode::ClampToEdge;
  _linearSampler = _device.CreateSampler(&linearDescriptor);

  wgpu::SamplerDescriptor nearestDescriptor{};
  nearestDescriptor.label = "Renderer Nearest Sampler";
  nearestDescriptor.minFilter = wgpu::FilterMode::Nearest;
  nearestDescriptor.magFilter = wgpu::FilterMode::Nearest;
  nearestDescriptor.addressModeU = wgpu::AddressMode::ClampToEdge;
  nearestDescriptor.addressModeV = wgpu::AddressMode::ClampToEdge;
  nearestDescriptor.addressModeW = wgpu::AddressMode::ClampToEdge;
  _nearestSampler = _device.CreateSampler(&nearestDescriptor);
}

void Renderer::createTextBuffers()
{
  _textCharacterData.reserve(textCharacterCount);

  wgpu::BufferDescriptor textCharacterBufferDescriptor{};
  textCharacterBufferDescriptor.label = "Renderer Text Character Buffer";
  textCharacterBufferDescriptor.size =
    textCharacterCount * sizeof(TextCharacterGPU);
  textCharacterBufferDescriptor.usage =
    wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
  _textCharacterBuffer = _device.CreateBuffer(&textCharacterBufferDescriptor);

  wgpu::BufferDescriptor textUniformBufferDescriptor{};
  textUniformBufferDescriptor.label = "Renderer Text Uniform Buffer";
  textUniformBufferDescriptor.size = sizeof(glm::mat4);
  textUniformBufferDescriptor.usage =
    wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
  _textUniformBuffer = _device.CreateBuffer(&textUniformBufferDescriptor);
}

void Renderer::createTextPipeline(wgpu::TextureFormat format)
{
  std::array<wgpu::BindGroupLayoutEntry, 4> bindGroupLayoutEntries{};
  bindGroupLayoutEntries[0].binding = 0;
  bindGroupLayoutEntries[0].visibility = wgpu::ShaderStage::Vertex;
  bindGroupLayoutEntries[0].buffer.type =
    wgpu::BufferBindingType::ReadOnlyStorage;

  bindGroupLayoutEntries[1].binding = 1;
  bindGroupLayoutEntries[1].visibility = wgpu::ShaderStage::Vertex;
  bindGroupLayoutEntries[1].buffer.type = wgpu::BufferBindingType::Uniform;

  bindGroupLayoutEntries[2].binding = 2;
  bindGroupLayoutEntries[2].visibility = wgpu::ShaderStage::Fragment;
  bindGroupLayoutEntries[2].texture.sampleType = wgpu::TextureSampleType::Float;
  bindGroupLayoutEntries[2].texture.viewDimension =
    wgpu::TextureViewDimension::e2D;

  bindGroupLayoutEntries[3].binding = 3;
  bindGroupLayoutEntries[3].visibility = wgpu::ShaderStage::Fragment;
  bindGroupLayoutEntries[3].sampler.type = wgpu::SamplerBindingType::Filtering;

  wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor{};
  bindGroupLayoutDescriptor.label = "Renderer Text Bind Group Layout";
  bindGroupLayoutDescriptor.entryCount =
    (uint32_t)bindGroupLayoutEntries.size();
  bindGroupLayoutDescriptor.entries = bindGroupLayoutEntries.data();
  auto bindGroupLayout =
    _device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);

  std::array<wgpu::BindGroupEntry, 4> bindGroupEntries{};
  bindGroupEntries[0].buffer = _textCharacterBuffer;
  bindGroupEntries[0].binding = 0;

  bindGroupEntries[1].buffer = _textUniformBuffer;
  bindGroupEntries[1].binding = 1;

  // temporary
  auto& font = this->font("assets/fonts/ARIALBD.TTF-msdf");

  bindGroupEntries[2].textureView = font.atlasView();
  bindGroupEntries[2].binding = 2;

  bindGroupEntries[3].sampler = _linearSampler;
  bindGroupEntries[3].binding = 3;

  wgpu::BindGroupDescriptor bindGroupDescriptor{};
  bindGroupDescriptor.label = "Renderer Text Bind Group";
  bindGroupDescriptor.entryCount = bindGroupEntries.size();
  bindGroupDescriptor.entries = bindGroupEntries.data();
  bindGroupDescriptor.layout = bindGroupLayout;
  _textBindGroup = _device.CreateBindGroup(&bindGroupDescriptor);

  const char* shaderCode = R"(
    const positions = array<vec2f, 4>(
      vec2f(0.0, 0.0),
      vec2f(1.0, 0.0),
      vec2f(0.0, -1.0),
      vec2f(1.0, -1.0)
    );

    struct VertexInput {
      @builtin(vertex_index) vertexIndex: u32,
      @builtin(instance_index) instanceIndex: u32,
    };

    struct VertexOutput {
      @builtin(position) position: vec4f,
      @location(0) uv: vec2f,
      @location(1) color: vec3f,
    };

    struct TextCharacter {
      transform: mat4x4<f32>,
      bounds: vec4f,
      color: vec3f,
      size: vec2f,
      position: vec2f,
    };

    @group(0) @binding(0) var<storage, read> characters: array<TextCharacter>;
    @group(0) @binding(1) var<uniform> viewProjection: mat4x4<f32>;
    @group(0) @binding(2) var fontTexture: texture_2d<f32>;
    @group(0) @binding(3) var fontSampler: sampler;

    @vertex 
    fn vsMain(in: VertexInput) -> VertexOutput {
      let character = characters[in.instanceIndex];

      var vertexPosition = positions[in.vertexIndex];
      vertexPosition *= character.size;
      vertexPosition += character.position;

      let uvs = array<vec2f, 4>(
        character.bounds.xz,
        character.bounds.yz,
        character.bounds.xw,
        character.bounds.yw
      );

      let uv = uvs[in.vertexIndex];

      var out: VertexOutput;
      out.position = viewProjection * character.transform * vec4f(vertexPosition, 0.0, 1.0);
      out.uv = uv;
      out.color = character.color;
      return out;
    }

    fn sampleMsdf(uv: vec2f) -> f32 {
      let c = textureSample(fontTexture, fontSampler, uv);
      return max(min(c.r, c.g), min(max(c.r, c.g), c.b));
    }

    @fragment 
    fn fsMain(in: VertexOutput) -> @location(0) vec4f {
      let msdf = sampleMsdf(in.uv);

      let pxRange = 4.0;
      let sz = vec2f(textureDimensions(fontTexture, 0));
      let dx = sz.x*length(vec2f(dpdxFine(in.uv.x), dpdyFine(in.uv.x)));
      let dy = sz.y*length(vec2f(dpdxFine(in.uv.y), dpdyFine(in.uv.y)));
      let toPixels = pxRange * inverseSqrt(dx * dx + dy * dy);
      let sigDist = msdf - 0.5;
      let pxDist = sigDist * toPixels;

      let edgeWidth = 0.5;

      let alpha = smoothstep(-edgeWidth, edgeWidth, pxDist);

      return vec4f(in.color, alpha);
    }
)";

  wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
  wgslDescriptor.code = shaderCode;
  wgslDescriptor.sType = wgpu::SType::ShaderSourceWGSL;

  wgpu::ShaderModuleDescriptor shaderModuleDescriptor{};
  shaderModuleDescriptor.label = "Renderer Shader Module";
  shaderModuleDescriptor.nextInChain = &wgslDescriptor;

  wgpu::ShaderModule shaderModule =
    _device.CreateShaderModule(&shaderModuleDescriptor);

  wgpu::BlendState blendState{};
  blendState.alpha.srcFactor = wgpu::BlendFactor::One;
  blendState.alpha.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
  blendState.alpha.operation = wgpu::BlendOperation::Add;
  blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
  blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
  blendState.color.operation = wgpu::BlendOperation::Add;

  wgpu::ColorTargetState colorTargetState{};
  colorTargetState.format = format;
  colorTargetState.blend = &blendState;
  colorTargetState.writeMask = wgpu::ColorWriteMask::All;

  wgpu::FragmentState fragmentState{};
  fragmentState.module = shaderModule;
  fragmentState.targetCount = 1;
  fragmentState.targets = &colorTargetState;

  wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor{};
  pipelineLayoutDescriptor.label = "Renderer Text Pipeline Layout";
  pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
  pipelineLayoutDescriptor.bindGroupLayouts = &bindGroupLayout;
  auto pipelineLayout = _device.CreatePipelineLayout(&pipelineLayoutDescriptor);

  wgpu::RenderPipelineDescriptor pipelineDescriptor{};
  pipelineDescriptor.label = "Renderer Text Pipeline";
  pipelineDescriptor.fragment = &fragmentState;
  pipelineDescriptor.vertex.module = shaderModule;
  pipelineDescriptor.primitive.topology =
    wgpu::PrimitiveTopology::TriangleStrip;
  pipelineDescriptor.layout = pipelineLayout;
  _textPipeline = _device.CreateRenderPipeline(&pipelineDescriptor);
}

void Renderer::flushText(const wgpu::RenderPassEncoder& renderPass)
{
  renderPass.SetPipeline(_textPipeline);
  renderPass.SetBindGroup(0, _textBindGroup);
  renderPass.Draw(4, (uint32_t)_textCharacterData.size(), 0, 0);

  _textCharacterData.clear();
}
}  // namespace graphics
