// Copyright 2017-2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __BLUETOOTHHID_H__
#define __BLUETOOTHHID_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_BT_HID_HOST_ENABLED

#include "bta_hh/common/bluedroid_user_config.h"
#include "bta_hh/common/bt_target.h"
#include "esp_hid/esp_hidh.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#include "esp_hid/esp_hidh.h"
#include "esp_hid/private/esp_hidh_private.h"
#include "gap/esp_hid_gap.h"

#ifdef __cplusplus
}
#endif

#include <cstring>


#define BTHID_TAG "BluetoothHID"

/**
     * @brief ESP_HIDH_CLOSE_EVENT
     */
typedef struct
{
  int reason;   /*!< Reason why the connection was closed. BLE Only */
} BHIDCloseEvt; /*!< HID callback param of ESP_HIDH_CLOSE_EVENT */

/**
     * @brief ESP_HIDH_BATTERY_EVENT
     */
typedef struct
{
  uint8_t level;  /*!< Battery Level (0-100%) */
} BHIDBatteryEvt; /*!< HID callback param of ESP_HIDH_BATTERY_EVENT */

/**
     * @brief ESP_HIDH_INPUT_EVENT
     */
typedef struct
{
  esp_hid_usage_t usage; /*!< HID report usage */
  uint16_t report_id;    /*!< HID report index */
  uint16_t length;       /*!< HID data length */
  uint8_t *data;         /*!< The pointer to the HID data */
  uint8_t map_index;     /*!< HID report map index */
} BHIDInputEvt;          /*!< HID callback param of ESP_HIDH_INPUT_EVENT */

/**
     * @brief ESP_HIDH_FEATURE_EVENT
     */
typedef struct
{
  esp_hid_usage_t usage; /*!< HID report usage */
  uint16_t report_id;    /*!< HID report index */
  uint16_t length;       /*!< HID data length */
  uint8_t *data;         /*!< The pointer to the HID data */
  uint8_t map_index;     /*!< HID report map index */
} BHIDFeatureEvt;        /*!< HID callback param of ESP_HIDH_FEATURE_EVENT */

enum BTHIDState
{
  Disconnected,
  Connected,
};

class IBluetoothDevice
{
public:
  virtual void OnOpenDevice() = 0;
  virtual void OnCloseDevice(BHIDCloseEvt *pCloseEvent) = 0;
  virtual void OnInputReport(BHIDInputEvt *pInputReport) = 0;
  virtual void OnFeatureReport(BHIDFeatureEvt *pFeatureReport) = 0;
  virtual void OnBatteryEvent(BHIDBatteryEvt *pBatteryReport) = 0;
};

class CBluetoothDeviceBase : IBluetoothDevice
{
  friend class CBluetoothHID;

public:
  CBluetoothDeviceBase(uint16_t VID, uint16_t PID)
  {
    m_VID = VID;
    m_PID = PID;
    m_State = BTHIDState::Disconnected;
  }

  /*virtual void OnOpenDevice() = 0;
  virtual void OnCloseDevice(BHIDCloseEvt *pCloseEvent) = 0;
  virtual void OnInputReport(BHIDInputEvt *pInputReport) = 0;
  virtual void OnFeatureReport(BHIDFeatureEvt *pFeatureReport) = 0;
  virtual void OnBatteryEvent(BHIDBatteryEvt *pBatteryReport) = 0;*/

  BTHIDState GetState()
  {
    return m_State;
  }

protected:
  esp_hidh_dev_t *dev()
  {
    return m_pDev;
  }

private:
  esp_hidh_dev_t *m_pDev;
  uint16_t m_PID;
  uint16_t m_VID;
  BTHIDState m_State;

