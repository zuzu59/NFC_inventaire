// Petit test pour voir comment fonctionne l'OTA avec un esp32-c3
// ATTENTION, ce code a été écrit pour un esp32-c3 super mini. Pas testé sur les autres boards !
//
#define zVERSION "zf240513.21.3359"
/*
Utilisation:

Astuce:

Installation:

Il faut disabled USB CDC On Boot et utiliser USBSerial. au lieu de Serial. pour la console !

Pour le lecteur de NFC RFID RC522, il faut installer cette lib depuis le lib manager sur Arduino:
https://github.com/miguelbalboa/rfid   (elle est vieille mais fonctionne encore super bien zf240510)

*/



// #define DEBUG true
// #undef DEBUG



// General
const int ledPin = 8;    // the number of the LED pin
const int zPulseDelayOn = 100;    // délai pour le blink
const int zPulseDelayOff = 200;    // délai pour le blink
const int zPulseDelayWait = 1000;    // délai pour le blink

//
// ATTENTION, il faudra changer la pin RST pour le lecteur RFID RC522 car elle est utilisée par la led builting zf240512.1246
//
const int buttonPin = 9;  // the number of the pushbutton pin
float rrsiLevel = 0;      // variable to store the RRSI level


#include "secrets.h"


#include <WiFi.h>
#include <WiFiClient.h>

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* host = "esp32-c3";



//
// OTA WEB server
//
#include "otaWebServer.h"




void setup(void) {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
  digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);

  USBSerial.begin(19200);
  USBSerial.setDebugOutput(true);       //pour voir les messages de debug des libs sur la console série !
  delay(3000);  //le temps de passer sur la Serial Monitor ;-)
  USBSerial.println("\n\n\n\n**************************************\nCa commence!");
  USBSerial.println(zVERSION);

  

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  USBSerial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    USBSerial.print(".");
  }
  USBSerial.println("");
  USBSerial.print("Connected to ");
  USBSerial.println(ssid);
  USBSerial.print("IP address: ");
  USBSerial.println(WiFi.localIP());

  // start OTA server
  otaWebServer();
}


void loop(void) {
  // OTA loop
  server.handleClient();

  digitalWrite(ledPin, LOW); delay(zPulseDelayOn); digitalWrite(ledPin, HIGH); delay(zPulseDelayOff);
  digitalWrite(ledPin, LOW); delay(zPulseDelayOn); digitalWrite(ledPin, HIGH); delay(zPulseDelayOff);
  delay(zPulseDelayWait);
  
}
