/*0
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is almost the same as with the WiFi Shield library,
 *  the most obvious difference being the different file you need to include:
 */
#include "WiFi.h"
#include "CachedScan.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <MicroNMEA.h>
#include "config.h"
#include "LocationStatus.h"
#include "LEDController.h"
#include "ArduinoJson.h"

CachedScan scan;

const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFjTCCA3WgAwIBAgIRANOxciY0IzLc9AUoUSrsnGowDQYJKoZIhvcNAQELBQAw\n" \
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTYxMDA2MTU0MzU1\n" \
"WhcNMjExMDA2MTU0MzU1WjBKMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n" \
"RW5jcnlwdDEjMCEGA1UEAxMaTGV0J3MgRW5jcnlwdCBBdXRob3JpdHkgWDMwggEi\n" \
"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCc0wzwWuUuR7dyXTeDs2hjMOrX\n" \
"NSYZJeG9vjXxcJIvt7hLQQWrqZ41CFjssSrEaIcLo+N15Obzp2JxunmBYB/XkZqf\n" \
"89B4Z3HIaQ6Vkc/+5pnpYDxIzH7KTXcSJJ1HG1rrueweNwAcnKx7pwXqzkrrvUHl\n" \
"Npi5y/1tPJZo3yMqQpAMhnRnyH+lmrhSYRQTP2XpgofL2/oOVvaGifOFP5eGr7Dc\n" \
"Gu9rDZUWfcQroGWymQQ2dYBrrErzG5BJeC+ilk8qICUpBMZ0wNAxzY8xOJUWuqgz\n" \
"uEPxsR/DMH+ieTETPS02+OP88jNquTkxxa/EjQ0dZBYzqvqEKbbUC8DYfcOTAgMB\n" \
"AAGjggFnMIIBYzAOBgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADBU\n" \
"BgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEBATAwMC4GCCsGAQUFBwIB\n" \
"FiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQub3JnMB0GA1UdDgQWBBSo\n" \
"SmpjBH3duubRObemRWXv86jsoTAzBgNVHR8ELDAqMCigJqAkhiJodHRwOi8vY3Js\n" \
"LnJvb3QteDEubGV0c2VuY3J5cHQub3JnMHIGCCsGAQUFBwEBBGYwZDAwBggrBgEF\n" \
"BQcwAYYkaHR0cDovL29jc3Aucm9vdC14MS5sZXRzZW5jcnlwdC5vcmcvMDAGCCsG\n" \
"AQUFBzAChiRodHRwOi8vY2VydC5yb290LXgxLmxldHNlbmNyeXB0Lm9yZy8wHwYD\n" \
"VR0jBBgwFoAUebRZ5nu25eQBc4AIiMgaWPbpm24wDQYJKoZIhvcNAQELBQADggIB\n" \
"ABnPdSA0LTqmRf/Q1eaM2jLonG4bQdEnqOJQ8nCqxOeTRrToEKtwT++36gTSlBGx\n" \
"A/5dut82jJQ2jxN8RI8L9QFXrWi4xXnA2EqA10yjHiR6H9cj6MFiOnb5In1eWsRM\n" \
"UM2v3e9tNsCAgBukPHAg1lQh07rvFKm/Bz9BCjaxorALINUfZ9DD64j2igLIxle2\n" \
"DPxW8dI/F2loHMjXZjqG8RkqZUdoxtID5+90FgsGIfkMpqgRS05f4zPbCEHqCXl1\n" \
"eO5HyELTgcVlLXXQDgAWnRzut1hFJeczY1tjQQno6f6s+nMydLN26WuU4s3UYvOu\n" \
"OsUxRlJu7TSRHqDC3lSE5XggVkzdaPkuKGQbGpny+01/47hfXXNB7HntWNZ6N2Vw\n" \
"p7G6OfY+YQrZwIaQmhrIqJZuigsrbe3W+gdn5ykE9+Ky0VgVUsfxo52mwFYs1JKY\n" \
"2PGDuWx8M6DlS6qQkvHaRUo0FMd8TsSlbF0/v965qGFKhSDeQoMpYnwcmQilRh/0\n" \
"ayLThlHLN81gSkJjVrPI0Y8xCVPB4twb1PFUd2fPM3sA1tJ83sZ5v8vgFv2yofKR\n" \
"PB0t6JzUA81mSqM3kxl5e+IZwhYAyO0OTg3/fs8HqGTNKd9BqoUwSRBzp06JMg5b\n" \
"rUCGwbCUDI0mxadJ3Bz4WxR6fyNpBK2yAinWEsikxqEt\n" \
"-----END CERTIFICATE-----\n";

