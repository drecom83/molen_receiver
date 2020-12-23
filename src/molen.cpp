#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal

//#include <ESP8266mDNS.h>
#include "handlemDNS.h"
#include <WiFiUdp.h>

#include "updateOverHTTP.h"

#include "settings.h"
#include "handleWebServer.h"
#include "handleHTTPClient.h"
#include "WiFiSettings.h"

// WIFI URL: http://192.168.4.1/ or http://molen.local/
/////////////////////
// Pin Definitions //
/////////////////////
// On a ESP8266-12 GPIO0 is used, physical name is pin D0
// On a ESP8266-12 GPIO5 is used, physical name is pin D5

// when using D0 only one direction of receiver works, so now using D8

// D5 gives troubles when it is high at the start.

const uint8_t IR_RECEIVE_1 = D5;    // Digital pin to read an incoming signal
const uint8_t IR_RECEIVE_2 = D6;    // Digital pin to read an incoming signal

const uint8_t IR_SEND = D8;         // switch for IR send LED. 0 = off, 1 = on

const uint8_t BUTTON = D7;          // Digital pin to read button-push
const uint8_t BLUE_LED = D4;
const uint8_t YELLOW_1_LED = D3;
const uint8_t YELLOW_2_LED = D2;
const uint8_t YELLOW_3_LED= D1;

const uint32_t RELAX_PERIOD = 2;    // Is also a small energy saving, in milliseconds
const uint32_t TOO_LONG = 60000;    // after this period the pulsesPerMinute = 0 (in milliseconds)
bool permissionToDetect = false;    // all sensors must have had a positive value 

uint32_t startPulse = millis();     // set the offset time for a passing a pulse
uint32_t pulsesPerMinute = 0;       // holds the value of pulses per minute
uint32_t revolutions = 0;           // holds the value of revolutions of the first axis, calculated with ratio
uint32_t viewPulsesPerMinute = 0;   // holds the value of ends per minute calculated with ratio

Settings settings = Settings();
Settings* pSettings = &settings;

//////////////////////
// WiFi Definitions //
//////////////////////
WiFiSettings wifiSettings = WiFiSettings(pSettings);
WiFiSettings* pWifiSettings = &wifiSettings;

// detectButtonFlag lets the program know that a network-toggle is going on
bool detectButtonFlag = false;

// Forward declaration
void setupWiFi();
void showSettings();
void switchToAccessPoint();
void handleShowWiFiMode();
void initServer();
void sendExample();

void ICACHE_RAM_ATTR detectPulse();
void echoInterruptOn();
void echoInterruptOff();

void ICACHE_RAM_ATTR detectButton();
void buttonInterruptOn();
void buttonInterruptOff();

void toggleWiFi();
/*
2^8 = 256
2^16 = 65536
2^32 = 4294967296
2^64 = 18446744073709551616
*/

ESP8266WebServer server(80);
WiFiClient wifiClient;
//MDNSResponder mdns;

// start Settings and EEPROM stuff
void saveSettings() {
  pSettings->saveSettings();
  showSettings();
}

void getSettings() {
  pSettings->getSettings();
  showSettings();
}

void eraseSettings() {
  pSettings->eraseSettings();
  pSettings->getSettings();   // otherwise the previous values of Settings are used
  showSettings();
}

void initSettings() {
  pSettings->initSettings();
  pSettings->getSettings();   // otherwise the previous values of Settings are used
  showSettings();
}
// end Settings and EEPROM stuff

void setupWiFi(){
  echoInterruptOff();  // to prevent error with Delay
  digitalWrite(YELLOW_1_LED, LOW);
  digitalWrite(YELLOW_2_LED, HIGH);
  digitalWrite(YELLOW_3_LED, LOW);

  WiFi.mode(WIFI_AP);

  String myssid = pWifiSettings->readAccessPointSSID();
  if (myssid == "")
  {
    myssid = "ESP-" + WiFi.macAddress();
  }
  String mypass = pWifiSettings->readAccessPointPassword();

  IPAddress local_IP(192,168,4,1);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);


  Serial.print("Setting soft-AP ... ");
  // mypass needs minimum of 8 characters
  Serial.println(WiFi.softAP(myssid,mypass,3,0) ? "Ready" : "Failed!");
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
  Serial.print("Connecting to AP mode");

  delay(500);
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  Serial.println(WiFi.softAPmacAddress());

  digitalWrite(YELLOW_1_LED, HIGH);
  digitalWrite(YELLOW_2_LED, LOW);
  
  pSettings->beginAsAccessPoint(true);
 
  echoInterruptOn();  // to prevent error with Delay

}

