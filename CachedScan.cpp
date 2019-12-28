#include "CachedScan.h"
#include "config.h"



void CachedScan::scanAndCache() {
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.scanDelete();
  WiFi.scanNetworks();
}


unsigned CachedScan::getCachedStationCount() {
  return _scanCount;
}


void CachedScan::connectToSomeStation(unsigned uTimeout) {
  unsigned uStop=millis()+uTimeout*1000;
  unsigned uLoopCount=0;
  unsigned uNetwork=0;
  /*Try to use known Wifi first:*/
  #ifdef PREFERED_WIFI_PASSWORD
    WiFi.begin(PREFERED_WIFI_SSID, PREFERED_WIFI_PASSWORD);
  #else
    WiFi.begin(PREFERED_WIFI_SSID);
  #endif
  Serial.println("");
  Serial.print("  ");
  Serial.printf("%32s",PREFERED_WIFI_SSID);
  Serial.print(" ");
  
  while(WiFi.status() != WL_CONNECTED ||
        millis() < uTimeout ) {
    if(uLoopCount>30) {
      uLoopCount=10;
      /*Use available, unencrypted networks if connection does not work.*/
      for(;uNetwork<_scanCount;++uNetwork) {
        if(WiFi.encryptionType(uNetwork)==WIFI_AUTH_OPEN) {
           Serial.println("x");
           Serial.print("  ");
           Serial.printf("%32s",WiFi.SSID(uNetwork).c_str());
           Serial.print(" ");
           WiFi.disconnect();
           WiFi.begin(WiFi.SSID(uNetwork).c_str());
           ++uNetwork;
           break;
        }
      }
      if(uNetwork>=_scanCount) {
        break; /*No more networks to try*/
      }
    }
    ++uLoopCount;
    delay(1000);
    Serial.print("."); 
  }
}

String CachedScan::serializeToString() {
  String sRet;
  if(_scanCount) {
    for(uint16_t i=0;i<_scanCount;++i) {
      sRet+=serializeItemToString(i)+String("\n");
    }
  } else {
    sRet="No WiFi in reach";
  }
  return sRet;
}

String CachedScan::serializeItemToString(uint16_t idx) {
  String sRet;
  if(idx<_scanCount) {
    sRet+=WiFi.SSID(idx);
    sRet+=" (rssi=";
    sRet+=String(WiFi.RSSI(idx));
    sRet+=", bssid=";
    sRet+=WiFi.BSSIDstr(idx);
    sRet+=", channel=";
    sRet+=WiFi.channel(idx);
    switch (WiFi.encryptionType(idx)) {
      case (WIFI_AUTH_OPEN):
        sRet+=", unencrypted)";
        break;
      case (WIFI_AUTH_WEP):
        sRet+=", encryption=WEP)";
        break;
      case (WIFI_AUTH_WPA_PSK):
        sRet+=", encryption=WPA_PSK)";
        break;
      case (WIFI_AUTH_WPA2_PSK):
        sRet+=", encryption=WPA2_PSK)";
        break;
      case (WIFI_AUTH_WPA_WPA2_PSK):
        sRet+=", encryption=WPA_WPA2_PSK)";
        break;
      case (WIFI_AUTH_WPA2_ENTERPRISE):
        sRet+=", encryption=WPA2_ENTERPRISE)";
        break;
      default:
        sRet+=", encryption=unknown)";
        break;
    }
    sRet+=")";
  }
  return sRet;
}

String CachedScan::serializeToJSON() {
  String sRet="[";
  if(_scanCount) {
    for(uint16_t i=0;i<_scanCount;++i) {
      sRet+=serializeItemToJSON(i);
      if(i+1<_scanCount) {
        sRet+=",\n";
      } else {
        sRet+=String("\n");
      }
    }
  }
  sRet+="]";
  return sRet;
}

String CachedScan::serializeItemToJSON(uint16_t idx) {
  String sRet;
  if(idx<_scanCount) {
    sRet+="{\"bssid\":\"";
    sRet+=WiFi.BSSIDstr(idx);
    sRet+="\",\"ssid\":\"";
    sRet+=mapEvilChars(WiFi.SSID(idx));
    sRet+="\",\"level\":";
    sRet+=String(WiFi.RSSI(idx));
    sRet+=",\"frequency\":";
    sRet+=String(2407+WiFi.channel(idx)*5);
    sRet+="}";
  }
  return sRet;
}

String CachedScan::mapEvilChars(String sIn) {
  return sIn;
}
