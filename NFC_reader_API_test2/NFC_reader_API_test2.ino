// Petit test pour voir comment lire une puce NFC et accéder à une API REST sur un serveur NocoDB avec un esp32-c3
// Et grande nouveauté, avec le support de OTA \o/
// ATTENTION, ce code a été écrit pour un esp32-c3 super mini. Pas testé sur les autres boards !
//
#define zVERSION "zf240517.1605"
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
https://dronebotworkshop.com/wifimanager/
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
 * RST/Reset   RST          GPIO10
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
const int zPulseDelayOn = 25;    // délai pour le blink
const int zPulseDelayOff = 50;    // délai pour le blink
const int zPulseDelayWait = 200;    // délai pour le blink
String newRFID = "00 00 00 00 00 00 00";


// WIFI
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include "secrets.h"
WiFiClient client;
HTTPClient http;

static void ConnectWiFi() {
    WiFi.mode(WIFI_STA); //Optional    
    WiFiManager wm;
    bool res;
    res = wm.autoConnect("esp32_wifi_config",""); // password protected ap
    if(!res) {
        USBSerial.println("Failed to connect");
        // ESP.restart();
    }
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
    USBSerial.print("SSID: ");
    USBSerial.println(WiFi.SSID());
    USBSerial.print("RSSI: ");
    USBSerial.println(WiFi.RSSI());
    USBSerial.print("IP: ");
    USBSerial.println(WiFi.localIP());
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

static void sendToDB(String zUidRfid, String zComment) {
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
          USBSerial.print("deserializeJson() failed: ");
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
        reqBody["UID RFID"] = zUidRfid;
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


// RFID MFRC522
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 7
#define RST_PIN 10
 
MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[7];

// start RFID
void startRFID() {
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  USBSerial.println("Prêt à scanner les tags NFC ᕗ");
}


// Lit un tag NFC 
int readRFID() {
  if (rfid.PICC_IsNewCardPresent()){
    USBSerial.println("Une carte est présente !");
    // Verify if the NUID has been readed
    if (rfid.PICC_ReadCardSerial()){
      // Halt PICC and Stop encryption on PCD
      rfid.PICC_HaltA(); rfid.PCD_StopCrypto1();
      // Get card type
      USBSerial.print("\n\n*****************\nCarte type: ");
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      USBSerial.println(rfid.PICC_GetTypeName(piccType));
      // New UID ?
      if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || 
        rfid.uid.uidByte[3] != nuidPICC[3] || rfid.uid.uidByte[4] != nuidPICC[4] || rfid.uid.uidByte[5] != nuidPICC[5] || 
        rfid.uid.uidByte[6] != nuidPICC[6] ) {
        // Store NUID into nuidPICC array
        for (byte i = 0; i < 7; i++) { nuidPICC[i] = rfid.uid.uidByte[i]; }
        convHex(rfid.uid.uidByte, rfid.uid.size);
        return(1);
      } else {
          return(3);
      }
    }
  }else {
    return(0);
  }
}


// Converti l'UID en hexa dans newRFID
void convHex(byte *buffer, byte bufferSize) {
  newRFID = "";
  for (byte i = 0; i < bufferSize; i++) {
    char hexStr[3]; // Crée un tableau temporaire pour stocker la valeur hexadécimale
    sprintf(hexStr, "%02X", buffer[i]); // Convertit la valeur en hexadécimal et la stocke dans hexStr
    newRFID += hexStr; // Concatène la valeur hexadécimale à newRFID
    newRFID += " "; // Ajoute un espace après chaque octet
  }
  newRFID.trim(); // Supprime les espaces supplémentaires à la fin de la chaîne
}




void setup() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW); delay(zPulseDelayOn); digitalWrite(ledPin, HIGH); delay(zPulseDelayOff);
    digitalWrite(ledPin, LOW); delay(zPulseDelayOn); digitalWrite(ledPin, HIGH); delay(zPulseDelayOff);
    delay(zPulseDelayWait);
    pinMode(buttonPin, INPUT_PULLUP);

    // start serial console
    USBSerial.begin(19200);
    USBSerial.setDebugOutput(true);       //pour voir les messages de debug des libs sur la console série !
    delay(3000);                          //le temps de passer sur la Serial Monitor ;-)
    USBSerial.println("\n\n\n\n**************************************\nCa commence !");
    USBSerial.println(zVERSION);

    // si le bouton FLASH de l'esp32-c3 est appuyé dans les 3 secondes après le boot, la config WIFI sera effacée !
    if ( digitalRead(buttonPin) == LOW) {
      WiFiManager wm;    
      wm.resetSettings();
      USBSerial.println("Config WIFI effacée !"); delay(1000);
      ESP.restart();
    }

    // start LED RGB
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    // start WIFI
    leds[0] = CRGB::Blue; FastLED.show();
    digitalWrite(ledPin, HIGH);
    USBSerial.println("Connect WIFI !");
    ConnectWiFi();
    leds[0] = CRGB::Green; FastLED.show();
    digitalWrite(ledPin, LOW);
    delay(200); 

    // start OTA server
    otaWebServer();

    // leds[6] = CRGB::Blue; FastLED.show();
    // USBSerial.println("Et en avant pour la connexion à la DB !");
    // USBSerial.print("\nAvec comme apiGetIndex: ");
    // USBSerial.println(apiGetIndex);
    // USBSerial.print("et comme apiPostNewRecord: ");
    // USBSerial.println(apiPostNewRecord);

    // // start API REST
    // sendToDB("C'est le boot il n'y a pas de TAG !", zVERSION);
    // digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
    // digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
    // digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
    // leds[6] = CRGB::Green; FastLED.show();
    // delay(3000); 
    // leds[6] = CRGB::Black; FastLED.show();

    // start RFID
    startRFID();

}


void loop() {
  // OTA loop
  server.handleClient();

  // Lit un tag NFC 
  leds[2] = CRGB::Blue; FastLED.show();
  int statRFID(readRFID());
  USBSerial.println(statRFID);
  if (statRFID == 1) {
    leds[2] = CRGB::Green; FastLED.show();
    USBSerial.println("Une nouvelle carte est détectée !");
    USBSerial.print("L'UID de la carte est: ");
    USBSerial.println(newRFID);
    delay(300);
    leds[2] = CRGB::Blue; FastLED.show();
  } else if (statRFID == 3) {
    leds[2] = CRGB::Orange; FastLED.show();
    USBSerial.println("Carte déjà lue !");
    delay(300);
    leds[2] = CRGB::Blue; FastLED.show();
  }




  digitalWrite(ledPin, LOW); delay(zPulseDelayOn); digitalWrite(ledPin, HIGH); delay(zPulseDelayOff);
  digitalWrite(ledPin, LOW); delay(zPulseDelayOn); digitalWrite(ledPin, HIGH); delay(zPulseDelayOff);
  delay(zPulseDelayWait);
}
