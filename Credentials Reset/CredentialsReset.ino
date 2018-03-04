#include "FS.h"
#include "WiFiManager.h"

void setup() {
  Serial.begin(115200);
  Serial.println("");
  delay(1000);
  Serial.println("Mounting FS...");

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  if (!SPIFFS.format()) {
    Serial.println("Failed to format");
  } else {
    Serial.println("File system formatted");
  }

  WiFiManager wifiManager;
  delay(1000);
  wifiManager.resetSettings();
  delay(1000);

  Serial.println("");
  Serial.println("");
  Serial.println("Flash New Sketch Now");
}

void loop() {
}
