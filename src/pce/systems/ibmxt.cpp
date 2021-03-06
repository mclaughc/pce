#include "pce/systems/ibmxt.h"
#include "YBaseLib/BinaryReader.h"
#include "YBaseLib/BinaryWriter.h"
#include "YBaseLib/ByteStream.h"
#include "YBaseLib/Log.h"
#include "pce/bus.h"
#include "pce/cpu_8086/cpu.h"
Log_SetChannel(Systems::ISAPC);

namespace Systems {
DEFINE_OBJECT_TYPE_INFO(IBMXT);
DEFINE_OBJECT_GENERIC_FACTORY(IBMXT);
BEGIN_OBJECT_PROPERTY_MAP(IBMXT)
PROPERTY_TABLE_MEMBER_UINT("RAMSize", 0, offsetof(IBMXT, m_ram_size), nullptr, 0)
PROPERTY_TABLE_MEMBER_UINT("VideoType", 0, offsetof(IBMXT, m_video_type), nullptr, 0)
PROPERTY_TABLE_MEMBER_STRING("BIOSPath", 0, offsetof(IBMXT, m_bios_file_path), nullptr, 0)
END_OBJECT_PROPERTY_MAP()

IBMXT::IBMXT(float cpu_frequency /* = 4770000.0f */, u32 memory_size /* = 640 * 1024 */,
             VideoType video_type /* = VideoType::Other */, const ObjectTypeInfo* type_info /* = &s_type_info */)
  : BaseClass(type_info), m_bios_file_path("romimages/PCXTBIOS.BIN"), m_ram_size(memory_size), m_video_type(video_type)
{
  m_bus = new Bus(PHYSICAL_MEMORY_BITS);
  m_cpu = CreateComponent<CPU_8086::CPU>("CPU", CPU_8086::MODEL_8088, cpu_frequency);
  AddComponents();
}

IBMXT::~IBMXT() = default;

bool IBMXT::Initialize()
{
  if (m_ram_size < 64 * 1024)
  {
    Log_ErrorPrintf("Invalid RAM size: %u bytes", m_ram_size);
    return false;
  }

  if (!BaseClass::Initialize())
    return false;

  AllocatePhysicalMemory(m_ram_size, true, true, true);

  if (!m_bus->CreateROMRegionFromFile(m_bios_file_path, 0, BIOS_ROM_ADDRESS_8K, 8192))
    return false;

  ConnectSystemIOPorts();
  SetSwitches();
  return true;
}

void IBMXT::Reset()
{
  BaseClass::Reset();

  m_nmi_mask = 0;
}

bool IBMXT::LoadSystemState(BinaryReader& reader)
{
  if (!BaseClass::LoadSystemState(reader))
    return false;

  m_nmi_mask = reader.ReadUInt8();

  return !reader.GetErrorState();
}

bool IBMXT::SaveSystemState(BinaryWriter& writer)
{
  if (!BaseClass::SaveSystemState(writer))
    return false;

  writer.WriteUInt8(m_nmi_mask);

  return !writer.InErrorState();
}

void IBMXT::AddComponents()
{
  m_interrupt_controller = CreateComponent<HW::i8259_PIC>("InterruptController");
  m_dma_controller = CreateComponent<HW::i8237_DMA>("DMAController");
  m_timer = CreateComponent<HW::i8253_PIT>("PIT");
  m_ppi = CreateComponent<HW::XT_PPI>("PPI");
  m_speaker = CreateComponent<HW::PCSpeaker>("Speaker");

  m_fdd_controller = CreateComponent<HW::FDC>("FDC", HW::FDC::Model_8272);
}

void IBMXT::ConnectSystemIOPorts()
{
  // Connect channel 0 of the PIT to the interrupt controller
  m_timer->SetChannelOutputChangeCallback(0,
                                          [this](bool value) { m_interrupt_controller->SetInterruptState(0, value); });

  // Connect channel 2 of the PIT to the speaker
  m_timer->SetChannelOutputChangeCallback(2, [this](bool value) { m_speaker->SetLevel(value); });

  // Connect PPI to speaker
  m_ppi->SetSpeakerGateCallback([this](bool enabled) { m_timer->SetChannelGateInput(2, enabled); });
  m_ppi->SetSpeakerEnableCallback([this](bool enabled) { m_speaker->SetOutputEnabled(enabled); });
  m_ppi->SetSpeakerOutputCallback([this]() -> bool { return m_timer->GetChannelOutputState(2); });

  // The XT has no second interrupt controller.
  m_bus->ConnectIOPortReadToPointer(0x00A0, this, &m_nmi_mask);
  m_bus->ConnectIOPortWriteToPointer(0x00A0, this, &m_nmi_mask);

  // We need to set up a fake DMA channel for memory refresh.
  m_dma_controller->ConnectDMAChannel(
    0, [](IOPortDataSize, u32*, u32) {}, [](IOPortDataSize, u32, u32) {});

  // Connect channel 1 of the PIT to trigger memory refresh.
  m_timer->SetChannelOutputChangeCallback(1, [this](bool value) { m_dma_controller->SetDMAState(0, value, 65536); });
}

void IBMXT::SetSwitches()
{
  // Switch settings.
  bool boot_loop = false;
  bool numeric_processor_installed = false;
  PhysicalMemoryAddress base_memory = GetBaseMemorySize();
  u32 num_disk_drives = m_fdd_controller->GetDriveCount();

  // From http://www.rci.rutgers.edu/~preid/pcxtsw.htm
  m_ppi->SetSwitch(1 - 1, !boot_loop);
  m_ppi->SetSwitch(2 - 1, !numeric_processor_installed);
  if (base_memory >= 640 * 1024)
  {
    m_ppi->SetSwitch(3 - 1, false);
    m_ppi->SetSwitch(4 - 1, false);
  }
  else if (base_memory >= 576 * 1024)
  {
    m_ppi->SetSwitch(3 - 1, true);
    m_ppi->SetSwitch(4 - 1, false);
  }
  else if (base_memory >= 512 * 1024)
  {
    m_ppi->SetSwitch(3 - 1, false);
    m_ppi->SetSwitch(4 - 1, true);
  }
  else
  {
    m_ppi->SetSwitch(3 - 1, true);
    m_ppi->SetSwitch(4 - 1, true);
  }

  switch (m_video_type)
  {
    case VideoType::MDA:
      m_ppi->SetSwitch(5 - 1, false);
      m_ppi->SetSwitch(6 - 1, false);
      break;
    case VideoType::CGA80:
      m_ppi->SetSwitch(5 - 1, false);
      m_ppi->SetSwitch(6 - 1, true);
      break;
    case VideoType::CGA40:
      m_ppi->SetSwitch(5 - 1, true);
      m_ppi->SetSwitch(6 - 1, false);
      break;
    case VideoType::Other:
    default:
      m_ppi->SetSwitch(5 - 1, true);
      m_ppi->SetSwitch(6 - 1, true);
      break;
  }

  switch (num_disk_drives)
  {
    case 4:
      m_ppi->SetSwitch(7 - 1, false);
      m_ppi->SetSwitch(8 - 1, false);
      break;
    case 3:
      m_ppi->SetSwitch(7 - 1, true);
      m_ppi->SetSwitch(8 - 1, false);
      break;
    case 2:
      m_ppi->SetSwitch(7 - 1, false);
      m_ppi->SetSwitch(8 - 1, true);
      break;
    case 1:
    case 0:
    default:
      m_ppi->SetSwitch(7 - 1, true);
      m_ppi->SetSwitch(8 - 1, true);
      break;
  }
}

void IBMXT::HandlePortRead(u32 port, u8* value)
{
  switch (port)
  {
      // NMI Mask
    case 0xA0:
      *value = m_nmi_mask;
      break;
  }
}

void IBMXT::HandlePortWrite(u32 port, u8 value)
{
  switch (port)
  {
    case 0xA0:
      Log_WarningPrintf("NMI Mask <- 0x%02X", ZeroExtend32(value));
      m_nmi_mask = value;
      break;
  }
}

} // namespace Systems