void setupWiFiManager () {
  bool networkConnected = false;
  echoInterruptOff();  // to prevent error with Delay

  digitalWrite(YELLOW_1_LED, LOW);
  digitalWrite(YELLOW_2_LED, HIGH);
  digitalWrite(YELLOW_3_LED, LOW);

  String mynetworkssid = pWifiSettings->readNetworkSSID();
  if (mynetworkssid != "") {
    String mynetworkpass = pWifiSettings->readNetworkPassword();
    WiFi.mode(WIFI_STA);
    WiFi.begin(mynetworkssid, mynetworkpass); 

    Serial.print("Connecting to a WiFi Network");
    int toomuch = 30;  //gives 30 seconds to connect to a Wifi network
    while ((WiFi.status() != WL_CONNECTED) && (toomuch > 0))
    {
      delay(1000);
      Serial.print(".");
      toomuch -=1;
    }
    if (toomuch > 0) {
      Serial.println();

      Serial.print("Connected, IP address: ");
      Serial.println("local ip address");
      Serial.println(WiFi.localIP());
      Serial.println(WiFi.gatewayIP());
      Serial.println(WiFi.macAddress());
    
      echoInterruptOn();  // to prevent error with Delay
      networkConnected = true;
      pSettings->setLastNetworkIP(WiFi.localIP().toString());

      digitalWrite(YELLOW_2_LED, LOW);
      digitalWrite(YELLOW_3_LED, HIGH);
      pSettings->beginAsAccessPoint(false);
    }
  }
  if (networkConnected == false) {
    // no network found, fall back to access point
    Serial.println("no network SSID found/selected, fall back to AccessPoint mode");
    switchToAccessPoint();
  }
}

void resetWiFiManagerToFactoryDefaults () {
  echoInterruptOff();  // to prevent error with Delay

  // WiFi.disconnect(true);  // true argument should also erase ssid and password
  // https://www.pieterverhees.nl/sparklesagarbage/esp8266/130-difference-between-esp-reset-and-esp-restart
  Serial.println("try to disconnect works only when WiFi.begin() was successfully called");
  int toomuch = 2;
  while (toomuch > 0) {
    int getValue = WiFi.disconnect(true);
    if (getValue != 0) {
      toomuch = 0;
    }
    Serial.println(String(getValue));
    delay(3000);
    Serial.println("waited 3 seconds");
    toomuch -= 1;
  }

  echoInterruptOn();  // to prevent error with Delay
}

void switchToAccessPoint() {
  echoInterruptOff();  // to prevent error with Delay

  pSettings->beginAsAccessPoint(!  pSettings->beginAsAccessPoint());  // toggle
  handleShowWiFiMode();
  delay(pSettings->WAIT_PERIOD);

  server.close();
  resetWiFiManagerToFactoryDefaults();
  delay(pSettings->WAIT_PERIOD);

  setupWiFi();
  delay(pSettings->WAIT_PERIOD);

  initServer();

  // start domain name server check
  /*
    mdns.close();
    while (mdns.begin("molen", WiFi.softAPIP())) {
      Serial.println("MDNS responder started");
      mdns.addService("http", "tcp", 80);
    }
  */
  mDNSnotifyAPChange();
  //startmDNS();
  // end domain name server check

  echoInterruptOn();  // to prevent error with Delay
}

