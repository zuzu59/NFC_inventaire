// Tests pour lire l'UID sur une carte NFC avec le lecteur RFID RC522
// ATTENTION, ce code a été écrit pour un esp32-c3 super mini. Pas testé sur les autres boards !
//
#define zVERSION "zf240512.1619"

//
// Utilisation:
//
// Astuce:
//
// Installation:
// 
// Il faut disabled USB CDC On Boot et utiliser USBSerial. au lieu de Serial. pour la console !
//
// Pour le lecteur de NFC RFID RC522, il faut installer cette lib depuis le lib manager sur Arduino:
// https://github.com/miguelbalboa/rfid   (elle est vieille mais fonctionne encore très bien zf240510)
//
// Pour le analog buttons il faut installer ces lib: 
//https://github.com/rlogiacco/AnalogButtons
//
// Sources:
// https://randomnerdtutorials.com/security-access-using-mfrc522-rfid-reader-with-arduino/
// https://forum.fritzing.org/t/need-esp32-c3-super-mini-board-model/20561
// https://www.aliexpress.com/item/1005006005040320.html
// https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino
//
/*
 * 
 * Pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      ESP32-c3
 *             Reader/PCB   super mini
 * Signal      Pin          Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          GPIO10   ATTENTION changé de 8 à 10 le 240512.1616, car au de la LED BUILTIN
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
//
// ATTENTION, il faudra changer la pin RST pour le lecteur RFID RC522 car elle est utilisée par la led builting zf240512.1246
//
const int buttonPin = 9;  // the number of the pushbutton pin


// MFRC522
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 7
#define RST_PIN 10               // ATTENTION changé de 8 à 9 le 240512.1616, car au de la LED BUILTIN
 
MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];


// Helper routine to dump a byte array as hex values to Serial. 
void printHex(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
      USBSerial.print(buffer[i] < 0x10 ? " 0" : " ");
      USBSerial.print(buffer[i], HEX);
    }
}


// Helper routine to dump a byte array as dec values to Serial.
void printDec(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
      USBSerial.print(' ');
      USBSerial.print(buffer[i], DEC);
    }
}


void setup() { 
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
    digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);


    USBSerial.begin(19200);
    USBSerial.setDebugOutput(true);       //pour voir les messages de debug des libs sur la console série !
    delay(3000);  //le temps de passer sur la Serial Monitor ;-)
    USBSerial.println("\n\n\n\n**************************************\nCa commence !\n");

    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522 

    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
    }

    USBSerial.println(F("This code scan ALL MIFARE UID ᕗ"));
    USBSerial.print(F("Using the following key:"));
    printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
 
void loop() {

    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! rfid.PICC_IsNewCardPresent())
      return;

    // Verify if the NUID has been readed
    if ( ! rfid.PICC_ReadCardSerial())
      return;

    digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
    digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);
    digitalWrite(ledPin, HIGH); delay(200); digitalWrite(ledPin, LOW); delay(200);

    USBSerial.print(F("\n\n*****************\nPICC type: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    USBSerial.println(rfid.PICC_GetTypeName(piccType));



    if (rfid.uid.uidByte[0] != nuidPICC[0] || 
      rfid.uid.uidByte[1] != nuidPICC[1] || 
      rfid.uid.uidByte[2] != nuidPICC[2] || 
      rfid.uid.uidByte[3] != nuidPICC[3] ) {
      USBSerial.println(F("A new card has been detected."));

      // Store NUID into nuidPICC array
      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = rfid.uid.uidByte[i];
      }
    
      USBSerial.println(F("The NUID tag is:"));
      USBSerial.print(F("In hex: "));
      printHex(rfid.uid.uidByte, rfid.uid.size);
      USBSerial.println();
      USBSerial.print(F("In dec: "));
      printDec(rfid.uid.uidByte, rfid.uid.size);
      USBSerial.println();
    }
    else USBSerial.println(F("Card read previously."));

    // Halt PICC
    rfid.PICC_HaltA();

    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
}

