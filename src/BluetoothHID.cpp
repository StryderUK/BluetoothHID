#include "BluetoothHID.h"

class CBluetoothHID BluetoothHID;

extern "C" void CBluetoothHID::hidh_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
  esp_hidh_event_t event = (esp_hidh_event_t)id;
  esp_hidh_event_data_t *param = (esp_hidh_event_data_t *)event_data;

  // Find active device
  dev_context *pDevice = NULL;
  for (size_t i = 0; i < BluetoothHID.m_BluetoothDevicesSize; i++)
  {
    if (param->open.dev == BluetoothHID.m_pBluetoothDevices[i]->dev)
    {
      pDevice = BluetoothHID.m_pBluetoothDevices[i];
      break;
    }
  }

  switch (event)
  {
  case ESP_HIDH_OPEN_EVENT:
  {
    //esp_hidh_dev_dump(param->open.dev, stdout);
    // Find device and open (VID, PID?)
    if (!pDevice)
    {
      for (size_t i = 0; i < BluetoothHID.m_BluetoothDevicesSize; i++)
      {
        dev_context *pDev = BluetoothHID.m_pBluetoothDevices[i];
        if (pDev)
        {
          if (pDev->pbtDev->m_PID == param->open.dev->config.product_id && pDev->pbtDev->m_VID == param->open.dev->config.vendor_id)
          {
            pDev->dev = param->open.dev;
            if (pDev->pbtDev)
            {
              //ESP_LOGV(BTHID_TAG, "Device Connected!", event);
              pDev->pbtDev->EventHandler(handler_args, base, id, event_data);
            }
            break;
          }
        }
      }
    }
    else
    {
      // Device of that type already exists, needed to call this again?
      if (pDevice->pbtDev)
      {
        pDevice->pbtDev->EventHandler(handler_args, base, id, event_data);
      }
    }
    break;
  }
  case ESP_HIDH_CLOSE_EVENT:
  {
    // Close device then call event handler
    if (pDevice)
    {
      if (pDevice->pbtDev)
        pDevice->pbtDev->EventHandler(handler_args, base, id, event_data);
      pDevice->dev = NULL;
    }
    // MUST call this function to free all allocated memory by this device*/
    esp_hidh_dev_free(param->close.dev);
    break;
  }
  default:
    //ESP_LOGI(BTHID_TAG, "EVENT: %d", event);
    // Find opened devs and call events
    if (pDevice)
    {
      if (pDevice->pbtDev)
        pDevice->pbtDev->EventHandler(handler_args, base, id, event_data);
    }
    break;
  }
}