#ifndef UPATEOVERHTTP_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define UPATEOVERHTTP_H
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

/* 
serverURL is the hostname
serverPort the portnumber
uploadScript is the path and filename of the uploadscript on the server
version is an optional parameter that can give the current version
*/  
String updateOverHTTP(WiFiClient wifiClient, String serverURL, uint16_t serverPort, String uploadScript, String version);

#endif
