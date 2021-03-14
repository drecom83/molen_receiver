#include "handleHTTPClient.h"
#include "base64.h"

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

void sendDataToTarget(asyncHTTPrequest* pRequest, WiFiClient wifiClient, Settings * pSettings, String macAddress)
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
  if (pRequest->readyState() == 0 || pRequest->readyState() == 4)
  {
    pRequest->open("POST", url.c_str());
    pRequest->setReqHeader("Content-Type", "application/json");
    pRequest->setReqHeader("Cache-Control", "no-cache");
    pRequest->setReqHeader("Connection", "keep-alive");
    pRequest->setReqHeader("Pragma", "no-cache");
    pRequest->setReqHeader("WWW-Authenticate", "Basic realm=\"model\", charset=\"UTF-8\"");
    pRequest->setReqHeader("Authorization", auth.c_str());
    pRequest->send(post_data);
  }
}

String getAsyncResponse(asyncHTTPrequest* pRequest) {
  if (pRequest->readyState() == 4)
  {
    if (pRequest->responseHTTPcode() == 200)
    {
      return pRequest->responseText();;
    }
  }
  return "";
}
