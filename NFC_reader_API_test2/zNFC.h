// zf240609.1913

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

String newRFID = "00 00 00 00 00 00 00";

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


// Converti l'UID en hexa dans newRFID
void convHex(byte *buffer, byte bufferSize) {
  newRFID = "";
  for (byte i = 0; i < bufferSize; i++) {
    char hexStr[3]; // Crée un tableau temporaire pour stocker la valeur hexadécimale
    sprintf(hexStr, "%02X", buffer[i]); // Convertit la valeur en hexadécimal et la stocke dans hexStr
    newRFID += hexStr; // Concatène la valeur hexadécimale à newRFID
    // newRFID += " "; // Ajoute un espace après chaque octet
  }
  // newRFID.trim(); // Supprime les espaces supplémentaires à la fin de la chaîne
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
        return(2);
      }
    }
  }else {
    return(0);
  }
}



