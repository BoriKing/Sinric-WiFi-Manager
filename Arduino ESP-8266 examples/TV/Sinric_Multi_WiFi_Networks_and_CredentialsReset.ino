#include <FS.h>                   // this needs to be first, or it all crashes and burns...
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WebSocketsClient.h>     // https://github.com/Links2004/arduinoWebSockets/releases 
#include <ArduinoJson.h>          // https://arduinojson.org/ or install via Arduino library manager
#include "WiFiManager.h"          // https://github.com/the-real-orca/WiFiManager
#include <DoubleResetDetector.h>  // https://github.com/datacute/DoubleResetDetector/tree/master/src
#include <Ticker.h>               // https://github.com/esp8266/Arduino/tree/master/libraries/Ticker
#include <IRremoteESP8266.h>      // https://github.com/markszabo/IRremoteESP8266
#include <IRsend.h>

#define DRD_TIMEOUT 1
#define DRD_ADDRESS 0
#define HEARTBEAT_INTERVAL 300000 // 5 Minutes
#define IR_LED 5                  // GPIO pin for IR LED

char a_k[37];
char f_dID[25];

IRsend irsend(IR_LED);
WebSocketsClient webSocket;
Ticker flipper;
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

uint64_t heartbeatTimestamp = 0;

bool isConnected = false;
bool shouldSaveConfig = false;
bool volumeUP = 0;
bool volumeDOWN = 0;

const int PIN_LED = 2;           // Onboard LED I/O pin 2 on ESP8266 12-E Module

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      isConnected = false;
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
        isConnected = true;
        Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
        Serial.printf("Waiting for commands from sinric.com ...\n");
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // On Off format https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"}

        // https://developer.amazon.com/docs/device-apis/alexa-channelcontroller.html
        // {"deviceId":"5a90faedd923a349530330c3","action":"SkipChannels","value":{"channelCount":1}}
        // {"deviceId":"5a90faedd923a349530330c3","action":"ChangeChannel","value":{"channel":{"number":"200"},"channelMetadata":{}}}

        // https://developer.amazon.com/docs/device-apis/alexa-playbackcontroller.html
        // {"deviceId":"5a90faedd923a349530330c3","action":"Play","value":{}}
        // {"deviceId":"5a90faedd923a349530330c3","action":"Pause","value":{}}
        // {"deviceId":"5a90faedd923a349530330c3","action":"Stop","value":{}}
        // {"deviceId":"5a90faedd923a349530330c3","action":"SetVolume","value":{"volume":57}}

        // https://developer.amazon.com/docs/device-apis/alexa-inputcontroller.html
        // {"deviceId":"5a90faedd923a349530330c3","action":"SelectInput","value":{"input":"HDMI 1"}}

        // https://developer.amazon.com/docs/device-apis/alexa-speaker.html
        // {"deviceId":"5a90faedd923a349530330c3","action":"AdjustVolume","value":{"volume":10,"volumeDefault":true}}
        // {"deviceId":"5a90faedd923a349530330c3","action":"SetMute","value":{"mute":true}}
        // {"deviceId":"5a90faedd923a349530330c3","action":"SetVolume","value":{"volume":57}}



        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
        String deviceId = json ["deviceId"];
        String action = json ["action"];
        if (deviceId == (f_dID)) {
          if (action == "setPowerState") {
            String value = json ["value"];
            if (value == "ON") {
              irsend.sendNEC(0x20DFD728, 32); //Power On Key
            } else {
              irsend.sendNEC(0x20DF10EF, 32); //Power Off Key = Power On Key
            }
          }
          else if (action == "SkipChannels") {
            // Alexa, channel up on device
            String value = json["value"]["channelCount"];
            if (value == "1") {
              irsend.sendNEC(0x20DF00FF, 32); //Channel Up Key
            } else {
              irsend.sendNEC(0x20DF807F, 32); //Channel Down Key
            }
          }
          else if (action == "SetMute") {
            // Alexa, mute device
            // Alexa, mute off device
            String value = json["value"]["mute"];
            if (value == "true") {
              irsend.sendNEC(0x20DF906F, 32); //Mute Key
            } else {
              irsend.sendNEC(0x20DF906F, 32); //Mute Off Key = Mute Key
            }
          }
          else if (action == "AdjustVolume") {
            // Alexa, volume up on device
            String volume = json["value"]["volume"];
            if (volume == "10") {
              volumeUP = 1;
            } else {
              volumeDOWN = 1;
            }
          }
          else if (action == "Pause") {
            // Alexa, pause on device
            String value = json["value"];
            irsend.sendNEC(0x20DFEC13, 32); //Pause Key
          }
          else if (action == "SetVolume") {
            // Alexa, volume xx on device
            String value = json["value"]["volume"];
            Serial.println("[WSc] value: " + value);
          }
          else if (action == "Stop") {
            // Alexa, stop on device
            String value = json["value"];
            irsend.sendNEC(0x20DF0CF3, 32); //Stop Key
          }
          else if (action == "Play") {
            // Alexa, play on device
            String value = json["value"];
            irsend.sendNEC(0x20DFCC33, 32); //Play Key
            //}
          }
          else if (action == "SelectInput") {
            // Alexa, HDMI xx on device
            String value = json["value"]["input"];
            Serial.println("[WSc] value: " + value);
          }
          else if (action == "ChangeChannel") {
            // Alexa, change channel to PBS on device
            String value = json["value"]["channel"];
          }
          else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
          }
        }
      }

      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}

