#pragma once
#ifdef ENABLE_VOODOO
#include "pce/component.h"
#include "pce/hw/pci_device.h"

class Display;
class MMIO;
class TimingEvent;

class voodoo_device;

namespace HW {
class Voodoo final : public Component, private PCIDevice
{
  DECLARE_OBJECT_TYPE_INFO(Voodoo, Component);
  DECLARE_GENERIC_COMPONENT_FACTORY(Voodoo);
  DECLARE_OBJECT_PROPERTY_MAP(Voodoo);

  enum class Type : u32
  {
    Voodoo1 = 0,
    Voodoo2 = 1
  };

public:
  Voodoo(const String& identifier, const ObjectTypeInfo* type_info = &s_type_info);
  ~Voodoo() override;

  bool Initialize(System* system, Bus* bus) override;
  void Reset() override;

  bool DoState(StateWrapper& sw) override;

protected:
  static constexpr PhysicalMemoryAddress DEFAULT_MEMORY_REGION_ADDRESS = 0xFF000000;
  static constexpr u32 MEMORY_REGION_SIZE = 16 * 1024 * 1024;
  static constexpr u32 MEMORY_REGION_DWORD_MASK = (MEMORY_REGION_SIZE / 4) - 1;

  void ResetConfigSpace(u8 function) override;
  u8 ReadConfigSpace(u8 function, u8 offset) override;
  void WriteConfigSpace(u8 function, u8 offset, u8 value) override;
  void OnMemoryRegionChanged(u8 function, MemoryRegion region, bool active) override;

  u8 HandleBusByteRead(u32 offset);
  void HandleBusByteWrite(u32 offset, u8 val);
  u16 HandleBusWordRead(u32 offset);
  void HandleBusWordWrite(u32 offset, u16 val);
  u32 HandleBusDWordRead(u32 offset);
  void HandleBusDWordWrite(u32 offset, u32 val);

  Type m_type = Type::Voodoo2;
  u32 m_clock_frequency = 0;
  u32 m_fb_mem_size = 4;
  u32 m_tmu_mem_size = 4;
  bool m_primary_display = true;

  std::unique_ptr<Display> m_display;

  std::unique_ptr<voodoo_device> m_device;

  MMIO* m_mmio_mapping = nullptr;
};

} // namespace HW
#endif