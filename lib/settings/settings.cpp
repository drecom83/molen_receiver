
#include "settings.h"

String Settings::getFirmwareVersion()
{
  return String(this->major) + "." + String(this->minor) + "." + String(this->patch);
}

String Settings::getDeviceKey()
{
  return this->deviceKey;
}

void Settings::setDeviceKey(String myDeviceKey)
{
  this->deviceKey = myDeviceKey;
}

uint16_t Settings::getSEND_PERIOD()
{
  return this->SEND_PERIOD;
}

String Settings::getTargetServer()
{
  return this->targetServer;
}

uint16_t Settings::getTargetPort()
{
  return this->targetPort;
}

String Settings::getTargetPath()
{
  return this->targetPath;
}

String Settings::getFactoryRoleModel()
{
  return this->factoryRoleModel;
}

String Settings::getRoleModel()
{
  return this->roleModel;
}

void Settings::setRoleModel(String roleModel)
{
  this->roleModel = roleModel;
  this->saveConfigurationSettings();
}

uint8_t Settings::atoi8_t(String s) 
{
    uint8_t i, n;
    n = 0;
    for (i = 0; s[i] >= '0' && s[i] <= '9'; i++)
        n = 10 * n +(s[i] - '0');
    return n;
}

uint16_t Settings::atoi16_t(String s) 
{
    uint8_t i;
    uint16_t n;
    n = 0;
    for (i = 0; s[i] >= '0' && s[i] <= '9'; i++)
        n = 10 * n +(s[i] - '0');
    return n;
}

uint32_t Settings::atoi32_t(String s) 
{
    uint8_t i;
    uint32_t n;
    n = 0;
    for (i = 0; s[i] >= '0' && s[i] <= '9'; i++)
        n = 10 * n +(s[i] - '0');
    return n;
}

uint16_t Settings::setupEEPROM()
{
  // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);
  if (!this->isInitialized())
  {
    this->initNumber = this->INITCHECK;
    return this->saveSettings();
  }
  delay(this->WAIT_PERIOD);
  return this->getSettings();
}

uint16_t Settings::setupUpdatedFirmware()
{
  uint16_t address = this->address;
  uint16_t firstAddress = this->address;
  if (this->isUpdated())
  { 
    delay(this->WAIT_PERIOD);

    EEPROM.begin(this->MAX_EEPROM_SIZE);
    //EEPROM.put(address, this->initNumber);
    address += sizeof(this->initNumber);
    EEPROM.put(address, this->major);
    address += sizeof(this->major);
    EEPROM.put(address, this->minor);
    address += sizeof(this->minor);
    EEPROM.put(address, this->patch);
    address += sizeof(this->patch);
    EEPROM.end();  // release RAM copy of EEPROM content
    
    delay(this->WAIT_PERIOD);
  }
  return firstAddress - address;
}

uint16_t Settings::saveSettings()
{
  // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);

  uint16_t firstAddress = this->address;
  uint16_t address = this->address;
 
 
  //EEPROM.begin(this->storageSize);
  EEPROM.begin(this->MAX_EEPROM_SIZE);

  //uint32_t start = micros();
  EEPROM.put(address, this->initNumber);
  address += sizeof(this->initNumber);
  EEPROM.put(address, this->major);
  address += sizeof(this->major);
  EEPROM.put(address, this->minor);
  address += sizeof(this->minor);
  EEPROM.put(address, this->patch);
  address += sizeof(this->patch);

  char myLanguage[3];  // one more for the null character
  strcpy(myLanguage, this->language.c_str());
  EEPROM.put(address, myLanguage);
  address += 3;

  EEPROM.put(address, this->startAsAccessPoint);
  address += sizeof(this->startAsAccessPoint);

  char myTargetServer[33];  // one more for the null character
  strcpy(myTargetServer, this->targetServer.c_str());
  EEPROM.put(address, myTargetServer);
  address += 33;

  EEPROM.put(address, this->targetPort);
  address += sizeof(this->targetPort);
  
  char myTargetPath[17];  // one more for the null character
  strcpy(myTargetPath, this->targetPath.c_str());
  EEPROM.put(address, myTargetPath);
  address += 17;

  char myRoleModel[33];  // one more for the null character
  strcpy(myRoleModel, this->roleModel.c_str());
  EEPROM.put(address, myRoleModel);
  address += 33;

  char myDeviceKey[37];  // one more for the null character
  strcpy(myDeviceKey, this->deviceKey.c_str());
  EEPROM.put(address, myDeviceKey);
  address += 37;

  EEPROM.commit();    // with success it will return true
  EEPROM.end();       // release RAM copy of EEPROM content

  delay(this->WAIT_PERIOD);

  return address - firstAddress;
}

