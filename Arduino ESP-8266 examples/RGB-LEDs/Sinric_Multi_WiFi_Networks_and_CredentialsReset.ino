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
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>              // https://github.com/FastLED/FastLED
#include <StreamString.h>

#define NUM_LEDS 16               //Number of led's
#define DATA_PIN D5               //GPIO pin #, D5 = GPIO-14
#define COLOR_ORDER GRB           //Color order
#define LED_TYPE    WS2812        //LED's type
#define DRD_TIMEOUT 1
#define DRD_ADDRESS 0
#define HEARTBEAT_INTERVAL 300000 // 5 Minutes

char a_k[37];
char f_dID[25];
char brt[4];


uint64_t heartbeatTimestamp = 0;

bool isConnected = false;
bool shouldSaveConfig = false;
bool lightstate = false;

const int PIN_LED = 2;           // Onboard LED I/O pin 2 on ESP8266 12-E Module
int BRIGHTNESS = 255;
int r = 255;
int g = 255;
int b = 255;

CRGBArray<NUM_LEDS> leds;
//CRGB leds[NUM_LEDS];
WebSocketsClient webSocket;
Ticker flipper;
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

void setPowerStateOnServer(String deviceId, String value);

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

        // For Light device type
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html
        // {"deviceId": xxxx, "action": "AdjustBrightness", value: 3} // https://developer.amazon.com/docs/device-apis/alexa-brightnesscontroller.html
        // {"deviceId": xxxx, "action": "setBrightness", value: 42} // https://developer.amazon.com/docs/device-apis/alexa-brightnesscontroller.html
        // {"deviceId": xxxx, "action": "SetColor", value: {"hue": 350.5,  "saturation": 0.7138, "brightness": 0.6501}} // https://developer.amazon.com/docs/device-apis/alexa-colorcontroller.html
        // {"deviceId": xxxx, "action": "DecreaseColorTemperature"} // https://developer.amazon.com/docs/device-apis/alexa-colortemperaturecontroller.html
        // {"deviceId": xxxx, "action": "IncreaseColorTemperature"} // https://developer.amazon.com/docs/device-apis/alexa-colortemperaturecontroller.html
        // {"deviceId": xxxx, "action": "SetColorTemperature", value: 2200} // https://developer.amazon.com/docs/device-apis/alexa-colortemperaturecontroller.html

        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
        String deviceId = json ["deviceId"];
        String action = json ["action"];
        if (deviceId == (f_dID)) {
          if (action == "setPowerState") {
            String value = json ["value"];
            if (value == "ON") {
              fill_solid( leds, NUM_LEDS, CRGB(r, g, b));
              lightstate = true;
            } else {
              FastLED.clear();
              lightstate = false;
            }
            FastLED.show();
          }
          else if (action == "SetBrightness") {
            String brightness = json ["value"];
            brightness.toCharArray(brt, sizeof(brt));
            BRIGHTNESS = atoi(brt);
            FastLED.setBrightness(BRIGHTNESS * 2.55);
            if (lightstate == true) {
              setPowerStateOnServer(f_dID, "ON");
              FastLED.show();
            }
          }
          else if (action == "SetColorTemperature") {
            String colorTemperature = json ["value"];
            if (colorTemperature == "2200") {
              r = 255, g = 146, b = 39;
            }
            else if (colorTemperature == "2700") {
              r = 255, g = 167, b = 87;
            }
            else if (colorTemperature == "4000") {
              r = 255, g = 206, b = 166;
            }
            else if (colorTemperature == "5500") {
              r = 255, g = 237, b = 222;
            }
            else if (colorTemperature == "7000") {
              r = 243, g = 242, b = 255;
            }
            if (lightstate == true) {
              fill_solid( leds, NUM_LEDS, CRGB(r, g, b));
              FastLED.show();
            }
          }
          else if (action == "SetColor") {
            String hue = json ["value"]["hue"];
            String saturation = json ["value"]["saturation"];
            String brightness = json ["value"]["brightness"];
            if (hue == "0" && saturation == "1" && brightness == "1") {
              r = 255, g = 0, b = 0;
            }
            else if (hue == "348" && saturation == "0.9087" && brightness == "0.8588") {
              r = 165, g = 14, b = 44;
            }
            else if (hue == "17" && saturation == "0.52" && brightness == "1") {
              r = 119, g = 37, b = 9;
            }
            else if (hue == "39" && saturation == "1" && brightness == "1") {
              r = 255, g = 69, b = 0;
            }
            else if (hue == "50" && saturation == "1" && brightness == "1") {
              r = 255, g = 215, b = 0;
            }
            else if (hue == "60" && saturation == "1" && brightness == "1") {
              r = 255, g = 255, b = 0;
            }
            else if (hue == "120" && saturation == "1" && brightness == "1") {
              r = 0, g = 128, b = 0;
            }
            else if (hue == "174" && saturation == "0.7188" && brightness == "0.8784") {
              r = 72, g = 209, b = 204;
            }
            else if (hue == "180" && saturation == "1" && brightness == "1") {
              r = 0, g = 255, b = 255;
            }
            else if (hue == "197" && saturation == "0.4231" && brightness == "0.9176") {
              r = 0, g = 191, b = 255;
            }
            else if (hue == "240" && saturation == "1" && brightness == "1") {
              r = 0, g = 0, b = 139;
            }
            else if (hue == "277" && saturation == "0.8619" && brightness == "0.9373") {
              r = 75, g = 0, b = 130;
            }
            else if (hue == "300" && saturation == "1" && brightness == "1") {
              r = 139, g = 0, b = 139;
            }
            else if (hue == "348" && saturation == "0.251" && brightness == "1") {
              r = 255, g = 20, b = 147;
            }
            else if (hue == "255" && saturation == "0.5" && brightness == "1") {
              r = 230, g = 230, b = 250;
            }
            if (lightstate == true) {
              setPowerStateOnServer(f_dID, "ON");
              fill_solid( leds, NUM_LEDS, CRGB(r, g, b));
              FastLED.show();
            }
          } else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
          }
        }
        else {
          Serial.print("Unknown Device: ");
          Serial.println(deviceId);
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}

void setPowerStateOnServer(String deviceId, String value) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["deviceId"] = deviceId;
  root["action"] = "setPowerState";
  root["value"] = value;
  StreamString databuf;
  root.printTo(databuf);

  webSocket.sendTXT(databuf);
}

void flip() {
  int state = digitalRead(PIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(PIN_LED, !state);     // set pin to the opposite state
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(PIN_LED, OUTPUT);
  delay(10);
  digitalWrite(PIN_LED, LOW);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

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

      // encode known networks to JSON
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
}
