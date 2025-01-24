#include "renderer.h"

#include <fstream>

namespace graphics
{
constexpr size_t vertexBufferSize = 1000 * sizeof(Vertex);
constexpr size_t indexBufferSize = vertexBufferSize * 2;

Renderer::Renderer(
  const wgpu::Device& device,
  const wgpu::Queue& queue,
  wgpu::TextureFormat format
)
  : _device(device), _queue(queue)
{
  CreateSamplers();
  CreateTextPipeline(format);
  CreateDrawBuffers();
}

void Renderer::DrawQuad(float x, float y)
{
  std::vector<Vertex> vertices = {
    {
      {-0.5f + x, -0.5f + y, +0.0f},
      {+0.0f, +1.0f},
    },
    {
      {+0.5f + x, -0.5f + y, +0.0f},
      {+1.0f, +1.0f},
    },
    {
      {-0.5f + x, +0.5f + y, +0.0f},
      {+0.0f, +0.0f},
    },
    {
      {+0.5f + x, +0.5f + y, +0.0f},
      {+1.0f, +0.0f},
    },
  };
  _queue.WriteBuffer(
    _vertexBuffer,
    _vertexBufferOffset,
    vertices.data(),
    vertices.size() * sizeof(Vertex)
  );

  std::vector<uint32_t> indices = {
    0 + _indexValueOffset,
    1 + _indexValueOffset,
    2 + _indexValueOffset,
    1 + _indexValueOffset,
    3 + _indexValueOffset,
    2 + _indexValueOffset
  };

  _queue.WriteBuffer(
    _indexBuffer,
    _indexBufferOffset,
    indices.data(),
    indices.size() * sizeof(uint32_t)
  );

  _vertexBufferOffset += vertices.size() * sizeof(Vertex);
  _indexBufferOffset += indices.size() * sizeof(uint32_t);
  _indexValueOffset += 4;
}

void Renderer::DrawText(
  const graphics::Text& text,
  const wgpu::TextureView& view,
  const wgpu::BindGroup& vertexGroup,
  const wgpu::BindGroup& fragmentGroup
)
{
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
  renderPass.SetPipeline(_textPipeline);
  renderPass.SetBindGroup(0, vertexGroup);
  renderPass.SetBindGroup(1, fragmentGroup);
  renderPass.Draw(4, (uint32_t)text.CharacterCount(), 0, 0);
  renderPass.End();

  wgpu::CommandBufferDescriptor commandDescriptor{};
  commandDescriptor.label = "Renderer Command Buffer";
  auto command = encoder.Finish(&commandDescriptor);

  _queue.Submit(1, &command);
}

void Renderer::Flush(
  const wgpu::TextureView& view,
  const wgpu::RenderPipeline& pipeline,
  const wgpu::BindGroup& bindGroup
)
{
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
  renderPass.SetPipeline(pipeline);
  renderPass.SetVertexBuffer(0, _vertexBuffer, 0, _vertexBufferOffset);
  renderPass.SetIndexBuffer(
    _indexBuffer,
    wgpu::IndexFormat::Uint32,
    0,
    _indexBufferOffset
  );
  renderPass.SetBindGroup(0, bindGroup);
  renderPass
    .DrawIndexed((uint32_t)(_indexBufferOffset / sizeof(uint32_t)), 1, 0, 0, 0);
  renderPass.End();

  wgpu::CommandBufferDescriptor commandDescriptor{};
  commandDescriptor.label = "Renderer Command Buffer";
  auto command = encoder.Finish(&commandDescriptor);

  _queue.Submit(1, &command);

  _vertexBufferOffset = 0;
  _indexBufferOffset = 0;
  _indexValueOffset = 0;
}

const graphics::Font& Renderer::Font(const std::filesystem::path& path)
{
  if (_fonts.find(path) != _fonts.end())
  {
    return _fonts.at(path);
  }

  _fonts.insert({path, graphics::Font(_device, _queue, path)});
  return _fonts.at(path);
}

void Renderer::CreateSamplers()
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

void Renderer::CreateTextPipeline(wgpu::TextureFormat format)
{
  wgpu::BindGroupLayoutEntry vertexLayoutEntry{};
  vertexLayoutEntry.binding = 0;
  vertexLayoutEntry.visibility = wgpu::ShaderStage::Vertex;
  vertexLayoutEntry.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

  wgpu::BindGroupLayoutDescriptor vertexLayoutDescriptor{};
  vertexLayoutDescriptor.label = "Renderer Text Vertex Bind Group Layout";
  vertexLayoutDescriptor.entryCount = 1;
  vertexLayoutDescriptor.entries = &vertexLayoutEntry;
  _textBindGroupLayouts[0] =
    _device.CreateBindGroupLayout(&vertexLayoutDescriptor);

  std::array<wgpu::BindGroupLayoutEntry, 2> fragmentLayoutEntries{};
  fragmentLayoutEntries[0].binding = 0;
  fragmentLayoutEntries[0].visibility = wgpu::ShaderStage::Fragment;
  fragmentLayoutEntries[0].texture.sampleType = wgpu::TextureSampleType::Float;
  fragmentLayoutEntries[0].texture.viewDimension =
    wgpu::TextureViewDimension::e2D;

  fragmentLayoutEntries[1].binding = 1;
  fragmentLayoutEntries[1].visibility = wgpu::ShaderStage::Fragment;
  fragmentLayoutEntries[1].sampler.type = wgpu::SamplerBindingType::Filtering;

  wgpu::BindGroupLayoutDescriptor fragmentLayoutDescriptor{};
  fragmentLayoutDescriptor.label = "Renderer Text Fragment Bind Group Layout";
  fragmentLayoutDescriptor.entryCount = (uint32_t)fragmentLayoutEntries.size();
  fragmentLayoutDescriptor.entries = fragmentLayoutEntries.data();
  _textBindGroupLayouts[1] =
    _device.CreateBindGroupLayout(&fragmentLayoutDescriptor);

  const char* shaderCode = R"(
    struct VertexInput {
      @builtin(vertex_index) vertexIndex: u32,
      @builtin(instance_index) instanceIndex: u32,
    };

    struct VertexOutput {
      @builtin(position) position: vec4f,
      @location(0) uv: vec2f,
    };

    struct TextCharacter {
      bounds: vec4f,
      size: vec2f,
      position: vec2f,
    };

    @group(0) @binding(0) var<storage, read> characters: array<TextCharacter>;

    @vertex fn vsMain(in: VertexInput) -> VertexOutput {
      var positions = array<vec2f, 4>(
        vec2f(0.0, 0.0),
        vec2f(1.0, 0.0),
        vec2f(0.0, -1.0),
        vec2f(1.0, -1.0)
      );

      var position = positions[in.vertexIndex];

      var character = characters[in.instanceIndex];

      var uvs = array<vec2f, 4>(
        character.bounds.xz,
        character.bounds.yz,
        character.bounds.xw,
        character.bounds.yw
      );

      var uv = uvs[in.vertexIndex];

      var out: VertexOutput;
      out.position = vec4f((position * character.size + character.position) * 0.003, 0.0, 1.0);
      out.uv = uv;
      return out;
    }

    @group(1) @binding(0) var fontTexture: texture_2d<f32>;
    @group(1) @binding(1) var fontSampler: sampler;

    fn sampleMsdf(uv: vec2f) -> f32 {
      let c = textureSample(fontTexture, fontSampler, uv);
      return max(min(c.r, c.g), min(max(c.r, c.g), c.b));
    }

    @fragment fn fsMain(in: VertexOutput) -> @location(0) vec4f {
      let pxRange = 4.0;
      let sz = vec2f(textureDimensions(fontTexture, 0));
      let dx = sz.x*length(vec2f(dpdxFine(in.uv.x), dpdyFine(in.uv.x)));
      let dy = sz.y*length(vec2f(dpdxFine(in.uv.y), dpdyFine(in.uv.y)));
      let toPixels = pxRange * inverseSqrt(dx * dx + dy * dy);
      let sigDist = sampleMsdf(in.uv) - 0.5;
      let pxDist = sigDist * toPixels;

      let edgeWidth = 0.5;

      let alpha = smoothstep(-edgeWidth, edgeWidth, pxDist);

      return vec4f(1.0, 0.0, 0.0, alpha);
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
  pipelineLayoutDescriptor.bindGroupLayoutCount =
    (uint32_t)_textBindGroupLayouts.size();
  pipelineLayoutDescriptor.bindGroupLayouts = _textBindGroupLayouts.data();
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

void Renderer::CreateDrawBuffers()
{
  wgpu::BufferDescriptor vertexBufferDescriptor{};
  vertexBufferDescriptor.label = "Renderer Vertex Buffer";
  vertexBufferDescriptor.size = vertexBufferSize;
  vertexBufferDescriptor.usage =
    wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
  _vertexBuffer = _device.CreateBuffer(&vertexBufferDescriptor);

  wgpu::BufferDescriptor indexBufferDescriptor{};
  indexBufferDescriptor.label = "Renderer Index Buffer";
  indexBufferDescriptor.size = indexBufferSize;
  indexBufferDescriptor.usage =
    wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
  _indexBuffer = _device.CreateBuffer(&indexBufferDescriptor);
}
}  // namespace graphics
