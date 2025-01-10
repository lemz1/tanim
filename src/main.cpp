#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <iostream>

int main()
{
#ifdef __EMSCRIPTEN__
  auto instance = wgpu::CreateInstance(nullptr);
#else
  wgpu::InstanceDescriptor instanceDescriptor{};
  auto instance = wgpu::CreateInstance(&instanceDescriptor);
#endif

  if (!instance)
  {
    std::cerr << "[WebGPU] Could not create Instance" << std::endl;
    return 1;
  }
}
