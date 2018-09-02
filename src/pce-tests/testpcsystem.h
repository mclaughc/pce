#pragma once

#include <memory>
#include <vector>

#include "pce/cpu_x86/cpu_x86.h"
#include "pce/hw/i8259_pic.h"
#include "pce/system.h"

class ByteStream;

class TestPCSystem : public System
{
  DECLARE_OBJECT_TYPE_INFO(TestPCSystem, System);
  DECLARE_OBJECT_NO_FACTORY(TestPCSystem);
  DECLARE_OBJECT_NO_PROPERTIES(TestPCSystem);

public:
  static const PhysicalMemoryAddress BIOS_ROM_ADDRESS = 0xF0000;
  static const uint32 BIOS_ROM_SIZE = 65536;

  TestPCSystem(CPU_X86::Model cpu_model = CPU_X86::MODEL_486, float cpu_frequency = 1000000.0f,
               CPUBackendType cpu_backend = CPUBackendType::Interpreter, uint32 ram_size = 1024 * 1024);

  ~TestPCSystem();

  CPU_X86::CPU* GetX86CPU() const { return static_cast<CPU_X86::CPU*>(m_cpu); }

  bool AddMMIOROMFromFile(const char* filename, PhysicalMemoryAddress address);

  bool Ready();

private:
  bool Initialize() override;

  void AddComponents();

  HW::i8259_PIC* m_interrupt_controller = nullptr;

  std::vector<std::unique_ptr<byte[]>> m_rom_data;
};