  void EventHandler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
  {
    esp_hidh_event_t event = (esp_hidh_event_t)id;
    esp_hidh_event_data_t *param = (esp_hidh_event_data_t *)event_data;

    switch (event)
    {
    case ESP_HIDH_OPEN_EVENT:
    {
      /*const uint8_t *bda = esp_hidh_dev_bda_get(param->open.dev);
      ESP_LOGI(BTHID_TAG, ESP_BD_ADDR_STR " OPEN: %s", ESP_BD_ADDR_HEX(bda), esp_hidh_dev_name_get(param->open.dev));
      esp_hidh_dev_dump(param->open.dev, stdout);*/
      m_State = BTHIDState::Connected;
      m_pDev = param->open.dev;
      this->OnOpenDevice();
      break;
    }
    case ESP_HIDH_BATTERY_EVENT:
    {
      /*const uint8_t *bda = esp_hidh_dev_bda_get(param->battery.dev);
      ESP_LOGI(BTHID_TAG, ESP_BD_ADDR_STR " BATTERY: %d%%", ESP_BD_ADDR_HEX(bda), param->battery.level);*/

      BHIDBatteryEvt BatteryReport = {param->battery.level};
      this->OnBatteryEvent(&BatteryReport);
      break;
    }
    case ESP_HIDH_INPUT_EVENT:
    {
      /*const uint8_t *bda = esp_hidh_dev_bda_get(param->input.dev);
      ESP_LOGI(BTHID_TAG, ESP_BD_ADDR_STR " INPUT: %8s, MAP: %2u, ID: %3u, Len: %d, Data:", ESP_BD_ADDR_HEX(bda), esp_hid_usage_str(param->input.usage), param->input.map_index, param->input.report_id, param->input.length);
      //ESP_LOG_BUFFER_HEX(TAG, param->input.data, param->input.length);
      for (uint8_t i = 0; i < param->input.length; i++)
      {
        printf("%02X ", param->input.data[i]);
      }
      printf("\n");*/

      BHIDInputEvt InputReport;
      memcpy((void *)&InputReport, (void *)((uint8_t *)param + sizeof(void *)), sizeof(InputReport));
      this->OnInputReport(&InputReport);
      break;
    }
    case ESP_HIDH_FEATURE_EVENT:
    {
      /*const uint8_t *bda = esp_hidh_dev_bda_get(param->feature.dev);
      ESP_LOGI(BTHID_TAG, ESP_BD_ADDR_STR " FEATURE: %8s, MAP: %2u, ID: %3u, Len: %d", ESP_BD_ADDR_HEX(bda), esp_hid_usage_str(param->feature.usage), param->feature.map_index, param->feature.report_id, param->feature.length);
      ESP_LOG_BUFFER_HEX(BTHID_TAG, param->feature.data, param->feature.length);*/

      BHIDFeatureEvt FeatureReport;
      memcpy((void *)&FeatureReport, (void *)((uint8_t *)param + sizeof(void *)), sizeof(FeatureReport));
      this->OnFeatureReport(&FeatureReport);
      break;
    }
    case ESP_HIDH_CLOSE_EVENT:
    {
      /*/onst uint8_t *bda = esp_hidh_dev_bda_get(param->close.dev);
      ESP_LOGI(BTHID_TAG, ESP_BD_ADDR_STR " CLOSE: '%s' %s", ESP_BD_ADDR_HEX(bda), esp_hidh_dev_name_get(param->close.dev), esp_hid_disconnect_reason_str(esp_hidh_dev_transport_get(param->close.dev), param->close.reason));
      */
      //MUST call this function to free all allocated memory by this device
      m_State = BTHIDState::Disconnected;
      m_pDev = NULL;

      BHIDCloseEvt CloseReport = {param->close.reason};
      this->OnCloseDevice(&CloseReport);
      // Manages with BluetoothHID class?
      //esp_hidh_dev_free(param->close.dev);
      break;
    }
    default:
      ESP_LOGI(BTHID_TAG, "UNHANDLED EVENT: %d", event);
      break;
    }
  }
};

#define MAX_BT_DEVICES 10
//class CBluetoothHID;
extern class CBluetoothHID BluetoothHID;

class CBluetoothHID
{
public:
  CBluetoothHID()
  {
    for (size_t i = 0; i < MAX_BT_DEVICES; i++)
    {
      m_pBluetoothDevices[i] = NULL;
    }
  }
  ~CBluetoothHID()
  {
    for (size_t i = 0; i < MAX_BT_DEVICES; i++)
    {
      if (m_pBluetoothDevices[i])
      {
        delete m_pBluetoothDevices[i];
        m_pBluetoothDevices[i] = NULL;
      }
    }
  }

  bool Init()
  {
    ESP_ERROR_CHECK(esp_hid_gap_init(ESP_BT_MODE_BTDM));
    ESP_ERROR_CHECK(esp_ble_gattc_register_callback(esp_hidh_gattc_event_handler));
    esp_hidh_config_t config = {
        .callback = CBluetoothHID::hidh_callback,
    };
    ESP_ERROR_CHECK(esp_hidh_init(&config));
    return true;
  }

  // TODO: add COD and/or Name filter
  // TODO: add scan timeout programically
  // TODO: Add complete callback
  void ScanAndConnectHIDDevice()
  {
    xTaskCreate(&hid_device_search, "hid_device_search", 4 * 1024, NULL, 2, NULL);
  }

