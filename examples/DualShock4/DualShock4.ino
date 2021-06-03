#include "DS4Controller.h"

DS4Controller DS4Instance;
DS4DataExt LastestState;
unsigned long lastOutputReport = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  
  BluetoothHID.Init();
  // Register device
  BluetoothHID.RegisterDevice(&DS4Instance);

  // If DS4 in pairing mode, scan and connect, sometime issues occur when connecting while
  // scanning so should only be called when pairing is needed
  BluetoothHID.ScanAndConnectHIDDevice();

  // Core 0 only for BT, do all processing in core 1 if needed e.g.
  // xTaskCreatePinnedToCore(&task, "DS4 Task", 16 * 1024, NULL, 5, NULL, 1);

  Serial.println("Started");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (DS4Instance.GetState() == BTHIDState::Connected) {
    if (DS4Instance.HasNewReport()) {
      switch (DS4Instance.GetReportingState()) {
        case DS4ReportingState::Standard: {
            DS4Instance.SetExtendedReporting();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            break;
          }
        case DS4ReportingState::Extended: {
            DS4Instance.GetLatestInputReport(&LastestState);
            //Serial.println(LastestState.LeftX);
            // Slow update if needed
            if ((millis() - lastOutputReport) > 5) {
              DS4Instance.SetLED(LastestState.LeftX, LastestState.LeftY, LastestState.RightX, false);
              DS4Instance.SetRumble(LastestState.LT, LastestState.RT, false);
              DS4Instance.UpdateState();

              lastOutputReport = millis();
            }
            break;
          }
        default:
          break;
      }
    }
  }
  vTaskDelay(5 / portTICK_PERIOD_MS);
}
