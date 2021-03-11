#include "handleHTTPClient.h"
#include "base64.h"


long lastSendMillis = millis();                // part of the period for sending data to the target server


/* send data to target server using ESP8266HTTPClient */
void handleHTTPClient(asyncHTTPrequest aRequest, WiFiClient wifiClient, Settings * pSettings, String macAddress)
  {
    long currentMillis = millis();

    // send data to the target server to check macaddress and devicekey of client
    if (currentMillis - lastSendMillis > pSettings->getSEND_PERIOD())
    {
      sendDataToTarget(aRequest, wifiClient, pSettings, macAddress);
      lastSendMillis = currentMillis;
    }
  }


// start client to send data to the server
String getSendData(Settings * pSettings, String macAddress) {
  String result = "{";
  result += "\"data\": {";
  result += "\"firmwareVersion\":";
  result += "\"";
  result += pSettings->getFirmwareVersion();
  result += "\",";
  result += "\"deviceKey\":";
  result += "\"";
  result += pSettings->getDeviceKey();
  result += "\",";
  result += "\"macAddress\":";
  result += "\"";
  result += macAddress;
  result += "\",";
  result += "\"roleModel\":";
  result += "\"";
  result += pSettings->getRoleModel();
  result += "\"";
  result += "}";
  result += "}\n";  // for the writestream
  return result;
}



void sendDataToTarget(asyncHTTPrequest aRequest, WiFiClient wifiClient, Settings * pSettings, String macAddress)
{
  String targetServer = pSettings->getTargetServer();
  uint16_t port =  pSettings->getTargetPort();
  String path =  pSettings->getTargetPath();
  String url = targetServer + ":" + String(port) + path;
 
  // Note: BasicAuthentication does not allow any colon characters
  //       replace them with an underscore
  String key = macAddress;
  key.replace(":", "_");
  // Note: String end with \n character that has to be removed in the header
  String auth = "Basic " + base64::encode(key + ":" + pSettings->getDeviceKey());
  auth.replace("\n","");

  String post_data = getSendData(pSettings, macAddress);
  if (aRequest.readyState() == 0 || aRequest.readyState() == 4)
  {
    aRequest.open("POST", url.c_str());
    aRequest.setReqHeader("Content-Type", "application/json");
    aRequest.setReqHeader("Cache-Control", "no-cache");
    aRequest.setReqHeader("Connection", "keep-alive");
    aRequest.setReqHeader("Pragma", "no-cache");
    aRequest.setReqHeader("WWW-Authenticate", "Basic realm=\"model\", charset=\"UTF-8\"");
    aRequest.setReqHeader("Authorization", auth.c_str());
    aRequest.send(post_data);
    delay(10);  // prevents a reboot
  }
}
