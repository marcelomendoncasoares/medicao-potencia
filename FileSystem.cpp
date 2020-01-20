
// Code by Marcelo Soares and Douglas Quintanilha
// Released to the public domain


/*----------------------------------------------------------------------------
 *  Implementation of the class FileSystem
 *  SD Card file system management
 */

#include "FileSystem.h"


//==============================================================================
// Return date and time using FAT_DATE macro to format fields
//
static void FileSystem::FATDateTime(uint16_t* date, uint16_t* time) {
  *date = FAT_DATE(timeCounter.getYear(), timeCounter.getMonth(), timeCounter.getDay());
  *time = FAT_TIME(timeCounter.getHour(), timeCounter.getMinutes(), timeCounter.getSeconds());
}
//------------------------------------------------------------------------------


//==============================================================================
// Initialize the SD Card module
//
bool FileSystem::begin() {
  return sd.begin();
}
//------------------------------------------------------------------------------


//==============================================================================
// Save active session parameters for autoconfig on RESET
//
bool FileSystem::saveActiveSession(char* dir, char* fileName) {

  File autoconfigFile;
  ArduinoOutStream fileStream(autoconfigFile);

  SdFile::dateTimeCallback(FATDateTime);
  autoconfigFile = sd.open(fileName, O_RDWR | O_CREAT);
  if (!autoconfigFile) {
    return false;
  }

  fileStream << dir;
  autoconfigFile.close();
  return true;
}
//------------------------------------------------------------------------------


//==============================================================================
// Delete autoconfig file
//
bool FileSystem::deleteAutoconfigFile(char* fileName) {

  // Store active directory name
  char dir[20];
  sd.vwd()->getName(dir, sizeof(dir));
  return (changeDir("/") && sd.remove(fileName) && changeDir(dir));
}
//------------------------------------------------------------------------------


//==============================================================================
// Restore saved session, if any
//
bool FileSystem::restoreSession(char* fileName) {

  File autoconfigFile;
  autoconfigFile = sd.open(fileName, O_RDONLY);
  if (!autoconfigFile) {
    return false;
  }

  uint8_t nBytes, i;
  nBytes = autoconfigFile.available();
  char restoreDirectory[nBytes + 1];

  for (i = 0; i < nBytes; ++i) {
    restoreDirectory[i] = char(autoconfigFile.read());
    if (restoreDirectory[i] == '\n' || restoreDirectory[i] == '\r') {
      break;
    }
  }
  restoreDirectory[i] = '\0';
  autoconfigFile.close();

  // ArduinoInStream fileStream(autoconfigFile, restoreDirectory, sizeof(restoreDirectory));
  // fileStream.readline();
  // autoconfigFile.close();
  // fileStream >> restoreDirectory;

  return changeDir(restoreDirectory);
}
//------------------------------------------------------------------------------


//==============================================================================
// Navigate to specified folder
//
bool FileSystem::changeDir (char* dir) {
  return sd.chdir(dir);
}
//------------------------------------------------------------------------------


//==============================================================================
// Create and navigate do folder
//
bool FileSystem::makeDir(char* dir) {
  
  if (!changeDir("/")) {
    return false;
  }
  if (!sd.exists(dir)) {
    SdFile::dateTimeCallback(FATDateTime);
    return sd.mkdir(dir);
  }
  return true;
}
//------------------------------------------------------------------------------


//==============================================================================
// Open file and record the calculated values on the SD Card
//
bool FileSystem::recordValues(char* fileName, Measure* measure) {

  File dataFile;
  ArduinoOutStream fileStream(dataFile);

  SdFile::dateTimeCallback(FATDateTime);
  dataFile = sd.open(fileName, O_RDWR | O_CREAT | O_AT_END);
  if (!dataFile) {
    return false;
  }

  fileStream << timeCounter.getDate() << COMMA << timeCounter.getTime() << COMMA << setprecision(4);
  fileStream << measure->getCurrentRMS() << COMMA << measure->getVoltageRMS() << COMMA;
  fileStream << measure->getRealPower() << COMMA << measure->getApparentPower() << COMMA << measure->getPowerFactor() << COMMA;
  fileStream << measure->getLastPeriod() << endl;
  dataFile.close();

  return true;
}
//------------------------------------------------------------------------------


//==============================================================================
// Transfer specified file 
//
bool FileSystem::transferFile(char* fileName, ArduinoOutStream* cout, Communicate* communicate) {
  
  File dataFile;

  dataFile = sd.open(fileName, O_RDONLY);
  if (dataFile) {
    *cout << DATA_HEADER << endl;
    while (dataFile.available() && communicate->isDeviceConnected()) {
      *cout << char(dataFile.read());
    }
    dataFile.close();
    return true;
  }

  return false;
}
//------------------------------------------------------------------------------


//==============================================================================
// Print free space on SD Card
//
void FileSystem::printFreeSpace(ArduinoOutStream* cout) {
  float freeSpace = 0.000512 * sd.vol()->freeClusterCount() * sd.vol()->blocksPerCluster();
  float cardSize = 0.000512 * sd.card()->cardSize();
  *cout << setprecision(3);
  *cout << F("Free Space: ") << freeSpace << F(" MB") << endl;
  *cout << F("Card Size: ") << cardSize << F(" MB") << endl;
  *cout << (1 - freeSpace/cardSize) * 100 << F("\% space used") << endl;
}
//------------------------------------------------------------------------------


//==============================================================================
// Similar to LS on Linux
//
void FileSystem::listFiles(Stream* commPort) {
  char activeDirectory[20];
  sd.vwd()->getName(activeDirectory, sizeof(activeDirectory));
  changeDir("/");
  sd.ls(commPort, 0xFF);
  changeDir(activeDirectory);
}
//------------------------------------------------------------------------------


//==============================================================================
// Wipe files from already formatted SD Card and reset module
//
bool FileSystem::wipeSDCard(Stream* commPort) {
  return (sd.wipe(commPort) && begin());
}
//------------------------------------------------------------------------------
