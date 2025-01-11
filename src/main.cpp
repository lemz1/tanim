#include <dawn/webgpu_cpp_print.h>
#include <webgpu/webgpu_cpp.h>

#include <iostream>

int main()
{
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
}
