# BluetoothHID (w/ DualShock 4 Bluetooth Example)

**WARNING: This project is in early stages of development, and may not work as intended**

Bluetooth his is a library to enable Bluetooth HID on the ESP32 with the aeduino-esp32 core (2.0.0-alpha1).

The example provided shows how to use it with a DualShock 4 and will change the LED with the left and right analogue sticks, as well as changing the rumble with LT+RT

If this is popular enough I will include the DualShock class within the main library as well as potentially add support for the DualShock 5

***NOTE: This library has only been tested with the DualShock 4 as this is the only bluetooth HID device I have spare***

## Table of Contents

- [Getting Started](#getting-started)
  - [Basic Usage](#basic-usage)
  - [Considerations](#considerations)
- [About](#about)

## Getting Started

1. Install esp32 board 2.0.0-alpha1 in Arduino (board link below as of writing)

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
```

2. Navigate to SDK location:

**Windows:**
```
C:\Users\<Username>\AppData\Local\Arduino15\packages\esp32\hardware\esp32
```

**Linux:**
```
~/.arduino15/packages/esp32/hardware/esp32/
```

3. Overwrite files with files in the sdk_files folder

**NOTE: If there are any issues and you need to revert to original, uninstall then re-install the esp boards via the Arduino board manager**

### Basic Usage

See example code ([DualShock4.ino](examples/DualShock4/DualShock4.ino)) for how to use the library

1. Open DualShock 4 example and upload to ESP32.
2. Put DualShock 4 into pairing mode (Holding share + PS Buttons together while controller is off, its easier to hold the share button first)
3. When DualShock 4 is flashing, reset the ESP32 (the ESP32 will scan for devices for 5 seconds on bootup)
4. The DualShock 4 should now pair to the ESP32

Pairing only needs to be done once, after that the DualShock will reconnect to the ESP32 until it is paired to something else.



### Considerations

The Bluetooth stack is running on core 0, and due to the fast reporting of HID devices, it would be best running all needed tasks on core 1 (which is usually the default Arduino core unless selected, so putting everything in the loop function is usually fine).

If a task is needed to be run, it can be pinned to core 1 using:
```
xTaskCreatePinnedToCore
```
Usage example:
```cpp
void process_task(void *pvParameters)
{
  while (true) {
    ...
    // Do processing of anything here on core 1 to not hold up BT Stack
    ...
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  // Should never get here!
  vTaskDelete(NULL);
}

void setup() {
...
  // BT Stack runs on Core 0, carry out all processing on core 1
  xTaskCreatePinnedToCore(&process_task, "Processing_Task", 8 * 1024, NULL, 2, NULL, 1);
}
```

## About

Initially I could not find a DualShock 4 library for the ESP32 that allowed pairing using the built-in BT module (some need the mac address of the connected PS4, and others need an external BT module). Luckily, the latest ESP-IDF (v4.2) has a HID module that supported Bluetooth. I got this to work in ESP-IDF, but the program would freeze, or I would get stack overflows due to the fast reporting of the DualShock 4 (filling the event queue up faster than it can process the events). So I decided to dig around to see if I could do some quick modifications to get it to work, and be compatible with Arduino, which most of my projects are coded in.

The module is based off of the [ESP HID Bluetooth example](https://github.com/espressif/esp-idf/tree/master/examples/bluetooth/esp_hid_device) (for ESP-IDF)

Hopefully this library will help someone else and save the pain of trying to get fast reporting HID devices working

It would be nice if espressif can enable the BT HID components in a future release, making this library redundant.