void switchToNetwork() {
  echoInterruptOff();  // to prevent error with Delay

  handleShowWiFiMode();
  delay(pSettings->WAIT_PERIOD);

  server.close();
  resetWiFiManagerToFactoryDefaults();
  delay(pSettings->WAIT_PERIOD);

  setupWiFiManager();
  delay(pSettings->WAIT_PERIOD);

  delay(pSettings->WAIT_PERIOD);
  initServer();

  /*
  mdns.close();
  while (mdns.begin("molen", WiFi.localIP())) {
    Serial.println("MDNS responder started");
    mdns.addService("http", "tcp", 80);
  }
  */
  mDNSnotifyAPChange();

  //startmDNS();

  echoInterruptOn();  // to prevent error with Delay
}

void writeResult(WiFiClient wifiClient, String result) {
  wifiClient.print(result);
  wifiClient.flush();
}

/* flashes PIN, unit is milliseconds (0-256) */
void flashPin(uint8_t pin, uint8_t ms) {
  digitalWrite(pin, HIGH);
  for (uint16_t i = 0; i <= ms; i++)
  {
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
  }
  digitalWrite(pin, LOW);
}

void checkGlobalPulseInLoop() {
  // sets value to 0 after a period of time
  uint32_t elapsedTime;
  if (millis() > startPulse) {  // check for overflow
    
    elapsedTime = millis() - startPulse;
    if (elapsedTime * pSettings->ratio > TOO_LONG) {
      pulsesPerMinute  = 0;
      viewPulsesPerMinute = 0;
    }   
  }
}

void setGlobalPulsesPerMinute() {
  /*
  start = millis();
  Returns the number of milliseconds since the Arduino board began
  running the current program. This number will overflow (go back to zero),
  after approximately 50 days.
  */
  // use previous value if an overflow of millis() occurs,
  // it does not have to be too precise
  uint32_t elapsedTime;
  if (millis() > startPulse) {  // check for overflow
    elapsedTime = millis() - startPulse;

    // measuremens shorter then the delay time are invalid (with an extra 50 ms to be sure)
    //if (elapsedTime > START_PERIOD) {
      if (elapsedTime > 0) {
        // get duration to get 1 pulse
        pulsesPerMinute = (uint32_t) round(60000 / elapsedTime);
      }
      else {
        pulsesPerMinute = 0;    // maybe slow movement, but rounded to 0
      }
   //}
    startPulse = millis();
  }
}

void delayInMillis(uint8_t ms)
{
  for (uint8_t i = 0; i <= ms; i++)
  {
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
  }
}

void ICACHE_RAM_ATTR detectButton() {  // ICACHE_RAM_ATTR is voor interrupts
  // this function is called after a change of pressed button  
  buttonInterruptOff();  // to prevent exception

  delayInMillis(10);      // prevent bounce
  
  if (digitalRead(BUTTON) == HIGH)
  {
    detectButtonFlag = true;
    // only toggle between AP and STA by using the button, not saving in EEPROM
  }
  buttonInterruptOn();  // to prevent exception
}

void buttonInterruptOn() {
  attachInterrupt(digitalPinToInterrupt(BUTTON), detectButton, CHANGE);
}

void buttonInterruptOff() {
  detachInterrupt(BUTTON);
}

void ICACHE_RAM_ATTR detectPulse() {  // ICACHE_RAM_ATTR is voor interrupts
  // this function is called after a change of every sensor-value
  // wait until both sensors are true, then permissionToDetect = true
  // if both sensors are false and permissionToDetect == true then it counts as a valid pulse
  // after a valid pulse the value of permissionToDetect is set to false to start over again
  echoInterruptOff();
    // for energy savings a delay is added of n milliseconds
  delayInMillis(RELAX_PERIOD);

  if ( (digitalRead(IR_RECEIVE_1) == true) && 
      (digitalRead(IR_RECEIVE_2) == true) &&
      (permissionToDetect == true) )
  {
    permissionToDetect = false;
    flashPin(BLUE_LED, 1);
  }

  if ( (digitalRead(IR_RECEIVE_1) == false) && 
      (digitalRead(IR_RECEIVE_2) == false) && 
      (permissionToDetect == false) )
  {
    permissionToDetect = true;  // start over again

    pSettings->setCounter(pSettings->getCounter() + 1); // added 1 pulse
    setGlobalPulsesPerMinute();
    // calculate with ratio
    if (pSettings->blades < 1) {
      pSettings->blades = 1;
    }

    viewPulsesPerMinute = round(pulsesPerMinute / pSettings->pulseFactor);

    revolutions = floor(pSettings->getCounter() / pSettings->pulseFactor);
  }
  echoInterruptOn();
}

