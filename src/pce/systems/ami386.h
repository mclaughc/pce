#pragma once
#include "pce/cpu_x86/cpu_x86.h"
#include "pce/hw/ds12887.h"
#include "pce/hw/fdc.h"
#include "pce/hw/hdc.h"
#include "pce/hw/i8042_ps2.h"
#include "pce/hw/i8237_dma.h"
#include "pce/hw/i8253_pit.h"
#include "pce/hw/i8259_pic.h"
#include "pce/hw/pcspeaker.h"
#include "pce/systems/isapc.h"

class ByteStream;

namespace Systems {

class AMI386 : public ISAPC
{
  DECLARE_OBJECT_TYPE_INFO(AMI386, ISAPC);
  DECLARE_OBJECT_GENERIC_FACTORY(AMI386);
  DECLARE_OBJECT_PROPERTY_MAP(AMI386);

public:
  static constexpr u32 PHYSICAL_MEMORY_BITS = 32;
  static constexpr PhysicalMemoryAddress BIOS_ROM_ADDRESS = 0xF0000;
  static constexpr u32 BIOS_ROM_SIZE = 65536;

  AMI386(CPU_X86::Model model = CPU_X86::MODEL_386, float cpu_frequency = 4000000.0f,
         u32 memory_size = 16 * 1024 * 1024, const ObjectTypeInfo* type_info = &s_type_info);
  ~AMI386();

  void SetBIOSFilePath(const std::string& path) { m_bios_file_path = path; }

  bool Initialize() override;
  void Reset() override;

  auto GetFDDController() const { return m_fdd_controller; }
  auto GetHDDController() const { return m_hdd_controller; }
  auto GetKeyboardController() const { return m_keyboard_controller; }
  auto GetDMAController() const { return m_dma_controller; }
  auto GetTimer() const { return m_timer; }
  auto GetCMOS() const { return m_cmos; }

private:
  virtual bool LoadSystemState(BinaryReader& reader) override;
  virtual bool SaveSystemState(BinaryWriter& writer) override;

  void ConnectSystemIOPorts();
  void AddComponents();
  void SetCMOSVariables();

  u8 IOReadSystemControlPortA();
  void IOWriteSystemControlPortA(u8 value);
  u8 IOReadSystemControlPortB();
  void IOWriteSystemControlPortB(u8 value);
  void UpdateKeyboardControllerOutputPort();

  std::string m_bios_file_path;

  HW::i8042_PS2* m_keyboard_controller = nullptr;
  HW::i8237_DMA* m_dma_controller = nullptr;
  HW::i8253_PIT* m_timer = nullptr;
  HW::i8259_PIC* m_interrupt_controller = nullptr;

  HW::DS12887* m_cmos = nullptr;

  HW::PCSpeaker* m_speaker = nullptr;

  HW::FDC* m_fdd_controller = nullptr;
  HW::HDC* m_hdd_controller = nullptr;

  bool m_cmos_lock = false;
  bool m_refresh_bit = false;
};

} // namespace Systems