#include "DS4Controller.h"

#include <Arduino.h>

#define DS4_FEATURE_REPORT_0x02_SIZE 37
#define DS4_OUTPUT_REPORT_0x11_SIZE 77

#define DS4_OUTPUT_UPDATE_CONTROL_INDEX 2
#define DS4_OUTPUT_RUMBLE_RIGHT_WEAK_INDEX 5
#define DS4_OUTPUT_RUMBLE_LEFT_STRONG_INDEX 6
#define DS4_OUTPUT_LED_INDEX_R 7
#define DS4_OUTPUT_LED_INDEX_G 8
#define DS4_OUTPUT_LED_INDEX_B 9
#define DS4_OUTPUT_LED_FLASH_BRIGHT_INDEX 10
#define DS4_OUTPUT_LED_FLASH_DARK_INDEX 11
#define DS4_OUTPUT_LED_CRC_INDEX 73

#define DS4_OUTPUT_UPDATE_MASK 0xFF
#define DS4_OUTPUT_UPDATE_RUMBLE (1) // << 0)
#define DS4_OUTPUT_UPDATE_LED (1 << 1)
#define DS4_OUTPUT_UPDATE_FLASH (1 << 2)

uint8_t ds4feature02[DS4_FEATURE_REPORT_0x02_SIZE];
size_t ds4feature02len;

const uint8_t OutputReportDefault[]  = { 0xc0, 0x20, 0xf0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x43, 0x00, 0x4d, 0x85, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x8e, 0x94, 0xdd
                                       };

DS4Controller::DS4Controller() : CBluetoothDeviceBase(DS4_VID, DS4_PID)
{
    report_num = 0;
    last_read_report_num = 0;
    m_ReportingState = DS4ReportingState::None;
    m_pOutputReport = new uint8_t[sizeof(OutputReportDefault)];
    memcpy(m_pOutputReport, &OutputReportDefault, sizeof(OutputReportDefault));

    m_pCrcTable = new uint32_t[256];
    generate_crc_table(m_pCrcTable);
}

DS4Controller::~DS4Controller()
{
    delete[] m_pCrcTable;
    delete[] m_pOutputReport;
}

void DS4Controller::OnOpenDevice()
{
  Serial.println(__FUNCTION__);
}

void DS4Controller::OnCloseDevice(BHIDCloseEvt *pCloseEvent)
{
  Serial.println(__FUNCTION__);
}

void DS4Controller::OnInputReport(BHIDInputEvt *pInputReport)
{
  // Standard report
  if (pInputReport->report_id == 0x01 && pInputReport->length == 9) {
    memcpy(&DS4, pInputReport->data, 7);
    report_num++;
    m_ReportingState = DS4ReportingState::Standard;

  }
  // Extended report
  else if (pInputReport->report_id == 0x11 && pInputReport->length == 77) {
    // Check data valid
    uint8_t bthdr[] = { 0xA1, 0x11 };
    uint32_t crc;

    crc = this->crc32_le(0xFFFFFFFF, (uint8_t*)&bthdr, sizeof(bthdr));
    crc = ~this->crc32_le(crc, pInputReport->data, pInputReport->length - sizeof(uint32_t));


    if (crc == ((DS4DataExt*)pInputReport->data)->crc32) {
      memcpy(&DS4Ext, pInputReport->data, 77);
      report_num++;
      m_ReportingState = DS4ReportingState::Extended;
    } else {
      ESP_LOGW(TAG, "CRC32 NG! 0x%08X != 0x%08X", crc, ((DS4DataExt*)pInputReport->data)->crc32);
    }
  }
}

void DS4Controller::OnFeatureReport(BHIDFeatureEvt *pFeatureReport)
{
  Serial.println(__FUNCTION__);
}

void DS4Controller::OnBatteryEvent(BHIDBatteryEvt *pBatteryReport)
{
  Serial.println(__FUNCTION__);
}

void DS4Controller::GetLatestInputReport(DS4Data *data) {
  memcpy(data, (void*)&DS4, sizeof(DS4Data));
  last_read_report_num = report_num;
}

