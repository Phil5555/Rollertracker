#ifndef CACHEDSCAN_H
#define CACHEDSCAN_H

#if defined(ESP32)
  #include "WiFi.h"
  #define ORIG_SCANCLASS WiFiScanClass
#elif defined(ESP8266)
  #include "ESP8266WiFi.h"
  #define ORIG_SCANCLASS ESP8266WiFiScanClass
#endif
   

class CachedScan: public ORIG_SCANCLASS {
  public:
    void scanAndCache();
    unsigned getCachedStationCount();
    void connectToSomeStation(unsigned uTimeout);
    String serializeToString();
    String serializeToJSON();
  private:
    String serializeItemToString(uint16_t idx);
    String serializeItemToJSON(uint16_t idx);
    String mapEvilChars(String sIn);
};

#endif