  bool RegisterDevice(CBluetoothDeviceBase *pDevice)
  {
    if (m_BluetoothDevicesSize >= MAX_BT_DEVICES)
    {
      return false;
    }

    // Search to see if VID & PID already registered
    bool bFound = false;
    for (size_t i = 0; i < m_BluetoothDevicesSize; i++)
    {
      CBluetoothDeviceBase *pDev = m_pBluetoothDevices[i]->pbtDev;
      if (pDev)
      {
        if (pDev->m_PID == pDevice->m_PID && pDev->m_VID == pDevice->m_VID)
        {
          bFound = true;
          break;
        }
      }
    }
    // class of this type already exists
    if (bFound)
    {
      return false;
    }

    dev_context *dc = new dev_context;
    dc->dev = NULL;
    dc->pbtDev = pDevice;

    m_pBluetoothDevices[m_BluetoothDevicesSize] = dc;
    m_BluetoothDevicesSize++;

    return true;
  }

  bool DegisterDevice(CBluetoothDeviceBase *pDevice)
  {
    if (m_BluetoothDevicesSize == 0)
    {
      return false;
    }

    // Search to see if VID & PID already registered
    bool bFound = false;
    size_t i;
    for (i = 0; i < m_BluetoothDevicesSize; i++)
    {
      CBluetoothDeviceBase *pDev = m_pBluetoothDevices[i]->pbtDev;
      if (pDev)
      {
        if (pDev->m_PID == pDevice->m_PID && pDev->m_VID == pDevice->m_VID)
        {
          bFound = true;
          break;
        }
      }
    }
    // class of this type already exists
    if (!bFound)
    {
      return false;
    }

    delete m_pBluetoothDevices[i];
    m_pBluetoothDevices[i] = NULL;
    m_BluetoothDevicesSize--;

    // Shift down
    for (; i < m_BluetoothDevicesSize; i++)
    {
      m_pBluetoothDevices[i]->pbtDev = m_pBluetoothDevices[i + 1]->pbtDev;
      m_pBluetoothDevices[i + 1]->pbtDev = NULL;
    }

    return true;
  }

private:
  struct dev_context
  {
    esp_hidh_dev_t *dev;
    CBluetoothDeviceBase *pbtDev;
  };

  dev_context *m_pBluetoothDevices[MAX_BT_DEVICES];
  size_t m_BluetoothDevicesSize;

#ifndef SCAN_DURATION_SECONDS
#define SCAN_DURATION_SECONDS 5
#endif /* SCAN_DURATION_SECONDS */

  static void hid_device_search(void *pvParameters)
  {
    size_t results_len = 0;
    esp_hid_scan_result_t *results = NULL;
    ESP_LOGV(BTHID_TAG, "SCAN...");
    //start scan for HID devices
    esp_hid_scan(SCAN_DURATION_SECONDS, &results_len, &results);
    ESP_LOGV(BTHID_TAG, "SCAN: %u results", results_len);
    if (results_len)
    {
      esp_hid_scan_result_t *r = results;
      esp_hid_scan_result_t *cr = NULL;
      while (r)
      {
        /*printf("  %s: " ESP_BD_ADDR_STR ", ", (r->transport == ESP_HID_TRANSPORT_BLE) ? "BLE" : "BT ", ESP_BD_ADDR_HEX(r->bda));
      printf("RSSI: %d, ", r->rssi);
      printf("USAGE: %s, ", esp_hid_usage_str(r->usage));*/
        if (r->transport == ESP_HID_TRANSPORT_BLE)
        {
          cr = r;
          /*printf("APPEARANCE: 0x%04x, ", r->ble.appearance);
        printf("ADDR_TYPE: '%s', ", ble_addr_type_str(r->ble.addr_type));*/
        }
        else
        {
          cr = r;
          /*printf("COD: %s[", esp_hid_cod_major_str(r->bt.cod.major));
        esp_hid_cod_minor_print(r->bt.cod.minor, stdout);
        printf("] srv 0x%03x, ", r->bt.cod.service);
        print_uuid(&r->bt.uuid);
        printf(", ");*/
        }
        /*printf("NAME: %s ", r->name ? r->name : "");
      printf("\n");*/
        r = r->next;
      }
      if (cr)
      {
        //open the last result
        //printf("Opening: %s: " ESP_BD_ADDR_STR ", [%s]\n", (cr->transport == ESP_HID_TRANSPORT_BLE) ? "BLE" : "BT ", ESP_BD_ADDR_HEX(cr->bda), cr->name ? cr->name : "");
        esp_hidh_dev_t *p_dev = esp_hidh_dev_open(cr->bda, cr->transport, cr->ble.addr_type);
        //printf("esp_hidh_dev_open returned %d\n", p_dev);
      }
      //free the results
      esp_hid_scan_results_free(results);
    }

    vTaskDelete(NULL);
  }

  static void hidh_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
};

#endif