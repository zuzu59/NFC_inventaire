// Petit test pour voir comment lire une puce NFC et accéder à une API REST sur un serveur NocoDB avec un esp32-c3
// Et grande nouveauté, avec le support de OTA et le WIFImanager \o/
// ATTENTION, ce code a été écrit pour un esp32-c3 super mini. Pas testé sur les autres boards !
//

#define zVERSION  "zf240625.1732"
#define zHOST     "nfc_dev"            // ATTENTION, tout en minuscule !
int zDelay1Interval = 1000;       // Délais en mili secondes pour le zDelay1

// #define DEBUG true

/*
Utilisation:

Astuce:

Installation:

Pour les esp32-c3 super mini, il faut:
 * choisir comme board ESP32C3 Dev Module
 * disabled USB CDC On Boot et utiliser USBSerial. au lieu de Serial. pour la console !
 * changer le schéma de la partition à Minimal SPIFFS (1.9MB APP with OTA/190kB SPIFFS)

Pour le WiFiManager, il faut installer cette lib depuis le lib manager sur Arduino:
https://github.com/tzapu/WiFiManager

Pour le lecteur de NFC RFID RC522, il faut installer cette lib depuis le lib manager sur Arduino:
https://github.com/miguelbalboa/rfid   (elle est vieille mais fonctionne encore super bien zf240510)

Pour JSON, il faut installer cette lib:
https://github.com/bblanchon/ArduinoJson

Pour le stick LED RGB il faut installer cette lib: 
https://github.com/FastLED/FastLED    (by Daniel Garcia)

Sources:
https://forum.fritzing.org/t/need-esp32-c3-super-mini-board-model/20561
https://www.aliexpress.com/item/1005006005040320.html
https://dronebotworkshop.com/wifimanager/
https://github.com/FastLED/FastLED/blob/master/examples/Blink/Blink.ino
https://lastminuteengineers.com/esp32-ota-web-updater-arduino-ide/
https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino
https://randomnerdtutorials.com/security-access-using-mfrc522-rfid-reader-with-arduino/
https://chat.mistral.ai/    pour toute la partie API REST et wifiAuto ᕗ
*/


// #undef DEBUG


// General
const int ledPin = 8;             // the number of the LED pin
const int buttonPin = 9;          // the number of the pushbutton pin

const int ledWifi               = 0;
const int ledProcFromager       = 1;
const int ledProcAddFromage     = 2;
const int ledProcAddInventaire  = 3;
const int ledProcAddTagCmd      = 4;
const int ledProcNotation       = 5;
const int ledOk                 = 6;
const int ledFree               = 7;


// Sonar Pulse
#include "zSonarpulse.h"


// WIFI
#include "zWifi.h"


// OTA WEB server
#include "otaWebServer.h"


// Stick LED RGB
#include <FastLED.h>
#define NUM_LEDS 8
#define DATA_PIN 3
CRGB leds[NUM_LEDS];


// NFC
#include "zNFC.h"


// NocoDB
#include "zNocoDB.h"


void setup() {
  // Pulse deux fois pour dire que l'on démarre
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); delay(zSonarPulseOn); digitalWrite(ledPin, HIGH); delay(zSonarPulseOff);
  digitalWrite(ledPin, LOW); delay(zSonarPulseOn); digitalWrite(ledPin, HIGH); delay(zSonarPulseOff);
  delay(zSonarPulseWait);

  // start LED RGB
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); FastLED.clear(); FastLED.show();

  // Start serial console
  USBSerial.begin(19200);
  USBSerial.setDebugOutput(true);       //pour voir les messages de debug des libs sur la console série !
  delay(3000);                          //le temps de passer sur la Serial Monitor ;-)
  USBSerial.println("\n\n\n\n**************************************\nCa commence !"); USBSerial.println(zHOST ", " zVERSION);

  // Start WIFI
  leds[ledWifi] = CRGB::Blue; FastLED.show(); digitalWrite(ledPin, HIGH);
  USBSerial.println("Connect WIFI !");
  zStartWifi();
  leds[ledWifi] = CRGB::Green; FastLED.show(); digitalWrite(ledPin, LOW);

  // Start OTA server
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

  // On indique qu'il n'y a pas de fromager !
  leds[ledProcFromager] = CRGB::Red; FastLED.show();
}


void loop() {

  // Lit un tag NFC 
  leds[ledFree] = CRGB::Blue; FastLED.show();
  int statRFID(readRFID());
#ifdef DEBUG
  USBSerial.println(statRFID);
#endif

  switch (statRFID){
    // Une nouvelle carte est détectée !
    case 1:
      leds[ledFree] = CRGB::Green; FastLED.show();
      USBSerial.println("Une nouvelle carte est détectée !");
      USBSerial.print("L'UID de la carte est: "); USBSerial.println(newRFID);
      // La sauvegarde dans la table Tag Log
      procTagLog();
      // Traitement du tag
      toDoTag();
      delay(300);
      leds[ledFree] = CRGB::Blue; FastLED.show();
      break;

    // Carte déjà lue !
    case 2:
      leds[ledFree] = CRGB::Orange; FastLED.show();
      USBSerial.println("Carte déjà lue !");
      delay(300);
      leds[ledFree] = CRGB::Blue; FastLED.show();
      break;
  }


  // OTA loop
  server.handleClient();
  // Un petit coup sonar pulse sur la LED pour dire que tout fonctionne bien
  sonarPulse();

}

