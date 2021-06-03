#ifndef __DS4CONTROLLER_H__
#define __DS4CONTROLLER_H__

#include "BluetoothHID.h"

#define DS4_VID 0x054c
#define DS4_PID 0x05c4

union PS4Buttons {
  struct {
    uint8_t dpad : 4;
    uint8_t square : 1;
    uint8_t cross : 1;
    uint8_t circle : 1;
    uint8_t triangle : 1;

    uint8_t l1 : 1;
    uint8_t r1 : 1;
    uint8_t l2 : 1;
    uint8_t r2 : 1;
    uint8_t share : 1;
    uint8_t options : 1;
    uint8_t l3 : 1;
    uint8_t r3 : 1;

    uint8_t ps : 1;
    uint8_t touchpad : 1;
    uint8_t reportCounter : 6;
  } __attribute__((packed));
  uint32_t val : 24;
} __attribute__((packed));

typedef struct {
  uint8_t LeftX;
  uint8_t LeftY;
  uint8_t RightX;
  uint8_t RightY;
  PS4Buttons Buttons;
  uint8_t LT;
  uint8_t RT;
} __attribute__((packed)) DS4Data;

typedef struct {
  uint8_t PacketCounter;
  uint32_t Finger1Data;
  uint32_t Finger2Data;
} __attribute__((packed)) TrackpadPacket;

typedef struct {
  uint8_t dummy0; // always 0xC0?
  uint8_t ReportID; // Always 0x00
  uint8_t LeftX;
  uint8_t LeftY;
  uint8_t RightX;
  uint8_t RightY;
  PS4Buttons Buttons;
  uint8_t LT;
  uint8_t RT;
  uint16_t Timestamp;
  uint8_t Battery;
  int16_t AngularVelocityX;
  int16_t AngularVelocityY;
  int16_t AngularVelocityZ;
  int16_t AccelerationX;
  int16_t AccelerationY;
  int16_t AccelerationZ;
  uint32_t dummy1; // Always 0x00;
  uint8_t dummy2; // Always 0x00;
  uint8_t peripheral;
  uint16_t dummy3; // Always 0x00
  uint8_t TrackpadPacketCount;
  TrackpadPacket Packet[4];
  uint16_t dummy4;
  uint32_t crc32;
} __attribute__((packed)) DS4DataExt;

enum DS4ReportingState {
  None,
  Standard,
  Extended,
};

struct DS4LEDColor_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

class DS4Controller : public CBluetoothDeviceBase {
  public:
    DS4Controller();
    ~DS4Controller();

    virtual void OnOpenDevice();
    virtual void OnCloseDevice(BHIDCloseEvt *pCloseEvent);
    virtual void OnInputReport(BHIDInputEvt *pInputReport);
    virtual void OnFeatureReport(BHIDFeatureEvt *pFeatureReport);
    virtual void OnBatteryEvent(BHIDBatteryEvt *pBatteryReport);
    void GetLatestInputReport(DS4Data *data);
    void GetLatestInputReport(DS4DataExt *data);
    bool HasNewReport();
    void SetExtendedReporting();
    DS4ReportingState GetReportingState();
    void SetRumble(uint8_t left, uint8_t right, bool update = true);
    void SetLED(uint8_t r, uint8_t g, uint8_t b, bool update = true);
    void UpdateState();

  private:
    void SendOutputReport();
    void generate_crc_table(uint32_t *crcTable);
    uint32_t crc32_le(unsigned int crc, unsigned char const * buf, unsigned int len);


    DS4Data DS4;
    DS4DataExt DS4Ext;
    size_t report_num;
    size_t last_read_report_num;
    DS4ReportingState m_ReportingState;
    // TODO: make struct for output report rather than editing buffer

    uint8_t *m_pOutputReport;
    DS4LEDColor_t DS4LEDColor;
    uint32_t *m_pCrcTable;
};

#endif // __DS4CONTROLLER_H__
