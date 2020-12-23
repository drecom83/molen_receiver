#ifndef WIFISETTINGS_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define WIFISETTINGS_H
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <EEPROM.h>               // used to store and read settings
#include "settings.h"

class WiFiSettings
{
private:
  /* Maximum size of EEPROM, SPI_FLASH_SEC_SIZE comes from spi_flash.h */
  const uint16_t MAX_EEPROM_SIZE = SPI_FLASH_SEC_SIZE;

  /* wait period in milliseconds, value comes from the Settings class */
  uint32_t WAIT_PERIOD = 0;

  /* maximum length of SSID Name string, excluding NULL character */
  const uint8_t MAX_SSID = 32;
  
  /* maximun length of password string, excluding NULL character */
  const uint8_t MAX_PASSWORD = 32;
  
  /* SSID */
  String ssidAccessPoint = "";

  /* Password */
  String passwordAccessPoint = "";

  /* SSID */
  String ssidNetwork = "";

  /* Password */
  String passwordNetwork = "";

  /* first address for WiFiSettings storage, value comes from the Settings class */
  uint16_t address = 0;

  /* sizeof of serialized variable, marked as 'to store' */
  uint16_t storageSize;

  /* Storage size check, default false */
  bool storageSizeIsAvailable = false;

public:
  WiFiSettings(Settings * pSettings)
  {
    this->WAIT_PERIOD = pSettings->WAIT_PERIOD;
    /* Storage for AP and Network SSID, plus AP and Network Password */
    this->storageSize = 132;   // including 4 NULL characters in total (1 for each part) 

    /* if there is not enough space on EEPROM, writing will fail and reading will return an empty String */
    if (pSettings->setOffsetAddress(this->storageSize) == true)  // is there enough space on EEPROM?
    {
      this->storageSizeIsAvailable = true;
      this->address = pSettings->getOffsetAddress();
    }
  };

  ~WiFiSettings()
  {
  };

  /* saves settings in EEPROM starting on EEPROM-address (default = 0), returns length of saved bytes */
  uint16_t saveAuthorizationAccessPoint();

  /* saves settings in EEPROM starting on EEPROM-address (default = 0), returns length of saved bytes */
  uint16_t saveAuthorizationNetwork();

  /* set the Access Point SSID in the class variable */
  void setAccessPointSSID(String ssid);

  /* get the Access Point SSID from the class variable */
  String getAccessPointSSID();

  /* set the Access Point Password in the class variable */
  void setAccessPointPassword(String password);

  /* get the Access Point Password in the class variable */
  String getAccessPointPassword();

  /* set the WiFi Network SSID in the class variable */
  void setNetworkSSID(String ssid);

  /* get the WiFi Network SSID from the class variable */
  String getNetworkSSID();

  /* set the WiFi Network Password in the class variable */
  void setNetworkPassword(String password);

  /* read AccessPoint SSID from EEPROM, does NOT save it in the class variable */
  String readAccessPointSSID();

  /* read AccessPoint Password from EEPROM, does NOT save it in the class variable */
  String readAccessPointPassword();

  /* read Network SSID from EEPROM, does NOT save it in the class variable */
  String readNetworkSSID();

 /* get the WiFi Network Password in the class variable */
  String getNetworkPassword();

  /* read Network Password from EEPROM, does NOT save it in the class variable */
  String readNetworkPassword();

  /* erase settings, set value ff on every EEPROM Access Point Settings address, set class AccessPoint vaiables SSID and Password to factory value, returns true if it succeeds */
  bool eraseAccessPointSettings();

  /* erase settings, set value ff on every EEPROM Network Settings address, set class Network vaiables SSID and Password to factory value, returns true if it succeeds */
  bool eraseNetworkSettings();

  /* erase settings, set value ff on every EEPROM WiFiSettings address, set class AccessPoint and Network vaiables SSID and Password to factory value, returns true if it succeeds */
  bool eraseWiFiSettings();

private:
  /* does the erase of EEPROM addresses */
  bool eraseSettings(uint16 startAddress, uint lastAddress);

public:
  /* Used in this class program to check the availablity of storage space */
  bool getStorageSizeIsAvailable();

};

#endif