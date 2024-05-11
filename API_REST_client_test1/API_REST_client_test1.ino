// Petit test pour voir comment accéder à une API REST sur un serveur NocoDB avec un esp32-c3
// ATTENTION, ce code a été écrit pour un esp32-c3 super mini. Pas testé sur les autres boards !
//
#define zVERSION "zf240511.1949"

//



#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// const char* serverName = apiServerName "/api/v2/tables/mccwrj43jwtogvs/records?limit=25&shuffle=0&offset=0";
const char* serverName = apiServerName "/api/v2/tables/mccwrj43jwtogvs/records?viewId=vwwm6yz0uhytc9er&fields=Index&sort=-Index&limit=1&shuffle=0&offset=0";



String token = apiToken;

void setup() {
    USBSerial.begin(19200);
    USBSerial.setDebugOutput(true);       //pour voir les messages de debug des libs sur la console série !
    delay(3000);  //le temps de passer sur la Serial Monitor ;-)
    USBSerial.println("\n\n\n\n**************************************\nCa commence !\n");

    USBSerial.print("serverName: ");
    USBSerial.println(serverName);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      USBSerial.println("Connecting to WiFi...");
    }

    USBSerial.println("Connected to WiFi");

}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);
    http.addHeader("accept", "application/json");
    http.addHeader("xc-token", token);

    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        USBSerial.println(payload);
      } else {
        USBSerial.printf("Error on HTTP request: %d\n", httpCode);
      }
    } else {
      USBSerial.println("Error on HTTP request");
    }

    http.end();
  } else {
    USBSerial.println("Error in WiFi connection");
  }

  delay(50000);
}