void flip() {
  int state = digitalRead(PIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(PIN_LED, !state);     // set pin to the opposite state
}

void setup() {
  irsend.begin();
  Serial.begin(115200);
  Serial.println();
  pinMode(PIN_LED, OUTPUT);
  delay(10);
  digitalWrite(PIN_LED, LOW);

  if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected");
    WiFiManager wifiManager;
    SPIFFS.format();
    delay(1000);
    wifiManager.resetSettings();
    delay(1000);
    {
      ESP.reset();
      delay(5000);
    }
  }

  WiFiManager wifiManager;
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");

      if (configFile) {
        Serial.println("opened config file...");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial); // debug output to console

        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(a_k, json["API key"]);
          strcpy(f_dID, json["Device Id"]);
          if ( json["Networks"].is<JsonArray>() ) {
            for (int i = 0; i < json["Networks"].size(); i++) {
              auto obj = json["Networks"][i];
              wifiManager.addAP(obj["SSID"], obj["Password"]);
            }
          }
        } else {
          Serial.println("failed to load json config");
          return;
        }
      }
    }
    SPIFFS.end();
  } else {
    Serial.println("failed to mount FS -> format");
    SPIFFS.format();
  }

  // Parameters
  WiFiManagerParameter custom_f_dh("Device");
  WiFiManagerParameter custom_a_kh("Sinric API");
  WiFiManagerParameter custom_a_k("API key:", "Key", a_k, 37);
  WiFiManagerParameter custom_f_dID("Device Id:", "Id", f_dID, 25);

  //WiFiManager
  flipper.attach(0.2, flip);
  wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_a_kh);
  wifiManager.addParameter(&custom_a_k);
  wifiManager.addParameter(&custom_f_dh);
  wifiManager.addParameter(&custom_f_dID);

  wifiManager.autoConnect("Sinric");
  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  digitalWrite(PIN_LED, HIGH);
  flipper.detach();

  //read updated parameters
  strcpy(a_k, custom_a_k.getValue());
  strcpy(f_dID, custom_f_dID.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    if (SPIFFS.begin()) {
      Serial.println("saving config...");
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      JsonArray& networks = json.createNestedArray("Networks");
      for (int i = 0; auto ap = wifiManager.getAP(i); i++ ) {
        JsonObject& obj = jsonBuffer.createObject();
        obj["SSID"] = ap->ssid;
        obj["Password"] = ap->pass;
        networks.add(obj);
      }
      json["API key"] = a_k;
      json["Device Id"] = f_dID;
      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        Serial.println("failed to open config file for writing");
        return;
      }
      json.printTo(Serial); Serial.println(); // debug output to console
      json.printTo(configFile);
      configFile.close();
      SPIFFS.end();
    }
  }
  //if you get here you have connected to the WiFi
  Serial.println();
  Serial.print("Connected to: "); Serial.println(WiFi.SSID());
  Serial.print("Your Local IP is: "); Serial.println(WiFi.localIP());

  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", a_k);

  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
}

void loop() {
  drd.loop();
  webSocket.loop();

  if (isConnected) {
    uint64_t now = millis();

    // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
    if ((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
      heartbeatTimestamp = now;
      webSocket.sendTXT("H");
    }
  }
  if (volumeUP == 1) {
    for (int x = 0; x < 10; x++)
    {
      irsend.sendNEC(0x20DF40BF, 32); //Volume Up Key
      delay(1000);
      volumeUP = 0;
    }
  } else if (volumeDOWN == 1) {
    for (int x = 0; x < 10; x++)
    {
      irsend.sendNEC(0x20DFC03F, 32); //Volume Down Key
      delay(1000);
      volumeDOWN = 0;
    }
  }
}
