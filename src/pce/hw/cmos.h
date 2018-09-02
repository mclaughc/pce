#pragma once

#include "common/clock.h"
#include "pce/component.h"
#include <YBaseLib/Assert.h>
#include <array>
#include <memory>
#include <vector>

class ByteStream;
class InterruptController;

namespace HW {

class CMOS : public Component
{
  DECLARE_OBJECT_TYPE_INFO(CMOS, Component);
  DECLARE_OBJECT_NO_FACTORY(CMOS);
  DECLARE_OBJECT_NO_PROPERTIES(CMOS);

public:
  CMOS(const String& identifier, const ObjectTypeInfo* type_info = &s_type_info);
  ~CMOS();

  bool Initialize(System* system, Bus* bus) override;
  void Reset() override;
  bool LoadState(BinaryReader& reader) override;
  bool SaveState(BinaryWriter& writer) override;

  uint8 GetVariable(uint8 index) const { return m_data[index]; }
  void SetVariable(uint8 index, uint8 value) { m_data[index] = value; }

  uint16 GetWordVariable(uint8 base_index) const;
  void SetWordVariable(uint8 base_index, uint16 value);

  void SetFloppyType(uint32 index, uint32 type);
  void SetFloppyCount(uint32 count);

protected:
  static const uint32 SERIALIZATION_ID = MakeSerializationID('C', 'M', 'O', 'S');
  static const uint32 IOPORT_INDEX_REGISTER = 0x70;
  static const uint32 IOPORT_DATA_PORT = 0x71;
  static const uint32 RTC_INTERRUPT = 8;

  enum RTC_REGISTERS
  {
    RTC_REGISTER_STATUS_REGISTER_B = 0x0B,
    RTC_REGISTER_STATUS_REGISTER_C = 0x0C,
  };
  enum RTC_SRB : uint8
  {
    RTC_SRB_PERIODIC_INTERRUPT_ENABLE = (1 << 6),
  };
  enum RTC_SRC : uint8
  {
    RTC_SRC_PERIODIC_INTERRUPT = (1 << 6),
  };

  void ConnectIOPorts(Bus* bus);
  bool HandleKnownCMOSRead(uint8 index, uint8* value);
  bool HandleKnownCMOSWrite(uint8 index, uint8 value);
  void IOWriteReadRegister(uint8* value);
  void IOWriteIndexRegister(uint8 value);
  void IOReadDataPort(uint8* value);
  void IOWriteDataPort(uint8 value);

  void UpdateRTCFrequency();
  void RTCInterruptEvent(CycleCount cycles);

  InterruptController* m_interrupt_controller = nullptr;
  std::array<uint8, 256> m_data = {};
  uint8 m_index_register = 0;
  bool m_nmi_enabled = false;

  Clock m_rtc_clock;
  CycleCount m_rtc_interrupt_rate = 1;
  TimingEvent::Pointer m_rtc_interrupt_event{};
};

} // namespace HW