void echoInterruptOn() {
  // 0 = ir_light, 1 is no ir_light
  attachInterrupt(IR_RECEIVE_1, detectPulse, CHANGE);
  attachInterrupt(IR_RECEIVE_2, detectPulse, CHANGE);
}

void echoInterruptOff() {
  detachInterrupt(IR_RECEIVE_1);
  detachInterrupt(IR_RECEIVE_2);
}

void handleCountPage() {
  if (pSettings->getLanguage() == "NL")
  {
    countPage_nl(server, pSettings);
  }
  else
  {
    countPage(server, pSettings);
  }
}

void handleShowWiFiMode()
{
  if (pSettings->getLanguage() == "NL")
  {
    showWiFiMode_nl(server, pSettings);
  }
  else
  {
    showWiFiMode(server, pSettings);
  }
}

void handleWiFi() {
  if (pSettings->getLanguage() == "NL")
  {
    wifi_nl(server, pSettings, pWifiSettings);
  }
  else
  {
    wifi(server, pSettings, pWifiSettings);
  }
}

void handleDevice() {
  if (pSettings->getLanguage() == "NL")
  {
    device_nl(server, pSettings);
  }
  else
  {
    device(server, pSettings);
  }
}

void handleSse() {
  sse(server, pSettings, revolutions, viewPulsesPerMinute);
}

void handleArguments() {
  if (pSettings->getLanguage() == "NL")
  {
    arguments_nl(server, pSettings);
  }
  else
  {
    arguments(server, pSettings);
  }
  showSettings();
}

void mydebug() {
  String result = "";
  String myIP = "";
  result += "IP address: ";
  if (WiFi.getMode() == WIFI_AP)
  {
    myIP = WiFi.softAPIP().toString();
  }
  if (WiFi.getMode() == WIFI_STA)
  {
    myIP = WiFi.localIP().toString();
  }

  result += myIP;
  result += "\r\n";

  Serial.println("wifi gegevens");
  Serial.print("readAccessPointSSID: ");
  Serial.println(pWifiSettings->readAccessPointSSID());
  Serial.print("readAccessPointPassword: ");
  Serial.println(pWifiSettings->readAccessPointPassword());
  Serial.print("readNetworkSSID: ");
  Serial.println(pWifiSettings->readNetworkSSID());
  Serial.print("readNetworkPassword: ");
  Serial.println(pWifiSettings->readNetworkPassword());

  Serial.print("Chip ID: ");
  Serial.println(ESP.getFlashChipId());
 
  Serial.print("Chip Real Size: ");
  Serial.println(ESP.getFlashChipRealSize());
 
  Serial.print("Chip Size: ");
  Serial.println(ESP.getFlashChipSize());
 
  Serial.print("Chip Speed: ");
  Serial.println(ESP.getFlashChipSpeed());
 
  Serial.print("Chip Mode: ");
  Serial.println(ESP.getFlashChipMode());

  Serial.print("firmware version: ");
  Serial.println(pSettings->getFirmwareVersion());

  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}

String updateFirmware()
{
  String serverUrl = pSettings->getTargetServer();
  uint16_t serverPort = pSettings->getTargetPort();
  String uploadScript = "/updateFirmware/";
  String version = pSettings->getFirmwareVersion();
  Serial.println(serverUrl);
  Serial.println(serverPort);
  Serial.println(uploadScript);
  Serial.println(version);
  String result = updateOverHTTP(wifiClient, serverUrl, serverPort, uploadScript, version);
  return result;
}

