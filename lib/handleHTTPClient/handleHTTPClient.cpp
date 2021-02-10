#include "handleHTTPClient.h"
#include "base64.h"

long lastSendMillis = millis();                // part of the period for sending data to the target server
//HTTPClient httpClient;    //Declare object of class HTTPClient
//String response = "";

/* send data to target server using ESP8266HTTPClient */
void handleHTTPClient(asyncHTTPrequest* pRequest, WiFiClient wifiClient, Settings * pSettings, String macAddress)
  {
    long currentMillis = millis();

    // send data to the target server to check macaddress and devicekey of client
    if (currentMillis - lastSendMillis > pSettings->getSEND_PERIOD())
    {
      sendDataToTarget(pRequest, wifiClient, pSettings, macAddress);
      lastSendMillis = currentMillis;
    }
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


void sendDataToTarget(asyncHTTPrequest* pRequest, WiFiClient wifiClient, Settings * pSettings, String macAddress)
{
  //String targetServer = "10.0.0.51";
  //uint16_t port = 8085;
  //String path = "/";
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


  if (pRequest->readyState() == 0 || pRequest->readyState() == 4)
  {
    pRequest->open("POST", url.c_str());
    pRequest->setReqHeader("Content-Type", "application/json");
    pRequest->setReqHeader("Cache-Control", "no-cache");
    pRequest->setReqHeader("Connection", "keep-alive");
    pRequest->setReqHeader("Pragma", "no-cache");
    pRequest->setReqHeader("Authorization", auth.c_str());
    pRequest->send(post_data);
  }


/*
  httpClient.begin(wifiClient, url);      //Specify request destination

  httpClient.addHeader("Content-Type", "application/json");  //Specify content-type header
  httpClient.addHeader("Cache-Control", "no-cache");
  httpClient.addHeader("Connection", "keep-alive");
  httpClient.addHeader("Pragma", "no-cache");
 
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
*/
}
// end client