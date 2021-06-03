# Bluetooth HID

## Table of Contents

- [About](#about)
- [Getting Started](#getting-started)
  - [Basic Usage](#basic-usage)

## About

This is a port of the ESP-IDF (v4.2) bluetooth and hid modules. The android core is currently not compiled with Bluetooth HID enable. When comiling with it enabled I then had issues with the event queue freezing as the device i was testing (DualShock 4) was filling up the event queue faster than it was being read.

In this library I have disabled the event queue the HID component and for input reports in the bluetooth component (events are handled straight away)

## Getting Started

Install esp32 board 2.0.0-alpha1 in Arduino (board link below as of writing)

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
```

Navigate to SDK location

Windows:
```
C:\Users\<Username>\AppData\Arduino15\packages\esp32\hardware\esp32
```

Linux:
```
~/.arduino15/packages/esp32/hardware/esp32/
```

Overwrite files with files in the sdk_files folder

### Basic Usage

See examples folder
