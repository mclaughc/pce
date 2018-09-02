#pragma once
#include "pce/component.h"
#include <vector>

class PCIBus;

class PCIDevice : public Component
{
  DECLARE_OBJECT_TYPE_INFO(PCIDevice, Component);
  DECLARE_OBJECT_NO_FACTORY(PCIDevice);
  DECLARE_OBJECT_PROPERTY_MAP(PCIDevice);

public:
  static constexpr uint32 NUM_CONFIG_REGISTERS = 64;

  PCIDevice(const String& identifier, uint16 vendor_id, uint16 device_id, uint32 num_functions = 1,
            const ObjectTypeInfo* type_info = &s_type_info);
  ~PCIDevice();

  u32 GetPCIBusNumber() const { return m_pci_bus_number; }
  u32 GetPCIDeviceNumber() const { return m_pci_device_number; }
  void SetLocation(u32 pci_bus_number, u32 pci_device_number);

  // Returns the PCI bus which this device is attached to.
  PCIBus* GetPCIBus() const;

  virtual bool Initialize(System* system, Bus* bus) override;
  virtual void Reset() override;

  virtual bool LoadState(BinaryReader& reader) override;
  virtual bool SaveState(BinaryWriter& writer) override;

  uint8 ReadConfigRegister(uint32 function, uint8 reg, uint8 index);
  void WriteConfigRegister(uint32 function, uint8 reg, uint8 index, uint8 value);

protected:
  virtual uint8 HandleReadConfigRegister(uint32 function, uint8 offset);
  virtual void HandleWriteConfigRegister(uint32 function, uint8 offset, uint8 value);

  uint32 m_num_functions = 0;
  uint32 m_pci_bus_number = 0xFFFFFFFFu;
  uint32 m_pci_device_number = 0xFFFFFFFFu;

  union ConfigSpace
  {
    uint32 dwords[NUM_CONFIG_REGISTERS];
    uint16 words[NUM_CONFIG_REGISTERS * 2];
    uint8 bytes[NUM_CONFIG_REGISTERS * 4];
  };

  std::vector<ConfigSpace> m_config_space;

private:
  static const uint32 SERIALIZATION_ID = MakeSerializationID('P', 'C', 'I', '-');
};