void handleVersion() {
  uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if (server.method() == HTTP_POST)
  {
    argumentCounter = server.args();
    String name = "";
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        name = server.arg(i);
      }
    }
    // search name 
    if (name == "update")
    {
      if (argumentCounter > 0)
      {
        result = updateFirmware();
      }
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    if (result.indexOf("failed") > -1)
    {
      result_nl = "[update] Update mislukt";
    }
    if (result.indexOf("no Update") > -1)
    {
      result_nl = "[update] Geen update aanwezig";
    }
    if (result.indexOf("ok") > -1)
    {
      result_nl = "[update] Update ok";
    }
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Connection", "keep-alive");
    server.sendHeader("Pragma", "no-cache");
    server.send(200, "text/html", result);
  }
  else
  {
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Connection", "keep-alive");
    server.sendHeader("Pragma", "no-cache");
    server.send(200, "text/html", result);
  }
}

/* void alive must be used in clients only
but for now, in develop-phase it is allowed here
TODO remove this when clients are available to test
*/
void alive() {


  String firstFreeHostname = findFirstFreeHostname();



  /* used to answer a xhr call from the browser that is connected to the server */
  String result = "";
  /*
  String myIP = "";
  result += "IP address: ";
  if (WiFi.getMode() == WIFI_AP)
  {
    myIP = WiFi.softAPIP().toString();
  }
  if (WiFi.getMode() == WIFI_STA)
  {
    myIP = WiFi.localIP().toString();
  }
  result += myIP;
  */
  result += firstFreeHostname;
  result += "\r\n";
  Serial.println(result);
  String allowServer = pSettings->getTargetServer() + ":" + pSettings->getTargetPort();
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Access-Control-Allow-Origin", allowServer);
  server.send(200, "text/html", result);
}

void showSettings() {
  if (pSettings->getLanguage() == "NL")
  {
    showSavedSettings_nl(server, pSettings);
  }
  else
  {
    showSavedSettings(server, pSettings);
  }
}

void handleHelp() {
  if (pSettings->getLanguage() == "NL")
  {
    help_nl(server, pSettings);
  }
  else
  {
    help(server, pSettings);
  }
}

void handleLanguage() {
  uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if (server.method() == HTTP_POST)
  {
    argumentCounter = server.args();  // if argumentCounter > 0 then save
    String name = "";
    String language = "";
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        name = server.arg(i);
      }
      if (server.argName(i) == "language") {
        language = server.arg(i);
      }
    }
    // search name 
    if (name == "help")
    {
      if (argumentCounter > 0)
      {
        pSettings->setLanguage(language);
      }
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    server.send(200, "text/plain", result_nl);
  }
  else
  {
    server.send(200, "text/plain", result);
  }
}

void handleNetworkSSID() {
  // creates a list of {ssid, including input field , dBm}
  String result = "";
  int numberOfNetworks = WiFi.scanNetworks();
  for(int i =0; i<numberOfNetworks; i++){ 

    if (i > 0) {
      result += ",";
    }
    result += "{ssid:";
    result += "'<span><input type=\"radio\" name=\"networkSSID\" onclick=\"selectNetworkSSID(this)\" value=\"";
    result += WiFi.SSID(i);
    result += "\">";
    result += WiFi.SSID(i);
    result += "</span>";
    result += "',";
    result += "dBm:'";
    result += WiFi.RSSI(i);
    result += "'}";
  }
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}

