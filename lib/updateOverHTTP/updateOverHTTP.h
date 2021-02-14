#ifndef UPATEOVERHTTP_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define UPATEOVERHTTP_H
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

const String UPDATEOVERHTTP_FAILED ="[update] Update Failed";
const String UPDATEOVERHTTP_NO_UPDATE = "[update] No Update";
const String UPDATEOVERHTTP_OK = "[update] Update ok";
const String UPDATEOVERHTTP_NO_INTERNET = "[update] No connection with the server";

/* 
serverURL is the hostname
serverPort the portnumber
uploadScript is the path and filename of the uploadscript on the server
version is an optional parameter that can give the current version
*/  
String updateOverHTTP(WiFiClient wifiClient, String serverURL, uint16_t serverPort, String uploadScript, String version);

#endif
