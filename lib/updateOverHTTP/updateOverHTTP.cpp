#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <ESP8266httpUpdate.h>
#include "updateOverHTTP.h"

String updateOverHTTP(WiFiClient wifiClient, String serverURL, uint16_t serverPort, String uploadScript, String version) {
  String result = "";
  if (WiFi.getMode() == STATION_MODE)
  {
    uint8_t pos = serverURL.indexOf("://");
    if (pos > -1)
    {
      serverURL = serverURL.substring(pos + 3);  // get rid of protocol
    }
    ESPhttpUpdate.rebootOnUpdate(true);
    t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, serverURL, serverPort, uploadScript, version);
    switch(ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            result += "[update] Update failed";
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("[update] Update no Update");
            result += "[update] Update no Update";
            break;
        case HTTP_UPDATE_OK:
            Serial.println("[update] Update ok"); // may not be called since we reboot the ESP
            result += "[update] Update ok";
            break;
    }
  }
  else {
    Serial.println("Device must be in station mode");
    result += "Device is not connected to internet\r\n";
  }
  return result;
}