void handleWifiConnect() {
  uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if (server.method() == HTTP_POST)
  {
    argumentCounter = server.args();  // if argumentCounter > 0 then save
    String name = "";
    String ssid = "";
    String password = "";
    String target = "";               // for action Erase
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        name = server.arg(i);
      }
      if (server.argName(i) == "ssid") {
        ssid = server.arg(i);
      }
      if (server.argName(i) == "password") {
        password = server.arg(i);
      }
      if (server.argName(i) == "target") {
        target = server.arg(i);
      }
    }
    // zoek name (is ap of network en dan de ssid en password)
    if (name == "ap")
    {
      pWifiSettings->setAccessPointSSID(ssid);
      pWifiSettings->setAccessPointPassword(password);
      if (argumentCounter > 0) {
        pWifiSettings->saveAuthorizationAccessPoint();
        result += "Access Point data has been saved\n";
      }
    }
    if (name == "network")
    {
      pWifiSettings->setNetworkSSID(ssid);
      pWifiSettings->setNetworkPassword(password);
      if (argumentCounter > 0) {
        pWifiSettings->saveAuthorizationNetwork();
        result += "Network connection data has been saved\n";
        result_nl += "Netwerk verbindingsgegevens zijn opgeslagen\n";
      }
    }
    if (name == "erase")
    {
      if (argumentCounter > 0) {
        if (target == "eraseAPData")
        {
          pWifiSettings->eraseAccessPointSettings();
          result += "Access Point data has been erased\n";
          result_nl += "Access Point gegevens zijn gewist\n";
        }
        if (target == "eraseNetworkData")
        {
          pWifiSettings->eraseNetworkSettings();
          result += "Network connection data has been erased\n";
          result_nl += "Netwerk verbindingsgegevens zijn gewist\n";
        }
        if (target == "eraseWiFiData")
        {
          pWifiSettings->eraseWiFiSettings();
          result += "Access Point data and Network connection data has been erased\n";
          result_nl += "Access Point gegevens en Netwerk verbindingsgegevens zijn gewist\n";
        }
      }
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    server.send(200, "text/plain", result_nl);
  }
  else
  {
    server.send(200, "text/plain", result);
  }
  Serial.println(result);
}

void handleDeviceSettings()
{
  uint8_t argumentCounter = 0;
  String result = "";
  String result_nl = "";

  if (server.method() == HTTP_POST)
  {
    // extract the settings-data and take action
    argumentCounter = server.args();  // if argumentCounter > 0 then saveConfigurationSettings
    String _name = "";
    String _startWiFiMode = "";
    String _counter = "";
    String _ratio = "";
    String _targetServer = "";
    String _targetPort = "";
    String _targetPath = "";
    String _allowSendingData = "";
    String _isOpen = "";
    String _showData = "";
    String _message = "";
    for (uint8_t i=0; i< server.args(); i++){
      if (server.argName(i) == "name") {
        _name = server.arg(i);
      }
      if (server.argName(i) == "startWiFiMode") {
        _startWiFiMode = server.arg(i);
      }
      if (server.argName(i) == "counter") {
        _counter = server.arg(i);
      }
      if (server.argName(i) == "ratio") {
        _ratio = server.arg(i);
      }
      if (server.argName(i) == "targetServer") {
        _targetServer = server.arg(i);
      }
      if (server.argName(i) == "targetPort") {
        _targetPort = server.arg(i);
      }
      if (server.argName(i) == "targetPath") {
        _targetPath = server.arg(i);
      }
      if (server.argName(i) == "allowSendingData") {
        _allowSendingData = server.arg(i);
      }
      if (server.argName(i) == "isOpen") {
        _isOpen = server.arg(i);
      }
      if (server.argName(i) == "showData") {
        _showData = server.arg(i);
      }
      if (server.argName(i) == "message") {
        _message = server.arg(i);
      }
    }
    // zoek name (is device, targetServer of targetserverData en dan de andere parameters)
    if (_name == "device")
    {
      if (_startWiFiMode == "ap") {
        pSettings->beginAsAccessPoint(true);
      }
      if (_startWiFiMode == "network") {
        pSettings->beginAsAccessPoint(false);
      }
      pSettings->setCounter(_counter);
      pSettings->setRatioArgument(_ratio);
    }
    if (_name == "targetServer")
    {
      pSettings->setTargetServer(_targetServer);
      pSettings->setTargetPort(_targetPort);
      pSettings->setTargetPath(_targetPath);
    }
    if (_name == "targetServerData")
    {
      pSettings->setAllowSendData(_allowSendingData);
      pSettings->setEntree(_isOpen);
      pSettings->setShowData(_showData);
      pSettings->setTargetServerMessage(_message);  // message will not be saved in EEPROM
    }
    if (argumentCounter > 0) {
      pSettings->saveConfigurationSettings();
      result += "Device data has been saved\n";
      result_nl += "Apparaatgegevens zijn opgeslagen\n";
    }
  }
  if (pSettings->getLanguage() == "NL")
  {
    server.send(200, "text/plain", result_nl);
  }
  else
  {
    server.send(200, "text/plain", result);
  }
  Serial.println(result);
}

