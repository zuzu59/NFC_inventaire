// Petit test pour voir comment accéder à une API REST sur un serveur NocoDB avec un esp32-c3
// ATTENTION, ce code a été écrit pour un esp32-c3 super mini. Pas testé sur les autres boards !
//
#define zVERSION "zf240512.0953"

//



#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
#include <ArduinoJson.h>

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

        // Allouer un objet DynamicJsonDocument pour stocker le JSON
        DynamicJsonDocument doc(1024);

        // Désérialiser le JSON
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          USBSerial.print(F("deserializeJson() failed: "));
          USBSerial.println(error.f_str());
          return;
        }

        // Récupérer le champ "Index" et l'incrémenter
        long index = doc["list"][0]["Index"].as<long>() + 1;
        USBSerial.print("Index incremented: ");
        USBSerial.println(index);



        // Créer le corps de la requête POST
        StaticJsonDocument<200> reqBody;
        reqBody["Index"] = index;
        reqBody["Commentaire"] = "toto";

        String jsonReqBody;
        serializeJson(reqBody, jsonReqBody);

        // Effectuer la requête POST
        http.addHeader("Content-Type", "application/json");
        http.addHeader("xc-token", token);

        httpCode = http.POST(jsonReqBody);

        if (httpCode > 0) {
          USBSerial.printf("POST request response code: %d\n", httpCode);

          if (httpCode == HTTP_CODE_OK) {
            String response = http.getString();
            USBSerial.println("POST request response:");
            USBSerial.println(response);
          }
        } else {
          USBSerial.println("Error on POST request");
        }










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