bool Settings::isUpdated() {
  uint16_t address = this->address;
  delay(this->WAIT_PERIOD);
  
  EEPROM.begin(this->MAX_EEPROM_SIZE);
  uint8_t currentMajor = this->major;
  uint8_t currentMinor = this->minor;
  uint16_t currentPatch = this->patch;
  //EEPROM.get(address, this->initNumber);
  address += sizeof(this->initNumber);
  EEPROM.get(address, currentMajor);
  address += sizeof(this->major);
  EEPROM.get(address, currentMinor);
  address += sizeof(this->minor);
  EEPROM.get(address, currentPatch);
  address += sizeof(this->patch);
  EEPROM.end();  // release RAM copy of EEPROM content

  delay(this->WAIT_PERIOD);
  bool result = this->major != currentMajor &&
                this->minor != currentMinor &&
                this->patch != currentPatch;

  return result;
}

bool Settings::isInitialized() {
  delay(this->WAIT_PERIOD);
  
  EEPROM.begin(sizeof(this->initNumber));
  this->initNumber = EEPROM.read(this->address);
  EEPROM.end();  // release RAM copy of EEPROM content

  delay(this->WAIT_PERIOD);
 
  return (this->initNumber == this->INITCHECK);
}

bool Settings::eraseSettings() {
  delay(this->WAIT_PERIOD);

  //EEPROM.begin(this->storageSize);
  EEPROM.begin(this->MAX_EEPROM_SIZE);
  // replace values in EEPROM with 0xff
  for (uint16_t i = 0; i < this->storageSize; i++) {
    EEPROM.write(this->address + i,0xff);
  }
  bool result = EEPROM.commit();    // with success it will return true
  EEPROM.end();  // release RAM copy of EEPROM content

  delay(this->WAIT_PERIOD);

  return result;
}

uint16_t Settings::initSettings()
{
  // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);

  uint16_t firstAddress = this->address;
  uint16_t address = this->address;

  //EEPROM.begin(this->storageSize);
  EEPROM.begin(this->MAX_EEPROM_SIZE);

  //uint32_t start = micros();
  EEPROM.put(address, this->factoryInitNumber);
  address += sizeof(this->factoryInitNumber);
  EEPROM.put(address, this->major);
  address += sizeof(this->major);
  EEPROM.put(address, this->minor);
  address += sizeof(this->minor);
  EEPROM.put(address, this->patch);
  address += sizeof(this->patch);
  char myFactoryLanguage[3];  // one more for the null character
  strcpy(myFactoryLanguage, this->factoryLanguage.c_str());
  EEPROM.put(address, myFactoryLanguage);
  address += 3;

  EEPROM.put(address, this->factoryStartAsAccessPoint);
  address += sizeof(this->factoryStartAsAccessPoint);

  char myFactoryTargetServer[33];  // one more for the null character
  strcpy(myFactoryTargetServer, this->factoryTargetServer.c_str());
  EEPROM.put(address, myFactoryTargetServer);
  address += 33;

  EEPROM.put(address, this->factoryTargetPort);
  address += sizeof(this->factoryTargetPort);
  
  char myFactoryTargetPath[17];  // one more for the null character
  strcpy(myFactoryTargetPath, this->factoryTargetPath.c_str());
  EEPROM.put(address, myFactoryTargetPath);
  address += 17;

  char myFactoryRoleModel[33];  // one more for the null character
  strcpy(myFactoryRoleModel, this->factoryRoleModel.c_str());
  EEPROM.put(address, myFactoryRoleModel);
  address += 33;

  //uint8_t myMaxDeviceKey = sizeof(this->deviceKey);
  char myDeviceKey[37];  // one more for the null character
  strcpy(myDeviceKey, this->factoryDeviceKey.c_str());
  EEPROM.put(address, myDeviceKey);
  address += 37;

  delay(this->WAIT_PERIOD);

  EEPROM.commit();    // with success it will return true
  EEPROM.end();       // release RAM copy of EEPROM content
  delay(this->WAIT_PERIOD);

  return address - firstAddress;
}

uint16_t Settings::getSettings()
{
  // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);

  uint16_t firstAddress = this->address;
  uint16_t address = this->address;

  //EEPROM.begin(this->storageSize);
  EEPROM.begin(this->MAX_EEPROM_SIZE);
  
  EEPROM.get(address, this->initNumber);
  address += sizeof(this->initNumber);
  //EEPROM.get(address, this->version); // is done at setupFirmware
  address += sizeof(this->major);
  address += sizeof(this->minor);
  address += sizeof(this->patch);

  char myLanguage[3];  // one more for the null character
  EEPROM.get(address, myLanguage);
  this->language = String(myLanguage);
  address += 3;

  EEPROM.get(address, this->startAsAccessPoint);
  address += sizeof(this->startAsAccessPoint);

  char myTargetServer[33];  // one more for the null character
  EEPROM.get(address, myTargetServer);
  this->targetServer = String(myTargetServer);
  address += 33;

  EEPROM.get(address, this->targetPort);
  address += sizeof(this->targetPort);
  
  char myTargetPath[17];  // one more for the null character
  EEPROM.get(address, myTargetPath);
  this->targetPath = String(myTargetPath);
  address += 17;

  char myRoleModel[33];  // one more for the null character
  EEPROM.get(address, myRoleModel);
  this->roleModel = String(myRoleModel);
  address += 33;

  //uint8_t myMaxDeviceKey = sizeof(this->deviceKey);
  char mydeviceKey[37];
  EEPROM.get(address, mydeviceKey);
  this->deviceKey = String(mydeviceKey);
  address += 37;

  EEPROM.end();  // release RAM copy of EEPROM content
  delay(this->WAIT_PERIOD);

  return address - firstAddress;
}

