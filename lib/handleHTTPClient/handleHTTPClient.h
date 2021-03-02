#ifndef HANDLEHTTPCLIENT_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define HANDLEHTTPCLIENT_H
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <ESP8266HTTPClient.h> 
#include "settings.h"
#include "asyncHTTPrequest.h"

/* send data to target server using ESP8266HTTPClient */
void handleHTTPClient(asyncHTTPrequest aRequest, WiFiClient wifiClient, Settings * pSettings, String macAddress);
/* contains the html that is send to the targetServer */
String getSendData(Settings * pSettings, String macddress);
/* sends data to the targetServer */
void sendDataToTarget(asyncHTTPrequest aRequest, WiFiClient wifiClient, Settings * pSettings, String macAddress);

#endif