HardwareSerial& gps = Serial2;
char nmeaBuffer[512];
StaticJsonDocument<2048> jsonDoc;
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));
LocationStatus locationStatus;
unsigned long uLastUpdate;
 
void processGPSData(void *) {
  while(true) {
    while (gps.available()) {
      char c = gps.read();
      nmea.process(c);
    }
    delay(1);
  }
}

void setup()
{
  locationStatus=INVALID;
  uLastUpdate=0;
  
  Serial.begin(115200);
  Serial.println("Setting up GPS");
  gps.begin(9600); // gps
  // Clear the list of messages which are sent.
  MicroNMEA::sendSentence(gps, "$PORZB");

  // Send only RMC and GGA messages.
  MicroNMEA::sendSentence(gps, "$PORZB,RMC,1,GGA,1");

  // Disable compatability mode (NV08C-CSM proprietary message) and
  // adjust precision of time and position fields
  MicroNMEA::sendSentence(gps, "$PNVGNME,2,9,1");
  // MicroNMEA::sendSentence(gps, "$PONME,2,4,1,0");

  xTaskCreatePinnedToCore( processGPSData, /* Task function. */
                           "GPSProcessing",  /* String with name of task. */
                           10000,            /* Stack size in words. */
                           NULL,             /* Parameter passed as input of the task */
                           1,                /* Priority of the task. */
                           NULL,             /* Task handle. */
                           1);               /*core id*/

  Serial.println("Activating LEDs");
  setupLEDController();
  Serial.println("Setup complete");
}

void loop()
{
  String sJSONStations;
  int i=0;
  Serial.println("Starting a new scan");
  locationStatus=SCANNING;
  scan.scanAndCache();
  sJSONStations=scan.serializeToJSON();
  Serial.println(sJSONStations);

  Serial.print("Waiting for WiFi to connect again ...");
  locationStatus=CONNECT;
  scan.connectToSomeStation(30);
  Serial.println("done");

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("No working WiFi found.");
    delay(1000);
    return;
  }

  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  locationStatus=REQUESTING;
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    client -> setCACert(rootCACertificate);
    {
      HTTPClient https;
      String sURL=LOCATE_URL;

      if(nmea.isValid()) {
        long lLat = nmea.getLatitude();
        long lLon = nmea.getLongitude();
        char cBuf[20];
        snprintf (cBuf, sizeof(cBuf), "%f", lLat);
        sURL+="?lat=";
        sURL+=cBuf;
      
        snprintf (cBuf, sizeof(cBuf), "%f", lLon);
        sURL+="&lon=";
        sURL+=cBuf;
      }
      
      Serial.println(sURL);
      if (https.begin(*client, sURL)) {
        https.addHeader("Content-Type", "application/json");
        int httpCode = https.POST(sJSONStations);
        if (httpCode > 0) {
          Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            Serial.println(payload);

            /*Parse answer:*/
            auto jsonError = deserializeJson(jsonDoc, payload.c_str());
            if(!jsonError) {
              const char * cLocId = jsonDoc["location"]["id"];
              Serial.print("Location Id: ");
              Serial.println(cLocId);
              if(strlen(cLocId)) {
                locationStatus=UPDATING;
                Serial.println("Constructing location update request");
                DynamicJsonDocument jsonRequest(3);
                jsonRequest["coordinates_id"] = cLocId;
                jsonRequest["secret"] = POSITION_SECRET;
                String sReq;
                serializeJson(jsonRequest, sReq);
                Serial.println(sReq.c_str());

                HTTPClient httpsReq;
                String sURLReq=POSITION_URL;
                Serial.print("Starting httpsReq client ...");
                if (httpsReq.begin(*client, sURLReq)) {
                  Serial.println(" ok");
                  httpsReq.addHeader("Content-Type", "application/json");
                  Serial.print("Requesting update ... ");
                  httpCode = httpsReq.PUT(sReq);
                  Serial.println(httpCode);
                  uLastUpdate=millis();
                } else {
                  Serial.println(" failed");
                }
              }
            }
          }
        } else {
          Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
        https.end();
      } else {
        Serial.printf("[HTTPS] Unable to connect\n");
      }
    }
  
    delete client;
  }

  if(uLastUpdate==0 ||
     uLastUpdate+POSITION_CONSIDERED_INVALID_MS < millis()) {
    locationStatus=INVALID;
  } else {
    locationStatus=UPDATED;
  }

  // Wait a bit before scanning again
  delay(60*1000);
}
