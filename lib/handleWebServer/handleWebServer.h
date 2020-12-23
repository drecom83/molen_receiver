#ifndef HANDLEWEBSERVER_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define HANDLEWEBSERVER_H
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include "settings.h"
#include "WiFiSettings.h"

/* show page to see the counter */
void countPage(ESP8266WebServer &server, Settings * pSettings);
void countPage_nl(ESP8266WebServer &server, Settings * pSettings);
/* get and handle arguments on the settings-page */
void arguments(ESP8266WebServer &server, Settings * pSettings);
void arguments_nl(ESP8266WebServer &server, Settings * pSettings);
/* show the help page */
void help(ESP8266WebServer &server, Settings * pSettings);
void help_nl(ESP8266WebServer &server, Settings * pSettings);
/* show WiFi mode */
void showWiFiMode(ESP8266WebServer &server, Settings * pSettings);
void showWiFiMode_nl(ESP8266WebServer &server, Settings * pSettings);
/* show saved setting values from EEPROM */
void showSavedSettings(ESP8266WebServer &server, Settings * pSettings);
void showSavedSettings_nl(ESP8266WebServer &server, Settings * pSettings);
/* choose settings for the device and the target server */
void device(ESP8266WebServer &server, Settings * pSettings);
void device_nl(ESP8266WebServer &server, Settings * pSettings);
/* choose wifi connection, (Access Point or Station -todo: get SSID for Station-) */
void wifi(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWifiSettings);
void wifi_nl(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWifiSettings);
/* sending data through sse */
// not language dependent
void sse(ESP8266WebServer &server, Settings * pSettings, uint32_t revolutions, uint32_t viewPulsesPerMinute);

#endif