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

#include "esp_hidd.h"
#include "private/esp_hidd_private.h"
#include "esp_event_base.h"

#if CONFIG_GATTS_ENABLE
#include "private/ble_hidd.h"
#endif /* CONFIG_GATTS_ENABLE */

ESP_EVENT_DEFINE_BASE(ESP_HIDD_EVENTS);

esp_err_t esp_hidd_dev_init(const esp_hid_device_config_t *config, esp_hid_transport_t transport, esp_event_handler_t callback, esp_hidd_dev_t **dev_out)
{

    esp_err_t ret = ESP_OK;
    esp_hidd_dev_t *dev = (esp_hidd_dev_t *)calloc(1, sizeof(esp_hidd_dev_t));
    if (dev == NULL) {
        return ESP_FAIL;
    }

    switch (transport) {
#if CONFIG_GATTS_ENABLE
    case ESP_HID_TRANSPORT_BLE:
        ret = esp_ble_hidd_dev_init(dev, config, callback);
        break;
#endif /* CONFIG_GATTS_ENABLE */
    default:
        ret = ESP_FAIL;
        break;
    }

    if (ret != ESP_OK) {
        free(dev);
        return ret;
    }
    dev->transport = transport;
    *dev_out = dev;
    return ret;
}

esp_err_t esp_hidd_dev_deinit(esp_hidd_dev_t *dev)
{
    if (dev == NULL) {
        return ESP_FAIL;
    }
    esp_err_t ret = dev->deinit(dev->dev);
    if (ret != ESP_OK) {
        return ret;
    }
    free(dev);
    return ret;
}

esp_hid_transport_t esp_hidd_dev_transport_get(esp_hidd_dev_t *dev)
{
    if (dev == NULL) {
        return ESP_HID_TRANSPORT_MAX;
    }
    return dev->transport;
}

bool esp_hidd_dev_connected(esp_hidd_dev_t *dev)
{
    if (dev == NULL) {
        return false;
    }
    return dev->connected(dev->dev);
}

esp_err_t esp_hidd_dev_battery_set(esp_hidd_dev_t *dev, uint8_t level)
{
    if (dev == NULL) {
        return ESP_FAIL;
    }
    return dev->battery_set(dev->dev, level);
}

esp_err_t esp_hidd_dev_input_set(esp_hidd_dev_t *dev, size_t map_index, size_t report_id, uint8_t *data, size_t length)
{
    if (dev == NULL) {
        return ESP_FAIL;
    }
    return dev->input_set(dev->dev, map_index, report_id, data, length);
}

esp_err_t esp_hidd_dev_feature_set(esp_hidd_dev_t *dev, size_t map_index, size_t report_id, uint8_t *data, size_t length)
{
    if (dev == NULL) {
        return ESP_FAIL;
    }
    return dev->feature_set(dev->dev, map_index, report_id, data, length);
}

esp_err_t esp_hidd_dev_event_handler_register(esp_hidd_dev_t *dev, esp_event_handler_t callback, esp_hidd_event_t event)
{
    if (dev == NULL) {
        return ESP_FAIL;
    }
    return dev->event_handler_register(dev->dev, callback, event);
}

esp_err_t esp_hidd_dev_event_handler_unregister(esp_hidd_dev_t *dev, esp_event_handler_t callback, esp_hidd_event_t event)
{
    if (dev == NULL) {
        return ESP_FAIL;
    }
    return dev->event_handler_unregister(dev->dev, callback, event);
}
