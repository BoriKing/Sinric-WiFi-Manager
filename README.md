# Sinric-WiFi-Manager Smart Devices(ESP-8266) for Alexa/Echo.

![gma](https://user-images.githubusercontent.com/24806817/36453051-b46efd5c-1664-11e8-8ced-872b60ffcffd.png)


**_ESP-8266 pinouts:_**

![test](https://user-images.githubusercontent.com/24806817/36623928-8f3ad4aa-18d7-11e8-9c35-eabf1195a8b8.png)


**_Features:_**

- Configure WiFi Network(s), Sinric API key and Device(s) ID(s) via WiFi Manager.
- Ability to reset credentials by resetting device Twice in less than 2 seconds.
- Onboard Blinking LED feedback while in Sinric Wifi Manager configuration or while connecting to Network. 
- Single, Multi Devices and Multi Networks examples.
- Examples with Physical ON/OFF or without.

**_Examples for:_**
- Lights
- Switches
- RGB-LEDs
- TV **_(this Example can be used for any IR device(s))_**
- Thermostad
- RFID Door Lock

**_Needed libraries:_**
- [WebSocketsClient](https://github.com/Links2004/arduinoWebSockets/releases)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) or install via Arduino library manager.
- [DNSServer](https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/src) or install via Arduino library manager.
- [WiFiManager](https://github.com/tzapu/WiFiManager) for **_Single Network_** or install via Arduino library manager or [WiFiManager](https://github.com/the-real-orca/WiFiManager) for **_Multi Networks_**.
- [DoubleResetDetector](https://github.com/datacute/DoubleResetDetector/tree/master/src) or install via Arduino library manager.
- [Ticker](https://github.com/esp8266/Arduino/tree/master/libraries/Ticker) or install via Arduino library manager.
- [ESP8266WebServer](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/src) or install via Arduino library manager.
- [IRremoteESP8266](https://github.com/markszabo/IRremoteESP8266) or install via Arduino library manager. **_(For TV Example only)_**
- [FastLED](https://github.com/FastLED/FastLED) **_(For RGB-LEDs and RFID Door Lock Example only)_**
- [MFRC522](https://github.com/miguelbalboa/rfid) **_(For RFID Door Lock Example only)_**


**_How to use:_**

**_1)_** Register for an account if you do not have one @ [**_sinric.com_**](https://sinric.com/login?returnUrl=%2F).

**_2)_** Login and create a smart home device(s).

**_3)_** Download(copy) the sketch example, flash and **_restart_** device after flashing. **_(if you have previously used WiFi Manager, [reset credentials](https://github.com/BoriKing/Sinric-WiFi-Manager/blob/master/Credentials%20Reset/CredentialsReset.ino) before Flashing sketch)_**.

**_4)_** Copy your API Key and Device(s) ID(s).

**_5)_** Connect to Sinric AP. After connecting, open your browser and go to(usually): **_http://192.168.4.1/_**

**_6)_** Paste/type your API Key and Device(s) ID(s) into Sinric WiFi Manager.

**_7)_** Enable/link [**_Sinric_**](https://www.amazon.com/dp/B078RGYWQQ) Alexa skill.

**_8)_** Discover Device(s).

**_9)_** Enjoy!


### This project is based on [kakopappa's](https://github.com/kakopappa/sinric) repo.
