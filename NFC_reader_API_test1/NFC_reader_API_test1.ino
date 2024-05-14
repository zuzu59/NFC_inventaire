// Petit test pour voir comment lire une puce NFC et accéder à une API REST sur un serveur NocoDB avec un esp32-c3
// Et grande nouveauté, avec le support de OTA \o/
// ATTENTION, ce code a été écrit pour un esp32-c3 super mini. Pas testé sur les autres boards !
//
#define zVERSION "zf240514.1303"
/*
Utilisation:

Astuce:

Installation:

Pour les esp32-c3 super mini, il faut:
 * choisir comme board ESP32C3 Dev Module
 * disabled USB CDC On Boot et utiliser USBSerial. au lieu de Serial. pour la console !
 * changer le schéma de la partition à Minimal SPIFFS (1.9MB APP with OTA/190kB SPIFFS)

Pour le lecteur de NFC RFID RC522, il faut installer cette lib depuis le lib manager sur Arduino:
https://github.com/miguelbalboa/rfid   (elle est vieille mais fonctionne encore super bien zf240510)

Pour JSON, il faut installer cette lib:
https://github.com/bblanchon/ArduinoJson

Pour le stick LED RGB il faut installer cette lib: 
https://github.com/FastLED/FastLED    (by Daniel Garcia)

Sources:
https://randomnerdtutorials.com/security-access-using-mfrc522-rfid-reader-with-arduino/
https://forum.fritzing.org/t/need-esp32-c3-super-mini-board-model/20561
https://www.aliexpress.com/item/1005006005040320.html
https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino
https://github.com/FastLED/FastLED/blob/master/examples/Blink/Blink.ino
https://lastminuteengineers.com/esp32-ota-web-updater-arduino-ide/
https://chat.mistral.ai/    pour toute la partie API REST ᕗ
*/

/*
 * 
 * Pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      ESP32-c3
 *             Reader/PCB   super mini
 * Signal      Pin          Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          GPIO10   ATTENTION changé de 8 à 10 le 240512.1616, au lieu de la LED BUILTIN
 * SPI SS      SDA(SS)      GPIO7
 * SPI MOSI    MOSI         GPIO6
 * SPI MISO    MISO         GPIO5
 * SPI SCK     SCK          GPIO4
 *
 */


// #define DEBUG true
// #undef DEBUG


// General
const int ledPin = 8;    // the number of the LED pin
const int buttonPin = 9;  // the number of the pushbutton pin
float rrsiLevel = 0;      // variable to store the RRSI level
const int zPulseDelayOn = 100;    // délai pour le blink
const int zPulseDelayOff = 200;    // délai pour le blink
const int zPulseDelayWait = 500;    // délai pour le blink


// WIFI
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"
WiFiClient client;
HTTPClient http;

static void ConnectWiFi() {
    USBSerial.printf("WIFI_SSID: %s\nWIFI_PASSWORD: %s\n", WIFI_SSID, WIFI_PASSWORD);
    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);  //c'est pour le Lolin esp32-c3 mini V1 ! https://www.wemos.cc/en/latest/c3/c3_mini_1_0_0.html
    int txPower = WiFi.getTxPower();
    USBSerial.print("TX power: ");
    USBSerial.println(txPower);
    USBSerial.println("Connecting");
    while(WiFi.status() != WL_CONNECTED){
        USBSerial.print(".");
        delay(100);
    }
    USBSerial.println("\nConnected to the WiFi network");
    USBSerial.print("Local ESP32 IP: ");
    USBSerial.println(WiFi.localIP());
    USBSerial.print("RSSI: ");
    USBSerial.println(WiFi.RSSI());
}


// OTA WEB server
const char* host = "esp32-c3";
#include "otaWebServer.h"


// Stick LED RGB
#include <FastLED.h>
#define NUM_LEDS 8
#define DATA_PIN 3
CRGB leds[NUM_LEDS];


// API JSON
#include <ArduinoJson.h>
const char* token = apiToken;
const char* apiGetIndex = apiServerName "/api/v2/tables/mccwrj43jwtogvs/records?viewId=vwwm6yz0uhytc9er&fields=Index&sort=-Index&limit=1&shuffle=0&offset=0";
const char* apiPostNewRecord = apiServerName "/api/v2/tables/mccwrj43jwtogvs/records";

static void sendToDB(const char * zComment) {
  if (WiFi.status() == WL_CONNECTED) {
    // Efectuer la requête GET pour récupérer l'Index du dernier enregistrement
    http.begin(apiGetIndex);
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
        reqBody["Commentaire"] = zComment;
        reqBody["RSSI"] = WiFi.RSSI();
        reqBody["IP"] = WiFi.localIP();

        String jsonReqBody;
        serializeJson(reqBody, jsonReqBody);
        // Effectuer la requête POST pour créer le nouvel enregistrement
        http.begin(apiPostNewRecord);
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
}



void setup() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
    digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);

    // start serial console
    USBSerial.begin(19200);
    USBSerial.setDebugOutput(true);       //pour voir les messages de debug des libs sur la console série !
    delay(3000);                          //le temps de passer sur la Serial Monitor ;-)
    USBSerial.println("\n\n\n\n**************************************\nCa commence !");
    USBSerial.println(zVERSION);

    // start LED RGB
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    // start WIFI
    leds[7] = CRGB::Blue; FastLED.show();
    digitalWrite(ledPin, HIGH);
    USBSerial.println("Connect WIFI !");
    ConnectWiFi();
    leds[7] = CRGB::Green; FastLED.show();
    digitalWrite(ledPin, LOW);
    delay(200); 

    // start OTA server
    otaWebServer();

    leds[1] = CRGB::Blue; FastLED.show();
    USBSerial.println("Et en avant pour la connexion à la DB !");
    USBSerial.print("\nAvec comme apiGetIndex: ");
    USBSerial.println(apiGetIndex);
    USBSerial.print("et comme apiPostNewRecord: ");
    USBSerial.println(apiPostNewRecord);

    sendToDB(zVERSION);
    digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
    digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
    digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
    leds[1] = CRGB::Green; FastLED.show();
    delay(3000); 
    leds[1] = CRGB::Black; FastLED.show();
}


void loop() {
  // OTA loop
  server.handleClient();

  digitalWrite(ledPin, LOW); delay(zPulseDelayOn); digitalWrite(ledPin, HIGH); delay(zPulseDelayOff);
  digitalWrite(ledPin, LOW); delay(zPulseDelayOn); digitalWrite(ledPin, HIGH); delay(zPulseDelayOff);
  delay(zPulseDelayWait);
}