void toggleWiFi()
{
  // only toggle by using the button, not saving in EEPROM
  pSettings->beginAsAccessPoint(!  pSettings->beginAsAccessPoint());  // toggle
  if (pSettings->beginAsAccessPoint() == true)
  {
    //switchToAccessPoint();
    setupWiFi();        // local network as access point
  }
  else
  {
    //switchToNetwork();
    setupWiFiManager();   // part of local network as station
  }
}

void initHardware()
{
  Serial.begin(115200);
  pinMode(IR_SEND, OUTPUT);      // default LOW
  pinMode(IR_RECEIVE_1, INPUT);  // default down
  pinMode(IR_RECEIVE_2, INPUT);  // default down

  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_1_LED, OUTPUT);
  pinMode(YELLOW_2_LED, OUTPUT);
  pinMode(YELLOW_3_LED, OUTPUT);
}

void initServer()
{
  server.close();
  // start webserver

  server.on("/help/", handleHelp);
  server.on("/count/", handleCountPage);

  // handles notFound
  server.onNotFound(handleHelp);

  // interactive pages
  server.on("/device/", handleDevice);
  server.on("/wifi/", handleWiFi);
  // handles input from interactive pages
  server.on("/networkssid/", handleNetworkSSID);
  server.on("/wifiConnect/", handleWifiConnect);
  server.on("/deviceSettings/", handleDeviceSettings);
  server.on("/language/", handleLanguage);

  // data handler
  server.on("/data.sse/", handleSse);

  // url-commands, not used in normal circumstances
  server.on("/ap/", switchToAccessPoint);
  server.on("/network/", switchToNetwork);
  server.on("/settings/", handleArguments);
  server.on("/eraseSettings/", eraseSettings);
  server.on("/initSettings/", initSettings);
  server.on("/getSettings/", getSettings);
  server.on("/saveSettings/", saveSettings);
  server.on("/reset/", resetWiFiManagerToFactoryDefaults);
  server.on("/update/", handleVersion);

  // handles debug
  server.on("/debug/", mydebug);

  // handles a check if this url is available
  // remove this when clients are availabe
  server.on("/alive/", alive);

  server.begin();
  Serial.println("HTTP server started");
}

void setup()
{
  /* It seems to help preventing ESPerror messages with mode(3,6) when
  using a delay */
  initHardware();
  digitalWrite(IR_RECEIVE_1, LOW);
  digitalWrite(IR_RECEIVE_2, LOW);

  delay(pSettings->WAIT_PERIOD);

  // see https://forum.arduino.cc/index.php?topic=121654.0 voor circuit brownout
  delay(pSettings->WAIT_PERIOD);
  // use EITHER setupWiFi OR setupWiFiManager
  
  if (pSettings->beginAsAccessPoint())
  {
    setupWiFi();        // local network as access point
  }
  else
  {
    setupWiFiManager();   // part of local network as station
  }

  delay(pSettings->WAIT_PERIOD);
  startmDNS();

  delay(pSettings->WAIT_PERIOD);

  initServer();
  delay(pSettings->WAIT_PERIOD);

  echoInterruptOn();

  buttonInterruptOn();
  digitalWrite(IR_SEND, HIGH);
}

void loop()
{
  // update should be run on every loop
  //mdns.update();
  MDNS.update();

  if (detectButtonFlag == true)
  {
    toggleWiFi();   // only toggle between AP and STA by using the button, not saving in EEPROM
    detectButtonFlag = false;
  }

  // For ESP8266WebServer
  server.handleClient();
  
  // For handleHTTPClient
  if ((WiFi.getMode() == WIFI_STA) && (pSettings->allowSendingData() == true))
  {
    /* send data to target server using ESP8266HTTPClient */
    handleHTTPClient(wifiClient, pSettings, String(WiFi.macAddress()), revolutions, viewPulsesPerMinute);
  }

  checkGlobalPulseInLoop();
}