void DS4Controller::GetLatestInputReport(DS4DataExt *data) {
  memcpy(data, (void*)&DS4Ext, sizeof(DS4DataExt));
  last_read_report_num = report_num;
}

bool DS4Controller::HasNewReport() {
  return (report_num != last_read_report_num);
}

void DS4Controller::SetExtendedReporting() {
  esp_hidh_dev_feature_get(this->dev(), 0, 0x02, DS4_FEATURE_REPORT_0x02_SIZE, (uint8_t*)&ds4feature02, &ds4feature02len);
}

DS4ReportingState DS4Controller::GetReportingState() {
  return m_ReportingState;
}

void DS4Controller::SetRumble(uint8_t left, uint8_t right, bool update) {
  m_pOutputReport[DS4_OUTPUT_RUMBLE_RIGHT_WEAK_INDEX] = left;
  m_pOutputReport[DS4_OUTPUT_RUMBLE_LEFT_STRONG_INDEX] = right;
  m_pOutputReport[DS4_OUTPUT_UPDATE_CONTROL_INDEX] |= (DS4_OUTPUT_UPDATE_RUMBLE & DS4_OUTPUT_UPDATE_MASK);

  if (update) {
    SendOutputReport();
  }
}

void DS4Controller::SetLED(uint8_t r, uint8_t g, uint8_t b, bool update) {
  if (DS4LEDColor.r != r || DS4LEDColor.g != g || DS4LEDColor.b != b) {
    DS4LEDColor.r = r;
    DS4LEDColor.g = g;
    DS4LEDColor.b = b;

    m_pOutputReport[DS4_OUTPUT_LED_INDEX_R] = r;
    m_pOutputReport[DS4_OUTPUT_LED_INDEX_G] = g;
    m_pOutputReport[DS4_OUTPUT_LED_INDEX_B] = b;
    m_pOutputReport[DS4_OUTPUT_UPDATE_CONTROL_INDEX] |= (DS4_OUTPUT_UPDATE_LED & DS4_OUTPUT_UPDATE_MASK);

    if (update) {
      SendOutputReport();
    }
  }
}

void DS4Controller::UpdateState() {
  SendOutputReport();
}

//esp_hidh_dev_output_set(esp_hidh_dev_t *dev, size_t map_index, size_t report_id, uint8_t *data, size_t length);
void DS4Controller::SendOutputReport() {
  // Calculate CRC
  uint8_t bthdr[] = { 0xA2, 0x11 };
  uint32_t crc;

  crc = this->crc32_le(0xFFFFFFFF, (uint8_t*)&bthdr, sizeof(bthdr));
  crc = ~this->crc32_le(crc, m_pOutputReport, sizeof(OutputReportDefault) - sizeof(uint32_t));

  *((uint32_t*)(m_pOutputReport + DS4_OUTPUT_LED_CRC_INDEX)) = crc;

  esp_hidh_dev_output_set(this->dev(), 0, 0x11, m_pOutputReport, sizeof(OutputReportDefault));
  // Clear control bits
  m_pOutputReport[DS4_OUTPUT_UPDATE_CONTROL_INDEX] &= ~DS4_OUTPUT_UPDATE_MASK;
}

void DS4Controller::generate_crc_table(uint32_t *crcTable) {
  const uint32_t POLYNOMIAL = 0xEDB88320; // 0x04C11DB7 reversed
  uint32_t remainder;
  uint8_t b = 0;
  do {
    // Start with the data byte
    remainder = b;
    for (unsigned long bit = 8; bit > 0; --bit) {
      if (remainder & 1)
        remainder = (remainder >> 1) ^ POLYNOMIAL;
      else
        remainder = (remainder >> 1);
    }
    crcTable[(size_t)b] = remainder;
  } while (0 != ++b);
}

uint32_t DS4Controller::crc32_le(unsigned int crc, unsigned char const * buf, unsigned int len)
{
  uint32_t i;
  for (i = 0; i < len; i++) {
    crc = m_pCrcTable[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
  }
  return crc;
}
