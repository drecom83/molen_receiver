#include "handleHTTPClient.h"

long lastSendMillis = millis();                // part of the period for sending data to the target server
HTTPClient httpClient;    //Declare object of class HTTPClient


/* send data to target server using ESP8266HTTPClient */
String handleHTTPClient(WiFiClient wifiClient, Settings * pSettings, String macAddress)
  {
    String response = "";
    long currentMillis = millis();

    // send data to the target server to check macaddress and devicekey of client
    if (currentMillis - lastSendMillis > pSettings->getSEND_PERIOD())
    {
      response = sendDataToTarget(wifiClient, pSettings, macAddress);
      lastSendMillis = currentMillis;
      Serial.println(response);
      return response;
    }
    return "";
  }

// start client to send data to the server (to check autorisation)
String getSendData(Settings * pSettings, String macAddress) {
  String result = "{";
  result += "\"data\": {";
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

String sendDataToTarget(WiFiClient wifiClient, Settings * pSettings, String macAddress)
{
  //String targetServer = "10.0.0.51";
  //uint16_t port = 8085;
  //String path = "/";
  String targetServer = pSettings->getTargetServer();
  uint16_t port =  pSettings->getTargetPort();
  String path =  pSettings->getTargetPath();
  String url = targetServer + ":" + String(port) + path;
  //httpClient.begin("http://192.168.1.88:8085/hello");      //Specify request destination
  httpClient.begin(wifiClient, url);      //Specify request destination
  //httpClient.begin(wifiClient, "http://10.0.0.10:9090/feed/");      //Specify request destination

  httpClient.addHeader("Content-Type", "application/json");  //Specify content-type header
  httpClient.addHeader("Cache-Control", "no-cache");
  httpClient.addHeader("Connection", "keep-alive");
  httpClient.addHeader("Pragma", "no-cache");
 
  //httpClient.setReuse(true);

  String post = getSendData(pSettings, macAddress);
  httpClient.POST(post);   //Send the request
  //int httpCode = httpClient.POST(post);   //Send the request
  String payload = httpClient.getString();                  //Get the response payload

  // TODO: For authentication/authorisation
  // TODO: Get the uuid(=deviceKey) from the payload and if it is different than
  // TODO: the current uuid(=deviceKey) then save the new deviceKey
  // TODO: The server determines is a deviceKey is valid

  //Serial.println(url);
  //Serial.println(post);
  //Serial.println(httpCode);   //Print HTTP return code

  httpClient.end();  //Close connection
  return payload;
}
// end client