uint16_t Settings::saveConfigurationSettings()
{
  // The function EEPROM.put() uses EEPROM.update() to perform the write, so does not rewrites the value if it didn't change.
  // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);

  uint16_t firstAddress = this->address;
  uint16_t address = this->address;
 
  //EEPROM.begin(this->storageSize);
  EEPROM.begin(this->MAX_EEPROM_SIZE);

  //EEPROM.put(address, this->initNumber);
  address += sizeof(this->initNumber);
  //EEPROM.put(address, this->version);
  address += sizeof(this->major);
  address += sizeof(this->minor);
  address += sizeof(this->patch);
  
  address += 3;  // language

  //bool check_startAsAccessPoint;
  //EEPROM.get(address, check_startAsAccessPoint);
  //if (check_startAsAccessPoint != this->startAsAccessPoint) {
    EEPROM.put(address, this->startAsAccessPoint);
  //}
  address += sizeof(this->startAsAccessPoint);
  
  //char check_myTargetServer[33];  // one more for the null character
  //EEPROM.get(address, check_myTargetServer);
  char myTargetServer[33];  // one more for the null character
  strcpy(myTargetServer, this->targetServer.c_str());
  //if (check_myTargetServer != myTargetServer) {
    EEPROM.put(address, myTargetServer);
  //}
  address += 33;

  //uint16_t check_targetPort;
  //EEPROM.get(address, check_targetPort);
  //if (check_targetPort != this->targetPort) {
    EEPROM.put(address, this->targetPort);
  //}
  address += sizeof(this->targetPort);

  //char check_myTargetPath[17];  // one more for the null character
  //EEPROM.get(address, check_myTargetPath);
  char myTargetPath[17];  // one more for the null character
  strcpy(myTargetPath, this->targetPath.c_str());
  //if (check_myTargetPath != myTargetPath) {
    EEPROM.put(address, myTargetPath);
  //}
  address += 17;

  char myRoleModel[33];  // one more for the null character
  strcpy(myRoleModel, this->roleModel.c_str());
  EEPROM.put(address, myRoleModel);
  address += 33;

  /*
  //char check_myDeviceKey[37];  // one more for the null character
  //EEPROM.get(address, check_myDeviceKey);
  char myDeviceKey[37];  // one more for the null character
  strcpy(myDeviceKey, this->deviceKey.c_str());
  //if (check_myDeviceKey != myDeviceKey) {
    EEPROM.put(address, myDeviceKey);
  //}
  */
 
///  address += 37;

  EEPROM.commit();    // with success it will return true
  EEPROM.end();       // release RAM copy of EEPROM content

  delay(this->WAIT_PERIOD);

  return address - firstAddress;
}

uint16_t Settings::getOffsetAddress()
{
  return this->addressOffset;
}

bool Settings::setOffsetAddress(uint16_t deltaAddress)
{
  if (this->getOffsetAddress() + deltaAddress > this->MAX_EEPROM_SIZE)
  {
    return false;
  }
  this->addressOffset += deltaAddress;
  return true;
}

bool Settings::beginAsAccessPoint()
{
  return this->startAsAccessPoint;
}

void Settings::beginAsAccessPoint(bool myBeginAsAccessPointValue)
{
  this->startAsAccessPoint = myBeginAsAccessPointValue;
}

String Settings::getFactoryStartModeWiFi()
{
  return this->factoryStartAsAccessPoint ? "ap" : "network";
}

String Settings::getFactoryTargetServer()
{
  return this->factoryTargetServer;
}

void Settings::setTargetServer(String targetServer)
{
  this->targetServer = targetServer;
}

uint16_t Settings::getFactoryTargetPort()
{
  return this->factoryTargetPort;
}

void Settings::setTargetPort(String targetPort)
{
  this->targetPort = this->atoi16_t(targetPort);
}

String Settings::getFactoryTargetPath()
{
  return this->factoryTargetPath;
}

void Settings::setTargetPath(String targetPath)
{
  this->targetPath = targetPath;
}

void Settings::setLanguage(String language)
{
  this->language = language;

   // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);

  uint16_t address = this->address;
 
  //EEPROM.begin(this->storageSize);
  EEPROM.begin(this->MAX_EEPROM_SIZE);
  address += sizeof(this->initNumber);
  address += sizeof(this->major);
  address += sizeof(this->minor);
  address += sizeof(this->patch);

  char myLanguage[3];  // one more for the null character
  strcpy(myLanguage, this->language.c_str());
  EEPROM.put(address, myLanguage);
  address += 3;

  EEPROM.commit();    // with success it will return true
  EEPROM.end();       // release RAM copy of EEPROM content

  delay(this->WAIT_PERIOD);
}

String Settings::getLanguage()
{
  return this->language;
}

void Settings::setLastNetworkIP(String lastNetworkIP)
{
  this->lastNetworkIP = lastNetworkIP;
}

String Settings::getLastNetworkIP()
{
  return this->lastNetworkIP;
}